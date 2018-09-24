#include "Arduino.h"
#include "ManageMPPT.h"

enum charger_mode {off, on, bulk, bat_float} charger_state;    // enumerated variable that holds state for charger state machine

//------------------------------------------------------------------------------------------------------
// This is interrupt service routine for Timer1 that occurs every 20uS.
//
//------------------------------------------------------------------------------------------------------

unsigned int seconds = 0;             // seconds from timer routine
unsigned int interrupt_counter = 0;   // counter for 20us interrrupt

void callback()
{
  if (interrupt_counter++ > ONE_SECOND) {        // increment interrupt_counter until one second has passed
    interrupt_counter = 0;                       // reset the counter
    seconds++;                                   // then increment seconds counter
  }
}

//------------------------------------------------------------------------------------------------------
// This routine reads and averages the analog inputs for this system, solar volts, solar amps and
// battery volts.
//------------------------------------------------------------------------------------------------------

double read_adc(int channel){

  double sum = 0;
  double temp;
  double NewTemp;
  int i;

  for (i=0; i<AVG_NUM; i++) {          // loop through reading raw adc values AVG_NUM number of times
    temp = analogRead(channel);        // read the input pin
 /*Serial.println("Temp (Mapped to 5 Volts= ");
  NewTemp = temp*5/1024;                //this commented out section is to showo adc values
  Serial.println(NewTemp);
  Serial.println("      ");
 */
    sum += temp;                       // store sum for averaging
    delayMicroseconds(1000);             // pauses
  }
  return(sum / AVG_NUM);               // divide sum by AVG_NUM to get average and return it
}

//------------------------------------------------------------------------------------------------------
// This routine uses the Timer1.pwm function to set the pwm duty cycle.
//------------------------------------------------------------------------------------------------------
void ManageMPPT::set_pwm_duty(void) {

  if (pwm > PWM_MAX) {             // check limits of PWM duty cyle and set to PWM_MAX
    pwm = PWM_MAX;
  }
  else if (pwm < PWM_MIN) {          // if pwm is less than PWM_MIN then set it to PWM_MIN
    pwm = PWM_MIN;
  }
  if (pwm < PWM_MAX) {
    Timer1.pwm(PWM_PIN,(PWM_FULL * (long)pwm / 100), 20);  // use Timer1 routine to set pwm duty cycle at 20uS period
    //Timer1.pwm(PWM_PIN,(PWM_FULL * (long)pwm / 100));
  }
  else if (pwm == PWM_MAX) {           // if pwm set to 100% it will be on full but we have
    Timer1.pwm(PWM_PIN,(PWM_FULL - 1), 20);                // keep switching so set duty cycle at 99.9%
    //Timer1.pwm(PWM_PIN,(PWM_FULL - 1));
  }
}


ManageMPPT::ManageMPPT(){
  sol_amps = 0;                       // solar amps
  sol_volts = 0;                      // solar volts before
  bat_volts = 0;                      // battery volts
  in_bat_amps = 0;                   // amps going into battery
  sol_watts = 0;                      // solar watts
  old_sol_watts = 0;              // solar watts from previous time through ppt routine
  prev_seconds = 0;        // seconds value from previous pass
  delta = PWM_INC;                  // variable used to modify pwm duty cycle for the ppt algorithm

  pinMode(PWM_ENABLE_PIN, OUTPUT);     // sets the digital pin as output
  TURN_OFF_MOSFETS;                    // turn off MOSFET driver chip
  charger_state = off;

  Timer1.initialize(20);               // initialize timer1, and set a 20uS period
  Timer1.pwm(PWM_PIN, 0);              // setup pwm on pin 9, 0% duty cycle
  Timer1.attachInterrupt(callback);    // attaches callback() as a timer overflow interrupt
  pwm = PWM_START;                          // pwm duty cycle 0-100%
}

//------------------------------------------------------------------------------------------------------
// This routine reads all the analog input values for the system. Then it multiplies them by the scale
// factor to get actual value in volts or amps.
//-----------------------------

void ManageMPPT::read_data(void) {

sol_volts = read_adc(SOL_VOLTS_CHAN) * SOL_VOLTS_SCALE;          //input of solar volts
//sol_amps = (sol_volts - sol_voltsb)/R;
bat_volts = read_adc(BAT_VOLTS_CHAN) * BAT_VOLTS_SCALE;          //input of battery volts

//in_bat_amps = ((read_adc(IN_BAT_AMPS_CHAN)) * IN_BAT_AMPS_SCALE - 13.425) ;// calculation gives wrong values, -13.43 for zeroing
                                                                //input of solar amps
sol_watts = sol_volts * sol_amps;


if (sol_volts > 10) {
  sol_amps = 1.5;
}
else if ( sol_volts < 9.99 && sol_volts > 7.1) {
  sol_amps = 1;
}
else if ( sol_volts < 7 && sol_volts > 5.1) {
  sol_amps = 0.77;
}
else if ( sol_volts < 5 && sol_volts > 3.1) {
  sol_amps = 0.48;
}
else if ( sol_volts < 3) {
  sol_amps = 0;
  }
}

//------------------------------------------------------------------------------------------------------
// This routine is the charger state machine. It has four states on, off, bulk and float.
// It's called once each time through the main loop to see what state the charger should be in.
// The battery charger can be in one of the following four states:
//
//  On State - this is charger state for MIN_SOL_WATTS < solar watts < LOW_SOL_WATTS. In this state isthe solar
//      watts input is too low for the bulk charging state but not low enough to go into the off state.
//      In this state we just set the pwm = 99.9% to get the most of low amount of power available.

//  Bulk State - this is charger state for solar watts > MIN_SOL_WATTS. This is where we do the bulk of the battery
//      charging and where we run the Peak Power Tracking alogorithm. In this state we try and run the maximum amount
//      of current that the solar panels are generating into the battery.

//  Float State - As the battery charges it's voltage rises. When it gets to the MAX_BAT_VOLTS we are done with the
//      bulk battery charging and enter the battery float state. In this state we try and keep the battery voltage
//      at MAX_BAT_VOLTS by adjusting the pwm value. If we get to pwm = 100% it means we can't keep the battery
//      voltage at MAX_BAT_VOLTS which probably means the battery is being drawn down by some load so we need to back
//      into the bulk charging mode.

//  Off State - This is state that the charger enters when solar watts < MIN_SOL_WATTS. The charger goes into this
//      state when there is no more power being generated by the solar panels. The MOSFETs are turned
//      off in this state so that power from the battery doesn't leak back into the solar panel.
//------------------------------------------------------------------------------------------------------
void ManageMPPT::run_charger(void) {

  static int off_count = OFF_NUM;

  switch (charger_state) {
    case on:
      if (sol_watts < MIN_SOL_WATTS) {                      // if watts input from the solar panel is less than
        charger_state = off;                                // the minimum solar watts then
        off_count = OFF_NUM;                                // go to the charger off state
        TURN_OFF_MOSFETS;
      }
      else if (bat_volts > (BATT_FLOAT - 0.1)) {            // else if the battery voltage has gotten above the float
        charger_state = bat_float;                          // battery float voltage go to the charger battery float state
      }
      else if (sol_watts < LOW_SOL_WATTS) {                 // else if the solar input watts is less than low solar watts
        pwm = PWM_MAX;                                      // it means there is not much power being generated by the solar panel
        set_pwm_duty();                          // so we just set the pwm = 100% so we can get as much of this power as possible
      }                                                     // and stay in the charger on state
      else {
        pwm = ((bat_volts * 10) / (sol_volts / 10)) + 5;    // else if we are making more power than low solar watts figure out what the pwm
        charger_state = bulk;                               // value should be and change the charger to bulk state
      }
      break;
    case bulk:
      if (sol_watts < MIN_SOL_WATTS) {                      // if watts input from the solar panel is less than
        charger_state = off;                                // the minimum solar watts then it is getting dark so
        off_count = OFF_NUM;                                // go to the charger off state
        TURN_OFF_MOSFETS;
      }
      else if (bat_volts > BATT_FLOAT) {                 // else if the battery voltage has gotten above the float
        charger_state = bat_float;                          // battery float voltage go to the charger battery float state
      }
      else if (sol_watts < LOW_SOL_WATTS) {                 // else if the solar input watts is less than low solar watts
        charger_state = on;                                 // it means there is not much power being generated by the solar panel
        TURN_ON_MOSFETS;                                    // so go to charger on state
      }
      else {                                                // this is where we do the Peak Power Tracking ro Maximum Power Point algorithm
        if (old_sol_watts >= sol_watts) {                   // if previous watts are greater change the value of
          delta = -delta;                 // delta to make pwm increase or decrease to maximize watts
        }
        pwm += delta;                                       // add delta to change PWM duty cycle for PPT algorythm (compound addition)
        old_sol_watts = sol_watts;                          // load old_watts with current watts value for next time
        set_pwm_duty();                   // set pwm duty cycle to pwm value
      }
      break;
    case bat_float:

      if (sol_watts < MIN_SOL_WATTS) {                      // if watts input from the solar panel is less than
        charger_state = off;                                // the minimum solar watts then it is getting dark so
        off_count = OFF_NUM;                                // go to the charger off state
        TURN_OFF_MOSFETS;
        set_pwm_duty();
      }
      else if (bat_volts > BATT_FLOAT) {                    // If we've charged the battery abovethe float voltage
        TURN_OFF_MOSFETS;                                   // turn off MOSFETs instead of modiflying duty cycle
        pwm = PWM_MAX;                                      // the charger is less efficient at 99% duty cycle
        set_pwm_duty();                                     // write the PWM
      }
      else if (bat_volts < BATT_FLOAT) {                    // else if the battery voltage is less than the float voltage - 0.1
        pwm = PWM_MAX;
        set_pwm_duty();                                     // start charging again
        TURN_ON_MOSFETS;
        if (bat_volts < (BATT_FLOAT - 0.1)) {               // if the voltage drops because of added load,
        charger_state = bulk;                               // switch back into bulk state to keep the voltage up
        }
      }
      break;
    case off:                                               // when we jump into the charger off state, off_count is set with OFF_NUM
      TURN_OFF_MOSFETS;
      if (off_count > 0) {                                  // this means that we run through the off state OFF_NUM of times with out doing
        off_count--;                                        // anything, this is to allow the battery voltage to settle down to see if the
      }                                                     // battery has been disconnected
      else if ((bat_volts > BATT_FLOAT) && (sol_watts > MIN_SOL_WATTS)) {
        charger_state = bat_float;                          // if battery voltage is still high and solar volts are high
        TURN_ON_MOSFETS;
      }
      else if ((bat_volts > MIN_BAT_VOLTS) && (bat_volts < BATT_FLOAT) && (sol_watts > MIN_SOL_WATTS)) {
        charger_state = bulk;
        TURN_ON_MOSFETS;
      }
      break;
    default:
      TURN_OFF_MOSFETS;
      break;
  }
}

void ManageMPPT::print_data(void) {

  //Serial.println(seconds,DEC);
  Serial.println("      ");

  Serial.print("Charging = ");
  if (charger_state == on) Serial.print("on   ");
  else if (charger_state == off) Serial.print("off  ");
  else if (charger_state == bulk) Serial.print("bulk ");
  else if (charger_state == bat_float) Serial.print("float");
  Serial.print("      ");

  Serial.print("pwm = ");
  if(charger_state == off)
  Serial.println(0,DEC);
  else
  Serial.print(pwm,DEC);
  Serial.println("      ");

  Serial.print(" Voltage (panel) = ");
  Serial.print(sol_volts ,10);
  Serial.print("      ");

  Serial.print("Amps (panel) = ");
  Serial.print(sol_amps);
  Serial.print("      ");

  Serial.print("Battery Voltage = ");
  Serial.print(bat_volts);
  Serial.print("      ");

  Serial.print("Amps into battery = ");
  Serial.print(in_bat_amps);
  Serial.println("      ");


delay(1000);
}

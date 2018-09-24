#include <ManageDevice.h>
#include <ManageDisplay.h>
#include <ManageTime.h>
#include <ManageThingSpeak.h>
#include <ManageSensors.h>
#include "TimerOne.h"               
 // using Timer1 library from http://www.arduino.cc/playground/Code/Timer1
#include <Wire.h> 

ManageThingSpeak thingSpeak;
ManageTime timeManager;
ManageSensors sensorManager(30,24,26,28);
ManageDisplay displayManager;
ManageDevice deviceManager(36,19,32,34);

/** MPPT Global Vars */

#define SOL_VOLTS_CHAN 0               
// defining the adc channel to read solar volts before 0.18ohm resistor
#define BAT_VOLTS_CHAN 1        
// defining adc channel for inputting battery
#define AVG_NUM 100                      
// number of iterations of the adc routine to average the adc readings

#define SOL_VOLTS_SCALE 0.0146484375        
//use 0.014... for 2k series 1k, if 1k series 1k 0.009765625
#define BAT_VOLTS_SCALE 0.0146484375      
// scaling value for raw adc reading to get battery volts // 5/1024 
* (R1+R2)/R2, R1=1k, R2=2k
// for use with 1k and 2k after battery voltage devider 0.0146484375

#define PWM_PIN 44                    
// the output pin for the pwm (timer 1 at 50kHz)
#define PWM_ENABLE_PIN 33            
// pin used to control shutoff function of the 
//IR2104 MOSFET driver (high, the mosfet driver is on)
#define PWM_FULL 1023               
 // the actual value used by the Timer1 routines for 100% pwm duty cycle
#define PWM_MAX 100                  
// the value for pwm duty cyle 0-100%
#define PWM_MIN 60                  
// the value for pwm duty cyle 0-100% 
//(below this value the current running in the system is = 0)
#define PWM_START 90                
// the value for pwm duty cyle 0-100%
#define PWM_INC 1                   
 //the value the increment to the pwm value for the ppt algorithm

#define TRUE 1
#define FALSE 0
#define ON TRUE
#define OFF FALSE

#define TURN_ON_MOSFETS digitalWrite(PWM_ENABLE_PIN, HIGH)     
 // enable MOSFET driver
#define TURN_OFF_MOSFETS digitalWrite(PWM_ENABLE_PIN, LOW)      
// disable MOSFET driver

#define ONE_SECOND 1          
 //count for number of interrupt in 1 second on interrupt period of 20us

#define LOW_SOL_WATTS 5.00          //value of solar watts, 5.00 watts
#define MIN_SOL_WATTS 1.00          //value of solar watts, 1.00 watts
#define MIN_BAT_VOLTS 11.00         //value of battery voltage, 11.00 volts          
#define MAX_BAT_VOLTS 14.10         //value of battery voltage, 14.10 volts
#define BATT_FLOAT 13.60            // battery voltage we want to stop charging at
#define HIGH_BAT_VOLTS 13.00        //value of battery voltage, 13.00 volts
#define OFF_NUM 10                   // number of iterations of off charger state
  
// global variables

double sol_amps;                       // solar amps 
double sol_volts;                      // solar volts before
float bat_volts;                      // battery volts 
float sol_watts;                      // solar watts
float old_sol_watts = 0;              //solar watts from previous time through the mppt
unsigned int seconds = 0;             // seconds from timer routine
unsigned int prev_seconds = 0;        // seconds value from previous pass
unsigned int interrupt_counter = 0;   // counter for 20us interrrupt
int delta = PWM_INC;                  
// variable used to modify pwm duty cycle for the ppt algorithm
int pwm = 0;                          // pwm duty cycle 0-100%


enum charger_mode {off, on, bulk, bat_float} charger_state;   
 // enumerated variable that holds state for charger state machine

/** End */


int alarm = 0;

void setup() {
  Serial.setTimeout(5);
  Serial.begin(9600);
  displayManager.writeBatteryPercentage(75);
  displayManager.updateMessage("Connecting to network");
  displayManager.updateDisplay("00:00:00");
  thingSpeak.StartGSM();
  displayManager.writeBatteryPercentage(75);//mpptManager.getBatPercent());
  displayManager.updateMessage("Connected");
  displayManager.updateDisplay("00:00:00");
  timeManager.setFinalTime(0);
  sensorManager.enablePower(true);

  /** MPPT Setup */

  pinMode(PWM_ENABLE_PIN, OUTPUT);     // sets the digital pin as output
  TURN_OFF_MOSFETS;                    // turn off MOSFET driver chip
  charger_state = off;                 // start with charger state as off

  Timer1.initialize(20);               // initialize timer1, and set a 20uS period
  Timer1.pwm(PWM_PIN, 0);              // setup pwm on pin 44, 0% duty cycle
  Timer1.attachInterrupt(callback);   
 // attaches callback() as a timer overflow interrupt
  pwm = PWM_START;                     //starting value for pwm  

  /** End */
}

/** Main */

void loop() {
  /** Check button */
  deviceManager.toggleButton();
  
  thingSpeak.setData(String(timeManager.getRemainingTimeSeconds()), 
String(alarm), String(sensorManager.getRain()),  
 String(sensorManager.getTemp()), String(sensorManager.getHumidity()), 
String(sol_volts), String(bat_volts), String(pwm));
  thingSpeak.accessThingSpeak("Update");
  thingSpeak.accessThingSpeak("Read");

  /** Update Sensor Information */
  sensorManager.updateSensors();

  /** Update MPPT Data */
  read_data();                         // read data from inputs
  run_charger();                       // run the charger state machine
  //print_data();                      // print data
  
  /** Update display */
  displayManager.writeBatteryPercentage(74); //mpptManager.getBatPercent());
  displayManager.updateDisplay(timeManager.getRemainingTime());

  /** Action Command from ThingSpeak */
  if (!thingSpeak.noCommands()){
    String command = thingSpeak.getNextCommand();
    if ((command.startsWith("P")) && (!sensorManager.getAlarmStatus())){
      command = command.substring(2);
      timeManager.setFinalTime(command.toInt());
      sensorManager.enablePower(true);
      displayManager.updateMessage("Device enabled, time purchased = " 
+ timeManager.getRemainingTime());
    }
    if (command.startsWith("Alarm")){
      sensorManager.setAlarmStatus(false);
      deviceManager.setAlarmWire();
      alarm = 0;
      displayManager.updateMessage("Alarm Disabled");
    }
    if (command.startsWith("M")){
      command = command.substring(2);
      displayManager.updateMessage(command.substring(0,(command.length()-1)));
      deviceManager.setLCDState(true);
    }
  }

  /** Check Time and Alarm Status */
  if(!timeManager.timeRemaining() && sensorManager.deviceEnabled()){
    sensorManager.enablePower(false);
    displayManager.updateMessage("Device Disabled");
  }
  if (sensorManager.getAlarmStatus() || deviceManager.getAlarmWire()){
    sensorManager.enablePower(false);
    alarm = 1;
    timeManager.setFinalTime(0);
    displayManager.updateMessage("Device Disabled");
  }
  
}

// This routine reads and averages the analog inputs for this system, 
//solar volts, solar amps and battery volts. 

double read_adc(int channel){
  
  double sum = 0;
  double temp;
  double NewTemp;
  int i;
  
  for (i=0; i<AVG_NUM; i++) {         
 // loop through reading raw adc values AVG_NUM number of times  
    temp = analogRead(channel);        // read the input pin  
 /*Serial.println("Temp (Mapped to 5 Volts= ");
  NewTemp = temp*5/1024;                
//this commented out section is to show adc values
  Serial.println(NewTemp);
  Serial.println("      ");
 */
    sum += temp;                       // store sum for averaging
    //delayMicroseconds(1000);             // pauses 
  }
  return(sum / AVG_NUM);               
// divide sum by AVG_NUM to get average and return it
}


// This routine reads all the analog input values for the system. 
// Then it multiplies them by the scale
// factor to get actual value in volts or amps. 
void read_data(void) {

sol_volts = read_adc(SOL_VOLTS_CHAN) * SOL_VOLTS_SCALE;       
//input of solar volts 
bat_volts = read_adc(BAT_VOLTS_CHAN) * BAT_VOLTS_SCALE;      
//input of battery volts
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

// This is interrupt service routine for Timer1 that occurs every 20uS.
//
void callback()
{
  if (interrupt_counter++ > ONE_SECOND) {        
// increment interrupt_counter until one second has passed
    interrupt_counter = 0;                       // reset the counter
    seconds++;                                   // then increment seconds counter
  }
}

// This routine uses the Timer1.pwm function to set the pwm duty cycle.
void set_pwm_duty(void) {

  if (pwm > PWM_MAX) {      // check limits of PWM duty cyle and set to PWM_MAX
    pwm = PWM_MAX;    
  }
  else if (pwm < PWM_MIN) {   // if pwm is less than PWM_MIN then set it to PWM_MIN
    pwm = PWM_MIN;
  }
  if (pwm < PWM_MAX) {
    Timer1.pwm(PWM_PIN,(PWM_FULL * (long)pwm / 100), 20);  
// use Timer1 routine to set pwm duty cycle at 20uS period
    //Timer1.pwm(PWM_PIN,(PWM_FULL * (long)pwm / 100));
  }                       
  else if (pwm == PWM_MAX) {         
  // if pwm set to 100% it will be on full
    Timer1.pwm(PWM_PIN,(PWM_FULL - 1), 20);               
 // keep switching so set duty cycle at 99.9% 
    //Timer1.pwm(PWM_PIN,(PWM_FULL - 1));              
  }                       
} 

// This routine is the charger state machine. 
// The battery charger can be in one of the following four states:
// 
//  On State - too low for bulk, set the pwm = 99.9% to
/ get the most of low amount of power available.
//
//  Bulk State - run the Peak Power Tracking algorithm. In this state we try and 
//run the maximum amount
//      of current that the solar panels are generating into the battery.
//
//  Float State -  In this state we try and keep the battery voltage
//      at MAX_BAT_VOLTS by adjusting the pwm value. If we get to pwm = 100%
// it means we can't keep the battery 
//      voltage at MAX_BAT_VOLTS which probably means the battery is being drawn 
//down by some load so we need to back
//      into the bulk charging mode.
//
//  Off State - The charger goes into this state when there is no more power being
// generated by the solar panels. 
//    The MOSFETs are turned off in this state so that power from the battery 
//doesn't leak back into the solar panel.
  
void run_charger(void) {
  
  static int off_count = OFF_NUM;

  switch (charger_state) {
    case on:                                        
      if (sol_watts < MIN_SOL_WATTS) {                      
// if sol_watts less than min_sol_watts
        charger_state = off;                                
// off state
        off_count = OFF_NUM;                                
        TURN_OFF_MOSFETS; 
      }
      else if (bat_volts > (BATT_FLOAT - 0.1)) {            
// batt_volts greater than batt_float
        charger_state = bat_float;                          
// float state
      }
      else if (sol_watts < LOW_SOL_WATTS) {                 
// sol_watts less than low_sol_watts, 
        pwm = PWM_MAX;                                      
// lowh power being generated by the solar panel
        set_pwm_duty();                                   
  // max out pwm to 100% so we can get as much of this power as possible
      }                                                     // on state
      else {                                          
        pwm = ((bat_volts * 10) / (sol_volts / 10)) + 5;    
// else if we are making more power than low solar watts figure out what the pwm
        charger_state = bulk;                               
// value should be and change the charger to bulk state 
      }
      break;
    case bulk:
      if (sol_watts < MIN_SOL_WATTS) {                     
 // sol_watts less than min_sol_watts
        charger_state = off;                                
// off state, most likely sun going down
        off_count = OFF_NUM;                                
        TURN_OFF_MOSFETS; 
      }
      else if (bat_volts > BATT_FLOAT) {                   
 // while in bulk, bat_volts greater than batt_float
        charger_state = bat_float;                          // float state
      }
      else if (sol_watts < LOW_SOL_WATTS) {                
 // sol_watts less than low_sol_watts
        charger_state = on;                                
 // not much power being generated by the solar panel
        TURN_ON_MOSFETS;                                    // on state
      }
      else {                                                
// running Maximum Power Point algorithm
        if (old_sol_watts >= sol_watts) {                  
 // if previous solar watts (old_sol_watts) are greater than current sol_watts
          delta = -delta;                                   
// delta to make pwm increase or decrease to maximize through put power
        }
        pwm += delta;                                       
// add delta to change PWM duty cycle
        old_sol_watts = sol_watts;                         
 // load old_sol_watts with current sol_watts value for next pass
        set_pwm_duty();                                     // set pwm
      }
      break;
    case bat_float:

      if (sol_watts < MIN_SOL_WATTS) {                     
 // sol_watts < min_sol_watts
        charger_state = off;                              
  // getting dark, off state
        off_count = OFF_NUM;                                
        TURN_OFF_MOSFETS; 
        set_pwm_duty();         
      }
      else if (bat_volts > BATT_FLOAT) {                 
   // bat_volts greater than batt_float, battery fully charged                
        TURN_OFF_MOSFETS;                                 
  // turn off MOSFETs 
        pwm = PWM_MAX;                                    
  // the charger is less efficient at 99% duty cycle
        set_pwm_duty();                                     // set PWM
      }
      else if (bat_volts < BATT_FLOAT) {               
     // else if the battery voltage is less than the float voltage - 0.1
        pwm = PWM_MAX;                                      
        set_pwm_duty();                                   
  // start charging again
        TURN_ON_MOSFETS;    
        if (bat_volts < (BATT_FLOAT - 0.1)) {             
  // if the voltage drops because of added load,
        charger_state = bulk;                              
 // switch back into bulk state to keep the voltage up
        }
      }
      break;
    case off:                                            
   // off state, off_count is set with OFF_NUM
      TURN_OFF_MOSFETS;
      if (off_count > 0) {                                 
 // go through off state OFF_NUM of times with out doing anything
        off_count--;                                       
 // to allow the battery voltage to settle down to see if the  
      }                                                     
// battery has been disconnected
      else if ((bat_volts > BATT_FLOAT) && (sol_watts > MIN_SOL_WATTS)) {
        charger_state = bat_float;                          
// if battery voltage is high and solar watts are high
        TURN_ON_MOSFETS;
      }    
      else if ((bat_volts > MIN_BAT_VOLTS) && (bat_volts < BATT_FLOAT) 
    && (sol_watts > MIN_SOL_WATTS)) {
        charger_state = bulk;
        TURN_ON_MOSFETS;
      }
      break;
    default:
      TURN_OFF_MOSFETS; 
      break;
  }
}

void print_data(void) {

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
  Serial.print(sol_volts ,2);
  Serial.print("      ");

  Serial.print("Amps (panel) = ");
  Serial.print(sol_amps);
  Serial.print("      ");

  Serial.print("Battery Voltage = ");
  Serial.print(bat_volts,2);
  Serial.print("      ");

}


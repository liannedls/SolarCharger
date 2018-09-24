/*
  ManageMPPT.h - Library for handling the communication with the server.
  Created by Sam Lukasik.
  Used for Project Micro Solar Charger.
*/

#ifndef ManageMPPT_h
#define ManageMPPT_h

#include "TimerOne.h"                // using Timer1 library from http://www.arduino.cc/playground/Code/Timer1
#include <Wire.h>

#define SOL_VOLTS_CHAN 0               // defining the adc channel to read solar volts before 0.18ohm resistor
#define BAT_VOLTS_CHAN 1        // defining adc channel for inputting battery
//#define IN_BAT_AMPS_CHAN 2         // defining the adc channel to read battery volts
#define AVG_NUM 100                      // number of iterations of the adc routine to average the adc readings

#define IN_BAT_AMPS_SCALE 0.02637837           //math? 0.026393581
#define SOL_VOLTS_SCALE 0.0146484375        //use 0.014... for 2k series 1k, if 1k series 1k 0.009765625
#define BAT_VOLTS_SCALE 0.0146484375      // scaling value for raw adc reading to get battery volts // 5/1024 * (R1+R2)/R2, R1=1k, R2=2k
                                          // for use with 1k and 2k after battery voltage devider 0.0146484375

#define PWM_PIN 44                    // the output pin for the pwm (only pin 9 avaliable for timer 1 at 50kHz)
#define PWM_ENABLE_PIN 33            // pin used to control shutoff function of the IR2104 MOSFET driver (hight the mosfet driver is on)
#define PWM_FULL 1023                // the actual value used by the Timer1 routines for 100% pwm duty cycle
#define PWM_MAX 100                  // the value for pwm duty cyle 0-100%
#define PWM_MIN 60                  // the value for pwm duty cyle 0-100% (below this value the current running in the system is = 0)
#define PWM_START 90                // the value for pwm duty cyle 0-100%
#define PWM_INC 1                    //the value the increment to the pwm value for the ppt algorithm

#define TRUE 1
#define FALSE 0
#define ON TRUE
#define OFF FALSE

#define TURN_ON_MOSFETS digitalWrite(PWM_ENABLE_PIN, HIGH)      // enable MOSFET driver
#define TURN_OFF_MOSFETS digitalWrite(PWM_ENABLE_PIN, LOW)      // disable MOSFET driver

#define ONE_SECOND 50000            //count for number of interrupt in 1 second on interrupt period of 20us

#define LOW_SOL_WATTS 5.00          //value of solar watts // this is 5.00 watts
#define MIN_SOL_WATTS 1.00          //value of solar watts // this is 1.00 watts
#define MIN_BAT_VOLTS 11.00         //value of battery voltage // this is 11.00 volts
#define MAX_BAT_VOLTS 14.10         //value of battery voltage// this is 14.10 volts
#define BATT_FLOAT 13.60            // battery voltage we want to stop charging at
#define HIGH_BAT_VOLTS 13.00        //value of battery voltage // this is 13.00 volts
#define LVD 11.5                    //Low voltage disconnect setting for a 12V system
#define OFF_NUM 9                   // number of iterations of off charger state

class ManageMPPT{
  public:
    ManageMPPT();
    void read_data(void);
    void set_pwm_duty(void);
    void run_charger(void);
    void print_data(void);
  private:
    double sol_amps;                       // solar amps
    double sol_volts;                      // solar volts before
    float bat_volts;                      // battery volts
    double in_bat_amps;                   // amps going into battery
    float sol_watts;                      // solar watts
    float old_sol_watts;              // solar watts from previous time through ppt routine
    unsigned int prev_seconds;        // seconds value from previous pass
    int delta;                  // variable used to modify pwm duty cycle for the ppt algorithm
    int pwm;                          // pwm duty cycle 0-100%
};

#endif

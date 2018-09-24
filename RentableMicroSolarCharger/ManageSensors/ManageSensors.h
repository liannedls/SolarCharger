/*
  ManageSensors.h - Library for handling the following sensors
	- Temp/Humidity
	- Rain
	- IR - To be Completed
  - Button
  Created by Sam Lukasik
  Used for Project Micro Solar Charger.
*/

#ifndef ManageSensors_h
#define ManageSensors_h

#include <Arduino.h>

class ManageSensors
{
public:
  ManageSensors(int pinRain, int pinIR, int pinOR, int pinPower);
	float getTemp();
	float getHumidity();
  int getRain();
	float dewPoint(float t, float h);
  void updateSensors();
  int getIRStatus();
  int readIR();
  void enablePower(bool status);
  void setAlarmStatus(bool status);
  bool getAlarmStatus();
  bool deviceEnabled();
private:
	float humidity;
	float temperature;
  int pinRain;
  int pinIR;
  int pinOR;
  int pinPower;
  int rain;
  int irStatus;
  bool alarmTriggered;
  bool deviceStatus;
};
#endif

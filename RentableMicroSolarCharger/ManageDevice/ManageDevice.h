/*
  ManageDevice.h - Library for handling the device components
	- GPS
  Created by Sam Lukasik
  Used for Project Micro Solar Charger.
*/

#include "Arduino.h"

#ifndef ManageDevice_h
#define ManageDevice_h

class ManageDevice
{
public:
  ManageDevice(int lcdPin, int buttonPin, int wireIn, int wireOut);
  void toggleButton();
  void setLCDState(bool status);
  bool getAlarmWire();
  void checkWireAlarm();
  void setAlarmWire();
private:
  int lcdPin;
  int buttonPin;
  int wireIn;
  int wireOut;
  unsigned long screenTimer;
  int duration;
  bool alarmWire;
};
#endif

/*
  ManageDisplay.h - Library for handling the device components
	- Solenoid
	- Fan
  Created by Sam Lukasik
  Used for Project Micro Solar Charger.
*/

#ifndef ManageDisplay_h
#define ManageDisplay_h

#include <Arduino.h>
#include <QueueList.h>

class ManageDisplay
{
public:
  ManageDisplay();
  void updateMessage(String m);
  String getNextMessage();
  bool messageEmpty();
  String writeTimeToDisplay();
  void writeBatteryPercentage(int p);
  void writeMessageToDisplay();
  void updateDisplay(String t);
private:
  byte batteryFull[8] = { B01110, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
  byte batteryThreeQuarters[8] = {B01110, B11011, B10001, B11111, B11111, B11111, B11111, B11111};
  byte batteryHalf[8] = {B01110, B11011, B10001, B10001, B11111, B11111, B11111, B11111};
  byte batteryOneQuarter[8] = {B01110, B11011, B10001, B10001, B10001, B11111, B11111, B11111};
  byte batteryTenPercent[8] = {B01110, B11011, B10001, B10001, B10001, B10001, B11111, B11111};
  byte batteryEmpty[8] = {B01110, B11011, B10001, B10001, B10001, B10001, B10001, B11111};
  byte batteryError[8] = {B01110, B11011, B10001, B11011, B10101, B11011, B10001, B11111};
  QueueList<String> message;
  float batteryCharge = 71;
};
#endif

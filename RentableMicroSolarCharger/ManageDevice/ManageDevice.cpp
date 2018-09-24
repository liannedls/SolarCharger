#include "ManageDevice.h"

volatile byte lcdState = LOW;
int debounce = 10;
unsigned long debounceTime = millis();

void toggle(){
  if ((millis() - debounceTime) > debounce){
      lcdState = !lcdState;
      debounceTime = millis();
  }
}

ManageDevice::ManageDevice(int lP, int bP, int wI, int wO){
  lcdPin = lP;
  buttonPin = bP;
  wireIn = wI;
  wireOut = wO;
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(lcdPin, OUTPUT);

  screenTimer = millis();
  duration = 50000;
  lcdState = HIGH;
  alarmWire = false;

  digitalWrite(lcdPin, lcdState);

  attachInterrupt(digitalPinToInterrupt(buttonPin), toggle, RISING);
}

void ManageDevice::toggleButton(){
  // set the LCD power pin:
  digitalWrite(lcdPin, lcdState);   //illuminates the screen

  if (millis() - screenTimer > duration){  //waits 30 seconds then turns off screen
    screenTimer = millis();
    lcdState = LOW;
  }
}

void ManageDevice::setLCDState(bool status){
  if(status){
    lcdState = HIGH;
  } else {
    lcdState = LOW;
  }
}

bool ManageDevice::getAlarmWire(){
  return alarmWire;
}

void ManageDevice::setAlarmWire(){
  alarmWire = false;
}

void ManageDevice::checkWireAlarm(){
  digitalWrite(wireOut, HIGH);
  if (digitalRead(wireIn) == LOW){
    alarmWire = true;
  }
}

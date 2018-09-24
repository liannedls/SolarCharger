/*
  ManageTime.h - Library for handling the timer.
  Created by Sam Lukasik.
  Used for Project Micro Solar Charger.
*/
#ifndef ManageTime_h
#define ManageTime_h

#include "Arduino.h"
#include <Time.h>
#include <TimeLib.h>

class ManageTime{
  public:
    ManageTime();
    //void printTime(time_t t)
    int getSeconds(int d, int h, int m, int s);
    String getRemainingTime();
    int getFinalTime();
	  void setFinalTime(int t);
    int getCurrentTime();
    int getRemainingTimeSeconds();
    int getStartTime();
	  void updateCurrentTime();
	  void updateStartTime();
	  bool timeRemaining();
  private:
	  time_t startTime;
	  time_t currentTime;
	  int finalTime;
};

#endif

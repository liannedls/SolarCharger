#include "Arduino.h"
#include "ManageTime.h"

ManageTime::ManageTime(){
  setTime(0,0,0,1,01,2017);
  startTime = now();
  finalTime = 0;
  currentTime = now();
}

int ManageTime::getSeconds(int d, int h, int m, int s){
  return (86400*d + 3600*h + 60*m + s);
}

void ManageTime::setFinalTime(int t){
  updateCurrentTime();
  finalTime = (t + (getCurrentTime()));
}

int ManageTime::getFinalTime(){
  return finalTime;
}

int ManageTime::getCurrentTime(){
  return getSeconds(day(currentTime),hour(currentTime),minute(currentTime),second(currentTime));
}

int ManageTime::getStartTime(){
  return getSeconds(day(startTime),hour(startTime),minute(startTime),second(startTime));
}

String ManageTime::getRemainingTime(){
  updateCurrentTime();
  if (timeRemaining()){
	   int tR = getRemainingTimeSeconds();
	   //int secs = tR % 60;
	   int mins = (tR % 3600)/60;
     int hrs = (tR % 86400)/3600;
     int dys = (tR % (86400*30))/86400;
     String temp = String(dys);
	   temp += ":";
	   temp += hrs;
	   temp += ":";
	   temp += mins;
     temp += "     ";
	   //temp += ":";
	   //temp += secs;
	   return temp;
  } else { return "00:00:00     ";}
}

int ManageTime::getRemainingTimeSeconds(){
  updateCurrentTime();
  if (timeRemaining()){
    int tR = getFinalTime() - getCurrentTime();
    return tR;
  }
  return 0;
}

void ManageTime::updateCurrentTime(){
	currentTime = now();
}

void ManageTime::updateStartTime(){
	startTime = now();
}
bool ManageTime::timeRemaining(){
	return (getCurrentTime() < getFinalTime());
}

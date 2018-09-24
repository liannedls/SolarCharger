/*
  ManageThingSpeak.h - Library for handling the communication with the server.
  Created by Sam Lukasik.
  Used for Project Micro Solar Charger.
*/

#ifndef ManageThingSpeak_h
#define ManageThingSpeak_h

// Include the GSM Library
#include "Arduino.h"
#include <QueueList.h>

// PIN number of SIM Card
#define PINNUMBER ""

// APN information of network provider
#define GPRS_APN "internet.com"
#define GPRS_LOGIN "wapuser1"
#define GPRS_PASSWORD "wap"

class ManageThingSpeak{
  public:
    ManageThingSpeak();
    void StartGSM();
    void updateThingSpeak();
    void checkTalkBackCommand();
    void accessThingSpeak(String action);
    String getData();
    String getNextCommand();
    bool noCommands();
    void setData(String timeRemaining, String alarmStatus, String rainStatus, String temperature, String humidity, String solVolt, String batVolt, String pwm);
  private:
    int counter;
    boolean lastConnected;
    int failedCounter;
    int data;
    String stringToThingSpeak;
    QueueList<String> talkBackQueue;
};

#endif

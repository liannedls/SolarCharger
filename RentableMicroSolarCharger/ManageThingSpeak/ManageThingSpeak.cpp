#include "Arduino.h"
#include "ManageThingSpeak.h"
#include "GSM.h"

GSMClient client;
GPRS gprs;
GSM gsmAccess;

//ThinkSpeak Channel Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "HTE0THDM6XML0BED";
String talkBackAPIKey = "KAXGPVM86C4GH9O0";
String talkBackID = "11041";
int updateThingSpeakInterval = 16;

ManageThingSpeak::ManageThingSpeak(){
  int counter = 0;
  boolean lastConnected = false;
  int failedCounter = 0;
  int data = 90000;
  String stringToThingSpeak = "field1=0";
}

void ManageThingSpeak::setData(String t, String iR, String rain, String temp, String hum, String solVolt, String batVolt, String pwm) {
      stringToThingSpeak = "field1=" + t + "&field2=" + iR + "&field3=" + rain + "&field4=" + temp + "&field5=" + hum + "&field6=" + solVolt + "&field7=" + batVolt + "&field8=" + pwm;
      //return StringToThingSpeak;
}

String ManageThingSpeak::getData(){
  return stringToThingSpeak;
}

void ManageThingSpeak::updateThingSpeak(){
  String tsData = stringToThingSpeak;
  if (client.connect(thingSpeakAddress, 80)){
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(tsData.length());
      client.print("\n\n");
      client.print(tsData);
      if (client.connected()){
          Serial.println("Connecting to ThingSpeak...");
          Serial.println();
          failedCounter = 0;
      } else {
          failedCounter++;
          Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
          Serial.println();
      }

  } else {
      failedCounter++;
      Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
      Serial.println();
  }
  if (failedCounter > 3 ) {StartGSM();}
  lastConnected = client.connected();
}

void ManageThingSpeak::checkTalkBackCommand(){
  String talkBackCommand;
  char charIn;
  String talkBackURL =  talkBackID + "/commands/execute?api_key=" + talkBackAPIKey;

  // Make a HTTP GET request to the TalkBack API:
  if (client.connect(thingSpeakAddress, 80)){
      Serial.println("Accessing Talk Back Server");
      client.print("GET /talkbacks/" + talkBackURL + " HTTP/1.0\n");
      client.println();
      if (client.connected()){
         failedCounter = 0;
         while(!client.available());
         while (client.available()) {
            if (counter == 18){
                charIn = client.read();
                if (charIn == '\n'){
                  counter++;
                } else if (charIn != " "){
                  talkBackCommand += charIn;
                }
            }  else {
                  charIn = client.read();
                  if (charIn == '\n'){
                      counter++;
                  }
            }
        }
       if (talkBackCommand.startsWith("P") || talkBackCommand.startsWith("M") || talkBackCommand.startsWith("A")){
          talkBackQueue.push(talkBackCommand);
       }
       counter = 0;
       } else {
            failedCounter++;
            Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
            Serial.println();
        }
  } else {
      failedCounter++;
      Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
      Serial.println();
  }
  if (failedCounter > 3 ) {StartGSM();}
  lastConnected = client.connected();
}

void ManageThingSpeak::StartGSM(){
    char server[] = "api.thingspeak.com"; // the base URL
    int port = 80; // the port, 80 for HTTP
    Serial.println("Starting Arduino web client.");
    boolean notConnected = true;
    failedCounter = 0;

    while(notConnected){
        if((gsmAccess.begin(PINNUMBER)==GSM_READY) & (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD)==GPRS_READY)) {
            notConnected = false;
        } else {
            Serial.println("Not connected");
            delay(1000);
        }
    }

    Serial.println("connecting...");
    // if you get a connection, report back via serial:
    if (client.connect(server, port)){
        Serial.println("connected");
    } else {
        Serial.println("connection failed");
    }
}

void ManageThingSpeak::accessThingSpeak(String action){
  if (client.available()){
        char c = client.read();
        Serial.print(c);
    }
    // Disconnect from ThingSpeak
    if (!client.connected() && lastConnected){
        Serial.println("...disconnected");
        Serial.println();
        client.stop();
    }
    // Try to connect to ThingSpeak
    if(!client.connected()){
        //String ToThingSpeak = readData();
        //Serial.println(ToThingSpeak);
        //updateThingSpeak(ToThingSpeak);
        //checkTalkBackCommand();
        if (action.equals("Update")){
          updateThingSpeak();
        } else {
          checkTalkBackCommand();
        }
    }
}

//Returns the next message in the queue
String ManageThingSpeak::getNextCommand(){
  return talkBackQueue.pop();
}

//Checks if the queue is empty
bool ManageThingSpeak::noCommands(){
  return talkBackQueue.isEmpty();
}

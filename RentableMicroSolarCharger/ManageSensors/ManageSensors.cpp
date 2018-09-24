

#include "ManageSensors.h"
#include <DHT.h>

DHT dht(22, DHT11);

ManageSensors::ManageSensors(int pR, int pIR, int pOR, int pPow){
	pinRain = pR;
	pinIR = pIR;
	pinOR = pOR;
	pinPower = pPow;
	pinMode(pinRain,INPUT);
	pinMode(pinIR, INPUT);
	pinMode(pinOR, OUTPUT);
	pinMode(pinPower, OUTPUT);
	digitalWrite(pinPower, LOW);
	temperature = 0;
	humidity = 0;
	rain = 1;
	setAlarmStatus(false);
	deviceStatus = false;
	dht.begin();
}

void ManageSensors::updateSensors(){
	temperature = dht.readTemperature();
	humidity = dht.readHumidity();
	rain = digitalRead(pinRain);
	irStatus = readIR();
	if (irStatus == 0){ // NEED TO CHANGE LATER ON
		setAlarmStatus(true);
	}
}

void ManageSensors::enablePower(bool status){
	if (status){
		digitalWrite(pinPower, HIGH);
		deviceStatus = true;
	} else {
		digitalWrite(pinPower, LOW);
		deviceStatus = false;
	}
}

bool ManageSensors::deviceEnabled(){
	return deviceStatus;
}

void ManageSensors::setAlarmStatus(bool status){
	if (status){
		alarmTriggered = true;
	} else {
		alarmTriggered = false;
	}
}

bool ManageSensors::getAlarmStatus(){
	return alarmTriggered;
}

int ManageSensors::readIR(){
	int halfPeriod = 13; //one period at 38.5khZ is aproximately 26 microseconds
  int cycles = 38; //26 microseconds * 38 is more or less 1 millisecond
  int i;
  for (i=0; i <=cycles; i++)
  {
    digitalWrite(pinOR, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(pinOR, LOW);
    delayMicroseconds(halfPeriod - 1);     // - 1 to make up for digitaWrite overhead
  }
  return digitalRead(pinIR);
}

float ManageSensors::dewPoint(float t, float h){
	// (1) Saturation Vapor Pressure = ESGG(T)
	double RATIO = 373.15 / (273.15 + t);
	double RHS = -7.90298 * (RATIO - 1);
	RHS += 5.02808 * log10(RATIO);
	RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
	RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
	RHS += log10(1013.246);

        // factor -3 is to adjust units - Vapor Pressure SVP * humidity
	double VP = pow(10, RHS - 3) * h;

        // (2) DEWPOINT = F(Vapor Pressure)
	double Temp = log(VP/0.61078);   // temp var
	return (241.88 * Temp) / (17.558 - Temp);
}

float ManageSensors::getTemp(){
	return temperature;
}

float ManageSensors::getHumidity(){
	return humidity;
}

int ManageSensors::getRain(){
	return rain;
}

int ManageSensors::getIRStatus(){
	return irStatus;
}

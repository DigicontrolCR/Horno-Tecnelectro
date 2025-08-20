/********************************************************
 * PID Basic Example
 * Reading analog input 0 to control analog PWM output 3
 ********************************************************/

#include <PID_v1.h>
#include <SPI.h>
#include "Adafruit_MAX31855.h"

#define PIN_INPUT 0
#define PIN_OUTPUT 25


#define MAXDO 19
#define MAXCS 5
#define MAXCLK 18

//Define Variables we'll be connecting to
double Setpoint, Input, Output;
double temperatura = 0;
double temperatura2 = 0;
double temperatura3 = 0;
//Specify the links and initial tuning parameters
double Kp = 2, Ki = 5, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

void setup() {
  Serial.begin(9600);
  //initialize the variables we're linked to
  Input = temperatura3;
  Setpoint = 100;

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
}

void loop() {
  temperatura = thermocouple.readCelsius();  //valor de la temperatura
  temperatura2 = int(temperatura);
  double temperatura3 = temperatura2;
  Serial.println(temperatura3);
  Serial.println(DIRECT);
  Input = temperatura3;
  myPID.Compute();
  analogWrite(PIN_OUTPUT, Output);
}

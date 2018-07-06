/*
  Library for the Sensirion SGP30 Indoor Air Quality Sensor
  By: Ciara Jekel
  SparkFun Electronics
  Date: June 28th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SGP30 Datasheet: https://cdn.sparkfun.com/assets/c/0/a/2/e/Sensirion_Gas_Sensors_SGP30_Datasheet.pdf

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813

  This example tasks a scheduler with reading air quality,
  freeing the main loop from wasteful delays.
*/

#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <TaskScheduler.h> // Click here to get the library: http://librarymanager/All#TaskScheduler
#include <Wire.h>

void t1Callback(void);

SGP30 mySensor; //create an object of the SGP30 class
bool results = false;
//run task t1Callback every 1000ms (1s) forever
Task t1(1000, TASK_FOREVER, &t1Callback);
Scheduler runner; //create an object of the Scheduler class

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);
  if (mySensor.begin() == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  //initialize scheduler
  runner.init();
  //add task t1 to the schedule
  runner.addTask(t1);
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();
  //enable t1 to run
  t1.enable();
}

void loop() {
  //First fifteen readings will be
  //CO2: 400 ppm  TVOC: 0 ppb
  runner.execute();
  if (results == true) {
    Serial.print("CO2: ");
    Serial.print(mySensor.CO2);
    Serial.print(" ppm\tTVOC");
    Serial.print(mySensor.TVOC);
    Serial.println(" ppb");
    results = false;
  }
}

void t1Callback(void) {
  mySensor.measureAirQuality();
  results = true;
}


/*
  Library for the Sensirion SGP30 Indoor Air Quality Sensor
  By: Ciara Jekel
  SparkFun Electronics
  Date: June 28th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SGP30 Datasheet: https://cdn.sparkfun.com/assets/c/0/a/2/e/Sensirion_Gas_Sensors_SGP30_Datasheet.pdf

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813

  This example measures CO2 and TVOC and reports any errors.
*/

#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <Wire.h>

SGP30 mySensor; //create an object of the SGP30 class
SGP30ERR error;
long t1, t2;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  //Sensor supports I2C speeds up to 400kHz
  Wire.setClock(400000);
  //Initialize sensor
  if (mySensor.begin() == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  //Initializes sensor for air quality readings
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();
  t1 = millis();
}

void loop() {
  //First fifteen readings will be
  //CO2: 400 ppm  TVOC: 0 ppb
  t2 = millis();
  if ( t2 >= t1 + 1000) //only will occur if 1 second has passed
  {
    t1 = t2;  //measure CO2 and TVOC levels
    error = mySensor.measureAirQuality();
    if (error == SUCCESS) {
      Serial.print("CO2: ");
      Serial.print(mySensor.CO2);
      Serial.print(" ppm\tTVOC: ");
      Serial.print(mySensor.TVOC);
      Serial.println(" ppb");
    }
    else if (error == ERR_BAD_CRC) {
      Serial.println("CRC Failed");
    }
    else if (error == ERR_I2C_TIMEOUT) {
      Serial.println("I2C Timed out");
    }
  }
}

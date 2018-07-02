/*
  Library for the Sensirion SGP30 Indoor Air Quality Sensor
  By: Ciara Jekel
  SparkFun Electronics
  Date: June 28th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SGP30 Datasheet: https://cdn.sparkfun.com/assets/c/0/a/2/e/Sensirion_Gas_Sensors_SGP30_Datasheet.pdf

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813

  This example performs the sensor's self test and displays the results.
*/

#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30#include <Wire.h>

SGP30 mySensor; //create an object of the SGP30 class
byte count = 0;
SGP30ERR error;
void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);
  if (mySensor.begin() == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  //measureTest() should not be called after a call to initAirQuality()
  error = mySensor.measureTest();
  if (error == SUCCESS) {
    Serial.println("Success!");
  }
  else if (error == ERR_BAD_CRC) {
    Serial.println("CRC Failed");
  }
  else if (error == ERR_I2C_TIMEOUT) {
    Serial.println("I2C Timed out");
  }
  else if (error == SELF_TEST_FAIL) {
    Serial.println("Self Test Failed");
  }
}

void loop() {

}


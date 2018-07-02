/*
  Library for the Sensirion SGP30 Indoor Air Quality Sensor
  By: Ciara Jekel
  SparkFun Electronics
  Date: June 28th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SGP30 Datasheet: https://cdn.sparkfun.com/assets/c/0/a/2/e/Sensirion_Gas_Sensors_SGP30_Datasheet.pdf

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813

  This example gets the sensor's serial ID and version number.
*/

#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <Wire.h>

SGP30 mySensor; //create an object of the SGP30 class

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
  //Get SGP30's ID
  mySensor.getSerialID();
  //Get version number
  mySensor.getFeatureSetVersion();
  Serial.print("SerialID: 0x");
  Serial.print((unsigned long)mySensor.serialID, HEX);
  Serial.print("\tFeature Set Version: 0x");
  Serial.println(mySensor.featureSetVersion, HEX);
}

void loop() {

}

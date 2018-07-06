/*
  Library for the Sensirion SGP30 Indoor Air Quality Sensor
  By: Ciara Jekel
  SparkFun Electronics
  Date: June 28th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SGP30 Datasheet: https://cdn.sparkfun.com/assets/c/0/a/2/e/Sensirion_Gas_Sensors_SGP30_Datasheet.pdf

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813

  This example performs a soft reset on the sensor and restores its baseline.
*/

#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <Wire.h>
#include <EEPROM.h>
long t1, t2;

SGP30 mySensor; //create an object of the SGP30 class
byte baselineC02Addr = 0x00;
byte baselineTVOCAddr = 0x02;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);
  if (mySensor.begin() == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();
  //First fifteen readings will be
  //CO2: 400 ppm  TVOC: 0 ppb
  //We will reset after 30 readings
  for (byte count = 0; count < 30; count++)
  {
    delay(1000); //Wait 1 second
    mySensor.measureAirQuality();
    Serial.print("CO2: ");
    Serial.print(mySensor.CO2);
    Serial.print(" ppm\t TVOC: ");
    Serial.print(mySensor.TVOC);
    Serial.println(" ppb");
  }
  //get the sensor's current baseline
  mySensor.getBaseline();
  //store baselineCO2 in location baselineCO2Addr
  EEPROM.put(baselineC02Addr, mySensor.baselineCO2);
  //store baselineTVOC in location baselineTVOCAddr
  EEPROM.put(baselineTVOCAddr, mySensor.baselineTVOC);
  Serial.println("Saved Baseline");
  //Soft reset. Will reset other devices if they support general call mode
  mySensor.generalCallReset();
  Serial.println("Reset Sensor");
  //get baselineCO2 from location baselineCO2Addr
  EEPROM.get(baselineC02Addr, mySensor.baselineCO2);
  //get baselineTVOC from location baselineTVOCAddr
  EEPROM.get(baselineTVOCAddr, mySensor.baselineTVOC);
  //delay to allow sensor to get to idle state
  delay(1);
  //A new initAirQuality” command has to be sent after every power-up or soft reset.
  mySensor.initAirQuality();
  //set the sensor's baseline to the previously gotten baseline
  mySensor.setBaseline(mySensor.baselineCO2, mySensor.baselineTVOC);
  Serial.println("Updated Baseline");
  t1 = millis();
}

void loop() {
  t2 = millis();
  if ( t2 >= t1 + 1000) //only will occur if 1 second has passed
  {
    t1 = t2;  //measure CO2 and TVOC levels
    mySensor.measureAirQuality();
    Serial.print("CO2: ");
    Serial.print(mySensor.CO2);
    Serial.print(" ppm\tTVOC: ");
    Serial.print(mySensor.TVOC);
    Serial.println(" ppb");
  }
}

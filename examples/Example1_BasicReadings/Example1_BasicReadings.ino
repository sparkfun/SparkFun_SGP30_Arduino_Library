#include "SparkFun_SGP30_Library.h"
#include <Wire.h>

SGP30 mySensor; //create an object of the SGP30 class
byte count = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);
  mySensor.begin();
  mySensor.initAirQuality();
  //ignore first 15 readings
  while (count < 15) {
    delay(1000); //Wait 1 second
    mySensor.measureAirQuality();
    count++;
  }
}

void loop() {
  delay(1000); //Wait 1 second
  mySensor.measureAirQuality();
  Serial.print("CO2: ");
  Serial.print(mySensor.CO2);
  Serial.print(" ppm\tTVOC");
  Serial.print(mySensor.TVOC);
  Serial.println(" ppb");

}

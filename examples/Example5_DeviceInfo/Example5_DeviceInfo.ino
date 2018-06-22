#include "SparkFun_SGP30_Library.h"
#include <Wire.h>

SGP30 mySensor; //create an object of the SGP30 class
byte count = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);
  mySensor.begin();
  mySensor.getSerialID();
  mySensor.getFeatureSetVersion();
  Serial.print("SerialID: 0x");
  Serial.print((uint32_t)(mySensor.serialID >> 32), HEX);
  Serial.print((uint32_t)mySensor.serialID, HEX);
  Serial.print("\tFeature Set Version: 0x");
  Serial.print(mySensor.featureSetVersion, HEX);
}

void loop() {

}

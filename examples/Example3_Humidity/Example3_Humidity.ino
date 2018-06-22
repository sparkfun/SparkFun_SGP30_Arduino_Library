/*


  This example uses the Si7021 to get relative humidity
*/


#include "SparkFun_SGP30_Library.h"
#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>

SGP30 mySensor; //create an instance of the SGP30 class
Weather hSensor; //create an instance of the Weather class

byte count = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);
  //Initialize the SGP30 
  mySensor.begin();
  
  //Initialize the humidity sensor and ping it
  hSensor.begin();
  // Measure Relative Humidity from the Si7021
  float humidity = hSensor.getRH();
  //Measure temperature (in C) from the Si7021
  float temperature = hSensor.getTemp();
  //Convert relative humidity to absolute humidity
  double absHumidity = RHtoAbsolute(humidity, temperature);
  //Convert the double type humidity to a fixed point 8.8bit number
  uint16_t sensHumidity = doubleToFixedPoint(absHumidity);
  //Set the humidity compensation on the SGP30 to the measured value
  mySensor.setHumidity(sensHumidity);
  
  //ignore first 15 readings
  mySensor.initAirQuality();
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

double RHtoAbsolute (float relHumidity, float tempC) {
  double Es = 6.11 * pow(10.0, (7.5 * tempC / (237.7 + tempC)));
  double vaporPressure = (relHumidity * Es) / 100; //millibars
  double absHumidity = 1000 * vaporPressure * 100 / ((tempC + 273) * 461.5); //Ideal gas law with unit conversions
}

uint16_t doubleToFixedPoint( double number) {
  int power = 1 << 8;
  double number2 = number * power;
  uint16_t value = floor(number2 + 0.5);
  return value;
}



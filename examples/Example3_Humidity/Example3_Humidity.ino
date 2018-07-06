/*
  Library for the Sensirion SGP30 Indoor Air Quality Sensor
  By: Ciara Jekel
  SparkFun Electronics
  Date: June 28th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SGP30 Datasheet: https://cdn.sparkfun.com/assets/c/0/a/2/e/Sensirion_Gas_Sensors_SGP30_Datasheet.pdf

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813

  This example gets relative humidity from a sensor, convertts it to absolute humidty,
  and updates the SGP30's humidity compensation with the absolute humidity value.
*/

#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include "SparkFun_Si7021_Breakout_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_Si7021
#include <Wire.h>

SGP30 mySensor; //create an instance of the SGP30 class
Weather hSensor; //create an instance of the Weather class
long t1, t2;

byte count = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);
  //Initialize the SGP30
  if (mySensor.begin() == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }

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
  //Initializes sensor for air quality readings
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();
  //Set the humidity compensation on the SGP30 to the measured value
  //If no humidity sensor attached, sensHumidity should be 0 and sensor will use default
  mySensor.setHumidity(sensHumidity);
  t1 = millis();
}

void loop() {
  //First fifteen readings will be
  //CO2: 400 ppm  TVOC: 0 ppb
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

double RHtoAbsolute (float relHumidity, float tempC) {
  double eSat = 6.11 * pow(10.0, (7.5 * tempC / (237.7 + tempC)));
  double vaporPressure = (relHumidity * eSat) / 100; //millibars
  double absHumidity = 1000 * vaporPressure * 100 / ((tempC + 273) * 461.5); //Ideal gas law with unit conversions
}

uint16_t doubleToFixedPoint( double number) {
  int power = 1 << 8;
  double number2 = number * power;
  uint16_t value = floor(number2 + 0.5);
  return value;
}



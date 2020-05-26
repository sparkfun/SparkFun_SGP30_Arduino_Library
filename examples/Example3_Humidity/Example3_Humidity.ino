/*
  Library for the Sensirion SGP30 Indoor Air Quality Sensor
  By: Ciara Jekel
  SparkFun Electronics
  Date: June 28th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SGP30 Datasheet: https://cdn.sparkfun.com/assets/c/0/a/2/e/Sensirion_Gas_Sensors_SGP30_Datasheet.pdf

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813

  This example gets relative humidity from a sensor, converts it to absolute humidty,
  and updates the SGP30's humidity compensation with the absolute humidity value.
*/

#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3
#include <Wire.h>

SGP30 mySensor; //create an instance of the SGP30 class
SHTC3 humiditySensor; //create an instance of the SHTC3 class
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
  humiditySensor.begin();
  // Call "update()" to command a measurement, wait for measurement to complete, and update the RH and T members of the object 
  SHTC3_Status_TypeDef result = humiditySensor.update();
  delay(190);  
  // Measure Relative Humidity from the SHTC3
  float humidity = humiditySensor.toPercent();

  //Measure temperature (in C) from the SHTC3
  float temperature = humiditySensor.toDegC();

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
  Serial.print("Absolute humidity compensation set to: ");
  Serial.print(absHumidity);
  Serial.println("g/m^3 ");
  delay(100);
  t1 = millis();
}

void loop() {
  //First fifteen readings will be
  //CO2: 400 ppm  TVOC: 0 ppb
  if ( millis() >= t1 + 1000) //only will occur if 1 second has passed
  {
    t1 = millis();  //measure CO2 and TVOC levels
    mySensor.measureAirQuality();
    Serial.print("CO2: ");
    Serial.print(mySensor.CO2);
    Serial.print(" ppm\tTVOC: ");
    Serial.print(mySensor.TVOC);
    Serial.println(" ppb");
  }
  if (Serial.available()) //check if new data is available on serial port
  {
    char ch = Serial.read(); 
    if (ch == 'h' || ch == 'H') //check if the char input matches either "h" or "H" and if it does, run the compensation routine from the setup
    {
      SHTC3_Status_TypeDef result = humiditySensor.update();
      delay(190);  
      // Measure Relative Humidity from the SHTC3
      float humidity = humiditySensor.toPercent();
    
      //Measure temperature (in C) from the SHTC3
      float temperature = humiditySensor.toDegC();
    
      //Convert relative humidity to absolute humidity
      double absHumidity = RHtoAbsolute(humidity, temperature);
    
      //Convert the double type humidity to a fixed point 8.8bit number
      uint16_t sensHumidity = doubleToFixedPoint(absHumidity);
    
      //Set the humidity compensation on the SGP30 to the measured value
      //If no humidity sensor attached, sensHumidity should be 0 and sensor will use default
      mySensor.setHumidity(sensHumidity);
      Serial.print("Absolute Humidity Compensation set to: ");
      Serial.print(absHumidity);
      Serial.println("g/m^3 ");
      delay(100);      
    }
  }
}

double RHtoAbsolute (float relHumidity, float tempC) {
  double eSat = 6.11 * pow(10.0, (7.5 * tempC / (237.7 + tempC)));
  double vaporPressure = (relHumidity * eSat) / 100; //millibars
  double absHumidity = 1000 * vaporPressure * 100 / ((tempC + 273) * 461.5); //Ideal gas law with unit conversions
  return absHumidity;
}

uint16_t doubleToFixedPoint( double number) {
  int power = 1 << 8;
  double number2 = number * power;
  uint16_t value = floor(number2 + 0.5);
  return value;
}

#include "SparkFun_SGP30_Library.h"
#include <Wire.h>

SGP30 mySensor; //create an object of the SGP30 class
byte count = 0;
bool intrpt = false;

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
  configureTimer1();
}

void loop() {
  if (intrpt == true) {
    Serial.print("CO2: ");
    Serial.print(mySensor.CO2);
    Serial.print(" ppm\tTVOC");
    Serial.print(mySensor.TVOC);
    Serial.println(" ppb");
    intrpt = false;
  }
}

void configureTimer1(void) {
  //https://www.instructables.com/id/Arduino-Timer-Interrupts/
  cli();//stop interrupts
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
}

ISR(TIMER1_COMPA_vect) {
  mySensor.measureAirQuality();
  intrpt = true;
}


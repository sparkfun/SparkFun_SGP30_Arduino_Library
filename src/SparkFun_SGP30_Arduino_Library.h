/*
  This is a library written for the SPG30
  By Ciara Jekel @ SparkFun Electronics, June 18th, 2018


  https://github.com/sparkfun/SparkFun_SGP30_Arduino_Library

  Development environment specifics:
  Arduino IDE 1.8.5

  SparkFun labored with love to create this code. Feel like supporting open
  source hardware? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813


  CRC lookup table from Bastian Molkenthin  http://www.sunshine2k.de/coding/javascript/crc/crc_js.html

  Copyright (c) 2015 Bastian Molkenthin

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/


#ifndef SparkFun_SGP30_Arduino_Library_h
#define SparkFun_SGP30_Arduino_Library_h

#include "Arduino.h"
#include <Wire.h>
typedef enum {
  SUCCESS = 0,
  ERR_BAD_CRC,
  ERR_I2C_TIMEOUT,
  SELF_TEST_FAIL
} SGP30ERR;

const uint8_t init_air_quality[2] = {0x20, 0x03};
const uint8_t measure_air_quality[2] = {0x20, 0x08};
const uint8_t get_baseline[2] = {0x20, 0x15};
const uint8_t set_baseline[2] = {0x20, 0x1E};
const uint8_t set_humidity[2] = {0x20, 0x61};
const uint8_t measure_test[2] = {0x20, 0x32};
const uint8_t get_feature_set_version[2] = {0x20, 0x2F};
const uint8_t get_serial_id[2] = {0x36, 0x82};
const uint8_t measure_raw_signals[2] = {0x20, 0x50};

class SGP30
{
    // user-accessible "public" interface
  public:

    uint16_t CO2;
    uint16_t TVOC;
    uint16_t baselineCO2;
    uint16_t baselineTVOC;
    uint16_t featureSetVersion;
    uint16_t H2;
    uint16_t ethanol;
    uint64_t serialID;

    //default constructor
    SGP30();

    //Start I2C communication using specified port
    bool begin(TwoWire &wirePort = Wire);//If user doesn't specificy then Wire will be used

    //Initializes sensor for air quality readings
    void initAirQuality(void);

    //Measure air quality
    //Call in regular intervals of 1 second to maintain synamic baseline calculations
    //CO2 returned in ppm, Total Volatile Organic Compounds (TVOC) returned in ppb
    //Will give fixed values of CO2=400 and TVOC=0 for first 15 seconds after init
    //returns false if CRC8 check failed and true if successful
    SGP30ERR measureAirQuality(void);

    //Returns the current calculated baseline from
    //the sensor's dynamic baseline calculations
    //Save baseline periodically to non volatile memory
    //(like EEPROM) to restore after new power up or
    //after soft reset using setBaseline();
    //returns false if CRC8 check failed and true if successful
    SGP30ERR getBaseline(void);

    //Updates the baseline to a previous baseline
    //Should only use with previously retrieved baselines
    //to maintain accuracy
    void setBaseline(uint16_t baselineCO2, uint16_t baselineTVOC);

    //Set humidity
    //humidity value is a fixed point 8.8 bit number
    //Value should be absolute humidity from humidity sensor
    //default value 0x0F80 = 15.5g/m^3
    //minimum value 0x0001 = 1/256g/m^3
    //maximum value 0xFFFF = 255+255/256 g/m^3
    //sending 0x0000 resets to default and turns off humidity compensation
    void setHumidity(uint16_t humidity);

    //gives feature set version number (see data sheet)
    //returns false if CRC8 check failed and true if successful
    SGP30ERR getFeatureSetVersion(void);

    //Intended for part verification and testing
    //these raw signals are used as inputs to the onchip calibrations and algorithms
    SGP30ERR measureRawSignals(void);


    //Soft reset - not device specific
    //will reset all devices that support general call mode
    void generalCallReset(void);

    //readout of serial ID register can identify chip and verify sensor presence
    //returns false if CRC8 check failed and true if successful
    SGP30ERR getSerialID(void);

    //Sensor runs on chip self test
    //returns true if successful
    SGP30ERR measureTest(void);


  private:
    //This stores the requested i2c port
    TwoWire * _i2cPort;

    //SGP30's I2C address
    const byte _SGP30Address = 0x58;

    //Generates CRC8 for SGP30 from lookup table
    uint8_t _CRC8(uint16_t twoBytes);

#ifdef SGP30_LOOKUP_TABLE
    //lookup table for CRC8  http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
    const uint8_t _CRC8LookupTable[16][16] = {
      {0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97, 0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E},
      {0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4, 0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D},
      {0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11, 0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8},
      {0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52, 0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB},
      {0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA, 0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13},
      {0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9, 0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50},
      {0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C, 0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95},
      {0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F, 0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6},
      {0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED, 0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54},
      {0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE, 0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17},
      {0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B, 0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2},
      {0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28, 0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91},
      {0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0, 0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69},
      {0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93, 0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A},
      {0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56, 0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF},
      {0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15, 0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC}
    };
#endif

};

#endif

/*
  This is a library written for the SPG30
  By Ciara Jekel @ SparkFun Electronics, June 18th, 2018


  https://github.com/sparkfun/SparkFun_SGP30_Arduino_Library

  Development environment specifics:
  Arduino IDE 1.8.5

  SparkFun labored with love to create this code. Feel like supporting open
  source hardware? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813


  CRC lookup table from Bastian Molkenthin

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


#include "SparkFun_SGP30_Arduino_Library.h"


//Constructor
SGP30::SGP30() {
  CO2 = 0;
  TVOC = 0;
  baselineCO2 = 0;
  baselineTVOC = 0;
  featureSetVersion = 0;
  H2 = 0;
  ethanol = 0;
  serialID = 0;
}

//Start I2C communication using specified port
//Returns true if successful or false if no sensor detected
bool SGP30::begin(TwoWire &wirePort) {
  _i2cPort = &wirePort; //Grab which port the user wants us to use
  _i2cPort->begin();
  getSerialID();
  if (serialID == 0) return false;
  return true;
}


//Initilizes sensor for air quality readings
//measureAirQuality should be called in 1 second intervals after this function
void SGP30::initAirQuality(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(init_air_quality, 2); //command to initialize air quality readings
  _i2cPort->endTransmission();
}

//Measure air quality
//Call in regular intervals of 1 second to maintain synamic baseline calculations
//CO2 returned in ppm, Total Volatile Organic Compounds (TVOC) returned in ppb
//Will give fixed values of CO2=400 and TVOC=0 for first 15 seconds after init
//Returns SUCCESS if successful or other error code if unsuccessful
SGP30ERR SGP30::measureAirQuality(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(measure_air_quality, 2); //command to measure air quality
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 10-12ms
  delay(12);
  //Comes back in 6 bytes, CO2 data(MSB) / data(LSB) / Checksum / TVOC data(MSB) / data(LSB) / Checksum
  uint8_t toRead;
  toRead = _i2cPort->requestFrom(_SGP30Address, (uint8_t)6);
  if (toRead != 6) return ERR_I2C_TIMEOUT; //Error out
  uint16_t _CO2 = _i2cPort->read() << 8; //store MSB in CO2
  _CO2 |= _i2cPort->read(); //store LSB in CO2
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(_CO2)) return ERR_BAD_CRC; //checksum failed
  uint16_t _TVOC = _i2cPort->read() << 8; //store MSB in TVOC
  _TVOC |= _i2cPort->read(); //store LSB in TVOC
  checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(_TVOC)) return ERR_BAD_CRC; //checksum failed
  CO2 = _CO2; //publish valid data
  TVOC = _TVOC; //publish valid data
  return SUCCESS;
}

//Returns the current calculated baseline from
//the sensor's dynamic baseline calculations
//Save baseline periodically to non volatile memory
//(like EEPROM) to restore after new power up or
//after soft reset using setBaseline();
//Returns SUCCESS if successful or other error code if unsuccessful
SGP30ERR SGP30::getBaseline(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(get_baseline, 2);
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 10ms
  delay(10);
  uint8_t toRead;
  //Comes back in 6 bytes, baselineCO2 data(MSB) / data(LSB) / Checksum / baselineTVOC data(MSB) / data(LSB) / Checksum
  toRead = _i2cPort->requestFrom(_SGP30Address, (uint8_t)6);
  if (toRead != 6) return ERR_I2C_TIMEOUT; //Error out
  uint16_t _baselineCO2 = _i2cPort->read() << 8; //store MSB in _baselineCO2
  _baselineCO2 |= _i2cPort->read(); //store LSB in _baselineCO2
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(_baselineCO2)) return ERR_BAD_CRC; //checksum failed
  uint16_t _baselineTVOC = _i2cPort->read() << 8; //store MSB in _baselineTVOC
  _baselineTVOC |= _i2cPort->read(); //store LSB in _baselineTVOC
  checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(_baselineTVOC)) return ERR_BAD_CRC; //checksum failed
  baselineCO2 = _baselineCO2; //publish valid data
  baselineTVOC = _baselineTVOC; //publish valid data
  return SUCCESS;
}

//Updates the baseline to a previous baseline
//Should only use with previously retrieved baselines
//to maintain accuracy
void SGP30::setBaseline(uint16_t baselineCO2, uint16_t baselineTVOC) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(set_baseline, 2); //command to set baseline
  _i2cPort->write(baselineTVOC >> 8); //write baseline TVOC MSB
  _i2cPort->write(baselineTVOC); //write baseline TVOC LSB
  _i2cPort->write(_CRC8(baselineTVOC)); //write checksum TVOC baseline
  _i2cPort->write(baselineCO2 >> 8); //write baseline CO2 MSB
  _i2cPort->write(baselineCO2); //write baseline CO2 LSB
  _i2cPort->write(_CRC8(baselineCO2)); //write checksum CO2 baseline
  _i2cPort->endTransmission();
}

//Set humidity
//humidity value is a fixed point 8.8 bit number
//Value should be absolute humidity from humidity sensor
//default value 0x0F80 = 15.5g/m^3
//minimum value 0x0001 = 1/256g/m^3
//maximum value 0xFFFF = 255+255/256 g/m^3
//sending 0x0000 resets to default and turns off humidity compensation
void SGP30::setHumidity(uint16_t humidity) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(set_humidity, 2); //command to set humidity
  _i2cPort->write(humidity >> 8); //write humidity MSB
  _i2cPort->write(humidity); //write humidity LSB
  _i2cPort->write(_CRC8(humidity)); //write humidity checksum
  _i2cPort->endTransmission();
}

//gives feature set version number (see data sheet)
//Returns SUCCESS if successful or other error code if unsuccessful
SGP30ERR SGP30::getFeatureSetVersion(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(get_feature_set_version, 2); //command to get feature version
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 1-2ms
  delay(2);
  uint8_t toRead;
  //Comes back in 3 bytes, data(MSB) / data(LSB) / Checksum
  toRead = _i2cPort->requestFrom(_SGP30Address, (uint8_t)3);
  if (toRead != 3) return ERR_I2C_TIMEOUT; //Error out
  uint16_t _featureSetVersion = _i2cPort->read() << 8; //store MSB in featureSetVerison
  _featureSetVersion |= _i2cPort->read(); //store LSB in featureSetVersion
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(_featureSetVersion)) return ERR_BAD_CRC; //checksum failed
  featureSetVersion = _featureSetVersion; //publish valid data
  return SUCCESS;
}

//Intended for part verification and testing
//these raw signals are used as inputs to the onchip calibrations and algorithms
//Returns SUCCESS if successful or other error code if unsuccessful
SGP30ERR SGP30::measureRawSignals(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(measure_raw_signals, 2); //command to measure raw signals
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 20-25ms
  delay(25);
  uint8_t toRead;
  //Comes back in 6 bytes, H2 data(MSB) / data(LSB) / Checksum / ethanol data(MSB) / data(LSB) / Checksum
  toRead = _i2cPort->requestFrom(_SGP30Address, (uint8_t)6);
  if (toRead != 6) return ERR_I2C_TIMEOUT; //Error out
  uint16_t _H2 = _i2cPort->read() << 8; //store MSB in _H2
  _H2 |= _i2cPort->read(); //store LSB in _H2
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(_H2)) return ERR_BAD_CRC; //checksumfailed
  uint16_t _ethanol = _i2cPort->read() << 8; //store MSB in ethanol
  _ethanol |= _i2cPort->read(); //store LSB in ethanol
  checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(_ethanol)) return ERR_BAD_CRC; //checksum failed
  H2 = _H2; //publish valid data
  ethanol = _ethanol; //publish valid data
  return SUCCESS;
}

//Soft reset - not device specific
//will reset all devices that support general call mode
void SGP30::generalCallReset(void) {
  _i2cPort->beginTransmission(0x00); //general call address
  _i2cPort->write(0x06); //reset command
  _i2cPort->endTransmission();
}

//readout of serial ID register can identify chip and verify sensor presence
//Returns SUCCESS if successful or other error code if unsuccessful
SGP30ERR SGP30::getSerialID(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(get_serial_id, 2); //command to get serial ID
  _i2cPort->endTransmission();
  //Hang out while measurement is taken.
  delay(1);
  uint8_t toRead;
  //Comes back in 9 bytes, H2 data(MSB) / data(LSB) / Checksum / ethanol data(MSB) / data(LSB) / Checksum
  toRead = _i2cPort->requestFrom(_SGP30Address, (uint8_t)9);
  if (toRead != 9) return ERR_I2C_TIMEOUT; //Error out
  uint16_t _serialID1 = _i2cPort->read() << 8; //store MSB to top of _serialID1
  _serialID1 |= _i2cPort->read(); //store next byte in _serialID1
  uint8_t checkSum1 = _i2cPort->read(); //verify checksum
  if (checkSum1 != _CRC8(_serialID1)) return ERR_BAD_CRC; //checksum failed
  uint16_t _serialID2 = _i2cPort->read() << 8; //store next byte to top of _serialID2
  _serialID2 |= _i2cPort->read(); //store next byte in _serialID2
  uint8_t checkSum2 = _i2cPort->read(); //verify checksum
  if (checkSum2 != _CRC8(_serialID2)) return ERR_BAD_CRC; //checksum failed
  uint16_t _serialID3 = _i2cPort->read() << 8; //store next byte to top of _serialID3
  _serialID3 |= _i2cPort->read() ; //store LSB in _serialID3
  uint8_t checkSum3 = _i2cPort->read(); //verify checksum
  if (checkSum3 != _CRC8(_serialID3)) return ERR_BAD_CRC; //checksum failed
  serialID = ((uint64_t)_serialID1 << 32) + ((uint64_t)_serialID2 << 16) + ((uint64_t)_serialID3); //publish valid data
  return SUCCESS;
}

//Sensor runs on chip self test
//Returns SUCCESS if successful or other error code if unsuccessful
SGP30ERR SGP30::measureTest(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(measure_test, 2); //command to get self test
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 200-220ms
  delay(220);
  uint8_t toRead;
  //Comes back in 3 bytes, data(MSB) / data(LSB) / Checksum
  toRead = _i2cPort->requestFrom(_SGP30Address, (uint8_t)3);
  if (toRead != 3) return ERR_I2C_TIMEOUT; //Error out
  uint16_t results = _i2cPort->read() << 8; //store MSB in results
  results |= _i2cPort->read(); //store LSB in results
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(results)) return ERR_BAD_CRC; //checksum failed
  if (results != 0xD400) return SELF_TEST_FAIL; //self test results incorrect
  return SUCCESS;
}

#ifndef SGP30_LOOKUP_TABLE
//Given an array and a number of bytes, this calculate CRC8 for those bytes
//CRC is only calc'd on the data portion (two bytes) of the four bytes being sent
//From: http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
//Tested with: http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
//x^8+x^5+x^4+1 = 0x31
uint8_t SGP30::_CRC8(uint16_t data)
{
  uint8_t crc = 0xFF; //Init with 0xFF

  crc ^= (data >> 8); // XOR-in the first input byte

  for (uint8_t i = 0 ; i < 8 ; i++)
  {
    if ((crc & 0x80) != 0)
      crc = (uint8_t)((crc << 1) ^ 0x31);
    else
      crc <<= 1;
  }
  crc ^= (uint8_t)data; // XOR-in the last input byte

  for (uint8_t i = 0 ; i < 8 ; i++)
  {
    if ((crc & 0x80) != 0)
      crc = (uint8_t)((crc << 1) ^ 0x31);
    else
      crc <<= 1;
  }

  return crc; //No output reflection
}
#endif

#ifdef SGP30_LOOKUP_TABLE
//Generates CRC8 for SGP30 from lookup table
uint8_t SGP30::_CRC8(uint16_t data) {
  uint8_t CRC = 0xFF; //inital value
  CRC ^= (uint8_t)(data >> 8); //start with MSB
  CRC = _CRC8LookupTable[CRC >> 4][CRC & 0xF]; //look up table [MSnibble][LSnibble]
  CRC ^= (uint8_t)data; //use LSB
  CRC = _CRC8LookupTable[CRC >> 4][CRC & 0xF]; //look up table [MSnibble][LSnibble]
  return CRC;
}
#endif



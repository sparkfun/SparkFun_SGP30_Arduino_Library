/*
  This is a library written for the SPG30 air quality sensor
  By Ciara Jekel @ SparkFun Electronics, June 18th, 2018

  The SGP30 is an indoor air quality sensor equipped with an I²C interface.


  https://github.com/sparkfun/SparkFun_SGP30_Arduino_Library

  Development environment specifics:
  Arduino IDE 1.8.5

  SparkFun labored with love to create this code. Feel like supporting open
  source hardware? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14813
*/

#include "SparkFun_SGP30_Library.h"

const uint8_t init_air_quality[2] = {0x20, 0x03};
const uint8_t measure_air_quality[2] = {0x20, 0x08};
const uint8_t get_baseline[2] = {0x20, 0x15};
const uint8_t set_baseline[2] = {0x20, 0x1E};
const uint8_t set_humidity[2] = {0x20, 0x61};
const uint8_t measure_test[2] = {0x20, 0x32};
const uint8_t get_feature_set_version[2] = {0x20, 0x2F};
const uint8_t get_serial_id[2] = {0x36, 0x82};
const uint8_t measure_raw_signals[2] = {0x20, 0x50};

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
SGP30ERR SGP30::begin(TwoWire &wirePort) {
  _i2cPort = &wirePort; //Grab which port the user wants us to use
  _i2cPort->begin();
  return SUCCESS;
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
//returns ERR_BAD_CRC if CRC8 check failed and SUCCESS if successful
SGP30ERR SGP30::measureAirQuality(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(measure_air_quality, 2); //command to measure air quality
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 10-12ms
  byte toRead;
  byte counter;
  for (counter = 0, toRead = 0 ; counter < 12 && toRead != 6 ; counter++)
  {
    delay(1);

    //Comes back in 6 bytes, CO2 data(MSB) / data(LSB) / Checksum / TVOC data(MSB) / data(LSB) / Checksum
    toRead = _i2cPort->requestFrom(_SGP30Address, 6);
  }
  if (counter == 12) return ERR_I2C_TIMEOUT; //Error out
  CO2 = _i2cPort->read() << 8; //store MSB in CO2
  CO2 |= _i2cPort->read(); //store LSB in CO2
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(CO2)) return ERR_BAD_CRC; //checksum failed
  TVOC = _i2cPort->read() << 8; //store MSB in TVOC
  TVOC |= _i2cPort->read(); //store LSB in TVOC
  checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(TVOC)) return ERR_BAD_CRC; //checksum failed
  _i2cPort->endTransmission();
  return SUCCESS;
}

//Returns the current calculated baseline from
//the sensor's dynamic baseline calculations
//Save baseline periodically to non volatile memory
//(like EEPROM) to restore after new power up or
//after soft reset using setBaseline();
//returns ERR_BAD_CRC if CRC8 check failed and SUCCESS if successful
SGP30ERR SGP30::getBaseline(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(get_baseline, 2);
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 10ms
  byte toRead;
  byte counter;
  for (counter = 0, toRead = 0 ; counter < 12 && toRead != 6 ; counter++)
  {
    delay(1);

    //Comes back in 6 bytes, baselineCO2 data(MSB) / data(LSB) / Checksum / baselineTVOC data(MSB) / data(LSB) / Checksum
    toRead = _i2cPort->requestFrom(_SGP30Address, 6);
  }
  if (counter == 12) return ERR_I2C_TIMEOUT; //Error out
  baselineCO2 = _i2cPort->read() << 8; //store MSB in baselineCO2
  baselineCO2 |= _i2cPort->read(); //store LSB in baselineCO2
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(baselineCO2)) return ERR_BAD_CRC; //checksum failed
  baselineTVOC = _i2cPort->read() << 8; //store MSB in baselineTVOC
  baselineTVOC |= _i2cPort->read(); //store LSB in baselineTVOC
  checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(baselineTVOC)) return ERR_BAD_CRC; //checksum failed
  _i2cPort->endTransmission();
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
//returns ERR_BAD_CRC if CRC8 check failed and SUCCESS if successful
SGP30ERR SGP30::getFeatureSetVersion(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(get_feature_set_version, 2); //command to get feature version
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 1-2ms
  byte toRead;
  byte counter;
  for (counter = 0, toRead = 0 ; counter < 3 && toRead != 3 ; counter++)
  {
    delay(1);

    //Comes back in 3 bytes, data(MSB) / data(LSB) / Checksum
    toRead = _i2cPort->requestFrom(_SGP30Address, 3);
  }
  if (counter == 3) return ERR_I2C_TIMEOUT; //Error out
  featureSetVersion = _i2cPort->read() << 8; //store MSB in featureSetVerison
  featureSetVersion |= _i2cPort->read(); //store LSB in featureSetVersion
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(featureSetVersion)) return ERR_BAD_CRC; //checksum failed
  _i2cPort->endTransmission();
  return SUCCESS;
}

//Intended for part verification and testing
//these raw signals are used as inputs to the onchip calibrations and algorithms
SGP30ERR SGP30::measureRawSignals(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(measure_raw_signals, 2); //command to measure raw signals
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 20-25ms
  byte toRead;
  byte counter;
  for (counter = 0, toRead = 0 ; counter < 5 && toRead != 6 ; counter++)
  {
    delay(5);

    //Comes back in 6 bytes, H2 data(MSB) / data(LSB) / Checksum / ethanol data(MSB) / data(LSB) / Checksum
    toRead = _i2cPort->requestFrom(_SGP30Address, 6);
  }
  if (counter == 5) return ERR_I2C_TIMEOUT; //Error out
  H2 = _i2cPort->read() << 8; //store MSB in H2
  H2 |= _i2cPort->read(); //store LSB in H2
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(H2)) return ERR_BAD_CRC; //checksumfailed
  ethanol = _i2cPort->read() << 8; //store MSB in ethanol
  ethanol |= _i2cPort->read(); //store LSB in ethanol
  checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != _CRC8(ethanol)) return ERR_BAD_CRC; //checksum failed
  _i2cPort->endTransmission();
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
//returns ERR_BAD_CRC if CRC8 check failed and SUCCESS if successful
SGP30ERR SGP30::getSerialID(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(get_serial_id, 2); //command to get serial ID
  _i2cPort->endTransmission();
  //Hang out while measurement is taken.
  byte toRead;
  byte counter;
  for (counter = 0, toRead = 0 ; counter < 5 && toRead != 9 ; counter++)
  {
    delay(1);

    //Comes back in 9 bytes, H2 data(MSB) / data(LSB) / Checksum / ethanol data(MSB) / data(LSB) / Checksum
    toRead = _i2cPort->requestFrom(_SGP30Address, 9);
  }
  if (counter == 5) return ERR_I2C_TIMEOUT; //Error out
  serialID = _i2cPort->read() << 40; //store MSB to top of serialID
  serialID |= _i2cPort->read() << 32; //store next byte in serialID
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != (uint16_t)(serialID >> 32)) return ERR_BAD_CRC; //checksum failed
  serialID = _i2cPort->read() << 24; //store next byte in serialID
  serialID |= _i2cPort->read() << 16; //store next byte in serialID
  checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != (uint16_t)(serialID >> 16)) return ERR_BAD_CRC; //checksum failed
  serialID = _i2cPort->read() << 8; //store next byte in serialID
  serialID |= _i2cPort->read() ; //store LSB in serialID
  checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != (uint16_t)serialID) return ERR_BAD_CRC; //checksum failed
  _i2cPort->endTransmission();
  return SUCCESS;
}

//Sensor runs on chip self test
//returns SUCCESS if successful
SGP30ERR SGP30::measureTest(void) {
  _i2cPort->beginTransmission(_SGP30Address);
  _i2cPort->write(measure_test, 2); //command to get self test
  _i2cPort->endTransmission();
  //Hang out while measurement is taken. datasheet says 200-220ms
  byte toRead;
  byte counter;
  for (counter = 0, toRead = 0 ; counter < 22 && toRead != 3 ; counter++)
  {
    delay(10);

    //Comes back in 3 bytes, data(MSB) / data(LSB) / Checksum
    toRead = _i2cPort->requestFrom(_SGP30Address, 3);
  }
  if (counter == 22) return ERR_I2C_TIMEOUT; //Error out
  uint16_t results;
  results = _i2cPort->read() << 8; //store MSB in results
  results = _i2cPort->read(); //store LSB in results
  uint8_t checkSum = _i2cPort->read(); //verify checksum
  if (checkSum != results) return ERR_BAD_CRC; //checksum failed
  if (results != 0xD400) return SELF_TEST_FAIL; //self test results incorrect
  _i2cPort->endTransmission();
  return SUCCESS;
}

//Generates CRC8 for SGP30 from lookup table
uint8_t SGP30::_CRC8(uint16_t twoBytes) {
  uint8_t CRC = 0xFF; //inital value
  CRC ^= (uint8_t)(twoBytes >> 8); //start with MSB
  CRC = _CRC8LookupTable[CRC >> 4][CRC & 0xF]; //look up table [MSnibble][LSnibble]
  CRC ^= (uint8_t)twoBytes; //use LSB
  CRC = _CRC8LookupTable[CRC >> 4][CRC & 0xF]; //look up table [MSnibble][LSnibble]
  return CRC;
}




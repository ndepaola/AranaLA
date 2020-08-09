#ifndef AranaBase_h
#define AranaBase_h

#include "Arduino.h"
#include <Wire.h>
#include <RtcDS3231.h> // use this or the one below?
// #include <DS3231M.h>
#include "limits.h"
#include <stdint.h>
#include <stdio.h>
#include "SD.h"
#include <SPI.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>

class AranaBase
{
public:
  void setup_RF(RH_RF95 &rf95);
  void setup_serial();

protected:
  int _RFM95_MISO, _RFM95_MOSI, _RFM95_RST, _RFM95_CS, _SD_CS, _LED;
  const int MSG_LENGTH = 8;
};

#endif

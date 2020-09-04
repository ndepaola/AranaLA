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

  // message types
  static const int MSG_START = 1;
  static const int MSG_STOP = 0;
  static const int MSG_FALSE_START = 2;
  static const int MSG_SYNC = 3;

  // system IDs
  static const int ID_GLOBAL = 0;
  static const int ID_START = 1;
  static const int ID_DB = 2;
  static const int ID_SPEAKER0 = 3;
  // const int ID_SPEAKER1 4; // TODO: for when we add a second speaker

protected:
  int _RFM95_MISO, _RFM95_MOSI, _RFM95_RST, _RFM95_CS, _SD_CS, _LED;
  const int MSG_OFFSET = 3; // number of bytes in message preamble (before msg time)
  const int MSG_LENGTH = sizeof(unsigned long) + MSG_OFFSET;
};

#endif

#ifndef AranaSend_h
#define AranaSend_h

#include "AranaBase.h"

class AranaSend : public AranaBase
{
public:
    AranaSend(int RFM95_MISO, int RFM95_MOSI, int RFM95_RST, int RFM95_CS, int SD_CS = NULL);
    void setup_SD();
    void send_RF_message(RH_RF95 &rf95, unsigned int targetID, unsigned int msgType, unsigned int msgID, unsigned long absoluteTime = NULL);
    void write_to_SD(int msgType, unsigned long msgTime, unsigned long syncTime);
};

#endif
#ifndef AranaSend_h
#define AranaSend_h

#include "AranaBase.h"

class AranaReceive : public AranaBase
{
public:
    AranaReceive(int RFM95_MISO, int RFM95_MOSI, int RFM95_RST, int RFM95_CS);
    bool receive_RF_message(RH_RF95 &rf95, unsigned int targetID, unsigned int &msgType, unsigned int &msgID, unsigned long &msgTime);
};

#endif
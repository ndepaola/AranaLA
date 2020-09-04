#include "Arduino.h"
#include "AranaReceive.h"

// Class constructor
AranaReceive::AranaReceive(int RFM95_MISO, int RFM95_MOSI, int RFM95_RST, int RFM95_CS)
{
    _RFM95_MISO = RFM95_MISO;
    _RFM95_MOSI = RFM95_MOSI;
    _RFM95_RST = RFM95_RST;
    _RFM95_CS = RFM95_CS;
}

bool AranaReceive::receive_RF_message(RH_RF95 &rf95, unsigned int targetID, unsigned int &msgType, unsigned int &msgID, unsigned long &msgTime)
{
    uint8_t msg[MSG_LENGTH];
    uint8_t len = MSG_LENGTH;
    bool received = rf95.recv(msg, &len);
    // Proceed if a message has been received, and if the message target
    // is the ID of the system calling this function
    if (received && (msg[2] == targetID || msg[2] == ID_GLOBAL))
    {
        // Save message type and ID to provided pointers
        msgType = msg[0];
        msgID = msg[1];

        // Decode message time
        msgTime = 0ul;
        if (msg[MSG_OFFSET] != 255)
        {
            for (int i = 0; i < MSG_LENGTH - MSG_OFFSET; i++)
            {
                msgTime = msgTime | ((unsigned long)(msg[i + MSG_OFFSET])) << (8 * ((MSG_LENGTH - MSG_OFFSET) - i - 1));
            }
        } // memcopy instead?
        else
        {
            // Null time - don't bother decoding it into an unsigned long
            // Serial.println("Null time");
        }

        return true;
    }
    else
    {
        return false;
    }
}
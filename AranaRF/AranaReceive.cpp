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

bool AranaReceive::receive_RF_message(RH_RF95 &rf95, char &msgType, unsigned int &msgTime)
{
    // TODO: Decoding message with hours, minutes, seconds, milliseconds
    uint8_t buf[MSG_LENGTH];
    uint8_t len = MSG_LENGTH;

    // Check if a message has been received
    if (rf95.recv(buf, &len))
    {
        // Start measuring execution time
        unsigned long current_micros = micros();

        // Retrieve msgType and msgTime from the received message
        msgType = buf[0];
        char systemTime[MSG_LENGTH - 2];
        for (int i = 0; i < MSG_LENGTH - 3; i++)
        {
            if ((char)buf[i + 2] == 'l')
            {
                systemTime[i] = '\0';
                break;
            }
            systemTime[i] = (char)buf[i + 2];
        }
        msgTime = atoi(systemTime);

        // Finish measuring execution time
        current_micros = micros() - current_micros;

        // Prints out the received message over serial
        // Serial.print(F("Received a message: "));
        // Serial.println((char *)buf);

        // Return true, since a message was received and decoded
        return true;
    }
    else
    {
        return false;
    }
}
#include "Arduino.h"
#include "AranaSend.h"

#define FILENAME "data.csv"
const char *race_types[] = {"stop", "start", "false start", "sync"};

// Class constructor
AranaSend::AranaSend(int RFM95_MISO, int RFM95_MOSI, int RFM95_RST, int RFM95_CS, int SD_CS)
{
    _RFM95_MISO = RFM95_MISO;
    _RFM95_MOSI = RFM95_MOSI;
    _RFM95_RST = RFM95_RST;
    _RFM95_CS = RFM95_CS;
    _SD_CS = SD_CS;
}

void AranaSend::setup_SD()
{
    // Serial.println(F("Beginning initialisation of SD card module."));
    digitalWrite(_RFM95_CS, HIGH);
    pinMode(_SD_CS, OUTPUT);
    digitalWrite(_SD_CS, LOW);

    // Wait until SD card is available
    while (!SD.begin(_SD_CS))
        ;

    // If necessary, create the CSV file with headers
    if (!SD.exists(FILENAME))
    {
        // Serial.println(F("File does not exist - creating file and writing headers."));
        File file = SD.open(FILENAME, FILE_WRITE);
        file.println(F("Message Type,Time,Sync Time"));
        file.close();
    }

    digitalWrite(_SD_CS, HIGH);
    digitalWrite(_RFM95_CS, LOW);

    // Serial.println(F("Completed initialisation of SD card module."));
}

void AranaSend::send_RF_message(RH_RF95 &rf95, unsigned int targetID, unsigned int msgType, unsigned int msgID, unsigned long absoluteTime)
{
    uint8_t msg[MSG_LENGTH];

    // Include message type and ID, and the target system for this message, in the first 3 bytes
    msg[0] = msgType;  // message type
    msg[1] = msgID;    // message ID
    msg[2] = targetID; // ID of the system this message is intended for

    // Encode the given system time (unsigned long) in the remaining 4 bytes of the message
    if (absoluteTime != NULL)
    {
        for (int i = 0; i < MSG_LENGTH - MSG_OFFSET; i++)
        {
            msg[i + MSG_OFFSET] = (uint8_t)(absoluteTime >> (8 * (sizeof(unsigned long) - 1 - i)));
        }
    } // memcopy instead?
    else
    {
        // No message time given - send a time with the 1st byte equal to 255, which indicates
        // that the receiving end does not need to decode the given time into an unsigned long
        msg[MSG_OFFSET] = 255;
    }

    // Send the message and wait until the packet has been sent
    rf95.send((const uint8_t *)&msg, sizeof(msg));
    rf95.waitPacketSent();
}

// Write to SD card
void AranaSend::write_to_SD(int msgType, unsigned long msgTime, unsigned long syncTime)
{
    digitalWrite(_SD_CS, LOW);

    // Serial.println(F("Writing to SD card..."));
    SD.begin(_SD_CS);

    // Construct line to insert into CSV file
    char data[25];
    snprintf_P(data, sizeof(data), PSTR("%s,%ul,%ul"), race_types[msgType], msgTime, syncTime);

    // Open CSV file, insert line, and close file
    File file = SD.open(FILENAME, FILE_WRITE);
    file.println(data);
    file.close();

    // Serial.println(F("Saved data and closed CSV file."));

    delay(20);
    digitalWrite(_SD_CS, HIGH);
}
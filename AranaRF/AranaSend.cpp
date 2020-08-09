#include "Arduino.h"
#include "AranaSend.h"

#define FILENAME "data.csv"
const char *race_types[] = {"stop", "start"};

// Class constructor
AranaSend::AranaSend(int RFM95_MISO, int RFM95_MOSI, int RFM95_RST, int RFM95_CS, int SD_CS, int LED)
{
    _RFM95_MISO = RFM95_MISO;
    _RFM95_MOSI = RFM95_MOSI;
    _RFM95_RST = RFM95_RST;
    _RFM95_CS = RFM95_CS;
    _SD_CS = SD_CS;
    _LED = LED;
}

void AranaSend::setup_SD()
{
    Serial.println(F("Beginning initialisation of SD card module."));
    digitalWrite(_RFM95_CS, HIGH);
    pinMode(_SD_CS, OUTPUT);
    digitalWrite(_SD_CS, LOW);

    // Wait until SD card is available
    while (!SD.begin(_SD_CS))
        ;

    // If necessary, create the CSV file with headers
    if (!SD.exists(FILENAME))
    {
        Serial.println(F("File does not exist - creating file and writing headers."));
        File file = SD.open(FILENAME, FILE_WRITE);
        file.println(F("Message Type,Time"));
        file.close();
    }

    digitalWrite(_SD_CS, HIGH);
    digitalWrite(_RFM95_CS, LOW);

    Serial.println(F("Completed initialisation of SD card module."));
}

void AranaSend::send_RF_message(RH_RF95 &rf95, int msgType)
{
    unsigned long msgTime = millis();
    char msg[MSG_LENGTH] = "";
    sprintf(msg, "%d %ul", msgType, msgTime);

    digitalWrite(_LED, HIGH);

    rf95.send((const uint8_t *)&msg, sizeof(msg));
    rf95.waitPacketSent();

    Serial.print(F("Transmitted a message: "));
    Serial.println(msg);

    write_to_SD(msgType, msgTime);
    Serial.println("");

    // delay(1000);
    // digitalWrite(LED, LOW);
}

// Write to SD card
void AranaSend::write_to_SD(int msgType, unsigned long msgTime)
{
    digitalWrite(_SD_CS, LOW);

    Serial.println(F("Writing to SD card..."));
    SD.begin(_SD_CS);

    // Construct line to insert into CSV file
    char data[25];
    snprintf_P(data, sizeof(data), PSTR("%s,%u"), race_types[msgType], msgTime);

    // Open CSV file, insert line, and close file
    File file = SD.open(FILENAME, FILE_WRITE);
    file.println(data);
    file.close();

    Serial.println(F("Saved data and closed CSV file."));

    delay(20);
    digitalWrite(_SD_CS, HIGH);
}
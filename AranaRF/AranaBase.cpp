#include "Arduino.h"
#include "AranaBase.h"

// Setup functions
void AranaBase::setup_serial()
{
    Serial.begin(9600);
    while (!Serial)
    {
        continue; // Wait for serial port to be available
    }

    Serial.println(F("Serial setup complete"));
}

void AranaBase::setup_RF(RH_RF95 &rf95)
{
    // Set pins for RF module
    // Serial.println(F("Beginning initialisation RF module."));
    pinMode(_RFM95_MISO, OUTPUT); // default slave select pin for client mode
    pinMode(_RFM95_MOSI, INPUT);
    pinMode(_RFM95_RST, OUTPUT);
    pinMode(_RFM95_CS, OUTPUT);
    digitalWrite(_RFM95_RST, LOW);
    pinMode(_RFM95_RST, OUTPUT);
    delayMicroseconds(100);
    pinMode(_RFM95_RST, INPUT);
    delay(5);
    if (!rf95.init())
    {
        // Serial.println(F("RF init failed :("));
        while (1)
            ;
    }
    rf95.setTxPower(13, false);
    // Serial.println(F("Completed initialisation of RF module."));
}

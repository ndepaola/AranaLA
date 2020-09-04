#include <AranaReceive.h>

#define RFM95_INT 3 // Interrupt
#define RFM95_RST 4
#define RFM95_CS 5
#define DFM_PIN 10
#define RFM95_MOSI 11
#define RFM95_MISO 12
#define RFM95_SCK 13

RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Declarations
unsigned int msgType;
unsigned int msgID;
unsigned long msgTime;

AranaReceive rflib(RFM95_MISO, RFM95_MOSI, RFM95_RST, RFM95_CS);

void setup()
{
    // Setup serial and RF communications
    // rflib.setup_serial();
    rflib.setup_RF(rf95);
    pinMode(DFM_PIN, OUTPUT);
    digitalWrite(DFM_PIN, HIGH);
    // Serial.println(F("Speaker: Finished setup.\n"));
}

void play_sound()
{
    digitalWrite(DFM_PIN, LOW);
    delay(100);
    digitalWrite(DFM_PIN, HIGH);
}

void loop()
{
    if (rf95.available() &&
        rflib.receive_RF_message(rf95, rflib.ID_GLOBAL, msgType, msgID, msgTime))
    {
        if (msgType == rflib.MSG_START)
        {
            // Serial.println("Playing race start sound");
            play_sound();
        }
        else if (msgType == rflib.MSG_FALSE_START)
        {
            // Serial.println("False start");
            play_sound();
            delay(500);
            play_sound();
        }
    }
}

// screen /dev/cu.usbmodem14201
#include <AranaSend.h>

#define DS3231_INT 2

#define RFM95_SCK 13
#define RFM95_MISO 12
#define RFM95_MOSI 11
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 3
#define RF95_FREQ 433.0
#define MSG_LENGTH 8

#define SD_CS 8

// Define Button pins
// #define START_BUTTON 45
// #define RESET_BUTTON 23

// #define LED 44
#define START_BUTTON 5
#define RESET_BUTTON 4

#define START_LED 1
#define RESET_LED 0

#define LED 6

unsigned long off = ULONG_MAX;

RH_RF95 rf95(RFM95_CS, RFM95_INT);

// DS3231M_Class DS3231M; // Create an instance of the DS3231M

RtcDS3231<TwoWire> rtc(Wire);

bool false_start = false;

unsigned long tick = 0;

AranaSend rflib(RFM95_MISO, RFM95_MOSI, RFM95_RST, RFM95_CS, SD_CS, LED);

void RTCInterruptHandler()
{
    tick++;
    //   Serial.println(tick);
}

void setupClock()
{
    pinMode(DS3231_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(DS3231_INT), RTCInterruptHandler, RISING);
    rtc.Begin();
    rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
    rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1kHz);
    // rtc.SetSquareWavePinClockFrequency(DS3231Sq);

    // pinMode(DS3231_INT, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(DS3231_INT), RTCInterruptHandler, RISING);
    // while (!DS3231M.begin())
    // DS3231M.begin();
    // DS3231M.
    // rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1kHz);
}

void setup()
{
    // Setup serial and RF communications
    rflib.setup_serial();
    rflib.setup_RF(rf95);
    rflib.setup_SD();

    pinMode(START_BUTTON, INPUT);
    pinMode(RESET_BUTTON, INPUT);
    pinMode(START_LED, OUTPUT);
    pinMode(RESET_LED, OUTPUT);
    pinMode(LED, OUTPUT);
    // pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(START_LED, LOW);
    digitalWrite(RESET_LED, LOW);
    // digitalWrite(LED, LOW);
    // digitalWrite(LED_BUILTIN, HIGH);
    // delay(1000);
    // digitalWrite(LED_BUILTIN, LOW);

    // setupClock();
    // setupSD();

    // Serial.println("Setup Complete");
    Serial.println("Start system: Finished setup.\n");
}

void loop()
{
    if (digitalRead(START_BUTTON) == HIGH && off == ULONG_MAX)
    {
        rflib.send_RF_message(rf95, 1);
        // digitalWrite(START_LED, HIGH);
        off = millis() + 1000;
    }
    else if (digitalRead(RESET_BUTTON) == HIGH && off == ULONG_MAX)
    {
        rflib.send_RF_message(rf95, 0);
        // digitalWrite(RESET_LED, HIGH);
        off = millis() + 1000;
    }
    if (millis() > off)
    {
        digitalWrite(START_LED, LOW);
        digitalWrite(RESET_LED, LOW);
        off = ULONG_MAX;
    }
}

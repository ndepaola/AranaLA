// screen /dev/cu.usbmodem14201
#include <AranaSend.h>

#define DS3231_INT 3

#define RFM95_SCK 13
#define RFM95_MISO 12
#define RFM95_MOSI 11
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 433.0

#define SD_CS 8

#define START_BUTTON 5
#define RESET_BUTTON 1
#define FALSE_START_BUTTON 4
#define SYNC_BUTTON 6

#define START_LED A3
#define RESET_LED A2
#define FALSE_START_LED A1
#define SYNC_LED A0

#define FLASH_LED 7
#define FLASH_DELAY 200 // milliseconds
#define FLASH_PAUSE 100

#define LATENCY 247 // milliseconds

unsigned long long syncTime = 0;
unsigned long off = ULONG_MAX;
unsigned long long start_button_pressed = 0;

unsigned int count = 0;
unsigned long long sync_button_activated = 0;


RH_RF95 rf95(RFM95_CS, RFM95_INT);

// DS3231M_Class DS3231M; // Create an instance of the DS3231M
// RtcDS3231<TwoWire> rtc(Wire);

unsigned long tick = 0;

AranaSend rflib(RFM95_MISO, RFM95_MOSI, RFM95_RST, RFM95_CS, SD_CS);

void RTCInterruptHandler()
{
    tick++;
}

void setupClock()
{
    pinMode(DS3231_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(DS3231_INT), RTCInterruptHandler, RISING);
}

void setup()
{
    // Setup serial and RF communications
    // rflib.setup_serial();
    rflib.setup_RF(rf95);
    rflib.setup_SD();

    pinMode(START_BUTTON, INPUT);
    pinMode(RESET_BUTTON, INPUT);
    pinMode(SYNC_BUTTON, INPUT);
    pinMode(FALSE_START_BUTTON, INPUT);

    pinMode(START_LED, OUTPUT);
    pinMode(RESET_LED, OUTPUT);
    pinMode(SYNC_LED, OUTPUT);
    pinMode(FALSE_START_LED, OUTPUT);
    pinMode(FLASH_LED, OUTPUT);

    digitalWrite(START_LED, LOW);
    digitalWrite(RESET_LED, LOW);
    digitalWrite(SYNC_LED, LOW);
    digitalWrite(FALSE_START_LED, LOW);

    // setupClock();

    // Serial.println("Start system: Finished setup.\n");
}

void start_race()
{
    // Serial.println("Starting");
    unsigned long msgTime = millis() - syncTime + LATENCY; // TODO: Use RTC
    rflib.send_RF_message(rf95, rflib.ID_GLOBAL, rflib.MSG_START, 1, msgTime);
    digitalWrite(START_LED, HIGH);
    delay(LATENCY);
    digitalWrite(FLASH_LED, HIGH);
    delay(FLASH_DELAY);
    digitalWrite(FLASH_LED, LOW);
    rflib.write_to_SD(rflib.MSG_START, msgTime, syncTime);
}

void reset()
{
    // Serial.println("Resetting");
    unsigned long msgTime = millis() - syncTime; // TODO: Use RTC
    rflib.send_RF_message(rf95, rflib.ID_GLOBAL, rflib.MSG_STOP, 1, msgTime);
    digitalWrite(RESET_LED, HIGH);
    rflib.write_to_SD(rflib.MSG_STOP, msgTime, syncTime);
}

void false_start()
{
    // Serial.println("False start");
    unsigned long msgTime = millis() - syncTime + LATENCY; // TODO: Use RTC
    rflib.send_RF_message(rf95, rflib.ID_GLOBAL, rflib.MSG_FALSE_START, 1, msgTime);
    digitalWrite(FALSE_START_LED, HIGH);
    delay(LATENCY);

    // Flash the bright LED twice in rapid succession
    digitalWrite(FLASH_LED, HIGH);
    delay(FLASH_DELAY);
    digitalWrite(FLASH_LED, LOW);
    delay(FLASH_PAUSE);
    digitalWrite(FLASH_LED, HIGH);
    delay(FLASH_DELAY);
    digitalWrite(FLASH_LED, LOW);
    rflib.write_to_SD(rflib.MSG_FALSE_START, msgTime, syncTime);
}

void sync()
{
    // Serial.println("Syncing");
    syncTime = millis();
    digitalWrite(SYNC_LED, HIGH);
    digitalWrite(FLASH_LED, HIGH);
    delay(FLASH_DELAY);
    digitalWrite(FLASH_LED, LOW);
    unsigned long msgTime = millis() - syncTime; // TODO: Use RTC
    rflib.write_to_SD(rflib.MSG_SYNC, msgTime, syncTime);
}

void loop()
{
    unsigned long curr_time = millis();
    if (digitalRead(START_BUTTON) == HIGH && (millis() >= off - 810 || off == ULONG_MAX))
    {
        unsigned long long next_button_pressed = millis();
        // if (next_button_pressed - start_button_pressed <= 200)
        // {
        //     continue;
        // }
        // else 
        if (next_button_pressed - start_button_pressed <= 4000 && !(millis() <= 4000))
        {
            false_start();
        }
        else
        {
            start_race();
        }
        off = millis() + 1000 - (millis() - curr_time);
        while (digitalRead(START_BUTTON) == HIGH)
        {
            delay(1);
        }
        start_button_pressed = next_button_pressed;
    }
    else if (digitalRead(RESET_BUTTON) == HIGH && off == ULONG_MAX)
    {
        reset();
        off = millis() + 1000 - (millis() - curr_time);
    }
    else if (digitalRead(FALSE_START_BUTTON) == HIGH && (millis() >= off - 950 || off == ULONG_MAX))
    {
        false_start();
        off = millis() + 1000 - (millis() - curr_time);
    }
    else if (digitalRead(SYNC_BUTTON) == HIGH)
    {
        if (count == 0)
        {
            sync_button_activated = millis();
            count++;
        }
        // else if (count < 300 && (millis() - sync_button_activated >= 10 && millis() - sync_button_activated < 12))
        // {
        //     sync_button_activated = millis();
        //     count++;
        // }
        else if (millis() - sync_button_activated >= 500)
        {
            count = 0;
        }
        else if (count < 300)
        {
            sync_button_activated = millis();
            digitalWrite(SYNC_LED, HIGH);
            delay(10);
            digitalWrite(SYNC_LED, LOW);
            count++;
        }
        else if (count == 300)
        {
            sync();
            off = millis() + 1000 - (millis() - curr_time);
            count = 0;
        }
        // sync();
        
    }

    if (millis() > off)
    {
        digitalWrite(START_LED, LOW);
        digitalWrite(RESET_LED, LOW);
        digitalWrite(FALSE_START_LED, LOW);
        digitalWrite(SYNC_LED, LOW);
        off = ULONG_MAX;
    }
}

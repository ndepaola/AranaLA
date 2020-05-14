#include <Wire.h>
#include <RtcDS3231.h>
#include "limits.h"
#include <stdint.h>
#include <stdio.h>
#include "SD.h"
#include <SPI.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
#define RFM95_SCK 52
#define RFM95_MISO 50
#define RFM95_MOSI 51
#define RFM95_CS 49
#define RFM95_RST 48
#define RFM95_INT 21
#define RF95_FREQ 433.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);
File file;
RtcDS3231 <TwoWire> rtcObject(Wire);

//Declarations
int sd_cs_pin = 30;
int led_pin = 6;
int speaker_pin = 7;
int tone_pin = 44;

bool writecounter = 1;
bool rfcounter = 0;

int button_go_pin = 12;
int sounddetect_sensor_pin = 13;
bool button_rf_read = 0;
bool sounddetect_value = 0;

unsigned long ticks = 0;
unsigned long off = ULONG_MAX;
unsigned long accurateticks = 0;

int button_sync_state = 0;
int button_sync_last_state = 0;

long time = 0;
long time_now = millis();

int counter = 0;
bool LED_on = false;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(RFM95_MISO, OUTPUT); // default slave select pin for client mode 
    pinMode(RFM95_MOSI, INPUT);
    pinMode(RFM95_RST, OUTPUT);
    pinMode(RFM95_CS, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
   
    rf95.setTxPower(13, false);

    Serial.begin(9600);
    while (!Serial) ; // Wait for serial port to be available
        if (!rf95.init())
    Serial.println("init failed");

    time = millis();
}

void loop() {
    time_now = millis();

    if (int(round((time_now - time) / 1000)) % 20 == 0) {
        if (counter == 2) {
            counter = 1;
        }
        else {
            counter++;
        }

        // std::string message = "Number of flashes: " + std::to_string(counter);

        // std::vector<uint8_t> myVector(message.begin(), message.end());
        // uint8_t *p = &myVector[0];

        // uint8_t data[] = "counter";

        uint8_t data[] = {counter};
        rf95.send(data, sizeof(data));
  
        rf95.waitPacketSent();
        // Now wait for a reply
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);

        if (rf95.waitAvailableTimeout(3000)) { 
            // Should be a reply message for us now   
            if (rf95.recv(buf, &len) {
            Serial.print("got reply: ");
            Serial.println((char*)buf);
        //      Serial.print("RSSI: ");
        //      Serial.println(rf95.lastRssi(), DEC);    
            }
            else {
            Serial.println("recv failed");
            }
        }
        else {
            Serial.println("No reply, is rf95_server running?");
        }
    }

    if (int(round((time_now - time) / 100)) % (10 / counter) == 0) {
        if (LED_on) {
            digitalWrite(LED_BUILTIN, LOW);
            LED_on = false;
        }
        else {
            digitalWrite(LED_BUILTIN, HIGH);
            LED_on = true;
        }
    }
}
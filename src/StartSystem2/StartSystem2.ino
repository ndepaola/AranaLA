#include <Wire.h>
//#include <RtcDS3231.h>
#include "limits.h"
#include <stdint.h>
#include <stdio.h>
#include "SD.h"
#include <SPI.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
// #define RFM95_SCK 52
// #define RFM95_MISO 50
// #define RFM95_MOSI 51
// #define RFM95_CS 49
// #define RFM95_RST 48
// #define RFM95_INT 21
#define RFM95_SCK 13
#define RFM95_MISO 12
#define RFM95_MOSI 11
#define RFM95_CS 4
#define RFM95_RST 5
#define RFM95_INT 3
#define RF95_FREQ 433.0

#define SD_CS 6

// Define Button pins
// #define START_BUTTON 45
// #define SYNC_BUTTON 23

// #define LED 44
#define START_BUTTON 7
#define SYNC_BUTTON 8

#define LED 9

unsigned long long off = ULONG_MAX;

volatile unsigned long long milliSeconds = millis();

RH_RF95 rf95(RFM95_CS, RFM95_INT);

File file;

bool falseStart = false;

void setupRF()
{
    Serial.println("Being setting up pins for RFM95.");
    pinMode(RFM95_MISO, OUTPUT); // default slave select pin for client mode 
    pinMode(RFM95_MOSI, INPUT);
    pinMode(RFM95_RST, OUTPUT);
    pinMode(RFM95_CS, OUTPUT);
    Serial.println("Finished setting up pins.");

    Serial.println("Resetting RFM95 Module.");
    // digitalWrite(RFM95_CS, LOW);
    // digitalWrite(RFM95_RST, HIGH);
    // delay(10);
    // digitalWrite(RFM95_RST, LOW);
    // Manual Reset
    digitalWrite(RFM95_RST, LOW);
    pinMode(RFM95_RST, OUTPUT);
    delayMicroseconds(100);
    pinMode(RFM95_RST, INPUT);
    delay(5);
    
    
    Serial.println("RFM95 Module reset.");

    // while (!rf95.init());
    if (!rf95.init()) {
      Serial.println("init failed");
      while(1);
    }
    Serial.println("RFM95 Module initialised!");

    Serial.println("Set transmission power to 13 dBm.");
    rf95.setTxPower(13, false);
    Serial.println("Power set to 13 dBm.");
}

void setupSerial()
{
    Serial.begin(9600);
    while (!Serial) {
      continue; // Wait for Serial port to be available
    }

    Serial.println("Serial setup complete");
    // Serial.println("Setup Complete");
}

void setupSD()
{
    //Serial.println("Initializing SD card...");
    digitalWrite(RFM95_CS, HIGH);
    delay(10);

    pinMode(SD_CS, OUTPUT);

    digitalWrite(SD_CS, LOW);

    delay(20);

    while (!SD.begin(SD_CS));

    delay(1000);

    file = SD.open("data.csv", FILE_WRITE);
    if (file)
    {        
        // RtcDateTime dt = rtcObject.GetDateTime();
        // char datestring[25];
        // // char datestring[25] = {'Time of Start,False Start'};
        // // snprintf_P(datestring, sizeof(datestring), PSTR("%02u/%02u/%04u %02u:%02u:%02u"), dt.Day(), dt.Month(), dt.Year(), dt.Hour(), dt.Minute(), dt.Second());
        // snprintf_P(datestring, sizeof(datestring), PSTR("Time of Start,False Start"));
        // Serial.println(datestring);    
        // file.println(datestring);
        // file.println("Time of Start,False Start");
        // file.println(milliSeconds);

        // String heading = "Time of Start,False Start";
        // int heading_len = heading.length() + 1;
        // char char_array[heading_len];
        // heading.toCharArray(char_array, heading_len);
        // file.println(heading);
        file.println("Time of Start,False Start");
        // file.flush();
        file.close();
        // Serial.println("Time of Start,False Start");
    }

    

    delay(20);

    digitalWrite(SD_CS, HIGH);

    delay(10);
    digitalWrite(RFM95_CS, LOW);
}

void setup()
{
    pinMode(START_BUTTON, INPUT);
    pinMode(SYNC_BUTTON, INPUT);
    pinMode(LED, OUTPUT);
    // pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(LED, LOW);
    // digitalWrite(LED_BUILTIN, HIGH);
    // delay(1000);
    // digitalWrite(LED_BUILTIN, LOW);

    setupSerial();

    delay(1000);

    setupRF();

    setupSD();

    Serial.println("Setup Complete");
}

void writeToSD(unsigned long now)
{
    digitalWrite(SD_CS, LOW);

    // SD.begin(SD_CS);

    file = SD.open("data.csv", FILE_WRITE);

    file.println(now); // record to SD card    
    // file.flush();

    file.close();

    delay(20);
    digitalWrite(SD_CS, HIGH);
}

void RaceStarted()
{
    unsigned long  now = millis();

    digitalWrite(LED, HIGH);
    // digitalWrite(LED_BUILTIN, HIGH);
    
    Serial.println("About to transmit message.");

    rf95.send((const uint8_t*)&now, sizeof(now)); // tell display board  
    rf95.waitPacketSent();

    Serial.println("Message sent!");
//    delay(1000);
//    digitalWrite(LED, LOW);

    writeToSD(now);

    off = now + 1000;
    Serial.println(now);
}

void loop()
{
    milliSeconds = millis();

    if (digitalRead(START_BUTTON) == HIGH && off == ULONG_MAX){
        RaceStarted();
    }

    if (milliSeconds > off)
    {
        //Serial.println(" turn off");
        digitalWrite(LED, LOW);
        // digitalWrite(LED_BUILTIN, LOW);
        off = ULONG_MAX;
    }
}

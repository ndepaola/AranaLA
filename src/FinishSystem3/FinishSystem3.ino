#include <SPI.h>
#include <Wire.h>
#include <stdint.h>
#include <stdio.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
#include "limits.h"
#define RFM95_SCK 13
#define RFM95_MISO 12
#define RFM95_MOSI 11
#define RFM95_CS 4
#define RFM95_RST 2
#define RFM95_INT 3 // Interrupt
#define RF95_FREQ 433.0

// #include <Wire.h>
// #include <RtcDS3231.h>

// clock data
volatile unsigned long tick = 0;

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void init_RF()
{
    // rf95.setFrequency(433);
    pinMode(RFM95_MISO, OUTPUT); // default slave select pin for client mode 
    pinMode(RFM95_MOSI, INPUT);
    pinMode(RFM95_RST, INPUT);
    pinMode(RFM95_CS, OUTPUT);

//    rf95.setTxPower(13, false);

    Serial.begin(9600);
    while (!Serial) {
      continue; // Wait for Serial port to be available
    }
    Serial.println("Welcome!");

    // Manual Reset
    digitalWrite(RFM95_RST, LOW);
    pinMode(RFM95_RST, OUTPUT);
    delayMicroseconds(100);
    pinMode(RFM95_RST, INPUT);
    delay(5);

    Serial.println("Manual Reset of RF Module Complete!");
    if (!rf95.init())
      Serial.println("init failed"); 

    rf95.setTxPower(13, false);

    Serial.println("done setup ...");
}

uint8_t buf[255];
uint8_t len;

void check_for_RF_message()
{
    if (rf95.recv(buf, &len)){
        // restart();
        Serial.println((char*)buf);
        Serial.println(len);
//        unsigned long long int milliseconds = buf;
//        Serial.println(milliseconds);
        Serial.println("Received.");
    }
}

void setup()
{   
    init_RF();
}

// void RTCInterruptHandler()
// {
// 	  tick++;
// }

void loop()
{
    if (rf95.available())
       check_for_RF_message();
    // if (counter <= 100000){
//        Serial.println("Made it 1");
        // if (rf95.available()) {
        //     Serial.println("Made it 2");
        //     if (rf95.recv(buf, &len)) {
        //         Serial.println((char*) buf);
        //         // counter++;
        //         // Serial.println(counter);
        //         // rf95.send((const uint8_t*)&counter, sizeof(counter)); // tell display board  
        //         // rf95.waitPacketSent();
        //     }
        // }
    // }
    // else{
    //     Serial.println("Finished!");
    //     while(1);
    // }
}

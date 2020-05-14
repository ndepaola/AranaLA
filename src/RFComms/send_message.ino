#include <Wire.h>
#include <RtcDS3231.h>
#include "limits.h"
#include <stdint.h>
#include <stdio.h>
#include "SD.h"
#include <SPI.h>
// #include <RH_RF95.h>
#include <RH_NRF905.h>
#include <RHSoftwareSPI.h>
#define nRF905_CS 4
#define nRF905_RST 5
#define nRF905_INT 3
#define nRF905_FREQ 433.0

// #define MISO 22
// #define MISO PB3
#define MOSI 51
#define MISO 50     // Use actual pin number on board, not chip
#define TxEN 5
#define CE 6
#define PWR A0
#define CD 2
#define AM 7
#define DR 3
#define SCK 52
#define CSN 4

void setup() {
    pinMode(MISO, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
}


// void main()
// {
//     setup();

//     while(1)
//     {

//     }
// }

void loop() {
    digitalWrite(MISO, HIGH); // sets the digital pin 13 on
    digitalWrite(LED_BUILTIN, HIGH);
    delay(3000);            // waits for a second
    digitalWrite(MISO, LOW);  // sets the digital pin 13 off
    digitalWrite(LED_BUILTIN, LOW);
    delay(3000);            // waits for a second
}
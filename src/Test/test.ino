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

// Define Button pins
// #define START_BUTTON 45
// #define SYNC_BUTTON 23

// #define LED 44
#define START_BUTTON 7
#define SYNC_BUTTON 8

#define LED 9

unsigned long off = ULONG_MAX;

volatile unsigned long long milliSeconds = 0;

bool isOn = false;

void setup()
{
    // TIMER 2 for interrupt frequency 1000 Hz:
    cli(); // stop interrupts
    TCCR2A = 0; // set entire TCCR2A register to 0
    TCCR2B = 0; // same for TCCR2B
    TCNT2  = 0; // initialize counter value to 0
    // set compare match register for 1000 Hz increments
    OCR2A = 249; // = 16000000 / (64 * 1000) - 1 (must be <256)
    // turn on CTC mode
    TCCR2B |= (1 << WGM21);
    // Set CS22, CS21 and CS20 bits for 64 prescaler
    TCCR2B |= (1 << CS22) | (0 << CS21) | (0 << CS20);
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);
    sei(); // allow interrupts

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    
}

ISR(TIMER2_COMPA_vect){
    //interrupt commands for TIMER 2 here
    milliSeconds++;

    if (milliSeconds % 500 == 0)
    {
        if (isOn)
        {
            digitalWrite(LED_BUILTIN, LOW);
            isOn = false;
        }
        else
        {
            digitalWrite(LED_BUILTIN, HIGH);
            isOn = true;
        }
    }
}

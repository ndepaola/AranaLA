#include <Wire.h>
//#include <RtcDS3231.h>
#include "limits.h"
#include <stdint.h>
#include <stdio.h>
//#include "SD.h"
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
// File file;
// RtcDS3231 <TwoWire> rtcObject(Wire);

//Declarations
volatile unsigned long milliSeconds = 0;

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


    // Serial.begin(9600);
    // Serial.println("setup()");
    
    // pinMode(53, OUTPUT); // default slave select pin for client mode 
    // rf95.setFrequency(433);
    pinMode(RFM95_MISO, OUTPUT); // default slave select pin for client mode 
    pinMode(RFM95_MOSI, INPUT);
    pinMode(RFM95_RST, OUTPUT);
    pinMode(RFM95_CS, OUTPUT);
    // digitalWrite(RFM95_RST, HIGH);

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
    
    rf95.setTxPower(13, false);

    if (!rf95.init())
      Serial.println("init failed"); 
//    while (!rf95.init()) 
//    {
//    }


    Serial.println("done setup ...");
}

void RaceStarted()
{
	  //Serial.print(" Race "); 
    unsigned long now = milliSeconds;
   
    rf95.send((const uint8_t*)&now, sizeof(now)); // tell display board  
    rf95.waitPacketSent();
    Serial.println(now);

}

void loop() 
{    
    RaceStarted();
}

ISR(TIMER2_COMPA_vect){
    //interrupt commands for TIMER 2 here
    milliSeconds++;
}

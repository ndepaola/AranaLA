#include <Wire.h>
#include <RtcDS3231.h>
#include "limits.h"
#include <stdint.h>
#include <stdio.h>
#include "SD.h"
#include <SPI.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
#define RFM95_CS 4
#define RFM95_RST 5
#define RFM95_INT 3
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

void setup() 
{
    Serial.begin(9600);
    Serial.println("setup()");
    
    pinMode(53, OUTPUT); // default slave select pin for client mode 
    pinMode(RFM95_RST, OUTPUT);
    pinMode(RFM95_CS, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
   
    rf95.setTxPower(13, false);

    pinMode(button_go_pin, INPUT);
    pinMode(sounddetect_sensor_pin, INPUT);
    pinMode(speaker_pin, OUTPUT);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);

    while (!rf95.init()) 
    {
    }

    rf95.setFrequency(433);
    
    //Serial.println("LoRa radio init OK!");

    pinMode(2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(2), milli, RISING);
    rtcObject.Begin();
    rtcObject.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
    rtcObject.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1kHz);

    initializeSD();
    
    tone(tone_pin, 200);

    digitalWrite(led_pin, HIGH);
    digitalWrite(speaker_pin, HIGH);
    delay(1000);
    digitalWrite(led_pin, LOW);
    digitalWrite(speaker_pin, LOW);

    Serial.println("done setup ...");
}

void RaceStarted(bool bang)
{
	  //Serial.print(" Race "); 
    unsigned long now = ticks;
    
    digitalWrite(led_pin, HIGH); // turn light on
    if (bang)
        digitalWrite(speaker_pin, HIGH); // turn speaker on
   
    rf95.send((const uint8_t*)&now, sizeof(now)); // tell display board  
    rf95.waitPacketSent();
     
    file.println(now); // record to SD card    
    file.flush();
    
    off = now + 1000;

    //Serial.println("started!");
}

void loop() 
{    
    Serial.print("0 1000 ");
    Serial.println(analogRead(A0));
    
    if (analogRead(A0) > 600 && off == ULONG_MAX) // Microphone detected loud bang!   
        RaceStarted(false);
            
    if (digitalRead(12) == LOW && off == ULONG_MAX) // Start button pressed
        RaceStarted(true);            
    
    if (ticks > off)
    {
        //Serial.println(" turn off");
        digitalWrite(led_pin, LOW);
        digitalWrite(speaker_pin, LOW);
        off = ULONG_MAX;
    }
}

void milli() 
{
    ticks++;
}

void initializeSD()
{
    //Serial.println("Initializing SD card...");
    pinMode(sd_cs_pin, OUTPUT);

    SD.begin(sd_cs_pin);

    file = SD.open("data.txt", FILE_WRITE);
    if (file)
    {        
        RtcDateTime dt = rtcObject.GetDateTime();
        char datestring[20];
        snprintf_P(datestring, sizeof(datestring), PSTR("%02u/%02u/%04u %02u:%02u:%02u"), dt.Day(), dt.Month(), dt.Year(), dt.Hour(), dt.Minute(), dt.Second());
        //Serial.println(datestring);    
        file.println(datestring);
    }
}

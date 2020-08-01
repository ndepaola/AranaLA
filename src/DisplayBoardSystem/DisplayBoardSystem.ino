#include <SPI.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include <RH_RF95.h>
#include "TimerOne.h"

// screen /dev/cu.usbmodem14201 9600

// display data
#define pin_a 6
#define pin_b 7
#define pin_sck 8
#define pin_noe 9
#define RFM95_SCK 13
#define RFM95_MISO 12
#define RFM95_MOSI 11
#define RFM95_CS 5
#define RFM95_RST 4
#define RFM95_INT 3
#define RF95_FREQ 433.0

#define MSG_LENGTH 16

#define ROWSIZE 16;

#define DISPLAY_REFRESH 500; // Microseconds between display refreshes

uint8_t bitmap[256];
RtcDS3231<TwoWire> rtcObject(Wire);
SPISettings settings(4000000, MSBFIRST, SPI_MODE0); // TODO: Verify that this is correct?

byte scan_row = 0;

void init_clock()
{
    pinMode(2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(2), RTCInterruptHandler, RISING);
    rtcObject.Begin();
    rtcObject.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
    rtcObject.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1kHz);
}

void init_display()
{
    memset(bitmap, 0xFF, 256);
    pinMode(pin_a, OUTPUT);
    pinMode(pin_b, OUTPUT);
    pinMode(pin_sck, OUTPUT);
    digitalWrite(pin_noe, LOW);
    pinMode(pin_noe, OUTPUT);
    SPI.begin();
}

void RTCInterruptHandler()
{
    // tick++;
}

void update_display()
{

    const int rowsize = 16;

    volatile uint8_t *rows[4] = { // Scanning out 4 interleaved rows
                                 bitmap + (scan_row + 0) * rowsize,
                                 bitmap + (scan_row + 4) * rowsize,
                                 bitmap + (scan_row + 8) * rowsize,
                                 bitmap + (scan_row + 12) * rowsize
    };
    cli();

    SPI.beginTransaction(settings);

    // We send out interleaved data for 4 rows at a time
    for (int i = 0; i < rowsize; i++)
    {
        SPI.transfer(*(rows[3]++));
        SPI.transfer(*(rows[2]++));
        SPI.transfer(*(rows[1]++));
        SPI.transfer(*(rows[0]++));
    }

    SPI.endTransaction();

    sei();

    digitalWrite(pin_sck, HIGH); // Latch DMD shift register output
    digitalWrite(pin_sck, LOW);  // (Deliberately left as digitalWrite to ensure decent latching time)

    digitalWrite(pin_a, scan_row & 0x01);
    digitalWrite(pin_b, scan_row & 0x02);

    scan_row = (scan_row + 1) % 4;

    analogWrite(pin_noe, 1);
}

void setup()
{
    init_clock();
    init_display();

    // TODO: Is it actually beneficial to wipe it twice?
    memset(bitmap, 0xFF, 256);
    delay(500);
    memset(bitmap, 0xFF, 256);
    delay(500);

    // TODO: This is just test code to illuminate the whole display to start with, remove this later
    for (int i = 0; i < 255; i++)
    {
        bitmap[i] = 0b00000000;
    }

    // Set up display refresh timer
    Timer1.initialize(DISPLAY_REFRESH);
    Timer1.attachInterrupt(update_display);
}

void loop()
{

}

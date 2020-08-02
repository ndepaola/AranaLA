// screen /dev/cu.usbmodem14201 9600
#include <SPI.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include <RH_RF95.h>
#include "TimerOne.h"

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

//#define ROWSIZE 16
#define DISPLAY_REFRESH_MS 500 // Microseconds between display refreshes. Was originally 500 but I found 5000 reduced flickering?
#define BITMAP_REFRESH_TICKS 100 // 1000 ticks (milliseconds) before display board updates
#define BRIGHTNESS 5

#define PANEL_WIDTH 32
#define PANEL_HEIGHT 16
#define SCREEN_WIDTH PANEL_WIDTH * 2
#define SCREEN_HEIGHT PANEL_HEIGHT * 2
#define SCREEN_WIDTH_B SCREEN_WIDTH / 8

#include "font.h"

// Clock variables
volatile unsigned short tick = 0;
unsigned char tenths = 0;
unsigned char seconds0 = 0;
unsigned char seconds1 = 0;
unsigned char minutes = 0;

uint8_t bitmap[256];
RtcDS3231<TwoWire> rtcObject(Wire);
SPISettings settings(4000000, MSBFIRST, SPI_MODE0); // TODO: Verify that this is correct?
byte scan_row = 0;

void init_clock()
{
    pinMode(2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(2), RTCInterruptHandler, RISING);
    rtcObject.Begin();
    // TODO: Check if this is exactly 1000 Hz or if it's actually 1024 Hz
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
    // Interrupt triggers every 0.001 seconds (from 1kHz square wave of DS3231)
    tick++;
}

void wipe_bitmap()
{
    memset(bitmap, 0xFF, 256);
}

void restart()
{
    wipe_bitmap();
    tick = 0;
    tenths = 0;
    seconds0 = 0;
    seconds1 = 0;
    minutes = 0;
}

void update_display()
{
    // Update the display board
    // This only shows one of four interleaved rows at a time, so it's attached to a 5ms timer interrupt
    // Scanning out 4 interleaved rows
    volatile uint8_t *rows[4] = {
        bitmap + (scan_row + 0) * PANEL_HEIGHT,
        bitmap + (scan_row + 4) * PANEL_HEIGHT,
        bitmap + (scan_row + 8) * PANEL_HEIGHT,
        bitmap + (scan_row + 12) * PANEL_HEIGHT};
    cli();

    SPI.beginTransaction(settings);

    // Send out interleaved data for 4 rows at a time
    for (int i = 0; i < PANEL_HEIGHT; i++)
    {
        SPI.transfer(*(rows[3]++));
        SPI.transfer(*(rows[2]++));
        SPI.transfer(*(rows[1]++));
        SPI.transfer(*(rows[0]++));
    }

    SPI.endTransaction();

    sei();

    // Write to display board digital pins
    digitalWrite(pin_sck, HIGH); // Latch DMD shift register output
    digitalWrite(pin_sck, LOW);  // (Deliberately left as digitalWrite to ensure decent latching time)

    digitalWrite(pin_a, scan_row & 0x01);
    digitalWrite(pin_b, scan_row & 0x02);

    // Update scan row for next call to this function
    scan_row = (scan_row + 1) % 4;

    // Set display board brightness
    // TODO: Control brightness with a dial on the display board
    analogWrite(pin_noe, BRIGHTNESS);
}

void update_clock()
{
    // This function triggers every 0.1 seconds
    // Update clock variables to reflect the new time, then update the bitmap
    tenths++;
    if (tenths >= 10)
    {
        tenths = 0;
        seconds0++;
    }
    if (seconds0 >= 10)
    {
        seconds0 = 0;
        seconds1++;
    }
    if (seconds1 >= 6)
    {
        seconds1 = 0;
        minutes++;
    }
    if (minutes >= 10)
    {
        // 10 minutes has elapsed - default to idle screen since we can't display 10 minutes
        display_idle();
    }
    else
    {
        // Update display with current time
        update_bitmap();
    }
}

void update_bitmap()
{
    // Update the bitmap to reflect the current clock state
    // Sorry for the magic numbers in this part of the code for digit locations
    wipe_bitmap();
    if (minutes > 0)
    {
        display_digit(digits[seconds0], 5, 5);
        display_digit(digits[seconds1], 3, 3);
        display_digit(colon, 1, 3);
        display_digit(digits[minutes], 0, 3);

    }
    else
    {
        display_digit(digits[tenths], 5, 5);   // SCREEN_WIDTH - DIGIT_WIDTH - 2
        display_digit(dot, 3, 5);
        display_digit(digits[seconds0], 2, 5); // SCREEN_WIDTH - 2 * (DIGIT_WIDTH + 2)
        if (seconds1 > 0)
        {
            display_digit(digits[seconds1], 0, 3); // SCREEN_WIDTH - 3 * (DIGIT_WIDTH + 2)
        }
    }
}

void display_digit (uint16_t *digit, int num_bytes, int byte_offset)
{
    // Instantiate the row and start byte, and define the bit masks to use later
    unsigned char row;
    unsigned char start_byte;
    unsigned char mask1 = 0xFFFF >> (DIGIT_WIDTH - (8 - byte_offset));
    unsigned char mask2 = 0xFFFF << (8 - byte_offset);

    // Write, line-by-line, the specified digit to the display board, at the given offset from the left side of the board
    for (int i = DIGIT_HEIGHT_OFFSET; i < DIGIT_HEIGHT + DIGIT_HEIGHT_OFFSET; ++i)
    {
        // Reference to current line of digit for convenience
        uint16_t curr_line = digit[i - DIGIT_HEIGHT_OFFSET];

        // When calculating the row number, note that the display board considers all four panels side-by-side
        // for bitmap memory purposes. Therefore, to index in the board's rows top to bottom, you need to
        // index as such: 0, 2, 4, 6, 8, 10, ... , 30, 1, 3, 5, 7, 9, ... , 27, 29, 31
        // The calculation for row below takes this into account
        row = 2 * i;
        if (i >= PANEL_HEIGHT)
        {
            row -= SCREEN_HEIGHT - 1;
        }
        start_byte = row * SCREEN_WIDTH_B + num_bytes;

        // Set the 16 bits for this line in three steps, since the digit could be spread over three bytes
        // Use bit masking to retain the existing contents of the byte where we don't need to overwrite it
        // First byte - if the digit is offset by n relative to the current byte, write bits 1:8-n bits to this byte
        bitmap[start_byte] = (bitmap[start_byte] & ~mask1) | (curr_line >> (8 + byte_offset) & mask1);
        // Second byte - write bits n+1:n+8 to this byte
        bitmap[start_byte + 1] = ((curr_line >> byte_offset));
        // Third byte - write bits (n+9:end) to this byte
        bitmap[start_byte + 2] = (bitmap[start_byte + 2] & ~mask2) | ((curr_line << (8 - byte_offset)) & mask2);
    }
}

void display_idle()
{
    wipe_bitmap();
    bitmap[0] = 0b01111111;
    bitmap[7] = 0b11111110;
    bitmap[248] = 0b01111111;
    bitmap[255] = 0b11111110;
}

void setup()
{
    // Initialise clock and display
    init_clock();
    init_display();

    // Clear display board
    wipe_bitmap();
    delay(500);

    // Set up display refresh timer
    Timer1.initialize(DISPLAY_REFRESH_MS);
    Timer1.attachInterrupt(update_display);
}

void loop()
{
    if (tick >= BITMAP_REFRESH_TICKS)
    {
        update_clock();
        tick = 0;
    }
}

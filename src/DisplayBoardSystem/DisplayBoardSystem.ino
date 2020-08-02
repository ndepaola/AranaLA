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
#define DISPLAY_REFRESH_MS 500   // Microseconds between display refreshes. Was originally 500 but I found 5000 reduced flickering?
#define BITMAP_REFRESH_TICKS 100 // 1000 ticks (milliseconds) before display board updates
#define BRIGHTNESS 5

#define PANEL_WIDTH 32
#define PANEL_HEIGHT 16
#define SCREEN_WIDTH PANEL_WIDTH * 2
#define SCREEN_HEIGHT PANEL_HEIGHT * 2
#define SCREEN_WIDTH_B SCREEN_WIDTH / 8

#include "font.h"

// Clock variables
// TODO: Unsigned char to take up less memory instead?
unsigned char tick = 0; // Ticks up to 100(?) then resets
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
    wipe_bitmap();
    if (minutes > 0)
    {
        display_digit(seconds0, SCREEN_WIDTH - DIGIT_WIDTH - 2);
        display_digit(seconds1, SCREEN_WIDTH - 2 * (DIGIT_WIDTH + 2));
        display_digit(minutes, SCREEN_WIDTH - 3 * (DIGIT_WIDTH + 2));
    }
    else
    {
        display_digit(tenths, SCREEN_WIDTH - DIGIT_WIDTH - 2);
        display_digit(seconds0, SCREEN_WIDTH - 2 * (DIGIT_WIDTH + 2));
        if (seconds1 > 0) {
            display_digit(seconds1, SCREEN_WIDTH - 3 * (DIGIT_WIDTH + 2));
        }
    }
}

void display_digit(int digit, int offset)
{
    // Reference to current digit for convenience
    const uint16_t *digit_char = digits[digit];

    // Determine the number of bytes from the left that the digit to be drawn is, and the offset starting from that byte
    unsigned char byte_offset = offset % 8;
    unsigned char num_bytes = (offset - byte_offset) / 8;

    // Write the specified digit to the display board, at the given offset from the left side of the board
    // Write line-by-line
    unsigned char row;
    unsigned char start_byte;
    unsigned char mask1;
    unsigned char mask2;
    for (int i = DIGIT_HEIGHT_OFFSET; i < DIGIT_HEIGHT + DIGIT_HEIGHT_OFFSET; i++)
    {
        // Reference to current line of digit for convenience
        uint16_t curr_line = digit_char[i - DIGIT_HEIGHT_OFFSET];

        // When calculating the row number, note that the display board considers all four panels side-by-side
        // for bitmap memory purposes. Therefore, to index in the board's rows top to bottom, you need to
        // index as such: 0, 2, 4, 6, 8, 10, ... , 30, 1, 3, 5, 7, 9, ... , 27, 29, 31
        // The calculation for row below takes this into account
        row = (2 * i - (SCREEN_HEIGHT - 1) * (i >= PANEL_HEIGHT));
        start_byte = row * SCREEN_WIDTH_B + num_bytes;

        // TODO: Does it matter that I compare every iteration of the loop?
        if (byte_offset == 0)
        {
            // Straightforward case - digit lands exactly within two bytes of bitmap
            bitmap[start_byte] = (uint8_t)(curr_line >> 8);
            bitmap[start_byte + 1] = (uint8_t)(curr_line);
        }
        else
        {
            // More generic case - digit is partially spread over three bytes of bitmap
            // Use bit masking to nly modify the bits within the digit's bounds
            mask1 = 0xFFFF >> (DIGIT_WIDTH - (8 - byte_offset));
            mask2 = 0xFFFF << (8 - byte_offset);
            bitmap[start_byte] = (uint8_t)(bitmap[start_byte] & ~mask1) | (curr_line >> (8 + byte_offset) & mask1);
            bitmap[start_byte + 1] = (uint8_t)((curr_line >> byte_offset));
            bitmap[start_byte + 2] = (uint8_t)(bitmap[start_byte + 2] & ~mask2) | ((curr_line << (8 - byte_offset)) & mask2);
        }
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
    // delay(1000);
    // display_idle();

    if (tick >= BITMAP_REFRESH_TICKS)
    {
        update_clock();
        tick = 0;
    }
}

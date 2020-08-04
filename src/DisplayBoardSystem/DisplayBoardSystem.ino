// screen /dev/cu.usbmodem14201 9600
#include <SPI.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include <RH_RF95.h>
#include "TimerOne.h"

// Pin definitions and RF parameters
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

// Display board parameters
#define BRIGHTNESS 1  // Between 1 and 255
#define DISPLAY_REFRESH_MS 500   // Microseconds between display refreshes
#define BITMAP_REFRESH_TICKS 100 // 100 ticks (microseconds) between clock updates
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 16
#define SCREEN_WIDTH PANEL_WIDTH * 2
#define SCREEN_HEIGHT PANEL_HEIGHT * 2
#define SCREEN_WIDTH_B SCREEN_WIDTH / 8

// Import font from separate file
#include "font.h"

// Clock variables
volatile unsigned long tick = 0;
unsigned char last_minutes = -1, last_tens = -1, last_seconds = -1, last_coln = -1;
bool is_idle = false;

// RF variables
char msgType;
unsigned short msgTime;

// Other misc. variables
RH_RF95 rf95(RFM95_CS, RFM95_INT);
uint8_t bitmap[256];
RtcDS3231<TwoWire> rtcObject(Wire);
SPISettings settings(4000000, MSBFIRST, SPI_MODE0);
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
    // Initialise screen by wiping bitmap and setting IO pins
    wipe_bitmap();
    pinMode(pin_a, OUTPUT);
    pinMode(pin_b, OUTPUT);
    pinMode(pin_sck, OUTPUT);
    digitalWrite(pin_noe, LOW);
    pinMode(pin_noe, OUTPUT);
    SPI.begin();
}

void init_RF()
{
    // TODO: Currently, this prevents setup from completing - check with Bradley
    // Assign pins to RF module
    pinMode(RFM95_MISO, OUTPUT);
    pinMode(RFM95_MOSI, INPUT);
    pinMode(RFM95_RST, OUTPUT);
    pinMode(RFM95_CS, OUTPUT);
    digitalWrite(RFM95_RST, LOW);
    pinMode(RFM95_RST, OUTPUT);
    delayMicroseconds(100);
    pinMode(RFM95_RST, INPUT);
    delay(5);
    // Pause until RFM96W has initialised properly
    if (!rf95.init())
    {
        while (1);
    }
    rf95.setTxPower(13, false);
}

void RTCInterruptHandler()
{
    // Interrupt triggers every 0.001 seconds (from 1kHz square wave of DS3231)
    tick++;
}

void wipe_bitmap()
{
    // Clear screen
    memset(bitmap, 0xFF, 256);
}

void restart()
{
    // Reset display board to start counting from 0.0 again
    wipe_bitmap();
    tick = 0;
    last_minutes = last_tens = last_seconds = last_coln = -1;
    is_idle = false;
}

bool check_for_RF_message(char &msgType, unsigned short &msgTime)
{
    // TODO: Will become outdated as Bradley continues work on RF communication code
    uint8_t buf[16];
    uint8_t len;

    // Check if a message has been received
    if (rf95.recv(buf, &len))
    {
        // Retrieve msgType and msgTime from the received message
        msgType = buf[0];
        char systemTime[MSG_LENGTH - 2];
        for (int i = 0; i < MSG_LENGTH - 3; i++)
        {
            if ((char)buf[i + 2] == 'l')
            {
                systemTime[i] = '\0';
                break;
            }
            systemTime[i] = (char)buf[i + 2];
        }
        msgTime = atoi(systemTime);

        if (msgType == '1')
        {
            // Race started
            is_idle = false;
            restart();
        }
        else if (msgType == '0')
        {
            // Reset message received
            is_idle = true;
            restart();
        }
        return true;
    }
    else
    {
        return false;
    }
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
    // TODO: Can this be cleaned up / written more concisely?
    int total_seconds = tick >> 10;

    if (total_seconds < 60)
    {
        int tenths = (10 * (tick & 0x3FF)) >> 10;
        int tens = total_seconds / 10;
        int seconds = total_seconds - tens * 10;

        if (last_coln != 1)
        {
            display_digit_8(dot, 4, 5);
            last_coln = 1;
        }
        if (tens > 0 && tens != last_tens)
        {
            display_digit_16(digits[tens], 0, 3);
            last_tens = tens;
        }
        if (last_seconds != seconds)
        {
            display_digit_16(digits[seconds], 2, 5);
            last_seconds = seconds;
        }
        display_digit_16(digits[tenths], 5, 5);
    }
    else if (total_seconds < 600)
    {
        int minutes = total_seconds / 60;
        int seconds = total_seconds - minutes * 60;
        int tens = seconds / 10;
        seconds -= tens * 10;

        if (last_coln != 2)
        {
            display_digit_8(colon, 2, 3);
            last_coln = 2;
        }
        if (last_minutes != minutes)
        {
            display_digit_16(digits[minutes], 0, 3);
            last_minutes = 0;
        }
        if (tens != last_tens)
        {
            display_digit_16(digits[tens], 3, 3);
            last_tens = tens;
        }

        if (last_seconds != seconds)
        {
            display_digit_16(digits[seconds], 5, 5);
            last_seconds = seconds;
        }
    }
    else
    {
        is_idle = true;
    }
}

void display_digit_16(uint16_t *digit, int num_bytes, int byte_offset)
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

void display_digit_8(uint8_t *digit, int num_bytes, int byte_offset)
{
    // Instantiate the row and start byte, and define the bit masks to use later
    unsigned char row;
    unsigned char start_byte;
    unsigned char mask1 = 0xFF >> byte_offset;
    unsigned char mask2 = 0xFF << (8 - byte_offset);

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
        bitmap[start_byte] = (bitmap[start_byte] & ~mask1) | (curr_line >> byte_offset & mask1);
        // Second byte - write bits (9-n:end) to this byte
        bitmap[start_byte + 1] = (bitmap[start_byte + 1] & ~mask2) | ((curr_line << (8 - byte_offset)) & mask2);
    }
}

void display_idle()
{
    // Wipe the screen but light up the four corner LEDs, to signify that the screen is powered on but idling
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

    // Set up display refresh timer
    Timer1.initialize(DISPLAY_REFRESH_MS);
    Timer1.attachInterrupt(update_display);

    restart();
}

void loop()
{
    if (rf95.available())
    {
        check_for_RF_message(msgType, msgTime);
    }
    if (is_idle)
    {
        display_idle();
    }
    else if (tick >= BITMAP_REFRESH_TICKS)
    {
        update_clock();
    }
}

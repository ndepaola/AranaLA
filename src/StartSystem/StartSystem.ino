// screen /dev/cu.usbmodem14201
#include <AranaSend.h>

// Define DS3231M interrupt pin
#define DS3231_INT 3

// Define RFM96 pins
#define RFM95_SCK 13
#define RFM95_MISO 12
#define RFM95_MOSI 11
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 433.0

// Define SD card pins
#define SD_CS 8

// Define button pins
#define START_BUTTON 5
#define RESET_BUTTON 1
#define FALSE_START_BUTTON 4
#define SYNC_BUTTON 6

// Define LED (for buttons) pins
#define START_LED A3
#define RESET_LED A2
#define FALSE_START_LED A1
#define SYNC_LED A0

// Define Flash LED pin and associated delays
#define FLASH_LED 7
#define FLASH_DELAY 200 // milliseconds
#define FLASH_PAUSE 100

// Define latency seen in speaker system (not 100% consistent delay from speaker)
#define LATENCY 247 // milliseconds

// Start button additional variables
unsigned long syncTime = 0;
unsigned long off = ULONG_MAX;
unsigned long start_button_pressed = 0;
unsigned long msgTimeStart = 0;
int msgTypeStart = 0;
bool update = false;

// Sync button 3 second timer variables
unsigned int count = 0;
unsigned long sync_button_activated = 0;

// DS3231M Variables
unsigned long tick = 0; // Max value should not exceed 471,859,200 (about 1/10 of max value for unsigned long)


// Initialise the RFM96 module
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Initialise the RFM96 messaging protocols library
AranaSend rflib(RFM95_MISO, RFM95_MOSI, RFM95_RST, RFM95_CS, SD_CS);

// Create a function to handle interrupts from the RTC - just count up the 'tick' variable by 1
void RTCInterruptHandler()
{
    tick++;
}

// Set the DS3231M pin to enable interrupts from the 32 kHz square wave
void setupClock()
{
    pinMode(DS3231_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(DS3231_INT), RTCInterruptHandler, RISING);
}

void setup_RF()
{
    // Set pins for RF module
    // Serial.println(F("Beginning initialisation RF module."));
    pinMode(RFM95_MISO, OUTPUT); // default slave select pin for client mode
    pinMode(RFM95_MOSI, INPUT);
    pinMode(RFM95_RST, OUTPUT);
    pinMode(RFM95_CS, OUTPUT);
    digitalWrite(RFM95_RST, LOW);
    pinMode(RFM95_RST, OUTPUT);
    delayMicroseconds(100);
    pinMode(RFM95_RST, INPUT);
    delay(5);
    while (!rf95.init())
    {
        digitalWrite(START_LED, HIGH);
        digitalWrite(RESET_LED, HIGH);
        digitalWrite(SYNC_LED, HIGH);
        digitalWrite(FALSE_START_LED, HIGH);
        
        delay(500);
        
        digitalWrite(START_LED, LOW);
        digitalWrite(RESET_LED, LOW);
        digitalWrite(SYNC_LED, LOW);
        digitalWrite(FALSE_START_LED, LOW);

        delay(500);
    }
    rf95.setTxPower(13, false);
    // Serial.println(F("Completed initialisation of RF module."));
}

#define FILENAME "data.csv"
void setup_SD()
{
    // Serial.println(F("Beginning initialisation of SD card module."));
    digitalWrite(RFM95_CS, HIGH);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, LOW);

    // Wait until SD card is available
    while (!SD.begin(SD_CS))
    {
        digitalWrite(START_LED, HIGH);
        digitalWrite(RESET_LED, HIGH);
        digitalWrite(SYNC_LED, HIGH);
        digitalWrite(FALSE_START_LED, HIGH);
        
        delay(150);
        
        digitalWrite(START_LED, LOW);
        digitalWrite(RESET_LED, LOW);
        digitalWrite(SYNC_LED, LOW);
        digitalWrite(FALSE_START_LED, LOW);

        delay(100);

        digitalWrite(START_LED, HIGH);
        digitalWrite(RESET_LED, HIGH);
        digitalWrite(SYNC_LED, HIGH);
        digitalWrite(FALSE_START_LED, HIGH);
        
        delay(150);
        
        digitalWrite(START_LED, LOW);
        digitalWrite(RESET_LED, LOW);
        digitalWrite(SYNC_LED, LOW);
        digitalWrite(FALSE_START_LED, LOW);

        delay(600);
    }

    // If necessary, create the CSV file with headers
    if (!SD.exists(FILENAME))
    {
        // Serial.println(F("File does not exist - creating file and writing headers."));
        File file = SD.open(FILENAME, FILE_WRITE);
        file.println(F("Message Type,Time,Sync Time"));
        file.close();
    }

    digitalWrite(SD_CS, HIGH);
    digitalWrite(RFM95_CS, LOW);

    // Serial.println(F("Completed initialisation of SD card module."));
}

// Setup the environment
void setup()
{
    // Initialise buttons as inputs
    pinMode(START_BUTTON, INPUT);
    pinMode(RESET_BUTTON, INPUT);
    pinMode(SYNC_BUTTON, INPUT);
    pinMode(FALSE_START_BUTTON, INPUT);

    // Initialise LEDs as outputs
    pinMode(START_LED, OUTPUT);
    pinMode(RESET_LED, OUTPUT);
    pinMode(SYNC_LED, OUTPUT);
    pinMode(FALSE_START_LED, OUTPUT);
    pinMode(FLASH_LED, OUTPUT);

    // Set LEDs to off by default
    digitalWrite(START_LED, LOW);
    digitalWrite(RESET_LED, LOW);
    digitalWrite(SYNC_LED, LOW);
    digitalWrite(FALSE_START_LED, LOW);

    // Setup serial and RF communications
    // rflib.setup_serial();
    // rflib.setup_RF(rf95);
    // rflib.setup_SD();
    setup_RF();
    setup_SD();
    

    // Setup RTC
    setupClock();

    // Serial.println("Start system: Finished setup.\n");
}

// Function called when START button is pressed
void start_race()
{
    // Serial.println("Starting");
    // Set the message time to be relative to the sync time, with the delay of the speaker accounted for
    unsigned long msgTime = tick - syncTime + LATENCY; // TODO: Use RTC

    // Send the RF message
    rflib.send_RF_message(rf95, rflib.ID_GLOBAL, rflib.MSG_START, 1, msgTime);

    // Turn on the START button LED
    delay(LATENCY);
    digitalWrite(START_LED, HIGH);

    // Turn on the Fash LED
    digitalWrite(FLASH_LED, HIGH);
    delay(FLASH_DELAY);
    
    // Turn off the Flash LED
    digitalWrite(FLASH_LED, LOW);

    // Record start time of race to SD card
    // rflib.write_to_SD(rflib.MSG_START, msgTime, syncTime);
    msgTypeStart = rflib.MSG_START;
    msgTimeStart = msgTime;
}

// Function called when the STOP button is pressed.  This function is designed to save power by setting the LED display into low power mode (no LEDs on)
void reset()
{
    // Serial.println("Resetting");
    // Set the message time to be relative to the sync time
    unsigned long msgTime = tick - syncTime; // TODO: Use RTC

    // Send the RF message
    rflib.send_RF_message(rf95, rflib.ID_GLOBAL, rflib.MSG_STOP, 1, msgTime);

    // Turn on the STOP button LED
    digitalWrite(RESET_LED, HIGH);

    // Save the information to the SD card
    rflib.write_to_SD(rflib.MSG_STOP, msgTime, syncTime);
}

// Function called when START button is pressed within 4 seconds of last START button press
void false_start()
{
    // Serial.println("False start");
    // Set the message time to be relative to the sync time, with the delay of the speaker accounted for
    unsigned long msgTime = tick - syncTime + LATENCY; // TODO: Use RTC

    // Send the RF message
    rflib.send_RF_message(rf95, rflib.ID_GLOBAL, rflib.MSG_FALSE_START, 1, msgTime);
    delay(LATENCY);

    // Flash the bright LED twice in rapid succession
    digitalWrite(FLASH_LED, HIGH);
    digitalWrite(START_LED, HIGH);
    delay(FLASH_DELAY);
    digitalWrite(FLASH_LED, LOW);
    digitalWrite(START_LED, LOW);
    delay(FLASH_PAUSE);
    digitalWrite(FLASH_LED, HIGH);
    digitalWrite(START_LED, HIGH);
    delay(FLASH_DELAY);
    digitalWrite(FLASH_LED, LOW);

    // Save information to the SD card
    // rflib.write_to_SD(rflib.MSG_FALSE_START, msgTime, syncTime);
    msgTypeStart = rflib.MSG_FALSE_START;
    msgTimeStart = msgTime;
}

void sync()
{
    // Serial.println("Syncing");
    digitalWrite(SYNC_LED, HIGH);
    digitalWrite(FLASH_LED, HIGH);
    // syncTime = tick;
    delay(FLASH_DELAY);
    digitalWrite(FLASH_LED, LOW);
    unsigned long msgTime = 0; // TODO: Use RTC
    rflib.write_to_SD(rflib.MSG_SYNC, msgTime, syncTime);
}

void loop()
{
    unsigned long curr_time = tick;
    // if (digitalRead(START_BUTTON) == HIGH && (tick >= off - 30720 || off == ULONG_MAX))
    if (digitalRead(START_BUTTON) == HIGH)
    {
        unsigned long next_button_pressed = tick;
        if (next_button_pressed - start_button_pressed <= 128000 && !(tick <= 128000))
        {
            false_start();
        }
        else
        {
            start_race();
        }
        // off = tick - syncTime + 32768 - ((tick - syncTime) - curr_time);
        off = curr_time + 32768;
        while (digitalRead(START_BUTTON) == HIGH)
        {
            delay(5);
        }
        start_button_pressed = next_button_pressed;
        update = true;
    }
    else if (digitalRead(RESET_BUTTON) == HIGH && off == ULONG_MAX)
    {
        reset();
        // off = tick - syncTime + 32768 - ((tick - syncTime) - curr_time);
        off = curr_time + 32768;
        while (digitalRead(RESET_BUTTON) == HIGH)
        {
            delay(5);
        }
    }
    else if (digitalRead(SYNC_BUTTON) == HIGH)
    {
        sync();
        off = curr_time + 32768;
        while (digitalRead(SYNC_BUTTON) == HIGH)
        {
            delay(5);
        }        
    }
    else if (digitalRead(FALSE_START_BUTTON) == HIGH && (tick >= off - 25600 || off == ULONG_MAX))
    {

    }

    if (tick - start_button_pressed > 128000 && update)
    {
        rflib.write_to_SD(msgTypeStart, msgTimeStart, syncTime);
        update = false;
    }

    if (curr_time > off)
    {
        digitalWrite(START_LED, LOW);
        digitalWrite(RESET_LED, LOW);
        digitalWrite(FALSE_START_LED, LOW);
        digitalWrite(SYNC_LED, LOW);
        off = ULONG_MAX;
    }
}

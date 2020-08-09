#include "SoftwareSerial.h"
// #include <Wire.h>
// #include <RtcDS3231.h>
#include "limits.h"
// #include <stdint.h>
// #include <stdio.h>
// #include "SD.h"
#include <SPI.h>
#include <RH_RF95.h>
// #include <RHSoftwareSPI.h>





#define RFM95_SCK 13
#define RFM95_MISO 12
#define RFM95_MOSI 11
// #define RFM95_CS 4
// #define RFM95_RST 2
#define RFM95_INT 3 // Interrupt
#define RF95_FREQ 433.0
#define MSG_LENGTH 16

#define RFM95_CS 5
#define RFM95_RST 4

RH_RF95 rf95(RFM95_CS, RFM95_INT);
SoftwareSerial mySerial(8, 9);


# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]

# define ACTIVATED LOW

boolean isPlaying = false;

// Declarations
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

unsigned long time_start = 0;
unsigned long time_now = millis();

volatile unsigned long milliSeconds = 0;

unsigned long counter = 0;
bool LED_on = false;

char msgType;
unsigned int msgTime;

// screen /dev/cu.usbmodem14101
bool check_for_RF_message(char &msgType, unsigned int &msgTime)
{
    uint8_t buf[16];
    uint8_t len;

    // Check if a message has been received 
    if (rf95.recv(buf, &len))
    {   
        // Start measuring execution time
        // unsigned long current_micros = micros();

        // Retrieve msgType and msgTime from the received message 
        msgType = buf[0];
        for(int i=0; i<12; i++){
            // Serial.println((char) buf[i]);
        }
        // Serial.println(" ");
        


        char systemTime[MSG_LENGTH-2];
        for(int i=0; i<MSG_LENGTH-3; i++) {
            if ((char) buf[i+2] == 'l') {
                systemTime[i] = '\0';
                break;
            }
            systemTime[i] = (char) buf[i+2];
        }
        msgTime = atoi(systemTime);

        // Finish measuring execution time
        // current_micros = micros() - current_micros;

        // Debugging prints
        // Serial.println("Received a new message:");
        // Serial.println("Execution time (microseconds)");
        // Serial.println(current_micros);  
        // Serial.println("Message type");
        // Serial.println(msgType);
        // Serial.println("Message time");
        // Serial.println(msgTime);
        // Serial.println("");
        return true;
    } else {
        return false;
    }     
}

void setup() {
    
    playFirst();
    isPlaying = true;
  
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);

    pinMode(RFM95_MISO, OUTPUT); // default slave select pin for client mode 
    pinMode(RFM95_MOSI, INPUT);
    pinMode(RFM95_RST, INPUT);
    pinMode(RFM95_CS, OUTPUT);

//    rf95.setTxPower(13, false);

    digitalWrite(LED_BUILTIN, LOW);
    delay(2000);
    digitalWrite(LED_BUILTIN, HIGH);

    // Serial.begin(9600);
    mySerial.begin(9600);
    // while (!Serial) {
    //   continue; // Wait for Serial port to be available
    // }

    // Manual Reset
    digitalWrite(RFM95_RST, LOW);
    pinMode(RFM95_RST, OUTPUT);
    delayMicroseconds(100);
    pinMode(RFM95_RST, INPUT);
    delay(5);
    
    if (!rf95.init()) {
    //   Serial.println("init failed");
      while(1);
    }
    // Serial.println("Welcome!");

    rf95.setTxPower(13, false);

    digitalWrite(LED_BUILTIN, HIGH);

    // Serial.println("Finished setup.");
}

void loop()
{
    if (rf95.available())
        
        if (check_for_RF_message(msgType, msgTime)) {
            // a message has been received 
            // print out the received message 
            // Serial.println("Message type");
            // Serial.println(msgType);
            // Serial.println("Message time");
            // Serial.println(msgTime);

            // if (msgType == '1')
            // {
                // if(isPlaying)
                // {

                // }
                // else
                // {
                play();
                isPlaying = true;
                // }
            // }
        }
}

void playFirst()
{
  execute_CMD(0x3F, 0, 0);
  delay(500);
  setVolume(20);
  delay(500);
  execute_CMD(0x11,0,1); 
  delay(500);
}

void play()
{
  execute_CMD(0x0D,0,1); 
  delay(500);
}

void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}

void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
    // Calculate the checksum (2 bytes)
word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
// Build the command line
byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
//Send the command line to the module
for (byte k=0; k<10; k++)
{
mySerial.write( Command_line[k]);
}
}
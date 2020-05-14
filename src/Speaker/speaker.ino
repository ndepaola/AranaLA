///              MP3 PLAYER PROJECT
/// http://educ8s.tv/arduino-mp3-player/
//////////////////////////////////////////


#include "SoftwareSerial.h"
SoftwareSerial mySerial(7, 6);
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]

# define ACTIVATED LOW

#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
#define RFM95_SCK 13
#define RFM95_MISO 12
#define RFM95_MOSI 11
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 3
#define RF95_FREQ 433.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

int buttonNext = 2;
int buttonPause = 5;
int buttonPrevious = 4;
boolean isPlaying = false;


void setupRF()
{
    Serial.println("Being setting up pins for RFM95.");
    pinMode(RFM95_MISO, OUTPUT); // default slave select pin for client mode 
    pinMode(RFM95_MOSI, INPUT);
    pinMode(RFM95_RST, OUTPUT);
    pinMode(RFM95_CS, OUTPUT);
    Serial.println("Finished setting up pins.");

    Serial.println("Resetting RFM95 Module.");
    // digitalWrite(RFM95_CS, LOW);
    // digitalWrite(RFM95_RST, HIGH);
    // delay(10);
    // digitalWrite(RFM95_RST, LOW);
    // Manual Reset
    digitalWrite(RFM95_RST, LOW);
    pinMode(RFM95_RST, OUTPUT);
    delayMicroseconds(100);
    pinMode(RFM95_RST, INPUT);
    delay(5);
    
    
    Serial.println("RFM95 Module reset.");

    // while (!rf95.init());
    if (!rf95.init()) {
      Serial.println("init failed");
      while(1);
    }
    Serial.println("RFM95 Module initialised!");

    Serial.println("Set transmission power to 13 dBm.");
    rf95.setTxPower(13, false);
    Serial.println("Power set to 13 dBm.");
}

void setupSerial()
{
    Serial.begin(9600);
    while (!Serial) {
      continue; // Wait for Serial port to be available
    }

    Serial.println("Serial setup complete");
    // Serial.println("Setup Complete");
}

void setup () {

pinMode(buttonPause, INPUT);
digitalWrite(buttonPause,HIGH);
// pinMode(buttonNext, INPUT);
// digitalWrite(buttonNext,HIGH);
// pinMode(buttonPrevious, INPUT);
// digitalWrite(buttonPrevious,HIGH);

    mySerial.begin (9600);
    delay(1000);
    playFirst();
    isPlaying = true;

    setupSerial();

    delay(1000);

    setupRF();

    Serial.println("Setup Complete");

}

uint8_t buf[255];
uint8_t len;

void check_for_RF_message()
{
    long other;
    uint8_t len;
    if (rf95.recv((uint8_t*)&buf, &len))
    {

        // Serial.println((char*)buf);
        Serial.println(other);
        Serial.println("Received.");
        // test

        if(isPlaying)
        {
        pause();
        isPlaying = false;
        }else
        {
        isPlaying = true;
        play();
        }
    }
    // if (rf95.recv(buf, &len))
        // restart();
        
}

void loop () { 

 if (digitalRead(buttonPause) == ACTIVATED)
  {
    if(isPlaying)
    {
      pause();
      isPlaying = false;
    }else
    {
      isPlaying = true;
      play();
    }

    Serial.println("Button pressed!");
  }


//  if (digitalRead(buttonNext) == ACTIVATED)
//   {
//     if(isPlaying)
//     {
//       playNext();
//     }
//   }

//    if (digitalRead(buttonPrevious) == ACTIVATED)
//   {
//     if(isPlaying)
//     {
//       playPrevious();
//     }
//   }
    if (rf95.available())
        check_for_RF_message();
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

void pause()
{
  execute_CMD(0x0E,0,0);
  delay(500);
}

void play()
{
  execute_CMD(0x0D,0,1); 
  delay(500);
}

void playNext()
{
  execute_CMD(0x01,0,1);
  delay(500);
}

void playPrevious()
{
  execute_CMD(0x02,0,1);
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
#include <Wire.h>
#include <RtcDS3231.h>
#include "limits.h"
#include <stdint.h>
#include <stdio.h>
#include "SD.h"
#include <SPI.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
#define RFM95_SCK 13
#define RFM95_MISO 12
#define RFM95_MOSI 11
#define RFM95_CS 4
#define RFM95_RST 2
#define RFM95_INT 3 // Interrupt
#define RF95_FREQ 433.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

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

unsigned long time_start = 0;
unsigned long time_now = millis();

volatile unsigned long milliSeconds = 0;

unsigned long counter = 0;
bool LED_on = false;


uint8_t buf[255];
uint8_t len;

void check_for_RF_message()
{
    long other;
    uint8_t len;
    if (rf95.recv((uint8_t*)&other, &len))
    {
        // Serial.println((char*)buf);
        Serial.println(other);
        Serial.println("Received.");
        // test
    }
    // if (rf95.recv(buf, &len))
        // restart();
        
}

void setup() {
    // // TIMER 2 for interrupt frequency 1000 Hz:
    // cli(); // stop interrupts
    // TCCR2A = 0; // set entire TCCR2A register to 0
    // TCCR2B = 0; // same for TCCR2B
    // TCNT2  = 0; // initialize counter value to 0
    // // set compare match register for 1000 Hz increments
    // OCR2A = 249; // = 16000000 / (64 * 1000) - 1 (must be <256)
    // // turn on CTC mode
    // TCCR2B |= (1 << WGM21);
    // // Set CS22, CS21 and CS20 bits for 64 prescaler
    // TCCR2B |= (1 << CS22) | (0 << CS21) | (0 << CS20);
    // // enable timer compare interrupt
    // TIMSK2 |= (1 << OCIE2A);
    // sei(); // allow interrupts
  
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

    Serial.begin(9600);
    while (!Serial) {
      continue; // Wait for Serial port to be available
    }

    // Manual Reset
    digitalWrite(RFM95_RST, LOW);
    pinMode(RFM95_RST, OUTPUT);
    delayMicroseconds(100);
    pinMode(RFM95_RST, INPUT);
    delay(5);
    
    if (!rf95.init()) {
      Serial.println("init failed");
      while(1);
    }
    Serial.println("Welcome!");
//    time_start = millis();

    rf95.setTxPower(13, false);

    digitalWrite(LED_BUILTIN, HIGH);

    Serial.println("Finished setup.");
}

void loop()
{
    if (rf95.available())
       check_for_RF_message();
//     if (counter <= 100000){
// //        Serial.println("Made it 1");
//         if (rf95.available()) {
//             Serial.println("Made it 2");
//             if (rf95.recv(buf, &len)) {
//                 Serial.println((char*) buf);
//                 counter++;
//                 Serial.println(counter);
//                 // rf95.send((const uint8_t*)&counter, sizeof(counter)); // tell display board  
//                 // rf95.waitPacketSent();
//             }
//         }
//     }
//     else{
//         Serial.println("Finished!");
//         while(1);
//     }
    
//   if (rf95.available())
//   {
//     // Should be a message for us now   
//     uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
//     uint8_t len = sizeof(buf);
//     if (rf95.recv(buf, &len))
//     {
//       digitalWrite(LED_BUILTIN, HIGH);
// //      RH_RF95::printBuffer("request: ", buf, len);
//       Serial.print("got request: ");
//       Serial.println((char*)buf);
// //      Serial.print("RSSI: ");
// //      Serial.println(rf95.lastRssi(), DEC);
      
//       // Send a reply
//       uint8_t data[] = "And hello back to you";
//       rf95.send(data, sizeof(data));
//       rf95.waitPacketSent();
//       Serial.println("Sent a reply");
//        digitalWrite(LED_BUILTIN, LOW);
//     }
//     else
//     {
//       Serial.println("recv failed");
//     }
//   }
}


// ISR(TIMER2_COMPA_vect){
//     //interrupt commands for TIMER 2 here
//     milliSeconds++;
  
//     if (milliSeconds % (500 / counter) == 0) {
//         if (LED_on) {
//             digitalWrite(LED_BUILTIN, LOW);
//             LED_on = false;
//         }
//         else {
//             digitalWrite(LED_BUILTIN, HIGH);
//             LED_on = true;
//         }
//     }

//     if (milliSeconds == 1000) {
//       milliSeconds = 0;
//     }
// }

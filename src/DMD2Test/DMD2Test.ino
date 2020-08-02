/*
  Countdown on a single DMD display
 */

#include <SPI.h>
#include <DMD2.h>
#include <fonts/Arial14.h>`

const int COUNTDOWN_FROM = 44;
int counter = COUNTDOWN_FROM;

// 2, 2 seems to fix the offset between horizontally adjacent panels 
SoftDMD dmd(2, 2);  // DMD controls the entire display // 1, 1 
DMD_TextBox box(dmd, 0, 2);  // "box" provides a text box to automatically write to/scroll the display // 0, 2

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  dmd.setBrightness(20);
  dmd.selectFont(Arial14);
  dmd.begin();
}

// the loop routine runs over and over again forever:
void loop() {
  // Serial.print(counter);
  // Serial.println(F("..."));
  // box.print(' ');
  // box.print(counter);
  box.println(F("wooooooooooooords"));
  // counter--;
  delay(1000);

  // if(counter == 0) {
  //   // for(int i = 0; i < 3; i++) {
  //   //   dmd.fillScreen(true);
  //   //   delay(500);
  //   //   dmd.clearScreen();
  //   //   delay(500);
  //   // }
  //   box.clear();
  //   counter = COUNTDOWN_FROM;
  // }
}
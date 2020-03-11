#include <SPI.h>

// display data
#define pin_a   6
#define pin_b   7
#define pin_sck 8
#define pin_noe 9

#define RF95_FREQ 433.0

byte scan_row;
uint8_t bitmap[256];
SPISettings settings(4000000, MSBFIRST, SPI_MODE0);

void init_display()
{
    memset(bitmap, 0xFF, 256);
    pinMode(pin_a, OUTPUT);
    pinMode(pin_b, OUTPUT);
    pinMode(pin_sck, OUTPUT);
    pinMode(pin_noe, OUTPUT); 
    SPI.begin();
}

#include <Wire.h>
#include <RtcDS3231.h>

// clock data
volatile unsigned long tick = 0;
unsigned long next_tenth = 0;
unsigned long next_display = 0;
RtcDS3231 <TwoWire> rtcObject(Wire);

void init_clock()
{
    pinMode(2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(2), RTCInterruptHandler, RISING);
    rtcObject.Begin();
    rtcObject.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
    rtcObject.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1kHz);  
}

#include <RH_RF95.h>

RH_RF95 rf95(4, 3);

void init_RF()
{
    if (!rf95.init())
      Serial.println("init failed"); 

    rf95.setFrequency(433);
}

uint8_t buf[255];
uint8_t len;

int last_minutes = -1, last_tens = -1, last_seconds = -1, last_coln = -1;

void restart()
{
    //Serial.println("restart clock!");
    tick = 0;
    last_minutes = last_tens = last_seconds = last_coln = -1;
    next_tenth = 0;
    next_display = 0;
    memset(bitmap, 0xFF, 256);
}

void check_for_RF_message()
{
    if (rf95.recv(buf, &len))
        restart();
}

void setup()
{ 
    //Serial.begin(9600);
    //while (!Serial) ; // Wait for serial port to be available
  
    init_RF();
    init_display();
    init_clock();
    update_bitmap(0);

    //Serial.println("init successful");
}

void RTCInterruptHandler()
{
	  tick++;
}

void update_display()
{      
    next_display = tick + 1;
    const int rowsize = 16;
    
	  uint8_t * rows[4] =
	  {
		    bitmap + (scan_row +  0) * rowsize,
		    bitmap + (scan_row +  4) * rowsize,
		    bitmap + (scan_row +  8) * rowsize,
		    bitmap + (scan_row + 12) * rowsize,
	  };

    rf95.DisableInterrupt();
    SPI.beginTransaction(settings);

	  for (int i = 0; i < rowsize; i++) 
	  {
		    SPI.transfer(*(rows[3] ++));
		    SPI.transfer(*(rows[2] ++));
		    SPI.transfer(*(rows[1] ++));
		    SPI.transfer(*(rows[0] ++));
	  }

    SPI.endTransaction();
    rf95.EnableInterrupt();

	  digitalWrite(pin_noe, LOW);
	  digitalWrite(pin_sck, HIGH);
	  digitalWrite(pin_sck, LOW);
	  digitalWrite(pin_a, scan_row & 0x01);
	  digitalWrite(pin_b, scan_row & 0x02);

	  scan_row = (scan_row + 1) % 4;
	  analogWrite(pin_noe, 255);
}


void update_bitmap(unsigned long time)
{   
    int total_seconds = time >> 10;

    if (total_seconds < 60)
    {
        int tenths = (10 * (time & 0x3FF)) >> 10;
        int tens = total_seconds / 10;
        int seconds = total_seconds - tens * 10;

        if (last_coln != 1)
        {
            dot_coln (36,4,1);
            last_coln = 1;
        }
        if (tens > 0 && tens != last_tens)
        {
            display_num(32, 2, tens); 
            last_tens = tens;
        } 
        if (last_seconds != seconds)
        {
            display_num(34, 4, seconds);
            last_seconds = seconds;
        }
        display_num(37, 6, tenths);
    }
    else if (total_seconds < 600)
    {
        int minutes = total_seconds / 60;
        int seconds = total_seconds - minutes * 60;
        int tens = seconds / 10;
        seconds -= tens * 10;

        if (last_coln != 2)
        {
            dot_coln(34,3,2);
            last_coln = 2;
        }
        if (last_minutes != minutes)
        {
            display_num(32, 2, minutes);
            last_minutes = 0;
        }
        if (tens != last_tens)
        {
            display_num(35, 4, tens);
            last_tens = tens;
        }

        if (last_seconds != seconds)
        {
            display_num(37, 6, seconds);
            last_seconds = seconds;
        }
    }
    else
        memset(bitmap, 0xFF, 256);
            
    unsigned long count =  (10 * time) >> 10;      
    next_tenth = (((count + 1) << 10) + 9) / 10; 
}

void loop()
{
   if (rf95.available())
       check_for_RF_message();
       
   if (tick >= next_tenth)
       update_bitmap(tick);
   
   if (tick > next_display)
       update_display();
}

const uint16_t font2[10][28] =
{  
    { 0x07E0, 0x0FF0, 0x1FF8, 0x3FFC, 0x7FFE, 0xFE7F, 0xFC3F, 0xF81F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF81F, 0xFC3F, 0xFE7F, 0x7FFE, 0x3FFC, 0x1FF8, 0x0FF0, 0x07E0 },  
	  { 0x07C0, 0x0FC0, 0x1FC0, 0x3FC0, 0x3FC0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x3FFC, 0x3FFC, 0x3FFC, 0x3FFC },
	  { 0x0FF0, 0x1FF8, 0x3FFC, 0x7FFE, 0xFC3F, 0xF81F, 0xF00F, 0xF00F, 0x000F, 0x000F, 0x000F, 0x000F, 0x001F, 0x003E, 0x007C, 0x00F8, 0x01F0, 0x03E0, 0x07C0, 0x0F80, 0x1F00, 0x3E00, 0x7C00, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF },
	  { 0x0FF0, 0x1FF8, 0x3FFC, 0x7FFE, 0xFC3E, 0xF81F, 0xF00F, 0xF00F, 0x000F, 0x000F, 0x001F, 0x003E, 0x0FFC, 0x0FF8, 0x0FF8, 0x0FFC, 0x003E, 0x001F, 0x000F, 0x000F, 0xF00F, 0xF00F, 0xF81F, 0xFC3F, 0x7FFE, 0x3FFC, 0x1FF8, 0x0FF0 },
	  { 0x00FC, 0x00FC, 0x01FC, 0x01FC, 0x03FC, 0x03FC, 0x07BC, 0x07BC, 0x0F3C, 0x0F3C, 0x1E3C, 0x1E3C, 0x3C3C, 0x3C3C, 0x783C, 0x783C, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x003C, 0x003C, 0x003C, 0x003C, 0x003C, 0x003C, 0x003C, 0x003C },
	  { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF000, 0xF000, 0xF000, 0xF000, 0xF000, 0xF000, 0xF000, 0xF000, 0xFFF8, 0xFFFC, 0xFFFE, 0xFFFF, 0x000F, 0x000F, 0x000F, 0x000F, 0x000F, 0xF00F, 0xF00F, 0xF00F, 0xFFFF, 0xFFFE, 0x3FFC, 0x1FF8 },
	  { 0x1FF8, 0x3FFC, 0x7FFE, 0xFFFF, 0xFC3F, 0xF81F, 0xF00F, 0xF00F, 0xF000, 0xF000, 0xF800, 0xFC00, 0xFFF8, 0xFFFC, 0xFFFE, 0xFFFF, 0xFC3F, 0xF81F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF81F, 0xFC3F, 0xFFFF, 0x7FFE, 0x3FFC, 0x1FF8 },
	  { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x000F, 0x000F, 0x000F, 0x000F, 0x000F, 0x000F, 0x000F, 0x001F, 0x003C, 0x0078, 0x00F0, 0x01E0, 0x03C0, 0x0780, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0F00 },
	  { 0x1FF8, 0x3FFC, 0x7FFE, 0xFFFF, 0xFC3F, 0xF81F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF81F, 0xFC3F, 0x3FFC, 0x1FF8, 0x1FF8, 0x3FFC, 0xFC3F, 0xF81F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF81F, 0xFC3F, 0xFFFF, 0x7FFE, 0x3FFC, 0x1FF8 },
    { 0x1FF8, 0x3FFC, 0x7FFE, 0xFFFF, 0xFC3F, 0xF81F, 0xF00F, 0xF00F, 0xF00F, 0xF00F, 0xF81F, 0xFC3F, 0xFFFF, 0x7FFF, 0x3FFF, 0x1FFF, 0x001f, 0x000F, 0x000F, 0x000F, 0xF00F, 0xF00F, 0xF81F, 0xFC3F, 0xFFFF, 0x7FFE, 0x3FFC, 0x1FF8 }
};

const uint8_t dot[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C};
const uint8_t colon[] ={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


void display_num(unsigned int bX, unsigned int set, unsigned int number)
{
  // Code to deal with offset if it doesn't fall at start of a bit array
  int opp = 8 - set;
  int origL = (0xFF >> set);
  int origR = (0xFF << opp);

  for (int i = bX - 24; i < 256; i = i + 8)
  {
      bitmap[i + 0] |= origL;
      bitmap[i + 1] |= 0xFF;
      bitmap[i + 2] |= origR;
  }

  for (int i = 0; i < 14; i++)
  {
      const uint16_t* font = font2[number];
                  
      bitmap[bX + 0 + 16 * i] &= ~(font[i]>>(8+set));    
      bitmap[bX + 1 + 16 * i] &= ~(font[i]>>set);
      bitmap[bX + 2 + 16 * i] &= ~(font[i]<<opp);
      
      bitmap[bX + 0 + 16 * i - 24] &= ~(font[i+14]>>(8+set));
      bitmap[bX + 1 + 16 * i - 24] &= ~(font[i+14]>>set);
      bitmap[bX + 2 + 16 * i - 24] &= ~(font[i+14]<<opp);
  }
}

void dot_coln (unsigned int bX, unsigned int set, unsigned int typer) 
{
    int opp = 8-set;
    int origL = (0XFF >> set); 
    int origR = (0XFF << (7-set));
  
    for (int i=bX-24; i<256; i=i+8)
    {
        bitmap[i] |= origL; 
        bitmap[i+1] |=  origR;
    }

    const uint8_t* font;
    if (typer == 1)  
        font = dot;
    else
        font = colon;
                               
    for (int i =0; i<14; i++)
    {
        bitmap[bX + 0 + 16 * i]&= ~(font[i]>>(set)); 
        bitmap[bX + 1 + 16 * i]&= ~(font[i]<<opp);  
        
        bitmap[bX + 0 + 16 * i - 24]&= ~(font[i+14]>>(set)); 
        bitmap[bX + 1 + 16 * i - 24]&= ~(font[i+14]<<opp);                       
   }
}

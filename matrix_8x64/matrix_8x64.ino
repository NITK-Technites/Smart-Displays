/*
  Smart Displays : 8x64 LED Matrix
  Normal code:
        Multiplexing( No timer Interrypts )

  code by:
  K S S M KAMAL
  NIT SURATHKAL
  Dept of ECE
*/

#include <SPI.h>
#include "fonts.h"

#define clock_pin 13  //clock pin for 74HC595
#define latch_pin 2   //latch pin for 74HC595
#define blank_pin 3   //blank pin for 74HC595
#define data_pin 11   //data pin for 74HC595
#define src_latch 4   //clock pin for TLC59213



unsigned long pause_time = 3000000;
unsigned long scroll_speed = 40000;

unsigned long present_ms = 0;
unsigned long last_ms = 0;
unsigned long pause_ms = 0;

int scroll_index = 0;
boolean pause;
boolean scroll_direction = 0;  //0 - scroll left and 1 - scroll right

uint64_t temp, led[8] = {0};  //led array with each element consisting 64 bits which corresponds to row data in multiplexing

uint64_t scroll_msg[2][8];   //led array with real and virtual matrix data
unsigned int msg_index = 0;

const byte char_set[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!#$%&'()*+-,.~:;<>?@[]/\\=_";

String msg = "TECHNITES";

void setup() {
  // put your setup code here, to run once:
  SPI.setBitOrder(LSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  //declaring pins as outputs
  pinMode(latch_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(blank_pin, OUTPUT);
  pinMode(src_latch, OUTPUT);
  SPI.begin();
  Serial.begin(115200);
  for (int i = 0; i < 9; i++)
  {
    SPI.transfer(0b00000000);
  }
  latch();
  //interrupts();
  msg_check();
  frame_decode();
  initialize();
}

void loop()
{
  present_ms = micros();
  if (present_ms - last_ms > scroll_speed)
  {
    last_ms = present_ms;
    if (pause == true) {
      finish_scroll();
      if (present_ms - pause_ms > pause_time)
      {
        if(Serial.available() > 0)
        {
          msg = Serial.readStringUntil('\n');
          msg_check();
        }
        initialize();
      }
    }
    else
      scroll_message();
  }
  scroll_multiplex();
 //multiplex();
}

void multiplex()
{
  uint8_t row_sel = 128;
  for (int i = 0; i < 8; i++)
  {
   // SPI_data_transfer(0);
    //SPI.transfer(0);
    //latch();
    SPI_data_transfer(led[i]);
    SPI.transfer(row_sel);
    row_sel = row_sel >> 1;
    latch();
  } 
}

void msg_check()
{
  if(msg.length() % 8 != 0)
     msg += "       ";
}
void scroll_multiplex()
{
  uint8_t row_sel = 128;
  for (int i = 0; i < 8; i++)
  {
   // SPI_data_transfer(0);
    //SPI.transfer(0);
    //latch();
    SPI_data_transfer(scroll_msg[0][i]);
    SPI.transfer(row_sel);
    row_sel = row_sel >> 1;
    latch();
  }
}

void initialize()
{
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      scroll_msg[i][j] = 0;
    }
  }
  msg_index = 0;
  scroll_frame_decode();
  pause = false;
}

void finish_scroll()
{
  for (int i = 0; i < 8; i++)
  {
    scroll_msg[0][i] = scroll_msg[0][i] << 1;
  }
}

void scroll_message()
{
  for (int i = 0; i < 8; i++)
  {
    scroll_msg[0][i] = scroll_msg[0][i] << 1;
    scroll_msg[0][i] |= (scroll_msg[1][i] >> 63);
    scroll_msg[1][i] = scroll_msg[1][i] << 1;
  }
  scroll_index++;
  if (scroll_index >= 64)
  {
    scroll_index = 0;
    scroll_frame_decode();
  }
}
void scroll_frame_decode()
{
  for (int i = 0; i < 8; i++)
  {
    scroll_msg[1][i] = 0;
  }
  int char_index[8] = {0};
  for (int i = 0; i < 8; i++)
  {
    if (msg[msg_index] == '\0')
    {
      msg_index = 0;
      pause = true;
      pause_ms = present_ms;
    }
    for (int j = 0; j < 89; j++)
    {
      if (msg[msg_index] == char_set[j])
      {
        char_index[i] = j;
        break;
      }
    }
    msg_index++;
  }

  for (int i = 7; i >= 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      temp = char_def[char_index[j] * 8 + i];
      scroll_msg[1][7 - i] |= (temp << (63 - 8 * (j + 1) + 1));
    }
  }
}
void led_on(int x, int y)
{
  uint64_t bitmask = 1;
  if (x < 0 || y < 0 || x >= 8 || y >= 64)
  {
    return;
  }
  led[x] = led[x] | (bitmask << (63 - y));
}

void led_off(int x, int y)
{
  uint64_t bitmask = 1;
  if (x < 0 || y < 0 || x >= 8 || y >= 64)
  {
    return;
  }
  led[x] = led[x] & (~(bitmask << (63 - y)));
}
void SPI_data_transfer(uint64_t data)
{
  //11011011..........01011100
  uint64_t temp = data;
  for (int i = 0; i < 8 ; i++)
  {
    temp = (data & 0xFF00000000000000) >> 56;
    SPI.transfer(temp);
    data = data << 8;
  }
}


void latch()
{
  //latches all the shift registers
  //latching 74HC595 shift registers
  PORTD |= 1 << latch_pin;
  PORTD &= ~(1 << latch_pin);
  //latching TLC59213 shift register
  PORTD |= 1 << src_latch;
  PORTD &= ~(1 << src_latch);

}

void frame_decode_7x5()
{
  int char_index[10] = {0};
}


void frame_decode()
{
  int char_index[8] = {0};
  
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 89; j++)
    {
      if (msg[i] == char_set[j])
      {
        char_index[i] = j;
        msg_index++;
        break;
      }
    }
  }

  for (int i = 7; i >= 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      temp = char_def[char_index[j] * 8 + i];
      led[7 - i] |= (temp << (63 - 8 * (j + 1) + 1));
    }
  }
}

//Interrupt service routines



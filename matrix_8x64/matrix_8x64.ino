/*
  Smart Displays : 8x64 LED Matrix
  Normal code:
        Multiplexing( No timer Interrypts )

  code by:
  K S S M KAMALK
  NIT SURATHKAL
  Dept of ECE
*/

#include <SPI.h>
#include "fonts.h"
#include <avr/pgmspace.h>

#define clock_pin 13  //clock pin for 74HC595
#define latch_pin 2   //latch pin for 74HC595
#define blank_pin 3   //blank pin for 74HC595
#define data_pin 11   //data pin for 74HC595
#define src_latch 4   //clock pin for TLC59213



unsigned long pause_time = 3000000;
unsigned long scroll_speed = 60000;

unsigned long present_ms = 0;
unsigned long last_ms = 0;
unsigned long pause_ms = 0;

uint8_t fade_column = 0, scroll_index = 0;
boolean flag = 0, pause;
boolean scroll_direction = 0;  //0 - scroll left and 1 - scroll right

uint64_t bitmask = 1, temp, led[8] = {0}; //led array with each element consisting 64 bits which corresponds to row data in multiplexing

uint64_t scroll_msg[2][8];   //led array with real and virtual matrix data
unsigned int msg_index = 0;
boolean font_sel = 1; // 0 for 8x8 font, 1 for 7x5 font
const byte char_set[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!#$%&'()*+-,.~:;<>?@[]/\\=_";

String msg = "VENKY LOVES HARRY ";

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
  //frame_decode_7x5();
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
        if (Serial.available() > 0)
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
  // scroll_multiplex();
  multiplex(scroll_msg[0]);

}

void multiplex(uint64_t arr[])
{
  uint8_t row_sel = 128;
  for (int i = 0; i < 8; i++)
  {
    // SPI_data_transfer(0);
    //SPI.transfer(0);
    //latch();
    SPI_data_transfer(arr[i]);
    SPI.transfer(row_sel);
    row_sel = row_sel >> 1;
    latch();
  }
}

void msg_check()
{
  if (msg.length() % 8 != 0)
    msg += "             ";
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
  if (font_sel == 0)
  {
    scroll_frame_decode();
  }
  else {
    frame_decode_7x5();
  }
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
    if (font_sel == 0)
    {
      scroll_frame_decode();
    }
    else {
      frame_decode_7x5();
    }
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
      temp = pgm_read_byte_near(char_def + (8 * char_index[j] + i));
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
      temp = pgm_read_byte_near(char_def + (8 * char_index[j] + i));
      led[7 - i] |= (temp << (63 - 8 * (j + 1) + 1));
    }
  }
}

//Interrupt service routines
void frame_decode_7x5()
{
  for (int i = 0; i < 8; i++)
  {
    scroll_msg[1][i] = 0;
  }
  uint8_t num_col = 0, max_char = 0, char_index_7x5[21] = {0};
  for (int i = 0; i < 21; i++)
  {
    if (msg[msg_index] == '\0')
    {
      msg_index = 0;
      pause = true;
      pause_ms = present_ms;
    }
    for (uint8_t j = 0; j < 96; j++)
    {
      if (msg[msg_index] == j + 32)
      {
        char_index_7x5[i] = j;
        num_col += pgm_read_byte_near(char_def_7x5 + (8 * j + 7));
        max_char++;
        break;
      }
    }
    msg_index++;
    if (num_col >= 64)
    {
      break;
    }
  }


  scroll_msg[1][7] = 0;
  for (int i = 6; i >= 0; i--)
  {
    uint16_t difference =  63;
    for (int j = 0; j < max_char; j++)
    {

      uint8_t shift_num = pgm_read_byte_near(char_def_7x5 + (8 * char_index_7x5[j] + 7));
      temp = pgm_read_byte_near(char_def_7x5 + (8 * char_index_7x5[j] + i));
      if (difference < 8)
        scroll_msg[1][6 - i] |= (temp >> (8 - shift_num));
      else
        scroll_msg[1][6 - i] |= temp << (difference - 8 + 1);
      difference -= shift_num;
    }
  }
}




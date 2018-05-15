#include <SPI.h>

#define clock_pin 13
#define data_pin 11
#define latch_pin 2
#define clk_pin 3
void setup() {
SPI.setBitOrder(MSBFIRST);
SPI.setDataMode(SPI_MODE0);
SPI.setClockDivider(SPI_CLOCK_DIV2);
pinMode(latch_pin, OUTPUT);
pinMode(data_pin, OUTPUT);
pinMode(clock_pin, OUTPUT);
SPI.begin();
}

void loop() {

SPI.transfer(0b0100001);
  PORTD |= 1 << latch_pin;
  PORTD &= 0 << latch_pin;
  PORTD |= 1 << latch_pin;
}

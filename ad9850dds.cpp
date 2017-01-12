
#include <Arduino.h>
#include "ad9850dds.h"

Ad9850dds::Ad9850dds(){
    /*
       Setup DDS pins and then some defaults
  */
  ddsFq_udPin = FQ_UD;
  ddsSdatPin  = SDAT;
  ddsSclkPin = SCLK;
  ddsResetPin = RESET;
}

void Ad9850dds::startDds() {

  // Configiure DDS control pins for digital output

  pinMode(ddsFq_udPin, OUTPUT);
  pinMode(ddsSdatPin, OUTPUT);
  pinMode(ddsSclkPin, OUTPUT);
  pinMode(ddsResetPin, OUTPUT);

  togglePin(ddsResetPin);
  togglePin(ddsSclkPin);
  togglePin(ddsFq_udPin);

}


// Subroutine to generate a positive pulse on 'pin'...
void Ad9850dds::togglePin(int pin) 
{
digitalWrite(pin, HIGH);
digitalWrite(pin, LOW); 
}

void Ad9850dds::send_byte(byte data_to_send)
{
  // Bit bang the byte over the SPI bus
  for (int i = 0; i < 8; i++, data_to_send >>= 1) {
    // Set Data bit on output pin
    digitalWrite(ddsSdatPin, data_to_send & 0x01);
    // Strobe the clock pin
    togglePin(ddsSclkPin);
  }
}
void Ad9850dds::setDDSFreq(long Freq_Hz)
{
  // Calculate the DDS word - from AD9850 Datasheet
  int32_t f = Freq_Hz * 4294967295 / 125000000;
  // Send one byte at a time
  for (int b = 0; b < 4; b++, f >>= 8) {
    //    SPI.transfer(f & 0xFF);
    send_byte(f & 0xFF);
  }
  // 5th byte needs to be zeros
  //SPI.transfer(0);
  send_byte(0);
  // Strobe the Update pin to tell DDS to use values
  togglePin(ddsFq_udPin);
}


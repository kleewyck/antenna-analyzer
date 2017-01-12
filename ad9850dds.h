/**
    Header for AntennaAnalyzer Class
*/
#ifndef ad9850dds_h
#define ad9850dds_h
/*
   Define location of DDS module
*/
#include <Arduino.h>

#define FQ_UD 4
#define SDAT 3
#define SCLK 5
#define RESET 2



class Ad9850dds
{
  public:
    Ad9850dds();
    // Ad9850dds(int sClk, int fwUd, int sData, int reset);
    void startDds();
    void setDDSFreq(long Freq_Hz);

    
  private:
    byte ddsFq_udPin;
    byte ddsSdatPin;
    byte ddsSclkPin;
    byte ddsResetPin; 
    void togglePin(int pin);
    void send_byte(byte data_to_send);
};
#endif

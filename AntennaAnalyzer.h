/**
    Header for AntennaAnalyzer Class
*/
#ifndef AntennaAnalyzer_h
#define AntennaAnalyzer_h

/*
   Define location of DDS module
*/
#include <Arduino.h>
#include "ad9850dds.h"

class AntennaAnalyzer
{
  private:
    long frequencyStart;
    long frequencyStop;
    int numberOfSteps;
    int forwardVoltage;
    int reverseVoltage;
    long frequencyStep;
    long currentStep;
    long minFreqency;
    long currentFrequency;
    double VSWR;
    double minVSWR = 999.0;
    boolean _activeScan;
    Ad9850dds dds;
    boolean firstTime;


  public:
    AntennaAnalyzer();
    void setFrequencyStart(const long value);
    void setFrequencyStop(const long  value);
    void setNumberOfSteps(const long value);
    long getFrequencyStart(void);
    long getFrequencyStop(void);
    int getNumberOfSteps(void);
    long getCurrentFrequency(void);
    long getMinFrequency(void);
    double getMinVswr(void);
    int getForwardVoltage(void);
    int getReverseVoltage(void);
    void reportStatus(void);
    void startScan(void);
    boolean moreData(char *outBuff);
    boolean activeScan(void);


};
#endif

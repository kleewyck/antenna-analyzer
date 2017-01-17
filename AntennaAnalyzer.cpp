/*
   Source for Antenna Analyzer Class
*/
#include <Arduino.h>
#include "ad9850dds.h"
#include "AntennaAnalyzer.h"

AntennaAnalyzer::AntennaAnalyzer()
{

  /*
     Default Sweep parms
  */
  frequencyStart = 3000000;
  frequencyStop = 40000000;
  numberOfSteps = 50;
  currentStep = 65535;
  _activeScan = false;


  dds.startDds();
  /*
    Set up analog stuff, need to parameterize these also
  */
  // Set up analog inputs on A0 and A1, internal reference voltage
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  analogReference(DEFAULT);
}


/*
   Class setters
*/
void AntennaAnalyzer::setFrequencyStart(const long value)
{
  frequencyStart = value;
}
void AntennaAnalyzer::setFrequencyStop(const long value) {
  frequencyStop = value;
}
void AntennaAnalyzer::setNumberOfSteps(const long value) {
  numberOfSteps = value;
}
/*
   Class Getters

*/
long AntennaAnalyzer::getFrequencyStart(void)
{
  return (frequencyStart);
}
long AntennaAnalyzer::getFrequencyStop(void)
{
  return (frequencyStop);
}
int AntennaAnalyzer::getNumberOfSteps(void)
{
  return (numberOfSteps);
}

boolean AntennaAnalyzer::activeScan(void)
{
  return (_activeScan);
}
long AntennaAnalyzer::getCurrentFrequency(void) {
  return currentFrequency;
}

int AntennaAnalyzer::getForwardVoltage(void) {
  return forwardVoltage;
}

int AntennaAnalyzer::getReverseVoltage(void) {
  return reverseVoltage;
}

long AntennaAnalyzer::getMinFrequency(void){
  return minFreqency;
}
double AntennaAnalyzer::getMinVswr(void){
  return minVSWR;
}

void AntennaAnalyzer::reportStatus(void)
{
  Serial.print("Starting Frequency : ");
  Serial.println(frequencyStart);
  Serial.print("Ending Frequency : ");
  Serial.println(frequencyStop);
  Serial.print("Number of steps : ");
  Serial.println(numberOfSteps);
}

void AntennaAnalyzer::startScan(void) {
  forwardVoltage = 0;
  reverseVoltage = 0;
  frequencyStep = (frequencyStop - frequencyStart) / numberOfSteps;
  currentStep = 0;
  minVSWR = 999.0;
  minFreqency = frequencyStart;
  firstTime = true;

  dds.setDDSFreq(frequencyStart);
  _activeScan = true;
}

boolean AntennaAnalyzer::moreData(char *outBuff) {
  boolean returnStatus = false;

  char strTmp[15];
  char tb[255];
  int  index = 0;

  if (currentStep <= numberOfSteps) {
    currentFrequency = ( long)(frequencyStart + currentStep * frequencyStep);

    dds.setDDSFreq(currentFrequency);
    delay(10);
    reverseVoltage = analogRead(A0);
    forwardVoltage = analogRead(A1);
    if (reverseVoltage >= forwardVoltage) {
      VSWR = 100.0;
    } else {
      VSWR = ((double)forwardVoltage + (double)reverseVoltage) / ((double)forwardVoltage - (double)reverseVoltage);
    }
    /**
     * Recorde the Minimum VSWR 
     */
    if (VSWR <= minVSWR)
    {
      minFreqency = currentFrequency;
      minVSWR = VSWR;
    }
    dtostrf(VSWR, 6, 2, strTmp);
    index = 0;
    if (firstTime) {
      outBuff[0] = '['; // Stuff the Start of Json into buff
      firstTime = false;
      index = 1;
    }
    sprintf(&outBuff[index],"{\"CF\":\"%lu\",\"VSWR\":\"%s\",\"FWDV\" :\"%i\",\"REVV\":\"%i\"}", currentFrequency,strTmp,forwardVoltage,reverseVoltage);
    currentStep++;
    if (currentStep > numberOfSteps) {
      strcat(outBuff, "]");
    } else {
      strcat(outBuff, ",");
    }
    returnStatus = true;
  } else {
    _activeScan = false;
  }
  return returnStatus;
}



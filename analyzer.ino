// Simple State Machine implementation for the Ham Radio 360
// Antennena Analyzer
// Author: Donald Gover
// Date: 12th Jan 2016
//  State Machine extended from code developed by Nick Gammon in his blog post
//  http://www.gammon.com.au/forum/?id=11425
//
#include <LiquidCrystal.h>
#include "Bounce2.h"
#include "AntennaAnalyzer.h"
//
// Define some Global Constants
//
#define MODEPIN A2            //Pin for Mode switch
#define BANDPIN A3            //Pin for Band Switch
#define MAXSWEAPSELECTION 11  //Max Number of preset Sweaps. 

// the possible states of the state-machine
typedef enum {  NONE, GOT_START, GOT_END, GOT_NUM, GOT_SCAN, GOT_FIXED, GOT_STATUSREPORT } states;
//
//  StandAlone Sweap Parms for each band.
//
typedef struct {
  char  descript[16];
  long  startFrequency;
  long  stopFrequency;
  long  numberOfSamples;
} bandParms;



bandParms bandParmsArray[MAXSWEAPSELECTION] = {{"1-30 MHz", 1000000, 30000000, 300},
  {"160m", 1800000, 2000000, 300},
  {"80m", 3500000, 4000000, 300},
  {"60m", 5000000, 6000000, 300},
  {"40m", 7000000, 7300000, 300},
  {"30m", 10100000, 10150000, 300},
  {"20m", 14000000, 14350000, 300},
  {"17m", 18068000, 18168000, 300},
  {"15m", 21000000, 21450000, 300},
  {"12m", 24890000, 24990000, 300},
  {"10m", 28000000, 29700000, 300}
};

// current state-machine state
states state = NONE;
// current partial number
long currentValue;
char outputBuffer[255];
char barGraph[17];
int currentSample;
int currentBand;
boolean endFlag;

/*
     Create Antenna Aanlyzer Class to start updating

*/
LiquidCrystal lcd(8, 9, 15, 14, 16, 10);
AntennaAnalyzer aa;
Bounce bandPinDebounce = Bounce();
Bounce modePinDebounce = Bounce();

void clearLCDLine(int row) {
  memset(barGraph, ' ', sizeof(barGraph));
  barGraph[sizeof(barGraph)] = 0x00;
  lcd.setCursor(0, row);
  lcd.print(barGraph);
}

void processFixedFreq (const long value)
{
  // do something with speed
  Serial.print ("Fixed Frequency = ");
  Serial.println (value);
} // end of processSpeed

void processDoScan (void)
{
  aa.startScan();
  clearLCDLine(0);
  clearLCDLine(1);
  lcd.setCursor(0, 0);
  lcd.print("Scan Started");
  currentSample = 0;
}


void handlePreviousState ()
{
  char buffer[16];
  memset(buffer, 0, sizeof(buffer));

  switch (state)
  {
    case GOT_START:
      aa.setFrequencyStart(currentValue);
      lcd.setCursor(0, 1);
      sprintf(buffer, "St Freq: %i", aa.getFrequencyStart());
      lcd.print(buffer);
      break;
    case GOT_END:
      aa.setFrequencyStop(currentValue);
      lcd.setCursor(0, 1);
      sprintf(buffer, "End Freq: %i", aa.getFrequencyStop());
      lcd.print(buffer);
      break;
    case GOT_SCAN:
      processDoScan();
      break;
    case GOT_FIXED:
      processFixedFreq(currentValue);
      break;
    case GOT_NUM:
      aa.setNumberOfSteps(currentValue);
      lcd.setCursor(0, 1);
      sprintf(buffer, "Num Step: %i", aa.getNumberOfSteps());
      lcd.print(buffer);
      break;
    case GOT_STATUSREPORT:
      aa.reportStatus();
      break;
  }  // end of switch

  currentValue = 0;
}  // end of handlePreviousState

void processIncomingByte (const byte c)
{
  if (isdigit (c))
  {
    currentValue *= 10;
    currentValue += c - '0';
  }  // end of digit
  else
  {

    // The end of the number signals a state change
    handlePreviousState ();

    // set the new state, if we recognize it
    switch (c)
    {
      case 'A':
        state = GOT_START;
        break;
      case 'B':
        state = GOT_END;
        break;
      case 'C':
        state = GOT_FIXED;
        break;
      case 'N':
        state = GOT_NUM;
        break;
      case 'S':
      case 's':
        state = GOT_SCAN;
        break;
      case '?':
        state = GOT_STATUSREPORT;
        break;
      default:
        state = NONE;
        break;
    }  // end of switch on incoming byte

  } // end of not digit

} // end of processIncomingByte

/*
   Setup the Board to get going here.
*/
void setup ()
{
  //
  // Set up Initial State
  //
  Serial.begin (115200);
  state = NONE;
  endFlag = false;
  currentBand = 0;
  pinMode(BANDPIN, INPUT);
  pinMode(MODEPIN, INPUT);
  bandPinDebounce.attach(BANDPIN);
  bandPinDebounce.interval(50);
  modePinDebounce.attach(MODEPIN);
  modePinDebounce.interval(50);
  //
  // Set Current Sweap Parms based on Mode in the current Analyzer Object
  //
  aa.setFrequencyStart(bandParmsArray[currentBand].startFrequency);
  aa.setFrequencyStop(bandParmsArray[currentBand].stopFrequency);
  aa.setNumberOfSteps(bandParmsArray[currentBand].numberOfSamples);
  /*
     Start LCD and Display initial Message
  */
  lcd.begin(16, 2);
  lcd.print("Antenna DLG");
  lcd.setCursor(0, 1);
  lcd.print(bandParmsArray[currentBand].descript);

}  // end of setup

void loop ()
{
  int bandPinValue = HIGH;  // Since there is a default Pullup status is inverse
  int modePinValue = HIGH;  // LOW indicates the button is pressed.
  //
  // Update Debouncer state
  //
  if (bandPinDebounce.update()) {
    bandPinValue = bandPinDebounce.read();
  }
  if (modePinDebounce.update()) {
    modePinValue = modePinDebounce.read();
  }

  //
  // Is there a character to read?
  //
  while (Serial.available ()) {
    processIncomingByte (Serial.read ());
  }
  //
  // Handle Buttons here and figure out what to do
  //

  if (bandPinValue == LOW && !aa.activeScan()) {  // Was the Band Pin Pressed? Move to next Band and update display
    currentBand++;
    if (currentBand >= MAXSWEAPSELECTION) {  //Make sure we wrap around
      currentBand = 0;
    }
    clearLCDLine(1);
    lcd.setCursor(0, 1);
    lcd.print(bandParmsArray[currentBand].descript);
  }
  if (modePinValue == LOW && !aa.activeScan()) {  // Going to use Mode Pin for the moment to start Scan...
    //
    // Set Sweap Parms based on Mode in the current Analyzer Object
    // Then start the scan!
    //
    aa.setFrequencyStart(bandParmsArray[currentBand].startFrequency);
    aa.setFrequencyStop(bandParmsArray[currentBand].stopFrequency);
    aa.setNumberOfSteps(bandParmsArray[currentBand].numberOfSamples);
    // Go Go Gadet
    processDoScan();
  }

  if (aa.moreData(outputBuffer) && aa.activeScan()) {
    Serial.print(outputBuffer);
    currentSample++;
    int barGraphSize = map(currentSample, 0, aa.getNumberOfSteps(), 0, 15);
    for (int x = 0; x < barGraphSize; x++)
      barGraph[x] = 'X';
    lcd.setCursor(0, 1);
    lcd.print(barGraph);
    endFlag = true;
  } else {
    if (endFlag) {
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(aa.getMinFrequency());
      lcd.print(",");
      lcd.print(aa.getMinVswr());
      lcd.print(":1    ");
      endFlag = false;
    }
  }
}  // end of loop

// Simple State Machine implementation for the Ham Radio 360
// Antennena Analyzer
// Author: Donald Gover
// Date: 12th Jan 2016
//  State Machine extended from code developed by Nick Gammon in his blog post
//  http://www.gammon.com.au/forum/?id=11425
//
#include <LiquidCrystal.h>
#include "AntennaAnalyzer.h"

// the possible states of the state-machine
typedef enum {  NONE, GOT_START, GOT_END, GOT_NUM, GOT_SCAN, GOT_FIXED, GOT_STATUSREPORT } states;

// current state-machine state
states state = NONE;
// current partial number
long currentValue;
char outputBuffer[255];
char barGraph[17];
int currentSample;
boolean endFlag;

/*
     Create Antenna Aanlyzer Class to start updating

*/
LiquidCrystal lcd(8, 9, 15, 14, 16, 10);
AntennaAnalyzer aa;


void processFixedFreq (const long value)
{
  // do something with speed
  Serial.print ("Fixed Frequency = ");
  Serial.println (value);
} // end of processSpeed

void processDoScan (const unsigned int value)
{
  // do something with gear
  Serial.println ("Scan Starting");
} // end of processGear


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
      aa.startScan();
      lcd.setCursor(0, 1);
      lcd.print("Scan Started");
      memcpy(barGraph, " ", sizeof(barGraph));
      barGraph[sizeof(barGraph)] = 0x00;
      currentSample = 0;
      lcd.setCursor(0, 1);
      lcd.print(barGraph);
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
  Serial.begin (115200);
  state = NONE;
  endFlag = false;

  /*
     Start LCD and Display initial Message
  */
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Antenna DLG");
  lcd.setCursor(0, 1);
  lcd.print("1-30 MHz");

}  // end of setup

void loop ()
{

  while (Serial.available ()) {
    processIncomingByte (Serial.read ());
  }


  if (aa.moreData(outputBuffer) && aa.activeScan()) {
    Serial.println(outputBuffer);
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
      endFlag =false;
    }
  }
}  // end of loop


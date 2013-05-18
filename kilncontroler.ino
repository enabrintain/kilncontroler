/******************************************************************* 
 This Code liberally uses examples found online and as such is
 entirely open. I'm sticking with Limor's BSD license.
 Phil Showers
 
 ******************************************************************* 
  This is an example for the Adafruit Thermocouple Sensor w/MAX31855K

  Designed specifically to work with the Adafruit Thermocouple Sensor
  ----> https://www.adafruit.com/products/269

  These displays use SPI to communicate, 3 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 
 *******************************************************************  
  The PID code is from http://playground.arduino.cc/Code/PIDLibrary
 ******************************************************************* 
  The SD Card stuff is from the SD card example in the arduino 
  example library
 ******************************************************************* 
  The double to string code is from
  https://gist.github.com/jmccrohan/2343665
 *******************************************************************/

#include "Adafruit_MAX31855.h"
#include "KilnRun.h" 
#include <LiquidCrystal.h>
#include <PID_v1.h>
#include <SPI.h>
#include <SD.h>
#include <Time.h>
#include <TimeAlarms.h>


const int RELAY = 0; // the pin for turning on and off the heating coils

const int SD_SCK = 13; // sd card
const int SD_MISO = 12; // sd card
const int SD_MOSI = 11; // sd card
const int SD_CS = 10; // sd card

const int THERM_DO = 1; // (data out) is an output from the MAX31855 (input to the microcontroller) which carries each bit of data
const int THERM_CS = 2; // (chip select) is an input to the MAX31855 (output from the microcontroller) which tells the chip when its time to read the thermocouple and output more data.
const int THERM_CLK = 3; // (clock) is an input to the MAX31855 (output from microcontroller) which indicates when to present another bit of data

const int CANDLE = 14; // candle (raise kiln to 200F and hold it there for 2, 4, 6, 8, or 12 hours before firing - cooks out all the water
const int CONE = 15; // choose kiln temp
const int HOLD = 16; // hold time at target temp, in hrs
const int SPEED = 17; // choose the speed/mode program 
const int START= 18; // starts the kiln heating program, CONE, HOLD, and SPEED are no longer read after starting, CLEAR will reset this.
const int CLEAR = 19; // pushing this resets things

//
KilnRun thisRun;

// Initialize the Thermocouple
Adafruit_MAX31855 thermocouple(THERM_CLK, THERM_CS, THERM_DO);
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);

//Define PID Variables we'll be connecting to
double Setpoint, Input, Output;
int WindowSize = 5000; 
unsigned long windowStartTime;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint,2,5,1, DIRECT);

//These are internal variables to the parseButtons
int lastCANDLE = HIGH;
int lastCONE = HIGH;
int lastHOLD = HIGH;
int lastSPEED = HIGH;
int lastSTART = HIGH;
int lastCLEAR = HIGH;

bool sdCardInited = false;

/*******************************
 * SETUP
 ********************************/
void setup() {
    
  /********** BUTTON SETUP **********/
  pinMode(CANDLE, INPUT); // initialize the button pin as a input
  digitalWrite(CANDLE, HIGH);       // turn on pullup resistor, *pull-up configuration — when the button is not pressed, the Arduino will sense high voltage. 
  pinMode(CONE, INPUT); // initialize the button pin as a input
  digitalWrite(CONE, HIGH);       // turn on pullup resistor
  pinMode(HOLD, INPUT); // initialize the button pin as a input
  digitalWrite(HOLD, HIGH);       // turn on pullup resistor
  pinMode(SPEED, INPUT); // initialize the button pin as a input
  digitalWrite(SPEED, HIGH);       // turn on pullup resistor
  pinMode(START, INPUT); // initialize the button pin as a input
  digitalWrite(START, HIGH);       // turn on pullup resistor
  pinMode(CLEAR, INPUT); // initialize the button pin as a input
  digitalWrite(CLEAR, HIGH);       // turn on pullup resistor
  
  // initialize the LED as an output
  pinMode(RELAY, OUTPUT); // initialize the relay pin as output
  digitalWrite(RELAY,LOW);
  
  
  /********** LCD SETUP **********/
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
   lcd.setCursor(0, 0);
  lcd.print("Stabilizing ");
   lcd.setCursor(0, 1);
  lcd.print(" Probe");
  // wait for MAX chip to stabilize
  delay(1500);
  
  
  /********** SD SETUP **********/
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(SD_CS, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CS)) {
    //Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  else
  {
   lcd.setCursor(0, 0);
  lcd.print("SD Init ");
   lcd.setCursor(0, 1);
  lcd.print(" Del Log");
  delay(1500);
    sdCardInited = true;
    SD.remove("kiln_log.txt");
    writeToSD("setup,buttons");
    writeToSD("setup,lcd");
    writeToSD("setup,sd card");
  }
  
  /********** PID Setup **********/
  //initialize the variables we're linked to
   lcd.setCursor(0, 0);
  lcd.print("PID Init ");
   lcd.setCursor(0, 1);
  lcd.print("");
  delay(1500);
  
  Input = analogRead(0);
  windowStartTime = millis();
  Setpoint = 100;
  //tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);
  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  writeToSD("setup,pid");
}// setup

void writeToSD(String msg)
{
  if(!sdCardInited)
    return;
  
  float floatvar = millis();
  char dtostrfbuffer[15];  
  dtostrf(floatvar,8, 2, dtostrfbuffer);
  String time = String(dtostrfbuffer);
    
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("kiln_log.csv", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(time + "," + msg);
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    //Serial.println("error opening datalog.txt");
  } 
}// writeToSD

/********************************
 * MAIN LOOP
 ********************************/
void loop()
{
   //clickRelay();
   
  
   /********** THERMOCOUPLE LOOP **********/
   double temperature = thermocouple.readFarenheit();
   
   writeToSD("loop,temp:,"+doubleToString(temperature));
   
   
   /********** LCD LOOP **********/
   // basic readout test, just print the current temp
   lcd.setCursor(0, 0);
   lcd.print("Int. Temp = ");
   lcd.println(thermocouple.readInternalF());
   lcd.print("  "); 
   lcd.setCursor(0, 1);
   if (isnan(temperature)) 
   {
     lcd.print("T/C Problem");
   } 
   else 
   {
     lcd.print("F = "); 
     lcd.print(temperature);
     lcd.print("  "); 
   }//*/
   
   
   /********** INPUT BUTTON LOOP **********/
   parseButtons();
   
   
   /********** PID LOOP **********/
   if(thisRun.isStarted())
     kiln(temperature);
     
     
   /********** DELAY 30 SEC **********/
   delay(3000);              // wait for 30 seconds
}// loop




/********************************
 * Called from MAIN LOOP
 ********************************/
 
 /**
  * This fn handles the task on turning on and off the kiln relay
  * TODO: add kiln.getTargetTemp(currentTemperature); somewhere...
  */
 void kiln(double currentTemperature)
 {
   Input = currentTemperature;
   myPID.Compute();

  /************************************************
   * turn the output pin on/off based on pid output
   ************************************************/
  if(millis() - windowStartTime>WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if(Output < millis() - windowStartTime) digitalWrite(RELAY,HIGH);
  else digitalWrite(RELAY,LOW);

  /*****************
  Setpoint = temperature;
  Input = temperature;
  Compute();
  analogWrite(3,Output);
  digitalWrite(relay, HIGH);   	// turn the relay on
  //  */
 }// kiln
 
void clickRelay()
{
  //Serial.println("low");
  digitalWrite(RELAY,LOW);
     
   /********** DELAY 30 SEC **********/
   delay(1000);              // wait for 30 seconds
  
  //Serial.println("high");
  digitalWrite(RELAY,HIGH);
     
   /********** DELAY 30 SEC **********/
   delay(1000);              // wait for 30 seconds
   //*/
}// clickRelay
 
int parseButtons()
{
   int buttonState = 0;         // current state of the button  
  
   buttonState = digitalRead(CANDLE); // read the pushbutton input pin   
   //   *pull-up configuration — when the button is not pressed, the Arduino will sense high voltage.
   if (buttonState != lastCANDLE) // compare the buttonState to its previous state
   {
     if (buttonState == HIGH) // if the state has changed, and the state is HIGH meaning the button is not pressed, perform the button's ev0ent
     {
       thisRun.candlePressed();
       writeToSD("parseButtons,candlePressed");
       //clickRelay(); // click the relay when the button is pressed
     }// button released
     else
     {
       //button state is LOW, meaning that the button is currently being pressed, when it is released the change will trigger the event
     }
   }//button state changed
   lastCANDLE = buttonState;// save the current state as the last state
   
   buttonState = digitalRead(CONE); // read the pushbutton input pin   
   //   *pull-up configuration — when the button is not pressed, the Arduino will sense high voltage.
   if (buttonState != lastCONE) // compare the buttonState to its previous state
   {
     if (buttonState == HIGH) // if the state has changed, and the state is HIGH meaning the button is not pressed, perform the button's event
     {
       thisRun.conePressed();
       writeToSD("parseButtons,conePressed");
       //clickRelay(); // click the relay when the button is pressed
     }// button released
     else
     {
       //button state is LOW, meaning that the button is currently being pressed, when it is released the change will trigger the event
     }
   }//button state changed
   lastCONE = buttonState;// save the current state as the last state
   
   buttonState = digitalRead(HOLD); // read the pushbutton input pin   
   //   *pull-up configuration — when the button is not pressed, the Arduino will sense high voltage.
   if (buttonState != lastHOLD) // compare the buttonState to its previous state
   {
     if (buttonState == HIGH) // if the state has changed, and the state is HIGH meaning the button is not pressed, perform the button's event
     {
       thisRun.holdPressed();
       writeToSD("parseButtons,holdPressed");
       //clickRelay(); // click the relay when the button is pressed
     }// button released
     else
     {
       //button state is LOW, meaning that the button is currently being pressed, when it is released the change will trigger the event
     }
   }//button state changed
   lastHOLD = buttonState;// save the current state as the last state
   
   buttonState = digitalRead(SPEED); // read the pushbutton input pin   
   //   *pull-up configuration — when the button is not pressed, the Arduino will sense high voltage.
   if (buttonState != lastSPEED) // compare the buttonState to its previous state
   {
     if (buttonState == HIGH) // if the state has changed, and the state is HIGH meaning the button is not pressed, perform the button's event
     {
       thisRun.speedPressed();
       writeToSD("parseButtons,speedPressed");
       //clickRelay(); // click the relay when the button is pressed
     }// button released
     else
     {
       //button state is LOW, meaning that the button is currently being pressed, when it is released the change will trigger the event
     }
   }//button state changed
   lastSPEED = buttonState;// save the current state as the last state
   
   buttonState = digitalRead(START); // read the pushbutton input pin   
   //   *pull-up configuration — when the button is not pressed, the Arduino will sense high voltage.
   if (buttonState != lastSTART) // compare the buttonState to its previous state
   {
     if (buttonState == HIGH) // if the state has changed, and the state is HIGH meaning the button is not pressed, perform the button's event
     {
       thisRun.startPressed();
       writeToSD("parseButtons,startPressed");
       //clickRelay(); // click the relay when the button is pressed
     }// button released
     else
     {
       //button state is LOW, meaning that the button is currently being pressed, when it is released the change will trigger the event
     }
   }//button state changed
   lastSTART = buttonState;// save the current state as the last state
   
   buttonState = digitalRead(CLEAR); // read the pushbutton input pin   
   //   *pull-up configuration — when the button is not pressed, the Arduino will sense high voltage.
   if (buttonState != lastCLEAR) // compare the buttonState to its previous state
   {
     if (buttonState == HIGH) // if the state has changed, and the state is HIGH meaning the button is not pressed, perform the button's event
     {
       thisRun.clearPressed();
       writeToSD("parseButtons,clearPressed");
       //clickRelay(); // click the relay when the button is pressed
     }// button released
     else
     {
       //button state is LOW, meaning that the button is currently being pressed, when it is released the change will trigger the event
     }
   }//button state changed
   lastCLEAR = buttonState;// save the current state as the last state
  return buttonState;
}// parseButton


String doubleToString(double input){
  
  float floatvar = millis();
  char dtostrfbuffer[15];  
  dtostrf(input,8, 1, dtostrfbuffer);
  
  return String(dtostrfbuffer);
  
}//*/



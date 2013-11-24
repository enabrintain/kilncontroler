/******************************************************************* 
 This Code liberally uses examples found online and as such is
 entirely open. I'm sticking with Limor's BSD license.
 Phil Showers (freenode:EnabrinTain)
 https://256.makerslocal.org/wiki/User:Enabrintain
 *******************************************************************/

#include "Adafruit_MAX31855.h"

#include <SPI.h>
#include <SD.h>

/**************************************************************************************************************************************************************************************************
 * Thermocouple Pins
 **************************************************************************************************************************************************************************************************/
const int THERM_DO_1 = A2; // (data out) is an output from the MAX31855 (input to the microcontroller) which carries each bit of data
const int THERM_CS_1 = 2; // (chip select) is an input to the MAX31855 (output from the microcontroller) which tells the chip when its time to read the thermocouple and output more data.
const int THERM_CLK_1 = 3; // (clock) is an input to the MAX31855 (output from microcontroller) which indicates when to present another bit of data

const int THERM_DO_2 = A3; // (data out) is an output from the MAX31855 (input to the microcontroller) which carries each bit of data
const int THERM_CS_2 = A4; // (chip select) is an input to the MAX31855 (output from the microcontroller) which tells the chip when its time to read the thermocouple and output more data.
const int THERM_CLK_2 = A5; // (clock) is an input to the MAX31855 (output from microcontroller) which indicates when to present another bit of data


/***********************************************************
 * SD Card IO Pins
 ***********************************************************/
//const int SCK =  13; // These are set by the system.
//const int MISO = 12; // These are set by the system.
//const int MOSI = 11; // These are set by the system.
const int CS = 10; // sd card


/***************************************************************************
 * Initialize the Thermocouple
 ***************************************************************************/
Adafruit_MAX31855 coldThermocouple(THERM_CLK_2, THERM_CS_2, THERM_DO_2);
Adafruit_MAX31855 hotThermocouple(THERM_CLK_1, THERM_CS_1, THERM_DO_1);


/*********************************
 * SD Card Init variable
 *********************************/
bool sdCardInited = false;


/**********************************************************************************************************************
 * SETUP STAGE 
 **********************************************************************************************************************/
void setup() {
  Serial.begin(9600);
  
  /********** SD SETUP **********/
  // make sure that the default chip select pin is set to output, even if you don't use it:
  pinMode(CS, OUTPUT);
  initSD();
  
  // give the MAX31855 chips a chance to stabilize
  delay(1500);
}// setup


/***********************************************************
 * Initialize the SD card logging
 ***********************************************************/
void initSD()
{
  // see if the card is present and can be initialized:
  if (!SD.begin(CS)) {
    Serial.println("Card failed, or not present");
    return;
  }
  else
  {
    sdCardInited = true;
    SD.remove("kiln_log.csv");
    
    logToSD("initSD");
    errToSD("initSD");
  }
}// initSD


/***********************************************************************************************************************************************************************************
 * MAIN LOOP STAGE: DO NOT PUT DELAYS IN THIS AS THEY WILL
 * BOOGER UP BUTTON DEBOUNCING.
 ***********************************************************************************************************************************************************************************/
void loop() {
  double temperature = readTemp(0);
}// loop


/***********************************************************************************************************************************************************************************
 * Reads the thermocouple boards (AdaFruit rocks!) and sums them together. cause kilns get hot...
 * the summing is necessary because physics. Thermocouples work by measuring the voltage diferential between wires running from the hot end to the cold end
 * When I switch to copper wire at the cold end, that causes another differential. so i'm ONLY measuring from the length of the thermocouple,
 * I could fix it by using the proper wire that is made of the same materials as the thermocouple, mechanically connected,
 * OR I could do it the more fun way and measure the temperature of the cold end (it gets up to 700F at Cone 8) and sum it with the hot end.
 * Since the MAX31855 has a thermister that it checks for ambient temp, that needs to be added back or else we're subtracting it twice:
 *
 * T(subH) = T(sub1) - T(subAMB)
 * T(subC) = T(sub2) - T(subAMB)
 * T = T(sub1) + T(sub2) - T(subAMB)
     = T(sub1) + ( T(sub2) - T(subAMB) )
     = T(sub1) + ( T(subC) ) 
     = ( T(subH) + T(subAMB) ) + T(subC)
 *
 * T = T(subH) + T(subC) + T(subAMB)
 ***********************************************************************************************************************************************************************************/
double readTemp(int tryCount)
{
  if(tryCount>10)
  {
    return -1;
  }
  
  if(tryCount>2)
  {
    //try resetting the chips
    coldThermocouple = Adafruit_MAX31855(THERM_CLK_2, THERM_CS_2, THERM_DO_2);
    hotThermocouple = Adafruit_MAX31855(THERM_CLK_1, THERM_CS_1, THERM_DO_1);    
    delay(2500);// wait for MAX31855 chip to stabilize
  }
  
  double hotTemp = hotThermocouple.readFarenheit(); // hot end of kiln thermocouple
  double coldTemp = coldThermocouple.readFarenheit(); // 'cold' end of thermocouple
  double ambientTemp = hotThermocouple.readInternalF(); // ambient temperature (we assume AMB is the same for both chips)
  double temperature = hotTemp + coldTemp + ambientTemp; // only add the temperature difference between the 'cold' end and the ambient temp.
  
  logToSD("readTemp,temp:,"+doubleToString(temperature)+",ambientTemp,"+
      doubleToString(ambientTemp)+",hotTemp,"+
      doubleToString(hotTemp)+",coldTemp,"+
      doubleToString(coldTemp)+",tryCount,"+
      doubleToString(tryCount));
  Serial.println("loop,temp:,"+doubleToString(temperature)+",ambientTemp,"+
      doubleToString(ambientTemp)+",hotTemp,"+
      doubleToString(hotTemp)+",coldTemp,"+
      doubleToString(coldTemp)+",tryCount,"+
      doubleToString(tryCount));
      
  if(isnan(temperature))
  {
    int err = hotThermocouple.readError();
    errToSD("readTemp,ERR," + doubleToString(err)); // extra info...
    errToSD("readTemp,temp:,"+doubleToString(temperature)+",ambientTemp,"+
      doubleToString(ambientTemp)+",hotTemp,"+
      doubleToString(hotTemp)+",coldTemp,"+
      doubleToString(coldTemp)+",tryCount,"+
      doubleToString(tryCount));
    Serial.println("ERR: " + doubleToString(err));
    temperature = readTemp(tryCount+1);
  }// if NAN, errlog...
  
  return temperature;
}// readTemp

/****************************************************
*   The double to string code is from
*  https://gist.github.com/jmccrohan/2343665
*****************************************************/
String doubleToString(double input)
{
  char dtostrfbuffer[15];  
  dtostrf(input,8, 1, dtostrfbuffer);  
  return String(dtostrfbuffer);
}// doubleToString


/***********************************************************
 * logToSD calls writeToSD, passing kiln_log.csv in as the
 * target fileName.
 ***********************************************************/
void logToSD(String msg)
{
  if(!sdCardInited)
    return;
    
  String fileName = "kiln_log.csv";
  writeToSD(fileName, msg);
}// logToSD


/***********************************************************
 * errToSD calls writeToSD, passing kiln_err.csv in as the
 * target fileName.
 ***********************************************************/
void errToSD(String msg)
{
  if(!sdCardInited)
    return;
    
  String fileName = "kiln_err.csv";
  writeToSD(fileName, msg);
}// logToSD


/***********************************************************************************************************************************************************************
 * SD card logging, opens the file and appends msg to it.
 ***********************************************************************************************************************************************************************/
void writeToSD(const String fileName, String msg)
{
  if(!sdCardInited)
    return;
  
  String time = doubleToString(millis());
  
  //string to char* conversion courtesy of http://stackoverflow.com/questions/11610264/convert-string-to-type-const-char-using-arduino
  char __fileName[sizeof(fileName)];
  fileName.toCharArray(__fileName, sizeof(__fileName));
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(__fileName, FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(time + "," + msg);
    dataFile.close();
  }// if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
}// writeToSD


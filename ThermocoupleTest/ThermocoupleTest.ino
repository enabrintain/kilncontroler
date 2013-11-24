
#include "Adafruit_MAX31855.h"



const int THERM_DO_1 = A2; // (data out) is an output from the MAX31855 (input to the microcontroller) which carries each bit of data
const int THERM_CS_1 = 2; // (chip select) is an input to the MAX31855 (output from the microcontroller) which tells the chip when its time to read the thermocouple and output more data.
const int THERM_CLK_1 = 3; // (clock) is an input to the MAX31855 (output from microcontroller) which indicates when to present another bit of data

const int THERM_DO_2 = A3; // (data out) is an output from the MAX31855 (input to the microcontroller) which carries each bit of data
const int THERM_CS_2 = A4; // (chip select) is an input to the MAX31855 (output from the microcontroller) which tells the chip when its time to read the thermocouple and output more data.
const int THERM_CLK_2 = A5; // (clock) is an input to the MAX31855 (output from microcontroller) which indicates when to present another bit of data



// Initialize the Thermocouple
Adafruit_MAX31855 coldThermocouple(THERM_CLK_2, THERM_CS_2, THERM_DO_2);
Adafruit_MAX31855 hotThermocouple(THERM_CLK_1, THERM_CS_1, THERM_DO_1);


void setup() {
  Serial.begin(9600);
}// setup

void loop() {
  // put your main code here, to run repeatedly: 
  double temperature = readTemp(0);
}


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
    // wait for MAX chip to stabilize
    delay(1500);
  }
  
  double hotTemp = hotThermocouple.readFarenheit(); // hot end of kiln thermocouple
  double coldTemp = coldThermocouple.readFarenheit(); // 'cold' end of thermocouple
  double ambientTemp = hotThermocouple.readInternalF(); // ambient temperature (of the chip that matters)
  double temperature = hotTemp + coldTemp - ambientTemp; // only add the temperature difference between the 'cold' end and the ambient temp.
  
  /*writeToSD("loop,temp:,"+doubleToString(temperature)+","+
      doubleToString(ambientTemp)+","+
      doubleToString(hotTemp)+","+
      doubleToString(coldTemp)+", "+
      doubleToString(tryCount));//*/
  Serial.println("loop,temp:,"+doubleToString(temperature)+",ambientTemp,"+
      doubleToString(ambientTemp)+",hotTemp,"+
      doubleToString(hotTemp)+",coldTemp,"+
      doubleToString(coldTemp)+",tryCount,"+
      doubleToString(tryCount));
      
  if(isnan(temperature))
  {
    int err = hotThermocouple.readError();
    Serial.println("ERR: " + doubleToString(err));
    delay(3000); //3sec delay
    temperature = readTemp(tryCount+1);
  }
  
  if(temperature==-1)
  {
    Serial.println("Temp Failed");
    delay(500);              // wait for 1/2 second
  }
  else
  {
    Serial.println(doubleToString(temperature)+"F     ");
    delay(500);              // wait for 1/2 second
  }
  
  return temperature;
}// readTemp


String doubleToString(double input)
{
  char dtostrfbuffer[15];  
  dtostrf(input,8, 1, dtostrfbuffer);  
  return String(dtostrfbuffer);
}// doubleToString

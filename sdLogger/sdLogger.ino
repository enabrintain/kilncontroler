

#include <SPI.h>
#include <SD.h>


//const int SCK = 13; // sd card
//const int MISO = 12; // sd card
//const int MOSI = 11; // sd card
const int CS = 10; // sd card


bool sdCardInited = false;

void setup() {
  Serial.begin(9600);
  
  /********** SD SETUP **********/
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(CS, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(CS)) {
    Serial.println("Card failed, or not present");
    return;
  }
  else
  {
    sdCardInited = true;
    SD.remove("kiln_log.csv");
    
    writeToSD("setup,buttons");
    writeToSD("setup,lcd");
    writeToSD("setup,sd card");
  }

}

void loop() {
  
  Serial.print("DERP");
  delay(1000);
  writeToSD("DERP");
}


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
  }// if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
}// writeToSD

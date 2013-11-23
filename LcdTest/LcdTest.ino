
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);

void setup() {

  /********** LCD SETUP **********/
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Stabilizing ");
   lcd.setCursor(0, 1);
  lcd.print(" Probe");
  delay(1000);
}

void clearLCD()
{
  lcd.setCursor(0, 0);
  lcd.print("            ");
  lcd.setCursor(0, 1);
  lcd.print("            ");
}// clearLCD

void loop() {
  clearLCD(); lcd.setCursor(0, 0); lcd.print("WORDS");
  
  delay(1000);
  
  clearLCD(); lcd.setCursor(0, 0); lcd.print("OTHER WORDS");
  delay(1000);
}

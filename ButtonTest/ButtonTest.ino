

const int DEBOUNCE = 75; // number of millis/samples to consider before declaring a debounced input
const int BUTTON = A0; // 6 buttons

long time = 0;  // the last time the output pin was sampled
int counter = 0; // how many times we have seen new value
int current_state = 0;  // the debounced input value
int ButtonVal;


void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(BUTTON, INPUT); // initialize the button pin as a input
}// setup

void loop() 
{
  // put your main code here, to run repeatedly: 
  parseButtons();
}// loop

void parseButtons()
{
  // If we have not gone on to the next millisecond
  if (millis() == time) return;
  
  // check analog pin for the button value and save it to ButtonVal
  ButtonVal = analogRead(BUTTON);
  
  //debounce
  if(ButtonVal == current_state && counter >0)
  {
    counter--;
  }
  if(ButtonVal != current_state)
  {
    counter++;
  }
  
  // If ButtonVal has shown the same value for long enough let's switch it
  if (counter >= DEBOUNCE)
  {
    counter = 0;
    current_state = ButtonVal;
    //Checks which button or button combo has been pressed
    if (ButtonVal > 20)
    {
      buttonPushed((ButtonVal+20)/144);
    }
  }
  
  time = millis();
}// parseButtons

void buttonPushed(int index)
{
      Serial.println(index);
  
}// buttonPushed


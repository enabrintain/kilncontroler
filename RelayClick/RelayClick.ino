

const int RELAY = A1; // the pin for turning on and off the heating coils
boolean flag = false;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(RELAY, OUTPUT); // initialize the relay pin as output
  digitalWrite(RELAY,flag);
}

void loop() {
  // put your main code here, to run repeatedly: 
  
  flag = !flag;
  digitalWrite(RELAY,flag);
  Serial.println("RELAY");
  delay(1500);
}

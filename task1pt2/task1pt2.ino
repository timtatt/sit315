int ledPin = 13;
int switchPin = 2;

volatile byte switchState = LOW;

void setup() {
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(switchPin), onSwitchChange, CHANGE);

  Serial.begin(9600);
}

void loop() {
  delay(50);
}

void onSwitchChange() {
  switchState = digitalRead(switchPin);
  digitalWrite(ledPin, switchState);
  Serial.println("Switch State Changed");
  delay(100);
}

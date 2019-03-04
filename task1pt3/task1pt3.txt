int ledPin = 13;
int switchPin = 2;

int trigPin = 11;
int echoPin = 12;

volatile byte switchState = LOW;
long duration;
int distance;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
	pinMode(switchPin, INPUT_PULLUP);
	pinMode(ledPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(switchPin), onSwitchChange, CHANGE);

	Serial.begin(9600);
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

//  Serial.print("Distance: ");
//  Serial.print(distance);
//  Serial.println("cm");

  if (distance < 10) {
    Serial.println("Danger!");
    digitalWrite(ledPin, HIGH);
    delay(25);
    digitalWrite(ledPin, LOW);
    delay(25);
  } else {
	  delay(50);
  }
}

void onSwitchChange() {
  switchState = digitalRead(switchPin);
  digitalWrite(ledPin, switchState);
  Serial.println("Switch State Changed");
  delay(100);
}

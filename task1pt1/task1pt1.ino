int ledPin = 13;
int switchPin = A0;

int switchState = LOW;

void setup() {
	pinMode(switchPin, INPUT);
	pinMode(ledPin, OUTPUT);

	Serial.begin(9600);
}

void loop() {
	switchState = digitalRead(switchPin);
	digitalWrite(ledPin, switchState);

	if (switchState == HIGH) {
		Serial.println("Button Pressed");
	}
	delay(50);
}

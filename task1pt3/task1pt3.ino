#include <TimerOne.h>

int ledPin = 13;
int switchPin = 2;

int trigPin = 11;
int echoPin = 3;

int timerUs = 50;
int tickCounts = 4000;

volatile byte switchState = LOW;
volatile long echoStart = 0;
volatile long echoEnd = 0;
volatile long echoDuration = 0;
volatile long echoDistance = 0;
volatile int triggerTimeCount = 0;
volatile long rangeFlasherCounter = 0;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
	pinMode(switchPin, INPUT_PULLUP);
	pinMode(ledPin, OUTPUT);

  Timer1.initialize(timerUs);
  Timer1.attachInterrupt(timerIsr);
  
//  attachInterrupt(digitalPinToInterrupt(switchPin), onSwitchChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(echoPin), onReceiveEcho, CHANGE);

	Serial.begin(9600);
}

void loop() {
  Serial.println(echoDuration / 58);
  delay(100);
}

void timerIsr() {
  triggerPulse();
  distanceFlasher();
}

void triggerPulse() {
  static volatile int state = 0;
  if (!(--triggerTimeCount)) {
    triggerTimeCount = tickCounts;
    state = 1;
  }

  switch (state) {
    case 1:
      digitalWrite(trigPin, HIGH);
      state = 2;
      break;
    case 2:
      digitalWrite(trigPin, LOW);
      state = 0;
      break;
  }
}

void onReceiveEcho() {
  switch (digitalRead(echoPin)) {
    case HIGH:
      echoEnd = 0;
      echoStart = micros();
      break;
    case LOW:
      echoEnd = micros();
      echoDuration = echoEnd - echoStart;
      echoDistance = echoDuration / 58;
      if (echoDistance < 10) {
        Serial.println("Danger!");
      }
      break;
  }
}

void distanceFlasher() {
  if (--rangeFlasherCounter <= 0) {
     if (echoDuration < 25000) {
        rangeFlasherCounter = echoDuration * 2;
     } else {
        rangeFlasherCounter = 25000;
     }
     if (switchState == LOW) {
        digitalWrite(ledPin, !digitalRead(ledPin));
     }
  }
}

void onSwitchChange() {
  switchState = digitalRead(switchPin);
  digitalWrite(ledPin, switchState);
  Serial.println("Switch State Changed");
  delay(100);
}

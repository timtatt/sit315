#include <PinChangeInterrupt.h>

int ledPin = 13;
int switchPin = A0;
int motionPin = 3;

volatile byte switchState = 0;

void setup() {
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(motionPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // = (16*10^6) / (1*1024) - 1 (must be <65536)
  // 1Hz Timer
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
  
  attachPCINT(digitalPinToPCINT(switchPin), onSwitchChange, CHANGE);
  attachPCINT(digitalPinToPCINT(motionPin), onMotionSensed, CHANGE);

  Serial.begin(9600);
}

ISR(TIMER1_COMPA_vect) {
  digitalWrite(ledPin, !digitalRead(ledPin));
}

void loop() {
  delay(5);
}

void onMotionSensed() {
  if (digitalRead(motionPin) == HIGH) {
    Serial.println("Got Ya!");
  } else {
    Serial.println("Shit where'd you go");
  }
}

void onSwitchChange() {
  switchState = digitalRead(switchPin);
  Serial.println("Switch State Changed");
  delay(100);
}

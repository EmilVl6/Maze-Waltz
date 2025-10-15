#include <Servo.h>

#define DELAY 5000
#define MOFFSET -10

Servo servoLeft;
Servo servoRight;

int valLeft, valRight, valMiddle = 0;

const int L_irLedPin = 10, L_irReceiverPin = 11, L_redLedPin = A2;
const int R_irLedPin =  2, R_irReceiverPin =  3, R_redLedPin = A0;
const int M_irLedPin =  6, M_irReceiverPin =  7, M_redLedPin = A1;

// Forward: 0 to 200 & Rotation: -100 to 100
void move(int forward, int rotation) {
  forward = constrain(forward, 0, 200);
  rotation = constrain(rotation, -100, 100);

  int leftPWM = 1500 - forward + rotation - MOFFSET/2;
  int rightPWM = 1500 + forward + rotation + MOFFSET/2;

  leftPWM = constrain(leftPWM, 1300, 1700);
  rightPWM = constrain(rightPWM, 1300, 1700);
  
  servoLeft.writeMicroseconds(leftPWM);
  servoRight.writeMicroseconds(rightPWM);
}

int irDetect(int irLedPin, int irReceiverPin, long frequency) {
  tone(irLedPin, frequency);
  delay(1);
  int ir = digitalRead(irReceiverPin);
  noTone(irLedPin);
  delay(1);
  return ir;
}

int irDistance(int irLedPin, int irReceiverPin) {
  int distance = 0;
  for(long f = 38000; f <= 42000; f += 1000) {
    distance += irDetect(irLedPin, irReceiverPin, f);
  }
  return distance;
}

int mapBrightness(int val) {
  int level = 5 - constrain(val, 0, 5);
  return map(level, 0, 5, 0, 255);
}

void setup() {
  servoLeft.attach(12); servoRight.attach(13);

  move(0,0);

  pinMode(L_irReceiverPin, INPUT); pinMode(L_irLedPin, OUTPUT); pinMode(L_redLedPin, OUTPUT);
  pinMode(R_irReceiverPin, INPUT); pinMode(R_irLedPin, OUTPUT); pinMode(R_redLedPin, OUTPUT);
  pinMode(M_irReceiverPin, INPUT); pinMode(M_irLedPin, OUTPUT); pinMode(M_redLedPin, OUTPUT);  

  Serial.begin(9600);
  delay(DELAY);
}

void loop() {
  valLeft = irDistance(L_irLedPin, L_irReceiverPin);
  valRight = irDistance(R_irLedPin, R_irReceiverPin);
  valMiddle = irDistance(M_irLedPin, M_irReceiverPin);

  analogWrite(L_redLedPin, mapBrightness(valLeft));
  analogWrite(R_redLedPin, mapBrightness(valRight));
  analogWrite(M_redLedPin, mapBrightness(valMiddle));

  Serial.print(valLeft); Serial.print(" "); Serial.print(valRight); Serial.print(" "); Serial.println(valMiddle);
}
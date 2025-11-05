#include <Servo.h>

Servo servoLeft;
Servo servoRight;

#define DELAY 1000
#define MOFFSET 0

const int L_irLedPin = 10, L_irReceiverPin = 11, L_redLedPin = A2;
const int R_irLedPin =  2, R_irReceiverPin =  3, R_redLedPin = A0;
const int M_irLedPin =  6, M_irReceiverPin =  7, M_redLedPin = A1;

void setup() {
  servoLeft.attach(12); servoRight.attach(13);

  move(0,0);

  pinMode(L_irReceiverPin, INPUT); pinMode(L_irLedPin, OUTPUT); pinMode(L_redLedPin, OUTPUT);
  pinMode(R_irReceiverPin, INPUT); pinMode(R_irLedPin, OUTPUT); pinMode(R_redLedPin, OUTPUT);
  pinMode(M_irReceiverPin, INPUT); pinMode(M_irLedPin, OUTPUT); pinMode(M_redLedPin, OUTPUT);  

  Serial.begin(9600);
  delay(DELAY);

  move(100, 0);
}

#define freqStart 38000
#define freqEnd 42000
#define freqStep 1000
#define distMax 5

#define Fwd 1
#define TurnL 2
#define TurnR 3
#define DeadEnd 4

#define timeExit 100
#define timeTurn 1700

#define moveFwd 100
#define veer 30
#define turn 40

int valLeft = 0, valRight = 0, valMiddle = 0; int MotionState = Fwd; bool deadEnd = 1;

void loop() {
  valLeft = irDistance(L_irLedPin, L_irReceiverPin, L_redLedPin);
  valRight = irDistance(R_irLedPin, R_irReceiverPin, R_redLedPin);
  valMiddle = irDistance(M_irLedPin, M_irReceiverPin, M_redLedPin);

  Serial.print(valLeft); Serial.print(" "); Serial.print(valRight); Serial.print(" "); Serial.println(valMiddle);

  //MAZE WALTZ
  switch(MotionState) {
    case Fwd: {
      Serial.println("Fwd");
      if(valLeft == valRight) {
        Serial.println("Straight");
        move(moveFwd, 0);
      }
      if(valLeft > valRight) {
        Serial.println("Veer Left");
        move(moveFwd - veer/2, -veer/2);
      }
      if(valRight > valLeft) {
        Serial.println("Veer Right");
        move(moveFwd - veer/2, veer/2);
      }
      if(valLeft>=distMax && valMiddle<distMax) {
        MotionState = TurnL;
        break;
      }
      if(valRight>=distMax && valMiddle<distMax) {
        MotionState = TurnR;
        break;
      }
      if(valLeft<distMax && valRight<distMax && valMiddle<distMax-2) {
        MotionState = DeadEnd;
        break;
      }
    break;
    }
    case TurnL: {
      Serial.println("TurnL");
      move(moveFwd/2, 0);
      delay(timeExit*5);
      move(turn/2, -turn/2);      
      delay(timeTurn*1.2);
      move(moveFwd, 0);
      delay(timeExit);
      MotionState=Fwd;
    break;
    }
    case TurnR: {
      Serial.println("TurnR");
      move(moveFwd/2, 0);
      delay(timeExit*5);
      move(turn/2, turn/2);      
      delay(timeTurn*1.1);
      move(moveFwd, 0);
      delay(timeExit);
      MotionState=Fwd;
    break;
    }
    case DeadEnd: {
      if(deadEnd) {
        Serial.println("DeadEnd");
        move(0, -turn);      
        delay(timeTurn*1.4);
        move(moveFwd, 0);
        delay(timeExit);
        MotionState=Fwd;
        deadEnd = 0;
      }
      else {
        Serial.println("End");
        move(0, 0);      
      }
    break;
    }    
  }

  //delay(100);
}

// Forward: -200 to 200 & Rotation: -100 to 100
void move(int forward, int rotation) {
  forward = constrain(forward, -200, 200);
  rotation = constrain(rotation, -100, 100);

  int leftPWM = 1500 - forward + rotation - MOFFSET/2;
  int rightPWM = 1500 + forward + rotation + MOFFSET/2;

  leftPWM = constrain(leftPWM, 1300, 1700);
  rightPWM = constrain(rightPWM, 1300, 1700);
  
  servoLeft.writeMicroseconds(leftPWM);
  servoRight.writeMicroseconds(rightPWM);
}

int irDetect(long frequency, int irLedPin, int irReceiverPin, int redLedPin) {
  tone(irLedPin, frequency);
  delay(1);
  int ir = digitalRead(irReceiverPin);
  digitalWrite(redLedPin, !ir);
  noTone(irLedPin);
  delay(1);
  return ir;
}

int irDistance(int irLedPin, int irReceiverPin, int redLedPin) {
  int distance = 0;
  for(long f = freqStart; f <= freqEnd; f += freqStep) {
    distance += irDetect(f, irLedPin, irReceiverPin, redLedPin);
  }
  return distance;
}
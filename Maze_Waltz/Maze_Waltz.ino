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
#define LongDeadEnd 5

#define timeExit 100
#define timeTurn 1700
#define timeForward 800

#define moveFwd 100
#define veer 30
#define turn 40

int valLeft = 0, valRight = 0, valMiddle = 0; 
int MotionState = Fwd; 
bool deadEnd = 1;
unsigned long forwardStartTime = 0;
bool inStraightSection = false;

void loop() {
  valLeft = irDistance(L_irLedPin, L_irReceiverPin, L_redLedPin);
  valRight = irDistance(R_irLedPin, R_irReceiverPin, R_redLedPin);
  valMiddle = irDistance(M_irLedPin, M_irReceiverPin, M_redLedPin);

  Serial.print(valLeft); Serial.print(" "); Serial.print(valRight); Serial.print(" "); Serial.println(valMiddle);

  //MAZE WALTZ
  switch(MotionState) {
    case Fwd: {
      Serial.println("Fwd");
      
      if(valRight >= distMax && valMiddle >= distMax) {
        move(moveFwd, 0);
        delay(timeExit*4);
        MotionState = TurnR;
        //delay(10);
        break;
      }
      else if(valRight >= distMax && valMiddle < distMax) {
        move(moveFwd, 0);
        delay(timeExit*2);
        MotionState = TurnR;
        //delay(10);
        break;
      }
      else if(valMiddle < distMax) {
        if(valLeft >= distMax) {
          move(moveFwd, 0);
          delay(timeExit*2);
          MotionState = TurnL;
          //delay(10);
        } else {
          MotionState = DeadEnd;
          //delay(10);
        }
        break;
      }
      else {
        if(valRight < distMax && valLeft < distMax) {
          Serial.println("Corridor - balanced");
          if(valLeft == valRight) {
            move(moveFwd, 0);
          }
          else if(valLeft > valRight) {
            move(moveFwd - veer/4, -veer/4);
          }
          else {
            move(moveFwd - veer/4, veer/4);
          }
        }
        else if(valRight < distMax) {
          Serial.println("Following right wall");
          move(moveFwd - veer/3, -veer/3);
        }
        else if(valLeft < distMax) {
          Serial.println("Following left wall");
          move(moveFwd - veer/3, veer/3);
        }
        else {
          Serial.println("Open area");
          move(moveFwd, 0);
        }
        
        if(valLeft < distMax && valRight < distMax && valMiddle >= distMax) {
          if(!inStraightSection) {
            inStraightSection = true;
            forwardStartTime = millis();
          }
        } else {
          inStraightSection = false;
        }
      }
    break;
    }
    case TurnL: {
      Serial.println("TurnL");
      move(moveFwd, 0);
      delay(timeExit*3);
      move(turn/2, -turn/2);      
      delay(timeTurn*1.2);
      move(moveFwd, 0);
      delay(timeExit);
      MotionState=Fwd;
    break;
    }
    case TurnR: {
      Serial.println("TurnR");
      move(turn/2, turn/2);      
      delay(timeTurn*1.1);
      move(moveFwd, 0);
      delay(timeExit);
      MotionState=Fwd;
    break;
    }
    case DeadEnd: {
      bool isLongDeadEnd = inStraightSection && (millis() - forwardStartTime > timeForward);
      
      if(isLongDeadEnd && deadEnd) {
        Serial.println("Long Dead End - FINISH!");
        MotionState = LongDeadEnd;
      }
      else if(deadEnd) {
        Serial.println("Short Dead End - Turn Around");
        move(-turn/2, -turn);
        delay(timeTurn*1.4);
        move(moveFwd, 0);
        delay(timeExit);
        MotionState = Fwd;
        deadEnd = 0;
      }
      else {
        Serial.println("Multiple dead ends - stopping");
        move(0, 0);
      }
    break;
    }
    case LongDeadEnd: {
      Serial.println("FINISH AREA REACHED!");
      move(0, 0);
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
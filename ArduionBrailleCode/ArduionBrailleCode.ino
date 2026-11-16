//#include<Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

//int pins[6] = {2,3,4,5,6,7};
//Servo servos[6];
//const int ANGLE_UP = 90;
//const int ANGLE_DOW = 0;

int lastletter = -1;
const int PULSE_UP = 350;
const int PULSE_DOWN = 150;
int brailleTable[26][6] = {
  {1,0,0,0,0,0}, // A  (1)
  {1,1,0,0,0,0}, // B  (1,2)
  {1,0,0,1,0,0}, // C  (1,4)
  {1,0,0,1,1,0}, // D  (1,4,5)
  {1,0,0,0,1,0}, // E  (1,5)
  {1,1,0,1,0,0}, // F  (1,2,4)
  {1,1,0,1,1,0}, // G  (1,2,4,5)
  {1,1,0,0,1,0}, // H  (1,2,5)
  {0,1,0,1,0,0}, // I  (2,4)
  {0,1,0,1,1,0}, // J  (2,4,5)
  {1,0,1,0,0,0}, // K  (1,3)
  {1,1,1,0,0,0}, // L  (1,2,3)
  {1,0,1,1,0,0}, // M  (1,3,4)
  {1,0,1,1,1,0}, // N  (1,3,4,5)
  {1,0,1,0,1,0}, // O  (1,3,5)
  {1,1,1,1,0,0}, // P  (1,2,3,4)
  {1,1,1,1,1,0}, // Q  (1,2,3,4,5)
  {1,1,1,0,1,0}, // R  (1,2,3,5)
  {0,1,1,1,0,0}, // S  (2,3,4)
  {0,1,1,1,1,0}, // T  (2,3,4,5)
  {1,0,1,0,0,1}, // U  (1,3,6)
  {1,1,1,0,0,1}, // V  (1,2,3,6)
  {0,1,0,1,1,1}, // W  (2,4,5,6)
  {1,0,1,1,0,1}, // X  (1,3,4,6)
  {1,0,1,1,1,1}, // Y  (1,3,4,5,6)
  {1,0,1,0,1,1}  // Z  (1,3,5,6)
};

void setup() {
  Serial.begin(9600);  
  pwm.begin();
  pwm.setPWMFreq(50);
  ResetALL();
  randomSeed(analogRead(A0));
  
  /*for(int i=0; i<6 ; i++)
  {
    servos[i].attach(pins[i]);
  }*/
}

void ResetALL(){
  for(int i = 0 ; i<6 ; i++)
  {
    pwm.setPWM(i, 0 , PULSE_DOWN);
  }
}

void raiseServos(int letterIndex){

  for(int i =0 ; i<6 ;i++){
    int raise = brailleTable[letterIndex][i];
    if (raise == 1){
      pwm.setPWM(i, 0 , PULSE_UP);
    }
    else {
      pwm.setPWM(i , 0 , PULSE_DOWN);
    } 
  }
}

void learnmode(){
  int letter;
  do {
    letter = random(26);
  }
  while(lastletter == letter);
  lastletter = letter ; 
  raiseServos(letter);
}
      
void learnAllLetters()
{
  for(int i=0 ; i<26 ; i++){
    raiseServos(i);
    delay(500);
  }
}

void loop() {
  
  if (Serial.available()>0)
  {
    char c = Serial.read();
    int letterIndex = c-'a';
    if(letterIndex>=0 && letterIndex <26)
    {
      raiseServos(letterIndex);
      Serial.println(c);
      delay(3000);
      ResetALL();
    }
  }
  }
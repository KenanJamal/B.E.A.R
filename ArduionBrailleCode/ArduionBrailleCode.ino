//#include<Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <ArduinoJson.h>
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

//int pins[6] = {2,3,4,5,6,7};
//Servo servos[6];
//const int ANGLE_UP = 90;
//const int ANGLE_DOW = 0;

int lastletter = -1;
const int PULSE_UP = 350;
const int PULSE_DOWN = 150;
const int LETTER_DELAY = 3000;

// letters loaded from the json file (english.json / spanish.json / japanese.json)
// sent over Serial by send_json.py
const int MAX_LETTERS = 50;   // japanese.json has 46
const int MAX_NAME = 4;       // longest name is "tsu" + '\0'
char letterNames[MAX_LETTERS][MAX_NAME];
byte brailleTable[MAX_LETTERS][6];   // dots 1..6
int letterCount = 0;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(5000);
  pwm.begin();
  pwm.setPWMFreq(50);
  ResetALL();
  randomSeed(analogRead(A0));
  Serial.println("Ready: waiting for json");

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

// Reads the json array from Serial one letter at a time, so the whole
// file never has to fit in RAM. Each element looks like:
//   { "name": "A", "value": [[1,0],[0,0],[0,0]] }
// value is 3 rows x 2 columns, which maps to braille dots:
//   [0][0]=1  [0][1]=4
//   [1][0]=2  [1][1]=5
//   [2][0]=3  [2][1]=6
bool loadJson(){
  if(!Serial.find("[")){
    return false;
  }
  letterCount = 0;
  JsonDocument doc;
  do {
    DeserializationError err = deserializeJson(doc, Serial);
    if(err){
      Serial.print("json error: ");
      Serial.println(err.c_str());
      letterCount = 0;   // don't keep a half loaded table
      return false;
    }
    if(letterCount >= MAX_LETTERS){
      Serial.println("too many letters, rest ignored");
      continue;
    }

    const char* name = doc["name"] | "";
    strncpy(letterNames[letterCount], name, MAX_NAME - 1);
    letterNames[letterCount][MAX_NAME - 1] = '\0';

    JsonArray value = doc["value"];
    for(int row = 0 ; row<3 ; row++){
      brailleTable[letterCount][row]     = value[row][0] | 0;   // dots 1,2,3
      brailleTable[letterCount][row + 3] = value[row][1] | 0;   // dots 4,5,6
    }
    letterCount++;
  } while(Serial.findUntil(",", "]"));

  Serial.print("loaded letters: ");
  Serial.println(letterCount);
  return true;
}

// throw away whatever is still coming in, until the line has been quiet
// for 200ms, so the rest of a broken json isn't read as letter commands
void dropLeftovers(){
  unsigned long lastByte = millis();
  while(millis() - lastByte < 200){
    if(Serial.available()){
      Serial.read();
      lastByte = millis();
    }
  }
}

void printLetter(int letterIndex){
  Serial.print(letterNames[letterIndex]);
  Serial.print(" -> ");
  for(int i=0 ; i<6 ; i++){
    Serial.print(brailleTable[letterIndex][i]);
  }
  Serial.println();
}

int findLetter(const char* name){
  for(int i=0 ; i<letterCount ; i++){
    if(strcasecmp(letterNames[i], name) == 0){
      return i;
    }
  }
  return -1;
}

void learnmode(){
  if(letterCount < 2) return;
  int letter;
  do {
    letter = random(letterCount);
  }
  while(lastletter == letter);
  lastletter = letter ;
  raiseServos(letter);
}

void learnAllLetters()
{
  for(int i=0 ; i<letterCount ; i++){
    printLetter(i);
    raiseServos(i);
    delay(LETTER_DELAY);
    ResetALL();
    delay(500);
  }
}

void loop() {

  if (Serial.available()>0)
  {
    char c = Serial.peek();
    if(c == '['){
      // a json file is coming: load it, then show every letter
      if(loadJson()){
        learnAllLetters();
      }
      dropLeftovers();
    }
    else {
      // single letter typed in the Serial Monitor, e.g. "a"
      char name[2] = { (char)Serial.read(), '\0' };
      int letterIndex = findLetter(name);
      if(letterIndex >= 0)
      {
        printLetter(letterIndex);
        raiseServos(letterIndex);
        delay(LETTER_DELAY);
        ResetALL();
      }
    }
  }
}

// Physical layout of the buttons (same as a braille cell):
//   1 4
//   2 5
//   3 6
// Button N is braille dot N.
const uint8_t BUTTON_PINS[6] = {2, 3, 4, 5, 6, 7};   // button 1..6 on pins 2..7

// How long (ms) the user has to press the NEXT dot after the last new dot.
// Every new dot pressed restarts this timer.
const unsigned long EXTRA_TIME = 3000;

#define D1 0x01
#define D2 0x02
#define D3 0x04
#define D4 0x08
#define D5 0x10
#define D6 0x20

// Checked against english.json (rows [1,2,3] x columns [dots 1-3 | dots 4-6]), all 26 letters match.
const uint8_t BRAILLE[26] = {
  D1,                        // A
  D1 | D2,                   // B
  D1 | D4,                   // C
  D1 | D4 | D5,              // D
  D1 | D5,                   // E
  D1 | D2 | D4,              // F
  D1 | D2 | D4 | D5,         // G
  D1 | D2 | D5,              // H
  D2 | D4,                   // I
  D2 | D4 | D5,              // J
  D1 | D3,                   // K
  D1 | D2 | D3,              // L
  D1 | D3 | D4,              // M
  D1 | D3 | D4 | D5,         // N
  D1 | D3 | D5,              // O
  D1 | D2 | D3 | D4,         // P
  D1 | D2 | D3 | D4 | D5,    // Q
  D1 | D2 | D3 | D5,         // R
  D2 | D3 | D4,              // S
  D2 | D3 | D4 | D5,         // T
  D1 | D3 | D6,              // U
  D1 | D2 | D3 | D6,         // V
  D2 | D4 | D5 | D6,         // W
  D1 | D3 | D4 | D6,         // X
  D1 | D3 | D4 | D5 | D6,    // Y
  D1 | D3 | D5 | D6          // Z
};

int8_t currentLetter = -1;
uint8_t chord = 0;                 // all dots pressed so far for this letter
uint8_t lastMask = 0;
unsigned long lastNewDotTime = 0;  // when the last NEW dot was pressed
uint16_t score = 0;
uint16_t attempts = 0;

void speak(const String &text) {
  Serial.println(text);
}

// returns a mask where bit i = button i+1 = dot i+1
uint8_t readButtons() {
  uint8_t m1 = 0, m2 = 0;
  for (uint8_t i = 0; i < 6; i++) if (digitalRead(BUTTON_PINS[i]) == LOW) m1 |= (1 << i);
  delay(15);
  for (uint8_t i = 0; i < 6; i++) if (digitalRead(BUTTON_PINS[i]) == LOW) m2 |= (1 << i);
  return (m1 == m2) ? m1 : lastMask;
}

// prints the button/dot numbers (1..6) for a mask
String dotsToText(uint8_t mask) {
  if (mask == 0) return "none";
  String s = "";
  for (uint8_t i = 0; i < 6; i++) {
    if (mask & (1 << i)) {
      if (s.length()) s += ", ";
      s += String(i + 1);
    }
  }
  return s;
}

void pickNewLetter() {
  int8_t next;
  do {
    next = random(26);
  } while (next == currentLetter);
  currentLetter = next;
}

void announceLetter() {
  speak("");
  speak(String("Letter: ") + (char)('A' + currentLetter));
  chord = 0;
  lastMask = 0;
}

// wait until every button is released, so a held button
// isn't counted as a press for the next letter
void waitForRelease() {
  while (readButtons() != 0) {}
  lastMask = 0;
}

void setup() {
  Serial.begin(9600);
  for (uint8_t i = 0; i < 6; i++) pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  randomSeed(analogRead(A0));

  speak("=== Braille Trainer ===");
  speak("Press the buttons of the letter you hear, one by one or together.");
  pickNewLetter();
  announceLetter();
}

void finishAttempt(bool correct, uint8_t expected) {
  attempts++;
  Serial.print("You pressed buttons: ");
  Serial.println(dotsToText(chord));

  if (correct) {
    score++;
    speak("Correct!");
    Serial.print("Score: ");
    Serial.print(score);
    Serial.print("/");
    Serial.println(attempts);
    delay(1000);
    waitForRelease();
    pickNewLetter();
    announceLetter();
  } else {
    speak("Wrong, try again.");
    Serial.print("(Expected buttons: ");
    Serial.print(dotsToText(expected));
    Serial.println(")");
    delay(1000);
    waitForRelease();
    announceLetter();
  }
}

void loop() {
  uint8_t mask = readButtons();

  // remember every dot that gets pressed; a NEW dot restarts the timer
  if (mask & ~chord) {
    chord |= mask;
    lastNewDotTime = millis();
  }
  lastMask = mask;

  if (chord == 0) return;   // nothing pressed yet, wait as long as needed

  uint8_t expected = BRAILLE[currentLetter];

  if (chord & ~expected) {
    // a dot that is not part of this letter
    finishAttempt(false, expected);
  } else if (chord == expected) {
    // all required dots have been pressed
    finishAttempt(true, expected);
  } else if (millis() - lastNewDotTime > EXTRA_TIME) {
    // right dots so far, but the user ran out of time
    speak("Time is up.");
    finishAttempt(false, expected);
  }
}
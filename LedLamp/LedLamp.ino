
// 2021/01/03 - adding ultrasonic control

#include "MiniRemote.h"
#include "TimeOfDay.h"
#include "Utils.h"
#include "ultrasonic.h"

//
// Alarm time configuration
//
int G_alarmHour = 6;
int G_alarmMinute = 0;
int G_alarmDurationMinutes = 4*60;

//
// Ultrasonic control
//
int ECHO_PIN = 2;
int TRIGGER_PIN = 3;
int ULTRASONIC_ENABLE = 4; // should wire to VCC
int TRIGGER_DISTANCE_CM = 10;

Ultrasonic G_ultrasonic(TRIGGER_PIN, ECHO_PIN, ULTRASONIC_ENABLE, TRIGGER_DISTANCE_CM);

//
// Illumination at alarm time and night-light mode as well as providing
// indicators while remote control functions are in use.
//
int BLUE_PIN = 8;
int GREEN_PIN = 9;
int RED_PIN = 10;

// Remote control
int IR_PIN = 11;

int FLASH_PIN = -1;

int G_rgbPins[] {RED_PIN, GREEN_PIN, BLUE_PIN};


//
// Program state
//
enum ProgramState { INIT, MAIN, SET_TIME, SET_ALARM, NIGHTLIGHT };

//
// Allow mixing of colors via bit mask.
//
int G_numColors = 0;
unsigned RED_MASK = 1 << ++G_numColors;
unsigned GREEN_MASK = 1 << ++G_numColors;
unsigned BLUE_MASK = 1 << ++G_numColors;

// Remote control
MiniRemote G_remote(IR_PIN, FLASH_PIN);

//
// Tracks the time of day once it is set
// via the buttons.
//
TimeOfDay G_timeOfDay(millis());

//
// TODO There is an edge case here where the stop time might
// flow into the next day (when start time is near midnight and
// duration plus start passes midnight)
//

unsigned long G_alarmStartMillis = 0;
unsigned long G_alarmDuration = 0;
unsigned long G_alarmStopMillis= 0;

void updateAlarmTime() {
  G_alarmStartMillis =
    TimeOfDay::hoursToMillis(G_alarmHour) +
    TimeOfDay::minutesToMillis(G_alarmMinute);
  
  G_alarmDuration = TimeOfDay::minutesToMillis(G_alarmDurationMinutes);
  G_alarmStopMillis = G_alarmStartMillis + G_alarmDuration;
}

unsigned getColorMask() {
  //
  // See if we are within the alarm window. If not, no colors are illuminated.
  //
  long now = G_timeOfDay.getTimeOfDay(millis());
  bool alarming = (now > G_alarmStartMillis && now < G_alarmStopMillis);
  if (!alarming)
    return 0;

  //
  // Divide the colors and combinations as a a desired fraction of the alarm window.
  //
  float fraction_of_alarm_elapsed = (float)(now - G_alarmStartMillis) / (float)G_alarmDuration;
  if (fraction_of_alarm_elapsed < 1./4.) return GREEN_MASK;
  if (fraction_of_alarm_elapsed < 2./4.) return BLUE_MASK;
  if (fraction_of_alarm_elapsed < 3./4.) return RED_MASK;
  if (fraction_of_alarm_elapsed < 4./4.) return RED_MASK|BLUE_MASK;
  return 0;
}

void setup() {
  Serial.begin(9600);

  G_remote.init();
  G_ultrasonic.init();

  Utils::outputPins(G_rgbPins, LENGTH(G_rgbPins));
  G_timeOfDay.setTime(12, 0, 0, millis()); // Default to noon
  updateAlarmTime();
}

void indicateTimeOfDay(bool blinkTime) {
  int hrs, mins, secs;
  G_timeOfDay.getTimeOfDay(&hrs, &mins, &secs, millis());

  // On the serial line
  char s[80];
  sprintf(s, "Current time: %02d:%02d:%02d", hrs, mins, secs);
  Serial.println(s);
  sprintf(s, "Alarm time: %02d:%02d", G_alarmHour, G_alarmMinute);
  Serial.println(s);

  if (blinkTime) {
    // Blink it on the LEDs with the following scheme:
    // Blink red to indicate each digit with a blue flash
    // in between digits to indicate a change in digits.
  
    int digits[] = {hrs / 10, hrs % 10, mins / 10, mins % 10};
  
    for (int i = 0; i < LENGTH(digits); ++i) {
      Utils::flashPin(RED_PIN, digits[i], 200);
      Utils::flashPin(BLUE_PIN, 1, 200);
      delay(500);
    }
  }
}

void indicateStateChange() {
  
  Utils::pinsLow(G_rgbPins, LENGTH(G_rgbPins));

  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < LENGTH(G_rgbPins); ++i) {
      Utils::flashPin(G_rgbPins[i], 1, 10);
    }
  }
}

ProgramState setTimeLoop(ProgramState prevState) {
  
  if (prevState != SET_TIME)
    indicateStateChange();
  
  static int S_timeDigitsCollected = 0;
  static int S_digitValues[4] = {0, 0, 0, 0};
  ProgramState nextState = SET_TIME;

  //
  // Illuminate pins to indicate time setting mode.
  //
  if (S_timeDigitsCollected > 0) {
    int indicatorPin = G_rgbPins[S_timeDigitsCollected - 1];
    Utils::pinsLow(G_rgbPins, LENGTH(G_rgbPins), indicatorPin);
    digitalWrite(indicatorPin, HIGH);
  }

  //
  // Now see if we got remote input and take action on it if so.
  //
  char buttonName[20];
  int buttonVal = -1;
  
  bool readSomething = G_remote.readInput(buttonName, &buttonVal);

  if (readSomething) {  
    if (strcmp(buttonName, "CH") == 0) {
      nextState = MAIN;
      Serial.println("Changing to main loop mode.");
    }
    else if (strcmp(buttonName, "DIGIT") == 0) {
      S_digitValues[S_timeDigitsCollected++] = buttonVal;
    }

    //
    // Once the final digit is entered, we can process it and change states.
    //
    if (S_timeDigitsCollected == LENGTH(S_digitValues)) {
      int newTime = 0;
      for (int i = 0; i < LENGTH(S_digitValues); ++i) {
        newTime = newTime * 10 + S_digitValues[i];
      }
      G_timeOfDay.setTime(10*S_digitValues[0] + S_digitValues[1], 10*S_digitValues[2] + S_digitValues[3], 0, millis());
      indicateTimeOfDay(false);
      nextState = MAIN;
    }
  }

  //
  // Clean up the state as we leave time setting mode.
  //
  if (nextState != SET_TIME) {
    S_timeDigitsCollected = 0;
    Utils::pinsLow(G_rgbPins, LENGTH(G_rgbPins));
  }
  
  return nextState;
}

ProgramState setAlarmLoop(ProgramState prevState) {
  
  if (prevState != SET_ALARM)
    indicateStateChange();
  
  static int S_timeDigitsCollected = 0;
  static int S_digitValues[4] = {0, 0, 0, 0};
  ProgramState nextState = SET_ALARM;

  //
  // Illuminate pins to indicate time setting mode.
  //
  if (S_timeDigitsCollected > 0) {
    int indicatorPin = G_rgbPins[S_timeDigitsCollected - 1];
    Utils::pinsLow(G_rgbPins, LENGTH(G_rgbPins), indicatorPin);
    digitalWrite(indicatorPin, HIGH);
  }

  //
  // Now see if we got remote input and take action on it if so.
  //
  char buttonName[20];
  int buttonVal = -1;
  
  bool readSomething = G_remote.readInput(buttonName, &buttonVal);

  if (readSomething) {  
    if (strcmp(buttonName, "CH") == 0) {
      nextState = MAIN;
      Serial.println("Changing to main loop mode.");
    }
    else if (strcmp(buttonName, "DIGIT") == 0) {
      S_digitValues[S_timeDigitsCollected++] = buttonVal;
    }

    //
    // Once the final digit is entered, we can process it and change states.
    //
    if (S_timeDigitsCollected == LENGTH(S_digitValues)) {
      int newTime = 0;
      for (int i = 0; i < LENGTH(S_digitValues); ++i) {
        newTime = newTime * 10 + S_digitValues[i];
      }
      G_alarmHour = 10*S_digitValues[0] + S_digitValues[1];
      G_alarmMinute = 10*S_digitValues[2] + S_digitValues[3];
      updateAlarmTime();
      nextState = MAIN;
    }
  }

  //
  // Clean up the state as we leave time setting mode.
  //
  if (nextState != SET_ALARM) {
    S_timeDigitsCollected = 0;
    Utils::pinsLow(G_rgbPins, LENGTH(G_rgbPins));
  }
  
  return nextState;
}

ProgramState nightlightLoop(ProgramState prevState) {
  
  ProgramState nextState = NIGHTLIGHT;

  if (prevState != NIGHTLIGHT) {
    Serial.println("Entered nightlight mode.");
    Utils::pinsHigh(G_rgbPins, LENGTH(G_rgbPins));
  }

  if (G_ultrasonic.hasFired()) {
    Utils::pinsLow(G_rgbPins, LENGTH(G_rgbPins));
    nextState = MAIN;
    Serial.println("Leaving nightlight mode.");
  }

  return nextState;
}

ProgramState mainLoop(ProgramState prevState) {
  
  if (prevState != MAIN)
    indicateStateChange();

  ProgramState nextState = MAIN;

  //
  // Illuminate when appropriate
  //
  unsigned colorMask = getColorMask();
  
  digitalWrite(RED_PIN, colorMask & RED_MASK);
  digitalWrite(GREEN_PIN, colorMask & GREEN_MASK);
  digitalWrite(BLUE_PIN, colorMask & BLUE_MASK);

  //
  // Now see if we got remote input and take action on it if so.
  //
  char buttonName[20];
  int buttonVal = -1;
  
  bool readSomething = G_remote.readInput(buttonName, &buttonVal);

  if (readSomething) {  
    if (strcmp(buttonName, "CH") == 0) {
      nextState = SET_TIME;
      Serial.println("Changing to set time mode.");
    }
    else if (strcmp(buttonName, "CH+") == 0) {
      nextState = SET_ALARM;
      Serial.println("Changing to alarm set mode.");
    }
    else if (strcmp(buttonName, "EQ") == 0) {
      indicateTimeOfDay(true);
    }
  }
  else if (G_ultrasonic.hasFired()) {
    nextState = NIGHTLIGHT;
  }
  return nextState;
}

void loop() {

  static ProgramState S_currentState = INIT;
  static ProgramState S_previousState = INIT;
  
  ProgramState nextState = MAIN;

  switch (S_currentState) {
    case MAIN:
      nextState = mainLoop(S_previousState);
      break;
    case SET_TIME:
      nextState = setTimeLoop(S_previousState);
      break;
    case SET_ALARM:
      nextState = setAlarmLoop(S_previousState);
      break;
    case NIGHTLIGHT:
      nextState = nightlightLoop(S_previousState);
      break;
    default:
      nextState = MAIN;
      break;
  }

  S_previousState = S_currentState;
  S_currentState = nextState;
}

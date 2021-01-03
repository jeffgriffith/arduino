#ifndef UTILS
#define UTILS

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

class Utils {
  public:
    static void writePins(int pins[], int numPins, int value, int exceptPin) {
      for (int i = 0; i < numPins; ++i)
        if (pins[i] != exceptPin)
          digitalWrite(pins[i], value);
    }

    static void pinsLow(int pins[], int numPins, int exceptPin) {
      writePins(pins, numPins, LOW, exceptPin);
    }

    static void pinsLow(int pins[], int numPins) {
      writePins(pins, numPins, LOW, -1);
    }

    static void pinsHigh(int pins[], int numPins, int exceptPin) {
      writePins(pins, numPins, HIGH, exceptPin);
    }

    static void pinsHigh(int pins[], int numPins) {
      writePins(pins, numPins, HIGH, -1);
    }

    static void outputPins(int pins[], int numPins) {
      for (int i = 0; i < numPins; ++i)
        pinMode(pins[i], OUTPUT);
    }
    
    static void inputPins(int pins[], int numPins) {
      for (int i = 0; i < numPins; ++i)
        pinMode(pins[i], INPUT);
    }

    static void flashPin(int pin, int n, int d) {
      int oldPinVal = digitalRead(pin);
      for (int i = 0; i < n; ++i) {
        digitalWrite(pin, HIGH);
        delay(d);
        digitalWrite(pin, LOW);
        delay(d);
      }
      digitalWrite(pin, oldPinVal);
    }
    
    static void flashPin(int pin, int n) {
      flashPin(pin, n, 50);
    }    
};

#endif

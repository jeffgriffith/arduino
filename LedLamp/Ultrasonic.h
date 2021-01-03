#ifndef _ULTRASONIC_
#define _ULTRASONIC_

class Ultrasonic {
  private:
    int _triggerPin;
    int _echoPin;
    int _powerPin;
    int _triggerDistanceCM = 0;
    bool _firing = false;
    unsigned long _lastTimeFired = 0;
    
  public:
    Ultrasonic(int triggerPin, int echoPin, int powerPin, int triggerDistanceCM) {
      _triggerPin = triggerPin;
      _echoPin = echoPin;
      _powerPin = powerPin;
      _triggerDistanceCM = triggerDistanceCM;
    }

    void init() {
      pinMode(_triggerPin, OUTPUT);
      pinMode(_echoPin, INPUT);

      //
      // Power up the given pin, otherwise we assume we are already on
      // (connected to VCC)
      //
      if (_powerPin >= 0) {
        pinMode(_powerPin, OUTPUT);
        digitalWrite(_powerPin, HIGH);
      }
    }

    int getDistanceCM() {

      //
      // The HC-SR04 requires a pulse of 10 microseconds
      // before reading the echo pin, so we take it low
      // for 2usec, high for 10usec, then back to low
      // before listening on the echo pin.
      // 
      digitalWrite(_triggerPin, LOW);
      delayMicroseconds(2);
      digitalWrite(_triggerPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(_triggerPin, LOW);

      //
      // Read the echo pin then calculate the distance based on
      // the speed of sound (centimeters traveled based on the
      // elapsed microseconds. Divide by 2 since this is an
      // echo (the trigger pin is right next to us).
      //
      long duration = pulseIn(_echoPin, HIGH);
      int distance = duration * 0.034 / 2; // speed of sound

      return distance;
    }

    bool hasFired() {
      
      int distanceCM = getDistanceCM();

      //
      // This should not be possible.
      // Perhaps it is disconnected?
      //
      if (distanceCM == 0) {
        return false;
      }

      bool triggered = distanceCM < _triggerDistanceCM;
      unsigned long now = millis();
      bool fire = false;
      
      if (triggered && (now - _lastTimeFired > 500)) {
        fire = true;
        _lastTimeFired = now;
      }

      return fire;
    }
    
  private:
};

#endif

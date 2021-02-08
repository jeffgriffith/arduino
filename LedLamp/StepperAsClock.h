#ifndef STEPPER_AS_CLOCK
#define STEPPER_AS_CLOCK

#include <Stepper.h>

class StepperAsClock {
  private:
    const int STEPS_PER_REVOLUTION = 2048;
    const int MOTOR_SPEED = 10; // RPMs
    Stepper *_stepper;
    int _stepperLocation = 0; // [0-STEPS_PER_REVOLUTION)

  public:
    StepperAsClock(int pin1, int pin2, int pin3, int pin4) {
      _stepper = new Stepper(STEPS_PER_REVOLUTION, pin1, pin2, pin3, pin4);
    }
    
    void init() {
      _stepper->setSpeed(MOTOR_SPEED);
    }

    void adjustClock(float fractionOfClock) {
        //
      // Set the stepper motor location
      //
      Serial.print("Start location: ");
      Serial.println(_stepperLocation);
      
      Serial.print("New location as fraction of dial: ");
      Serial.println(fractionOfClock);
      
      int locationOnDial = (int)((float)STEPS_PER_REVOLUTION * fractionOfClock);
      Serial.print("New location in steps: ");
      Serial.println(locationOnDial);
    
      int stepAdjustment = locationOnDial - _stepperLocation;
      Serial.print("Effecdtive step adjustment: ");
      Serial.println(stepAdjustment);
    
      // Find the shortest direction to adjust
      if (abs(stepAdjustment) > STEPS_PER_REVOLUTION / 2) {
        if (stepAdjustment > 0)
          stepAdjustment -= STEPS_PER_REVOLUTION;
        else
          stepAdjustment += STEPS_PER_REVOLUTION;
      }

      Serial.print("Actual step adjustment:");
      Serial.println(stepAdjustment);
      
      //_stepper->step(-stepAdjustment);
      Serial.println("STEPPER DISABLED");
      _stepperLocation = locationOnDial;  
    }
};

#endif

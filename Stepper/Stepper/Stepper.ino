#include <Stepper.h>
int G_stepsPerRevolution = 2048;
Stepper G_stepper(G_stepsPerRevolution, 8,10,9,11);

void setup() {
  Serial.begin(9600);
  G_stepper.setSpeed(10); // RPMs
}

void loop() {
  G_stepper.step(2 * G_stepsPerRevolution);
  delay(500);
  G_stepper.step(-G_stepsPerRevolution * 2);
  delay(500);
}

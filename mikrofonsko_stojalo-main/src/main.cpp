#include <Arduino.h>
#include "motor_control.h"
#include "web_control.h"

void setup() {
  Serial.begin(115200);
  delay(2500);
  Serial.println("Starting up...");
  setupMotorPwm();
  setupWeb();
  setupMotorEncoder();
}

void loop() {
  handleWebClient();
  if (calibrating) {
    updateCalibration();
  } else {
    updateMotor();
  }
}
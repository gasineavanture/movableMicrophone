#pragma once

// Parametri motorja, ki jih spreminjajo spletni in serijski ukazi.
extern int hitrost;
extern float zeljena_poz;
extern float duty_mid;
extern int PWM_MAX;
extern int PWM_MIN;
extern int zgornja_limita;
extern int faktor;
extern float KP;
extern float KD;
extern bool stopRequested;
extern bool calibrating;

void setupMotorPwm();
void setupMotorEncoder();
void updateMotor();
void setMotorDuty(float duty);
void goHome();
float getCurrentPositionMm();
void updateCalibration();

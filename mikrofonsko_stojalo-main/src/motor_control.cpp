#include <Arduino.h>
#include <ESP32Encoder.h>
#include "config.h"
#include "motor_control.h"

// ================= ENKODER =================
ESP32Encoder encoder;

volatile long position = 0;

//hitrost(0-100), zeljena_poz(0-350mm), s-stop, r-reset pozicije-kalibracija oz goHome()
//position/faktor(za pretvorbo v mm)


// ================= REGULACIJA =================
int hitrost=30;
float zeljena_poz = 0;
float current_duty = 128.0;  // Start at middle
float target_duty = 0;
float duty_mid = 127; // 50% duty = midpoint
int  PWM_MAX=duty_mid+int(hitrost/faktor_hitrost*1.5);
int  PWM_MIN=duty_mid-int(hitrost/faktor_hitrost);

const float SOFT_START_RAMP = 0.065;  // sprememba duty na cikel....daljši zagon vecja st
const float FAST_STOP_RAMP = 0.1;
int nicelni_tok=1240;
float odstopanje=500;
int home_hitrost=50;
int spodnja_limita=-6000; //tam 1,5cm
int zgornja_limita=150000; //tam 35cm
bool stopRequested = false; 

// PD Controller
int faktor= 425;
float last_error = 0.0;
float KP = 0.01; //0.01 max
float KD = 0;   

unsigned long bellowError = 0;
long lastPos = 0;

unsigned long update_time=0;

// kalibracija
bool calibrating = false;
int calibrationStep = 0;
unsigned long calibrationTimer = 0;

// ================= MOTOR CONTROL =================
// PWM_MIN = full reverse
// duty_mid = stop
// PWM_MAX = full forward
void setMotorDuty(float duty) {
  /*if(analogRead(analogPin)>(nicelni_tok+odstopanje)||analogRead(analogPin)<(nicelni_tok-odstopanje)){
    ledcWrite(0, 0);
    ledcWrite(1, 0);
  }else {*/
    duty = constrain(duty, PWM_MIN, PWM_MAX);
    ledcWrite(0, (int)duty);
    ledcWrite(1, (int)(2 * duty_mid - duty));
  //}
}

void goHome(){
  if (calibrating) return;

  calibrating = true;
  calibrationStep = 0;
}

void updateCalibration() {
  if (!calibrating) return;

  if (calibrationStep == 0) {
    nicelni_tok = analogRead(analogPin);
    digitalWrite(ENABLE_PIN, HIGH);
    setMotorDuty(duty_mid - 30);
    calibrationStep = 1;
  }

  else if (calibrationStep == 1) {
    // iščemo spodnji fizični limit
    if (abs(nicelni_tok - analogRead(analogPin)) > 100) {
      setMotorDuty(duty_mid);
      encoder.setCount(spodnja_limita);
      zeljena_poz = 0;
      calibrationTimer = millis();
      calibrationStep = 2;
    }
  }

  else if (calibrationStep == 2) {
    if (millis() - calibrationTimer >= 500) {
      digitalWrite(ENABLE_PIN, HIGH);
      setMotorDuty(duty_mid + 40);
      calibrationStep = 3;
    }
  }

  else if (calibrationStep == 3) {
    if (encoder.getCount() >= long(zeljena_poz * faktor)) {
      setMotorDuty(duty_mid);
      digitalWrite(ENABLE_PIN, LOW);
      calibrating = false;
      Serial.println("Calibration finished");
    }
  }
}

void setupMotorPwm() {
  pinMode(analogPin, INPUT);
  pinMode(digitalniPin, INPUT);

  // PWM A (channel 0)
  ledcAttachPin(PWM_PIN_A, 0);
  ledcSetup(0, PWM_FREQ, PWM_RES);

  // pin B (channel 1)
  ledcAttachPin(PWM_PIN_B, 1);
  ledcSetup(1, PWM_FREQ, PWM_RES);

  // začetni duty 50%
  ledcWrite(0, 128);
  ledcWrite(1, 128);

  Serial.println("PWM setup done.");
}

void setupMotorEncoder() {
  // ENCODER
  encoder.attachFullQuad(ENC_A, ENC_B);
  encoder.setCount(0);
  pinMode(ENABLE_PIN, OUTPUT);

  goHome();
}

void updateMotor() {
  if (zeljena_poz > float(zgornja_limita) / faktor) zeljena_poz = float(zgornja_limita) / faktor;
  if (zeljena_poz < float(spodnja_limita) / faktor) zeljena_poz = float(spodnja_limita) / faktor;

  position = encoder.getCount(); 
  long napaka = long(zeljena_poz * faktor) - position; 
  /*
  // Zaščita pred preseganjem fizičnih limitov
  if(position > zgornja_limita) {
    position = zgornja_limita;
    setMotorDuty(duty_mid-10);  // Stop motor
    digitalWrite(ENABLE_PIN, LOW);
  }else if(position < 0) {
    position = 0;
    setMotorDuty(duty_mid+10);  // Stop motor
    digitalWrite(ENABLE_PIN, LOW);
  } else{*/
    
  if (stopRequested) {
    if (current_duty > duty_mid) {
      current_duty = max(current_duty - FAST_STOP_RAMP, duty_mid);
    }
    else if (current_duty < duty_mid) {
      current_duty = min(current_duty + FAST_STOP_RAMP, duty_mid);
    }

    setMotorDuty(current_duty);

    if (current_duty == duty_mid) {
      zeljena_poz = float(position) / faktor;
      target_duty = duty_mid;
      last_error = 0;
      bellowError = 0;

      digitalWrite(ENABLE_PIN, LOW);

      stopRequested = false;

      Serial.println("Motor stopped");
    }

    if ((millis() -update_time)>250 && lastPos != position) {
      Serial.print("Pos: ");
      Serial.print(position );
      Serial.print(" | Zeljena: ");
      Serial.print(zeljena_poz*faktor);
      Serial.print(" | Napaka: ");
      Serial.print(napaka);
      Serial.print(" | Duty: ");
      Serial.println(current_duty);
      Serial.print(" | P:");
      Serial.print(KP);
      Serial.print(" D:");
      Serial.print(KD);
      Serial.print(" |");
      Serial.println(analogRead(analogPin)*(3.3/4096));
      update_time=millis();
    }

    lastPos = position;

    return;
  }

  if (abs(napaka) < 1000) {
    // Close to target - ramp down before stopping
    if (bellowError == 0) bellowError = millis();
    else if (millis() - bellowError > 750) {
      // Soft stop - ramp down to mid
      if (current_duty > duty_mid) {
        current_duty = max(current_duty - SOFT_START_RAMP, duty_mid);
      } else if (current_duty < duty_mid) {
        current_duty = min(current_duty + SOFT_START_RAMP, duty_mid);
      } else {
        //disable motor
        last_error = 0;  // Reset derivative for next start
        setMotorDuty(duty_mid);
        digitalWrite(ENABLE_PIN, LOW);
      }
      setMotorDuty(current_duty);
    }
  } else {
    // PD control
    bellowError = 0;
    digitalWrite(ENABLE_PIN, HIGH);
    
    float error = napaka;  
    float error_derivative = error - last_error;  
    last_error = error;  

    // PD output
    float pd_output = KP * error + KD * error_derivative;
    target_duty = duty_mid + pd_output;
    target_duty = constrain(target_duty, PWM_MIN, PWM_MAX);
    
    // Soft ramp
    if (current_duty < target_duty) {
      current_duty = min(current_duty + SOFT_START_RAMP, target_duty);
    } else if (current_duty > target_duty) {
      current_duty = max(current_duty - SOFT_START_RAMP, target_duty);
    }
    
    setMotorDuty(current_duty);
  }

  if ((millis() -update_time)>250 && lastPos != position) {
    Serial.print("Pos: ");
    Serial.print(position );
    Serial.print(" | Zeljena: ");
    Serial.print(zeljena_poz*faktor);
    Serial.print(" | Napaka: ");
    Serial.print(napaka);
    Serial.print(" | Target Duty: ");
    Serial.println(target_duty);
    Serial.print(" | Current Duty: ");
    Serial.println(current_duty);
    Serial.print(" | P:");
    Serial.print(KP);
    Serial.print(" D:");
    Serial.print(KD);
    Serial.print(" |");
    Serial.println(analogRead(analogPin)*(3.3/4096));
    update_time=millis();
  }

  lastPos = position;
}

float getCurrentPositionMm() {
  return float(encoder.getCount()) / faktor;
}

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "secrets.h"
#include "json_storage.h"
#include "motor_control.h"
#include "web_control.h"
#include "web_page.h"

namespace {
WebServer server(80);
int activePersonId = -1;

// ================= HANDLERS =================
bool checkAuth() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return false;
  }

  return true;
}

void handleRoot() {
  if (!checkAuth()) return;
  server.send(200, "text/html", HTML_PAGE);
}

void handleMove() {
  if (!checkAuth()) return;
  if (calibrating) {
    server.send(409, "text/plain", "Calibration in progress");
    return;
  }

  activePersonId = -1;

  // premik za fiksno razdaljo
  if (server.hasArg("cm")) {
    float cm = server.arg("cm").toFloat();
    zeljena_poz += (cm * 10);

    if(zeljena_poz > int(zgornja_limita/faktor)){
      zeljena_poz = int(zgornja_limita/faktor);
    } else if(zeljena_poz < 0){
      zeljena_poz = 0;
    }

    stopRequested = false;
    digitalWrite(ENABLE_PIN, HIGH);
    server.send(200, "text/plain", "Moving by " + String(cm) + "cm");
    return;
  }

  // premik v kontinuiranem načinu
  if (server.hasArg("dir")) {
    String dir = server.arg("dir");
    int maxPoz = int(zgornja_limita / faktor);
    if (dir == "up") {
        zeljena_poz += 5;
        if (zeljena_poz > maxPoz) {
            zeljena_poz = maxPoz;
        }
        stopRequested = false;
        digitalWrite(ENABLE_PIN, HIGH);
        server.send(200, "text/plain", "Moving up");
        return;
    } else if (dir == "down") {
        zeljena_poz -= 5;
        if (zeljena_poz < 0) {
            zeljena_poz = 0;
        }
        stopRequested = false;
        digitalWrite(ENABLE_PIN, HIGH);
        server.send(200, "text/plain", "Moving down");
        return;
    }

    server.send(400, "text/plain", "Invalid dir parameter");
    return;
}
  server.send(400, "text/plain", "Missing cm or dir parameter");
}

void handleStop() {
  if (!checkAuth()) return;
  if (calibrating) {
    server.send(409, "text/plain", "Calibration in progress");
    return;
  }

  stopRequested = true;
  server.send(200, "text/plain", "OK: STOP");
}

void handleSaveJson() {
  if (!checkAuth()) return;

  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Request body is missing");
    return;
  }

  if (!saveJsonString(server.arg("plain"))) {
    server.send(500, "text/plain", "Could not save JSON");
    return;
  }
  server.send(200, "text/plain", "JSON saved");
}

void handleReadJson() {
  if (!checkAuth()) return;

  String json;
  if (!readJsonString(json)) {
    initializePeopleJson(json);
    if (!saveJsonString(json)) {
      server.send(500, "text/plain", "Could not initialize JSON in FLASH");
      return;
    }
  }
  server.send(200, "application/json", json);
}

void handlePosition() {
  if (!checkAuth()) return;

  String response = "{\"currentCm\":";
  response += String(getCurrentPositionMm() / 10.0, 1);
  response += ",\"targetCm\":";
  response += String(zeljena_poz / 10.0, 1);
  response += ",\"activePersonId\":";
  response += String(activePersonId);
  response += ",\"speed\":";
  response += String(hitrost);
  response += "}";
  server.send(200, "application/json", response);
}

void handlePerson() {
  if (!checkAuth()) return;
  if (calibrating) {
    server.send(409, "text/plain", "Calibration in progress");
    return;
  }

  if (!server.hasArg("id")) {
    server.send(400, "text/plain", "Missing id parameter");
    return;
  }

  int id = server.arg("id").toInt();
  long targetCounts;
  if (!readPersonHeight(id, targetCounts)) {
    server.send(404, "text/plain", "Person slot not found");
    return;
  }

  zeljena_poz = float(targetCounts) / faktor;
  activePersonId = id;
  stopRequested = false;
  digitalWrite(ENABLE_PIN, HIGH);
  server.send(200, "text/plain", "Moving to person " + String(id));
}

void handleUpdateJson() {
  if (!checkAuth()) return;

  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Request body is missing");
    return;
  }

  DynamicJsonDocument request(512);
  if (deserializeJson(request, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  int id = request["id"] | -1;
  const char* name = request["name"] | "";
  long height = request["height"] | -1L;
  String updatedJson;
  if (height < 0 || !updatePersonJson(id, name, height, updatedJson)) {
    server.send(400, "text/plain", "Invalid person data");
    return;
  }

  if (!saveJsonString(updatedJson)) {
    server.send(500, "text/plain", "Could not save JSON");
    return;
  }

  server.send(200, "application/json", updatedJson);
}

void handleMotorSpeed() {
  if (!checkAuth()) return;
  if (calibrating) {
    server.send(409, "text/plain", "Calibration in progress");
    return;
  }

  if (!server.hasArg("speed")) {
    server.send(400, "text/plain", "Missing speed parameter");
    return;
  }

  int speed = server.arg("speed").toInt();

  hitrost = speed;
  PWM_MAX=duty_mid+int(hitrost/faktor_hitrost*1.5);
  PWM_MIN=duty_mid-int(hitrost/faktor_hitrost);
    
  server.send(200, "text/plain", "Motor speed set to " + String(speed) + "%");
}

void handleCalibrate() {
  if (!checkAuth()) return;
  if (calibrating) {
    server.send(409, "text/plain", "Calibration in progress");
    return;
  }
  goHome();
  server.send(200, "text/plain", "Calibration started");
}

void getPeopleJson() {
  if (!checkAuth()) return;

  String json;
  if (!readJsonString(json)) {
    initializePeopleJson(json);
    if (!saveJsonString(json)) {
      server.send(500, "text/plain", "Could not initialize JSON in FLASH");
      return;
    }
  }
  server.send(200, "application/json", json);
}

void handleReset(){
  if (!checkAuth()) return;
  if (calibrating) {
    server.send(409, "text/plain", "Calibration in progress");
    return;
  }
  ESP.restart();
}
}

void setupWeb() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());
  Serial.println(WiFi.RSSI());

  // SERVER
  server.on("/", handleRoot);
  server.on("/move", handleMove);
  server.on("/stop", handleStop);
  server.on("/setSpeed", handleMotorSpeed);
  server.on("/json/save", handleSaveJson);
  server.on("/json/read", handleReadJson);
  server.on("/json/patch", handleUpdateJson);
  server.on("/position", handlePosition);
  server.on("/person", handlePerson);
  server.on("/calibrate", handleCalibrate);
  server.on("/people", getPeopleJson);
  server.on("/reset", handleReset);
  server.begin();
}

void handleWebClient() {
  server.handleClient();
}

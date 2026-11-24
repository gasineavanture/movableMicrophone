#pragma once
#include <Arduino.h>

bool saveJsonString(const String& json);
bool readJsonString(String& json);
bool initializePeopleJson(String& json);
bool updatePersonJson(int id, const String& name, long height, String& updatedJson);
bool readPersonHeight(int id, long& height);
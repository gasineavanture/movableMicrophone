#include "json_storage.h"
#include <ArduinoJson.h>
#include <Preferences.h>

namespace {
constexpr const char* STORAGE_NAMESPACE = "mic-stand";
constexpr const char* PEOPLE_KEY = "people";
}

void createEmptyPeople(JsonDocument& document) {
  JsonArray people = document.to<JsonArray>();
  for (int index = 0; index < 10; index++) {
    JsonObject person = people.createNestedObject();
    person["name"] = "";
    person["height"] = 0;
  }
}

bool saveJsonString(const String& json) {
  if (json.isEmpty()) {
    return false;
  }

  Preferences storage;
  if (!storage.begin(STORAGE_NAMESPACE, false)) {
    return false;
  }

  // Avoid unnecessary FLASH writes when the data has not changed.
  bool saved = storage.getString(PEOPLE_KEY, "") == json;
  if (!saved) {
    saved = storage.putString(PEOPLE_KEY, json) == json.length();
  }
  storage.end();
  return saved;
}

bool readJsonString(String& json) {
  Preferences storage;
  if (!storage.begin(STORAGE_NAMESPACE, true)) {
    return false;
  }

  json = storage.getString(PEOPLE_KEY, "");
  storage.end();
  return !json.isEmpty();
}

bool initializePeopleJson(String& json) {
  DynamicJsonDocument document(4096);
  createEmptyPeople(document);
  serializeJson(document, json);
  return true;
}

bool updatePersonJson(int id, const String& name, long height, String& updatedJson) {
  if (id < 0 || id >= 10) {
    return false;
  }

  DynamicJsonDocument document(4096);
  String json;
  JsonArray people;

  if (readJsonString(json) && !deserializeJson(document, json) && document.is<JsonArray>()) {
    people = document.as<JsonArray>();
  } else {
    createEmptyPeople(document);
    people = document.as<JsonArray>();
  }

  if (people.size() != 10) {
    return false;
  }

  JsonObject person = people[id];
  person["name"] = name;
  person["height"] = height;
  serializeJson(document, updatedJson);
  return true;
}

bool readPersonHeight(int id, long& height) {
  if (id < 0 || id >= 10) {
    return false;
  }

  String json;
  DynamicJsonDocument document(4096);
  if (!readJsonString(json) || deserializeJson(document, json) || !document.is<JsonArray>()) {
    return false;
  }

  JsonArray people = document.as<JsonArray>();
  if (people.size() != 10) {
    return false;
  }

  JsonVariant item = people[id]["height"];
  if (!item.is<long>() && !item.is<int>()) {
    return false;
  }

  height = item.as<long>();
  return true;
}

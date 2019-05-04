#include <Arduino.h>
#include <ArduinoJson.h>

DynamicJsonBuffer jsonBuffer;
JsonObject& json_object = jsonBuffer.createObject();
void setup() {
  Serial.begin(115200);

  json_object["top"]["next"] = "hello world";
  json_object.printTo(Serial);
}

void loop() {
    json_object.printTo(Serial);
}
#include <Arduino.h>
#include "IRmqttGlobal.h"
#include "IRmqttWiFi.h"
#include <FS.h>

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  setupWiFi();
  user_settings.loadSettings();
}

void loop() {
  udp_client.listenLoop();
  mqtt_client.Loop();
  // 更新系统时间
  system_now_time = millis();
}
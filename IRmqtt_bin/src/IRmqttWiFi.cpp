#include "IRmqttSerial.h"
#include <WiFiManager.h>

#define LED_PIN D4
#define INTERRUPT_PIN D2

void setupWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.setAutoReconnect(true);
  WiFiManager wifimanager;
  wifimanager.autoConnect();
  pinMode(INTERRUPT_PIN, INPUT);
  //attachInterrupt(INTERRUPT_PIN, resetWiFi, RISING);  
}

void resetWiFi() {
  WiFi.disconnect();
  delay(1000);
  ESP.restart();
}
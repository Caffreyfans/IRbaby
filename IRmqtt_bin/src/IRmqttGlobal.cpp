#include "IRmqttGlobal.h"
#include <Arduino.h>
#include "IRmqttSerial.h"

WiFiClient wifi_client;
udpClient udp_client;
userSettings user_settings;
mqttClient mqtt_client(wifi_client);
irmqttIR irmqtt_ir;
unsigned long system_setup_time = millis();
unsigned long system_now_time = millis();


String getChipId() {
  char chip_id[sizeof(uint32_t) + 2] ;
  utoa(ESP.getChipId(), chip_id, HEX);
  for (auto &i : chip_id) {
    i = toUpperCase(i);
  }
  return String(chip_id);
}

#include <FS.h>
#include "IRmqttSerial.h"
#include "IRmqttGlobal.h"

#define SAVE_SETTINGS_SUCCESS 1
#define SAVE_SETTINGS_FAILED 0

#define LOAD_SETTINGS_SUCCESS 1
#define LOAD_SETTINGS_FAILED 0

boolean userSettings::saveSettings(JsonObject& save_json) {
  
  if (save_json.containsKey("mqtt")) {
    _mqtt.host = save_json["mqtt"]["host"].asString();
    _mqtt.password = save_json["mqtt"]["password"].asString();
    _mqtt.port = save_json["mqtt"]["port"].asString();
    _mqtt.user = save_json["mqtt"]["user"].asString();
    mqtt_client.reloadSettings();
  }

  if (save_json.containsKey("recv_pin")) {
    _ir_pin.recv_pin = save_json["recv_pin"].asString();
    irmqtt_ir.reloadPin();
  }

  if (save_json.containsKey("send_pin")) {
    _ir_pin.send_pin = save_json["send_pin"].asString();
    irmqtt_ir.reloadPin();
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& config = jsonBuffer.createObject();
  JsonObject& mqtt = config.createNestedObject("mqtt");
  JsonObject& pin = config.createNestedObject("pin");
  pin["recv_pin"] = _ir_pin.recv_pin;
  pin["send_pin"] = _ir_pin.send_pin;
  mqtt["host"] = _mqtt.host;
  mqtt["port"] = _mqtt.port;
  mqtt["user"] = _mqtt.user;
  mqtt["password"] = _mqtt.password;

  File cache = SPIFFS.open("/config", "w");
  boolean FILE_FLAG = SAVE_SETTINGS_FAILED;
  if (cache) {
    config.printTo(cache);
    FILE_FLAG = SAVE_SETTINGS_SUCCESS;
  } else {
    DEBUGLN("Open /config error");
  }
  
  
  cache.close();
  return FILE_FLAG;
}

boolean userSettings::loadSettings() {
  DynamicJsonBuffer jsonBuffer;
  File cache = SPIFFS.open("/config", "r");
  boolean FILE_FLAG = LOAD_SETTINGS_FAILED;
  if (cache) {
    JsonObject& save_json = jsonBuffer.parseObject(cache);
    FILE_FLAG = LOAD_SETTINGS_SUCCESS;
    if (save_json.containsKey("mqtt")) {
      _mqtt.host = save_json["mqtt"]["host"].asString();
      _mqtt.password = save_json["mqtt"]["password"].asString();
      _mqtt.port = save_json["mqtt"]["port"].asString();
      _mqtt.user = save_json["mqtt"]["user"].asString();
      mqtt_client.reloadSettings();
    }
    if (save_json.containsKey("pin")) {
      _ir_pin.recv_pin = save_json["pin"]["recv_pin"].asString();
      _ir_pin.send_pin = save_json["pin"]["send_pin"].asString();
      irmqtt_ir.reloadPin();
    }
    LOGLN("[load the user settings]:");
    save_json.prettyPrintTo(LOG_ESP_PORT);
    
  } else {
    DEBUGLN("/config is not exsits");
  }
  cache.close();
  return FILE_FLAG;
}

MQTT userSettings::getMQTT() {
  return _mqtt;
}

IR_PIN userSettings::getIrPin() {
  return _ir_pin;
}

void userSettings::clear(String file_path) {
  SPIFFS.remove(file_path);
  LOGLN("SPIFFS format succeed, now to restart:");
  for (int i = 3; i >= 0; i--) {
    LOGF("time delay to restart: %d\n", i);
  }
  ESP.restart();
}
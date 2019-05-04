#ifndef IRMQTT_USER_SETTINGS_H_
#define IRMQTT_USER_SETTINGS_H_
#include <ArduinoJson.h>
#include <Arduino.h>
#include "IRmqttGlobal.h"

struct MQTT {
  String host;
  String port;
  String user;
  String password;
};

struct IR_PIN {
  String send_pin;
  String recv_pin;
};

class userSettings {
  public:
    boolean saveSettings(JsonObject& save_json);
    boolean loadSettings();
    MQTT getMQTT();
    IR_PIN getIrPin();
    void clear(String file_path);
  private:
    MQTT _mqtt;
    IR_PIN _ir_pin;
};
#endif  //IRMQTT_USER_SETTINGS_H_
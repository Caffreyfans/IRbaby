/******************************
 * 全局变量、函数声明
******************************/

#ifndef IRMQTT_GLOBAL_H_
#define IRMQTT_GLOBAL_H_

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "IRmqttMqtt.h"
#include "IRmqttUdp.h"
#include "IRmqttUserSettings.h"
#include "IRmqttIR.h"

extern userSettings user_settings;
extern mqttClient mqtt_client;
extern udpClient udp_client;
extern WiFiClient _wifi_client;
extern unsigned long system_setup_time;
extern unsigned long system_now_time;
extern irmqttIR irmqtt_ir;
extern String getChipId();


#endif  // IRMQTT_GLOBAL_H_
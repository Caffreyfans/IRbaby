#ifndef IRMQTT_MQTT_H_
#define IRMQTT_MQTT_H_

#include <PubSubClient.h>

class mqttClient {
  public:
    mqttClient(WiFiClient& wifi_client);
    boolean Connect();
    boolean checkSettings();
    void reloadSettings();  
    void Loop();
  private:
    WiFiClient* _wifi_client;
    PubSubClient* _mqtt_client;
    String _topic_head = "";
    const char* _host = nullptr;
    int _port = 1883;
    const char* _user = nullptr;
    const char* _password = nullptr;
    boolean _mqtt_lock;
};

void callback(char* topic, byte* payload, unsigned int length);
#endif
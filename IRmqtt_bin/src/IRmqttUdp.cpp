#include <ESP8266WiFi.h>
#include "IRmqttSerial.h"
#include "IRmqttGlobal.h"
#include "IRmqttIR.h"

#define UDP_PORT 4210
#define CONNECTED 1
#define CONNECT_FAILED 0

#define SUCCESS 1
#define FAILED 0

udpClient::udpClient() {
  _udp_port = UDP_PORT;
  _wifiudp.begin(_udp_port);
}

void udpClient::listenLoop() {

  // 对接收到的 UDP 报文，并对报文进行分析处理
  int packetSize = _wifiudp.parsePacket();
  if (packetSize) {
    // 清除 json buffer，放在这里只是利用这里的循环机制,不属于这里的逻辑
    jsonBuffer.clear();

    int len = _wifiudp.read(_incomingPacket, UDP_PACKET_SIZE);
    if (len > 0)
    {
      _incomingPacket[len] = '\0';
    }
    String tmp(_incomingPacket);
    JsonObject& receive_json = jsonBuffer.parseObject(tmp);
    JsonObject& ret_msg = jsonBuffer.createObject();
    ret_msg["ret"] = jsonBuffer.createObject();
    DEBUG("[UDP receive]: ");
    receive_json.printTo(DEBUG_ESP_PORT);
    DEBUGLN();

    // 上传 IP 到请求端
    if (receive_json.containsKey("upload_ip")) {
      String remote_address = receive_json["upload_ip"];
      String chip_id = getChipId();
      DEBUG(chip_id);
      ret_msg["ret"]["device"] = jsonBuffer.createObject();
      ret_msg["ret"]["device"][chip_id] = WiFi.localIP().toString();
      IPAddress remote_ip;
      if (remote_ip.fromString(remote_address)) {
        returnMsg(&ret_msg, remote_ip);
      }
    }

    // 清除文件
    if (receive_json.containsKey("clear")) {
      String file_path = receive_json["clear"];
      user_settings.clear(file_path);
    }

    // 保存上传设置
    if (receive_json.containsKey("save")) {
      JsonObject& content_json = receive_json["save"].as<JsonObject &>();
      user_settings.saveSettings(content_json);

      if (content_json.containsKey("mqtt")) { 
        if (mqtt_client.Connect() == true) {
          ret_msg["ret"]["mqtt"] = CONNECTED;
        } else {
          ret_msg["ret"]["mqtt"] = CONNECT_FAILED;
        }
        returnMsg(&ret_msg, _wifiudp.remoteIP());
      }
    }

    // 红外测试
    if (receive_json.containsKey("ir")) {
      if (receive_json["ir"].asObject().containsKey("send")) {
        if (receive_json["ir"]["send"]["test"]) {
          String device_type = receive_json["ir"]["send"]["device_type"].asString();
          String index_id = receive_json["ir"]["send"]["index_id"].asString();
          irmqtt_ir.downloadFile(true, index_id);
          String topic = "/IRmqtt/ESP8266/" + device_type + "/" + index_id + "/" + "mode";
          String command = "cool";
          irmqtt_ir.sendIR(topic, command, true);
        }
        else {
          String index_id = receive_json["ir"]["send"]["index_id"].asString();
          irmqtt_ir.downloadFile(false, index_id);
        }
      }

      if (receive_json["ir"].asObject().containsKey("recv")) {
        if (receive_json["ir"]["recv"]["recorder"]) {
          ret_msg["ret"]["ir"] = jsonBuffer.createObject();
          ret_msg["ret"]["ir"]["get_raw"] = irmqtt_ir.recvIR();
          returnMsg(&ret_msg, _wifiudp.remoteIP());
        }

        if (receive_json["ir"]["recv"]["send"]) {
          String topic("/IRmqtt/ESP8266/custom");
          DEBUGLN("custom");
          irmqtt_ir.sendIR(topic, "test", false);
        }

        if (receive_json["ir"]["recv"].asObject().containsKey("save")) {
          irmqtt_ir.saveCustom(receive_json["ir"]["recv"]["save"]);
        }

        // if (receive_json["ir"]["recv"].asObject().containsKey("read")) {
        //   irmqtt_ir.readCustom(receive_json["ir"]["recv"]["read"]);
        // }
      }

    }
  }
}

int udpClient::returnMsg(JsonObject* msg_ptr, IPAddress remote_ip) {
  _wifiudp.beginPacket(remote_ip, _udp_port);
  msg_ptr->printTo(_wifiudp);
  return _wifiudp.endPacket();
}
/****************************************
 * Description: UDP 报文发送、接收和处理
*****************************************/

#ifndef IRMQTT_UDP_H_
#define IRMQTT_UDP_H_

#include <WiFiUdp.h>
#include <ArduinoJson.h>
#define UDP_PACKET_SIZE 255

class udpClient {
  public:
    udpClient();
    void listenLoop();
    int returnMsg(JsonObject* msg_ptr, IPAddress remote_ip);

  private:
    WiFiUDP _wifiudp;
    uint16_t _udp_port;
    char _incomingPacket[UDP_PACKET_SIZE];
    DynamicJsonBuffer jsonBuffer;
};
#endif // IRMQTT_UDP_H_
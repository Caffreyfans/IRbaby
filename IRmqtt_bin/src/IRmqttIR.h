#ifndef IRMQTT_IR_H_
#define IRMQTT_IR_H_

#include <Arduino.h>
#include <IRsend.h>
#include <IRrecv.h>
#include "../lib/Irext/include/ir_decode.h"

typedef struct {
  String device_type;
  String filename;
  String set_type;
} stringTopic;

typedef struct {
  uint16_t* raw_buffer;
  uint16_t length;
} recvRaw;

class irmqttIR {
  public:
    irmqttIR();
    boolean downloadFile(boolean test, String index_id);
    boolean sendIR(String topic, String command, boolean test);
    boolean recvIR();
    boolean saveCustom(String filename);
    boolean readCustom(String filename);
    void reloadPin();
  protected:
    boolean sendAC(String filename, boolean test);
    boolean sendCustom(String filename);
    int coverStringToEnum(String cmd, String type);
  private:
    t_remote_ac_status _ac_status;
    IRsend* _irsend = nullptr;
    IRrecv* _irrecv = nullptr;
    recvRaw _recv_raw;
    decode_results _results;
};
#endif
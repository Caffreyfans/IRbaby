#include "IRmqttIR.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include "IRmqttSerial.h"
#include "IRmqttGlobal.h"
#include <IRutils.h>

#define DOWNLOAD_SUCCESS 1
#define DOWNLOAD_FAILED 0
#define SENDRAW_SUCCESS 1
#define SENDRAW_FAILED 0
#define DOWNLOAD_HEAD "http://irext-debug.oss-cn-hangzhou.aliyuncs.com/irda_"
#define TMP_PATH "/bin/tmp/"
#define SAVE_PATH "/bin/save/"

#define IR_FREQUENCY 38 // 红外载波频率

irmqttIR::irmqttIR()
{
  _ac_status = {
      // 设置空调默认状态
      AC_POWER_ON,
      AC_TEMP_24,
      AC_MODE_COOL,
      AC_SWING_ON,
      AC_WS_AUTO,
      1,
      0,
      0};
  _recv_raw.length = 0;
  _recv_raw.raw_buffer = nullptr;
  reloadPin();
}

boolean irmqttIR::downloadFile(boolean test, String index_id)
{
  HTTPClient http_client;
  String download_url = DOWNLOAD_HEAD;
  download_url += index_id;
  boolean DOWNLOAD_FLAG = DOWNLOAD_SUCCESS;
  String file_path;
  if (test)
  {
    file_path += TMP_PATH;
  }
  else
  {
    file_path += SAVE_PATH;
  }
  file_path += index_id;
  if (!SPIFFS.exists(file_path))
  {
    File cache = SPIFFS.open(file_path, "w");
    File *filestream = &cache;
    if (cache)
    {
      DEBUG("Download url: ");
      DEBUGLN(download_url);
      http_client.begin(download_url);
      int http_code = http_client.GET();
      if (http_code == HTTP_CODE_OK)
      {
        DEBUGF("[Download success]: %s\n", index_id.c_str());
        http_client.writeToStream(filestream);
      }
      else
      {
        SPIFFS.remove(file_path);
        DOWNLOAD_FLAG = DOWNLOAD_FAILED;
        DEBUGF("[Download error]: %s\n", index_id.c_str());
        DEBUGF("[Error code]: %d\n", http_code);
      }
    }
    else
    {
      DOWNLOAD_FLAG = DOWNLOAD_FAILED;
      DEBUGF("[Open file error]: %s\n", file_path.c_str());
    }
    cache.close();
    http_client.end();
  }
  else
  {
    DEBUGF("%s already exsits\n", file_path.c_str());
  }
  return DOWNLOAD_FLAG;
}

boolean irmqttIR::sendIR(String topic, String command, boolean test)
{
  // topic example: /IRmqtt/ESP8266/ac/filename/mode
  // command example: cool
  int str_start = 0, str_end = 0, count = 0;
  String tmp;
  stringTopic string_topic;

  do
  {
    str_start = topic.indexOf("/", str_start);
    str_end = topic.indexOf("/", str_start + 1);
    tmp = topic.substring(str_start + 1, str_end);
    str_start = str_end;
    count++;
    switch (count)
    {
    case 3:
      string_topic.device_type = tmp;
      break;
    case 4:
      string_topic.filename = tmp;
      break;
    case 5:
      string_topic.set_type = tmp;
      break;
    }
  } while (str_start < topic.length());

  // 处理空调发射情况
  if (string_topic.device_type == "ac")
  {
    if (string_topic.set_type == "mode")
    {
      if (command == "off")
      {
        _ac_status.ac_power = AC_POWER_OFF;
      }
      else
      {
        _ac_status.ac_power = AC_POWER_ON;
        _ac_status.ac_mode = (t_ac_mode)coverStringToEnum(command, string_topic.set_type);
      }
    }

    if (string_topic.set_type == "temperature")
    {
      int tmp = command.toInt();
      _ac_status.ac_temp = t_ac_temperature(tmp - 16);
    }

    if (string_topic.set_type == "fan")
    {
      _ac_status.ac_wind_speed = (t_ac_wind_speed)coverStringToEnum(command, string_topic.set_type);
    }

    if (string_topic.set_type == "swing")
    {
      _ac_status.ac_wind_dir = (t_ac_swing)coverStringToEnum(command, string_topic.set_type);
    }

    sendAC(string_topic.filename, test);
  }

  if (string_topic.device_type == "custom")
  {

    sendCustom(command);
  }
}

boolean irmqttIR::sendAC(String filename, boolean test)
{

  DEBUGLN("*** ac_status ***");
  DEBUGF("mode %d\n", _ac_status.ac_mode);
  DEBUGF("power %d\n", _ac_status.ac_power);
  DEBUGF("temp %d\n", _ac_status.ac_temp);
  DEBUGF("wind_speed %d\n", _ac_status.ac_wind_speed);
  DEBUGF("display %d\n", _ac_status.ac_display);
  DEBUGF("swing %d\n", _ac_status.ac_wind_dir);

  String open_path;
  if (test)
  {
    open_path += TMP_PATH;
  }
  else
  {
    open_path += SAVE_PATH;
  }
  open_path += filename;
  boolean flag = false;
  if (SPIFFS.exists(open_path))
  {
    File f = SPIFFS.open(open_path, "r");
    if (f)
    {
      UINT16 content_length = f.size();
      if (content_length != 0)
      {
        UINT16 user_data[512];
        UINT8 *content = (UINT8 *)malloc(content_length * sizeof(UINT8));
        f.seek(0L, fs::SeekSet);
        f.readBytes((char *)content, content_length);
        INT8 ret = ir_binary_open(IR_CATEGORY_AC, 1, content, content_length);
        int length = ir_decode(2, user_data, &_ac_status, 0);
        for (int i = 0; i < length; i++)
        {
          DEBUGF("%d ", user_data[i]);
        }
        ir_close();
        free(content);
        if (_irsend != nullptr)
        {
          _irsend->sendRaw(user_data, length, IR_FREQUENCY);
        }
        else
        {
          DEBUGLN("_irsend is nullptr");
        }
        flag = true;
      }
    }
    f.close();
  }
  return flag;
}

boolean irmqttIR::sendCustom(String filename)
{
  if (filename != "")
  {
    readCustom(filename);
    DEBUGLN("read from file");
  }
  DEBUGLN("send custom");
  _irsend->sendRaw(_recv_raw.raw_buffer, _recv_raw.length, IR_FREQUENCY);
}

int irmqttIR::coverStringToEnum(String cmd, String type)
{
  String swing_str[] = {"on", "off"};
  String fan_speed[] = {"auto", "low", "medium", "high"};
  String mode_str[] = {"cool", "heat", "auto", "fan", "dry"};
  if (type == "mode")
    for (int i = 0; i < sizeof(mode_str) / sizeof(mode_str[0]); i++)
    {
      if (cmd == mode_str[i])
        return i;
    }

  if (type == "swing")
    for (int i = 0; i < sizeof(swing_str) / sizeof(swing_str[0]); i++)
    {
      if (cmd == swing_str[i])
        return i;
    }

  if (type == "fan")
    for (int i = 0; i < sizeof(fan_speed) / sizeof(fan_speed[0]); i++)
    {
      if (cmd == fan_speed[i])
        return i;
    }
}

void irmqttIR::reloadPin()
{
  const uint16_t kCaptureBufferSize = 1024;
  const uint8_t kTimeout = 50;
  if (_irsend != nullptr)
  {
    delete _irsend;
  }
  if (_irsend != nullptr)
  {
    delete _irrecv;
  }
  IR_PIN ir_pin = user_settings.getIrPin();
  if (ir_pin.send_pin != "")
  {
    _irsend = new IRsend(ir_pin.send_pin.toInt());
    _irsend->begin();
  }
  if (ir_pin.recv_pin != "")
  {
    _irrecv = new IRrecv(ir_pin.recv_pin.toInt(), kCaptureBufferSize, kTimeout, true);
    _irrecv->enableIRIn();
  }
}

boolean irmqttIR::recvIR()
{
  unsigned long start = millis();
  unsigned long end = millis();
  while (end - start < 5000)
  {
    if (_irrecv->decode(&_results))
    {
      // Dump data
      String output;
      for (uint16_t i = 1; i < _results.rawlen; i++)
      {
        uint32_t usecs;
        for (usecs = _results.rawbuf[i] * kRawTick; usecs > UINT16_MAX;
             usecs -= UINT16_MAX)
        {
          output += uint64ToString(UINT16_MAX);
          if (i % 2)
            output += ", 0,  ";
          else
            output += ",  0, ";
        }
        output += uint64ToString(usecs, 10);
        if (i < _results.rawlen - 1)
          output += ", "; // ',' not needed on the last one
        if (i % 2 == 0)
          output += " "; // Extra if it was even.
      }

      _recv_raw.length = getCorrectedRawLength(&_results);
      if (_recv_raw.raw_buffer != nullptr)
        free(_recv_raw.raw_buffer);
      _recv_raw.raw_buffer = reinterpret_cast<uint16_t *>(malloc(_recv_raw.length * sizeof(uint16_t)));
      uint16_t start_from = 0;
      uint16_t index = 0;
      int count = 0;
      do
      {
        index = output.indexOf(',', start_from);
        _recv_raw.raw_buffer[count] = output.substring(start_from, index).toInt();
        start_from = index + 1;
        count++;
      } while (count < _recv_raw.length);
      _irrecv->resume();

      for (int i = 0; i < _recv_raw.length; i++)
      {
        DEBUGF("%d ", _recv_raw.raw_buffer[i]);
      }
      return true;
    }
    end = millis();
    yield();
  }
  return false;
}

boolean irmqttIR::saveCustom(String filename)
{
  String file_path;
  file_path += SAVE_PATH;
  file_path += filename;
  File cache = SPIFFS.open(file_path, "w");
  if (cache)
  {
    DEBUGLN("write success");
    cache.write((const char *)&_recv_raw.length, sizeof(uint16_t));
    cache.write((const char *)_recv_raw.raw_buffer, sizeof(uint16_t) * _recv_raw.length);
  }
  cache.close();
}

boolean irmqttIR::readCustom(String filename)
{
  String open_path;
  open_path += SAVE_PATH;
  open_path += filename;
  if (_recv_raw.raw_buffer != nullptr)
  {
    free(_recv_raw.raw_buffer);
    _recv_raw.raw_buffer = nullptr;
    _recv_raw.length = 0;
  }
  File cache = SPIFFS.open(open_path, "r");
  if (cache)
  {
    cache.readBytes((char *)&_recv_raw.length, sizeof(uint16_t));
    DEBUGLN(_recv_raw.length);
    _recv_raw.raw_buffer = reinterpret_cast<uint16_t *>(malloc(_recv_raw.length * sizeof(uint16_t)));
    if (_recv_raw.raw_buffer == nullptr)
      DEBUGLN("nullptr");
    cache.read((uint8_t *)_recv_raw.raw_buffer, sizeof(uint16_t) * _recv_raw.length);
  }
  DEBUGLN();
  DEBUGF("length = %d\n", _recv_raw.length);
  for (int i = 0; i < _recv_raw.length; i++)
  {
    DEBUGF("%d ", _recv_raw.raw_buffer[i]);
  }
  cache.close();
}
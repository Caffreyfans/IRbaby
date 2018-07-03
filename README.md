# IRext

`固件` `app`

---

- [ ] ESP上固件
- [ ] 手机端app

任务计划：
先完成ESP上的固件，然后通过调试工具完成调试，之后再写手机端app。

```seq
Title: 功能原理流程图
APP->ESP8266: Smartconfig配网
ESP8266->APP: 返回IP
APP->ESP8266: 发送要下载的bin文件信息
ESP8266->ESP8266: HTTP下载并保存该文件
MQTT->>ESP8266: 发送设备控制状态
ESP8266->ESP8266: 根据发送的设备状态解码得到红外时序码
ESP8266->>受控设备: 发送红外信号
```

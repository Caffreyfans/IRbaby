# IRext

`固件` `app`

---
## 什么是Irmqtt?
Irmqtt是 [Irext](https://github.com/irext/irext-core)的一个衍生项目。
Irmqtt的作用是帮助你只需要一个ESP8266系列的模块和一个红外二级管就可以控制家里红外设备。
它的优势在于：
  > * 成本低廉只需要一个ESP8266模块或开发板如NodeMcu)和一个红外二级管
  > * 不需要人为学习红外码快速匹配到电器
  > * 有mqtt接口可以方便接入homeassistant

## 开发状态

Irmqtt 仍然处于前期开发阶段，未经充分测试与验证，不推荐用于生产环境。

**目前的交互协议可能随时改变，不保证向后兼容，升级新版本时需要注意公告说明同时升级服务端和客户端。**

## 说明
现已完成，当前版本只支持空调

## 时序图
![Irmqtt时序图](https://github.com/Caffreyfans/IRmqtt/blob/master/src/Irmqtt.svg)
## 使用方法
见[演示视频](https://v.youku.com/v_show/id_XMzc5MTAxMDYxNg==.html?x&sharefrom=android&sharekey=174a4f326234a5ca62e09327ce0931682)

## 发射管连接图
![红外连接图](https://camo.githubusercontent.com/8b4e10e4d829d417cc29a5d5a563f650fb4beabf/687474703a2f2f667269747a696e672e6f72672f6d656469612f667269747a696e672d7265706f2f70726f6a656374732f652f657370383236362d69722d7472616e736d69747465722f696d616765732f49522532305472616e736d69747465725f62622e706e67)

也可以尝试不用三级管，直接连接

## HomeAssistant配置文件
```yaml
climate:
  - platform: mqtt
    name: Study
    modes:
      - cool
      - heat
      - auto
      - fan
      - dry
      - off
    swing_modes:
      - on
      - off
    fan_modes:
      - high
      - medium
      - low
      - auto
    power_command_topic: "irmqtt/ac/power/set"
    mode_command_topic: "irmqtt/ac/mode/set"
    temperature_command_topic: "irmqtt/ac/temperature/set"
    fan_mode_command_topic: "irmqtt/ac/fan/set"
    swing_mode_command_topic: "irmqtt/ac/swing/set"
    min_temp: 16
    max_temp: 30
```

## 特别感谢
[Strawmanbobi](https://github.com/strawmanbobi) Irext开源库的作者，给予我技术和精神上的支持
<img src="http://irext.net/images/bobi_qr.png" align="left" height="160" width="160">

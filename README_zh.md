


# IRmqtt
[![forthebadge](https://forthebadge.com/images/badges/built-with-love.svg)](https://forthebadge.com)

[README](README.md) | [中文文档](README_zh.md)

---
## 什么是IRmqtt?
IRmqtt使用[Irext](https://github.com/irext/irext-core)开源红外库，IRext 提供数以万计的红外设备遥控编码。
IRmqtt帮助你通过ESP系列模块，只需对其进行简单设置就可以在HomeAssisant里的所有红外控制设备。

## 开发状态
v1.0 只支持空调设备
**IRmqtt目前仍处于开发阶，目前的交互协议可能随时改变，不保证向后兼容，升级新版本时需要注意公告说明同时升级固件和移动端。**

## 时序图
![IRmqtt时序图](https://github.com/Caffreyfans/IRmqtt/blob/master/src/IRmqtt_zh.svg)

## 使用方法
自己编译或者直接下载编议好的固件与apk安装包，将固件刷入到ESP系列模块，并在手机上安装好apk。
见[演示视频](https://tv.sohu.com/upload/static/share/share_play.html#105448709_334912730_0_9001_0)

## 发射管连接图
![红外连接图](https://camo.githubusercontent.com/8b4e10e4d829d417cc29a5d5a563f650fb4beabf/687474703a2f2f667269747a696e672e6f72672f6d656469612f667269747a696e672d7265706f2f70726f6a656374732f652f657370383236362d69722d7472616e736d69747465722f696d616765732f49522532305472616e736d69747465725f62622e706e67)

也可以尝试不用三级管，直接连接。红外二级管长引脚接gpio，短脚接地。

## HomeAssistant配置文件
请不要修改topic
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
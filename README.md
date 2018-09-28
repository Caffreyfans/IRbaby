


# IRmqtt
[![forthebadge](https://forthebadge.com/images/badges/built-with-love.svg)](https://forthebadge.com)

[README](README.md) | [中文文档](README_zh.md)

---
## What's IRmqtt?
IRmqtt uses the [Irext](https://github.com/irext/irext-core) open source infrared library, and IRext provides tens of thousands of infrared device remote control codes.

IRmqtt helps you pass the ESP series of modules, all of which can be easily set up in [Infrared Control Devices] in [HomeAssisant] (https://www.home-assistant.io/).

## Status
V1.0 only supports air conditioning equipment

**IRmqtt is still in the development stage. The current interactive protocol may change at any time. It is not guaranteed to be backward compatible. When upgrading the new version, you need to pay attention to the announcement and upgrade the firmware and app.**

## Timing diagram
![IRmqtt Timing diagram](https://github.com/Caffreyfans/IRmqtt/blob/master/src/IRmqtt_en.svg)

## Usage
Compile or directly download the compiled firmware and apk installation package, brush the firmware into the ESP series module, and install the apk on the mobile phone.

## Infrared tube connection diagram
![Infrared tube connection diagram图](https://camo.githubusercontent.com/8b4e10e4d829d417cc29a5d5a563f650fb4beabf/687474703a2f2f667269747a696e672e6f72672f6d656469612f667269747a696e672d7265706f2f70726f6a656374732f652f657370383236362d69722d7472616e736d69747465722f696d616765732f49522532305472616e736d69747465725f62622e706e67)

You can also try to connect directly without a three-stage tube. The infrared diode has a long pin connected to gpio and a short pin to gnd.

## HomeAssistant configuration
Please do not modify the topic
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

## Special thanks to
[Strawmanbobi](https://github.com/strawmanbobi) The author of the Irext open source library gives me technical and spiritual support
<img src="http://irext.net/images/bobi_qr.png" align="left" height="160" width="160">
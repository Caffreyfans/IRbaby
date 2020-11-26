## IRbaby
**([中文版](README.md))|([English](README_en.md))**
**IRbaby use [IRext](https://irext.net/) private IR library, it provide tens of thousands of infrared device remote control codes. IRbaby is an ESP8266 complete IR control program, with hardware support to achieve a universal infrared remote control effect similar to that on the market. And it can be quickly deployed in HomeAssistant by simply setting it up.** 

[![forthebadge](https://forthebadge.com/images/badges/built-with-love.svg)](https://forthebadge.com)

---

## Features

* IRext powerful IR libaray

* support MQTT control api

* support UDP control api

* support record IR raw signal

* offline decoding

* Homeassistant mqtt auto discovery  

  ## Architecture

  ![struction](http://irbaby.caffreyfans.top/src/architecture.svg)

  ## Setup

  > 1. **Download ESP8266 firmware and flash to device. [IRbaby-firmware](https://github.com/Caffreyfans/IRbaby-firmware/releases)**
  > 2. **Power on the device, search for the signal connected to the `ESP etc.` on the mobile phone, and enter `192.168.4.1` in the browser to set up the network**
  > 3. **Download `Android` client and run it on phone, set the MQTT and IR receiver and transceiver pins of the device**

  ## Connect to Homeassistant

|                                                              |                                                              |                                                           |
| ------------------------------------------------------------ | ------------------------------------------------------------ | --------------------------------------------------------- |
| ![discovery device](http://irbaby.caffreyfans.top/src/discovery.jpg) | ![setup device](http://irbaby.caffreyfans.top/src/device_setting.jpg) | ![add appliance](http://irbaby.caffreyfans.top/src/select.jpg) |
| ![parse appliance](http://irbaby.caffreyfans.top/src/parse.jpg)     | ![already appliance](http://irbaby.caffreyfans.top/src/main.jpg)      | ![export config](http://irbaby.caffreyfans.top/src/mqtt.jpg)   |

## material
### IR receiver optional (if recording function is required)
|                                                           |                                                             |
| --------------------------------------------------------- | ----------------------------------------------------------- |
| ![Nodemcu](http://irbaby.caffreyfans.top/src/nodemcu.jpg) | ![IR](http://irbaby.caffreyfans.top/src/ir_led.jpg) |
![IR receiver](http://irbaby.caffreyfans.top/src/ir_receiver.jpg) | ![Triode](http://irbaby.caffreyfans.top/src/transistor.jpg) |

## About line connection

![接线](http://irbaby.caffreyfans.top/src/connect.jpg)

Remarks: When connecting the infrared diode, you can also try to connect directly without the tertiary tube. The long pin of the infrared diode is connected to GPIO, and the short pin is grounded. If the infrared receiver is connected to the module as shown on the icon. The infrared receiver is not necessary, if you do not use the code recording function, you can ignore the infrared receiver. As long as you have an infrared transmitter, an ESP8266 and an Android phone, you can try this project. In addition, the current project only supports air-conditioning control, other functions are temporarily not supported, and will be added later. The control client currently only supports Android, and the cross-platform client is also being added later



## Special thanks to
[Strawmanbobi](https://github.com/strawmanbobi) the author of IRext private IR library, give me technical and spiritual support.
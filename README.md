# IRmqtt Firmware

IRmqtt 使用 [Irext](https://github.com/irext/irext-core) 开源红外库，IRext 提供数以万计的红外设备遥控编码。
IRmqtt 帮助你使用 ESP8266 系列模块，只需对其进行简单设置就可以快速在 [HomeAssisant](https://www.home-assistant.io) 里的控制红外设备。

[![forthebadge](https://forthebadge.com/images/badges/built-with-love.svg)](https://forthebadge.com)

---

## 特点

* *Irext* 强大开源红外码库
* 支持大多数基于 ESP8266 的板子
* *MQTT* 支持
* 支持录码

## 版本信息
---
> **2019-05-06**<br />
> v2.1 修复无法获取设备问题，先直接运行 IRmqtt-tool.exe 不再依赖 python3 环境

---
> **2019-05-04**<br />
> v2.0 增加录码功能  

---
> **2018-08-04**<br />
> v1.0 只支持空调设备  

---
> **IRmqtt目前仍处于开发阶，目前的交互协议可能随时改变，不保证向后兼容，升级新版本时需要注意公告说明同时升级固件和客户端。**

## 材料
### 红外接收头可选
|||
|---|---|
|![Nodemcu](src/nodemcu.jpg) | ![红外二级管](src/ir_led.jpg) |
![红外接收头](src/ir_receiver.jpg) | ![三级管](src/transistor.jpg) |


## 使用方法

1. 到 **releases** 页面下载最新文件
2. 利用 **FlashESP8266.exe** 刷写 **IRmqtt.bin** 到你的 **ESP8266** 模块
3. 通过任意移动端（笔记本、平板、手机）连接 **ESP** 开头的 **SSID**，然后浏览器进入 **192.168.4.1** 给模块配置网络
<!--4. 利用 **python3** 环境执行 **IRmqtt-tool.py** 给你的模块进行设置-->

![时序图](src/sequence.jpg)

## 关于连线
![红外连接图](https://camo.githubusercontent.com/8b4e10e4d829d417cc29a5d5a563f650fb4beabf/687474703a2f2f667269747a696e672e6f72672f6d656469612f667269747a696e672d7265706f2f70726f6a656374732f652f657370383236362d69722d7472616e736d69747465722f696d616765732f49522532305472616e736d69747465725f62622e706e67)

![接线](https://raw.githubusercontent.com/Caffreyfans/IRmqtt/dev/src/connect.jpg)

备注：红外二级管连接的时候也可以尝试不用三级管，直接连接。红外二级管长引脚接gpio，短脚接地。红外接收头的话就照着上图标示的那样与模块连接。

## 建议观看演示视频
[视频地址](https://www.bilibili.com/video/av51492029/)

## 特别感谢
[Strawmanbobi](https://github.com/strawmanbobi) Irext开源库的作者，给予我技术和精神上的支持
<img src="http://irext.net/images/bobi_qr.png" align="left" height="160" width="160">
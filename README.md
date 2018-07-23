# IRext

`固件` `app`

---
## 任务计划：
> 先完成ESP上的固件，然后通过调试工具完成调试，之后再写手机端app。

## 任务进度
> - [ ] 固件部分
  - [ ] 实现解码API调用
  - [ ] 根据mqtt 消息发送的设备状态得到相应的红外码（实现mqtt 接口)
  - [ ] 对接APP端UDP数据
  - [ ] 文件下载好之后返回APP端下载成功消息
> - [ ] APP端
  - [ ] 将`ESP-TOUCH APP`嵌入到本APP中
  - [ ] 选择好设备后发送下载文件命令到ESP设备
  - [ ] 实现mqtt 配置界面，用户完成后发送给ESP设备
---
## 功能原理流程图
> ![功能原理流程图](https://www.caffreyfans.top/src/Irext/irext_theory.svg)

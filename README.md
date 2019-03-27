##  qcloud-iot-sdk-tecent-at-based 

### 介绍

qcloud-iot-sdk-tecent-at-based 面向使用支持腾讯AT指令的模组(2/3/4/5G、NB、WIFI等)接入腾讯物联网平台的终端设备开发者，典型应用场景为MCU+腾讯定制AT模组，SDK完成实现了MCU和模组数据交互的AT框架，并基于AT框架配合腾讯AT指令，实现了MQTT、影子及数据模板的功能，同时提供了示例sample。指令集定义见doc目录《腾讯云IoT AT指令集.pdf》。开发者只需根据所选的通信总线（I2C/SPI/UART/7816），适配实现物理层总线的初始化、收、发、超时判断接口即可，适配层接口单独剥离在port目录。

### qcloud-iot-sdk-tecent-at-based软件架构
![](https://i.imgur.com/E6qiUrO.jpg)
//待补充

### 目录结构

| 名称            | 说明 |
| ----            | ---- |
| docs            | 文档目录，包含腾讯AT指令集定义 |
| port            | HAL层移植目录，需要实现串口的收发接口(中断接收)，延时函数，模组上下电及os相关接口|
| sample          | 应用示例，示例使用MQTT、影子、数据模板的使用方式|
| include         | SDK对外头文件及设备信息配置头文件 |
| src             | AT框架及协议逻辑实现 |
|   ├───── at_client  | at client抽象，实现RX解析，命令下行，urc匹配，resp异步匹配|
|   	├───── common  |log、ringbuff、定时器实现|
|   	├───── shadow  |基于AT框架的shadow逻辑实现|
| README.md       | SDK使用说明 |

### 移植指导


### SDK接口说明
 关于 SDK 的更多使用方式及接口了解, 参见 qcloud_iot_api_export.h

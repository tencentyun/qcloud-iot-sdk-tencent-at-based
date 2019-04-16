##  qcloud-iot-sdk-tecent-at-based 

### 介绍

qcloud-iot-sdk-tecent-at-based 面向使用支持腾讯AT指令的模组(2/3/4/5G、NB、WIFI等)接入腾讯物联网平台的终端设备开发者，典型应用场景为MCU+腾讯定制AT模组，SDK完成实现了MCU和模组数据交互的AT框架，并基于AT框架配合腾讯AT指令，实现了MQTT、影子及数据模板的功能，同时提供了示例sample。开发者需要实现的HAL层适配接口见hal_export.h，需要实现串口的收发接口(中断接收)，延时函数，模组上下电及os相关接口适配（互斥锁、动态内存申请释放、线程创建），适配层接口单独剥离在port目录。

### qcloud-iot-sdk-tecent-at-based软件架构
![AT_SDK_FRAMEWORK.jpg](https://i.loli.net/2019/04/16/5cb53f4a322a6.jpg)


### 目录结构

| 名称            | 说明 |
| ----            | ---- |
| docs            | 文档目录，包含腾讯AT指令集定义 |
| port            | HAL层移植目录，需要实现串口的收发接口(中断接收)，延时函数，模组上下电及os相关接口|
| sample          | 应用示例，示例使用MQTT、影子、数据模板的使用方式|
| src             | AT框架及协议逻辑实现 |
|   ├───── event  | 事件功能协议封装 |
|   	├───── module_at  |at client抽象，实现RX解析，命令下行，urc匹配，resp异步匹配|
|   	├───── shadow  |基于AT框架的shadow逻辑实现|
|   	├───── mqtt  |基于AT框架的mqtt协议实现|
|   	├───── utils  |json、timer、链表等应用|
|   	├───── include  |SDK对外头文件及设备信息配置头文件|
| tools         | 代码生成脚本 |
| README.md       | SDK使用说明 |

### 移植指导
根据所选的嵌入式平台，适配 hal_export.h 头文件对应的hal层API的移植实现。主要有串口收发（中断接收）、模组开关机、任务/线程创建、动态内存申请/释放、时延、打印等API。可参考基于STM32+FreeRTOS的AT-SDK[移植示例](http://git.code.oa.com/iotcloud_teamIII/Iot-hub-at-sdk-based-stm32-freertos.git)

### SDK接口说明
 关于 SDK 的更多使用方式及接口了解, 参见 qcloud_iot_api_export.h

/**
 ******************************************************************************
 * @file			:  dev_config.h
 * @brief			:  dev config head file
 ******************************************************************************
 *
 * History:      <author>          <time>        <version>
 *               yougaliu          2019-3-20        1.0
 * Desc:          ORG.
 * Copyright (c) 2019 Tencent. 
 * All rights reserved.
 ******************************************************************************
 */
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#define  AUTH_MODE_KEY		

#ifdef AUTH_MODE_CERT
	/* 产品名称, 与云端同步设备状态时需要  */
	#define QCLOUD_IOT_MY_PRODUCT_ID            "YOUR_PRODUCT_ID"
	/* 设备名称, 与云端同步设备状态时需要 */
	#define QCLOUD_IOT_MY_DEVICE_NAME           "YOUR_DEVICE_NAME"

    /* 客户端证书文件名  非对称加密使用*/
    #define QCLOUD_IOT_CERT_FILENAME          "YOUR_DEVICE_NAME_cert.crt"
    /* 客户端私钥文件名 非对称加密使用*/
    #define QCLOUD_IOT_KEY_FILENAME           "YOUR_DEVICE_NAME_private.key"

	#define 	PATH_MAX   255
    static char sg_cert_file[PATH_MAX + 1];      //客户端证书全路径
    static char sg_key_file[PATH_MAX + 1];       //客户端密钥全路径

#else
	/* 产品名称, 与云端同步设备状态时需要  */
	#define QCLOUD_IOT_MY_PRODUCT_ID            "03UKNYBUZG"
	/* 设备名称, 与云端同步设备状态时需要 */
	#define QCLOUD_IOT_MY_DEVICE_NAME           "dev2"
    #define QCLOUD_IOT_DEVICE_SECRET            "rcahtVijnPKtaY/RR5ovBA=="
#endif

/* 产品名称的最大长度 */
#define MAX_SIZE_OF_PRODUCT_ID                                    	(10)

/* 设备ID的最大长度 */
#define MAX_SIZE_OF_DEVICE_NAME                                     (64)

/* psk最大长度 */
#define MAX_SIZE_OF_DEVICE_SERC  	 								(24)

typedef struct _sDevInfo_{
	char productId[MAX_SIZE_OF_PRODUCT_ID + 4];
	char devName[MAX_SIZE_OF_DEVICE_NAME + 4];
	char devSerc[MAX_SIZE_OF_DEVICE_SERC + 4];
}sDevInfo;

typedef enum{
	eNOTLS = 0,
	ePSKTLS = 1,
	eCERTTLS = 2,	
}eTlsMode;

/**
 * @brief 服务质量等级
 *
 * 服务质量等级表示PUBLISH消息分发的质量等级
 */
typedef enum _QoS {
    QOS0 = 0,    // 至多分发一次
    QOS1 = 1,    // 至少分发一次, 消息的接收者需回复PUBACK报文
    QOS2 = 2     // 仅分发一次, 目前腾讯物联云不支持该等级
} QoS;

	
#define  USE_TLS_MODE			ePSKTLS


typedef struct {
	eTlsMode 					tlsmod;
    uint32_t					command_timeout;		 // 发布订阅信令读写超时时间 ms
    uint32_t					keep_alive_interval_ms;	 // 心跳周期, 单位: s

    uint8_t         			clean_session;			 // 清理会话标志位

    uint8_t                   	auto_connect_enable;     // 是否开启自动重连 1:启用自动重连 0：不启用自动重连  建议为1
} MQTTInitParams;


/**
 * MQTT初始化参数结构体默认值定义
 */
#define DEFAULT_MQTTINIT_PARAMS {USE_TLS_MODE, 5000, 240, 1, 1}

#endif

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
#include "qcloud_iot_export_mqtt.h"

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
	#define QCLOUD_IOT_MY_PRODUCT_ID            "50SW2Y9V7D"
	/* 设备名称, 与云端同步设备状态时需要 */
	#define QCLOUD_IOT_MY_DEVICE_NAME           "light_01"
    #define QCLOUD_IOT_DEVICE_SECRET            "6cauZYzMJzxoSrkXWaAmCw=="
#endif

typedef struct {
	char	product_id[MAX_SIZE_OF_PRODUCT_ID + 4];
	char 	device_name[MAX_SIZE_OF_DEVICE_NAME  + 4];
	char	devSerc[MAX_SIZE_OF_DEVICE_SERC  + 4];
	char	client_id[MAX_SIZE_OF_CLIENT_ID + 1];
} DeviceInfo;


DeviceInfo* iot_device_info_get(void);
eAtResault iot_device_info_init(const char *product_id, const char *device_name, const char *device_serc);

#endif

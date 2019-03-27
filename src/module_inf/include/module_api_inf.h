 /**
 ******************************************************************************
 * @file			:  module_api_inf.h
 * @brief			:  head file of api for application based on at
 ******************************************************************************
 *
 * History:      <author>          <time>        <version>
 *               yougaliu          2019-3-20        1.0
 * Desc:          ORG.
 * Copyright (c) 2019 Tencent. 
 * All rights reserved.
 ******************************************************************************
 */
#ifndef _MODULE_API_INF_H_
#define _MODULE_API_INF_H_
#include "stdint.h"
#include "dev_config.h"

typedef enum _eAtResault_{
    AT_ERR_SUCCESS    = 0,      // 表示成功返回
    AT_ERR_FAILURE    = -100,   // 表示失败返回
    AT_ERR_INVAL      = -101,   // 表示参数无效错误
    AT_ERR_NULL       = -102,   // 表示空指针

	AT_ERR_TIMEOUT	  = -201,
	AT_ERR_RESP_NULL  = -202,
} eAtResault;

typedef enum{
	eMODULE_CELLULAR = 0, //2/3/4/5G NB etc.
	eMODULE_WIFI = 1,
	eMODULE_DEFAULT = 2,
}eModuleType;

eAtResault module_init(eModuleType eType);
eAtResault module_handshake(uint32_t timeout);
eAtResault module_info_set(sDevInfo *pInfo, eTlsMode eMode);
eAtResault module_mqtt_conn(MQTTInitParams init_params);
eAtResault module_mqtt_discon(void);
eAtResault module_mqtt_pub(const char *topic, QoS eQos, const char *payload);
eAtResault module_mqtt_publ(const char *topic, QoS eQos, const char *payload);
eAtResault module_mqtt_sub(const char *topic, QoS eQos);
eAtResault module_mqtt_unsub(const char *topic);
eAtResault module_mqtt_state(void);
#endif

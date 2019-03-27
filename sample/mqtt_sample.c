/**
******************************************************************************
* @file           : mqtt_sample.c
* @brief          : mqtt sample based on AT cmd
******************************************************************************
*
* Copyright (c) 2019 Tencent. 
* All rights reserved.
******************************************************************************
*/	
#include "qcloud_iot_api_export.h"
#include "ringbuff.h"
#include "at_client.h"
#include "string.h"


void init_task(void *arg)
{	
	Log_d("init_task Entry...");
	if(AT_ERR_SUCCESS != module_init(eMODULE_WIFI)) 
	{
		Log_e("module init failed");
	}
	else
	{
		Log_d("module init success");
		
		osThreadId threadId;
		at_client_t pclient;
		pclient = at_client_get();		
		
		//	Parser Func should run in a separate thread
		if((NULL != pclient)&&(NULL != pclient->parser))
		{
			//hal_thread_create(&threadId, PARSE_THREAD_STACK_SIZE, osPriorityAboveNormal, pclient->parser, pclient);
			hal_thread_create(&threadId, PARSE_THREAD_STACK_SIZE, osPriorityNormal, pclient->parser, pclient);
		}	
	}

	hal_thread_destroy(NULL);
}


void mqtt_demo_task(void *arg)
{
	osThreadId threadId;
	at_client_t pclient;
	eAtResault Ret;
	sDevInfo testInfo;
	int count = 0;
	char payload[256];
	pclient = at_client_get();	

	Log_d("mqtt_demo_task Entry...");
	

	if(AT_ERR_SUCCESS != module_init(eMODULE_WIFI)) 
	{
		Log_e("module init failed");
	}
	else
	{
		Log_d("module init success");	
	}
	
	//	Parser Func should run in a separate thread
	if((NULL != pclient)&&(NULL != pclient->parser))
	{
		hal_thread_create(&threadId, PARSE_THREAD_STACK_SIZE, osPriorityNormal, pclient->parser, pclient);
	}


	while(AT_STATUS_INITIALIZED != pclient->status)
	{	
		HAL_SleepMs(1000);
	}
	
	Log_d("Start shakehands with module...");
	Ret = module_handshake(CMD_TIMEOUT_MS);
	if(AT_ERR_SUCCESS != Ret)
	{
		Log_e("module connect fail,Ret:%d", Ret);
		goto exit;
	}
	else
	{
		Log_d("module connect success");
	}
	
	
	memset((uint8_t *)&testInfo, 0x0, sizeof(sDevInfo));
	strncpy(testInfo.productId, QCLOUD_IOT_MY_PRODUCT_ID, MAX_SIZE_OF_PRODUCT_ID);
	strncpy(testInfo.devName, QCLOUD_IOT_MY_DEVICE_NAME, MAX_SIZE_OF_DEVICE_NAME);
	strncpy(testInfo.devSerc, QCLOUD_IOT_DEVICE_SECRET, MAX_SIZE_OF_DEVICE_SERC);

	Ret = module_info_set(&testInfo, USE_TLS_MODE);
	if(AT_ERR_SUCCESS != Ret)
	{
		Log_e("module info set fail,Ret:%d", Ret);
		goto exit;
	}
	else
	{
		Log_d("module info set success");
	}
	
	MQTTInitParams init_params = DEFAULT_MQTTINIT_PARAMS;
	Ret = module_mqtt_conn(init_params);
	if(AT_ERR_SUCCESS != Ret)
	{
		Log_e("module mqtt conn fail,Ret:%d", Ret);
		goto exit;
	}
	else
	{
		Log_d("module mqtt conn success");
	}

	
	Ret = module_mqtt_sub("03UKNYBUZG/dev2/data", QOS0);
	if(AT_ERR_SUCCESS != Ret)
	{
		Log_e("module mqtt sub fail,Ret:%d", Ret);
		goto exit;
	}
	else
	{
		Log_d("module mqtt sub success");
	}


	while(1)
	{
		HAL_SleepMs(1000);
		memset(payload, 0, 256);
		HAL_Snprintf(payload, 256, "{\"action\": \"publish_test\", \"count\": \"%d\"}",count++);
		Log_d("pub_msg:%s", payload);
		Ret = module_mqtt_pub("03UKNYBUZG/dev2/data", QOS0, payload);
		if(AT_ERR_SUCCESS != Ret)
		{
			Log_e("module mqtt pub fail,Ret:%d", Ret);
			//goto exit;
		}
		else
		{
			Log_d("module mqtt pub success");
		}	
		
	}

	
exit:

	hal_thread_destroy(NULL);
}

void mqtt_sample(void)
{
	//osThreadId init_threadId;
	osThreadId demo_threadId;
	
#ifdef OS_USED
	//hal_thread_create(&init_threadId, 512, osPriorityNormal, init_task, NULL);
	hal_thread_create(&demo_threadId, 512, osPriorityNormal, mqtt_demo_task, NULL);
	hal_thread_destroy(NULL);
#else
	#error os should be used just now
#endif
}

/**
 ******************************************************************************
 * @file			: module_api_inf.c
 * @brief			: api for application based on at
 ******************************************************************************
 *
 * History:      <author>          <time>        <version>
 *               yougaliu          2019-3-20        1.0
 * Desc:          ORG.
 * Copyright (c) 2019 Tencent. 
 * All rights reserved.
 ******************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "qcloud_iot_api_export.h"
#include "at_client.h"



static void urc_pub_recv_func(const char *data, uint32_t size)
{
	Log_d("receve pub_msg(%d):%s", size, data);
}

static void urc_discon_func(const char *data, uint32_t size)
{
	Log_d("receve disconnect urc(%d):%s", size, data);
	hal_thread_destroy(NULL);
}

static void urc_ota_status_func(const char *data, uint32_t size)
{
	Log_d("receve ota status urc(%d):%s", size, data);
}


static at_urc urc_table[] = {
        {"+TCMQTTRCVPUB:", ":", urc_pub_recv_func},
        {"+TCMQTTDISCON",  "\r\n", urc_discon_func},
        {"+TCOTASTATUS",  ":", urc_ota_status_func},
};


eAtResault module_init(eModuleType eType)
{
	eAtResault ret;
	at_client_t p_client;

	p_client = at_client_get();

	if(NULL == p_client)
	{
		Log_e("no at client get");
		ret = AT_ERR_FAILURE;
		goto exit; 
	}

	if(AT_STATUS_INITIALIZED == p_client->status)
	{
		Log_e("at client has been initialized");
		ret = AT_ERR_FAILURE;
		goto exit;
	}
	
	
    /* initialize AT client */
    ret = at_client_init(p_client);
	if(AT_ERR_SUCCESS != ret)
 	{
		Log_e("at client init fail,ret:%d", ret);
		goto exit;
	}
	else
	{
		Log_d("at client init success");
	}

	/*module power on should after at client init for AT uart irq use ringbuff*/
	if(AT_ERR_SUCCESS != module_power_on())
 	{
		Log_e("module power on fail");
		goto exit;
	}
	
    /* register URC data execution function  */
    at_set_urc_table(p_client, urc_table, sizeof(urc_table) / sizeof(urc_table[0]));


	Log_d("urc table addr:%p, size:%d", p_client->urc_table, p_client->urc_table_size);
	for(int i=0; i < p_client->urc_table_size; i++)
	{
		Log_d("%s",p_client->urc_table[i].cmd_prefix);
	}

exit:

    return ret;
}

#if 0
eAtResault module_handshake(uint32_t timeout)
{
	eAtResault result = AT_ERR_SUCCESS;
	at_response_t resp = NULL;
	at_client_t client = at_client_get();
	Timer timer;


	if (client == NULL)
	{
		Log_e("input AT Client object is NULL, please create or get AT Client object!");
		return AT_ERR_FAILURE;
	}

	resp = at_create_resp(16, 0, CMD_TIMEOUT_MS);
	if (resp == NULL)
	{
		Log_e("No memory for response object!");
		return AT_ERR_RESP_NULL;
	}

#ifdef OS_USED
	HAL_MutexLock(client->lock);
#endif

	client->resp = resp;
	resp->line_counts = 0;

	HAL_Timer_countdown_ms(&timer, timeout);

	at_send_data("ATE0\r\n", 6);
	at_send_data("ATE0\r\n", 6);

	/* Check whether it is already connected */	
	do
	{		
		at_send_data("AT\r\n", 4);	
		//Log_d("AT cmd send");
#ifdef OS_USED	
		HAL_SleepMs(CMD_RESPONSE_INTERVAL_MS);
#else
		HAL_DelayMs(CMD_RESPONSE_INTERVAL_MS);	
#endif	


		if (client->resp_notice)
		{
			if(AT_RESP_OK != client->resp_status)
			{
				Log_e("resp err,status:%d", client->resp_status);
				result = AT_ERR_FAILURE;
			}
			else
			{
				break;
			}
			
		}
		else
		{
			continue;
		}		
			
	}while (!HAL_Timer_expired(&timer));

	if(HAL_Timer_expired(&timer))
	{
		Log_d("read ring buff timeout");
		result = AT_ERR_TIMEOUT;
	}

	at_delete_resp(resp);

	client->resp = NULL;
	
#ifdef OS_USED
	HAL_MutexUnlock(client->lock);
#endif
	return result;
}
#endif

eAtResault module_handshake(uint32_t timeout)
{
	eAtResault result = AT_ERR_SUCCESS;
	at_response_t resp = NULL;

	resp = at_create_resp(256, 0, CMD_TIMEOUT_MS);
	if (resp == NULL)
	{
		Log_e("No memory for response object!");
		return AT_ERR_RESP_NULL;
	}
		
    /* disable echo */
    if(AT_ERR_SUCCESS != at_exec_cmd(resp, "ATE0"))
    {
    	Log_e("cmd ATE0 exec err");
		result = AT_ERR_FAILURE;
		//goto exit;
	}

    /* get module version */
	if(AT_ERR_SUCCESS !=  at_exec_cmd(resp, "AT+GMR"))
    {
    	Log_e("cmd AT+GMR exec err");
		result = AT_ERR_FAILURE;
		//goto exit;
	}

	Log_d("Module info(%d):", resp->line_counts);
    /* show module version */
    for (int i = 0; i < resp->line_counts - 1; i++)
    {
        HAL_Printf("\n\r%s", at_resp_get_line(resp, i + 1));
    }

//exit:

	if(resp)
	{
		at_delete_resp(resp);
	}

	return result;
}



/*
* config dev info to module, do this operate only once in factroy is suggested
*/
eAtResault module_info_set(sDevInfo *pInfo, eTlsMode eMode)
{
	eAtResault result = AT_ERR_SUCCESS;
	at_response_t resp = NULL;

	resp = at_create_resp(64, 0, CMD_TIMEOUT_MS);
	
	/* get module version */
	if(AT_ERR_SUCCESS !=  at_exec_cmd(resp, "AT+TCDEVINFOSET=%d,\"%s\",\"%s\",\"%s\"",\
									eMode,pInfo->productId, pInfo->devName, pInfo->devSerc))
    {
    	Log_e("cmd AT+TCDEVINFOSET exec err");
		result = AT_ERR_FAILURE;
		//goto exit;
	}

	if(resp)
	{
		at_delete_resp(resp);
	}
	
    return result;
}

/* mqtt setup connect */
eAtResault module_mqtt_conn(MQTTInitParams init_params)
{
	eAtResault result = AT_ERR_SUCCESS;
	at_response_t resp = NULL;

	resp = at_create_resp(64, 0, CMD_TIMEOUT_MS);
	
	if(AT_ERR_SUCCESS !=  at_exec_cmd(resp, "AT+TCMQTTCONN=%d,%d,%d,%d,%d",init_params.tlsmod,\
										init_params.command_timeout, init_params.keep_alive_interval_ms,\
										init_params.clean_session,  init_params.auto_connect_enable))
	{
		Log_e("cmd AT+TCMQTTCONN exec err");
		result = AT_ERR_FAILURE;
		//goto exit;
	}

	if(resp)
	{
		at_delete_resp(resp);
	}
	
	return result;
}

/* mqtt disconn */
eAtResault module_mqtt_discon(void)
{
	eAtResault result = AT_ERR_SUCCESS;
	at_response_t resp = NULL;

	resp = at_create_resp(64, 0, CMD_TIMEOUT_MS);
	
	if(AT_ERR_SUCCESS !=  at_exec_cmd(resp, "AT+TCMQTTDISCONN"))			
	{
		Log_e("cmd AT+TCMQTTDISCONN exec err");
		result = AT_ERR_FAILURE;
	}

	if(resp)
	{
		at_delete_resp(resp);
	}
	
	return result;
}

/* mqtt pub msg */
eAtResault module_mqtt_pub(const char *topic, QoS eQos, const char *payload)
{
	eAtResault result = AT_ERR_SUCCESS;
	at_response_t resp = NULL;

	resp = at_create_resp(64, 0, CMD_TIMEOUT_MS);	
	if(AT_ERR_SUCCESS !=  at_exec_cmd(resp, "AT+TCMQTTPUB=\"%s\",%d,\"%s\"",topic,eQos,payload))				
	{
		Log_e("cmd AT+TCMQTTPUB exec err");
		result = AT_ERR_FAILURE;
	}

	if(resp)
	{
		at_delete_resp(resp);
	}
	
	return result;
}

eAtResault module_mqtt_publ(const char *topic, QoS eQos, const char *payload)
{
	//To DO
    return AT_ERR_SUCCESS;
}


eAtResault module_mqtt_sub(const char *topic, QoS eQos)
{
	eAtResault result = AT_ERR_SUCCESS;
	at_response_t resp = NULL;

	resp = at_create_resp(64, 0, CMD_TIMEOUT_MS);	
	if(AT_ERR_SUCCESS !=  at_exec_cmd(resp, "AT+TCMQTTSUB=\"%s\",%d",topic,eQos))				
	{
		Log_e("cmd AT+TCMQTTSUB exec err");
		result = AT_ERR_FAILURE;
	}

	if(resp)
	{
		at_delete_resp(resp);
	}
	
	return result;
}

eAtResault module_mqtt_unsub(const char *topic)
{
	eAtResault result = AT_ERR_SUCCESS;
	at_response_t resp = NULL;

	resp = at_create_resp(64, 0, CMD_TIMEOUT_MS);	
	if(AT_ERR_SUCCESS !=  at_exec_cmd(resp, "AT+TCMQTTUNSUB=\"%s\"",topic))				
	{
		Log_e("cmd AT+TCMQTTUNSUB exec err");
		result = AT_ERR_FAILURE;
	}

	if(resp)
	{
		at_delete_resp(resp);
	}
	
	return result;
}


eAtResault module_mqtt_state(void)
{
	eAtResault result = AT_ERR_SUCCESS;
	at_response_t resp = NULL;

	resp = at_create_resp(64, 0, CMD_TIMEOUT_MS);
	
	if(AT_ERR_SUCCESS !=  at_exec_cmd(resp, "AT+TCMQTTSTATE")) 			
	{
		Log_e("cmd AT+TCMQTTSTATE exec err");
		result = AT_ERR_FAILURE;
	}

	if(resp)
	{
		at_delete_resp(resp);
	}
	
	return result;
}


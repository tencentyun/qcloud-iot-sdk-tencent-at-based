/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 Tencent. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef QCLOUD_IOT_EXPORT_DATA_TEMPLATE_H_
#define QCLOUD_IOT_EXPORT_DATA_TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "config.h"
#include "qcloud_iot_export_mqtt.h"
#include "qcloud_iot_export_method.h"

#define  MAX_CONTORL_REPLY_STATUS_LEN		64		   // control�ظ���Ϣ�У�status�ִ�����󳤶ȣ����Ը��߾����Ʒ�����޸�

/**
 * @brief ��������ģ������ݵ�����
 */
#define TYPE_TEMPLATE_INT    	JINT32
#define TYPE_TEMPLATE_ENUM    	JINT32
#define TYPE_TEMPLATE_FLOAT  	JFLOAT
#define TYPE_TEMPLATE_BOOL   	JINT8
#define TYPE_TEMPLATE_STRING 	JSTRING
#define TYPE_TEMPLATE_TIME 		JUINT32
#define TYPE_TEMPLATE_JOBJECT 	JOBJECT

typedef int32_t   TYPE_DEF_TEMPLATE_INT;
typedef int32_t   TYPE_DEF_TEMPLATE_ENUM;
typedef float     TYPE_DEF_TEMPLATE_FLOAT;
typedef char      TYPE_DEF_TEMPLATE_BOOL;
typedef char      TYPE_DEF_TEMPLATE_STRING;
typedef uint32_t  TYPE_DEF_TEMPLATE_TIME;
typedef void *    TYPE_DEF_TEMPLATE_OBJECT;

#ifdef EVENT_POST_ENABLED					//�Ƿ�ʹ������ģ����¼�����

#define TYPE_STR_INFO			"info"
#define TYPE_STR_ALERT			"alert"
#define TYPE_STR_FAULT			"fault"

//���ʹ���¼�ʱ��������뱣֤ʱ�����׼ȷ��UTCʱ��ms��������ж�Ϊ����
//#define  EVENT_TIMESTAMP_USED			 

#define  FLAG_EVENT0 			(1U<<0)
#define  FLAG_EVENT1			(1U<<1)
#define  FLAG_EVENT2			(1U<<2)
#define  FLAG_EVENT3			(1U<<3)
#define  FLAG_EVENT4 			(1U<<4)
#define  FLAG_EVENT5			(1U<<5)
#define  FLAG_EVENT6			(1U<<6)
#define  FLAG_EVENT7			(1U<<7)
#define  FLAG_EVENT8 			(1U<<8)
#define  FLAG_EVENT9			(1U<<9)

#define  ALL_EVENTS_MASK		(0xFFFFFFFF)


typedef enum {
	eEVENT_INFO,
	eEVENT_ALERT,    
    eEVENT_FAULT, 
}eEventType;

typedef struct  _sEvent_{
	char 	 *event_name;		 //�¼�����	
	char 	 *type;			 	 //�¼�����	
    uint32_t timestamp;			 //�¼�ʱ��	
	uint8_t eventDataNum;		 //�¼����Ե����
    DeviceProperty *pEventData;  //�¼����Ե�
} sEvent;

#endif

typedef enum _eControlReplyCode_{
		eDEAL_SUCCESS = 0,
		eDEAL_FAIL = -1,
}eReplyCode;

/**
 * @brief control msg reply ����
 */
typedef struct _sControlReplyPara {

    uint32_t  timeout_ms;         						      // ����ʱʱ��, ��λ:ms
    
    eReplyCode   code;    							  // �ظ�code��0���ɹ� ��0��ʧ��

    char      status_msg[MAX_CONTORL_REPLY_STATUS_LEN];       // ������Ϣ

} sReplyPara;


/**
 * @brief ��������ģ�������״̬
 */
typedef enum _eDataState_{
    eNOCHANGE = 0,
	eCHANGED = 1,	
} eDataState;

/**
 * @brief ��������ģ������Խṹ
 */
typedef struct {
    DeviceProperty data_property;
    eDataState state;
} sDataPoint;

/**
 * @brief ��ȡdata template client
 *
 * @return ShadowClient
 */
void *get_template_client(void);


/**
 * @brief ����TemplateClient
 *
 * @param client �����صĹ���ɹ���data template client
 *
 * @return ����NULL: ����ʧ��
 */
int IOT_Template_Construct(void **client);

/**
 * @brief �ͻ���Ŀǰ�Ƿ�������
 *
 * @param pClient Template Client�ṹ��
 * @return ����true, ��ʾ�ͻ���������
 */
bool IOT_Template_IsConnected(void *handle);

/**
 * @brief ����TemplateClient �ر�MQTT����
 *
 * @param pClient TemplateClient����
 *
 * @return ����QCLOUD_ERR_SUCCESS, ��ʾ�ɹ�
 */
int IOT_Template_Destroy(void *handle);

/**
 * @brief ��Ϣ����, �¼��ظ���ʱ����, ��ʱ������
 *
 * @param handle     data template client
 * @param timeout_ms ��ʱʱ��, ��λ:ms
 */
void IOT_Template_Yield(void *handle, uint32_t timeout_ms);


/**
 * @brief ע�ᵱǰ�豸���豸����
 *
 * @param pClient    Client�ṹ��
 * @param pProperty  �豸����
 * @param callback   �豸���Ը��»ص���������
 * @return           ����QCLOUD_ERR_SUCCESS, ��ʾ����ɹ�
 */
int IOT_Template_Register_Property(void *handle, DeviceProperty *pProperty, OnPropRegCallback callback);

/**
 * @brief ɾ���Ѿ�ע������豸����
 *
 * @param pClient    Client�ṹ��
 * @param pProperty  �豸����
 * @return           ����QCLOUD_ERR_SUCCESS, ��ʾ����ɹ�
 */
int IOT_Template_UnRegister_Property(void *handle, DeviceProperty *pProperty);

/**
 * @brief ��JSON�ĵ�������reported�ֶΣ������Ǹ���
 *
 *
 * @param jsonBuffer    Ϊ�洢JSON�ĵ�׼�����ַ���������
 * @param sizeOfBuffer  ��������С
 * @param count         �ɱ�����ĸ���, �����ϱ����豸���Եĸ���
 * @return              ����QCLOUD_ERR_SUCCESS, ��ʾ�ɹ�
 */
int IOT_Template_JSON_ConstructReportArray(void *handle, char *jsonBuffer, size_t sizeOfBuffer, uint8_t count, DeviceProperty *pDeviceProperties[]); 

/**
 * @brief  ����ģ���첽��ʽ�ϱ�����
 *
 * @param pClient             data template client
 * @param pJsonDoc          ���ϱ�������ģ����ĵ�����   
 * @param sizeOfBuffer      �ĵ�����
 * @param callback            �ϱ����ݵ� ��Ӧ�����ص�����
 * @param userContext       �û�����, ������Ӧ����ʱͨ���ص���������
 * @param timeout_ms        ����ʱʱ��, ��λ:ms
 * @return                  ����QCLOUD_ERR_SUCCESS, ��ʾ�ϱ��ɹ�
 */
int IOT_Template_Report(void *handle, char *pJsonDoc, size_t sizeOfBuffer, OnReplyCallback callback, void *userContext, uint32_t timeout_ms);

/**
 * @brief  ����ģ��ͬ����ʽ�ϱ�����
 *
 * @param pClient             data template client
 * @param pJsonDoc          ���ϱ�������ģ����ĵ�����   
 * @param sizeOfBuffer      �ĵ�����
 * @param timeout_ms       ����ʱʱ��, ��λ:ms
 * @return                  ����QCLOUD_ERR_SUCCESS, ��ʾ�ϱ��ɹ�
 */
int IOT_Template_Report_Sync(void *handle, char *pJsonDoc, size_t sizeOfBuffer, uint32_t timeout_ms);


 /**
  * @brief	�첽��ʽ��ȡ�����ƶ�����ģ��״̬
  *                 һ������ͬ���豸�����ڼ����˵���������
  *
  * @param pClient		   data template client
  * @param callback 		   ���ݻ�ȡ����� ��Ӧ�����ص�����
  * @param userContext	   �û�����, ������Ӧ����ʱͨ���ص���������
  * @param timeout_ms	   ����ʱʱ��, ��λ:ms
  * @return 				  ����QCLOUD_ERR_SUCCESS, ��ʾ����ɹ�
  */
int IOT_Template_GetStatus(void *handle, OnReplyCallback callback, void *userContext, uint32_t timeout_ms);

 /**
  * @brief	ͬ����ʽ��ȡ�����ƶ�����ģ��״̬
  * @param pClient		   data template client
  * @param timeout_ms	   ����ʱʱ��, ��λ:ms
  * @return 				  ����QCLOUD_ERR_SUCCESS, ��ʾ����ɹ�
  */
int IOT_Template_GetStatus_sync(void *handle, uint32_t timeout_ms);

/**
 * @brief  ɾ�������ڼ��control data
 * @param pClient		  data template client
 * @param code	  		  0���ɹ�     ��0��ʧ��
 * @param pClientToken	  ��Ӧ��get_status_reply	�е�clientToken
 * @return					 ����QCLOUD_ERR_SUCCESS, ��ʾ�����ɹ�
 */
int IOT_Template_ClearControl(void *handle, char *pClientToken, OnRequestCallback callback, uint32_t timeout_ms); 


/**
 * @brief  �ظ�����control��Ϣ
 * @param pClient		   data template client
 * @param jsonBuffer		   ���ڹ���ظ���������buffer
 * @param sizeOfBuffer	  ���ݳ���
 *	@param replyPara		  �ظ��������ɹ�/ʧ�ܼ���Ӧ�ĸ�����Ϣ  
 * @return					 ����QCLOUD_ERR_SUCCESS, ��ʾ����ɹ�
 */ 
int IOT_Template_ControlReply(void *handle, char *pJsonDoc, size_t sizeOfBuffer, sReplyPara *replyPara);

/**
 * @brief  ����ϵͳ��Ϣ�ϱ�json���ݰ�
 * @param pClient		   data template client
 * @param jsonBuffer	   ���ڹ�������buffer
 * @param sizeOfBuffer	   ���ݳ���
 * @param pPlatInfo		   ƽ̨ϵͳ��Ϣ������ 
 * @param pSelfInfo		   �豸���Զ����ϵͳ��Ϣ����ѡ
 * @return				   ����QCLOUD_ERR_SUCCESS, ��ʾ����ɹ�

 *ϵͳ��Ϣ�ϱ�����ʽ
* {
*  "method": "report_info",
*  "clientToken": "client-token1618",
*  "params": {
*  "module_hardinfo": "ģ�����Ӳ���ͺ�  N10,,��ģ���豸����",
*  "module_softinfo":  "ģ�������汾,��ģ���豸����",
*  "fw_ver":       "mcu�̼��汾,����",
*  "imei":       "�豸imei�ţ���ѡ�ϱ�",
*  "lat":        "γ�ȣ��軻��Ϊ10����,��ѡ�ϱ�,��Ҫλ�÷����豸����,�ƶ��豸��Ҫʵʱ��ȡ"
*  "lon":        "���ȣ��軻��Ϊ10����,��ѡ�ϱ�,��Ҫλ�÷����豸����,�ƶ��豸��Ҫʵʱ��ȡ",
*  "device_label": {
*    "append_info": "�豸���Զ���Ĳ�Ʒ������Ϣ"
* ...
*   }
* }
*/
int IOT_Template_JSON_ConstructSysInfo(void *handle, char *jsonBuffer, size_t sizeOfBuffer, DeviceProperty *pPlatInfo, DeviceProperty *pSelfInfo); 

/**
 * @brief �첽��ʽ�ϱ�ϵͳ��Ϣ
 * @param pClient		   data template client
 * @param jsonBuffer	   ϵͳ��ϢJson��
 * @param sizeOfBuffer	   ϵͳ��ϢJson������
 * @param callback		   ��Ӧ�ص�
 * @param userContext	   �û�����, ��Ӧ����ʱͨ���ص���������
 * @param timeout_ms	   ��Ӧ��ʱʱ��, ��λ:ms
 * @return				   ����QCLOUD_ERR_SUCCESS, ��ʾ�ϱ��ɹ�
*/
int IOT_Template_Report_SysInfo(void *handle, char *pJsonDoc, size_t sizeOfBuffer, OnReplyCallback callback, void *userContext, uint32_t timeout_ms);

/**
 * @brief ͬ����ʽ�ϱ�ϵͳ��Ϣ
 * @param pClient		   data template client
 * @param jsonBuffer	   ϵͳ��ϢJson��
 * @param sizeOfBuffer	   ϵͳ��ϢJson������
 * @param timeout_ms	   ͬ���ȴ���Ӧ��ʱʱ��, ��λ:ms
 * @return				   ����QCLOUD_ERR_SUCCESS, ��ʾ�ϱ��ɹ�
*/
int IOT_Template_Report_SysInfo_Sync(void *handle, char *pJsonDoc, size_t sizeOfBuffer, uint32_t timeout_ms);


#ifdef EVENT_POST_ENABLED
/**
 * @brief �¼��ϱ��ظ��ص���
 *
 * @param msg    	 �¼���Ӧ���ص��ĵ�
 * @param context    data template client
 *
 */
typedef void (*OnEventReplyCallback)(char *msg, void *context);


/**
 * @brief �����¼����
 *
 * @param  flag  ���÷������¼���
 */
void IOT_Event_setFlag(void *client, uint32_t flag);

/**
 * @brief ����¼����
 *
 * @param  flag  ��������¼���
 */
void IOT_Event_clearFlag(void *client, uint32_t flag);

/**
 * @brief ��ȡ����λ���¼���
 *
 * @return ����λ���¼���
 */
uint32_t IOT_Event_getFlag(void *client);

/**
 * @brief �¼�client��ʼ����ʹ���¼�����ǰ���ȵ���
 *
 * @param c    shadow ʵ��ָ��
 */
int IOT_Event_Init(void *c);

/**
 * @brief �����¼��������Ѿ���ʱ������
 * 
 * @param client   data template client
 */

void handle_template_expired_event(void *client);


/**
 * @brief �¼��ϱ��������¼����飬SDK����¼���json��ʽ��װ
 * @param pClient shadow ʵ��ָ��
 * @param pJsonDoc    ���ڹ���json��ʽ�ϱ���Ϣ��buffer
 * @param sizeOfBuffer    ���ڹ���json��ʽ�ϱ���Ϣ��buffer��С
 * @param event_count     ���ϱ����¼�����
 * @param pEventArry	  ���ϱ����¼�����ָ
 * @param replyCb	  �¼��ظ���Ϣ�Ļص� 
 * @return @see IoT_Error_Code	  
 */
int IOT_Post_Event(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, uint8_t event_count, sEvent *pEventArry[], OnEventReplyCallback replyCb);                                            

/**
 * @brief �¼��ϱ����û������ѹ����õ��¼���json��ʽ��SDK�����¼�ͷ�����ϱ�
 * @param pClient shadow ʵ��ָ��
 * @param pJsonDoc    ���ڹ���json��ʽ�ϱ���Ϣ��buffer
 * @param sizeOfBuffer    ���ڹ���json��ʽ�ϱ���Ϣ��buffer��С
 * @param pEventMsg     ���ϱ����¼�json��Ϣ 
 *  json�¼���ʽ��
 *  �����¼���
 *	 {"method": "event_post",
 *		"clientToken": "123",
 *		"version": "1.0",
 *		"eventId": "PowerAlarm",
 *		"type": "fatal",
 *		"timestamp": 1212121221,
 *		"params": {
 *			"Voltage": 2.8,
 *			"Percent": 20
 *		}
 *	}
 *
 *  ����¼���
 *	 {
 *		 "eventId": "PowerAlarm",
 *		 "type": "fatal",
 *		 "timestamp": 1212121221,
 *		 "params": {
 *			 "Voltage": 2.8,
 *			 "Percent": 20
 *		 }
 *	 },
 *	 {
 *		 "name": "PowerAlarm",
 *		 "type": "fatal",
 *		 "timestamp": 1212121223,
 *		 "params": {
 *			 "Voltage": 2.1,
 *			 "Percent": 10
 *		 }
 *	 },
 *   ....
 *
 * @param replyCb	  �¼��ظ���Ϣ�Ļص� 
 * @return @see IoT_Error_Code	  
 */
int IOT_Post_Event_Raw(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, char *pEventMsg, OnEventReplyCallback replyCb);                                            


#endif

#ifdef ACTION_ENABLED
/**
 * @brief ע�ᵱǰ�豸����Ϊ
 *
 * @param pClient    Client�ṹ��
 * @param pProperty  �豸����
 * @param callback   �豸���Ը��»ص���������
 * @return           ����QCLOUD_ERR_SUCCESS, ��ʾ����ɹ�
 */

int IOT_Template_Register_Action(void *handle, DeviceAction *pAction, OnActionHandleCallback callback);

/**
 * @brief ɾ���Ѿ�ע������豸��Ϊ
 *
 * @param pClient    Client�ṹ��
 * @param pProperty  �豸����
 * @return           ����QCLOUD_ERR_SUCCESS, ��ʾ����ɹ�
 */

int IOT_Template_UnRegister_Action(void *handle, DeviceAction *pAction); 

/**
* @brief �豸��Ϊ�ظ� 
* @param pClient		  handle to data_template client
* @param pClientToken	  correspond to the clientToken of action msg 
* @param pJsonDoc	  	  data buffer for reply
* @param sizeOfBuffer     length of data buffer
* @param pAction 		  pointer of action 	
* @param replyPara        action reply info
* @return				  QCLOUD_RET_SUCCESS when success, or err code for failure
*/ 

int IOT_ACTION_REPLY(void *pClient, const char *pClientToken, char *pJsonDoc, size_t sizeOfBuffer, DeviceAction *pAction, sReplyPara *replyPara);

#endif



#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_TEMPLATE_H_ */

#ifndef _QCLOUD_CONFIG_H_
#define _QCLOUD_CONFIG_H_

#define AUTH_MODE_KEY

#define EVENT_POST_ENABLED
#define ACTION_ENABLED
#define DEBUG_DEV_INFO_USED

#define OS_USED

#define MODULE_TYPE_ESP8266
//#define MODULE_TYPE_N21
//#define MODULE_TYPE_L206D


#define AT_CMD_MAX_LEN                 1024
#define RING_BUFF_LEN         		   AT_CMD_MAX_LEN	 //uart ring buffer len

#define MAX_PAYLOAD_LEN_PUB			   200				//AT+TCMQTTPUB 最长支持的数据长度，大于这个长度需要启用AT+TCMQTTPUBL

#define QUOTES_TRANSFER_NEED			1				// mqtt payload 双引号是否需要转义。 默认需要 
#define COMMA_TRANSFER_NEED				1				// mqtt payload 逗号是否需要转义。   默认不需要，目前只有ESP8266逗号需要转义 
#ifdef  COMMA_TRANSFER_NEED	 
#define T_	"\\" 							
#else
#define T_	
#endif


/* #undef AUTH_MODE_CERT */
/* #undef AUTH_WITH_NOTLS */
/* #undef SYSTEM_COMM */

/* #undef DEV_DYN_REG_ENABLED */
/* #undef LOG_UPLOAD */
/* #undef IOT_DEBUG */
/* #undef DEBUG_DEV_INFO_USED */
/* #undef AT_TCP_ENABLED */
/* #undef AT_UART_RECV_IRQ */
/* #undef AT_OS_USED */
/* #undef AT_DEBUG */
#endif

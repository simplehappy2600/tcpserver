/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#pragma once
#ifdef LIB_TS_SERVER_EXPORTS
#define LIB_TS_SERVER_API __declspec(dllexport)
#else
#define LIB_TS_SERVER_API __declspec(dllimport)
#endif

extern "C" {

#include "ts_result.h"

/*
 * ͨ�ò��� 
 *		cb	: ����첽�ص�: ��cb��ΪNULLʱ, ��������NULL; cbΪNULL, ͬ������
 *		devid: �豸ID		
 *		cmd	: �������	
 */

/*
 * init service
 * param port: �����˿�, 0: ʹ��Ĭ�� 
 */
LIB_TS_SERVER_API ts_result_c * ts_init_server(int port, TS_RESULT_CB cb);
/*
 * close service
 */
LIB_TS_SERVER_API ts_result_c * ts_fini_server(TS_RESULT_CB cb);

//cmd get info from client
LIB_TS_SERVER_API ts_result_c * send_cmd_info(uint32_t devid, int cmd, TS_RESULT_CB cb);
//cmd to control client
LIB_TS_SERVER_API ts_result_c * send_cmd_pause(uint32_t devid, uint8_t status, TS_RESULT_CB cb);

/*
 * free result
 */
LIB_TS_SERVER_API void ts_free_result(ts_result_c *p);

}
/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#pragma once
#ifdef LIB_PM_SERVER_EXPORTS
#define LIB_PM_SERVER_API __declspec(dllexport)
#else
#define LIB_PM_SERVER_API __declspec(dllimport)
#endif

extern "C" {

#include "ts_result.h"

/*
 * 通用参数 
 *		cb	: 结果异步回调: 当cb不为NULL时, 函数返回NULL; cb为NULL, 同步调用
 *		devid: 设备ID		
 *		cmd	: 命令代码	
 */

/*
 * init service
 * param port: 监听端口, 0: 使用默认6666 
 */
LIB_PM_SERVER_API ts_result_c * pm_init_server(int port, TS_RESULT_CB cb);
/*
 * close service
 */
LIB_PM_SERVER_API ts_result_c * pm_fini_server(TS_RESULT_CB cb);

//cmd get info from client
LIB_PM_SERVER_API ts_result_c * send_cmd_info(uint32_t devid, int cmd, TS_RESULT_CB cb);
//cmd to control client
LIB_PM_SERVER_API ts_result_c * pm_cmd_pause(uint32_t devid, uint8_t status, TS_RESULT_CB cb);

/*
 * free result
 */
LIB_PM_SERVER_API void pm_free_result(ts_result_c *p);

}
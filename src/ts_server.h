/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#pragma once

class ts_server_c {
public:
	static void ts_loop_th(void *arg);
	static void send_req_async(uv_async_t* handle);
};


/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#pragma once

void ts_loop_th(void *arg);

void send_req_async(uv_async_t* handle);
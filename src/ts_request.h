/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#pragma once
#include <stdint.h>
#include "uv.h"
#include "ts_result.h"

class ts_client_t;

//请求的数据
typedef union ts_req_data_u {	
	struct {			
		char *buf;
		int buf_len;
	} buf_data_s;
	uint8_t status;
	int port;
} ts_req_data_t;

//
class ts_req_t {
public:
	static ts_req_t* ts_req_t::newInstance(uint32_t devid, uint16_t cmd, TS_RESULT_CB result_cb);
	static void clean(ts_req_t *req);
	void start_timer();
	void finish(int err, char *msg);
	void finish(bool del_from_client, int err, char *msg);	
private:
	ts_req_t(){}
	ts_req_t(uint32_t devid, uint16_t cmd, TS_RESULT_CB result_cb);
public:
	uint32_t devid;
	uint16_t cmd;
	int seq;
	uv_sem_t *sem;
	
	uv_timer_t *timer;
	ts_client_t *client;

	ts_req_data_t data;
	ts_result_c* result;
	TS_RESULT_CB result_cb;	
private:
	
} ;
/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description:  use libuv v1.7.5  https://github.com/libuv/libuv
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#include "stdafx.h"
#include "lib_ts_server.h"
#include "ts_protocol.h"
#include "ts_util.h"
#include "ts_server.h"
#include "ts_result.h"

LIB_TS_SERVER_API ts_result_c * ts_fini_server()
{
	//TODO
	uv_stop(uv_default_loop());

	return NULL;
}

LIB_TS_SERVER_API ts_result_c * ts_init_server(int port, TS_RESULT_CB cb)
{
	ts_util::p(__FUNCTION__);

	ts_result_c *result = new ts_result_c;
	if (!result){
		printf("new pm_result_c fail");
		return NULL;
	}

	ts_req_c *req = ts_req_c::newInstance(NULL, NULL, cb);
	req->data.port = port;
	req->result = result;	

	uv_sem_t *sem = NULL;
	if (!req->result_cb){
		req->sem = sem = (uv_sem_t *)malloc(sizeof(uv_sem_t));
		int r = uv_sem_init(sem, 0);
		if (r != 0){
			result->setResult(TS_ERR::INIT_ERR, "uv_sem_init fail");
			goto pm_fail;
		}
	}
	
	uv_thread_t *loop_th = (uv_thread_t *)malloc(sizeof(uv_thread_t));	
	if (!loop_th){
		result->setResult(TS_ERR::INIT_ERR, "malloc fail");
		goto pm_fail;
	}

	int r = uv_thread_create(loop_th, ts_server_c::ts_loop_th, req);	
	if (r != 0){
		result->setResult(TS_ERR::INIT_ERR, "uv_thread_create fail");
		goto pm_fail;
	}

	if (sem){
		uv_sem_wait(sem);
		uv_sem_destroy(sem);
	}

	ts_util::p(__FUNCTION__ " done");
	
	return cb ? NULL : result;

pm_fail:
	if (loop_th){
		free(loop_th);
	}
    return result;
}
static ts_result_c * send_req(ts_req_c *req)
{
	ts_util::p(__FUNCTION__ " req: 0%x", req);
	ts_result_c *result = req->result_cb ? NULL : req->result;

	uv_sem_t *sem = NULL;
	if (!req->result_cb){
		req->sem = sem = (uv_sem_t *)malloc(sizeof(uv_sem_t));
		int r = uv_sem_init(req->sem, 0);
		if (r != 0){
			req->result->setResult(TS_ERR::LIBUV_ERR, "uv_sem_init fail");
			goto pm_exit;
		}
	}

	uv_async_t *async = (uv_async_t *)malloc(sizeof(uv_async_t));
	int r = uv_async_init(uv_default_loop(), async, ts_server_c::send_req_async);	
	async->data = req;
	r = uv_async_send(async);
	if (r != 0){
		free(async);
		req->result->setResult(TS_ERR::LIBUV_ERR, "uv_async_send fail");
		goto pm_exit;
	}	
	
	if (sem){
		ts_util::p(__FUNCTION__ " waiting......");
		uv_sem_wait(sem);
		uv_sem_destroy(sem);
	}
	
	ts_util::p(__FUNCTION__ " out");
	
pm_exit:
	if (sem){
		free(sem);
	}
	return result;
}

LIB_TS_SERVER_API ts_result_c *send_cmd_info(uint32_t devid, int cmd, TS_RESULT_CB cb)
{
	ts_util::p(__FUNCTION__ " -------------------------in------------------");

	ts_req_c *req = ts_req_c::newInstance(devid, cmd, cb);
	req->result = new ts_result_c;

	return send_req(req);
}

static ts_result_c*  send_cmd_pause(uint32_t devid, int cmd, uint8_t status, TS_RESULT_CB cb)
{
	ts_util::p(__FUNCTION__);
	ts_result_c *result = new ts_result_c;
	if (!result){
		return NULL;
	}

	ts_req_c *req = ts_req_c::newInstance(devid, cmd, cb);	
	req->result = result;
	
	req->data.status = status;	

	return send_req(req);
}

LIB_TS_SERVER_API void ts_free_result(ts_result_c *p)
{
	if (p) {
		delete p;
	}
}
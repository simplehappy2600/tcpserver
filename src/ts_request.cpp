/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#include "stdafx.h"
#include "ts_request.h"
#include "ts_client.h"
#include "ts_util.h"

ts_req_t* ts_req_t::newInstance(uint32_t devid, uint16_t cmd, TS_RESULT_CB result_cb)
{
	return new ts_req_t(devid, cmd, result_cb);
}

void ts_req_t::clean(ts_req_t *req)
{
	if (req){
		//
		delete req;
	}
}

ts_req_t::ts_req_t(uint32_t devid, uint16_t cmd, TS_RESULT_CB result_cb){
	this->seq = NULL;
	this->sem = NULL;
	this->timer = NULL;
	this->client = NULL;
	this->result = NULL;		
	this->devid = devid;
	this->cmd = cmd;
	this->result_cb = NULL;//result_cb;
}

static void pm_cmd_req_timeout(uv_timer_t* handle)
{		
	ts_req_t *req = (ts_req_t *)handle->data;	
	ts_util::p(__FUNCTION__ " req: 0x%x", req);

	//TODO 调用uv_timer_stop会不会有问题?
	req->finish(TS_ERR::TIMEOUT, "timeout");
}

void ts_req_t::start_timer()
{	
	ts_util::p(__FUNCTION__ " start timer req: %x", this);

	this->timer = (uv_timer_t *)malloc(sizeof(uv_timer_t));
	if (this->timer){
		uv_timer_init(uv_default_loop(), this->timer);
		this->timer->data = this;
		uv_timer_start(this->timer, pm_cmd_req_timeout, 5*1000, 0);	//TODO	
	} else {
		//TODO
		ts_util::p(__FUNCTION__ " malloc fail");
	}
}

void ts_req_t::finish(int err, char *msg)
{
	this->finish(true, err, msg);
}

void ts_req_t::finish(bool del_from_client, int err, char *msg)
{
	ts_util::p(__FUNCTION__" req: %x", this);
	if (del_from_client && this->client){
		this->client->del_req(this);
	}
	
	if (this->result){
		this->result->setResult(err, msg);
	}

	if (this->timer){
		ts_util::p(__FUNCTION__" stop timer");
		uv_timer_stop(this->timer);
		free(this->timer);
		this->timer = NULL;
	}

	if (this->result_cb){
		//TODO 放到其他线程中, 避免影响当前loop
		ts_util::p(__FUNCTION__" call result_cb");
		this->result_cb(this->result);
	} 

	if (this->sem){
		ts_util::p(__FUNCTION__" post sem");
		uv_sem_post(this->sem);
	}
}
/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#include "stdafx.h"
#include "ts_client.h"
#include "ts_protocol.h"
#include "ts_result.h"
#include "ts_util.h"

//----------------------------------------------------------------
static ts_client_holder pm_clients;

ts_client_holder::ts_client_holder()
{
	uv_mutex_init(&this->mutex);
}

void ts_client_holder::add(ts_client_c *client)
{
	uv_mutex_lock(&this->mutex);

	ts_client_c *p = clients[client->devid];
	if (!p){
		clients[client->devid] = client;
	}

	uv_mutex_unlock(&this->mutex);
}

void ts_client_holder::remove(ts_client_c *client)
{
	uv_mutex_lock(&this->mutex);

	ts_client_c *p = clients[client->devid];
	if (p){
		clients.erase(client->devid);
	}	

	uv_mutex_unlock(&this->mutex);
}

ts_client_c *ts_client_holder::get(uint32_t devid)
{
	uv_mutex_lock(&this->mutex);
	ts_client_c *p = clients[devid];
	uv_mutex_unlock(&this->mutex);

	return p;
}

//----------------------------------------------------------------
ts_client_c::ts_client_c(uv_tcp_t *handle) : req_seq(0), handle(NULL), devid(-1), recv_data_len(0), send_buf_ex(NULL)
{
	this->handle = handle;
	handle->data = this;
	ts_util::p(__FUNCTION__ " 0x%x", this);
	recv_buf_len = TS_RECV_BUF_LEN;
	this->recv_buf = (char *)malloc(recv_buf_len);
}

ts_client_c::~ts_client_c()
{
	if (this->recv_buf){
		free(this->recv_buf);
	}
}

void ts_client_c::reg()
{	
	ts_util::p(__FUNCTION__ " devid: 0x%x", this->devid);
	pm_clients.add(this);
}

void ts_client_c::unreg()
{
	ts_util::p(__FUNCTION__ "devid: 0x%x", this->devid);
	pm_clients.remove(this);
}

uint16_t ts_client_c::getSeq()
{
	req_seq++;
	if (req_seq == 0){
		req_seq++;
	}
	return this->req_seq;
}

static void pm_write_cb(uv_write_t *req, int status) {

	ts_client_c *client = (ts_client_c *)req->data;

    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
		
		uv_close((uv_handle_t *)client->handle, NULL);
    }
    free(req);

	if (client->send_buf_ex){
		ts_util::p(__FUNCTION__ " free send_buf_ex: %x", client->send_buf_ex);
		free(client->send_buf_ex);
		client->send_buf_ex = NULL;
	}
}

int ts_client_c::send_adu(char *send_buf, int len)
{
	ts_util::p(__FUNCTION__ "send: ");
	ts_util::showHex((unsigned char *)send_buf, len);

	uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
	if (req){
		req->data = this;
		uv_buf_t wrbuf = uv_buf_init(send_buf, len);
		return uv_write(req, (uv_stream_t *)this->handle, &wrbuf, 1, pm_write_cb);	
	} else {
		//TODO
		ts_util::p(__FUNCTION__ " malloc fail");
		return -1;
	}
}

void ts_client_c::close()
{		
	ts_util::p(__FUNCTION__);
	this->unreg();
	
	ts_util::p(__FUNCTION__" reqs size: %d", reqs.size());
	for(std::map<uint32_t, ts_req_c *>::iterator iter = reqs.begin(); iter != reqs.end(); iter++) {
		ts_req_c *req = iter->second;
		req->finish(false, TS_ERR::NET_ERR, "connection closed");
	}

	reqs.clear();
}

void ts_client_c::send_req(ts_req_c *req)
{
	ts_client_c *pmc = pm_clients.get(req->devid);
	if (!pmc){		
		ts_util::p(__FUNCTION__ " client not exits: %x\n", req->devid);
		req->finish(TS_ERR::DEV_OFFLINE, "device offline");
		ts_req_c::clean(req);
		return;
	}

	switch (req->cmd){
	case TS_CMD::INFO_1:	
		pmc->send_cmd_info(req);
		return;	
	case TS_CMD::PAUSE:	
		pmc->send_cmd_pause(req);
		return;
	default:		
		req->finish(TS_ERR::CMD_ERROR, "error command");
		return;
	};	
}

void ts_client_c::send_resp(uint16_t cmd, uint16_t seq, int err, char *msg)
{
	uint16_t resp_cmd = CMD_RESP(cmd, err);
	int msg_len = msg ? strlen(msg) : 0;
	int pdu_len = sizeof(uint32_t) + msg_len;
	char *buf = this->get_send_buf(pdu_len);
	ts_pdu_str_t *pdu = (ts_pdu_str_t *)(buf + LEN_ADU_HEAD);
	pdu->total = htonl(msg_len);
	if (msg){		
		memcpy(pdu->data, msg, msg_len);
	}

	this->send_pdu(buf, resp_cmd, seq, pdu_len);
}

char *ts_client_c::get_send_buf(int pdu_len)
{
	int adu_len = LEN_ADU_HEAD + pdu_len + LEN_ADU_TAIL;
	if (adu_len <= TS_SEND_BUF_LEN){
		return this->send_buf;
	} else {
		ts_util::p(__FUNCTION__" new send_buf_ex" );
		this->send_buf_ex = (char *)malloc(adu_len);
		return this->send_buf_ex;
	}
}

void ts_client_c::send_cmd_info(ts_req_c *req)
{
	ts_util::p(__FUNCTION__);
	this->send_req_pdu(get_send_buf(0), req, 0);	
}

void ts_client_c::send_cmd_pause(ts_req_c *req)
{
	int pdu_len = sizeof(uint8_t);
	char *send_buf = get_send_buf(pdu_len);
	//pdu
	uint8_t *pdu = (uint8_t *)(send_buf + LEN_ADU_HEAD);
	*pdu = req->data.status;	

	this->send_req_pdu(send_buf, req, pdu_len);
}

void ts_client_c::send_req_pdu(char *send_buf, ts_req_c *req, uint32_t pdu_len)
{			
	ts_util::p(__FUNCTION__);
		
	this->add_req(req);		
	req->start_timer();

	send_pdu(send_buf, req, pdu_len);
}

void ts_client_c::send_pdu(char *send_buf, ts_req_c *req, uint32_t pdu_len)
{			
	ts_util::p(__FUNCTION__);

	send_pdu(send_buf, req->cmd, req->seq, pdu_len);
}

void ts_client_c::send_pdu(char *send_buf, uint16_t cmd, uint16_t seq, uint32_t pdu_len)
{
	ts_util::p(__FUNCTION__ " cmd[0x%x],seq[%d], pdu_len[%d]", cmd, seq, pdu_len);
	ts_adu_head_t *send_adu_head = (ts_adu_head_t *)send_buf;
	send_adu_head->cmd = htons(cmd);
	send_adu_head->seq = htons(seq);
	send_adu_head->len = htonl(pdu_len);

	ts_adu_tail_t *send_adu_tail = (ts_adu_tail_t *)(send_buf + LEN_ADU_HEAD + pdu_len);
	send_adu_tail->crc = htons(ts_util::crc16((unsigned char *)send_buf, LEN_ADU_HEAD + pdu_len));

	int adu_len = LEN_ADU_HEAD + pdu_len + LEN_ADU_TAIL;
	this->send_adu(send_buf, adu_len);
}

void ts_client_c::add_req(ts_req_c *req)
{	
	req->seq = this->getSeq();
	req->client = this;	
	this->reqs[req->seq] = req;	
	ts_util::p(__FUNCTION__ " add req: 0x%x, seq: %d", req, req->seq);
	ts_util::p(__FUNCTION__ " reqs size: %d", reqs.size());
}

void ts_client_c::del_req(ts_req_c *req)
{
	ts_util::p(__FUNCTION__ " del req: %x", req);
	this->reqs.erase(req->seq);
	req->client = NULL;
	ts_util::p(__FUNCTION__ " reqs size: %d", reqs.size());
}

ts_req_c *ts_client_c::get_req(int seq)
{	
	return this->reqs[seq];
}
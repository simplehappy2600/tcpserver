/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/
#pragma once

#include <map>
#include <vector>
#include "ts_request.h"

//TODO config
#define TS_RECV_BUF_LEN 1024
#define TS_SEND_BUF_LEN 512

//¿Í»§¶Ë
class ts_client_t {
public:
	ts_client_t(uv_tcp_t *handle);
	~ts_client_t();
	void reg();
	void unreg();
	void close();

	char *get_send_buf(int pdu_len);
	static void send_req(ts_req_t *req);
	int send_adu(char *buf, int len);
	void send_pdu(char *send_buf, ts_req_t *req, uint32_t pdu_len);
	void send_pdu(char *send_buf, uint16_t cmd, uint16_t seq, uint32_t pdu_len);	
	void send_resp(uint16_t cmd, uint16_t seq, int err, char *msg);

	void add_req(ts_req_t *req);
	void del_req(ts_req_t *req);
	ts_req_t *get_req(int seq);
private:	
	uint16_t getSeq();
	void send_cmd_info(ts_req_t *req);	
	void send_cmd_pause(ts_req_t *req);			
	void send_req_pdu(char *send_buf, ts_req_t *req, uint32_t pdu_len);	
public:
	uv_tcp_t *handle;
	char *recv_buf;
	int recv_buf_len;
	int recv_data_len;

	//TODO 
	char send_buf[TS_SEND_BUF_LEN];	
	//TODO 
	char *send_buf_ex;

	uint32_t devid;		 //deviceId
private:
	std::map<uint32_t, ts_req_t *> reqs;
	uint16_t req_seq;
} ;

class ts_client_holder {
public:
	ts_client_holder();
	void add(ts_client_t *client);
	void remove(ts_client_t *client);
	ts_client_t *get(uint32_t devid);
private:
	uv_mutex_t mutex;
	std::map<uint32_t, ts_client_t*> clients;
};


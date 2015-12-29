/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#include "stdafx.h"
#include "lib_ts_server.h"
#include "ts_protocol.h"
#include "ts_util.h"

#define DEFAULT_PORT 8899
#define DEFAULT_BACKLOG 128

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {

	ts_client_t* pmc = (ts_client_t *)handle->data;	

	if (pmc->recv_data_len ==  pmc->recv_buf_len){
		ts_util::p(__FUNCTION__ " realloc recv buf");
		pmc->recv_buf_len += TS_RECV_BUF_LEN;
		pmc->recv_buf = (char *)realloc(pmc->recv_buf, pmc->recv_buf_len);
	}

	buf->base = pmc->recv_buf + pmc->recv_data_len;
	buf->len = pmc->recv_buf_len - pmc->recv_data_len;	
}

void pm_close_cb(uv_handle_t* handle)
{
	ts_util::p(__FUNCTION__ " connection close");
	if (handle->data){		
		ts_client_t* pmc = (ts_client_t*)handle->data;		
		pmc->close();		
		ts_util::p(__FUNCTION__ " free ts_client_t: 0x%x", pmc);
		delete pmc;
	} else {

	}
}

//数据接收
void pm_read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
	
	if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "\nRead error %s", uv_err_name(nread));
		}
        uv_close((uv_handle_t*) client, pm_close_cb);
    } else if (nread > 0) {
		ts_client_t *pmc = (ts_client_t *)client->data;
		pmc->recv_data_len += nread;
		parse_adu(pmc);
    }
}

void on_new_connection(uv_stream_t *server, int status) 
{	
	ts_util::p(__FUNCTION__);

    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), client);
	uv_tcp_nodelay(client, true);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
		ts_util::p(__FUNCTION__ " accept connection");
		//新建客户端，开始接收数据
		ts_client_t *pmc = new ts_client_t(client);
        uv_read_start((uv_stream_t*) client, alloc_buffer, pm_read_cb);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

//loop在此线程
void ts_loop_th(void *arg)
{	
	ts_util::p(__FUNCTION__ "in");

	ts_req_t *req = (ts_req_t *)arg;
	int port = req->data.port;	

	int r = 0;
	do{
		uv_tcp_t *server = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
		r = uv_tcp_init(uv_default_loop(), server);
		if (r != 0){
			req->result->setResult(TS_ERR::INIT_ERR, "uv_tcp_init fail");
			break;
		}

		if (port <= 0){
			port = DEFAULT_PORT;
		}
		struct sockaddr_in addr;
		r = uv_ip4_addr("0.0.0.0", port, &addr);
		if (r != 0){
			req->result->setResult(TS_ERR::INIT_ERR, "uv_ip4_addr fail");
			break;
		}

		r = uv_tcp_bind(server, (const struct sockaddr*)&addr, 0);
		if (r != 0){
			req->result->setResult(TS_ERR::INIT_ERR, "uv_tcp_bind fail");
			break;
		}
		r = uv_listen((uv_stream_t*) server, DEFAULT_BACKLOG, on_new_connection);
		if (r != 0){
			//fprintf(stderr, "Listen error %s\n", uv_strerror(r));
			req->result->setResult(TS_ERR::INIT_ERR, "uv_listen fail");
			break;
		}
	}while(false);

	req->finish(TS_ERR::OK, NULL);

	if (r == 0){
		r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		if (r != 0){
			//TODO log
			ts_util::p(__FUNCTION__" call loop fail");
		}
	} else {
		ts_util::p(__FUNCTION__" init fail");
	}

	return;
}

void send_req_async(uv_async_t* handle)
{
	ts_util::p(__FUNCTION__);

	ts_req_t *req = (ts_req_t *)handle->data;
	ts_client_t::send_req(req);

	free(handle);
}
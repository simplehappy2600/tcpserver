/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#pragma once

namespace TS_ERR {
	const int OK			= 0;
	const int UNKNOWN		= -1;
	const int DEV_OFFLINE	= -2;
	const int CMD_ERROR		= -3;
	const int TIMEOUT		= -4;
	const int NET_ERR		= -5;
	const int INIT_ERR		= -6;
	const int LIBUV_ERR		= -7;
}

class ts_result_c{
public:	
	int code;	//TS_ERR错误码
	int count;	//data个数 参见协议pdu
private:
	char *msg;	//错误消息
	char *data;	//数据(多\n分割) 参见协议pdu
public:
	ts_result_c() : code(TS_ERR::UNKNOWN), count(0), msg(NULL), data(NULL){}
	~ts_result_c();

	char *getMsg(){return this->msg;}
	char *getData(){return this->data;}

	void setMsg(char *val);	
	void setData(char *val);
	void setData(char *val, int len);

	void setResult(int code, char *msg);
} ;

typedef void (*TS_RESULT_CB)(ts_result_c *r);
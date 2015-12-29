/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#include "stdafx.h"
#include "ts_result.h"
#include <stdlib.h>
#include <string.h>

ts_result_c::~ts_result_c()
{
	if (this->msg){
		free(this->msg);
	}
	if (this->data){
		free(this->data);
	}
}

void ts_result_c::setMsg(char *val)
{
	if (this->msg){
		free(this->msg);
		this->msg = NULL;
	}
	if (val){
		int len = strlen(val) + 1;
		this->msg = (char *)malloc(len);
		memcpy(this->msg, val, len);
	}
}

void ts_result_c::setData(char *val){
	setData(val, strlen(val));
}

void ts_result_c::setData(char *val, int len)
{
	if (this->data){
		free(this->data);
		this->data = NULL;
	}	
	if (val){
		this->data = (char *)malloc(len + 1);
		memcpy(this->data, val, len);
		this->data[len] = NULL;
	}
}

void ts_result_c::setResult(int code, char *msg)
{
	this->code = code;
	this->setMsg(msg);
}
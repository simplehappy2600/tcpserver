/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#pragma once

#include <vector>
#include <string>
#include "lib_ts_server.h"
#include "ts_client.h"

class ts_util {
public:	
	static uint16_t crc16(unsigned char * buf, int len);
	static void showHex(unsigned char *buf, int len);
	static int p(const char * _Format, ...);
};
/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#include "stdafx.h"
#include "ts_util.h"
#include <stdarg.h>

uint16_t ts_util::crc16(unsigned char * buf, int len)
{
	//TODO
	return 0;
}

//TODO
void ts_util::showHex(unsigned char *buf, int len)
{
	static char bs[1024];
	for (int i = 0; i < len; i++){
		printf("%02x", buf[i]);
	}
	//p("");
}

int ts_util::p(const char * format, ...)
{
	//return 0;
	printf("\nth_%08d: ", GetCurrentThreadId());
	
    va_list args;  
    va_start(args, format);  
	//char buf [256];  
    //vsprintf_s (buf , format, args);  
    //printf ("%s", buf);  
	printf_s(format, args);
    va_end (args);    

	return 0;
}
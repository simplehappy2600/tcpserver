/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

/*
adu frame define:
	cmd(2byte) | seq(2byte) | len_of_pdu(4byte) | pdu(?byte) | crc(2byte)
	|                                                                   |
	~~~~~~~~~~~~~~~adu~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmd bits define:
	b15 b14 b13 b12 b11 b10 b9 b8  b7 b6 b05 b04 b03 b02 b01 b00
	~~  ~~~~~~~~~~~~~~~~~~~~~~~~~  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	resp          error                       cmd
*/

#pragma once

#include <stdint.h>
#include "ts_client.h"

#ifdef _MSC_VER
#include <Winsock2.h>
#endif

namespace TS_CMD {	
	const int INFO_1				= 0x0001;
	const int PAUSE					= 0x0002;	
	//client register
	const int REG					= 0x00FF;
}

//0-ÎÞ´íÎó£¬1-ÃüÁî´íÎó£¬2-crcÐ£Ñé´íÎó£¬3-ÆäËû´íÎó¡£
namespace TS_CMD_ERR {
	const int OK		= 0;
	const int CMD		= 1;
	const int CRC		= 2;
	const int UNKNOWN	= 3;
}

#pragma pack (1)
typedef struct {
	uint16_t cmd;
	uint16_t seq;
	uint32_t len;
} ts_adu_head_t;

typedef struct {
	uint16_t crc;	
} ts_adu_tail_t;

typedef struct {
	uint32_t total;
	char data[];
} ts_pdu_str_t;

typedef struct {
	uint8_t data;
} ts_pdu_byte_t;

typedef struct {	
	uint32_t devid;
} ts_pdu_devid_t;

#pragma pack ()

#define LEN_ADU_HEAD (2+2+4)

//CRC16
#define LEN_ADU_TAIL 2

#define CMD_RESP(cmd, err) ((cmd) | 0x8000 | ((err) & 0x3F00))
#define CMD_ERR(cmd) (((cmd) & 0x3F00) >> 8)
#define CMD_CMD(cmd) ((cmd) & 0x00FF)

void parse_adu(ts_client_t *pmc);
void parse_pdu(ts_client_t *pmc, ts_adu_head_t *adu_head);
void parse_pdu_reg(ts_client_t *pmc, ts_adu_head_t *adu_head);
ts_req_t* parse_pdu_info(ts_client_t *pmc, ts_adu_head_t *adu_head);
ts_req_t* parse_pdu_none(ts_client_t *pmc, ts_adu_head_t *adu_head);

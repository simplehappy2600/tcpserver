/*************************************************
Copyright: simplehappy2600@126.com
Date:2015-12-15
Description: 
    https://github.com/simplehappy2600/tcpserver
**************************************************/

#include "stdafx.h"
#include "ts_protocol.h"
#include "ts_util.h"

void parse_adu(ts_client_c *pmc)
{	
	ts_util::p(__FUNCTION__"+++++++++++++++++++++++++++++++++++");
	ts_util::showHex((unsigned char *)pmc->recv_buf, pmc->recv_data_len);	

	if (pmc->recv_data_len < LEN_ADU_HEAD + LEN_ADU_TAIL) {
		return;
	}

	ts_adu_head_t *adu_head = (ts_adu_head_t *)pmc->recv_buf;
	int pdu_len = ntohl(adu_head->len);
	int adu_len = LEN_ADU_HEAD + pdu_len + LEN_ADU_TAIL;
	if (pmc->recv_data_len < adu_len) {
		return;
	}

	ts_util::p(__FUNCTION__ "recv: ");
	//check CRC
	ts_adu_tail_t *send_adu_tail = (ts_adu_tail_t *)(pmc->recv_buf + LEN_ADU_HEAD + pdu_len);	
	uint16_t crc = ts_util::crc16((unsigned char *)pmc->recv_buf, LEN_ADU_HEAD + pdu_len);
	if (false && crc != ntohs(send_adu_tail->crc)){
		//TODO
		ts_util::p(__FUNCTION__ "crc not ok, close connection");
		pmc->close();
		return;
	}
	
	uint16_t cmd = ntohs(adu_head->cmd);
	ts_util::p(__FUNCTION__ " cmd: 0x%x", cmd);
	int cmd_err = CMD_ERR(cmd);	
	if (cmd_err == TS_CMD_ERR::OK){
		parse_pdu(pmc, adu_head);
	} else {
		//TODO log
		ts_util::p(__FUNCTION__ " cmd error %x", cmd);
	}

	if (pmc->recv_data_len > adu_len){
		memcpy(pmc->recv_buf, pmc->recv_buf + adu_len, pmc->recv_data_len - adu_len);
		pmc->recv_data_len -= adu_len;
		parse_adu(pmc);
	} else {
		pmc->recv_data_len = 0;
		if (pmc->recv_buf_len > TS_RECV_BUF_LEN){
			free(pmc->recv_buf);
			pmc->recv_buf_len = TS_RECV_BUF_LEN;
			pmc->recv_buf = (char *)malloc(pmc->recv_buf_len);
		}
	}
}

void parse_pdu(ts_client_c *pmc, ts_adu_head_t *adu_head)
{
	ts_req_c* req = NULL;
	uint16_t cmd = CMD_CMD(ntohs(adu_head->cmd));
	switch (cmd){
	case TS_CMD::REG:
		parse_pdu_reg(pmc, adu_head);
		break;
	case TS_CMD::INFO_1:	
		req = parse_pdu_info(pmc, adu_head);
		break;	
	case TS_CMD::PAUSE:	
		req = parse_pdu_none(pmc, adu_head);
		break;
	default:
		ts_util::p(__FUNCTION__ " unknown cmd: 0x%x", cmd);		
		pmc->send_resp(CMD_RESP(cmd, TS_CMD_ERR::OK), ntohs(adu_head->seq), TS_CMD_ERR::CMD, "command is invalid");
		break;
	}

	if (req){
		ts_req_c::clean(req);
	}
}

void parse_pdu_reg(ts_client_c *pmc, ts_adu_head_t *adu_head)
{
	ts_pdu_devid_t * pdu_devid = (ts_pdu_devid_t *)(pmc->recv_buf + LEN_ADU_HEAD);	
	pmc->devid = ntohl(pdu_devid->devid);
	ts_util::p(__FUNCTION__ " device id: 0x%x", pmc->devid);
	pmc->reg();

	ts_adu_head_t *recv_adu_head = (ts_adu_head_t *)pmc->recv_buf;
	uint16_t cmd = ntohs(recv_adu_head->cmd);	
	uint16_t seq = ntohs(recv_adu_head->seq);
	pmc->send_pdu(pmc->get_send_buf(0), CMD_RESP(cmd, TS_CMD_ERR::OK), seq, 0);
}

ts_req_c* parse_pdu_info(ts_client_c *pmc, ts_adu_head_t *adu_head)
{
	ts_pdu_str_t *pdu = (ts_pdu_str_t *)(pmc->recv_buf + LEN_ADU_HEAD);	
	int seq = ntohs(adu_head->seq);	
	ts_util::p(__FUNCTION__ " seq: %d", seq);
	
	ts_req_c *req = pmc->get_req(seq);
	if (req){
		ts_util::p(__FUNCTION__ " get req[%x] by seq: %d", req, seq);
		req->result->count = ntohl(pdu->total);
		req->result->setData(pdu->data, ntohl(adu_head->len) - sizeof(pdu->total));

		req->finish(TS_ERR::OK, "ok");

		//resp
		pmc->send_resp(req->cmd, req->seq, TS_CMD_ERR::OK, NULL);

	} else {
		//ignore
		//TODO log
		ts_util::p(__FUNCTION__ "cannot get req by seq: %d", seq);
	}

	return req;
}

ts_req_c* parse_pdu_none(ts_client_c *pmc, ts_adu_head_t *adu_head)
{	
	int seq = ntohs(adu_head->seq);
	
	ts_req_c *req = pmc->get_req(seq);
	if (req){		
		req->finish(TS_ERR::OK, "ok");
	} else {
		//ignore
		//TODO log
		ts_util::p(__FUNCTION__ "cannot get req by seq: %d", seq);
	}

	return req;
}
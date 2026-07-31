// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 AIROHA Inc
*/
#ifndef _ECNT_HOOK_IFC_H_
#define _ECNT_HOOK_IFC_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include "ecnt_hook.h"
#include "ecnt_hook_ifc_type.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

static inline int IFC_API_SET_GLOBAL_ENABLE(unsigned char enableMode) {
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_GLOBAL_ENABLE;
	in_data.ifcEnable = enableMode;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_GET_GLOBAL_ENABLE(void) {
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_GET_GLOBAL_ENABLE;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.ifcEnable;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_SET_LUT_RULE_AUTO(struct ecnt_ifc_param *ifc_param_ptr) {
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_LUT_RULE_AUTO;
	memcpy(&in_data.ifc_param, ifc_param_ptr, sizeof(struct ecnt_ifc_param));
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_DEL_LUT_RULE_AUTO(struct ecnt_ifc_param *ifc_param_ptr) {
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_DEL_LUT_RULE_AUTO;
	memcpy(&in_data.ifc_param, ifc_param_ptr, sizeof(struct ecnt_ifc_param));
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_SET_LUT0_RULE(uint ifcIndex, IFC_Mode_t enMode, uint field, uint command, uint mask_low, uint mask_high, uint key_low, uint key_high) {
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_LUT0_RULE;
	in_data.ifcIndex = ifcIndex;
	in_data.enMode = enMode;
	in_data.field = field;
	in_data.command = command;
	in_data.mask_low = mask_low;
	in_data.mask_high = mask_high;
	in_data.key_low = key_low;
	in_data.key_high = key_high;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_SET_LUT1_RULE(uint ifcIndex, IFC_Mode_t enMode, uint endFlag, uint field, uint command, uint mask_low, uint key_low, uint key_high) {
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_LUT1_RULE;
	in_data.ifcIndex = ifcIndex;
	in_data.enMode = enMode;
	in_data.endFlag = endFlag;
	in_data.field = field;
	in_data.command = command;
	in_data.mask_low = mask_low;
	in_data.key_low = key_low;
	in_data.key_high = key_high;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_SET_ACTION(uint ifcIndex, uint actIdx, IFC_Mode_t enMode, uint value0, uint value1, uint value2, uint value3) {
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_ACTION;
	in_data.ifcIndex = ifcIndex;
	in_data.actIdx = actIdx;
	in_data.enMode = enMode;
	in_data.value0 = value0;
	in_data.value1 = value1;
	in_data.value2 = value2;
	in_data.value3 = value3;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_SET_VIP_BY_ETYPE(unsigned short ether_type)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_VIP;
	in_data.field = IFC_VIP_FIELD_ETYPE;
	in_data.ifc_param.key[0] = ether_type;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_SET_VIP_BY_IPV4_PROTO(unsigned short proto)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_VIP;
	in_data.field = IFC_VIP_FIELD_IPV4_PROTO;
	in_data.ifc_param.key[0] = proto;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_SET_VIP_BY_IPV6_NXTHDR(unsigned short nxthdr)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_VIP;
	in_data.field = IFC_VIP_FIELD_IPV6_NXTHDR;
	in_data.ifc_param.key[0] = nxthdr;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*
parameter:
		sport/dport -- TCP/UDP port,0 means not care
		mainly for VOIP
*/
static inline int IFC_API_SET_VIP_BY_PORT(unsigned short sport,unsigned short dport)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_VIP;
	in_data.field = IFC_VIP_FIELD_PORT;
	in_data.ifc_param.key[0] = sport;
	in_data.ifc_param.key[1] = dport;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*
parameter:
		sip -- format is hex,ex 192.168.1.1->0xc0a80101,0 means not care
		sport -- TCP/UDP sport,0 means not care
		mainly for TR069
*/
static inline int IFC_API_SET_VIP_BY_SIP_AND_SPORT(unsigned int sip,unsigned short sport)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_VIP;
	in_data.field = IFC_VIP_FIELD_SIP_SPORT;
	in_data.ifc_param.key[0] = sip;
	in_data.ifc_param.key[1] = sport;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_DEL_VIP_BY_ETYPE(unsigned short ether_type)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_DEL_VIP;
	in_data.field = IFC_VIP_FIELD_ETYPE;
	in_data.ifc_param.key[0] = ether_type;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_DEL_VIP_BY_IPV4_PROTO(unsigned short proto)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_DEL_VIP;
	in_data.field = IFC_VIP_FIELD_IPV4_PROTO;
	in_data.ifc_param.key[0] = proto;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_DEL_VIP_BY_IPV6_NXTHDR(unsigned short nxthdr)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_DEL_VIP;
	in_data.field = IFC_VIP_FIELD_IPV6_NXTHDR;
	in_data.ifc_param.key[0] = nxthdr;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*
parameter:
		sport/dport -- TCP/UDP port,0 means not care
*/
static inline int IFC_API_DEL_VIP_BY_PORT(unsigned short sport,unsigned short dport)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_DEL_VIP;
	in_data.field = IFC_VIP_FIELD_PORT;
	in_data.ifc_param.key[0] = sport;
	in_data.ifc_param.key[1] = dport;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*
parameter:
		dip -- format is hex,ex 192.168.1.1->0xc0a80101,0 means not care
		dport -- TCP/UDP dport,0 means not care
		mainly for TR069
*/
static inline int IFC_API_DEL_VIP_BY_SIP_AND_SPORT(unsigned int sip,unsigned short sport)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_DEL_VIP;
	in_data.field = IFC_VIP_FIELD_SIP_SPORT;
	in_data.ifc_param.key[0] = sip;
	in_data.ifc_param.key[1] = sport;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int IFC_API_GET_IDX(void)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_GET_IDX;
	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	{
		return in_data.ifcIndex;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int IFC_API_SET_RULE(uint ifcIndex, uint fieldIdx, uint key, uint mask)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_RULE;
	in_data.ifcIndex = ifcIndex;
	in_data.field = fieldIdx;
	in_data.key_low = key;
	in_data.mask_low = mask;

	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int IFC_API_SET_ACT(uint ifcIndex, IFC_Mode_t enMode, uint endFlag, uint act_en, uint actIdx, uint value0, uint value1, uint value2)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_ACT;
	in_data.ifcIndex = ifcIndex;
	in_data.enMode = enMode;
	in_data.endFlag = endFlag;

	in_data.value3 = act_en;
	in_data.actIdx = actIdx;
	in_data.value0 = value0;
	in_data.value1 = value1;
	in_data.value2 = value2;

	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int IFC_API_SET_MIB(uint mib_cmd, uint mib_id, struct ecnt_mib_cnt *cnt)
{
	/* CID:711139 */
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_SET_MIB;
	in_data.ifcIndex = mib_cmd;
	in_data.field = mib_id;

	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	cnt->byte_low = in_data.value0;
	cnt->byte_high = in_data.value1;
	cnt->pkt_low = in_data.value2;
	cnt->pkt_high = in_data.value3;

	if(ret != ECNT_HOOK_ERROR)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int IFC_API_GET_STAT(uint ifcIndex)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_GET_STAT;
	in_data.ifcIndex = ifcIndex;

	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int IFC_API_SET_QDMA_FTP(unsigned char tcp_type, unsigned int tcp_dip)
{
	int ret = 0;
	struct ecnt_ifc_data in_data ={0};
	memset(&in_data, 0, sizeof(struct ecnt_ifc_data));

	in_data.function_id = IFC_SET_QDMA_FTP;
	in_data.api_data.tcp_data.tcp_type = tcp_type;
	in_data.api_data.tcp_data.tcp_dip = tcp_dip;

	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int IFC_API_GET_HIT_IDX_ACTION(unsigned short hit_idx, unsigned char* action)
{
	struct ecnt_ifc_data in_data ={0};
	int ret = 0;

	in_data.function_id = IFC_GET_HIT_IDX_ACTION;
	in_data.ifcIndex    = hit_idx;

	ret = __ECNT_HOOK(ECNT_IFC, ECNT_IFC_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
    {
        *action = in_data.actIdx;
        return in_data.retValue;
    }

    return ECNT_HOOK_ERROR;
}

#endif /* _ECNT_HOOK_IFC_H_ */


/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software)
are protected under relevant copyright laws. The information contained herein
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or
its licensors. Without the prior written permission of EcoNet and/or its licensors,
any reproduction, modification, use or disclosure of EcoNet Software, and
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE.
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH
ECONET SOFTWARE.
***************************************************************/
#ifndef _ECNT_HOOK_PPE_H_
#define _ECNT_HOOK_PPE_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include "ecnt_hook.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define MAX_NAME_LENGTH             (16)
#define MC_EXT_PORT_MAX_NUM	(4)
/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
typedef enum {
	PPE_DISABLE = 0 ,
	PPE_ENABLE
} PPE_Enable_t;

typedef enum {
	PPE_NOT_PPPOE = 0 ,
	PPE_IS_PPPOE
} PPE_PPPOE_t;

enum {
    ECNT_DRIVER_PPE_API = 0,
	ECNT_DRIVER_PPE_API_MCST_EX,
};

typedef enum {
	PPE_API_ID_CLEAN_FOE_TABLE = 0,
    PPE_API_ID_DUMP_FOE_PKT,
    PPE_API_ID_ADD_FOE_ENTRY,
    PPE_API_ID_DLF_CLEAR_INVALID,
    PPE_API_ID_DLF_IF_ENABLE,
    PPE_API_ID_SET_TLS_CFG,
    PPE_API_ID_SET_TLS_VID,
    PPE_API_ID_SET_KA_CFG,
    PPE_API_ID_GET_AGING_CFG,
    PPE_API_ID_SET_AGING_CFG,
	PPE_API_ID_UPDATE_MULTICAST_LIST,
	PPE_API_ID_CLEAR_MULTICAST_LIST,
    PPE_API_GET_MC_ORIGDEV,
    PPE_API_ID_MULTICAST_SUBSCRIBE_GROUP,
    PPE_API_ID_MULTICAST_GET_LOCAL,
    PPE_API_ID_SET_PPE_PORT,
    PPE_API_ID_CLEAN_FOEENTRY_BY_GEMPORT,
    PPE_API_ID_SET_MULTICAST_VLAN_1TON,
	PPE_API_ID_MAX_NUM
} PPE_HookFunctionID_t ;

typedef enum {
	PPE_MCST_EX_API_ID_GET_PORTMASK = 0,
	PPE_MCST_EX_MAX_NUM
} PPE_MCST_EX_HookFunctionID_t ;

struct ecnt_ppe_tls_cfg
{
	unchar vid_idx[5];/*VID_IDX0~4*/
	unchar sp;
	unchar mode;
	unchar range;
	unchar vid_en;
	unchar ma;
	unchar cm;
	unchar en;
};

typedef enum
{
	KA_DISABLE = 0,
	KA_UNICAST = 1,
	KA_DUPLICATE = 3,
}KA_Cfg_Value_t;

typedef enum
{
	PPE_MULTICAST_PROTO_IPV4 = 4,
	PPE_MULTICAST_PROTO_IPV6 = 6,
}PPE_MULTICAST_PROTO;

typedef struct
{
	unsigned int proto;/*ipv4 or ipv6*/
	unsigned int vlan_tag_num;
	unsigned short outer_tci;
	unsigned short inner_tci;
	unsigned char grp_addr[16];
	unsigned char src_addr[16];
	/*TCSUPPORT_CT&&TCSUPPORT_PORTBIND*/
	struct net_device* ori_dev;
#if defined(CONFIG_BRIDGE_VLAN_FILTERING)
	unsigned short br_vid;
#endif
	unsigned int ext_port_num;
	struct net_device* ext_port[MC_EXT_PORT_MAX_NUM];
}PPE_MULTICAST_INFO_t;

typedef struct
{
	PPE_MULTICAST_INFO_t* info;
	unsigned int update_mode;
	unsigned int op_type;
	unsigned int port_mask;
	unsigned int local;
}PPE_MULTICAST_UPDATE_t;

typedef enum
{
	PPE_MULTICAST_UPDATE_MODE_VLAN=1<<0,
	PPE_MULTICAST_UPDATE_MODE_GRPIP=1<<1,
	PPE_MULTICAST_UPDATE_MODE_SRCIP=1<<2,
}PPE_MULTICAST_UPDATE_MODE;

typedef struct mcvlan_info_s {
    int vlan_id;
    unsigned char vlan_op;
    unsigned char vlan_vpm;
}MCVLAN_INFO;

typedef struct mc_port_info_s{
    unsigned char portid;
    unsigned char is_ipv6;
    unsigned char multicast_addr[16];
    MCVLAN_INFO* mcvlan_info;
}MC_PORT_INFO;

struct ecnt_ppe_data {
	PPE_HookFunctionID_t function_id;
	int retValue;
	uint index;
	union {
		void *skb;
		void *hwnat_tuple;
		struct ecnt_ppe_tls_cfg *ppe_tls_cfg;
		struct
		{
			ushort vid_lo;
			ushort vid_hi;
		}tls_vid;
		KA_Cfg_Value_t ka_cfg;
		uint aging_cfg;
		uint ppe_port;
		int gemport;
		PPE_MULTICAST_UPDATE_t multicast_update;
		PPE_MULTICAST_INFO_t*  multicast_info;
	};
    MC_PORT_INFO* mcport_info;
};

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
static inline int PPE_API_CLEAN_FOE_TABLE(void)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id = PPE_API_ID_CLEAN_FOE_TABLE;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_DUMP_FOE_PKT(void *skb, uint index)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id = PPE_API_ID_DUMP_FOE_PKT;

    in_data.skb = skb;
    in_data.index = index;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_ADD_FOE_ENTRY(void *hwnat_tuple)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id = PPE_API_ID_ADD_FOE_ENTRY;

    in_data.hwnat_tuple = hwnat_tuple;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_DLF_CLEAR_INVALID_INDEX(void)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id = PPE_API_ID_DLF_CLEAR_INVALID ;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_DLF_IF_ENABLE(void)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id = PPE_API_ID_DLF_IF_ENABLE ;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_SET_TLS_CFG(uint index,struct ecnt_ppe_tls_cfg *ppe_tls_cfg)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id = PPE_API_ID_SET_TLS_CFG ;

	in_data.index = index;
	in_data.ppe_tls_cfg = ppe_tls_cfg;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_SET_TLS_VID(uint index,ushort vid_lo,ushort vid_hi)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id = PPE_API_ID_SET_TLS_VID ;

	in_data.index = index;
	in_data.tls_vid.vid_lo = vid_lo;
	in_data.tls_vid.vid_hi = vid_hi;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_SET_KA_CFG(KA_Cfg_Value_t ka_cfg)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_ID_SET_KA_CFG;

	in_data.ka_cfg = ka_cfg;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_GET_AGING_CFG(uint *aging_cfg)
{
	/* CID:708733 */
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_ID_GET_AGING_CFG;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	*aging_cfg = in_data.aging_cfg;

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_SET_AGING_CFG(uint aging_cfg)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_ID_SET_AGING_CFG;

	in_data.aging_cfg = aging_cfg;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_MULTICAST_HWNATENTRY_LIST_UPDATE(PPE_MULTICAST_INFO_t* info,unsigned int update_mode,unsigned int op_type, unsigned int port_mask, unsigned int local)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_ID_UPDATE_MULTICAST_LIST;
	in_data.multicast_update.info = info;
	in_data.multicast_update.update_mode = update_mode;
	in_data.multicast_update.op_type = op_type;
	in_data.multicast_update.port_mask = port_mask;
	in_data.multicast_update.local = local;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_MULTICAST_HWNATENTRY_LIST_CLEAR(void)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_ID_CLEAR_MULTICAST_LIST;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_MULTICAST_HWNAT_GET_PORTMASK(PPE_MULTICAST_INFO_t* info,unsigned int foe_index,unsigned int* port_mask)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = (PPE_HookFunctionID_t)PPE_MCST_EX_API_ID_GET_PORTMASK;
	in_data.index = foe_index;
	in_data.multicast_info = info;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API_MCST_EX, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
	{
		*port_mask = in_data.retValue;
		return ret;
	}
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_MULTICAST_SUBSCRIBE_GROUP(PPE_MULTICAST_INFO_t *info,unsigned int update_mode)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_ID_MULTICAST_SUBSCRIBE_GROUP;
	in_data.multicast_update.info = info;
	in_data.multicast_update.update_mode = update_mode;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_MULTICAST_GET_LOCAL(PPE_MULTICAST_INFO_t* info,unsigned int* local)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_ID_MULTICAST_GET_LOCAL;
	in_data.multicast_info = info;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
	{
		*local = in_data.retValue;
		return ret;
	}
	else
		return ECNT_HOOK_ERROR;
}

static inline int PPE_API_GET_MULTICAST_ORIGDEV(PPE_MULTICAST_INFO_t* info){
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_GET_MC_ORIGDEV;
	in_data.multicast_update.info = info;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

}

static inline int PPE_API_SET_PPE_PORT(uint ppe_port)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_ID_SET_PPE_PORT;

	in_data.ppe_port = ppe_port;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}


static inline int PPE_API_SET_MULTICAST_VLAN_1TON(MC_PORT_INFO *mcport_info)
{
	struct ecnt_ppe_data in_data ={0};
	int ret = 0;

	in_data.function_id  = PPE_API_ID_SET_MULTICAST_VLAN_1TON;

	in_data.mcport_info = mcport_info;

	ret = __ECNT_HOOK(ECNT_PPE, ECNT_DRIVER_PPE_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

#endif /* _ECNT_HOOK_PPE_H_ */



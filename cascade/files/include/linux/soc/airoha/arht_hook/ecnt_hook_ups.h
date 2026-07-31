/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _ECNT_HOOK_UPS_H
#define _ECNT_HOOK_UPS_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook.h>
#include <ecnt_hook/ecnt_hook_ups_tbl.h>


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
enum ECNT_UPS_SUBTYPE
{
    ECNT_DRIVER_UPS_API,
};

typedef enum {
	ECNT_UPS_FUNCTION_SET_FLTBL_INGRESS_INFO = 0,
	ECNT_UPS_FUNCTION_SET_FLTBL_EGRESS_INFO,
	ECNT_UPS_FUNCTION_READ_TBL,
	ECNT_UPS_FUNCTION_WRITE_TBL,
	ECNT_UPS_FUNCTION_READ_TCAM,
	ECNT_UPS_FUNCTION_WRITE_TCAM,
	ECNT_UPS_FUNCTION_ENABLE_VACL,
	ECNT_UPS_FUNCTION_READ_VACL_KEY,
	ECNT_UPS_FUNCTION_WRITE_VACL_KEY,
	ECNT_UPS_FUNCTION_READ_VACL_POLICY,
	ECNT_UPS_FUNCTION_WRITE_VACL_POLICY,
	ECNT_UPS_FUNCTION_ENABLE_EACL,
	ECNT_UPS_FUNCTION_READ_EACL_KEY,
	ECNT_UPS_FUNCTION_WRITE_EACL_KEY,
	ECNT_UPS_FUNCTION_READ_EACL_POLICY,
	ECNT_UPS_FUNCTION_WRITE_EACL_POLICY,	
	ECNT_UPS_FUNCTION_PORT_FLUSH,
	ECNT_UPS_FUNCTION_CHN_OFFLINE,
	ECNT_UPS_FUNCTION_MAX_NUM,
} ECNT_UPS_HookFunctionID_t ;

struct ups_fltbl_ingress_info
{
	struct 
	{
		uint8_t in_dev_name[32];
	}input;

	struct
	{
		uint32_t l3if_idx;
	}output;
};

struct ups_fltbl_egress_info
{
	struct
	{
		uint8_t dmac[6];
		uint8_t out_dev_name[32];
		uint8_t eport;
		uint8_t nat_type;
	}input;

	struct
	{
		uint32_t nhop_idx;
	}output;
};

struct ups_tbl_data
{
	UPS_TBL tbl_name;
	int tbl_idx;
	void* data;
	int data_size;
};

struct ups_tcam_data
{
	UPS_TCAM tcam_name;
	int tcam_idx;
	int valid;
	void* data;
	void* data_mask;
};

struct ups_vacl_en_data
{
	VACL_UCP_SEL ucp_sel;
	int enable;
	VACL_KEY_BLK1_SEL blk1_sel; 
	VACL_KEY_BLK2_SEL blk2_sel; 
};

struct ups_vacl_key_data
{
	VACL_UCP_SEL ucp_sel;
	int index;	
	int valid;
	struct vacl_key_s vacl_key;
	struct vacl_key_s vacl_key_mask;
};

struct ups_vacl_policy_data
{
	VACL_UCP_SEL ucp_sel;
	int index;	
	struct ipp_vacl_ucp_policy_s vacl_policy;
};

struct ups_eacl_en_data
{
	EACL_UCP_SEL ucp_sel;
	int enable;
	EACL_KEY_BLK1_SEL blk1_sel; 
	EACL_KEY_BLK2_SEL blk2_sel; 
	EACL_KEY_BLK3_SEL blk3_sel; 
};

struct ups_eacl_key_data
{
	EACL_UCP_SEL ucp_sel;
	int index;	
	int valid;
	struct eacl_key_s eacl_key;
	struct eacl_key_s eacl_key_mask;
};

struct ups_eacl_policy_data
{
	EACL_UCP_SEL ucp_sel;
	int index;	
	struct epp_eacl_ucp_policy_s eacl_policy;
};

struct ups_rsel_chn_offline_s
{
	uint32_t chn_id;
	int offline;
};

struct ecnt_ups_data
{
	ECNT_UPS_HookFunctionID_t function_id;
	union
	{	struct ups_fltbl_ingress_info* ing_info;
		struct ups_fltbl_egress_info* egr_info;
		struct ups_tbl_data *tbl_data;
		struct ups_tcam_data *tcam_data;
		struct ups_vacl_en_data* vacl_en_data;
		struct ups_vacl_key_data *vacl_key_data;
		struct ups_vacl_policy_data *vacl_policy_data;
		struct ups_eacl_en_data* eacl_en_data;
		struct ups_eacl_key_data *eacl_key_data;
		struct ups_eacl_policy_data *eacl_policy_data;
		uint32_t port_id;
		struct ups_rsel_chn_offline_s chn_offline;
	}api_data;
	int retValue;
};

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int ECNT_UPS_SET_FLTBL_INGRESS_INFO(struct ups_fltbl_ingress_info* info) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_SET_FLTBL_INGRESS_INFO; 
	in_data.api_data.ing_info = info;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_SET_FLTBL_EGRESS_INFO(struct ups_fltbl_egress_info* info) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_SET_FLTBL_EGRESS_INFO; 
	in_data.api_data.egr_info = info;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_READ_TBL(struct ups_tbl_data* tbl_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_READ_TBL; 
	in_data.api_data.tbl_data = tbl_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_WRITE_TBL(struct ups_tbl_data* tbl_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_WRITE_TBL; 
	in_data.api_data.tbl_data = tbl_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_READ_TCAM(struct ups_tcam_data* tbl_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_READ_TCAM; 
	in_data.api_data.tcam_data = tbl_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_WRITE_TCAM(struct ups_tcam_data* tbl_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_WRITE_TCAM; 
	in_data.api_data.tcam_data = tbl_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_ENABLE_VACL(struct ups_vacl_en_data* vacl_en_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_ENABLE_VACL; 
	in_data.api_data.vacl_en_data = vacl_en_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_READ_VACL_KEY(struct ups_vacl_key_data* vacl_key_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_READ_VACL_KEY; 
	in_data.api_data.vacl_key_data = vacl_key_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_WRITE_VACL_KEY(struct ups_vacl_key_data* vacl_key_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_WRITE_VACL_KEY; 
	in_data.api_data.vacl_key_data = vacl_key_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_READ_VACL_POLICY(struct ups_vacl_policy_data* vacl_policy_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_READ_VACL_POLICY; 
	in_data.api_data.vacl_policy_data = vacl_policy_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_WRITE_VACL_POLICY(struct ups_vacl_policy_data* vacl_policy_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_WRITE_VACL_POLICY; 
	in_data.api_data.vacl_policy_data = vacl_policy_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_ENABLE_EACL(struct ups_eacl_en_data* eacl_en_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_ENABLE_EACL; 
	in_data.api_data.eacl_en_data = eacl_en_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_READ_EACL_KEY(struct ups_eacl_key_data* eacl_key_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_READ_EACL_KEY; 
	in_data.api_data.eacl_key_data = eacl_key_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_WRITE_EACL_KEY(struct ups_eacl_key_data* eacl_key_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_WRITE_EACL_KEY; 
	in_data.api_data.eacl_key_data = eacl_key_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_READ_EACL_POLICY(struct ups_eacl_policy_data* eacl_policy_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_READ_EACL_POLICY; 
	in_data.api_data.eacl_policy_data = eacl_policy_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_WRITE_EACL_POLICY(struct ups_eacl_policy_data* eacl_policy_data) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_WRITE_EACL_POLICY; 
	in_data.api_data.eacl_policy_data = eacl_policy_data;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_PORT_FLUSH(uint32_t port_id) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_PORT_FLUSH; 
	in_data.api_data.port_id = port_id;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_UPS_RSEL_CHN_OFFLINE(uint32_t chn_id,int offline) {
	struct ecnt_ups_data in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_UPS_FUNCTION_CHN_OFFLINE; 
	in_data.api_data.chn_offline.chn_id = chn_id;
	in_data.api_data.chn_offline.offline = offline;
	ret = __ECNT_HOOK(ECNT_UPS_CORE, ECNT_DRIVER_UPS_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

#endif

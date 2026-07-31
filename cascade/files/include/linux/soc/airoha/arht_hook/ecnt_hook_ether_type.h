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

#ifndef _ECNT_HOOK_ETHER_TYPE_H
#define _ECNT_HOOK_ETHER_TYPE_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif
#include <ecnt_hook/eth_global_def.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define ECNT_DRIVER_API  0

#define MAX_VAILD_PORT_ENTRY  (16)

#define TRTCM_MATCH_MAC_RULE_NUM  		10
#define ACL_MAC_MATCH_TABLE_NUM 		3
#define ACL_MAC_UP_STREAM  				0
#define ACL_MAC_DOWN_STREAM  			1

#define TRTCM_RULE_NUM  32

#if 0
#ifdef TCSUPPORT_MAX_PACKET_2000
#define GDM1_LONG_LEN_VALUE 4004  //(RX_MAX_PKT_LEN -20),RX_MAX_PKT_LEN is defined in femac.c,20 is used for 4*VLAN,
#else
#define GDM1_LONG_LEN_VALUE 1700  //(RX_MAX_PKT_LEN-20),RX_MAX_PKT_LEN is defined in femac.c,20 is used for 4*VLAN
#endif

#if defined(TCSUPPORT_XPON_HAL_API)
#define XFI_ITF_PREFIX "eth"
#define XFI_ITF_START 0
#else
#define XFI_ITF_PREFIX "eth0."
#define XFI_ITF_START 1
#endif
#endif

/************************************************************************
*               M A C R O S
*************************************************************************
*/
#ifndef __KERNEL__
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
#endif
/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
/* Warning: same sequence with function array 'ether_operation' */
typedef enum {
    ETHER_FUNCTION_GSW_PBUS_READ,
    ETHER_FUNCTION_GSW_PBUS_WRITE,
    ETHER_FUNCTION_SET_RX_OCT_MODE,
    ETHER_FUNCTION_SET_TX_OCT_MODE,
    ETHER_FUNCTION_GET_PORT_COUNTER,
    ETHER_FUNCTION_CLEAR_ALL_PORT_COUNTER,
    ETHER_FUNCTION_GET_PER_PORT_MIB_COUNTER,
    ETHER_FUNCTION_CLEAR_PER_PORT_MIB_COUNTER,
    ETHER_FUNCTION_GET_PORT_STATUS,
    ETHER_FUNCTION_GET_PORT_LOOPBACK,
    ETHER_FUNCTION_SET_PORT_LOOPBACK,
    ETHER_FUNCTION_GET_CONFIGURATION_STATUS,
    ETHER_FUNCTION_GET_PORT_ADMIN,
    ETHER_FUNCTION_SET_PORT_ADMIN,
    ETHER_FUNCTION_SET_AUTONEG_RESTART_AUTO_CONFIG,
    ETHER_FUNCTION_GET_AUTO_DETECTION,
    ETHER_FUNCTION_SET_AUTO_DETECTION,
    ETHER_FUNCTION_GET_TRAFFIC_DESCRIPTOR,
    ETHER_FUNCTION_SET_TRAFFIC_DESCRIPTOR,
    ETHER_FUNCTION_CLEAN_TRAFFIC_DESCRIPTOR,
    ETHER_FUNCTION_GET_PORT_MAP,
    ETHER_FUNCTION_SET_PORT_MIRROR,
    ETHER_FUNCTION_GET_DROP_CRC_COUNTER,
    ETHER_FUNCTION_SET_PORT_MIRROR_ENABLE,
    ETHER_FUNCTION_SET_PORT_MIRROR_PORTBASED,
    ETHER_FUNCTION_MDIO_READ,
    ETHER_FUNCTION_MDIO_WRITE,
    ETHER_FUNCTION_MAC_SNED,
    ETHER_FUNCTION_EXT_GSW_PBUS_READ,
    ETHER_FUNCTION_EXT_GSW_PBUS_WRITE,
    ETHER_FUNCTION_SET_RATELIMIT_SWITCH,
    ETHER_FUNCTION_SET_MACTABLE_SYNC_EN,
    ETHER_FUNCTION_RGMII_SETTING,
    ETHER_FUNCTION_RGMII_MODE,
    ETHER_FUNCTION_SET_PORT_MATRIX,
    ETHER_FUNCTION_SET_PORT_LINKSTATE,
    ETHER_FUNCTION_SET_PER_VLAN_ACTION,
    ETHER_FUNCTION_SET_PER_PORT_VLAN_ACTION,
    ETHER_FUNCTION_MAC_AUTOBENCH_LOOPBACK,
    ETHER_FUNCTION_CLEAN_MACTABLE,
    ETHER_FUNCTION_USE_QDMA_WAN,
    ETHER_FUNCTION_RECV_PKT,
    ETHER_FUNCTION_GET_FORCE_DSTQ,
    ETHER_FUNCTION_CTAG_WHITE_LIST_MODE,
    ETHER_FUNCTION_CTAG_WHITE_LIST_PER_PORT_MODE,
    ETHER_FUNCTION_CTAG_WHITE_LIST_ADD,
    ETHER_FUNCTION_CTAG_WHITE_LIST_DEL,
    ETHER_FUNCTION_GET_PHY_ADDR,
    ETHER_FUNCTION_ADD_ARL_DIPTBL,
    ETHER_FUNCTION_ADD_ARL_SIPTBL,
    ETHER_FUNCTION_ADD_ARL_IPTBL_MULTI,
    ETHER_FUNCTION_SET_FLOW_CONTROL,
    ETHER_FUNCTION_GET_FLOW_CONTROL,
    ETHER_FUNCTION_MDIO_CL45_READ,
    ETHER_FUNCTION_MDIO_CL45_WRITE,
    ETHER_FUNCTION_GET_WOL,
    ETHER_FUNCTION_SET_WOL,
    ETHER_FUNCTION_GSW_REG_WRITE,
    ETHER_FUNCTION_GSW_REG_READ,
    ETHER_FUNCTION_MAX_NUM,
}ETHER_HookFunction_t;

typedef enum{
	PORT_LINK_DOWN = 0,
	PORT_LINK_UP,
}ETHER_PORT_LINKSTATE;

typedef enum{	
	ECNT_ETH_RX = 1,
	ECNT_ETH_TX,
}ecnt_eth_subtype;

typedef enum {
	NOT_ETHER_USE_QDMA_WAN = 0,
	USE_QDMA_WAN_ETHER,
	USE_QDMA_WAN_PON_XDSL,
}Wan_Mode;

typedef struct ECNT_ETHER_Data {
	ETHER_HookFunction_t function_id;	/* need put at first item */
	int retValue;
	
	union {
		struct {
	        unsigned int add;
	        unsigned int dev_add;
	        unsigned int reg;
	        unsigned int data;
	    }phy;
        struct sk_buff *skb;
        struct 
        {
            u8 wan_type;
            u8 interface;
            u8 mode;
        }traffic_setting;
        unsigned char ratelimit_En;
        struct
        {
            int portMatrixGroup[6];
            int type;
        }matrix_setting;
        struct
        {
            unsigned char lan_port;
            unsigned char switch_port;
        }port_map;
        struct
        {
            unsigned char port_no;
            unsigned char linkstate;
        }port_state;
        struct
        {
            unsigned char port_id;
            unsigned int o_vid;
            unsigned int n_vid;
            ECNT_SWITCH_VLAN_MODE vlan_mode;
            unsigned char enable;
        }vlantable_setting;
        int set_wan_mode;
        struct{
            void *msg_p;
            u32 msg_len;
            struct sk_buff *skb;
            u32 rx_len;
        }rx_info;
        int force_dstq;
        struct
        {
            u8 mode;
            u8 port;
            u16 vid;
        }ctag_white_list;
        struct
        {
            u32 dip;
            u32 destportmap;
            u32 status;
            u32 leaky_en;
            u32 eg_tag;
            u32 usr_pri;
            u32 is_extend_gsw;
        }arl_diptbl_info;
        struct
        {
            u32 dip;
            u32 sip;
            u32 destportmap;
            u32 is_extend_gsw;
        }arl_siptbl_info;
        struct
        {
            u32 dip;
            u32 sip;
            u32 sw_mask;
            u32 is_extend_gsw;
        }arl_mul_iptbl_info;
        struct
        {
            unsigned char port_id;
            mt7530_switch_api_MibCntType MibCntType;
            unsigned int cnt;
        }mib_counter;
        struct
        {
            unsigned char port_id;
            unsigned int rx_discard;
            unsigned int tx_discard;
            unsigned int rx_error;
            unsigned int tx_error;
        }drop_crc_counter;
        struct sk_buff **pskb;
        struct
        {
            unsigned char port_id;
            mt7530_switch_api_cnt port_cnt;
        }port_counter;
        struct
        {
            unsigned char port_id;
            unsigned char enable;
        }port_loopback;
        struct
        {
            unsigned char port_id;
            int status;
        }port_status;
        struct
        {
            unsigned char port_id;
            mt7530_switch_api_trafficDescriptor_t descriptor;
        }traffic_desc;
        struct
        {
            unsigned char port_id;
            uint8_t auto_detection;
        }auto_detec;
        struct
        {
            unsigned char port_id;
            uint8_t admin;
        }port_admin;        
        struct
        {
            unsigned char port_id;
            int status;
        }configuration_status;
        struct
        {
            unsigned char port_id;
        }auto_neg_restart;
        unsigned char oct_mode;
        struct
        {
            unsigned char port_id;
            unsigned char enable;
            mt7530_switch_api_port_mirror_port_based_t portbased;
        }port_mirror_cfg;
		struct
        {
            mt7530_switch_api_wol_status_t result;
        }wol_status;
		struct
        {
            mt7530_switch_api_wol_config_t data;
        }wol_cfg;
	} ether_private;
}ECNT_ETHER_Data_s;


/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/



#endif /*_ETHER_HOOK_TYPE_H*/

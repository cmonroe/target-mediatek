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
#ifndef _ECNT_HOOK_UPS_TYPE_H
#define _ECNT_HOOK_UPS_TYPE_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook.h>
#include <linux/types.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
typedef enum {
	DEB_AGING_MAP = 0,
	DEB_TSID_MAP,
	EFLEXT_EXT_BIT_SEL,
	EPAR_IPORT_PRTY,
	EPP_BD_PRTY,
	EPP_EACL_UCP0_POLICY,
	EPP_EACL_UCP1_POLICY,
	EPP_EACL_UCP2_POLICY,
	EPP_EACL_UCP3_POLICY,
	EPP_EACL_UCP4_POLICY,
	EPP_EACL_UCP5_POLICY,
	EPP_EACL_UCP6_POLICY,
	EPP_EACL_UCP7_POLICY,
	EPP_EVP_PRTY,
	EPP_FD_PRTY,
	EPP_L2_MTU,
	EPP_L3_MTU,
	EPP_L3IF_MAC,
	EPP_L3IF_PRTY,
	EPP_NHOP,
	EPP_PHB_TO_DSCP,
	EPP_PHB_TO_PCP_DEI,
	EPP_PORT_PRTY,
	EPP_TUNITABLE,
	EPP_VLAN,
	EPR_MET,
	FDD_PORT_PRTY,
	FLTBL_CFG,
	IFLEXT_EXT_BIT_SEL,
	IPAR_IPORT_PRTY,
	IPP_BD_PRTY,
	IPP_DEFAULT_PCP_DEI_TO_PHB,
	IPP_DOS_PROFILE,
	IPP_DQ_CHANNEL_PRTY,
	IPP_DSCP_TO_PHB,
	IPP_EL3_MTU,
	IPP_EL3IF_PRTY,
	IPP_EVP_PRTY,
	IPP_FD_PRTY,
	IPP_GEM_DS_PRTY_SRAM,
	IPP_GEM_PRTY,
	IPP_GEM_UNTAG_DSCP_TO_PCP,
	IPP_GEM_UNTAG_PCP,
	IPP_IL3IF_PRTY,
	IPP_IVP_PRTY,
	IPP_L2_MTU,
	IPP_L3_MTU,
	IPP_LAG,
	IPP_LAG_NON_UCFWD,
	IPP_MULTI_DEST,
	IPP_NHOP,
	IPP_PCP_DEI_TO_PHB,
	IPP_PORT_ISOLATION,
	IPP_STORM_CONTROL_PRTY,
	IPP_TUNT_IP_SRAM,
	IPP_UNS_PCP_TO_GEM_IDX,
	IPP_VACL_UCP0_POLICY,
	IPP_VACL_UCP1_POLICY,
	IPP_VACL_UCP2_POLICY,
	IPP_VACL_UCP3_POLICY,
	IPP_VACL_UCP4_POLICY,
	IPP_VACL_UCP5_POLICY,
	IPP_VACL_UCP6_POLICY,
	IPP_VACL_UCP7_POLICY,
	IPP_VLAN,
	IPP_VLAN_ISOLATION,
	IPP_VRF_PRTY,
	NAT_IPV6_PREFIX,
	PPPOE_PRTY,
	SCH_LV0_MAP,
	SCH_LV1_MAP,
	SPTAG_PRTY,
	IPP_L2_FDB
} UPS_TBL;

typedef enum
{
	IPP_VACL_UCP0_RULE_TCAM = 0,
	IPP_VACL_UCP1_RULE_TCAM,
	IPP_VACL_UCP2_RULE_TCAM,
	IPP_VACL_UCP3_RULE_TCAM,
	IPP_VACL_UCP4_RULE_TCAM,
	IPP_VACL_UCP5_RULE_TCAM,
	IPP_VACL_UCP6_RULE_TCAM,
	IPP_VACL_UCP7_RULE_TCAM,
	EPP_EACL_UCP0_RULE_TCAM,
	EPP_EACL_UCP1_RULE_TCAM,
	EPP_EACL_UCP2_RULE_TCAM,
	EPP_EACL_UCP3_RULE_TCAM,
	EPP_EACL_UCP4_RULE_TCAM,
	EPP_EACL_UCP5_RULE_TCAM,
	EPP_EACL_UCP6_RULE_TCAM,
	EPP_EACL_UCP7_RULE_TCAM,
	IPP_GEM_DS_PRTY_TCAM,
	IPP_TUNT_IP_TCAM,
}UPS_TCAM;
/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
typedef struct ipar_iport_prty_s
{
	uint64_t outer_tpid_enable  : 4;
	uint64_t inner_tpid_enable  : 4;
	uint64_t rx_sptag_en        : 1;
	uint64_t sptag_grp_sel      : 2;
	uint64_t rx_flextag_en      : 2;
	uint64_t eth_md_en          : 1;
	uint64_t udplite_en         : 1;
	uint64_t ipv4_cksc_dis      : 1;
	uint64_t tcp_cksc_dis       : 1;
	uint64_t udp_cksc_dis       : 1;
	uint64_t udplite_cksc_dis   : 1;
	uint64_t rx_ptp_eth_en      : 1;
	uint64_t rx_ptp_ip_en       : 1;
	uint64_t rx_dm_en           : 1;
	uint64_t is_lag             : 1;
	uint64_t phy_lag_idx        : 4;
	uint64_t sa_secure          : 1;
	uint64_t sw_lbn             : 4;
	uint64_t dos_profile_id     : 2;
	uint64_t tunt_en            : 1;
	uint64_t mgt_ctl_en         : 1;
	uint64_t predict_pkt_len    : 14;
	uint64_t reserved           : 4;
}ipar_iport_prty_t;

typedef struct epar_iport_prty_s
{
	uint32_t outer_tpid_enable  : 4;
	uint32_t inner_tpid_enable  : 4;
	uint32_t rx_sptag_en        : 1;
	uint32_t sptag_grp_sel      : 2;
	uint32_t rx_flextag_en      : 2;
	uint32_t eth_md_en          : 1;
	uint32_t udplite_en         : 1;
	uint32_t ipv4_cksc_dis      : 1;
	uint32_t tcp_cksc_dis       : 1;
	uint32_t udp_cksc_dis       : 1;
	uint32_t udplite_cksc_dis   : 1;
	uint32_t rx_ptp_eth_en      : 1;
	uint32_t rx_ptp_ip_en       : 1;
	uint32_t reserved           : 4; 
}epar_iport_prty_t;

#define ACCEPT_FRAME_TYPE_DROP_ALL 0
#define ACCEPT_FRAME_TYPE_TAG_ONLY 1
#define ACCEPT_FRAME_TYPE_UNTAG_ONLY 2
#define ACCEPT_FRAME_TYPE_ACCEPT_ALL 3
typedef struct fdd_port_prty_s
{
	/*first 64bit*/
	uint64_t port_grp							  : 4;
	uint64_t accept_frame_type 				  : 2;
	uint64_t is_vlan_trusted					  : 1;
	uint64_t is_1p_trusted 					  : 1;
	uint64_t is_assign_otag					  : 1;
	uint64_t is_assign_itag					  : 1;
	uint64_t outer_tpid_idx					  : 3;
	uint64_t outer_vid 						  : 12;
	uint64_t outer_pcp 						  : 3;
	uint64_t outer_dei 						  : 1;
	uint64_t inner_tpid_idx					  : 3;
	uint64_t inner_vid 						  : 12;
	uint64_t inner_pcp 						  : 3;
	uint64_t inner_dei 						  : 1;
	uint64_t is_bypass_vlan_filter 			  : 1;
	uint64_t is_bypass_vlan_member_filter		  : 1;
	uint64_t vlan_miss_action					  : 1;
	uint64_t is_pcp_dei_to_phb_profile_valid	  : 1;
	uint64_t pcp_dei_to_phb_profile_idx		  : 4;
	uint64_t is_dscp_to_phb_profile_valid		  : 1;
	uint64_t dscp_to_phb_profile_idx			  : 4;
	uint64_t mtu_idx1						 	 : 3;

	/*second 64bit*/
	uint64_t mtu_idx2							  : 1;
	uint64_t sa_miss_learn_ctrl				  : 2;
	uint64_t sa_miss_drop						  : 1;
	uint64_t sa_miss_to_cpu					  : 1;
	uint64_t sa_move_learn_ctrl				  : 2;
	uint64_t sa_move_drop						  : 1;
	uint64_t sa_move_to_cpu					  : 1;
	uint64_t sa_learn_fail_to_cpu				  : 1;
	uint64_t same_port_fwd 					  : 1;
	uint64_t igr_mirror_session				  : 4;
	uint64_t stp_state 						  : 2;
	uint64_t fcs_err_action					  : 2;
	uint64_t mtu_fail_action					  : 2;
	uint64_t inner_l2fwd_l4chksum_fail_action	  : 2;
	uint64_t inner_l3fwd_l4chksum_fail_action	  : 2;
	uint64_t outer_l4chksum_fail_action		  : 2;
	uint64_t ecn_capable						  : 1;
	uint64_t mtr_en							  : 1;
	uint64_t mtr_idx							  : 5;
	uint64_t reserved							  : 30; 
}fdd_port_prty_t;

typedef struct ipp_fd_prty_s
{
	// First 32 bits
	uint32_t mc_filter_mode:2;
	uint32_t dont_learn:1;
	uint32_t is_buuc_idx_l3rep:1;
	uint32_t buuc_multi_dest_idx:5;
	uint32_t is_umc_idx_l3rep:1;
	uint32_t umc_multi_dest_idx:5;
	uint32_t is_l3_enabled:1;
	uint32_t l3_ifidx:8;    
	uint32_t fltbl_cfg_idx:4;
	uint32_t igr_mirror_session:4;

	// Second 32 bits
	uint32_t is_pcp_dei_to_phb_profile_valid:1;
	uint32_t pcp_dei_to_phb_profile_idx:4;
	uint32_t is_dscp_to_phb_profile_valid:1;
	uint32_t dscp_to_phb_profile_idx:4;
	uint32_t sa_overwrite_da:1;
	uint32_t rsv:21;   // Remaining bits for padding
}ipp_fd_prty_t;

typedef struct ipp_nhop_s{
	uint16_t is_lag:1;
	uint16_t phy_lag_idx:4;
	uint16_t l3_ifidx:8;
	uint16_t grp:3;
}ipp_nhop_t;

typedef struct epp_nhop_s{
	uint8_t l3_ifidx;
	uint8_t mac[6];
	uint8_t no_mod_ttl:1;
	uint8_t grp:3;
}epp_nhop_t;

typedef struct ipp_evp_prty_s{
	uint64_t is_lag:1;
	uint64_t phy_lag_idx:4;
	uint64_t epp_delt_len:7;
	uint64_t ul_nhop:8;
	uint64_t mtr_en:1;
	uint64_t mtr_idx:9;
	uint64_t grp:3;
	uint64_t unused:31;
}ipp_evp_prty_t;

typedef struct epp_evp_prty_s{
	uint64_t mtu_idx:4;
	uint64_t is_mtu_fail_drop:1;
	uint64_t is_phb_to_pcp_dei_profile_valid:1;
	uint64_t phb_to_pcp_dei_profile:4;
	uint64_t is_phb_to_dscp_profile_valid:1;
	uint64_t phb_to_dscp_profile_idx:4;	
	uint64_t tuni_idx:7;	
	uint64_t egr_mirror_session:4;
	uint64_t cnt_en:1;
	uint64_t cnt_idx:8;
	uint64_t grp:3;
	uint64_t unused:26;
}epp_evp_prty_t;


typedef struct ipp_il3if_prty_s
{
	uint64_t mtu_idx            : 4;
	uint64_t my_mac_idx         : 2;
	uint64_t is_mtu_fail_drop   : 1;
	uint64_t is_mtu_fail_to_cpu : 1;
	uint64_t is_ipv4_enabled    : 1;
	uint64_t is_ipv6_enabled    : 1;
	uint64_t vrf_idx            : 7;
	uint64_t cnt_en             : 1;
	uint64_t cnt_idx            : 8;
	uint64_t mtr_en             : 1;
	uint64_t mtr_idx            : 9;
	uint64_t reserved           : 28;
}ipp_il3if_prty_t;

typedef struct ipp_el3if_prty_s
{
	uint32_t mtu_idx:4;
	uint32_t is_mtu_fail_drop:1;
	uint32_t is_mtu_fail_to_cpu:1;
	uint32_t is_no_mod_vlan:1;
	uint32_t epp_delt_len:7;
	uint32_t ul_nhop:8;
	uint32_t mtr_en:1;
	uint32_t mtr_idx:9;
}ipp_el3if_prty_t;

typedef struct epp_l3if_prty_s
{
	uint64_t mtu_idx             : 4;
	uint64_t mtu_fail_action     : 2;
	uint64_t my_mac_idx          : 7;
	uint64_t fdid                : 13;
	uint64_t is_no_mod_mac_sa    : 1;
	uint64_t is_no_mod_mac_da    : 1;
	uint64_t is_no_mod_vlan      : 1;
	uint64_t sip_prf_idx         : 7;
	uint64_t dip_prf_idx         : 7;
	uint64_t napt_type           : 5;
	uint64_t pppoe_idx           : 6;
	uint64_t vlan_action         : 1;
	uint64_t cnt_en              : 1;
	uint64_t cnt_idx             : 8;

	uint32_t tuni_idx            : 7; 
	uint32_t rsv            : 7; 
}epp_l3if_prty_t;

typedef struct epp_l3if_mac_s
{
	uint8_t mac_addr[6];
}epp_l3if_mac_t;

typedef enum
{
	PPPOE_DISABLE=0,
	PPPOE_ENABLE,
	PPPOE_UC_TO_MC,
}PPPOE_CMD;
typedef struct pppoe_prty_s
{
	uint32_t pppoe_cmd:2;
	uint32_t pppoe_session_id:16;
}pppoe_prty_t;

#define L3_LKP_MISS_DROP 0
#define L3_LKP_MISS_COPY2CPU 1
typedef struct ipp_vrf_prty_s
{
	uint32_t vrf_idx:10;
	uint32_t ipv4_hdr_option:2;
	uint32_t l3_lookup_miss:1;
	uint32_t is_ttl_0_to_cpu:1;
	uint32_t is_ttl_1_to_cpu:1;
	uint32_t rsv:17;
}ipp_vrf_prty_t;

typedef struct ipp_l3_mtu_s
{
	uint32_t mtu_size:14;
	uint32_t rsv:18;
}ipp_l3_mtu_t;

typedef struct epp_l3_mtu_s
{
	uint32_t mtu_size:14;
	uint32_t rsv:18;
}epp_l3_mtu_t;

typedef struct epp_fd_prty_s
{
	uint32_t fd_group_label:6;
	uint32_t vid_as_ovid:1;
	uint32_t vid:12;
	uint32_t is_phb_to_pcp_dei_profile_valid:1;
	uint32_t phb_to_pcp_dei_profile_idx:4;
	uint32_t is_phb_to_dscp_profile_vaild:1;
	uint32_t phb_to_dscp_profile_idx:4;
	uint32_t tpid_idx:3;
}epp_fd_prty_t;

typedef struct ipp_l2_mtu_prty_s
{
	uint16_t l2_mtu_size  : 14;
	uint16_t reserved 	: 2;
}ipp_l2_mtu_prty_t;

typedef struct ipp_vlan_prty_s
{
	// First 32 bits
	uint32_t is_valid:1;
	uint32_t port_list:15;
	uint32_t pcp_to_gem_profile_idx:9;
	uint32_t reserved:7;

}ipp_vlan_prty_t;

typedef struct ipp_vlan_isolation_prty_s{
	uint16_t egress_port_list_mask	:15;
	uint16_t reserved					:1;
}ipp_vlan_isolation_prty_t;

typedef struct ipp_port_isolation_prty_s{
	uint16_t egress_port_list_mask	:15;
	uint16_t reserved					:1;
}ipp_port_isolation_prty_t;

typedef struct ipp_lag_non_uc_fwd_s
{
	uint16_t egress_port_list_mask  : 15;
	uint16_t reserved 	:1;
}ipp_lag_non_uc_fwd_t;

typedef struct ipp_multi_dest_prty_s
{
	uint16_t mc_port_list:13;
	uint16_t lag_hash_select:2;
	uint16_t reserved:1;

}ipp_multi_dest_prty_t;


typedef struct deb_tsid_map_s
{
	uint8_t tsid  : 5;
	uint8_t reserved 	:3;
}deb_tsid_map_t;

typedef struct epp_port_prty_s{
	uint64_t port_grp							:4;
	uint64_t padding_en						:1;
	uint64_t sptag_cfg						:2;
	uint64_t sptag_en							:1;
	uint64_t default_sptag					:16;
	uint64_t mtu_idx							:4;
	uint64_t is_mtu_fail_drop					:1;
	uint64_t is_phb_to_pcp_dei_profile_valid	:1;
	uint64_t phb_to_pcp_dei_profile_idx		:4;
	uint64_t phb_to_dscp_profile_valid		:1;
	uint64_t phb_to_dscp_profile_idx			:4;
	uint64_t egr_mirror_session				:4;
	uint64_t stp_state						:2;
	uint64_t resreved							:19;
}epp_port_prty_t;

typedef struct ipp_vacl_ucp_policy_s
{
	uint64_t act_priority : 4;
	uint64_t tags_to_remove : 2;
	uint64_t otpid_action : 2;
	uint64_t otpid_idx : 3;
	uint64_t itpid_action : 2;
	uint64_t itpid_idx : 3;
	uint64_t ovid_action : 2;
	uint64_t ovid : 12;
	uint64_t ivid_action : 2;
	uint64_t ivid : 12;
	uint64_t opcp_treatment : 4;
	uint64_t ipcp_treatment : 4;
	uint64_t odei_action : 3;
	uint64_t idei_action : 3;
	uint64_t ivp_enable : 1;
	uint64_t ivp_idx_lo : 5;

	uint64_t ivp_idx_hi:7;
	uint64_t bd_enable : 1;
	uint64_t bd_idx : 6;
	uint64_t do_not_learn : 1;
	uint64_t bypass_vlan_filter : 1;
	uint64_t bypass_vlan_isolation : 1;
	uint64_t trust_dscp_enable : 1;
	uint64_t dscp_to_phb_profile_idx : 4;
	uint64_t tc_enable : 1;
	uint64_t tc : 3;
	uint64_t color_enable : 1;
	uint64_t color : 2;
	uint64_t pcp_to_gem_profile_enable : 1;
	uint64_t pcp_to_gem_profile_idx : 9;
	uint64_t flow_table_use_acl_label : 1;
	uint64_t acl_label : 13;
	uint64_t copy_to_cpu : 1;
	uint64_t vacl_crsn_idx : 3;
	uint64_t drop : 2;
	uint64_t is_ctrl : 1;
	uint64_t is_bpdu : 1;
	uint64_t cnt_en : 1;
	uint64_t cnt_idx_lo:2;
		
	uint32_t cnt_idx_hi: 5;
	uint32_t dstr_cnt_en : 1;
	uint32_t dstr_cnt_idx : 6;
	uint32_t mtr_en : 1;
	uint32_t mtr_idx : 9;
	uint32_t vacl_frc_q : 1;
	uint32_t frc_qid : 4;
	uint32_t dfl_disable : 1;
	uint32_t unused:4;
}ipp_vacl_ucp_policy_t;

typedef struct epp_eacl_ucp_policy_s {
	uint64_t act_priority:4;
	uint64_t tags_to_remove:2;
	uint64_t otpid_action:2;
	uint64_t otpid_idx:3;
	uint64_t itpid_action:2;
	uint64_t itpid_idx:3;
	uint64_t ovid_action:2;
	uint64_t ovid:12;
	uint64_t ivid_action:2;
	uint64_t ivid:12;
	uint64_t color_select:1;
	uint64_t g_opcp_treatment:4;
	uint64_t y_opcp_treatment:4;
	uint64_t r_opcp_treatment:4;
	uint64_t g_ipcp_treatment:4;
	uint64_t y_ipcp_treatment_lo:3;
	
	uint64_t y_ipcp_treatment_hi:1;
	uint64_t r_ipcp_treatment:4;
	uint64_t g_odei_action:2;
	uint64_t g_odei:1;
	uint64_t y_odei_action:2;
	uint64_t y_odei:1;
	uint64_t r_odei_action:2;
	uint64_t r_odei:1;
	uint64_t g_idei_action:2;
	uint64_t g_idei:1;
	uint64_t y_idei_action:2;
	uint64_t y_idei:1;
	uint64_t r_idei_action:2;
	uint64_t r_idei:1;
	uint64_t g_dscp_remark:1;
	uint64_t g_dscp:6;
	uint64_t y_dscp_remark:1;
	uint64_t y_dscp:6;
	uint64_t r_dscp_remark:1;
	uint64_t r_dscp:6;
	uint64_t mirror_session:4;
	uint64_t g_copy_to_cpu:1;
	uint64_t y_copy_to_cpu:1;
	uint64_t r_copy_to_cpu:1;
	uint64_t g_drop:2;
	uint64_t y_drop:2;
	uint64_t r_drop:2;
	uint64_t eacl_crsn_idx:3;
	uint64_t cnt_en:1; 
	uint64_t cnt_idx_lo:4;

	uint32_t cnt_idx_hi:3;
	uint32_t dstr_cnt_en:1;
	uint32_t dstr_cnt_idx:4;
	uint32_t mtr_en:1;
	uint32_t mtr_idx:9;
	uint32_t eflext_ext_idx:6;
	uint32_t unused:8;
} epp_eacl_ucp_policy_t;

enum
{
	FLOW_MISS_ACTION_DROP=0,
	FLOW_MISS_ACTION_TO_CPU,
	FLOW_MISS_ACTION_BUILD_NEW,
	FLOW_MISS_ACTION_FOLLOW_L2,
};
typedef struct fltbl_cfg_s
{
	uint64_t mc_flow_en:1;
	uint64_t mc_flow_miss_action:2;
	uint64_t l2_fltbl_en:1;
	uint64_t l2b_etype_filter_en:16;
	uint64_t l2b_pppoe_filter_en:16;
	uint64_t ipv4_proto_chk:16;
	uint64_t ipv6_nxth_chk_lo:12;

	uint16_t ipv6_nxth_chk_hi:4;
	uint16_t flow_miss_action:2;
	uint16_t unused:10;
}fltbl_cfg_t;

typedef struct ipp_l2_fdb_s
{
	uint64_t fdid:13;
	uint64_t mac:48;
	uint64_t is_src_filtering:1;
	uint64_t is_dst_filtering:1;
	uint64_t da_mark_tc:1;
	
	uint32_t sa_mark_tc:1;
	uint32_t tc:3;
	uint32_t sicky:1;
	uint32_t ifidx:14;
	uint32_t sa_hit:2;
	uint32_t is_static:1;
	uint32_t state:2;
	uint32_t resreved:8;
}ipp_l2_fdb_t;

/*vacl fix key*/
typedef struct vacl_fix_key_s
{
	uint64_t tcam_mode:1;
	uint64_t blk1_key_sel:3;
	uint64_t blk2_key_sel:3;
	uint64_t port_list:15 ;
	uint64_t is_lag:1 ;
	uint64_t is_tunnel_decap:1 ;
	uint64_t is_itag_exist:1 ;
	uint64_t is_otag_exist:1 ;
	uint64_t inner_tpid_idx:3 ;
	uint64_t outer_tpid_idx:3 ;
	uint64_t packet_type:2 ;
	uint64_t l2_frame_type:2 ;
	uint64_t l4_frame_type:4 ;
	uint64_t udp_frame_type:4 ;
	uint64_t is_pppoe:1 ;
	uint64_t is_parser_error:1 ;
	uint64_t is_drop:1 ;
	uint64_t reserverd:15;
	uint64_t unused:2;
}vacl_fix_key_t;

/*key_block1_group0*/
typedef struct vacl_key_blk1_grp0_s{                
	uint8_t  dst_mac[6];
	uint8_t  src_mac[6];
	uint16_t ether_type;                              
	uint16_t ppp_session_id;                                                             
}vacl_key_blk1_grp0_t;

/*key_block1_group1*/
typedef struct vacl_key_blk1_grp1_s{
	uint8_t  dst_mac[6];
	uint8_t  src_mac[6];
	
	uint32_t  tos:8;
	uint32_t frag_status:2;
	uint32_t ttl_status:2;
	uint32_t  protocol:8;
	uint32_t option_status:2;
	uint32_t reserved_1:10;
}vacl_key_blk1_grp1_t;

/*key_block1_group2*/
typedef struct vacl_key_blk1_grp2_s{
	uint32_t src_ip;
	uint32_t dst_ip;
	uint8_t  protocol;
	uint8_t tos;
	uint8_t  dst_mac[6];
}vacl_key_blk1_grp2_t;

/*key_block1_group3*/
typedef struct vacl_key_blk1_grp3_s{
	uint32_t src_ip;
	uint32_t dst_ip;
	uint8_t  protocol;
	uint8_t tos;
	uint8_t  src_mac[6];
}vacl_key_blk1_grp3_t;

/*key_block1_group4*/
typedef struct vacl_key_blk1_grp4_s{
	uint32_t src_ip_v6[4];
}vacl_key_blk1_grp4_t;

/*key_block1_group5*/
typedef struct vacl_key_blk1_grp5_s{
	uint32_t dst_ip_v6[4];
}vacl_key_blk1_grp5_t;

/*key_block1_group6*/
typedef struct vacl_key_blk1_grp6_s{
	uint32_t src_ip_v6_upper[2];
	uint32_t dst_ip_v6_upper[2];
}vacl_key_blk1_grp6_t;

/*key_block1_group7*/
typedef struct vacl_key_blk1_grp7_s{
	uint64_t protocol:8;
	uint64_t ether_type:16;
	uint64_t l4_sport:16;
	uint64_t l4_dport:16;
	uint64_t gem_llid_lo:8;
	
	uint64_t gem_llid_hi:8;
	uint64_t ovid:12;
	uint64_t opcp:3;
	uint64_t odei:1;
	uint64_t ivid:12;
	uint64_t ipcp:3;
	uint64_t idei:1;
	uint64_t tag_count:3;
	uint64_t is_l3_tunnel_enable:1;
	uint64_t vp_l3if_idx:12;
	uint64_t reserved:8;
}vacl_key_blk1_grp7_t;

typedef union
{
	struct vacl_key_blk1_grp0_s grp0;
	struct vacl_key_blk1_grp1_s grp1;
	struct vacl_key_blk1_grp2_s grp2;
	struct vacl_key_blk1_grp3_s grp3;
	struct vacl_key_blk1_grp4_s grp4;
	struct vacl_key_blk1_grp5_s grp5;
	struct vacl_key_blk1_grp6_s grp6;
	struct vacl_key_blk1_grp7_s grp7;
}vacl_key_blk1_t;

/*key_block2_group0*/
typedef struct vacl_key_blk2_grp0_s{
	uint16_t ovid:12;
	uint16_t opcp:3;
	uint16_t odei:1;
	
	uint16_t ivid:12;
	uint16_t ipcp:3;
	uint16_t idei:1;

	uint16_t ether_type;

	uint16_t tag_count:3;
	uint16_t reserved:13;
}vacl_key_blk2_grp0_t;

/*key_block2_group1*/
typedef struct vacl_key_blk2_grp1_s{
	uint32_t src_ip;
	uint32_t dst_ip;
} vacl_key_blk2_grp1_t;

/*key_block2_group2*/
typedef struct vacl_key_blk2_grp2_s{
	uint64_t protocol:8;		
	uint64_t tos:8;
	uint64_t frag_status:2;
	uint64_t ttl_status:2;
	uint64_t option_status:2;
	uint64_t l4_sport:16;
	uint64_t l4_dport:16;
	uint64_t tcp_flags:9;
	uint64_t reserved:1; 
} vacl_key_blk2_grp2_t;

/*key_block2_group3*/
typedef struct vacl_key_blk2_grp3_s{
	uint8_t dst_mac[6];
	uint8_t protocol;
	uint8_t reserved_2;
} vacl_key_blk2_grp3_t;

/*key_block2_group4*/
typedef struct vacl_key_blk2_grp4_s{
	uint8_t src_mac[6];
	uint8_t protocol;
	uint8_t reserved_2;
} vacl_key_blk2_grp4_t;

/*key_block2_group5*/
typedef struct vacl_key_blk2_grp5_s{
	uint64_t is_l3_tunnel_enable:1;
	uint64_t vp_l3if_idx:12;
	uint64_t tunnel_type:4;
	uint64_t tunnel_opcp:3;
	uint64_t tunnel_odei:1;
	uint64_t tunnel_ipcp:3;
	uint64_t tunnel_idei:1;
	uint64_t tunnel_dscp:6;
	uint64_t tunnel_sid:32;
	uint64_t reserved_2:1;
} vacl_key_blk2_grp5_t;

/*key_block2_group6*/
typedef struct vacl_key_blk2_grp6_s{
	uint64_t lou_result:10;
	uint64_t mac_lou_result:2;
	uint64_t ip_lou_result:2;
	uint64_t ipv6_lou_result:2;
	uint64_t l4_sport:16;
	uint64_t l4_dport:16;
	uint64_t reserved_2:16;
} vacl_key_blk2_grp6_t;

typedef  union
{
	struct vacl_key_blk2_grp0_s grp0;
	struct vacl_key_blk2_grp1_s grp1;
	struct vacl_key_blk2_grp2_s grp2;
	struct vacl_key_blk2_grp3_s grp3;
	struct vacl_key_blk2_grp4_s grp4;
	struct vacl_key_blk2_grp5_s grp5;
	struct vacl_key_blk2_grp6_s grp6;
}vacl_key_blk2_t;

/*key_double*/
typedef struct vacl_key_double_s{
	 uint64_t tcam_mode:1;
	 uint64_t dw_blk1_key_sel:3;
	 uint64_t dw_blk2_key_sel:3;
	 uint64_t flow_label:20;
	 uint64_t tos:8;
	 uint64_t frag_status:2;
	 uint64_t ttl_status:2;
	 uint64_t ext_next_header:8;
	 uint64_t reserved_3:3;
	 uint64_t unused:2;
}vacl_key_double_t;

typedef struct vacl_key_s
{
	vacl_fix_key_t fix_key;
	vacl_key_blk1_t key_blk1;
	vacl_key_blk2_t key_blk2;	
}vacl_key_t;

typedef struct vacl_key_mask_s
{
	vacl_fix_key_t fix_key_mask;
	vacl_key_blk1_t key_blk1_mask;
	vacl_key_blk2_t key_blk2_mask;	
}vacl_key_mask_t;

typedef enum {
	VACL_KEY_BLK1_GRP0=0,
	VACL_KEY_BLK1_GRP1,
	VACL_KEY_BLK1_GRP2,
	VACL_KEY_BLK1_GRP3,
	VACL_KEY_BLK1_GRP4,
	VACL_KEY_BLK1_GRP5,
	VACL_KEY_BLK1_GRP6,
	VACL_KEY_BLK1_GRP7,
}VACL_KEY_BLK1_SEL;

typedef enum {
	VACL_KEY_BLK2_GRP0=0,
	VACL_KEY_BLK2_GRP1,
	VACL_KEY_BLK2_GRP2,
	VACL_KEY_BLK2_GRP3,
	VACL_KEY_BLK2_GRP4,
	VACL_KEY_BLK2_GRP5,
	VACL_KEY_BLK2_GRP6,
}VACL_KEY_BLK2_SEL;

typedef enum {
	VACL_UCP0=0,
	VACL_UCP1,
	VACL_UCP2,
	VACL_UCP3,
	VACL_UCP4,
	VACL_UCP5,
	VACL_UCP6,
	VACL_UCP7,
}VACL_UCP_SEL;

/*fix key*/
typedef struct eacl_fix_key_s{
	uint64_t tcam_mode:1;	
	uint64_t blk1_key_sel:3;	
	uint64_t blk2_key_sel:3;	
	uint64_t blk3_key_sel:2;
	uint64_t packet_type:2 ;
	uint64_t l2_frame_type:2 ;
	uint64_t l4_frame_type:4 ;
	uint64_t udp_frame_type:4 ;
	uint64_t is_pppoe:1;
	uint64_t port_list:15;
	uint64_t reserved:27;
}eacl_fix_key_t;

/*key_block1_group1*/
typedef struct eacl_key_blk1_grp0_s{ 
	uint8_t  dst_mac[6];
	uint8_t  src_mac[6];
	uint16_t ether_type;                              
	uint16_t reserved;                                                         
}eacl_key_blk1_grp0_t;

typedef struct eacl_key_blk1_grp1_s{ 
	uint8_t dst_mac[6];
	uint8_t src_mac[6];
	uint32_t tos:8;
	uint32_t frag_status:2;
	uint32_t ttl_status:2;
	uint32_t protocol:8;
	uint32_t option_status:2;
	uint32_t fdid_label:6;
	uint32_t reserved:4;
} eacl_key_blk1_grp1_t;

typedef struct eacl_key_blk1_grp2_s{
	uint32_t src_ip;
	uint32_t dst_ip;
	uint8_t protocol;
	uint8_t tos;
	uint8_t dst_mac[6];
} eacl_key_blk1_grp2_t;

typedef struct eacl_key_blk1_grp3_s{
	uint32_t src_ip;
	uint32_t dst_ip;
	uint8_t protocol;
	uint8_t tos;
	uint8_t src_mac[6];
} eacl_key_blk1_grp3_t;

typedef struct eacl_key_blk1_grp4_s{
	uint32_t src_ip_v6[4];
} eacl_key_blk1_grp4_t;

typedef struct eacl_key_blk1_grp5_s{
	uint32_t dst_ip_v6[4];
} eacl_key_blk1_grp5_t;

typedef struct eacl_key_blk1_grp6_s{
	uint32_t src_ip_v6_upper[2];
	uint32_t dst_ip_v6_upper[2];
} eacl_key_blk1_grp6_t;

typedef struct eacl_key_blk1_grp7_s{
	uint64_t protocol:8;
	uint64_t ether_type:16;
	uint64_t l4_sport:16;
	uint64_t l4_dport:16;
	uint64_t gem_llid_lo:8;

	uint64_t gem_llid_hi:8;
	uint64_t outer_tpid_idx:3;
	uint64_t inner_tpid_idx:3;
	uint64_t ovid:12;
	uint64_t opcp:3;
	uint64_t odei:1;
	uint64_t ivid:12;
	uint64_t ipcp:3;
	uint64_t idei:1;
	uint64_t tag_counti:3;
	uint64_t eif:14;
	uint64_t reserved:1;
} eacl_key_blk1_grp7_t;

typedef union
{
	struct eacl_key_blk1_grp0_s grp0;
	struct eacl_key_blk1_grp1_s grp1;
	struct eacl_key_blk1_grp2_s grp2;
	struct eacl_key_blk1_grp3_s grp3;
	struct eacl_key_blk1_grp4_s grp4;
	struct eacl_key_blk1_grp5_s grp5;
	struct eacl_key_blk1_grp6_s grp6;
	struct eacl_key_blk1_grp7_s grp7;
}eacl_key_blk1_t;

typedef struct eacl_key_blk2_grp0_s{
	uint32_t src_ip;
	uint32_t dst_ip;
} eacl_key_blk2_grp0_t;

typedef struct eacl_key_blk2_grp1_s{
	uint8_t dst_mac[6];
	uint8_t protocol;
	uint8_t reserved; 
} eacl_key_blk2_grp1_t;

typedef struct eacl_key_blk2_grp2_s{
	uint8_t src_mac[6];
	uint8_t protocol;
	uint8_t reserved;
} eacl_key_blk2_grp2_t;

typedef struct eacl_key_blk2_grp3_s{
	uint64_t protocol:8;
	uint64_t tos:8;
	uint64_t frag_status:2;
	uint64_t ttl_status:2;
	uint64_t option_status:2;
	uint64_t l4_sport:16;
	uint64_t l4_dport:16;
	uint64_t tcp_flags:9;
	uint64_t reserved:1;
} eacl_key_blk2_grp3_t;

typedef struct eacl_key_blk2_grp4_s{
	uint64_t tag_count:3;
	uint64_t outer_tpid_idx:3;
	uint64_t inner_tpid_idx:3;
	uint64_t ovid:12;
	uint64_t opcp:3;
	uint64_t odei:1;
	uint64_t ivid:12;
	uint64_t ipcp:3;
	uint64_t idei:1;
	uint64_t eif:14;
	uint64_t fdid_label:8;
	uint64_t reserved:1;
} eacl_key_blk2_grp4_t;

typedef struct eacl_key_blk2_grp5_s{
	uint64_t iport_id:4;
	uint64_t fdid:13;
	uint64_t acl_label:13;
	uint64_t eif:14;
	uint64_t reserved:20;
} eacl_key_blk2_grp5_t;

typedef struct eacl_key_blk2_grp6_s{
	uint64_t fdid:13;
	uint64_t tag_count:3;
	uint64_t outer_tpid_idx:3;
	uint64_t inner_tpid_idx:3;
	uint64_t ovid:12;
	uint64_t opcp:3;
	uint64_t odei:1;
	uint64_t ivid:12;
	uint64_t ipcp:3;
	uint64_t idei:1;
	uint64_t reserved:10;
} eacl_key_blk2_grp6_t;

typedef union
{
	struct eacl_key_blk2_grp0_s grp0;
	struct eacl_key_blk2_grp1_s grp1;
	struct eacl_key_blk2_grp2_s grp2;
	struct eacl_key_blk2_grp3_s grp3;
	struct eacl_key_blk2_grp4_s grp4;
	struct eacl_key_blk2_grp5_s grp5;
	struct eacl_key_blk2_grp6_s grp6;
}eacl_key_blk2_t;

typedef struct eacl_key_blk3_grp0_s{
	uint64_t iport_id:4;
	uint64_t fdid:13;
	uint64_t color:2;
	uint64_t is_mirror:1;
	uint64_t is_drop:1;
	uint64_t reserved:21;
	uint64_t unused:22;
} eacl_key_blk3_grp0_t;

typedef struct eacl_key_blk3_grp1_s{
	uint64_t tag_count:3;
	uint64_t outer_tpid_idx:3;
	uint64_t inner_tpid_idx:3;
	uint64_t ovid:12;
	uint64_t opcp:3;
	uint64_t odei:1;
	uint64_t ivid:12;
	uint64_t ipcp:3;
	uint64_t idel:1;
	uint64_t is_drop:1;
	uint64_t unused:22;
} eacl_key_blk3_grp1_t;

typedef struct eacl_key_blk3_grp2_s{
	uint64_t iport_id:4;
	uint64_t eif:14;
	uint64_t acl_lebal:13;
	uint64_t color:2;
	uint64_t is_mirror:1;
	uint64_t is_drop:1;
	uint64_t reserved:7;
	uint64_t unused:22;
} eacl_key_blk3_grp2_t;

typedef union
{
	struct eacl_key_blk3_grp0_s grp0;
	struct eacl_key_blk3_grp1_s grp1;
	struct eacl_key_blk3_grp2_s grp2;
}eacl_key_blk3_t;

/*key_double*/
typedef struct eacl_key_double_s{
	uint64_t tcam_mode:1;
	uint64_t dw_blk1_key_sel:3;
	uint64_t dw_blk2_key_sel:3;
	uint64_t dw_blk3_key_sel:2;
	uint64_t flow_label:20;
	uint64_t tos:8;
	uint64_t frag_status:2;
	uint64_t ttl_status:2;
	uint64_t ext_next_header:8;
	uint64_t reserved:15;
}eacl_key_double_t;

typedef struct eacl_key_s
{
	eacl_fix_key_t fix_key;
	eacl_key_blk1_t key_blk1;
	eacl_key_blk2_t key_blk2;		
	eacl_key_blk3_t key_blk3;	
}eacl_key_t;

typedef struct eacl_key_mask_s
{
	eacl_fix_key_t fix_key_mask;
	eacl_key_blk1_t key_blk1_mask;
	eacl_key_blk2_t key_blk2_mask;
	eacl_key_blk3_t key_blk3_mask;
}eacl_key_mask_t; 

typedef enum {
	EACL_KEY_BLK1_GRP0=0,
	EACL_KEY_BLK1_GRP1,
	EACL_KEY_BLK1_GRP2,
	EACL_KEY_BLK1_GRP3,
	EACL_KEY_BLK1_GRP4,
	EACL_KEY_BLK1_GRP5,
	EACL_KEY_BLK1_GRP6,
	EACL_KEY_BLK1_GRP7,
}EACL_KEY_BLK1_SEL;

typedef enum {
	EACL_KEY_BLK2_GRP0=0,
	EACL_KEY_BLK2_GRP1,
	EACL_KEY_BLK2_GRP2,
	EACL_KEY_BLK2_GRP3,
	EACL_KEY_BLK2_GRP4,
	EACL_KEY_BLK2_GRP5,
	EACL_KEY_BLK2_GRP6,
}EACL_KEY_BLK2_SEL;

typedef enum {
	EACL_KEY_BLK3_GRP0=0,
	EACL_KEY_BLK3_GRP1,
	EACL_KEY_BLK3_GRP2,
}EACL_KEY_BLK3_SEL;

typedef enum {
	EACL_UCP0=0,
	EACL_UCP1,
	EACL_UCP2,
	EACL_UCP3,
}EACL_UCP_SEL;

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
#endif

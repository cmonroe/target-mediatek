/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 AIROHA Inc
 */
#ifndef AIROHA_FUNCTION_H
#define AIROHA_FUNCTION_H


#include "arht_dp_api.h"
#include "en7581/soc_en7581.h"
#include "an7583/soc_an7583.h"
#include "arht_fe.h"
#include "arht_qdma.h"
#include "arht_ppe.h"
u32 arht_set_bucket_size(struct airoha_qdma *qdma, int channel, enum trtcm_mode_type mode, u32 bucket_size);

int qdma_wan_tx(struct sk_buff *skb, u32 msg0, u32 msg1, struct port_info *pinfo);
int SendToPpe(struct sk_buff * skb);
int speedtest_tx_offload(struct sk_buff *skb, struct airoha_foe_entry *foe_entry, struct airoha_ppe *ppe, struct port_info *pinfo);
extern void airoha_dp_api_qdma_meter_default_config(struct airoha_qdma *qdma);
extern int airoha_dp_api_debugfs_init(struct airoha_eth *eth);
int airoha_receive_handler(struct airoha_queue *q, struct airoha_eth *eth, int len, int p);
int arht_tx_modified(struct airoha_flow_table_entry *e, u32 hash);
int pon_fill_forward_path(struct net_device_path_ctx *ctx,
				      struct net_device_path *path);
void arht_npu_retry_send_msg(struct airoha_npu *npu, int ret,
					u32 val, dma_addr_t dma_addr, int size, int func_id);

void airoha_ppe_set_init_cfg(struct airoha_eth *eth, int ppe_no);
bool airoha_ppe_check_tx_modified(struct airoha_flow_table_entry *e, u32 hash);
void  qdma_get_tr471_rxmsg(int rx_ring,unsigned int * rx_byte_cnt_l,unsigned int * rx_byte_cnt_h,unsigned int * err_cnt,unsigned int * drop_cnt);
int airoha_eth_fast_tx(struct sk_buff *skb, int channel);
void arht_modified_ppe_entry(struct airoha_foe_entry *hwe, struct net_device *netdev, 
						struct airoha_flow_data *data, u32 priority, int dsa_port);

void arht_fe_extra_init(struct airoha_eth *eth);
void arht_qdma_hw_extra_init(struct airoha_qdma *qdma);
void arht_set_ppe_dft_cport(struct airoha_qdma *qdma, struct airoha_gdm_port *port);
int airoha_phylink_pcs_setup(struct net_device *dev, struct airoha_gdm_port *port);
int arht_extra_probe(struct airoha_eth *eth);
extern void airoha_ppe_debugfs_extra_create(struct dentry *root);
extern int (*ra_sw_nat_hook_sendto_ppe)(struct sk_buff *skb);
extern int (*offload_eth_fast_tx_hook)(struct sk_buff *skb, int channel);
extern void (*get_tr471_rx_msg_hook)(int rx_ring, unsigned int * rx_byte_cnt_l, unsigned int * rx_byte_cnt_h, unsigned int * err_cnt,unsigned int * drop_cnt);
extern int airoha_hsgmii_init(void);
extern void airoha_hsgmii_exit(void);
int airoha_get_lan_id(struct net_device *dev);
void airoha_flow_table_entries_lan(struct rhashtable *flow_table, struct net_device *dev);
int airoha_receive_hook(struct airoha_queue *q, struct airoha_qdma_desc *desc, 
				int p, int len);
void arht_conntrack_get_cnt(u32 hash, struct airoha_foe_stats64 *stats);
void arht_conntrack_free_cnt(u32 hash);

#endif /* AIROHA_FUNCTION_H */

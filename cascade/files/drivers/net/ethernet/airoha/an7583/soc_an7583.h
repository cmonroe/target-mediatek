#ifndef __SOC_AN7583_H__
#define __SOC_AN7583_H__


unsigned int an7583_get_fe_fport(struct sk_buff *skb, struct net_device *dev, u32 portid);
void an7583_set_ppe_dft_cport(struct airoha_eth *eth, int port_id, int qdma_id);
int an7583_support_eth_monitor(int p);
inline int xsi_check_index_valid(uint hsgmii_index);
int xsi_mac_set_ipg(struct airoha_gdm_dev *dev, uint hsgmii_index, uint ipg);
int xsi_mac_set_tx_rx_frag_len(struct airoha_gdm_dev *dev, uint hsgmii_index, uint frag_len);
int xsi_mac_set_tx_rx_fc_en(struct airoha_gdm_dev *dev, uint hsgmii_index);
int an7583_channel_retire(struct airoha_eth *eth, struct airoha_gdm_dev *dev);
void an7583_qdma_regs_setting(struct airoha_qdma *qdma, struct airoha_eth *eth);
void an7583_fe_regs_setting(struct airoha_eth *eth);
unsigned int an7583_get_qdma_channel(struct airoha_gdm_dev *dev, u32 sptag);
unsigned int an7583_get_qdma_buf_size(int id);

#endif /* __SOC_AN7583_H__ */
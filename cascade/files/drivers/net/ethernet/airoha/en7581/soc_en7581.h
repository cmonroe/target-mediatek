#ifndef _SOC_EN7581_H_
#define _SOC_EN7581_H_

unsigned int en7581_get_fe_fport(struct sk_buff *skb, struct net_device *dev, u32 portid);
int en7581_support_eth_monitor(int p);
void an7581_qdma_regs_setting(struct airoha_qdma *qdma, struct airoha_eth *eth);
void an7581_fe_regs_setting(struct airoha_eth *eth);
unsigned int en7581_get_qdma_channel(struct airoha_gdm_dev *dev, u32 sptag);
unsigned int en7581_get_qdma_buf_size(int id);

#endif /* _SOC_EN7581_H_ */
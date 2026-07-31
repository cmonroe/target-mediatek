#include "../airoha_regs.h"
#include "../airoha_eth.h"
#include "../arht_dp_api.h"
#include "soc_en7581.h"

unsigned int en7581_get_fe_fport(struct sk_buff *skb, struct net_device *dev, u32 portid)
{
	u8 fport;
	fport = (portid == AIROHA_GDM4_IDX) ? FE_PSE_PORT_GDM4 : portid;
	return fport;
}



int en7581_support_eth_monitor(int p)
{
	/*gdm2 and gdm3 not detect, skip*/
	if(p == AIROHA_PORTS_GDM2_ID || p == AIROHA_PORTS_GDM3_ID)
		return 0;
	return 1;
}

void an7581_qdma_regs_setting(struct airoha_qdma *qdma, struct airoha_eth *eth)
{
	airoha_qdma_wr(qdma, REG_TXQ_TOTALTHR, 0x39992e00);
	airoha_qdma_wr(qdma, REG_TXQ_CHNLTHR_CFG, 0x39990400);
	airoha_qdma_wr(qdma, REG_TXQ_QUEUETHR_CFG, 0x20000020);
	airoha_qdma_set(qdma, QDMA_CSR_QOS_AGING_CFG, QDMA_QOS_AGING_EN);
	airoha_qdma_set(qdma, QDMA_CSR_QOS_AGING_CFG, QDMA_QOS_AGING_FAST_REPLACE);

	return;
}

void an7581_fe_regs_setting(struct airoha_eth *eth)
{
	return;
}

unsigned int en7581_get_qdma_channel(struct airoha_gdm_dev *dev, u32 sptag)
{
	const struct airoha_eth_soc_data *soc;
	int serdes_idx;
	u32 portid;
	int nbq;

	if (!dev || !dev->port || !dev->eth)
		return 0;

	soc = dev->eth->soc;
	portid = dev->port->id;
	nbq = dev->nbq;

	switch (portid) {
	case AIROHA_GDM3_IDX:
		serdes_idx = (nbq == 5) ? SERDES_PCIE1_IDX : SERDES_PCIE0_IDX;
		break;
	case AIROHA_GDM4_IDX:
		serdes_idx = (nbq == 1) ? SERDES_USB_IDX : SERDES_ETH_IDX;
		break;
	default:
		/* GDM1: DSA switch ports, use sptag-based mapping */
		return LAN_IDX_FROM_TX_SPTAG((u16)(sptag & 0xFF)) %  AIROHA_MAX_NUM_CHANNELS;
	}

	return (u32)soc->chnl[serdes_idx] % AIROHA_MAX_NUM_CHANNELS;
}

unsigned int en7581_get_qdma_buf_size(int id)
{
	u32 buf_size;

	buf_size = id ? AIROHA_MAX_PACKET_SIZE / 2 : AIROHA_MAX_PACKET_SIZE;

	return buf_size;
}

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/regmap.h>

#include "../airoha_regs.h"
#include "../airoha_eth.h"
#include "../arht_dp_api.h"
#include "soc_an7583.h"

/************************************************************************
*                  D  E F I N E S   &&   C O N S T A N T S
*************************************************************************
*/
#define GET_XSI_GDM(eth,serdes_id)            ((eth)->soc->gdm[(serdes_id)])
#define GET_XSI_CHANNEL(eth,serdes_id)        ((eth)->soc->chnl[(serdes_id)])
#define GET_XSI_RETIRE_CHANNEL(eth,serdes_id) ((eth)->soc->retire_channel[(serdes_id)])

#define XSI_GLB_CFG (0x0)
#define XSI_GLB_CFG_F_IPG_NUM_SHIFT (10)
#define XSI_GLB_CFG_F_IPG_NUM_MASK (0xFC00)
#define XSI_GLB_CFG_F_TX_FRAG_LEN_SHIFT (17)
#define XSI_GLB_CFG_F_TX_FRAG_LEN_MASK (0x3E0000)
#define XSI_GLB_CFG_F_RX_FRAG_LEN_SHIFT (22)
#define XSI_GLB_CFG_F_RX_FRAG_LEN_MASK (0x7C00000)
#define XSI_GLB_CFG_F_RX_FC_EN (1<<4)
#define XSI_GLB_CFG_F_TX_FC_EN (1<<5)

/************************************************************************
*                  S T A T I C   F U N C T I O N S
*************************************************************************
*/


void an7583_set_ppe_dft_cport(struct airoha_eth *eth, int port_id, int qdma_id)
{
	u8 dft_port;
	u32 shift = port_id * 4;
	
	if(qdma_id == 0)
		dft_port = FE_PSE_PORT_CDM1;
	else
		dft_port = FE_PSE_PORT_CDM2;
	airoha_fe_rmw(eth,REG_PPE1_DFT_CPORT_0, 0xF << shift, dft_port << shift);
}

int an7583_support_eth_monitor(int p)
{
	/*gdm2 not detect, skip*/
	if (p == AIROHA_PORTS_GDM2_ID)
		return 0;
	return 1;
}

inline int xsi_check_index_valid(uint hsgmii_index)
{
	if(hsgmii_index >= SERDES_MAX_IDX)
		return 0;
	else 
		return 1 ;

}

int xsi_mac_set_ipg(struct airoha_gdm_dev *dev, uint hsgmii_index, uint ipg)
{
	if(0 == xsi_check_index_valid(hsgmii_index))
		return -1;

	if(dev->xfi_mac){
		regmap_update_bits(dev->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_IPG_NUM_MASK,ipg << XSI_GLB_CFG_F_IPG_NUM_SHIFT);
	}
	return 0;
}

int xsi_mac_set_tx_rx_frag_len(struct airoha_gdm_dev *dev, uint hsgmii_index, uint frag_len)
{
	if(0 == xsi_check_index_valid(hsgmii_index))
		return -1;
	
	if(dev->xfi_mac){
		regmap_update_bits(dev->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_TX_FRAG_LEN_MASK,frag_len << XSI_GLB_CFG_F_TX_FRAG_LEN_SHIFT);
		regmap_update_bits(dev->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_RX_FRAG_LEN_MASK,frag_len << XSI_GLB_CFG_F_RX_FRAG_LEN_SHIFT);
	}
	return 0;
}

int xsi_mac_set_tx_rx_fc_en(struct airoha_gdm_dev *dev, uint hsgmii_index)
{
	if(0 == xsi_check_index_valid(hsgmii_index))
		return -1;
	
	if(dev->xfi_mac){
		regmap_set_bits(dev->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_TX_FC_EN);
		regmap_set_bits(dev->xfi_mac, XSI_GLB_CFG,XSI_GLB_CFG_F_RX_FC_EN);
	}
	return 0;
}

int an7583_channel_retire(struct airoha_eth *eth, struct airoha_gdm_dev *dev)
{

	int ret = 0;
	struct ecnt_fe_data fe_data = {0};
	FE_Gdma_Sel_t gdm_idx;
	u8 retire_channle, hsgmii_index;

	/*Only support gdm3 channel retire for 7583 */
	if(dev->port->id != AIROHA_GDM3_IDX){
		return -1;
	}
	hsgmii_index = SERDES_ETH_IDX;
	
	gdm_idx = GET_XSI_GDM(eth,hsgmii_index);
	retire_channle = GET_XSI_RETIRE_CHANNEL(eth,hsgmii_index);

	fe_data.gdm_sel = gdm_idx;
	fe_data.channel = retire_channle;
	fe_api_set_channel_retire_one(&fe_data);
	mdelay(1);

	xsi_mac_set_ipg(dev,hsgmii_index, 10);

	//usxgmii mode disable tx_rx frag & use fe frag
	xsi_mac_set_tx_rx_frag_len(dev,hsgmii_index,4);
	
	xsi_mac_set_tx_rx_fc_en(dev,hsgmii_index);
	return ret;
}

void an7583_qdma_regs_setting(struct airoha_qdma *qdma, struct airoha_eth *eth)
{
	int mask = 0;
	int val = 0;
	
	int id = qdma - &eth->qdma[0];
	airoha_qdma_wr(qdma, REG_TXQ_TOTALTHR, 0x39992e00);
	airoha_qdma_wr(qdma, REG_TXQ_CHNLTHR_CFG, 0x39990400);
	airoha_qdma_wr(qdma, REG_TXQ_QUEUETHR_CFG, 0x20000020);
	airoha_qdma_set(qdma, QDMA_CSR_QOS_AGING_CFG, QDMA_QOS_AGING_EN);
	airoha_qdma_set(qdma, QDMA_CSR_QOS_AGING_CFG, QDMA_QOS_AGING_FAST_REPLACE);

	//set qdma multi issue
	mask = GLOBAL_CFG_RD_BYPASS_WR_MASK | GLOBAL_CFG_MAX_ISSUE_NUM_MASK;
	val = GLOBAL_CFG_RD_BYPASS_WR_MASK | FIELD_PREP(GLOBAL_CFG_MAX_ISSUE_NUM_MASK, 3);
	airoha_qdma_rmw(qdma, REG_QDMA_GLOBAL_CFG,mask,val);

	if(id == 0)
	{
		//set qdma egress ratemeter cfg
		mask = (EGRESS_RATE_METER_EQ_RATE_EN_MASK | EGRESS_RATE_METER_WINDOW_SZ_MASK | EGRESS_RATE_METER_TIMESLICE_MASK);
		val = (FIELD_PREP(EGRESS_RATE_METER_WINDOW_SZ_MASK, 1) | FIELD_PREP(EGRESS_RATE_METER_TIMESLICE_MASK, 0x740));
		airoha_qdma_rmw(qdma, REG_EGRESS_RATE_METER_CFG,mask,val);
	}

	mask = (TXQ_CNGST_TXQ_TOTAL_MAX_THR_MASK | TXQ_CNGST_TXQ_TOTAL_MIN_THR_MASK);
	val = (FIELD_PREP(TXQ_CNGST_TXQ_TOTAL_MAX_THR_MASK, BUFF_FAST_TOTAL_MAX_THRH) 
		| FIELD_PREP(TXQ_CNGST_TXQ_TOTAL_MIN_THR_MASK, BUFF_FAST_TOTAL_MIN_THRH));
	airoha_qdma_rmw(qdma, REG_QDMA_TXQ_TOTAL_FAST_THR,mask,val);
	
	return;
}


void an7583_fe_regs_setting(struct airoha_eth *eth)
{
	//set cdm2 faq 
	airoha_fe_wr(eth, REG_FAQ_CFG(1),0x07e6);
	airoha_fe_wr(eth, REG_FAQTHR_CFG(1),0xc40003f0);
	airoha_fe_set(eth, REG_FAQ_CFG(1), FAQ_EN_MASK);	
	//set qbi fttr chn disable
	airoha_fe_wr(eth, REG_QBI_FTTR_CHANNEL_CFG, 0);
	return;
}

unsigned int an7583_get_qdma_channel(struct airoha_gdm_dev *dev, u32 sptag)
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
		serdes_idx = SERDES_ETH_IDX;
		break;
	case AIROHA_GDM4_IDX:
		serdes_idx = (nbq == 0) ? SERDES_PCIE0_IDX : SERDES_USB_IDX;
		break;
	default:
		/* GDM1: DSA switch ports, use sptag-based mapping */
		return LAN_IDX_FROM_TX_SPTAG((u16)(sptag & 0xFF)) %  AIROHA_MAX_NUM_CHANNELS;
	}

	return (u32)soc->chnl[serdes_idx] % AIROHA_MAX_NUM_CHANNELS;
}

unsigned int an7583_get_qdma_buf_size(int id)
{
	u32 buf_size;

	buf_size = AIROHA_MAX_PACKET_SIZE / 2;

	return buf_size;
}


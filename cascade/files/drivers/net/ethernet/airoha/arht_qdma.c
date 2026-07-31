// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 AIROHA Inc
 * Author:  2024 AIROHA Inc
 */
#include "airoha_eth.h"
#include "airoha_regs.h"
#include "airoha_function.h"



/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/

/* QDMA_CSR_TXWRR_WEIGHT_CFG	 */
#define QDMA_CSR_TXWRR_WEIGHT_CFG					(0x1024)
#define TXWRR_RWCMD									(1<<31)
#define TXWRR_RWCMD_DONE							(1<<30)
#define TXWRR_CHNL_IDX_SHIFT						(19)
#define TXWRR_CHNL_IDX_MASK							(0x1F<<TXWRR_CHNL_IDX_SHIFT)
#define TXWRR_QUEUE_IDX_SHIFT						(16)
#define TXWRR_QUEUE_IDX_MASK						(0x7<<TXWRR_QUEUE_IDX_SHIFT)
#define TXWRR_WRR_VALUE_SHIFT						(0)
#define TXWRR_WRR_VALUE_MASK						(0xFF<<TXWRR_WRR_VALUE_SHIFT)

/* QDMA_CSR_PERCHNL_QOS_MODE */
#define TXQOS_CHNL_QOS_MODE_SHIFT(idx)				((idx&0x7)<<2)
#define TXQOS_CHNL_QOS_MODE_MASK(idx)				(0x7<<TXQOS_CHNL_QOS_MODE_SHIFT(idx))
#define QDMA_CSR_PERCHNL_QOS_MODE(base, i)			(base+0x1040+(i<<2))

#define qdmaGetPerChnlQosMode(qdma, chnl) \
    ((airoha_qdma_rr(qdma, QDMA_CSR_PERCHNL_QOS_MODE(0, (chnl >> 3))) & TXQOS_CHNL_QOS_MODE_MASK(chnl)) >> TXQOS_CHNL_QOS_MODE_SHIFT(chnl))


#define qdmaSetPerChnlQosMode(qdma, chnl, val) \
    airoha_qdma_rmw(qdma, QDMA_CSR_PERCHNL_QOS_MODE(0, (chnl >> 3)), TXQOS_CHNL_QOS_MODE_MASK(chnl), (val << TXQOS_CHNL_QOS_MODE_SHIFT(chnl)))
#define GET_METER_IDX(index)    (index&0xff)
#define GET_METER_GRP(index)    ((index>>8)&0x3)

#define TRTCM_MODE_SHIFT					(30)

extern uint trtcmBucketByteUnit[TRTCM_MODE_MAX] ;
extern uint trtcmBucketPacketUnit[TRTCM_MODE_MAX] ;

extern struct airoha_eth *glb_eth;

/* For EN7580,EN7528: record TXQ_DIS_CFG_CHN register value , the register only can write, can not read */
uint TXQ_DIS_CFG_VALUE[TXQ_DIS_CFG_REG_NUM] = {0, 0, 0, 0, 0, 0, 0, 0};
//#endif
DEFINE_SPINLOCK(qdma_cfg_lock);
uint trtcmBucketByteUnit[TRTCM_MODE_MAX] ;
EXPORT_SYMBOL(trtcmBucketByteUnit);
uint trtcmBucketPacketUnit[TRTCM_MODE_MAX] ;
EXPORT_SYMBOL(trtcmBucketPacketUnit);
uint trtcmCfgBase[TRTCM_MODE_MAX] ;


/*return the max bucketSize shift close to value*/
int biSearchGetBucketSizeShift(uint value, uint lo, uint hi, uint unit)
{
	int mid = 0;
	
	if(lo > hi )
		return -EINVAL;

	if((value > 0) && (value < unit))
		return 0;

	mid = (lo + hi) / 2;

	if((unit<<mid) == value){
		return mid;
	}else if((unit<<lo) == value){
		return lo;
	}else if((unit<<hi) == value){
		return hi;
	}else if((unit<<mid) > value){
		if((mid - lo) <= 1)
			return mid;
		
		return biSearchGetBucketSizeShift(value, lo, mid, unit);
	}else{
		if((hi - mid) <= 1)
			return hi;
		
		return biSearchGetBucketSizeShift(value, mid, hi, unit);
	}
	
}

static int qdmalan_airoha_qdma_get_trtcm_param(struct airoha_qdma *qdma, int channel,
				       u32 addr, enum trtcm_param_type param,
				       enum trtcm_mode_type mode,
				       u32 *val_low, u32 *val_high)
{
	u32 idx = QDMA_METER_IDX(channel), group = QDMA_METER_GROUP(channel);
	u32 config = FIELD_PREP(TRTCM_PARAM_TYPE_MASK, param) |
			  FIELD_PREP(TRTCM_METER_GROUP_MASK, group) |
			  FIELD_PREP(TRTCM_PARAM_INDEX_MASK, idx) |
			  FIELD_PREP(TRTCM_PARAM_RATE_TYPE_MASK, mode);

	airoha_qdma_wr(qdma, REG_TRTCM_CFG_PARAM(addr), config);

	*val_low = airoha_qdma_rr(qdma, REG_TRTCM_DATA_LOW(addr));
	if (val_high)
		*val_high = airoha_qdma_rr(qdma, REG_TRTCM_DATA_HIGH(addr));

	return 0;
}

static int qdmalan_airoha_qdma_set_trtcm_param(struct airoha_qdma *qdma, int channel,
				       u32 addr, enum trtcm_param_type param,
				       enum trtcm_mode_type mode, u32 val)
{
	uint valueLo_tmp = 0;
	uint valueHi_tmp = 0;
	int retry_num = 100;
	u32 idx = QDMA_METER_IDX(channel), group = QDMA_METER_GROUP(channel);
	u32 config = TRTCM_PARAM_RW_MASK |
		     FIELD_PREP(TRTCM_PARAM_TYPE_MASK, param) |
		     FIELD_PREP(TRTCM_METER_GROUP_MASK, group) |
		     FIELD_PREP(TRTCM_PARAM_INDEX_MASK, idx) |
		     FIELD_PREP(TRTCM_PARAM_RATE_TYPE_MASK, mode);
			 
	do{	
		airoha_qdma_wr(qdma, REG_TRTCM_DATA_LOW(addr), val);
		wmb();
		airoha_qdma_wr(qdma, REG_TRTCM_CFG_PARAM(addr), config);
	
		qdmalan_airoha_qdma_get_trtcm_param(qdma, channel, addr, param, mode, &valueLo_tmp, &valueHi_tmp);
		if(val == valueLo_tmp)
			break;
	}while(--retry_num); //check if write successfully

	
	return 0;
}

static int qdmalan_airoha_qdma_set_trtcm_config(struct airoha_qdma *qdma, int channel,
					u32 addr, enum trtcm_mode_type mode,
					bool enable, u32 enable_mask)
{
	u32 val;

	if (qdmalan_airoha_qdma_get_trtcm_param(qdma, channel, addr, TRTCM_MISC_MODE,
					mode, &val, NULL))
		return -EINVAL;

	val = enable ? val | enable_mask : val & ~enable_mask;

	return qdmalan_airoha_qdma_set_trtcm_param(qdma, channel, addr, TRTCM_MISC_MODE,
					   mode, val);
}

static int qdmalan_airoha_qdma_set_trtcm_token_bucket(struct airoha_qdma *qdma,
					      int channel, u32 addr,
					      enum trtcm_mode_type mode,
					      u32 rate_val, u32 bucket_size)
{
	u32 val, config, tick, unit, rate, rate_frac, rl_mode_cfg, bucket_unit;
	int err;

	if (qdmalan_airoha_qdma_get_trtcm_param(qdma, channel, addr, TRTCM_MISC_MODE,
					mode, &config, NULL))
		return -EINVAL;

	val = airoha_qdma_rr(qdma, addr);
	tick = FIELD_GET(INGRESS_FAST_TICK_MASK, val);
	if (config & TRTCM_TICK_SEL)
		tick *= FIELD_GET(INGRESS_SLOW_TICK_RATIO_MASK, val);
	if (!tick)
		return -EINVAL;

	unit = (config & TRTCM_PKT_MODE) ? 1000000 / tick : 8000 / tick;
	if (!unit)
		return -EINVAL;

	rate = rate_val / unit;
	rate_frac = rate_val % unit;
	rate_frac = FIELD_PREP(TRTCM_TOKEN_RATE_MASK, rate_frac) / unit;
	rate = FIELD_PREP(TRTCM_TOKEN_RATE_MASK, rate) |
	       FIELD_PREP(TRTCM_TOKEN_RATE_FRACTION_MASK, rate_frac);

	err = qdmalan_airoha_qdma_set_trtcm_param(qdma, channel, addr,
					  TRTCM_TOKEN_RATE_MODE, mode, rate);
	if (err)
		return err;

	val = max_t(u32, bucket_size, MIN_TOKEN_SIZE);
	/* val = min_t(u32, __fls(val), MAX_TOKEN_SIZE_OFFSET); */
    err = qdmalan_airoha_qdma_get_trtcm_param(qdma, channel, REG_EGRESS_TRTCM_CFG, TRTCM_MISC_MODE, mode, &rl_mode_cfg, NULL);
    if(err){
        return err;
    }

    if (TRTCM_PKT_MODE == ((rl_mode_cfg & TRTCM_PKT_MODE) >> 1))
    {
		bucket_unit = trtcmBucketPacketUnit[EGRESS_TRTCM];
	}else{
		bucket_unit = trtcmBucketByteUnit[EGRESS_TRTCM];
	}

	val = biSearchGetBucketSizeShift(val, 0, 17, bucket_unit);
	val = min_t(u32, val, MAX_TOKEN_SIZE_OFFSET);

	return qdmalan_airoha_qdma_set_trtcm_param(qdma, channel, addr,
					   TRTCM_BUCKETSIZE_SHIFT_MODE,
					   mode, val);
}

int qdma_set_qdmalan_tx_ratelimit(QDMA_TxRateLimitSet_T *txRateLimitPtr)
{
	int i = 0, err = 0;
	struct airoha_qdma *qdma = &glb_eth->qdma[0];
	int channel = txRateLimitPtr->chnlIdx;
	int rate = txRateLimitPtr->rateLimitValue;
	u32 bucket_size = 0, unit = 0;
	u32 cfg = airoha_qdma_rr(qdma, REG_EGRESS_TRTCM_CFG);
	u32 tick = FIELD_GET(EGRESS_FAST_TICK_MASK, cfg);

	/* If rate >= 5Gbps (5000000 kbps), use fixed bucket_size 4096000;
	 * otherwise calculate bucket_size from tick register directly
	 */
	if (rate >= 5000000) {
		bucket_size = 4096000;
	} else {
		if (tick)
			tick *= FIELD_GET(EGRESS_SLOW_TICK_RATIO_MASK, cfg);
		if (!tick)
			tick = 1;

		unit = 8000 / tick;
		if (!unit)
			unit = 1;

		bucket_size = rate / unit + 1;
		if (bucket_size < 4096) {
			bucket_size = 4096;
		}
		else if (bucket_size > 4096000) {
			bucket_size = 4096000;
		}
	}

	for (i = 0; i <= TRTCM_PEAK_MODE; i++) {
		err = qdmalan_airoha_qdma_set_trtcm_config(qdma, channel,
						   REG_EGRESS_TRTCM_CFG, i,
						   !!rate, TRTCM_METER_MODE);
		if (err)
			return err;

		err = qdmalan_airoha_qdma_set_trtcm_token_bucket(qdma, channel,
							 REG_EGRESS_TRTCM_CFG,
							 i, rate, bucket_size);
		if (err)
			return err;
	}

	return 0;
}
EXPORT_SYMBOL(qdma_set_qdmalan_tx_ratelimit);

/************************************************************************
*                  QDMA APIs
*************************************************************************
*/
#define TXQ_DIS_CFG_REG_NUM				8
#define TXQ_DIS_QUEUE_CLOSE_OFFSET(chnl)		((chnl)&0xFC)
#define TXQ_DIS_CHANNEL_QUEUE_OFFSET(chnl,queue)	(1<<((queue)+(((chnl)&0x03)*8)))
#define QDMA_CSR_QUEUE_CLOSE_CFG(idx)         (0x00a0+TXQ_DIS_QUEUE_CLOSE_OFFSET(idx))


__inline__ static int qdmaChecConfigDone(struct airoha_qdma *base, uint offset, uint doneBit)
{
	int RETRY = 3 ;
	volatile uint regValue = 0 ;
	
	while(RETRY--) {
		//regValue = IO_GREG(reg) ;
		regValue = airoha_qdma_rr(base, offset);
		if(regValue & doneBit) {
			break ;
		}
		mdelay(1) ;
	}
	if(RETRY < 0) {
		return -ETIME ;
	}

	return 0;
}

static int qdmaSetQueueClose_sw(struct airoha_qdma *base, unchar channel, unchar queue){
	unchar chnl_offset = (channel>>2);
	uint txq_dis_cfg = 0;

	if(chnl_offset >= TXQ_DIS_CFG_REG_NUM){
		QDMA_ERR("wrong channel index.\n") ;
		return -1;
	}

	txq_dis_cfg = TXQ_DIS_CFG_VALUE[chnl_offset];
	TXQ_DIS_CFG_VALUE[chnl_offset] = (txq_dis_cfg | TXQ_DIS_CHANNEL_QUEUE_OFFSET(channel,queue));
	//IO_SREG(QDMA_CSR_QUEUE_CLOSE_CFG(base,channel), TXQ_DIS_CFG_VALUE[chnl_offset]);
	airoha_qdma_wr(base, QDMA_CSR_QUEUE_CLOSE_CFG(channel), TXQ_DIS_CFG_VALUE[chnl_offset]);
	
	return 0;
}

static int qdmaSetQueueOpen_sw(struct airoha_qdma *base, unchar channel, unchar queue){
	unchar chnl_offset = (channel>>2);
	uint txq_dis_cfg = 0;

	if(chnl_offset >= TXQ_DIS_CFG_REG_NUM){
		QDMA_ERR("wrong channel index.\n") ;
		return -1;
	}

	txq_dis_cfg = TXQ_DIS_CFG_VALUE[chnl_offset];
	TXQ_DIS_CFG_VALUE[chnl_offset] = (txq_dis_cfg & (~(TXQ_DIS_CHANNEL_QUEUE_OFFSET(channel,queue))));
	//IO_SREG(QDMA_CSR_QUEUE_CLOSE_CFG(base,channel), TXQ_DIS_CFG_VALUE[chnl_offset]);
	airoha_qdma_wr(base, QDMA_CSR_QUEUE_CLOSE_CFG(channel), TXQ_DIS_CFG_VALUE[chnl_offset]);
	
	return 0;
}

static uint qdmaIsQueueClosed_sw(unchar channel, unchar queue){
	unchar chnl_offset = (channel>>2);
	uint txq_dis_cfg = 0;
	uint ret = 0;

	if(chnl_offset >= TXQ_DIS_CFG_REG_NUM){
		QDMA_ERR("wrong channel index.\n") ;
		return 0;
	}

	txq_dis_cfg = TXQ_DIS_CFG_VALUE[chnl_offset];
	ret = (txq_dis_cfg & TXQ_DIS_CHANNEL_QUEUE_OFFSET(channel, queue));

	return ret;
}

static int qdmaSetTxQosSchedulerUpstream(struct airoha_qdma *base, unchar channel, unchar mode, ushort weight[8])
{
	int i = 0 ;
	//struct airoha_qdma *base = &glb_eth->qdma[1];
	uint wrrCfg = 0 ;
	
	for(i=0 ; i<8 ; i++) {
		wrrCfg = (TXWRR_RWCMD | 
				  ((weight[i]<<TXWRR_WRR_VALUE_SHIFT)&TXWRR_WRR_VALUE_MASK) |
				  ((channel<<TXWRR_CHNL_IDX_SHIFT)&TXWRR_CHNL_IDX_MASK) |
				  ((i<<TXWRR_QUEUE_IDX_SHIFT)&TXWRR_QUEUE_IDX_MASK)) ;
		
		//IO_SREG(QDMA_CSR_TXWRR_WEIGHT_CFG(base), wrrCfg) ;
		
		airoha_qdma_wr(base, QDMA_CSR_TXWRR_WEIGHT_CFG, wrrCfg);
	
		if(qdmaChecConfigDone(base, QDMA_CSR_TXWRR_WEIGHT_CFG, TXWRR_RWCMD_DONE) < 0) {
			QDMA_ERR("Timeout for setting WRR configuration, channel:%d, queue:%d.\n", channel, i) ;
			return -ETIME ;
		}
	}

	qdmaSetPerChnlQosMode(base, channel, mode) ;
	
	return 0 ;
}

static int qdmaGetTxQosSchedulerUpstream(struct airoha_qdma *base, unchar channel, unchar *pMode, ushort weight[8])
{
	int i = 0 ;
	uint wrrCfg = 0 ;
	
	*pMode = qdmaGetPerChnlQosMode(base, channel) ;

	for(i=0 ; i<8 ; i++) {
		wrrCfg = (((channel<<TXWRR_CHNL_IDX_SHIFT)&TXWRR_CHNL_IDX_MASK) |
				  ((i<<TXWRR_QUEUE_IDX_SHIFT)&TXWRR_QUEUE_IDX_MASK)) ;
		//IO_SREG(QDMA_CSR_TXWRR_WEIGHT_CFG(base), wrrCfg) ;
		airoha_qdma_wr(base, QDMA_CSR_TXWRR_WEIGHT_CFG, wrrCfg);
		
		if(qdmaChecConfigDone(base, QDMA_CSR_TXWRR_WEIGHT_CFG, TXWRR_RWCMD_DONE) < 0) {
			return -ETIME ;
		}
		//wrrCfg = IO_GREG(QDMA_CSR_TXWRR_WEIGHT_CFG(base)) ;
		
		wrrCfg = airoha_qdma_rr(base, QDMA_CSR_TXWRR_WEIGHT_CFG);
		weight[i] =  ((wrrCfg&TXWRR_WRR_VALUE_MASK)>>TXWRR_WRR_VALUE_SHIFT) ;
	}
	return 0 ;
}

static int qdma_set_tx_qos_upstream(unsigned int mainType, struct ECNT_QDMA_Data *qdma_data) 
{
	struct airoha_qdma *base = &glb_eth->qdma[1];

	int i = 0;
	unchar qosType = 0 ;
	ushort weight[CONFIG_QDMA_QUEUE];
	int weightNum[QDMA_TXQOS_TYPE_NUMS]={8, 0, 7, 6, 5, 4, 3, 2};
	QDMA_TxQosScheduler_T *pTxQos = qdma_data->qdma_private.qdma_tx_qos.pTxQos ;

	if(mainType == ECNT_QDMA_LAN)
		base = &glb_eth->qdma[0];


	if(pTxQos->channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}
	
	if(pTxQos->qosType >= QDMA_TXQOS_TYPE_NUMS) {
		return -EINVAL ;
	}
	qosType = (unchar)pTxQos->qosType;

    for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
        if( i >= weightNum[qosType] ){/*SP : open the queue*/
            qdmaSetQueueOpen_sw(base,pTxQos->channel,i) ;
        }else{/*WRR: check weight */
            if(pTxQos->queue[i].weight == 0){
                qdmaSetQueueClose_sw(base,pTxQos->channel,i) ;
            }else{
                qdmaSetQueueOpen_sw(base,pTxQos->channel,i) ;
            }
        }
    }

	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
        /*in case ,the packet queued in QDMA*/
        /*if wrr=0 , set wrr=1 and close the queue*/
        /*if DE add ageout function, need to be deleted*/
        if(pTxQos->queue[i].weight == 0)
            pTxQos->queue[i].weight = 1 ;

		weight[i] = pTxQos->queue[i].weight ;
	}
	
	return qdmaSetTxQosSchedulerUpstream(base, pTxQos->channel, qosType, weight) ;
}

int qdma_api_set_tx_qos_upstream(unsigned int mainType, QDMA_TxQosScheduler_T *pTxQos)
{
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TX_QOS;
	in_data.qdma_private.qdma_tx_qos.pTxQos = pTxQos ;
	ret = qdma_set_tx_qos_upstream(mainType, &in_data);
	return ret;
}
EXPORT_SYMBOL(qdma_api_set_tx_qos_upstream);

int qdma_get_tx_qos_upstream(unsigned int mainType, struct ECNT_QDMA_Data *qdma_data)
{
    struct airoha_qdma *base = &glb_eth->qdma[1];

	int ret = 0, i = 0 ;
	unchar qosType = 0 ;
	ushort weight[CONFIG_QDMA_QUEUE] ;
	QDMA_TxQosScheduler_T *pTxQos = qdma_data->qdma_private.qdma_tx_qos.pTxQos ;

	if(mainType == ECNT_QDMA_LAN)
		base = &glb_eth->qdma[0];
	
	if(pTxQos->channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}

	ret = qdmaGetTxQosSchedulerUpstream(base, pTxQos->channel, &qosType, weight) ;
	if(ret < 0) {
		return -EFAULT ;
	}
	
	pTxQos->qosType = qosType ;
	
	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
    //if DE add ageout function, need to be deleted
        if((weight[i]==1) && (qdmaIsQueueClosed_sw(pTxQos->channel,i)>0))
            weight[i] = 0 ;

		pTxQos->queue[i].weight = weight[i] ;
	}

	return 0 ;
}

int qdma_api_get_tx_qos_upstream(unsigned int mainType, QDMA_TxQosScheduler_T *pTxQos)
{
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TX_QOS;
	in_data.qdma_private.qdma_tx_qos.pTxQos = pTxQos ;
	ret = qdma_get_tx_qos_upstream(mainType, &in_data);
	return ret;
}
EXPORT_SYMBOL(qdma_api_get_tx_qos_upstream);

static int qdma_general_check_index_valid(GENERAL_TrtcmModuleType_T trtcmModule, GENERAL_TrtcmMode_T trtcmMode , ushort index)
{
    unchar meterIdx = GET_METER_IDX(index);
    unchar meterGrp = GET_METER_GRP(index);
    unchar maxMeterIdx[3] = {GENERAL_INGRESS_INDEX_MAX,GENERAL_INGRESS_INDEX_MAX_GRP1,GENERAL_INGRESS_INDEX_MAX_GRP2};
    
    /*Global ratelimit: no index check*/
    if(trtcmModule == GLB_RATECTL){
        return 0 ;
    }
	/*check index range*/
	/* CID:926666 */
	if( (trtcmModule == INGRESS_TRTCM) && (trtcmMode == TRTCM_RATELIMIT_MODE) ){
		if( (meterGrp >= 3) || (meterIdx >= maxMeterIdx[meterGrp]) ){ /*0~127*/
			return -EINVAL ;
		}
	}else if( (trtcmModule == INGRESS_TRTCM) && (trtcmMode == TRTCM_MODE) ){
		if( (meterGrp >= 3) || (meterIdx >= (maxMeterIdx[meterGrp]>>1)) ){/*0~63*/
			return -EINVAL ;
		}
	}else if( trtcmModule == SLA_TRTCM ){
		if( meterIdx > GENERAL_SLA_INDEX_MAX ){/*SLA index 0~3 + queue 0~7 => max value is 0x1F*/
			return -EINVAL ;
		}
	}else if( trtcmModule == EGRESS_QUEUE_RATELIMIT ){
		if( QUEUE_ALL_NUM < 256 && meterIdx > QUEUE_ALL_NUM ){/*0~255*/
			return -EINVAL ;
		}
	}else{/*Egress : the hardware fix TRTCM_MODE*/
		if( meterIdx > (CONFIG_QDMA_CHANNEL-1) ){/*LAN: 0~12 ; WAN: 0~31*/
			return -EINVAL ;
		}
	}

	return 0 ;
}

static int generalGetBucketSizeByRate(uint rateLimitValue)
{
	int bucksize = 0;

	if(rateLimitValue <= (METER_1K<<2))
	{
		bucksize = (64<<10);
	}
	else if(rateLimitValue <= (METER_1K<<3))
	{
		bucksize = (512<<10);
	}
	else if(rateLimitValue <= (METER_1K<<4))
	{
		bucksize = (METER_1M);
	}
	else if(rateLimitValue <= (METER_1K<<7))
	{
		bucksize = (METER_1M * 8);
	}
	else if(rateLimitValue <= (METER_1K<<8))
	{
		bucksize = (METER_1M * 24);
	}
	else
	{
		bucksize = (METER_1M * 64);
	}

	return bucksize;
}

/*RxRing Ratelimit APIs*/
static inline void qdma_init_trtcm(void)
{
    trtcmCfgBase[INGRESS_TRTCM] = REG_INGRESS_TRTCM_CFG;
    trtcmBucketByteUnit[INGRESS_TRTCM] = 1024;
    trtcmBucketPacketUnit[INGRESS_TRTCM] = 16;
	
    trtcmCfgBase[SLA_TRTCM]= REG_SLA_TRTCM_CFG;
    trtcmBucketByteUnit[SLA_TRTCM] = 1024;
    trtcmBucketPacketUnit[SLA_TRTCM] = 16;
	
    trtcmCfgBase[EGRESS_TRTCM]=  REG_EGRESS_TRTCM_CFG;
    trtcmBucketByteUnit[EGRESS_TRTCM] = 1024;
    trtcmBucketPacketUnit[EGRESS_TRTCM] = 16;

    trtcmCfgBase[GLB_RATECTL]= REG_GLB_TRTCM_CFG;
    trtcmBucketByteUnit[GLB_RATECTL] = 256;
    trtcmBucketPacketUnit[GLB_RATECTL] = 256;

    return;
}

static int airoha_generalGetTrtcmMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType){
	uint offset =0;

	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	offset = trtcmCfgBase[trtcmModuleType]; 

	return FIELD_GET(TRTCM_MODE_MASK, airoha_qdma_rr(qdma, offset));
}

static int generalGetRatelimitParaConfig(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType
	, GENERAL_TrtcmParaType_T paraType, ushort index, uint *valueLo,uint *valueHi) {
	uint trtcmParaCfg = 0 ;
	uint trtcmBase =0;
	u32 status;

	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (((paraType<<RATELIMIT_PARA_TYPE_SHIFT)&RATELIMIT_PARA_TYPE_MASK) |
				((QDMA_METER_IDX(index)<<RATELIMIT_PARA_IDX_INDEX_SHIFT)&RATELIMIT_PARA_IDX_INDEX_MASK)) ;

    trtcmParaCfg |= (QDMA_METER_GROUP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);

	airoha_qdma_wr(qdma, TRTCM_PARAM_CFG(trtcmBase), trtcmParaCfg);

	if(read_poll_timeout(airoha_qdma_rr, status,
					status & TRTCM_PARAM_RW_DONE_MASK,
					USEC_PER_MSEC, 10 * USEC_PER_MSEC,
					true, qdma,
					TRTCM_PARAM_CFG(trtcmBase)))
			return -ETIMEDOUT;

	*valueLo = airoha_qdma_rr(qdma, TRTCM_DATA_LO(trtcmBase));
	*valueHi = airoha_qdma_rr(qdma, TRTCM_DATA_HI(trtcmBase));
	
	return 0;
}

static int generalSetRatelimitParaConfig(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType
	, GENERAL_TrtcmParaType_T paraType, ushort index, uint valueLo) {
	uint trtcmParaCfg = 0 ;
	uint trtcmBase =0;
	ulong flags=0 ;
	uint valueLo_tmp = 0;
	uint valueHi_tmp = 0;
	u32 status;

	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (RATELIMIT_PARA_RWCMD |
				((paraType<<RATELIMIT_PARA_TYPE_SHIFT)&RATELIMIT_PARA_TYPE_MASK) |
				((QDMA_METER_IDX(index)<<RATELIMIT_PARA_IDX_INDEX_SHIFT)&RATELIMIT_PARA_IDX_INDEX_MASK));

    trtcmParaCfg |= (QDMA_METER_GROUP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);
	
	do{	
		spin_lock_irqsave(&qdma_cfg_lock, flags) ;
		airoha_qdma_wr(qdma, TRTCM_DATA_LO(trtcmBase), valueLo);
		wmb();
		airoha_qdma_wr(qdma, TRTCM_PARAM_CFG(trtcmBase), trtcmParaCfg) ;
		spin_unlock_irqrestore(&qdma_cfg_lock, flags) ;
	
		generalGetRatelimitParaConfig(qdma, trtcmModuleType, paraType, index, &valueLo_tmp, &valueHi_tmp);
	}while(valueLo != valueLo_tmp);

	if(read_poll_timeout(airoha_qdma_rr, status,
					status & TRTCM_PARAM_RW_DONE_MASK,
					USEC_PER_MSEC, 10 * USEC_PER_MSEC,
					true, qdma,
					TRTCM_PARAM_CFG(trtcmBase)))
			return -ETIMEDOUT;

	return 0;
}


/*set chnl/ring/flow/etc trtcm enable/disable*/
static int generalSetRatelimitMeterMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType
, GENERAL_TrtcmMeter_T meterMode, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (meterMode < GENERAL_METER_DISABLE) || (meterMode > GENERAL_METER_ENABLE)){
		pr_err("Trtcm Meter Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(generalGetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, &valueLo, &valueHi) < 0){
		pr_err("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (meterMode == GENERAL_METER_ENABLE) ? (valueLo|TRTCM_METER_MODE):(valueLo &(~TRTCM_METER_MODE)) ;
	
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, valueLo) < 0){
		pr_err("Set TRTCM Para Config : Ratelimit Meter Enable Failed.\n");
		return -EFAULT;	
	}

	return 0;
}

static int generalSetRatelimitPktMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmPktMode_T pktMode, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (pktMode < TRTCM_BYTE_MODE) || (pktMode > TRTCM_PACKET_MODE) ){
		printk("Trtcm Packet Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(generalGetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, &valueLo, &valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (pktMode == TRTCM_PACKET_MODE) ? (valueLo|TRTCM_PKT_MODE):(valueLo &(~TRTCM_PKT_MODE)) ;
	
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, valueLo) < 0){
		printk("Set TRTCM Para Config : Ratelimit Packet Enable Failed.\n");
		return -EFAULT;	
	}
		
	return 0;
}

static int generalSetRatelimitTickSel(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmTickSel_T tickSel, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (tickSel < TRTCM_FAST_TICK) || (tickSel > TRTCM_SLOW_TICK) ){
		printk("Trtcm TickSel Index should be 0 or 1.\n");
		return -EINVAL;
	}

	if(generalGetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, &valueLo, &valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (tickSel == TRTCM_SLOW_TICK) ? (valueLo|TRTCM_TICK_SEL):(valueLo &(~TRTCM_TICK_SEL)) ;
			
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, valueLo) < 0){
		printk("Set TRTCM Para Config : Ratelimit Tick Sel Failed.\n");
		return -EFAULT;	
	}

	return 0;
}

static int airoha_generalGetTrtcmParaConfig(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmParaType_T paraType, enum trtcm_mode_type rateType, ushort index, uint *valueLo,uint *valueHi) {
	uint trtcmParaCfg = 0 ;
	uint trtcmBase =0;
	u32 status;

	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType > EGRESS_TRTCM) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (((paraType<<TRTCM_PARA_TYPE_SHIFT)&TRTCM_PARA_TYPE_MASK) |
				((QDMA_METER_IDX(index)<<TRTCM_PARA_IDX_INDEX_SHIFT)&TRTCM_PARA_IDX_INDEX_MASK) |
				((rateType<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)&TRTCM_PARA_IDX_RATE_TYPE_MASK)) ;

    trtcmParaCfg |= (QDMA_METER_GROUP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);

	airoha_qdma_wr(qdma, TRTCM_PARAM_CFG(trtcmBase), trtcmParaCfg);
	if(read_poll_timeout(airoha_qdma_rr, status,
					status & TRTCM_PARAM_RW_DONE_MASK,
					USEC_PER_MSEC, 10 * USEC_PER_MSEC,
					true, qdma,
					TRTCM_PARAM_CFG(trtcmBase))){
		printk("Timeout for Get TRTCM configuration.\n") ;
		return -ETIME ;
	}

	*valueLo = airoha_qdma_rr(qdma, TRTCM_DATA_LO(trtcmBase));
	*valueHi = airoha_qdma_rr(qdma, TRTCM_DATA_HI(trtcmBase));
	
	return 0;
}

static int airoha_generalSetTrtcmParaConfig(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmParaType_T paraType, enum trtcm_mode_type rateType, ushort index, uint valueLo) {
	uint trtcmParaCfg = 0 ;
	uint trtcmBase =0;
	ulong flags=0 ;
	uint valueLo_tmp = 0;
	uint valueHi_tmp = 0;
	u32 status;
	/*get trtcm config addr*/
	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType > EGRESS_TRTCM) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	trtcmParaCfg = (TRTCM_PARA_RWCMD |
				((paraType<<TRTCM_PARA_TYPE_SHIFT)&TRTCM_PARA_TYPE_MASK) |
				((QDMA_METER_IDX(index)<<TRTCM_PARA_IDX_INDEX_SHIFT)&TRTCM_PARA_IDX_INDEX_MASK) |
				((rateType<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)&TRTCM_PARA_IDX_RATE_TYPE_MASK)) ;

    trtcmParaCfg |= (QDMA_METER_GROUP(index)<<TRTCM_PARA_METER_GROUP_SHIFT);
	
	do{	
		spin_lock_irqsave(&qdma_cfg_lock, flags) ;
		airoha_qdma_wr(qdma, TRTCM_DATA_LO(trtcmBase), valueLo);
		wmb();
		airoha_qdma_wr(qdma, TRTCM_PARAM_CFG(trtcmBase), trtcmParaCfg) ;
		spin_unlock_irqrestore(&qdma_cfg_lock, flags) ;
		airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType, paraType, rateType, index, &valueLo_tmp, &valueHi_tmp);
	}while(valueLo != valueLo_tmp);
	
	if(read_poll_timeout(airoha_qdma_rr, status,
					status & TRTCM_PARAM_RW_DONE_MASK,
					USEC_PER_MSEC, 10 * USEC_PER_MSEC,
					true, qdma,
					TRTCM_PARAM_CFG(trtcmBase))) {
		printk("Timeout for set TRTCM configuration.\n") ;
		return -ETIME ;
	}

	return 0;
}

/*set chnl/ring/flow/etc trtcm enable/disable*/
static int airoha_generalSetTrtcmMeterMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmMeter_T meterMode, enum trtcm_mode_type rateType, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (meterMode < GENERAL_METER_DISABLE) || (meterMode > GENERAL_METER_ENABLE)){
		printk("Trtcm Meter Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, &valueLo,&valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (meterMode == GENERAL_METER_ENABLE) ? (valueLo|TRTCM_METER_MODE):(valueLo &(~TRTCM_METER_MODE)) ;
	
	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, valueLo) < 0){
		printk("Set TRTCM Para Config : %s Meter Enable Failed.\n",rateType ? "PIR" : "CIR");
		return -EFAULT;	
	}

	return 0;
}

static int airoha_generalSetTrtcmPktMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmPktMode_T pktMode, enum trtcm_mode_type rateType, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (pktMode < TRTCM_BYTE_MODE) || (pktMode > TRTCM_PACKET_MODE) ){
		printk("Trtcm Packet Mode should be 0 or 1.\n");
		return -EINVAL;
	}

	if(airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, &valueLo,&valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (pktMode == TRTCM_PACKET_MODE) ? (valueLo|TRTCM_PKT_MODE):(valueLo &(~TRTCM_PKT_MODE)) ;
	
	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, valueLo) < 0){
		printk("Set TRTCM Para Config : %s Packet Enable Failed.\n",rateType ? "PIR" : "CIR");
		return -EFAULT;	
	}
		
	return 0;
}

static int airoha_generalSetTrtcmTickSel(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmTickSel_T tickSel, enum trtcm_mode_type rateType, ushort index){
	uint valueLo = 0, valueHi = 0;

	if( (tickSel < TRTCM_FAST_TICK) || (tickSel > TRTCM_SLOW_TICK) ){
		printk("Trtcm TickSel Index should be 0 or 1.\n");
		return -EINVAL;
	}

	if(airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, &valueLo,&valueHi) < 0){
		printk("Get TRTCM Para Config Failed.\n");
		return -EFAULT;
	}

	valueLo = (tickSel == TRTCM_SLOW_TICK) ? (valueLo|TRTCM_TICK_SEL):(valueLo &(~TRTCM_TICK_SEL)) ;
			
	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_MISC, rateType, index, valueLo) < 0){
		printk("Set TRTCM Para Config : %s Tick Sel Failed.\n",rateType ? "PIR" : "CIR");
		return -EFAULT;	
	}

	return 0;

}

static int airoha_qdma_general_set_ratelimit_mode_cfg(struct airoha_qdma *qdma, GENERAL_TrtcmRatelimitCfg_T rxRateLimitCfg)
{
	int ret=0;
	enum trtcm_mode_type rateType;
	GENERAL_TrtcmMode_T trtcmMode = airoha_generalGetTrtcmMode(qdma, rxRateLimitCfg.trtcmModule);

    /*no trtcm mode for mtr_grp1&mtr_grp2*/
    if(QDMA_METER_GROUP(rxRateLimitCfg.Index) > 0){
        trtcmMode = TRTCM_RATELIMIT_MODE;
	}

	/*1. check index range*/
	if( (ret = qdma_general_check_index_valid(rxRateLimitCfg.trtcmModule, trtcmMode, rxRateLimitCfg.Index)) < 0 ){
		pr_err("Invalid meter index: %d\n", rxRateLimitCfg.Index);
		return ret ;
	}

	/*2. set cfg*/
	if( trtcmMode == TRTCM_RATELIMIT_MODE ){/*ratelimit mode, set just one index*/
		if((ret = generalSetRatelimitMeterMode(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.MeterEn, rxRateLimitCfg.Index )) < 0){
			pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
			return ret;
		}
		if((ret = generalSetRatelimitPktMode(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.PktMode, rxRateLimitCfg.Index )) < 0){
			pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
			return ret;
		}
		if((ret = generalSetRatelimitTickSel(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.TickSel, rxRateLimitCfg.Index )) < 0){
			pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
			return ret;
		}
	}else{/*trtcm mode, set CIR & PIR the cfg*/
		for( rateType = TRTCM_COMMIT_MODE ; rateType <= TRTCM_PEAK_MODE ; rateType++ ){
			if((ret = airoha_generalSetTrtcmMeterMode(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.MeterEn, rateType , rxRateLimitCfg.Index )) < 0){
				pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
				return ret;
			}
			if((ret = airoha_generalSetTrtcmPktMode(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.PktMode, rateType ,rxRateLimitCfg.Index )) < 0){
				pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
				return ret;
			}
			if((ret = airoha_generalSetTrtcmTickSel(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.TickSel, rateType ,rxRateLimitCfg.Index )) < 0){
				pr_err("Fault:Set ratelimit mode cfg error.\n") ; 
				return ret;
			}
		}
	}
	
	return 0 ;
}
/*set trtcm mode: TRTCM mode or Ratelimit Mode*/
static int generalSetTrtcmMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, GENERAL_TrtcmMode_T trtcmMode){
	uint trtcmBase =0;

	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];
	
	if( (trtcmMode < TRTCM_RATELIMIT_MODE) || (trtcmMode > TRTCM_MODE) )
		return -EINVAL;
/*
	qdmaSetGeneralTrtcmMode(trtcmBase,trtcmMode);
*/
	airoha_qdma_rmw(qdma, trtcmBase,
			TRTCM_MODE_MASK, FIELD_PREP(TRTCM_MODE_MASK, trtcmMode));

	return 0;
}
static int airoha_generalGetTrtcmPktMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, enum trtcm_mode_type rateType, ushort index){

	uint valueLo = 0, valueHi = 0;

	airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType,TRTCM_MISC, rateType, index, &valueLo, &valueHi);

	return  ((valueLo & TRTCM_PKT_MODE) >> 1);
}

static int airoha_generalSetTrtcmBucketSize(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, uint bucketSize, enum trtcm_mode_type rateType, ushort index){
	int bucketSize_shift = 0;
	uint bucketUnit = 0;

	/*max: 128Mbyte*/
	if(bucketSize > 0x8000000){
		return -EINVAL;	
	}

	if ((int)TRTCM_BYTE_MODE == airoha_generalGetTrtcmPktMode(qdma, trtcmModuleType, rateType, index)){
		bucketUnit = trtcmBucketByteUnit[trtcmModuleType];
	}else{
		bucketUnit = trtcmBucketPacketUnit[trtcmModuleType];
	}

	bucketSize_shift = biSearchGetBucketSizeShift(bucketSize, 0, 17, bucketUnit);
	
	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_BUCKETSIZE_SHIFT, rateType, index, bucketSize_shift) < 0){
		printk("Set TRTCM Bucket Size Failed.\n");
		return -EFAULT;
	}
	
	return 0;
}

static int airoha_generalGetTrtcmTickSel(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, enum trtcm_mode_type rateType, ushort index){

	uint valueLo = 0, valueHi = 0;

	airoha_generalGetTrtcmParaConfig(qdma, trtcmModuleType,TRTCM_MISC, rateType, index, &valueLo, &valueHi);

	return  (valueLo & TRTCM_TICK_SEL);
}
/*get trtcm  ratelimit fasttick*/
static int airoha_generalGetTrtcmFastTick(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType){
	uint trtcmBase =0;

	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType]; 

	return FIELD_GET(TRTCM_FAST_TICK_MASK, airoha_qdma_rr(qdma, trtcmBase));
}
/*get trtcm  ratelimit slow tick ratio*/
static ushort airoha_generalGetTrtcmSlowTickRatio(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType){
	uint trtcmBase =0;

	if( (trtcmModuleType < INGRESS_TRTCM) || (trtcmModuleType >= TRTCM_MODE_MAX) )
		return -EINVAL;
	trtcmBase = trtcmCfgBase[trtcmModuleType];

	return FIELD_GET(TRTCM_SLOW_TICKRATIO_MASK, airoha_qdma_rr(qdma, trtcmBase));
}

static uint airoha_generalGetTrtcmSlowTick(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType)
{
	ushort fastTick = 0;
	ushort slowTickRatio = 0;
	uint slowTick = 0;
	fastTick = airoha_generalGetTrtcmFastTick(qdma, trtcmModuleType);
	slowTickRatio = airoha_generalGetTrtcmSlowTickRatio(qdma, trtcmModuleType);

	slowTick = (uint)fastTick * (uint)slowTickRatio ;
	return  slowTick;
}

/*
 tokenRate_integer = ratelimitvalue /unit 
 tokenRate_fraction =( ratelimtvalue % unit )
so should make sure the 2 params valid.
*/
static int airoha_generalSetTrtcmTokenRate(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, uint rateLimitValue, enum trtcm_mode_type rateType, ushort index){
	uint tokenRate,bucketSize,tokenRate_integer = 0;
	ushort tokenRate_fraction = 0;
	int curTicksel = 0;
	ushort rateLimitUnit = 0 ;

	/*1. get tick sel*/
	if( (int)TRTCM_FAST_TICK == airoha_generalGetTrtcmTickSel(qdma, trtcmModuleType, rateType, index) )/*fast tick mode*/
		curTicksel = airoha_generalGetTrtcmFastTick(qdma, trtcmModuleType) ;
	else/*slow tick mode*/
		curTicksel = airoha_generalGetTrtcmSlowTick(qdma, trtcmModuleType) ;

	/*2. get ratelimit mode, calculate Unit*/
	if(0 >= curTicksel)
	{
		printk("tick = 0 , set error.\n");
		return -EINVAL;
	}
	else
	{
		if( (int)TRTCM_BYTE_MODE == airoha_generalGetTrtcmPktMode(qdma, trtcmModuleType, rateType, index) )
			rateLimitUnit = 8000 / curTicksel ;  	/** 8bits X 1000 / (curTicksel X 10e-6 s)  kbps **/
		else
			rateLimitUnit = 1000000 / curTicksel ;  /** 1 / (curTicksel X 10e-6 s) pps **/
	}

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , set error.\n");
		return -EINVAL;
	}

	/*3. calculate tokenRate*/
	tokenRate_integer = rateLimitValue / rateLimitUnit ;
	tokenRate_fraction =( rateLimitValue % rateLimitUnit) * 64 / rateLimitUnit ;

	if( (tokenRate_integer > 0x3FFFF) || (tokenRate_fraction > 0x3F) ){
		printk("tokenRate overflow.\n");
		return -EINVAL;
	}

	tokenRate = (tokenRate_integer << TRTCM_TOKEN_RATE_INTEGER_SHIFT) | tokenRate_fraction;

	if(airoha_generalSetTrtcmParaConfig(qdma, trtcmModuleType, TRTCM_TOKEN_RATE, rateType, index, tokenRate) < 0){
		printk("Set TRTCM Token Rate Failed.\n");
		return -EFAULT;
	}

	/*4. calculate bucketSize*/
	/*Egress & Global & SLA: bucketsize should be small to prevent burst*/
	/*Ingress: bucketsize should be bigger for TCP flow*/
	if(trtcmModuleType == INGRESS_TRTCM){
		if( (int)TRTCM_BYTE_MODE == airoha_generalGetTrtcmPktMode(qdma, trtcmModuleType, rateType, index) )
			bucketSize = generalGetBucketSizeByRate(rateLimitValue);
		else
			bucketSize = rateLimitValue<<RATELIMIT_PKT_MODE_BUCKET_SHIFT;
	}else{
		bucketSize = tokenRate_integer+1;
		if(bucketSize < 4096)/* if bucketsize is lower than 4096, 1518Byte flow will lead to bucket overflow */
			bucketSize = 4096;
	}
    
	return bucketSize;

}
static uint airoha_generalGetTrtcmTokenRate(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType,enum trtcm_mode_type rateType, ushort index){
	uint valueLo = 0, valueHi = 0;
	uint tokenRate_integer = 0;
	ushort tokenRate_fraction = 0;
	int curTicksel = 0;
	ushort rateLimitUnit = 0 ;
	/*1. get tick sel*/
	if((int)TRTCM_FAST_TICK == airoha_generalGetTrtcmTickSel(qdma, trtcmModuleType, rateType, index) )/*fast tick mode*/
		curTicksel = airoha_generalGetTrtcmFastTick(qdma, trtcmModuleType) ;
	else/*slow tick mode*/
		curTicksel = airoha_generalGetTrtcmSlowTick(qdma, trtcmModuleType) ;

	/*2. get ratelimit mode, calculate Unit*/
	if(0 >= curTicksel)
	{
		printk("tick = 0 , set error.\n");
		return -EINVAL;
	}
	else
	{
		if((int) TRTCM_BYTE_MODE == airoha_generalGetTrtcmPktMode(qdma, trtcmModuleType, rateType, index) )
			rateLimitUnit = 8000 / curTicksel ;  	/** 8bits X 1000 / (curTicksel X 10e-6 s)  kbps **/
		else
			rateLimitUnit = 1000000 / curTicksel ;  /** 1 / (curTicksel X 10e-6 s) pps **/
	}

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , set error.\n");
		return -EINVAL;
	}

	

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , get error.\n");
		return -EINVAL;
	}

	/*3.calculate ratelimit value*/
	generalGetRatelimitParaConfig(qdma,trtcmModuleType, TRTCM_TOKEN_RATE, index, &valueLo, &valueHi);
	tokenRate_integer = (valueLo &  TRTCM_TOKEN_RATE_INTEGER_MASK ) >> TRTCM_TOKEN_RATE_INTEGER_SHIFT;
	tokenRate_fraction = (valueLo & TRTCM_TOKEN_RATE_FRACTION_MASK );

	return	( (tokenRate_integer*rateLimitUnit) + (tokenRate_fraction*rateLimitUnit/64) );
}

static int generalGetRatelimitPktMode(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, ushort index){

	uint valueLo = 0, valueHi = 0;

	generalGetRatelimitParaConfig(qdma, trtcmModuleType,TRTCM_MISC, index, &valueLo, &valueHi);

	return  ((valueLo & TRTCM_PKT_MODE) >> 1);
}
static int generalSetRatelimitBucketSize(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, uint bucketSize, ushort index){
	int bucketSize_shift = 0;
    uint bucketUnit = 0;

    /*max: 128Mbyte*/
	if(bucketSize > 0x8000000){
		return -EINVAL;	
	}

	if (TRTCM_BYTE_MODE == generalGetRatelimitPktMode(qdma, trtcmModuleType, index)){
		bucketUnit = trtcmBucketByteUnit[trtcmModuleType];
	}else{
		bucketUnit = trtcmBucketPacketUnit[trtcmModuleType];
	}

	bucketSize_shift = biSearchGetBucketSizeShift(bucketSize, 0, 17, bucketUnit);
	
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_BUCKETSIZE_SHIFT, index, bucketSize_shift) < 0){
		printk("Set TRTCM Bucket Size Failed.\n");
		return -EFAULT;
	}
	
	return 0;
}
static int generalGetRatelimitTickSel(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, ushort index){

	uint valueLo = 0, valueHi = 0;

	generalGetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_MISC, index, &valueLo, &valueHi);

	return  (valueLo & TRTCM_TICK_SEL);
}
/*
 tokenRate_integer = ratelimitvalue /unit 
 tokenRate_fraction =( ratelimtvalue % unit )
so should make sure the 2 params valid.
*/
static int generalSetRatelimitTokenRate(struct airoha_qdma *qdma, GENERAL_TrtcmModuleType_T trtcmModuleType, uint rateLimitValue, ushort index){
	int bucketSize = 0;
	uint tokenRate = 0, tokenRate_integer = 0;
	ushort tokenRate_fraction = 0;
	int curTicksel = 0;
	ushort rateLimitUnit = 0 ;

	/*1. get tick sel*/
	if( TRTCM_FAST_TICK == generalGetRatelimitTickSel(qdma, trtcmModuleType, index) )/*fast tick mode*/
		curTicksel = airoha_generalGetTrtcmFastTick(qdma, trtcmModuleType) ;
	else/*slow tick mode*/
		curTicksel = airoha_generalGetTrtcmSlowTick(qdma, trtcmModuleType) ;

	/*2. get ratelimit mode, calculate Unit*/
	if(0 >= curTicksel)
	{
		printk("tick = 0 , set error.\n");
		return -EINVAL;
	}
	else
	{
		if( TRTCM_BYTE_MODE == generalGetRatelimitPktMode(qdma, trtcmModuleType, index) )
			rateLimitUnit = 8000 / curTicksel ;  	/** 8bits X 1000 / (curTicksel X 10e-6 s)  kbps **/
		else
			rateLimitUnit = 1000000 / curTicksel ;  /** 1 / (curTicksel X 10e-6 s) pps **/
	}

	if( 0 == rateLimitUnit )
	{
		printk("rateLimitUnit = 0 , set error.\n");
		return -EINVAL;
	}

	/*3. calculate tokenRate*/
	tokenRate_integer = rateLimitValue / rateLimitUnit ;
	tokenRate_fraction =( rateLimitValue % rateLimitUnit) * 64 / rateLimitUnit ;

	if( (tokenRate_integer > 0x3FFFF) || (tokenRate_fraction > 0x3F) ){
		printk("tokenRate overflow.\n");
		return -EINVAL;
	}

	tokenRate = (tokenRate_integer << TRTCM_TOKEN_RATE_INTEGER_SHIFT) | tokenRate_fraction;
	
	if(generalSetRatelimitParaConfig(qdma, trtcmModuleType, TRTCM_TOKEN_RATE, index, tokenRate) < 0){
		printk("Set TRTCM Token Rate Failed.\n");
		return -EFAULT;
	}

	/*4. calculate bucketSize*/
	/*Egress & Global: bucketsize should be small to prevent burst*/
	/*Ingress: bucketsize should be bigger for TCP flow*/
	if(trtcmModuleType == INGRESS_TRTCM){
		if( TRTCM_BYTE_MODE == generalGetRatelimitPktMode(qdma, trtcmModuleType, index) )
			bucketSize = generalGetBucketSizeByRate(rateLimitValue);
		else
			bucketSize = rateLimitValue<<RATELIMIT_PKT_MODE_BUCKET_SHIFT;
	}else{
		bucketSize = tokenRate_integer+1;
		if(bucketSize < 4096)/* if bucketsize is lower than 4096, 1518Byte flow will lead to bucket overflow */
			bucketSize = 4096;
	}
    
	return bucketSize;
}
static int airoha_qdma_general_set_ratelimit_mode_value(struct airoha_qdma *qdma, GENERAL_TrtcmRatelimitSet_T rxRateLimitCfg)
{
	int ret=0;
	enum trtcm_mode_type rateType ;
	GENERAL_TrtcmMode_T trtcmMode = airoha_generalGetTrtcmMode(qdma, rxRateLimitCfg.trtcmModule) ;
    int bucketSize=0;

    /*no trtcm mode for mtr_grp1&mtr_grp2*/
    if(QDMA_METER_GROUP(rxRateLimitCfg.Index) > 0){
        trtcmMode = TRTCM_RATELIMIT_MODE;
	}

	/*1. check index range*/
	if( (ret = qdma_general_check_index_valid(rxRateLimitCfg.trtcmModule, trtcmMode, rxRateLimitCfg.Index)) < 0 )
		return ret ;

	/*2. set value*/
	if( trtcmMode == TRTCM_RATELIMIT_MODE ){/*ratelimit mode, set just one index*/
		if((bucketSize = generalSetRatelimitTokenRate(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.RateLimitValue, rxRateLimitCfg.Index)) < 0) {
			pr_err("Fault: set ratelimit mode value error.\n");
			return bucketSize ;
		}
        /*set BucketSize*/
		if((ret = generalSetRatelimitBucketSize(qdma, rxRateLimitCfg.trtcmModule, bucketSize, rxRateLimitCfg.Index))< 0) {
			pr_err("Fault: set ratelimit mode value error.\n");
			return ret ;
		}
	}else{/*trtcm mode, set CIR & PIR the cfg*/
		for( rateType = TRTCM_COMMIT_MODE ; rateType <= TRTCM_PEAK_MODE ; rateType++ ){
			if((bucketSize = airoha_generalSetTrtcmTokenRate(qdma, rxRateLimitCfg.trtcmModule, rxRateLimitCfg.RateLimitValue, rateType, rxRateLimitCfg.Index)) < 0) {
				pr_err("Fault: set ratelimit mode value error.\n");
				return bucketSize ;
			}
			if((ret = airoha_generalSetTrtcmBucketSize(qdma, rxRateLimitCfg.trtcmModule, bucketSize, rateType, rxRateLimitCfg.Index))< 0) {
				pr_err("Fault: set ratelimit mode value error.\n");
				return ret ;
			}
		}
	}
	
	return 0 ;
}

static int qdma_dev_trtcm_cfg_init(void)
{
	memset(trtcmCfgBase, 0, sizeof(uint) * TRTCM_MODE_MAX);
	memset(trtcmBucketByteUnit, 0, sizeof(uint) * TRTCM_MODE_MAX);
	memset(trtcmBucketPacketUnit, 0, sizeof(uint) * TRTCM_MODE_MAX);

    qdma_init_trtcm();
	
	return 0;
}

int airoha_dp_api_qdma_set_meter_cfg(struct airoha_qdma *qdma, unsigned char group_id, uint meterIdx, uint value)
{
	GENERAL_TrtcmRatelimitCfg_T rxRateLimitCfg;

    /*cfg setting*/
    if(value > 0){
        rxRateLimitCfg.MeterEn = GENERAL_METER_ENABLE;
    }else{
        rxRateLimitCfg.MeterEn = GENERAL_METER_DISABLE;
    }    
    rxRateLimitCfg.PktMode = TRTCM_PACKET_MODE;
    rxRateLimitCfg.trtcmModule = INGRESS_TRTCM;
    rxRateLimitCfg.TickSel = TRTCM_FAST_TICK; // default FAST
    rxRateLimitCfg.PktMode = TRTCM_BYTE_MODE; // kbps
    rxRateLimitCfg.Index = (group_id << 8) | meterIdx;

	return airoha_qdma_general_set_ratelimit_mode_cfg(qdma, rxRateLimitCfg);
}

int airoha_dp_api_qdma_get_meter_value(struct airoha_qdma *qdma,uint meterIdx){
	int rate = 0;	
	uint trtcmModule = INGRESS_TRTCM;
	uint rateType = TRTCM_COMMIT_MODE;
	
	if((rate = airoha_generalGetTrtcmTokenRate(qdma, trtcmModule, rateType, meterIdx)) < 0) {
		pr_err("Fault: get ratelimit mode value error.\n");
		return rate ;
	}
	return rate ;
}
int airoha_dp_api_qdma_set_meter_value(struct airoha_qdma *qdma,uint meterIdx,uint value)
{
	/* CID:1088150 */
    GENERAL_TrtcmRatelimitSet_T rxRateLimitSet = {0};
	  
    /*ratelimit glb setting*/
    rxRateLimitSet.trtcmModule = INGRESS_TRTCM;

	/*ratelimit each meter*/
	rxRateLimitSet.Index = meterIdx; 
	rxRateLimitSet.RateLimitValue = value;
    
	return airoha_qdma_general_set_ratelimit_mode_value(qdma, rxRateLimitSet);
}

void airoha_dp_api_qdma_meter_default_config(struct airoha_qdma *qdma)
{
    int ringIdx = 0;
	
	/* CID:1088152 */
	GENERAL_TrtcmRatelimitCfg_T rxRateLimitCfg = {0};
    GENERAL_TrtcmRatelimitSet_T rxRateLimitSet = {0};
    int tickerSel[AIROHA_NUM_RX_RING] = {TRTCM_SLOW_TICK, TRTCM_FAST_TICK, TRTCM_SLOW_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_SLOW_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK
                                , TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK, TRTCM_FAST_TICK};
    /* ring2 for DLF/icmp flow*/
     u32 rateVal[AIROHA_NUM_RX_RING] = {4000, 1000000, 4000, 4000, 4000, 1000000, 1000000, 4000,
                            200, 200, 1000000, 125000, 1000000, 1000000, 1000000, 1000000,
                            1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000,
                            1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000}; /* array length need adjust according to demand*/

	/* set default setting for global array */
	qdma_dev_trtcm_cfg_init();
    
    /*cfg glb setting*/
    rxRateLimitCfg.MeterEn = GENERAL_METER_ENABLE;
    rxRateLimitCfg.PktMode = TRTCM_PACKET_MODE;
    
    rxRateLimitCfg.trtcmModule = INGRESS_TRTCM;
    
    /*ratelimit glb setting*/
    rxRateLimitSet.trtcmModule = INGRESS_TRTCM;

    for(ringIdx=0; ringIdx<AIROHA_NUM_RX_RING; ringIdx++){
        /*cfg each ring*/
        rxRateLimitCfg.TickSel = tickerSel[ringIdx];
        rxRateLimitCfg.Index = ringIdx; 
        airoha_qdma_general_set_ratelimit_mode_cfg(qdma, rxRateLimitCfg);

        /*ratelimit each ring*/
        rxRateLimitSet.Index = ringIdx; 
        rxRateLimitSet.RateLimitValue = rateVal[ringIdx];
        airoha_qdma_general_set_ratelimit_mode_value(qdma, rxRateLimitSet);
    }
    generalSetTrtcmMode(qdma, INGRESS_TRTCM, TRTCM_RATELIMIT_MODE);
	
    return;

}

u32 arht_set_bucket_size(struct airoha_qdma *qdma, int channel, enum trtcm_mode_type mode, u32 bucket_size){

	u32 val, rl_mode_cfg, bucket_unit;
	int err;
	val = max_t(u32, bucket_size, MIN_TOKEN_SIZE);
	err = airoha_qdma_get_trtcm_param(qdma, channel, REG_EGRESS_TRTCM_CFG, TRTCM_MISC_MODE, mode, &rl_mode_cfg, NULL);
	if(err){
		val = min_t(u32, __fls(val), MAX_TOKEN_SIZE_OFFSET);
		return val;
	}

	if (TRTCM_PKT_MODE == ((rl_mode_cfg & TRTCM_PKT_MODE) >> 1))
	{
		bucket_unit = trtcmBucketPacketUnit[EGRESS_TRTCM];
	}else{
		bucket_unit = trtcmBucketByteUnit[EGRESS_TRTCM];
	}

	val = biSearchGetBucketSizeShift(val, 0, 17, bucket_unit);
	val = min_t(u32, val, MAX_TOKEN_SIZE_OFFSET);
	return val;

}



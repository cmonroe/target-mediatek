#ifndef __ARHT_QDMA_H_
#define __ARHT_QDMA_H_

#include "arht_hook/ecnt_hook_qdma_type.h"

#if defined(QDMA_LAN)
#define qdma_path "qdma_lan"
#else
#define qdma_path "qdma_wan"
#endif

#define CONFIG_DEBUG 1
#ifdef CONFIG_DEBUG
	static inline const char *get_basename(const char *path)
	{
		const char *basename = strrchr(path, '/');
		return basename ? basename + 1 : path;
	}
	#define QDMA_ERR(F, B...)			printk("%s: %s [%d]: " F, qdma_path, get_basename(__FILE__), __LINE__, ##B)
	#define QDMA_LOG(F, B...)			printk("%s: %s [%d]: " F, qdma_path, get_basename(__FILE__), __LINE__, ##B)
#else
	#define QDMA_ERR(F,B...)			
	#define QDMA_LOG(F,B...)			printk("%s: " F, qdma_path, ##B)
#endif

#define CONFIG_QDMA_CHANNEL	  32
#define CONFIG_QDMA_QUEUE	  8
#define QUEUE_ALL_NUM     (CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE)

#define GENERAL_INGRESS_INDEX_MAX               (127)   /*EN7580: 128 ratelimit. or 64 trtcm*/
#define GENERAL_INGRESS_INDEX_MAX_GRP1               (31)   /*grp1: 32 ratelimit. or 16 trtcm*/
#define GENERAL_INGRESS_INDEX_MAX_GRP2               (15)   /*grp2: 16 ratelimit. or 8 trtcm*/
#define GENERAL_SLA_INDEX_MAX                                   (31)

#define TRTCM_MODE_MASK			BIT(30)
#define TRTCM_PARAM_CFG(trtcm_base)                (trtcm_base+0x4)
#define TRTCM_DATA_LO(trtcm_base)                  (trtcm_base+0x8)
#define TRTCM_DATA_HI(trtcm_base)                  (trtcm_base+0xc)
#define TRTCM_PARA_METER_GROUP_SHIFT    		(26)
#define TRTCM_PARA_IDX_RATE_TYPE_SHIFT  		(16)
#define TRTCM_PARA_IDX_RATE_TYPE_MASK   		(1<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)  //pir&pbs or cir&cbs
#define TRTCM_PARA_IDX_INDEX_SHIFT              (17)
#define TRTCM_PARA_IDX_INDEX_MASK               (0x3F<<TRTCM_PARA_IDX_INDEX_SHIFT)       //7580
#define TRTCM_PARA_TYPE_SHIFT                   (28)
#define TRTCM_PARA_TYPE_MASK                    (0x3<<TRTCM_PARA_TYPE_SHIFT)
#define TRTCM_PARA_RWCMD                                (1<<31)
#define TRTCM_PARA_RWCMD_DONE                   (1<<30)
#define RATELIMIT_PARA_IDX_INDEX_SHIFT          (16)
#define RATELIMIT_PARA_IDX_INDEX_MASK           (0xFF<<RATELIMIT_PARA_IDX_INDEX_SHIFT)   //7581
#define RATELIMIT_PARA_RWCMD                            (1<<31)
#define RATELIMIT_PARA_RWCMD_DONE                       (1<<30)
#define RATELIMIT_PARA_TYPE_SHIFT                       (28)
#define RATELIMIT_PARA_TYPE_MASK                        (0x3<<RATELIMIT_PARA_TYPE_SHIFT)
#define TRTCM_FAST_TICK_SHIFT                           (0)
#define RATELIMIT_BYTE_MODE_BUCKET_SHIFT                (10)
#define RATELIMIT_PKT_MODE_BUCKET_SHIFT                 (0)
#define TRTCM_FAST_TICK_MASK                            (0xFFFF<<TRTCM_FAST_TICK_SHIFT)
#define TRTCM_SLOW_TICKRATIO_SHIFT                      (16)
#define TRTCM_SLOW_TICKRATIO_MASK                       (0x3FFF<<TRTCM_SLOW_TICKRATIO_SHIFT)
#define METER_1K        (1000)
#define METER_1M        (METER_1K<<10)
#define TRTCM_TOKEN_RATE_INTEGER_SHIFT          (6)
#define TRTCM_TOKEN_RATE_INTEGER_MASK           (0x3FFFF<<TRTCM_TOKEN_RATE_INTEGER_SHIFT)

#define TXQ_DIS_CFG_REG_NUM				8
#define TXQ_DIS_QUEUE_CLOSE_OFFSET(chnl)                    ((chnl)&0xFC)
#define TXQ_DIS_CHANNEL_SHIFT(chnl)                         (((chnl)&0x03)*8)
#define TXQ_DIS_CHANNEL_MASK(chnl)                          (0xFF<<TXQ_DIS_CHANNEL_SHIFT(chnl))

#define TRTCM_TOKEN_BUCKET_SIZE_MASK	GENMASK(5, 0)


int biSearchGetBucketSizeShift(uint value, uint lo, uint hi, uint unit);
int qdma_set_qdmalan_tx_ratelimit(QDMA_TxRateLimitSet_T *txRateLimitPtr);
int qdma_api_set_tx_qos_upstream(unsigned int mainType, QDMA_TxQosScheduler_T *pTxQos);
int qdma_get_tx_qos_upstream(unsigned int mainType, struct ECNT_QDMA_Data *qdma_data);
int qdma_api_get_tx_qos_upstream(unsigned int mainType, QDMA_TxQosScheduler_T *pTxQos);
int airoha_dp_api_qdma_set_meter_cfg(struct airoha_qdma *qdma, unsigned char group_id, uint meterIdx, uint ratelimit_value);
int airoha_dp_api_qdma_get_meter_value(struct airoha_qdma *qdma,uint meterIdx);
int airoha_dp_api_qdma_set_meter_value(struct airoha_qdma *qdma,uint meterIdx,uint value);

/* previous prototype for arht_ppe.c */
int airoha_ppe_clean_entry_by_gemport(unsigned int gemport_id);
#endif /* __ARHT_QDMA_H_ */
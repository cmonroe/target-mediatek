/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 AIROHA Inc
 * Author: Airoha Inc
 */

#ifndef __ARHT_LOOPBACK_H__
#define __ARHT_LOOPBACK_H__

#include <linux/skbuff.h>
#include <linux/types.h>

#include "airoha_eth.h"
#include "arht_dp_api.h"

/* Core loopback functions */
void skb_swap_mac_addr(struct sk_buff *skb);
int airoha_ppe_hwnat_loopback_bind(struct sk_buff *skb, struct port_info *pinfo,
				   unsigned int crsn, u8 fport);

/* Module initialization and cleanup */
void airoha_hwnat_loopback_init(void);
void airoha_hwnat_loopback_exit(void);

#endif /* __ARHT_LOOPBACK_H__ */
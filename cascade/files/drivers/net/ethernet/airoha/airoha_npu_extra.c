// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 AIROHA Inc
 */

#include <linux/devcoredump.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>
#include <linux/of_reserved_mem.h>
#include <linux/regmap.h>

#include "airoha_eth.h"
#include "airoha_npu.h"
#include "airoha_function.h"


void arht_npu_retry_send_msg(struct airoha_npu *npu, int ret,
					u32 val, dma_addr_t dma_addr, int size, int func_id)
{

	int retry_cnt = 3;
	int offset = 0;
	if (!ret && FIELD_GET(MBOX_MSG_STATUS, val) == NPU_MBOX_SUCCESS)
		return;

	while (retry_cnt) {
		regmap_write(npu->regmap, REG_CR_MBQ0_CTRL(0) + offset, dma_addr);
		regmap_write(npu->regmap, REG_CR_MBQ0_CTRL(1) + offset, size);
		val = FIELD_PREP(MBOX_MSG_FUNC_ID, func_id) | MBOX_MSG_WAIT_RSP;
		regmap_write(npu->regmap, REG_CR_MBQ0_CTRL(3) + offset, val);
		regmap_read(npu->regmap, REG_CR_MBQ0_CTRL(2) + offset, &val);
		regmap_write(npu->regmap, REG_CR_MBQ0_CTRL(2) + offset, val + 1);
		ret = regmap_read_poll_timeout_atomic(npu->regmap,
					      REG_CR_MBQ0_CTRL(3) + offset,
					      val, (val & MBOX_MSG_DONE),
					      100, 1000 * MSEC_PER_SEC);
		if (!ret && FIELD_GET(MBOX_MSG_STATUS, val) == NPU_MBOX_SUCCESS)
			break;
		retry_cnt--;

		}
	return;
}
static int airoha_npu_tr471_send_msg(struct airoha_npu *npu, int func_type, char *data)
{
	int err = 0;
	struct tr471_mbox_data *tr471_data;

	tr471_data = kzalloc(sizeof(struct tr471_mbox_data), GFP_ATOMIC);
	if (!tr471_data)
		return -ENOMEM;
	tr471_data->func_type = func_type;

	memcpy(tr471_data->private, data, sizeof(tr471_data->private));

	err = airoha_npu_send_msg(npu, NPU_FUNC_TR471, tr471_data,
				  sizeof(*tr471_data));
	kfree(tr471_data);
	return err;
}

static int airoha_npu_tr471_get_msg(struct airoha_npu *npu, int func_type, char *data)
{
	int err = 0;
	struct tr471_mbox_data *tr471_data;

	tr471_data = kzalloc(sizeof(struct tr471_mbox_data), GFP_ATOMIC);
	if (!tr471_data)
		return -ENOMEM;
	tr471_data->func_type = func_type;
	memcpy(tr471_data->private, data, sizeof(tr471_data->private));
	err = airoha_npu_send_msg(npu, NPU_FUNC_TR471, tr471_data,
				  sizeof(*tr471_data));
	memcpy(data, tr471_data->private, sizeof(tr471_data->private));
	kfree(tr471_data);
	return err;
}


static int airoha_npu_wlan_txrx_reg_addr_set(struct airoha_npu *npu,
					     int ifindex, u32 dir,
					     u32 in_counter_addr,
					     u32 out_status_addr,
					     u32 out_counter_addr)
{
	struct wlan_mbox_data *wlan_data;
	int err;

	wlan_data = kzalloc(sizeof(*wlan_data), GFP_ATOMIC);
	if (!wlan_data)
		return -ENOMEM;

	wlan_data->ifindex = ifindex;
	wlan_data->func_type = NPU_OP_SET;
	wlan_data->func_id = WLAN_FUNC_SET_WAIT_INODE_TXRX_REG_ADDR;
	wlan_data->txrx_addr.dir = dir;
	wlan_data->txrx_addr.in_counter_addr = in_counter_addr;
	wlan_data->txrx_addr.out_status_addr = out_status_addr;
	wlan_data->txrx_addr.out_counter_addr = out_counter_addr;

	err = airoha_npu_send_msg_need_retry(npu, NPU_FUNC_WIFI, wlan_data,
				  sizeof(*wlan_data));
	kfree(wlan_data);

	return err;
}
static int airoha_npu_wlan_dbg_counter_address_get(struct airoha_npu *npu,
					    int ifindex, void *data, int data_len, gfp_t gfp)

{
	return airoha_npu_wlan_msg_get(npu, ifindex,
				       WLAN_FUNC_GET_WAIT_DBG_COUNTER, data, data_len, gfp);
}
int airoha_npu_l4s_setup(struct airoha_npu *npu,
				  u32 set_cmd,
				  u32 set_water_mark)
{
	int err;
	struct ppe_mbox_data *ppe_data;

	if (!AIROHA_L4S_FLAG) /* l4s is disabled */
		return 0;

	ppe_data = kzalloc(sizeof(*ppe_data), GFP_ATOMIC);
	if (!ppe_data)
		return -ENOMEM;

	ppe_data->func_type = NPU_OP_SET;
	ppe_data->func_id = PPE_FUNC_SET_WAIT_L4S_SETUP;
	ppe_data->l4s_info.set_cmd = set_cmd;
	ppe_data->l4s_info.set_water_mark = set_water_mark;

	err = airoha_npu_send_msg(npu, NPU_FUNC_PPE, ppe_data,
				  sizeof(*ppe_data));
	kfree(ppe_data);

	return err;
}
EXPORT_SYMBOL_GPL(airoha_npu_l4s_setup);


int airoha_npu_extra_init(struct airoha_npu *npu)
{

	npu->ops.wlan_get_dbg_counter_address = airoha_npu_wlan_dbg_counter_address_get;
	npu->ops.wlan_set_txrx_reg_addr = airoha_npu_wlan_txrx_reg_addr_set;
	npu->ops.l4s_setup = airoha_npu_l4s_setup;
	npu->ops.tr471_send_msg = airoha_npu_tr471_send_msg;
	npu->ops.tr471_get_msg = airoha_npu_tr471_get_msg;
	return 0;
}
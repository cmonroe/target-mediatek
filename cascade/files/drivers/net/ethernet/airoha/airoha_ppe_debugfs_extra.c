// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 AIROHA Inc
 * Author: Lorenzo Bianconi <lorenzo@kernel.org>
 */

#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include "airoha_eth.h"
#include "airoha_regs.h"
#include "airoha_function.h"

int airoha_debug_level = AIROHA_DEBUG_LEVEL_NONE;
int airoha_l4s_flag = AIROHA_L4S_DISABLE;


static ssize_t airoha_debug_level_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char level_str[12];
    int len = snprintf(level_str, sizeof(level_str), "%d\n", airoha_debug_level);
    return simple_read_from_buffer(buf, count, ppos, level_str, len);
}

static ssize_t airoha_debug_level_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char level_str[12];
    int level;

    if (count >= sizeof(level_str))
        return -EINVAL;

    if (copy_from_user(level_str, buf, count))
        return -EFAULT;

    level_str[count] = '\0';
    if (kstrtoint(level_str, 10, &level))
        return -EINVAL;

    if (level < AIROHA_DEBUG_LEVEL_NONE || level > AIROHA_DEBUG_LEVEL_DBG)
        return -EINVAL;

    airoha_debug_level = level;
    return count;
}

// File operations for the fast path enable control
static ssize_t airoha_fast_bit_enable_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char enable_str[100];
	struct airoha_eth *eth = glb_eth;
	if (IS_ERR_OR_NULL(eth)) {
		return PTR_ERR(eth);
	}
    int len = snprintf(enable_str, sizeof(enable_str), "wan_fastpath %u\nlan_fastpath %u\nxsi_ether_fastpath %u\n",
						eth->qdma_init.wan_fastpath, eth->qdma_init.lan_fastpath,
						eth->qdma_init.xsi_ether_fastpath);
    return simple_read_from_buffer(buf, count, ppos, enable_str, len);
}

static ssize_t airoha_fast_bit_enable_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char input_str[64];
	struct airoha_eth *eth = glb_eth;
	u32 val = 0;

	if (IS_ERR_OR_NULL(eth)) {
		return PTR_ERR(eth);
	}
    if (count >= sizeof(input_str))
		return -EINVAL;

    if (copy_from_user(input_str, buf, count))
		return -EFAULT;

    input_str[count] = '\0';

	/* wan_fastpath [0/1 - hwnat fastpath to GDM2]
	 * lan_fastpath [0/1 -  hwnat fastpath to GDM1]
	 * xsi_ether_fastpath [0/1 - hwnat fastpath to HSGMII/XFI]
	 */
	 if (strncmp(input_str, "wan_fastpath", 12) == 0) {
	 if (sscanf(input_str, "wan_fastpath %u", &val) == 1) {
	 	if (val > 1)
	 		return -EINVAL;
	 	eth->qdma_init.wan_fastpath = val;
	 } else {
	 	return -EINVAL;
	 }
	 } else if (strncmp(input_str, "lan_fastpath", 12) == 0) {
	 if (sscanf(input_str, "lan_fastpath %u", &val) == 1) {
	 	if (val > 1)
	 		return -EINVAL;
	 	eth->qdma_init.lan_fastpath = val;
	 } else {
	 	return -EINVAL;
	 }
	 } else if (strncmp(input_str, "xsi_ether_fastpath", 18) == 0) {
	 if (sscanf(input_str, "xsi_ether_fastpath %u", &val) == 1) {
	 	if (val > 1)
	 		return -EINVAL;
	 	eth->qdma_init.xsi_ether_fastpath = val;
	 } else {
	 	return -EINVAL;
	 }
	 } else if (strncmp(input_str, "help", 4) == 0) {
	 pr_info("Usage echo [Parameter] [Value] > /sys/kernel/debug/airoha_ppe/fast_and_slow_path\n"
	 "Parameter:\n" "wan_fastpath [0/1 - hwnat fastpath to GDM2]\n"
	 "lan_fastpath [0/1 - hwnat fastpath to GDM1]\n"
	 "xsi_ether_fastpath [0/1 - hwnat fastpath to HSGMII/XFI]\n");
	 } else {
	 return -EINVAL;
	 }

    return count;
}
static ssize_t airoha_bind_age_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char input_str[64];
	unsigned int tcp, udp, tcp_fin, non_l4, i;
	struct airoha_eth *eth = glb_eth;

	if (IS_ERR_OR_NULL(eth))
		return -EINVAL;

	if (count >= sizeof(input_str))
		return -EINVAL;
	if (copy_from_user(input_str, buf, count))
		return -EFAULT;
	input_str[count] = '\0';
	if (sscanf(input_str, "%u %u %u %u", &tcp, &udp, &tcp_fin, &non_l4) != 4)
		return -EINVAL;

	for (i = 0; i < eth->soc->num_ppe; i++) {
		airoha_fe_rmw(eth, REG_PPE_BND_AGE0(i),
							PPE_BIND_AGE0_DELTA_NON_L4 |
							PPE_BIND_AGE0_DELTA_UDP,
							FIELD_PREP(PPE_BIND_AGE0_DELTA_NON_L4, non_l4) |
							FIELD_PREP(PPE_BIND_AGE0_DELTA_UDP, udp));
		airoha_fe_rmw(eth, REG_PPE_BND_AGE1(i),
							PPE_BIND_AGE1_DELTA_TCP_FIN |
							PPE_BIND_AGE1_DELTA_TCP,
							FIELD_PREP(PPE_BIND_AGE1_DELTA_TCP_FIN, tcp_fin) |
							FIELD_PREP(PPE_BIND_AGE1_DELTA_TCP, tcp));
	}

	return count;
}
#ifdef CONFIG_NET_AIROHA_L4S
static ssize_t airoha_l4s_control_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char l4s_str[256];
    int len;
	switch (airoha_l4s_flag) {
	case AIROHA_L4S_DISABLE:
		len = snprintf(l4s_str, sizeof(l4s_str), "AIROHA_L4S_DISABLE\n");
		break;
	case AIROHA_L4S_ENABLE:
		len = snprintf(l4s_str, sizeof(l4s_str), "AIROHA_L4S_ENABLE\n");
		break;
	case AIROHA_L4S_DEBUG:
		len = snprintf(l4s_str, sizeof(l4s_str), "AIROHA_L4S_DEBUG\n");
		break;
	default:
		len = snprintf(l4s_str, sizeof(l4s_str), "UNKNOWN\n");
		break;
	}

    return simple_read_from_buffer(buf, count, ppos, l4s_str, len);
}

static ssize_t airoha_l4s_control_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char l4s_str[256];
    char cmd[32] = {0};
    u32 set_cmd = 0;
    u32 set_water_mark = 0;
    struct airoha_eth *eth = glb_eth;
    struct airoha_npu *npu = NULL;
    int ret = 0;

    // 检查 eth 是否有效
    if (!eth) {
        printk(KERN_ALERT "airoha_l4s_control_write: glb_eth is NULL\n");
        return -EINVAL;
    }

    // 检查 count 合法性
    if (count >= sizeof(l4s_str)) {
        printk(KERN_ALERT "airoha_l4s_control_write: input too long\n");
        return -EINVAL;
    }

    // 拷贝用户数据
    if (copy_from_user(l4s_str, buf, count)) {
        printk(KERN_ALERT "airoha_l4s_control_write: copy_from_user failed\n");
        return -EFAULT;
    }
    l4s_str[count] = '\0';

    // 解析命令
    ret = sscanf(l4s_str, "%31s %u", cmd, &set_water_mark);

    if (!strcmp(cmd, "disable")) {
        airoha_l4s_flag = AIROHA_L4S_DISABLE;
        set_cmd = AIROHA_L4S_DISABLE;
        printk(KERN_ALERT "Disable L4S during traffic flow will force the clearing of HWNAT entries and re-learning!!!\n");
    } else if (!strcmp(cmd, "enable")) {
        airoha_l4s_flag = AIROHA_L4S_ENABLE;
        set_cmd = AIROHA_L4S_ENABLE;
        printk(KERN_ALERT "Enable L4S during traffic flow requires manual clearing of HWNAT entries and re-learning for immediate effect!!!\n");
    } else if (!strcmp(cmd, "debug")) {
        airoha_l4s_flag = AIROHA_L4S_DEBUG;
        set_cmd = AIROHA_L4S_DEBUG;
    } else if (!strcmp(cmd, "set_qid")) {
        if (ret != 2) {
            printk(KERN_ALERT "set_qid requires a value (0~7)\n");
            return -EINVAL;
        }
        set_cmd = AIROHA_L4S_SETQID;
        // 可根据需要检查 set_water_mark 范围
    } else if (!strcmp(cmd, "qlen")) {
        if (ret != 2) {
            printk(KERN_ALERT "qlen requires a value\n");
            return -EINVAL;
        }
        set_cmd = AIROHA_L4S_QDMA_WATER_MARK;
    } else if (!strcmp(cmd, "ai")) {
        if (ret != 2) {
            printk(KERN_ALERT "ai requires a value (0/1)\n");
            return -EINVAL;
        }
        if (airoha_l4s_flag || set_water_mark == 0) {
            set_cmd = AIROHA_L4S_AI;
        } else {
            printk(KERN_ALERT "Enable AI requires L4S enabled!!!\n");
            return -EFAULT;
        }
    } else {
        printk(KERN_ALERT "usage: echo enable/disable/debug/set_qid(0~7)/qlen(<len>)/ai(0/1) >/sys/kernel/debug/airoha-eth/ppe/l4s\n");
        return -EINVAL;
    }

    // 获取 npu
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0)
    npu = airoha_npu_get(eth->dev, &eth->ppe->foe_stats_dma);
#else
    npu = airoha_npu_get(eth->dev);
#endif
    if (!npu || IS_ERR(npu)) {
        request_module("airoha-npu");
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0)
        npu = airoha_npu_get(eth->dev, &eth->ppe->foe_stats_dma);
#else
        npu = airoha_npu_get(eth->dev);
#endif
        if (!npu || IS_ERR(npu)) {
            printk(KERN_ALERT "airoha_l4s_control_write: npu is NULL or error\n");
            return -EINVAL;
        }
    }

    // 检查 ops 和 l4s_setup
    if (!npu->ops.l4s_setup) {
        printk(KERN_ALERT "airoha_l4s_control_write: npu->ops.l4s_setup is NULL\n");
        return -EINVAL;
    }

    npu->ops.l4s_setup(npu, set_cmd, set_water_mark);
    return count;
}

static const struct file_operations airoha_l4s_control_fops = {
    .read = airoha_l4s_control_read,
    .write = airoha_l4s_control_write,
};
#endif

static const struct file_operations airoha_debug_level_fops = {
    .read = airoha_debug_level_read,
    .write = airoha_debug_level_write,
};

static const struct file_operations airoha_fast_and_slow_path_fops = {
    .read = airoha_fast_bit_enable_read,
    .write = airoha_fast_bit_enable_write,
};

static const struct file_operations airoha_bind_age_fops = {
    .write = airoha_bind_age_write,
};

void airoha_ppe_debugfs_extra_create(struct dentry *root)
{
	debugfs_create_file("debug_level", 0644, root, NULL,
				&airoha_debug_level_fops);
	debugfs_create_file("fast_and_slow_path", 0644, root, NULL,
				&airoha_fast_and_slow_path_fops);
	debugfs_create_file("bind_age", 0644, root, NULL,
				&airoha_bind_age_fops);
	#ifdef CONFIG_NET_AIROHA_L4S
	debugfs_create_file("l4s", 0644, root, NULL,
				&airoha_l4s_control_fops);
	#endif
	return;
}

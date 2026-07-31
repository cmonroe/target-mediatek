// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 AIROHA Inc
 * Author: Airoha Inc
 */

#include <linux/etherdevice.h>
#include <linux/if_pppox.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/uaccess.h>

#include "airoha_eth.h"
#include "airoha_regs.h"
#include "arht_loopback.h"
#include "arht_ppe.h"

/* Global configuration flags */
static int hwnat_loopback_enable;
static int hwnat_loopback_mac_swap;

/* L2B ether type index tracking for mac_swap feature */
static int l2b_ipv4_etype_index = L2B_ETYPE_INVALID_INDEX;
static int l2b_ipv6_etype_index = L2B_ETYPE_INVALID_INDEX;
static int l2b_pppoe_ipv4_etype_index = L2B_ETYPE_INVALID_INDEX;
static int l2b_pppoe_ipv6_etype_index = L2B_ETYPE_INVALID_INDEX;

/* External dependencies */
extern spinlock_t ppe_lock;
extern struct airoha_eth *glb_eth;
extern u8 get_dscp_from_skb(struct sk_buff *skb, int type);
extern void airoha_set_default_acnt_meter_idx(struct airoha_foe_entry *hwe, int type);
extern int (*hwnat_loopback_hook)(struct sk_buff *skb, struct port_info *pinfo,
				  unsigned int crsn, u8 fport);
extern int (*airoha_ppe_foe_commit_entry_ptr)(struct airoha_ppe *ppe,
					      struct airoha_foe_entry *e,
					      u32 hash, bool rx_wlan);

/* Proc filesystem entry */
static struct proc_dir_entry *hwnat_loopback_proc_entry;

/* Forward declarations for proc operations */
static ssize_t airoha_hwnat_loopback_read(struct file *file, char __user *buf,
					  size_t count, loff_t *ppos);
static ssize_t airoha_hwnat_loopback_write(struct file *file,
					   const char __user *buf,
					   size_t count, loff_t *ppos);

static const struct proc_ops airoha_hwnat_loopback_fops = {
	.proc_read = airoha_hwnat_loopback_read,
	.proc_write = airoha_hwnat_loopback_write,
};

/**
 * skb_swap_mac_addr() - Swap source and destination MAC addresses in skb
 * @skb: Socket buffer containing the Ethernet frame
 *
 * This function swaps the source and destination MAC addresses in the
 * Ethernet header of the provided socket buffer.
 */
void skb_swap_mac_addr(struct sk_buff *skb)
{
	u8 tmp_mac[ETH_ALEN];

	ether_addr_copy(tmp_mac, skb->data);
	ether_addr_copy(skb->data, skb->data + ETH_ALEN);
	ether_addr_copy(skb->data + ETH_ALEN, tmp_mac);
}

/* Default meter indices for loopback entries */
#define LOOPBACK_METER0_IDX	0x7f
#define LOOPBACK_METER1_IDX	0x1f

/**
 * airoha_loopback_get_pppoe_sid() - Get PPPoE Session ID from skb
 * @skb: Socket buffer containing the Ethernet frame
 * @vlan_num: Number of VLAN tags in the packet
 * @pppid: Pointer to store the PPPoE Session ID
 *
 * This function extracts the PPPoE Session ID directly from the packet data
 * by calculating the correct offset based on VLAN tags. Unlike pppoe_hdr()
 * which relies on skb->network_header, this function uses skb->data to
 * ensure correct PPPoE header location.
 *
 * PPPoE packet structure:
 * [Ethernet Header 14B][VLAN(s) vlan_num*4B][PPPoE Header 6B][PPP Protocol 2B][Payload]
 *
 * PPPoE Header structure (6 bytes):
 * - ver/type: 1 byte
 * - code: 1 byte
 * - sid: 2 bytes (Session ID)
 * - length: 2 bytes
 */
static void airoha_loopback_get_pppoe_sid(struct sk_buff *skb, u16 vlan_num,
					  u16 *pppid)
{
	u16 etype;
	unsigned int pppoe_offset;

	*pppid = 0;

	/* Calculate offset to EtherType field after VLAN tags */
	/* ETH_ALEN * 2 (12 bytes for dst+src MAC) + vlan_num * 4 (VLAN tags) */
	etype = *(u16 *)(skb->data + ETH_ALEN * 2 + vlan_num * VLAN_HLEN);

	/* Check if this is a PPPoE Session packet (0x8864) */
	if (etype == htons(ETH_P_PPP_SES)) {
		struct pppoe_hdr *ppph;

		/* PPPoE header starts after: ETH_HLEN (14) + VLAN tags + EtherType (2) */
		pppoe_offset = ETH_HLEN + vlan_num * VLAN_HLEN;
		ppph = (struct pppoe_hdr *)(skb->data + pppoe_offset);

		*pppid = ntohs(ppph->sid);
	}
}

static u32 airoha_ppe_loopback_get_timestamp(struct airoha_ppe *ppe)
{
	u16 timestamp = airoha_fe_rr(ppe->eth, REG_FE_FOE_TS);

	return FIELD_GET(AIROHA_FOE_IB1_BIND_TIMESTAMP, timestamp);
}

static bool airoha_loopback_is_bound_or_not_ready(struct airoha_foe_entry *hwe,
						  unsigned int crsn)
{
	u32 state = FIELD_GET(AIROHA_FOE_IB1_BIND_STATE, hwe->ib1);

	return state == AIROHA_FOE_STATE_BIND ||
	       crsn != PPE_CPU_REASON_HIT_UNBIND_RATE_REACHED;
}

static u32 airoha_loopback_prepare_ib1(u32 ib1, u16 vn, u16 vpm, u16 pppoe, u32 ts)
{
	ib1 &= ~(AIROHA_FOE_IB1_BIND_VLAN_LAYER | AIROHA_FOE_IB1_BIND_VPM |
		 AIROHA_FOE_IB1_BIND_PPPOE | AIROHA_FOE_IB1_BIND_TIMESTAMP |
		 AIROHA_FOE_IB1_BIND_STATE);

	return ib1 |
	       FIELD_PREP(AIROHA_FOE_IB1_BIND_VLAN_LAYER, vn) |
	       FIELD_PREP(AIROHA_FOE_IB1_BIND_VPM, vpm) |
	       FIELD_PREP(AIROHA_FOE_IB1_BIND_PPPOE, !!pppoe) |
	       FIELD_PREP(AIROHA_FOE_IB1_BIND_TIMESTAMP, ts) |
	       FIELD_PREP(AIROHA_FOE_IB1_BIND_STATE, AIROHA_FOE_STATE_BIND);
}

static u32 airoha_loopback_prepare_data(struct port_info *pinfo)
{
	return FIELD_PREP(AIROHA_FOE_ACTDP, pinfo->udf) |
	       FIELD_PREP(AIROHA_FOE_CHANNEL, pinfo->channel) |
	       FIELD_PREP(AIROHA_FOE_QID,
			  (AIROHA_NUM_QOS_QUEUES - 1) -
			  (pinfo->txq % AIROHA_NUM_QOS_QUEUES)) |
	       FIELD_PREP(AIROHA_FOE_SHAPER_ID, LOOPBACK_METER0_IDX);
}

static u32 airoha_loopback_prepare_ib2(struct airoha_ppe *ppe,
				       struct port_info *pinfo, u8 fport)
{
	u32 ib2;
	bool fast = false;

	ib2 = FIELD_PREP(AIROHA_FOE_IB2_NBQ, pinfo->nbq) |
	      FIELD_PREP(AIROHA_FOE_IB2_PSE_PORT, fport) |
	      AIROHA_FOE_IB2_PSE_QOS;

	/* Determine fast path based on fport or pinfo->fast */
	if (pinfo->fast) {
		fast = true;
	} else {
		switch (fport) {
		case FE_PSE_PORT_GDM1:
			if (ppe->eth->qdma_init.lan_fastpath)
				fast = true;
			break;
		case FE_PSE_PORT_GDM2:
			if (ppe->eth->qdma_init.wan_fastpath)
				fast = true;
			break;
		case FE_PSE_PORT_GDM3:
		case FE_PSE_PORT_GDM4:
			if (ppe->eth->qdma_init.xsi_ether_fastpath)
				fast = true;
			break;
		default:
			break;
		}
	}

	if (fast)
		ib2 |= AIROHA_FOE_IB2_FAST_PATH;

	return ib2;
}

static struct airoha_foe_mac_info_common *
airoha_loopback_configure_entry(struct airoha_foe_entry *hwe, int type,
				u32 data, u32 ib2, int dscp)
{
	struct airoha_foe_mac_info_common *l2 = NULL;
	u32 meter_grp2 = FIELD_PREP(AIROHA_FOE_METER_GRP2, LOOPBACK_METER1_IDX);

	switch (type) {
	case PPE_PKT_TYPE_IPV4_HNAPT:
		hwe->ipv4.new_tuple.src_port = hwe->ipv4.orig_tuple.src_port;
		hwe->ipv4.new_tuple.dest_port = hwe->ipv4.orig_tuple.dest_port;
		fallthrough;
	case PPE_PKT_TYPE_IPV4_ROUTE:
		hwe->ipv4.data = data;
		hwe->ipv4.ib2 = ib2 | FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);
		hwe->ipv4.new_tuple.src_ip = hwe->ipv4.orig_tuple.src_ip;
		hwe->ipv4.new_tuple.dest_ip = hwe->ipv4.orig_tuple.dest_ip;
		hwe->ipv4.l2.meter |= meter_grp2;
		l2 = &hwe->ipv4.l2.common;
		break;
	case PPE_PKT_TYPE_BRIDGE:
		hwe->bridge32.data = data;
		hwe->bridge32.ib2 = ib2;
		hwe->bridge32.meter |= meter_grp2;
		l2 = &hwe->bridge32.l2;
		break;
	case PPE_PKT_TYPE_IPV6_ROUTE_3T:
	case PPE_PKT_TYPE_IPV6_ROUTE_5T:
		hwe->ipv6.data = data;
		hwe->ipv6.ib2 = ib2 | FIELD_PREP(AIROHA_FOE_IB2_DSCP, dscp);
		hwe->ipv6.meter |= meter_grp2;
		l2 = &hwe->ipv6.l2;
		break;
	}

	return l2;
}

/**
 * airoha_ppe_hwnat_loopback_bind() - Bind packet to hardware NAT for loopback
 * @skb: Socket buffer containing the packet
 * @pinfo: Port information structure containing hardware info (channel, nbq, sptag, etc.)
 * @crsn: CPU reason code
 * @fport: Forward port number
 *
 * This function binds a packet to the hardware NAT flow offload engine (FOE)
 * for loopback processing, configuring VLAN, QoS, MAC address handling, and
 * packet type-specific fields (IPv4/IPv6/Bridge).
 *
 * Note: The caller must fill in hardware-specific information in the function
 * parameters, including fport, and the following fields in pinfo structure:
 * channel, nbq, sptag, etc.
 *
 * Return: 0 on success, negative error code on failure
 */
int airoha_ppe_hwnat_loopback_bind(struct sk_buff *skb, struct port_info *pinfo,
				   unsigned int crsn, u8 fport)
{
	struct airoha_foe_mac_info_common *l2;
	u16 vn = 0, vid1 = 0, vid2 = 0, vpm = 0, pppid = 0;
	struct airoha_foe_entry *hwe;
	struct airoha_ppe *ppe;
	u32 data, ib1, ib2;
	int ret = 0;
	u32 hash;
	int type;
	int dscp;
	u32 ts;

	if (!hwnat_loopback_enable || !is_Valid_Foe_Entry(skb))
		return -EINVAL;

	ppe = glb_eth->ppe;
	hash = FOE_ENTRY_NUM(skb);

	spin_lock_bh(&ppe_lock);

	hwe = airoha_ppe_foe_get_entry_locked(ppe, hash);
	if (!hwe) {
		ret = -ENOENT;
		goto unlock;
	}

	/* If already bound or not hit unbind rate, just swap MAC if needed */
	if (airoha_loopback_is_bound_or_not_ready(hwe, crsn)) {
		if (hwnat_loopback_mac_swap)
			skb_swap_mac_addr(skb);
		goto unlock;
	}

	if (airoha_ppe_foe_get_vlan_info(skb, &vn, &vid1, &vid2)) {
		ret = -EINVAL;
		goto unlock;
	}

	airoha_ppe_foe_get_vlan_vpm(skb, &vpm);
	airoha_loopback_get_pppoe_sid(skb, vn, &pppid);

	type = FIELD_GET(AIROHA_FOE_IB1_BIND_PACKET_TYPE, hwe->ib1);
	airoha_set_default_acnt_meter_idx(hwe, type);
	dscp = get_dscp_from_skb(skb, type);
	ts = airoha_ppe_loopback_get_timestamp(ppe);

	data = airoha_loopback_prepare_data(pinfo);
	ib1 = airoha_loopback_prepare_ib1(hwe->ib1, vn, vpm, pppid, ts);
	ib2 = airoha_loopback_prepare_ib2(ppe, pinfo, fport);

	l2 = airoha_loopback_configure_entry(hwe, type, data, ib2, dscp);
	if (!l2) {
		ret = -EINVAL;
		goto unlock;
	}

	l2->etype = pinfo->stag;
	l2->vlan1 = vid1;
	l2->vlan2 = vid2;

	if (hwnat_loopback_mac_swap && type != PPE_PKT_TYPE_BRIDGE)
		skb_swap_mac_addr(skb);

	/*
	 * For loopback with mac_swap, pass dev=NULL to airoha_set_ppe_mac()
	 * to force using shrink table for IPv6 src_mac setting.
	 * This avoids the "keep smac" logic in airoha_is_bridge_packet()
	 * which would incorrectly keep the original src_mac.
	 */
	airoha_set_ppe_mac(hwe, hwnat_loopback_mac_swap ? NULL : skb->dev,
			   skb->data + ETH_ALEN, skb->data, pppid);

	hwe->ib1 = ib1;

	if (hash < ppe->eth->soc->ppe_sram_etry_num)
		airoha_ppe_foe_commit_entry_ptr(ppe, hwe, hash, 0);

unlock:
	spin_unlock_bh(&ppe_lock);
	return ret;
}

/**
 * hwnat_loopback_mac_swap_enable() - Enable MAC swap L2B blacklist configuration
 *
 * Adds IPv4 (0x0800), IPv6 (0x86dd), PPPoE IPv4 (0x0021), and PPPoE IPv6 (0x0057)
 * to L2B blacklist to force these packets to use 3-tuple/5-tuple HWNAT rules
 * instead of L2B rules, enabling MAC swapping.
 *
 * Return: 0 on success, negative error code on failure
 */
static int hwnat_loopback_mac_swap_enable(void)
{
	bool ipv4_existed = false, ipv6_existed = false;
	bool pppoe_ipv4_existed = false, pppoe_ipv6_existed = false;
	int ret;

	/* Configure IPv4 ether type (0x0800) */
	ret = ppe_set_l2b_ether_type_auto(0x0800, 0, &ipv4_existed);
	if (ret < 0)
		return ret;
	if (!ipv4_existed)
		l2b_ipv4_etype_index = ret;

	/* Configure IPv6 ether type (0x86dd) */
	ret = ppe_set_l2b_ether_type_auto(0x86dd, 0, &ipv6_existed);
	if (ret < 0)
		goto err_ipv6;
	if (!ipv6_existed)
		l2b_ipv6_etype_index = ret;

	/* Configure PPPoE IPv4 protocol (0x0021) */
	ret = ppe_set_l2b_ether_type_auto(0x0021, 1, &pppoe_ipv4_existed);
	if (ret < 0)
		goto err_pppoe_ipv4;
	if (!pppoe_ipv4_existed)
		l2b_pppoe_ipv4_etype_index = ret;

	/* Configure PPPoE IPv6 protocol (0x0057) */
	ret = ppe_set_l2b_ether_type_auto(0x0057, 1, &pppoe_ipv6_existed);
	if (ret < 0)
		goto err_pppoe_ipv6;
	if (!pppoe_ipv6_existed)
		l2b_pppoe_ipv6_etype_index = ret;

	hwnat_loopback_mac_swap = 1;
	pr_info("hwnat_loopback_mac_swap enabled\n");
	pr_info("Note: Packets with etype 0x0800/0x86dd/PPPoE(0x0021/0x0057) will use 3-tuple/5-tuple acceleration rules\n");

	return 0;

err_pppoe_ipv6:
	if (!pppoe_ipv4_existed) {
		ppe_clear_l2b_ether_type_auto(0x0021, 1);
		l2b_pppoe_ipv4_etype_index = L2B_ETYPE_INVALID_INDEX;
	}
err_pppoe_ipv4:
	if (!ipv6_existed) {
		ppe_clear_l2b_ether_type_auto(0x86dd, 0);
		l2b_ipv6_etype_index = L2B_ETYPE_INVALID_INDEX;
	}
err_ipv6:
	if (!ipv4_existed) {
		ppe_clear_l2b_ether_type_auto(0x0800, 0);
		l2b_ipv4_etype_index = L2B_ETYPE_INVALID_INDEX;
	}
	return ret;
}

/**
 * hwnat_loopback_mac_swap_disable() - Disable MAC swap L2B blacklist configuration
 *
 * Clears the L2B blacklist entries that were configured during enable.
 * Pre-existing entries (not configured by this module) are not cleared.
 */
static void hwnat_loopback_mac_swap_disable(void)
{
	/* Only clear entries that we configured (not pre-existing) */
	if (l2b_ipv4_etype_index != L2B_ETYPE_INVALID_INDEX) {
		ppe_clear_l2b_ether_type_auto(0x0800, 0);
		l2b_ipv4_etype_index = L2B_ETYPE_INVALID_INDEX;
	}

	if (l2b_ipv6_etype_index != L2B_ETYPE_INVALID_INDEX) {
		ppe_clear_l2b_ether_type_auto(0x86dd, 0);
		l2b_ipv6_etype_index = L2B_ETYPE_INVALID_INDEX;
	}

	if (l2b_pppoe_ipv4_etype_index != L2B_ETYPE_INVALID_INDEX) {
		ppe_clear_l2b_ether_type_auto(0x0021, 1);
		l2b_pppoe_ipv4_etype_index = L2B_ETYPE_INVALID_INDEX;
	}

	if (l2b_pppoe_ipv6_etype_index != L2B_ETYPE_INVALID_INDEX) {
		ppe_clear_l2b_ether_type_auto(0x0057, 1);
		l2b_pppoe_ipv6_etype_index = L2B_ETYPE_INVALID_INDEX;
	}

	hwnat_loopback_mac_swap = 0;
	pr_info("hwnat_loopback_mac_swap disabled\n");
}

/**
	* airoha_hwnat_loopback_write() - Write handler for loopback proc interface
	* @file: File pointer
	* @buf: User buffer containing the command
	* @count: Number of bytes to write
	* @ppos: File position pointer
	*
	* Supported commands:
	*   echo enable [1|0]    - Enable/disable loopback functionality
	*   echo mac_swap [1|0]  - Enable/disable MAC address swapping
	*
	* Return: Number of bytes written on success, negative error code on failure
	*/
static ssize_t airoha_hwnat_loopback_write(struct file *file,
					   const char __user *buf,
					   size_t count, loff_t *ppos)
{
	char input_str[64];
	char cmd[32];
	int val, ret;
	char *ptr, *token;

	if (count >= sizeof(input_str))
		return -EINVAL;

	if (copy_from_user(input_str, buf, count))
		return -EFAULT;

	input_str[count] = '\0';
	ptr = input_str;
	token = strsep(&ptr, " \t");
	if (!token || !ptr || strscpy(cmd, token, sizeof(cmd)) < 0 || kstrtoint(strim(ptr), 10, &val)) 
	{
		pr_info("Usage: echo [enable|mac_swap] [0|1]\n");
		return -EINVAL;
	}

	if (!strcmp(cmd, "enable")) {
		hwnat_loopback_enable = !!val;
		pr_info("hwnat_loopback_enable: %d\n", hwnat_loopback_enable);
	} else if (!strcmp(cmd, "mac_swap")) {
		if (val && !hwnat_loopback_mac_swap) {
			ret = hwnat_loopback_mac_swap_enable();
			if (ret)
				return ret;
		} else if (!val && hwnat_loopback_mac_swap) {
			hwnat_loopback_mac_swap_disable();
		} else {
			pr_info("hwnat_loopback_mac_swap: %d (unchanged)\n",
				hwnat_loopback_mac_swap);
		}
	} else {
		pr_info("Usage: echo [enable|mac_swap] [0|1]\n");
		return -EINVAL;
	}

	return count;
}

/**
 * airoha_hwnat_loopback_read() - Read handler for loopback proc interface
 * @file: File pointer
 * @buf: User buffer to write data to
 * @count: Maximum number of bytes to read
 * @ppos: File position pointer
 *
 * Displays current loopback configuration settings.
 *
 * Return: Number of bytes read on success, negative error code on failure
 */
static ssize_t airoha_hwnat_loopback_read(struct file *file, char __user *buf,
					  size_t count, loff_t *ppos)
{
	ssize_t buf_size = 128;
	char *buffer;
	ssize_t ret;
	int len = 0;

	buffer = kmalloc(buf_size, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	len += scnprintf(buffer + len, buf_size - len,
			 "hwnat_loopback_enable: %d\n", hwnat_loopback_enable);
	len += scnprintf(buffer + len, buf_size - len,
			 "hwnat_loopback_mac_swap: %d\n",
			 hwnat_loopback_mac_swap);

	ret = simple_read_from_buffer(buf, count, ppos, buffer, len);

	kfree(buffer);

	return ret;
}

/**
 * airoha_hwnat_loopback_init() - Initialize loopback module
 *
 * Creates the proc interface and registers the loopback hook function.
 * This function is called during PPE initialization.
 */
void airoha_hwnat_loopback_init(void)
{
	/* Create proc interface for runtime configuration */
	hwnat_loopback_proc_entry = proc_create("hwnat_loopback", 0644, NULL,
						&airoha_hwnat_loopback_fops);
	if (!hwnat_loopback_proc_entry)
		pr_err("Failed to create hwnat_loopback proc entry\n");

	/* Register loopback hook function */
	rcu_assign_pointer(hwnat_loopback_hook, airoha_ppe_hwnat_loopback_bind);
}

/**
 * airoha_hwnat_loopback_exit() - Cleanup loopback module
 *
 * Removes the proc interface and unregisters the loopback hook function.
 * This function is called during PPE cleanup.
 */
void airoha_hwnat_loopback_exit(void)
{
	/* Unregister loopback hook function */
	rcu_assign_pointer(hwnat_loopback_hook, NULL);

	/* Remove proc interface */
	if (hwnat_loopback_proc_entry) {
		proc_remove(hwnat_loopback_proc_entry);
		hwnat_loopback_proc_entry = NULL;
	}
}
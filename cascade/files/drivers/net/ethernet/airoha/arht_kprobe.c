// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 AIROHA Inc
 * All Rights Reserved.
 *
 * All kprobe registrations for Dynamic IFC and skip_copy feature.
 */

#include <linux/module.h>
#include <linux/kernel.h>

#if __has_include("airoha_function.h") && __has_include("arht_dynamic_ifc.h") && __has_include("arht_kprobe.h")
#define ARHT_KPROBE_FULL_SUPPORT 1
#else
#define ARHT_KPROBE_FULL_SUPPORT 0
#endif

#if ARHT_KPROBE_FULL_SUPPORT
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <linux/etherdevice.h> 

#include <net/ip.h>
#include <net/ipv6.h>
#include <net/inet_sock.h>
#include <net/tcp.h>
#include <net/sock.h>
#include <net/protocol.h>
#include <net/dst.h>

#include "airoha_regs.h"
#include "airoha_eth.h"
#include "airoha_function.h"
#include "arht_dynamic_ifc.h"
#include "arht_kprobe.h"


#define SKIP_COPY_MIN_BYTES	 1024

/************************************************************************************************
*				   E X T E R N A L     D A T A	 D E C L A R A T I O N S
*************************************************************************************************
*/
extern struct airoha_eth *glb_eth;

/************************************************************************************************
*				   E X T E R N A L     F U N C T I O N	 D E C L A R A T I O N S
*************************************************************************************************
*/
extern int (*offload_eth_fast_tx_hook)(struct sk_buff *skb, int channel);


static struct kprobe kp_tcp_recvmsg;
static struct kprobe kp_tcp_sendmsg;
static struct kprobe kp_tcp_v4_do_rcv;
static struct kprobe kp_tcp_v6_do_rcv;
static struct kprobe kp_ip_local_out;
static struct kprobe kp_ip6_local_out;

static atomic_t skip_copy_kprobe_refcnt = ATOMIC_INIT(0);
static DEFINE_MUTEX(skip_copy_kprobe_mutex);


/************************************************************************************************
*
*************************************************************************************************
*/
static uint8_t* airoha_get_macaddr(void)
{
	static uint8_t addr[6] = {0};
	static int mac_cached = 0;
	static DEFINE_SPINLOCK(mac_spinlock);
	struct airoha_eth *eth;
	unsigned long flags;
	u32 val;

	eth = READ_ONCE(glb_eth);
	if (!eth || !eth->fe_regs) {
		pr_warn_ratelimited("arht_kprobe: glb_eth not ready\n");
		return addr;
	}

	if (likely(smp_load_acquire(&mac_cached))){
		return addr;
	}

	spin_lock_irqsave(&mac_spinlock, flags);
	if (mac_cached) {
		spin_unlock_irqrestore(&mac_spinlock, flags);
		return addr;
	}

	/* Read the high part of the MAC address */
	val = airoha_fe_rr(eth, REG_FE_LAN_MAC_H);
	addr[0] = (val >> 16) & 0xFF;
	addr[1] = (val >> 8) & 0xFF;
	addr[2] = val & 0xFF;

	/* Read the low part of the MAC address */
	val = airoha_fe_rr(eth, REG_FE_MAC_LMIN(REG_FE_LAN_MAC_H));
	addr[3] = (val >> 16) & 0xFF;
	addr[4] = (val >> 8) & 0xFF;
	addr[5] = val & 0xFF;

	smp_store_release(&mac_cached, 1);
	spin_unlock_irqrestore(&mac_spinlock, flags);

	return addr;
}

void *arht_kprobe_resolve_symbol(const char *name)
{
	struct kprobe kp_resolve;
	void *addr;
	int ret;

	memset(&kp_resolve, 0, sizeof(kp_resolve));
	kp_resolve.symbol_name = name;
	ret = register_kprobe(&kp_resolve);
	if (ret < 0) {
		pr_err("arht_kprobe: resolve symbol '%s' failed: %d\n", name, ret);
		return NULL;
	}
	addr = (void *)kp_resolve.addr;
	unregister_kprobe(&kp_resolve);
	return addr;
}
EXPORT_SYMBOL(arht_kprobe_resolve_symbol);

/*
 * MSG_TRUNC on TCP recv: skips skb_copy_datagram_msg() in
 * tcp_recvmsg_locked(), advancing copied_seq without user-space copy.
 * Only applied to benchmark apps (iperf3/ookla) that discard received
 * data content. This is a stable kernel behavior since Linux 4.x.
 */
static int kp_tcp_recvmsg_pre(struct kprobe *p, struct pt_regs *regs)
{
	int flags;
	struct sock *sk;
	struct tcp_sock *tp;
	u32 rcv_nxt, copied_seq, pending;

	if (!regs){
		return 0;
	}

	sk = (struct sock *)regs->regs[0];
	if (!sk || !sk_fullsock(sk)){
		return 0;
	}

	if (READ_ONCE(sk->sk_mark) != SK_MARK_LOCAL_OFFLOAD){
		return 0;
	}

	flags = (int)regs->regs[3];
	if (flags & (MSG_PEEK | MSG_OOB | MSG_ERRQUEUE)){
		return 0;
	}

	tp = tcp_sk(sk);
	rcv_nxt = READ_ONCE(tp->rcv_nxt);
	copied_seq = READ_ONCE(tp->copied_seq);
	if (before(copied_seq, rcv_nxt))
	{
		pending = rcv_nxt - copied_seq;
		if (pending > SKIP_COPY_MIN_BYTES && pending < (1U << 30)) {
			regs->regs[3] |= MSG_TRUNC;
		}
	}

	return 0;
}

int arht_skip_copy_kprobe_enable(void)
{
	int ret = 0;

	mutex_lock(&skip_copy_kprobe_mutex);
	if (atomic_inc_return(&skip_copy_kprobe_refcnt) > 1) {
		/* Already registered by someone else */
		mutex_unlock(&skip_copy_kprobe_mutex);
		return 0;
	}

	memset(&kp_tcp_recvmsg, 0, sizeof(kp_tcp_recvmsg));
	kp_tcp_recvmsg.symbol_name = "tcp_recvmsg";
	kp_tcp_recvmsg.pre_handler = kp_tcp_recvmsg_pre;
	ret = register_kprobe(&kp_tcp_recvmsg);
	if (ret < 0) {
		atomic_dec(&skip_copy_kprobe_refcnt);
		mutex_unlock(&skip_copy_kprobe_mutex);
		pr_err("arht_kprobe: kprobe on tcp_recvmsg failed: %d\n", ret);
		return ret;
	}
	mutex_unlock(&skip_copy_kprobe_mutex);
	pr_info("arht_kprobe: kprobe on tcp_recvmsg at %pS\n", kp_tcp_recvmsg.addr);

	return 0;
}
EXPORT_SYMBOL(arht_skip_copy_kprobe_enable);

void arht_skip_copy_kprobe_disable(void)
{
	int refcnt = 0;

	mutex_lock(&skip_copy_kprobe_mutex);
	refcnt = atomic_dec_return(&skip_copy_kprobe_refcnt);
	if (refcnt > 0) {
		mutex_unlock(&skip_copy_kprobe_mutex);
		return;
	}

	if (refcnt < 0) {
		atomic_set(&skip_copy_kprobe_refcnt, 0);
		mutex_unlock(&skip_copy_kprobe_mutex);
		return;
	}

	/* refcnt == 0, last user, unregister */
	unregister_kprobe(&kp_tcp_recvmsg);
	memset(&kp_tcp_recvmsg, 0, sizeof(kp_tcp_recvmsg));
	mutex_unlock(&skip_copy_kprobe_mutex);
	pr_info("arht_kprobe: disable kprobe on tcp_recvmsg.\n");
}
EXPORT_SYMBOL(arht_skip_copy_kprobe_disable);

static int kp_tcp_do_rcv_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct sock *sk;
	struct sk_buff *skb;
	struct inet_sock *inet;

	if (!regs){
		return 0;
	}

	sk = (struct sock *)regs->regs[0];
	skb = (struct sk_buff *)regs->regs[1];

	if (!sk || !skb || !sk_fullsock(sk)){
		return 0;
	}

	inet = inet_sk(sk);

	/* Delegate to dynamic_ifc business logic */
	difc_handle_tcp_do_rcv(sk, skb, inet->inet_sport, inet->inet_dport);

	return 0;
}

/**
 * difc_skip_original_func - Simulate an immediate return from the probed function.
 * @regs: pt_regs snapshot captured at the probe point (function entry).
 * @ret_val: The value the probed function should appear to return (placed in x0).
 *
 * ARM64 calling convention: x0 = return value, x30 (LR) = return address.
 * By setting PC = LR we make execution resume at the probed function's caller,
 * as if the probed function executed "mov x0, #ret_val; ret".
 *
 * IMPORTANT: The caller (pre_handler) MUST return non-zero after calling this,
 * so that the kprobe framework skips single-stepping.  If single-step were
 * performed, it would execute the instruction at the (now modified) PC, which
 * is the caller's code - not the probed function's next instruction.
 *
 * Prerequisites:
 *  - PC matches probe address (we are at function entry)
 *  - LR is a valid kernel text address (safe to jump to)
 *  - LR != probe address (no recursive re-entry)
 *  - SP is 16-byte aligned (ABI requirement)
 */
static inline void difc_skip_original_func(struct pt_regs *regs, unsigned long ret_val)
{
	if (!regs){
		return;
	}

	regs->regs[0] = ret_val;
	regs->pc = regs->regs[30];
}

/**
 * kp_local_out_fast_tx_pre - Fast-path TX bypass for dynamic IFC flows.
 *
 * Intercepts __ip_local_out / __ip6_local_out and redirects matched flows
 * directly to hardware via fast_tx_fn, bypassing the normal IP output path.
 *
 * NOTE: GSO/TSO packets are intentionally NOT filtered here.
 * The underlying hardware (QDMA) supports TSO natively - it reads
 * skb_shinfo(skb)->gso_size and performs hardware segmentation.
 * Therefore, passing GSO-marked skbs to fast_tx_fn is correct behavior.
 */
static int kp_local_out_fast_tx_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct sock *sk;
	struct sk_buff *skb;
	struct inet_sock *inet;
	uint8_t *mac;
	int (*fast_tx_fn)(struct sk_buff *, int);
	bool is_ipv4 = (p == &kp_ip_local_out);
	int ret;

	if (!regs){
		return 0;
	}

	sk = (struct sock *)regs->regs[1];
	skb = (struct sk_buff *)regs->regs[2];

	if (!sk || !skb || !sk_fullsock(sk)){
		return 0;
	}

	if (sk->sk_protocol != IPPROTO_TCP || sk->sk_state != TCP_ESTABLISHED) {
		return 0;
	}

	fast_tx_fn = READ_ONCE(offload_eth_fast_tx_hook);
	if (unlikely(!fast_tx_fn)){
		return 0;
	}

	mac = airoha_get_macaddr();
	if (is_zero_ether_addr(mac)) {
		return 0;
	}

	if (is_ipv4) {
		if (skb_headlen(skb) < skb_network_offset(skb) + sizeof(struct iphdr))
			return 0;
	} else {
		if (skb_headlen(skb) < skb_network_offset(skb) + sizeof(struct ipv6hdr))
			return 0;
		if (skb->len < skb_network_offset(skb) + sizeof(struct ipv6hdr))
			return 0;
	}

	inet = inet_sk(sk);
	/* Delegate to dynamic_ifc business logic */
	ret = difc_handle_local_out(sk, skb, inet->inet_sport, inet->inet_dport, is_ipv4, mac, fast_tx_fn);
	if (ret > 0) {
		/* skb consumed, skip original function */
		difc_skip_original_func(regs, 0);
		/* intentional: skip single-step to bypass probed func (ARM64 kprobe convention) */
		return 1;
	}

	return 0;
}

static int kp_tcp_sendmsg_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct sock *sk;

	if (difc_get_app_num() == 0){
		return 0;
	}

	if (!regs){
		return 0;
	}

	sk = (struct sock *)regs->regs[0];
	if (!sk || sk->sk_state != TCP_ESTABLISHED)
		return 0;

	if (!difc_is_current_app()){
		return 0;
	}

	difc_add_entry_from_sock(sk);

	return 0;
}

int arht_kprobe_difc_init(void)
{
	int ret;

	ret = arht_skip_copy_kprobe_enable();
	if (ret < 0) {
		pr_warn("arht_kprobe: skip_copy kprobe enable failed: %d\n", ret);
		return ret;
	}

	memset(&kp_tcp_sendmsg, 0, sizeof(kp_tcp_sendmsg));
	kp_tcp_sendmsg.symbol_name = "tcp_sendmsg";
	kp_tcp_sendmsg.pre_handler = kp_tcp_sendmsg_pre;
	ret = register_kprobe(&kp_tcp_sendmsg);
	if (ret < 0) {
		pr_err("arht_kprobe: register kprobe tcp_sendmsg failed: %d\n", ret);
		goto err_skip_copy;
	}

	/*
	 * Hook ip_output / ip6_output instead of __ip_local_out / __ip6_local_out.
	 *
	 * __ip_local_out is called BEFORE NF_INET_POST_ROUTING, so conntrack
	 * confirmation has not yet happened at that point.  Packets intercepted
	 * there bypass nf_confirm (priority INT_MAX at POST_ROUTING), leaving
	 * the conntrack entry unconfirmed.  Reply packets then cannot match any
	 * confirmed entry and are marked INVALID by conntrack, causing them to
	 * be dropped by customer firewall rules that block INVALID-state traffic.
	 *
	 * ip_output / ip6_output are invoked by dst_output() which is called
	 * from __ip_local_out AFTER all NF_INET_POST_ROUTING hooks (including
	 * nf_confirm) have completed.  Hooking here guarantees the conntrack
	 * entry is already confirmed before we redirect the packet to hardware,
	 * so reply packets are correctly classified as ESTABLISHED.
	 *
	 * Function signatures are identical to __ip_local_out:
	 *   int ip_output(struct net *net, struct sock *sk, struct sk_buff *skb)
	 *   int ip6_output(struct net *net, struct sock *sk, struct sk_buff *skb)
	 * ARM64 register layout: net=x0, sk=x1, skb=x2  (unchanged)
	 */
	memset(&kp_ip_local_out, 0, sizeof(kp_ip_local_out));
	kp_ip_local_out.symbol_name = "ip_output";
	kp_ip_local_out.pre_handler = kp_local_out_fast_tx_pre;
	ret = register_kprobe(&kp_ip_local_out);
	if (ret < 0) {
		pr_err("arht_kprobe: register kprobe ip_output failed: %d\n", ret);
		goto err_unreg_sendmsg;
	}

	memset(&kp_ip6_local_out, 0, sizeof(kp_ip6_local_out));
	kp_ip6_local_out.symbol_name = "ip6_output";
	kp_ip6_local_out.pre_handler = kp_local_out_fast_tx_pre;
	ret = register_kprobe(&kp_ip6_local_out);
	if (ret < 0) {
		pr_err("arht_kprobe: register kprobe ip6_output failed: %d\n", ret);
		goto err_unreg_ip4_out;
	}

	memset(&kp_tcp_v4_do_rcv, 0, sizeof(kp_tcp_v4_do_rcv));
	kp_tcp_v4_do_rcv.symbol_name = "tcp_v4_do_rcv";
	kp_tcp_v4_do_rcv.pre_handler = kp_tcp_do_rcv_pre;
	ret = register_kprobe(&kp_tcp_v4_do_rcv);
	if (ret < 0) {
		pr_err("arht_kprobe: register kprobe tcp_v4_do_rcv failed: %d\n", ret);
		goto err_unreg_ip6_out;
	}

	memset(&kp_tcp_v6_do_rcv, 0, sizeof(kp_tcp_v6_do_rcv));
	kp_tcp_v6_do_rcv.symbol_name = "tcp_v6_do_rcv";
	kp_tcp_v6_do_rcv.pre_handler = kp_tcp_do_rcv_pre;
	ret = register_kprobe(&kp_tcp_v6_do_rcv);
	if (ret < 0) {
		pr_err("arht_kprobe: register kprobe tcp_v6_do_rcv failed: %d\n", ret);
		goto err_unreg_v4_rcv;
	}

	pr_info("arht_kprobe: all kprobes registered successfully!\n");
	return 0;

/*
 * Error cleanup labels: each label is reached when the NEXT registration
 * (below it in the registration sequence) fails. The label unregisters
 * the LAST successfully registered kprobe and falls through.
 *
 * Registration order: sendmsg -> ip4_out -> ip6_out -> v4_rcv -> v6_rcv
 * Cleanup order (reverse): v4_rcv -> ip6_out -> ip4_out -> sendmsg -> skip_copy
 */
err_unreg_v4_rcv:
	unregister_kprobe(&kp_tcp_v4_do_rcv);   /* v6_rcv failed, undo v4_rcv */
	/* fall through */
err_unreg_ip6_out:
	unregister_kprobe(&kp_ip6_local_out);   /* v4_rcv failed, undo ip6_out */
	/* fall through */
err_unreg_ip4_out:
	unregister_kprobe(&kp_ip_local_out);	/* ip6_out failed, undo ip4_out */
	/* fall through */
err_unreg_sendmsg:
	unregister_kprobe(&kp_tcp_sendmsg);	 	/* ip4_out failed, undo sendmsg */
	/* fall through */
err_skip_copy:
	arht_skip_copy_kprobe_disable();
	return ret;
}
EXPORT_SYMBOL(arht_kprobe_difc_init);

void arht_kprobe_difc_exit(void)
{
	unregister_kprobe(&kp_tcp_v6_do_rcv);
	unregister_kprobe(&kp_tcp_v4_do_rcv);
	unregister_kprobe(&kp_ip6_local_out);
	unregister_kprobe(&kp_ip_local_out);
	unregister_kprobe(&kp_tcp_sendmsg);
	arht_skip_copy_kprobe_disable();
	pr_info("arht_kprobe: all kprobes unregistered\n");
}
EXPORT_SYMBOL(arht_kprobe_difc_exit);

#else /* !ARHT_KPROBE_FULL_SUPPORT */

/* Empty translation unit - no functionality needed */

#endif


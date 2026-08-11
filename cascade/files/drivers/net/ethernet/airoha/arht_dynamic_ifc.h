/*********************************************************************************
 * Declaration and function prototype for arht dynamic IFC
 *
 * Copyright (c) 2024 AIROHA Inc
 * All Rights Reserved.
 *********************************************************************************/
#ifndef ARHT_DYNAMIC_IFC_H_
#define ARHT_DYNAMIC_IFC_H_

/*
 * IFC API Atomic Safety Guarantee:
 * ---------------------------------
 * The following DIFC_API macros/functions are used within spinlock-protected
 * (IRQ-disabled) critical sections throughout this module:
 *
 *   - DIFC_API_SET_LUT_RULE_AUTO()
 *   - DIFC_API_SET_ACTION()
 *   - DIFC_API_DEL_LUT_RULE_AUTO()
 *
 * These APIs internally only perform memory-mapped I/O register operations
 * (readl/writel or equivalent) on the IFC hardware. They do NOT:
 *   - Acquire any mutex or semaphore
 *   - Call schedule() or any potentially-sleeping function
 *   - Allocate memory with GFP_KERNEL
 *   - Perform any operation that may block
 *
 * Therefore, all call sites within spin_lock_irqsave() sections are safe
 * and intentional. This has been verified against the ecnt_hook_ifc
 * implementation. If the underlying IFC API implementation changes to
 * include sleeping operations in the future, these call sites MUST be
 * refactored to move IFC operations outside the spinlock.
 */

#ifndef PPE_MAGIC_DYNAMIC_IFC
#define PPE_MAGIC_DYNAMIC_IFC   0x72b2
#endif

#define PPE_CPU_REASON_BIT		27
#define PPE_CPU_MASK			(0x1F << PPE_CPU_REASON_BIT)

#define IFC_LRO_RING_START		12
#define IFC_LRO_RING_NUM		4
#define IFC_DEFAULT_RING		1
#define IFC_RING_SLOT_FREE		(-1)

#define LRO_AGG_NUM_DEFAULT        18
#define LRO_AGG_NUM_MIN            1
#define LRO_AGG_NUM_MAX            255

#define MAX_DYNAMIC_IFC_NUM	8
#define MAX_DYNAMIC_IFC_APP_NUM	16

#define EXPIRE_TIME		(1*HZ)

#define FOE_ENTRY_NUM(skb)		(skb_get_hash(skb) & 0xFFFF)

#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_ERR  1
#define DEBUG_LEVEL_WARN 2
#define DEBUG_LEVEL_INFO 3
#define DYNAMIC_IFC_LOG(level, fmt, ...) \
	do { \
		if (level <= debug_level) \
			printk(fmt, ##__VA_ARGS__); \
	} while (0)

/*****************************************************************************
 * Data structures
 *****************************************************************************/
struct dynamic_ifc {
	struct dst_entry *rx_dst;
	struct dst_entry *tx_dst;
	struct timer_list age_timer;
	struct sock *sk;
	unsigned short local_port;
	unsigned short remote_port;
	unsigned short hash;
	unsigned long last_tx;
	unsigned int valid:1;
	unsigned int skip_copy:1;
	unsigned int ifc_valid:1;
	unsigned int timer_active:1;
	unsigned int resv:28;
	unsigned int ifc_index;
	int ring_id;
};


/*****************************************************************************
 * Function declarations
 *****************************************************************************/
/**
 * difc_handle_tcp_do_rcv - Called from kprobe on tcp_v4_do_rcv / tcp_v6_do_rcv
 * Updates rx_dst, hash, triggers IFC rule creation when rate-reached.
 */
int difc_handle_tcp_do_rcv(struct sock *sk, struct sk_buff *skb,
                           __be16 sport, __be16 dport);

/**
 * difc_handle_local_out - Called from kprobe on __ip_local_out / __ip6_local_out
 * Performs fast-tx bypass for matched flows.
 * Returns: >0 if skb consumed (caller should skip original func), 0 otherwise.
 */
int difc_handle_local_out(struct sock *sk, struct sk_buff *skb,
                          __be16 sport, __be16 dport,
                          bool is_ipv4, uint8_t *mac,
                          int (*fast_tx_fn)(struct sk_buff *, int));

/**
 * difc_add_entry_from_sock - Called from kprobe on tcp_sendmsg
 * Creates a new dynamic IFC session entry for the given socket.
 */
void difc_add_entry_from_sock(struct sock *sk);

/**
 * difc_get_app_num - Returns current number of registered apps
 */
int difc_get_app_num(void);

/**
 * difc_is_current_app - Check if current task is a dynamic IFC app
 */
int difc_is_current_app(void);

/**
 * difc_resolve_ecnt_hook - Resolve __ECNT_HOOK address
 * Called during kprobe init in process context.
 * Returns: 0 on success, negative errno on failure.
 */
int difc_resolve_ecnt_hook(void);

/**
 * difc_clear_ecnt_hook - Clear resolved hook function pointer
 */
void difc_clear_ecnt_hook(void);


#endif /* ARHT_DYNAMIC_IFC_H_ */

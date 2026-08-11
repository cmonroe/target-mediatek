/*********************************************************************************
 * Declaration and function prototype for arht kprobe
 *
 * Copyright (c) 2024 AIROHA Inc
 * All Rights Reserved.
 *********************************************************************************/

#ifndef ARHT_KPROBE_H_
#define ARHT_KPROBE_H_

#include <linux/types.h>

#define SK_MARK_LOCAL_OFFLOAD   0xAE000001

/* skip_copy kprobe on tcp_recvmsg - shared utility */
int arht_skip_copy_kprobe_enable(void);
void arht_skip_copy_kprobe_disable(void);

/**
 * arht_kprobe_resolve_symbol - Resolve a kernel symbol address via kprobe trick
 * @name: symbol name to resolve
 * Return: resolved address, or NULL on failure
 */
void *arht_kprobe_resolve_symbol(const char *name);

/* Dynamic IFC kprobe group - register/unregister all probes */
int arht_kprobe_difc_init(void);
void arht_kprobe_difc_exit(void);

#endif /* ARHT_KPROBE_H_ */
 
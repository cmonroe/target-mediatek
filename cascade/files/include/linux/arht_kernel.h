
#ifndef _ECNT_KERNEL_H
#define _ECNT_KERNEL_H

#include <linux/version.h>

#ifdef CONFIG_TC3162_IMEM
#define __IMEM  __attribute__  ((__section__(".imem_text")))
#else
#define __IMEM
#endif

#if defined(CONFIG_TC3162_DMEM) && !defined(CONFIG_MIPS_TC3262)
#define __DMEM  __attribute__  ((__section__(".dmem_data")))
#else
#define __DMEM  
#endif
/*
 *      Display an IP address in readable format.
 */

#ifdef __LITTLE_ENDIAN
#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[3], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[0]
#define NIPTWOBYTE(addr) \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[0]
#else
#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[3]
#define NIPTWOBYTE(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1]
#endif

#define NIPQUAD_FMT "%u.%u.%u.%u"

#define NIP6(addr) \
	ntohs((addr).s6_addr16[0]), \
	ntohs((addr).s6_addr16[1]), \
	ntohs((addr).s6_addr16[2]), \
	ntohs((addr).s6_addr16[3]), \
	ntohs((addr).s6_addr16[4]), \
	ntohs((addr).s6_addr16[5]), \
	ntohs((addr).s6_addr16[6]), \
	ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define NIP6_SEQFMT "%04x%04x%04x%04x%04x%04x%04x%04x"
#ifdef CONFIG_TC3162_IMEM
#define __IMEM  __attribute__  ((__section__(".imem_text")))
#else
#define __IMEM
#endif

#if defined(CONFIG_TC3162_DMEM) && !defined(CONFIG_MIPS_TC3262)
#define __DMEM  __attribute__  ((__section__(".dmem_data")))
#else
#define __DMEM  
#endif
/*
 *      Display an IP address in readable format.
 */

#ifdef __LITTLE_ENDIAN
#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[3], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[0]
#define NIPTWOBYTE(addr) \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[0]
#else
#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[3]
#define NIPTWOBYTE(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1]
#endif

#define NIPQUAD_FMT "%u.%u.%u.%u"

#define NIP6(addr) \
	ntohs((addr).s6_addr16[0]), \
	ntohs((addr).s6_addr16[1]), \
	ntohs((addr).s6_addr16[2]), \
	ntohs((addr).s6_addr16[3]), \
	ntohs((addr).s6_addr16[4]), \
	ntohs((addr).s6_addr16[5]), \
	ntohs((addr).s6_addr16[6]), \
	ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define NIP6_SEQFMT "%04x%04x%04x%04x%04x%04x%04x%04x"

#ifndef TIMER_FUN_PAAM
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
#define TIMER_FUN_PAAM unsigned long
#else
#define TIMER_FUN_PAAM struct timer_list *
#endif
#endif

#endif

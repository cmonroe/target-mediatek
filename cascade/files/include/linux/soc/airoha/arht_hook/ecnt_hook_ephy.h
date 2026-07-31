/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _ECNT_HOOK_EPHY_H
#define _ECNT_HOOK_EPHY_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook.h>

//#include <asm/tc3162/tc3162.h>	

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define ECNT_DRIVER_API  0

/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/


/* Warning: same sequence with function array 'qdma_operation' */
typedef enum { 
    EPHY_FUNCTION_MONITOR,
    EPHY_FUNCTION_POWER_DOWN,
	EPHY_FUNCTION_PHY_MII_WRITE_TRDBG,
	EPHY_FUNCTION_MT_MII_REG_WRITE,
	EPHY_FUNCTION_MT_MII_REG_READ,
	EPHY_FUNCTION_MT_EMII_REG_WRITE,
	EPHY_FUNCTION_MT_EMII_REG_READ,
	EPHY_FUNCTION_MONITOR_PHY_INIT_FLAG,
#if defined(TCSUPPORT_CPU_EN7583)
	EPHY_FUNCTION_DISABLE_LDPS,
#endif
#if  defined(TCSUPPORT_CPU_EN7512) || defined(TCSUPPORT_CPU_EN7521)
#if !defined(TCSUPPORT_CPU_EN7516) && !defined(TCSUPPORT_CPU_EN7527)
	EPHY_FUNCTION_MT7512FE_READ_PROBE,
	EPHY_FUNCTION_MT7512FE_SNR_SUM,
	EPHY_FUNCTION_MT7512_FE_READ_ADC_SUM,
#endif
#endif
	EPHY_FUNCTION_SLT_MODE,
	EPHY_FUNCTION_SET_MODE,
    EPHY_FUNCTION_MAX_NUM,
} EPHY_HookFunction_t ;

typedef struct ECNT_EPHY_Data {
	EPHY_HookFunction_t function_id;	/* need put at first item */
	int retValue;
	char phy_add;
	union {
		
		struct{
			u8 port_num;
			u8 mode;
		}probe;
		struct{
			u8 port_num;
			u16 cnt;
		}snr;
		struct{
			u8 phyaddr; 
			char *type; 
			u32 data_addr; 
			u32 value; 
			u8 ch_num;
		}trDbg;
		struct{
			u32 port_num;
			u32 dev_num;
			u32 reg_num;
			u32 reg_data;
		}mii;
		struct{
			u8 port_num;
			u8 reg_num;
		}miiRead;
		struct{
			u8 port_num;
		}adc;
		struct{
			u8 port_num;
			u16 speed_mode;
		}slt;
		struct{
			u8 set_mode;
			u16 parameter;
		}set;
	} ephy_private;
}ECNT_EPHY_Data_s;


/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int EPHY_API_GET_MONITOR(void){
    ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_MONITOR;
    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}

static inline int EPHY_API_POWER_DOWN(int phy_add){
    ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_POWER_DOWN;
    in_data.phy_add = phy_add;
    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}


/* int mtPhyMiiWrite_TrDbg(u8 phyaddr, char *type, u32 data_addr ,u32 value, u8 ch_num) */
static inline int EPHY_API_PHY_MII_WRITE_TRDBG(u8 phyaddr, char *type, u32 data_addr ,u32 value, u8 ch_num){
	/* CID:710965 */
	ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_PHY_MII_WRITE_TRDBG;

	in_data.ephy_private.trDbg.phyaddr = phyaddr;
	in_data.ephy_private.trDbg.type = type;
	in_data.ephy_private.trDbg.data_addr = data_addr;
	in_data.ephy_private.trDbg.value = value;
	in_data.ephy_private.trDbg.ch_num = ch_num;
    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.retValue;
}

/* void mtMiiRegWrite(u32 port_num, u32 reg_num, u32 reg_data) */
static inline int EPHY_API_MT_MII_REG_WRITE(u32 port_num, u32 reg_num, u32 reg_data){
    /* CID:710505 */
	ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_MT_MII_REG_WRITE;
	in_data.ephy_private.mii.port_num = port_num;
	in_data.ephy_private.mii.reg_num = reg_num;
	in_data.ephy_private.mii.reg_data = reg_data;

    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.retValue;
}

/* u32 mtMiiRegRead(u8 port_num,u8 reg_num) */
static inline int EPHY_API_MT_MII_REG_READ(u8 port_num,u8 reg_num){
	/* CID:712082 */
    ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_MT_MII_REG_READ;
	in_data.ephy_private.miiRead.port_num = port_num;
	in_data.ephy_private.miiRead.reg_num = reg_num;

    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.retValue;
}

/* void mtEMiiRegWrite(u32 port_num, u32 dev_num, u32 reg_num, u32 reg_data) */
static inline int EPHY_API_MT_EMII_REG_WRITE(u32 port_num, u32 dev_num, u32 reg_num, u32 reg_data){
    /* CID:708533 */
	ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_MT_EMII_REG_WRITE;
	in_data.ephy_private.mii.port_num = port_num;
	in_data.ephy_private.mii.dev_num = dev_num;
	in_data.ephy_private.mii.reg_num= reg_num;
	in_data.ephy_private.mii.reg_data = reg_data;
    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.retValue;
}

/* u32 mtEMiiRegRead(u32 port_num, u32 dev_num, u32 reg_num) */
static inline int EPHY_API_MT_EMII_REG_READ(u32 port_num, u32 dev_num, u32 reg_num){
    /* CID:715790 */
	ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_MT_EMII_REG_READ;
	in_data.ephy_private.mii.port_num = port_num;
	in_data.ephy_private.mii.dev_num = dev_num;
	in_data.ephy_private.mii.reg_num = reg_num;
    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.retValue;
}

static inline int EPHY_API_GET_MONITOR_PHY_INIT_FLAG(void) {
	ECNT_EPHY_Data_s in_data = {0};
	int ret = 0;
	in_data.function_id = EPHY_FUNCTION_MONITOR_PHY_INIT_FLAG;
	ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if (ret == ECNT_HOOK_ERROR)
		return ECNT_HOOK_ERROR;
	else
		return in_data.retValue;
}

#if defined(TCSUPPORT_CPU_EN7583)
static inline int EPHY_API_DISABLE_LDPS(void) {
	ECNT_EPHY_Data_s in_data = {0};
	int ret = 0;
	in_data.function_id = EPHY_FUNCTION_DISABLE_LDPS;
	ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if (ret != ECNT_HOOK_ERROR)
		return ECNT_CONTINUE;
	else
		return ECNT_HOOK_ERROR;
}
#endif

#if  defined(TCSUPPORT_CPU_EN7512) || defined(TCSUPPORT_CPU_EN7521)
#if !defined(TCSUPPORT_CPU_EN7516) && !defined(TCSUPPORT_CPU_EN7527)
/* int32 mt7512FEReadProbe(u8 port_num, u8 mode) */
static inline int EPHY_API_MT7512FE_READ_PROBE(u8 port_num, u8 mode){
    ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_MT7512FE_READ_PROBE;
	in_data.ephy_private.probe.port_num = port_num;
	in_data.ephy_private.probe.mode = mode;
    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.retValue;
}

/* u16 mt7512FEReadSnrSum(u8 port_num, u16 cnt) */
static inline int EPHY_API_MT7512FE_SNR_SUM(u8 port_num, u16 cnt){
    ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_MT7512FE_SNR_SUM;
	in_data.ephy_private.snr.port_num = port_num;
	in_data.ephy_private.snr.cnt = cnt;
    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.retValue;

}

/* int32 mt7512FEReadAdcSum(u8 port_num); */
static inline int EPHY_API_MT7512_FE_READ_ADC_SUM(u8 port_num){
    ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_MT7512_FE_READ_ADC_SUM;
	in_data.ephy_private.adc.port_num = port_num;

    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.retValue;
}

#endif
#endif
/* void ephySLTMode(u8 port_num, u16 speed_mode); */
static inline int EPHY_API_SLT_MODE(u8 port_num, u16 speed_mode){
    ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_SLT_MODE;
	in_data.ephy_private.slt.port_num = port_num;
	in_data.ephy_private.slt.speed_mode = speed_mode;

    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}

/* void ephySETMode(u8 port_num, u16 speed_mode); */
static inline int EPHY_API_SET_MODE(u8 set_mode, u16 parameter){
    ECNT_EPHY_Data_s in_data = {0};
    int ret=0;
    in_data.function_id = EPHY_FUNCTION_SET_MODE;
	in_data.ephy_private.set.set_mode = set_mode;
	in_data.ephy_private.set.parameter = parameter;

    ret = __ECNT_HOOK(ECNT_ETHER_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}



#endif /*_TCETHERPHY_HOOK_H*/

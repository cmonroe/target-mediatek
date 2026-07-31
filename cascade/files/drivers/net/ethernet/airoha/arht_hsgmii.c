// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 AIROHA Inc
 * Author:  2024 AIROHA Inc
 */

#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/phy.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/of_mdio.h>
#include <linux/of_address.h>
#include <linux/mii.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/timekeeping.h>
#include <linux/timex.h>
#include <linux/firmware.h>
#include <linux/crc32.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include "arht_hsgmii.h"
#include "arht_dp_api.h"

uint8_t serdes_map_by_ordes[MAX_SERDES_NUM] = {3,4,0,1,2}; /*pon-eth-wifi1-wifi2-usb1*/
uint8_t serdes_map_by_ordes_num =  sizeof(serdes_map_by_ordes)/sizeof(serdes_map_by_ordes[0]);
Mdio_priv_data_t *hsgmii_priv_p = NULL;
static int dev_cnt = 0;
struct phy_device *an8831_phy_dev[AN8831_PHY_NUM] = {NULL};
struct phy_device **an8831_dev_all = an8831_phy_dev;
int debuglevel = 0;
static struct phy_device *an8831_dev;
static struct mii_bus *an8831_phy_bus;
static int an8831_phy_addr = 29;
static struct an_mdi_cfg __priv_data = { 0 };
static int en8811_is_exist = 0;
static int an8831_is_exist = 0;

static uint8_t phy_state = 1;
struct mii_bus *airoha_mii_bus[MAX_MDIO_BUS] = {NULL};
EXPORT_SYMBOL(airoha_mii_bus);

extern void fast_path_speed_threshold_init(void);
extern int (*xsi_mac_set_ratelimit_hook)(uint hsgmii_index, unsigned int type, uint rate, uint mode);

int airphy_set_mode(struct phy_device *phydev, AIR_PORT_MODE_T dbg_mode)
{
    int ret = 0, val = 0;
	struct mii_bus *mbus = phydev->mdio.bus;
    int addr = phydev->mdio.addr;

    switch(dbg_mode)
    {
        case AIR_PORT_MODE_FORCE_100:
            pr_notice("\nForce 100M\n");
            val = mbus->read(mbus, addr, MII_ADVERTISE) | BIT(8);
            ret = mbus->write(mbus, addr, MII_ADVERTISE, val);

            val = mbus->read(mbus, addr, MII_CTRL1000) & ~BIT(9);
            ret = mbus->write(mbus, addr, MII_CTRL1000, val);

            val = mbus->read_c45(mbus, addr, 0x7, 0x20) & ~BIT(7);
            ret = mbus->write_c45(mbus, addr, 0x7, 0x20, val);

            val = mbus->read(mbus, addr, MII_BMCR) | BIT(9);
            ret = mbus->write(mbus, addr, MII_BMCR, val);
            break;
        case AIR_PORT_MODE_FORCE_1000:
            pr_notice("\nForce 1000M\n");
            val = mbus->read(mbus, addr, MII_ADVERTISE) & ~BIT(8);
            ret = mbus->write(mbus, addr, MII_ADVERTISE, val);

            val = mbus->read(mbus, addr, MII_CTRL1000) | BIT(9);
            ret = mbus->write(mbus, addr, MII_CTRL1000, val);

            val = mbus->read_c45(mbus, addr, 0x7, 0x20) & ~BIT(7);
            ret = mbus->write_c45(mbus, addr, 0x7, 0x20, val);

            val = mbus->read(mbus, addr, MII_BMCR) | BIT(9);
            ret = mbus->write(mbus, addr, MII_BMCR, val);
            break;
        case AIR_PORT_MODE_FORCE_2500:
            pr_notice("\nForce 2500M\n");
            val = mbus->read(mbus, addr, MII_ADVERTISE) & ~BIT(8);
            ret = mbus->write(mbus, addr, MII_ADVERTISE, val);

            val = mbus->read(mbus, addr, MII_CTRL1000) & ~BIT(9);
            ret = mbus->write(mbus, addr, MII_CTRL1000, val);

            val = mbus->read_c45(mbus, addr, 0x7, 0x20) | BIT(7);
            ret = mbus->write_c45(mbus, addr, 0x7, 0x20, val);

            val = mbus->read(mbus, addr, MII_BMCR) | BIT(9);
            ret = mbus->write(mbus, addr, MII_BMCR, val);
            break;
        case AIR_PORT_MODE_AUTONEGO:
            pr_notice("\nAutonego mode\n");
			val = mbus->read(mbus, addr, MII_ADVERTISE) | BIT(8);
			ret = mbus->write(mbus, addr, MII_ADVERTISE, val);

			val = mbus->read(mbus, addr, MII_CTRL1000) | BIT(9);
			ret = mbus->write(mbus, addr, MII_CTRL1000, val);

			val = mbus->read_c45(mbus, addr, 0x7, 0x20) | BIT(7);
			ret = mbus->write_c45(mbus, addr, 0x7, 0x20, val);

			val = mbus->read(mbus, addr, MII_BMCR) | BIT(9);
			ret = mbus->write(mbus, addr, MII_BMCR,val );
            break;
        case AIR_PORT_MODE_POWER_DOWN:
            pr_notice("\nPower Down\n");
            val = mbus->read(mbus, addr, MII_BMCR) | BIT(11);
            ret = mbus->write(mbus, addr, MII_BMCR, val);
            break;
        case AIR_PORT_MODE_POWER_UP:
            pr_notice("\nPower Up\n");
            val = mbus->read(mbus, addr, MII_BMCR) & ~BIT(11);
            ret = mbus->write(mbus, addr, MII_BMCR, val);
            break;
        case AIR_PORT_MODE_POWER_STATE:
            pr_notice("\nPower State\n");
            ret = mbus->read(mbus, addr, MII_BMCR) & BIT(11);
            break;
        default:
            pr_notice("\nWrong Port mode\n");
            break;
    }
    return ret;
}	

static int EN8811_get_LinkInfo(char *kbuf, size_t kbuf_size, uint8_t index, struct phy_device *phydev, char *phyItfName)
{
	int speed = 0;
	int duplex = 0;

	if(!CHECK_PHY_DEV(phydev)){
		EN8811_LOG(EN8811_LOG_ERROR,"Invalid PHY device\n");
		return index;
	}
	index += snprintf(kbuf + index, kbuf_size - index, "PHY[%s]: ", phyItfName);

	speed = phydev->speed;
	duplex = phydev->duplex;   
	//link status
	if (speed == SPEED_UNKNOWN) {
		index += snprintf(kbuf + index, kbuf_size - index, "Down \n\n");
		return index;
	}else if (speed == SPEED_10000){
		index += snprintf(kbuf + index, kbuf_size - index, "10Gbps/");
	}else if (speed == SPEED_5000){
		index += snprintf(kbuf + index, kbuf_size - index, "5Gbps/");
	}else if (speed == SPEED_2500){
		index += snprintf(kbuf + index, kbuf_size - index, "2.5Gbps/");
	}else if (speed == SPEED_1000){
		index += snprintf(kbuf + index, kbuf_size - index, "1000Mbps/");
	}else if (speed == SPEED_100){
		index += snprintf(kbuf + index, kbuf_size - index, "100Mbps/");
	}
		
	//full or half duplex
	if (duplex == DUPLEX_FULL)
		index += snprintf(kbuf + index, kbuf_size - index, "Full");
	else if (duplex == DUPLEX_HALF)
		index += snprintf(kbuf + index, kbuf_size - index, "Half");

	index += snprintf(kbuf + index, kbuf_size - index, "\n\n");

	return index;
}

static int EN8811_cl22_op(struct phy_device *phydev, uint32_t reg, uint32_t reg_bit, uint8_t op)
{
	int ret = 0;
	uint32_t reg_data = 0 ;
	struct mii_bus *mbus = phydev->mdio.bus;
	int addr = phydev->mdio.addr;
	
	reg_data = mbus->read(mbus, addr, reg);

	if(EN8811_WRITE_1 == op)
	{
		reg_data |= reg_bit;
	}
	else if(EN8811_WRITE_0 == op)
	{
		reg_data &= ~reg_bit;
	}else{
		EN8811_LOG(EN8811_LOG_WARNING,"input error: %d \n", op);
		return EN8811_ERROR;
	}

	ret = mbus->write(mbus, addr, reg, reg_data);
	if(ret == EN8811_ERROR){
		EN8811_LOG(EN8811_LOG_ERROR,"write error, reg:%d, reg_data:%d \n", reg, reg_data);
		return EN8811_ERROR;
	}
	return EN8811_CONTINUE;
}

inline static void get_itfname_remove_suffix(char *str, char *itfname, uint8_t iftname_size)
{
	const char *itf_prefix = "itf=";
	const char *start_of_value;

	start_of_value = strstr(str, itf_prefix);
	if(start_of_value){
		start_of_value += strlen(itf_prefix);
		strscpy(itfname,start_of_value,iftname_size);
		*str = '\0';
	}
}

inline static void EN8811_remove_itfname_prefix(char *phyItfName)
{
	const char *prefix = "itf=";
	char *match = 0;

	match = strstr(phyItfName,prefix);
	if(match){
		match += strlen(prefix);
		memmove(phyItfName, match , strlen(match) + 1); /*remove prefix  'itf='*/
	}
}

static int EN8811_check_writeproc_para(const char *buffer, unsigned long count, PARSE_DATA_T *input_para, Mdio_priv_data_t *mdio_priv_data_p)
{
	char val_string[STRING_LEN] = {'\0'}, cmd[CMD_LEN] = {'\0'}, param[CMD_LEN] = {'\0'}, subparam[CMD_LEN] = {'\0'};
	struct phy_device* phy_dev = NULL;
	struct mii_bus *mbus = NULL;
	int ret = 0;
	char phyItfName[CMD_LEN] = {'\0'};

	if (count > sizeof(val_string) - 1){
		return -EINVAL;
	}

	if (copy_from_user(val_string, buffer, count)){
		return -EINVAL;
	}

	/*start to parse input infomation*/
	ret = sscanf(val_string, "%s %s %s %s", cmd, param, subparam, phyItfName);
	EN8811_LOG(EN8811_LOG_DBG,"[before prase: %d] cmd:%s, param:%s, subparam:%s, phyItfName:%s\n", ret, cmd, param, subparam, phyItfName);
	if(ret == 4)
	{
		if(!EN8811_JUDGE_INPUT_ITFNAME(phyItfName)){
			get_default_ItfName(mdio_priv_data_p, phyItfName);
		}else{
			EN8811_remove_itfname_prefix(phyItfName);
		}
	}
	else if(ret == 3)
	{
		if(!EN8811_JUDGE_INPUT_ITFNAME(subparam)){
			get_default_ItfName(mdio_priv_data_p, phyItfName);
		}else{
			get_itfname_remove_suffix(subparam, phyItfName, STRING_LEN);
		}
	}
	else if(ret == 2)
	{
		if(!EN8811_JUDGE_INPUT_ITFNAME(param)){
			get_default_ItfName(mdio_priv_data_p, phyItfName);
		}else{
			get_itfname_remove_suffix(param, phyItfName,STRING_LEN);
		}
	}
	else if(ret == 1)
	{
		if(strstr(cmd, "bps") || strstr(cmd, "restartAN") || strstr(cmd, "Auto"))
		{
			get_default_ItfName(mdio_priv_data_p, phyItfName);
		}
		else{
			if(!EN8811_JUDGE_INPUT_ITFNAME(cmd))
			{
				pr_err("Please check your input parameter: echo itf=[itfname] > /proc/tc3163/en8811_\n");
				return -EINVAL;
			}
			else{
				get_itfname_remove_suffix(cmd, phyItfName,STRING_LEN);
			}
		}
	}
	else{
		pr_err("Please check your input parameter\n");
		return -EINVAL;
	}
	EN8811_LOG(EN8811_LOG_DBG,"[after prase] cmd:%s, param:%s, subparam:%s, phyItfName:%s\n",cmd, param, subparam, phyItfName);

	/*start to check valid*/
	if(!check_phy_itfname_valid(mdio_priv_data_p, phyItfName)){
		return -EINVAL;
	}

	/*save	info to input_para*/
	phy_dev = get_phyDev_by_ItfName(mdio_priv_data_p, phyItfName);
	if(!CHECK_PHY_DEV(phy_dev)){
		return -EINVAL;
	}
	input_para->phyDev = phy_dev;

	mbus = phy_dev->mdio.bus;
	if(!CHECK_POINTER(mbus)){
		return -EINVAL;
	}
	input_para->mbus = mbus;

	strncpy(input_para->cmd,cmd,CMD_LEN);
	strncpy(input_para->param,param,CMD_LEN);
	strncpy(input_para->subparam,subparam,CMD_LEN);
	strncpy(input_para->phyItfName,phyItfName,CMD_LEN);

	return 0;
}

static ssize_t en8811_debug_level_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *kbuf;
	size_t kbuf_size = 1024;

	if (*ppos > 0) {
		return 0; /* End of file */
	}

	kbuf = mem_alloc(kbuf_size);
	if(!CHECK_POINTER(kbuf)){
		return -ENOMEM;
	}

	len += snprintf(kbuf + len, kbuf_size - len, "echo 0-5 > proc/en8811_debug\n");
	len += snprintf(kbuf + len, kbuf_size - len, "[0]: disable all message.\n");
	len += snprintf(kbuf + len, kbuf_size - len, "[1]: enable error message.\n");
	len += snprintf(kbuf + len, kbuf_size - len, "[2]: enable warning message.\n");
	len += snprintf(kbuf + len, kbuf_size - len, "[3]: enable info message.\n");
	len += snprintf(kbuf + len, kbuf_size - len, "[4]: enable debug message.\n");
	len += snprintf(kbuf + len, kbuf_size - len, "[5]: enable data dump message.\n");
	len += snprintf(kbuf + len, kbuf_size - len, "[6]: Dump all en8811 private data.\n");
	len += snprintf(kbuf + len, kbuf_size - len, "\n debuglevel:%hhu\n\n", debuglevel);

	if (*ppos >= len)
	{
		mem_free(kbuf);
		return 0;
	}

	if (count > len - *ppos)
	{
		count = len - *ppos;
	}

	if (copy_to_user(buf, kbuf + *ppos, count)) {
		mem_free(kbuf);
		return -EFAULT;
	}
	*ppos += count;

	mem_free(kbuf);
	return count;
}

static ssize_t en8811_debug_level_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[STRING_LEN];
	uint8_t debug = 0, debug_tmp = 0;
	Mdio_priv_data_t *mdio_priv_data_p = NULL;
	int ret = 0;

	mdio_priv_data_p = hsgmii_priv_p;
	if (!CHECK_PHY_PRIV(mdio_priv_data_p)) {
		return count;
	}

	if (count > sizeof(val_string) - 1) {
		return -EINVAL;
	}

	if (copy_from_user(val_string, buf, count)) {
		return -EFAULT;
	}

	val_string[count] = '\0';

	ret = sscanf(val_string, "%hhu", &debug);
	if (!ret) {
		return -EINVAL;
	}

	if (debug < EN8811_LOG_MAX_LEVEL) {
		debuglevel = debug;
		return count;
	} else {
		debug_tmp = debuglevel;
	}

	if (debug == EN8811_LOG_MAX_LEVEL) {
		debuglevel = EN8811_LOG_OFF;
		print_phy_privdataInfo(mdio_priv_data_p);
	} else {
		printk("echo 0-5 > proc/en8811_debug\n");
		EN8811_LOG(EN8811_LOG_ERROR, "Please check the input parameter!\n");
	}

	debuglevel = debug_tmp;
	return count;
}

static ssize_t en8811_serdes_info_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	char *kbuf;
	size_t kbuf_size = 1024;
	int len = 0;
	int i;

	if (*ppos > 0)
		return 0; // EOF

	kbuf = kmalloc(kbuf_size, GFP_KERNEL);
	if (!kbuf) {
		return -ENOMEM;
	}

	len += snprintf(kbuf + len, kbuf_size - len, "-----------Serdes Map Info----\n");
	len += snprintf(kbuf + len, kbuf_size - len, "name	   serdes	  phy_addr\n");

	for (i = 0; i < SERDES_MAX_IDX; i++) {
		const char *name = itfname_info[i] ? itfname_info[i] : "--";
		const char *serdes = serdes_info[i] ? serdes_info[i] : "--";
		if (serdes_map[i].phy_addr == 0)
			len += snprintf(kbuf + len, kbuf_size - len, "%-8s %-13s --\n", name, serdes);
		else
			len += snprintf(kbuf + len, kbuf_size - len, "%-8s %-13s %d\n", name, serdes, serdes_map[i].phy_addr);
	}

	if (len > count)
		len = count;

	if (copy_to_user(buf, kbuf, len)) {
		kfree(kbuf);
		return -EFAULT;
	}

	kfree(kbuf);
	*ppos += len;
	return len;
}

static ssize_t en8811_link_state_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *kbuf;
	size_t kbuf_size = 1024;
	uint8_t i = 0;
	struct phy_device* phyDev = NULL;
	uint8_t phyAddr = INVALID_PHY_ADDR;
	uint8_t all_phyAddrs_num = 0;
	Mdio_priv_data_t *mdio_priv_data_p = NULL;
	char *phyItfName=NULL;

	if (*ppos > 0) {
		return 0; /* End of file */
	}

	kbuf = mem_alloc(kbuf_size);
	if(!CHECK_POINTER(kbuf)){
		return -ENOMEM;
	}

	mdio_priv_data_p = hsgmii_priv_p;
	if(!CHECK_PHY_PRIV(mdio_priv_data_p))
	{
		mem_free(kbuf);
		return count;
	}

	all_phyAddrs_num = MDIO_PHY_NUM(mdio_priv_data_p);

	for(i = 0; i < all_phyAddrs_num; i++)
	{
		phyAddr = MDIO_REG_ADDR(mdio_priv_data_p, i);
		EN8811_LOG(EN8811_LOG_DBG, "phy[%d] addr= %d\n", i, phyAddr);
		
		phyDev = GET_PHY_DEV(mdio_priv_data_p, phyAddr);
		if(!CHECK_PHY_DEV(phyDev)){
			EN8811_LOG(EN8811_LOG_ERROR, "Invalid PHY device\n");
			mem_free(kbuf);
			return -EINVAL;
		}

		phyItfName = get_ItfName_by_phyAddr(mdio_priv_data_p, phyAddr);
		len += EN8811_get_LinkInfo(kbuf + len, kbuf_size, kbuf_size - len, phyDev, phyItfName);
	}

	if (*ppos >= len) {
		mem_free(kbuf);
		return 0;
	}

	if (count > len - *ppos) {
		count = len - *ppos;
	}

	if (copy_to_user(buf, kbuf + *ppos, count)) {
		mem_free(kbuf);
		return -EFAULT;
	}
	*ppos += count;
	
	mem_free(kbuf);
	return count;
}

static ssize_t en8811_speed_mode_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *kbuf;
	size_t kbuf_size = 1024;
	
	if (*ppos > 0) {
		return 0; /* End of file */
	}

	kbuf = mem_alloc(kbuf_size);
	if(!CHECK_POINTER(kbuf)){
		return -ENOMEM;
	}
	
	len += snprintf(kbuf + len, kbuf_size - len, "Restart AN  : echo restartAN [itf=phy_itfname]> /proc/en8811_speed_mode \n\n");
	len += snprintf(kbuf + len, kbuf_size - len, "Open flow control : echo fc open [itf=phy_itfname] > /proc/en8811_speed_mode \n");
	len += snprintf(kbuf + len, kbuf_size - len, "Close flow control: echo fc close [itf=phy_itfname] > /proc/en8811_speed_mode \n\n");
	len += snprintf(kbuf + len, kbuf_size - len, "Enable An  : echo Auto [itf=phy_itfname] > /proc/en8811_speed_mode \n");
	len += snprintf(kbuf + len, kbuf_size - len, "Set 2.5G FD: echo 2.5Gbps [itf=phy_itfname] > /proc/en8811_speed_mode \n");
	len += snprintf(kbuf + len, kbuf_size - len, "Set 1G FD  : echo 1Gbps [itf=phy_itfname]   > /proc/en8811_speed_mode \n");
	len += snprintf(kbuf + len, kbuf_size - len, "Set 100M FD: echo 100Mbps [itf=phy_itfname] > /proc/en8811_speed_mode \n\n");

	if (*ppos >= len)
	{
		mem_free(kbuf);
		return 0;
	}

	if (count > len - *ppos)
	{
		count = len - *ppos;
	}

	if (copy_to_user(buf, kbuf + *ppos, count)) {
		mem_free(kbuf);
		return -EFAULT;
	}
	*ppos += count;
	
	mem_free(kbuf);
	return count;
}

static ssize_t en8811_speed_mode_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int ret = 0;
	char cmd[CMD_LEN] = {0}, param[CMD_LEN] = {0};
	uint8_t phyAddr = INVALID_PHY_ADDR;
	struct phy_device* phyDev = NULL;
	struct mii_bus *mbus = NULL;
	PARSE_DATA_T parse_data;
	Mdio_priv_data_t *mdio_priv_data_p = NULL;
	char* phyItfName = NULL;

	mdio_priv_data_p = hsgmii_priv_p;
	if(!CHECK_PHY_PRIV(mdio_priv_data_p)){
		return count;
	}
	
	memset(&parse_data, 0, sizeof(PARSE_DATA_T));
	ret = EN8811_check_writeproc_para(buf,count,&parse_data,mdio_priv_data_p);
	if(ret){
		return ret;
	}else{
		phyItfName = parse_data.phyItfName;
		phyAddr = get_phyAddr_by_ItfName(phyItfName);
		phyDev = parse_data.phyDev;
		mbus = parse_data.mbus;
		strcpy(cmd,parse_data.cmd);
		strcpy(param,parse_data.param);
	}

	if(!strcmp(cmd, "restartAN"))
	{
		ret = EN8811_cl22_op(phyDev, EN8811_RestartAN_REG, EN8811_RestartAN_BIT, EN8811_WRITE_1);
		if(ret == EN8811_ERROR)
		{
			goto CHECK_INPUT;
		}

		do{
			mdelay(100);
			ret = mbus->read(mbus, phyAddr, EN8811_AN_COMPLETE_REG);
			ret &= EN8811_AN_COMPLETE_BIT;
		}while(ret);
		printk("EN8811[%s] restartAN successly!\n\n", phyItfName);
	}
	else if(!strcmp(cmd, "fc"))
	{
		if(!strcmp(param, "open"))
		{
			ret = EN8811_cl22_op(phyDev, EN8811_FC_REG, EN8811_FC_BIT, EN8811_WRITE_1);
			if(ret == EN8811_ERROR)
			{
				goto CHECK_INPUT;
			}
			printk("EN8811[%s] fc opened successly!\n\n", phyItfName);
		}
		else if(!strcmp(param, "close"))
		{
			ret = EN8811_cl22_op(phyDev, EN8811_FC_REG, EN8811_FC_BIT, EN8811_WRITE_0);
			if(ret == EN8811_ERROR)
			{
				goto CHECK_INPUT;
			}
			printk("EN8811[%s] fc closed successly!\n\n", phyItfName);
		}
		else
			goto CHECK_INPUT;
	}
	else if(!strcmp(cmd, "2.5Gbps"))
	{
		ret = airphy_set_mode(phyDev, AIR_PORT_MODE_FORCE_2500);
		if(ret == EN8811_ERROR)
		{
			goto CHECK_INPUT;
		}
		printk("EN8811[%s] set 2.5Gbps successly!\n\n", phyItfName);
	}
	else if(!strcmp(cmd, "1Gbps"))
	{
		ret = airphy_set_mode(phyDev, AIR_PORT_MODE_FORCE_1000);
		if(ret == EN8811_ERROR)
		{
			goto CHECK_INPUT;
		}
		printk("EN8811[%s] set 1Gbps successly!\n\n", phyItfName);
	}
	else if(!strcmp(cmd, "100Mbps"))
	{
		ret = airphy_set_mode(phyDev, AIR_PORT_MODE_FORCE_100);
		if(ret == EN8811_ERROR)
		{
			goto CHECK_INPUT;
		}
		printk("EN8811[%s] set 100Mbps successly!\n\n", phyItfName);
	}
	else if(!strcmp(cmd, "Auto"))
	{
		ret = airphy_set_mode(phyDev, AIR_PORT_MODE_AUTONEGO);;
		if(ret == EN8811_ERROR)
		{
			goto CHECK_INPUT;
		}
		printk("EN8811[%s] set auto successly!\n\n", phyItfName);
	}
	else
		goto CHECK_INPUT;

	return count;
CHECK_INPUT:
		pr_err("Please check the input parameter!\n\n");
		return -EINVAL;
}

static ssize_t en8811_power_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char *kbuf;
	size_t kbuf_size = 1024;
	
	if (*ppos > 0) {
		return 0; /* End of file */
	}

	kbuf = mem_alloc(kbuf_size);
	if(!CHECK_POINTER(kbuf)){
		return -ENOMEM;
	}

	len += snprintf(kbuf + len, kbuf_size - len, "Set Power on : echo power on	[itf=phy_itfname] > /proc/en8811_power \n");
	len += snprintf(kbuf + len, kbuf_size - len, "Set Power off: echo power off [itf=phy_itfname] > /proc/en8811_power \n\n");

	if (*ppos >= len)
	{
		mem_free(kbuf);
		return 0;
	}

	if (count > len - *ppos)
	{
		count = len - *ppos;
	}

	if (copy_to_user(buf, kbuf + *ppos, count)) {
		mem_free(kbuf);
		return -EFAULT;
	}
	*ppos += count;
	
	mem_free(kbuf);
	return count;
}

static ssize_t en8811_power_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int ret = 0;
	char cmd[CMD_LEN] = {0}, param[CMD_LEN] = {0};
	struct phy_device* phyDev = NULL;
	PARSE_DATA_T parse_data;
	Mdio_priv_data_t *mdio_priv_data_p = NULL;
	char *phyItfName = NULL;

	mdio_priv_data_p = hsgmii_priv_p;
	if(!CHECK_PHY_PRIV(mdio_priv_data_p))
	{
		return count;
	}
	
	memset(&parse_data, 0, sizeof(PARSE_DATA_T));
	ret = EN8811_check_writeproc_para(buf, count, &parse_data, mdio_priv_data_p);
	if(ret){
		return ret;
	}else{
		phyItfName = parse_data.phyItfName;
		phyDev = parse_data.phyDev;
		strcpy(cmd,parse_data.cmd);
		strcpy(param,parse_data.param);
	}

	if(!strcmp(cmd, "power"))
	{
		if(!strcmp(param, "on"))
		{
			ret = airphy_set_mode(phyDev, AIR_PORT_MODE_POWER_UP);
			if(ret == EN8811_ERROR)
			{
				goto CHECK_INPUT;
			}
			printk("EN8811[%s] power on successly!\n\n",phyItfName);
		}
		else if(!strcmp(param, "off"))
		{
			ret = airphy_set_mode(phyDev, AIR_PORT_MODE_POWER_DOWN);
			if(ret == EN8811_ERROR)
			{
				goto CHECK_INPUT;
			}
			printk("EN8811[%s] power off successly!\n\n",phyItfName);
		}
		else if(!strcmp(param, "state"))
		{
			ret = airphy_set_mode(phyDev, AIR_PORT_MODE_POWER_STATE);
			if(ret == EN8811_ERROR)
			{
				goto CHECK_INPUT;
			}
			printk("EN8811[%s] power down state: %s\n\n",phyItfName, ret ? "off" : "on");
		}
		else
			goto CHECK_INPUT;
	}
	else
		goto CHECK_INPUT;

	return count;

CHECK_INPUT:
		pr_err("Please check the input parameter!\n");
		return -EINVAL;
}

static const struct proc_ops proc_debug_level_fops = {
	.proc_read	= en8811_debug_level_read_proc,
	.proc_write	= en8811_debug_level_write_proc,
};

static const struct proc_ops proc_serdes_fops = {
	.proc_read	= en8811_serdes_info_read_proc,
};

static const struct proc_ops proc_link_st_fops = {
	.proc_read	= en8811_link_state_read_proc,
};

static const struct proc_ops proc_speed_mode_fops = {
	.proc_read	= en8811_speed_mode_read_proc,
	.proc_write	= en8811_speed_mode_write_proc,
};

static const struct proc_ops proc_power_fops = {
	.proc_read	= en8811_power_read_proc,
	.proc_write	= en8811_power_write_proc,
};

int en8811_proc_init(void)
{
	struct proc_dir_entry *en8811_proc;

	/* set debug level */
    en8811_proc = proc_create("en8811_debug", 0, NULL, &proc_debug_level_fops);
    en8811_proc = proc_create("en8811_serdes", 0, NULL, &proc_serdes_fops);
    en8811_proc = proc_create("en8811_link_st", 0, NULL, &proc_link_st_fops);
    en8811_proc = proc_create("en8811_speed_mode", 0, NULL, &proc_speed_mode_fops);
    en8811_proc = proc_create("en8811_power", 0, NULL, &proc_power_fops);

	pr_info("en8811_proc init ok!\n");
	return 0;
}

int en8811_proc_exit(void)
{
    remove_proc_entry("en8811_debug", 0);
	remove_proc_entry("en8811_serdes", 0);
    remove_proc_entry("en8811_link_st", 0);
    remove_proc_entry("en8811_speed_mode", 0);
    remove_proc_entry("en8811_power", 0);
    
	pr_info("en8811_proc exit ok!\n");
	return 0;
}

static inline char* GET_PHY_ITFNAME(uint8_t serdesID)
{
    if ( serdesID >= 0 && serdesID < SERDES_MAX_IDX){
        return itfname_info[serdesID];
    }
    else{
        return INVALID_ITF_NAME;
    }
} 

static inline char* get_ifname_by_phy_addr(int phy_addr)
{
    int i;

    for (i = 0; i < SERDES_MAX_IDX; i++) {
        if (serdes_map_aeon[i].phy_addr == phy_addr) {
            return itfname_info[serdes_map_aeon[i].serdes_id];
        }
    }
    return INVALID_ITF_NAME;
}

static void get_serdesInfo_from_map(uint8_t phyAddr, uint8_t* serdesID, char** serdesName)
{
    uint8_t i = 0;

    for(i = 0; i< PHYSERDES_MAP_SIZE; i++){
        if(GET_SERDES_PHYADDR(i) == phyAddr){
            *serdesID = GET_SERDES_ID(i);
            *serdesName = GET_SERDES_NAME(*serdesID);
            return;
        }
    }
    *serdesID = SERDES_INVALID_IDX;
    *serdesName = INVALID_ITF_NAME;
}

static void set_en8811_phydev(Mdio_priv_data_t *mdio_priv_data_p, struct phy_device *phyDev)
{
    uint8_t phyAddr = INVALID_PHY_ADDR, serdesID = SERDES_INVALID_IDX, phyNum = 0;
    char *serdesName = NULL;
    uint32_t phy_id = 0;
    PHY_STATUS_t phy_status = EN8811_PHY_OFF;

    if(!CHECK_PHY_PRIV(mdio_priv_data_p)){
        return;
    }
    phyNum = MDIO_PHY_NUM(mdio_priv_data_p);
    if(!CHECK_PHY_DEV(phyDev)){
        return;
    }
    phyAddr = phyDev->mdio.addr;
    phy_id = phyDev->phy_id;
    if(!CHECK_PHY_ADDR(phyAddr)){
        return;
    }

    if(phy_id == 0){
        EN8811_LOG(EN8811_LOG_WARNING,"PHY[%hhu]:INVALID phy_id\n",phyAddr);
        return;
    }
    get_serdesInfo_from_map(phyAddr,&serdesID,&serdesName);
    
    PHY_DEV_PTR(mdio_priv_data_p, phyAddr) = phyDev;
    PHY_DEV_ADDR(mdio_priv_data_p, phyAddr) = phyAddr;
    PHY_DEV_ID(mdio_priv_data_p, phyAddr) = phy_id;
    PHY_STATUS(mdio_priv_data_p, phyAddr) = phy_status;
    MDIO_REG_ADDR(mdio_priv_data_p, phyNum) = phyAddr;
    SET_SERDES_ID_SYNC(mdio_priv_data_p, phyAddr, serdesID);
    (MDIO_PHY_NUM(mdio_priv_data_p))++;
    if(MDIO_PHY_DEFAULT(mdio_priv_data_p) == INVALID_PHY_ADDR){
        MDIO_PHY_DEFAULT(mdio_priv_data_p) = phyAddr;
    }

    pr_info("Adding[%hhu]: PHY[%hhu], %s\n",MDIO_PHY_NUM(mdio_priv_data_p),phyAddr,serdesName);
}

static uint8_t scan_register_phydev(Mdio_priv_data_t *mdio_priv_data_p)
{
    struct mii_bus *bus;
    struct phy_device *phydev;
    uint8_t phyAddr = INVALID_PHY_ADDR;

    bus = MDIO_BUS_PRT(mdio_priv_data_p);
    if(bus == NULL){
        pr_warn("bus is NULL\n");
        return 0;
    }

	/*scan 8811*/    
    for(phyAddr = MIN_PHY_ADDR_EN8811; phyAddr <= MAX_PHY_ADDR_EN8811; phyAddr++){
        phydev = mdiobus_get_phy(bus, phyAddr);
        pr_info("phydev[%hhu] = %p\n", phyAddr, phydev);
        if(phydev){
			en8811_is_exist = 1;
            set_en8811_phydev(mdio_priv_data_p,phydev);          
        }
    }

	/*scan 8831*/
    for(phyAddr = MIN_PHY_ADDR_AN8831; phyAddr <= MAX_PHY_ADDR_AN8831; phyAddr++){
        phydev = mdiobus_get_phy(bus, phyAddr);
        pr_info("phydev[%hhu] = %p\n", phyAddr, phydev);
        if(phydev){
			an8831_is_exist = 1;
            an8831_dev_all[dev_cnt] = phydev;
    		dev_cnt += 1;        
        }
    }

    if(en8811_is_exist == 0 && an8831_is_exist == 0){
        pr_warn("not find any phy device \n");
        return 0;
    }
    return 1;
}

static void print_serdes_mapinfo(Mdio_priv_data_t *mdio_priv_data_p)
{
    uint8_t new_serdesID = SERDES_INVALID_IDX,  serdesID = SERDES_INVALID_IDX;
    uint8_t phyAddr = INVALID_PHY_ADDR;
    char* serdesName = "";
    char* phy_itf_name = "";
    char addr_str[5];

    printk("-------Serdes Map Info:-------\n");
    printk("%-5s   %-4s     %-10s\n","name","addr","serdes");
    for(serdesID = 0; serdesID < serdes_map_by_ordes_num; serdesID++){
        new_serdesID = serdes_map_by_ordes[serdesID];

        phyAddr = GET_SERDES_PHYADDR(new_serdesID);
        if(phyAddr == 0){
            strncpy(addr_str, " -- ", sizeof(addr_str));
            addr_str[sizeof(addr_str)-1] = '\0';
        }
        else{
            snprintf(addr_str, sizeof(addr_str), "%02d", phyAddr);
        }

        serdesName = GET_SERDES_NAME(new_serdesID);
        phy_itf_name = GET_PHY_ITFNAME(new_serdesID);
        printk("%-6s  %-5s  %-1s\n",
                        (strncmp(phy_itf_name,INVALID_ITF_NAME,4) == 0) ? " --- ":phy_itf_name,
                        addr_str,
                        serdesName);
    }
    printk("-----------------------------\n\n");
}

static void init_en8811_priv_data(Mdio_priv_data_t *mdio_priv_data_p)
{
    uint8_t i = 0;
    
    for(i = MIN_PHY_ADDR_EN8811; i <= MAX_PHY_ADDR_EN8811; i++){
        PHY_DEV_PTR(mdio_priv_data_p,i) = NULL;
        PHY_DEV_ADDR(mdio_priv_data_p,i) = INVALID_PHY_ADDR;
        PHY_SERDES_ID(mdio_priv_data_p,i) = SERDES_INVALID_IDX;
        PHY_DEV_ID(mdio_priv_data_p,i) = 0;
        PHY_STATUS(mdio_priv_data_p,i) = 0;
        MDIO_REG_ADDR(mdio_priv_data_p,i) = INVALID_PHY_ADDR;
        MDIO_REG_SDID(mdio_priv_data_p,i) = SERDES_INVALID_IDX;
    }
    MDIO_PHY_NUM(mdio_priv_data_p) = 0;
    MDIO_PHY_DEFAULT(mdio_priv_data_p) = INVALID_PHY_ADDR;
}

void* mem_alloc(int size)
{
    void* ptr = NULL;
    
    if (size > 0)
    {
        ptr = kmalloc(size, GFP_KERNEL);
        if (ptr != NULL)
        {
            memset(ptr,0,size);
            EN8811_LOG(EN8811_LOG_DBG,"Memory allocation successful, size = %d\n",size);
        }
        else{
            EN8811_LOG(EN8811_LOG_ERROR,"Memory allocation failed, size = %d\n",size);
        }
    }
    
    return ptr;
} 

void mem_free(void* ptr)
{
    if (ptr)
    {
        kfree(ptr);
        ptr = NULL;
        EN8811_LOG(EN8811_LOG_DBG,"Memory freed successfully\n");
    } 
}

uint8_t init_hsgmii_priv(Mdio_priv_data_t *mdio_priv_data_p)
{
    uint8_t ret = 0;

    init_en8811_priv_data(mdio_priv_data_p);
 
    /*Save has registered phy device*/
    ret = scan_register_phydev(mdio_priv_data_p);
    if(ret == 0){
        pr_err("scan_register_phydev failed!\n");
		return 0;
    }
    else{
        pr_info("Initializing mdio_priv_data_p ok!\n");
    }

    return 1;
}

void print_phy_registeredInfo(Mdio_priv_data_t *mdio_priv_data_p)
{
    uint8_t default_serdesID = SERDES_INVALID_IDX;
    uint8_t default_phyAddr = 0;
    uint8_t registered_phy_num = 0;

    if(!CHECK_PHY_PRIV(mdio_priv_data_p)){
        return;
    }

    print_serdes_mapinfo(mdio_priv_data_p);

    registered_phy_num = MDIO_PHY_NUM(mdio_priv_data_p);
    if(registered_phy_num == 0){
        printk("nothing of registerd phy devices\n");
        return;
    }

    default_phyAddr = MDIO_PHY_DEFAULT(mdio_priv_data_p);
    default_serdesID = MDIO_REG_SDID(mdio_priv_data_p,default_phyAddr);
    printk("Default phyDev address: %hhu, Serdes name:%s ,Interface name:%s\n\n",
                    default_phyAddr,GET_SERDES_NAME(default_serdesID),GET_PHY_ITFNAME(default_serdesID));
}

void print_phy_privdataInfo(Mdio_priv_data_t *mdio_priv_data_p)
{
    uint8_t i = 0;
    
    if(!CHECK_PHY_PRIV(mdio_priv_data_p)){
        return;
    }

    pr_info("-------Dump EN8811 Private Data-------\n");
    pr_info("mii_bus: %s\n", MDIO_BUS_PRT(mdio_priv_data_p) ? "Full" : "None");
    for(i = MIN_PHY_ADDR_EN8811; i <= MAX_PHY_ADDR_EN8811; i++){
        pr_info("---phy_devices[%hhu]: %s---\n", i, PHY_DEV_PTR(mdio_priv_data_p,i) ? "Full" : "None");
        if(PHY_STATUS(mdio_priv_data_p,i) == EN8811_PHY_ON){
            pr_info("phy status: ON\n");
        }
        else{
            pr_info("phy status: OFF\n");
        }

        if(PHY_DEV_ADDR(mdio_priv_data_p,i) == INVALID_PHY_ADDR){
            pr_info("phy_addr : None\n");
        }
        else{
            pr_info("phy_addr : %hhu\n", PHY_DEV_ADDR(mdio_priv_data_p,i));
        }

        if(PHY_SERDES_ID(mdio_priv_data_p,i) == SERDES_INVALID_IDX){
            pr_info("serdes_id: None\n");
        }
        else{
            pr_info("serdes_id: %hhu\n", PHY_SERDES_ID(mdio_priv_data_p,i));
        }
        pr_info("phy_id   : 0x%04x\n", PHY_DEV_ID(mdio_priv_data_p,i));
    }
    print_phy_registeredInfo(mdio_priv_data_p);
}

uint8_t check_phy_itfname_valid(Mdio_priv_data_t *mdio_priv_data_p, char *itfname)
{
    int i, serdesID = SERDES_INVALID_IDX;

    for(i = MIN_PHY_ADDR_EN8811; i <= MAX_PHY_ADDR_EN8811; i++)
    {
        serdesID = MDIO_REG_SDID(mdio_priv_data_p,i);
        if(serdesID == SERDES_INVALID_IDX){
            continue;
        }
        if(strncmp(itfname,itfname_info[serdesID],STRING_LEN) == 0) {
            return EN8811_TRUE;
        }
    }
    pr_err("PhyItfname[%s] is invalid\n",itfname);
    return EN8811_FALSE;
}

uint8_t get_phyAddr_by_ItfName(char *iftname)
{
    uint8_t i = 0;

    for(i=0; i< MAX_SERDES_NUM; i++){
        if(strcmp(iftname, GET_PHY_ITFNAME(i)) == 0){
            return GET_SERDES_PHYADDR(i);
        }
    }

    return INVALID_PHY_ADDR;
}

struct phy_device* get_phyDev_by_ItfName(Mdio_priv_data_t *mdio_priv_data_p, char *iftname)
{
    uint8_t i = 0, phyAddr = 0;

    for(i=0; i< MAX_SERDES_NUM; i++){
        if(strcmp(iftname, GET_PHY_ITFNAME(i)) == 0){
            phyAddr = GET_SERDES_PHYADDR(i);
            return  GET_PHY_DEV(mdio_priv_data_p, phyAddr);    
        }
    }

    return NULL;
}

char* get_ItfName_by_phyAddr(Mdio_priv_data_t *mdio_priv_data_p, uint8_t phyAddr)
{
    uint8_t i = 0, serdesID = SERDES_INVALID_IDX;

    if(CHECK_PHY_ADDR(phyAddr) == EN8811_TRUE){
        for(i = MIN_PHY_ADDR_EN8811;  i <= MAX_PHY_ADDR_EN8811; i++){
            if(phyAddr == i){
                serdesID = MDIO_REG_SDID(mdio_priv_data_p,phyAddr);
                return itfname_info[serdesID];
            }
        }
    }

    return INVALID_ITF_NAME;
}

void get_default_ItfName(Mdio_priv_data_t *mdio_priv_data_p, char* phyItfName)
{
    const char *itfname_str;
    uint8_t default_phy_addr = 0;

    EN8811_LOG(EN8811_LOG_INFO,"use default phy device!\n");
    if(!MDIO_PHY_NUM(mdio_priv_data_p)){
        strscpy(phyItfName , INVALID_ITF_NAME,CMD_LEN);
        return;
    }

    default_phy_addr = MDIO_PHY_DEFAULT(mdio_priv_data_p);
    itfname_str = get_ItfName_by_phyAddr(mdio_priv_data_p,default_phy_addr);
    strscpy(phyItfName , itfname_str,CMD_LEN);
    return;
}

unsigned short aeon_mdio_read_reg(struct phy_device *phydev, unsigned int reg_addr)
{
    unsigned short val = 0;
    unsigned short dev_addr = (reg_addr >> 17) & 0x1F;
    unsigned short phy_reg = (reg_addr >> 1) & 0xFFFF;
    val = phy_read_mmd(phydev, dev_addr, phy_reg); 
    return val;
}

void aeon_mdio_write_reg(struct phy_device *phydev, unsigned int reg_addr, unsigned short value)
{
    unsigned short dev_addr = (reg_addr >> 17) & 0x1F;
    unsigned short phy_reg = (reg_addr >> 1) & 0xFFFF;
    phy_write_mmd(phydev, dev_addr, phy_reg, value);
    return;
}

unsigned short aeon_mdio_read_reg_field(struct phy_device *phydev, unsigned int reg_addr, unsigned short field)
{
    unsigned short val = aeon_mdio_read_reg(phydev, reg_addr);
    unsigned short width = (field & 0xFF);
    unsigned short offset = ((field >> 8) & 0xFF);
    unsigned short mask = ((1 << width) - 1);
    return ((val >> offset) & mask);
}

void aeon_mdio_write_reg_field(struct phy_device *phydev, unsigned int reg_addr, unsigned short field, unsigned short value)
{
    unsigned short val = aeon_mdio_read_reg(phydev, reg_addr);
    unsigned short width = (field & 0xFF);
    unsigned short offset = ((field >> 8) & 0xFF);
    unsigned short mask = ((1 << width) - 1) << offset;
    val = (val & ~ mask) | ((value & ((1 << width) - 1)) << offset);
    aeon_mdio_write_reg(phydev, reg_addr, val);
    return;
}

void aeon_send_ipc_cmd(struct phy_device *phydev, unsigned short cmd)
{
    aeon_mdio_write_reg(phydev, IPC_CMD_BASEADDR, cmd);
    return;
}

void aeon_set_ipc_data_reg(struct phy_device *phydev, unsigned int len, unsigned short *val)
{
    int ii;
    if (len >= 8) len = 8;
    for (ii = 0; ii < len; ii++) {
        aeon_mdio_write_reg(phydev, IPC_DATA0_BASEADDR + 2 * ii, *(val + ii));
    }
    return;
}

unsigned short aeon_get_ipc_status(struct phy_device *phydev)
{
    unsigned short val;
    val = aeon_mdio_read_reg(phydev, IPC_STS_BASEADDR);

    return val;
}

void aeon_ipc_parse_sts(unsigned short sts, unsigned short *status,
                   unsigned short* opcode, unsigned short* size,
                   unsigned short *parity)
{
    /*
    * """Parse the 16-bit full status into components.
    * 16-bit status register is laid out as follows:
    * [1 parity][5 size][6 opcode][4 status]
    */
    unsigned short status_mask = (1 << IPC_NB_STATUS) - 1;
    unsigned short opcode_mask = (1 << IPC_NB_OPCODE) - 1;
    unsigned short size_mask = (1 << IPC_PAYLOAD_NB) - 1;
    // Clip off status bits
    *status = sts & status_mask;
    sts = (sts >> IPC_NB_STATUS);
    // Clip off opcode
    *opcode = (sts & opcode_mask);
    sts = (sts >> IPC_NB_OPCODE);
    // Clip off size
    *size = (sts & size_mask);
    sts = (sts >> IPC_PAYLOAD_NB);
    // Get parity bit
    *parity = sts & 1;
    return;
}

void aeon_match_device(unsigned short phy_addr, struct phy_device **phydev)
{
	unsigned int dev_addr = 0;
	int i = 0, flag = 0;

	for (i = 0; i < AN8831_PHY_NUM; i++) {
		if (an8831_dev_all[i] != NULL) {
			dev_addr = phydev_addr(an8831_dev_all[i]);
			if (dev_addr == phy_addr) {
				flag = 1;
				*phydev = an8831_dev_all[i];
				break;
			}
		} else {
			pr_err("error! The phy_dev you choosed is null!\n");
			return;
		}
	}
	if (flag == 0)
		pr_err("error! The phy_addr you choosed is wrong!\n");
}

void aeon_receive_ipc_data(struct phy_device *phydev, unsigned short len,
			   unsigned short *data)
{
	int ii;

	if (len > 8)
		len = 8;
	for (ii = 0; ii < len; ii++) {
		*(data + ii) = aeon_mdio_read_reg(phydev, IPC_DATA0_BASEADDR +
								  (ii << 1));
	}
}

void aeon_send_ipc_msg(struct phy_device *phydev, unsigned int len,
		       unsigned short *val, short opcode, short size)
{
	unsigned short cmd;

	aeon_set_ipc_data_reg(phydev, len, val);
	aeon_ipc_build_cmd(&cmd, opcode, size);
	aeon_send_ipc_cmd(phydev, cmd);
}

/* IPC Layer functions */
static unsigned int ipc_cmd_num;
unsigned int get_par(void)
{
	return ipc_cmd_num & 0x1;
}

void aeon_ipc_build_cmd(unsigned short *cmd, short opcode, short size)
{
	/*
 * """Construct the full command word.
 * 16-bit register is laid out as follows:
 * [1 cmd par][4 reserved][5 size][6 opcode]
 */

	unsigned short opcode_mask = (1 << IPC_NB_OPCODE) - 1;
	unsigned short size_mask = (1 << IPC_PAYLOAD_NB) - 1;
	unsigned short opcode_bits = opcode & opcode_mask;
	unsigned short size_bits = size & size_mask;
	unsigned short _cmd = 0;

	_cmd = (size_bits << IPC_NB_OPCODE) + opcode_bits;

	if (get_par() == 0)
		_cmd &= ~IPC_CMD_PARITY;
	else
		_cmd |= IPC_CMD_PARITY;

	*cmd = _cmd;
	ipc_cmd_num++;
}

unsigned short aeon_ipc_wait_cmd_done(struct phy_device *phydev,
				      unsigned long *ns,
				      unsigned short *ret_size)
{
	/*
    * """Wait until IPC status handshake returns DONE or READY.
    * timeout : seconds
    * Returns
    * -------
    *  status : int
    *  Return status:
    *  opcode : int
    *  serviced.
    *  ret_size : int
    *  Number of bytes in the return.
    */
	struct timespec64 t1, t2;
	unsigned long _to, _ns = 0;
	unsigned short sts, opcode, ret_par;
	unsigned short status = 0, par = 0;

	if (ns)
		_to = *ns;
	else
		_to = IPC_TIMEOUT;

	ktime_get_real_ts64(&t1);
	while ((par == 0) || ((status != IPC_STS_CMD_SUCCESS) &&
			      (status != IPC_STS_CMD_ERROR))) {
		mdelay(10);
		sts = aeon_get_ipc_status(phydev);
		aeon_ipc_parse_sts(sts, &status, &opcode, ret_size, &ret_par);
		par = (get_par() != ret_par);

		// Check return status
		if (status == IPC_STS_CMD_ERROR)
			break;

		// Check timeout
		ktime_get_real_ts64(&t2);
		_ns = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec -
		      t1.tv_nsec;
		if (_ns > _to)
			break;
	}

	return status;
}

void aeon_ipc_sync_parity(struct phy_device *phydev)
{
    unsigned long noop_to = 20;
    struct timespec64 t1, t2;
    unsigned long _to = IPC_TIMEOUT, _ns = 0;
    unsigned short cmd, par = 0;
    unsigned short sts, status, opcode, size, ret_par;
    struct device *dev = phydev_dev(phydev);

    // Send first noop, no need to wait reply
    aeon_ipc_build_cmd(&cmd, IPC_CMD_NOOP, 0);
    aeon_send_ipc_cmd(phydev, cmd);
    mdelay(noop_to);

    // Send second noop, expect the correct parity to return
    aeon_ipc_build_cmd(&cmd, IPC_CMD_NOOP, 0);
    aeon_send_ipc_cmd(phydev, cmd);
    par = 0;
    ktime_get_real_ts64(&t1);
    while (par == 0) {
        mdelay(10);
        sts = aeon_get_ipc_status(phydev);
        aeon_ipc_parse_sts(sts, &status, &opcode, &size, &ret_par);
        par = (get_par() != ret_par);

        // Check timeout
        ktime_get_real_ts64(&t2);
        _ns = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec;
        if (_ns > _to)
            break;
    }

    if (par == 0) {
        dev_err(dev, "IPC sync failure: NOOP 3, sts: %x\n", aeon_get_ipc_status(phydev));
    }
    return;
}

void aeon_ipc_get_fw_version(char *version, struct phy_device *phydev)
{
	unsigned short status, ret_size;
	unsigned short data = IPC_CMD_INFO_VERSION;
	struct device *dev = phydev_dev(phydev);

	aeon_send_ipc_msg(phydev, 1, &data, IPC_CMD_INFO, 2);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "get FW version command failed %x\n", status);
		return;
	}
	aeon_receive_ipc_data(phydev, 8, (unsigned short *)version);
}

void aeon_ipc_send_bulk_write(unsigned int mem_addr, unsigned int size,
			      struct phy_device *phydev)
{
	unsigned short status, ret_size;
	unsigned short msg[4] = { mem_addr & 0xffff, mem_addr >> 16,
				  size & 0xffff, size >> 16 };
	struct device *dev = phydev_dev(phydev);

	aeon_send_ipc_msg(phydev, 4, msg, IPC_CMD_BULK_WRITE, 8);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "IPC send bulk write command failed %x\n", status);
		return;
	}
}

void aeon_ipc_send_bulk_data(unsigned short bw_type, unsigned short size,
			     void *data, struct phy_device *phydev)
{
	unsigned short status, ret_size;
	struct device *dev = phydev_dev(phydev);
	// ipc data register is 16 bits, total 16 bytes per call.
	switch (bw_type) {
	case BW8:
		size = (size + 1) / 2;
		break;
	case BW32:
		size = size * 2;
		break;
	case BW16:
	default:
		break;
	}

	aeon_send_ipc_msg(phydev, size, (unsigned short *)data,
			  IPC_CMD_BULK_DATA, size * 2);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "IPC Bulk data command failed: %x\n", status);
		return;
	}
}

void aeon_ipc_cfg_param_direct(unsigned int data_len, unsigned short *data,
			       struct phy_device *phydev)
{
	unsigned short _data = IPC_CMD_CFG_DIRECT;
	unsigned short cmd, status, ret_size;
	struct device *dev = phydev_dev(phydev);

	aeon_set_ipc_data_reg(phydev, data_len + 1, data - 1);
	aeon_set_ipc_data_reg(phydev, 1, &_data);

	aeon_ipc_build_cmd(&cmd, IPC_CMD_CFG_PARAM, 2 * (data_len + 1));
	aeon_send_ipc_cmd(phydev, cmd);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "IPC cfg param direct return status: %x\n",
			status);
		return;
	}
}

void aeon_cu_an_set_top_spd(unsigned short top_spd, struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 3;

	data[0] = CFG_CU_AN;
	data[1] = IPC_CMD_CU_AN_TOP_SPD;
	data[2] = top_spd;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_cu_an_restart(struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 2;

	data[0] = CFG_CU_AN;
	data[1] = MDI_CFG_CU_AN_RESTART;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_cu_an_set_eee_spd(unsigned short speed_mode,
			    struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 3;

	data[0] = CFG_CU_AN;
	data[1] = MDI_CFG_CU_AN_EEE_SPD;
	data[2] = speed_mode;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_cu_an_set_trd_swap(unsigned short en, unsigned short trd_swap, struct phy_device *phydev)
{
  unsigned short data[8], reg_num = 4;
  
  data[0] = CFG_CU_AN;
  data[1] = IPC_CMD_CU_AN_TRD_SWAP;
  data[2] = en;
  data[3] = trd_swap;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_cu_an_set_ms_cfg(unsigned short port_type, unsigned short ms_man_en,
			   unsigned short ms_man_val, struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 5;

	data[0] = CFG_CU_AN;
	data[1] = IPC_CMD_CU_AN_MS_CFG;
	data[2] = port_type;
	data[3] = ms_man_en;
	data[4] = ms_man_val;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_cu_an_get_ms_cfg(unsigned short *ms_related_cfg,
			   struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 2;

	data[0] = CFG_CU_AN;
	data[1] = IPC_CMD_CU_AN_GET_MS_CFG;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
	aeon_receive_ipc_data(phydev, 3, (unsigned short *)ms_related_cfg);
}

void aeon_cu_an_set_cfr(unsigned short cfr, struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 3;

	data[0] = CFG_CU_AN;
	data[1] = IPC_CMD_CU_AN_CFR;
	data[2] = cfr;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_cu_an_set_fast_retrain(unsigned short speed_mode,
				 unsigned short thp_bypass,
				 struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 4;

	data[0] = CFG_CU_AN;
	data[1] = MDI_CFG_CU_AN_FR_SPD;
	data[2] = speed_mode >> 4;
	data[3] = thp_bypass;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_cu_an_enable_downshift(unsigned short enable,
				 unsigned short retry_limit,
				 struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 4;

	data[0] = CFG_CU_AN;
	data[1] = MDI_CFG_CU_AN_DS;
	data[2] = enable;
	data[3] = retry_limit;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_sds_pcs_set_cfg(unsigned short pcs_mode, unsigned short sds_spd,
			  struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 4;
	// the order should match firmware cfg parameter sequence.
	data[0] = CFG_SDS_PCS;
	data[1] = CFG_SDS_PCS;
	data[2] = pcs_mode;
	data[3] = sds_spd;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_sds_pma_set_cfg(unsigned short vga_adapt, unsigned short slc_adapt,
                          unsigned short ctle_adapt, unsigned short dfe_adapt,
                          struct phy_device *phydev)
{
  unsigned short data[8], reg_num = 6;
  // the order should match firmware cfg parameter sequence.
  data[0] = CFG_SDS_PMA;
  data[1] = CFG_SDS_PMA;
  data[2] = vga_adapt;
  data[3] = slc_adapt;
  data[4] = ctle_adapt;
  data[5] = dfe_adapt;

  aeon_ipc_cfg_param_direct(reg_num, data, phydev);
  return;
}

void aeon_cu_an_enable_aeon_oui(unsigned short nstd_pbo,
				struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 3;

	data[0] = CFG_CU_AN;
	data[1] = MDI_CFG_CU_AN_AEON_OUI;
	data[2] = nstd_pbo;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_auto_eee_cfg(unsigned short enable, unsigned int idle_th,
		       struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 5;

	data[0] = CFG_AUTO_EEE;
  	data[1] = CFG_AUTO_EEE;
	data[2] = enable;
	data[3] = idle_th & 0xFFFF;
	data[4] = (idle_th >> 16) & 0xFFFF;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_ipc_temp_monitor(unsigned short sub_cmd, unsigned short params,
			   unsigned short *temperature,
			   struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 3;

	data[0] = CFG_TEMP_MON;
	data[1] = sub_cmd;
	data[2] = params;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
	if (sub_cmd == 0x4)
		aeon_receive_ipc_data(phydev, 3, (unsigned short *)temperature);
}

void aeon_dpc_ra_enable(struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 1;

	data[0] = CFG_DPC_RA;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_ipc_set_led_cfg(unsigned short led0, unsigned short led1,
			  unsigned short led2, unsigned short led3,
			  unsigned short led4, unsigned short polarity,
			  unsigned short blink, struct phy_device *phydev)
{
	unsigned short status, ret_size;
	unsigned short cfg[7] = {
		led0, led1, led2, led3, led4, polarity, blink
	};
	struct device *dev = phydev_dev(phydev);

	aeon_send_ipc_msg(phydev, 7, cfg, IPC_CMD_SET_LED, 14);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "set led command failed %x\n", status);
		return;
	}
}

int aeon_read_status(struct phy_device *phydev)
{
	int ret = 0, reg = 0;
	char hcd_status = 0;
	struct device *dev = phydev_dev(phydev);

	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 1;
	phydev->asym_pause = 1;

	reg = aeon_mdio_read_reg(phydev, AN_REG_GIGA_STD_STATUS_BASEADDR);
	if (reg < 0) {
		dev_err(dev, "MII_BMSR reg %d!\n", reg);
		return reg;
	}
	if (reg & BMSR_LSTATUS) {
		phydev->link = 1;
		hcd_status = aeon_mdio_read_reg_field(phydev, 0xF0010, 0x804);
		switch (hcd_status) {
		case 0xE:
			phydev->speed = SPEED_10000;
			phydev->duplex = DUPLEX_FULL;
			break;
		case 0xD:
			phydev->speed = SPEED_5000;
			phydev->duplex = DUPLEX_FULL;
			break;
		case 0xC:
			phydev->speed = SPEED_2500;
			phydev->duplex = DUPLEX_FULL;
			break;
		case 0xB:
			phydev->speed = SPEED_1000;
			phydev->duplex = DUPLEX_FULL;
			break;
		case 0xA:
			phydev->speed = SPEED_100;
			phydev->duplex = DUPLEX_FULL;
			break;
		case 0x3:
			phydev->speed = SPEED_1000;
			phydev->duplex = DUPLEX_HALF;
			break;
		case 0x2:
			phydev->speed = SPEED_100;
			phydev->duplex = DUPLEX_HALF;
			break;
		default:
			break;
		}
	} else {
		phydev->link = 0;
	}
	return ret;
}

void aeon_ipc_set_sys_reboot(struct phy_device *phydev)
{
	unsigned short data = IPC_CMD_SYS_REBOOT;

	aeon_send_ipc_msg(phydev, 1, &data, IPC_CMD_SYS_CPU, 2);
}

void aeon_pkt_chk_cfg(unsigned short enable, struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 4;

	data[0] = CFG_DPC_PKT_CHK;
  	data[1] = CFG_DPC_PKT_CHK;
	data[2] = enable;
	data[3] = 0;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_sds_wait_eth_cfg(unsigned short sds_wait_eth_delay,
			   struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 5;

	data[0] = CFG_DPC_SDS_WAIT_ETH;
  	data[1] = CFG_DPC_SDS_WAIT_ETH;
	data[2] = sds_wait_eth_delay;
	data[3] = 2;
	data[4] = 1;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_ipc_phy_enable_mode(unsigned short enable, struct phy_device *phydev)
{
	unsigned short status, ret_size;
	unsigned short cfg[2] = { IPC_CMD_SYS_CPU_PHY_ENABLE, enable };
	struct device *dev = phydev_dev(phydev);

	aeon_send_ipc_msg(phydev, 2, cfg, IPC_CMD_SYS_CPU, 4);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "IPC set phy return status: %x\n", status);
		return;
	}
}

#ifdef DUAL_FLASH
int aeon_ipc_sys_cpu_info(unsigned short sub_cmd, unsigned int flash_addr,
			  unsigned int mem_addr, struct phy_device *phydev)
{
	unsigned short status, ret_size;
	unsigned short cfg[5] = { 0 };
	int val = 0;
	struct device *dev = phydev_dev(phydev);

	cfg[0] = sub_cmd;
	if (sub_cmd == IPC_CMD_SYS_CPU_IMAGE_CHECK) {
		cfg[1] = flash_addr & 0xFFFF;
		cfg[2] = (flash_addr >> 16) & 0xFFFF;
		cfg[3] = mem_addr & 0xFFFF;
		cfg[4] = (mem_addr >> 16) & 0xFFFF;
	}

	aeon_send_ipc_msg(phydev, 5, cfg, IPC_CMD_SYS_CPU, 10);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "spu info command failed %x\n", status);
		return 1;
	}
	if (sub_cmd == IPC_CMD_SYS_CPU_IMAGE_CHECK) {
		aeon_receive_ipc_data(phydev, 1, cfg);
		val = cfg[0];
	} else if (sub_cmd == IPC_CMD_SYS_IMAGE_OFST) {
		aeon_receive_ipc_data(phydev, 2, cfg);
		val = cfg[0] + (cfg[1] << 16);
	}
	return val;
}

void aeon_ipc_set_fsm_mode(unsigned short fsm, unsigned short mode,
			   struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 4;

	data[0] = CFG_NG_PHYCTRL;
	data[1] = IPC_CMD_CFG_PHYCTRL_PAUSED;
	data[2] = fsm;
	data[3] = mode;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_ipc_write_flash(unsigned int flash_addr, unsigned int mem_addr,
			  unsigned short size, struct phy_device *phydev)
{
	unsigned short status, ret_size;
	unsigned short cfg[5] = { 0 };
	struct device *dev = phydev_dev(phydev);

	cfg[0] = flash_addr & 0xFFFF;
	cfg[1] = flash_addr >> 16;
	cfg[2] = mem_addr & 0xFFFF;
	cfg[3] = mem_addr >> 16;
	cfg[4] = size;

	aeon_send_ipc_msg(phydev, 5, cfg, IPC_CMD_FLASH_WRITE, 10);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "write flash command failed %x\n", status);
		return;
	}
}

void aeon_ipc_erase_flash(unsigned int flash_addr, unsigned int size,
			  unsigned short mode, struct phy_device *phydev)
{
	unsigned short status, ret_size;
	unsigned long ts = 6000000000;
	unsigned long *_to = &ts;
	unsigned short cfg[5] = { 0 };
	struct device *dev = phydev_dev(phydev);

	if ((flash_addr + size) >= FLASH_CHIP_SIZE)
		size = FLASH_CHIP_SIZE - flash_addr - 1;

	cfg[0] = flash_addr & 0xFFFF;
	cfg[1] = flash_addr >> 16;
	cfg[2] = size & 0xFFFF;
	cfg[3] = size >> 16;
	cfg[4] = mode;

	aeon_send_ipc_msg(phydev, 5, cfg, IPC_CMD_FLASH_ERASE, 10);
	status = aeon_ipc_wait_cmd_done(phydev, _to, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "erase flash command failed %x\n", status);
		return;
	}
}

void aeon_update_flash(const char *firmware, unsigned int flash_start,
		       struct phy_device *phydev)
{
	int sector_ofst, total, ret, data_ofst;
	unsigned char buf[FLASH_SECTOR_SIZE] = { 0 };
	unsigned short *wdata = (unsigned short *)buf;
	unsigned int temp_mem_addr = 0x33e000, flash_addr, image_size;
	unsigned short crc, dlen;
	const struct firmware *fw;
	struct device *dev = phydev_dev(phydev);

	aeon_ipc_set_fsm_mode(CFG_FSM_NGPHY, 0, phydev);
	aeon_ipc_set_fsm_mode(CFG_FSM_GPHY, 0, phydev);
	aeon_ipc_set_fsm_mode(CFG_FSM_CU_AN, 0, phydev);
	aeon_ipc_set_fsm_mode(CFG_FSM_DPC, 0, phydev);
	aeon_ipc_set_fsm_mode(CFG_FSM_SS, 0, phydev);

	ret = request_firmware_direct(&fw, firmware, dev);
	if (ret < 0) {
		dev_err(dev, "failed to load flash bin %s, ret: %d\n", firmware,
			ret);
		return;
	}
	crc = ~crc32(~0, fw->data, fw->size);
	dev_info(dev, "%s: crc32=0x%x\n", firmware, crc);
	// pad length so that fsm won't stuck at read back
	image_size = (fw->size + 3) & 0xFFFFFFFC;

	// erase first
	sector_ofst = 0;
	aeon_ipc_erase_flash(flash_start, image_size, ERASE_MODE_BLOCK, phydev);

	while ((sector_ofst << 12) < image_size) {
		flash_addr = flash_start + FLASH_SECTOR_SIZE * sector_ofst;
		dlen = 0;
		total = (image_size - (sector_ofst << 12)) >> 1;
		data_ofst = sector_ofst * FLASH_SECTOR_SIZE;
		memcpy(buf, fw->data + data_ofst, FLASH_SECTOR_SIZE);

		if (total > (FLASH_SECTOR_SIZE >> 1))
			total = (FLASH_SECTOR_SIZE >> 1);

		dev_info(dev, "sector_ofst : %u", sector_ofst);
		dev_info(dev, "  data_ofst : 0x%x\n", data_ofst);
		dev_info(dev, "flash_addr : 0x%x\n", flash_addr);

		dev_info(dev, "Origin params : %u  %u  %u  %u  %u  %u  %u  %u\n",
		       *(wdata), *(wdata + 1), *(wdata + 2), *(wdata + 3),
		       *(wdata + 4), *(wdata + 5), *(wdata + 6), *(wdata + 7));

		aeon_ipc_send_bulk_write(temp_mem_addr, FLASH_SECTOR_SIZE,
					 phydev);
		// upload to system memory
		while (dlen < total) {
			if ((total - dlen) > 8) {
				aeon_ipc_send_bulk_data(BW16, 8, wdata + dlen,
							phydev);
				dlen += 8;
			} else if ((total - dlen) > 0) {
				aeon_ipc_send_bulk_data(BW16, total - dlen,
							wdata + dlen, phydev);
				dlen = total;
			}
		}
		sector_ofst++;

		// write to flash
		aeon_ipc_write_flash(flash_addr, temp_mem_addr,
				     FLASH_SECTOR_SIZE, phydev);
	}
	release_firmware(fw);
}

void aeon_burn_image(unsigned char include_bootloader,
		     struct phy_device *phydev)
{
	unsigned int new_addr = 0, old_addr = 0;
	struct device *dev = phydev_dev(phydev);
	int ofst;
	// Disable WDT
	aeon_ipc_set_wdt(0, phydev);
	if (include_bootloader == 0) {
		ofst = aeon_ipc_sys_cpu_info(IPC_CMD_SYS_IMAGE_OFST, new_addr,
					     old_addr, phydev);
		if ((ofst == 0) || (ofst == IMAGE2_OFST)) {
			new_addr = IMAGE1_HDR_OFST;
			old_addr = IMAGE2_HDR_OFST;
		} else if (ofst == IMAGE1_OFST) {
			new_addr = IMAGE2_HDR_OFST;
			old_addr = IMAGE1_HDR_OFST;
		}
		dev_info(dev, "new_addr: %u, old_addr : %u\n", new_addr, old_addr);
		aeon_update_flash(FLASH_BIN, new_addr, phydev);
		ofst = aeon_ipc_sys_cpu_info(IPC_CMD_SYS_CPU_IMAGE_CHECK,
					     new_addr, 0x33d000, phydev);
		if (ofst) {
			aeon_update_flash(CLR_FLASH_IMAGE, old_addr, phydev);
		} else {
			dev_err(dev, "check image failed\n");
			return;
		}
	} else {
		aeon_update_flash(BOOT_LOADER_BIN, new_addr, phydev);
	}
	// Enable WDT
	aeon_ipc_set_wdt(1, phydev);
}
#endif

void aeon_sds_restart_an(struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 1;

	data[0] = CFG_SDS_RESTART_AN;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_ipc_ng_test_mode(unsigned short test_mode, unsigned short tone,
			   struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 4;

	data[0] = CFG_NG_PHYCTRL;
	data[1] = IPC_CMD_CFG_NG_TESTMODE;
	data[2] = test_mode;
	data[3] = tone;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_cu_an_enable(unsigned short enable, struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 3;

	data[0] = CFG_CU_AN;
	data[1] = MDI_CFG_CU_AN_ENABLE;
	data[2] = enable;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_set_man_mdi(struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 4;

	data[0] = CFG_CU_AN;
	data[1] = MDI_CFG_MAN_MDI;
	data[2] = 1;
	data[3] = MDI;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_set_man_duplex(unsigned short duplex, struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 4;

	data[0] = CFG_CU_AN;
	data[1] = MDI_CFG_CU_AN_DUPLEX;
	data[2] = 1;
	data[3] = duplex;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_ng_test_mode(unsigned short top_spd, unsigned short test_mode,
		       unsigned short tone, struct phy_device *phydev)
{
	unsigned short ms = 1;

	aeon_ipc_ng_test_mode(0, 0, phydev);
	// switch speed
	aeon_cu_an_set_top_spd(top_spd, phydev);
	// enable AN
	aeon_cu_an_enable(1, phydev);
	// restart AN
	aeon_cu_an_restart(phydev);
	mdelay(500);
	if (test_mode == 3)
		ms = 0;
	aeon_cu_an_set_ms_cfg(0, 1, ms, phydev);
	aeon_set_man_mdi(phydev);
	aeon_ipc_ng_test_mode(test_mode, tone, phydev);
	// disable AN
	aeon_cu_an_enable(0, phydev);
	aeon_mdio_write_reg_field(phydev, 0xF014E, 0x4, 0);
	aeon_mdio_write_reg_field(phydev, 0xF014C, 0xC01, 1);
	udelay(50);
	aeon_mdio_write_reg_field(phydev, 0xF014C, 0xC01, 0);
}

void aeon_1g_test_mode(unsigned short test_mode, struct phy_device *phydev)
{
	unsigned short ms = 1;
	// enable AN
	aeon_cu_an_enable(1, phydev);
	aeon_mdio_write_reg_field(phydev, 0xFFFD2, 0xD03, 0);
	// restart AN
	aeon_cu_an_restart(phydev);
	mdelay(500);
	if (test_mode == 3)
		ms = 0;
	aeon_cu_an_set_ms_cfg(0, 1, ms, phydev);
	aeon_set_man_mdi(phydev);
	// switch speed
	aeon_cu_an_set_top_spd(MDI_CFG_SPD_T1G, phydev);
	aeon_mdio_write_reg_field(phydev, 0xFFFD2, 0xD03, test_mode);
	// disable AN
	aeon_cu_an_enable(0, phydev);
}

void aeon_man_configure(struct phy_device *phydev)
{
	unsigned short coeffs[12] = { 50, 200, 250, 250, 200, 50,
				      0,  0,   0,   0,   0,   0 };
	int i, j;

	aeon_mdio_write_reg_field(phydev, 0x3C208C, 0xB, 0x20);
	aeon_mdio_write_reg_field(phydev, 0x3C2002, 0x106, 6);
	aeon_mdio_write_reg_field(phydev, 0x3C2078, 0x306, 4);
	aeon_mdio_write_reg_field(phydev, 0xF0026, 0xC01, 1);
	aeon_mdio_write_reg_field(phydev, 0x3C201E, 0x201, 1);

	aeon_mdio_write_reg_field(phydev, 0xFFFE0, 0x501, 1);
	aeon_mdio_write_reg_field(phydev, 0xFFFE0, 0x401, 0);
	aeon_mdio_write_reg_field(phydev, 0xFFFE0, 0xA01, 1);
	aeon_mdio_write_reg_field(phydev, 0xFFFE0, 0x201, 0);
	aeon_mdio_write_reg_field(phydev, 0xFFFE0, 0x101, 0);

	aeon_mdio_write_reg_field(phydev, 0x3C1602, 0xF01, 1);
	aeon_mdio_write_reg_field(phydev, 0x3C1602, 0xE01, 1);
	aeon_mdio_write_reg_field(phydev, 0x3C1602, 0xF01, 0);
	aeon_mdio_write_reg_field(phydev, 0x3C1602, 0xE01, 0);

	aeon_mdio_write_reg_field(phydev, 0x3C2020, 0x901, 1);
	aeon_mdio_write_reg_field(phydev, 0x3C2020, 0x901, 0);

	for (i = 0; i < 4; i++) {
		aeon_mdio_write_reg_field(phydev, 0x41402 + i * 0x200, 0x901,
					  1);
		for (j = 0; j < 12; j++) {
			aeon_mdio_write_reg_field(phydev, 0x41402 + i * 0x200,
						  0x9, coeffs[j] & 0x1FF);
			aeon_mdio_write_reg_field(phydev, 0x41400 + i * 0x200,
						  0x104, j);
			aeon_mdio_write_reg_field(phydev, 0x41400 + i * 0x200,
						  0x1, 1);
			aeon_mdio_write_reg_field(phydev, 0x41400 + i * 0x200,
						  0x1, 0);
		}
	}
}

void aeon_100m_test_mode(struct phy_device *phydev)
{
	// enable AN
	aeon_cu_an_enable(1, phydev);
	aeon_cu_an_set_ms_cfg(0, 1, 0, phydev);
	aeon_set_man_mdi(phydev);
	// set half duplex
	aeon_set_man_duplex(0, phydev);
	// switch speed
	aeon_cu_an_set_top_spd(MDI_CFG_SPD_T100, phydev);
	// disable AN
	aeon_cu_an_enable(0, phydev);
	aeon_man_configure(phydev);
}

void aeon_ipc_set_tx_fullscale_delta(unsigned short speed,
				     unsigned short *delta,
				     struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 5, i = 0;

	data[0] = CFG_NG_PHYCTRL;
	data[1] = IPC_CMD_CFG_TX_FULLSCALE;
	data[2] = speed;
	for (i = 0; i < 4; i++) {
		data[3] = i;
		data[4] = *(delta + i);

		aeon_ipc_cfg_param_direct(reg_num, data, phydev);
	}
}

void aeon_ipc_set_wol(unsigned short en, unsigned short *val,
		      struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 5;

	data[0] = CFG_WOL;
	data[1] = en;
	data[2] = val[0];
	data[3] = val[1];
	data[4] = val[2];

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_ipc_smi_command(unsigned short *val, struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 3;

	data[0] = CFG_SMI_COMMAND;
	data[1] = val[0];
	data[2] = val[1];

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_ipc_irq_en(unsigned short *val, struct phy_device *phydev)
{
	struct device *dev = phydev_dev(phydev);
	unsigned short status = 0;
	unsigned short ret_size;
	unsigned short data[8], reg_num = 3;

	data[0] = IPC_CMD_IRQ_EN;
	data[1] = val[0];
	data[2] = val[1];

	aeon_send_ipc_msg(phydev, reg_num, data, IPC_CMD_CFG_IRQ, reg_num * 2);
	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "set irq en command failed %x\n", status);
		return;
	}
}

void aeon_ipc_irq_clr(unsigned short val, struct phy_device *phydev)
{
	struct device *dev = phydev_dev(phydev);
	unsigned short status = 0;
	unsigned short ret_size;
	unsigned short data[8], reg_num = 2;

	data[0] = IPC_CMD_IRQ_CLR;
	data[1] = val;

	aeon_send_ipc_msg(phydev, reg_num, data, IPC_CMD_CFG_IRQ, reg_num * 2);
	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "set irq clr command failed %x\n", status);
		return;
	}
}

void aeon_ipc_irq_query(unsigned short *irq, struct phy_device *phydev)
{
	struct device *dev = phydev_dev(phydev);
	unsigned short status = 0;
	unsigned short ret_size;
	unsigned short data[8], reg_num = 1;

	data[0] = IPC_CMD_IRQ_QUERY;

	aeon_send_ipc_msg(phydev, reg_num, data, IPC_CMD_CFG_IRQ, reg_num * 2);
	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "set irq query command failed %x\n", status);
		return;
	}
	aeon_receive_ipc_data(phydev, 1, (unsigned short *)irq);
}

void aeon_ipc_cable_diag(unsigned short sub_cmd, unsigned short *data_rcv,
			 struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 2, data_num = 0;

	data[0] = CFG_CABLE_DIAG;
	data[1] = sub_cmd;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
	switch (sub_cmd) {
	case IPC_CMD_CABLE_DIAG_CHAN_LEN:
		data_num = 4;
		aeon_receive_ipc_data(phydev, data_num,
				      (unsigned short *)data_rcv);
		break;
	case IPC_CMD_CABLE_DIAG_PPM_OFST:
		data_num = 2;
		aeon_receive_ipc_data(phydev, data_num,
				      (unsigned short *)data_rcv);
		break;
	case IPC_CMD_CABLE_DIAG_SNR_MARG:
	case IPC_CMD_CABLE_DIAG_CHAN_SKW:
		data_num = 8;
		aeon_receive_ipc_data(phydev, data_num,
				      (unsigned short *)data_rcv);
		break;
	}
}

void aeon_ipc_get_tx_fullscale_delta(unsigned short speed,
				     unsigned short *delta,
				     struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 3;

	data[0] = CFG_NG_PHYCTRL;
	data[1] = IPC_CMD_GET_TX_FULLSCALE;
	data[2] = speed;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
	aeon_receive_ipc_data(phydev, 4, (unsigned short *)delta);
}

void aeon_ipc_clear_log(struct phy_device *phydev)
{
	unsigned short status, ret_size;
	unsigned short cfg[1] = { IPC_CMD_READ_LOG_CLEAR };
	struct device *dev = phydev_dev(phydev);

	aeon_send_ipc_msg(phydev, 1, cfg, IPC_CMD_LOG, 2);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);
	if (status != IPC_STS_CMD_SUCCESS) {
		dev_err(dev, "clear log command failed %x\n", status);
		return;
	}
}

void aeon_ipc_set_wdt(unsigned short en, struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 2;

	data[0] = CFG_WDT;
	data[1] = en;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

void aeon_ipc_eye_scan(unsigned short subcmd, unsigned short *data_rcv,
			unsigned short sds_id, unsigned short eye_num,
			struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 4, data_num = 0;

	data[0] = CFG_EYE_DRAW;
	data[1] = subcmd;
	data[2] = sds_id;
	data[3] = eye_num;

	if (IPC_CMD_EYE_SCAN_GET == subcmd) {
		data_num = 4;
		reg_num = 2;
	} else if (IPC_CMD_EYE_SCAN == subcmd) {
		data_num = 1;
	}
	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
	aeon_receive_ipc_data(phydev, data_num,
		(unsigned short *)data_rcv);
}

void aeon_ipc_read_mem(unsigned short addr1, unsigned short addr2,
			unsigned short num, unsigned short *params, struct phy_device *phydev)
{
	unsigned short status, ret_size;
	unsigned short cfg[3] = {addr1, addr2, num};
	struct device *dev = phydev_dev(phydev);
	
	aeon_send_ipc_msg(phydev, 3, cfg, IPC_CMD_RMEM16, 6);

	status = aeon_ipc_wait_cmd_done(phydev, NULL, &ret_size);  
	if (status != IPC_STS_CMD_SUCCESS) {
	  dev_err(dev, "read mem command failed %x\n", status);
	  return;
	}
	aeon_receive_ipc_data(phydev, num, (unsigned short*)params);
}

void aeon_ipc_set_mac_cnt(unsigned long long mac_tot_cnt, unsigned long long mac_crc_cnt,
			struct phy_device *phydev)
{
	unsigned short data[8], reg_num = 6;

	data[0] = CFG_MAC_CNT;
	data[1] = IPC_CMD_MAC_TOT;
	data[2] = mac_tot_cnt & 0xFFFF;
	data[3] = (mac_tot_cnt >> 16) & 0xFFFF;
	data[4] = (mac_tot_cnt >> 32) & 0xFFFF;
	data[5] = (mac_tot_cnt >> 48) & 0xFFFF;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
	mdelay(2);

	data[0] = CFG_MAC_CNT;
	data[1] = IPC_CMD_MAC_CRC;
	data[2] = mac_crc_cnt & 0xFFFF;
	data[3] = (mac_crc_cnt >> 16) & 0xFFFF;
	data[4] = (mac_crc_cnt >> 32) & 0xFFFF;
	data[5] = (mac_crc_cnt >> 48) & 0xFFFF;

	aeon_ipc_cfg_param_direct(reg_num, data, phydev);
}

/*8831 api end*/
int parse_cmd_args(const char* input, struct parsed_cmd* result, int max_args)
{
    char buffer[MAX_BUF];
    char *token, *ptr;
    int count = 0;
	size_t len;

    if (!input || !result || max_args > MAX_ARGS)
        return -EINVAL;

    strncpy(buffer, input, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

	// clear '\n'
	len = strlen(buffer);
	if (len > 0 && buffer[len - 1] == '\n')
    	buffer[len - 1] = '\0';

	ptr = buffer;

    token = strsep(&ptr, " ");
    while (token && *token == '\0')
        token = strsep(&ptr, " ");

    if (!token)
        return -EINVAL;

    strncpy(result->cmd, token, sizeof(result->cmd) - 1);
    result->cmd[sizeof(result->cmd) - 1] = '\0';

    while ((token = strsep(&ptr, " ")) != NULL && count < max_args) {
        if (*token == '\0')
            continue;

        if (kstrtol(token, 0, &result->args[count]) != 0)
            return -EINVAL;

        count++;
    }

    result->argc = count;
    return 0;
}
static inline void printk_restart_an_usage(void)
{	
	pr_info("================Please input:===================\n");
	pr_info("Restart AN: echo RestartAN > /proc/aeon_restart_an \n");
}

static inline void printk_cfg_speed_usage(void)
{	
	pr_info("================Please input:===================\n");
	pr_info("Set 10G FD: echo 10Gbps > /proc/aeon_speed_mode \n");
	pr_info("Set 5G FD: echo 5Gbps > /proc/aeon_speed_mode \n");
	pr_info("Set 2.5G FD: echo 2.5Gbps > /proc/aeon_speed_mode \n");
	pr_info("Set 1G FD: echo 1Gbps   > /proc/aeon_speed_mode \n");
	pr_info("Set 100M FD: echo 100Mbps > /proc/aeon_speed_mode \n\n");
}
static inline void printk_choose_phy_usage(void)
{	
	pr_info("================Please input:===================\n");
	pr_info("echo dev [phy_addr] > /proc/aeon_choose_device \n\n");
}

static inline void printk_phy_enable_usage(void)
{	
	pr_info("================Please input:===================\n");
	pr_info("echo phyenable [enable] > /proc/aeon_phy_enable \n\n");
}
static ssize_t aeon_restart_an_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	printk_restart_an_usage();
	return 0;
}

static ssize_t aeon_restart_an_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[MAX_BUF] = {0};
	struct parsed_cmd cmdinfo = {0};
	unsigned short ms_related_cfg[4] = {0};

	if (count >= sizeof(val_string)) {
		return -EINVAL;
	}
	if (copy_from_user(val_string, buf, count))
		return -EFAULT;

	val_string[count] = '\0';

	if (parse_cmd_args(val_string, &cmdinfo, 0) != 0) {
		return -EINVAL;
	}

	if (!strcmp(cmdinfo.cmd, "RestartAN")) {
		if (cmdinfo.argc != 0)
			return -EINVAL;

		aeon_cu_an_get_ms_cfg(ms_related_cfg, an8831_dev);
		if (__priv_data.top_spd != 0xF) {
			aeon_cu_an_set_top_spd(__priv_data.top_spd, an8831_dev);
		}
		if (__priv_data.eee_spd != 0xFF) {
			aeon_cu_an_set_eee_spd(__priv_data.eee_spd, an8831_dev);
		}
		if (__priv_data.ms_en != 0xF) {
			ms_related_cfg[1] = __priv_data.ms_en;
			aeon_cu_an_set_ms_cfg(ms_related_cfg[0],
				ms_related_cfg[1], ms_related_cfg[2], an8831_dev);
		}
		if (__priv_data.ms_config != 0xF) {
			ms_related_cfg[2] = __priv_data.ms_config;
			aeon_cu_an_set_ms_cfg(ms_related_cfg[0],
				ms_related_cfg[1], ms_related_cfg[2], an8831_dev);
		}
		if (__priv_data.port_type != 0xF) {
			ms_related_cfg[0] = __priv_data.port_type;
			aeon_cu_an_set_ms_cfg(ms_related_cfg[0],
				ms_related_cfg[1], ms_related_cfg[2], an8831_dev);
		}
		if ((__priv_data.smt_spd.enable != 0xF) 
			|| (__priv_data.smt_spd.retry_limit != 0xF)) {
			aeon_cu_an_enable_downshift(__priv_data.smt_spd.enable, 
				__priv_data.smt_spd.retry_limit, an8831_dev);
		}
		if (__priv_data.nstd_pbo != 0xFF) {
			aeon_cu_an_enable_aeon_oui(__priv_data.nstd_pbo, an8831_dev);
		}
		if ((__priv_data.trd_swap != 0xF)
			|| (__priv_data.trd_ovrd != 0xF)) {
			aeon_cu_an_set_trd_swap(__priv_data.trd_ovrd, __priv_data.trd_swap, an8831_dev);
		}
		if (__priv_data.cfr != 0xF) {
			aeon_cu_an_set_cfr(__priv_data.cfr, an8831_dev);
		}
		aeon_ipc_sync_parity(an8831_dev);
		aeon_cu_an_restart(an8831_dev);
		pr_info("AN-related CFG finish, restart AN successfully!\n");
	} else {
		printk_restart_an_usage();
	}

	return count;
}
static ssize_t aeon_speed_mode_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	printk_cfg_speed_usage();
	return 0;
}

static ssize_t aeon_speed_mode_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[MAX_BUF] = {0};
	struct parsed_cmd cmdinfo = {0};

	if (count >= sizeof(val_string)) {
		return -EINVAL;
	}
	if (copy_from_user(val_string, buf, count))
		return -EFAULT;

	val_string[count] = '\0';

	if (parse_cmd_args(val_string, &cmdinfo, 0) != 0) {
		return -EINVAL;
	}

	if (cmdinfo.argc != 0)
		return -EINVAL;

	if (!strcmp(cmdinfo.cmd, "10Gbps")) {
		__priv_data.top_spd = MDI_CFG_SPD_T10G;
		pr_info("Set 10Gbps successfully!\n");
	} else if (!strcmp(cmdinfo.cmd, "5Gbps")) {
		__priv_data.top_spd = MDI_CFG_SPD_T5G;
		pr_info("Set 5Gbps successfully!\n");
	} else if (!strcmp(cmdinfo.cmd, "2.5Gbps")) {
		__priv_data.top_spd = MDI_CFG_SPD_T2P5G;
		pr_info("Set 2.5Gbps successfully!\n");
	} else if (!strcmp(cmdinfo.cmd, "1Gbps")) {
		__priv_data.top_spd = MDI_CFG_SPD_T1G;
		pr_info("Set 1Gbps successfully!\n");
	} else if (!strcmp(cmdinfo.cmd, "100Mbps")) {
		__priv_data.top_spd = MDI_CFG_SPD_T100;
		pr_info("Set 100Mbps successfully!\n");
	} else {
		printk_cfg_speed_usage();
	}

	return count;
}
static ssize_t aeon_choose_device_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	printk_choose_phy_usage();
	return 0;
}

static ssize_t aeon_choose_device_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[MAX_BUF] = {0};
	struct parsed_cmd cmdinfo = {0};

	if (count >= sizeof(val_string)) {
		return -EINVAL;
	}
	if (copy_from_user(val_string, buf, count))
		return -EFAULT;

	val_string[count] = '\0';

	if (parse_cmd_args(val_string, &cmdinfo, 1) != 0) {
		return -EINVAL;
	}

	if (!strcmp(cmdinfo.cmd, "dev")) {
		if (cmdinfo.argc != 1)
			return -EINVAL;

		aeon_match_device(cmdinfo.args[0], &an8831_dev);
	} else {
		printk_choose_phy_usage();
	}

	return count;
}
static ssize_t aeon_eth_status_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int len = 0, i = 0;
    char *kbuf;
    size_t kbuf_size = 1024;
    unsigned short link_status, speed, value;

    if (*ppos > 0)
        return 0; // EOF
        
    kbuf = kmalloc(kbuf_size, GFP_KERNEL);
    if (!kbuf){
        return -ENOMEM;
	}

	for (i = 0; i < AN8831_PHY_NUM; i++) 
	{
		if (an8831_dev_all[i] != NULL) 
		{
		    value = phy_read_mmd(an8831_dev_all[i], 0x7, 0xFFE1);
		    link_status = (value & 0x7) >> 2;
			len += snprintf(kbuf + len, kbuf_size - len, "PHY[%s]: ", get_ifname_by_phy_addr(an8831_dev_all[i]->mdio.addr));

		    if (link_status) 
			{
		        value = phy_read_mmd(an8831_dev_all[i], 0x1E, 0x4002);
		        speed = value & 0xFF;

		        if (speed == 0x3)
		            len += snprintf(kbuf + len, kbuf_size - len, "10Gbps\n");
		        else if (speed == 0x5)
		            len += snprintf(kbuf + len, kbuf_size - len, "5Gbps\n");
		        else if (speed == 0x9)
		            len += snprintf(kbuf + len, kbuf_size - len, "2.5Gbps\n");
		        else if (speed == 0x10)
		            len += snprintf(kbuf + len, kbuf_size - len, "1000Mbps\n");
		        else if (speed == 0x20)
		            len += snprintf(kbuf + len, kbuf_size - len, "100Mbps\n");
		        else
		            len += snprintf(kbuf + len, kbuf_size - len, "Unknown speed\n");
		    }
			else{
				len += snprintf(kbuf + len, kbuf_size - len, "Down\n");
			}
		}
	}

    if (*ppos >= len) {
        kfree(kbuf);
        return 0;
    }

    if (count > len - *ppos)
        count = len - *ppos;

    if (copy_to_user(buf, kbuf + *ppos, count)) {
        kfree(kbuf);
        return -EFAULT;
    }

    *ppos += count;
    kfree(kbuf);
    return count;
}

static ssize_t aeon_serdes_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char *kbuf;
    size_t kbuf_size = 1024;
    int len = 0;
    int i;

    if (*ppos > 0)
        return 0; // EOF

    kbuf = kmalloc(kbuf_size, GFP_KERNEL);
    if (!kbuf) {
        return -ENOMEM;
    }

    len += snprintf(kbuf + len, kbuf_size - len, "-----------Serdes Map Info----\n");
    len += snprintf(kbuf + len, kbuf_size - len, "name     serdes         phy_addr\n");

    for (i = 0; i < SERDES_MAX_IDX; i++) {
        const char *name = itfname_info[i] ? itfname_info[i] : "--";
        const char *serdes = serdes_info[i] ? serdes_info[i] : "--";
        if (serdes_map_aeon[i].phy_addr == 0)
            len += snprintf(kbuf + len, kbuf_size - len, "%-8s %-13s --\n", name, serdes);
        else
            len += snprintf(kbuf + len, kbuf_size - len, "%-8s %-13s %d\n", name, serdes, serdes_map_aeon[i].phy_addr);
    }

    if (len > count)
        len = count;

    if (copy_to_user(buf, kbuf, len)) {
        kfree(kbuf);
        return -EFAULT;
    }

    kfree(kbuf);
    *ppos += len;
    return len;
}

static ssize_t aeon_phy_enable_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	printk_phy_enable_usage();
	return 0;
}

static ssize_t aeon_phy_enable_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[MAX_BUF] = {0};
	struct parsed_cmd cmdinfo = {0};

	if (count >= sizeof(val_string)) {
		return -EINVAL;
	}
	if (copy_from_user(val_string, buf, count)){
		return -EFAULT;
	}

	val_string[count] = '\0';

	if (parse_cmd_args(val_string, &cmdinfo, 1) != 0) {
		return -EINVAL;
	}

	if (!strcmp(cmdinfo.cmd, "phyenable")) 
	{
		if (cmdinfo.argc != 1){
			return -EINVAL;
		}

		aeon_ipc_sync_parity(an8831_dev);
		aeon_ipc_phy_enable_mode(cmdinfo.args[0], an8831_dev);
		phy_state = cmdinfo.args[0];
		pr_info("Set phy successfully!\n");
	}
	else if (!strcmp(cmdinfo.cmd, "phystate")) 
	{
		pr_info("phy state: %s\n", phy_state ? "on" : "off");
	} else {
		printk_phy_enable_usage();
	}

	return count;
}

int an8831_phy_is_empty(void)
{
	int is_empty = 1, i = 0; 
	for (i = 0; i < AN8831_PHY_NUM; i++) {
		if (an8831_dev_all[i] != NULL) {
			is_empty = 0;
			break;
		}
	}
	return i;
}
static const struct proc_ops proc_aeon_speed_mode_fops = {
	.proc_read	= aeon_speed_mode_read_proc,
	.proc_write	= aeon_speed_mode_write_proc,
};
static const struct proc_ops proc_aeon_restart_an_fops = {
	.proc_read	= aeon_restart_an_read_proc,
	.proc_write	= aeon_restart_an_write_proc,
};
static const struct proc_ops proc_aeon_choose_device_fops = {
	.proc_read	= aeon_choose_device_read_proc,
	.proc_write	= aeon_choose_device_write_proc,
};
static const struct proc_ops proc_aeon_eth_status_fops = {
	.proc_read	= aeon_eth_status_read_proc,
};
static const struct proc_ops proc_aeon_serdes_fops = {
	.proc_read	= aeon_serdes_read_proc,
};

static const struct proc_ops proc_aeon_phy_enable_fops = {
	.proc_read	= aeon_phy_enable_read_proc,
	.proc_write	= aeon_phy_enable_write_proc,
};

unsigned int HSGMII_BASE_REG[XSI_IDX_MAX];

static void airoha_hsgmii_base_reg_init(void)
{
	HSGMII_BASE_REG[XSI_PCIE0_IDX] = XSI_PCIE0_BASE;
	HSGMII_BASE_REG[XSI_PCIE1_IDX] = XSI_PCIE1_BASE;
	HSGMII_BASE_REG[XSI_USB_IDX] = XSI_USB_BASE;
	HSGMII_BASE_REG[XSI_AE_IDX] = XSI_AE_BASE;
	HSGMII_BASE_REG[XSI_ETH_IDX] = XSI_ETH_BASE;
}

static int xsi_check_index_valid(uint hsgmii_index)
{
	if(hsgmii_index >= XSI_IDX_MAX)
		return 0;
	else 
		return 1 ;

}

static int xsi_mac_set_mpi_mbi_disable(uint hsgmii_index)
{
	xsiSetRxmpiDisable(hsgmii_index);
	xsiSetRxmbiDisable(hsgmii_index);
	xsiSetTxmpiDisable(hsgmii_index);
	xsiSetTxmbiDisable(hsgmii_index);
	return 0;
}

static int xsi_mac_set_mpi_mbi_enable(uint hsgmii_index)
{
	xsiSetRxmpiEnable(hsgmii_index);
	xsiSetRxmbiEnable(hsgmii_index);
	xsiSetTxmpiEnable(hsgmii_index);
	xsiSetTxmbiEnable(hsgmii_index);
	return 0;
}

int xsi_mac_api_do_logic_reset(int hsgmii_index)
{
	if(0 == xsi_check_index_valid(hsgmii_index))
		return -1;

	xsi_mac_set_mpi_mbi_disable(hsgmii_index);
	mdelay(1);

	//logic reset and release mac logic_rst
	xsimaclogicrst(hsgmii_index);
	xsimaclogicrstenable(hsgmii_index);
	
	//do cnt clear
	xsiClearAllCnt(hsgmii_index);

	//mbi & mpi enable
	xsi_mac_set_mpi_mbi_enable(hsgmii_index);
	return 0;
}
EXPORT_SYMBOL(xsi_mac_api_do_logic_reset);

static int get_bucketsize_shift(uint value, uint lo, uint hi, uint unit)
{
	int mid = 0;
	
	if(lo > hi )
		return -EINVAL;

	if((value > 0) && (value < unit))
		return 0;

	mid = (lo + hi) / 2;

	if((unit<<mid) == value){
		return mid;
		
	}else if((unit<<mid) > value){
		if((mid - lo) <= 1)
			return mid;
		
		return get_bucketsize_shift(value, lo, mid, unit);
	}else{
		if((hi - mid) <= 1)
			return hi;
		
		return get_bucketsize_shift(value, mid, hi, unit);
	}
}

static int hsgmii_set_ratelimit_param(uint hsgmii_index,uint paraType,uint cfg_id,uint valueLo)
{
	uint paraCfg = 0;
	uint rc_cfg_reg = 0;
	uint rc_data_L_reg = 0;
	bool isEN7581 = (glb_eth && airoha_is_7581(glb_eth));

	if( (hsgmii_index >= XSI_PCIE0_IDX) && (hsgmii_index <= XSI_ETH_IDX))
	{
		if(cfg_id >= HSGMII_RX_UC_RATE && cfg_id <= HSGMII_RX_MC_RATE ){
			rc_cfg_reg = HSGMII_BASE_REG[hsgmii_index]+ RX_CFG ;
			rc_data_L_reg = HSGMII_BASE_REG[hsgmii_index]+ RC_WR_DATA_L;
		}else if(SUPPORT_TX_TOTAL_RATELIMIT && (cfg_id == HSGMII_TX_TOTAL_RATE)){
			rc_cfg_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_TX_CFG ;
			rc_data_L_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RC_WR_DATA_L;
		}else if(cfg_id == HSGMII_RX_TOTAL_RATE){
			rc_cfg_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RX_CFG ;
			rc_data_L_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RC_WR_DATA_L;
			if(isEN7581)
				cfg_id = 0;
		} else {
			return 0;
		}
	}
	else 
	{
		return 0;	
	}
	
	regWrite32(rc_data_L_reg, valueLo);
	
	paraCfg = (HSGMII_RC_CFG_EN | HSGMII_RC_CFG_PARA_RWCMD | ((paraType<<HSGMII_RC_CFG_PARA_TYPE_SHIFT)&HSGMII_RC_CFG_PARA_TYPE_MASK) | 
		((cfg_id<<HSGMII_RC_CFG_ID_SHIFT)&HSGMII_RC_CFG_ID_MASK));

	regWrite32(rc_cfg_reg, paraCfg);
	
	printk("rc_cfg_reg[%x] = 0x%x, rc_data_L_reg[%x]=0x%x\n", rc_cfg_reg, paraCfg, rc_data_L_reg, valueLo);
	return 0;
}

static int hsgmii_get_ratelimit_param(uint hsgmii_index,uint paraType,uint cfg_id,uint *valueLo,uint *valueHi)
{
	uint paraCfg = 0;
	uint rc_cfg_reg = 0;
	uint rc_data_L_reg = 0;
	uint rc_data_H_reg = 0;
	bool isEN7581 = (glb_eth && airoha_is_7581(glb_eth));

	if( (hsgmii_index >= XSI_PCIE0_IDX) && (hsgmii_index <= XSI_ETH_IDX))
	{
		if(cfg_id >= HSGMII_RX_UC_RATE && cfg_id <= HSGMII_RX_MC_RATE ){
			rc_cfg_reg = HSGMII_BASE_REG[hsgmii_index]+ RX_CFG ;
			rc_data_L_reg = HSGMII_BASE_REG[hsgmii_index]+ RC_RD_DATA_L;
			rc_data_H_reg = HSGMII_BASE_REG[hsgmii_index]+ RC_RD_DATA_H;
		}else if(SUPPORT_TX_TOTAL_RATELIMIT && (cfg_id == HSGMII_TX_TOTAL_RATE)){
			rc_cfg_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_TX_CFG ;
			rc_data_L_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RC_RD_DATA_L;
			rc_data_H_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RC_RD_DATA_H;
		}else if(cfg_id == HSGMII_RX_TOTAL_RATE){
			rc_cfg_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RX_CFG ;
			rc_data_L_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RC_RD_DATA_L;
			rc_data_H_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RC_RD_DATA_H;
			if(isEN7581)
				cfg_id = 0;
		} else {
			return 0;
		}
	}
	else 
	{
		return 0;
	}

	
	paraCfg = (((paraType<<HSGMII_RC_CFG_PARA_TYPE_SHIFT)&HSGMII_RC_CFG_PARA_TYPE_MASK) | 
		((cfg_id<<HSGMII_RC_CFG_ID_SHIFT)&HSGMII_RC_CFG_ID_MASK));

	regWrite32(rc_cfg_reg, paraCfg);
	
	*valueLo = regRead32(rc_data_L_reg);
	*valueHi = regRead32(rc_data_H_reg);
	
	return 0;
}

int xsi_mac_set_ratelimit(uint hsgmii_index, unsigned int type, uint rate, uint mode)
{

	//unsigned int val = 0;
	unsigned int meter_en = 0;
	unsigned int tickSel = 0;
	unsigned int rateLimitUnit = 0;
	int bucketSize_shift = 0;
	int curTicksel = 0,bucketSize=0;
	uint tokenRate = 0;
	unsigned int rx_rc_cfg_reg = 0;
	unsigned int rc_data_h_reg = 0;
	uint tokenRate_integer = 0;
	unsigned int tokenRate_fraction = 0;
	uint valueLo = 0, valueHi = 0;

	if( (hsgmii_index >= XSI_PCIE0_IDX) && (hsgmii_index <= XSI_ETH_IDX))
	{
		if(type >= HSGMII_RX_UC_RATE && type <= HSGMII_RX_MC_RATE ){
			rx_rc_cfg_reg = HSGMII_BASE_REG[hsgmii_index]+ RX_RC_CFG ;
			rc_data_h_reg = HSGMII_BASE_REG[hsgmii_index]+ RC_WR_DATA_H;
		}else if(SUPPORT_TX_TOTAL_RATELIMIT && (type == HSGMII_TX_TOTAL_RATE)){
			rx_rc_cfg_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_TX_RC_CFG ;
			rc_data_h_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RC_WR_DATA_H;
		}else if(type == HSGMII_RX_TOTAL_RATE){
			rx_rc_cfg_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RX_RC_CFG ;
			rc_data_h_reg = HSGMII_BASE_REG[hsgmii_index]+ TOTAL_RC_WR_DATA_H;
		} else {
			return 0;
		}
	}
	else 
	{
		return 0;
	}

	/* check rate value */
	if(0 == rate)
	{
		meter_en = HSGMII_RC_CFG_METER_DISABLE;
	}
	else
	{
		meter_en = HSGMII_RC_CFG_METER_ENABLE;
	}
	
	if(rate == 0){
		regWrite32(rx_rc_cfg_reg, 0);
		printk("disable use rc_cfg_reg[%x] = 0x%x, rc_data_h_reg[%x]=0\n", rx_rc_cfg_reg, 0, rc_data_h_reg);	
	}
	else if((mode == HSGMII_RC_CFG_BYTE_MODE) || ((mode == HSGMII_RC_CFG_PKT_MODE) && (rate > 20000)))
	{
		/* default tick 125us */
		curTicksel = 125;
		tickSel = HSGMII_RC_CFG_FAST_TICK;
		regWrite32(rx_rc_cfg_reg,0x8001007d);
		printk("byte mode use rc_cfg_reg[%x] = 0x%x, rc_data_h_reg[%x]=0\n", rx_rc_cfg_reg, 0x8001007d, rc_data_h_reg);
	}
	else  //HSGMII_RC_CFG_PKT_MODE && rate < 20000
	{
		curTicksel = 125*32;
		tickSel = HSGMII_RC_CFG_SLOW_TICK;
		regWrite32(rx_rc_cfg_reg,0x8020007d);
		printk("packet mode use rc_cfg_reg[%x] = 0x%x, rc_data_h_reg[%x]=0\n", rx_rc_cfg_reg, 0x8020007d, rc_data_h_reg);
	}
	regWrite32(rc_data_h_reg,0x0);

	/* set basic parameters */
	hsgmii_get_ratelimit_param(hsgmii_index,HSGMII_RC_CFG_PARA_MISC, type, &valueLo, &valueHi);
	valueLo = (meter_en == HSGMII_RC_CFG_METER_ENABLE) ? (valueLo|HSGMII_RC_CFG_PARA_METER_EN):(valueLo &(~HSGMII_RC_CFG_PARA_METER_EN));
	valueLo = (mode == HSGMII_RC_CFG_PKT_MODE) ? (valueLo|HSGMII_RC_CFG_PARA_PPS_MODE):(valueLo &(~HSGMII_RC_CFG_PARA_PPS_MODE));
	if(SUPPORT_TX_TOTAL_RATELIMIT)
	{
		valueLo = (tickSel == HSGMII_RC_CFG_SLOW_TICK) ? (valueLo|HSGMII_RC_CFG_PARA_TICK_SEL):(valueLo &(~HSGMII_RC_CFG_PARA_TICK_SEL));
		//valueLo = (valueLo &(~HSGMII_RC_CFG_PARA_TICK_SEL));
	}
	valueLo = (valueLo &(~HSGMII_RC_CFG_PARA_TICK_SEL));
	printk("set basicParameter: \t");
	hsgmii_set_ratelimit_param(hsgmii_index,HSGMII_RC_CFG_PARA_MISC, type, valueLo);
	
	/* set token unit  */
	if(HSGMII_RC_CFG_BYTE_MODE == mode)
	{
		/* 8bits X 1000 / (curTicksel X 10e-6 s)  kbps */
		rateLimitUnit = (8000 / curTicksel);
	}
	else
	{
		/* 1 / (curTicksel X 10e-6 s) pps */
		rateLimitUnit = (1000000 / curTicksel);
	}
	
	/* calculate tokenRate */
	tokenRate_integer = (rate / rateLimitUnit);
	tokenRate_fraction = ((rate % rateLimitUnit) * 64 / rateLimitUnit);
	if((tokenRate_integer > 0x3FFFF) || (tokenRate_fraction > 0x3F))
	{
		printk("tokenRate overflow.\n");
		return -EINVAL;
	}
	
	/* set token rate */
	tokenRate = ((tokenRate_integer << RC_TOKEN_RATE_INTEGER_SHIFT) | tokenRate_fraction);
	printk("set tokenRate: \t\t");
	hsgmii_set_ratelimit_param(hsgmii_index,HSGMII_RC_CFG_PARA_TOKEN_RATE, type, tokenRate);
	
	
	/* get bucket size  */
	if(HSGMII_RC_CFG_BYTE_MODE == mode)
	{
		bucketSize = (rate<<RC_BYTE_MODE_BUCKET_SHIFT);
	}
	else
	{
		bucketSize = (rate<<RC_PKT_MODE_BUCKET_SHIFT);
	}

	/*max: 32M bucket size */
	if(bucketSize > 0x2000000)
	{
		bucketSize = 0x2000000;
	}
	
	/* set bucketsize_shift */
	bucketSize_shift = get_bucketsize_shift(bucketSize, 0, 15, 1024);
	printk("set bucketSize_shift: \t");
	hsgmii_set_ratelimit_param(hsgmii_index,HSGMII_RC_CFG_PARA_BUCK_SHIFT, type, bucketSize_shift);
	
	return 0;
}

int an8831_proc_init(void)
{
    struct proc_dir_entry *an8831_proc;
	int dev_num = an8831_phy_is_empty();
	if (dev_num == AN8831_PHY_NUM) {
		pr_info("error! phy_dev is null! \n");
		return -EINVAL;
	} else {
		an8831_dev = an8831_dev_all[dev_num];
		an8831_phy_bus = phydev_mdio_bus(an8831_dev);
		if (an8831_phy_bus == NULL) {
			pr_info("air_phy_bus error! \n");
			return -EINVAL;
		}			
		an8831_phy_addr = phydev_addr(an8831_dev);
		if (an8831_phy_addr == -1) {
			pr_info("air_phy_addr error! \n");
			return -EINVAL;
		}
	}
	// init the data structure
	__priv_data.top_spd = 0xF;
	__priv_data.eee_spd = 0xFF;
	__priv_data.fr_spd = 0xF;
	__priv_data.thp_byp = 0xF;
	__priv_data.port_type = 0xF;
	__priv_data.ms_en = 0xF;
	__priv_data.ms_config = 0xF;
	__priv_data.nstd_pbo = 0xFF;
	__priv_data.smt_spd.enable = 0xF;
	__priv_data.smt_spd.retry_limit = 0xF;
	__priv_data.trd_ovrd = 0xF;
	__priv_data.trd_swap = 0xF;
	__priv_data.cfr = 0xF;

	an8831_proc = proc_create("aeon_speed_mode", 0, NULL,  &proc_aeon_speed_mode_fops);
	an8831_proc = proc_create("aeon_restart_an", 0, NULL, &proc_aeon_restart_an_fops);
	an8831_proc = proc_create("aeon_choose_device", 0, NULL, &proc_aeon_choose_device_fops);
	an8831_proc = proc_create("aeon_eth_status", 0, NULL, &proc_aeon_eth_status_fops);
	an8831_proc = proc_create("aeon_serdes", 0, NULL, &proc_aeon_serdes_fops);
	an8831_proc = proc_create("aeon_phy_enable", 0, NULL, &proc_aeon_phy_enable_fops);

	return 0;
}

int an8831_proc_exit(void)
{
	//judge an8831 exist
	int dev_num = an8831_phy_is_empty();
	if (dev_num == AN8831_PHY_NUM) {
		pr_info("error! phy_dev is null! \n");
		return 0;
	} else {
		an8831_phy_bus = NULL;
		an8831_phy_addr = -1;
	}
	remove_proc_entry("aeon_set_speed_mode", 0);
	remove_proc_entry("aeon_restart_an", 0);
	remove_proc_entry("aeon_choose_device", 0);
	remove_proc_entry("aeon_serdes", 0);
	remove_proc_entry("aeon_phy_enable", 0);

    return 0;
}

int airoha_hsgmii_init(void)
{
    int ret = 0;

	airoha_hsgmii_base_reg_init();
	fast_path_speed_threshold_init();
	rcu_assign_pointer(xsi_mac_set_ratelimit_hook, xsi_mac_set_ratelimit);
	
    /* Create and save bus pointer to global variable */
	hsgmii_priv_p = (Mdio_priv_data_t*)mem_alloc(sizeof(Mdio_priv_data_t));
    if (hsgmii_priv_p == NULL) {
        pr_err(" Failed to allocate memory for hsgmii_priv_p\n");
        return -ENOMEM;
    }
	/* default use mdio_0 */
    if(airoha_mii_bus[0]){
        hsgmii_priv_p->mii_bus = airoha_mii_bus[0];
    }
    else{
        mem_free(hsgmii_priv_p);
        return -EFAULT;
    }

    ret = init_hsgmii_priv(hsgmii_priv_p);
    if(ret == 0){
        mem_free(hsgmii_priv_p);
        pr_err("Failed to initialize data structure!\n");
        return ret;
    }
	if (en8811_is_exist){
    	en8811_proc_init();
	}

	if (an8831_is_exist){
		an8831_proc_init();
	}

	pr_info("airoha hsgmii init ok!\n");
    return 0;
}

void airoha_hsgmii_exit(void)
{
	rcu_assign_pointer(xsi_mac_set_ratelimit_hook, NULL);
    mem_free(hsgmii_priv_p);
	if (en8811_is_exist){
    	en8811_proc_exit();
	}

	if (an8831_is_exist){
		an8831_proc_exit();
	}

    pr_info("airoha hsgmii exit ok!\n");
}


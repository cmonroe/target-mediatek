// spdx-license-identifier: gpl-2.0
/*
 * drivers/net/dsa/host_api/mxl862xx_host_api_impl.c - dsa driver for maxlinear mxl862xx switch chips family
 *
 * copyright (c) 2024 maxlinear inc.
 *
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 *
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 *
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 *
 */

#include <linux/crc16.h>
#include "mxl862xx_mmd_apis.h"

#define CTRL_BUSY_MASK		MMD_API_CTRL_BUSY
#define CTRL_CMD_MASK		MMD_API_ID_MASK

#define DATA_REG_FIRST		MXL862XX_MMD_REG_DATA_FIRST
#define DATA_MAX_WORD		MXL862XX_MMD_REG_DATA_MAX_SIZE

#define MDIO_HBER_DBG		1

#define MDIO_ACCESS_RETRY	5

#define MAX_BUSY_LOOP		1000 /* roughly 10ms */
#define WAIT_MIN		10
#define WAIT_MAX		15

#define THR_RST_DATA		5

#define ENABLE_GETSET_OPT	1

//#define C22_MDIO

#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
static struct {
	uint16_t ctrl;
	int16_t ret;
	mmd_api_data_t data;
} shadow = { .ctrl = ~0, .ret = -1, .data = { { 0 } } };
#endif

static bool crc_chk_en = false;
static unsigned int mdio_msecs = 0;

/* required for Clause 22 extended read/write access */
#define MXL862XX_MMDDATA			0xE
#define MXL862XX_MMDCTRL			0xD

#define MXL862XX_ACTYPE_ADDRESS			(0 << 14)
#define MXL862XX_ACTYPE_DATA			(1 << 14)

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
#include <linux/delay.h>
#endif

#ifdef C22_MDIO
static int c22_mdiobus_write(struct mii_bus *bus, int addr, u32 regnum, u16 val)
{
	int ret, i;

	for (i = 0; i < MDIO_ACCESS_RETRY; i++) {
		ret = __mdiobus_write(bus, addr, regnum, val);
		if (ret < 0)
			return ret;

		ret = __mdiobus_read(bus, addr, regnum);
		if (ret < 0)
			return ret;

		if (val == (uint16_t)ret)
			return 0;
	}

	return MMD_API_MDIO_BUS_ERR;
}

/**
 *  write access to MMD register of PHYs via Clause 22 extended access
 */
static int __mxl862xx_c22_ext_mmd_write(const mxl862xx_device_t *dev, struct mii_bus *bus, int sw_addr, int mmd,
			    int reg, u16 val)
{
	int res;

	/* Set the DevID for Write Command */
	res = c22_mdiobus_write(bus, sw_addr, MXL862XX_MMDCTRL, mmd);
	if (res < 0)
		goto error;

	/* Issue the write command */
	res = c22_mdiobus_write(bus, sw_addr, MXL862XX_MMDDATA, reg);
	if (res < 0)
		goto error;

	/* Set the DevID for Write Command */
	res = c22_mdiobus_write(bus, sw_addr, MXL862XX_MMDCTRL, MXL862XX_ACTYPE_DATA | mmd);
	if (res < 0)
		goto error;

	/* Issue the write command */
	if (mmd == 0x1e && reg == 0)	/* ctrl register can't write twice */
		res = __mdiobus_write(bus, sw_addr, MXL862XX_MMDDATA, val);
	else
		res = c22_mdiobus_write(bus, sw_addr, MXL862XX_MMDDATA, val);
	if (res < 0)
		goto error;

error:
	return res;
}

/**
 *  read access to MMD register of PHYs via Clause 22 extended access
 */
static int __mxl862xx_c22_ext_mmd_read(const mxl862xx_device_t *dev, struct mii_bus *bus, int sw_addr, int mmd, int reg)
{
	int res;

/* Set the DevID for Write Command */
	res = c22_mdiobus_write(bus, sw_addr, MXL862XX_MMDCTRL, mmd);
	if (res < 0)
		goto error;

	/* Issue the write command */
	res = c22_mdiobus_write(bus, sw_addr, MXL862XX_MMDDATA, reg);
	if (res < 0)
		goto error;

	/* Set the DevID for Write Command */
	res = c22_mdiobus_write(bus, sw_addr, MXL862XX_MMDCTRL, MXL862XX_ACTYPE_DATA | mmd);
	if (res < 0)
		goto error;

	/* Read the data */
	res = __mdiobus_read(bus, sw_addr, MXL862XX_MMDDATA);
	if (res < 0)
		goto error;

error:
	return res;
}
#endif

int mxl862xx_read(const mxl862xx_device_t *dev, uint32_t regaddr)
{
	int mmd = MXL862XX_MMD_DEV;
#ifdef C22_MDIO
	int ret = __mxl862xx_c22_ext_mmd_read(dev, dev->bus, dev->sw_addr, mmd, regaddr);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
	u32 addr = MII_ADDR_C45 | (mmd << 16) | (regaddr & 0xffff);
	int ret = __mdiobus_read(dev->bus, dev->sw_addr, addr);
#else
	int ret = __mdiobus_c45_read(dev->bus, dev->sw_addr, mmd, regaddr);
#endif
	return ret;
}

int mxl862xx_write(const mxl862xx_device_t *dev, uint32_t regaddr, uint16_t data)
{
	int mmd = MXL862XX_MMD_DEV;
#ifdef C22_MDIO
	int ret = __mxl862xx_c22_ext_mmd_write(dev, dev->bus, dev->sw_addr, mmd, regaddr, data);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
	u32 addr = MII_ADDR_C45 | (mmd << 16) | (regaddr & 0xffff);
	int ret = __mdiobus_write(dev->bus, dev->sw_addr, addr, data);
#else
	int ret = __mdiobus_c45_write(dev->bus, dev->sw_addr, mmd, regaddr, data);
#endif
	return ret;
}

#define CRC6_INIT	0x00	/* initial value for CRC */

#if !IS_ENABLED(CRC6_0x31)
/* 3GPP CRC6 (0x6F) */
static uint8_t crc6(uint16_t ctrl, uint16_t len_ret)
{
	static uint8_t const crc6_table[256] = {
		0x00, 0x2f, 0x31, 0x1e, 0x0d, 0x22, 0x3c, 0x13,
		0x1a, 0x35, 0x2b, 0x04, 0x17, 0x38, 0x26, 0x09,
		0x34, 0x1b, 0x05, 0x2a, 0x39, 0x16, 0x08, 0x27,
		0x2e, 0x01, 0x1f, 0x30, 0x23, 0x0c, 0x12, 0x3d,
		0x07, 0x28, 0x36, 0x19, 0x0a, 0x25, 0x3b, 0x14,
		0x1d, 0x32, 0x2c, 0x03, 0x10, 0x3f, 0x21, 0x0e,
		0x33, 0x1c, 0x02, 0x2d, 0x3e, 0x11, 0x0f, 0x20,
		0x29, 0x06, 0x18, 0x37, 0x24, 0x0b, 0x15, 0x3a,
		0x0e, 0x21, 0x3f, 0x10, 0x03, 0x2c, 0x32, 0x1d,
		0x14, 0x3b, 0x25, 0x0a, 0x19, 0x36, 0x28, 0x07,
		0x3a, 0x15, 0x0b, 0x24, 0x37, 0x18, 0x06, 0x29,
		0x20, 0x0f, 0x11, 0x3e, 0x2d, 0x02, 0x1c, 0x33,
		0x09, 0x26, 0x38, 0x17, 0x04, 0x2b, 0x35, 0x1a,
		0x13, 0x3c, 0x22, 0x0d, 0x1e, 0x31, 0x2f, 0x00,
		0x3d, 0x12, 0x0c, 0x23, 0x30, 0x1f, 0x01, 0x2e,
		0x27, 0x08, 0x16, 0x39, 0x2a, 0x05, 0x1b, 0x34,
		0x1c, 0x33, 0x2d, 0x02, 0x11, 0x3e, 0x20, 0x0f,
		0x06, 0x29, 0x37, 0x18, 0x0b, 0x24, 0x3a, 0x15,
		0x28, 0x07, 0x19, 0x36, 0x25, 0x0a, 0x14, 0x3b,
		0x32, 0x1d, 0x03, 0x2c, 0x3f, 0x10, 0x0e, 0x21,
		0x1b, 0x34, 0x2a, 0x05, 0x16, 0x39, 0x27, 0x08,
		0x01, 0x2e, 0x30, 0x1f, 0x0c, 0x23, 0x3d, 0x12,
		0x2f, 0x00, 0x1e, 0x31, 0x22, 0x0d, 0x13, 0x3c,
		0x35, 0x1a, 0x04, 0x2b, 0x38, 0x17, 0x09, 0x26,
		0x12, 0x3d, 0x23, 0x0c, 0x1f, 0x30, 0x2e, 0x01,
		0x08, 0x27, 0x39, 0x16, 0x05, 0x2a, 0x34, 0x1b,
		0x26, 0x09, 0x17, 0x38, 0x2b, 0x04, 0x1a, 0x35,
		0x3c, 0x13, 0x0d, 0x22, 0x31, 0x1e, 0x00, 0x2f,
		0x15, 0x3a, 0x24, 0x0b, 0x18, 0x37, 0x29, 0x06,
		0x0f, 0x20, 0x3e, 0x11, 0x02, 0x2d, 0x33, 0x1c,
		0x21, 0x0e, 0x10, 0x3f, 0x2c, 0x03, 0x1d, 0x32,
		0x3b, 0x14, 0x0a, 0x25, 0x36, 0x19, 0x07, 0x28
	};

	uint32_t data = (((uint32_t)(len_ret & 0x3ff) << 16) | ctrl);
	uint8_t crc = CRC6_INIT;
	size_t i;

	for (i = 0; i < sizeof(data); i++, data >>= 8) {
		crc = crc6_table[(crc << 2) ^ (data & 0xff)] & 0x3f;
	}

	return crc;
}
#else
#define CRC6_POLY	0x31	/* the polynomial for CRC-6 */
static uint8_t crc6(uint16_t ctrl, uint16_t len_ret)
{
	/* prepare data
	 * lower 10 bits of length/return register is bit 16~25 in data
	 * ctrl register is bit 0~15 in data
	 */
	uint32_t data = (((uint32_t)(len_ret & 0x3ff) << 16) | ctrl);

	uint8_t crc = CRC6_INIT;
	int i;

	for (i = 25; i >= 0; i--) {
		/* XOR the top bit of CRC with the current data bit */
		uint8_t bit = (data >> i) & 1;

		if (((crc >> 5) & 1) ^ bit) {
			/* shift and XOR with the polynomial */
			crc = (crc << 1) ^ CRC6_POLY;
		} else {
			/* just shift left if no XOR is needed */
			crc <<= 1;
		}

		/* keep CRC within 6 bits */
		crc &= 0x3f;
	}

	return crc;
}
#endif

static void crc6_upd(uint16_t *pctrl, uint16_t *plenret)
{
	uint16_t crc;
	uint16_t ctrl, len_ret;

#if IS_ENABLED(MDIO_HBER_DBG)
	if (*plenret > MMD_API_PARAM_LEN_MSK || (*pctrl & BIT(13))) {
		pr_err("mxl:%s:%u: pctrl %04x, plenret %04x\n",
		       __func__, __LINE__, *pctrl, *plenret);
	}
#endif

	/* set bit 13 for magic */
	*pctrl |= BIT(13);

	/* generate CRC6 */
	crc = crc6(*pctrl, *plenret);

	/* put CRC MSB in ctrl bit 13~14 */
	ctrl = (*pctrl & ~0x6000);
	ctrl |= ((crc & 0x30) << 9);

	/* put CRC LSB in len_ret bit 10~13 */
	len_ret = *plenret | ((crc & 0x0f) << 10);

	/* put Ctrl bit 13 (duplication of busy bit) in len_ret bit 15
	 * and Ctrl bit 14 (CRC check flag) in len_ret bit 14
	 */
	len_ret |= ((*pctrl & BIT(14)) | ((*pctrl & BIT(13)) << 2));

	*pctrl = ctrl;
	*plenret = len_ret;
}

static bool crc6_chk(uint16_t ctrl, uint16_t len_ret)
{
	uint16_t crc1, crc2;

	/* extract CRC6 */
	crc1 = ((ctrl >> 9) & 0x30) | ((len_ret >> 10) & 0x0f);

	/* restore ctrl bit 13 (0) and 14 (MSB of return value) */
	ctrl &= ~0x6000;
	ctrl |= (len_ret & MMD_API_RET_MSB_MSK);
	ctrl |= ((len_ret & BIT(15)) >> 2);

	crc2 = crc6(ctrl, len_ret);

	if (crc1 != crc2) {
#if IS_ENABLED(MDIO_HBER_DBG)
		pr_warn("mxl:%s:%u: ctrl %04x, len_ret %04x, crc %04x calc %04x",
			__func__, __LINE__, ctrl, len_ret & 0x3ff, crc1, crc2);
#endif
		return false;
	}

	return true;
}

/* retry read for MDIO bus access failure */
static int __mxl862xx_read_retry(const mxl862xx_device_t *dev, uint32_t regaddr)
{
	int ret, i;

	for (i = 0; i < MDIO_ACCESS_RETRY; i++) {
		ret = mxl862xx_read(dev, regaddr);

		/* return valid result */
		if (ret >= 0)
			return ret;

		/* retry read if IO error happened */
		if (mdio_msecs)
			msleep(mdio_msecs);
	}

#if IS_ENABLED(MDIO_HBER_DBG)
	pr_err("mxl:%s:%u: reg %02x, ret %d", __func__, __LINE__,
	       regaddr, ret);
#endif
	return ret;
}

/* retry write for MDIO bus access failure */
static int __mxl862xx_write_retry(const mxl862xx_device_t *dev,
				  uint32_t regaddr, uint16_t data)
{
	int ret, i;

	for (i = 0; i < MDIO_ACCESS_RETRY; i++) {
		ret = mxl862xx_write(dev, regaddr, data);
		if (ret == 0)
			return 0;

		/* retry write if IO error happened */
		if (mdio_msecs)
			msleep(mdio_msecs);
	}

#if IS_ENABLED(MDIO_HBER_DBG)
	pr_err("mxl:%s:%u: reg %02x, data %04x, ret %d", __func__, __LINE__,
	       regaddr, data, ret);
#endif
	return ret < 0 ? ret : -EIO;
}

/* retry write if read back value mismatch */
static int __mxl862xx_write_ver(const mxl862xx_device_t *dev,
				uint32_t regaddr, uint16_t data)
{
	int ret, i;

	for (i = 0; i < MDIO_ACCESS_RETRY; i++) {
		ret = mxl862xx_write(dev, regaddr, data);

		/* retry write if IO error happened */
		if (ret != 0) {
			if (mdio_msecs)
				msleep(mdio_msecs);
			continue;
		}

		ret = __mxl862xx_read_retry(dev, regaddr);

		/* return success if data read is valid and
		 * equals to data written
		 */
		if (ret >= 0 && (uint16_t)ret == data)
			return 0;
	}

#if IS_ENABLED(MDIO_HBER_DBG)
	pr_err("mxl:%s:%u: reg %02x, data %04x, ret %d", __func__, __LINE__,
	       regaddr, data, ret);
#endif
	/* return -EIO if no valid error code
	 * return error code otherwise
	 */
	return ret < 0 ? ret : -EIO;
}

static int __wait_ctrl_busy(const mxl862xx_device_t *dev)
{
	int ret, i;

	for (i = 0; i < MAX_BUSY_LOOP; i++) {
		ret = mxl862xx_read(dev, MXL862XX_MMD_REG_CTRL);
		if (ret < 0) {
			goto busy_check_exit;
		}

		if (!(ret & CTRL_BUSY_MASK)) {
			ret = 0;
			goto busy_check_exit;
		}

		usleep_range(WAIT_MIN, WAIT_MAX);
	}
	ret = -ETIMEDOUT;
busy_check_exit:
	return ret;
}

static int __wait_ctrl_busy_crc(const mxl862xx_device_t *dev, int16_t *plenret)
{
	int ret, i;
	uint32_t c_bak = ~0, r_bak = ~0, cnt = 0;

	for (i = 0; i < MAX_BUSY_LOOP; i++) {
		uint16_t ctrl, len_ret;

		ret = __mxl862xx_read_retry(dev, MXL862XX_MMD_REG_CTRL);
		if (ret < 0) {
#if IS_ENABLED(MDIO_HBER_DBG)
			pr_err("mxl:%s:%u: CTRL register read failed: %d",
			       __func__, __LINE__, ret);
#endif
			goto busy_check_exit;
		}

		if ((ret & CTRL_BUSY_MASK) != 0) {
			/* retry if BUSY bit is not cleared */
			usleep_range(WAIT_MIN, WAIT_MAX);
			continue;
		}

		ctrl = (ret & 0xffff);

		ret = __mxl862xx_read_retry(dev, MXL862XX_MMD_REG_LEN_RET);
		if (ret < 0) {
#if IS_ENABLED(MDIO_HBER_DBG)
			pr_err("mxl:%s:%u: LEN/RET register read failed: %d",
			       __func__, __LINE__, ret);
#endif
			goto busy_check_exit;
		}

		len_ret = (ret & 0xffff);

		if ((len_ret & BIT(15)) == 0) {
#if IS_ENABLED(MDIO_HBER_DBG)
			pr_err("mxl:%s:%u: magic missing in len_ret %04x, ctrl %04x",
			       __func__, __LINE__, len_ret, ctrl);
#endif
			ret = MMD_API_CRC6_ERR;
			goto busy_check_exit;
		}

		if (!crc6_chk(ctrl, len_ret)) {
			/* checksum error */
			if (c_bak != ctrl || r_bak != len_ret) {
				/* read different value
				 * could be random error
				 */
				c_bak = ctrl;
				r_bak = len_ret;
				cnt = 0;
				/* retry */
				continue;
			}
			cnt++;
			if (cnt >= MDIO_ACCESS_RETRY) {
				/* read same wrong value multiple times
				 * suspect broken in CL22 Ext for CL45
				 * redo the command
				 */
#if IS_ENABLED(MDIO_HBER_DBG)
				pr_err("mxl:%s:%u: crc6 fail %u times, ctrl %04x, len_ret %04x",
			       __func__, __LINE__, cnt, ctrl, len_ret);
#endif
				ret = MMD_API_CRC16_ERR;
				goto busy_check_exit;
			}
		}

		ret = 0;

		if (!plenret) {
			/* no need length/return register value
			 * return success
			 */
			goto busy_check_exit;
		}

		if ((len_ret & MMD_API_RET_MSB_MSK) != 0) {
			/* return value MSB decides negative value (sign) */
			*plenret = (int16_t)(len_ret | ~MMD_API_RET_LSB_MSK);
		} else {
			/* extract return value LSB */
			*plenret = (int16_t)(len_ret & MMD_API_RET_LSB_MSK);
		}

#if IS_ENABLED(MDIO_HBER_DBG)
		if (*plenret != 0) {
			pr_err("mxl:%s:%u: ctrl %04x, len_ret %04x, *plenret: %d",
			       __func__, __LINE__, ctrl, len_ret, *plenret);
		}
#endif
		goto busy_check_exit;
	}

	ret = -ETIMEDOUT;
busy_check_exit:
	return ret;
}

static int __mxl862xx_rst_data(const mxl862xx_device_t *dev)
{
	int ret;

	ret = __mxl862xx_write_ver(dev, MXL862XX_MMD_REG_LEN_RET, 0);
	if (ret < 0)
		return ret;

	ret = mxl862xx_write(dev, MXL862XX_MMD_REG_CTRL,
			MMD_API_RST_DATA | CTRL_BUSY_MASK);
	if (ret < 0)
		return ret;

	return __wait_ctrl_busy(dev);
}

static int __mxl862xx_rst_data_crc(const mxl862xx_device_t *dev)
{
	int ret;
	uint16_t ctrl, len_ret;
	int i;

	/* prepare ctrl and length/return with CRC check enable
	 * and CRC6
	 */
	len_ret = 0;
	ctrl = CTRL_BUSY_MASK | MMD_API_CRC_CHK | MMD_API_RST_DATA;
	crc6_upd(&ctrl, &len_ret);

	for (i = 0; i < MDIO_ACCESS_RETRY; i++) {
		int16_t sw_ret = 0;

		/* write length/return value and read back to verify */
		ret = __mxl862xx_write_ver(dev, MXL862XX_MMD_REG_LEN_RET,
					   len_ret);
		if (ret < 0)
			return ret;

		/* write ctrl value and rewrite if IO error happened */
		ret = __mxl862xx_write_retry(dev, MXL862XX_MMD_REG_CTRL, ctrl);
		if (ret < 0)
			return ret;

		/* wait for busy bit clear and return value received */
		ret = __wait_ctrl_busy_crc(dev, &sw_ret);
		/* retry if CRC6 errror detected/suspected locally */
		if (ret == MMD_API_CRC6_ERR)
			continue;
		/* return unhandled error */
		if (ret < 0)
			return ret;

		/* retry if CRC6 error detected by switch */
		if (sw_ret == MMD_API_CRC6_ERR)
			continue;

		/* return success as long as reset command tansmitted
		 * successfully
		 */
		return 0;
	}

	/* return IO error if all attempts failed */
	return -EIO;
}

static int __mxl862xx_set_data(const mxl862xx_device_t *dev, uint16_t words)
{
	int ret;
	uint16_t cmd;

	ret = __mxl862xx_write_ver(dev, MXL862XX_MMD_REG_LEN_RET,
				   DATA_MAX_WORD * sizeof(uint16_t));
	if (ret < 0)
		return ret;

	cmd = words / DATA_MAX_WORD - 1;
	if (!(cmd < 2))
		return -EINVAL;

	cmd += MMD_API_SET_DATA_0;
	ret = mxl862xx_write(dev, MXL862XX_MMD_REG_CTRL, cmd | CTRL_BUSY_MASK);
	if (ret < 0)
		return ret;

	return __wait_ctrl_busy(dev);
}

static int __mxl862xx_set_data_crc(const mxl862xx_device_t *dev, uint16_t words)
{
	int ret;
	uint16_t cmd, ctrl, len_ret;
	int i;

	cmd = words / DATA_MAX_WORD - 1;
	if (!(cmd < 2))
		return -EINVAL;
	cmd += MMD_API_SET_DATA_0;

	/* prepare ctrl and length/return with CRC check enable
	 * and CRC6
	 */
	len_ret = DATA_MAX_WORD * sizeof(uint16_t);
	ctrl = CTRL_BUSY_MASK | MMD_API_CRC_CHK | cmd;
	crc6_upd(&ctrl, &len_ret);

	for (i = 0; i < MDIO_ACCESS_RETRY; i++) {
		int16_t sw_ret = 0;

		/* write length/return value and read back to verify */
		ret = __mxl862xx_write_ver(dev, MXL862XX_MMD_REG_LEN_RET,
					   len_ret);
		if (ret < 0)
			return ret;

		/* write ctrl value and rewrite if IO error happened */
		ret = __mxl862xx_write_retry(dev, MXL862XX_MMD_REG_CTRL, ctrl);
		if (ret < 0)
			return ret;

		/* wait for busy bit clear and return value received */
		ret = __wait_ctrl_busy_crc(dev, &sw_ret);
		/* retry if CRC6 errror detected/suspected locally */
		if (ret == MMD_API_CRC6_ERR)
			continue;
		/* return unhandled error */
		if (ret < 0)
			return ret;

		/* retry if CRC6 error detected by switch */
		if (sw_ret == MMD_API_CRC6_ERR)
			continue;

		return (int)sw_ret; /* should be 0 for this command */
	}

	/* return IO error if all attempts failed */
	return -EIO;
}

static int __mxl862xx_get_data(const mxl862xx_device_t *dev, uint16_t words)
{
	int ret;
	uint16_t cmd;

	ret = __mxl862xx_write_ver(dev, MXL862XX_MMD_REG_LEN_RET,
				   DATA_MAX_WORD * sizeof(uint16_t));
	if (ret < 0)
		return ret;

	cmd = words / DATA_MAX_WORD;
	if (!(cmd > 0 && cmd < 3))
		return -EINVAL;
	cmd += MMD_API_GET_DATA_0;
	ret = mxl862xx_write(dev, MXL862XX_MMD_REG_CTRL, cmd | CTRL_BUSY_MASK);
	if (ret < 0)
		return ret;

	return __wait_ctrl_busy(dev);
}

static int __mxl862xx_get_data_crc(const mxl862xx_device_t *dev, uint16_t words)
{
	int ret;
	uint16_t cmd, ctrl, len_ret;
	int i;

	cmd = words / DATA_MAX_WORD;
	if (!(cmd > 0 && cmd < 3))
		return -EINVAL;
	cmd += MMD_API_GET_DATA_0;

	/* prepare ctrl and length/return with CRC check enable
	 * and CRC6
	 */
	len_ret = DATA_MAX_WORD * sizeof(uint16_t);
	ctrl = CTRL_BUSY_MASK | MMD_API_CRC_CHK | cmd;
	crc6_upd(&ctrl, &len_ret);

	for (i = 0; i < MDIO_ACCESS_RETRY; i++) {
		int16_t sw_ret = 0;

		/* write length/return value and read back to verify */
		ret = __mxl862xx_write_ver(dev, MXL862XX_MMD_REG_LEN_RET,
					   len_ret);
		if (ret < 0)
			return ret;

		/* write ctrl value and rewrite if IO error happened */
		ret = __mxl862xx_write_retry(dev, MXL862XX_MMD_REG_CTRL, ctrl);
		if (ret < 0)
			return ret;

		/* wait for busy bit clear and return value received */
		ret = __wait_ctrl_busy_crc(dev, &sw_ret);
		/* retry if CRC6 errror detected/suspected locally */
		if (ret == MMD_API_CRC6_ERR)
			continue;
		/* return unhandled error */
		if (ret < 0)
			return ret;

		/* retry if CRC6 error detected by switch */
		if (sw_ret == MMD_API_CRC6_ERR)
			continue;

		return (int)sw_ret; /* should be 0 for this command */
	}

	/* return IO error if all attempts failed */
	return -EIO;
}

static int __mxl862xx_send_cmd(const mxl862xx_device_t *dev, uint16_t cmd, uint16_t size,
			  int16_t *presult)
{
	int ret;

	ret = __mxl862xx_write_ver(dev, MXL862XX_MMD_REG_LEN_RET, size);
	if (ret < 0) {
		return ret;
	}

	ret = mxl862xx_write(dev, MXL862XX_MMD_REG_CTRL, cmd | CTRL_BUSY_MASK);
	if (ret < 0) {
		return ret;
	}

	ret = __wait_ctrl_busy(dev);
	if (ret < 0) {
		return ret;
	}

	ret = mxl862xx_read(dev, MXL862XX_MMD_REG_LEN_RET);
	if (ret < 0) {
		return ret;
	}

	*presult = ret;
	return 0;
}

static int __mxl862xx_send_cmd_crc(const mxl862xx_device_t *dev, uint16_t cmd,
				   uint16_t size, int16_t *presult)
{
	int ret;
	uint16_t ctrl, len_ret;
	int i;

	/* prepare ctrl and length/return with CRC check enable
	 * and CRC6
	 */
	len_ret = size;
	ctrl = CTRL_BUSY_MASK | MMD_API_CRC_CHK | cmd;
	crc6_upd(&ctrl, &len_ret);

#if IS_ENABLED(MDIO_HBER_DBG)
	{
		static uint32_t dbg_data[256] = {0};
		static uint16_t dbg_pos = 0;

		uint32_t tmp = (((uint32_t)len_ret << 16) | ctrl);
		uint16_t i;

		for (i = 0; i < dbg_pos && i < 256; i++) {
			if (dbg_data[i] == tmp)
				break;
		}
		if (i == dbg_pos && dbg_pos < 256) {
			pr_err("mxl:%s:%u: dbg_data[%u] %08x",
			       __func__, __LINE__, dbg_pos, tmp);
			dbg_data[dbg_pos] = tmp;
			dbg_pos++;
		}
	}
#endif

	for (i = 0; i < MDIO_ACCESS_RETRY; i++) {
		int16_t sw_ret = 0;

		/* write length/return value and read back to verify */
		ret = __mxl862xx_write_ver(dev, MXL862XX_MMD_REG_LEN_RET,
					   len_ret);
		if (ret < 0)
			return ret;

		/* write ctrl value and rewrite if IO error happened */
		ret = __mxl862xx_write_retry(dev, MXL862XX_MMD_REG_CTRL, ctrl);
		if (ret < 0)
			return ret;

		/* wait for busy bit clear and return value received */
		ret = __wait_ctrl_busy_crc(dev, &sw_ret);
		/* retry if CRC6 errror detected/suspected locally */
		if (ret == MMD_API_CRC6_ERR)
			continue;
		/* return unhandled error */
		if (ret < 0)
			return ret;

		/* retry if CRC6 error detected by switch */
		if (sw_ret == MMD_API_CRC6_ERR)
			continue;

		*presult = sw_ret;

		return 0;
	}

	/* return IO error if all attempts failed */
	return -EIO;
}

static bool __mxl862xx_cmd_r_valid(uint16_t cmd_r)
{
#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	return (shadow.ctrl == cmd_r && shadow.ret >= 0) ? true : false;
#else
	return false;
#endif
}

/* This is usually used to implement CFG_SET command.
 * With previous CFG_GET command executed properly, the retrieved data
 * are shadowed in local structure. WSP FW has a set of shadow too,
 * so that only the difference to be sent over SMDIO.
 */
static int __mxl862xx_api_wrap_cmd_r(const mxl862xx_device_t *dev, uint16_t cmd,
				void *pdata, uint16_t size, uint16_t r_size)
{
#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	int ret;

	uint16_t max, i;
	uint16_t *data;
	int16_t result = 0;

	max = (size + 1) / 2;
	data = pdata;

	ret = __wait_ctrl_busy(dev);
	if (ret < 0) {
		return ret;
	}

	for (i = 0; i < max; i++) {
		uint16_t off = i % DATA_MAX_WORD;

		if (i && off == 0) {
			/* Send command to set data when every
			 * DATA_MAX_WORD of WORDs are written and reload
			 * next batch of data from last CFG_GET.
			 */
			ret = __mxl862xx_set_data(dev, i);
			if (ret < 0) {
				return ret;
			}
		}

		if (data[i] == shadow.data.data[i])
			continue;

		mxl862xx_write(dev, DATA_REG_FIRST + off, le16_to_cpu(data[i]));
	}

	ret = __mxl862xx_send_cmd(dev, cmd, size, &result);
	if (ret < 0) {
		return ret;
	}

	if (result < 0) {
		return result;
	}

	max = (r_size + 1) / 2;
	for (i = 0; i < max; i++) {
		uint16_t off = i % DATA_MAX_WORD;

		if (i && off == 0) {
			/* Send command to fetch next batch of data
			 * when every DATA_MAX_WORD of WORDs are read.
			 */
			ret = __mxl862xx_get_data(dev, i);
			if (ret < 0) {
				return ret;
			}
		}

		ret = mxl862xx_read(dev, DATA_REG_FIRST + off);
		if (ret < 0) {
			return ret;
		}

		if ((i * 2 + 1) == r_size) {
			/* Special handling for last BYTE
			 * if it's not WORD aligned.
			 */
			*(uint8_t *)&data[i] = ret & 0xFF;
		} else {
			data[i] = cpu_to_le16((uint16_t)ret);
		}
	}

	shadow.data.data[max] = 0;
	memcpy(shadow.data.data, data, r_size);

	return result;
#else /* defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT */
	ARG_UNUSED(dev);
	ARG_UNUSED(cmd);
	ARG_UNUSED(pdata);
	ARG_UNUSED(size);
	return -ENOTSUP;
#endif /* defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT */
}

/* This is v1 (CRC check) version of __mxl862xx_api_wrap_cmd_r */
static int __mxl862xx_api_wrap_cmd_r_crc(const mxl862xx_device_t *dev,
					 uint16_t cmd, uint16_t *wdata,
					 uint16_t *rdata, uint16_t size)
{
#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	int ret;

	uint16_t max, i, j;
	int16_t result = 0;

	if (size > 0)
		max = (size + 3) / 2;
	else
		max = 0;

	ret = __wait_ctrl_busy_crc(dev, NULL);
	if (ret < 0 &&
	    (ret != MMD_API_CRC6_ERR ||
	     __wait_ctrl_busy(dev) < 0)) {
#if IS_ENABLED(MDIO_HBER_DBG)
		pr_err("mxl:%s:%u: __wait_ctrl_busy_crc(dev %p), ret %d\n",
		       __func__, __LINE__, dev, ret);
#endif
		return ret;
	}

	for (i = 0; i < max; i++) {
		uint16_t off = i % DATA_MAX_WORD;

		if (i && off == 0) {
			/* Send command to set data when every
			 * DATA_MAX_WORD of WORDs are written and reload
			 * next batch of data from last CFG_GET.
			 */
			ret = __mxl862xx_set_data_crc(dev, i);
#if IS_ENABLED(MDIO_HBER_DBG)
			if (ret != 0) {
				pr_err("mxl:%s:%u: __mxl862xx_set_data_crc(dev %p, i %u), ret %d\n",
				       __func__, __LINE__, dev, (uint32_t)i,
				       ret);
			}
#endif
			if (ret < 0) {
				return ret;
			}
		}

		if (wdata[i] == shadow.data.data[i])
			continue;

		__mxl862xx_write_ver(dev, DATA_REG_FIRST + off,
				     le16_to_cpu(wdata[i]));
	}

	ret = __mxl862xx_send_cmd_crc(dev, cmd, size, &result);
#if IS_ENABLED(MDIO_HBER_DBG)
	if (ret != 0) {
		pr_err("mxl:%s:%u: __mxl862xx_send_cmd_crc(dev %p, cmd %04x, size %u), ret %d\n",
		       __func__, __LINE__, dev, (uint32_t)cmd, (uint32_t)size,
		       ret);
	}
#endif
	if (ret < 0) {
		return ret;
	}

#if IS_ENABLED(MDIO_HBER_DBG)
	if (result != 0) {
		pr_err("mxl:%s:%u: __mxl862xx_send_cmd_crc(dev %p, cmd %04x, size %u), result %d\n",
		       __func__, __LINE__, dev, (uint32_t)cmd, (uint32_t)size,
		       (int32_t)result);
	}
#endif
	if (result < 0) {
		/* let upper layer to handle MMD_API_CRC16_ERR and other error
		 * because the last result data is spoiled
		 */
		return result;
	}

	/* no need fetch data, should not happen in GET/SET pair */
	if (size == 0)
		return result;

	for (j = 0; j < MDIO_ACCESS_RETRY; j++) {
		for (i = 0; i < max; i++) {
			uint16_t off = i % DATA_MAX_WORD;

			if (i && off == 0) {
				/* Send command to fetch next batch of data
				 * when every DATA_MAX_WORD of WORDs are read.
				 */
				ret = __mxl862xx_get_data_crc(dev, i);
#if IS_ENABLED(MDIO_HBER_DBG)
				if (ret != 0) {
					pr_err("mxl:%s:%u: __mxl862xx_get_data_crc(dev %p, i %u), ret %d\n",
					       __func__, __LINE__, dev,
					       (uint32_t)i, ret);
				}
#endif
				if (ret < 0) {
					return ret;
				}
			}

			ret = __mxl862xx_read_retry(dev, DATA_REG_FIRST + off);
			if (ret < 0) {
#if IS_ENABLED(MDIO_HBER_DBG)
				pr_err("mxl:%s:%u: mxl862xx_read_retry(dev %p, off %u), ret %d\n",
				       __func__, __LINE__, dev,
				       (uint32_t)(DATA_REG_FIRST + off), ret);
#endif
				return ret;
			}

			if ((i * 2 + 1) == size + 2) {
				/* Special handling for last BYTE
				 * if it's not WORD aligned.
				 */
				*(uint8_t *)&rdata[i] = ret & 0xFF;
			} else {
				rdata[i] = cpu_to_le16((uint16_t)ret);
			}
		}

		/* exit if CRC verification succeeded */
		if (crc16(0xffff, (void *)rdata, size + 2) == 0)
			break;
	}
	/* return IO error if all attempts failed */
	if (j == MDIO_ACCESS_RETRY)
		return -EIO;

	memcpy(shadow.data.data, rdata, max * 2);

	return result;
#else /* defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT */
	ARG_UNUSED(dev);
	ARG_UNUSED(cmd);
	ARG_UNUSED(pdata);
	ARG_UNUSED(size);
	return -ENOTSUP;
#endif /* defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT */
}

int __mxl862xx_api_wrap(const mxl862xx_device_t *dev, uint16_t cmd, void *pdata,
			uint16_t size, uint16_t cmd_r, uint16_t r_size)
{
	int ret;
	uint16_t max, i, cnt;
	uint16_t *data;
	int16_t result = 0;

	if (!dev || (!pdata && size)) {
#if IS_ENABLED(MDIO_HBER_DBG)
		pr_err("mxl:%s:%u: dev %p, pdata %p, size %u, ret %d\n",
		       __func__, __LINE__, dev, pdata, (uint32_t)size, -EINVAL);
#endif
		return -EINVAL;
	}

	if (!(size <= sizeof(mmd_api_data_t)) || !(r_size <= size)) {
#if IS_ENABLED(MDIO_HBER_DBG)
		pr_err("mxl:%s:%u: size %u, sizeof(mmd_api_data_t) %u, r_size %u, ret %d\n",
		       __func__, __LINE__, (uint32_t)size,
		       (uint32_t)sizeof(mmd_api_data_t),
		       (uint32_t)r_size, -EINVAL);
#endif
		return -EINVAL;
	}

	if (__mxl862xx_cmd_r_valid(cmd_r)) {
		/* Special handling for GET and SET command pair. */
		ret = __mxl862xx_api_wrap_cmd_r(dev, cmd, pdata, size, r_size);
#if IS_ENABLED(MDIO_HBER_DBG)
		if (ret != 0) {
			pr_err("mxl:%s:%u: __mxl862xx_api_wrap_cmd_r(dev %p, cmd %04x, size %u, r_size %u), ret %d\n",
			       __func__, __LINE__, dev, (uint32_t)cmd,
			       (uint32_t)size, (uint32_t)r_size, ret);
		}
#endif
		goto EXIT;
	}

	max = (size + 1) / 2;
	data = pdata;

	/* Check whether it's worth to issue RST_DATA command. */
	for (i = cnt = 0; i < max && cnt < THR_RST_DATA; i++) {
		if (!data[i])
			cnt++;
	}

	ret = __wait_ctrl_busy(dev);
#if IS_ENABLED(MDIO_HBER_DBG)
	if (ret != 0) {
		pr_err("mxl:%s:%u: __wait_ctrl_busy(dev %p), ret %d\n",
		       __func__, __LINE__, dev, ret);
	}
#endif
	if (ret < 0) {
		goto EXIT;
	}

	if (cnt >= THR_RST_DATA) {
		/* Issue RST_DATA commdand. */
		ret = __mxl862xx_rst_data(dev);
#if IS_ENABLED(MDIO_HBER_DBG)
		if (ret != 0) {
			pr_err("mxl:%s:%u: __mxl862xx_rst_data(dev %p), ret %d\n",
			       __func__, __LINE__, dev, ret);
		}
#endif
		if (ret < 0)
			goto EXIT;

		for (i = 0, cnt = 0; i < max; i++) {
			uint16_t off = i % DATA_MAX_WORD;

			if (i && off == 0) {
				uint16_t cnt_old = cnt;

				cnt = 0;

				/* No actual data was written. */
				if (!cnt_old)
					continue;

				/* Send command to set data when every
				 * DATA_MAX_WORD of WORDs are written and
				 * clear the MMD register space.
				 */
				ret = __mxl862xx_set_data(dev, i);
#if IS_ENABLED(MDIO_HBER_DBG)
				if (ret != 0) {
					pr_err("mxl:%s:%u: __mxl862xx_set_data(dev %p), ret %d\n",
					       __func__, __LINE__, dev, ret);
				}
#endif
				if (ret < 0)
					goto EXIT;
			}

			/* Skip '0' data. */
			if (!data[i])
				continue;

			mxl862xx_write(dev, DATA_REG_FIRST + off,
				       le16_to_cpu(data[i]));
			cnt++;
		}
	} else {
		for (i = 0; i < max; i++) {
			uint16_t off = i % DATA_MAX_WORD;

			if (i && off == 0) {
				/* Send command to set data when every
				 * DATA_MAX_WORD of WORDs are written.
				 */
				ret = __mxl862xx_set_data(dev, i);
#if IS_ENABLED(MDIO_HBER_DBG)
				if (ret != 0) {
					pr_err("mxl:%s:%u: __mxl862xx_set_data(dev %p, i %u), ret %d\n",
					       __func__, __LINE__, dev,
					       (uint32_t)i, ret);
				}
#endif
				if (ret < 0)
					goto EXIT;
			}

			mxl862xx_write(dev, DATA_REG_FIRST + off,
				       le16_to_cpu(data[i]));
		}
	}

	ret = __mxl862xx_send_cmd(dev, cmd, size, &result);
#if IS_ENABLED(MDIO_HBER_DBG)
	if (ret != 0) {
		pr_err("mxl:%s:%u: __mxl862xx_send_cmd(dev %p, cmd %04x, size %u), ret %d\n",
		       __func__, __LINE__, dev, (uint32_t)cmd,
		       (uint32_t)size, ret);
	}
#endif
	if (ret < 0)
		goto EXIT;

#if IS_ENABLED(MDIO_HBER_DBG)
	if (result != 0) {
		pr_err("mxl:%s:%u: __mxl862xx_send_cmd(dev %p, cmd %04x, size %u), result %d\n",
		       __func__, __LINE__, dev, (uint32_t)cmd,
		       (uint32_t)size, (int32_t)result);
	}
#endif
	if (result < 0) {
		ret = result;
		goto EXIT;
	}

	max = (r_size + 1) / 2;
	for (i = 0; i < max; i++) {
		uint16_t off = i % DATA_MAX_WORD;

		if (i && off == 0) {
			/* Send command to fetch next batch of data
			 * when every DATA_MAX_WORD of WORDs are read.
			 */
			ret = __mxl862xx_get_data(dev, i);
#if IS_ENABLED(MDIO_HBER_DBG)
			if (ret != 0) {
				pr_err("mxl:%s:%u: __mxl862xx_get_data(dev %p, i %u), ret %d\n",
				       __func__, __LINE__, dev, (uint32_t)i,
				       ret);
			}
#endif
			if (ret < 0)
				goto EXIT;
		}

		ret = mxl862xx_read(dev, DATA_REG_FIRST + off);
		if (ret < 0) {
#if IS_ENABLED(MDIO_HBER_DBG)
			pr_err("mxl:%s:%u: mxl862xx_read(dev %p, off %u), ret %d\n",
			       __func__, __LINE__, dev,
			       (uint32_t)(DATA_REG_FIRST + off), ret);
#endif
			goto EXIT;
		}

		if ((i * 2 + 1) == r_size) {
			/* Special handling for last BYTE
			 * if it's not WORD aligned.
			 */
			*(uint8_t *)&data[i] = ret & 0xFF;
		} else {
			data[i] = cpu_to_le16((uint16_t)ret);
		}
	}

#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	if ((cmd != 0x1801) && (cmd != 0x1802))
		shadow.data.data[max] = 0;
	memcpy(shadow.data.data, data, r_size);
#endif

	ret = result;

EXIT:
#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	shadow.ctrl = cmd;
	shadow.ret = ret;
#endif
	return ret;
}

static int __mxl862xx_api_wrap_crc(const mxl862xx_device_t *dev, uint16_t cmd,
				   void *pdata, uint16_t size, uint16_t cmd_r,
				   uint16_t r_size)
{
	/* command buffer protected by lock */
	static uint16_t wdata[sizeof(mmd_api_data_t)] = {0};
	static uint16_t rdata[sizeof(mmd_api_data_t)] = {0};

	int ret;
	uint16_t max, i, j, cnt;
	int16_t result = 0;

	if (!dev || (!pdata && size)) {
#if IS_ENABLED(MDIO_HBER_DBG)
		pr_err("mxl:%s:%u: dev %p, pdata %p, size %u, ret %d\n",
		       __func__, __LINE__, dev, pdata, (uint32_t)size, -EINVAL);
#endif
		return -EINVAL;
	}

	if (!(size + 2 <= sizeof(mmd_api_data_t)) || !(r_size <= size)) {
#if IS_ENABLED(MDIO_HBER_DBG)
		pr_err("mxl:%s:%u: size %u, sizeof(mmd_api_data_t) %u, r_size %u, ret %d\n",
		       __func__, __LINE__, (uint32_t)size,
		       (uint32_t)sizeof(mmd_api_data_t), (uint32_t)r_size,
		       -EINVAL);
#endif
		return -EINVAL;
	}

	/* cache data and append CRC16 */
	if (size > 0) {
		uint16_t crc;

		wdata[(size + 1) / 2] = 0;	/* clear last word to 0 */
		memcpy(wdata, pdata, size);
		crc = cpu_to_le16(crc16(0xffff, (void *)wdata, size));
		memcpy((uint8_t *)wdata + size, &crc, 2);	/* attach CRC */

		max = (size + 3) / 2;
	} else {
		max = 0;
	}

	if (__mxl862xx_cmd_r_valid(cmd_r)) {
		/* Special handling for GET and SET command pair. */
		ret = __mxl862xx_api_wrap_cmd_r_crc(dev, cmd, wdata, rdata,
						    size);
#if IS_ENABLED(MDIO_HBER_DBG)
		if (ret != 0) {
			pr_err("mxl:%s:%u: __mxl862xx_api_wrap_cmd_r_crc(dev %p, cmd %04x, size %u, r_size %u), ret %d\n",
			       __func__, __LINE__, dev, (uint32_t)cmd,
			       (uint32_t)size, (uint32_t)r_size, ret);
		}
#endif
		/* re-send data if data transmission error
		 * otherwise, exit
		 */
		if (ret != MMD_API_CRC16_ERR)
			goto EXIT;
	}

	/* Check whether it's worth to issue RST_DATA command. */
	for (i = cnt = 0; i < max && cnt < THR_RST_DATA; i++) {
		if (!wdata[i])
			cnt++;
	}

	ret = __wait_ctrl_busy_crc(dev, NULL);
	if (ret < 0 &&
	    (ret != MMD_API_CRC6_ERR ||
	     __wait_ctrl_busy(dev) < 0)) {
#if IS_ENABLED(MDIO_HBER_DBG)
		pr_err("mxl:%s:%u: __wait_ctrl_busy_crc(dev %p), ret %d\n",
		       __func__, __LINE__, dev, ret);
#endif
		goto EXIT;
	}

	for (j = 0; j < MDIO_ACCESS_RETRY; j++) {
		if (cnt >= THR_RST_DATA) {
			/* Issue RST_DATA commdand. */
			ret = __mxl862xx_rst_data_crc(dev);
#if IS_ENABLED(MDIO_HBER_DBG)
			if (ret != 0) {
				pr_err("mxl:%s:%u: __mxl862xx_rst_data_crc(dev %p), ret %d\n",
				       __func__, __LINE__, dev, ret);
			}
#endif
			if (ret < 0)
				goto EXIT;

			for (i = 0, cnt = 0; i < max; i++) {
				uint16_t off = i % DATA_MAX_WORD;

				if (i && off == 0) {
					uint16_t cnt_old = cnt;

					cnt = 0;

					/* No actual data was written. */
					if (!cnt_old)
						continue;

					/* Send command to set data when every
					 * DATA_MAX_WORD of WORDs are written
					 * and clear the MMD register space.
					 */
					ret = __mxl862xx_set_data_crc(dev, i);
#if IS_ENABLED(MDIO_HBER_DBG)
					if (ret != 0) {
						pr_err("mxl:%s:%u: __mxl862xx_set_data_crc(dev %p), ret %d\n",
						       __func__, __LINE__,
						       dev, ret);
					}
#endif
					if (ret < 0)
						goto EXIT;
				}

				/* Skip '0' data. */
				if (!wdata[i])
					continue;

				__mxl862xx_write_ver(dev, DATA_REG_FIRST + off,
						     le16_to_cpu(wdata[i]));
				cnt++;
			}
		} else {
			for (i = 0; i < max; i++) {
				uint16_t off = i % DATA_MAX_WORD;

				if (i && off == 0) {
					/* Send command to set data when every
					 * DATA_MAX_WORD of WORDs are written.
					 */
					ret = __mxl862xx_set_data_crc(dev, i);
#if IS_ENABLED(MDIO_HBER_DBG)
					if (ret != 0) {
						pr_err("mxl:%s:%u: __mxl862xx_set_data_crc(dev %p, i %u), ret %d\n",
						       __func__, __LINE__,
						       dev, (uint32_t)i, ret);
					}
#endif
					if (ret < 0)
						goto EXIT;
				}

				__mxl862xx_write_ver(dev, DATA_REG_FIRST + off,
						     le16_to_cpu(wdata[i]));
			}
		}

		ret = __mxl862xx_send_cmd_crc(dev, cmd, size, &result);
#if IS_ENABLED(MDIO_HBER_DBG)
		if (ret != 0) {
			pr_err("mxl:%s:%u: __mxl862xx_send_cmd_crc(dev %p, cmd %04x, size %u), ret %d\n",
			       __func__, __LINE__, dev, (uint32_t)cmd,
			       (uint32_t)size, ret);
		}
#endif
		/* retry if re-run command is detected locally */
		if (ret == MMD_API_CRC16_ERR)
			continue;
		if (ret < 0)
			goto EXIT;

#if IS_ENABLED(MDIO_HBER_DBG)
		if (result != 0) {
			pr_err("mxl:%s:%u: __mxl862xx_send_cmd_crc(dev %p, cmd %04x, size %u), result %d\n",
			       __func__, __LINE__, dev, (uint32_t)cmd,
			       (uint32_t)size, (int32_t)result);
		}
#endif
		/* retry if CRC16 of data verification failed */
		if (result == MMD_API_CRC16_ERR)
			continue;
		if (result < 0) {
			ret = result;
			goto EXIT;
		} else {
			break;
		}
	}
	/* return IO error if all attempts failed */
	if (j == MDIO_ACCESS_RETRY) {
		ret = -EIO;
		goto EXIT;
	}

	/* no need fetch data */
	if (size == 0) {
		ret = result;
		goto EXIT;
	}

	for (j = 0; j < MDIO_ACCESS_RETRY; j++) {
		for (i = 0; i < max; i++) {
			uint16_t off = i % DATA_MAX_WORD;

			if (i && off == 0) {
				/* Send command to fetch next batch of data
				 * when every DATA_MAX_WORD of WORDs are read.
				 */
				ret = __mxl862xx_get_data_crc(dev, i);
#if IS_ENABLED(MDIO_HBER_DBG)
				if (ret != 0) {
					pr_err("mxl:%s:%u: __mxl862xx_get_data_crc(dev %p, i %u), ret %d\n",
					       __func__, __LINE__, dev,
					       (uint32_t)i, ret);
				}
#endif
				if (ret < 0)
					goto EXIT;
			}

			ret = __mxl862xx_read_retry(dev, DATA_REG_FIRST + off);
			if (ret < 0) {
#if IS_ENABLED(MDIO_HBER_DBG)
				pr_err("mxl:%s:%u: mxl862xx_read_retry(dev %p, off %u), ret %d\n",
				       __func__, __LINE__, dev,
				       (uint32_t)(DATA_REG_FIRST + off), ret);
#endif
				goto EXIT;
			}

			if ((i * 2 + 1) == size + 2) {
				/* Special handling for last BYTE
				 * if it's not WORD aligned.
				 */
				*(uint8_t *)&rdata[i] = ret & 0xFF;
			} else {
				rdata[i] = cpu_to_le16((uint16_t)ret);
			}
		}

		/* exit if CRC verification succeeded */
		if (crc16(0xffff, (void *)rdata, size + 2) == 0)
			break;
	}
	/* return IO error if all attempts failed */
	if (j == MDIO_ACCESS_RETRY) {
		ret = -EIO;
		goto EXIT;
	}

#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	memcpy(shadow.data.data, rdata, max * 2);
#endif

	ret = result;

EXIT:
	if (size > 0 && ret >= 0)
		memcpy(pdata, rdata, r_size);

#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	shadow.ctrl = cmd;
	shadow.ret = ret;
#endif
	return ret;
}

int mxl862xx_api_wrap(const mxl862xx_device_t *dev, uint16_t cmd, void *pdata,
		      uint16_t size, uint16_t cmd_r, uint16_t r_size)
{
	int ret;

	mutex_lock_nested(&dev->bus->mdio_lock, MDIO_MUTEX_NESTED);
	if (crc_chk_en) {
		ret = __mxl862xx_api_wrap_crc(dev, cmd, pdata, size, cmd_r,
					      r_size);
	} else {
		ret = __mxl862xx_api_wrap(dev, cmd, pdata, size, cmd_r, r_size);
	}
	mutex_unlock(&dev->bus->mdio_lock);

	return ret;
}

int mxl862xx_api_crc_chk_en(const mxl862xx_device_t *dev, bool enable,
			    unsigned int msecs)
{
	int ret = 0;
	uint16_t ctrl, len_ret;
	union __attribute__((packed)) {
		struct {
			struct sys_fw_image_version ver;
			uint16_t crc;
		};
		uint16_t data16[(sizeof(struct sys_fw_image_version) + 3) / 2];
		uint8_t data8[sizeof(struct sys_fw_image_version) + 2];
	} parm = {0};
	int i;

	if (!dev || !dev->bus)
		return -EINVAL;

	mutex_lock_nested(&dev->bus->mdio_lock, MDIO_MUTEX_NESTED);

	if (!enable) {
		/* disable CRC check */
		crc_chk_en = false;
		goto EXIT;
	}

	/* set delay during MDIO bus access failure */
	mdio_msecs = msecs;

	if (crc_chk_en) {
		/* CRC check is already enabled */
		goto EXIT;
	}

	/* prepare v1 FW version command (with CRC check) */
	len_ret = sizeof(parm.ver);
	ctrl = CTRL_BUSY_MASK | MMD_API_CRC_CHK | SYS_MISC_FW_VERSION;
	crc6_upd(&ctrl, &len_ret);
#if IS_ENABLED(MDIO_HBER_DBG)
	pr_err("mxl:%s:%u: ctrl %04x, len_ret %04x\n",
	       __func__, __LINE__, ctrl, len_ret);
#endif

	/* prepare data CRC */
	parm.crc = crc16(0xffff, (void *)parm.data8, sizeof(parm.ver));
	parm.crc = cpu_to_le16(parm.crc);

	/* write data */
	for (i = 0; i < ARRAY_SIZE(parm.data16); i++) {
		ret = __mxl862xx_write_ver(dev, DATA_REG_FIRST + i,
					   parm.data16[i]);
		if (ret) {
			/* write with verification failed */
#if IS_ENABLED(MDIO_HBER_DBG)
			pr_err("mxl:%s:%u: MDIO write with verification failed: %d",
				__func__, __LINE__, ret);
#endif
			goto EXIT;
		}
	}

	/* write length/return register and verify read back value */
	ret = __mxl862xx_write_ver(dev, MXL862XX_MMD_REG_LEN_RET, len_ret);
	if (ret) {
		/* write with verification failed */
#if IS_ENABLED(MDIO_HBER_DBG)
		pr_err("mxl:%s:%u: MDIO write with verification failed: %d",
			__func__, __LINE__, ret);
#endif
		goto EXIT;
	}

	/* write control register */
	ret = __mxl862xx_write_retry(dev, MXL862XX_MMD_REG_CTRL, ctrl);
	if (ret) {
		/* write with retry failed */
#if IS_ENABLED(MDIO_HBER_DBG)
		pr_err("mxl:%s:%u: MDIO write with retry failed: %d",
			__func__, __LINE__, ret);
#endif
		goto EXIT;
	}

	/* loop to wait for response of MXL862XX */
	for (i = 0; i < MAX_BUSY_LOOP; i++) {
		ret = __mxl862xx_read_retry(dev, MXL862XX_MMD_REG_CTRL);
		if (ret < 0) {
#if IS_ENABLED(MDIO_HBER_DBG)
			pr_err("mxl:%s:%u: CTRL register read failed: %d",
				__func__, __LINE__, ret);
#endif
			goto EXIT;
		}

		if ((ret & CTRL_BUSY_MASK) != 0) {
			/* retry if BUSY bit is not cleared */
			usleep_range(WAIT_MIN, WAIT_MAX);
			continue;
		}

		ctrl = (ret & 0xffff);

		ret = __mxl862xx_read_retry(dev, MXL862XX_MMD_REG_LEN_RET);
		if (ret < 0) {
#if IS_ENABLED(MDIO_HBER_DBG)
			pr_err("mxl:%s:%u: LEN/RET register read failed: %d",
				__func__, __LINE__, ret);
#endif
			goto EXIT;
		}

		if ((int16_t)ret == (int16_t)-134 ||
		    (int16_t)ret == (int16_t)-22) {
#if IS_ENABLED(MDIO_HBER_DBG)
			/* this is not supported */
			pr_warn("mxl:%s:%u: ENOTSUP (-134) is returned by MXL862XX",
				__func__, __LINE__);
#endif
			ret = -ENOTSUPP;
			goto EXIT;
		}

		len_ret = (ret & 0xffff);

		if (!crc6_chk(ctrl, len_ret)) {
			/* retry if checksum error */
			continue;
		}

		if ((len_ret & MMD_API_RET_MSK) == 0) {
			/* return success, enable CRC check */
			pr_info("%s:%u: CRC check is supported by MXL862XX",
				__func__, __LINE__);
			crc_chk_en = true;
			ret = 0;
		} else {
			/* return error, disable CRC check */
			len_ret &= MMD_API_RET_LSB_MSK;
			pr_warn("%s:%u: error code (%04x - %d) is return by MXL862XX",
				__func__, __LINE__, len_ret,
				(int32_t)(0xfffffc00 | len_ret));
			ret = -ENOTSUPP;
		}
		break;
	}

EXIT:
	mutex_unlock(&dev->bus->mdio_lock);
	return ret;
}

// SPDX-License-Identifier: GPL-2.0
/*
 * drivers/net/dsa/host_api/mxl862xx_api_fw_upgrade.c - dsa driver for Maxlinear mxl862xx switch chips family
 *
 * Copyright (C) 2024 MaxLinear Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <net/dsa.h>
#include "mxl862xx_host_api_impl.h"
#include "mxl862xx_api.h"
#include "mxl862xx_mmd_apis.h"

#define FFU_TURN_OFF_NETDEV	1
#define FFU_ON_NETDEV_AGAIN	0
#define FFU_FILEPATH_OPEN	1

#if !IS_ENABLED(FFU_TURN_OFF_NETDEV)
#undef FFU_ON_NETDEV_AGAIN
#endif

#define IMG_TYPE_MASK		0xFFFAFFFA
#define IMG_TYPE_V1_FULL	0xF48AF48A
#define IMG_TYPE_V1_UPGRADE	0xF48BF48B
#define IMG_TYPE_V2_FULL	0xF48EF48E
#define IMG_TYPE_V2_UPGRADE	0xF48FF48F

#define SB_PDI_CTRL		0xE100
#define SB_PDI_ADDR		0xE101
#define SB_PDI_DATA		0xE102
#define SB_PDI_STAT		0xE103

#define SB_PDI_CTRL_RD		0x01
#define SB_PDI_CTRL_WR		0x02

#define FW_DL_MDIO_RDY_MAGIC	0xC55C
#define FW_DL_MDIO_START_MAGIC	0xF48F
#define FW_DL_MDIO_START_ACK	(FW_DL_MDIO_START_MAGIC + 1)
#define FW_DL_MDIO_TFX_SUCCESS	0x0000
#define FW_DL_MDIO_TFX_FAIL	0x0001
#define FW_DL_MDIO_FINISH_MAGIC	0x3CC3

#define TX_PAGE_SIZE		(32 * 1024)

#define SMDIO_REG_ADDR_REG	0x1F
#define SMDIO_REG_BASE_REG	0x00

/* set base address of SMDIO access
 * SMDIO is using indirect access with regnum = base + offset
 */
static inline int smdio_set_base(const mxl862xx_device_t *dev, u32 base)
{
	return mdiobus_write(dev->bus, dev->sw_addr, SMDIO_REG_ADDR_REG, base);
}

/* read SMDIO register with offset */
static inline int smdio_rd_val(const mxl862xx_device_t *dev, u32 off)
{
	return mdiobus_read(dev->bus, dev->sw_addr, SMDIO_REG_BASE_REG + off);
}

/* write SMDIO register with offset */
static inline int smdio_wr_val(const mxl862xx_device_t *dev, u32 off, u16 val)
{
	return mdiobus_write(dev->bus, dev->sw_addr, SMDIO_REG_BASE_REG + off,
			     val);
}

#ifdef USE_SMDIO_READ_FUNC
/* read SMDIO register with obsolute register number */
static int smdio_read(const mxl862xx_device_t *dev, u32 regnum)
{
	int ret;

	ret = smdio_set_base(dev, regnum);
	if (ret < 0)
		return ret;

	return smdio_rd_val(dev, 0);
}
#endif

/* write SMDIO register with obsolute register number */
static int smdio_write(const mxl862xx_device_t *dev, u32 regnum, u16 val)
{
	int ret;

	ret = smdio_set_base(dev, regnum);
	if (ret < 0)
		return ret;

	return smdio_wr_val(dev, 0, val);
}

/* incremental write SMDIO register starting from obsolute register number (base)
 * the offset is increased for every value write
 * number of data (num) must be less than 31 (0x1f)
 */
static int smdio_write_cont(const mxl862xx_device_t *dev, u32 base,
			    const u16 *data, size_t num)
{
	int ret;
	size_t i;

	ret = smdio_set_base(dev, base);
	if (ret < 0)
		return ret;

	for (i = 0; i < num; i++) {
		ret = smdio_wr_val(dev, i, data[i]);
		if (ret < 0)
			return ret;
	}

	return 0;
}

/* write multiple bytes to single SMDIO register (regnum)
 * since SMDIO register is 16-bit word, every 2 bytes are converted into
 * 1 word in little endian format (1st byte is lower 8 bits and 2nd byte
 * is higher 8 bits).
 * if number of bytes is odd number, last word is padded 0 in higher 8 bits.
 */
static int smdio_write_byte_cont(const mxl862xx_device_t *dev, u32 regnum,
				 const u8 *data, size_t num)
{
	int ret;
	size_t i, _num;

	ret = smdio_set_base(dev, regnum);
	if (ret < 0)
		return ret;

	_num = (num & ~1);
	for (i = 0; i < _num; i += 2) {
		u16 val = ((u16)data[i + 1] << 8) | data[i];

		ret = smdio_wr_val(dev, 0, val);
		if (ret < 0)
			return ret;
	}

	if (_num < num)
		return smdio_wr_val(dev, 0, data[_num]);
	else
		return 0;
}

static void sb_pdi_reset(const mxl862xx_device_t *dev)
{
	const u16 val[3] = {0};
	int ret;

	ret = smdio_write_cont(dev, SB_PDI_CTRL, val, ARRAY_SIZE(val));
	if (ret) {
		dev_err(dev->dev, "SB register reset failed: %d\n", ret);
	}
}

/* wait until expected state value is read */
static bool wait_until(const mxl862xx_device_t *dev, u16 exp_val, u32 timeout)
{
	smdio_set_base(dev, SB_PDI_STAT);

	while (timeout >= 10) {
		int ret = smdio_rd_val(dev, 0);

		if (ret < 0) {
			dev_err(dev->dev, "SB state read failed: %d\n", ret);
		} else if (ret == (int)exp_val) {
			return true;
		}

		msleep(10);
		timeout -= 10;
	}

	return false;
}

/* wait until state value changes from exp_val
 * and return the new value in pret
 */
static bool wait_until_not(const mxl862xx_device_t *dev, u16 exp_val,
			   u16 *pret, u32 timeout)
{
	smdio_set_base(dev, SB_PDI_STAT);

	while (timeout >= 10) {
		int ret = smdio_rd_val(dev, 0);

		if (ret < 0) {
			dev_err(dev->dev, "SB state read failed: %d\n", ret);
		} else if (ret != (int)exp_val) {
			*pret = (u16)ret;
			return true;
		}

		msleep(10);
		timeout -= 10;
	}

	return false;
}

static int fw_upgrade(const mxl862xx_device_t *dev, const u8 *data, size_t size)
{
	int ret;
	u32 dword;
	const u32 *pdword;
	size_t hdr_sz, img_sz, i;
	u32 page;
	u16 stat_val;

	if (!data)
		return -EINVAL;

	if (size < 20)
		return -EINVAL;

	/* check image type */
	pdword = (const u32 *)data;
	dword = __le32_to_cpu(pdword[0]);
	if ((dword & IMG_TYPE_MASK) != IMG_TYPE_V1_FULL) {
		dev_err(dev->dev, "Invalid image type");
		return -EINVAL;
	}

	/* calculate header size */
	if ((dword & BIT(2)) == 0) {
		hdr_sz = 20;
	} else {
		hdr_sz = 28;
	}
	if (size < hdr_sz)
		return -EINVAL;

	/* calculate image size */
	img_sz = 0;
	for (i = 4; i < hdr_sz; i += 8) {
		u16 idx = (i >> 2);
		u32 val = __le32_to_cpu(pdword[idx]);

		dev_dbg(dev->dev, "size[%u] = %u\n", (u32)idx >> 1, val);

		img_sz +=  val;
	}
	dev_info(dev->dev, "Header size %lu bytes, image size %lu bytes\n", hdr_sz, img_sz);
	if (size < hdr_sz + img_sz) {
		dev_err(dev->dev, "Data size (%lu) is smaller than expected binary file size (%lu)\n",
			size, hdr_sz + img_sz);
		return -EINVAL;
	}
	if (img_sz % TX_PAGE_SIZE == FW_DL_MDIO_TFX_FAIL) {
		dev_err(dev->dev, "Image size (%lu) without header should not be multiple of page size %d + %d\n",
			img_sz, TX_PAGE_SIZE, FW_DL_MDIO_TFX_FAIL);
		return -EINVAL;
	}

	/* trigger device to enter firmware upgrade mode
	 * ignore return value because it's not reliable
	 */
	ret = mxl862xx_api_wrap(dev, SYS_MISC_FW_UPDATE, NULL, 0, 0, 0);
	if (ret && ret != MMD_API_MDIO_BUS_ERR) {
		dev_warn(dev->dev, "SYS_MISC_FW_UPDATE returns %d", ret);
	}

	msleep(1000);

	sb_pdi_reset(dev);

	/* wait for target to be ready for firmware upgrade */
	if (wait_until(dev, FW_DL_MDIO_RDY_MAGIC, 3000)) {
		dev_info(dev->dev, "Ready for firmware upgrade\n");
	} else {
		/* no backward compatible support in this driver */
		dev_err(dev->dev, "Entering upgrade mode timeout\n");
		return -ETIMEDOUT;
	}

	/* send START signal to target which is in firmware upgrade mode */
	smdio_write(dev, SB_PDI_STAT, FW_DL_MDIO_START_MAGIC);

	if (wait_until(dev, FW_DL_MDIO_START_ACK, 1000)) {
		dev_info(dev->dev, "Starting firmware upgrade\n");
	} else {
		dev_err(dev->dev, "Starting firmware upgrade timeout\n");
		return -ETIMEDOUT;
	}

	/* send image header */
	smdio_write(dev, SB_PDI_CTRL, SB_PDI_CTRL_WR);
	smdio_write_byte_cont(dev, SB_PDI_DATA, data, hdr_sz);
	sb_pdi_reset(dev);
	smdio_write(dev, SB_PDI_STAT, hdr_sz);

	if (wait_until(dev, hdr_sz + 1, 1000)) {
		dev_info(dev->dev, "Image header is transmitted\n");
	} else {
		dev_err(dev->dev, "Image header transmission timeout\n");
		return -ETIMEDOUT;
	}

	/* move data point to image data */
	data += hdr_sz;

	/* wait for target to be ready for data transmission */
	if (!wait_until(dev, 0, 70 * 1000)) {
		dev_err(dev->dev, "Preparation for data transmission timeout\n");
		return -ETIMEDOUT;
	}

	dev_info(dev->dev, "Start data transmission\n");

	i = 0;
	page = 0;
	stat_val = ~0;
	while (i < img_sz) {
		u16 to_write = min((size_t)TX_PAGE_SIZE, img_sz - i);

		/* transmit one page (32768) or rest of image */
		smdio_write(dev, SB_PDI_CTRL, SB_PDI_CTRL_WR);
		smdio_write_byte_cont(dev, SB_PDI_DATA, data, to_write);
		sb_pdi_reset(dev);
		dev_dbg(dev->dev, "Page %u is transmitted\n", page);

		/* notify one page is transmitted */
		smdio_write(dev, SB_PDI_STAT, to_write);

		/* wait for response from target */
		stat_val = to_write;
		if (!wait_until_not(dev, to_write, &stat_val, 10 * 1000)) {
			dev_err(dev->dev, "Page handling timeout\n");
			return -ETIMEDOUT;
		}
		dev_dbg(dev->dev, "Page %u is responded: %u\n",
			 page, (u32)stat_val);

		data += to_write;
		i += to_write;
		page++;
	}

	smdio_write(dev, SB_PDI_STAT, FW_DL_MDIO_FINISH_MAGIC);

	if (stat_val == 0) {
		dev_info(dev->dev, "Upgrade completed successfully\n");
		return 0;
	} else {
		dev_err(dev->dev, "Upgrade failed with code %u from target\n",
			(u32)stat_val);
		return -EIO;
	}
}

static int mxl862xx_request_firmware(const struct firmware **ppfw,
				     const char *path, struct device *dev)
{
#if IS_ENABLED( FFU_FILEPATH_OPEN)
	int ret = 0;
	void *buf = NULL;
	struct firmware *pfw;

	__module_get(THIS_MODULE);

	pfw = kzalloc(sizeof(*pfw), GFP_KERNEL);
	if (!pfw) {
		ret = -ENOMEM;
		goto EXIT;
	}

	ret = kernel_read_file_from_path(path, 0, &buf, INT_MAX, NULL,
					 READING_FIRMWARE);
	if (ret < 0) {
		kfree(pfw);
		goto EXIT;
	}

	pfw->size = ret;
	pfw->data = buf;

	*ppfw = pfw;
	ret = 0;

EXIT:
	module_put(THIS_MODULE);
	return ret;
#else
	return request_firmware_direct(pfw, name, dev);
#endif
}

static void mxl862xx_release_firmware(const struct firmware *pfw)
{
#if IS_ENABLED(FFU_FILEPATH_OPEN)
	vfree(pfw->data);
	kfree(pfw);
#else
	release_firmware(fw);
#endif
}

int mxl862xx_fw_upgrade(const mxl862xx_device_t *dev, const char *filename)
{
	int ret;
	const struct firmware *fw;
#if IS_ENABLED(FFU_TURN_OFF_NETDEV)
	struct dsa_switch *ds;
	bool port_state[17] = {0};
	size_t i;
#endif

	if (!dev || !filename)
		return -EINVAL;

	ret = mxl862xx_request_firmware(&fw, filename, dev->dev);
	if (ret) {
		dev_err(dev->dev, "Failed to load firmware %s: %d\n",
			filename, ret);
		return ret;
	}

#if IS_ENABLED(FFU_TURN_OFF_NETDEV)
	ds = dev_get_drvdata(dev->dev);
	for (i = 0; i < ds->num_ports && i < ARRAY_SIZE(port_state); i++) {
		const struct dsa_port *dp = dsa_to_port(ds, i);

		if (!dp || !dp->slave)
			continue;

		/* Skip CPU Port */
		if (dp->type == DSA_PORT_TYPE_CPU)
			continue;

		port_state[i] = netif_running(dp->slave);

		rtnl_lock();
		dev_close(dp->slave);
		rtnl_unlock();
	}
#endif

	ret = fw_upgrade(dev, fw->data, fw->size);

	mxl862xx_release_firmware(fw);

	if (ret != 0)
		return ret;

#if IS_ENABLED(FFU_ON_NETDEV_AGAIN)
	for (i = 0; i < ds->num_ports && i < ARRAY_SIZE(port_state); i++) {
		const struct dsa_port *dp = dsa_to_port(ds, i);
		int rtn;

		if (!dp || !dp->slave || !port_state[i])
			continue;

		/* Skip CPU Port */
		if (dp->type == DSA_PORT_TYPE_CPU)
			continue;

		rtnl_lock();
		rtn = dev_open(dp->slave, NULL);
		rtnl_unlock();
		dev_warn(dev->dev,
			 "%s:%u: dev_open returns %d\n",
			 __func__, __LINE__, rtn);
	}
#endif

	return 0;
}

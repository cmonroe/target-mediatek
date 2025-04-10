#ifndef __MXL862XX_IOCTL_H__
#define __MXL862XX_IOCTL_H__

#define MXL862xx_IOCTL_TYPE	'X'
#define MXL862xx_IOCTL_CMD_WRAP	_IOWR(MXL862xx_IOCTL_TYPE, 1, struct mxl862xx_cmd_wrap)
#define MXL862xx_IOCTL_MDIO_RD	_IOWR(MXL862xx_IOCTL_TYPE, 2, struct mxl862xx_mdio_wrap)
#define MXL862xx_IOCTL_MDIO_WR	_IOWR(MXL862xx_IOCTL_TYPE, 3, struct mxl862xx_mdio_wrap)
#define MXL862xx_IOCTL_FFU	_IOWR(MXL862xx_IOCTL_TYPE, 4, struct mxl862xx_ffu_wrap)

struct mxl862xx_cmd_wrap {
	uint16_t cmd;
	uint16_t size;
	uint16_t cmd_r;
	uint16_t size_r;
	int ret;
	uint8_t data[];
};

struct mxl862xx_mdio_wrap {
	uint8_t phyaddr;
	uint8_t mmd;
	uint16_t regnum;
	uint16_t val;
	int ret;
};

struct mxl862xx_ffu_wrap {
	char filename[256];
};

#endif

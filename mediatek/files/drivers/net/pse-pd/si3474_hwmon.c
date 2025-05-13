// SPDX-License-Identifier: GPL-2.0-only
/*
 * HWMON Driver for the Skyworks Si3474 PoE PSE Controller
 *
 */
#if IS_REACHABLE(CONFIG_HWMON)

#include <linux/hwmon.h>
#include <linux/i2c.h>

#include "si3474.h"

/* Misc registers */
#define TEMPERATURE_REG 0x2C
#define TEMP_SCALE_MILLIC 652 // 0.652 * 1000

static umode_t si3474_hwmon_is_visible(const void *drvdata,
				       enum hwmon_sensor_types type, u32 attr,
				       int channel)
{
	return 0444;
}

static int si3474_hwmon_read(struct device *dev, enum hwmon_sensor_types type,
			     u32 attr, int channel, long *value)
{
	struct si3474_priv *priv = dev_get_drvdata(dev);
	int ret;

	ret = i2c_smbus_read_byte_data(priv->client[0], TEMPERATURE_REG);
	if (ret < 0)
		return ret;

	*value = (((s8)ret * TEMP_SCALE_MILLIC)) - 20000;
	return 0;
};

static const struct hwmon_channel_info *const si3474_hwmon_info[] = {
	HWMON_CHANNEL_INFO(temp, HWMON_T_INPUT), NULL
};

static const struct hwmon_ops si3474_hwmon_ops = {
	.is_visible = si3474_hwmon_is_visible,
	.read = si3474_hwmon_read,
};

static const struct hwmon_chip_info si3474_hwmon_chip_info = {
	.ops = &si3474_hwmon_ops,
	.info = si3474_hwmon_info,
};

int si3474_hwmon_probe(struct device *dev, struct si3474_priv *priv)
{
	struct device *hwmon_dev;

	hwmon_dev = devm_hwmon_device_register_with_info(
		dev, "si3474", priv, &si3474_hwmon_chip_info, NULL);

	return PTR_ERR_OR_ZERO(hwmon_dev);
}

#endif

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the Skyworks Si3474 PoE PSE Controller
 */

#ifndef SI3474_H
#define SI3474_H

#include <linux/device.h>
#include <linux/of.h>
#include <linux/pse-pd/pse.h>

#define SI3474_MAX_CHANS 8

struct si3474_pi_desc {
	u8 chan[2];
	bool is_4p;
};

struct si3474_priv {
	struct i2c_client *client[2];
	struct pse_controller_dev pcdev;
	struct device_node *np;
	struct si3474_pi_desc pi[SI3474_MAX_CHANS];
};

#if IS_REACHABLE(CONFIG_HWMON)
int si3474_hwmon_probe(struct device *dev, struct si3474_priv *priv);
#else
static inline int si3474_hwmon_probe(struct device *dev,
				     struct si3474_priv *priv)
{
	return 0;
};
#endif

#endif /* SI3474_H */

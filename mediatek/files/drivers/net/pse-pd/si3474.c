// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the Skyworks Si3474 PoE PSE Controller
 *
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pse-pd/pse.h>

#define SI3474_MAX_CHANS 8

#define MANUFACTURER_ID 0x08
#define IC_ID 0x05
#define SI3474_DEVICE_ID (MANUFACTURER_ID << 3 | IC_ID)

/* Misc registers */
#define VENDOR_IC_ID_REG 0x1B
#define TEMPERATURE_REG 0x2C
#define FIRMWARE_REVISION_REG 0x41
#define CHIP_REVISION_REG 0x43

/* Main status registers */
#define POWER_STATUS_REG 0x10
#define PB_POWER_ENABLE 0x19

struct si3474_port_desc {
	u8 chan[2];
	bool is_4p;
};

struct si3474_priv {
	struct i2c_client *client;
	struct pse_controller_dev pcdev;
	struct device_node *np;
	struct si3474_port_desc port[SI3474_MAX_CHANS];
};

static struct si3474_priv *to_si3474_priv(struct pse_controller_dev *pcdev)
{
	return container_of(pcdev, struct si3474_priv, pcdev);
}

static int si3474_ethtool_get_status(struct pse_controller_dev *pcdev,
				     unsigned long id,
				     struct netlink_ext_ack *extack,
				     struct pse_control_status *status)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client = priv->client;
	bool enabled = FALSE;
	bool delivering = FALSE;
	uint8_t chan0, chan1;
	int32_t ret;

	ret = i2c_smbus_read_byte_data(client, POWER_STATUS_REG);
	if (ret < 0) {
		status->c33_pw_status = ETHTOOL_C33_PSE_PW_D_STATUS_UNKNOWN;
		status->c33_admin_state = ETHTOOL_C33_PSE_ADMIN_STATE_UNKNOWN;
		return ret;
	}

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 < 4 && chan1 < 4) {
		enabled = (ret & (BIT(chan0) | BIT(chan1))) != 0;
		delivering = (ret & (BIT(chan0 + 4) | BIT(chan1 + 4))) != 0;
	}

	if (delivering)
		status->c33_pw_status = ETHTOOL_C33_PSE_PW_D_STATUS_DELIVERING;
	else
		status->c33_pw_status = ETHTOOL_C33_PSE_PW_D_STATUS_DISABLED;

	if (enabled)
		status->c33_admin_state = ETHTOOL_C33_PSE_ADMIN_STATE_ENABLED;
	else
		status->c33_admin_state = ETHTOOL_C33_PSE_ADMIN_STATE_DISABLED;

	return 0;
}

/* Parse pse-pis subnode into chan array of si3474_priv */
static int si3474_get_of_channels(struct si3474_priv *priv)
{
	struct device_node *pse_node, *node;
	struct pse_pi *pi;
	uint32_t port_no, chan_id;
	int ret = 0;

	pse_node = of_get_child_by_name(priv->np, "pse-pis");
	if (!pse_node) {
		dev_warn(&priv->client->dev,
			 "Unable to parse DT PSE port-matrix, no pse-pis node\n");
		return -EINVAL;
	}

	for_each_child_of_node(pse_node, node) {
		if (!of_node_name_eq(node, "pse-pi"))
			continue;

		ret = of_property_read_u32(node, "reg", &port_no);
		if (ret) {
			dev_err(&priv->client->dev,
				"Failed to read pse-pi reg property\n");
			ret = -EINVAL;
			goto out;
		}
		if (port_no >= SI3474_MAX_CHANS) {
			dev_err(&priv->client->dev, "Invalid port number %u\n",
				port_no);
			ret = -EINVAL;
			goto out;
		}

		pi = &priv->pcdev.pi[port_no];
		if (!pi->pairset[0].np) {
			dev_err(&priv->client->dev,
				"pairset[0] np is NULL, port %u\n",
				port_no);
			ret = -EINVAL;
			goto out;
		}

		ret = of_property_read_u32(pi->pairset[0].np, "reg", &chan_id);
		if (ret) {
			dev_err(
			    &priv->client->dev,
			    "Failed to read channel reg property, ret:%d \n",
			    ret);
			ret = -EINVAL;
			goto out;
		}
		priv->port[port_no].chan[0] = chan_id;

		// FIXME: Prepared for tuple pairsets only
		if (!pi->pairset[1].np) {
			dev_err(&priv->client->dev,
				"pairset[1] np is NULL, port %u\n",
				port_no);
			ret = -EINVAL;
			goto out;
		}

		ret = of_property_read_u32(pi->pairset[1].np, "reg", &chan_id);
		if (ret) {
			dev_err(&priv->client->dev,
				"Failed to read channel reg property\n");
			ret = -EINVAL;
			goto out;
		}
		priv->port[port_no].chan[1] = chan_id;
		priv->port[port_no].is_4p = TRUE;
	}

out:
	of_node_put(pse_node);
	of_node_put(node);
	return ret;
}

static int si3474_setup_pi_matrix(struct pse_controller_dev *pcdev)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	int ret;

	ret = si3474_get_of_channels(priv);
	if (ret < 0) {
		dev_warn(&priv->client->dev,
			 "Unable to parse DT PSE port-matrix\n");
	}
	return ret;
}

static int si3474_pi_enable(struct pse_controller_dev *pcdev, int id)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client = priv->client;
	uint8_t chan0, chan1;
	uint16_t val = 0;
	int32_t ret;

	if (id >= SI3474_MAX_CHANS)
		return -ERANGE;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 >= 4 || chan1 >= 4)
		return -ERANGE;

	val = (BIT(chan0) | BIT(chan1));
	ret = i2c_smbus_write_word_data(client, PB_POWER_ENABLE, val);

	if (ret)
		return ret;

	return 0;
}

static int si3474_pi_disable(struct pse_controller_dev *pcdev, int id)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client = priv->client;
	uint8_t chan0, chan1;
	uint16_t val = 0;
	int32_t ret;

	if (id >= SI3474_MAX_CHANS)
		return -ERANGE;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 >= 4 || chan1 >= 4)
		return -ERANGE;

	val = (BIT(chan0 + 4) | BIT(chan1 + 4));
	ret = i2c_smbus_write_word_data(client, PB_POWER_ENABLE, val);

	if (ret)
		return ret;

	return 0;
}

static int si3474_pi_is_enabled(struct pse_controller_dev *pcdev, int id)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client = priv->client;
	bool enabled = FALSE;
	uint8_t chan0, chan1;
	int32_t ret;

	ret = i2c_smbus_read_byte_data(client, POWER_STATUS_REG);
	if (ret < 0)
		return ret;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 < 4 && chan1 < 4) {
		enabled = (ret & (BIT(chan0) | BIT(chan1))) != 0;
	}

	return enabled;
}

static const struct pse_controller_ops si3474_ops = {
    .setup_pi_matrix = si3474_setup_pi_matrix,
    .pi_enable = si3474_pi_enable,
    .pi_disable = si3474_pi_disable,
    .pi_is_enabled = si3474_pi_is_enabled,
    .ethtool_get_status = si3474_ethtool_get_status,
};

static int si3474_i2c_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct si3474_priv *priv;
	int ret;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(dev, "i2c check functionality failed\n");
		return -ENXIO;
	}

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	ret = i2c_smbus_read_byte_data(client, VENDOR_IC_ID_REG);
	if (ret < 0)
		return ret;

	if (ret != SI3474_DEVICE_ID) {
		dev_err(dev, "Wrong device ID: 0x%x\n", ret);
		return -ENXIO;
	}

	ret = i2c_smbus_read_byte_data(client, FIRMWARE_REVISION_REG);
	if (ret < 0)
		return ret;

	dev_info(&client->dev, "Firmware revision: 0x%x\n", ret);

	ret = i2c_smbus_read_byte_data(client, CHIP_REVISION_REG);
	if (ret < 0)
		return ret;

	dev_info(&client->dev, "Chip revision: 0x%x\n", ret);

	priv->client = client;
	i2c_set_clientdata(client, priv);
	priv->np = dev->of_node;

	priv->pcdev.owner = THIS_MODULE;
	priv->pcdev.ops = &si3474_ops;
	priv->pcdev.dev = dev;
	priv->pcdev.types = ETHTOOL_PSE_C33;
	priv->pcdev.nr_lines = SI3474_MAX_CHANS;
	ret = devm_pse_controller_register(dev, &priv->pcdev);
	if (ret) {
		return dev_err_probe(dev, ret,
				     "Failed to register PSE controller\n");
	}

	return ret;
}

static const struct i2c_device_id si3474_id[] = {{"si3474"}, {}};
MODULE_DEVICE_TABLE(i2c, si3474_id);

static const struct of_device_id si3474_of_match[] = {
    {
	.compatible = "skyworks,si3474",
    },
    {},
};
MODULE_DEVICE_TABLE(of, si3474_of_match);

static struct i2c_driver si3474_driver = {
    .probe = si3474_i2c_probe,
    .id_table = si3474_id,
    .driver =
	{
	    .name = "si3474",
	    .of_match_table = si3474_of_match,
	},
};
module_i2c_driver(si3474_driver);

MODULE_AUTHOR("Piotr Kubik <piotr.kubik@adtran.com>");
MODULE_DESCRIPTION("Skyworks Si3474 PoE PSE Controller driver");
MODULE_LICENSE("GPL");

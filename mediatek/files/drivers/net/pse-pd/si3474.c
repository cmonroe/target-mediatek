// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the Skyworks Si3474 PoE PSE Controller
 *
 */


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
#define PORT_MODE_REG 0x12
#define PB_POWER_ENABLE_REG 0x19

/* PORTn Current */
#define PORT1_CURRENT_LSB_REG 0x30

/* PORTn Current [mA], return in [nA] */
/* 1000 * ((PORTn_CURRENT_MSB << 8) + PORTn_CURRENT_LSB) / 16384 */
#define SI3474_NA_STEP (1000 * 1000 * 1000 / 16384)

/* VPWR Voltage */
#define VPWR_LSB_REG 0x2E
#define VPWR_MSB_REG 0x2F

/* PORTn Voltage */
#define PORT1_VOLTAGE_LSB_REG 0x32

/* VPWR Voltage [V], return in [uV] */
/* 60 * (( VPWR_MSB << 8) + VPWR_LSB) / 16384 */
#define SI3474_UV_STEP (1000 * 1000 * 60 / 16384)

struct si3474_port_desc {
	u8 chan[2];
	bool is_4p;
};

struct si3474_priv {
	struct i2c_client *client[2];
	struct pse_controller_dev pcdev;
	struct device_node *np;
	struct si3474_port_desc port[SI3474_MAX_CHANS];
};

static struct si3474_priv *to_si3474_priv(struct pse_controller_dev *pcdev)
{
	return container_of(pcdev, struct si3474_priv, pcdev);
}

static int si3474_pi_get_admin_state(struct pse_controller_dev *pcdev, int id,
				     struct pse_admin_state *admin_state)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client;
	bool is_enabled = false;
	u8 chan0, chan1;
	s32 ret;

	if (id >= SI3474_MAX_CHANS)
		return -ERANGE;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 < 4)
		client = priv->client[0];
	else
		client = priv->client[1];

	ret = i2c_smbus_read_byte_data(client, PORT_MODE_REG);
	if (ret < 0) {
		admin_state->c33_admin_state =
		    ETHTOOL_C33_PSE_ADMIN_STATE_UNKNOWN;
		return ret;
	}


	is_enabled = ((ret & (0x03 << (2 * (chan0 % 4)))) |
		      (ret & (0x03 << (2 * (chan1 % 4))))) != 0;
	
	if (is_enabled)
		admin_state->c33_admin_state =
		    ETHTOOL_C33_PSE_ADMIN_STATE_ENABLED;
	else
		admin_state->c33_admin_state =
		    ETHTOOL_C33_PSE_ADMIN_STATE_DISABLED;

	return 0;
}

static int si3474_pi_get_pw_status(struct pse_controller_dev *pcdev, int id,
				   struct pse_pw_status *pw_status)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client;
	bool delivering = false;
	u8 chan0, chan1;
	s32 ret;

	if (id >= SI3474_MAX_CHANS)
		return -ERANGE;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 < 4)
		client = priv->client[0];
	else
		client = priv->client[1];

	ret = i2c_smbus_read_byte_data(client, POWER_STATUS_REG);
	if (ret < 0) {
		pw_status->c33_pw_status = ETHTOOL_C33_PSE_PW_D_STATUS_UNKNOWN;
		return ret;
	}

	delivering = (ret & (BIT((chan0 % 4) + 4) | BIT((chan1 % 4) + 4))) != 0;

	if (delivering)
		pw_status->c33_pw_status =
		    ETHTOOL_C33_PSE_PW_D_STATUS_DELIVERING;
	else
		pw_status->c33_pw_status = ETHTOOL_C33_PSE_PW_D_STATUS_DISABLED;

	return 0;
}

/* Parse pse-pis subnode into chan array of si3474_priv */
static int si3474_get_of_channels(struct si3474_priv *priv)
{
	struct device_node *pse_node, *node;
	struct pse_pi *pi;
	u32 port_no, chan_id;
	s8 pairset_cnt;
	s32 ret = 0;

	pse_node = of_get_child_by_name(priv->np, "pse-pis");
	if (!pse_node) {
		dev_warn(&priv->client[0]->dev,
			 "Unable to parse DT PSE port-matrix, no pse-pis node\n");
		return -EINVAL;
	}

	for_each_child_of_node(pse_node, node) {
		if (!of_node_name_eq(node, "pse-pi"))
			continue;

		ret = of_property_read_u32(node, "reg", &port_no);
		if (ret) {
			dev_err(&priv->client[0]->dev,
				"Failed to read pse-pi reg property\n");
			ret = -EINVAL;
			goto out;
		}
		if (port_no >= SI3474_MAX_CHANS) {
			dev_err(&priv->client[0]->dev, "Invalid port number %u\n",
				port_no);
			ret = -EINVAL;
			goto out;
		}

		pairset_cnt = of_property_count_elems_of_size(node, "pairsets",
							      sizeof(u32));
		if (!pairset_cnt) {
			dev_err(&priv->client[0]->dev,
				"Failed to get pairsets property\n");
			ret = -EINVAL;
			goto out;
		}

		pi = &priv->pcdev.pi[port_no];
		if (!pi->pairset[0].np) {
			dev_err(&priv->client[0]->dev,
				"Missing pairset reference, port: %u\n",
				port_no);
			ret = -EINVAL;
			goto out;
		}

		ret = of_property_read_u32(pi->pairset[0].np, "reg", &chan_id);
		if (ret) {
			dev_err(&priv->client[0]->dev,
				"Failed to read channel reg property, ret:%d\n",
				ret);
			ret = -EINVAL;
			goto out;
		}
		priv->port[port_no].chan[0] = chan_id;
		priv->port[port_no].is_4p = FALSE;

		if (pairset_cnt == 2) {
			if (!pi->pairset[1].np) {
				dev_err(&priv->client[0]->dev,
					"Missing pairset reference, port: %u\n",
					port_no);
				ret = -EINVAL;
				goto out;
			}

			ret = of_property_read_u32(pi->pairset[1].np, "reg",
						   &chan_id);
			if (ret) {
				dev_err(&priv->client[0]->dev,
					"Failed to read channel reg property\n");
				ret = -EINVAL;
				goto out;
			}
			priv->port[port_no].chan[1] = chan_id;
			priv->port[port_no].is_4p = TRUE;
		} else {
			dev_err(&priv->client[0]->dev,
				"Number of pairsets incorrect - only 4p configurations supported\n");
			ret = -EINVAL;
			goto out;
		}
	}

out:
	of_node_put(pse_node);
	of_node_put(node);
	return ret;
}

static int si3474_setup_pi_matrix(struct pse_controller_dev *pcdev)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	s32 ret;

	ret = si3474_get_of_channels(priv);
	if (ret < 0) {
		dev_warn(&priv->client[0]->dev,
			 "Unable to parse DT PSE port-matrix\n");
	}
	return ret;
}

static int si3474_pi_enable(struct pse_controller_dev *pcdev, int id)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client;
	u8 chan0, chan1;
	u8 val = 0;
	s32 ret;

	if (id >= SI3474_MAX_CHANS)
		return -ERANGE;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 < 4)
		client = priv->client[0];
	else
		client = priv->client[1];

	/* Release port from shutdown */
	ret = i2c_smbus_read_byte_data(client, PORT_MODE_REG);
	if (ret < 0)
		return ret;

	val = (u8)ret;
	val |= (0x03 << (2 * (chan0 % 4)));
	val |= (0x03 << (2 * (chan1 % 4)));

	ret = i2c_smbus_write_byte_data(client, PORT_MODE_REG, val);
	if (ret)
		return ret;

	/* Give time for transition to complete */
	ssleep(1);

	/* Trigger port to power up */
	val = (BIT(chan0 % 4) | BIT(chan1 % 4));
	ret = i2c_smbus_write_byte_data(client, PB_POWER_ENABLE_REG, val);

	return 0;
}

static int si3474_pi_disable(struct pse_controller_dev *pcdev, int id)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client;
	u8 chan0, chan1;
	u8 val = 0;
	s32 ret;

	if (id >= SI3474_MAX_CHANS)
		return -ERANGE;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 < 4)
		client = priv->client[0];
	else
		client = priv->client[1];

	/* Trigger port to power down */
	val = (BIT((chan0 % 4) + 4) | BIT((chan1 % 4) + 4));
	ret = i2c_smbus_write_byte_data(client, PB_POWER_ENABLE_REG, val);

	/* Shutdown port */
	ret = i2c_smbus_read_byte_data(client, PORT_MODE_REG);
	if (ret < 0)
		return ret;

	val = (u8)ret;
	val &= ~(0x03 << (2 * (chan0 % 4)));
	val &= ~(0x03 << (2 * (chan1 % 4)));

	ret = i2c_smbus_write_byte_data(client, PORT_MODE_REG, val);
	if (ret)
		return ret;

	return 0;
}

static int si3474_pi_is_enabled(struct pse_controller_dev *pcdev, int id)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client;
	bool enabled = false;
	u8 chan0, chan1;
	s32 ret;

	if (id >= SI3474_MAX_CHANS)
		return -ERANGE;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 < 4)
		client = priv->client[0];
	else
		client = priv->client[1];

	ret = i2c_smbus_read_byte_data(client, POWER_STATUS_REG);
	if (ret < 0)
		return ret;

	enabled = (ret & (BIT(chan0 % 4) | BIT(chan1 % 4))) != 0;

	return enabled;
}

static int si3474_pi_get_chan_current(struct si3474_priv *priv, u8 chan)
{
	struct i2c_client *client;
	s32 ret;
	u8 reg;
	u64 tmp_64;

	if (chan < 4)
		client = priv->client[0];
	else
		client = priv->client[1];

	/* Registers 0x30 to 0x3d */
	reg = PORT1_CURRENT_LSB_REG + (chan % 4) * 4;

	ret = i2c_smbus_read_word_data(client, reg);
	if (ret < 0)
		return ret;

	tmp_64 = ret * SI3474_NA_STEP;

	/* uA = nA / 1000 */
	tmp_64 = DIV_ROUND_CLOSEST_ULL(tmp_64, 1000);
	return (int)tmp_64;
}

static int si3474_pi_get_chan_voltage(struct si3474_priv *priv, u8 chan)
{
	struct i2c_client *client;
	s32 ret;
	u8 reg;
	u32 val;

	if (chan < 4)
		client = priv->client[0];
	else
		client = priv->client[1];

	/* Registers 0x32 to 0x3f */
	reg = PORT1_VOLTAGE_LSB_REG + (chan % 4) * 4;

	ret = i2c_smbus_read_word_data(client, reg);
	if (ret < 0)
		return ret;

	val = ret * SI3474_UV_STEP;

	return (int)val;
}

static int si3474_pi_get_voltage(struct pse_controller_dev *pcdev, int id)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	struct i2c_client *client;
	u8 chan0, chan1;
	s32 ret;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	if (chan0 < 4)
		client = priv->client[0];
	else
		client = priv->client[1];

	/* Check which channels are enabled*/
	ret = i2c_smbus_read_byte_data(client, POWER_STATUS_REG);
	if (ret < 0)
		return ret;

	/* Take voltage from the first enabled channel */
	if (ret & BIT(chan0 % 4))
		ret = si3474_pi_get_chan_voltage(priv, chan0);
	else if (ret & BIT(chan1))
		ret = si3474_pi_get_chan_voltage(priv, chan1);
	else
		/* 'should' be no voltage in this case */
		return 0;

	return ret;
}

static int si3474_pi_get_actual_pw(struct pse_controller_dev *pcdev, int id)
{
	struct si3474_priv *priv = to_si3474_priv(pcdev);
	s32 ret;
	u32 uV, uA;
	u64 tmp_64;
	u8 chan0, chan1;

	if (id >= SI3474_MAX_CHANS)
		return -ERANGE;

	ret = si3474_pi_get_voltage(&priv->pcdev, id);
	if (ret < 0)
		return ret;
	uV = ret;

	chan0 = priv->port[id].chan[0];
	chan1 = priv->port[id].chan[1];

	ret = si3474_pi_get_chan_current(priv, chan0);
	if (ret < 0)
		return ret;
	uA = ret;

	ret = si3474_pi_get_chan_current(priv, chan1);
	if (ret < 0)
		return ret;
	uA += ret;

	tmp_64 = uV;
	tmp_64 *= uA;
	/* mW = uV * uA / 1000000000 */
	return DIV_ROUND_CLOSEST_ULL(tmp_64, 1000000000);
}

static const struct pse_controller_ops si3474_ops = {
	.setup_pi_matrix = si3474_setup_pi_matrix,
	.pi_enable = si3474_pi_enable,
	.pi_disable = si3474_pi_disable,
	.pi_get_actual_pw = si3474_pi_get_actual_pw,
	.pi_get_voltage = si3474_pi_get_voltage,
	.pi_is_enabled = si3474_pi_is_enabled,
	.pi_get_admin_state = si3474_pi_get_admin_state,
	.pi_get_pw_status = si3474_pi_get_pw_status,
};

static int si3474_i2c_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct si3474_priv *priv;
	s32 ret;
	u8 fw_version;

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
	fw_version = ret;

	ret = i2c_smbus_read_byte_data(client, CHIP_REVISION_REG);
	if (ret < 0)
		return ret;

	dev_info(&client->dev, "Chip revision: 0x%x, firmware version: 0x%x\n",
		 ret, fw_version);

	priv->client[0] = client;
	i2c_set_clientdata(client, priv);

	priv->client[1] = i2c_new_ancillary_device(priv->client[0], "slave",
						priv->client[0]->addr+1);
	if (IS_ERR(priv->client[1]))
		return PTR_ERR(priv->client[1]);

	ret = i2c_smbus_read_byte_data(priv->client[1], VENDOR_IC_ID_REG);
	if (ret < 0) {
		dev_err(&priv->client[1]->dev, "Cannot access slave PSE controller\n");
		goto out_err_slave;
	}

	if (ret != SI3474_DEVICE_ID) {
		dev_err(&priv->client[1]->dev, "Wrong device ID for slave PSE controller: 0x%x\n", ret);
		ret = -ENXIO;
		goto out_err_slave;
	}

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

out_err_slave:
	i2c_unregister_device(priv->client[1]);
	return ret;

}

static void si3474_i2c_remove(struct i2c_client *client)
{
	struct si3474_priv *priv = i2c_get_clientdata(client);
	i2c_unregister_device(priv->client[1]);
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
	.remove = si3474_i2c_remove,
	.id_table = si3474_id,
	.driver = {
		.name = "si3474",
		.of_match_table = si3474_of_match,
	},
};
module_i2c_driver(si3474_driver);

MODULE_AUTHOR("Piotr Kubik <piotr.kubik@adtran.com>");
MODULE_DESCRIPTION("Skyworks Si3474 PoE PSE Controller driver");
MODULE_LICENSE("GPL");

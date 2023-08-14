// SPDX-License-Identifier: GPL-2.0-only

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>

/**
 * Driver for SmartRG RGBW LED microcontroller.
 * RGBW LED is connected to a Holtek HT45F0062 that is on the I2C bus.
 *
 */

struct srg_led {
	struct mutex lock;
	struct i2c_client *client;
	struct led_classdev led_red;
	struct led_classdev led_green;
	struct led_classdev led_blue;
	struct led_classdev led_white;
	u8 index_red;
	u8 index_green;
	u8 index_blue;
	u8 index_white;
	u8 control[5];
};


static int
srg_led_i2c_write(struct srg_led *sysled, u8 reg, u8 value)
{
	return i2c_smbus_write_byte_data(sysled->client, reg, value);
}

/*
 * MC LED Command: 0 = OFF, 1 = ON, 2 = Flash, 3 = Pulse, 4 = Blink
 * */
static int
srg_led_control_sync(struct srg_led *sysled)
{
	int i, ret;

	for (i = 1; i < 5; i++) {
		if (sysled->control[i] > 1) {
			ret = srg_led_i2c_write(sysled, i, sysled->control[i]);
			if (i < 4) {
				msleep(1);
			}
		}
	}
	return ret;
}

/*
 * This function overrides the led driver timer trigger to offload
 * flashing to the micro-controller.  The negative effect of this
 * is the inability to configure the delay_on and delay_off periods.
 *
 * */
#define SRG_LED_RGBW_PULSE(color)		\
static int						\
srg_led_set_##color##_pulse(struct led_classdev *led_cdev,	\
				unsigned long *delay_on,	\
				unsigned long *delay_off)	\
{								\
	struct srg_led *sysled = container_of(led_cdev,		\
						struct srg_led,	\
						led_##color);	\
	int ret;						\
	mutex_lock(&sysled->lock);				\
	sysled->control[sysled->index_##color] = 3;		\
	ret = srg_led_control_sync(sysled);			\
	mutex_unlock(&sysled->lock);				\
	return ret;						\
}

SRG_LED_RGBW_PULSE(red);
SRG_LED_RGBW_PULSE(green);
SRG_LED_RGBW_PULSE(blue);
SRG_LED_RGBW_PULSE(white);

#define SRG_LED_CONTROL_RGBW(color)		\
static int						\
srg_led_set_##color##_brightness(struct led_classdev *led_cdev,	\
				enum led_brightness value)	\
{								\
	struct srg_led *sysled = container_of(led_cdev,		\
						struct srg_led,	\
						led_##color);	\
	int ret, index, control=value;				\
	mutex_lock(&sysled->lock);				\
	if (value == 255) {					\
		control = 1;					\
	}							\
	if (control > 4) {					\
		index = sysled->index_##color + 4;		\
		ret = srg_led_i2c_write(sysled, index, control); \
	} else {						\
		sysled->control[sysled->index_##color] = control; \
		ret = srg_led_i2c_write(sysled, sysled->index_##color, control); \
		msleep(1);					\
	}							\
	mutex_unlock(&sysled->lock);				\
	return ret;						\
}

SRG_LED_CONTROL_RGBW(red);
SRG_LED_CONTROL_RGBW(green);
SRG_LED_CONTROL_RGBW(blue);
SRG_LED_CONTROL_RGBW(white);


static u8
srg_led_init_led(struct device_node *np, struct srg_led *sysled,
				struct led_classdev *led_cdev)
{
	struct led_init_data init_data = {};
	int ret;
	int index;

	if (!np)
		return 0;

	init_data.fwnode = of_fwnode_handle(np);

	led_cdev->name = of_get_property(np, "label", NULL) ? : np->name;
	led_cdev->brightness = LED_OFF;
	led_cdev->max_brightness = LED_FULL;

	ret = devm_led_classdev_register_ext(&sysled->client->dev,
						led_cdev, &init_data);
	if (ret) {
		dev_err(&sysled->client->dev,
				"srg_led_init_led: led register %s error ret %d!n",
				led_cdev->name, ret);
		return 0;
	}

	ret = of_property_read_u32(np, "reg", &index);
        if (ret) {
                dev_err(&sysled->client->dev,
				"srg_led_init_led: no reg defined in np!\n");
                return 0;
        }

	return index;
}

static int
srg_led_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device_node *np = client->dev.of_node;
	struct srg_led *sysled;
	int ret = 0;

	sysled = devm_kzalloc(&client->dev, sizeof(*sysled), GFP_KERNEL);
	if (!sysled)
		return -ENOMEM;

	sysled->client = client;

	mutex_init(&sysled->lock);

	i2c_set_clientdata(client, sysled);

	sysled->led_red.brightness_set_blocking = srg_led_set_red_brightness;
	sysled->led_red.blink_set = srg_led_set_red_pulse;
	sysled->index_red = srg_led_init_led(of_get_child_by_name(np, "system_red"),
						sysled, &sysled->led_red);

	srg_led_set_red_brightness(&sysled->led_red, LED_OFF);
	msleep(5);

	sysled->led_green.brightness_set_blocking = srg_led_set_green_brightness;
	sysled->led_green.blink_set = srg_led_set_green_pulse;
	sysled->index_green = srg_led_init_led(of_get_child_by_name(np, "system_green"),
						sysled, &sysled->led_green);

	srg_led_set_green_brightness(&sysled->led_green, LED_OFF);
	msleep(5);

	sysled->led_blue.brightness_set_blocking = srg_led_set_blue_brightness;
	sysled->led_blue.blink_set = srg_led_set_blue_pulse;
	sysled->index_blue = srg_led_init_led(of_get_child_by_name(np, "system_blue"),
						sysled, &sysled->led_blue);

	srg_led_set_blue_brightness(&sysled->led_blue, LED_OFF);
	msleep(5);

	sysled->led_white.brightness_set_blocking = srg_led_set_white_brightness;
	sysled->led_white.blink_set = srg_led_set_white_pulse;
	sysled->index_white = srg_led_init_led(of_get_child_by_name(np, "system_white"),
						sysled, &sysled->led_white);

	srg_led_set_white_brightness(&sysled->led_white, LED_OFF);

	dev_err(&client->dev, "srg_led_probe done\n");
	return ret;
}

static void 
srg_led_remove(struct i2c_client *client)
{
	struct srg_led *sysled = i2c_get_clientdata(client);

	mutex_destroy(&sysled->lock);
}


static const struct i2c_device_id srg_led_id[] = {
	{ "srg-sysled", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, srg_led_id);

static const struct of_device_id of_srg_led_match[] = {
	{ .compatible = "srg,sysled", },
	{},
};
MODULE_DEVICE_TABLE(of, of_srg_led_match);

static struct i2c_driver srg_sysled_driver = {
	.driver = {
		.name	= "srg-sysled",
		.of_match_table = of_srg_led_match,
	},
	.probe		= srg_led_probe,
	.remove		= srg_led_remove,
	.id_table	= srg_led_id,
};
module_i2c_driver(srg_sysled_driver);


MODULE_DESCRIPTION("SmartRG system LED driver");
MODULE_AUTHOR("Shen Loh <shen.loh@adtran.com>");
MODULE_LICENSE("GPL v2");

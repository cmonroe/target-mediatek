/* Copyright (c) 2022-2023, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <sound/soc.h>
#include <linux/debugfs.h>
#include <sound/pcm_params.h>

#define STUB_RATES	SNDRV_PCM_RATE_8000_192000
#define STUB_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_U16_LE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_U24_LE | \
			SNDRV_PCM_FMTBIT_S32_LE | \
			SNDRV_PCM_FMTBIT_U32_LE)

struct dummy_chip {
	struct device *dev;
	struct snd_soc_component *component;
};

static int dummy_component_probe(struct snd_soc_component *component)
{
	dev_info(component->dev, "%s\n", __func__);
	return 0;
}

static void dummy_component_remove(struct snd_soc_component *component)
{
	struct dummy_chip *chip = snd_soc_component_get_drvdata(component);
	dev_info(component->dev, "%s\n", __func__);
	chip->component = NULL;
}

static const struct snd_soc_dapm_widget dummy_component_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("VINP"),
	SND_SOC_DAPM_OUTPUT("VOUTP"),
};

static const struct snd_soc_dapm_route dummy_component_dapm_routes[] = {
	{ "VOUTP", NULL, "aif_playback"},
	{ "aif_capture", NULL, "VINP"},
};

static const struct snd_soc_component_driver dummy_component_driver = {
	.probe = dummy_component_probe,
	.remove = dummy_component_remove,

	.dapm_widgets = dummy_component_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(dummy_component_dapm_widgets),
	.dapm_routes = dummy_component_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(dummy_component_dapm_routes),

	.idle_bias_on = false,
};

static int dummy_component_aif_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *hw_params, struct snd_soc_dai *dai)
{
	int word_len = params_physical_width(hw_params);
	int aud_bit = params_width(hw_params);

	dev_dbg(dai->dev, "format: 0x%08x\n", params_format(hw_params));
	dev_dbg(dai->dev, "rate: 0x%08x\n", params_rate(hw_params));
	dev_dbg(dai->dev, "word_len: %d, aud_bit: %d\n", word_len, aud_bit);
	if (word_len > 32 || word_len < 16) {
		dev_err(dai->dev, "not supported word length\n");
		return -ENOTSUPP;
	}

	dev_dbg(dai->dev, "%s: --\n", __func__);
	return 0;
}

static const struct snd_soc_dai_ops dummy_component_aif_ops = {
	.hw_params = dummy_component_aif_hw_params,
};

static struct snd_soc_dai_driver dummy_codec_dai = {
	.name = "slic-dummy-aif",
	.playback = {
		.stream_name	= "aif_playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= STUB_RATES,
		.formats	= STUB_FORMATS,
	},
	.capture = {
		.stream_name	= "aif_capture",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates = STUB_RATES,
		.formats = STUB_FORMATS,
	},
	/* dai properties */
	.symmetric_rates = 1,
	.symmetric_channels = 1,
	.symmetric_samplebits = 1,
	/* dai operations */
	.ops = &dummy_component_aif_ops,
};

static int slic_dummy_codec_probe(struct platform_device *pdev)
{
	return snd_soc_register_component(&pdev->dev, &dummy_component_driver,
				      &dummy_codec_dai, 1);
}

static int slic_dummy_codec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id slic_dummy_codec_dt_match[] = {
	{.compatible = "d2,slic-dummy-codec",},
	{}
};

static struct platform_driver slic_dummy_codec = {
	.driver = {
		   .name = "slic-dummy-codec",
		   .owner = THIS_MODULE,
		   .of_match_table = slic_dummy_codec_dt_match,
		   },
	.probe = slic_dummy_codec_probe,
	.remove = slic_dummy_codec_remove
};

module_platform_driver(slic_dummy_codec);

/* Module information */
MODULE_DESCRIPTION("slic dummy codec");
MODULE_LICENSE("GPL");

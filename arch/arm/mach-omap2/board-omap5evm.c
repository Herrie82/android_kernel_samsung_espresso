/*
 * Board support file for OMAP5430 based EVM.
 *
 * Copyright (C) 2010-2011 Texas Instruments
 * Author: Santosh Shilimkar <santosh.shilimkar@ti.com>
 *
 * Based on mach-omap2/board-4430sdp.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/hardware/gic.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <linux/gpio.h>
#include <linux/hwspinlock.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/memblock.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c/pca953x.h>
#include <linux/i2c/twl.h>
#include <linux/input/matrix_keypad.h>
#include <linux/platform_data/omap-abe-twl6040.h>
#include <linux/platform_data/omap4-keypad.h>
#include <linux/mfd/palmas.h>
#include <linux/mfd/twl6040.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>

#include <mach/hardware.h>

#include <plat/common.h>
#include <plat/i2c.h>
#include <plat/omap_hsi.h>
#include <plat/omap4-keypad.h>
#include <plat/mmc.h>
#include <plat/omap4-keypad.h>
#include <plat/omap_apps_brd_id.h>
#include <plat/drm.h>
#include <plat/remoteproc.h>
#include <plat/usb.h>

#include <video/omapdss.h>
#include <video/omap-panel-lg4591.h>

#include "board-54xx-sevm.h"
#include "common.h"
#include "common-board-devices.h"
#include "hsmmc.h"
#include "mux.h"
#include "omap5_ion.h"

/* USBB3 to SMSC LAN9730 */
#define GPIO_ETH_NRESET	172

/* USBB2 to SMSC 4640 HUB */
#define GPIO_HUB_NRESET	173

static const uint32_t evm5430_keymap[] = {
	KEY(0, 0, KEY_RESERVED),
	KEY(0, 1, KEY_RESERVED),
	KEY(0, 2, KEY_RESERVED),
	KEY(0, 3, KEY_RESERVED),
	KEY(0, 4, KEY_RESERVED),
	KEY(0, 5, KEY_RESERVED),
	KEY(0, 6, KEY_RESERVED),
	KEY(0, 7, KEY_RESERVED),

	KEY(1, 0, KEY_RESERVED),
	KEY(1, 1, KEY_RESERVED),
	KEY(1, 2, KEY_RESERVED),
	KEY(1, 3, KEY_RESERVED),
	KEY(1, 4, KEY_RESERVED),
	KEY(1, 5, KEY_RESERVED),
	KEY(1, 6, KEY_RESERVED),
	KEY(1, 7, KEY_RESERVED),

	KEY(2, 0, KEY_RESERVED),
	KEY(2, 1, KEY_RESERVED),
	KEY(2, 2, KEY_VOLUMEUP),
	KEY(2, 3, KEY_VOLUMEDOWN),
	KEY(2, 4, KEY_SEND),
	KEY(2, 5, KEY_HOME),
	KEY(2, 6, KEY_END),
	KEY(2, 7, KEY_SEARCH),

	KEY(3, 0, KEY_RESERVED),
	KEY(3, 1, KEY_RESERVED),
	KEY(3, 2, KEY_RESERVED),
	KEY(3, 3, KEY_RESERVED),
	KEY(3, 4, KEY_RESERVED),
	KEY(3, 5, KEY_RESERVED),
	KEY(3, 6, KEY_RESERVED),
	KEY(3, 7, KEY_RESERVED),

	KEY(4, 0, KEY_RESERVED),
	KEY(4, 1, KEY_RESERVED),
	KEY(4, 2, KEY_RESERVED),
	KEY(4, 3, KEY_RESERVED),
	KEY(4, 4, KEY_RESERVED),
	KEY(4, 5, KEY_RESERVED),
	KEY(4, 6, KEY_RESERVED),
	KEY(4, 7, KEY_RESERVED),

	KEY(5, 0, KEY_RESERVED),
	KEY(5, 1, KEY_RESERVED),
	KEY(5, 2, KEY_RESERVED),
	KEY(5, 3, KEY_RESERVED),
	KEY(5, 4, KEY_RESERVED),
	KEY(5, 5, KEY_RESERVED),
	KEY(5, 6, KEY_RESERVED),
	KEY(5, 7, KEY_RESERVED),

	KEY(6, 0, KEY_RESERVED),
	KEY(6, 1, KEY_RESERVED),
	KEY(6, 2, KEY_RESERVED),
	KEY(6, 3, KEY_RESERVED),
	KEY(6, 4, KEY_RESERVED),
	KEY(6, 5, KEY_RESERVED),
	KEY(6, 6, KEY_RESERVED),
	KEY(6, 7, KEY_RESERVED),

	KEY(7, 0, KEY_RESERVED),
	KEY(7, 1, KEY_RESERVED),
	KEY(7, 2, KEY_RESERVED),
	KEY(7, 3, KEY_RESERVED),
	KEY(7, 4, KEY_RESERVED),
	KEY(7, 5, KEY_RESERVED),
	KEY(7, 6, KEY_RESERVED),
	KEY(7, 7, KEY_RESERVED),
};

static struct matrix_keymap_data evm5430_keymap_data = {
	.keymap                 = evm5430_keymap,
	.keymap_size            = ARRAY_SIZE(evm5430_keymap),
};

static struct omap4_keypad_platform_data evm5430_keypad_data = {
	.keymap_data            = &evm5430_keymap_data,
	.rows                   = 8,
	.cols                   = 8,
};

static struct omap_board_data keypad_data = {
	.id                     = 1,
};

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux NULL
#endif

#if defined(CONFIG_TI_EMIF) || defined(CONFIG_TI_EMIF_MODULE)
#ifndef CONFIG_MACH_OMAP_5430ZEBU
static struct __devinitdata emif_custom_configs custom_configs = {
	.mask	= EMIF_CUSTOM_CONFIG_LPMODE,
	.lpmode	= EMIF_LP_MODE_DISABLE
};
#endif
#endif

static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
					MMC_CAP_1_8V_DDR,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.nonremovable	= true,
		.ocr_mask	= MMC_VDD_29_30,
		.no_off_init	= true,
	},
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= 67,
		.gpio_wp	= -EINVAL,
		.ocr_mask	= MMC_VDD_29_30,
	},
	{
		.mmc            = 3,
		.caps           = MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD,
		.gpio_cd        = -EINVAL,
		.gpio_wp        = -EINVAL,
		.ocr_mask       = MMC_VDD_165_195,
		.nonremovable   = true,
	},
	{}	/* Terminator */
};

#define HDMI_GPIO_HPD 193

#ifdef CONFIG_OMAP5_SEVM_PALMAS
#define OMAP5_GPIO_END	0

static struct palmas_gpadc_platform_data omap5_palmas_gpadc = {
	.ch3_current = 0,
	.ch0_current = 0,
	.bat_removal = 0,
	.start_polarity = 0,
};

/* Initialisation Data for Regulators */

static struct palmas_reg_init omap5_smps12_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 0,
	.tstep = 0,
};

static struct palmas_reg_init omap5_smps45_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 0,
	.tstep = 0,
};

static struct palmas_reg_init omap5_smps6_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 1,
	.tstep = 0,
};

static struct palmas_reg_init omap5_smps7_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 1,
};

static struct palmas_reg_init omap5_smps8_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 0,
	.tstep = 0,
};

static struct palmas_reg_init omap5_smps9_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 0,
	.vsel = 0xbd,
};

static struct palmas_reg_init omap5_smps10_init = {
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldo1_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldo2_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldo3_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldo4_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldo5_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldo6_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldo7_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldo8_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldo9_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldoln_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init omap5_ldousb_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init *palmas_omap_reg_init[] = {
	&omap5_smps12_init,
	NULL, /* SMPS123 not used in this configuration */
	NULL, /* SMPS3 not used in this configuration */
	&omap5_smps45_init,
	NULL, /* SMPS457 not used in this configuration */
	&omap5_smps6_init,
	&omap5_smps7_init,
	&omap5_smps8_init,
	&omap5_smps9_init,
	&omap5_smps10_init,
	&omap5_ldo1_init,
	&omap5_ldo2_init,
	&omap5_ldo3_init,
	&omap5_ldo4_init,
	&omap5_ldo5_init,
	&omap5_ldo6_init,
	&omap5_ldo7_init,
	&omap5_ldo8_init,
	&omap5_ldo9_init,
	&omap5_ldoln_init,
	&omap5_ldousb_init,

};

/* Constraints for Regulators */
static struct regulator_init_data omap5_smps12 = {
	.constraints = {
		.min_uV			= 600000,
	.max_uV			= 1310000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data omap5_smps45 = {
	.constraints = {
		.min_uV			= 600000,
		.max_uV			= 1310000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data omap5_smps6 = {
	.constraints = {
		.min_uV			= 1200000,
		.max_uV			= 1200000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_consumer_supply omap5_vdds1v8_main_supply[] = {
	REGULATOR_SUPPLY("vio", "1-004b"),
};

static struct regulator_init_data omap5_smps7 = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= ARRAY_SIZE(omap5_vdds1v8_main_supply),
	.consumer_supplies	= omap5_vdds1v8_main_supply,
};

static struct regulator_init_data omap5_smps8 = {
	.constraints = {
		.min_uV			= 600000,
		.max_uV			= 1310000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_consumer_supply omap5_adac_supply[] = {
	REGULATOR_SUPPLY("v2v1", "1-004b"),
};

static struct regulator_init_data omap5_smps9 = {
	.constraints = {
		.min_uV			= 2100000,
		.max_uV			= 2100000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= ARRAY_SIZE(omap5_adac_supply),
	.consumer_supplies	= omap5_adac_supply,

};

static struct regulator_consumer_supply omap5_vbus_supply[] = {
	REGULATOR_SUPPLY("vbus", "1-0048"),
};

static struct regulator_init_data omap5_smps10 = {
	.constraints = {
		.min_uV			= 5000000,
		.max_uV			= 5000000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= ARRAY_SIZE(omap5_vbus_supply),
	.consumer_supplies	= omap5_vbus_supply,
};

static struct regulator_init_data omap5_ldo1 = {
	.constraints = {
		.min_uV			= 2800000,
		.max_uV			= 2800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_consumer_supply omap5evm_lcd_panel_supply[] = {
	REGULATOR_SUPPLY("panel_supply", "omapdss_dsi.0"),
};

static struct regulator_init_data omap5_ldo2 = {
	.constraints = {
		.min_uV			= 2900000,
		.max_uV			= 2900000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
		.apply_uV		= 1,
	},
	.num_consumer_supplies	= ARRAY_SIZE(omap5evm_lcd_panel_supply),
	.consumer_supplies	= omap5evm_lcd_panel_supply,
};

static struct regulator_init_data omap5_ldo3 = {
	.constraints = {
		.min_uV			= 3000000,
		.max_uV			= 3000000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data omap5_ldo4 = {
	.constraints = {
		.min_uV			= 2200000,
		.max_uV			= 2200000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data omap5_ldo5 = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data omap5_ldo6 = {
	.constraints = {
		.min_uV			= 1500000,
		.max_uV			= 1500000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_consumer_supply omap5_dss_phy_supply[] = {
	REGULATOR_SUPPLY("vdds_dsi", "omapdss"),
	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dsi.0"),
	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dsi.1"),
	REGULATOR_SUPPLY("vdds_hdmi", "omapdss_hdmi"),
};

static struct regulator_init_data omap5_ldo7 = {
	.constraints = {
		.min_uV			= 1500000,
		.max_uV			= 1500000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
		.apply_uV		= 1,
	},
	.num_consumer_supplies	= ARRAY_SIZE(omap5_dss_phy_supply),
	.consumer_supplies	= omap5_dss_phy_supply,
};

static struct regulator_init_data omap5_ldo8 = {
	.constraints = {
		.min_uV			= 1500000,
		.max_uV			= 1500000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_consumer_supply omap5_mmc1_io_supply[] = {
	REGULATOR_SUPPLY("vmmc_aux", "omap_hsmmc.0"),
};
static struct regulator_init_data omap5_ldo9 = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 3300000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= ARRAY_SIZE(omap5_mmc1_io_supply),
	.consumer_supplies	= omap5_mmc1_io_supply,
};

static struct regulator_init_data omap5_ldoln = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data omap5_ldousb = {
	.constraints = {
		.min_uV			= 3250000,
		.max_uV			= 3250000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data *palmas_omap5_reg[] = {
	&omap5_smps12,
	NULL, /* SMPS123 not used in this configuration */
	NULL, /* SMPS3 not used in this configuration */
	&omap5_smps45,
	NULL, /* SMPS457 not used in this configuration */
	&omap5_smps6,
	&omap5_smps7,
	&omap5_smps8,
	&omap5_smps9,
	&omap5_smps10,

	&omap5_ldo1,
	&omap5_ldo2,
	&omap5_ldo3,
	&omap5_ldo4,
	&omap5_ldo5,
	&omap5_ldo6,
	&omap5_ldo7,
	&omap5_ldo8,
	&omap5_ldo9,
	&omap5_ldoln,
	&omap5_ldousb,
};

static struct palmas_pmic_platform_data omap5_palmas_pmic = {
	.reg_data = palmas_omap5_reg,
	.reg_init = palmas_omap_reg_init,

	.ldo6_vibrator = 0,
};

static struct palmas_resource_platform_data omap5_palmas_resource = {
	.clk32kg_mode_sleep = 0,
	.clk32kgaudio_mode_sleep = 0,
	.regen1_mode_sleep = 0,
	.regen2_mode_sleep = 0,
	.sysen1_mode_sleep = 0,
	.sysen2_mode_sleep = 0,

	.sysen2_mode_active = 1,

	.nsleep_res = 0,
	.nsleep_smps = 0,
	.nsleep_ldo1 = 0,
	.nsleep_ldo2 = 0,

	.enable1_res = 0,
	.enable1_smps = 0,
	.enable1_ldo1 = 0,
	.enable1_ldo2 = 0,

	.enable2_res = 0,
	.enable2_smps = 0,
	.enable2_ldo1 = 0,
	.enable2_ldo2 = 0,
};

static struct palmas_usb_platform_data omap5_palmas_usb = {
	.wakeup = 1,
};

static struct palmas_platform_data palmas_omap5 = {
	.gpio_base = OMAP5_GPIO_END,

	.power_ctrl = POWER_CTRL_NSLEEP_MASK | POWER_CTRL_ENABLE1_MASK |
			POWER_CTRL_ENABLE1_MASK,

	.gpadc_pdata = &omap5_palmas_gpadc,
	.pmic_pdata = &omap5_palmas_pmic,
	.usb_pdata = &omap5_palmas_usb,
	.resource_pdata = &omap5_palmas_resource,
};
#endif  /* CONFIG_OMAP5_SEVM_PALMAS */

static struct twl6040_codec_data twl6040_codec = {
	/* single-step ramp for headset and handsfree */
	.hs_left_step	= 0x0f,
	.hs_right_step	= 0x0f,
	.hf_left_step	= 0x1d,
	.hf_right_step	= 0x1d,
};

static struct twl6040_vibra_data twl6040_vibra = {
	.vibldrv_res = 8,
	.vibrdrv_res = 3,
	.viblmotor_res = 10,
	.vibrmotor_res = 10,
	.vddvibl_uV = 0,	/* fixed volt supply - VBAT */
	.vddvibr_uV = 0,	/* fixed volt supply - VBAT */
};

static struct twl6040_platform_data twl6040_data = {
	.codec		= &twl6040_codec,
	.vibra		= &twl6040_vibra,
	.audpwron_gpio	= 145,
	.irq_base	= TWL6040_CODEC_IRQ_BASE,
};

static struct platform_device omap5evm_dmic_codec = {
	.name	= "dmic-codec",
	.id	= -1,
};

static struct platform_device omap5evm_spdif_dit_codec = {
	.name           = "spdif-dit",
	.id             = -1,
};

static struct platform_device omap5evm_hdmi_audio_codec = {
	.name	= "hdmi-audio-codec",
	.id	= -1,
};

static struct omap_abe_twl6040_data omap5evm_abe_audio_data = {
	/* Audio out */
	.has_hs		= ABE_TWL6040_LEFT | ABE_TWL6040_RIGHT,
	/* HandsFree through expasion connector */
	.has_hf		= ABE_TWL6040_LEFT | ABE_TWL6040_RIGHT,
	/* PandaBoard: FM TX, PandaBoardES: can be connected to audio out */
	.has_aux	= ABE_TWL6040_LEFT | ABE_TWL6040_RIGHT,
	/* PandaBoard: FM RX, PandaBoardES: audio in */
	.has_afm	= ABE_TWL6040_LEFT | ABE_TWL6040_RIGHT,
	.has_abe	= 1,
	.has_dmic	= 1,
	.has_hsmic	= 1,
	.has_mainmic	= 1,
	.has_submic	= 1,
	/* Jack detection. */
	.jack_detection	= 1,
	/* MCLK input is 19.2MHz */
	.mclk_freq	= 19200000,
	.card_name = "OMAP5EVM",

};

static struct platform_device omap5evm_abe_audio = {
	.name		= "omap-abe-twl6040",
	.id		= -1,
	.dev = {
		.platform_data = &omap5evm_abe_audio_data,
	},
};

static struct platform_device *omap5evm_devices[] __initdata = {
	&omap5evm_dmic_codec,
	&omap5evm_spdif_dit_codec,
	&omap5evm_hdmi_audio_codec,
	&omap5evm_abe_audio,
};

static struct regulator_consumer_supply omap5_evm_vmmc1_supply[] = {
	REGULATOR_SUPPLY("vmmc", "omap_hsmmc.0"),
	REGULATOR_SUPPLY("vmmc", "omap_hsmmc.1"),
};

static struct regulator_init_data omap5_evm_vmmc1 = {
	.constraints = {
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on	= true,
	},
	.num_consumer_supplies = ARRAY_SIZE(omap5_evm_vmmc1_supply),
	.consumer_supplies = omap5_evm_vmmc1_supply,
};

static struct fixed_voltage_config omap5_evm_sd_dummy = {
	.supply_name = "vmmc_supply",
	.microvolts = 3000000, /* 3.0V */
	.gpio = -EINVAL,
	.init_data = &omap5_evm_vmmc1,
};

static struct platform_device dummy_sd_regulator_device = {
	.name		= "reg-fixed-voltage",
	.id		= 1,
	.dev = {
		.platform_data = &omap5_evm_sd_dummy,
	}
};

static struct panel_lg4591_data dsi_panel;
static struct omap_dss_board_info omap5evm_dss_data;

static void omap5evm_lcd_init(void)
{
	int r;

	r = gpio_request_one(dsi_panel.reset_gpio, GPIOF_DIR_OUT,
		"lcd1_reset_gpio");
	if (r)
		pr_err("%s: Could not get lcd1_reset_gpio\n", __func__);
}

static void omap5evm_hdmi_init(void)
{
	int r;

	r = gpio_request_one(HDMI_GPIO_HPD, GPIOF_DIR_IN,
				"hdmi_gpio_hpd");
	if (r)
		pr_err("%s: Could not get HDMI\n", __func__);

	/* Need to configure HPD as a gpio in mux */
	omap_hdmi_init(0);
}

static void __init omap5evm_display_init(void)
{
	omap5evm_lcd_init();
	omap5evm_hdmi_init();
	omap_display_init(&omap5evm_dss_data);
}

static void lg_panel_set_power(bool enable)
{
}

static struct panel_lg4591_data dsi_panel = {
	.reset_gpio = 183,
	.set_power = lg_panel_set_power,
};

static struct omap_dss_device omap5evm_lcd_device = {
	.name			= "lcd",
	.driver_name		= "lg4591",
	.type			= OMAP_DISPLAY_TYPE_DSI,
	.data			= &dsi_panel,
	.phy.dsi		= {
		.clk_lane	= 1,
		.clk_pol	= 0,
		.data1_lane	= 2,
		.data1_pol	= 0,
		.data2_lane	= 3,
		.data2_pol	= 0,
		.data3_lane	= 4,
		.data3_pol	= 0,
		.data4_lane	= 5,
		.data4_pol	= 0,
	},
	.clocks = {
		.dispc = {
			.channel = {
				.lck_div	= 1,	/* LCD */
				.pck_div	= 2,	/* PCD */
				.lcd_clk_src	= OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DISPC,
			},
			.dispc_fclk_src = OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DISPC,
		},
		.dsi = {
			.regn		= 19,	/* DSI_PLL_REGN */
			.regm		= 233,	/* DSI_PLL_REGM */

			.regm_dispc	= 3,	/* PLL_CLK1 (M4) */
			.regm_dsi	= 3,	/* PLL_CLK2 (M5) */
			.lp_clk_div	= 9,	/* LPDIV */

			.dsi_fclk_src	= OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DSI,
		},
	},
	.panel = {
		.dsi_mode	= OMAP_DSS_DSI_VIDEO_MODE,
		.width_in_um	= 55400,
		.height_in_um	= 98300,
	},
	.channel		= OMAP_DSS_CHANNEL_LCD,
};

static int omap5evm_panel_enable_hdmi(struct omap_dss_device *dssdev)
{
	return 0;
}

static void omap5evm_panel_disable_hdmi(struct omap_dss_device *dssdev)
{

}

static struct omap_dss_hdmi_data sdp54xx_hdmi_data = {
	.hpd_gpio = HDMI_GPIO_HPD,
};

static struct omap_dss_device omap5evm_hdmi_device = {
	.name = "hdmi",
	.driver_name = "hdmi_panel",
	.type = OMAP_DISPLAY_TYPE_HDMI,
	.platform_enable = omap5evm_panel_enable_hdmi,
	.platform_disable = omap5evm_panel_disable_hdmi,
	.channel = OMAP_DSS_CHANNEL_DIGIT,
	.data = &sdp54xx_hdmi_data,
};

static struct omap_dss_device *omap5evm_dss_devices[] = {
	&omap5evm_lcd_device,
	&omap5evm_hdmi_device,
};

static struct omap_dss_board_info omap5evm_dss_data = {
	.num_devices	= ARRAY_SIZE(omap5evm_dss_devices),
	.devices	= omap5evm_dss_devices,
	.default_device	= &omap5evm_lcd_device,
};

/*
 * I2C GPIO Expander - TCA6424
 */

/* Mounted on Base-Board */
static struct pca953x_platform_data omap5evm_gpio_expander_info = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
};

static struct i2c_board_info __initdata omap5evm_i2c_5_boardinfo[] = {
	{
		I2C_BOARD_INFO("tca6424", 0x22),
		.platform_data = &omap5evm_gpio_expander_info,
	},
};

static struct i2c_board_info __initdata omap5evm_i2c_1_boardinfo[] = {
#ifdef CONFIG_OMAP5_SEVM_PALMAS
	{
		I2C_BOARD_INFO("twl6035", 0x48),
		.platform_data = &palmas_omap5,
		.irq = OMAP44XX_IRQ_SYS_1N,
	},
#endif
	{
		I2C_BOARD_INFO("twl6040", 0x4b),
		.platform_data = &twl6040_data,
		.irq = OMAP44XX_IRQ_SYS_2N,
	},
};

static struct omap_i2c_bus_board_data __initdata omap5_i2c_1_bus_pdata;
static struct omap_i2c_bus_board_data __initdata omap5_i2c_2_bus_pdata;
static struct omap_i2c_bus_board_data __initdata omap5_i2c_3_bus_pdata;
static struct omap_i2c_bus_board_data __initdata omap5_i2c_4_bus_pdata;
static struct omap_i2c_bus_board_data __initdata omap5_i2c_5_bus_pdata;

static void __init omap_i2c_hwspinlock_init(int bus_id, int spinlock_id,
				struct omap_i2c_bus_board_data *pdata)
{
	/* spinlock_id should be -1 for a generic lock request */
	if (spinlock_id < 0)
		pdata->handle = hwspin_lock_request();
	else
		pdata->handle = hwspin_lock_request_specific(spinlock_id);

	if (pdata->handle != NULL) {
		pdata->hwspin_lock_timeout = hwspin_lock_timeout;
		pdata->hwspin_unlock = hwspin_unlock;
	} else
		pr_err("I2C hwspinlock request failed for bus %d\n", bus_id);
}

static int __init omap_5430evm_i2c_init(void)
{

	omap_i2c_hwspinlock_init(1, 0, &omap5_i2c_1_bus_pdata);
	omap_i2c_hwspinlock_init(2, 1, &omap5_i2c_2_bus_pdata);
	omap_i2c_hwspinlock_init(3, 2, &omap5_i2c_3_bus_pdata);
	omap_i2c_hwspinlock_init(4, 3, &omap5_i2c_4_bus_pdata);
	omap_i2c_hwspinlock_init(5, 4, &omap5_i2c_5_bus_pdata);

	omap_register_i2c_bus_board_data(1, &omap5_i2c_1_bus_pdata);
	omap_register_i2c_bus_board_data(2, &omap5_i2c_2_bus_pdata);
	omap_register_i2c_bus_board_data(3, &omap5_i2c_3_bus_pdata);
	omap_register_i2c_bus_board_data(4, &omap5_i2c_4_bus_pdata);
	omap_register_i2c_bus_board_data(5, &omap5_i2c_5_bus_pdata);

	omap5_i2c_pullup(5, OMAP5_I2C_PULLUP_EN, OMAP5_I2C_GLITCH_FREE_DIS);

	omap_register_i2c_bus(1, 400, omap5evm_i2c_1_boardinfo,
					ARRAY_SIZE(omap5evm_i2c_1_boardinfo));
	omap_register_i2c_bus(2, 400, NULL, 0);
	omap_register_i2c_bus(3, 400, NULL, 0);
	omap_register_i2c_bus(4, 400, NULL, 0);
	omap_register_i2c_bus(5, 400, omap5evm_i2c_5_boardinfo,
					ARRAY_SIZE(omap5evm_i2c_5_boardinfo));

	return 0;
}

/*
 * HSI usage is declared using bootargs variable:
 * board-omap5evm.modem_ipc=hsi
 * Variable modem_ipc is used to catch bootargs parameter value.
 */
static char *modem_ipc = "n/a";
module_param(modem_ipc, charp, 0);
MODULE_PARM_DESC(modem_ipc, "Modem IPC setting");

static const struct usbhs_omap_board_data usbhs_bdata __initconst = {
	.port_mode[0] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_HSIC,
	.port_mode[2] = OMAP_EHCI_PORT_MODE_HSIC,
	.phy_reset  = true,
	.reset_gpio_port[0]  = -EINVAL,
	.reset_gpio_port[1]  = GPIO_HUB_NRESET,
	.reset_gpio_port[2]  = GPIO_ETH_NRESET
};

static void __init omap_ehci_ohci_init(void)
{
	omap_mux_init_signal("gpio_172", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_NONE);
	omap_mux_init_signal("gpio_173", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_NONE);
	usbhs_init(&usbhs_bdata);
	return;
}

static void __init omap_5430evm_init(void)
{
	int status;
#if defined(CONFIG_TI_EMIF) || defined(CONFIG_TI_EMIF_MODULE)
#ifndef CONFIG_MACH_OMAP_5430ZEBU
	omap_emif_set_device_details(1, &lpddr2_elpida_4G_S4_x2_info,
			lpddr2_elpida_4G_S4_timings,
			ARRAY_SIZE(lpddr2_elpida_4G_S4_timings),
			&lpddr2_elpida_S4_min_tck,
			&custom_configs);

	omap_emif_set_device_details(2, &lpddr2_elpida_4G_S4_x2_info,
			lpddr2_elpida_4G_S4_timings,
			ARRAY_SIZE(lpddr2_elpida_4G_S4_timings),
			&lpddr2_elpida_S4_min_tck,
			&custom_configs);
#endif
#endif
	omap5_mux_init(board_mux, NULL, OMAP_PACKAGE_CBL);
	omap_sdrc_init(NULL, NULL);
	omap_create_board_props();
	omap_5430evm_i2c_init();
	omap_serial_init();
	platform_device_register(&dummy_sd_regulator_device);
	sevm_dock_init();
	sevm_touch_init();
	sevm_sensor_init();

	/* omap5evm_modem_init shall be called before omap_ehci_ohci_init */
	if (!strcmp(modem_ipc, "hsi"))
		omap5evm_modem_init(true);
	else
		omap5evm_modem_init(false);

	status = omap4_keyboard_init(&evm5430_keypad_data, &keypad_data);
	if (status)
		pr_err("Keypad initialization failed: %d\n", status);

	omap_ehci_ohci_init();

	omap5sevm_connectivity_init();
	omap_hsmmc_init(mmc);
	usb_dwc3_init();
	platform_add_devices(omap5evm_devices, ARRAY_SIZE(omap5evm_devices));

	omap_init_dmm_tiler();
	omap5_register_ion();
	omap5evm_display_init();
}

static void __init omap_5430evm_reserve(void)
{
	omap_rproc_reserve_cma(RPROC_CMA_OMAP5);

	/* do the static reservations first */
	memblock_remove(PHYS_ADDR_SMC_MEM, PHYS_ADDR_SMC_SIZE);
	memblock_remove(PHYS_ADDR_DUCATI_MEM, PHYS_ADDR_DUCATI_SIZE);

	omap5_ion_init();

	omap_reserve();
}

MACHINE_START(OMAP5_SEVM, "OMAP5 sevm board")
	/* Maintainer: Santosh Shilimkar - Texas Instruments Inc */
	.atag_offset	= 0x100,
	.reserve	= omap_5430evm_reserve,
	.map_io		= omap5_map_io,
	.init_early	= omap_5430evm_init_early,
	.init_irq	= gic_init_irq,
	.handle_irq	= gic_handle_irq,
	.init_machine	= omap_5430evm_init,
	.timer		= &omap5_timer,
MACHINE_END
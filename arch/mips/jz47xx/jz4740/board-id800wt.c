/*
 * Copyright (C) 2010 Lars-Peter Clausen <lars@metafoo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or later
 * as published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>

#include <jz4740/platform.h>

#include <linux/input.h>
#include <linux/power_supply.h>
#include <linux/pwm_backlight.h>

#include "../clock.h"

#include <jz4740_fb.h>
#include <jz4740_nand.h>

/* NAND */
static struct nand_ecclayout id800wt_ecclayout = {
	.oobfree = {
		{
			.offset = 2,
			.length = 4,
		},
		{
			.offset = 42,
			.length = 22,
		},
	}
};

static struct mtd_partition id800wt_partitions[] = {
	{ .name = "NAND BOOT partition",
	  .offset = 0 * 0x100000,
	  .size = 2 * 0x100000,
	},
	{ .name = "NAND KERNEL partition",
	  .offset = 2 * 0x100000,
	  .size = 4 * 0x100000,
	},
	{ .name = "NAND ROOTFS partition",
	  .offset = 6 * 0x100000,
	  .size = 498 * 0x100000,
	},
};

static struct jz_nand_platform_data id800wt_nand_pdata = {
	.ecc_layout = &id800wt_ecclayout,
	.partitions = id800wt_partitions,
	.num_partitions = ARRAY_SIZE(id800wt_partitions),
	.busy_gpio = JZ_GPIO_PORTC(30),
};

/* Display */
static struct fb_videomode id800wt_video_modes[] = {
	{
		.name = "800x600",
		.xres = 800,
		.yres = 600,
		.refresh = 40,
		.left_margin = 0,
		.right_margin = 255,
		.upper_margin = 0,
		.lower_margin = 35,
		.hsync_len = 1,
		.vsync_len = 1,
		.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
	},
};

static struct jz4740_fb_platform_data id800wt_fb_pdata = {
	.width		= 60,
	.height		= 45,
	.num_modes	= ARRAY_SIZE(id800wt_video_modes),
	.modes		= id800wt_video_modes,
	.bpp		= 16,
	.lcd_type	= JZ_LCD_TYPE_SPECIAL_TFT_1,
	.pixclk_falling_edge = 1,
	.special_tft_config = {
		.spl = JZ4740_FB_SPECIAL_TFT_CONFIG(1051, 1053),
		.cls = JZ4740_FB_SPECIAL_TFT_CONFIG(631, 744),
		.ps  = JZ4740_FB_SPECIAL_TFT_CONFIG(0, 45),
		.rev = JZ4740_FB_SPECIAL_TFT_CONFIG(0, 0),
	},
};

/* Backlight */
static int id800wt_backlight_invert(struct device *dev, int brightness)
{
	return 255 - brightness;
}

static struct platform_pwm_backlight_data id800wt_backlight_data = {
	.pwm_id = 7,
	.max_brightness = 255,
	.dft_brightness = 255,
	.pwm_period_ns = 8000000,
	.notify = id800wt_backlight_invert,
};

static struct platform_device id800wt_backlight_device = {
	.name = "pwm-backlight",
	.id = -1,
	.dev = {
		.platform_data = &id800wt_backlight_data,
		.parent = &jz4740_framebuffer_device.dev,
	},
};

static struct platform_device *jz_platform_devices[] __initdata = {
	&jz4740_usb_ohci_device,
	&jz4740_udc_device,
	&jz4740_nand_device,
	&jz4740_framebuffer_device,
	&jz4740_i2s_device,
	&jz4740_codec_device,
	&jz4740_pcm_device,
	&jz4740_rtc_device,
	&jz4740_adc_device,
	&id800wt_backlight_device,
};

static int __init id800wt_init_platform_devices(void)
{
	jz4740_framebuffer_device.dev.platform_data = &id800wt_fb_pdata;
	jz4740_nand_device.dev.platform_data = &id800wt_nand_pdata;

	jz4740_serial_device_register();

	jz_gpio_enable_pullup(JZ_GPIO_LCD_PS);
	jz_gpio_enable_pullup(JZ_GPIO_LCD_REV);

	return platform_add_devices(jz_platform_devices,
					ARRAY_SIZE(jz_platform_devices));
}

struct jz4740_clock_board_data jz4740_clock_bdata = {
	.ext_rate = 12000000,
	.rtc_rate = 32768,
};

static int __init id800wt_board_setup(void)
{
	printk("Sungale pictureframe id800wt setup\n");

	if (id800wt_init_platform_devices())
		panic("Failed to initalize platform devices\n");

	return 0;
}
arch_initcall(id800wt_board_setup);

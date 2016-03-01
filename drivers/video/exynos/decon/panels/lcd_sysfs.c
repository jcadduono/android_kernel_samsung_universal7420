/* linux/drivers/video/exynos_decon/panel/lcd_sysfs.c
 *
 * Header file for Samsung MIPI-DSI Panel SYSFS driver.
 *
 * Copyright (c) 2013 Samsung Electronics
 * JiHoon Kim <jihoonn.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/lcd.h>

#include "../dsim.h"
#include "dsim_panel.h"
#include "panel_info.h"
#include "dsim_backlight.h"

enum weakness_hbm_state {
	W_HBM_OFF = 0,
	W_HBM_INTERPOLATION = 1,
	W_HBM_GALLERY = 2
};
#define W_HBM_STEP	(5)

#if defined(CONFIG_SEC_FACTORY) && defined(CONFIG_EXYNOS_DECON_LCD_MCD)
#ifdef CONFIG_PANEL_S6E3HF3_DYNAMIC			// only edge panel

void mcd_mode_set(struct dsim_device *dsim)
{
	struct panel_private *panel = &dsim->priv;
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if(panel->mcd_on == 0)		// mcd_off
	{
		pr_info("%s MCD off : %d\n", __func__, panel->mcd_on);
		dsim_write_hl_data(dsim, SEQ_MCD_OFF_SET1, ARRAY_SIZE(SEQ_MCD_OFF_SET1));
		dsim_write_hl_data(dsim, SEQ_MCD_OFF_SET2, ARRAY_SIZE(SEQ_MCD_OFF_SET2));
		dsim_write_hl_data(dsim, SEQ_MCD_OFF_SET3, ARRAY_SIZE(SEQ_MCD_OFF_SET3));
		dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
		dsim_write_hl_data(dsim, SEQ_MCD_OFF_SET4, ARRAY_SIZE(SEQ_MCD_OFF_SET4));
	} else {					// mcd on
		pr_info("%s MCD on : %d\n", __func__, panel->mcd_on);
		dsim_write_hl_data(dsim, SEQ_MCD_ON_SET1, ARRAY_SIZE(SEQ_MCD_ON_SET1));
		dsim_write_hl_data(dsim, SEQ_MCD_ON_SET2, ARRAY_SIZE(SEQ_MCD_ON_SET2));
		dsim_write_hl_data(dsim, SEQ_MCD_ON_SET3, ARRAY_SIZE(SEQ_MCD_ON_SET3));
		dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
		dsim_write_hl_data(dsim, SEQ_MCD_ON_SET4, ARRAY_SIZE(SEQ_MCD_ON_SET4));
	}
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
}

#else
void mcd_mode_set(struct dsim_device *dsim)
{
	int i = 0;
	struct panel_private *panel = &dsim->priv;
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F1, ARRAY_SIZE(SEQ_TEST_KEY_ON_F1));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if(panel->mcd_on == 0)		// mcd_off
	{
		pr_info("%s MCD off : %d\n", __func__, panel->mcd_on);
		for(i = 0; i < (sizeof(SEQ_MCD_OFF_SET1) / sizeof(SEQ_MCD_OFF_SET1[0])); i++)
			dsim_write_hl_data(dsim, SEQ_MCD_OFF_SET1[i], ARRAY_SIZE(SEQ_MCD_OFF_SET1[i]));
		dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
		msleep(100);
		for(i = 0; i < (sizeof(SEQ_MCD_OFF_SET2) / sizeof(SEQ_MCD_OFF_SET2[0])); i++)
			dsim_write_hl_data(dsim, SEQ_MCD_OFF_SET2[i], ARRAY_SIZE(SEQ_MCD_OFF_SET2[i]));
	} else {					// mcd on
		pr_info("%s MCD on : %d\n", __func__, panel->mcd_on);
		for(i = 0; i < (sizeof(SEQ_MCD_ON_SET1) / sizeof(SEQ_MCD_ON_SET1[0])); i++)
			dsim_write_hl_data(dsim, SEQ_MCD_ON_SET1[i], ARRAY_SIZE(SEQ_MCD_ON_SET1[i]));
		dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
		msleep(100);
		for(i = 0; i < (sizeof(SEQ_MCD_ON_SET2) / sizeof(SEQ_MCD_ON_SET2[0])); i++)
			dsim_write_hl_data(dsim, SEQ_MCD_ON_SET2[i], ARRAY_SIZE(SEQ_MCD_ON_SET2[i]));
	}
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F1, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F1));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
}
#endif
static ssize_t mcd_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%u\n", priv->mcd_on);

	return strlen(buf);
}

static ssize_t mcd_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);
	int value;
	int rc;

	dsim = container_of(priv, struct dsim_device, priv);

	rc = kstrtouint(buf, (unsigned int)0, &value);
	if (rc < 0)
		return rc;

	if ((priv->state == PANEL_STATE_RESUMED) && (priv->mcd_on != value)) {
		priv->mcd_on = value;
		mcd_mode_set(dsim);
	}

	dev_info(dev, "%s: %d\n", __func__, priv->mcd_on);

	return size;
}

static DEVICE_ATTR(mcd_mode, 0664, mcd_mode_show, mcd_mode_store);

#endif

#ifdef CONFIG_LCD_HMT
#if defined(CONFIG_PANEL_S6E3HF2_DYNAMIC)
static struct lcd_seq_info SEQ_HMT_AID_FORWARD_SET[] = {
	{(u8 *)SEQ_HMT_AID_FORWARD1, ARRAY_SIZE(SEQ_HMT_AID_FORWARD1), 0},
	{(u8 *)SEQ_HMT_AID_FORWARD2, ARRAY_SIZE(SEQ_HMT_AID_FORWARD2), 0},
};

static struct lcd_seq_info SEQ_HMT_REVERSE_SET[] = {
	{(u8 *)SEQ_HMT_AID_REVERSE1, ARRAY_SIZE(SEQ_HMT_AID_REVERSE1), 0},
	{(u8 *)SEQ_HMT_AID_REVERSE2, ARRAY_SIZE(SEQ_HMT_AID_REVERSE2), 0},
};

static struct lcd_seq_info SEQ_HMT_ON_SET[] = {
	{(u8 *)SEQ_HMT_ON1, ARRAY_SIZE(SEQ_HMT_ON1), 0},
	{(u8 *)SEQ_HMT_ON2, ARRAY_SIZE(SEQ_HMT_ON2), 0},
	{(u8 *)SEQ_HMT_ON3, ARRAY_SIZE(SEQ_HMT_ON3), 0},
};

static struct lcd_seq_info SEQ_HMT_OFF_SET[] = {
	{(u8 *)SEQ_HMT_OFF1, ARRAY_SIZE(SEQ_HMT_OFF1), 0},
};
#elif defined(CONFIG_PANEL_S6E3HA2_DYNAMIC)
static struct lcd_seq_info SEQ_HMT_AID_FORWARD_SET[] = {
	{(u8 *)SEQ_HMT_AID_FORWARD1, ARRAY_SIZE(SEQ_HMT_AID_FORWARD1), 0},
	{(u8 *)SEQ_HMT_AID_FORWARD2, ARRAY_SIZE(SEQ_HMT_AID_FORWARD2), 0},
};

static struct lcd_seq_info SEQ_HMT_REVERSE_SET[] = {
	{(u8 *)SEQ_HMT_AID_REVERSE1, ARRAY_SIZE(SEQ_HMT_AID_REVERSE1), 0},
	{(u8 *)SEQ_HMT_AID_REVERSE2, ARRAY_SIZE(SEQ_HMT_AID_REVERSE2), 0},
};

static struct lcd_seq_info SEQ_HMT_ON_SET[] = {
	{(u8 *)SEQ_HMT_ON1, ARRAY_SIZE(SEQ_HMT_ON1), 0},
	{(u8 *)SEQ_HMT_ON2, ARRAY_SIZE(SEQ_HMT_ON2), 0},
	{(u8 *)SEQ_HMT_ON3, ARRAY_SIZE(SEQ_HMT_ON3), 0},
};

static struct lcd_seq_info SEQ_HMT_OFF_SET[] = {
	{(u8 *)SEQ_HMT_OFF1, ARRAY_SIZE(SEQ_HMT_OFF1), 0},
	{(u8 *)SEQ_HMT_OFF2, ARRAY_SIZE(SEQ_HMT_OFF2), 0},
};
#elif defined(CONFIG_PANEL_S6E3HF3_DYNAMIC) || defined(CONFIG_PANEL_S6E3HA3_DYNAMIC)
static struct lcd_seq_info SEQ_HMT_AID_FORWARD_SET[] = {
	{(u8 *)SEQ_HMT_AID_FORWARD1, ARRAY_SIZE(SEQ_HMT_AID_FORWARD1), 0},
};

static struct lcd_seq_info SEQ_HMT_REVERSE_SET[] = {
	{(u8 *)SEQ_HMT_AID_REVERSE1, ARRAY_SIZE(SEQ_HMT_AID_REVERSE1), 0},
};

static struct lcd_seq_info SEQ_HMT_ON_SET[] = {
	{(u8 *)SEQ_HMT_ON1, ARRAY_SIZE(SEQ_HMT_ON1), 0},
};

static struct lcd_seq_info SEQ_HMT_OFF_SET[] = {
	{(u8 *)SEQ_HMT_OFF1, ARRAY_SIZE(SEQ_HMT_OFF1), 0},
};

#endif

static ssize_t hmt_brightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "index : %d, brightenss : %d\n", priv->hmt_br_index, priv->hmt_brightness);

	return strlen(buf);
}

static ssize_t hmt_brightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);
	int value;
	int rc;

	dsim = container_of(priv, struct dsim_device, priv);

	rc = kstrtouint(buf, (unsigned int)0, &value);
	if (rc < 0)
		return rc;
	if (priv->state != PANEL_STATE_RESUMED) {
		dev_info(dev, "%s: panel is off\n", __func__);
		return -EINVAL;
	}

	if (priv->hmt_on == HMT_OFF) {
		dev_info(dev, "%s: hmt is not on\n", __func__);
		return -EINVAL;
	}

	if (priv->hmt_brightness != value) {
		mutex_lock(&priv->lock);
		priv->hmt_brightness = value;
		mutex_unlock(&priv->lock);
		dsim_panel_set_brightness_for_hmt(dsim, 0);
	}

	dev_info(dev, "%s: %d\n", __func__, value);
	return size;
}


static int hmt_write_set(struct dsim_device *dsim, struct lcd_seq_info *seq, u32 num)
{
	int ret = 0, i;

	for (i = 0; i < num; i++) {
		if (seq[i].cmd) {
			ret = dsim_write_hl_data(dsim, seq[i].cmd, seq[i].len);
			if (ret != 0) {
				dsim_err("%s failed.\n", __func__);
				return ret;
			}
		}
		if (seq[i].sleep)
			usleep_range(seq[i].sleep * 1000 , seq[i].sleep * 1000);
	}
	return ret;
}

int hmt_set_mode(struct dsim_device *dsim, bool wakeup)
{
	struct panel_private *priv = &(dsim->priv);

	mutex_lock(&priv->lock);
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));

	if(priv->hmt_on == HMT_ON) {
		// on set
		hmt_write_set(dsim, SEQ_HMT_ON_SET, ARRAY_SIZE(SEQ_HMT_ON_SET));
		hmt_write_set(dsim, SEQ_HMT_REVERSE_SET, ARRAY_SIZE(SEQ_HMT_REVERSE_SET));
		if(wakeup) {
			dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
			dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE_L, ARRAY_SIZE(SEQ_GAMMA_UPDATE_L));
			msleep(120);
		}
	} else if(priv->hmt_on == HMT_OFF) {
		// off set
		hmt_write_set(dsim, SEQ_HMT_OFF_SET, ARRAY_SIZE(SEQ_HMT_OFF_SET));
		hmt_write_set(dsim, SEQ_HMT_AID_FORWARD_SET, ARRAY_SIZE(SEQ_HMT_AID_FORWARD_SET));

	} else {
		pr_info("hmt state is invalid %d !\n", priv->hmt_on);
	}

	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));
	mutex_unlock(&priv->lock);

	if(priv->hmt_on == HMT_ON)
		dsim_panel_set_brightness_for_hmt(dsim, 1);
	else if (priv->hmt_on == HMT_OFF)
		dsim_panel_set_brightness(dsim, 1);
	else ;

	return 0;
}

static ssize_t hmt_on_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%u\n", priv->hmt_on);

	return strlen(buf);
}

static ssize_t hmt_on_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);
	int value;
	int rc;
	dsim = container_of(priv, struct dsim_device, priv);

	rc = kstrtoint(buf, 0, &value);
	if (rc < 0)
		return rc;

	if (priv->state != PANEL_STATE_RESUMED) {
		dev_info(dev, "%s: panel is off\n", __func__);
		return -EINVAL;
	}

	if (priv->hmt_on != value) {
		mutex_lock(&priv->lock);
		priv->hmt_on = value;
		mutex_unlock(&priv->lock);
		dev_info(dev, "++%s: %d\n", __func__, priv->hmt_on);
		hmt_set_mode(dsim, false);
		dev_info(dev, "--%s: %d\n", __func__, priv->hmt_on);
	} else
		dev_info(dev, "%s: hmt already %s\n", __func__, value ? "on" : "off");

	return size;
}

static DEVICE_ATTR(hmt_bright, 0664, hmt_brightness_show, hmt_brightness_store);
static DEVICE_ATTR(hmt_on, 0664, hmt_on_show, hmt_on_store);

#endif

#ifdef CONFIG_LCD_ALPM
#if defined(CONFIG_PANEL_S6E3HF3_DYNAMIC)
int alpm_set_mode(struct dsim_device *dsim, int enable)
{
	struct panel_private *priv = &(dsim->priv);
	if(priv->alpm_support != 1) {
		pr_info("%s this panel do not support alpm %d!\n", __func__, priv->alpm_support);
		return 0;
	}
	if((enable != ALPM_ON) && (enable != ALPM_OFF)) {
		pr_info("alpm state is invalid %d !\n", priv->alpm);
		return 0;
	}
	dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
	usleep_range(17000, 17000);
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if(enable == ALPM_ON) {
		dsim_write_hl_data(dsim, SEQ_ALPM2NIT_MODE_ON, ARRAY_SIZE(SEQ_ALPM2NIT_MODE_ON));
	} else if(enable == ALPM_OFF) {
		dsim_write_hl_data(dsim, SEQ_NORMAL_MODE_ON, ARRAY_SIZE(SEQ_NORMAL_MODE_ON));
	}
	dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
	dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE_L, ARRAY_SIZE(SEQ_GAMMA_UPDATE_L));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	dsim_write_hl_data(dsim, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON)); /* (workaround) DDI 0x0A register : DISP_ON bit not upset */
	usleep_range(17000, 17000);
	dsim_write_hl_data(dsim, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));

	priv->current_alpm = dsim->alpm = enable;

	return 0;
}
#else
int alpm_set_mode(struct dsim_device *dsim, int enable)
{
	struct panel_private *priv = &(dsim->priv);
	if((enable != ALPM_ON) && (enable != ALPM_OFF)) {
		pr_info("alpm state is invalid %d !\n", priv->alpm);
		return 0;
	}
	dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
	usleep_range(17000, 17000);
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if(enable == ALPM_ON) {
		priv->mtpForALPM[34] = priv->mtpForALPM[35]= 0;
		dsim_write_hl_data(dsim, priv->mtpForALPM, ARRAY_SIZE(priv->mtpForALPM));
		dsim_write_hl_data(dsim, SEQ_ALPM2NIT_MODE_ON, ARRAY_SIZE(SEQ_ALPM2NIT_MODE_ON));
	} else if(enable == ALPM_OFF) {
		priv->mtpForALPM[34] = priv->prev_VT[0];
		priv->mtpForALPM[35] = priv->prev_VT[1];
		dsim_write_hl_data(dsim, priv->mtpForALPM, ARRAY_SIZE(priv->mtpForALPM));
		dsim_write_hl_data(dsim, SEQ_NORMAL_MODE_ON, ARRAY_SIZE(SEQ_NORMAL_MODE_ON));
	}
	dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
	dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE_L, ARRAY_SIZE(SEQ_GAMMA_UPDATE_L));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	usleep_range(17000, 17000);
	dsim_write_hl_data(dsim, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));

	priv->current_alpm = dsim->alpm = enable;

	return 0;
}
#endif
static ssize_t alpm_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%d\n", priv->alpm);

	dev_info(dev, "%s: %d\n", __func__, priv->alpm);

	return strlen(buf);
}

#if defined (CONFIG_SEC_FACTORY)
static int prev_brightness = 0;
#endif

static ssize_t alpm_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);
	int value;

	dsim = container_of(priv, struct dsim_device, priv);

	sscanf(buf, "%9d", &value);
	dev_info(dev, "%s: %d \n", __func__, value);

	mutex_lock(&priv->alpm_lock);
#if defined (CONFIG_SEC_FACTORY)
	if (value) {
		if ((priv->state == PANEL_STATE_RESUMED) && !priv->current_alpm) {
			prev_brightness = priv->bd->props.brightness;
			priv->bd->props.brightness = UI_MIN_BRIGHTNESS;
			dsim_panel_set_brightness(dsim, 1);
			alpm_set_mode(dsim, ALPM_ON);
		}
	} else {
		if ((priv->state == PANEL_STATE_RESUMED) && priv->current_alpm) {
			priv->bd->props.brightness = prev_brightness;
			alpm_set_mode(dsim, ALPM_OFF);
			dsim_panel_set_brightness(dsim, 1);
		}
	}
#else
	if (value) {
		if ((priv->state == PANEL_STATE_RESUMED) && !priv->current_alpm)
			alpm_set_mode(dsim, ALPM_ON);
	} else {
		if ((priv->state == PANEL_STATE_RESUMED)&& priv->current_alpm)
			alpm_set_mode(dsim, ALPM_OFF);
#if defined(CONFIG_PANEL_S6E3HF3_DYNAMIC)
		usleep_range(17000, 17000);
		dsim_panel_set_brightness(dsim, 1);
#endif
	}
#endif
	priv->alpm = value;

	mutex_unlock(&priv->alpm_lock);

	dev_info(dev, "%s: %d %d\n", __func__, priv->alpm, dsim->alpm);

	return size;
}

static DEVICE_ATTR(alpm, 0664, alpm_show, alpm_store);
#endif

static ssize_t lcd_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "SDC_%02X%02X%02X\n", priv->id[0], priv->id[1], priv->id[2]);

	return strlen(buf);
}

static ssize_t window_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%02x %02x %02x\n", priv->id[0], priv->id[1], priv->id[2]);

	return strlen(buf);
}

static ssize_t brightness_table_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *panel = dev_get_drvdata(dev);

	char *pos = buf;
	int nit, i;
	for (i = 0; i <= UI_MAX_BRIGHTNESS; i++) {
		nit = (panel->weakness_hbm_comp == 1) ? panel->hbm_inter_br_tbl[i] : panel->br_tbl[i];
		pos += sprintf(pos, "%3d %3d\n", i, nit);
	}
	return pos - buf;
}

static ssize_t auto_brightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%u\n", priv->auto_brightness);

	return strlen(buf);
}

static ssize_t auto_brightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);
	int value;
	int rc;

	dsim = container_of(priv, struct dsim_device, priv);

	rc = kstrtouint(buf, (unsigned int)0, &value);
	if (rc < 0)
		return rc;
	else {
		if (priv->auto_brightness != value) {
			dev_info(dev, "%s: %d, %d\n", __func__, priv->auto_brightness, value);
			mutex_lock(&priv->lock);
			priv->auto_brightness = value;
			mutex_unlock(&priv->lock);
			dsim_panel_set_brightness(dsim, 0);
		}
	}
	return size;
}

static ssize_t siop_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%u\n", priv->siop_enable);

	return strlen(buf);
}

static ssize_t siop_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);
	int value;
	int rc;

	dsim = container_of(priv, struct dsim_device, priv);

	rc = kstrtouint(buf, (unsigned int)0, &value);
	if (rc < 0)
		return rc;
	else {
		if (priv->siop_enable != value) {
			dev_info(dev, "%s: %d, %d\n", __func__, priv->siop_enable, value);
			mutex_lock(&priv->lock);
			priv->siop_enable = value;
			mutex_unlock(&priv->lock);
#ifdef CONFIG_LCD_HMT
			if(priv->hmt_on == HMT_ON)
				dsim_panel_set_brightness_for_hmt(dsim, 1);
			else
#endif
				dsim_panel_set_brightness(dsim, 1);
		}
	}
	return size;
}

static ssize_t temperature_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char temp[] = "-20, -19, 0, 1\n";

	strcat(buf, temp);
	return strlen(buf);
}

static ssize_t temperature_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);
	int value;
	int rc;

	dsim = container_of(priv, struct dsim_device, priv);

	rc = kstrtoint(buf, 10, &value);

	if (rc < 0)
		return rc;
	else {
		mutex_lock(&priv->lock);
		priv->temperature = value;
		mutex_unlock(&priv->lock);
#ifdef CONFIG_LCD_HMT
		if(priv->hmt_on == HMT_ON)
			dsim_panel_set_brightness_for_hmt(dsim, 1);
		else
#endif
			dsim_panel_set_brightness(dsim, 1);
		dev_info(dev, "%s: %d, %d\n", __func__, value, priv->temperature);
	}

	return size;
}

static ssize_t color_coordinate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%u, %u\n", priv->coordinate[0], priv->coordinate[1]);
	return strlen(buf);
}

static ssize_t manufacture_date_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);
	u16 year;
	u8 month, day, hour, min;

	year = ((priv->date[0] & 0xF0) >> 4) + 2011;
	month = priv->date[0] & 0xF;
	day = priv->date[1] & 0x1F;
	hour = priv->date[2] & 0x1F;
	min = priv->date[3] & 0x3F;

	sprintf(buf, "%d, %d, %d, %d:%d\n", year, month, day, hour, min);
	return strlen(buf);
}

static ssize_t read_mtp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return strlen(buf);
}

static ssize_t read_mtp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);
	unsigned int reg, pos, length, i;
	unsigned char readbuf[256] = {0xff, };
	unsigned char writebuf[2] = {0xB0, };

	dsim = container_of(priv, struct dsim_device, priv);

	sscanf(buf, "%x %d %d", &reg, &pos, &length);

	if (!reg || !length || pos < 0 || reg > 0xff || length > 255 || pos > 255)
		return -EINVAL;
	if (priv->state != PANEL_STATE_RESUMED)
		return -EINVAL;

	writebuf[1] = pos;
	if (dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0)) < 0)
		dsim_err("fail to write F0 on command.\n");

	if (dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC)) < 0)
		dsim_err("fail to write test on fc command.\n");

	if (dsim_write_hl_data(dsim, writebuf, ARRAY_SIZE(writebuf)) < 0)
		dsim_err("fail to write global command.\n");

	if (dsim_read_hl_data(dsim, reg, length, readbuf) < 0)
		dsim_err("fail to read id on command.\n");

	if (dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0)) < 0)
		dsim_err("fail to write F0 on command.\n");

	if (dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC)) < 0)
		dsim_err("fail to write test on fc command.\n");

	dsim_info("READ_Reg addr : %02x, start pos : %d len : %d \n", reg, pos, length);
	for (i = 0; i < length; i++)
		dsim_info("READ_Reg %dth : %02x \n", i + 1, readbuf[i]);

	return size;
}

#ifdef CONFIG_LCD_BURNIN_CORRECTION
static ssize_t ldu_correction_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return strlen(buf);
}

static ssize_t ldu_correction_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);
	int rc;
	int value;

	dsim = container_of(priv, struct dsim_device, priv);
	rc = kstrtouint(buf, (unsigned int)0, &value);
	if (rc < 0)
		return rc;
	else {
		if((value < 0) || (value > 7)) {
			pr_info("%s out of range %d\n", __func__, value);
			return -EINVAL;
		}
		if(priv->ldu_tbl[0] == NULL) {
			pr_info("%s do not support ldu correction %d\n", __func__, value);
			return -EINVAL;
		}

		priv->ldu_correction_state = value;
		priv->br_tbl = priv->ldu_tbl[priv->ldu_correction_state];
		dev_info(dev, "%s: %d\n", __func__, priv->ldu_correction_state);
		dsim_panel_set_brightness(dsim, 1);
	}
	return size;
}
static DEVICE_ATTR(ldu_correction, 0664, ldu_correction_show, ldu_correction_store);

#endif

#ifdef CONFIG_PANEL_AID_DIMMING
static void show_aid_log(struct dsim_device *dsim)
{
	int i, j, k;
	struct dim_data *dim_data = NULL;
	struct SmtDimInfo *dimming_info = NULL;
#ifdef CONFIG_LCD_HMT
	struct SmtDimInfo *hmt_dimming_info = NULL;
#endif
	struct panel_private *panel = &dsim->priv;
	u8 temp[256];


	if (panel == NULL) {
		dsim_err("%s:panel is NULL\n", __func__);
		return;
	}

	dim_data = (struct dim_data*)(panel->dim_data);
	if (dim_data == NULL) {
		dsim_info("%s:dimming is NULL\n", __func__);
		return;
	}

	dimming_info = (struct SmtDimInfo*)(panel->dim_info);
	if (dimming_info == NULL) {
		dsim_info("%s:dimming is NULL\n", __func__);
		return;
	}

	dsim_info("MTP VT : %d %d %d\n",
			dim_data->vt_mtp[CI_RED], dim_data->vt_mtp[CI_GREEN], dim_data->vt_mtp[CI_BLUE] );

	for(i = 0; i < VMAX; i++) {
		dsim_info("MTP V%d : %4d %4d %4d \n",
			vref_index[i], dim_data->mtp[i][CI_RED], dim_data->mtp[i][CI_GREEN], dim_data->mtp[i][CI_BLUE] );
	}

	for(i = 0; i < MAX_BR_INFO; i++) {
		memset(temp, 0, sizeof(temp));
		for(j = 1; j < OLED_CMD_GAMMA_CNT; j++) {
			if (j == 1 || j == 3 || j == 5)
				k = dimming_info[i].gamma[j++] * 256;
			else
				k = 0;
			snprintf(temp + strnlen(temp, 256), 256, " %d", dimming_info[i].gamma[j] + k);
		}
		dsim_info("nit :%3d %s\n", dimming_info[i].br, temp);
	}
#ifdef CONFIG_LCD_HMT
	hmt_dimming_info = (struct SmtDimInfo*)(panel->hmt_dim_info);
	if (hmt_dimming_info == NULL) {
		dsim_info("%s:dimming is NULL\n", __func__);
		return;
	}
	for(i = 0; i < HMT_MAX_BR_INFO; i++) {
		memset(temp, 0, sizeof(temp));
		for(j = 1; j < OLED_CMD_GAMMA_CNT; j++) {
			if (j == 1 || j == 3 || j == 5)
				k = hmt_dimming_info[i].gamma[j++] * 256;
			else
				k = 0;
			snprintf(temp + strnlen(temp, 256), 256, " %d", hmt_dimming_info[i].gamma[j] + k);
		}
		dsim_info("nit :%3d %s\n", hmt_dimming_info[i].br, temp);
	}
#endif
}


static ssize_t aid_log_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);

	dsim = container_of(priv, struct dsim_device, priv);

	show_aid_log(dsim);
	return strlen(buf);
}

#endif

static ssize_t manufacture_code_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%02X%02X%02X%02X%02X\n",
		priv->code[0], priv->code[1], priv->code[2], priv->code[3], priv->code[4]);

	return strlen(buf);
}

static ssize_t cell_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
		priv->date[0] , priv->date[1], priv->date[2], priv->date[3], priv->date[4],
		priv->date[5],priv->date[6], (priv->coordinate[0] &0xFF00)>>8,priv->coordinate[0] &0x00FF,
		(priv->coordinate[1]&0xFF00)>>8,priv->coordinate[1]&0x00FF);

	return strlen(buf);
}

static ssize_t auto_brightness_level_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);
	sprintf(buf, "%u\n", priv->auto_brightness_level);

	return strlen(buf);
}

static ssize_t brightness_step_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);
	sprintf(buf, "%u\n", priv->brightness_step);

	return strlen(buf);
}


#ifdef CONFIG_LCD_HBM_INTERPOLATION
static int find_hbm_table(struct dsim_device *dsim, int nit, int mode)
{
	int retVal = 0;
	int i;
	int current_gap;
	int minVal = 20000;

	for (i = 0; i < 256; i++) {
		if (mode == W_HBM_INTERPOLATION)	// setting
			current_gap = nit - dsim->priv.hbm_inter_br_tbl[i];
		else if (mode == W_HBM_GALLERY)	// gallery
			current_gap = nit - dsim->priv.gallery_br_tbl[i];
		else
			current_gap = nit - dsim->priv.gallery_br_tbl[i];

		if (current_gap < 0)
			current_gap *= -1;
		if (minVal > current_gap) {
			minVal = current_gap;
			retVal = i;
		}
	}
	return retVal;
}

static ssize_t weakness_hbm_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct panel_private *priv = dev_get_drvdata(dev);

	sprintf(buf, "%d\n", priv->weakness_hbm_comp);

	return strlen(buf);
}

#if defined(CONFIG_PANEL_S6E3HF3_DYNAMIC) || defined(CONFIG_PANEL_S6E3HA3_DYNAMIC)
// for noble/zero2
static ssize_t weakness_hbm_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int rc;
	int value, i;
	int hbm_nit, origin_nit, p_br, gap_nit;
	int index_array[6];
	int nFlagForInter = 0;
	int force = 0;
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);

	dsim = container_of(priv, struct dsim_device, priv);

	rc = kstrtouint(buf, (unsigned int)0, &value);

	if (rc < 0)
		return rc;
	else {
		if(priv->hbm_inter_br_tbl == NULL) {
			pr_info("this panel don't support interpolation\n");
			return size;
		}
		if (priv->weakness_hbm_comp != value){
			if((priv->weakness_hbm_comp == 1) && (value == 2)) {
				pr_info("%s don't support color blind -> gallery\n", __func__);
				return size;
			}
			if((priv->weakness_hbm_comp == 1) || (value == 1)) {
				p_br = priv->bd->props.brightness;
				hbm_nit = priv->hbm_inter_br_tbl[p_br];
				nFlagForInter = 1;
				origin_nit = priv->br_tbl[p_br];
				gap_nit = (hbm_nit - origin_nit) / 5;
				index_array[5] = priv->bd->props.brightness;
				for(i = 0; i < 5; i++) {
					index_array[i] = find_hbm_table(dsim, origin_nit, nFlagForInter);
					origin_nit += gap_nit;
				}
				if(value)
					priv->weakness_hbm_comp = value;
				for(i = 1; i < 5; i++) {
					if(value)
						priv->bd->props.brightness = index_array[i];
					else
						priv->bd->props.brightness = index_array[5 - i];
					dsim_panel_set_brightness(dsim, 0);
					msleep(20);
				}
				if(!value)
					priv->weakness_hbm_comp = value;
				priv->bd->props.brightness = index_array[5];
			} else if((priv->weakness_hbm_comp == 2) || (value == 2)) {
				priv->weakness_hbm_comp = value;
				force = 1;
			} else {
				pr_info("%s invalid input %d \n", __func__, value);
				return size;
			}


			dsim_panel_set_brightness(dsim, force);

			dev_info(dev, "%s: %d, %d\n", __func__, priv->weakness_hbm_comp, value);
		}
	}
	return size;
}

#else
static ssize_t weakness_hbm_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int rc;
	int value, i;
	int hbm_nit, origin_nit, p_br, gap_nit;
	int index_array[W_HBM_STEP];
	int update_br;
	int final_br;
	int nFlagForInter = 0;
	struct dsim_device *dsim;
	struct panel_private *priv = dev_get_drvdata(dev);

	dsim = container_of(priv, struct dsim_device, priv);

	rc = kstrtouint(buf, (unsigned int)0, &value);
	if (rc < 0)
		return rc;

	if (priv->weakness_hbm_comp != value) {
		if ((priv->weakness_hbm_comp == W_HBM_INTERPOLATION) && (value == W_HBM_GALLERY)) {
			/* Interpolation->Gallery */
			pr_info("%s don't support color blind -> gallery\n", __func__);
			return size;
		}

		p_br = priv->bd->props.brightness;
		final_br = priv->bd->props.brightness;

		if ((priv->weakness_hbm_comp == W_HBM_INTERPOLATION) || (value == W_HBM_INTERPOLATION)) {
			/* Gallery->Interpolation or OFF->Interpolation or Interpolation->OFF */
			hbm_nit = priv->hbm_inter_br_tbl[p_br];
			nFlagForInter = W_HBM_INTERPOLATION;
		} else if ((priv->weakness_hbm_comp == W_HBM_GALLERY) || (value == W_HBM_GALLERY)) {
			/* OFF->Gallery or Gallery->OFF */
			hbm_nit = priv->gallery_br_tbl[p_br];
			nFlagForInter = W_HBM_GALLERY;
		}

		// index_array[0](=normal_br) ~ index_array[high](=hbm_br) //
		origin_nit = priv->br_tbl[p_br];
		gap_nit = (hbm_nit - origin_nit) / W_HBM_STEP;
		for (i = 0; i < W_HBM_STEP; i++) {
			index_array[i] = find_hbm_table(dsim, origin_nit, nFlagForInter);
			origin_nit += gap_nit;
		}

		if (value != W_HBM_OFF)
			priv->weakness_hbm_comp = value;

		priv->is_br_override = true;
		for (i = 1; i < W_HBM_STEP; i++) {

			if (value != W_HBM_OFF)
				update_br = index_array[i];
			else
				update_br = index_array[W_HBM_STEP - i];

			priv->override_br_value = update_br;
			priv->bd->props.brightness = update_br;
			dsim_panel_set_brightness(dsim, 0);

			msleep(20);

			if (priv->bd->props.brightness != update_br)
				final_br = priv->bd->props.brightness;
		}
		priv->is_br_override = false;

		if (value == W_HBM_OFF)
			priv->weakness_hbm_comp = value;

		priv->bd->props.brightness = final_br;
		dsim_panel_set_brightness(dsim, 0);

		dev_info(dev, "%s: %d, %d\n", __func__, priv->weakness_hbm_comp, value);
	}

	return size;
}
#endif

static DEVICE_ATTR(weakness_hbm_comp, 0664, weakness_hbm_show, weakness_hbm_store);
#endif

static DEVICE_ATTR(lcd_type, 0444, lcd_type_show, NULL);
static DEVICE_ATTR(window_type, 0444, window_type_show, NULL);
static DEVICE_ATTR(manufacture_code, 0444, manufacture_code_show, NULL);
static DEVICE_ATTR(cell_id, 0444, cell_id_show, NULL);
static DEVICE_ATTR(brightness_table, 0444, brightness_table_show, NULL);
static DEVICE_ATTR(auto_brightness, 0644, auto_brightness_show, auto_brightness_store);
static DEVICE_ATTR(siop_enable, 0664, siop_enable_show, siop_enable_store);
static DEVICE_ATTR(temperature, 0664, temperature_show, temperature_store);
static DEVICE_ATTR(color_coordinate, 0444, color_coordinate_show, NULL);
static DEVICE_ATTR(manufacture_date, 0444, manufacture_date_show, NULL);
static DEVICE_ATTR(read_mtp, 0664, read_mtp_show, read_mtp_store);

static DEVICE_ATTR(auto_brightness_level, 0444, auto_brightness_level_show, NULL);
static DEVICE_ATTR(brightness_step, 0444, brightness_step_show, NULL);



#ifdef CONFIG_PANEL_AID_DIMMING
static DEVICE_ATTR(aid_log, 0444, aid_log_show, NULL);
#endif

void lcd_init_sysfs(struct dsim_device *dsim)
{
	int ret = -1;

	ret = device_create_file(&dsim->lcd->dev, &dev_attr_lcd_type);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->lcd->dev, &dev_attr_window_type);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->lcd->dev, &dev_attr_manufacture_code);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->lcd->dev, &dev_attr_cell_id);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->lcd->dev, &dev_attr_brightness_table);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->priv.bd->dev, &dev_attr_auto_brightness);
	if (ret < 0)
		dev_err(&dsim->priv.bd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->priv.bd->dev, &dev_attr_auto_brightness_level);
	if (ret < 0)
		dev_err(&dsim->priv.bd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->priv.bd->dev, &dev_attr_brightness_step);
	if (ret < 0)
		dev_err(&dsim->priv.bd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->lcd->dev, &dev_attr_siop_enable);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->lcd->dev, &dev_attr_temperature);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->lcd->dev, &dev_attr_color_coordinate);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&dsim->lcd->dev, &dev_attr_manufacture_date);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

#ifdef CONFIG_PANEL_AID_DIMMING
	ret = device_create_file(&dsim->lcd->dev, &dev_attr_aid_log);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);
#endif
	ret = device_create_file(&dsim->lcd->dev, &dev_attr_read_mtp);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);

#ifdef CONFIG_LCD_BURNIN_CORRECTION
	ret = device_create_file(&dsim->lcd->dev, &dev_attr_ldu_correction);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);
#endif
#if defined(CONFIG_SEC_FACTORY) && defined(CONFIG_EXYNOS_DECON_LCD_MCD)
	ret = device_create_file(&dsim->lcd->dev, &dev_attr_mcd_mode);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);
#endif

#ifdef CONFIG_LCD_HMT
#ifdef CONFIG_PANEL_AID_DIMMING
	ret = device_create_file(&dsim->lcd->dev, &dev_attr_hmt_bright);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);
#endif
	ret = device_create_file(&dsim->lcd->dev, &dev_attr_hmt_on);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);
#endif

#ifdef CONFIG_LCD_ALPM
	ret = device_create_file(&dsim->lcd->dev, &dev_attr_alpm);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);
#endif

#ifdef CONFIG_LCD_HBM_INTERPOLATION
	ret = device_create_file(&dsim->priv.bd->dev ,&dev_attr_weakness_hbm_comp);
	if (ret < 0)
		dev_err(&dsim->lcd->dev, "failed to add sysfs entries, %d\n", __LINE__);
#endif
}



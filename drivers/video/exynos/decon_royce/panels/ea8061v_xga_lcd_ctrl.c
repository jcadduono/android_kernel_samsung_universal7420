/*
 * drivers/video/decon/panels/S6E3HA0_lcd_ctrl.c
 *
 * Samsung SoC MIPI LCD CONTROL functions
 *
 * Copyright (c) 2014 Samsung Electronics
 *
 * Jiun Yu, <jiun.yu@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifdef CONFIG_PANEL_AID_DIMMING
#include "aid_dimming.h"
#include "dimming_core.h"
#include "ea8061v_xga_aid_dimming.h"
#endif
#include "ea8061v_xga_param.h"
#include "../dsim.h"
#include <video/mipi_display.h>



#ifdef CONFIG_PANEL_AID_DIMMING
static unsigned char EA8061V_SEQ_HBM_OFF[] = {
	0xB6,
	0x00 /* write C8h 40th para to B6 4th para*/
};

static unsigned char EA8061V_SEQ_HBM_ON[] = {
	0xB6,
	0x00 /* write B6h 4th default to B6 4th para*/
};

static unsigned char *HBM_TABLE[HBM_STATUS_MAX] = {EA8061V_SEQ_HBM_OFF, EA8061V_SEQ_HBM_ON};
static const unsigned char *ACL_CUTOFF_TABLE[ACL_STATUS_MAX] = {EA8061V_SEQ_ACL_OFF, EA8061V_SEQ_ACL_15};
static const unsigned char *ACL_OPR_TABLE[ACL_OPR_MAX] = {EA8061V_SEQ_ACL_OFF_OPR, EA8061V_SEQ_ACL_ON_OPR};

static const unsigned int br_tbl [256] = {
	2, 2, 2, 3,	4, 5, 6, 7,	8,	9,	10,	11,	12,	13,	14,	15,		// 16
	16,	17,	18,	19,	20,	21,	22,	23,	25,	27,	29,	31,	33,	36,   	// 14
	39,	41,	41,	44,	44,	47,	47,	50,	50,	53,	53,	56,	56,	56,		// 14
	60,	60,	60,	64,	64,	64,	68,	68,	68,	72,	72,	72,	72,	77,		// 14
	77,	77,	82,	82,	82,	82,	87,	87,	87,	87,	93,	93,	93,	93,		// 14
	98,	98,	98,	98,	98,	105, 105, 105, 105,	111, 111, 111,		// 12
	111, 111, 111, 119, 119, 119, 119, 119, 126, 126, 126,		// 11
	126, 126, 126, 134, 134, 134, 134, 134,	134, 134, 143,
	143, 143, 143, 143, 143, 152, 152, 152, 152, 152, 152,
	152, 152, 162, 162,	162, 162, 162, 162,	162, 172, 172,
	172, 172, 172, 172,	172, 172, 183, 183, 183, 183, 183,
	183, 183, 183, 183, 195, 195, 195, 195, 195, 195, 195,
	195, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207,
	220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 234,
	234, 234, 234, 234,	234, 234, 234, 234,	234, 234, 249,
	249, 249, 249, 249, 249, 249, 249, 249, 249, 249, 249,
	265, 265, 265, 265, 265, 265, 265, 265, 265, 265, 265,
	265, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282,
	282, 282, 282, 300, 300, 300, 300, 300,	300, 300, 300,
	300, 300, 300, 300, 316, 316, 316, 316, 316, 316, 316,
	316, 316, 316, 316, 316, 333, 333, 333, 333, 333, 333,
	333, 333, 333, 333, 333, 333, 360							//7
};


static const unsigned int hbm_interpolation_br_tbl[256] = {
	2,		2,		2,		4,		7,		9,		11,		14,		16,		19,		21,		23,		26,		28,		30,		33,
	35,		37,		40,		42,		45,		47,		49,		52,		54,		56,		59,		61,		63,		66,		68,		71,
	73,		75,		78,		80,		82,		85,		87,		89,		92,		94,		97,		99,		101,	104,	106,	108,
	111,	113,	115,	118,	120,	123,	125,	127,	130,	132,	134,	137,	139,	141,	144,	146,
	149,	151,	153,	156,	158,	160,	163,	165,	167,	170,	172,	175,	177,	179,	182,	184,
	186,	189,	191,	193,	196,	198,	201,	203,	205,	208,	210,	212,	215,	217,	219,	222,
	224,	227,	229,	231,	234,	236,	238,	241,	243,	245,	248,	250,	253,	255,	257,	260,
	262,	264,	267,	269,	271,	274,	276,	279,	281,	283,	286,	288,	290,	293,	295,	297,
	300,	302,	305,	307,	309,	312,	314,	316,	319,	321,	323,	326,	328,	331,	333,	335,
	338,	340,	342,	345,	347,	349,	352,	354,	357,	359,	361,	364,	366,	368,	371,	373,
	375,	378,	380,	383,	385,	387,	390,	392,	394,	397,	399,	401,	404,	406,	409,	411,
	413,	416,	418,	420,	423,	425,	427,	430,	432,	435,	437,	439,	442,	444,	446,	449,
	451,	453,	456,	458,	461,	463,	465,	468,	470,	472,	475,	477,	479,	482,	484,	487,
	489,	491,	494,	496,	498,	501,	503,	505,	508,	510,	513,	515,	517,	520,	522,	524,
	527,	529,	531,	534,	536,	539,	541,	543,	546,	548,	550,	553,	555,	557,	560,	562,
	565,	567,	569,	572,	574,	576,	579,	581,	583,	586,	588,	591,	593,	595,	598,	600
};

static const unsigned gallery_br_tbl[256] = {
	 2,  2,  2,  3,  5,  6,  8,  10,  11,  13,  15,  16,  18,  20,  21,  23,
	 25,  26,  28,  29,  32,  33,  34,  36,  38,  39,  41,  43,  44,  46,  48,  50,
	 51,  53,  55,  56,  57,  60,  61,  62,  64,  66,  68,  69,  71,  73,  74,  76,
	 78,  79,  81,  83,  84,  86,  88,  89,  91,  92,  94,  96,  97,  99,  101, 102,
	 104, 106, 107, 109, 111, 112, 114, 116, 117, 119, 120, 123, 124, 125, 127, 129,
	 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 147, 148, 151, 152, 153, 155,
	 157, 159, 160, 162, 164, 165, 167, 169, 170, 172, 174, 175, 177, 179, 180, 182,
	 183, 185, 187, 188, 190, 192, 193, 195, 197, 198, 200, 202, 203, 205, 207, 208,
	 210, 211, 214, 215, 216, 218, 220, 221, 223, 225, 226, 228, 230, 232, 233, 235,
	 237, 238, 239, 242, 243, 244, 246, 248, 250, 251, 253, 255, 256, 258, 260, 261,
	 263, 265, 266, 268, 270, 271, 273, 274, 276, 278, 279, 281, 283, 284, 286, 288,
	 289, 291, 293, 294, 296, 298, 299, 301, 302, 305, 306, 307, 309, 311, 312, 314,
	 316, 317, 319, 321, 323, 324, 326, 328, 329, 330, 333, 334, 335, 337, 339, 341,
	 342, 344, 346, 347, 349, 351, 352, 354, 356, 357, 359, 361, 362, 364, 365, 367,
	 369, 370, 372, 374, 375, 377, 379, 380, 382, 384, 385, 387, 389, 390, 392, 393,
	 396, 397, 398, 400, 402, 403, 405, 407, 408, 410, 412, 414, 415, 417, 419, 430,
};


static const short center_gamma[NUM_VREF][CI_MAX] = {
	{0x000, 0x000, 0x000},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x100, 0x100, 0x100},
};


static signed char aid9562[5] = {0xB2, 0x00, 0x06, 0x04, 0xDD};	/* 0 samsung_brightness_aor 95.62% */
static signed char aid9470[5] = {0xB2, 0x00, 0x06, 0x04, 0xD1};	/* 1 samsung_brightness_aor 94.7% */
static signed char aid9393[5] = {0xB2, 0x00, 0x06, 0x04, 0xC7};	/* 2 samsung_brightness_aor 93.93% */
static signed char aid9293[5] = {0xB2, 0x00, 0x06, 0x04, 0xBA};	/* 3 samsung_brightness_aor 92.93% */
static signed char aid9209[5] = {0xB2, 0x00, 0x06, 0x04, 0xAF};	/* 4 samsung_brightness_aor 92.09% */
static signed char aid9117[5] = {0xB2, 0x00, 0x06, 0x04, 0xA3};	/* 5 samsung_brightness_aor 91.17% */
static signed char aid9017[5] = {0xB2, 0x00, 0x06, 0x04, 0x96};	/* 6 samsung_brightness_aor 90.17% */
static signed char aid8932[5] = {0xB2, 0x00, 0x06, 0x04, 0x8B};	/* 7 samsung_brightness_aor 89.32% */
static signed char aid8840[5] = {0xB2, 0x00, 0x06, 0x04, 0x7F};	/* 8 samsung_brightness_aor 88.4% */
static signed char aid8748[5] = {0xB2, 0x00, 0x06, 0x04, 0x73};	/* 9 samsung_brightness_aor 87.48% */
static signed char aid8648[5] = {0xB2, 0x00, 0x06, 0x04, 0x66};	/* 10 samsung_brightness_aor 86.48% */
static signed char aid8556[5] = {0xB2, 0x00, 0x06, 0x04, 0x5A};	/* 11 samsung_brightness_aor 85.56% */
static signed char aid8479[5] = {0xB2, 0x00, 0x06, 0x04, 0x50};	/* 12 samsung_brightness_aor 84.79% */
static signed char aid8295[5] = {0xB2, 0x00, 0x06, 0x04, 0x38};	/* 13 samsung_brightness_aor 82.95% */
static signed char aid8210[5] = {0xB2, 0x00, 0x06, 0x04, 0x2D};	/* 14 samsung_brightness_aor 82.1% */
static signed char aid8103[5] = {0xB2, 0x00, 0x06, 0x04, 0x1F};	/* 15 samsung_brightness_aor 81.03% */
static signed char aid8018[5] = {0xB2, 0x00, 0x06, 0x04, 0x14};	/* 16 samsung_brightness_aor 80.18% */
static signed char aid7834[5] = {0xB2, 0x00, 0x06, 0x03, 0xFC};	/* 17 samsung_brightness_aor 78.34% */
static signed char aid7727[5] = {0xB2, 0x00, 0x06, 0x03, 0xEE};	/* 18 samsung_brightness_aor 77.27% */
static signed char aid7565[5] = {0xB2, 0x00, 0x06, 0x03, 0xD9};	/* 19 samsung_brightness_aor 75.65% */
static signed char aid7358[5] = {0xB2, 0x00, 0x06, 0x03, 0xBE};	/* 20 samsung_brightness_aor 73.58% */
static signed char aid7266[5] = {0xB2, 0x00, 0x06, 0x03, 0xB2};	/* 21 samsung_brightness_aor 72.66% */
static signed char aid7081[5] = {0xB2, 0x00, 0x06, 0x03, 0x9A};	/* 22 samsung_brightness_aor 70.81% */
static signed char aid6897[5] = {0xB2, 0x00, 0x06, 0x03, 0x82};	/* 23 samsung_brightness_aor 68.97% */
static signed char aid6628[5] = {0xB2, 0x00, 0x06, 0x03, 0x5F};	/* 24 samsung_brightness_aor 66.28% */
static signed char aid6436[5] = {0xB2, 0x00, 0x06, 0x03, 0x46};	/* 25 samsung_brightness_aor 64.36% */
static signed char aid6237[5] = {0xB2, 0x00, 0x06, 0x03, 0x2C};	/* 26 samsung_brightness_aor 62.37% */
static signed char aid5968[5] = {0xB2, 0x00, 0x06, 0x03, 0x09};	/* 27 samsung_brightness_aor 59.68% */
static signed char aid5676[5] = {0xB2, 0x00, 0x06, 0x02, 0xE3};	/* 28 samsung_brightness_aor 56.76% */
static signed char aid5392[5] = {0xB2, 0x00, 0x06, 0x02, 0xBE};	/* 29 samsung_brightness_aor 53.92% */
static signed char aid5108[5] = {0xB2, 0x00, 0x06, 0x02, 0x99};	/* 30 samsung_brightness_aor 51.08% */
static signed char aid4831[5] = {0xB2, 0x00, 0x06, 0x02, 0x75};	/* 31 samsung_brightness_aor 48.31% */
static signed char aid4447[5] = {0xB2, 0x00, 0x06, 0x02, 0x43};	/* 32 samsung_brightness_aor 44.47% */
static signed char aid4048[5] = {0xB2, 0x00, 0x06, 0x02, 0x0F};	/* 33 samsung_brightness_aor 40.48% */
static signed char aid3671[5] = {0xB2, 0x00, 0x06, 0x01, 0xDE};	/* 34 samsung_brightness_aor 36.71% */
/*static signed char aid3671[5] = {0xB2, 0x00, 0x06, 0x01, 0xDE};	 35 samsung_brightness_aor 36.71% */
static signed char aid3257[5] = {0xB2, 0x00, 0x06, 0x01, 0xA8};	/* 36 samsung_brightness_aor 32.57% */
static signed char aid2811[5] = {0xB2, 0x00, 0x06, 0x01, 0x6E};	/* 37 samsung_brightness_aor 28.11% */
static signed char aid2296[5] = {0xB2, 0x00, 0x06, 0x01, 0x2B};	/* 38 samsung_brightness_aor 22.96% */
static signed char aid1782[5] = {0xB2, 0x00, 0x06, 0x00, 0xE8};	/* 39 samsung_brightness_aor 17.82% */
static signed char aid1206[5] = {0xB2, 0x00, 0x06, 0x00, 0x9D};	/* 40 samsung_brightness_aor 12.06% */
static signed char aid6450[5] = {0xB2, 0x00, 0x06, 0x00, 0x54};	/* 41 samsung_brightness_aor 6.45% */
static signed char aid0840[5] = {0xB2, 0x00, 0x06, 0x00, 0x0B};	/* 42 samsung_brightness_aor 0.84% */



// aid sheet opmanual
struct SmtDimInfo dimming_info_RG[MAX_BR_INFO] = {
	{.br = 5, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_5nit, .cTbl = EA_ctbl_5nit, .aid = aid9562, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 6, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_6nit, .cTbl = EA_ctbl_6nit, .aid = aid9470, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 7, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_7nit, .cTbl = EA_ctbl_7nit, .aid = aid9393, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 8, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_8nit, .cTbl = EA_ctbl_8nit, .aid = aid9293, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 9, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_9nit, .cTbl = EA_ctbl_9nit, .aid = aid9209, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 10, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_10nit, .cTbl = EA_ctbl_10nit, .aid = aid9117, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 11, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_11nit, .cTbl = EA_ctbl_11nit, .aid = aid9017, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 12, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_12nit, .cTbl = EA_ctbl_12nit, .aid = aid8932, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 13, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_13nit, .cTbl = EA_ctbl_13nit, .aid = aid8840, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 14, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_14nit, .cTbl = EA_ctbl_14nit, .aid = aid8748, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 15, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_15nit, .cTbl = EA_ctbl_15nit, .aid = aid8648, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 16, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_16nit, .cTbl = EA_ctbl_16nit, .aid = aid8556, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 17, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_17nit, .cTbl = EA_ctbl_17nit, .aid = aid8479, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 19, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_19nit, .cTbl = EA_ctbl_19nit, .aid = aid8295, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 20, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_20nit, .cTbl = EA_ctbl_20nit, .aid = aid8210, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 21, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_21nit, .cTbl = EA_ctbl_21nit, .aid = aid8103, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 22, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_22nit, .cTbl = EA_ctbl_22nit, .aid = aid8018, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 24, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_24nit, .cTbl = EA_ctbl_24nit, .aid = aid7834, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 25, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_25nit, .cTbl = EA_ctbl_25nit, .aid = aid7727, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 27, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_27nit, .cTbl = EA_ctbl_27nit, .aid = aid7565, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 29, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_29nit, .cTbl = EA_ctbl_29nit, .aid = aid7358, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 30, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_30nit, .cTbl = EA_ctbl_30nit, .aid = aid7266, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 32, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_32nit, .cTbl = EA_ctbl_32nit, .aid = aid7081, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 34, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_34nit, .cTbl = EA_ctbl_34nit, .aid = aid6897, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 37, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_37nit, .cTbl = EA_ctbl_37nit, .aid = aid6628, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 39, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_39nit, .cTbl = EA_ctbl_39nit, .aid = aid6436, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 41, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_41nit, .cTbl = EA_ctbl_41nit, .aid = aid6237, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 44, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_44nit, .cTbl = EA_ctbl_44nit, .aid = aid5968, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 47, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_47nit, .cTbl = EA_ctbl_47nit, .aid = aid5676, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 50, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_50nit, .cTbl = EA_ctbl_50nit, .aid = aid5392, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 53, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_53nit, .cTbl = EA_ctbl_53nit, .aid = aid5108, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 56, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_56nit, .cTbl = EA_ctbl_56nit, .aid = aid4831, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 60, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_60nit, .cTbl = EA_ctbl_60nit, .aid = aid4447, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 64, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_64nit, .cTbl = EA_ctbl_64nit, .aid = aid4048, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 68, .refBr = 113, .cGma = gma2p15, .rTbl = EA_rtbl_68nit, .cTbl = EA_ctbl_68nit, .aid = aid3671, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 72, .refBr = 118, .cGma = gma2p15, .rTbl = EA_rtbl_72nit, .cTbl = EA_ctbl_72nit, .aid = aid3671, .elvCaps = elv_caps_72, .elv = elv_72, .way = W1},
	{.br = 77, .refBr = 127, .cGma = gma2p15, .rTbl = EA_rtbl_77nit, .cTbl = EA_ctbl_77nit, .aid = aid3671, .elvCaps = elv_caps_77, .elv = elv_77, .way = W1},
	{.br = 82, .refBr = 135, .cGma = gma2p15, .rTbl = EA_rtbl_82nit, .cTbl = EA_ctbl_82nit, .aid = aid3671, .elvCaps = elv_caps_82, .elv = elv_82, .way = W1},
	{.br = 87, .refBr = 142, .cGma = gma2p15, .rTbl = EA_rtbl_87nit, .cTbl = EA_ctbl_87nit, .aid = aid3671, .elvCaps = elv_caps_88, .elv = elv_88, .way = W1},
	{.br = 93, .refBr = 153, .cGma = gma2p15, .rTbl = EA_rtbl_93nit, .cTbl = EA_ctbl_93nit, .aid = aid3671, .elvCaps = elv_caps_92, .elv = elv_92, .way = W1},
	{.br = 98, .refBr = 162, .cGma = gma2p15, .rTbl = EA_rtbl_98nit, .cTbl = EA_ctbl_98nit, .aid = aid3671, .elvCaps = elv_caps_98, .elv = elv_98, .way = W1},
	{.br = 105, .refBr = 171, .cGma = gma2p15, .rTbl = EA_rtbl_105nit, .cTbl = EA_ctbl_105nit, .aid = aid3671, .elvCaps = elv_caps_105, .elv = elv_105, .way = W1},
	{.br = 111, .refBr = 179, .cGma = gma2p15, .rTbl = EA_rtbl_111nit, .cTbl = EA_ctbl_111nit, .aid = aid3671, .elvCaps = elv_caps_111, .elv = elv_111, .way = W1},
	{.br = 119, .refBr = 192, .cGma = gma2p15, .rTbl = EA_rtbl_119nit, .cTbl = EA_ctbl_119nit, .aid = aid3671, .elvCaps = elv_caps_119, .elv = elv_119, .way = W1},
	{.br = 126, .refBr = 201, .cGma = gma2p15, .rTbl = EA_rtbl_126nit, .cTbl = EA_ctbl_126nit, .aid = aid3671, .elvCaps = elv_caps_126, .elv = elv_126, .way = W1},
	{.br = 134, .refBr = 213, .cGma = gma2p15, .rTbl = EA_rtbl_134nit, .cTbl = EA_ctbl_134nit, .aid = aid3671, .elvCaps = elv_caps_134, .elv = elv_134, .way = W1},
	{.br = 143, .refBr = 227, .cGma = gma2p15, .rTbl = EA_rtbl_143nit, .cTbl = EA_ctbl_143nit, .aid = aid3671, .elvCaps = elv_caps_143, .elv = elv_143, .way = W1},
	{.br = 152, .refBr = 241, .cGma = gma2p15, .rTbl = EA_rtbl_152nit, .cTbl = EA_ctbl_152nit, .aid = aid3671, .elvCaps = elv_caps_152, .elv = elv_152, .way = W1},
	{.br = 162, .refBr = 255, .cGma = gma2p15, .rTbl = EA_rtbl_162nit, .cTbl = EA_ctbl_162nit, .aid = aid3671, .elvCaps = elv_caps_162, .elv = elv_162, .way = W1},
	{.br = 172, .refBr = 255, .cGma = gma2p15, .rTbl = EA_rtbl_172nit, .cTbl = EA_ctbl_172nit, .aid = aid3257, .elvCaps = elv_caps_172, .elv = elv_172, .way = W1},
	{.br = 183, .refBr = 255, .cGma = gma2p15, .rTbl = EA_rtbl_183nit, .cTbl = EA_ctbl_183nit, .aid = aid2811, .elvCaps = elv_caps_183, .elv = elv_183, .way = W1},
	{.br = 195, .refBr = 255, .cGma = gma2p15, .rTbl = EA_rtbl_195nit, .cTbl = EA_ctbl_195nit, .aid = aid2296, .elvCaps = elv_caps_195, .elv = elv_195, .way = W1},
	{.br = 207, .refBr = 255, .cGma = gma2p15, .rTbl = EA_rtbl_207nit, .cTbl = EA_ctbl_207nit, .aid = aid1782, .elvCaps = elv_caps_207, .elv = elv_207, .way = W1},
	{.br = 220, .refBr = 255, .cGma = gma2p15, .rTbl = EA_rtbl_220nit, .cTbl = EA_ctbl_220nit, .aid = aid1206, .elvCaps = elv_caps_220, .elv = elv_220, .way = W1},
	{.br = 234, .refBr = 255, .cGma = gma2p15, .rTbl = EA_rtbl_234nit, .cTbl = EA_ctbl_234nit, .aid = aid6450, .elvCaps = elv_caps_234, .elv = elv_234, .way = W1},
	{.br = 249, .refBr = 255, .cGma = gma2p15, .rTbl = EA_rtbl_249nit, .cTbl = EA_ctbl_249nit, .aid = aid0840, .elvCaps = elv_caps_249, .elv = elv_249, .way = W1},		// 249 ~ 360 acl off
	{.br = 265, .refBr = 272, .cGma = gma2p15, .rTbl = EA_rtbl_265nit, .cTbl = EA_ctbl_265nit, .aid = aid0840, .elvCaps = elv_caps_265, .elv = elv_265, .way = W1},
	{.br = 282, .refBr = 289, .cGma = gma2p15, .rTbl = EA_rtbl_282nit, .cTbl = EA_ctbl_282nit, .aid = aid0840, .elvCaps = elv_caps_282, .elv = elv_282, .way = W1},
	{.br = 300, .refBr = 306, .cGma = gma2p15, .rTbl = EA_rtbl_300nit, .cTbl = EA_ctbl_300nit, .aid = aid0840, .elvCaps = elv_caps_300, .elv = elv_300, .way = W1},
	{.br = 316, .refBr = 320, .cGma = gma2p15, .rTbl = EA_rtbl_316nit, .cTbl = EA_ctbl_316nit, .aid = aid0840, .elvCaps = elv_caps_316, .elv = elv_316, .way = W1},
	{.br = 333, .refBr = 340, .cGma = gma2p15, .rTbl = EA_rtbl_333nit, .cTbl = EA_ctbl_333nit, .aid = aid0840, .elvCaps = elv_caps_333, .elv = elv_333, .way = W1},
	{.br = 360, .refBr = 360, .cGma = gma2p20, .rTbl = EA_rtbl_360nit, .cTbl = EA_ctbl_360nit, .aid = aid0840, .elvCaps = elv_caps_360, .elv = elv_360, .way = W2},
#if 0
/*hbm interpolation , lihh TBD*/
	{.br = 382, .refBr = 382, .cGma = gma2p20, .rTbl = EA_rtbl_360nit, .cTbl = EA_ctbl_360nit, .aid = aid0840, .elvCaps = elv_caps_360, .elv = elv_360, .way = W3},  // hbm is acl on
	{.br = 407, .refBr = 407, .cGma = gma2p20, .rTbl = EA_rtbl_360nit, .cTbl = EA_ctbl_360nit, .aid = aid0840, .elvCaps = elv_caps_360, .elv = elv_360, .way = W3},  // hbm is acl on
	{.br = 433, .refBr = 433, .cGma = gma2p20, .rTbl = EA_rtbl_360nit, .cTbl = EA_ctbl_360nit, .aid = aid0840, .elvCaps = elv_caps_360, .elv = elv_360, .way = W3},  // hbm is acl on
	{.br = 461, .refBr = 461, .cGma = gma2p20, .rTbl = EA_rtbl_360nit, .cTbl = EA_ctbl_360nit, .aid = aid0840, .elvCaps = elv_caps_360, .elv = elv_360, .way = W3},  // hbm is acl on
	{.br = 491, .refBr = 491, .cGma = gma2p20, .rTbl = EA_rtbl_360nit, .cTbl = EA_ctbl_360nit, .aid = aid0840, .elvCaps = elv_caps_360, .elv = elv_360, .way = W3},  // hbm is acl on
	{.br = 517, .refBr = 517, .cGma = gma2p20, .rTbl = EA_rtbl_360nit, .cTbl = EA_ctbl_360nit, .aid = aid0840, .elvCaps = elv_caps_360, .elv = elv_360, .way = W3},  // hbm is acl on
	{.br = 545, .refBr = 545, .cGma = gma2p20, .rTbl = EA_rtbl_360nit, .cTbl = EA_ctbl_360nit, .aid = aid0840, .elvCaps = elv_caps_360, .elv = elv_360, .way = W3},  // hbm is acl on
/* hbm */
	{.br = 600, .refBr = 600, .cGma = gma2p20, .rTbl = EA_rtbl_360nit, .cTbl = EA_ctbl_360nit, .aid = aid0840, .elvCaps = elv_caps_360, .elv = elv_360, .way = W4}, // hbm is acl on
#endif
};



static int set_gamma_to_center(struct SmtDimInfo *brInfo)
{
	int i, j;
	int ret = 0;
	unsigned int index = 0;
	unsigned char *result = brInfo->gamma;

	result[index++] = OLED_CMD_GAMMA;

	for (i = V255; i >= V0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			if (i == V255) {
				result[index++] = (unsigned char)((center_gamma[i][j] >> 8) & 0x01);
				result[index++] = (unsigned char)center_gamma[i][j] & 0xff;
			} else {
				result[index++] = (unsigned char)center_gamma[i][j] & 0xff;
			}
		}
	}
	result[index++] = 0x00;
	result[index++] = 0x00;

	return ret;
}

static int gammaToVolt255(int gamma)
{
	int ret;

	if (gamma > vreg_element_max[V255]) {
		dsim_err("%s : gamma overflow : %d\n", __FUNCTION__, gamma);
		gamma = vreg_element_max[V255];
	}
	if (gamma < 0) {
		dsim_err("%s : gamma undeflow : %d\n", __FUNCTION__, gamma);
		gamma = 0;
	}

	ret = (int)v255_trans_volt[gamma];

	return ret;
}

static int voltToGamma(int hbm_volt_table[][3], int* vt, int tp, int color)
{
	int ret;
	int t1, t2;
	unsigned long temp;

	if(tp == V3)
	{
		t1 = DOUBLE_MULTIPLE_VREGOUT - hbm_volt_table[V3][color];
		t2 = DOUBLE_MULTIPLE_VREGOUT - hbm_volt_table[V11][color];
	}
	else
	{
		t1 = vt[color] - hbm_volt_table[tp][color];
		t2 = vt[color] - hbm_volt_table[tp + 1][color];
	}

	temp = ((unsigned long)t1 * (unsigned long)fix_const[tp].de) / (unsigned long)t2;
	ret = temp - fix_const[tp].nu;

	return ret;

}

static int set_gamma_to_hbm(struct SmtDimInfo *brInfo, struct dim_data *dimData, u8 *hbm)
{
	int ret = 0;
	unsigned int index = 0;
	unsigned char *result = brInfo->gamma;
	int i, j, idx;
	int temp = 0;
	int voltTableHbm[NUM_VREF][CI_MAX];

	memset(result, 0, OLED_CMD_GAMMA_CNT);

	result[index++] = OLED_CMD_GAMMA;

	memcpy(result+1, hbm, EA8061V_HBMGAMMA_LEN);


	memcpy(voltTableHbm[V0], dimData->volt[0], sizeof(voltTableHbm[V0]));
	memcpy(voltTableHbm[V3], dimData->volt[1], sizeof(voltTableHbm[V3]));
	memcpy(voltTableHbm[V11], dimData->volt[10], sizeof(voltTableHbm[V11]));
	memcpy(voltTableHbm[V23], dimData->volt[26], sizeof(voltTableHbm[V23]));
	memcpy(voltTableHbm[V35], dimData->volt[40], sizeof(voltTableHbm[V35]));
	memcpy(voltTableHbm[V51], dimData->volt[56], sizeof(voltTableHbm[V51]));
	memcpy(voltTableHbm[V87], dimData->volt[100], sizeof(voltTableHbm[V87]));
	memcpy(voltTableHbm[V151], dimData->volt[173], sizeof(voltTableHbm[V151]));
	memcpy(voltTableHbm[V203], dimData->volt[233], sizeof(voltTableHbm[V203]));


	idx = 0;
	for(i = CI_RED; i < CI_MAX; i++, idx += 2) {
		temp = (hbm[idx] << 8) | (hbm[idx + 1]);
		voltTableHbm[V255][i] = gammaToVolt255(temp + dimData->mtp[V255][i]);
	}

	idx = 1;
	for (i = V255; i >= V0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			if (i == V255) {
				idx += 2;
			} else if(i == V0) {
				idx++;
			} else {
				temp = voltToGamma(voltTableHbm, dimData->volt_vt, i, j) - dimData->mtp[i][j];
				if(temp <= 0)
					temp = 0;
				result[idx] = temp;
				idx ++;
			}
		}
	}

	dsim_info("============ TUNE HBM GAMMA ========== : \n");
	for (i= 0; i < EA8061V_HBMGAMMA_LEN; i ++) {
		dsim_info("HBM GAMMA[%d] : %x\n", i, result[i]);
	}
	return ret;
}

/* gamma interpolaion table */
const unsigned int tbl_hbm_inter[7] = {
	94, 201, 311, 431, 559, 670, 789
};

static int interpolation_gamma_to_hbm(struct SmtDimInfo *dimInfo, int br_idx)
{
	int i, j;
	int ret = 0;
	int idx = 0;
	int tmp = 0;
	int hbmcnt, refcnt, gap = 0;
	int ref_idx = 0;
	int hbm_idx = 0;
	int rst = 0;
	int hbm_tmp, ref_tmp;
	unsigned char *result = dimInfo[br_idx].gamma;

	for (i = 0; i < MAX_BR_INFO; i++) {
		if (dimInfo[i].br == EA8061V_MAX_BRIGHTNESS)
			ref_idx = i;

		if (dimInfo[i].br == EA8061V_HBM_BRIGHTNESS)
			hbm_idx = i;
	}

	if ((ref_idx == 0) || (hbm_idx == 0)) {
		dsim_info("%s failed to get index ref index : %d, hbm index : %d\n",
					__func__, ref_idx, hbm_idx);
		ret = -EINVAL;
		goto exit;
	}

	result[idx++] = OLED_CMD_GAMMA;
	tmp = (br_idx - ref_idx) - 1;

	hbmcnt = 1;
	refcnt = 1;

	for (i = V255; i >= V0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			if (i == V255) {
				hbm_tmp = (dimInfo[hbm_idx].gamma[hbmcnt] << 8) | (dimInfo[hbm_idx].gamma[hbmcnt+1]);
				ref_tmp = (dimInfo[ref_idx].gamma[refcnt] << 8) | (dimInfo[ref_idx].gamma[refcnt+1]);

				if (hbm_tmp > ref_tmp) {
					gap = hbm_tmp - ref_tmp;
					rst = (gap * tbl_hbm_inter[tmp]) >> 10;
					rst += ref_tmp;
				}
				else {
					gap = ref_tmp - hbm_tmp;
					rst = (gap * tbl_hbm_inter[tmp]) >> 10;
					rst = ref_tmp - rst;
				}
				result[idx++] = (unsigned char)((rst >> 8) & 0x01);
				result[idx++] = (unsigned char)rst & 0xff;
				hbmcnt += 2;
				refcnt += 2;
			} else {
				hbm_tmp = dimInfo[hbm_idx].gamma[hbmcnt++];
				ref_tmp = dimInfo[ref_idx].gamma[refcnt++];

				if (hbm_tmp > ref_tmp) {
					gap = hbm_tmp - ref_tmp;
					rst = (gap * tbl_hbm_inter[tmp]) >> 10;
					rst += ref_tmp;
				}
				else {
					gap = ref_tmp - hbm_tmp;
					rst = (gap * tbl_hbm_inter[tmp]) >> 10;
					rst = ref_tmp - rst;
				}
				result[idx++] = (unsigned char)rst & 0xff;
			}
		}
	}

	dsim_info("tmp index : %d\n", tmp);

exit:
	return ret;
}
static int init_dimming(struct dsim_device *dsim, u8 *mtp, u8 *hbm)
{
	int i, j;
	int pos = 0;
	int ret = 0;
	short temp;
	int method = 0;
	struct dim_data *dimming;
	unsigned char panelrev = 0x00;
	struct panel_private *panel = &dsim->priv;
	struct SmtDimInfo *diminfo = NULL;
	int string_offset;
	char string_buf[1024];

	dimming = (struct dim_data *)kmalloc(sizeof(struct dim_data), GFP_KERNEL);
	if (!dimming) {
		dsim_err("failed to allocate memory for dim data\n");
		ret = -ENOMEM;
		goto error;
	}

	panelrev = panel->id[2] & 0x0F;
	dsim_info("%s : Panel rev : %d\n", __func__, panelrev);

	dsim_info("%s init dimming info for daisy rev.G panel\n", __func__);
	diminfo = (void *)dimming_info_RG;

	panel->dim_data= (void *)dimming;
	panel->dim_info = (void *)diminfo;
	panel->br_tbl = (unsigned int *)br_tbl;
	panel->hbm_inter_br_tbl = (unsigned int *)hbm_interpolation_br_tbl;
	panel->gallery_br_tbl = (unsigned int*)gallery_br_tbl;
	panel->hbm_tbl = (unsigned char **)HBM_TABLE;
	panel->acl_cutoff_tbl = (unsigned char **)ACL_CUTOFF_TABLE;
	panel->acl_opr_tbl = (unsigned char **)ACL_OPR_TABLE;
	panel->hbm_index = MAX_BR_INFO - 1;
	panel->hbm_elvss_comp = EA8061V_HBM_ELVSS_COMP;

	for (j = 0; j < CI_MAX; j++) {
		temp = ((mtp[pos] & 0x01) ? -1 : 1) * mtp[pos+1];
		dimming->t_gamma[V255][j] = (int)center_gamma[V255][j] + temp;
		dimming->mtp[V255][j] = temp;
		pos += 2;
	}

	for (i = V203; i > V0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			temp = ((mtp[pos] & 0x80) ? -1 : 1) * (mtp[pos] & 0x7f);
			dimming->t_gamma[i][j] = (int)center_gamma[i][j] + temp;
			dimming->mtp[i][j] = temp;
			pos++;
		}
	}
	/* for vt */
	for (j = 0; j < CI_MAX; j++) {
		dimming->t_gamma[V0][j] = (int)center_gamma[V0][j];
		dimming->mtp[V0][j] = 0;
	}

	for (j = 0; j < CI_MAX; j++) {
		dimm_info("vt_mtp %d from mtp pos %d : \n",mtp[pos],pos);
		dimming->vt_mtp[j] = mtp[pos];
		pos++;
	}

#ifdef SMART_DIMMING_DEBUG
	dimm_info("Center Gamma Info : \n");
	for(i=0;i<VMAX;i++) {
		dsim_info("Gamma : %3d %3d %3d : %3x %3x %3x\n",
			dimming->t_gamma[i][CI_RED], dimming->t_gamma[i][CI_GREEN], dimming->t_gamma[i][CI_BLUE],
			dimming->t_gamma[i][CI_RED], dimming->t_gamma[i][CI_GREEN], dimming->t_gamma[i][CI_BLUE] );
	}
#endif
	dimm_info("VT MTP : \n");
	dimm_info("Gamma : %3d %3d %3d : %3x %3x %3x\n",
			dimming->vt_mtp[CI_RED], dimming->vt_mtp[CI_GREEN], dimming->vt_mtp[CI_BLUE],
			dimming->vt_mtp[CI_RED], dimming->vt_mtp[CI_GREEN], dimming->vt_mtp[CI_BLUE] );

	dimm_info("MTP Info : \n");
	for(i=0;i<VMAX;i++) {
		dimm_info("Gamma : %3d %3d %3d : %3x %3x %3x\n",
			dimming->mtp[i][CI_RED], dimming->mtp[i][CI_GREEN], dimming->mtp[i][CI_BLUE],
			dimming->mtp[i][CI_RED], dimming->mtp[i][CI_GREEN], dimming->mtp[i][CI_BLUE] );
	}

	ret = generate_volt_table(dimming);
	if (ret) {
		dimm_err("[ERR:%s] failed to generate volt table\n", __func__);
		goto error;
	}

	for (i = 0; i < MAX_BR_INFO; i++) {
		method = diminfo[i].way;

		if (method == DIMMING_METHOD_FILL_CENTER) {
			ret = set_gamma_to_center(&diminfo[i]);
			if (ret) {
				dsim_err("%s : failed to get center gamma\n", __func__);
				goto error;
			}
		}
		else if (method == DIMMING_METHOD_FILL_HBM) {
			ret = set_gamma_to_hbm(&diminfo[i], dimming, hbm);
			if (ret) {
				dsim_err("%s : failed to get hbm gamma\n", __func__);
				goto error;
			}
		}
	}

	for (i = 0; i < MAX_BR_INFO; i++) {
		method = diminfo[i].way;
		if (method == DIMMING_METHOD_AID) {
			ret = cal_gamma_from_index(dimming, &diminfo[i]);
			if (ret) {
				dsim_err("%s : failed to calculate gamma : index : %d\n", __func__, i);
				goto error;
			}
		}
		if (method == DIMMING_METHOD_INTERPOLATION) {
			ret = interpolation_gamma_to_hbm(diminfo, i);
			if (ret) {
				dsim_err("%s : failed to calculate gamma : index : %d\n", __func__, i);
				goto error;
			}
		}
	}

	for (i = 0; i < MAX_BR_INFO; i++) {
		memset(string_buf, 0, sizeof(string_buf));
		string_offset = sprintf(string_buf, "gamma[%3d] : ",diminfo[i].br);

		for(j = 0; j < GAMMA_CMD_CNT; j++)
			string_offset += sprintf(string_buf + string_offset, "%d ", diminfo[i].gamma[j]);

		dsim_info("%s\n", string_buf);
	}

error:
	return ret;

}
#endif




static int ea8061v_read_init_info(struct dsim_device *dsim, unsigned char *mtp, unsigned char* hbm)
{
	int i = 0;
	int ret = 0;
	struct panel_private *panel = &dsim->priv;
	unsigned char buf[EA8061V_MTP_DATE_SIZE] = {0, };
	unsigned char bufForCoordi[EA8061V_COORDINATE_LEN] = {0,};
	unsigned char hbm_gamma_etc[12] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,0,0,0};
	ret = dsim_write_hl_data(dsim, EA8061V_SEQ_LEVEL2_KEY_UNLOCK_F0, ARRAY_SIZE(EA8061V_SEQ_LEVEL2_KEY_UNLOCK_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_LEVEL2_KEY_UNLOCK_F0\n", __func__);
		panel->lcdConnected = PANEL_DISCONNEDTED;
		goto read_fail;
	}

	ret = dsim_write_hl_data(dsim, EA8061V_SEQ_MTP_KEY_UNLOCK_F1, ARRAY_SIZE(EA8061V_SEQ_MTP_KEY_UNLOCK_F1));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_MTP_KEY_UNLOCK_F1\n", __func__);
		panel->lcdConnected = PANEL_DISCONNEDTED;
		goto read_fail;
	}

	ret = dsim_read_hl_data(dsim, EA8061V_ID_REG, EA8061V_ID_LEN, dsim->priv.id);
	if (ret != EA8061V_ID_LEN) {
		dsim_err("%s : can't find connected panel. check panel connection\n", __func__);
		panel->lcdConnected = PANEL_DISCONNEDTED;
		goto read_fail;
	}

	dsim_info("READ ID : ");
	for(i = 0; i < EA8061V_ID_LEN; i++)
		dsim_info("%02x, ", dsim->priv.id[i]);
	dsim_info("\n");

	ret = dsim_read_hl_data(dsim, EA8061V_MTP_ADDR, EA8061V_MTP_DATE_SIZE, buf);
	if (ret != EA8061V_MTP_DATE_SIZE) {
		dsim_err("failed to read mtp, check panel connection\n");
		goto read_fail;
	}
	memcpy(mtp, buf, EA8061V_MTP_SIZE);
	memcpy(dsim->priv.date, &buf[40], ARRAY_SIZE(dsim->priv.date));
	dsim_info("READ MTP SIZE : %d\n", EA8061V_MTP_SIZE);
	dsim_info("=========== MTP INFO =========== \n");
	for (i = 0; i < EA8061V_MTP_SIZE; i++)
		dsim_info("MTP[%2d] : %2d : %2x\n", i, mtp[i], mtp[i]);

	// coordinate
	ret = dsim_read_hl_data(dsim, EA8061V_COORDINATE_REG, EA8061V_COORDINATE_LEN, bufForCoordi);
	if (ret != EA8061V_COORDINATE_LEN) {
		dsim_err("fail to read coordinate on command.\n");
		goto read_fail;
	}
	dsim->priv.coordinate[0] = bufForCoordi[0] << 8 | bufForCoordi[1];	/* X */
	dsim->priv.coordinate[1] = bufForCoordi[2] << 8 | bufForCoordi[3];	/* Y */
	dsim_info("READ coordi : ");
	for(i = 0; i < 2; i++)
		dsim_info("%d, ", dsim->priv.coordinate[i]);
	dsim_info("\n");


	// tset
	ret = dsim_read_hl_data(dsim, TSET_REG, TSET_LEN - 1, dsim->priv.tset);
	if (ret < TSET_LEN - 1) {
		dsim_err("fail to read tset on command.\n");
		goto read_fail;
	}
	dsim_info("READ tset : ");
	for(i = 0; i < TSET_LEN - 1; i++)
		dsim_info("%x, ", dsim->priv.tset[i]);
	dsim_info("\n");


	// elvss
	ret = dsim_read_hl_data(dsim, ELVSS_REG, ELVSS_LEN - 1, dsim->priv.elvss_set);
	if (ret < ELVSS_LEN - 1) {
		dsim_err("fail to read elvss on command.\n");
		goto read_fail;
	}
	dsim_info("READ elvss : ");
	for(i = 0; i < ELVSS_LEN - 1; i++)
		dsim_info("%x \n", dsim->priv.elvss_set[i]);


	// record HBM ON/OFF para.
	EA8061V_SEQ_HBM_ON[1] = mtp[39];
	EA8061V_SEQ_HBM_OFF[1] = dsim->priv.elvss_set[3];


	// read HBM gamma
	memcpy(hbm, &mtp[33], 6); //CAh 34th to 39 th para.
	memcpy(&hbm[6], &mtp[42], 15); //CAh 34th to 39 th para.
	memcpy(&hbm[21], hbm_gamma_etc, 12);

	dsim_info("HBM Gamma : ");
	for(i = 1; i < EA8061V_HBMGAMMA_LEN; i++)
		dsim_info("hbm gamma[%d] : %x\n", i, hbm[i]);

	return 0;

read_fail:
	return -ENODEV;
}
static int ea8061v_xga_dump(struct dsim_device *dsim)
{
	int ret = 0;
	int i;
	unsigned char id[EA8061V_ID_LEN];
	unsigned char rddpm[4];
	unsigned char rddsm[4];
	unsigned char err_buf[4];

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}

	ret = dsim_read_hl_data(dsim, 0xEA, 3, err_buf);
	if (ret != 3) {
		dsim_err("%s : can't read Panel's EA Reg\n",__func__);
		goto dump_exit;
	}

	dsim_info("=== Panel's 0xEA Reg Value ===\n");
	dsim_info("* 0xEA : buf[0] = %x\n", err_buf[0]);
	dsim_info("* 0xEA : buf[1] = %x\n", err_buf[1]);

	ret = dsim_read_hl_data(dsim, EA8061V_RDDPM_ADDR, 3, rddpm);
	if (ret != 3) {
		dsim_err("%s : can't read RDDPM Reg\n",__func__);
		goto dump_exit;
	}

	dsim_info("=== Panel's RDDPM Reg Value : %x ===\n", rddpm[0]);

	if (rddpm[0] & 0x80)
		dsim_info("* Booster Voltage Status : ON\n");
	else
		dsim_info("* Booster Voltage Status : OFF\n");

	if (rddpm[0] & 0x40)
		dsim_info("* Idle Mode : On\n");
	else
		dsim_info("* Idle Mode : OFF\n");

	if (rddpm[0] & 0x20)
		dsim_info("* Partial Mode : On\n");
	else
		dsim_info("* Partial Mode : OFF\n");

	if (rddpm[0] & 0x10)
		dsim_info("* Sleep OUT and Working Ok\n");
	else
		dsim_info("* Sleep IN\n");

	if (rddpm[0] & 0x08)
		dsim_info("* Normal Mode On and Working Ok\n");
	else
		dsim_info("* Sleep IN\n");

	if (rddpm[0] & 0x04)
		dsim_info("* Display On and Working Ok\n");
	else
		dsim_info("* Display Off\n");

	ret = dsim_read_hl_data(dsim, EA8061V_RDDSM_ADDR, 3, rddsm);
	if (ret != 3) {
		dsim_err("%s : can't read RDDSM Reg\n",__func__);
		goto dump_exit;
	}

	dsim_info("=== Panel's RDDSM Reg Value : %x ===\n", rddsm[0]);

	if (rddsm[0] & 0x80)
		dsim_info("* TE On\n");
	else
		dsim_info("* TE OFF\n");

	if (rddsm[0] & 0x02)
		dsim_info("* S_DSI_ERR : Found\n");

	if (rddsm[0] & 0x01)
		dsim_info("* DSI_ERR : Found\n");

	// id
	ret = dsim_read_hl_data(dsim, EA8061V_ID_REG, EA8061V_ID_LEN, id);
	if (ret != EA8061V_ID_LEN) {
		dsim_err("%s : can't read panel id\n",__func__);
		goto dump_exit;
	}

	dsim_info("READ ID : ");
	for(i = 0; i < EA8061V_ID_LEN; i++)
		dsim_info("%02x, ", id[i]);
	dsim_info("\n");

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_F0\n", __func__);
	}
dump_exit:
	dsim_info(" - %s\n", __func__);
	return ret;

}
static int ea8061v_xga_probe(struct dsim_device *dsim)
{
	int ret = 0;
	struct panel_private *panel = &dsim->priv;
	unsigned char mtp[EA8061V_MTP_SIZE] = {0, };
	unsigned char hbm[EA8061V_HBMGAMMA_LEN] = {0, };
	panel->dim_data = (void *)NULL;
	panel->lcdConnected = PANEL_CONNECTED;
	
	dsim_info(" +  : %s\n", __func__);

	ret = ea8061v_read_init_info(dsim, mtp, hbm);
	if (panel->lcdConnected == PANEL_DISCONNEDTED) {
		dsim_err("dsim : %s lcd was not connected\n", __func__);
		goto probe_exit;
	}

#ifdef CONFIG_PANEL_AID_DIMMING
	ret = init_dimming(dsim, mtp, hbm);
	if (ret) {
		dsim_err("%s : failed to generate gamma tablen\n", __func__);
	}
#endif
#ifdef CONFIG_EXYNOS_DECON_MDNIE_LITE
	panel->mdnie_support = 1;
#endif

probe_exit:
	return ret;

}


static int ea8061v_xga_displayon(struct dsim_device *dsim)
{
	int ret = 0;

	dsim_info("MDD : %s was called\n", __func__);
	
	ret = dsim_write_hl_data(dsim, EA8061V_SEQ_DISPLAY_ON, ARRAY_SIZE(EA8061V_SEQ_DISPLAY_ON));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DISPLAY_ON\n", __func__);
 		goto displayon_err;
	}

displayon_err:
	return ret;

}

static int ea8061v_xga_exit(struct dsim_device *dsim)
{
	int ret = 0;

	dsim_info("MDD : %s was called\n", __func__);

	ret = dsim_write_hl_data(dsim, EA8061V_SEQ_DISPLAY_OFF, ARRAY_SIZE(EA8061V_SEQ_DISPLAY_OFF));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DISPLAY_OFF\n", __func__);
		goto exit_err;
	}

	ret = dsim_write_hl_data(dsim, EA8061V_SEQ_SLEEP_IN, ARRAY_SIZE(EA8061V_SEQ_SLEEP_IN));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SLEEP_IN\n", __func__);
		goto exit_err;
	}

	msleep(120);

exit_err:
	return ret;
}

static int ea8061v_xga_init(struct dsim_device *dsim)
{
	int ret = 0;

	dsim_info("MDD : %s was called\n", __func__);
	
	ret += dsim_write_hl_data(dsim, EA8061V_SEQ_LEVEL2_KEY_UNLOCK_F0, ARRAY_SIZE(EA8061V_SEQ_LEVEL2_KEY_UNLOCK_F0));
	ret += dsim_write_hl_data(dsim, EA8061V_SEQ_MTP_KEY_UNLOCK_F1, ARRAY_SIZE(EA8061V_SEQ_MTP_KEY_UNLOCK_F1));

	/*  Common Setting */
	ret += dsim_write_hl_data(dsim, EA8061V_SEQ_COMMON_B8, ARRAY_SIZE(EA8061V_SEQ_COMMON_B8));
	ret += dsim_write_hl_data(dsim, EA8061V_SEQ_COMMON_BA, ARRAY_SIZE(EA8061V_SEQ_COMMON_BA));

	/* H sync.	It use last packet*/
	ret += dsim_write_hl_data(dsim, EA8061V_SEQ_HSYNC_B9, ARRAY_SIZE(EA8061V_SEQ_HSYNC_B9));

	/* Brightness Control. It use last packet*/
	ret += dsim_write_hl_data(dsim, EA8061V_SEQ_GAMMA_CONDITION_CA, ARRAY_SIZE(EA8061V_SEQ_GAMMA_CONDITION_CA));
	ret += dsim_write_hl_data(dsim, EA8061V_BL_CTRL_B2, ARRAY_SIZE(EA8061V_BL_CTRL_B2));
	ret += dsim_write_hl_data(dsim, EA8061V_BL_CTRL_B6, ARRAY_SIZE(EA8061V_BL_CTRL_B6));
	//ret += dsim_write_hl_data(dsim, EA8061V_BL_CTRL_B5, ARRAY_SIZE(EA8061V_BL_CTRL_B5));
	//ret += dsim_write_hl_data(dsim, EA8061V_SEQ_ACL_OFF_55, ARRAY_SIZE(EA8061V_SEQ_ACL_OFF_55));
	ret += dsim_write_hl_data(dsim, EA8061V_SEQ_GAMMA_UPDATE_F7, ARRAY_SIZE(EA8061V_SEQ_GAMMA_UPDATE_F7));

	/* Sleep Out(11h) */
	ret += dsim_write_hl_data(dsim, EA8061V_SEQ_SLEEP_OUT, ARRAY_SIZE(EA8061V_SEQ_SLEEP_OUT));
	msleep(120);
	
	ret += dsim_write_hl_data(dsim, EA8061V_SEQ_MTP_KEY_LOCK_F1, ARRAY_SIZE(EA8061V_SEQ_MTP_KEY_LOCK_F1));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : init sequence\n", __func__);
		goto init_exit;
	}

init_exit:
	return ret;
}


struct dsim_panel_ops ea8061v_panel_ops = {
	.probe		= ea8061v_xga_probe,
	.displayon	= ea8061v_xga_displayon,
	.exit		= ea8061v_xga_exit,
	.init		= ea8061v_xga_init,
	.dump 		= ea8061v_xga_dump,
};


struct dsim_panel_ops *dsim_panel_get_priv_ops(struct dsim_device *dsim)
{
	return &ea8061v_panel_ops;
}


static int __init get_lcd_type(char *arg)
{
	unsigned int lcdtype;

	get_option(&arg, &lcdtype);

	dsim_info("--- Parse LCD TYPE ---\n");
	dsim_info("LCDTYPE : EA8061V (XGA) : %x\n", lcdtype);

	return 0;
}
early_param("lcdtype", get_lcd_type);


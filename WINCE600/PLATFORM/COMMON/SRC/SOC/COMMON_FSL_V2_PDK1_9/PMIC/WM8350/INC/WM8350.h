/*******************************************************************************
 *
 * Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
 *
 * This software as well as any related documentation may only be used or
 * copied in accordance with the terms of the Wolfson Microelectronics plc
 * agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
 *
 * The information in this file is furnished for informational use only,
 * is subject to change without notice, and should not be construed as a
 * commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
 * assumes no responsibility or liability for any errors or inaccuracies that
 * may appear in the software or any related documention.
 *
 * Except as permitted by the agreement(s), no part of the software or any
 * related documention may be reproduced, stored in a retrieval system, or
 * transmitted in any form or by any means without the express written
 * consent of Wolfson Microelectronics plc.
 *                                                                         *//**
 * @file   WM8350.h
 * @brief  Device-specific definitions for the Wolfson WM8350.
 *
 * @version $Id: WM8350.h 640 2007-06-15 22:01:17Z ib $
 *
 ******************************************************************************/
#ifndef __WM8350_H__
#define __WM8350_H__

#include "WMInterrupts.h"

/*
 * Define this to TRUE if you want to enable Hibernation of the PMIC
 */
#define WM_PMIC_HIBERNATE                TRUE

/**
 * The mask of valid GPIOs.
 */
#define WM8350_VALID_GPIOS \
    ( WM_GPIO_0 | WM_GPIO_1 | WM_GPIO_2 | WM_GPIO_3 | \
      WM_GPIO_4 | WM_GPIO_5 | WM_GPIO_6 | WM_GPIO_7 | \
      WM_GPIO_8 | WM_GPIO_9 | WM_GPIO_10 | WM_GPIO_11 | \
      WM_GPIO_12 )

/**
 * Function codes for the different GPIOs.
 */

/** Alternate functions for GPIO 0 - inputs */
#define WM8350_GP0_FN_GPIO                           0
#define WM8350_GP0_FN_PWR_ON                         1
#define WM8350_GP0_FN_LDO_EN                         2
#define WM8350_GP0_FN_LPWR1                          3
#define WM8350_GP0_FN_PWR_OFF                        4

/** Alternate functions for GPIO 0 - outputs */
#define WM8350_GP0_FN_GPIO                           0
#define WM8350_GP0_FN_PWR_ON                         1
#define WM8350_GP0_FN_VRTC                           2
#define WM8350_GP0_FN_POR_B                          3
#define WM8350_GP0_FN_RST                            4

/** Alternate functions for GPIO 1 - inputs */
#define WM8350_GP1_FN_GPIO                           0
#define WM8350_GP1_FN_PWR_ON                         1
#define WM8350_GP1_FN_LDO_EN                         2
#define WM8350_GP1_FN_LPWR2                          3
#define WM8350_GP1_FN_WAKEUP                         4

/** Alternate functions for GPIO 1 - outputs */
#define WM8350_GP1_FN_GPIO                           0
#define WM8350_GP1_FN_DO_CONF                        1
#define WM8350_GP1_FN_RST                            2
#define WM8350_GP1_FN_MEMRST                         3
#define WM8350_GP1_FN_32KHZ                          4

/** Alternate functions for GPIO 2 - inputs */
#define WM8350_GP2_FN_GPIO                           0
#define WM8350_GP2_FN_PWR_ON                         1
#define WM8350_GP2_FN_WAKEUP                         2
#define WM8350_GP2_FN_32KHZ                          3
#define WM8350_GP2_FN_L_PWR3                         4

/** Alternate functions for GPIO 2 - outputs */
#define WM8350_GP2_FN_GPIO                           0
#define WM8350_GP2_FN_PWR_ON                         1
#define WM8350_GP2_FN_VRTC                           2
#define WM8350_GP2_FN_32KHZ                          3
#define WM8350_GP2_FN_RST                            4

/** Alternate functions for GPIO 3 - inputs */
#define WM8350_GP3_FN_GPIO                           0
#define WM8350_GP3_FN_PWR_ON                         1
#define WM8350_GP3_FN_LDO_EN                         2
#define WM8350_GP3_FN_PWR_OFF                        3
#define WM8350_GP3_FN_FLASH                          4

/** Alternate functions for GPIO 3 - outputs */
#define WM8350_GP3_FN_GPIO                           0
#define WM8350_GP3_FN_P_CLK                          1
#define WM8350_GP3_FN_VRTC                           2
#define WM8350_GP3_FN_32KHZ                          3
#define WM8350_GP3_FN_MEMRST                         4

/** Alternate functions for GPIO 4 - inputs */
#define WM8350_GP4_FN_GPIO                           0
#define WM8350_GP4_FN_MR                             1
#define WM8350_GP4_FN_FLASH                          2
#define WM8350_GP4_FN_HIBERNATE                      3
#define WM8350_GP4_FN_MASK_IN                        4

/** Alternate functions for GPIO 4 - outputs */
#define WM8350_GP4_FN_GPIO                           0
#define WM8350_GP4_FN_MEMRST                         1
#define WM8350_GP4_FN_ADA                            2
#define WM8350_GP4_FN_FLASH_OUT                      3
#define WM8350_GP4_FN_VCC_FAULT                      4
#define WM8350_GP4_FN_MICSHT                         5
#define WM8350_GP4_FN_MICDET                        10

/** Alternate functions for GPIO 5 - inputs */
#define WM8350_GP5_FN_GPIO                           0
#define WM8350_GP5_FN_LPWR1                          1
#define WM8350_GP5_FN_ADCLRCLK                       2
#define WM8350_GP5_FN_HIBERNATE                      3
#define WM8350_GP5_FN_PWR_OFF                        4

/** Alternate functions for GPIO 5 - outputs */
#define WM8350_GP5_FN_GPIO                           0
#define WM8350_GP5_FN_P_CLK                          1
#define WM8350_GP5_FN_ADCLRCLK                       2
#define WM8350_GP5_FN_OSC32KHZ                       3
#define WM8350_GP5_FN_BATT_FAULT                     4
#define WM8350_GP5_FN_MICSHT                         5
#define WM8350_GP5_FN_ADA                            6
#define WM8350_GP5_FN_CODEC_OPCLK                    7
#define WM8350_GP5_FN_MICDET                        10

/** Alternate functions for GPIO 6 - inputs */
#define WM8350_GP6_FN_GPIO                           0
#define WM8350_GP6_FN_LPWR2                          1
#define WM8350_GP6_FN_FLASH                          2
#define WM8350_GP6_FN_HIBERNATE                      3

/** Alternate functions for GPIO 6 - outputs */
#define WM8350_GP6_FN_GPIO                           0
#define WM8350_GP6_FN_MEMRST                         1
#define WM8350_GP6_FN_ADA                            2
#define WM8350_GP6_FN_RTC_DSW                        3
#define WM8350_GP6_FN_MICDET                         4
#define WM8350_GP6_FN_MICSHT                         5
#define WM8350_GP6_FN_ADCLRCLKB                      6

/** Alternate functions for GPIO 7 - inputs */
#define WM8350_GP7_FN_GPIO                           0
#define WM8350_GP7_FN_LPWR3                          1
#define WM8350_GP7_FN_MASK_IN                        2
#define WM8350_GP7_FN_HIBERNATE                      3

/** Alternate functions for GPIO 7 - outputs */
#define WM8350_GP7_FN_GPIO                           0
#define WM8350_GP7_FN_P_CLK                          1
#define WM8350_GP7_FN_VCC_FAULT                      2
#define WM8350_GP7_FN_BATT_FAULT                     3
#define WM8350_GP7_FN_MICDET                         4
#define WM8350_GP7_FN_MICSHT                         5
#define WM8350_GP7_FN_ADA                            6
#define WM8350_GP7_FN_FLL_CLK                       12

/** Alternate functions for GPIO 8 - inputs */
#define WM8350_GP8_FN_GPIO                           0
#define WM8350_GP8_FN_MR                             1
#define WM8350_GP8_FN_ADCBCLK                        2
#define WM8350_GP8_FN_PWR_OFF                        3
#define WM8350_GP8_FN_HIBERNATE                      4

/** Alternate functions for GPIO 8 - outputs */
#define WM8350_GP8_FN_GPIO                           0
#define WM8350_GP8_FN_VCC_FAULT                      1
#define WM8350_GP8_FN_ADCBCLK                        2
#define WM8350_GP8_FN_BATT_FAULT                     3
#define WM8350_GP8_FN_RST                            4

/** Alternate functions for GPIO 9 - inputs */
#define WM8350_GP9_FN_GPIO                           0
#define WM8350_GP9_FN_HEARTBEAT                      1
#define WM8350_GP9_FN_MASK_IN                        2
#define WM8350_GP9_FN_PWR_OFF                        3
#define WM8350_GP9_FN_HIBERNATE                      4

/** Alternate functions for GPIO 9 - outputs */
#define WM8350_GP9_FN_GPIO                           0
#define WM8350_GP9_FN_VCC_FAULT                      1
#define WM8350_GP9_FN_LINE_GT_BATT                   2
#define WM8350_GP9_FN_BATT_FAULT                     3
#define WM8350_GP9_FN_MEMRST                         4

/** Alternate functions for GPIO 10 - inputs */
#define WM8350_GP10_FN_GPIO                          0
#define WM8350_GP10_FN_PWR_OFF                       3

/** Alternate functions for GPIO 10 - outputs */
#define WM8350_GP10_FN_GPIO                          0
#define WM8350_GP10_FN_ISINKC                        1
#define WM8350_GP10_FN_LINE_GT_BATT                  2
#define WM8350_GP10_FN_CH_IND                        3

/** Alternate functions for GPIO 11 - inputs */
#define WM8350_GP11_FN_GPIO                          0
#define WM8350_GP11_FN_WAKEUP                        2

/** Alternate functions for GPIO 11 - outputs */
#define WM8350_GP11_FN_GPIO                          0
#define WM8350_GP11_FN_ISINKD                        1
#define WM8350_GP11_FN_LINE_GT_BATT                  2
#define WM8350_GP11_FN_CH_IND                        3

/** Alternate functions for GPIO 12 - inputs */
#define WM8350_GP12_FN_GPIO                          0

/** Alternate functions for GPIO 12 - outputs */
#define WM8350_GP12_FN_GPIO                          0
#define WM8350_GP12_FN_ISINKE                        1
#define WM8350_GP12_FN_LINE_GT_BATT                  2
#define WM8350_GP12_FN_LIN_EN                        3
#define WM8350_GP12_FN_32KHZ                         4

/**
 * IDs for the interrupts on the WM8350.
 */
typedef enum WM8350_INTERRUPT_ID
{
    /* Top-level interrupts */
    WM8350_INT_WKUP                 = 0x00,
    WM8350_INT_USB                  = 0x01,
    WM8350_INT_CHG                  = 0x02,
    WM8350_INT_SYS                  = 0x03,
    WM8350_INT_RTC                  = 0x04,
    WM8350_INT_AUXADC               = 0x05,
    WM8350_INT_GP                   = 0x06,
    WM8350_INT_CODEC                = 0x07,
    WM8350_INT_EXT                  = 0x08,
    WM8350_INT_CS                   = 0x09,
    /* not present                  = 0x0A, */
    /* not present                  = 0x0B, */
    WM8350_INT_UV                   = 0x0C,
    WM8350_INT_OC                   = 0x0D,
    /* not present                  = 0x0E, */
    /* not present                  = 0x0F, */

    /* Second-level interrupts */
    WM8350_INT_CHG_VBATT_LT_2P85    = 0x10,
    WM8350_INT_CHG_VBATT_LT_3P1     = 0x11,
    WM8350_INT_CHG_VBATT_LT_3P9     = 0x12,
    /* not present                  = 0x13, */
    /* not present                  = 0x14, */
    WM8350_INT_RTC_ALM              = 0x15,
    WM8350_INT_RTC_SEC              = 0x16,
    WM8350_INT_RTC_PER              = 0x17,
    /* not present                  = 0x18, */
    WM8350_INT_CHG_FAST_RDY         = 0x19,
    WM8350_INT_CHG_START            = 0x1A,
    WM8350_INT_CHG_END              = 0x1B,
    WM8350_INT_CHG_TO               = 0x1C,
    WM8350_INT_CHG_BATT_FAIL        = 0x1D,
    WM8350_INT_CHG_BATT_COLD        = 0x1E,
    WM8350_INT_CHG_BATT_HOT         = 0x1F,

    WM8350_INT_SYS_WDOG_TO          = 0x20,
    WM8350_INT_SYS_CHIP_GT140       = 0x21,
    WM8350_INT_SYS_CHIP_GT115       = 0x22,
    WM8350_INT_SYS_HYST_COMP_FAIL   = 0x23,
    WM8350_INT_AUXADC_DCOMP1        = 0x24,
    WM8350_INT_AUXADC_DCOMP2        = 0x25,
    WM8350_INT_AUXADC_DCOMP3        = 0x26,
    WM8350_INT_AUXADC_DCOMP4        = 0x27,
    WM8350_INT_AUXADC_DATARDY       = 0x28,
    /* not present                  = 0x29, */
    WM8350_INT_USB_LIMIT            = 0x2A,
    /* not present                  = 0x2B, */
    WM8350_INT_CS2                  = 0x2C,
    WM8350_INT_CS1                  = 0x2D,
    /* not present                  = 0x2E, */
    /* not present                  = 0x2F, */

    /* not present                  = 0x30, */
    /* not present                  = 0x31, */
    /* not present                  = 0x32, */
    /* not present                  = 0x33, */
    /* not present                  = 0x34, */
    /* not present                  = 0x35, */
    /* not present                  = 0x36, */
    /* not present                  = 0x37, */
    /* not present                  = 0x38, */
    /* not present                  = 0x39, */
    /* not present                  = 0x3A, */
    /* not present                  = 0x3B, */
    /* not present                  = 0x3C, */
    /* not present                  = 0x3D, */
    /* not present                  = 0x3E, */
    /* not present                  = 0x3F, */

    WM8350_INT_UV_DC1               = 0x40,
    WM8350_INT_UV_DC2               = 0x41,
    WM8350_INT_UV_DC3               = 0x42,
    WM8350_INT_UV_DC4               = 0x43,
    WM8350_INT_UV_DC5               = 0x44,
    WM8350_INT_UV_DC6               = 0x45,
    /* not present                  = 0x46, */
    /* not present                  = 0x47, */
    WM8350_INT_UV_LDO1              = 0x48,
    WM8350_INT_UV_LDO2              = 0x49,
    WM8350_INT_UV_LDO3              = 0x4A,
    WM8350_INT_UV_LDO4              = 0x4B,
    /* not present                  = 0x3C, */
    /* not present                  = 0x3D, */
    /* not present                  = 0x3E, */
    /* not present                  = 0x3F, */

    /* not present                  = 0x50, */
    /* not present                  = 0x51, */
    /* not present                  = 0x52, */
    /* not present                  = 0x53, */
    /* not present                  = 0x54, */
    /* not present                  = 0x55, */
    /* not present                  = 0x56, */
    /* not present                  = 0x57, */
    /* not present                  = 0x58, */
    /* not present                  = 0x59, */
    /* not present                  = 0x5A, */
    /* not present                  = 0x5B, */
    /* not present                  = 0x5C, */
    /* not present                  = 0x5D, */
    /* not present                  = 0x5E, */
    WM8350_INT_OC_LS                = 0x5F,

    WM8350_INT_GP0                  = 0x60,
    WM8350_INT_GP1                  = 0x61,
    WM8350_INT_GP2                  = 0x62,
    WM8350_INT_GP3                  = 0x63,
    WM8350_INT_GP4                  = 0x64,
    WM8350_INT_GP5                  = 0x65,
    WM8350_INT_GP6                  = 0x66,
    WM8350_INT_GP7                  = 0x67,
    WM8350_INT_GP8                  = 0x68,
    WM8350_INT_GP9                  = 0x69,
    WM8350_INT_GP10                 = 0x6A,
    WM8350_INT_GP11                 = 0x6B,
    WM8350_INT_GP12                 = 0x6C,
    /* not present                  = 0x6D, */
    /* not present                  = 0x6E, */
    /* not present                  = 0x6F, */

    WM8350_INT_WKUP_GP_WAKEUP       = 0x70,
    WM8350_INT_WKUP_ON_KEY          = 0x71,
    WM8350_INT_WKUP_GP_PWR_ON       = 0x72,
    WM8350_INT_WKUP_WDOG_RST        = 0x73,
    WM8350_INT_WKUP_CONV_FAULT      = 0x74,
    WM8350_INT_WKUP_HIB_STATE       = 0x75,
    WM8350_INT_WKUP_OFF_STATE       = 0x76,
    /* not present                  = 0x77, */
    WM8350_INT_CODEC_MICD           = 0x78,
    WM8350_INT_CODEC_MICSCD         = 0x79,
    WM8350_INT_CODEC_JCK_DET_R      = 0x7A,
    WM8350_INT_CODEC_JCK_DET_L      = 0x7B,
    /* not present                  = 0x7C, */
    WM8350_INT_EXT_BATT_FB          = 0x7D,
    WM8350_INT_EXT_WALL_FB          = 0x7E,
    WM8350_INT_EXT_USB_FB           = 0x7F,

    WM8350_MAX_INTERRUPT            = 0x7F
} WM8350_INTERRUPT_ID;

#define WM8350_INTERRUPT_BANKS      8

/**
 * The full set of interrupts on the WM8350.
 */
typedef struct WM8350_INTERRUPT_BITS
{
    /* Top-level interrupts */
    int WKUP : 1;
    int USB : 1;
    int CHG : 1;
    int SYS : 1;
    int RTC : 1;
    int AUXADC : 1;
    int GP : 1;
    int CODEC : 1;
    int EXT : 1;
    int CS : 1;
    int dummy1: 2;
    int UV : 1;
    int OC : 1;
    int dummy2: 2;

    /* Second-level interrupts */
    int CHG_VBATT_LT_2P85 : 1;
    int CHG_VBATT_LT_3P1 : 1;
    int CHG_VBATT_LT_3P9 : 1;
    int dummy3: 2;
    int RTC_ALM : 1;
    int RTC_SEC : 1;
    int RTC_PER : 1;
    int dummy4: 1;
    int CHG_FAST_RDY : 1;
    int CHG_START : 1;
    int CHG_END : 1;
    int CHG_TO : 1;
    int CHG_BATT_FAIL : 1;
    int CHG_BATT_COLD : 1;
    int CHG_BATT_HOT : 1;

    int SYS_WDOG_TO : 1;
    int SYS_CHIP_GT140 : 1;
    int SYS_CHIP_GT115 : 1;
    int SYS_HYST_COMP_FAIL : 1;
    int AUXADC_DCOMP1 : 1;
    int AUXADC_DCOMP2 : 1;
    int AUXADC_DCOMP3 : 1;
    int AUXADC_DCOMP4 : 1;
    int AUXADC_DATARDY : 1;
    int dummy5: 1;
    int USB_LIMIT : 1;
    int dummy6: 1;
    int CS2 : 1;
    int CS1 : 1;
    int dummy7: 2;

    int dummy8: 16;

    int UV_DC1 : 1;
    int UV_DC2 : 1;
    int UV_DC3 : 1;
    int UV_DC4 : 1;
    int UV_DC5 : 1;
    int UV_DC6 : 1;
    int dummy9: 2;
    int UV_LDO1 : 1;
    int UV_LDO2 : 1;
    int UV_LDO3 : 1;
    int UV_LDO4 : 1;
    int dummy10: 4;

    int dummy11: 15;
    int OC_LS : 1;

    int GP0 : 1;
    int GP1 : 1;
    int GP2 : 1;
    int GP3 : 1;
    int GP4 : 1;
    int GP5 : 1;
    int GP6 : 1;
    int GP7 : 1;
    int GP8 : 1;
    int GP9 : 1;
    int GP10 : 1;
    int GP11 : 1;
    int GP12 : 1;
    int dummy12: 3;

    int WKUP_GP_WAKEUP : 1;
    int WKUP_ON_KEY : 1;
    int WKUP_GP_PWR_ON : 1;
    int WKUP_WDOG_RST : 1;
    int WKUP_CONV_FAULT : 1;
    int WKUP_HIB_STATE : 1;
    int WKUP_OFF_STATE : 1;
    int dummy13: 1;
    int CODEC_MICD : 1;
    int CODEC_MICSCD : 1;
    int CODEC_JCK_DET_R : 1;
    int CODEC_JCK_DET_L : 1;
    int dummy14: 1;
    int EXT_BATT_FB : 1;
    int EXT_WALL_FB : 1;
    int EXT_USB_FB : 1;
} WM8350_INTERRUPT_BITS;

/**
 * A union to help casting to and from a WM_INTERRUPT_SET.
 * Since a WM_INTERRUPT_SET is a (WM_INTERRUPT_BANK *), set is a
 * WM_INTERRUPT_SET.
 */
typedef union WM8350_INTERRUPT_SET
{
    WM_INTERRUPT_BANK       set[WM8350_INTERRUPT_BANKS];
    WM8350_INTERRUPT_BITS   ints;
} WM8350_INTERRUPT_SET;

/**
 * A definition to use to initialise a set to all interrupts.
 *
 * Use this in the declaration.  For example:
 *
 * WM8350_INTERRUPT_SET interrupts = WM8350_ALL_INTERRUPTS;
 */
#define WM8350_ALL_INTERRUPTS { 0x33FF, 0xFEE7, 0x35FF, 0x0000, 0x0F3F, 0x8000, 0x1FFF, 0xEF7F }

#endif    /* __WM8350_H__ */
/*------------------------------ END OF FILE ---------------------------------*/

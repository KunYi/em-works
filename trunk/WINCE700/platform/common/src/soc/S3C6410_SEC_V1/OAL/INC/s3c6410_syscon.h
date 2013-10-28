//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Header:  s3c6410_syscon.h
//
//  Defines the System Controller register layout and associated types
//  and constants.
//
#ifndef __S3C6410_SYSCON_H
#define __S3C6410_SYSCON_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  S3C6410_SYSCON_REG
//
//  System Controller registers. This register bank is located by the
//  constant S3C6410_BASE_REG_XX_SYSCON in the configuration file
//  s3c6410_reg_base_cfg.h.
//

typedef struct
{
    UINT32 APLL_LOCK;        // 0x00
    UINT32 MPLL_LOCK;        // 0x04
    UINT32 EPLL_LOCK;        // 0x08
    UINT32 APLL_CON;        // 0x0c

    UINT32 MPLL_CON;        // 0x10
    UINT32 EPLL_CON0;        // 0x14
    UINT32 EPLL_CON1;        // 0x18
    UINT32 CLK_SRC;            // 0x1c

    UINT32 CLK_DIV0;        // 0x20
    UINT32 CLK_DIV1;        // 0x24
    UINT32 CLK_DIV2;        // 0x28
    UINT32 CLK_OUT;            // 0x2c

    UINT32 HCLK_GATE;        // 0x30
    UINT32 PCLK_GATE;        // 0x34
    UINT32 SCLK_GATE;        // 0x38
    UINT32 PAD0;            // 0x3c

    UINT32 PAD1[48];            // 0x40~0xff

    UINT32 AHB_CON0;        // 0x100
    UINT32 AHB_CON1;        // 0x104
    UINT32 AHB_CON2;        // 0x108
    UINT32 PAD2;            // 0x10c

    UINT32 SDMA_SEL;        // 0x110
    UINT32 SW_RST;            // 0x114
    UINT32 SYS_ID;            // 0x118
    UINT32 PAD3;            // 0x11c

    UINT32 MEM_SYS_CFG;    // 0x120
    UINT32 QOS_OVERRIDE0;    // 0x124
    UINT32 QOS_OVERRIDE1;    // 0x128
    UINT32 MEM_CFG_STAT;    // 0x12c

    UINT32 PAD4[436];        // 0x130~0x7ff

    UINT32 PAD5;            // 0x800
    UINT32 PWR_CFG;        // 0x804
    UINT32 EINT_MASK;        // 0x808
    UINT32 PAD6;            // 0x80c

    UINT32 NORMAL_CFG;        // 0x810
    UINT32 STOP_CFG;        // 0x814
    UINT32 SLEEP_CFG;        // 0x818
    UINT32 PAD7;            // 0x81c

    UINT32 OSC_FREQ;        // 0x820
    UINT32 OSC_STABLE;        // 0x824
    UINT32 PWR_STABLE;        // 0x828
    UINT32 FPC_STABLE;        // 0x82c

    UINT32 MTC_STABLE;        // 0x830
    UINT32 PAD8[3];            // 0x834~0x83f

    UINT32 PAD9[48];            // 0x840~0x8ff

    UINT32 OTHERS;            // 0x900
    UINT32 RST_STAT;        // 0x904
    UINT32 WAKEUP_STAT;    // 0x908
    UINT32 BLK_PWR_STAT;    // 0x90c

    UINT32 PAD10[60];        // 0x910~0x9ff

    UINT32 INFORM0;            // 0xa00
    UINT32 INFORM1;            // 0xa04
    UINT32 INFORM2;            // 0xa08
    UINT32 INFORM3;            // 0xa0c

    UINT32 INFORM4;            // 0xa10
    UINT32 INFORM5;            // 0xa14
    UINT32 INFORM6;            // 0xa18
    UINT32 INFORM7;            // 0xa1c
} S3C6410_SYSCON_REG, *PS3C6410_SYSCON_REG;

//------------------------------------------------------------------------------

typedef struct _REG_OTHERS
{
    unsigned int    : 8;    // RESERVED
    unsigned int stable_counter : 1;
    unsigned int    : 8;    // RESERVED
    unsigned int usb_sig_mask   : 1;
    unsigned int    : 2;    // RESERVED
    unsigned int clear_dbgack   : 1;
    unsigned int clear_batf_int : 1;
    unsigned int syncack        : 4;
    unsigned int syncmode       : 1;
    unsigned int syncmuxsel     : 1;
    unsigned int    : 3;    // RESERVED
    unsigned int spniden        : 1;
    unsigned int spiden         : 1;
    unsigned int cp15disable    : 1;
} SYSCON_REG_OTHERS;

#define BIT_STABLE_COUNTER_TYPE     (23)
#define BIT_USB_SIG_MASK            (16)
#define BIT_CLEAR_DBGACK            (13)
#define BIT_CLEAR_BATF_INT          (12)
#define BIT_SYNCACK                 (8)
#define BIT_SYNCMODE                (7)
#define BIT_SYNCMUXSEL              (6)
#define BIT_SPNIDEN                 (2)
#define BIT_SPIDEN                  (1)
#define BIT_CP15DISABLE             (0)

typedef struct _REG_MEM_SYS_CFG
{
    unsigned int    :17;   // RESERVED
    unsigned int indep_cf           : 1;
    unsigned int ncfg_addr_expand   : 1;
    unsigned int bus_width          : 1;
    unsigned int ebi_pri            : 1;
    unsigned int ebi_fix_pri        : 3;
    unsigned int addr_expand        : 1;
    unsigned int    : 1;    //RESERVED
    unsigned int mp0_cs_cfg         : 6;
} SYSCON_REG_MEM_SYS_CFG;

#define BIT_INDEP_CF                (14)
#define BIT_nCFG_ADDR_EXPAND        (13)
#define BIT_ROMBUS_WIDTH            (12)
#define ROMBUS_16BIT_WIDTH          (1<<12)
#define ROMBUS_8BIT_WIDTH           (0<<12)
#define BIT_EBI_PRI                 (11)
#define BIT_EBI_FIX_PRI             (8)
#define BIT_ADDR_EXPAND             (7)
#define BIT_MP0_CS_CFG              (0)

typedef struct _REG_NORMAL_CFG
{
    unsigned int     : 1;   // RESERVED
    unsigned int irom: 1;   // IROM
    unsigned int     :13;   // RESERVED = 0x1FFF
    unsigned int domain_etm : 1; 
    unsigned int domain_s   : 1;
    unsigned int domain_f   : 1;
    unsigned int domain_p   : 1;
    unsigned int domain_i   : 1;
    unsigned int     : 1;
    unsigned int domain_g   : 1;
    unsigned int domain_v   : 1;
    unsigned int     : 9;   // RESERVED = 0x100
}  SYSCON_REG_NORMAL_CFG;

#define BIT_IROM        (30)
#define BIT_DOMAIN_ETM  (16)
#define BIT_DOMAIN_S    (15)
#define BIT_DOMAIN_F    (14)
#define BIT_DOMAIN_P    (13)
#define BIT_DOMAIN_I    (12)
#define BIT_DOMAIN_G    (10)
#define BIT_DOMAIN_V    (9)

typedef struct _REG_BLK_PWR_STAT
{
    unsigned int    :25;    // RESERVED
    unsigned int deep_stop_wakeup: 1;
    unsigned int sw_reset: 1;
    unsigned int e_sleep_wakeup:1;
    unsigned int sleep_wakeup:1;
    unsigned int wdt_reset:1;
    unsigned int warm_reset:1;
    unsigned int hw_reset:1;
} SYSCON_REG_BLK_PWR_STAT;

#define BIT_BLK_G               (7)
#define BIT_BLK_ETM             (6)
#define BIT_BLK_S               (5)
#define BIT_BLK_F               (4)
#define BIT_BLK_P               (3)
#define BIT_BLK_I               (2)
#define BIT_BLK_V               (1)
#define BIT_BLK_TOP             (0)

#if __cplusplus
    }
#endif

#endif    // __S3C6410_SYSCON_H

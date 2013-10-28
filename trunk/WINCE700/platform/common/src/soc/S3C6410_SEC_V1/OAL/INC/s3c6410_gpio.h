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
//  Header:  s3c6410_gpio.h
//
//  Defines the Input/Output Ports (GPIO) control registers and associated
//  types and constants.
//
#ifndef __S3C6410_GPIO_H
#define __S3C6410_GPIO_H

#if __cplusplus
extern "C" {
#endif

typedef struct
{
    UINT32 GPACON;        // 000
    UINT32 GPADAT;        // 004
    UINT32 GPAPUD;        // 008
    UINT32 GPACONSLP;    // 00c

    UINT32 GPAPUDSLP;    // 010
    UINT32 PAD1[3];        // 014~01f

    UINT32 GPBCON;        // 020
    UINT32 GPBDAT;        // 024
    UINT32 GPBPUD;        // 028
    UINT32 GPBCONSLP;    // 02c

    UINT32 GPBPUDSLP;    // 030
    UINT32 PAD2[3];        // 034~03f

    UINT32 GPCCON;        // 040
    UINT32 GPCDAT;        // 044
    UINT32 GPCPUD;        // 048
    UINT32 GPCCONSLP;    // 04c

    UINT32 GPCPUDSLP;    // 050
    UINT32 PAD3[3];        // 054~05f

    UINT32 GPDCON;        // 060
    UINT32 GPDDAT;        // 064
    UINT32 GPDPUD;        // 068
    UINT32 GPDCONSLP;    // 06c

    UINT32 GPDPUDSLP;    // 070
    UINT32 PAD4[3];        // 074~07f

    UINT32 GPECON;        // 080
    UINT32 GPEDAT;        // 084
    UINT32 GPEPUD;        // 088
    UINT32 GPECONSLP;    // 08c

    UINT32 GPEPUDSLP;    // 090
    UINT32 PAD5[3];        // 094~09f

    UINT32 GPFCON;        // 0a0
    UINT32 GPFDAT;        // 0a4
    UINT32 GPFPUD;        // 0a8
    UINT32 GPFCONSLP;    // 0ac

    UINT32 GPFPUDSLP;    // 0b0
    UINT32 PAD6[3];        // 0b4~0bf

    UINT32 GPGCON;        // 0c0
    UINT32 GPGDAT;        // 0c4
    UINT32 GPGPUD;        // 0c8
    UINT32 GPGCONSLP;    // 0cc

    UINT32 GPGPUDSLP;    // 0d0
    UINT32 PAD7[3];        // 0d4~0df

    UINT32 GPHCON0;        // 0e0
    UINT32 GPHCON1;        // 0e4
    UINT32 GPHDAT;        // 0e8
    UINT32 GPHPUD;        // 0ec

    UINT32 GPHCONSLP;    // 0f0
    UINT32 GPHPUDSLP;    // 0f4
    UINT32 PAD8[2];        // 0f8~0ff

    UINT32 GPICON;        // 100
    UINT32 GPIDAT;        // 104
    UINT32 GPIPUD;        // 108
    UINT32 GPICONSLP;    // 10c

    UINT32 GPIPUDSLP;    // 110
    UINT32 PAD9[3];        // 114~11f

    UINT32 GPJCON;        // 120
    UINT32 GPJDAT;        // 124
    UINT32 GPJPUD;        // 128
    UINT32 GPJCONSLP;    // 12c

    UINT32 GPJPUDSLP;    // 130
    UINT32 PAD10[3];        // 134~13f

    // GPK, GPL, GPM, GPN are Below

    UINT32 GPOCON;        // 140
    UINT32 GPODAT;        // 144
    UINT32 GPOPUD;        // 148
    UINT32 GPOCONSLP;    // 14c

    UINT32 GPOPUDSLP;    // 150
    UINT32 PAD11[3];        // 154~15f

    UINT32 GPPCON;        // 160
    UINT32 GPPDAT;        // 164
    UINT32 GPPPUD;        // 168
    UINT32 GPPCONSLP;    // 16c

    UINT32 GPPPUDSLP;    // 170
    UINT32 PAD12[3];        // 174~17f

    UINT32 GPQCON;        // 180
    UINT32 GPQDAT;        // 184
    UINT32 GPQPUD;        // 188
    UINT32 GPQCONSLP;    // 18c

    UINT32 GPQPUDSLP;    // 190
    UINT32 PAD13[3];        // 194~19f

    UINT32 SPCON;        // 1a0
    UINT32 PAD14[3];        // 1a4~1af

    UINT32 MEM0CONSTOP;    // 1b0
    UINT32 MEM1CONSTOP;    // 1b4
    UINT32 PAD15[2];            // 1b8~1bf

    UINT32 MEM0CONSLP0;    // 1c0
    UINT32 MEM0CONSLP1;    // 1c4
    UINT32 MEM1CONSLP;        // 1c8
    UINT32 PAD16;            // 1cc

    UINT32 MEM0DRVCON;        // 1d0
    UINT32 MEM1DRVCON;        // 1d4
    UINT32 PAD17[2];            // 1d8~1df

    UINT32 PAD18[8];            // 1e0~1ff

    UINT32 EINT12CON;        // 200
    UINT32 EINT34CON;        // 204
    UINT32 EINT56CON;        // 208
    UINT32 EINT78CON;        // 20c

    UINT32 EINT9CON;        // 210
    UINT32 PAD19[3];            // 214~21f

    UINT32 EINT12FLTCON;    // 220
    UINT32 EINT34FLTCON;    // 224
    UINT32 EINT56FLTCON;    // 228
    UINT32 EINT78FLTCON;    // 22c

    UINT32 EINT9FLTCON;        // 230
    UINT32 PAD20[3];            // 234~23f

    UINT32 EINT12MASK;        // 240
    UINT32 EINT34MASK;        // 244
    UINT32 EINT56MASK;        // 248
    UINT32 EINT78MASK;        // 24c

    UINT32 EINT9MASK;        // 250
    UINT32 PAD21[3];            // 254~25f

    UINT32 EINT12PEND;        // 260
    UINT32 EINT34PEND;        // 264
    UINT32 EINT56PEND;        // 268
    UINT32 EINT78PEND;        // 26c

    UINT32 EINT9PEND;        // 270
    UINT32 PAD22[3];            // 274~27f

    UINT32 PRIORITY;        // 280
    UINT32 SERVICE;            // 284
    UINT32 SERVICEPEND;        // 288
    UINT32 PAD23;            // 28f

    UINT32 PAD24[348];        // 290~7ff

    UINT32 GPKCON0;            // 800
    UINT32 GPKCON1;            // 804
    UINT32 GPKDAT;            // 808
    UINT32 GPKPUD;            // 80c

    UINT32 GPLCON0;            // 810
    UINT32 GPLCON1;            // 814
    UINT32 GPLDAT;            // 818
    UINT32 GPLPUD;            // 81c

    UINT32 GPMCON;            // 820
    UINT32 GPMDAT;            // 824
    UINT32 GPMPUD;            // 828
    UINT32 PAD25;            // 82f

    UINT32 GPNCON;            // 830
    UINT32 GPNDAT;            // 834
    UINT32 GPNPUD;            // 838
    UINT32 PAD26;            // 83f

    UINT32 PAD27[16];        // 840~87f

    UINT32 SPCONSLP;        // 880
    UINT32 PAD28[3];            // 884~88f

    UINT32 PAD29[28];        // 890~8ff

    UINT32 EINT0CON0;        // 900
    UINT32 EINT0CON1;        // 904
    UINT32 PAD30[2];            // 908~90f

    UINT32 EINT0FLTCON0;    // 910
    UINT32 EINT0FLTCON1;    // 914
    UINT32 EINT0FLTCON2;    // 918
    UINT32 EINT0FLTCON3;    // 91c

    UINT32 EINT0MASK;        // 920
    UINT32 EINT0PEND;        // 924
    UINT32 PAD31[2];            // 928~92f

    UINT32 SLPEN;            // 930
    UINT32 PAD32[3];            // 934~93f
} S3C6410_GPIO_REG, *PS3C6410_GPIO_REG;

//------------------------------------------------------------------------------
// Enumerates for GPIO Control

typedef enum
{
    EINT_SIGNAL_LOW_LEVEL = 0x0,
    EINT_SIGNAL_HIGH_LEVEL = 0x1,
    EINT_SIGNAL_FALL_EDGE = 0x2,
    EINT_SIGNAL_RISE_EDGE = 0x4,
    EINT_SIGNAL_BOTH_EDGE = 0x6
} EINT_SIGNAL_METHOD;

typedef enum
{
    EINT_FILTER_DISABLE = 0,
    EINT_FILTER_DELAY,
    EINT_FILTER_DIGITAL
} EINT_FILTER_METHOD;

typedef enum {
    EINT0CON_EINT0 = 0,
    EINT0CON_EINT1 = 0,
    EINT0CON_EINT2 = 4,
    EINT0CON_EINT3 = 4,
    EINT0CON_EINT4 = 8,
    EINT0CON_EINT5 = 8,
    EINT0CON_EINT6 = 12,
    EINT0CON_EINT7 = 12,
    EINT0CON_EINT8 = 16,
    EINT0CON_EINT9 = 16,
    EINT0CON_EINT10 = 20,
    EINT0CON_EINT11 = 20,
    EINT0CON_EINT12 = 24,
    EINT0CON_EINT13 = 24,
    EINT0CON_EINT14 = 28,
    EINT0CON_EINT15 = 28
} BIT_EINT0CON0_DEF;

typedef enum {
    FLTWIDTH_0 = 0,
    FLTWIDTH_1 = 0,
    FLTSEL_0 = 6,
    FLTSEL_1 = 6,
    FLTEN_0 = 7,
    FLTEN_1 = 7,
    FLTWIDTH_2 = 8,
    FLTWIDTH_3 = 8,
    FLTSEL_2 = 14,
    FLTSEL_3 = 14,
    FLTEN_2 = 15,
    FLTEN_3 = 15,
    FLTWIDTH_4 = 16,
    FLTWIDTH_5 = 16,
    FLTSEL_4 = 22,
    FLTSEL_5 = 22,
    FLTEN_4 = 23,
    FLTEN_5 = 23,
    FLTWIDTH_6 = 24,
    FLTWIDTH_7 = 24,
    FLTSEL_6 = 30,
    FLTSEL_7 = 30,
    FLTEN_6 = 31,
    FLTEN_7 = 31,
} BIT_EINT0FLTCON0_DEF;

typedef enum {
    FLTWIDTH_8 = 0,
    FLTWIDTH_9 = 0,
    FLTSEL_8 = 6,
    FLTSEL_9 = 6,
    FLTEN_8 = 7,
    FLTEN_9 = 7,
    FLTWIDTH_10 = 8,
    FLTWIDTH_11 = 8,
    FLTSEL_10 = 14,
    FLTSEL_11 = 14,
    FLTEN_10 = 15,
    FLTEN_11 = 15,
    FLTWIDTH_12 = 16,
    FLTWIDTH_13 = 16,
    FLTSEL_12 = 22,
    FLTSEL_13 = 22,
    FLTEN_12 = 23,
    FLTEN_13 = 23,
    FLTWIDTH_14 = 24,
    FLTWIDTH_15 = 24,
    FLTSEL_14 = 30,
    FLTSEL_15 = 30,
    FLTEN_14 = 31,
    FLTEN_15 = 31,
} BIT_EINT0FLTCON1_DEF;

typedef struct _GPIO_REG_EINT0CON0
{
    unsigned int     : 1;   // RESERVED
    unsigned int eint_15_14 : 3; 
    unsigned int     : 1;   // RESERVED
    unsigned int eint_13_12 : 3; 
    unsigned int     : 1;   // RESERVED
    unsigned int eint_11_10 : 3; 
    unsigned int     : 1;   // RESERVED
    unsigned int eint_9_8 : 3; 
    unsigned int     : 1;   // RESERVED
    unsigned int eint_7_6 : 3; 
    unsigned int     : 1;   // RESERVED
    unsigned int eint_5_4 : 3; 
    unsigned int     : 1;   // RESERVED
    unsigned int eint_3_2 : 3; 
    unsigned int     : 1;   // RESERVED
    unsigned int eint_1_0 : 3; 
}  GPIO_REG_EINT0CON0;

// GPIO Control Macros
#define GPN_MAX_NUMBER      (16)
#define GPNCON_BIT_WIDTH    (2)
#define GPNCON_BITMASK      (0x3)
#define GPNCON_INPUT        (0x0)
#define GPNCON_OUTPUT       (0x1)
#define GPNCON_EXTINT       (0x2)
#define GPNCON_KEYPADROW    (0x3)   //< only for GPN0~GPN7, This value is reserved for GPN8~GPN15

#define GPNPUD_BIT_WIDTH    (2)
#define GPNPUD_BITMASK      (0x3)
#define GPNPUD_DISABLED     (0x0)
#define GPNPUD_PULLDOWN     (0x1)
#define GPNPUD_PULLUP       (0x2)

#define GPNDAT_BIT_WIDTH    (1)
#define GPNDAT_BITMASK      (0x1)

#define EINT0CON0_BIT_WIDTH (4)
#define EINT0CON0_BITMASK   (0x7)
#define EINT0CON0_LOWLEVEL  (0x0)
#define EINT0CON0_HIGHLEVEL (0x1)
#define EINT0CON0_FALLINGEDGE   (0x2)
#define EINT0CON0_RISINGEDGE    (0x4)
#define EINT0CON0_BOTHEDGE      (0x6)

#define EINT0FILTER_WIDTH_MASK     (0x3f)
#define EINT0FILTERCON_MASK         (0xff)

#define GET_GPIO(mapped_s, port, portnumber)       (mapped_s##->##port & ( ##port##_BITMASK<<(portnumber*##port##_BIT_WIDTH)))
#define GET_CGPIO(mapped_s, port, portnumber)      (mapped_s##->##port & ~( ##port##_BITMASK<<(portnumber*##port##_BIT_WIDTH)))
#define SET_GPIO(mapped_s, port, portnumber, v)    (mapped_s##->##port |= ((v)<<(portnumber*##port##_BIT_WIDTH)))
//#define SET_GPIO(mapped_s, port, portnumber, v)    (mapped_s##->##port = (GET_CGPIO(mapped_s, port, portnumber) | ((v)<<(portnumber*##port##_BIT_WIDTH))))
#define CLEAR_GPIO(mapped_s, port, portnumber)     (mapped_s##->##port &= ~( ##port##_BITMASK<<(portnumber*##port##_BIT_WIDTH)))

#define GET_GPNCON(mapped_s, portnumber)       (mapped_s##->GPNCON & (GPNCON_BITMASK<<(portnumber*GPNCON_BIT_WIDTH)))
#define GET_CGPNCON(mapped_s, portnumber)      (mapped_s##->GPNCON & ~(GPNCON_BITMASK<<(portnumber*GPNCON_BIT_WIDTH)))
#define SET_GPNCON(mapped_s, portnumber, v)    (mapped_s##->GPNCON = (GET_CGPNCON(mapped_s, portnumber) | ((v)<<(portnumber*GPNCON_BIT_WIDTH))))
#define CLEAR_GPNCON(mapped_s, portnumber)     (mapped_s##->GPNCON &= ~(GPNCON_BITMASK<<(portnumber*GPNCON_BIT_WIDTH)))


#if __cplusplus
}
#endif

#endif // __S3C6410_GPIO_H

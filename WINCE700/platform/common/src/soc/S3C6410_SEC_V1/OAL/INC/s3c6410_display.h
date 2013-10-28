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
//  Header: S3C6410_display.h
//
//  Defines the Display Controller CPU register layout and definitions.
//
#ifndef __S3C6410_DISPLAY_H
#define __S3C6410_DISPLAY_H

#if __cplusplus
    extern "C"
    {
#endif

//------------------------------------------------------------------------------
//  Type: S3C6410_DISPLAY_REG
//
//  Display Controller registers. This register bank is located by the constant
//  CPU_BASE_REG_XX_DISPLAY in the configuration file cpu_base_reg_cfg.h.
//

typedef struct
{
    UINT32 VIDCON0;            // 0x00
    UINT32 VIDCON1;            // 0x04
    UINT32 VIDCON2;            // 0x08
    UINT32 PAD0;            // 0x0c

    UINT32 VIDTCON0;        // 0x10
    UINT32 VIDTCON1;        // 0x14
    UINT32 VIDTCON2;        // 0x18
    UINT32 VIDTCON3;        // 0x1c

    UINT32 WINCON0;        // 0x20
    UINT32 WINCON1;        // 0x24
    UINT32 WINCON2;        // 0x28
    UINT32 WINCON3;        // 0x2c

    UINT32 WINCON4;        // 0x30
    UINT32 PAD2[3];            // 0x34~3f

    UINT32 VIDOSD0A;        // 0x40
    UINT32 VIDOSD0B;        // 0x44
    UINT32 VIDOSD0C;        // 0x48
    UINT32 PAD3[1];            // 0x4c

    UINT32 VIDOSD1A;        // 0x50
    UINT32 VIDOSD1B;        // 0x54
    UINT32 VIDOSD1C;        // 0x58
    UINT32 VIDOSD1D;        // 0x5c

    UINT32 VIDOSD2A;        // 0x60
    UINT32 VIDOSD2B;        // 0x64
    UINT32 VIDOSD2C;        // 0x68
    UINT32 VIDOSD2D;        // 0x6c

    UINT32 VIDOSD3A;        // 0x70
    UINT32 VIDOSD3B;        // 0x74
    UINT32 VIDOSD3C;        // 0x78
    UINT32 PAD4[1];            // 0x7c

    UINT32 VIDOSD4A;        // 0x80
    UINT32 VIDOSD4B;        // 0x84
    UINT32 VIDOSD4C;        // 0x88
    UINT32 PAD5[1];            // 0x8c

    UINT32 PAD6[4];            // 0x90~9f

    UINT32 VIDW00ADD0B0;    // 0xa0
    UINT32 VIDW00ADD0B1;    // 0xa4
    UINT32 VIDW01ADD0B0;    // 0xa8
    UINT32 VIDW01ADD0B1;    // 0xac

    UINT32 VIDW02ADD0;        // 0xb0
    UINT32 PAD7[1];            // 0xb4
    UINT32 VIDW03ADD0;        // 0xb8
    UINT32 PAD8[1];            // 0x8c

    UINT32 VIDW04ADD0;        // 0xc0
    UINT32 PAD9[3];            // 0xc4 ~ 0xcf

    UINT32 VIDW00ADD1B0;    // 0xd0
    UINT32 VIDW00ADD1B1;    // 0xd4
    UINT32 VIDW01ADD1B0;    // 0xd8
    UINT32 VIDW01ADD1B1;    // 0xdc

    UINT32 VIDW02ADD1;        // 0xe0
    UINT32 PAD10[1];            // 0xe4
    UINT32 VIDW03ADD1;        // 0xe8
    UINT32 PAD11[1];            // 0xec

    UINT32 VIDW04ADD1;        // 0xf0
    UINT32 PAD12[3];            // 0xf4 ~ 0xff


    UINT32 VIDW00ADD2;        // 0x100
    UINT32 VIDW01ADD2;        // 0x104
    UINT32 VIDW02ADD2;        // 0x108
    UINT32 VIDW03ADD2;        // 0x10c

    UINT32 VIDW04ADD2;        // 0x110
    UINT32 PAD13[3];            // 0x114 ~ 0x11f

    UINT32 PAD14[4];            // 0x120 ~ 0x12f

    UINT32 VIDINTCON0;        // 0x130
    UINT32 VIDINTCON1;        // 0x134
    UINT32 PAD15[2];            // 0x138 ~ 0x13f

    UINT32 W1KEYCON0;        // 0x140
    UINT32 W1KEYCON1;        // 0x144
    UINT32 W2KEYCON0;        // 0x148
    UINT32 W2KEYCON1;        // 0x14c

    UINT32 W3KEYCON0;        // 0x150
    UINT32 W3KEYCON1;        // 0x154
    UINT32 W4KEYCON0;        // 0x158
    UINT32 W4KEYCON1;        // 0x15c

    UINT32 PAD16[4];            // 0x160 ~ 0x16f

    UINT32 DITHMODE;        // 0x170
    UINT32 PAD17[3];            // 0x174~17f

    UINT32 WIN0MAP;        //0x180,
    UINT32 WIN1MAP;        // 0x184
    UINT32 WIN2MAP;        // 0x188
    UINT32 WIN3MAP;        // 0x18c

    UINT32 WIN4MAP;        //0x190
    UINT32 PAD18[3];            // 0x194~19f

    UINT32 WPALCON;        //0x1a0
    UINT32 TRIGCON;            //0x1a4
    UINT32 PAD19[2];            // 0x1a8~17f

    UINT32 I80IFCONA0;        // 0x1b0
    UINT32 I80IFCONA1;        // 0x1b4
    UINT32 I80IFCONB0;        // 0x1b8
    UINT32 I80IFCONB1;        // 0x1bc

    UINT32 PAD21[4];            // 0x1c0 ~ 0x1cf

    UINT32 LDI_CMDCON0;    // 0x1d0
    UINT32 LDI_CMDCON1;    //0x1d4
    UINT32 PAD22[2];            // 0x1d8~1df

    UINT32 SIFCCON0;    // 0x1e0
    UINT32 SIFCCON1;    //0x1e4
    UINT32 SIFCCON2;    //0x1e8
    UINT32 PAD23[1];    //0x1ec

    UINT32 PAD24[36];    //0x1f0 ~ 0x27f

    UINT32 LDI_CMD0;   //0x280
    UINT32 LDI_CMD1;   //0x284
    UINT32 LDI_CMD2;   //0x288
    UINT32 LDI_CMD3;   //0x28c

    UINT32 LDI_CMD4;   //0x290
    UINT32 LDI_CMD5;   //0x294
    UINT32 LDI_CMD6;   //0x298
    UINT32 LDI_CMD7;   //0x29c

    UINT32 LDI_CMD8;   //0x2a0
    UINT32 LDI_CMD9;     //0x2a4
    UINT32 LDI_CMD10;  //0x2a8
    UINT32 LDI_CMD11;  //0x2ac

    UINT32 PAD25[20];    //0x2b0 ~ 0x2ff


    UINT32 W2PDATA01;   //0x300
    UINT32 W2PDATA23;   //0x304
    UINT32 W2PDATA45;   //0x308
    UINT32 W2PDATA67;   //0x30c

    UINT32 W2PDATA89;   //0x310
    UINT32 W2PDATAAB;  //0x314
    UINT32 W2PDATACD;  //0x318
    UINT32 W2PDATAEF;   //0x31c

    UINT32 W3PDATA01;   //0x320
    UINT32 W3PDATA23;   //0x324
    UINT32 W3PDATA45;   //0x328
    UINT32 W3PDATA67;   //0x32c

    UINT32 W3PDATA89;   //0x330
    UINT32 W3PDATAAB;  //0x334
    UINT32 W3PDATACD;   //0x338
    UINT32 W3PDATAEF;   //0x33c

    UINT32 W4PDATA01;   //0x340
    UINT32 W4PDATA23;   //0x344
    UINT32 PAD26[2];        //0x348~0x34f

    UINT32 PAD27[44];        //0x350~0x3ff

    UINT32 W0PRAMSTART;   //0x400
    UINT32 PAD28[3];        //0x404~0x40f

    UINT32 PAD29[252];        //0x410~0x7ff

    UINT32 W1PRAMSTART;   //0x800
} S3C6410_DISPLAY_REG, *PS3C6410_DISPLAY_REG;

#if __cplusplus
}
#endif

#endif    // __S3C6410_DISPLAY_H
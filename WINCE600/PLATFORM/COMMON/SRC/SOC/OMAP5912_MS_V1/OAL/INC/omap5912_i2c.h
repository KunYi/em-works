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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  Header:  omap5912_i2c.h
//
#ifndef __OMAP5912_I2C_H
#define __OMAP5912_I2C_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT16 REV;                 // 0000
    UINT16 IE;                  // 0002
    UINT16 STAT;                // 0004
    UINT16 RESERVED_0006;       // 0006
    UINT16 SYSS;                // 0008
    UINT16 BUF;                 // 000A
    UINT16 CNT;                 // 000C
    UINT16 DATA;                // 000E
    UINT16 SYSC;                // 0010
    UINT16 CON;                 // 0012
    UINT16 OA;                  // 0014
    UINT16 SA;                  // 0016
    UINT16 PSC;                 // 0018
    UINT16 SCLL;                // 001A
    UINT16 SCLH;                // 001C
    UINT16 SYSTEST;             // 001E
} OMAP5912_I2C_REGS;

//------------------------------------------------------------------------------

#define I2C_STAT_SBD            (1 << 15)
#define I2C_STAT_BB             (1 << 12)
#define I2C_STAT_ROVR           (1 << 11)
#define I2C_STAT_XUDF           (1 << 10)
#define I2C_STAT_AAS            (1 << 9)
#define I2C_STAT_GC             (1 << 5)
#define I2C_STAT_XRDY           (1 << 4)
#define I2C_STAT_RRDY           (1 << 3)
#define I2C_STAT_ARDY           (1 << 2)
#define I2C_STAT_NACK           (1 << 1)
#define I2C_STAT_AL             (1 << 0)

#define I2C_SYSS_RDONE          (1 << 0)

#define I2C_SYSC_SRST           (1 << 1)

#define I2C_CON_EN              (1 << 15)
#define I2C_CON_BE              (1 << 14)
#define I2C_CON_STB             (1 << 11)
#define I2C_CON_MST             (1 << 10)
#define I2C_CON_TRX             (1 << 9)
#define I2C_CON_XA              (1 << 8)
#define I2C_CON_STP             (1 << 1)
#define I2C_CON_STT             (1 << 0)

//------------------------------------------------------------------------------

#endif // __OMAP5912_I2C_H

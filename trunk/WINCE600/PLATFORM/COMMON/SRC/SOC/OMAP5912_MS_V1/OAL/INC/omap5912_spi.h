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
//  Header:  omap5912_spi.h
//
#ifndef __OMAP5912_SPI_H
#define __OMAP5912_SPI_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 REV;                 // 0x0000
    UINT32 RESERVED0004[3];     // 0x0004
    UINT32 SCR;                 // 0x0010
    UINT32 SSR;                 // 0x0014
    UINT32 ISR;                 // 0x0018
    UINT32 IER;                 // 0x001C
    UINT32 RESERVED0020;        // 0x0020
    UINT32 SET1;                // 0x0024
    UINT32 SET2;                // 0x0028
    UINT32 CTRL;                // 0x002C
    UINT32 STATUS;              // 0x0030
    UINT32 TX;                  // 0x0034
    UINT32 RX;                  // 0x0038
    UINT32 TEST;                // 0x003C
} OMAP5912_SPI_REGS;

//------------------------------------------------------------------------------

#define SPI_SET1_EN_CLK         (1 << 0)
#define SPI_SET1_PVT1           (0 << 1)
#define SPI_SET1_PVT128         (7 << 1)
#define SPI_SET1_MSK0           (1 << 4)
#define SPI_SET1_MSK1           (1 << 5)

#define SPI_SET2_L4             (1 << 14)
#define SPI_SET2_L3             (1 << 13)
#define SPI_SET2_L2             (1 << 12)
#define SPI_SET2_L1             (1 << 11)
#define SPI_SET2_L0             (1 << 10)
#define SPI_SET2_P4             (1 << 9)
#define SPI_SET2_P3             (1 << 8)
#define SPI_SET2_P2             (1 << 7)
#define SPI_SET2_P1             (1 << 6)
#define SPI_SET2_P0             (1 << 5)
#define SPI_SET2_C4             (1 << 4)
#define SPI_SET2_C3             (1 << 3)
#define SPI_SET2_C2             (1 << 2)
#define SPI_SET2_C1             (1 << 1)
#define SPI_SET2_C0             (1 << 0)

#define SPI_CTRL_AD0            (0 << 7)
#define SPI_CTRL_AD1            (1 << 7)
#define SPI_CTRL_AD2            (2 << 7)
#define SPI_CTRL_NB16           (15 << 2)
#define SPI_CTRL_WR             (1 << 1)
#define SPI_CTRL_RD             (1 << 0)

#define SPI_STATUS_WE           (1 << 1)
#define SPI_STATUS_RE           (1 << 0)

//------------------------------------------------------------------------------

#endif // __OMAP5912_SPI_H


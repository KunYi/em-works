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
//  Header:  omap5912_ulpd.h
//
#ifndef __OMAP5912_ULPD_H
#define __OMAP5912_ULPD_H

//------------------------------------------------------------------------------

typedef volatile struct {

    UINT16 COUNTER_32_LSB;              // 0000
    UINT16 RESERVED_0002;               // 0002
    UINT16 COUNTER_32_MSB;              // 0004
    UINT16 RESERVED_0006;               // 0006
    UINT16 COUNTER_HIGH_FREQ_LSB;       // 0008
    UINT16 RESERVED_000A;               // 000A
    UINT16 COUNTER_HIGH_FREQ_MSB;       // 000C
    UINT16 RESERVED_000E;               // 000E
    UINT16 GAUGING_CTRL;                // 0010
    UINT16 RESERVED_0012;               // 0012
    UINT16 IT_STATUS;                   // 0014
    UINT16 RESERVED_0016[7];            // 0016
    UINT16 SLICER_SETUP;                // 0024
    UINT16 RESERVED_0026;               // 0026
    UINT16 VTCXO_SETUP;                 // 0028
    UINT16 RESERVED_002A;               // 002A
    UINT16 RF_SETUP;                    // 002C
    UINT16 RESERVED_002E;               // 002E
    UINT16 CLOCK_CTRL;                  // 0030
    UINT16 RESERVED_0032;               // 0032
    UINT16 SOFT_REQ;                    // 0034
    UINT16 RESERVED_0036;               // 0036
    UINT16 COUNTER_32_FIQ;              // 0038
    UINT16 RESERVED_003A[3];            // 003A
    UINT16 STATUS_REQ;                  // 0040
    UINT16 RESERVED_0042;               // 0042
    UINT16 PLL_DIV;                     // 0044
    UINT16 RESERVED_0046[3];            // 0046
    UINT16 PLL_CTRL_STATUS;             // 004C
    UINT16 RESERVED_004E;               // 004E
    UINT16 POWER_CTRL;                  // 0050
    UINT16 RESERVED_0052;               // 0052
    UINT16 STATUS_REQ2;                 // 0054
    UINT16 RESERVED_0056;               // 0056
    UINT16 SLEEP_STATUS;                // 0058
    UINT16 RESERVED_005A[7];            // 005A
    UINT16 SOFT_DISABLE_REQ;            // 0068
    UINT16 RESERVED_006A;               // 006A
    UINT16 RESET_STATUS;                // 006C
    UINT16 RESERVED_006E;               // 006E
    UINT16 REVISION_NUMBER;             // 0070
    UINT16 RESERVED_0072;               // 0072
    UINT16 MCBSP2_CLK_DIV_CTRL_SEL;     // 0074
    UINT16 RESERVED_0076;               // 0076
    UINT16 MCBSP1_CLK_DIV_CTRL_SEL;     // 0078
    UINT16 RESERVED_007A;               // 007A
    UINT16 CAM_CLK_CTRL;                // 007C
	UINT16 RESERVED_007E;               // 007E
	UINT16 SOFT_REQ_REG2;               // 0080
    UINT16 RESERVED_0082;               // 0082
} OMAP5912_ULPD_REGS;

//------------------------------------------------------------------------------

#define SOFT_DPLL_REQ               (1 << 0)
#define SOFT_MCBSP2_PLL_REQ         (1 << 2)
#define SOFT_USBH_PLL_REQ           (1 << 3)
#define SOFT_USBD_PLL_REQ           (1 << 4)
#define SOFT_MCBSP1_PLL_REQ         (1 << 6)
#define SOFT_CAM_PLL_REQ            (1 << 7)
#define SOFT_USBO_PLL_REQ           (1 << 8)
#define SOFT_UART1_PLL_REQ          (1 << 9)
#define SOFT_UART3_PLL_REQ          (1 << 11)
#define SOFT_MMC_PLL_REQ            (1 << 12)
#define SOFT_EAC12M_PLL_REQ         (1 << 14)

#define POWER_CTRL_OSC_STOP_EN      (1 << 9)
#define POWER_CTRL_DEEP_SLEEP_EN    (1 << 4)

#define DEEP_SLEEP                  (1 << 0)
#define BIG_SLEEP                   (1 << 1)

//------------------------------------------------------------------------------

#endif

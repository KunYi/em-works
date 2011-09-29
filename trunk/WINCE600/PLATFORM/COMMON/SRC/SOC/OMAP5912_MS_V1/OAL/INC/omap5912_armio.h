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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File:  omap5912_armio.h
//
#ifndef __OMAP5912_ARMIO_H
#define __OMAP5912_ARMIO_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT16 INPUT_LATCH;     // 0000
    UINT16 UNUSED0002;      // 0002
    UINT16 OUTPUT_REG;      // 0004
    UINT16 UNUSED0006;      // 0006
    UINT16 IO_CONTROL;      // 0008
    UINT16 UNUSED000A[3];   // 000A
    UINT16 KBD_ROW_LATCH;   // 0010
    UINT16 RESERVED_0012;   // 0012
    UINT16 KBD_COL_LATCH;   // 0014
    UINT16 RESERVED_0016;   // 0016
    UINT16 EVENT_MODE;      // 0018
    UINT16 RESERVED_001A;   // 001A
    UINT16 IO_INT_EDGE;     // 001C
    UINT16 RESERVED_001E;   // 001E
    UINT16 KBD_INT_STAT;    // 0020
    UINT16 RESERVED_0022;   // 0022
    UINT16 IO_INT_STAT;     // 0024
    UINT16 RESERVED_0026;   // 0026
    UINT16 KBD_INT_MASK;    // 0028
    UINT16 RESERVED_002A;   // 002A
    UINT16 IO_INT_MASK;     // 002C
    UINT16 RESERVED_002E;   // 002E
    UINT16 IO_DEB_TIME;     // 0030
    UINT16 RESERVED_0032;   // 0032
    UINT16 IO_DEB_LATCH;    // 0034
    UINT16 RESERVED_0036;   // 0036
} OMAP5912_ARMIO_REGS;

//------------------------------------------------------------------------------

#define OMAP5912_ARMIO_PIN0              (0<<1)
#define OMAP5912_ARMIO_PIN1              (1<<1)
#define OMAP5912_ARMIO_PIN2              (2<<1)
#define OMAP5912_ARMIO_PIN3              (3<<1)
#define OMAP5912_ARMIO_PIN4              (4<<1)
#define OMAP5912_ARMIO_GPIO_EVENT_MODE   (1<<0)

#define OMAP5912_ARMIO_EDGE_SELECT0      (1<<0)
#define OMAP5912_ARMIO_EDGE_SELECT1      (1<<1)
#define OMAP5912_ARMIO_EDGE_SELECT2      (1<<2)
#define OMAP5912_ARMIO_EDGE_SELECT3      (1<<3)
#define OMAP5912_ARMIO_EDGE_SELECT4      (1<<4)

#define OMAP5912_ARMIO_KBD_INT           (1<<0)

#define OMAP5912_ARMIO_GPIO_INT0         (1<<0)
#define OMAP5912_ARMIO_GPIO_INT1         (1<<1)
#define OMAP5912_ARMIO_GPIO_INT2         (1<<2)
#define OMAP5912_ARMIO_GPIO_INT3         (1<<3)
#define OMAP5912_ARMIO_GPIO_INT4         (1<<4)

#define OMAP5912_ARMIO_KBD_MASKIT        (1<<0)

#define OMAP5912_ARMIO_GPIO_MASKIT       (1<<0)

//------------------------------------------------------------------------------

#endif // __OMAP5912_ARMIO_H

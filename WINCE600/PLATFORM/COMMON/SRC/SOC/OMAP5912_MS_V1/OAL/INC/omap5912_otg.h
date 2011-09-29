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
//  File:  omap5912_otg.h
//
#ifndef __OMAP5912_OTG_H
#define __OMAP5912_OTG_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 REV;                     // Revision
    UINT32 SYSCON_1;                // System configuration 1
    UINT32 SYSCON_2;                // System configuration 2
    UINT32 CTRL;                    // Device control
    UINT32 IRQ_EN;                  // Interrupt enable
    UINT32 IRQ_SRC;                 // Interrupt source
    UINT32 OUTCTRL;                 // Output control
    UINT32 RESERVED_001C;           // Reserved
    UINT32 TEST;                    // Debug control
    UINT32 RESERVED_0024[54];       // Reserved
    UINT32 VC;                      // Vendor code  
} OMAP5912_OTG_REGS;

//------------------------------------------------------------------------------

#define OTG_SYSCON_1_RESET_DONE     (1 << 2)
#define OTG_SYSCON_1_SOFT_RESET     (1 << 1)

#define OTG_SYSCON_2_ENABLE         (1 << 31)
#define OTG_SYSCON_2_SYNCHRO        (1 << 30)
#define OTG_SYSCON_2_MST16          (1 << 29)
#define OTG_SYSCON_2_SRP_GPDATA     (1 << 28)
#define OTG_SYSCON_2_SRP_GPDVBUS    (1 << 27)

#define OTG_CTRL_ASESSVLD           (1 << 20)
#define OTG_CTRL_BSESSEND           (1 << 19)
#define OTG_CTRL_BSESSVLD           (1 << 18)
#define OTG_CTRL_VBUSVLD            (1 << 17)
#define OTG_CTRL_ID                 (1 << 16)
#define OTG_CTRL_DRIVER_SEL         (1 << 15)
#define OTG_CTRL_A_SETB_HNPEN       (1 << 12)
#define OTG_CTRL_A_BUSREQ           (1 << 11)
#define OTG_CTRL_B_HNPEN            (1 << 9)
#define OTG_CTRL_B_BUSREQ           (1 << 8)
#define OTG_CTRL_BUSDROP            (1 << 7)
#define OTG_CTRL_PD                 (1 << 5)
#define OTG_CTRL_PU                 (1 << 4)
#define OTG_CTRL_DRV_VBUS           (1 << 3)
#define OTG_CTRL_PD_VBUS            (1 << 2)
#define OTG_CTRL_PU_VBUS            (1 << 1)
#define OTG_CTRL_PU_ID              (1 << 0)

#define OTG_IRQ_DRIVER_SWITCH       (1 << 15)
#define OTG_IRQ_A_VBUS_ERR          (1 << 13)
#define OTG_IRQ_A_REQ_TMROUT        (1 << 12)
#define OTG_IRQ_A_SRP_DETECT        (1 << 11)
#define OTG_IRQ_B_HNP_FAIL          (1 << 10)
#define OTG_IRQ_B_SRP_TMROUT        (1 << 9)
#define OTG_IRQ_B_SRP_DONE          (1 << 8)
#define OTG_IRQ_B_SRP_STARTED       (1 << 7)
#define OTG_IRQ_OPRT_CHG            (1 << 0)
//------------------------------------------------------------------------------

#endif

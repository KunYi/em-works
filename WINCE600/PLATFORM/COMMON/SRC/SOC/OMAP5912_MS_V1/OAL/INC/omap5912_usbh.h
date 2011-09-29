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
//  File:  omap5912_usbh.h
//
#ifndef __OMAP5912_USBH_H
#define __OMAP5912_USBH_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 HC_REVISION;                 // 0000
    UINT32 HC_CONTROL;                  // 0004
    UINT32 HC_COMMAND_STATUS;           // 0008
    UINT32 HC_INTERRUPT_STATUS;         // 000C
    UINT32 HC_INTERRUPT_ENABLE;         // 0010
    UINT32 HC_INTERRUPT_DISABLE;        // 0014
    UINT32 HC_HCCA;                     // 0018
    UINT32 HC_PERIOD_CURRENT_ED;        // 001C
    UINT32 HC_CONTROL_HEAD_ED;          // 0020
    UINT32 HC_CONTROL_CURRENT_ED;       // 0024
    UINT32 HC_BULK_HEAD_ED;             // 0028
    UINT32 HC_BULK_CURRENT_ED;          // 002C
    UINT32 HC_DONE_HEAD;                // 0030
    UINT32 HC_FM_INTERVAL;              // 0034
    UINT32 HC_FM_RMN;                   // 0038
    UINT32 HC_FM_NUMBER;                // 003C
    UINT32 HC_PERIODIC_START;           // 0040
    UINT32 HC_LS_THRESHOLD;             // 0044
    UINT32 HC_RH_DESCRIPTOR_A;          // 0048
    UINT32 HC_RH_DESCRIPTOR_B;          // 004C
    UINT32 HC_RH_STATUS;                // 0050
    UINT32 HC_RH_PORT_STATUS[15];       // 0054
	UINT32 HC_UE_ADDR;                  // 00E0
	UINT32 HC_UE_STAT;                  // 00E4
	UINT32 HC_TM_OUT_CTRL;              // 00E8
	UINT32 HC_HST_REV;                  // 00EC
} OMAP5912_USBH_REGS;

//------------------------------------------------------------------------------

#endif // __OMAP5912_USBH_H

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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  MX31_usbcommon.h
//
//  Provides definitions for usb module based on Freescale MX31 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX31_USBCOMMON_H
#define __MX31_USBCOMMON_H


#if __cplusplus
extern "C" {
#endif

BOOL USBClockInit(void);
BOOL BSPUSBClockCreateFileMapping(void);
void BSPUSBClockDeleteFileMapping(void);
BOOL BSPUSBClockDisable(BOOL fStop);

typedef enum {
    USB_SEL_H2 = 0,
    USB_SEL_H1,
    USB_SEL_OTG
} USB_SEL_TYPE;

// ISP1504 ULPI Register offset
#define ISP1504_VENDERID_LOW_R  0
#define ISP1504_VENDERID_HIGH_R 1
#define ISP1504_PRODUCT_LOW_R   2
#define ISP1504_PRODUCT_HIGH_R  3
#define ISP1504_FUNCTION_CTRL_RW    4
#define ISP1504_FUNCTION_CTRL_S     5
#define ISP1504_FUNCTION_CTRL_C     6
#define ISP1504_INTERFACE_CTRL_RW   7
#define ISP1504_INTERFACE_CTRL_S    8
#define ISP1504_INTERFACE_CTRL_C    9
#define ISP1504_OTG_CTRL_RW         0xA
#define ISP1504_OTG_CTRL_S          0xB
#define ISP1504_OTG_CTRL_C          0xC
#define ISP1504_INTR_RISING_RW      0xD
#define ISP1504_INTR_RISING_S       0xE
#define ISP1504_INTR_RISING_C       0xF
#define ISP1504_INTR_FALLING_RW     0x10
#define ISP1504_INTR_FALLING_S      0x11
#define ISP1504_INTR_FALLING_C      0x12
#define ISP1504_INTR_STATUS_R       0x13
#define ISP1504_INTR_LATCH_RC       0x14
#define ISP1504_DEBUG_R             0x15
#define ISP1504_SCRATCH_RW          0x16
#define ISP1504_SCRATCH_S           0x17
#define ISP1504_SCRATCH_C           0x18
#define ISP1504_ACCESS_EXT_W        0x2F
#define ISP1504_POWER_CTRL_RW       0x3D
#define ISP1504_POWER_CTRL_S        0x3E
#define ISP1504_POWER_CTRL_C        0x3F

UCHAR ISP1504_ReadReg(volatile DWORD * reg, UCHAR idx);
BOOL ISP1504_WriteReg(volatile DWORD * reg, UCHAR idx, UCHAR data);

// USB3317 ULPI Register offset
#define USB3317_VENDERID_LOW_R  0
#define USB3317_VENDERID_HIGH_R 1
#define USB3317_PRODUCT_LOW_R   2
#define USB3317_PRODUCT_HIGH_R  3
#define USB3317_FUNCTION_CTRL_RW    4
#define USB3317_FUNCTION_CTRL_S     5
#define USB3317_FUNCTION_CTRL_C     6
#define USB3317_INTERFACE_CTRL_RW   7
#define USB3317_INTERFACE_CTRL_S    8
#define USB3317_INTERFACE_CTRL_C    9
#define USB3317_OTG_CTRL_RW         0xA
#define USB3317_OTG_CTRL_S          0xB
#define USB3317_OTG_CTRL_C          0xC
#define USB3317_INTR_RISING_RW      0xD
#define USB3317_INTR_RISING_S       0xE
#define USB3317_INTR_RISING_C       0xF
#define USB3317_INTR_FALLING_RW     0x10
#define USB3317_INTR_FALLING_S      0x11
#define USB3317_INTR_FALLING_C      0x12
#define USB3317_INTR_STATUS_R       0x13
#define USB3317_INTR_LATCH_RC       0x14
#define USB3317_DEBUG_R             0x15
#define USB3317_SCRATCH_RW          0x16
#define USB3317_SCRATCH_S           0x17
#define USB3317_SCRATCH_C           0x18
#define USB3317_ACCESS_EXT_W        0x2F
#define USB3317_POWER_CTRL_RW       0x3D
#define USB3317_POWER_CTRL_S        0x3E
#define USB3317_POWER_CTRL_C        0x3F

UCHAR USB3317_ReadReg(volatile DWORD * reg, UCHAR idx);
BOOL USB3317_WriteReg(volatile DWORD * reg, UCHAR idx, UCHAR data);

#ifdef __cplusplus
}
#endif

#endif // __MX31_USBCOMMON_H

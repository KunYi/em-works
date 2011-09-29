//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  COMMON_usbcommon.h
//
//  Provides definitions for usb module based on Freescale common SoC.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_USBCOMMON_H
#define __COMMON_USBCOMMON_H


#if __cplusplus
extern "C" {
#endif

BOOL USBClockInit(void);
BOOL BSPUSBClockCreateFileMapping(void);
void BSPUSBClockDeleteFileMapping(void);
BOOL BSPUSBClockSwitch(BOOL fOn);

extern DWORD FslUfnGetKitlDMABuffer();
extern DWORD FslUfnGetPageNumber(void *p, size_t size);
extern DWORD FslUfnGetPageSize();
extern DWORD FslUfnGetPageShift();
extern BOOL  FslUfnIsUSBKitlEnable();

typedef enum {
    USB_SEL_H2 = 0,
    USB_SEL_H1,
    USB_SEL_OTG
} USB_SEL_TYPE;

#ifdef DEBUG

#undef ZONE_ERROR
#define ZONE_ERROR              DEBUGZONE(0)
#undef ZONE_WARNING
#define ZONE_WARNING            DEBUGZONE(1)
#undef ZONE_TRANSFER
#define ZONE_TRANSFER           DEBUGZONE(3)
#undef ZONE_INTERRUPTS
#define ZONE_INTERRUPTS         DEBUGZONE(8)
#undef ZONE_POWER
#define ZONE_POWER              DEBUGZONE(9)
#undef ZONE_FUNCTION
#define ZONE_FUNCTION           DEBUGZONE(12)
//Don't use 15, because kitl use 15 as error
#undef ZONE_PDD
#define ZONE_PDD                DEBUGZONE(13) 
#endif

// USB3317 ULPI Register offset
#define USB3317_VENDORID_LOW_R  0
#define USB3317_VENDORID_HIGH_R 1
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
#define USB3317_POWER_CTRL_RW       0x39
#define USB3317_POWER_CTRL_S        0x3A
#define USB3317_POWER_CTRL_C        0x3B

UCHAR USB3317_ReadReg(volatile DWORD * reg, UCHAR idx);
BOOL USB3317_WriteReg(volatile DWORD * reg, UCHAR idx, UCHAR data);

#ifdef __cplusplus
}
#endif

#endif // __COMMON_USBCOMMON_H

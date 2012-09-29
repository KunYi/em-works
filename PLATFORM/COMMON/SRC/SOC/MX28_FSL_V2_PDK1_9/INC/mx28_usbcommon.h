//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  MX28_usbcommon.h
//
//  Provides definitions for usb module based on Freescale MX28 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX28_USBCOMMON_H
#define __MX28_USBCOMMON_H


#if __cplusplus
extern "C" {
#endif

BOOL USBClockInit(void);
BOOL BSPUSBClockCreateFileMapping(void);
void BSPUSBClockDeleteFileMapping(void);
BOOL BSPUSBClockSwitch(BOOL fOn);


#ifdef __cplusplus
}
#endif

#endif // __MX28_USBCOMMON_H

//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  MX28_usbname.h
//
//  Provides definitions for usb module based on Freescale MX28 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX28_USBNAME_H
#define __MX28_USBNAME_H


#if __cplusplus
extern "C" {
#endif

#define USBFunctionObjectName                     TEXT("USBFunc")
#define USBXcvrObjectName                         TEXT("USBXCVR")
#define USBHostObjectName                         TEXT("USBHost")
#define USBHostDetachName                         TEXT("USBDetach")
#define USBClockGatingName                        TEXT("USB_CLOCK_GATING")


#ifdef __cplusplus
}
#endif

#endif // __MX28_USBNAME_H

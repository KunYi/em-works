//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  COMMON_usbname.h
//
//  Provides definitions for usb module based on Freescale common SoC.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_USBNAME_H
#define __COMMON_USBNAME_H


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

#endif // __COMMON_USBNAME_H

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
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        bulverde_usbotg.h

Abstract:

        Bulverde USB OTG hardware describtion.

--*/

#ifndef __BULVERDE_USBOTG_H_
#define __BULVERDE_USBOTG_H_
#include <ceotgbus.h>
#define IOCTL_BUS_USBOTG_BULVERDE_GET_EVENT      _BUSACCESS_CTL_CODE(BUS_USBOTG_EXTENTION_FUNCTIONCODE+11)

typedef struct {
    DWORD   IDF:1;
    DWORD   IDR:1;
    DWORD   SDF:1;
    DWORD   SDR:1;
    
    DWORD   SVF:1;
    DWORD   SVR:1;
    DWORD   VV44F:1;
    DWORD   VV44R:1;

    DWORD   VV40F:1;
    DWORD   VV40R:1;
    DWORD   :6;

    DWORD   XF:1;
    DWORD   XR:1;
    DWORD   :6;

    DWORD   SF:1;
    DWORD   :7;
} UDCOTG_bit;

typedef union {
    UDCOTG_bit  bit;
    DWORD       ul;
} UDCOTG, *PUDCOTG ;

typedef struct {
    DWORD   CPVEN:1;
    DWORD   CPVPE:1;
    DWORD   DPPDE:1;
    DWORD   DMPDE:1;    
    DWORD   DPPUE:1;
    DWORD   DMPUE:1;
    DWORD   DPPUBE:1;
    DWORD   DMPUBE:1;
    DWORD   EXSP:1;
    DWORD   EXSUS:1;
    DWORD   IDON:1;
    DWORD   :5;
    
    DWORD   HXS:1;
    DWORD   HXOE:1;
    DWORD   :6;
    DWORD   SEOS:3;
    DWORD   :5;
}UP2OCR_bit;
typedef union {
    UP2OCR_bit  bit;
    DWORD       ul;
} UP2OCR, *PUP2OCR;

typedef struct {
    DWORD   CFG:2;
    DWORD   :30;
} UP3OCR_bit;
typedef union {
    UP3OCR_bit  bit;
    DWORD       ul;
}   UP3OCR, *PUP3OCR;

// OTG sharing same interrupt with USB Function Controller. We need this to forward Interrupt to OTG from USBFN.
#define IOCTL_BUS_USBOTG_BULVERDE_OTG_ISR _BUSACCESS_CTL_CODE(BUS_USBOTG_EXTENTION_PLATFORM_CODE+0)
// Input.
typedef struct {
    UDCOTG      udcotgisr;
    UDCOTG      udcotgicr;
} BULVERDE_OTGINPUT;
// Output is udcotgicr Bulver OTG need to set.


#endif




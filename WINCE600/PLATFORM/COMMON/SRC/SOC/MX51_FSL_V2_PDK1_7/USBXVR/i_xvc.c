//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
// File: 
//     i_XVC.c
// 
// Description:
//     USB Transceiver Driver is a stream interface driver which exposes the 
//     stream interface functions. This driver detects the USB plug in type and 
//     accordingly activates the Host or Function controller driver. Upon 
//     unplug, the respective driver gives back the control to the transceiver 
//     driver.
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"
#include "xvc.c"


DWORD GetUSBOTGIRQ(void)
{
    return IRQ_USB_OTG;
}

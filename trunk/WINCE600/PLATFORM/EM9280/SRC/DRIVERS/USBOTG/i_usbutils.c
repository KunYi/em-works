//-----------------------------------------------------------------------------
//
// Copyright (C) 2009-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  i_UsbUtils.c
//  This file is linked to UsbUtils.c in the USB Common
//
//

#include <..\USBCommon\UsbUtils.c>

//-----------------------------------------------------------------------------
//
//  Function: BSPGetUSBControllerType
//
//  This is used by the public common CSP library to be used to return the USB controller
//  currently using.
//
//  Parameters:
//      None.
//
//  Returns:
//      USB_SEL_OTG, USB_SEL_H2 or USB_SEL_H1. In this case, it would be USB_SEL_OTG
//
//-----------------------------------------------------------------------------
WORD BSPGetUSBControllerType(void)
{
    return USB_SEL_OTG;
}


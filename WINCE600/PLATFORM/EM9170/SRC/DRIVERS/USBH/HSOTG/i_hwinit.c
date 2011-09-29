//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  i_hwinit.c
//  This file is linked to hwinit.c in the USBH Common
//
//------------------------------------------------------------------------------

#define USB_HOST_MODE 1             // OTG high speed
#include <..\common\hwinit.c>

#ifdef DEBUG
//------------------------------------------------------------------------------
// Function: GetUSBPortType
//
// Description:
//     This function is to return the USB port in string format.
//
// Parameters:
//     NULL
//     
// Returns:
//     Pointer to the USB Port in string format
//
//------------------------------------------------------------------------------
const TCHAR *GetUSBPortType(void)
{
    static const TCHAR* cszDeviceType = TEXT("OTG High Speed");
    return cszDeviceType;    
}
#endif

//------------------------------------------------------------------------------
// Function: BSPGetUSBControllerType
//
// Description: This function is to return the USB Core Controller type in 
//              WORD format. This is called by CSP public code when accessing 
//              the type of controller it is using.
//
// Parameters:
//     NULL
//     
// Returns:
//     USB_SEL_OTG
//
//------------------------------------------------------------------------------
WORD BSPGetUSBControllerType(void)
{
    return USB_SEL_OTG;
}

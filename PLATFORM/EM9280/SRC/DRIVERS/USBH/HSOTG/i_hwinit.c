//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  i_hwinit.c
//  This file is linked to hwinit.c in the USBH Common
//
//
#define USB_HOST_MODE 3   // Host 1 high speed
#include <..\common\hwinit.c>

#ifdef DEBUG
//-----------------------------------------------------------------------------
//
//  Function: GetUSBPortType
//  This function is to return the USB port in string format.
//
//  Parameters:
//     NULL
//     
//  Returns:
//     Pointer to the USB Port in string format
//
//-----------------------------------------------------------------------------
const TCHAR *GetUSBPortType(void) 
{
   static const TCHAR* cszDeviceType = TEXT("H1 High Speed");
   return cszDeviceType;
}
#endif

//-----------------------------------------------------------------------------
//
//  Function: BSPGetUSBControllerType
//
//  This function is to return the USB Core Controller type in WORD format.
//  This is called by CSP public code when accessing the type of controller it
//  is using.
//
//  Parameters:
//     NULL
//     
//  Returns:
//     USB_SEL_H2
//
//-----------------------------------------------------------------------------
WORD BSPGetUSBControllerType(void)
{
    return USB_SEL_OTG;
}

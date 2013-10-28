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
// Copyright (C) 2004-2005, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//---------------------------------------------------------------------------
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  ddgpeuser.h
//
//  Debugging and some hardware specific macros
//
//------------------------------------------------------------------------------
#ifndef _DRIVERS_DISPLAY_DDLCDC_DDGPEUSER_H
#define _DRIVERS_DISPLAY_DDLCDC_DDGPEUSER_H


//------------------------------------------------------------------------------
// Defines

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef max3
#define max3(a, b, c)       max(max(a,b),c) // Get the biggest value of three data
#endif

#ifndef GPE_BACK_BUFFER
#define GPE_BACK_BUFFER     0x00000000 // NOT used in WindowCE 500
#endif
#ifndef DDFLIP_WAITNOTBUSY
#define DDFLIP_WAITNOTBUSY  DDFLIP_WAIT
#endif

// some macros to help in debugging
#ifdef DEBUG
#ifndef HAL_ZONE_INIT
#define HAL_ZONE_INIT     GPE_ZONE_INIT
#endif
#ifndef HAL_ZONE_CREATE
#define HAL_ZONE_CREATE   GPE_ZONE_CREATE
#endif
#ifndef HAL_ZONE_ERROR
#define HAL_ZONE_ERROR    GPE_ZONE_ERROR
#endif
#ifndef HAL_ZONE_WARNING
#define HAL_ZONE_WARNING  GPE_ZONE_WARNING
#endif
#endif

#ifndef DEBUGENTER
#define DEBUGENTER(func)                                        \
    {                                                           \
        DEBUGMSG(GPE_ZONE_ENTER,(TEXT("Entering function %s\r\n"),TEXT(#func)));        \
    }
#endif
#ifndef DEBUGLEAVE
#define DEBUGLEAVE(func)                                        \
    {                                                           \
        DEBUGMSG(GPE_ZONE_ENTER,(TEXT("Leaving function %s\r\n"),TEXT( #func )));   \
    }
#endif

#endif // _DRIVERS_DISPLAY_DDLCDC_DDGPEUSER_H


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Functions



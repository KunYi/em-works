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
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  owire.h
//
//  Definitions for One-Wire Driver
//
//------------------------------------------------------------------------------

#ifndef __OWIRE_H__
#define __OWIRE_H__

#include "csp.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// IOCTL to perform a software reset on the OWIRE
#define OWIRE_IOCTL_RESET                  1
// IOCTL to perform the reset sequence with
// reset pulse and presence pulse
#define OWIRE_IOCTL_RESET_PRESENCE_PULSE   2

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Functions
HANDLE OwireOpenHandle(void);
BOOL OwireCloseHandle(HANDLE);
BOOL OwireReset(HANDLE);
BOOL OwireResetPresencePulse(HANDLE);
BOOL OwireRead(HANDLE, BYTE *, DWORD);
BOOL OwireWrite(HANDLE, BYTE *, DWORD);

#ifdef __cplusplus
}
#endif

#endif   // __OWIRE_H__

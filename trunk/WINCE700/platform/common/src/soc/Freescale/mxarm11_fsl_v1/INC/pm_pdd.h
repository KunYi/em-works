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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pm_pdd.h
//
//  Definitions shared between the ARM11 common and BSP specific Power
//  Manager implementation.
//
//------------------------------------------------------------------------------

#ifndef __PM_PDD_H__
#define __PM_PDD_H__

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
//
// Type: PLATFORM_ACTIVITY_STATE, *PPLATFORM_ACTIVITY_STATE
//
// Describes the system activity states.  These are independent of factors
// such as AC power vs. battery, in cradle or not, etc.  OEMs may choose to add
// their own activity states if they customize this module.
//
//-----------------------------------------------------------------------------
typedef enum
{
    UserActive, UserInactive,
    UserIdle,
    SystemActive, SystemInactive,
    Suspend
} PLATFORM_ACTIVITY_STATE, *PPLATFORM_ACTIVITY_STATE;


#ifdef __cplusplus
}
#endif

#endif // __PM_PDD_H__

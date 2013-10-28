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
// Copyright (C) 2003-2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: emmadbg.h
//
// Debug zone definitions for the enhanced multimedia architecture driver
//
//------------------------------------------------------------------------------

#ifndef __EMMADBG_H__
#define __EMMADBG_H__

//------------------------------------------------------------------------------
// Defines

#ifdef DEBUG
#define DEBUGMASK(bit)       (1 << (bit))

// Debug zone bit positions
#define ZONEID_ERROR      0
#define ZONEID_WARN       1
#define ZONEID_INIT       2
#define ZONEID_FUNCTION   3
#define ZONEID_IOCTL      4
#define ZONEID_DEVICE     5

// Debug zone masks
#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)
#define ZONEMASK_WARN     (1<<ZONEID_WARN)
#define ZONEMASK_INIT     (1<<ZONEID_INIT)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_IOCTL    (1<<ZONEID_IOCTL)
#define ZONEMASK_DEVICE   (1<<ZONEID_DEVICE)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_IOCTL        DEBUGZONE(ZONEID_IOCTL)
#define ZONE_DEVICE       DEBUGZONE(ZONEID_DEVICE)

extern DBGPARAM dpCurSettings;
#endif //DEBUG

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Functions
#endif   // __EMMADBG_H_

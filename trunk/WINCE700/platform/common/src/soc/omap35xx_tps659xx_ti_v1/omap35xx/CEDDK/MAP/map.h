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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//------------------------------------------------------------------------------
//
//  File:  map.h
//
//  This file contains the data structure used to define to map the hardware
//  register regions onto to memory.


#ifndef _MAP_H_
#define _MAP_H_


//------------------------------------------------------------------------------
// This data structure is used to define a memory layout entry for a platform.
//
typedef struct {
    DWORD       dwStart;
    DWORD       dwSize;
    void       *pv;
} MEMORY_REGISTER_ENTRY;

#endif _MAP_H_
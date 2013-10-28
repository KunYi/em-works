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
 *    Module Name:  
 *       oemwake.h
 *
 *    Abstract:
 *       Platform-specific macros and prototypes related to system wake
 *    sources.  Platform-independent definitions are in nkintr.h.
 *
 */

#ifndef __OEMWAKE_H_
#define __OEMWAKE_H_

#include <nkintr.h>

DWORD OEMPowerManagerInit(void);
DWORD OEMSetWakeupSource( DWORD dwSources);
DWORD OEMResetWakeupSource( DWORD dwSources);
DWORD OEMGetWakeupSource(void);
void OEMClearIntSources(void);

// This is only used for using CPU interrupt as wake up sources.
void OEMIndicateIntSource(DWORD dwSources);
#endif

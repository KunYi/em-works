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
//
//  File:  guids.cpp
//
//  This file instantiates the GUIDs used by the wave driver.
//
//
// In all other files in the wave driver, INITGUID is not defined.
// This will cause the GUIDs to be declared as extern in the other 
// files, but in this file they will be instantiated.

#define INITGUID

#include <windows.h>
#include <wavedev.h>
#include <mmddk.h>
#include <mmreg.h>
#include <pm.h>
#include "debug.h"
#include "wavemain.h"
#include "audiolin.h"
#include "audioctrl.h"
#include "mixermgr.h"
#include "audiomgr.h"
#include "strmctxt.h"
#include "strmmgr.h"
#include "istrmmgr.h"
#include "ostrmmgr.h"
#include <memtxapi.h>
#include <WaveTopologyGuids.h>
#include "dasfaudioport.h"
#include "dmtaudioport.h"
#include "StreamClass.h"

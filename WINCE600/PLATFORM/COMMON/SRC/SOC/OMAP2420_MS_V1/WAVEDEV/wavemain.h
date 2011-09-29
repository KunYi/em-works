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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
#pragma once

#include <windows.h>
#include <wavedev.h>
#include <mmddk.h>
#include <devload.h>
#include <linklist.h>
#include <audiosys.h>
#include <wfmtmidi.h>

class StreamContext;
class WaveStreamContext;
class InputStreamContext;
class OutputStreamContext;
class DeviceContext;
class InputDeviceContext;
class OutputDeviceContext;
class HardwareContext;

#include "wavepdd.h"
#include "devctxt.h"
#include "hwctxt.h"
#include "omap2420.h"
#include "ceddk.h"
#include "ceddkex.h"
#include "wavext.h"
#include "dmactxt.h"
#include "strmctxt.h"
#include "midistrm.h"

//#undef DEBUGMSG
//#define DEBUGMSG(x, y) RETAILMSG(1,y)
//#define DEBUG

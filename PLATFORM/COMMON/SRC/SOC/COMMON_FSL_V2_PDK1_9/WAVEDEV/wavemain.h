//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
#pragma once
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4005 4115 4127 4201 4204 4214)
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <string.h>
#include <nkintr.h>
#include <excpt.h>
#include <wavedbg.h>
#include <wavedev.h>
#include <waveddsi.h>
#include <mmddk.h>
#include <cardserv.h>
#include <devload.h>

#include <linklist.h>
#undef RemoveEntryList
#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Entry;\
    _EX_Entry = (Entry);\
    _EX_Entry->Blink->Flink = _EX_Entry->Flink;\
    _EX_Entry->Flink->Blink = _EX_Entry->Blink;\
    }
#undef InsertTailList
#define InsertTailList(_ListHead,_Entry) {\
      PLIST_ENTRY _EX_ListHead = _ListHead; \
      PLIST_ENTRY _EX_Blink = _EX_ListHead->Blink; \
      (_Entry)->Flink = _EX_ListHead; \
      (_Entry)->Blink = _EX_Blink; \
      _EX_Blink->Flink = _Entry; \
      _EX_ListHead->Blink = _Entry; \
    }
#undef InsertHeadList
#define InsertHeadList(_ListHead,_Entry) {\
      PLIST_ENTRY _EX_ListHead = _ListHead; \
      PLIST_ENTRY _EX_Flink = _EX_ListHead->Flink; \
      (_Entry)->Flink = _EX_Flink; \
      (_Entry)->Blink = _EX_ListHead; \
      _EX_Flink->Blink = _Entry; \
      _EX_ListHead->Flink = _Entry; \
    }

#include <audiosys.h>
#include <wfmtmidi.h>
#pragma warning(pop)

class StreamContext;
class WaveStreamContext;
class InputStreamContext;
class OutputStreamContext;
class DeviceContext;
class MixerDeviceContext;
class InputDeviceContext;
class OutputDeviceContext;
class HardwareContext;

#include "wavepdd.h"
#include "devctxt.h"
#include "hwctxt.h"
// #include "recctxt.h"
#include "strmctxt.h"
#include "midistrm.h"
#include "mixerdrv.h"


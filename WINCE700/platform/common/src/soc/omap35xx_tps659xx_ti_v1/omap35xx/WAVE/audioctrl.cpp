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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//

#include <windows.h>
#include <wavedev.h>
#include <mmddk.h>
#include "debug.h"
#include "wavemain.h"
#include "audioctrl.h"
#include "audiolin.h"

//------------------------------------------------------------------------------
// CAudioLineBase
//
CAudioControlBase::CAudioControlBase() : m_hReference(NULL)
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CAudioControlBase()"));

    m_ControlId = -1;

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CAudioControlBase()"));
}


//------------------------------------------------------------------------------
// CAudioLineBase
//
void
CAudioControlBase::copy_ControlInfo(
    MIXERCONTROL *pControlInfo
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+copy_ControlInfo()"));

    pControlInfo->cbStruct = sizeof(MIXERCONTROL);
    pControlInfo->dwControlID = get_ControlId();
    pControlInfo->dwControlType = get_ControlType();
    pControlInfo->cMultipleItems = get_MultipleItemCount();
    pControlInfo->fdwControl = get_StatusFlag();

    wcscpy_s(pControlInfo->szName,_countof(pControlInfo->szName), get_Name());
    wcscpy_s(pControlInfo->szShortName,_countof(pControlInfo->szShortName), get_ShortName());

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-copy_ControlInfo()"));
}


//------------------------------------------------------------------------------
//
//  Base class volume audio control
//
DWORD
CAudioControlBase::put_Value(
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlags
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CAudioControlBase::get_Value()"));

    DWORD mmRet = get_AudioLine()->put_AudioValue(this, pDetail, dwFlags);

    DEBUGMSG(ZONE_FUNCTION,
        (L"WAV:-CAudioControlBase::get_Value(mmRet=%0x08)", mmRet)
        );

    return mmRet;
}


//------------------------------------------------------------------------------
//
//  Base class volume audio control
//
DWORD
CAudioControlBase::get_Value(
    PMIXERCONTROLDETAILS pDetail,
    DWORD dwFlags
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"WAV:+CAudioControlBase::get_Value()"));

    DWORD mmRet = get_AudioLine()->get_AudioValue(this, pDetail, dwFlags);

    DEBUGMSG(ZONE_FUNCTION, (L"WAV:-CAudioControlBase::get_Value()"));

    return mmRet;
}

//------------------------------------------------------------------------------


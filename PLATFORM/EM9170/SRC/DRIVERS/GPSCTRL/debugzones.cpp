//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include "windows.h"
#include "tchar.h"
#include "Log/DebugZones.h"

//using namespace GlobalLocate::GPSCtl::Log;

DBGPARAM dpCurSettings = 
{
    TEXT("GPS Control Driver"), 
    {
        TEXT("INIT"),TEXT("IOCTL"),TEXT(""),TEXT(""),
        TEXT(""),TEXT("MST_PowerUp"),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT("FUNCTION"),TEXT("WARNING"), TEXT("ERROR")
    },
    // By default, turn on the zones for INIT alone.
    ZONEMASK_INIT | ZONEMASK_OPEN
}; 

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
//------------------------------------------------------------------------------
//
//  File:  timer.c
//
#include <windows.h>
#include <oal_timer.h>
#include <oal_log.h>

//------------------------------------------------------------------------------
VOID OALTimerSetCount(UINT32 count);

//------------------------------------------------------------------------------
//
//  Function:  OALTimerCountsSinceSysTick
//
INT32 OALTimerCountsSinceSysTick()
{
    return OALTimerGetCompare() - OALTimerGetCount();
}

//------------------------------------------------------------------------------
//
//  Function:  OALTimerReduceSysTick
//
UINT32 OALTimerReduceSysTick(UINT32 count, UINT32 margin)
{
    UINT32 rc, edge, compare;

    edge = OALTimerGetCount();
    compare = OALTimerGetCompare();

    // Update the countdown value
    OALTimerSetCompare(count);

    if (edge > margin) 
    {
        // We are far enough from the interrupt that we should return the
        // number of ticks that would have occurred since the last tick based
        // on the new number of counts per tick. We will avoid the interrupt and
        // just start fresh.
        rc = (compare - edge) / count;
        OALTimerSetCount(count); 
    } 
    else {
        // We are close enough to the next interrupt that we will let the
        // interrupt occur. We will also return the number of counts that would
        // have happened since the last tick based on the new number of counts
        // per tick, but we will subtract one because another is just about to
        // happen.
        rc = (compare / count) - 1;
    }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALTimerExtendSysTick
//
VOID OALTimerExtendSysTick(UINT32 count, UINT32 margin, UINT32 offset)
{
    UINT32 compare, edge;

    edge = OALTimerGetCount();
    compare = OALTimerGetCompare();

    if (edge > ((count - compare) + margin + offset))
    {
        // We are far enough away from the interrupt that we should just
        // increase the time it will take to hit the next interrupt taking into
        // account the offset provided.
        OALTimerSetCount(edge - (count - offset - compare)); 
    }
    OALTimerSetCompare(count);
}

//------------------------------------------------------------------------------
//
//  Function:  OALTimerRecharge
//
//  Stubbed out since the SH4 countdown timers will automatically reload.
//
VOID OALTimerRecharge(UINT32 period, UINT32 margin)
{
    return;
}

//------------------------------------------------------------------------------

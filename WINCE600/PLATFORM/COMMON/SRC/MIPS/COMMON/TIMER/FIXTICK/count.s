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
//
#include <kxmips.h>

        .text

//------------------------------------------------------------------------------
//
//  Function:  OALTimerGetCount()
//
//  Returns actual counter value
//
LEAF_ENTRY(OALTimerGetCount)

        .set    noreorder
        .set    noat

        j       ra
        mfc0    v0, count

        .set    at
        .set    reorder
        .end    OALTimerGetCount

//------------------------------------------------------------------------------
//
//  Function:  OALTimerGetCompare()
//
//  Returns actual counter value
//
LEAF_ENTRY(OALTimerGetCompare)

        .set    noreorder
        .set    noat

        j       ra
        mfc0    v0, compare

        .set    at
        .set    reorder
        .end    OALTimerGetCompare


//------------------------------------------------------------------------------
//
//  Function:  OALTimerSetCompare()
//
//  Returns actual counter value
//
LEAF_ENTRY(OALTimerSetCompare)

        .set    noreorder
        .set    noat

        j       ra
        mtc0    a0, compare

        .set    at
        .set    reorder
        .end    OALTimerSetCompare

//------------------------------------------------------------------------------

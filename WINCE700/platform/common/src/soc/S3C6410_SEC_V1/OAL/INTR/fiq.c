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
//  File: interrupt.c
//
//  This file implement fast interrupt handler for S3C3210X SoC. It is
//  implemented in separate file to allow replacement in platform.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandlerFIQ
//
void OEMInterruptHandlerFIQ()
{
    return;
}

//------------------------------------------------------------------------------


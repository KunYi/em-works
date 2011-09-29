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
//  File:  priority.c
//
#include <windows.h>

//------------------------------------------------------------------------------
//
//  Global:  IntrPriority
//
//  This table is used by kernel to deside which interrupt should get priority
//  in situation when multiple interrupts requests are pending. It contains
//  one entry for each combination of bits in the 6-bit interrupt field. 
//  The entry defines which interrupt should be serviced. The value is
//  the bit number multiplied by four. 
//
UINT8 IntrPriority[] = {
    -4,                             // 000 000 for spurious interrupt
    0,                              // 000 001
    1*4,                            // 000 010
    0,                              // 000 011
    2*4,                            // 000 100
    0,                              // 000 101
    1*4,                            // 000 110
    0,                              // 000 111
    3*4,                            // 001 000
    0, 1*4, 0, 2*4, 0, 1*4, 0,
    4*4,                            // 010 000
    0, 1*4, 0, 2*4, 0, 1*4, 0,
    3*4,                            // 011 000
    0, 1*4, 0, 2*4, 0, 1*4, 0,
    5*4,                            // 100 000
    0, 1*4, 0, 2*4, 0, 1*4, 0,
    3*4,                            // 101 000
    0, 1*4, 0, 2*4, 0, 1*4, 0,
    4*4,                            // 110 000
    0, 1*4, 0, 2*4, 0, 1*4, 0,
    3*4,                            // 111 000
    0, 1*4, 0, 2*4, 0, 1*4, 0
};

//------------------------------------------------------------------------------
//
//  Global:  IntrPriority
//
//  This table tells the kernel which interrupt mask to set in the CPU
//  during the execution of the interrupt service routine. A one indicates
//  the interrupt at that bit is masked.
//
UINT8 IntrMask[] = {
    0x00,                           // 00 0000 for spurious interrupt
    0x3f,                           // 00 0001
    0x3e,                           // 00 0010
    0x3c,                           // 00 0100
    0x38,                           // 00 1000
    0x30,                           // 01 0000
    0x20,                           // 10 0000
    0x00                            // padding
};

//------------------------------------------------------------------------------

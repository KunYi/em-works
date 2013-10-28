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
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: emma.h
//
// Definitions for eMMA (Post-Processor and Pre-Processor)
//
//------------------------------------------------------------------------------
#ifndef __EMMA_H__
#define __EMMA_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines 

//------------------------------------------------------------------------------
// Local Variables
//
// Color conversion coefficient table
//
// RGB to YUV
static const UINT8 rgb2yuv_tbl[4][10] = {
    //  0     1     2     3     4     5     6     7     8  x0 
    {0x4D ,0x4B ,0x3A ,0x57 ,0x55 ,0x40 ,0x40 ,0x6B ,0x29 ,0},  // A1
    {0x42 ,0x41 ,0x32 ,0x4C ,0x4A ,0x38 ,0x38 ,0x5E ,0x24 ,1},  // A0
    {0x36 ,0x5C ,0x25 ,0x3B ,0x63 ,0x40 ,0x40 ,0x74 ,0x18 ,0},  // B1
    {0x2F ,0x4F ,0x20 ,0x34 ,0x57 ,0x38 ,0x38 ,0x66 ,0x15 ,1}   // B0
};

// YUV to RGB
static const UINT16 yuv2rgb_tbl[4][6] = {
    //  0     1     2     3     4   x0 
    {0x80 ,0xB4 ,0x2C ,0x5B ,0xE3  ,0}, // A1
    {0x95 ,0xCC ,0x32 ,0x68 ,0x102 ,1}, // A0
    {0x80 ,0xCA ,0xF0 ,0x3C ,0xED  ,0}, // B1
    {0x95 ,0xE6 ,0x1B ,0x44 ,0x10E ,1}  // B0
};

#ifdef __cplusplus
}
#endif

#endif // __EMMA_H__


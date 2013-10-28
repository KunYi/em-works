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
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: inc/slcdc.h
//
//  Header file for slcdc driver.
//
//------------------------------------------------------------------------------
#ifndef _INC_SLCDC_H_
#define _INC_SLCDC_H_

//------------------------------------------------------------------------------
// MACRO DEFINITIONS 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
extern SCODE BspSetMode(int modeId, HPALETTE *pPalette);
extern SCODE BspGetModeInfo(GPEMode *pMode, int modeNo);
extern int BspNumModes(void);
extern void BspPowerHandler(BOOL bOff);
extern SCODE BspBltComplete(void);

extern void BSPSlcdcClassInit(void);
extern void BSPSlcdcClassDeInit(void);

extern ULONG *BSPGetSLCDCBitMasks(void);
extern UINT16 *BspGetVideoMem(void);
extern GPEMode BspGetMode(void);

#endif   /* _INC_SLCDC_H_ */

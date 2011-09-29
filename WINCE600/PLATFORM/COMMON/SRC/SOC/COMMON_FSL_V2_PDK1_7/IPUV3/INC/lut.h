//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  lut.h
//
//  Look-Up Table memory interface definitions
//
//-----------------------------------------------------------------------------

#ifndef __LUT_H__
#define __LUT_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Functions
BOOL LUTRegsInit();
void LUTRegsCleanup();
BOOL LUTWriteSingleEntry(DWORD dwIndex, DWORD dwLUTEntry);
BOOL LUTWriteRangeOfEntries(DWORD dwStartIndex, DWORD dwEndIndex, DWORD *pdwLUTEntries);

#ifdef __cplusplus
}
#endif

#endif //__LUT_H__


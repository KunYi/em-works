//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: updatesb.h
//
//
//------------------------------------------------------------------------------

#ifndef __UPDATESB_H
#define __UPDATESB_H

//------------------------------------------------------------------------------

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
#define UPDATE_SIGN     0x546F6E79

#define RoundToNearestDWORD(x)  (((x) + 3) & (~3))

#define MUTEX_NAME  L"SB_UPDATE_FROM_SD"

#define BUFFER_SIZE     (1*1024*1024)     //1MB
//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

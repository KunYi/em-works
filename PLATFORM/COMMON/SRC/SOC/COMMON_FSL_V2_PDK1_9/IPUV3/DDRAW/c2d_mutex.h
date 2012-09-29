//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  c2d_mutex.h
//
//  Header file of c2d mutex.
//
//------------------------------------------------------------------------------

#ifndef __C2D_MUTEX_H
#define __C2D_MUTEX_H

#include <windows.h>


extern HANDLE g_C2DMutex;
extern const LPCWSTR g_C2DMutexName;

#define C2D_ENTER OpenMutex(g_C2DMutexName, &g_C2DMutex)
#define C2D_EXIT  ReleaseMutex(g_C2DMutex)


void OpenMutex(LPCWSTR name, HANDLE* handle);

#endif  

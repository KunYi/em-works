//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  c2d_mutex.cpp
//
//  Implementation of c2d mutex.
//
//------------------------------------------------------------------------------

#if defined(USE_C2D_ROUTINES)

#include "c2d_mutex.h"

const LPCWSTR  g_C2DMutexName = TEXT("{D84F7812-95F4-4a8b-9D88-330B578A7758}");
HANDLE         g_C2DMutex     = 0;


void OpenMutex(LPCWSTR name, HANDLE* handle)
{
    if (*handle == 0)
    {
        *handle = CreateMutex(NULL, FALSE, name); 
    }
    if (*handle != 0)
    {            
        DWORD retVal = WaitForSingleObject(*handle, INFINITE);
        if (retVal == WAIT_FAILED)
        {
            ERRORMSG(TRUE, (TEXT("C2DGPE::OpenMutex - Mutex wait failed.\r\n")));
        }
    }
    else
    {
        ERRORMSG(TRUE, (TEXT("C2DGPE::OpenMutex - Failed to create mutex.\r\n")));
    }        
}
#endif    //end #if defined(USE_C2D_ROUTINES)

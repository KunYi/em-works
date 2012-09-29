//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable:4201)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#include <winddi.h>
#include <ddgpe.h>
#pragma warning(pop)

#include "cameradbg.h"

BOOL
__stdcall
DllMain(
   HANDLE   hDllHandle, 
   DWORD    dwReason, 
   VOID   * lpReserved
   ) 
{
    BOOL bRc = TRUE;
    
    UNREFERENCED_PARAMETER( lpReserved );
    
    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH: 
        {
           DEBUGREGISTER( reinterpret_cast<HMODULE>( hDllHandle ) );
           DEBUGMSG( ZONE_INIT, ( _T("*** DLL_PROCESS_ATTACH - Current Process: 0x%x, ID: 0x%x ***\r\n"), GetCurrentProcess(), GetCurrentProcessId() ) );

           DisableThreadLibraryCalls( reinterpret_cast<HMODULE>( hDllHandle ) );

           break;
        } 

        case DLL_PROCESS_DETACH: 
        {
            DEBUGMSG( ZONE_INIT, ( _T("*** DLL_PROCESS_DETACH - Current Process: 0x%x, ID: 0x%x ***\r\n"), GetCurrentProcess(), GetCurrentProcessId() ) );

            break;
        } 
            
        case DLL_THREAD_DETACH:
        {
            break;
        }
            
        case DLL_THREAD_ATTACH:
        {
            break;
        }
            
        default:
        {
            break;
        }
    }
    
    return bRc;
}

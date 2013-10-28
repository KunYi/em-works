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

//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

    dmaAdpt.cpp

Abstract:  

    Contains DMA support routines for Adapter.
    
Functions:

Notes:

--*/
#include <windows.h>
#include <oaldma.h>
#include <dma_adpt.hpp>
#include <dma_chn.hpp>
#include <dma_tran.hpp>
/* Debug Zones.
 */
#ifdef DEBUG

#define DBG_INIT    0x0001
#define DBG_OPEN    0x0002
#define DBG_READ    0x0004
#define DBG_WRITE   0x0008
#define DBG_CLOSE   0x0010
#define DBG_IOCTL   0x0020
#define DBG_THREAD  0x0040
#define DBG_EVENTS  0x0080
#define DBG_CRITSEC 0x0100
#define DBG_IO      0x0200
#define DBG_MAXIO   0x0400
#define DBG_UNUSED2 0x0800
#define DBG_ALLOC   0x1000
#define DBG_FUNCTION 0x2000
#define DBG_WARNING 0x4000
#define DBG_ERROR   0x8000

extern "C"
DBGPARAM dpCurSettings = {
     TEXT("DMA"), {
     TEXT("Init"),TEXT("Open"),TEXT("Read"),TEXT("Write"),
     TEXT("Close"),TEXT("Ioctl"),TEXT("Thread"),TEXT("Events"),
     TEXT("CritSec"),TEXT("IO"),TEXT("PDD"),TEXT("Unused2"),
     TEXT("Alloc"),TEXT("Function"),TEXT("Warning"),TEXT("Error") },
     DBG_ERROR 
};
#endif


DmaAdapter * g_pDmaAdapter = NULL ;

/*
 @doc INTERNAL
 @func    BOOL | DllMain | Process attach/detach api.
 *
 @rdesc The return is a BOOL, representing success (TRUE) or failure (FALSE).
 */
extern "C"
BOOL WINAPI
DllEntry(
     HINSTANCE hinstDll,             /*@parm Instance pointer. */
     DWORD     dwReason,                  /*@parm Reason routine is called. */
     LPVOID    lpReserved                      /*@parm system parameter. */
     )
{
     if ( dwReason == DLL_PROCESS_ATTACH ) {
          DEBUGREGISTER(hinstDll);
          DEBUGMSG (ZONE_INIT, (TEXT("DMA process attach\r\n")));
        DisableThreadLibraryCalls((HMODULE) hinstDll);
     }

     if ( dwReason == DLL_PROCESS_DETACH      ) {
          DEBUGMSG (ZONE_INIT, (TEXT("DMA process detach called\r\n")));
     }

     return TRUE;
}


extern "C" DWORD
DMA_Init (LPCTSTR lpActivePath)
{
    DmaAdapter * pDmaAdapter = new DmaAdapter(lpActivePath);
    if (pDmaAdapter && pDmaAdapter->Init())
        return (DWORD)pDmaAdapter;
    else {
        if (pDmaAdapter)
            delete pDmaAdapter;
        return 0;
    }
}
extern "C" void
DMA_Deinit (DWORD dwData)
{
    DmaAdapter * pDmaAdapter = (DmaAdapter * )dwData;
    if (pDmaAdapter)
        delete pDmaAdapter;
}
extern "C" BOOL 
DMA_PowerUp(DWORD dwData)
{
    DmaAdapter * pDmaAdapter = (DmaAdapter * )dwData;
        pDmaAdapter->PowerMgmtCallback(FALSE);
    return TRUE;
}
extern "C" BOOL 
DMA_PowerDown(DWORD dwData)
{
    DmaAdapter * pDmaAdapter = (DmaAdapter * )dwData;
    if (pDmaAdapter)
        pDmaAdapter->PowerMgmtCallback(TRUE);
    return TRUE;
}

extern "C" HANDLE
DMA_Open(
        HANDLE  pHead,          // @parm Handle returned by COM_Init.
        DWORD   AccessCode,     // @parm access code.
        DWORD   ShareMode       // @parm share mode - Not used in this driver.
        )
{
    return pHead;
}
extern "C" BOOL
DMA_Close(HANDLE pOpenHead)
{
    return TRUE;
}
extern "C" BOOL
DMA_IOControl(HANDLE pOpenHead,
              DWORD dwCode, PBYTE pBufIn,
              DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
              PDWORD pdwActualOut)
{
    DmaAdapter *  pDmaAdapter = (DmaAdapter * )pOpenHead;
    if (GetDirectCallerProcessId() != GetCurrentProcessId()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
        
    if (pDmaAdapter)
        return pDmaAdapter->IOControl(dwCode,pBufIn,dwLenIn,pBufOut,dwLenOut,pdwActualOut);
    return FALSE;
}


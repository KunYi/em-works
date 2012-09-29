//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//--------------------------------------------------------------------------
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//--------------------------------------------------------------------------
//
// Module Name:
//     hcddrv.cpp
//
// Abstract:
//
// Notes:
//

#include "globals.hpp"
#include "Hcd.hpp"
#include <cphysmem.hpp>
#include <cardserv.h>
#include <devload.h>

/*
#undef DEBUGMSG
#define DEBUGMSG(x,y) NKDbgPrintfW y
#undef RETAILMSG
#define RETAILMSG(x,y) NKDbgPrintfW y
*/
// Debug Zones.
#ifdef DEBUG

// Note: the debug zones are already defined in PUBLIC\COMMON\OAK\INC\uhcdddsi.h
//       and PUBLIC\COMMON\OAK\INC\hcdddsi.h. We just need to ensure that the
//       dpCurSettings definitions match what is given in those header files.


DBGPARAM dpCurSettings = {
    TEXT("USB HCD"),
    {
        TEXT("Uhcd"),
        TEXT("Init"),
        TEXT("Registers"),
        TEXT("Hub"),
        TEXT("Attach"),
        TEXT("Descriptors"),
        TEXT("Function"),
        TEXT("Pipe"),
        TEXT("Transfer"),
        TEXT("QH"),
        TEXT("TD"),
        TEXT("CPhysMem"),
        TEXT("Verbose"),
        TEXT("Warning"),
        TEXT("Error"),
        TEXT("Unused")
    },
    0x6012 // Default to ZONE_INIT | ZONE_ATTACH | ZONE_WARNING | ZONE_ERROR
};
#endif

extern "C" HINSTANCE g_hInstance;
DWORD g_IstThreadPriority;
extern "C"
DWORD g_dwContext = 0 ;
CRITICAL_SECTION    g_CSection;

extern HCD_FUNCS gc_HcdFuncs =
{
    sizeof(HCD_FUNCS),      //DWORD                   dwCount;

    &HcdGetFrameNumber,     //LPHCD_GET_FRAME_NUMBER      lpGetFrameNumber;
    &HcdGetFrameLength,     //LPHCD_GET_FRAME_LENGTH      lpGetFrameLength;
    &HcdSetFrameLength,     //LPHCD_SET_FRAME_LENGTH      lpSetFrameLength;
    &HcdStopAdjustingFrame, //LPHCD_STOP_ADJUSTING_FRAME  lpStopAdjustingFrame;
    &HcdOpenPipe,           //LPHCD_OPEN_PIPE             lpOpenPipe;
    &HcdClosePipe,          //LPHCD_CLOSE_PIPE            lpClosePipe;
    &HcdResetPipe,          //LPHCD_RESET_PIPE            lpResetPipe;
    &HcdIsPipeHalted,       //LPHCD_IS_PIPE_HALTED        lpIsPipeHalted;
    &HcdIssueTransfer,      //LPHCD_ISSUE_TRANSFER        lpIssueTransfer;
    &HcdAbortTransfer,      //LPHCD_ABORT_TRANSFER        lpAbortTransfer;
    &HcdDisableDevice,      //LPHCD_DISABLE_DEVICE        lpDisableDevice;
    &HcdSuspendResume       //LPHCD_SUSPEND_RESUME        lpSuspendResume;
};

BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    if ( dwReason == DLL_PROCESS_ATTACH ) {
        DEBUGREGISTER((HINSTANCE)hinstDLL);
        DEBUGMSG (ZONE_INIT,(TEXT("HCD driver DLL attach\r\n")));
        DisableThreadLibraryCalls((HMODULE) hinstDLL);
        g_dwContext = 0;
        InitializeCriticalSection( &g_CSection );
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        DeleteCriticalSection( &g_CSection );
    }
    return HcdPdd_DllMain(hinstDLL, dwReason, lpvReserved);
}

BOOL HcdGetFrameNumber(LPVOID lpvHcd, LPDWORD lpdwFrameNumber)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->GetFrameNumber(lpdwFrameNumber);
}

BOOL HcdGetFrameLength(LPVOID lpvHcd, LPUSHORT lpuFrameLength)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->GetFrameLength(lpuFrameLength);
}

BOOL HcdSetFrameLength(LPVOID lpvHcd, HANDLE hEvent, USHORT uFrameLength)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->SetFrameLength(hEvent, uFrameLength);
}

BOOL HcdStopAdjustingFrame(LPVOID lpvHcd)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->StopAdjustingFrame();
}

BOOL HcdOpenPipe(LPVOID lpvHcd, UINT iDevice,
        LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
        LPUINT lpiEndpointIndex)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->OpenPipe(iDevice, lpEndpointDescriptor, lpiEndpointIndex);
}

BOOL HcdClosePipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->ClosePipe(iDevice, iEndpointIndex);
}

BOOL HcdResetPipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->ResetPipe(iDevice, iEndpointIndex);
}

BOOL HcdIsPipeHalted(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex, LPBOOL lpbHalted)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->IsPipeHalted(iDevice, iEndpointIndex, lpbHalted);
}

BOOL HcdIssueTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
        DWORD dwFlags, LPCVOID lpvControlHeader,
        DWORD dwStartingFrame, DWORD dwFrames, LPCDWORD aLengths,
        DWORD dwBufferSize, LPVOID lpvBuffer, ULONG paBuffer,
        LPCVOID lpvCancelId, LPDWORD adwIsochErrors, LPDWORD adwIsochLengths,
        LPBOOL lpfComplete, LPDWORD lpdwBytesTransfered, LPDWORD lpdwError)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->IssueTransfer(iDevice, iEndpointIndex, lpStartAddress,
            lpParameter, dwFlags, lpvControlHeader, dwStartingFrame, dwFrames,
            aLengths, dwBufferSize, lpvBuffer, paBuffer, lpvCancelId,
            adwIsochErrors, adwIsochLengths,lpfComplete, lpdwBytesTransfered,
            lpdwError);
}

BOOL HcdAbortTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
        LPCVOID lpvCancelId)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->AbortTransfer(iDevice, iEndpointIndex, lpStartAddress,
            lpParameter, lpvCancelId);
}

BOOL HcdDisableDevice(LPVOID lpvHcd, UINT iDevice, BOOL fReset)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->DisableDevice(iDevice,fReset);
}

BOOL HcdSuspendResume(LPVOID lpvHcd, UINT iDevice, BOOL fSuspend)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->SuspendResume(iDevice, fSuspend);
}


// -----------------------------------------------------------------------------
// Function to read the interrupt thread priority from the registry.
// If it is not in the registry then a default value is returned.
// -----------------------------------------------------------------------------
static DWORD
GetInterruptThreadPriority(
    LPWSTR lpszActiveKey
    )
{
    HKEY hDevKey;
    DWORD dwValType;
    DWORD dwValLen;
    DWORD dwPrio;

    dwPrio = DEFAULT_UHCD_IST_PRIORITY;

    hDevKey = OpenDeviceKey(lpszActiveKey);
    if (hDevKey) {
        dwValLen = sizeof(DWORD);
        RegQueryValueEx(
            hDevKey,
            TEXT("Priority256"),
            NULL,
            &dwValType,
            (PUCHAR)&dwPrio,
            &dwValLen);
        RegCloseKey(hDevKey);
    }
    return dwPrio;
}


extern "C" LPVOID HcdMdd_CreateMemoryObject(DWORD cbSize, DWORD cbHighPrioritySize,
                                             PUCHAR pVirtAddr, PUCHAR pPhysAddr)
{
    //
    // We need at least a USBPAGE for Special allocation and a PAGE for normal
    // allocation.
    //
    ASSERT((cbHighPrioritySize + (2*USBPAGESIZE)) < cbSize);

    CPhysMem * pobMem = new CPhysMem(cbSize,cbHighPrioritySize,pVirtAddr,pPhysAddr);
    if (pobMem)
        if ( ! pobMem->InittedOK() ) {
            delete pobMem;
            pobMem = 0;
        }

    return pobMem;
}

extern "C" LPVOID HcdMdd_CreateHcdObject_Customized(LPVOID lpvUhcdPddObject,
        LPVOID lpvMemoryObject, DWORD dwOffset,  LPCWSTR szRegKey, PUCHAR ioPortBase,
        DWORD dwSysIntr)
{
    CHcd * pobUhcd = CreateHCDObject(lpvUhcdPddObject,(CPhysMem *)lpvMemoryObject, dwOffset, szRegKey,ioPortBase,dwSysIntr);
    
    if ( pobUhcd != NULL ) {
        if ( !pobUhcd->DeviceInitialize())  // This is where USB thread are created
        {
            delete pobUhcd;
            pobUhcd = NULL;
        }
    }

    return pobUhcd;
}

extern "C" BOOL HcdMdd_DestroyHcdObject(LPVOID lpvUhcdObject)
{
    CHcd * pobUhcd = (CHcd *)lpvUhcdObject;
    delete pobUhcd;

    return TRUE;
}

extern "C" BOOL HcdMdd_DestroyMemoryObject(LPVOID lpvMemoryObject)
{
    CPhysMem * pobMem = (CPhysMem *)lpvMemoryObject;
    delete pobMem;

    return TRUE;
}

extern "C" BOOL HcdMdd_PowerUp(LPVOID lpvUhcdObject)
{
    CHcd * pobUhcd = (CHcd *)lpvUhcdObject;

    DEBUGMSG(ZONE_ATTACH, (TEXT("USB2COM:HcdMdd_PowerUp called\r\n")));
    pobUhcd->PowerMgmtCallback(FALSE);

    return TRUE;
}

extern "C" BOOL HcdMdd_PowerDown(LPVOID lpvUhcdObject)
{
    CHcd * pobUhcd = (CHcd *)lpvUhcdObject;
    pobUhcd->PowerMgmtCallback(TRUE);

    return TRUE;
}

extern "C"  DWORD   HcdMdd_SetCapability (LPVOID lpvUhcdObject, DWORD dwCapability )
{
    CHcd * pobUhcd = (CHcd *)lpvUhcdObject;
    return pobUhcd->SetCapability(dwCapability);
}

extern "C" BOOL HcdMdd_SetSubHubTestMode(LPVOID lpvUhcdObject, const WORD port, const WORD testmode)
{
    CHcd * pobUhcd = (CHcd *)lpvUhcdObject;
    RETAILMSG(1,(TEXT("HcdMdd_SetSubHubTestMode\r\n")));
    pobUhcd->WakeUpSysIntr();
    Sleep(10);
    return pobUhcd->GetRootHub()->SubHubTestModeEnable(port,testmode);
}

// Stream functions
extern "C" DWORD HCD_Init(DWORD dwContext)
{
    HKEY ActiveKey;
    WCHAR RegKeyPath[DEVKEY_LEN];
    DWORD status;
    DWORD ValType;
    DWORD ValLen;

    DEBUGMSG (ZONE_INIT, (TEXT("EHCD!HCD_Init\r\n")));
    
    // Open driver's ACTIVE key
    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                (LPCWSTR)dwContext,
                0,
                0,
                &ActiveKey);
    if (status != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR,
            (TEXT("EHCD!HCD_Init RegOpenKeyEx(%s) returned %d.\r\n"),
            (LPCWSTR)dwContext, status));
        return NULL;
    }

    // Get Key value, which points to driver's key
    ValLen = sizeof(RegKeyPath);
    status = RegQueryValueEx(
                ActiveKey,
                DEVLOAD_DEVKEY_VALNAME,
                NULL,
                &ValType,
                (PUCHAR)RegKeyPath,
                &ValLen);
    if (status != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR,
            (TEXT("EHCD!HCD_Init RegQueryValueEx(%s\\%s) returned %d\r\n"),
            (LPCWSTR)dwContext, DEVLOAD_DEVKEY_VALNAME, status));
        RegCloseKey(ActiveKey);
        return NULL;
    }

    RegCloseKey(ActiveKey);
    g_IstThreadPriority = GetInterruptThreadPriority((LPTSTR)dwContext);
    RegKeyPath[DEVKEY_LEN-1] = 0 ;
    
    EnterCriticalSection( &g_CSection );
    g_dwContext = dwContext;
    DWORD dwReturn = HcdPdd_Init((DWORD)RegKeyPath);
    g_dwContext = 0;
    LeaveCriticalSection( &g_CSection );
    
    return dwReturn;
}


extern "C" void HCD_PowerUp(DWORD hDeviceContext)
{
    HcdPdd_PowerUp(hDeviceContext);

    return;
}


extern "C" void HCD_PowerDown(DWORD hDeviceContext)
{
    HcdPdd_PowerDown(hDeviceContext);

    return;
}


extern "C" BOOL HCD_Deinit(DWORD hDeviceContext)
{
    DEBUGMSG (ZONE_INIT, (TEXT("UHCD: HCD_Deinit\r\n")));
    return HcdPdd_Deinit(hDeviceContext);
}


extern "C" DWORD HCD_Open(DWORD hDeviceContext, DWORD AccessCode,
        DWORD ShareMode)
{
    return HcdPdd_Open(hDeviceContext, AccessCode, ShareMode);
}


extern "C" BOOL HCD_Close(DWORD hOpenContext)
{
    return HcdPdd_Close(hOpenContext);
}


extern "C" DWORD HCD_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    return HcdPdd_Read(hOpenContext, pBuffer, Count);
}


extern "C" DWORD HCD_Write(DWORD hOpenContext, LPCVOID pSourceBytes,
        DWORD NumberOfBytes)
{
    return HcdPdd_Write(hOpenContext, pSourceBytes, NumberOfBytes);
}


extern "C" DWORD HCD_Seek(DWORD hOpenContext, LONG Amount, DWORD Type)
{
    return HcdPdd_Seek(hOpenContext, Amount, Type);
}


extern "C" BOOL HCD_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
        DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    return HcdPdd_IOControl(hOpenContext, dwCode, pBufIn, dwLenIn, pBufOut,
            dwLenOut, pdwActualOut);
}


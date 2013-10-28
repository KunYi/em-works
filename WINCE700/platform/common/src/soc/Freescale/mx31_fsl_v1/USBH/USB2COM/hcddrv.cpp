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
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//--------------------------------------------------------------------------
// Copyright (C) 2007, Freescale Semiconductor, Inc. All Rights Reserved.
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

#ifdef IRAM_PATCH
static CHcd* g_pHcd = NULL;
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

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdGetFrameNumber(LPVOID lpvHcd, LPDWORD lpdwFrameNumber)
#else
BOOL HcdGetFrameNumber(LPVOID lpvHcd, LPDWORD lpdwFrameNumber)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->GetFrameNumber(lpdwFrameNumber);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdGetFrameLength(LPVOID lpvHcd, LPUSHORT lpuFrameLength)
#else
BOOL HcdGetFrameLength(LPVOID lpvHcd, LPUSHORT lpuFrameLength)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->GetFrameLength(lpuFrameLength);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdSetFrameLength(LPVOID lpvHcd, HANDLE hEvent, USHORT uFrameLength)
#else
BOOL HcdSetFrameLength(LPVOID lpvHcd, HANDLE hEvent, USHORT uFrameLength)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->SetFrameLength(hEvent, uFrameLength);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdStopAdjustingFrame(LPVOID lpvHcd)
#else
BOOL HcdStopAdjustingFrame(LPVOID lpvHcd)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->StopAdjustingFrame();
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdOpenPipe(LPVOID lpvHcd, UINT iDevice,
        LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
        LPUINT lpiEndpointIndex)
#else
BOOL HcdOpenPipe(LPVOID lpvHcd, UINT iDevice,
        LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
        LPUINT lpiEndpointIndex)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->OpenPipe(iDevice, lpEndpointDescriptor, lpiEndpointIndex);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdClosePipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex)
#else
BOOL HcdClosePipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->ClosePipe(iDevice, iEndpointIndex);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdResetPipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex)
#else
BOOL HcdResetPipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->ResetPipe(iDevice, iEndpointIndex);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdIsPipeHalted(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex, LPBOOL lpbHalted)
#else
BOOL HcdIsPipeHalted(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex, LPBOOL lpbHalted)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->IsPipeHalted(iDevice, iEndpointIndex, lpbHalted);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdIssueTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
        DWORD dwFlags, LPCVOID lpvControlHeader,
        DWORD dwStartingFrame, DWORD dwFrames, LPCDWORD aLengths,
        DWORD dwBufferSize, LPVOID lpvBuffer, ULONG paBuffer,
        LPCVOID lpvCancelId, LPDWORD adwIsochErrors, LPDWORD adwIsochLengths,
        LPBOOL lpfComplete, LPDWORD lpdwBytesTransfered, LPDWORD lpdwError)
#else
BOOL HcdIssueTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
        DWORD dwFlags, LPCVOID lpvControlHeader,
        DWORD dwStartingFrame, DWORD dwFrames, LPCDWORD aLengths,
        DWORD dwBufferSize, LPVOID lpvBuffer, ULONG paBuffer,
        LPCVOID lpvCancelId, LPDWORD adwIsochErrors, LPDWORD adwIsochLengths,
        LPBOOL lpfComplete, LPDWORD lpdwBytesTransfered, LPDWORD lpdwError)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->IssueTransfer(iDevice, iEndpointIndex, lpStartAddress,
            lpParameter, dwFlags, lpvControlHeader, dwStartingFrame, dwFrames,
            aLengths, dwBufferSize, lpvBuffer, paBuffer, lpvCancelId,
            adwIsochErrors, adwIsochLengths,lpfComplete, lpdwBytesTransfered,
            lpdwError);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdAbortTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
        LPCVOID lpvCancelId)
#else
BOOL HcdAbortTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
        LPCVOID lpvCancelId)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->AbortTransfer(iDevice, iEndpointIndex, lpStartAddress,
            lpParameter, lpvCancelId);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdDisableDevice(LPVOID lpvHcd, UINT iDevice, BOOL fReset)
#else
BOOL HcdDisableDevice(LPVOID lpvHcd, UINT iDevice, BOOL fReset)
#endif
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    return pHcd->DisableDevice(iDevice,fReset);
}

#if 0  // Remove-W4: Warning C4211 workaround
static BOOL HcdSuspendResume(LPVOID lpvHcd, UINT iDevice, BOOL fSuspend)
#else
BOOL HcdSuspendResume(LPVOID lpvHcd, UINT iDevice, BOOL fSuspend)
#endif
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

#ifdef IRAM_PATCH
extern "C" void HcdMdd_ReInitIRamMemObject(LPVOID lpvIRamDataObject, LPVOID lpvIRamEleObject)
{
    UNREFERENCED_PARAMETER(lpvIRamEleObject);
    RETAILMSG(1, (L"we reinit the 2 iram section here\r\n"));
    ((CPhysMem*)lpvIRamDataObject)->ReInit();
}
#endif

#ifdef IRAM_PATCH
extern "C" LPVOID HcdMdd_CreateHcdIRamObject(LPVOID lpvUhcdPddObject,
        LPVOID* lpvMemoryObject, LPCWSTR szRegKey, PUCHAR ioPortBase,
        DWORD dwSysIntr)
{
    CHcd * pobUhcd = CreateHCDObject(lpvUhcdPddObject,(CPhysMem**)lpvMemoryObject,szRegKey,ioPortBase,dwSysIntr);
    if ( pobUhcd != NULL ) {
        if ( !pobUhcd->DeviceInitialize( )) {
            delete pobUhcd;
            pobUhcd = NULL;
        }
    }
    g_pHcd = pobUhcd;

    return pobUhcd;
}
#else
extern "C" LPVOID HcdMdd_CreateHcdObject(LPVOID lpvUhcdPddObject,
        LPVOID lpvMemoryObject, LPCWSTR szRegKey, PUCHAR ioPortBase,
        DWORD dwSysIntr)
{
    CHcd * pobUhcd = CreateHCDObject(lpvUhcdPddObject,(CPhysMem *)lpvMemoryObject,szRegKey,ioPortBase,dwSysIntr);

    if ( pobUhcd != NULL ) {
        if ( !pobUhcd->DeviceInitialize( )) {
            delete pobUhcd;
            pobUhcd = NULL;
        }
    }

    return pobUhcd;
}
#endif

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
    DWORD dwReturn =HcdPdd_Init((DWORD)RegKeyPath);
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
#ifdef HCD_IOCTRL
    HcdPdd_Open(hDeviceContext, AccessCode, ShareMode);
    return hDeviceContext;
#else
    return HcdPdd_Open(hDeviceContext, AccessCode, ShareMode);
#endif
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
#ifdef HCD_IOCTRL
    BOOL bRet = FALSE;
    switch (dwCode) 
    {
        case IOCTL_POWER_CAPABILITIES:
            if (pBufOut != NULL  && dwLenOut >= sizeof(POWER_CAPABILITIES)  && pdwActualOut != NULL)  {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                memset(ppc, 0, sizeof(*ppc));              
                ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D4);
                *pdwActualOut = sizeof(*ppc);
                bRet = TRUE;
            }
            goto EXIT;
            break;

        case IOCTL_POWER_SET: 
            if(pBufOut != NULL  && dwLenOut == sizeof(CEDEVICE_POWER_STATE)  && pdwActualOut != NULL) {
                CEDEVICE_POWER_STATE dx = *(PCEDEVICE_POWER_STATE) pBufOut;
                if (VALID_DX(dx))  {
                    // Any request that is not D0 becomes a D4 request
                    if (dx ==D0) 
                    {
                        RETAILMSG(1, (L"HCD D0 state\r\n"));
                    }
                    else
                    {
                        RETAILMSG(1, (L"HCD D4 state\r\n"));
                        //g_pHcd->RemoveAllFromBusyPipeList();
                    }
                    bRet = TRUE;
                } 
            }
            goto EXIT;
            break;
        
        case IOCTL_POWER_GET: 
            goto EXIT;
            break;
            
        default:
            break;
    }
EXIT:
    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("RADIO_IOControl -\r\n")));
#endif
    return HcdPdd_IOControl(hOpenContext, dwCode, pBufIn, dwLenIn, pBufOut,
            dwLenOut, pdwActualOut);
}


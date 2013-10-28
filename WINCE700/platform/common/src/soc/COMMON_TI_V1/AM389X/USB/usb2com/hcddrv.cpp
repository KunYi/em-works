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
// Module Name:
//     hcddrv.cpp
//
// Abstract:
//
// Notes:
//

#include "globals.hpp"
#include "Hcd.hpp"
#include <usb200.h>
#include <cphysmem.hpp>
#include <devload.h>

// Debug Zones.
#ifdef DEBUG

#define DBG_UHCD                (1 << 0)
#define DBG_INIT                (1 << 1)
#define DBG_REGISTERS           (1 << 2)
#define DBG_HUB                 (1 << 3)
#define DBG_ATTACH              (1 << 4)
#define DBG_DESCRIPTORS         (1 << 5)
#define DBG_FUNCTION            (1 << 6)
#define DBG_PIPE                (1 << 7)
#define DBG_TRANSFER            (1 << 8)
#define DBG_QH                  (1 << 9)
#define DBG_TD                  (1 << 10)
#define DBG_CPHYSMEM            (1 << 11)
#define DBG_VERBOSE             (1 << 12)
#define DBG_WARNING             (1 << 13)
#define DBG_ERROR               (1 << 14)
#define DBG_COMPLIANCE          (1 << 15)

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
        TEXT("Compliance")
    },
    DBG_ERROR | DBG_WARNING | DBG_ATTACH | DBG_TRANSFER | DBG_UHCD | DBG_INIT
};
#endif

#define POWER_DOWN_CHECK if (pHcd->m_fDevicePowerDown) { SetLastError(USB_NOT_ACCESSED_ALT); return FALSE; }

DWORD               g_dwContext = 0;
DWORD               g_IstThreadPriority = 101;
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


// OpenedPipesCache holds pointers to opened Pipes.
// This allows HcdIssueTransfer() to call CPipeAbs::IssueTransfer()
// directly, instead of calling down through several C++ classes.
//
// This optimization is purely to avoid the C++ function call overhead
// introduced by too many classes.  The optimization is useful for
// small transfers.

typedef struct {
    CPipeAbs*       pCPipeAbs;
    DWORD           dwPipeId;
    volatile LONG   lInUse;
    DWORD           dwReservedUnusedPad;
} OPENED_PIPE_CACHE_ENTRY;

void InvalidateOpenedPipesCacheEntries(UINT iDevice, CHcd* pHcd)
{
    OPENED_PIPE_CACHE_ENTRY* pCache = (OPENED_PIPE_CACHE_ENTRY*)pHcd->m_pOpenedPipesCache;
    if (pCache) {

        UINT uiKey = iDevice<<16;

        pHcd->Lock();
        for (UINT i=0; i<pHcd->m_dwOpenedPipeCacheSize; i++) {
            if ((0xFFFF0000 & pCache[i].dwPipeId) == uiKey) {
                DEBUGMSG (ZONE_ATTACH,(TEXT("EHCD!InvalidateOpenedPipesCacheEntries() #%u: deviceId=%u, pipeId=%u\r\n"),
                    i,iDevice,(pCache[i].dwPipeId>>8)&0xff));
                //
                // Do not proceed while CPipe is in use -
                // CPipe class will be destroyed right after this fxn exits
                //
                while (pCache[i].lInUse) { Sleep(1); }

                pCache[i].pCPipeAbs = NULL;
                pCache[i].dwPipeId = 0;
            }
        }
        pHcd->Unlock();

    }
}


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


static BOOL HcdGetFrameNumber(LPVOID lpvHcd, LPDWORD lpdwFrameNumber)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;
    BOOL fRetVal = pHcd->GetFrameNumber(lpdwFrameNumber);
    return fRetVal;
}

static BOOL HcdGetFrameLength(LPVOID lpvHcd, LPUSHORT lpuFrameLength)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;
    BOOL fRetVal = pHcd->GetFrameLength(lpuFrameLength);
    return fRetVal;
}

static BOOL HcdSetFrameLength(LPVOID lpvHcd, HANDLE hEvent, USHORT uFrameLength)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;
    BOOL fRetVal = pHcd->SetFrameLength(hEvent, uFrameLength);
    return fRetVal;
}

static BOOL HcdStopAdjustingFrame(LPVOID lpvHcd)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;
    BOOL fRetVal = pHcd->StopAdjustingFrame();
    return fRetVal;
}

static BOOL HcdOpenPipe(LPVOID lpvHcd, UINT iDevice,
        LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
        LPUINT lpiEndpointIndex)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;

    OPENED_PIPE_CACHE_ENTRY* pCache = (OPENED_PIPE_CACHE_ENTRY*)pHcd->m_pOpenedPipesCache;

    CPipeAbs* pCPipe = (CPipeAbs*)0xFFFFFFFF;
    BOOL fRetVal = pHcd->OpenPipe(iDevice, lpEndpointDescriptor, lpiEndpointIndex, (LPVOID*)(&pCPipe) );

    if (fRetVal && pCPipe && pCache &&
        (lpEndpointDescriptor->bmAttributes&USB_ENDPOINT_TYPE_MASK) != USB_ENDPOINT_TYPE_CONTROL)
    {
        UINT uiKey = (iDevice<<16)|((*lpiEndpointIndex&0xFF)<<8)|0x80;

        // We don't want to cache Endpoint 0, which is dedicated to control transfers.
        // We only cache Bulk, Interrupt, and Isochronous pipes.
        ASSERT(*lpiEndpointIndex != 0);

        pHcd->Lock();

        // Is the pipe already in our cache?
        for (UINT i=0; i<pHcd->m_dwOpenedPipeCacheSize; i++) {
            if (pCache[i].dwPipeId == (uiKey|i) &&
                pCache[i].pCPipeAbs != NULL ) {
                    goto alreadyOpened;
            }
        }

        // Add the pipe to our cache.
        bool bSuccess = FALSE;
        for (UINT i=0; i<pHcd->m_dwOpenedPipeCacheSize; i++) {
            if (pCache[i].pCPipeAbs == NULL) {
                ASSERT(pCache[i].lInUse==0);
                pCache[i].lInUse = 0;
                pCache[i].pCPipeAbs = pCPipe;
                pCache[i].dwPipeId = uiKey|i;
                bSuccess = TRUE;
                break;
            }
        }
        if (!bSuccess) {
            DEBUGMSG(ZONE_WARNING, (TEXT("WARNING: HcdOpenPipe(): OpenedPipesCache full; increase 'HcdPipeCache' value in Registry.\r\n")));
        }

alreadyOpened:
        pHcd->Unlock();
    }

    return fRetVal;
}

static BOOL HcdClosePipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;

    OPENED_PIPE_CACHE_ENTRY* pCache = (OPENED_PIPE_CACHE_ENTRY*)pHcd->m_pOpenedPipesCache;

    //
    // Cache lookups & mods are prevented while CPipe class may be destroyed
    //
    if (iEndpointIndex != 0 && pCache != NULL) {
        pHcd->Lock();
    }

    BOOL fRetVal = pHcd->ClosePipe(iDevice, iEndpointIndex);

    if (iEndpointIndex != 0 && pCache != NULL) {

        UINT uiKey = (iDevice<<16)|((iEndpointIndex&0xFF)<<8)|0x80;

        for (UINT i=0; i<pHcd->m_dwOpenedPipeCacheSize; i++) {
            if (pCache[i].dwPipeId == (uiKey|i)) {
                //
                // CPipe class is already destroyed, just remove Cache entry
                // If use count is non-zero, then CPipe is destroyed while doing transfer!
                //
                ASSERT(pCache[i].lInUse==0);
                pCache[i].pCPipeAbs = NULL;
                pCache[i].dwPipeId = 0;
                pCache[i].lInUse = 0;
                break;
            }
        }
        pHcd->Unlock();
    }

    return fRetVal;
}

static BOOL HcdResetPipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;
    BOOL fRetVal = pHcd->ResetPipe(iDevice, iEndpointIndex);
    return fRetVal;
}

static BOOL HcdIsPipeHalted(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex, LPBOOL lpbHalted)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;
    BOOL fRetVal = pHcd->IsPipeHalted(iDevice, iEndpointIndex, lpbHalted);
    return fRetVal;
}

static BOOL HcdIssueTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
        DWORD dwFlags, LPCVOID lpvControlHeader,
        DWORD dwStartingFrame, DWORD dwFrames, LPCDWORD aLengths,
        DWORD dwBufferSize, LPVOID lpvBuffer, ULONG paBuffer,
        LPCVOID lpvCancelId, LPDWORD adwIsochErrors, LPDWORD adwIsochLengths,
        LPBOOL lpfComplete, LPDWORD lpdwBytesTransfered, LPDWORD lpdwError)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;

    OPENED_PIPE_CACHE_ENTRY* pCache = NULL;
    if (!pHcd->m_fDevicePowerDown)
        (OPENED_PIPE_CACHE_ENTRY*)pHcd->m_pOpenedPipesCache;

    BOOL fRetVal;
    volatile LONG* plInUse = NULL;
    CPipeAbs* pCCPipe = NULL;

    // We don't cache pipes to Endpoint 0, which is always a Control Transfer endpoint.
    // Control Transfers require different handling than Bulk, Interrupt, and Isochronous.
    if (iEndpointIndex != 0 && pCache)
    {
        UINT uiKey = (iDevice<<16)|((iEndpointIndex&0xFF)<<8)|0x80;

        //
        // prevent Cache modifications during lookup then release global CS as fast as possible
        //
        pHcd->Lock();
        for (UINT i=0; i<pHcd->m_dwOpenedPipeCacheSize; i++) {
            if (pCache[i].dwPipeId == (uiKey|i)) {
                //
                // entry found, mark it in use to prevent untimely removal
                //
                plInUse = &(pCache[i].lInUse);
                InterlockedIncrement(plInUse);
                pCCPipe = pCache[i].pCPipeAbs;
                break;
            }
        }
        pHcd->Unlock();

        //
        // if cached CPipe found, then invoke it directly
        //
        if (pCCPipe != NULL) {
            HCD_REQUEST_STATUS status = pCCPipe->IssueTransfer( iDevice,
                                                                lpStartAddress,
                                                                lpParameter,
                                                                dwFlags,
                                                                lpvControlHeader,
                                                                dwStartingFrame,
                                                                dwFrames,
                                                                aLengths,
                                                                dwBufferSize,
                                                                lpvBuffer,
                                                                paBuffer,
                                                                lpvCancelId,
                                                                adwIsochErrors,
                                                                adwIsochLengths,
                                                                lpfComplete,
                                                                lpdwBytesTransfered,
                                                                lpdwError );
            //
            // let it go - CPipe class is not used any more
            //
            InterlockedDecrement(plInUse);

            if (status == requestOK ) {
                fRetVal = TRUE;
            }
            else {
                fRetVal = FALSE;
                if (status == requestIgnored)
                    SetLastError(ERROR_DEV_NOT_EXIST);
            }
            goto doneHcdIssueTransfer;
        }
    }


    // Pipe was not in the cache; take the non-cached route through the C++ methods (more function-call overhead)
    // This is expected for Control Transfers (transfers to Endpoint 0).

    // bounce back all transfers when device is powered down
    POWER_DOWN_CHECK;

    // We package the parameters into a struct and pass the pointer down.
    // This prevents pushing/popping of all 16 individual arguments as we call down
    // into the driver.  This is an optimization to reduce function call overhead
    // for very small transfers.
    ISSUE_TRANSFER_PARAMS* pITP = NULL;

#if defined x86 || defined _X86_
    // NOTE: we are taking A POINTER TO THE ARGUMENTS ON THE STACK, treating it as a
    // struct ISSUE_TRANSFER_PARAMS*, and passing that into pHcd->IssueTransfer().
    // Only x86 platforms will always put all parms on the stack
    pITP = (ISSUE_TRANSFER_PARAMS*)(&iDevice);
#else
    #pragma warning(disable: 4533)
    ISSUE_TRANSFER_PARAMS itp = { iDevice,iEndpointIndex,
                                  lpStartAddress,lpParameter,
                                  dwFlags,lpvControlHeader,
                                  dwStartingFrame,dwFrames,aLengths,dwBufferSize,
                                  lpvBuffer,paBuffer,lpvCancelId,
                                  adwIsochErrors,adwIsochLengths,
                                  lpfComplete,lpdwBytesTransfered,lpdwError };
    pITP = &itp;
#endif
    ASSERT(lpdwError==pITP->lpdwError);
    fRetVal = pHcd->IssueTransfer( pITP );

doneHcdIssueTransfer:

    return fRetVal;
}

static BOOL HcdAbortTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
        LPCVOID lpvCancelId)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;
    BOOL fRetVal = pHcd->AbortTransfer(iDevice, iEndpointIndex, lpStartAddress,
            lpParameter, lpvCancelId);
    return fRetVal;
}

static BOOL HcdDisableDevice(LPVOID lpvHcd, UINT iDevice, BOOL fReset)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    POWER_DOWN_CHECK;
    BOOL fRetVal = pHcd->DisableDevice(iDevice,fReset);
    return fRetVal;
}

static BOOL HcdSuspendResume(LPVOID lpvHcd, UINT iDevice, BOOL fSuspend)
{
    CHcd * const pHcd = (CHcd *)lpvHcd;
    DEBUGMSG( ZONE_INIT, (_T("+++ HcdSuspendResume(%x,%x,%u)\r\n"),lpvHcd,iDevice,fSuspend));
    BOOL fRetVal = pHcd->SuspendResume(iDevice, fSuspend);
    DEBUGMSG( ZONE_INIT, (_T("--- HcdSuspendResume(%x,%x,%u) = %u\r\n"),lpvHcd,iDevice,fSuspend,fRetVal));
    return fRetVal;
}


// -----------------------------------------------------------------------------
// Function to read the interrupt thread priority from the registry.
// If it is not in the registry then a default value is returned.
// -----------------------------------------------------------------------------
static DWORD
GetInterruptThreadPriority(
    LPCWSTR lpszActiveKey
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


extern "C" LPVOID HcdMdd_CreateMemoryObject( DWORD cbSize, DWORD cbHighPrioritySize,
                                             PUCHAR pVirtAddr, PUCHAR pPhysAddr )
{
    //
    // We need at least a USBPAGE for Special allocation and a PAGE for normal
    // allocation.
    //
    ASSERT((cbHighPrioritySize + (2*USBPAGESIZE)) < cbSize);

    CPhysMem * pobMem = new CPhysMem(cbSize,cbHighPrioritySize,pVirtAddr,pPhysAddr);
    if (pobMem) {
        if ( ! pobMem->InittedOK() ) {
            delete pobMem;
            pobMem = 0;
        }
    }
    return pobMem;
}

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
        else {
            //
            // Allocate opened pipe cache - if it fails, will not be critical
            // we expect <m_dwOpenedPipeCacheSize> to be set by CHW initialization
            //
            if (pobUhcd->m_dwOpenedPipeCacheSize) {
                PREFAST_ASSUME (pobUhcd->m_dwOpenedPipeCacheSize >= 4 && pobUhcd->m_dwOpenedPipeCacheSize <= 120 ); // size  constrained to >= MIN_PIPE_CACHE_SIZE && <= MAX_PIPE_CACHE_SIZE when registry is read
                OPENED_PIPE_CACHE_ENTRY* pCache = new OPENED_PIPE_CACHE_ENTRY[pobUhcd->m_dwOpenedPipeCacheSize];
                if (pCache) {
                    pobUhcd->m_pOpenedPipesCache = pCache;

                    memset(pCache,0,sizeof(OPENED_PIPE_CACHE_ENTRY)*pobUhcd->m_dwOpenedPipeCacheSize);
                    DEBUGMSG (ZONE_INIT, (TEXT("HcdMdd_CreateHcdObject() pipe cache of size %u (%u bytes) allocated\r\n"),
                        pobUhcd->m_dwOpenedPipeCacheSize,sizeof(OPENED_PIPE_CACHE_ENTRY)*pobUhcd->m_dwOpenedPipeCacheSize));
                }
                else {
                    DEBUGMSG (ZONE_INIT, (TEXT("WARNING! HcdMdd_CreateHcdObject() failed to allocate pipe cache of size %u\r\n"),pobUhcd->m_dwOpenedPipeCacheSize));
                }
            }
        }
    }

    return pobUhcd;
}

extern "C" BOOL HcdMdd_DestroyHcdObject(LPVOID lpvUhcdObject)
{
    CHcd * pobUhcd = (CHcd *)lpvUhcdObject;
    if (pobUhcd->m_pOpenedPipesCache) {
        OPENED_PIPE_CACHE_ENTRY* pCache = (OPENED_PIPE_CACHE_ENTRY*)pobUhcd->m_pOpenedPipesCache;
        delete [] pCache;
    }
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
    pobUhcd->PowerMgmtCallback(FALSE);
    return TRUE;
}

extern "C" BOOL HcdMdd_PowerDown(LPVOID lpvUhcdObject)
{
    CHcd * pobUhcd = (CHcd *)lpvUhcdObject;
    pobUhcd->PowerMgmtCallback(TRUE);
    OPENED_PIPE_CACHE_ENTRY* pCache = (OPENED_PIPE_CACHE_ENTRY*)pobUhcd->m_pOpenedPipesCache;
    if (pCache)
        memset(pCache,0,sizeof(OPENED_PIPE_CACHE_ENTRY)*pobUhcd->m_dwOpenedPipeCacheSize);
    return TRUE;
}

extern "C"  DWORD   HcdMdd_SetCapability (LPVOID lpvUhcdObject, DWORD dwCapability )
{
    CHcd * pobUhcd = (CHcd *)lpvUhcdObject;
    DWORD dwRet = pobUhcd->SetCapability(dwCapability);
    return dwRet;
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


extern "C" BOOL HCD_Deinit(DWORD hDeviceContext)
{
    DEBUGMSG (ZONE_INIT, (TEXT("EHCD!HCD_Deinit\r\n")));
    EnterCriticalSection( &g_CSection );
    DWORD dwReturn = HcdPdd_Deinit(hDeviceContext);
    LeaveCriticalSection( &g_CSection );
    return dwReturn;
}


extern "C" void HCD_PowerUp(DWORD hDeviceContext)
{
    HcdPdd_PowerUp(hDeviceContext);
}

extern "C" void HCD_PowerDown(DWORD hDeviceContext)
{
    HcdPdd_PowerDown(hDeviceContext);
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


extern "C" BOOL HCD_IOControl(DWORD hOpenContext, DWORD dwCode, BYTE const*const pBufIn,
        DWORD dwLenIn, BYTE const*const pBufOut, DWORD dwLenOut, DWORD const*const pdwActualOut)
{
    BOOL retVal = FALSE;
    SetLastError(ERROR_INVALID_PARAMETER);

RETAILMSG(1, (L"HCD_IOControl %d\r\n", dwCode)); 

    __try {

        switch(dwCode) {

#ifdef USB_IF_ELECTRICAL_TEST_MODE

            case IOCTL_USB_EHCI_GET_NUM_PORTS:
                DEBUGMSG(1/*ZONE_COMPLIANCE*/,(TEXT("EHCD!HCD_IOControl(IOCTL_USB_EHCI_GET_NUM_PORTS)\r\n")));
                if(pdwActualOut && pBufOut && dwLenOut >= sizeof(DWORD)) {
                    CHcd* pHcd = (CHcd*)hOpenContext;
                    * (DWORD*) pBufOut = pHcd->GetNumOfPorts();
                    * (DWORD*) pdwActualOut = sizeof(DWORD);
                    SetLastError(ERROR_SUCCESS);
                    retVal = TRUE;
                }
                break;

            case IOCTL_USB_EHCI_SET_TEST_MODE:
                DEBUGMSG(1/*ZONE_COMPLIANCE*/,(TEXT("EHCD!HCD_IOControl(IOCTL_USB_EHCI_SET_TEST_MODE)\r\n")));
                if(pBufIn && dwLenIn >= sizeof(IO_USB_EHCI_SET_TEST_MODE)) {
                    PIO_USB_EHCI_SET_TEST_MODE pTestInfo = (PIO_USB_EHCI_SET_TEST_MODE)pBufIn;
                    CHcd* pHcd = (CHcd*)hOpenContext;
                    retVal = (ComplTestInvalid != pHcd->SetTestMode(pTestInfo->dwPortNum, pTestInfo->dwTestMode));
                    if(retVal) {
                        SetLastError(ERROR_SUCCESS);
                    }
                }
                break;

            case IOCTL_USB_TEST_MODE_BEGIN:
                DEBUGMSG(1/*ZONE_COMPLIANCE*/,(TEXT("EHCD!HCD_IOControl(IOCTL_USB_TEST_MODE_BEGIN)\r\n")));
                break;
            case IOCTL_USB_TEST_MODE_END:
                DEBUGMSG(1/*ZONE_COMPLIANCE*/,(TEXT("EHCD!HCD_IOControl(IOCTL_USB_TEST_MODE_END)\r\n")));
                break;

#endif //#ifdef USB_IF_ELECTRICAL_TEST_MODE

            case IOCTL_POWER_CAPABILITIES:
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__\r\n", __LINE__));				
                if (pBufOut!=NULL && dwLenOut>=sizeof(POWER_CAPABILITIES) )
                {
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__\r\n", __LINE__));				
                    PPOWER_CAPABILITIES pPwrCap = (PPOWER_CAPABILITIES)pBufOut;
                    // full power & OFF are always advertized
                    pPwrCap->DeviceDx   = DX_MASK(D0)|DX_MASK(D4);
                    pPwrCap->WakeFromDx = DX_MASK(D0)|DX_MASK(D4);
                    pPwrCap->InrushDx   = DX_MASK(D0)|DX_MASK(D4);
                    pPwrCap->Flags      = 0;
                    pPwrCap->Power[0]   = 200; // HC drains little power; connected devices must specify their own consumption
                    pPwrCap->Power[1]   = PwrDeviceUnspecified;
                    pPwrCap->Power[2]   = PwrDeviceUnspecified;
                    pPwrCap->Power[3]   = PwrDeviceUnspecified;
                    pPwrCap->Power[4]   = 0;
                    pPwrCap->Latency[0] = 0;
                    pPwrCap->Latency[1] = PwrDeviceUnspecified;
                    pPwrCap->Latency[2] = PwrDeviceUnspecified;
                    pPwrCap->Latency[3] = PwrDeviceUnspecified;
                    pPwrCap->Latency[4] = 500;  // half a sec to complete power up

                    // sleep is only supported if bit 0 of "HcdCapability" is set
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__\r\n", __LINE__));				
                    if ((HCD_SUSPEND_RESUME&((CHcd*)hOpenContext)->GetCapability())!=0) {
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__\r\n", __LINE__));				
                        pPwrCap->DeviceDx   = DX_MASK(D0)|DX_MASK(D3)|DX_MASK(D4);
                        pPwrCap->WakeFromDx = DX_MASK(D0)|DX_MASK(D3)|DX_MASK(D4);
                        pPwrCap->InrushDx   = DX_MASK(D0)|DX_MASK(D3)|DX_MASK(D4);
                        pPwrCap->Power[3]   = 10;   // trickle of power only
                        pPwrCap->Latency[3] = 500;  // half a sec to complete power up
                    }
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__\r\n", __LINE__));				

                    if (pdwActualOut!=NULL)
                        * (DWORD*) pdwActualOut = sizeof(POWER_CAPABILITIES);
                    retVal = TRUE;
                    SetLastError(ERROR_SUCCESS);
					DEBUGMSG(1/*ZONE_INIT*/,(TEXT("EHCD!HCD_IOControl() %x: IOCTL_POWER_CAPABILITIES=%x success\r\n"),hOpenContext,pPwrCap->DeviceDx));
                }
                break;

            case IOCTL_POWER_GET:
                if (pBufOut!=NULL && dwLenOut>=sizeof(CEDEVICE_POWER_STATE) )
                {
                    CHcd* pHcd = (CHcd*)hOpenContext;
                    *((CEDEVICE_POWER_STATE*)pBufOut) = pHcd->m_DevPwrState;
                    DEBUGMSG(1/*ZONE_INIT*/,(TEXT("EHCD!HCD_IOControl() %x: IOCTL_POWER_GET is %u\r\n"),pHcd,pHcd->m_DevPwrState));
                    retVal = TRUE;
                    SetLastError(ERROR_SUCCESS);
                }
                break;

            case IOCTL_POWER_SET:
                if (pBufOut!=NULL && dwLenOut>=sizeof(CEDEVICE_POWER_STATE) )
                {
                    CHcd* pHcd = (CHcd*)hOpenContext;
                    LONG pwrState = *((LONG*)pBufOut);
                    CEDEVICE_POWER_STATE dx = *((CEDEVICE_POWER_STATE*)pBufOut);

                    // sleep is only supported if bit 0 of "HcdCapability" is set
                    if ((HCD_SUSPEND_RESUME&((CHcd*)hOpenContext)->GetCapability())!=0)
                    {
                        //supported states D0, D3, D4 (check IOCTL_POWER_CAPABILITIES)
                        switch(dx)
                        {
                            case D0:
                            case D1:
                            case D2:
                                dx = D0;
                                break;
                            case D3:
                                dx = D3;
                                break;
                            case D4:
                                dx = D4;
                                break;
                            default:
                                dx = D0;
                                break;
                        }
                    }
                    else
                    {
                        //supported states D0, D4 (check IOCTL_POWER_CAPABILITIES)
                        switch(pwrState)
                        {
                            case D0:
                            case D1:
                            case D2:
                            case D3:
                                dx = D0;
                                break;
                            case D4:
                                dx = D4;
                                break;
                            default:
                                dx = D0;
                                break;
                        }
                    }
                    pwrState = (LONG)dx;
                    pHcd->Lock();
                    //
                    // set the new power state and disable pipe cache if HC is powering down
                    //
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__\r\n", __LINE__));				
                    pwrState = InterlockedExchange((volatile LONG*)(&(pHcd->m_DevPwrState)),pwrState);
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__\r\n", __LINE__));				
                    if (*((CEDEVICE_POWER_STATE*)pBufOut) > D2) {
                        pHcd->m_fDevicePowerDown = TRUE;
                    }
                    else {
                        pHcd->m_fDevicePowerDown = FALSE;
                    }
                    pHcd->Unlock();
                    DEBUGMSG(1/*ZONE_INIT*/,(TEXT("EHCD!HCD_IOControl() %x: IOCTL_POWER_SET to %u from %u\r\n"),
                        pHcd,pHcd->m_DevPwrState,pwrState));
                    retVal = TRUE;
                    SetLastError(ERROR_SUCCESS);
                }
                break;

            default:
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__\r\n", __LINE__));				
                retVal = HcdPdd_IOControl(hOpenContext, dwCode, (PBYTE)pBufIn, dwLenIn,
                                            (PBYTE)pBufOut, dwLenOut, (PDWORD)pdwActualOut);
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__\r\n", __LINE__));				
				
                if(retVal) {
                    SetLastError(ERROR_SUCCESS);
                }
                break;
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(1/*ZONE_INIT*/,(TEXT("EHCD!HCD_IOControl(%x) BAD PARAMETER(s) 0x%x/%u, 0x%x/%u, 0x%x!!!\r\n"),
            dwCode,pBufIn,dwLenIn,pBufOut,dwLenOut,pdwActualOut));
    }
DEBUGMSG(1,(L"EHCD!HCD_IOControl __%d__; retVal %d\r\n", __LINE__, retVal));				

    return retVal;
}


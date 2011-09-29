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
//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 
//------------------------------------------------------------------------------
//
//  File: stub.c
//
//  This file contains the functions USB Funcation PDD OS layer.

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <celog.h>
#include <oal.h>
#pragma warning(pop)

#pragma warning(disable: 4100)

#include "usbd.h"

extern CRITICAL_SECTION g_csRegister;

static USBFN_PDD g_usbfn_pdd; 

extern DWORD GetUSBBaseReg();

USBFN_PDD * FslUfnGetUsbFnPdd()
{
    return &g_usbfn_pdd;
}


DWORD FslUfnRequestIrq(USBFN_PDD *pPdd)
{
    return  ERROR_SUCCESS;
}

BOOL  FslUfnIsUSBKitlEnable()
{
    //Kitl must return FALSE, otherwise usbkitl init will fail. 
    return FALSE;
}

DWORD FslUfnCreateThread(USBFN_PDD *pPdd)
{
    return ERROR_SUCCESS;
}

DWORD WINAPI UsbFnInterruptThread(VOID *pPddContext)
{
    DWORD timeout=0;
    return InterruptHandle((USBFN_PDD *)pPddContext,0, &timeout);
}

void FslUfnTrigIrq(USBFN_PDD *pPdd)
{
    UsbFnInterruptThread(pPdd);
}

void FslUfnDeinit(USBFN_PDD *pPdd)
{
    return; 
}

#define PAGEALIGN_SIZE(x)  (((x) + 0x1000 - 1) & 0xFFFFF000)
void FslUfnGetDMABuffer(USBFN_PDD *pPdd)
{
    DWORD qhHead;

    qhHead = FslUfnGetKitlDMABuffer();    

    pPdd->qhbuf_phy = PAGEALIGN_SIZE(qhHead);
    pPdd->qhbuffer = OALPAtoVA(pPdd->qhbuf_phy, FALSE); 

    
    pPdd->phyAddr_TD1.LowPart = pPdd->qhbuf_phy + PAGEALIGN_SIZE(sizeof(USBFN_QH_BUF_T));
    pPdd->Buffer_Td1  = OALPAtoVA(pPdd->phyAddr_TD1.LowPart, FALSE);

    pPdd->phyAddr_TD2.LowPart = pPdd->phyAddr_TD1.LowPart + PAGEALIGN_SIZE(MAX_SIZE_PER_BP);
    pPdd->Buffer_Td2  = OALPAtoVA(pPdd->phyAddr_TD2.LowPart, FALSE);

    //RETAILMSG(1, (_T("usekinfo 0x%x"), UserKInfo));
        
    //RETAILMSG(1, (_T("qhbuff_phy  :0x%x  v: 0x%x\r\n"), pPdd->qhbuf_phy, pPdd->qhbuffer));
    //RETAILMSG(1, (_T("phyAddr_TD1 :0x%x  v: 0x%x\r\n"), pPdd->phyAddr_TD1.LowPart, pPdd->Buffer_Td1));
    //RETAILMSG(1, (_T("phyAddr_TD2 :0x%x  v: 0x%x\r\n"), pPdd->phyAddr_TD2.LowPart, pPdd->Buffer_Td2));
}

BOOL BSPUSBClockSwitch(BOOL fOn)
{
    //RETAILMSG(1, (_T("\r\n"), ADDRESS_AND_SIZE_TO_SPAN_PAGES(100, 10)));
    return TRUE;
}


BOOL BSPUSBClockCreateFileMapping(void)
{
    return TRUE;
}


DWORD GetDeviceRegistryParams(
    LPCWSTR context, VOID *pBase, DWORD count, 
    const DEVICE_REGISTRY_PARAM params[]
)
{
    USBFN_PDD * pPdd= (USBFN_PDD *)pBase;
    pPdd->memBase = CSP_BASE_REG_PA_USB ;
    pPdd->IsOTGSupport = FALSE;
    return ERROR_SUCCESS;
}


BOOL SetInterruptEvent(
  DWORD idInt
)
{
    UsbFnInterruptThread(&g_usbfn_pdd);
    return TRUE;
}


DWORD FslUfnGetPageNumber(void *p, size_t size)
{
    return ((((ULONG)(p) & (0x1000 - 1)) + (size) + (0x1000 - 1)) >> 12);
}


DWORD FslUfnGetPageSize()
{
    return 0x1000;
}


DWORD FslUfnGetPageShift()
{
    //UserKInfo[KINX_PFN_SHIFT]
    //hear is 0, not 12. 
    return 0;
}


void   FslInitializeCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
)
{
    return;
}


void FslEnterCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
)
{
    return;
}


void  FslLeaveCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
)
{
    return;
}


//------------------------------------------------------------------------------
// External Variables 

//extern ROMHDR * volatile const pTOC;

//------------------------------------------------------------------------------
// LocalFree
//
//   Local function used to Free Memory
//
// Return Value:
//    HLOCAL NULL indicates success
//
HLOCAL
WINAPI
LocalFree (
    HLOCAL hMem
    ) {
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hMem);
    return 0;
}


LONG WINAPI
InterlockedIncrement(
    LONG volatile *Addend
    )
{
    return ++(*Addend);
}


LONG WINAPI
InterlockedDecrement(
    LONG volatile *Addend
    )
{
    return --(*Addend);
}



HANDLE CreateBusAccessHandle(
  __in LPCTSTR lpActiveRegPath
)
{
    return (HANDLE)1;
}


VOID CloseBusAccessHandle (
  __in_opt HANDLE hBusAccess
)
{
    return; 
}

BOOL CloseHandle(
  HANDLE hObject
)
{
    return TRUE;
}


VOID MmUnmapIoSpace( 
  __in PVOID BaseAddress, 
  __in ULONG NumberOfBytes 
)
{
    return;
}


void CeLogData(
  BOOL fTimeStamp, 
  WORD wID, 
  PVOID pData, 
  WORD wLen, 
  DWORD dwZoneUser, 
  DWORD dwZoneCE, 
  WORD wFlag, 
  BOOL fFlagged 
)
{
    return;
}


BOOL InterruptInitialize(
  DWORD idInt,
  HANDLE hEvent,
  LPVOID pvData,
  DWORD cbData
)
{
    return TRUE;
}


VOID InterruptDone(
  DWORD idInt
)
{
    return ;
}

PVOID MmMapIoSpace( 
  __in PHYSICAL_ADDRESS PhysicalAddress, 
  __in ULONG NumberOfBytes, 
  __in BOOLEAN CacheEnable 
)
{
    return OALPAtoVA((UINT32)PhysicalAddress.QuadPart,CacheEnable);
}

VOID InterruptDisable(
  DWORD idInt
)
{
    return;
}


VOID CacheSync(
  int flags
)
{
    OALCleanDCache();
    OALFlushDCache();
}

BOOL LockPages(
  LPVOID lpvAddress,
  DWORD cbSize,
  PDWORD pPFNs,
  int fOptions
)
{
    //TODO cbsize > 4k
    pPFNs[0] = OALVAtoPA(lpvAddress);
    return TRUE;
}

DWORD GetLastError(void)
{
    return 0;
}



BOOL UnlockPages(
  LPVOID lpvAddress,
  DWORD cbSize
)
{
    return TRUE;
}

HRESULT CeOpenCallerBuffer(
  PVOID * ppDestMarshalled,
  PVOID pSrcUnmarshalled,
  DWORD cbSrc,
  DWORD ArgumentDescriptor,
  BOOL ForceDuplicate
)
{
    *ppDestMarshalled=pSrcUnmarshalled;
    return ERROR_SUCCESS;
}


HRESULT CeCloseCallerBuffer(
   PVOID pDestMarshalled,
  PVOID pSrcUnmarshalled,
  DWORD cbSrc,
  DWORD ArgumentDescriptor
)
{
    return ERROR_SUCCESS;
}


HRESULT StringCchLengthW(      
    __in LPCTSTR psz,
    __in size_t cchMax,
    __out_opt size_t *pcch
)
{
    if(pcch!=NULL) *pcch=0;
    return ERROR_SUCCESS;
}


HRESULT StringCchVPrintfW(      
    __out_ecount(cchDest) STRSAFE_LPWSTR pszDest,
    __in size_t cchDest,
    __in __format_string STRSAFE_LPCWSTR pszFormat,
    __in va_list argList
)
{
    return ERROR_SUCCESS;
}


BOOL SetDevicePowerState (
  __in_opt HANDLE hBusAccess,
  __in CEDEVICE_POWER_STATE PowerState,
  __reserved PVOID pReserved
)
{
    return TRUE;
}


void Sleep(
  DWORD dwMilliseconds
)
{
    return;
}

#define MAX_PRINT_BUFFER 128
VOID FslNKDbgPrintfW(LPCWSTR format, ...)
{
    va_list pArgList;
    WCHAR buffer[MAX_PRINT_BUFFER];

    va_start(pArgList, format);
    NKwvsprintfW(buffer, format, pArgList, MAX_PRINT_BUFFER);
    OEMWriteDebugString(buffer);
}

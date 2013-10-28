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
//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:
    system.c

Abstract:
    Device dependant part of the USB Universal Host Controller Driver (EHCD).

Notes:
--*/

//#pragma optimize( "", off )

#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ddkreg.h>
#include <devload.h>
#include <hcdddsi.h>
#include "csp.h"

#define REG_PHYSICAL_PAGE_SIZE TEXT("PhysicalPageSize")
#define REG_OTG_SUPPORT TEXT("OTGSupport")

extern BOOL InitializeTransceiver(PCSP_USB_REGS* regs, DWORD * phyregs,
                                  DWORD * sysintr, DWORD dwOTGSupport, TCHAR *pOTGGroup);
extern BOOL BSPUsbhCheckConfigPower(UCHAR bPort, DWORD dwCfgPower, DWORD dwTotalPower);

// Amount of memory to use for HCD buffer
#define gcTotalAvailablePhysicalMemory (0x20000) // 128K
#define gcHighPriorityPhysicalMemory (gcTotalAvailablePhysicalMemory/4)   // 1/4 as high priority


typedef struct _SEHCDPdd
{
    LPVOID lpvMemoryObject;
    LPVOID lpvEHCDMddObject;
    PVOID pvVirtualAddress;                        // DMA buffers as seen by the CPU
    DWORD dwPhysicalMemSize;
    PHYSICAL_ADDRESS LogicalAddress;        // DMA buffers as seen by the DMA controller and bus interfaces
    DMA_ADAPTER_OBJECT AdapterObject;
    TCHAR szDriverRegKey[MAX_PATH];
    PUCHAR ioPortBase;
    DWORD dwSysIntr;
    CRITICAL_SECTION csPdd;                     // serializes access to the PDD object
    HANDLE          IsrHandle;
    HANDLE hParentBusHandle;
    DWORD   dwOTGSupport;
    TCHAR szOTGGroup[15];
} SEHCDPdd;

#define UnusedParameter(x)  x = x

/* HcdPdd_DllMain
 *
 *  DLL Entry point.
 *
 * Return Value:
 */
extern BOOL HcdPdd_DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    UnusedParameter(hinstDLL);
    UnusedParameter(dwReason);
    UnusedParameter(lpvReserved);

    return TRUE;
}

static BOOL
GetRegistryPhysicalMemSize(
    LPCWSTR RegKeyPath,         // IN - driver registry key path
    DWORD * lpdwPhyscialMemSize)       // OUT - base address
{
    HKEY hKey;
    DWORD dwData;
    DWORD dwSize;
    DWORD dwType;
    BOOL  fRet=FALSE;
    DWORD dwRet;
    // Open key
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegKeyPath,0,0,&hKey);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!EHCD:GetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"),
                             RegKeyPath, dwRet));
        return FALSE;
    }

    // Read base address, range from registry and determine IOSpace
    dwSize = sizeof(dwData);
    dwRet = RegQueryValueEx(hKey, REG_PHYSICAL_PAGE_SIZE, 0, &dwType, (PUCHAR)&dwData, &dwSize);
    if (dwRet == ERROR_SUCCESS) {
        if (lpdwPhyscialMemSize)
            *lpdwPhyscialMemSize = dwData;
        fRet=TRUE;
    }
    RegCloseKey(hKey);
    return fRet;
}


//------------------------------------------------------------------------------
//
// Function:  ConfigureEHCICard
//
// This is to configure the EHCI register to mapped from bus address to virtual address space
//
// Parameter :
//
//   [IN] pddObject - Pointer to PDD context
//   [OUT] pioPortBase - Pointer to register base in virtual address
//   [IN] dwAddrLen - size of the address to map
//   [IN] dwIOSpace - IO port base in physical address
//   [IN] IfcType - INTERFACE_TYPE
//   [IN] dwBusNumber - Bus number where devices resides
//
// Return:
//   The virtual address would be in pioPortBase
//   TRUE - success
//   FALSE - failed
//
//----------------------------------------------------------------------------------
BOOL
ConfigureEHCICard(
    SEHCDPdd * pPddObject, // IN - contains PDD reference pointer.
    PUCHAR *pioPortBase,   // IN - contains physical address of register base
                           // OUT- contains virtual address of register base
    DWORD dwAddrLen,
    DWORD dwIOSpace,
    INTERFACE_TYPE IfcType,
    DWORD dwBusNumber
    )
{
    ULONG               inIoSpace = dwIOSpace;
    ULONG               portBase;
    PHYSICAL_ADDRESS    ioPhysicalBase = {0, 0};

    portBase = (ULONG)*pioPortBase;
    ioPhysicalBase.LowPart = portBase;

    if (!BusTransBusAddrToVirtual(pPddObject->hParentBusHandle, IfcType, dwBusNumber, ioPhysicalBase, dwAddrLen, &inIoSpace, (PPVOID)pioPortBase)) {
        DEBUGMSG(ZONE_ERROR, (L"EHCD: Failed TransBusAddrToVirtual\r\n"));
        return FALSE;
    }

    DEBUGMSG(ZONE_INIT,
             (TEXT("EHCD: ioPhysicalBase 0x%X, IoSpace 0x%X\r\n"),
              ioPhysicalBase.LowPart, inIoSpace));
    DEBUGMSG(ZONE_INIT,
             (TEXT("EHCD: ioPortBase 0x%X, portBase 0x%X\r\n"),
              *pioPortBase, portBase));

    return TRUE;
}


/* InitializeEHCI
 *
 *  Configure and initialize EHCI card
 *
 * Return Value:
 *  Return TRUE if card could be located and configured, otherwise FALSE
 */
#define OTGSUPPORT_VALNAME (TEXT("OTGSupport"))
#define OTGGROUP_VALNAME   (TEXT("OTGGroup"))
static BOOL
InitializeEHCI(
    SEHCDPdd * pPddObject,    // IN - Pointer to PDD structure
    LPCWSTR szDriverRegKey)   // IN - Pointer to active registry key string
{
    PUCHAR ioPortBase = NULL;
    DWORD dwAddrLen;
    DWORD dwIOSpace;
    BOOL InstallIsr = FALSE;
    BOOL fResult = FALSE;
    LPVOID pobMem = NULL;
    LPVOID pobEHCD = NULL;
    DWORD PhysAddr;
    DWORD dwHPPhysicalMemSize;
    HKEY    hKey;
    DWORD dwOTGSupport = 0;

    DDKWINDOWINFO dwi;
    DDKISRINFO dii;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,szDriverRegKey,0,0,&hKey)!= ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("InitializeEHCI:GetRegistryConfig RegOpenKeyEx(%s) failed\r\n"),
                             szDriverRegKey));
        return FALSE;
    }
    dwi.cbSize=sizeof(dwi);
    dii.cbSize=sizeof(dii);
    if ( DDKReg_GetWindowInfo(hKey, &dwi ) !=ERROR_SUCCESS || DDKReg_GetIsrInfo (hKey, &dii ) != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("InitializeEHCI:DDKReg_GetWindowInfo or  DDKReg_GetWindowInfo failed\r\n")));
        goto InitializeEHCI_Error;
    }

    if (dwi.dwNumMemWindows!=0) {
        PhysAddr = dwi.memWindows[0].dwBase;
        dwAddrLen= dwi.memWindows[0].dwLen;
        dwIOSpace = 0;
    }
    else if (dwi.dwNumIoWindows!=0) {
        PhysAddr= dwi.ioWindows[0].dwBase;
        dwAddrLen = dwi.ioWindows[0].dwLen;
        dwIOSpace = 1;
    }
    else
        goto InitializeEHCI_Error;

    if ( hKey!=NULL)  {
        DWORD dwType;
        DWORD dwLength = sizeof(DWORD);
        RegQueryValueEx(hKey, OTGSUPPORT_VALNAME, 0, &dwType, (PUCHAR)&dwOTGSupport, &dwLength);
    }

    pPddObject->dwOTGSupport = dwOTGSupport;

    if (hKey != NULL)  {
        DWORD dwType = REG_SZ;
        DWORD dwLength = sizeof(TCHAR)*15;
        RegQueryValueEx(hKey, OTGGROUP_VALNAME, NULL, &dwType, (LPBYTE)pPddObject->szOTGGroup, &dwLength);
        DEBUGMSG(1, (TEXT("OTGGroupName = %s\r\n"), pPddObject->szOTGGroup));
    }

    DEBUGMSG(ZONE_INIT,(TEXT("EHCD: Read config from registry: Base Address: 0x%X, Length: 0x%X, I/O Port: %s, SysIntr: 0x%X, Interface Type: %u, Bus Number: %u\r\n"),
                        PhysAddr, dwAddrLen, dwIOSpace ? L"YES" : L"NO", dii.dwSysintr,dwi.dwInterfaceType, dwi.dwBusNumber));

    ioPortBase = (PUCHAR)PhysAddr;

    if (!(fResult = ConfigureEHCICard(pPddObject, &ioPortBase, dwAddrLen, dwIOSpace, dwi.dwInterfaceType, dwi.dwBusNumber))) {
        goto InitializeEHCI_Error;
    }

    InitializeTransceiver((PCSP_USB_REGS*)&ioPortBase,(DWORD*)&PhysAddr, &dii.dwSysintr, dwOTGSupport, pPddObject->szOTGGroup);

    DEBUGMSG(1, (TEXT("USBHost OTGSupport = %d\r\n"), dwOTGSupport));

#if 0
    if (dii.szIsrDll[0] != 0 && dii.szIsrHandler[0]!=0 && dii.dwIrq<0xff && dii.dwIrq>0 ) {
        // Install ISR handler
        pPddObject->IsrHandle = LoadIntChainHandler(dii.szIsrDll, dii.szIsrHandler, (BYTE)dii.dwIrq);

        if (!pPddObject->IsrHandle) {
            DEBUGMSG(ZONE_ERROR, (L"EHCD: Couldn't install ISR handler\r\n"));
        } else {
            GIISR_INFO Info;
            PHYSICAL_ADDRESS PortAddress = {PhysAddr, 0};

            DEBUGMSG(ZONE_INIT, (L"EHCD: Installed ISR handler, Dll = '%s', Handler = '%s', Irq = %d\r\n",
                dii.szIsrDll, dii.szIsrHandler, dii.dwIrq));

            if (!BusTransBusAddrToStatic(pPddObject->hParentBusHandle,dwi.dwInterfaceType, dwi.dwBusNumber, PortAddress, dwAddrLen, &dwIOSpace, &(PVOID)PhysAddr)) {
                DEBUGMSG(ZONE_ERROR, (L"EHCD: Failed TransBusAddrToStatic\r\n"));
                return FALSE;
            }

            // Set up ISR handler
            Info.SysIntr = dii.dwSysintr;
            Info.CheckPort = TRUE;
            Info.PortIsIO = (dwIOSpace) ? TRUE : FALSE;
            Info.UseMaskReg = TRUE;
            Info.PortAddr = PhysAddr + *ioPortBase + 0x04;
            Info.PortSize = sizeof(DWORD);
            Info.MaskAddr = PhysAddr + *ioPortBase + 0x08;

            if (!KernelLibIoControl(pPddObject->IsrHandle, IOCTL_GIISR_INFO, &Info, sizeof(Info), NULL, 0, NULL)) {
                DEBUGMSG(ZONE_ERROR, (L"EHCD: KernelLibIoControl call failed.\r\n"));
            }
        }
    }
#endif

    // The PDD can supply a buffer of contiguous physical memory here, or can let the
    // MDD try to allocate the memory from system RAM.  We will use the HalAllocateCommonBuffer()
    // API to allocate the memory and bus controller physical addresses and pass this information
    // into the MDD.
    if (GetRegistryPhysicalMemSize(szDriverRegKey,&pPddObject->dwPhysicalMemSize)) {
        // A quarter for High priority Memory.
        dwHPPhysicalMemSize = pPddObject->dwPhysicalMemSize/4;
        // Align with page size.
        pPddObject->dwPhysicalMemSize = (pPddObject->dwPhysicalMemSize + PAGE_SIZE -1) & ~(PAGE_SIZE -1);
        dwHPPhysicalMemSize = ((dwHPPhysicalMemSize +  PAGE_SIZE -1) & ~(PAGE_SIZE -1));
    }
    else
        pPddObject->dwPhysicalMemSize=0;

    if (pPddObject->dwPhysicalMemSize<gcTotalAvailablePhysicalMemory) { // Setup Minimun requirement.
        pPddObject->dwPhysicalMemSize = gcTotalAvailablePhysicalMemory;
        dwHPPhysicalMemSize = gcHighPriorityPhysicalMemory;
    }

    pPddObject->AdapterObject.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
    pPddObject->AdapterObject.InterfaceType = dwi.dwInterfaceType;
    pPddObject->AdapterObject.BusNumber = dwi.dwBusNumber;
    if ((pPddObject->pvVirtualAddress = HalAllocateCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, &pPddObject->LogicalAddress, FALSE)) == NULL) {
        goto InitializeEHCI_Error;
    }
    if (!(pobMem = HcdMdd_CreateMemoryObject(pPddObject->dwPhysicalMemSize, dwHPPhysicalMemSize, (PUCHAR) pPddObject->pvVirtualAddress, (PUCHAR) pPddObject->LogicalAddress.LowPart))) {
        goto InitializeEHCI_Error;
    }

    if (!(pobEHCD = HcdMdd_CreateHcdObject(pPddObject, pobMem, szDriverRegKey, ioPortBase, dii.dwSysintr))) {
        goto InitializeEHCI_Error;
    }

    pPddObject->lpvMemoryObject = pobMem;
    pPddObject->lpvEHCDMddObject = pobEHCD;
    _tcsncpy_s(pPddObject->szDriverRegKey, _countof(pPddObject->szDriverRegKey), szDriverRegKey, _TRUNCATE);
    pPddObject->ioPortBase = ioPortBase;
    pPddObject->dwSysIntr = dii.dwSysintr;

    if ( hKey!=NULL)  {
        DWORD dwCapability;
        DWORD dwType;
        DWORD dwLength = sizeof(DWORD);
        if (RegQueryValueEx(hKey, HCD_CAPABILITY_VALNAME, 0, &dwType, (PUCHAR)&dwCapability, &dwLength) == ERROR_SUCCESS)
            HcdMdd_SetCapability (pobEHCD,dwCapability);
        RegCloseKey(hKey);
    }

    return TRUE;

InitializeEHCI_Error:
    if (pPddObject->IsrHandle) {
        FreeIntChainHandler(pPddObject->IsrHandle);
        pPddObject->IsrHandle = NULL;
    }

    if (pobEHCD)
        HcdMdd_DestroyHcdObject(pobEHCD);
    if (pobMem)
        HcdMdd_DestroyMemoryObject(pobMem);
    if(pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);

    pPddObject->lpvMemoryObject = NULL;
    pPddObject->lpvEHCDMddObject = NULL;
    pPddObject->pvVirtualAddress = NULL;
    if ( hKey!=NULL)
        RegCloseKey(hKey);

    return FALSE;
}

/* HcdPdd_Init
 *
 *   PDD Entry point - called at system init to detect and configure EHCI card.
 *
 * Return Value:
 *   Return pointer to PDD specific data structure, or NULL if error.
 */
extern DWORD
HcdPdd_Init(
    DWORD dwContext)  // IN - Pointer to context value. For device.exe, this is a string
                      //      indicating our active registry key.
{
    SEHCDPdd *  pPddObject = malloc(sizeof(SEHCDPdd));
    BOOL        fRet = FALSE;

    if (pPddObject) {
        pPddObject->pvVirtualAddress = NULL;
        InitializeCriticalSection(&pPddObject->csPdd);
        pPddObject->IsrHandle = NULL;
        pPddObject->hParentBusHandle = CreateBusAccessHandle((LPCWSTR)g_dwContext);

        DEBUGMSG(1, (TEXT("InitializeEHCI\r\n")));
        fRet = InitializeEHCI(pPddObject, (LPCWSTR)dwContext);

        if(!fRet)
        {
            if (pPddObject->hParentBusHandle)
                CloseBusAccessHandle(pPddObject->hParentBusHandle);

            DeleteCriticalSection(&pPddObject->csPdd);
            free(pPddObject);
            pPddObject = NULL;
        }
    }

    return (DWORD)pPddObject;
}

/* HcdPdd_CheckConfigPower
 *
 *    Check power required by specific device configuration and return whether it
 *    can be supported on this platform.  For CEPC, this is trivial, just limit to
 *    the 500mA requirement of USB.  For battery powered devices, this could be
 *    more sophisticated, taking into account current battery status or other info.
 *
 * Return Value:
 *    Return TRUE if configuration can be supported, FALSE if not.
 */
extern BOOL HcdPdd_CheckConfigPower(
    UCHAR bPort,         // IN - Port number
    DWORD dwCfgPower,    // IN - Power required by configuration
    DWORD dwTotalPower)  // IN - Total power currently in use on port
{
    return BSPUsbhCheckConfigPower(bPort, dwCfgPower, dwTotalPower);
}

//----------------------------------------------------------
//
// Function : HcdPdd_PowerUp
//
// This function is called when the system is resume from suspend
// state
//
// Paramter :
//   hDeviceContext - pointer to PDD object
//
// Return:
//   Nil
//
//-----------------------------------------------------------
extern void HcdPdd_PowerUp(DWORD hDeviceContext)
{
    SEHCDPdd * pPddObject = (SEHCDPdd *)hDeviceContext;
    HcdMdd_PowerUp(pPddObject->lpvEHCDMddObject);

    return;
}

//----------------------------------------------------------
//
// Function : HcdPdd_PowerDown
//
// This function is called when the system is in suspend
// state
//
// Paramter :
//   hDeviceContext - pointer to PDD object
//
// Return:
//   Nil
//
//-----------------------------------------------------------
extern void HcdPdd_PowerDown(DWORD hDeviceContext)
{
    SEHCDPdd * pPddObject = (SEHCDPdd *)hDeviceContext;

    HcdMdd_PowerDown(pPddObject->lpvEHCDMddObject);

    return;
}

//----------------------------------------------------------
//
// Function : HcdPdd_Deinit
//
// This function is called when the USB host driver is de-initialized
//
// Paramter :
//   hDeviceContext - pointer to PDD object
//
// Return:
//   TRUE - success
//
//-----------------------------------------------------------
extern BOOL HcdPdd_Deinit(DWORD hDeviceContext)
{
    SEHCDPdd * pPddObject = (SEHCDPdd *)hDeviceContext;

    if(pPddObject->lpvEHCDMddObject)
        HcdMdd_DestroyHcdObject(pPddObject->lpvEHCDMddObject);
    if(pPddObject->lpvMemoryObject)
        HcdMdd_DestroyMemoryObject(pPddObject->lpvMemoryObject);
    if(pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);

    if (pPddObject->IsrHandle) {
        FreeIntChainHandler(pPddObject->IsrHandle);
        pPddObject->IsrHandle = NULL;
    }

    if (pPddObject->hParentBusHandle)
        CloseBusAccessHandle(pPddObject->hParentBusHandle);

    free(pPddObject);
    return TRUE;

}

//----------------------------------------------------------
//
// Function : HcdPdd_Open
//
// This function is called when the USB device is opened
//
// Paramter :
//   Not used
//
// Return:
//   1
//
//-----------------------------------------------------------
extern DWORD HcdPdd_Open(DWORD hDeviceContext, DWORD AccessCode,
        DWORD ShareMode)
{
    UnusedParameter(hDeviceContext);
    UnusedParameter(AccessCode);
    UnusedParameter(ShareMode);

#ifdef USB_IF_ELECTRICAL_TEST_MODE
    return (DWORD) (((SEHCDPdd*)hDeviceContext)->lpvEHCDMddObject);
#endif //#ifdef USB_IF_ELECTRICAL_TEST_MODE

    return 1; // we can be opened, but only once!
}

//----------------------------------------------------------
//
// Function : HcdPdd_Open
//
// This function is called when the USB device is closed
//
// Paramter :
//   Not used
//
// Return:
//   1
//
//-----------------------------------------------------------
extern BOOL HcdPdd_Close(DWORD hOpenContext)
{
    UnusedParameter(hOpenContext);

    return TRUE;
}

//----------------------------------------------------------
//
// Function : HcdPdd_Read
//
// This function is called when the USB device is read (Stream API)
//
// Paramter :
//   Not used
//
// Return:
//   -1
//
//-----------------------------------------------------------
extern DWORD HcdPdd_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pBuffer);
    UnusedParameter(Count);

    return (DWORD)-1; // an error occured
}

//----------------------------------------------------------
//
// Function : HcdPdd_Write
//
// This function is called when the USB device is written (Stream API)
//
// Paramter :
//   Not used
//
// Return:
//   -1
//
//-----------------------------------------------------------
extern DWORD HcdPdd_Write(DWORD hOpenContext, LPCVOID pSourceBytes,
        DWORD NumberOfBytes)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pSourceBytes);
    UnusedParameter(NumberOfBytes);

    return (DWORD)-1;
}

//----------------------------------------------------------
//
// Function : HcdPdd_Seek
//
// This function is called when the USB device is seek (Stream API)
//
// Paramter :
//   Not used
//
// Return:
//   -1
//
//-----------------------------------------------------------
extern DWORD HcdPdd_Seek(DWORD hOpenContext, LONG Amount, DWORD Type)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(Amount);
    UnusedParameter(Type);

    return (DWORD)-1;
}

//----------------------------------------------------------
//
// Function : HcdPdd_IOControl
//
// This function is called when the USB device is IOCTRL (Stream API)
//
// Paramter :
//   Not used
//
// Return:
//   FALSE
//
//-----------------------------------------------------------
extern BOOL HcdPdd_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
        DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(dwCode);
    UnusedParameter(pBufIn);
    UnusedParameter(dwLenIn);
    UnusedParameter(pBufOut);
    UnusedParameter(dwLenOut);
    UnusedParameter(pdwActualOut);

    return FALSE;
}


// Manage WinCE suspend/resume events

// This gets called by the MDD's IST when it detects a power resume.
// By default it has nothing to do.
extern void HcdPdd_InitiatePowerUp (DWORD hDeviceContext)
{
    return;
}

#pragma optimize( "", on )



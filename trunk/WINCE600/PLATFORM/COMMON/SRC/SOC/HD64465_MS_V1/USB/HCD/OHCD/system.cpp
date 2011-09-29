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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*

  Copyright(c) 1998,1999 Renesas Technology Corp.

	Module Name:

		system.c

	Revision History:

		26th April 1999		Released
		14th June  1999		Put the sources in order

*/
#include <windows.h>
#include <hcdddsi.h>
#include <cc.h>

// Registry key and value names
#define OHCI_DRIVER_KEY         TEXT("Drivers\\BuiltIn\\OHCI")
#define USE_EXISTING_VALUE_NAME TEXT("UseExistingSettings")
#define IRQ_VALUE_NAME          TEXT("Irq")
#define IOBASE_VALUE_NAME       TEXT("MemBase")

// Amount of memory to use for HCD buffer

static const DWORD gcTotalAvailablePhysicalMemory = 0x1000; // 4K total Sram size for HD64465
static const DWORD gcHighPriorityPhysicalMemory = 0; // 0 due to limited memory on HD64465

#define HD64465_USB_REGADDR		((PVOID)(HD64465_BASE + HD64465_USB_OFFSET)) // HCD  base register
#define HD64465_USB_REGSIZE		0x0060
#define HD64465_HCCA_PHYADDR	((PVOID)(HD64465_BASE + HD64465_EMBEDED_SDRAM_OFFSET)) // HCCA base register

typedef struct _SOhcdPdd
{
    LPVOID lpvMemoryObject;
    LPVOID lpvOhcdMddObject;
} SOhcdPdd;

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


/* GetRegistryConfig
 *
 *   Function to get the IRQ and I/O port range from the registry.  
 *   Note: Will need to be changed to support multiple instances.
 *
 * Return Value:
 *   TRUE for success, FALSE for error
 */
static BOOL
GetRegistryConfig(
    DWORD * lpdwUseExistingSettings, // OUT- Receives value that indicates whether to 
                                     //      just use the resources assigned by the BIOS.  
    DWORD * lpdwIrq,    // OUT- Receives IRQ value
    DWORD * lpdwIoBase) // OUT- Receives I/O base
{
    HKEY hKey;
    DWORD dwData;
    DWORD dwSize;
    DWORD dwType;
    BOOL  fRet=FALSE;
    DWORD dwRet;

    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,OHCI_DRIVER_KEY,0,0,&hKey);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!OHCD:GetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"),
                             OHCI_DRIVER_KEY, dwRet));
        return FALSE;
    }
    dwSize = sizeof(dwData);
    dwRet = RegQueryValueEx(hKey,USE_EXISTING_VALUE_NAME,0,&dwType,(PUCHAR)&dwData,&dwSize);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (TEXT("!OHCD:GetRegistryConfig RegQueryValueEx(%s) failed %d\r\n"),
                              USE_EXISTING_VALUE_NAME, dwRet));
        goto GetRegistryConfig_exit;
    }
    *lpdwUseExistingSettings = dwData;

    dwSize = sizeof(dwData);
    dwRet = RegQueryValueEx(hKey,IRQ_VALUE_NAME,0,&dwType,(PUCHAR)&dwData,&dwSize);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (TEXT("!OHCD:GetRegistryConfig RegQueryValueEx(%s) failed %d\r\n"),
                              IRQ_VALUE_NAME, dwRet));
        goto GetRegistryConfig_exit;
    }
    *lpdwIrq = dwData;

    dwSize = sizeof(dwData);
    dwRet = RegQueryValueEx(hKey,IOBASE_VALUE_NAME,0,&dwType,(PUCHAR)&dwData,&dwSize);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!OHCD:GetRegistryConfig RegQueryValueEx(%s) failed %d\r\n"),
                             IOBASE_VALUE_NAME, dwRet));
        goto GetRegistryConfig_exit;
    }
    *lpdwIoBase = dwData;
    fRet = TRUE;

GetRegistryConfig_exit:
    RegCloseKey(hKey);
    return fRet;
}   // GetRegistryConfig


/* SetRegistryConfig
 *
 *   Function to set the IRQ and I/O port range in the registry.
 *   Note: Will need to be changed to support multiple instances.
 *
 * Return Value:
 *   TRUE for success, FALSE for error
 */
static BOOL
SetRegistryConfig(
    DWORD dwIrq,    // IN - IRQ value
    DWORD dwIoBase) // IN - I/O base
{
    HKEY hKey;
    BOOL  fRet=FALSE;
    DWORD dwRet;

    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,OHCI_DRIVER_KEY,0,0,&hKey);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!OHCD:SetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"),
                             OHCI_DRIVER_KEY, dwRet));
        return FALSE;
    }

    dwRet = RegSetValueEx(hKey,IRQ_VALUE_NAME,0,REG_DWORD,(PUCHAR)&dwIrq,sizeof(DWORD));
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (TEXT("!OHCD:SetRegistryConfig RegQueryValueEx(%s) failed %d\r\n"),
                              IRQ_VALUE_NAME, dwRet));
        goto SetRegistryConfig_exit;
    }

    dwRet = RegSetValueEx(hKey,IOBASE_VALUE_NAME,0,REG_DWORD,(PUCHAR)&dwIoBase,sizeof(DWORD));
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!OHCD:SetRegistryConfig RegQueryValueEx(%s) failed %d\r\n"),
                             IOBASE_VALUE_NAME, dwRet));
        goto SetRegistryConfig_exit;
    }
    fRet = TRUE;

SetRegistryConfig_exit:
    RegCloseKey(hKey);
    return fRet;
}   // SetRegistryConfig


/* HcdMdd_CreateMemoryObject2
 *
 *  Initialize the shared memory area used by the HCI driver.
 *  Note: Bypass invalid ASSERT in HcdMdd_CreateMemoryObject.
 *
 * Return Value:
 *  Return pointer to CPhysMem object, or NULL if error.
 */
#include <cphysmem.hpp>

static LPVOID
HcdMdd_CreateMemoryObject2(
    DWORD cbSize,
    DWORD cbHighPrioritySize,
    PUCHAR pVirtAddr,
    PUCHAR pPhysAddr)
{
    CPhysMem * pobMem = new CPhysMem(cbSize,cbHighPrioritySize,pVirtAddr,pPhysAddr);
    if(pobMem)
    {
        if(!pobMem->InittedOK())
        {
            delete pobMem;
            pobMem = 0;
        }
    }
    return pobMem;
}

/* InitializeOHCI
 *
 *  Configure and initialize OHCI card
 *
 * Return Value:
 *  Return TRUE if card could be located and configured, otherwise FALSE
 */
static BOOL 
InitializeOHCI(
    SOhcdPdd * pPddObject,    // IN - Pointer to PDD structure
    LPCWSTR szDriverRegKey)   // IN - Pointer to active registry key string
{
    DWORD dwUseExistingSettings;
    DWORD dwIoPortBase = 0;
    DWORD dwSysIntr, dwIRQ;
    BOOL fResult = FALSE;
    LPVOID pobMem = NULL;
    LPVOID pobOhcd = NULL;
    LPVOID pvUsbRegister;
    LPVOID pvUsbHcca;

    if (!GetRegistryConfig(&dwUseExistingSettings, &dwIRQ, &dwIoPortBase)) {
        RETAILMSG(1,(TEXT("!OHCD: Error reading registry settings\r\n")));
        return FALSE;
    }
    DEBUGMSG(ZONE_INIT,(TEXT("OHCD: Read config from registry: Use existing: %u, IRQ: %u, I/O base: %X\r\n"),
                        dwUseExistingSettings,dwIRQ,dwIoPortBase));

    pvUsbRegister = VirtualAlloc(0, HD64465_USB_REGSIZE, MEM_RESERVE, PAGE_NOACCESS);

	if ( pvUsbRegister == NULL ) 
	{
		ERRORMSG(1, (TEXT("InitializeOHCI: VirtualAlloc failed! (1)\r\n")));
	}
	if ( !VirtualCopy(pvUsbRegister, (LPVOID)(dwIoPortBase+HD64465_USB_OFFSET), HD64465_USB_REGSIZE, PAGE_READWRITE|PAGE_NOCACHE) ) 
	{
		ERRORMSG(1, (TEXT("InitializeOHCI: VirtualCopy failed! (1)\r\n")));
	}

    fResult=1;

    if(fResult)
    {

		dwSysIntr=dwIRQ;

        // The PDD can supply a buffer of contiguous physical memory here, or can let the 
        // MDD try to allocate the memory from system RAM.  In our case, let the MDD do it.

   	    pvUsbHcca = VirtualAlloc(0,gcTotalAvailablePhysicalMemory, MEM_RESERVE, PAGE_NOACCESS);
	    if ( pvUsbHcca == NULL ) 
	    {
		    ERRORMSG(1, (TEXT("InitializeOHCI: VirtualAlloc failed! (2)\r\n")));
	    }
	    if ( !VirtualCopy(pvUsbHcca, (LPVOID)(dwIoPortBase+HD64465_EMBEDED_SDRAM_OFFSET),gcTotalAvailablePhysicalMemory, PAGE_READWRITE|PAGE_NOCACHE) ) 
	    {
		    ERRORMSG(1, (TEXT("InitializeOHCI: VirtualCopy failed! (1)\r\n")));
	    }
	
        pobMem = HcdMdd_CreateMemoryObject2(gcTotalAvailablePhysicalMemory, 
                                            gcHighPriorityPhysicalMemory, (PUCHAR)pvUsbHcca,(PUCHAR)(dwIoPortBase+HD64465_EMBEDED_SDRAM_OFFSET)); 

        if(pobMem)
        {
            pobOhcd = HcdMdd_CreateHcdObject(pPddObject, pobMem,
                    szDriverRegKey, (PUCHAR)pvUsbRegister, dwSysIntr);

            fResult = pobOhcd ? TRUE : FALSE;
        }
        else
            fResult = FALSE;

        if(!fResult)
        {
            if(pobOhcd)
                HcdMdd_DestroyHcdObject(pobOhcd);
            if(pobMem)
                HcdMdd_DestroyMemoryObject(pobMem);

            pobOhcd = NULL;
            pobMem = NULL;
        }
    }

    pPddObject->lpvMemoryObject = pobMem;
    pPddObject->lpvOhcdMddObject = pobOhcd;

    HcdMdd_SetCapability(pPddObject->lpvOhcdMddObject, HCD_ROOT_HUB_INTERRUPT);
    return fResult;
}

/* HcdPdd_Init
 *
 *   PDD Entry point - called at system init to detect and configure OHCI card.
 *
 * Return Value:
 *   Return pointer to PDD specific data structure, or NULL if error.
 */
extern DWORD 
HcdPdd_Init(
    DWORD dwContext)  // IN - Pointer to context value. For device.exe, this is a string 
                      //      indicating our active registry key.
{
    SOhcdPdd *  pPddObject = (SOhcdPdd*)malloc(sizeof(SOhcdPdd));
    BOOL        fRet = FALSE;

    if(pPddObject)
    {
        fRet = InitializeOHCI(pPddObject, (LPCWSTR)dwContext);
    }

    if(!fRet)
    {
        free(pPddObject);
        pPddObject = NULL;
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
    UnusedParameter(bPort);

    return ((dwCfgPower + dwTotalPower) > 500) ? FALSE : TRUE;
}

extern void HcdPdd_PowerUp(DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;
    HcdMdd_PowerUp(pPddObject->lpvOhcdMddObject);

    return;
}


extern void HcdPdd_PowerDown(DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;
    HcdMdd_PowerDown(pPddObject->lpvOhcdMddObject);

    return;
}


extern BOOL HcdPdd_Deinit(DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;

    if(pPddObject->lpvOhcdMddObject)
        HcdMdd_DestroyHcdObject(pPddObject->lpvOhcdMddObject);
    if(pPddObject->lpvMemoryObject)
        HcdMdd_DestroyMemoryObject(pPddObject->lpvMemoryObject);

    return TRUE;
}


extern DWORD HcdPdd_Open(DWORD hDeviceContext, DWORD AccessCode,
        DWORD ShareMode)
{
    UnusedParameter(hDeviceContext);
    UnusedParameter(AccessCode);
    UnusedParameter(ShareMode);

    return 1; // we can be opened, but only once!
}


extern BOOL HcdPdd_Close(DWORD hOpenContext)
{
    UnusedParameter(hOpenContext);

    return TRUE;
}


extern DWORD HcdPdd_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pBuffer);
    UnusedParameter(Count);

    return (DWORD)-1; // an error occured
}


extern DWORD HcdPdd_Write(DWORD hOpenContext, LPCVOID pSourceBytes,
        DWORD NumberOfBytes)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pSourceBytes);
    UnusedParameter(NumberOfBytes);

    return (DWORD)-1;
}


extern DWORD HcdPdd_Seek(DWORD hOpenContext, LONG Amount, DWORD Type)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(Amount);
    UnusedParameter(Type);

    return (DWORD)-1;
}


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

extern void HcdPdd_InitiatePowerUp(DWORD dwDeviceContext)
{
    UnusedParameter(dwDeviceContext);
}

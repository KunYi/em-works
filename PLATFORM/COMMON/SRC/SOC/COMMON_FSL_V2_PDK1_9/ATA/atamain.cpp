//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  atamain.cpp
//
//       IDE/ATA and ATA/ATAPI are stream interface drivers.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <ata.h>

// DLL instance handle; differentiate this driver from other ATAPI instances
// (what other buses?)
HINSTANCE g_hInstance;

// List of active devices/disks
CDisk *g_pDiskRoot = NULL;

// Protect global variables
CRITICAL_SECTION g_csMain;

// Debug
extern void CSPATASYSINTRInit();
extern void CSPATASYSINTRDisable();

extern "C" BOOL RegisterDbgZones(HMODULE hMod, LPDBGPARAM lpdbgparam);

// Definitions

typedef CDisk *(* POBJECTFUNCTION)(HKEY hKey);

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
//
//    Function : CPort
//           CPort Constructor
//    Parameters:
//           void
//    Return:
//           void
//
//------------------------------------------------------------------------------

CPort::CPort()
{
    InitializeCriticalSection(&m_csPort);
    // initialize flags
    m_fInitialized = 0;
    m_dwFlag = 0;
    // initialize I/O ports
    m_dwRegBase = 0;
    m_dwRegAlt = 0;
    m_dwBMR = 0;
    m_dwBMRStatic = 0;
    // initialize interrupt data
    m_hIRQEvent = NULL;
    m_hThread = NULL;
    CSPATASYSINTRInit();
}

//------------------------------------------------------------------------------
//
//    Function : ~CPort
//           CPort Destructor
//    Parameters:
//           void
//    Return:
//           void
//
//------------------------------------------------------------------------------

CPort::~CPort(
    )
{
    DeleteCriticalSection(&m_csPort);
    // unmap ATA channel's I/O windows
    if (m_dwRegBase) {
        MmUnmapIoSpace((LPVOID)m_dwRegBase, ATA_REG_LENGTH);
    }
    if (m_dwRegAlt) {
        MmUnmapIoSpace((LPVOID)m_dwRegAlt, ATA_REG_LENGTH);
    }
    if (m_dwBMR) {
        MmUnmapIoSpace((LPVOID)m_dwBMR, 16);
    }
    // close interrupt event handle
    if (m_hIRQEvent) {
        CloseHandle(m_hIRQEvent);
    }
    // close interrupt thread
    if (m_hThread) {
        CloseHandle(m_hThread);
    }
    // disable interrupt
    CSPATASYSINTRDisable();
    // free DSK_ registry value set
    if (m_pDskReg[0]) {
        LocalFree(m_pDskReg[0]);
    }
    if (m_pDskReg[1]) {
        LocalFree(m_pDskReg[1]);
    }
}

//------------------------------------------------------------------------------
//
//    Function : TakeCS
//           Acquire exclusive access to IDE/ATA channel's I/O window
//    Parameters:
//           void
//    Return:
//           void
//
//------------------------------------------------------------------------------

VOID
CPort::TakeCS(
    )
{
    EnterCriticalSection(&m_csPort);
}

//------------------------------------------------------------------------------
//
//    Function : ReleaseCS
//          Release exclusive access to IDE/ATA channel's I/O window
//    Parameters:
//          void
//    Return:
//          void
//
//------------------------------------------------------------------------------

VOID
CPort::ReleaseCS(
    )
{
    LeaveCriticalSection(&m_csPort);
}

//------------------------------------------------------------------------------
//
//    Function : AtaLoadRegKey
//           Write I/O window and interrupt data to debug output
//
//    Parameters:
//           void
//    Return:
//           void
//
//------------------------------------------------------------------------------

VOID
CPort::PrintInfo(
    )
{
    DEBUGMSG(ZONE_INIT, (TEXT("dwRegBase   = %08X\r\n"), m_dwRegBase));
    DEBUGMSG(ZONE_INIT, (TEXT("dwRegAlt    = %08X\r\n"), m_dwRegAlt));
}

//------------------------------------------------------------------------------
//
//    Function : AtaLoadRegKey
//           This function is used by an Xxx_Init function to fetch the name of
//           and return a handle to the instance/device ("Key") key from an
//           Active key
//    Parameters:
//          hActiveKey : handle to DSK active Key
//          pszDevKey  : pointer to device key
//    Return:
//          return handle to DSK active key, otherwise return NULL handle.
//
//------------------------------------------------------------------------------

HKEY
AtaLoadRegKey(
    HKEY hActiveKey,
    TCHAR **pszDevKey
    )
{
    DWORD dwValueType;        // registry value type
    DWORD dwValueLength = 0;  // registry value length
    PTSTR szDeviceKey = NULL; // name of device key; value associated with "Key"
    HKEY hDeviceKey = NULL;   // handle to device key; handle to "Key"

    // query the value of "Key" with @dwValueLength=0, to determine the actual
    // length of the value (so as to allocate the exact amount of memory)

    if (ERROR_SUCCESS == RegQueryValueEx(hActiveKey, DEVLOAD_DEVKEY_VALNAME, NULL, &dwValueType, NULL, &dwValueLength)) {

        // allocate just enough memory to store the value of "Key"
        szDeviceKey = (PTSTR)LocalAlloc(LPTR, dwValueLength);
        if (szDeviceKey) {

            // read the actual value of "Key" and null terminate the target buffer
            RegQueryValueEx(hActiveKey, DEVLOAD_DEVKEY_VALNAME, NULL, &dwValueType, (PBYTE)szDeviceKey, &dwValueLength);
            DEBUGCHK(dwValueLength != 0);
            szDeviceKey[(dwValueLength / sizeof(TCHAR)) - 1] = 0;

            // open the device key
            if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDeviceKey, 0, 0, &hDeviceKey)) {
                DEBUGMSG(ZONE_INIT, (_T(
                    "AtaLoadRegyKey> Failed to open %s\r\n"
                    ), szDeviceKey));
                hDeviceKey = NULL;
            }
        }
    }
    if (!hDeviceKey) {
        if (szDeviceKey) {
            LocalFree(szDeviceKey);
        }
        *pszDevKey = NULL;
    }
    else {
        *pszDevKey = szDeviceKey;
    }
    return hDeviceKey;
}

//------------------------------------------------------------------------------
//
//    Function : AtaIsValidDisk
//          This function is used to determine whether a target disk instance
//          is valid
//    Parameters:
//          pDisk : Pointer Cdisk object
//
//    Return:
//          On success return true, Otherwise return flase.
//
//------------------------------------------------------------------------------

BOOL
AtaIsValidDisk(
    CDisk *pDisk
    )
{
    CDisk *pTemp = g_pDiskRoot;
    while (pTemp) {
        if (pTemp == pDisk) {
            return TRUE;
        }
        pTemp = pTemp->m_pNextDisk;
    }
    return FALSE;
}

//------------------------------------------------------------------------------
//
//    Function : AtaGetRegistryResources
//          This function is used to fetch an IDE/ATA channel's I/O window from
//          its instance key; this function recovers gracefully if an OEM has a
//          proprietary registry configuration that doesn't specify bus type or
//          bus number
//    Parameters:
//          hDevKey : Handle to active device key
//          pdwi : Pointer to window info
//
//    Return:
//          On success return true, Otherwise return flase.
//
//------------------------------------------------------------------------------

BOOL
AtaGetRegistryResources(
    HKEY hDevKey,
    PDDKWINDOWINFO pdwi
    )
{
    DEBUGCHK(pdwi != NULL);

    if (!pdwi) {
        return FALSE;
    }

    // fetch I/O window information
    pdwi->cbSize = sizeof(*pdwi);
    if (ERROR_SUCCESS != ::DDKReg_GetWindowInfo(hDevKey, pdwi)) {
        return FALSE;
    }

    // if interface not specified, then assume PCI
    if (pdwi->dwInterfaceType == InterfaceTypeUndefined) {
        DEBUGMSG(ZONE_WARNING, (_T(
            "Ata!AtaGetRegistryResources> bus type not specified, using PCI as default\r\n"
            )));
        pdwi->dwInterfaceType = PCIBus;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//    Function : GetDSKRegistryValueSet
//         This function reads the DSK registry value set from the IDE/ATA
//         controller's registry key
//
//    Parameters:
//         hDSKInstanceKey : Active registry instance key
//         pDskReg : Pointer to registry value structure.
//
//    Return:
//       On success return true, Otherwise return flase.
//
//------------------------------------------------------------------------------

BOOL
GetDSKRegistryValueSet(
    HKEY hDSKInstanceKey,
    PDSKREG pDskReg
    )
{
    BOOL fRet;

    // fetch device ID
    fRet = AtaGetRegistryValue(hDSKInstanceKey, REG_VAL_DSK_DEVICEID, &pDskReg->dwDeviceId);
    if (!fRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Failed to read %s from DSK instance key\r\n"
            ), REG_VAL_DSK_DEVICEID));
        return FALSE;
    }
    if (!((0 <= pDskReg->dwDeviceId) && (pDskReg->dwDeviceId <= 3))) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Bad value(%d) for %s in DSK instance key; valid: {0, 1, 2, 3}\r\n"
            ), pDskReg->dwDeviceId, REG_VAL_DSK_DEVICEID));
        return FALSE;
    }

    // fetch interrupt driven I/O boolean
    fRet = AtaGetRegistryValue(hDSKInstanceKey, REG_VAL_DSK_INTERRUPTDRIVEN, &pDskReg->dwInterruptDriven);
    if (!fRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Failed to read %s from DSK instance key\r\n"
            ), REG_VAL_DSK_INTERRUPTDRIVEN));
        return FALSE;
    }
    if (pDskReg->dwInterruptDriven >= 2) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Bad value(%d) for %s in DSK instance key; valid: {0, 1}\r\n"
            ), pDskReg->dwInterruptDriven, REG_VAL_DSK_INTERRUPTDRIVEN));
        return FALSE;
    }

    // fetch DMA triple
    fRet = AtaGetRegistryValue(hDSKInstanceKey, REG_VAL_DSK_DMA, &pDskReg->dwDMA);
    if (!fRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Failed to read %s from DSK instance key\r\n"
            ), REG_VAL_DSK_DMA));
        return FALSE;
    }
    if (pDskReg->dwDMA >= 0x03) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Bad value(%d) for %s in DSK instance key; valid: {0=no DMA, 1 or 2=MDMA/UDMA}\r\n"
            ), pDskReg->dwDMA, REG_VAL_DSK_DMA));
        return FALSE;
    }

    // fetch double buffer size
    fRet = AtaGetRegistryValue(hDSKInstanceKey, REG_VAL_DSK_DOUBLEBUFFERSIZE, &pDskReg->dwDoubleBufferSize);
    if (!fRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Failed to read %s from DSK instance key\r\n"
            ), REG_VAL_DSK_DOUBLEBUFFERSIZE));
        return FALSE;
    }
    if (
        (pDskReg->dwDoubleBufferSize < REG_VAL_DSK_DOUBLEBUFFERSIZE_MIN) ||
        (pDskReg->dwDoubleBufferSize > REG_VAL_DSK_DOUBLEBUFFERSIZE_MAX)
    ) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Bad value(%d) for %s in DSK instance key; valid: {%d, ..., %d}\r\n"
            ), pDskReg->dwDoubleBufferSize, REG_VAL_DSK_DOUBLEBUFFERSIZE, REG_VAL_DSK_DOUBLEBUFFERSIZE_MIN, REG_VAL_DSK_DOUBLEBUFFERSIZE_MAX));
        return FALSE;
    }

    // fetch DRQ data block size; this has to be validated by a sub-class
    fRet = AtaGetRegistryValue(hDSKInstanceKey, REG_VAL_DSK_DRQDATABLOCKSIZE, &pDskReg->dwDrqDataBlockSize);
    if (!fRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Failed to read %s from DSK instance key\r\n"
            ), REG_VAL_DSK_DRQDATABLOCKSIZE));
        return FALSE;
    }

    // fetch write cache boolean
    fRet = AtaGetRegistryValue(hDSKInstanceKey, REG_VAL_DSK_WRITECACHE, &pDskReg->dwWriteCache);
    if (!fRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Failed to read %s from DSK instance key\r\n"
            ), REG_VAL_DSK_WRITECACHE));
        return FALSE;
    }
    if (pDskReg->dwWriteCache >= 2) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Bad value(%d) for %s in DSK instance key; valid: {0, 1}\r\n"
            ), pDskReg->dwWriteCache, REG_VAL_DSK_WRITECACHE));
        return FALSE;
    }

    // fetch look-ahead boolean
    fRet = AtaGetRegistryValue(hDSKInstanceKey, REG_VAL_DSK_LOOKAHEAD, &pDskReg->dwLookAhead);
    if (!fRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Failed to read %s from DSK instance key\r\n"
            ), REG_VAL_DSK_LOOKAHEAD));
        return FALSE;
    }
    if (pDskReg->dwLookAhead >= 2) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Bad value(%d) for %s in DSK instance key; valid: {0, 1}\r\n"
            ), pDskReg->dwLookAhead, REG_VAL_DSK_LOOKAHEAD));
        return FALSE;
    }

    // fetch transfer mode
    fRet = AtaGetRegistryValue(hDSKInstanceKey, REG_VAL_DSK_TRANSFERMODE, &pDskReg->dwTransferMode);
    if (!fRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Failed to read %s from DSK instance key\r\n"
            ), REG_VAL_DSK_TRANSFERMODE));
        return FALSE;
    }

    // fetch IORDY Enable
    fRet = AtaGetRegistryValue(hDSKInstanceKey, REG_VAL_DSK_IORDYENABLE, &pDskReg->dwIORDYEnable);
    if (!fRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!GetDSKRegistryValueSet> Failed to read %s from DSK instance key\r\n"
            ), REG_VAL_DSK_IORDYENABLE));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//    Function : DSK_Init
//       This function is called as a result of IDE_Init calling ActivateDevice on
//       HKLM\Drivers\@BUS\@IDEAdapter\DeviceX, to initialize a master or slave
//       device on a particular IDE/ATA channel of a particular IDE/ATA controller.
//       That is, an "IDE" driver is a bus driver for devices to one its IDE/ATA
//       controller's channels.
//
//       This function is responsible for creating a CDisk instance to associate
//       with a device.  This function reads the "Object" value from its instance
//       key to determine which CDisk (sub)type to instantiate and calls Init on the
//       CDisk instance to initialize the device.  If the device is not present, then
//       Init will fail.  The "Object" value maps to a function that creates an
//       instance of the target CDisk (sub)type.
//
//       Note that this driver model is convoluted.  A CDisk (sub)type instance
//       corresponds to both an IDE/ATA controller and an ATA/ATAPI device.
//
//    Parameters:
//       dwContext - pointer to string containing the registry path to the active key
//       of the associated device; the active key contains a key to the device's instance
//       key, which stores all of the device's configuration information
//
//    Return:
//       On success, return handle to device (to identify device); this handle is
//       passed to all subsequent DSK_Xxx calls.  Otherwise, return null.
//
//------------------------------------------------------------------------------

#define DSKINIT_UNDO_CLS_KEY_ACTIVE 0x1
#define DSKINIT_UNDO_CLS_KEY_DEVICE 0x2

CPort *pPort;                  // port

EXTERN_C
DWORD
DSK_Init(
    DWORD dwContext
    )
{
    DWORD dwUndo = 0;                     // undo bitset

    PTSTR szActiveKey = (PTSTR)dwContext; // name of device's active key
    HKEY hActiveKey;                      // handle to device's active key
    PTSTR szDevKey = NULL;                // name of device's instance key
    HKEY hDevKey = NULL;                  // handle to device's instance key

    DWORD dwDeviceId = 0;                 // device ID; 0 => master, 1 => slave

    CDisk *pDisk = NULL;                  // return

    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_Init+")));

    // guard global data; i.e., g_pDiskRoot

    EnterCriticalSection(&g_csMain);

    // open device's active key

    if ((ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, szActiveKey, 0, 0, &hActiveKey))) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!DSK_Init> Failed to open device's active key(%s)\r\n"
            ), szActiveKey));
        goto exit;
    }
    dwUndo |= DSKINIT_UNDO_CLS_KEY_ACTIVE;
    DUMPREGKEY(ZONE_INIT, szActiveKey, hActiveKey);

    // read name of and open device's instance key from device's active key
    if ((hDevKey = AtaLoadRegKey(hActiveKey, &szDevKey)) == 0) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!DSK_Init> Failed to fetch/open device's instance key from device's active key(%s)\r\n"
            ), szActiveKey));
        goto exit;
    }
    dwUndo |= DSKINIT_UNDO_CLS_KEY_DEVICE;
    DUMPREGKEY(ZONE_INIT, szDevKey, hDevKey);

    // fetch heap address of port instance from device's instance key

    pDisk = CreateMXHD(hDevKey);
    pPort = new CPort();
    dwDeviceId = 0;

    PDSKREG  pDskReg;                        // ATA/ATAPI device's registry value set

    pDskReg = (PDSKREG)LocalAlloc(LPTR, sizeof(DSKREG));
    // PREFAST
    if(pPort != NULL) {
        pPort->m_pDskReg[0] = pDskReg;
        pPort->m_pDskReg[1] = pDskReg;
    }

    if (!GetDSKRegistryValueSet(hDevKey, pDskReg)) {
            DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
                "Ata!DSK_Init> Failed to read DSK_ registry value set from registry; device key(%s)\r\n"
                ), szDevKey));
    }

    // if successful, write the name of the device's active and instance keys to
    // its CDisk instance, and add the CDisk instance to the IDE/ATA bus driver's
    // list of active disk devices

    if (pDisk) {

        // allocate the sterile, maximal SG_REQ for safe I/O
        pDisk->m_pSterileIoRequest = (PSG_REQ)LocalAlloc(
            LPTR,
            (sizeof(SG_REQ) + ((MAX_SG_BUF) - 1) * sizeof(SG_BUF))
            );

        if (NULL == pDisk->m_pSterileIoRequest) {
            delete pDisk;
            pDisk = NULL;
            goto exit;
        }

        // this information is used for ATA/ATAPI power management

        pDisk->SetActiveKey(szActiveKey);
        pDisk->SetDeviceKey(szDevKey);

        // inform the CDisk instance as to which device it is

        pDisk->m_pPort = pPort;
        pDisk->m_dwDeviceId = dwDeviceId;
        pDisk->m_dwDevice = dwDeviceId;

        // configure register block

        pDisk->ConfigureRegisterBlock(4);

        if(!CSPATAEnableClock(TRUE)) // Enable the clock before initialization
        {
            DEBUGMSG(ZONE_WARNING, (TEXT(" ATA Clock cannot be enabled!\r\n")));
        }
        
        // initialize device

        if (!pDisk->Init(hActiveKey)) {
            delete pDisk;
            pDisk = NULL;
            if(!CSPATAEnableClock(FALSE)) // Disable clock if init fails
            {
                DEBUGMSG(ZONE_WARNING, (TEXT(" ATA Clock cannot be disabled!\r\n")));
            }
            goto exit;
        }

        pDisk->GetDeviceInfo(&pDisk->m_storagedeviceinfo);

        if(!CSPATAEnableClock(FALSE)) // Disable clock after initialization
        {
            DEBUGMSG(ZONE_WARNING, (TEXT(" ATA Clock cannot be disabled!\r\n")));
        }

        // add CDisk instance to IDE/ATA controller's list of active devices

        pDisk->m_pNextDisk = g_pDiskRoot;
        g_pDiskRoot = pDisk;
    }

exit:;

    // clean up
    if (NULL == pDisk) {
        if (dwUndo & DSKINIT_UNDO_CLS_KEY_ACTIVE) {
            RegCloseKey(hActiveKey);
        }
        if (dwUndo & DSKINIT_UNDO_CLS_KEY_DEVICE) {
            RegCloseKey(hDevKey);
        }
        // pPort is deleted in IDE_Deinit
    }
    if (szDevKey) {
        LocalFree(szDevKey);
    }

    LeaveCriticalSection(&g_csMain);

    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_Init-")));
    return (DWORD)pDisk;
}

//------------------------------------------------------------------------------
//
//   Function : DSK_Deinit
//         This function deallocates the associated CDisk instance.
//
//   Parameters:
//         dwHandle - pointer to associated CDisk instance (initially returned
//         by DSK_Init)
//
//Return:
//    This function always succeeds.
//
//------------------------------------------------------------------------------

EXTERN_C
BOOL
DSK_Deinit(
    DWORD dwHandle
    )
{
    CDisk *pDiskPrev = NULL;
    CDisk *pDiskCur = g_pDiskRoot;

    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_Deinit+")));

    EnterCriticalSection(&g_csMain);

    // find the CDisk instance in global CDisk list

    while (pDiskCur) {
        if (pDiskCur == (CDisk *)dwHandle) {
            break;
        }
        pDiskPrev = pDiskCur;
        pDiskCur = pDiskCur->m_pNextDisk;
    }

    // remove CDisk instance from global CDisk list

    if (pDiskCur) {
        if (pDiskPrev) {
            pDiskPrev = pDiskCur->m_pNextDisk;
        }
        else {
            g_pDiskRoot = pDiskCur->m_pNextDisk;
        }
        delete pDiskCur;
    }

    // PREFAST
    if (NULL != pPort) {
        delete pPort;
        pPort = NULL;
    }
    LeaveCriticalSection(&g_csMain);
    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_Deinit-")));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//   Function : DSK_Open
//       This function opens a CDisk instance for use.
//
//   Parameters:
//      dwHandle - pointer to associated CDisk instance (initially returned by
//      DSK_Init)
//      dwAccess - specifes how the caller would like too use the device (read
//      and/or write) [this argument is ignored]
//      dwShareMode - specifies how the caller would like this device to be
//      shared [this argument is ignored]
//
//   Return:
//      On success, return handle to "open" CDisk instance; this handle is the
//      same as dwHandle.  Otherwise, return null.
//
//------------------------------------------------------------------------------

EXTERN_C
DWORD
DSK_Open(
    HANDLE dwHandle,
    DWORD dwAccess,
    DWORD dwShareMode
    )
{
    CDisk *pDisk = (CDisk *)dwHandle;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwAccess);
    UNREFERENCED_PARAMETER(dwShareMode);
    
    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_Open+")));

    EnterCriticalSection(&g_csMain);

    // validate the CDisk instance

    if (!AtaIsValidDisk(pDisk)) {
        pDisk = NULL;
    }

    LeaveCriticalSection(&g_csMain);

    // if the CDisk instance is valid, then open; open just increments the
    // instance's open count

    if (pDisk) {
        pDisk->Open();
    }
    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_Open-")));
    return (DWORD)pDisk;
}

//------------------------------------------------------------------------------
//
//   Function : DSK_Close
//       This function closes a CDisk instance.
//
//   Parameters:
//       dwHandle - pointer to associated CDisk instance (initially returned by
//       DSK_Init)
//
//   Return:
//       On success, return true.  Otherwise, return false.
//
//------------------------------------------------------------------------------

EXTERN_C
BOOL
DSK_Close(
    DWORD dwHandle
    )
{
    CDisk *pDisk = (CDisk *)dwHandle;

    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_Close+")));

    EnterCriticalSection(&g_csMain);

    // validate the CDisk instance

    if (!AtaIsValidDisk(pDisk)) {
        pDisk = NULL;
    }

    LeaveCriticalSection(&g_csMain);

    // if CDisk instance is valid, then close; close just decrements the
    // instance's open count

    if (pDisk) {
        pDisk->Close();
    }
    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_Close-")));
    return (pDisk != NULL);
}

//------------------------------------------------------------------------------
//
//   Function : DSK_IOControl
//        This function processes an IOCTL_DISK_Xxx/DISK_IOCTL_Xxx I/O control.
//
//   Parameters:
//       dwHandle - pointer to associated CDisk instance (initially returned by
//       DSK_Init)
//       dwIOControlCode - I/O control to perform
//       pInBuf - pointer to buffer containing the input data of the I/O control
//       nInBufSize - size of pInBuf (bytes)
//       pOutBuf - pointer to buffer that is to receive the output data of the
//       I/O control
//       nOutBufSize - size of pOutBuf (bytes)
//       pBytesReturned - pointer to DWORD that is to receive the size (bytes)
//       of the output data of the I/O control
//       pOverlapped - ignored
//
//   Return:
//        On success, return true.  Otherwise, return false.
//
//------------------------------------------------------------------------------

EXTERN_C
BOOL
DSK_IOControl(
    DWORD dwHandle,
    DWORD dwIoControlCode,
    PBYTE pInBuf,
    DWORD nInBufSize,
    PBYTE pOutBuf,
    DWORD nOutBufSize,
    PDWORD pBytesReturned,
    PDWORD pOverlapped)
{
    CDisk *pDisk = (CDisk *)dwHandle;
    DWORD SafeBytesReturned = 0;
    BOOL fRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pOverlapped);
    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_IOControl+")));

    EnterCriticalSection(&g_csMain);

    // validate CDisk instance

    if (!AtaIsValidDisk(pDisk)) {
        pDisk = NULL;
    }


    if (!pDisk) {
        return FALSE;
    }

    // DISK_IOCTL_INITIALIZED is a deprecated IOCTL; what does PostInit do?

    if (dwIoControlCode == DISK_IOCTL_INITIALIZED) {
        fRet = pDisk->PostInit((PPOST_INIT_BUF)pInBuf);
    }
    else {

        IOREQ IOReq;

        // build I/O request structure

        memset(&IOReq, 0, sizeof(IOReq));
        IOReq.dwCode = dwIoControlCode;
        IOReq.pInBuf = pInBuf;
        IOReq.dwInBufSize = nInBufSize;
        IOReq.pOutBuf = pOutBuf;
        IOReq.dwOutBufSize = nOutBufSize;
        IOReq.pBytesReturned = &SafeBytesReturned;
        IOReq.hProcess = (HANDLE)GetDirectCallerProcessId();

        // perform I/O control

        __try {
            fRet = pDisk->PerformIoctl(&IOReq);

            // if the caller supplied pBytesReturned, then write the value
            // from our safe copy

            if (pBytesReturned) {
                *pBytesReturned = SafeBytesReturned;
            }

        } __except(filter()) {
            fRet = FALSE;
            SetLastError(ERROR_GEN_FAILURE);
        }
    }


    LeaveCriticalSection(&g_csMain);

    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_IOControl-")));
    return fRet;
}

//------------------------------------------------------------------------------
//
//   Function : DSK_PowerUp
//          This function resumes the device.
//
//   Parameters:
//          None
//
//   Return:
//          On success, return true.  Otherwise, return false.
//
//------------------------------------------------------------------------------

EXTERN_C
VOID
DSK_PowerUp(
    VOID
    )
{
    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_PowerUp+")));
    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_PowerUp-")));
}

//------------------------------------------------------------------------------
//
//  Function : DSK_PowerDown
//         This function suspends a device.
//
//  Parameters:
//         None
//
//  Return:
//         On success, return true.  Otherwise, return false.
//
//------------------------------------------------------------------------------

EXTERN_C
VOID
DSK_PowerDown(
    VOID
    )
{
    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_PowerDown+")));
    DEBUGMSG(ZONE_FUNC, (_T("ATA_MX: DSK_PowerDown-")));
}

//------------------------------------------------------------------------------
//
//  Function: DllMain
//        This function is the main ATAPI.DLL entry point.
//
//  Parameters:
//        hInstance - a handle to the dll; this value is the base address of the
//        DLL
//        dwReason - the reason for the DLL is being entered
//        lpReserved - not used
//
//  Return:
//        On success, return true.  Otherwise, return false.
//
//------------------------------------------------------------------------------
BOOL
WINAPI
DllMain(
    HANDLE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpReserved);
    switch (dwReason) {

    case DLL_PROCESS_ATTACH:

        // initialize global data
        g_hInstance = (HINSTANCE)hInstance;
        InitializeCriticalSection(&g_csMain);
        // register debug zones
        RegisterDbgZones((HMODULE)hInstance, &dpCurSettings);
        DisableThreadLibraryCalls((HMODULE)hInstance);
        DEBUGMSG(ZONE_FUNC, (_T("ATA DLL_PROCESS_ATTACH\r\n")));

        break;

    case DLL_PROCESS_DETACH:

        // deinitialize global data
        DeleteCriticalSection(&g_csMain);
        DEBUGMSG(ZONE_FUNC, (TEXT("ATA DLL_PROCESS_DETACH\r\n")));

        break;
    }

    return TRUE;
}


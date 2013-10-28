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
//  File:  kitlioctl.c
//
#include <windows.h>
#include <oal.h>
#include <x86kitl.h>
#include <oal_kitl.h>
#include <specstrings.h>
#include <resmgr.h>

static const int SERIAL_KITL_PORT = 2;

//------------------------------------------------------------------------------
//
//  Function:  RegisterKITL
//
//  This function is used to update registry for KITL PCI info.
//  It is almost the same as the function "OALPCIRegisterAsUsed" except calling the function "PCIReadBARs".
//  It will fill up complete info on registry, includes all BAR's address and length.
//
void RegisterKITL(__out_opt PCI_REG_INFO* pPCIInfo);

//------------------------------------------------------------------------------
//
//  Function:  ReserveKitlIRQ
//
// If we're using an interrupt for KITL, make sure nothing else can share its IRQ
// by reserving it in the resource manager
//
static BOOL ReserveKitlIRQ()
{
    BOOL retVal = FALSE;
    UINT32 kitlFlags;

    if(!OALKitlGetFlags(&kitlFlags))
    {
        KITL_RETAILMSG(ZONE_WARNING,("WARN: Reserve KITL IRQ: Could not get KITL flags, bailing out\r\n"));
        return FALSE;
    }

    // Are we using active, interrupt-based KITL?
    if((g_pNKGlobal->pfnKITLIoctl) &&
      (kitlFlags & OAL_KITL_FLAGS_ENABLED) &&
      !(kitlFlags & OAL_KITL_FLAGS_PASSIVE) &&
      !(kitlFlags & OAL_KITL_FLAGS_POLL))
    {
        DWORD dwStatus, dwDisp, dwEnableKITLSharedIRQ;
        DWORD dwSize = sizeof(DWORD);
        HKEY hkResourcesReserved;
        HKEY hkNoReserve;
        DWORD kitlIRQ = g_pX86Info->ucKitlIrq;
        const WCHAR szReservedIRQPath[] = L"Drivers\\Resources\\IRQ\\Reserved";
        const WCHAR szNoReservePath[] = L"Platform";

        // Check the registry to see if we've added a platform-specific key telling us disable IRQ sharing with the KITL NIC
        dwStatus = NKRegOpenKeyEx(HKEY_LOCAL_MACHINE, szNoReservePath, 0, 0, &hkNoReserve);
        if(!dwStatus) 
        {
            dwStatus = NKRegQueryValueEx(hkNoReserve, TEXT("DisableKITLSharedIRQ"), NULL, NULL, (BYTE*)(&dwEnableKITLSharedIRQ), &dwSize);
            if(!dwStatus) 
            {
                if(dwEnableKITLSharedIRQ == 1) 
                {
                    // Registry has requested that disable IRQ sharing for KITL (this can help verify debugger hangs
                    // that occur due to IRQ-sharing hangs)
 
                    // If we got here then go ahead and reserve the IRQ
                    dwStatus = NKRegCreateKeyEx(HKEY_LOCAL_MACHINE, szReservedIRQPath, 0, NULL, 0, 0, NULL, &hkResourcesReserved, &dwDisp);
                    if(!dwStatus)
                    {
                        dwStatus = NKRegSetValueEx(hkResourcesReserved, TEXT("KitlIRQ"), 0, REG_DWORD, (BYTE*)(&kitlIRQ), sizeof(DWORD));
                        if(!dwStatus)
                        {
                            KITL_RETAILMSG (ZONE_INIT,("Reserve KITL IRQ: Reserved IRQ %d.  Other device drivers will not load on this IRQ.\r\n",kitlIRQ));
                            retVal = TRUE;
                        }
                        else
                        {
                            KITL_RETAILMSG(ZONE_INIT, ("Reserve KITL IRQ: Error creating reserved value\r\n"));
                        }
                    }
                    else
                    {
                        KITL_RETAILMSG(ZONE_INIT, ("Reserve KITL IRQ: Error creating reserved key\r\n"));
                    }
                }
            }
            else
            {
                KITL_RETAILMSG(ZONE_INIT, ("Reserve KITL IRQ: DisableKITLSharedIRQ specified\r\n"));
            }
        }
    }
    else
    {
        // Not using active interrupt-based KITL, don't need to reserve an IRQ
        KITL_RETAILMSG(1, ("Reserve KITL IRQ: No IRQ reserved, KITL polling mode was specified\r\n"));        
        retVal = TRUE;
    }

    if(!retVal)
    {        
        KITL_RETAILMSG(ZONE_ERROR, ("Reserve KITL IRQ: No IRQ reserved, KITL NIC IRQ may be shared with other devices.\r\n"));
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
//  Function:  SetDeviceDriverFlags
//
static BOOL SetDeviceDriverFlags(
                                 __in LPCWSTR szKeyPath, 
                                 DWORD flags
                                 )
{
    BOOL rc = FALSE;
    HKEY hKey;
    DWORD value;

    // Open/create key
    if (NKRegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL, 0, 0, NULL,&hKey, &value) != ERROR_SUCCESS) 
        goto cleanUp;

    // Set value
    rc = NKRegSetValueEx(hKey, L"Flags", 0, REG_DWORD, (UCHAR*)&flags, sizeof(DWORD)) == ERROR_SUCCESS;

    // Close key
    NKRegCloseKey(hKey);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  GetDeviceDriverPath
//
static BOOL GetDeviceDriverPath(
                                 __in LPCWSTR szKeyName,
                                 DWORD cbDriverKeyPath,
                                 __out_bcount(cbDriverKeyPath) LPCWSTR szDriverKeyPath
                                 )
{
    BOOL rc = FALSE;
    HKEY hKey;
    DWORD value;
    WCHAR szSerialKeyName[] = L"Drivers\\BootArg\\";

    // Open/create key
    if (NKRegCreateKeyEx(HKEY_LOCAL_MACHINE, szSerialKeyName, 0, NULL, 0, 0, NULL,&hKey, &value) != ERROR_SUCCESS) 
        goto cleanUp;

    // Set value
    rc = NKRegQueryValueEx(hKey, szKeyName, NULL, NULL, (UCHAR*)(szDriverKeyPath), &cbDriverKeyPath) == ERROR_SUCCESS;

    // Close key
    NKRegCloseKey(hKey);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  DisableSerialDriverForKITLPort
//
static BOOL DisableSerialDriverForKITLPort()
{
    if(g_pX86Info->KitlTransport == KTS_SERIAL) 
    {
        // "SerialDbg " - the final blank character will be filled in later in this function
        WCHAR szSerialSubKeyName[] = L"SerialDbg ";
        WCHAR szSerialDriverKey[MAX_PATH];

        // convert the COM port to a character
        szSerialSubKeyName[_countof(szSerialSubKeyName)-2] = (WCHAR)SERIAL_KITL_PORT + '0';

        
        if (GetDeviceDriverPath(szSerialSubKeyName, sizeof(szSerialDriverKey), szSerialDriverKey) == FALSE
            || SetDeviceDriverFlags(szSerialDriverKey, DEVFLAGS_NOLOAD) == FALSE)
        {
            RETAILMSG(ZONE_ERROR, (L"Error: Failed to reserve KITL Serial Port, This could cause unexpected behavior including crashes"));
            return FALSE;
        }
    }

    return TRUE;
}

void UpdateResourceMap(const PCI_REG_INFO* KitlPCIInfo)
{
    DWORD i = 0;
    for (i = 0; i < KitlPCIInfo->IoBase.Num; ++i)
    {
        BOOL bRet = ResourceRequest(
            RESMGR_IOSPACE, 
            KitlPCIInfo->IoBase.Reg[i], 
            KitlPCIInfo->IoLen.Reg[i]);

        if (bRet == FALSE)
        {
            RETAILMSG(OAL_WARN, 
                (L"Failed to reserve KITL IO Memory, This could cause unexpected behavior including crashes"));
        }

    }
}

static BOOL g_fPostInit = FALSE;
BOOL IsAfterPostInit() { return g_fPostInit; }

//------------------------------------------------------------------------------
//
//  Function:  OEMKitlIoctl
//
//  Receives kernel IO Controls which allows KITL to react to them 
//
BOOL OEMKitlIoctl(
                  DWORD code, 
                  __in_bcount(inSize) void * pInBuffer, 
                  DWORD inSize, 
                  __out_bcount(outSize) void * pOutBuffer, 
                  DWORD outSize, 
                  __out DWORD * pOutSize
                  )
{
    BOOL fRet = FALSE;
    switch (code)
    {
        case IOCTL_HAL_INITREGISTRY:
            {
                PCI_REG_INFO PCIInfo;
                RegisterKITL(&PCIInfo);
                UpdateResourceMap(&PCIInfo);
            }
            DisableSerialDriverForKITLPort();
            ReserveKitlIRQ();
            NKSetLastError (ERROR_NOT_SUPPORTED);
            // return FALSE, and set last error to Not-supported so the IOCTL gets routed to OEMIoControl
            break;

        case IOCTL_HAL_POSTINIT:
            g_fPostInit = TRUE;
            __fallthrough;

        default:
            fRet = OALIoCtlVBridge (code, pInBuffer, inSize, pOutBuffer, outSize, (UINT32*)pOutSize);
    }

    return fRet;
}

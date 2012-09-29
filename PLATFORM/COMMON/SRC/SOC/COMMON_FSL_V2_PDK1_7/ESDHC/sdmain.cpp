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

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE

/*---------------------------------------------------------------------------
* Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
// Module Name:
//
//    main.cpp
//
// Abstract:
//
//    Freescale ESDHC Driver entry points
//
// Notes:
//
///////////////////////////////////////////////////////////////////////////////

#include "esdhc.h"

// initialize debug zones
SD_DEBUG_INSTANTIATE_ZONES(
                           TEXT("ESDHC"), // module name
                           ZONE_ENABLE_INIT | ZONE_ENABLE_ERROR | ZONE_ENABLE_WARN,
                           TEXT("Interrupts"),
                           TEXT("Send Handler "), 
                           TEXT("Responses"), 
                           TEXT("Receive Data"),                   
                           TEXT("Clock Control"), 
                           TEXT("Transmit Data"), 
                           TEXT("Function State Poll"), 
                           TEXT(""),
                           TEXT(""),
                           TEXT(""),
                           TEXT(""));


///////////////////////////////////////////////////////////////////////////////
//  DllEntry - the main dll entry point
//  Input:  hInstance - the instance that is attaching
//          Reason - the reason for attaching
//          pReserved - not much
//  Output: 
//  Return: Always TRUE
//  Notes:  this is only used to initialize the zones
///////////////////////////////////////////////////////////////////////////////
STDAPI_(BOOL) DllEntry(HINSTANCE  hInstance,
              ULONG      Reason,
              LPVOID     pReserved)
{
    UNREFERENCED_PARAMETER(pReserved);
    
    BOOL fRet = TRUE;

    if (Reason == DLL_PROCESS_ATTACH) {
        DEBUGREGISTER(hInstance);
        DisableThreadLibraryCalls((HMODULE) hInstance);

        if (!SDInitializeCardLib()) {
            fRet = FALSE;
        }
        else if (!SD_API_SUCCESS(SDHCDInitializeHCLib())) {
            SDDeinitializeCardLib();
            fRet = FALSE;
        }
    }
    else if (Reason == DLL_PROCESS_DETACH) {
        SDHCDDeinitializeHCLib();
        SDDeinitializeCardLib();
    }

    return fRet;
}

///////////////////////////////////////////////////////////////////////////////
//  Deinit - the deinit entry point for the driver
//  Input:  hDeviceContext - the context returned from SHC_Init
//  Output: 
//  Return: TRUE
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
extern "C"
BOOL SHC_Deinit(DWORD hDeviceContext)
{    
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC +Deinit\n")));

    PCESDHCBase pController = (PCESDHCBase) hDeviceContext;

    delete pController;
    
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC -Deinit\n")));

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//  Init - the init entry point for the CE driver instance
//  Input:  dwContext - the context passed from device manager
//  Output: 
//  Return: return DWORD identifer for the instance
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
extern "C"
DWORD SHC_Init(DWORD dwContext)
{
    LPCTSTR pszActiveKey = (LPCTSTR) dwContext;
    DWORD   dwRet = 0;

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC +Init\n")));    
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC Active RegPath: %s \n"), pszActiveKey));

    PREFAST_SUPPRESS(28197, "pController is not to be deleted until SHC_Deinit is called");
    PCESDHCBase pController = new CESDHCBase();
    if (!pController) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC Failed to create controller object\n")));
        goto EXIT;
    }

    if (!pController->Init(pszActiveKey)) {
        goto EXIT;
    }

    // Return the controller instance
    dwRet = (DWORD) pController;

EXIT:
    if ( (dwRet == 0) && pController ) {
        SHC_Deinit((DWORD) pController);
    }

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC -Init\n")));

    return dwRet;
}


///////////////////////////////////////////////////////////////////////////////
//  Open - the open entry point for the driver
//  Input:  hDeviceContext - the context returned from SHC_Init
//  Output: 
//  Return: pController context
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
extern "C"
DWORD SHC_Open(
               DWORD hDeviceContext,
               DWORD,
               DWORD
               )
{
    #pragma warning(push)
    #pragma warning(disable: 4100)
    PCESDHCBase pController = (PCESDHCBase) hDeviceContext;
    #pragma warning(pop)
    return (DWORD) pController;
}


///////////////////////////////////////////////////////////////////////////////
//  Close - the close entry point for the driver
//  Input:  hOpenContext - the context returned from SHC_Open
//  Output: 
//  Return: TRUE
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
extern "C"
BOOL SHC_Close(
               DWORD hOpenContext
               )
{
    UNREFERENCED_PARAMETER(hOpenContext);
    //PCESDHCBase pController = (PCESDHCBase) hOpenContext;
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//  PowerDown - the power down entry point for the driver
//  Input:  hDeviceContext - the device context from SHC_Init
//  Output: 
//  Return: 
//  Notes:  
//      Indicates powerdown 
///////////////////////////////////////////////////////////////////////////////
extern "C"
void SHC_PowerDown(DWORD hDeviceContext)
{
    PCESDHCBase pController = (PCESDHCBase) hDeviceContext;

    // shut down power to the card
    pController->m_dwCurrentPowerLevel = 0;
    pController->BspESDHCSetSlotVoltage(pController->m_dwCurrentPowerLevel);
    
    
}


///////////////////////////////////////////////////////////////////////////////
//  PowerUp - the power up entry point for the driver
//  Input:  hDeviceContext - the device context from SHC_Init
//  Output: 
//  Return:
//  Notes: 
//          On power up, the indication is made to the bus driver and the
//          IST is triggered in order to remove the current instance 
///////////////////////////////////////////////////////////////////////////////
extern "C"
void SHC_PowerUp(DWORD hDeviceContext)
{
    PCESDHCBase pController = (PCESDHCBase) hDeviceContext;

    // turn on power to the card
    pController->m_dwCurrentPowerLevel = SD_VDD_WINDOW_3_0_TO_3_1;
    pController->BspESDHCSetSlotVoltage(pController->m_dwCurrentPowerLevel);

    // Simulate a card ejection/insertion (in case a card has been swapped
    // while suspended!).    
    pController->m_bReinsertTheCard = TRUE;
    SetEvent(pController->m_hControllerISTEvent);
}


///////////////////////////////////////////////////////////////////////////////
//  IOControl - the iocontrol entry point for the driver
//  Input:  hOpenContext - the context returned from SHC_Open
//  Output: 
//  Return: TRUE, if success
//  Notes:  handles power management IOCTLs
///////////////////////////////////////////////////////////////////////////////
extern "C"
BOOL SHC_IOControl(
                   DWORD hOpenContext,
                   DWORD dwCode,
                   PBYTE pBufIn,
                   DWORD dwLenIn,
                   PBYTE pBufOut,
                   DWORD dwLenOut,
                   PDWORD pdwActualOut 
                   )
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("SDHC +IOControl\n")));

    PCESDHCBase pController = (PCESDHCBase) hOpenContext;
    DWORD dwErr = pController->IOControl(dwCode, pBufIn, dwLenIn, pBufOut,
        dwLenOut, pdwActualOut);

    if (dwErr != ERROR_SUCCESS) {
        SetLastError(dwErr);
    }

    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("SDHC -IOControl\n")));

    return (dwErr == ERROR_SUCCESS);
}


// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE


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

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE

// Driver entry points

#include <SDCardDDK.h>
#include "SDHC.h"
//#include "SDHCBus.h"
#include <nkintr.h>

// initialize debug zones
SD_DEBUG_INSTANTIATE_ZONES(
    TEXT("SDHC"), // module name
    ZONE_ENABLE_INIT | ZONE_ENABLE_ERROR | ZONE_ENABLE_WARN,
    //| SHC_SDBUS_INTERACT_ZONE_ON | SHC_BUSY_STATE_ZONE_ON,
    TEXT("Interrupts"),
    TEXT("Send Handler "), 
    TEXT("Responses"), 
    TEXT("Receive Data"),                   
    TEXT("Clock Control"), 
    TEXT("Transmit Data"), 
    TEXT("SDBus Interaction"), 
    TEXT("Card Busy State"),
    TEXT(""),
    TEXT(""),
    TEXT(""));

void SDHCDDeinit( HANDLE h );

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
    if ( Reason == DLL_PROCESS_ATTACH ) {
        SD_DEBUG_ZONE_REGISTER(hInstance, NULL);
        DisableThreadLibraryCalls((HMODULE) hInstance);

        if( !SDInitializeCardLib() )
        {
            return FALSE;
        }
        else if( !SD_API_SUCCESS( SDHCDInitializeHCLib() ) )
        {
            SDDeinitializeCardLib();
            return FALSE;
        }
    }
    else if ( Reason == DLL_PROCESS_DETACH ) 
    {
        SDHCDDeinitializeHCLib();
        SDDeinitializeCardLib();
    }
    
    return(TRUE);
}





extern "C"
BOOL SHC_Deinit(DWORD hDeviceContext);

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

    PCSDIOControllerBase pController = CreateSDIOController();
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
    
    CSDIOControllerBase *pController = (CSDIOControllerBase*) hDeviceContext;    
    pController->FreeHostContext( TRUE, TRUE );
    
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC -Deinit\n")));
    
    return TRUE;
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
    CSDIOControllerBase * pController = (CSDIOControllerBase*) hDeviceContext;
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
    CSDIOControllerBase * pController = (CSDIOControllerBase *) hOpenContext;
    return TRUE;
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
    BOOL rc = FALSE;
    
    CSDIOControllerBase * pController = (CSDIOControllerBase*) hOpenContext;
    
    // Check if we get correct context
    if (pController == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"ERROR: SHC_IOControl: "
            L"Incorrect context paramer\r\n" ));
        goto clean;
    }

    
    rc = pController->IOControl(dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
    
clean:
    return rc;
}

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE


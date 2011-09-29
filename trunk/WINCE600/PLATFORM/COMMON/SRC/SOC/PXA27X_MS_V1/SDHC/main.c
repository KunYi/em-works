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
//
// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE
//
// Module Name:
//
//    Main.c   
//
// Abstract:
//
//    Driver entry points for PXA27x SDIO driver 
//
// Notes:
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <bulverde.h>
#include <Devload.h>
#include "SDCardDDK.h"
#include "SDHCD.h"
#include "SD.h"

    // initialize debug zones
SD_DEBUG_INSTANTIATE_ZONES(
     TEXT("PXA27X SDIO Driver"), // module name
     ZONE_ENABLE_INIT | ZONE_ENABLE_ERROR | ZONE_ENABLE_WARN /*| SDH_SDBUS_INTERACTION_ZONE_ON*/, 
     // | SDH_RECEIVE_ZONE_ON | SDH_TRANSMIT_ZONE_ON | SDH_SEND_ZONE_ON,
     //| SDH_INTERRUPT_ZONE_ON,    // initial settings
     TEXT("Interrupts"),
     TEXT("Send Handler "), 
     TEXT("Responses"), 
     TEXT("Receive Data"),                   
     TEXT("Clock Control"), 
     TEXT("Transmit Data"), 
     TEXT("Function"), 
     TEXT("SDBus Interaction"),
     TEXT(""),
     TEXT(""),
     TEXT(""));

#define SDH_SLOTS 1
#define SDH_REGISTRY_BASE_PATH TEXT("Drivers\\SDCARD\\HostController\\XSDSC")

///////////////////////////////////////////////////////////////////////////////
//  CardDllEntry - the main dll entry point
//  Input:  hInstance - the instance that is attaching
//          Reason - the reason for attaching
//          pReserved - 
//  Output: 
//  Return: TRUE
//  Notes:  this is only used to initialize the zones
///////////////////////////////////////////////////////////////////////////////
BOOL DllEntry(HINSTANCE  hInstance,
                       ULONG      Reason,
                       LPVOID     pReserved)
{
    if ( Reason == DLL_PROCESS_ATTACH ) {
        SD_DEBUG_ZONE_REGISTER(hInstance, SDH_REGISTRY_BASE_PATH);
        DisableThreadLibraryCalls( (HMODULE) hInstance );

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

    if ( Reason == DLL_PROCESS_DETACH ) {
        SDHCDDeinitializeHCLib();
        SDDeinitializeCardLib();
    }

    return(TRUE);
}



///////////////////////////////////////////////////////////////////////////////
//  SDH_Deinit - the deinit entry point 
//  Input:  hDeviceContext - the context returned from SDH_Init
//  Output: 
//  Return: always returns TRUE
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL SDH_Deinit(DWORD hDeviceContext)
{
    PSDCARD_HC_CONTEXT pHostContext;

    DbgPrintZo(SDCARD_ZONE_INIT, (TEXT("SDH: +SDH_Deinit\n")));

    pHostContext = (PSDCARD_HC_CONTEXT)hDeviceContext;

        // deregister the host controller
    SDHCDDeregisterHostController(pHostContext);

    if( pHostContext && pHostContext->pHCSpecificContext )
    {
        free( pHostContext->pHCSpecificContext );
    }

        // cleanup the context
    SDHCDDeleteContext((PSDCARD_HC_CONTEXT)hDeviceContext);

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_Init - the init entry point 
//  Input:  dwContext - the context for this init
//  Output: 
//  Return: returns instance context
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
DWORD SDH_Init(DWORD dwContext)
{
    PSDCARD_HC_CONTEXT      pHostContext;   // new HC context
    SD_API_STATUS           status;         // SD status
    PSDH_HARDWARE_CONTEXT pController;    // new instance
    HKEY hKeyDevice;
    LPCTSTR pszActiveKey;

    DbgPrintZo(SDCARD_ZONE_INIT, (TEXT("SDH: +SDH_Init\n")));    

    DbgPrintZo(SDCARD_ZONE_INIT, (TEXT("SDH: Active RegPath: %s \n"),
        (PTSTR)dwContext));

    pController = NULL;

    // allocate the context
    status = SDHCDAllocateContext(SDH_SLOTS, 
                                  &pHostContext);

    if (!SD_API_SUCCESS(status)) {
        DbgPrintZo(SDCARD_ZONE_ERROR, 
            (TEXT("SDH: Failed to allocate context : 0x%08X \n"), status));
        return 0;
    }

    // create our extension 
    pController = (PSDH_HARDWARE_CONTEXT)malloc( sizeof(SDH_HARDWARE_CONTEXT) );
    if( pController == NULL )
    {
        DbgPrintZo(SDCARD_ZONE_ERROR, 
            (TEXT("SDH: Failed to allocate extension\n")));
        return 0;
    }
    memset( pController, 0, sizeof(SDH_HARDWARE_CONTEXT) );

    // Set our extension
    pHostContext->pHCSpecificContext = pController;

    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHostContext);

    pszActiveKey = (LPCTSTR) dwContext;
    
    pController->pszActiveKey = pszActiveKey;
    pController->hBusAccessHandle = CreateBusAccessHandle( pszActiveKey );

    hKeyDevice = OpenDeviceKey(pszActiveKey);
    if (!hKeyDevice || !LoadRegistrySettings(hKeyDevice, pController) ) {
        DbgPrintZo(SDCARD_ZONE_ERROR, 
        (TEXT("SDH: Failed load the registry settings\n")));
        return 0;
    }
    RegCloseKey( hKeyDevice );

    DbgPrintZo(SDCARD_ZONE_INIT, 
               (TEXT("SDH: Real RegPath: %s \n"),pController->RegPath));
   
    // save off the host context
    pController->pHCContext = pHostContext;

    // set the name
    SDHCDSetHCName(pHostContext, TEXT("Lubbock"));

    // set init handler
    SDHCDSetControllerInitHandler(pHostContext,SDInitialize);  
    // set deinit handler    
    SDHCDSetControllerDeinitHandler(pHostContext, SDDeinitialize);
    // set the bus request handler
    SDHCDSetBusRequestHandler(pHostContext,SDHBusRequestHandler);   
    // set the cancel I/O handler
    SDHCDSetCancelIOHandler(pHostContext, SDHCancelIoHandler);   
    // set the slot option handler
    SDHCDSetSlotOptionHandler(pHostContext, SDHSlotOptionHandler); 
    

    // now register the host controller 
    status = SDHCDRegisterHostController(pHostContext);

    if (!SD_API_SUCCESS(status)) {
        if( pController )
        {
            free( pController );
        }
        SDHCDDeleteContext(pHostContext);
        DbgPrintZo(SDCARD_ZONE_ERROR, 
                (TEXT("SDH: Failed to register host controller: %0x08X \n"),status));
        return 0;
    }

    DbgPrintZo(SDCARD_ZONE_INIT, (TEXT("SDH: -SDH_Init\n")));

    // return the Host Controller context
    return (DWORD)pHostContext;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_IOControl - the I/O control entry point
//  Input:  hOpenContext - the context returned from SDH_Open
//          dwCode - the ioctl code
//          pBufIn - the input buffer from the user
//          dwLenIn - the length of the input buffer
//          pBufOut - the output buffer from the user
//          dwLenOut - the length of the output buffer
//          pdwActualOut - the size of the transfer
//  Output: 
//  Return: FALSE
//  Notes:  Not used
///////////////////////////////////////////////////////////////////////////////
BOOL SDH_IOControl(DWORD   hOpenContext,
                   DWORD   dwCode,
                   PBYTE   pBufIn,
                   DWORD   dwLenIn,
                   PBYTE   pBufOut,
                   DWORD   dwLenOut,
                   PDWORD  pdwActualOut)
{
    DbgPrintZo(SDCARD_ZONE_FUNC, (TEXT("SDH: +-SDH_IOControl \n")));
    return FALSE;;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_Open - the open entry point for the bus driver
//  Input:  hDeviceContext - the device context from SDH_Init
//          AccessCode - the desired access
//          ShareMode - the desired share mode
//  Output: 
//  Return: 0
//  Notes:  not used
///////////////////////////////////////////////////////////////////////////////
DWORD SDH_Open(DWORD    hDeviceContext,
               DWORD    AccessCode,
               DWORD    ShareMode)
{
    DbgPrintZo(SDCARD_ZONE_FUNC, (TEXT("SDH: +-SDH_Open\n")));
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
//  SDH_Close - the close entry point 
//  Input:  hOpenContext - the context returned from SDH_Open
//  Output: 
//  Return: TRUE
//  Notes:  not used
///////////////////////////////////////////////////////////////////////////////
BOOL SDH_Close(DWORD hOpenContext)
{
    DbgPrintZo(SDCARD_ZONE_FUNC, (TEXT("SDH: +-SDH_Close\n")));
    return TRUE;
}

void SDControllerPowerDown( PSDH_HARDWARE_CONTEXT pController );
void SDControllerPowerUp( PSDH_HARDWARE_CONTEXT pController );

///////////////////////////////////////////////////////////////////////////////
//  SDH_PowerDown - the power down entry point
//  Input:  hDeviceContext - the device context from SDH_Init
//  Output: 
//  Notes:  preforms no actions
///////////////////////////////////////////////////////////////////////////////
void SDH_PowerDown(DWORD    hDeviceContext)
{
    PSDCARD_HC_CONTEXT pHostContext;
    PSDH_HARDWARE_CONTEXT pController;

    DbgPrintZo(SDCARD_ZONE_FUNC, (TEXT("SDH: +SDH_PowerDown\n")));

    pHostContext = (PSDCARD_HC_CONTEXT)hDeviceContext;
    ASSERT( pHostContext );
    if( pHostContext )
    {
        pController = pHostContext->pHCSpecificContext;
        SDControllerPowerDown( pController );
    }
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_PowerUp - the power up entry point 
//  Input:  hDeviceContext - the device context from SDH_Init
//  Output: 
//  Notes:  preforms no actions
///////////////////////////////////////////////////////////////////////////////
void SDH_PowerUp(DWORD  hDeviceContext)
{
    PSDCARD_HC_CONTEXT pHostContext;
    PSDH_HARDWARE_CONTEXT pController;

    DbgPrintZo(SDCARD_ZONE_FUNC, (TEXT("SDH: +SDH_PowerDown\n")));

    pHostContext = (PSDCARD_HC_CONTEXT)hDeviceContext;
    ASSERT( pHostContext );
    if( pHostContext )
    {
        pController = pHostContext->pHCSpecificContext;
        SDControllerPowerUp( pController );
    }
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_Read - the read entry point for the CE file system wrapper
//  Input:  hOpenContext - the context from SDH_Open
//          pBuffer - the user's buffer
//          Count - the size of the transfer
//  Output: 
//  Return: zero
//  Notes:  not used
///////////////////////////////////////////////////////////////////////////////
DWORD SDH_Read(DWORD    hOpenContext,
               LPVOID   pBuffer,
               DWORD    Count)
{
    DbgPrintZo(SDCARD_ZONE_FUNC, (TEXT("SDH: +-SDH_Read\n")));
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_Seek - the seek entry point for the CE file system wrapper
//  Input:  hOpenContext - the context from SDH_Open
//          Amount - the amount to seek
//          Type - the type of seek
//  Output: 
//  Return: zero
//  Notes:  not used
///////////////////////////////////////////////////////////////////////////////
DWORD SDH_Seek(DWORD    hOpenContext,
               long     Amount,
               DWORD    Type)
{
    DbgPrintZo(SDCARD_ZONE_FUNC, (TEXT("SDH: +-SDH_Seek\n")));
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_Write - the write entry point for the CE file system wrapper
//  Input:  hOpenContext - the context from SDH_Open
//          pBuffer - the user's buffer
//          Count - the size of the transfer
//  Output: 
//  Return: zero
//  Notes:  Not used
///////////////////////////////////////////////////////////////////////////////
DWORD SDH_Write(DWORD   hOpenContext,
                LPCVOID pBuffer,
                DWORD   Count)
{
    DbgPrintZo(SDCARD_ZONE_FUNC, (TEXT("SDH: +-SDH_Write\n")));
    return 0;
}

#define CONTROLLER_IRQ_TEXT _T("ControllerIRQ")
#define CONTROLLER_IST_PRI_TEXT _T("ControllerISTPriority")
#define CLOCK_ALWAYS_ON_TEXT _T("ClockAlwaysOn")
#define CLOCK_ON_IF_INTERRUPTS_ENABLED_TEXT _T("ClockOnIfInterruptsEnabled")
#define MAXIMUM_CLOCK_FREQUENCY_TEXT _T("MaximumClockFrequency")
#define CONTROLLER_DMA_ISR_DLL_TEXT _T("DMAIsrDll")
#define CONTROLLER_DMA_ISR_HANDLER_TEXT _T("DMAIsrHandler")
#define CONTROLLER_DMA_IRQ_TEXT _T("DMAIRQ")
#define CONTROLLER_DMA_SYSINT_TEXT _T("DMASysIntr")
#define CONTROLLER_DMA_CHANNEL_TEXT _T("DMAChannel")
#define CONTROLLER_DMA_IST_PRI_TEXT _T("DMAISTPriority")
#define CONTROLLER_DMA_BUFFER_SIZE_TEXT  _T("DMABufferSize")
#define CONTROLLER_POLLING_MODE_TEXT _T("PollingModeSize")

BOOL LoadRegistrySettings( HKEY hKeyDevice, PSDH_HARDWARE_CONTEXT pController )
{
    DWORD dwRegVal;
    DWORD dwDataSize;
    DWORD dwType;

    dwDataSize = sizeof(DWORD);
    if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_IRQ_TEXT,
                                          NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
        REG_DWORD == dwType )
    {
        pController->dwSDMMCIrq = dwRegVal;
    }
    else
    {
        return FALSE;
    }

    // get the DMA channel
    dwDataSize = sizeof(DWORD);
    if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_DMA_CHANNEL_TEXT,
                                          NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
        REG_DWORD == dwType )
    {
        if( dwRegVal > 31 )
        {
            pController->dwDmaChannel = 0xffffffff;
            return FALSE;
        }
        pController->dwDmaChannel = dwRegVal;

        // get the DMA IRQ
        dwDataSize = sizeof(DWORD);
        if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_DMA_IRQ_TEXT,
                                              NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
            REG_DWORD == dwType )
        {
            pController->dwDmaIRQ = dwRegVal;
        }
        else
        {
            pController->dwDmaIRQ = IRQ_DMAC;
        }

        // get the DMA buffer size
        dwDataSize = sizeof(DWORD);
        if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_DMA_BUFFER_SIZE_TEXT,
                                              NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
            REG_DWORD == dwType )
        {
            pController->dwDmaBufferSize = dwRegVal;
        }
        else
        {
            pController->dwDmaBufferSize = 0;
        }

        // get the DMA SysInt
        dwDataSize = sizeof(DWORD);
        if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_DMA_SYSINT_TEXT,
                                              NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
            REG_DWORD == dwType )
        {
            pController->dwDmaSysIntr = dwRegVal;
        }
        else
        {
            pController->dwDmaSysIntr = SYSINTR_UNDEFINED;
        }

        dwDataSize = sizeof(pController->wszDmaIsrDll);
        if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_DMA_ISR_DLL_TEXT,
                                              NULL, &dwType, (LPBYTE)pController->wszDmaIsrDll, &dwDataSize ) &&
            REG_SZ == dwType )
        {
        }
        else
        {
            pController->wszDmaIsrDll[0] = 0;
        }

        dwDataSize = sizeof(pController->wszDmaIsrHandler);
        if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_DMA_ISR_HANDLER_TEXT,
                                              NULL, &dwType, (LPBYTE)pController->wszDmaIsrHandler, &dwDataSize ) &&
            REG_SZ == dwType )
        {
        }
        else
        {
            pController->wszDmaIsrHandler[0] = 0;
        }

        dwDataSize = sizeof(DWORD);
        if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_DMA_IST_PRI_TEXT,
                                              NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
            REG_DWORD == dwType )
        {
            pController->DmaIstThreadPriority = dwRegVal;
        }
        else
        {
            pController->DmaIstThreadPriority = SDH_DMA_CONTROLLER_PRIORITY;
        }
    }
    else
    {
        pController->dwDmaChannel = 0xffffffff;
        pController->wszDmaIsrDll[0] = 0;
        pController->wszDmaIsrHandler[0] = 0;
    }

    dwDataSize = sizeof(DWORD);
    if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CLOCK_ALWAYS_ON_TEXT,
                                          NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
        REG_DWORD == dwType )
    {
        pController->fClockAlwaysOn = dwRegVal ? TRUE : FALSE;
    }
    else
    {
        pController->fClockAlwaysOn = FALSE;
    }

    dwDataSize = sizeof(DWORD);
    if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CLOCK_ON_IF_INTERRUPTS_ENABLED_TEXT,
                                          NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
        REG_DWORD == dwType )
    {
        pController->fClockOnIfInterruptsEnabled = dwRegVal ? TRUE : FALSE;
    }
    else
    {
        pController->fClockOnIfInterruptsEnabled = FALSE;
    }

    dwDataSize = sizeof(DWORD);
    if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_IST_PRI_TEXT,
                                          NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
        REG_DWORD == dwType )
    {
        pController->ControllerIstThreadPriority = dwRegVal;
    }
    else
    {
        pController->ControllerIstThreadPriority = SDH_CARD_CONTROLLER_PRIORITY;
    }

    dwDataSize = sizeof(DWORD);
    if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, MAXIMUM_CLOCK_FREQUENCY_TEXT,
                                          NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
        REG_DWORD == dwType )
    {
        pController->dwMaximumSDClockFrequency = dwRegVal;
    }
    else
    {
        pController->dwMaximumSDClockFrequency = MAXIMUM_SDCLOCK_FREQUENCY;
    }

    dwDataSize = sizeof(DWORD);
    if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_POLLING_MODE_TEXT,
                                          NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
        REG_DWORD == dwType )
    {
        pController->dwPollingModeSize = dwRegVal;
    }
    else
    {
        pController->dwPollingModeSize = NUM_BYTE_FOR_POLLING_MODE;
    }
    return LoadPlatformRegistrySettings(hKeyDevice);
}

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE


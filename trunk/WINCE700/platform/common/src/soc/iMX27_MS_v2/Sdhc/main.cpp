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
//---------------------------------------------------------------------------
//
// Copyright (C) 2003-2004, MOTOROLA, INC. All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:       drivers/sdhc/main.cpp
//  Purpose:    Entry points for SDHC driver
//
//-----------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <Devload.h>
#include "csp.h"
#include "sdhc.h"
#define CONTROLLER_IST_PRI_TEXT _T("ControllerISTPriority")
#define CONTROLLER_INDEX_PRI_TEXT _T("Index")
#define CONTROLLER_POLLING_MODE_TEXT _T("PollingModeSize")

#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("SDHC"),
    {
        _T("Init"), _T("DeInit"), _T("Ioctl"), _T(""),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T("DMA"),_T("Interrupt"),_T("Command"),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    (ZONEMASK_ERROR | ZONEMASK_WARN)
};
#endif

/*******************************************************************************
 GLOBAL OR STATIC VARIABLES
*******************************************************************************/

/*******************************************************************************
 STATIC FUNCTION PROTOTYPES
*******************************************************************************/
static BOOL LoadRegistrySettings(HKEY hKeyDevice, PSDH_HARDWARE_CONTEXT pController);

/*******************************************************************************
 EXPORTED FUNCTIONS
*******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//  CardDllEntry - the main dll entry point
//  Input:  hInstance - the instance that is attaching
//          Reason - the reason for attaching
//          pReserved -
//  Output:
//  Return: TRUE
//  Notes:  this is only used to initialize the zones
///////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllEntry(HINSTANCE  hInstance,
                       ULONG      Reason,
                       LPVOID     pReserved)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +DllEntry\n")));
    if ( Reason == DLL_PROCESS_ATTACH )
    {
        DEBUGMSG(ZONE_INFO, (TEXT("DLL_PROCESS_ATTACH\n")));
        DEBUGREGISTER((HMODULE) hInstance); //register debug zones
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

    if ( Reason == DLL_PROCESS_DETACH )
    {
        DEBUGMSG(ZONE_INFO, (TEXT("DLL_PROCESS_DETACH\n")));
        SDHCDDeinitializeHCLib();
        SDDeinitializeCardLib();
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: -DllEntry\n")));
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
    PSDH_HARDWARE_CONTEXT pController;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +SDH_Deinit\n")));

    pHostContext = (PSDCARD_HC_CONTEXT)hDeviceContext;
    pController = (PSDH_HARDWARE_CONTEXT)pHostContext->pHCSpecificContext ;

    // deregister the host controller
    SDHCDDeregisterHostController(pHostContext);

    // pHCSpecificContext stores address of SDH_HARDWARE_CONTEXT.
    // After SDHCDeregisterHostController(), the heap pointed by pHostContext
    // is freed. So now storing the pHCSpecificContext pointer locally and 
    // freeing it after SDHCDDeregisterHostController()
    if( pController )
    {
        free( pController );
    }

    // cleanup the context
    SDHCDDeleteContext((PSDCARD_HC_CONTEXT)pHostContext);


    DEBUGMSG(ZONE_FUNCTION  , (TEXT("SDH: -SDH_Deinit\n")));
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

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +SDH_Init\n")));
    DEBUGMSG(ZONE_INFO, (TEXT("SDH: Active RegPath: %s \n"),(PTSTR)dwContext));

    pController = NULL;
    // allocate the context
    status = SDHCDAllocateContext(SDH_SLOTS,  &pHostContext);
    if (!SD_API_SUCCESS(status))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SDH: Failed to allocate context : 0x%08X \n"), status));
        goto _errExit;
    }
    // create our extension
    pController = (PSDH_HARDWARE_CONTEXT)malloc( sizeof(SDH_HARDWARE_CONTEXT) );
    if( pController == NULL )
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SDH: Failed to allocate extension\n")));
        goto _errExit;
    }
    memset( pController, 0, sizeof(SDH_HARDWARE_CONTEXT) );
    // Set our extension
    pHostContext->pHCSpecificContext = pController;

    pszActiveKey = (LPCTSTR) dwContext;
    hKeyDevice = OpenDeviceKey(pszActiveKey);
    if (!hKeyDevice || !LoadRegistrySettings(hKeyDevice, pController) )
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SDH: Failed load the registry settings\n")));
        goto _errExit;
    }

    RegCloseKey( hKeyDevice );
    // save off the host context
    pController->pHCContext = pHostContext;

    //Set a unique name for each host controller
    if (pController->ControllerIndex == 1)
    {
        SDHCDSetHCName(pHostContext, TEXT("MX27_1"));
    }
    else if (pController->ControllerIndex == 2)
    {
        SDHCDSetHCName(pHostContext, TEXT("MX27_2"));
    }
    else if (pController->ControllerIndex == 3)
    {
        SDHCDSetHCName(pHostContext, TEXT("MX27_3"));
    }

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
    if (!SD_API_SUCCESS(status))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SDH: Failed to register host controller: %0x08X \n"),status));
        goto _errExit;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: -SDH_Init\n")));

    // return the Host Controller context
    return (DWORD)pHostContext;

    _errExit:
    if(pHostContext)
    {
        SDHCDDeleteContext(pHostContext);
    }
    if(pController)
    {
        free(pController);
    }
    if(hKeyDevice)
    {
        RegCloseKey(hKeyDevice);
    }
    return 0;
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
//  Return: TRUE if success else FALSE. SetLastError() if FALSE.
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
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +-SDH_IOControl \n")));

    PSDCARD_HC_CONTEXT pHostContext = (PSDCARD_HC_CONTEXT)hOpenContext;

    BOOL bRetVal = FALSE;

    switch (dwCode) {
    // Power management functions.
    // Return device specific power capabilities.
    case IOCTL_POWER_CAPABILITIES:
    {
        DEBUGMSG(ZONE_FUNCTION, (L"SDH_IOControl: IOCTL_POWER_CAPABILITIES\r\n"));
        // Check arguments.
        if ( pBufOut == NULL || dwLenOut < sizeof(POWER_CAPABILITIES))
        {
            DEBUGMSG(ZONE_FUNCTION, (L"SDH_IOControl: IOCTL_POWER_CAPABILITIES Invalid parameter.\r\n"));
            break;
        }

        PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
        // Clear capabilities structure.
        memset(ppc, 0, sizeof(POWER_CAPABILITIES));

        // Set power capabilities. Supports D0 and D4.
        ppc->DeviceDx = DX_MASK(D0)|DX_MASK(D4);

        DEBUGMSG(ZONE_FUNCTION, (L"SDH_IOControl: IOCTL_POWER_CAPABILITIES = 0x%x\r\n", ppc->DeviceDx));

        // Update returned data size.
        if (pdwActualOut)
            *pdwActualOut = sizeof(POWER_CAPABILITIES);
        bRetVal = TRUE;
        break;
    }


    case IOCTL_POWER_SET:
    {
        // Check arguments.
        if (pBufOut == NULL || dwLenOut < sizeof(CEDEVICE_POWER_STATE))
        {
            DEBUGMSG(ZONE_FUNCTION, (L"SDH_IOControl: IOCTL_POWER_SET Invalid parameter.\r\n"));
            break;
        }

        CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;

        DEBUGMSG(ZONE_FUNCTION, (L"SDH_IOControl: IOCTL_POWER_SET = %d.\r\n", NewDx));

        // Check for any valid power state.
        if (VALID_DX(NewDx))
        {
            if(NewDx != D0)
                NewDx = D4;
            SDSetPowerState(pHostContext, NewDx);
            bRetVal = TRUE;
        }
        else
        {
            DEBUGMSG(ZONE_FUNCTION, (L"SDH_IOControl: IOCTL_POWER_SET invalid power state.\r\n"));
        }
        break;
    }

    // Return the current device power state.
    case IOCTL_POWER_GET:
    {
        PSDH_HARDWARE_CONTEXT    pController;
        // get our extension
        pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHostContext);

        if(pBufOut != NULL && dwLenOut == sizeof(CEDEVICE_POWER_STATE))
        {
            CEDEVICE_POWER_STATE CurrentDx = pController->CurrentPowerState;
            *(PCEDEVICE_POWER_STATE)pBufOut = CurrentDx;

            if (pdwActualOut)
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
            bRetVal = TRUE;
        }
        else
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        break;
    }
    default:
        DEBUGMSG(ZONE_FUNCTION, (L"SDH_IOControl: Unknown IOCTL_xxx(0x%0.8X)\r\n", dwCode));
        break;

    }

    DEBUGMSG(ZONE_FUNCTION, (L"-SDH_IOControl(rc = %d)\r\n", bRetVal));
    return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_Open - the open entry point for the bus driver
//  Input:  hDeviceContext - the device context from SDH_Init
//          AccessCode - the desired access
//          ShareMode - the desired share mode
//  Output:
//  Return: Device context
//  Notes:  not used
///////////////////////////////////////////////////////////////////////////////
DWORD SDH_Open(DWORD    hDeviceContext,
               DWORD    AccessCode,
               DWORD    ShareMode)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +-SDH_Open \n")));
    PSDCARD_HC_CONTEXT pHostContext = (PSDCARD_HC_CONTEXT)hDeviceContext;
    return (DWORD) pHostContext;
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
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +-SDH_Close\n")));
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_PowerDown - the power down entry point
//  Input:  hDeviceContext - the device context from SDH_Init
//  Output:
//  Notes:  preforms no actions
///////////////////////////////////////////////////////////////////////////////
void SDH_PowerDown(DWORD    hDeviceContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +-SDH_PowerDown\n")));
    PSDCARD_HC_CONTEXT pHostContext = (PSDCARD_HC_CONTEXT)hDeviceContext;
    SDPowerDown(pHostContext);
    return;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_PowerUp - the power up entry point
//  Input:  hDeviceContext - the device context from SDH_Init
//  Output:
//  Notes:  preforms no actions
///////////////////////////////////////////////////////////////////////////////
void SDH_PowerUp(DWORD  hDeviceContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +-SDH_PowerUp\n")));
    PSDCARD_HC_CONTEXT pHostContext = (PSDCARD_HC_CONTEXT)hDeviceContext;
    SDPowerUp(pHostContext);
    return;
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
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +-SDH_Read\n")));
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
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +-SDH_Seek\n")));
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
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDH: +-SDH_Write\n")));
    return 0;
}

BOOL LoadRegistrySettings( HKEY hKeyDevice, PSDH_HARDWARE_CONTEXT pController )
{
    DWORD dwRegVal;
    DWORD dwDataSize;
    DWORD dwType;

    dwDataSize = sizeof(DWORD);
    if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_IST_PRI_TEXT,
                                          NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
        REG_DWORD == dwType )
    {
        pController->ControllerIstThreadPriority = dwRegVal;
    }
    else
    {
        pController->ControllerIstThreadPriority = SDHC_DEFAULT_CARD_CONTROLLER_PRIORITY;
    }
    if( ERROR_SUCCESS == RegQueryValueEx( hKeyDevice, CONTROLLER_INDEX_PRI_TEXT,
                                          NULL, &dwType, (LPBYTE)&dwRegVal, &dwDataSize ) &&
        REG_DWORD == dwType )
    {
        pController->ControllerIndex = dwRegVal;
    }
    else
    {
        return FALSE ;
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
    return BSPSdhcLoadPlatformRegistrySettings(hKeyDevice);
}

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE


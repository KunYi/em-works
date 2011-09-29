//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  sdhcmain.cpp
//
//------------------------------------------------------------------------------

#include "sdhc.h"

// initialize debug zones
SD_DEBUG_INSTANTIATE_ZONES(
                           TEXT("SDHC"), // module name
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

#define SDHC_SLOT               1

extern BOOL     bReinsertTheCard;
extern HANDLE   hCardDetectEvent;

PSDHC_HARDWARE_CONTEXT pController;    // new instance


///////////////////////////////////////////////////////////////////////////////
//  DllEntry - the main dll entry point
//  Input:  hInstance - the instance that is attaching
//          Reason - the reason for attaching
//          pReserved - not much
//  Output: 
//  Return: Always TRUE
//  Notes:  this is only used to initialize the zones
///////////////////////////////////////////////////////////////////////////////
STDAPI_(BOOL)DllEntry(HINSTANCE  hInstance,
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
BOOL SHC_Deinit(DWORD hDeviceContext)
{
    PSDCARD_HC_CONTEXT pHostContext;

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SSP1 SHC_Deinit\r\n")));

    pHostContext = (PSDCARD_HC_CONTEXT)hDeviceContext;

    // deregister the host controller
    SDHCDDeregisterHostController(pHostContext);

    if(pHostContext && pHostContext->pHCSpecificContext )
    {
        free(pHostContext->pHCSpecificContext);
    }

    // cleanup the context
    SDHCDDeleteContext((PSDCARD_HC_CONTEXT)hDeviceContext);

    if(pController)
    {
        free(pController);
        pController = NULL;
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//  FslSDHCIsEthKitlEnable - checks whether Ethernet kitl is enabled on the same SSP1 port
//  Input:
//
//  Output:
//  Return: TRUE if ETH kitl is enabled, FALSE otherwise
//  Notes:
//
///////////////////////////////////////////////////////////////////////////////
BOOL  FslSDHCIsEthKitlEnable()
{
    DWORD address = 0;
    if(!KernelIoControl(IOCTL_KITL_GET_INFO,NULL,0,&address, sizeof(address),NULL ))
        return FALSE;
    
    //ETH KITL address will map to SSP1
    if(address == CSP_BASE_REG_PA_SSP1 )
    {
        return TRUE;
    }
    
    return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
//  Init - the init entry point for the CE driver instance
//  Input:  dwContext - the context passed from device manager
//  Output: 
//  Return: return DWORD identifer for the instance
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
DWORD SHC_Init(DWORD dwContext)
{
    UNREFERENCED_PARAMETER(dwContext);
    PSDCARD_HC_CONTEXT    pHostContext;   // new HC context
    SD_API_STATUS         status;         // SD status

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDHC: +SHC_Init\n")));

    pController = NULL;

    //Checkout whether Ethernet kitl is enable. If ETH kitl enable, it is also using SSP1, so do not load the SD driver. 
    if(FslSDHCIsEthKitlEnable())
    {
        RETAILMSG(1,(_T("SDHC Driver load not possible because Ethernet Kitl Enabled\r\n")));
        return 0;
    }        

    // allocate the context
    status = SDHCDAllocateContext(SDHC_SLOT,
                                  &pHostContext);

    if(!SD_API_SUCCESS(status)) 
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC: Failed to allocate context : 0x%08X \n"), status));
        return 0;
    }

    // create our extension
    pController = (PSDHC_HARDWARE_CONTEXT)malloc(sizeof(PSDHC_HARDWARE_CONTEXT));
    if(pController == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC: Failed to allocate extension\n")));
        return 0;
    }
    memset(pController, 0, sizeof(SDHC_HARDWARE_CONTEXT));

    // Set our extension
    pHostContext->pHCSpecificContext = pController;

    pController = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHostContext);

    // save off the host context
    pController->pHCContext = pHostContext;

    // set the name
    SDHCDSetHCName(pHostContext, TEXT("SSP1"));

    // set init handler
    SDHCDSetControllerInitHandler(pHostContext, SDInitialize);
    // set deinit handler
    SDHCDSetControllerDeinitHandler(pHostContext, SDDeinitialize);
    // set the bus request handler
    SDHCDSetBusRequestHandler(pHostContext, SDHCBusRequestHandler);
    // set the cancel I/O handler
    SDHCDSetCancelIOHandler(pHostContext, SDHCCancelIoHandler);
    // set the slot option handler
    SDHCDSetSlotOptionHandler(pHostContext, SDHCSlotOptionHandler);


    // now register the host controller
    status = SDHCDRegisterHostController(pHostContext);

    if(!SD_API_SUCCESS(status)) 
    {
        if(pController)
        {
            free(pController);
        }
        SDHCDDeleteContext(pHostContext);
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDHC: Failed to register host controller: %0x08X \n"),status));
        return 0;
    }

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDHC: -SHC_Init\n")));

    // return the Host Controller context
    return (DWORD)pHostContext;
}

///////////////////////////////////////////////////////////////////////////////
//  SHC_IOControl - the I/O control entry point
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
BOOL SHC_IOControl(DWORD   hOpenContext,
                   DWORD   dwCode,
                   PBYTE   pBufIn,
                   DWORD   dwLenIn,
                   PBYTE   pBufOut,
                   DWORD   dwLenOut,
                   PDWORD  pdwActualOut)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(dwCode);
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);
    
    return FALSE;;
}

///////////////////////////////////////////////////////////////////////////////
//  Open - the open entry point for the driver
//  Input:  hDeviceContext - the context returned from SHC_Init
//  Output: 
//  Return: pController context
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
DWORD SHC_Open(DWORD    hDeviceContext,
               DWORD    AccessCode,
               DWORD    ShareMode)
{
    UNREFERENCED_PARAMETER(hDeviceContext);
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
//  Close - the close entry point for the driver
//  Input:  hOpenContext - the context returned from SHC_Open
//  Output: 
//  Return: TRUE
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL SHC_Close(DWORD hOpenContext)
{
    UNREFERENCED_PARAMETER(hOpenContext);

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
void SHC_PowerDown(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER(hDeviceContext);
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
void SHC_PowerUp(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER(hDeviceContext);
    DEBUGMSG(SDCARD_ZONE_INFO, (_T("SHC_PowerUp+++\r\n")));
    // Simulate a card ejection/insertion (in case a card has been swopped
    // while suspended!).
    bReinsertTheCard = TRUE;
    SetEvent(hCardDetectEvent);

    DEBUGMSG(SDCARD_ZONE_INFO, (_T("SHC_PowerUp---\r\n")));
}


// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE


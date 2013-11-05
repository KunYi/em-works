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
//  File:  backlight_pdd.cpp
//
//  Backlight PDD driver source code, Subarctic
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <pm.h>
#include "backlight_class.h"
#include "ceddkex.h"

//these are already defined.
#undef ZONE_ERROR
#undef ZONE_FUNCTION

#include <backlightddsi.h>


// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
// disable PREFAST warning for use of Backlight object 
#pragma warning (disable: 6320 6011)

CBacklightRoot* g_pBacklight;
///=============================================================================
/// Exported functions
///=============================================================================
///     backlight mdd & pdd interface
///         PddInit()
///         PddDeInit()
///         PddIOControl()
///         PddUpdateBacklight()
///
BOOL PddInit(DWORD);
VOID PddDeInit(DWORD);
BOOL PddUpdateBacklight(DWORD, CEDEVICE_POWER_STATE, BOOL, DWORD, DWORD*);
BOOL PddIOControl(DWORD, DWORD, LPBYTE, DWORD, LPBYTE, DWORD, LPDWORD);


//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

///=============================================================================
/// Local structures
///=============================================================================

// This structure is in place to store any local variables that the PDD
// might want to store, which is returned to the MDD
typedef struct {
    // Current Backlight device power state.
    CEDEVICE_POWER_STATE CurDx;
} BKL_PDD_INFO;

//==============================================================================
// Description: PddInit - PDD should always implement this function. MDD calls
//              it during initialization to fill up the function table with rest
//              of the DDSI functions.
//
// Arguments:   [IN] pszActiveKey - current active backlight driver key.
//              [IN] pMddIfc - MDD interface info
//              [OUT] pPddIfc - PDD interface (the function table to be filled)
//              [IN] hKeyToWatch - HKEY of the key watched by the MDD (e.g.
//                       HKEY_CURRENT_USER). This is configurable in the
//                       backlight registry read by the MDD (REG_REGKEY_VALUE).
//              [IN] szSubKey - subkey of hKeyToWatch if applicable (e.g.
//                       L"ControlPanel\\Backlight"). This is configurable in
//                       the backlight registry read by the MDD
//                      (REG_REGSUBKEY_VALUE).
//
// Ret Value:   The PDD context - pddInfo.
//
DWORD
WINAPI
PddInit(
    const LPCTSTR pszActiveKey,
    const BKL_MDD_INTERFACE_INFO* mddIfc,
    BKL_PDD_INTERFACE_INFO* pddIfc,
    const HKEY hKeyToWatch,
    const LPCTSTR szSubKey
    )
{
    BKL_PDD_INFO* pddInfo = NULL;

    UNREFERENCED_PARAMETER(mddIfc);
    UNREFERENCED_PARAMETER(hKeyToWatch);
    UNREFERENCED_PARAMETER(szSubKey);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddInit+\r\n")));

    pddInfo = (BKL_PDD_INFO*) LocalAlloc(LPTR, sizeof(BKL_PDD_INFO));
    if (!pddInfo)
    {
        //Failed to allocate Pdd structure
        DEBUGMSG(ZONE_ERROR, (TEXT("PddInit:: Alloc falied!\r\n")));
        return NULL;
    }


    pddIfc->version = 1;
    pddIfc->pfnDeinit = PddDeInit;
    pddIfc->pfnIoctl = PddIOControl;
    pddIfc->pfnUpdate = PddUpdateBacklight;
    
    g_pBacklight = GetBacklightObject();
    if (g_pBacklight == NULL)
    {
        goto error;
    }

    if (g_pBacklight->Initialize(pszActiveKey) == FALSE)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_Init: "
            L"Failed to initialize backlight object\r\n"
            ));
        goto error;
    }

    // Set Current backlight power state to D0
    g_pBacklight->SetPowerState(D0);
    pddInfo->CurDx = D0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddInit-\r\n")));

    return (DWORD) pddInfo;

error:
    g_pBacklight->Uninitialize();    

    return 0;
}

//==============================================================================
// Description: PddDeInit - MDD calls it during deinitialization. PDD should
//              deinit hardware and deallocate memory etc.
//
// Arguments:   [IN] pddContext. pddInfo returned in PddInit.
//
// Ret Value:   None
//
VOID
WINAPI
PddDeInit(
    DWORD pddContext
    )
{
    BKL_PDD_INFO* pddInfo = (BKL_PDD_INFO*) pddContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddDeInit+\r\n")));

    g_pBacklight->Uninitialize();

    if (g_pBacklight)
    {
        delete g_pBacklight;
    }

    if (pddInfo)
    {
        LocalFree(pddInfo);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddDeInit-\r\n")));
}

//==============================================================================
// Description: PddIOControl - IOCTLs passed to PDD.
//
// Arguments:   [IN] pddContext.  pddInfo returned in PddInit.
//
// Ret Value:   TRUE if success else FALSE. SetLastError() if FALSE.
//
BOOL
WINAPI
PddIOControl(
             DWORD  dwContext,
             DWORD  Ioctl,
             PUCHAR pInBuf,
             DWORD  InBufLen,
             PUCHAR pOutBuf,
             DWORD  OutBufLen,
             PDWORD pdwBytesTransferred
             )
{
    BOOL   bRc = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddIOControl+\r\n")));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwContext);
    UNREFERENCED_PARAMETER(Ioctl);
    UNREFERENCED_PARAMETER(pInBuf);

    UNREFERENCED_PARAMETER(InBufLen);
    UNREFERENCED_PARAMETER(pOutBuf);
    UNREFERENCED_PARAMETER(OutBufLen);
    UNREFERENCED_PARAMETER(pdwBytesTransferred);

    // we are not handling any IOCTLs here, everthing is done in MDD.

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddIOControl-\r\n")));

    return bRc;
}
//==============================================================================
// Description: PddUpdateBacklight - Update the current backlight state.
//
// Arguments:   [IN]  pddContext.  pddInfo returned in PddInit.
//              [IN]  powerState - current power state (D0...D4).
//              [IN]  fOnAC - whether the device on external power (TRUE if on
//                       external power else FALSE).
//              [IN]  dwBrightnessToSet - requested brightness to set.
//              [OUT] pdwActualBrightness - actual brightness of backlight upon
//                       return.
//
// Ret Value:   TRUE if success else FALSE.
//
BOOL
PddUpdateBacklight(
    DWORD pddContext,
    CEDEVICE_POWER_STATE powerState,
    BOOL fOnAC,
    DWORD dwBrightnessToSet,
    DWORD* pdwActualBrightness
    )
{
    BOOL fOk = FALSE;
    BKL_PDD_INFO* pddInfo = (BKL_PDD_INFO*) pddContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddUpdateBacklight+\r\n")));
    RETAILMSG(1, (TEXT("PddUpdateBacklight: %d+\r\n"), powerState));

    UNREFERENCED_PARAMETER(fOnAC);


    pddInfo->CurDx = powerState;

    fOk = g_pBacklight->SetPowerState(powerState);
    if (fOk)
    {
        *pdwActualBrightness = dwBrightnessToSet;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddUpdateBacklight-\r\n")));
    RETAILMSG(1, (TEXT("PddUpdateBacklight-\r\n")));
    return fOk;
}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This function is an optional method of entry into a DLL.
//
// Parameters:
//      hInstDll
//          [IN] Handle to the DLL.
//      dwReason
//          [IN] Specifies a flag indicating why the DLL entry-point
//               function is being called.
//      lpvReserved
//          [IN] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      This function returns a handle that identifies
//      the open context of the device to the calling application.
//
//-----------------------------------------------------------------------------
extern "C" BOOL WINAPI DllEntry(
                                   HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hInstDll);
    UNREFERENCED_PARAMETER(lpvReserved);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DllEntry+\r\n")));

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HINSTANCE)hInstDll);
            DEBUGMSG(ZONE_INFO, (TEXT("DllEntry: DLL_PROCESS_ATTACH\r\n")));
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INFO, (TEXT("DllEntry: DLL_PROCESS_DETACH\r\n")));
            break;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DllEntry-\r\n")));

    // return TRUE for success
    return TRUE;
}

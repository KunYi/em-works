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
/*
==============================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
==============================================================
*/
//
//  File:  smartreflexpolicy.cpp
//


//******************************************************************************
//  #INCLUDES
//******************************************************************************
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <dvfs.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <omap35xx_clocks.h>

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

#ifndef ZONE_ERROR
#define ZONE_ERROR          DEBUGZONE(0)
#endif

#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"ProxyDriver", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"Undefined" ,  L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0000
};

#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define POLICY_CONTEXT_COOKIE       'strx'


//-----------------------------------------------------------------------------
//  Local structures

typedef struct
{
    DWORD                           dwActiveCoreDevices;
    BOOL                               bDisplayLPROn;
    BOOL                               bSmartReflexState;
} SmartReflexPolicyInfo_t;

typedef struct
{
    DWORD                           cookie;
} SmartReflexPolicyContext_t;

//-----------------------------------------------------------------------------
//  local variables
static SmartReflexPolicyInfo_t          s_SmartReflexLoadPolicyInfo;

//-----------------------------------------------------------------------------
// 
//  Function:  EnableSmartReflex
//
//  Function that decide wether or not to turn on SmartReflex
//
VOID
UpdateSmartReflex()
{

    if(s_SmartReflexLoadPolicyInfo.bDisplayLPROn == FALSE &&
        s_SmartReflexLoadPolicyInfo.bSmartReflexState == FALSE
           )
        {
        RETAILMSG(ZONE_INFO,(L"SmartReflexPolicy: SmartReflex *ON*"));
        s_SmartReflexLoadPolicyInfo.bSmartReflexState = TRUE;
        KernelIoControl(
            IOCTL_SMARTREFLEX_CONTROL,
            &s_SmartReflexLoadPolicyInfo.bSmartReflexState, 
            sizeof(BOOL),
            NULL,
            0,
            NULL);
        }
    
    if(s_SmartReflexLoadPolicyInfo.bDisplayLPROn == TRUE &&
        s_SmartReflexLoadPolicyInfo.dwActiveCoreDevices == 0 &&
        s_SmartReflexLoadPolicyInfo.bSmartReflexState == TRUE
           )
        {
        RETAILMSG(ZONE_INFO,(L"SmartReflexPolicy: SmartReflex *OFF*"));
        s_SmartReflexLoadPolicyInfo.bSmartReflexState = FALSE;
        KernelIoControl(
            IOCTL_SMARTREFLEX_CONTROL,
            &s_SmartReflexLoadPolicyInfo.bSmartReflexState, 
            sizeof(BOOL),
            NULL,
            0,
            NULL);
        }

}


//-----------------------------------------------------------------------------
// 
//  Function:  SMARTREFLEX_InitPolicy
//
//  Policy  initialization
//

HANDLE
SMARTREFLEX_InitPolicy(
    _TCHAR const *szContext
    )
{
    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_InitPolicy(0x%08x)\r\n",
      szContext
      ));

    // initializt global structure
    memset(&s_SmartReflexLoadPolicyInfo, 0, sizeof(SmartReflexPolicyInfo_t));       
    s_SmartReflexLoadPolicyInfo.dwActiveCoreDevices = 0x0;
    s_SmartReflexLoadPolicyInfo.bDisplayLPROn = FALSE;
    s_SmartReflexLoadPolicyInfo.bSmartReflexState = FALSE;


    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_InitPolicy()\r\n"));
    return (HANDLE)&s_SmartReflexLoadPolicyInfo;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  SMARTREFLEX_DeinitPolicy
//
//  Policy uninitialization
//
BOOL
SMARTREFLEX_DeinitPolicy(
    HANDLE hPolicyAdapter
    )
{
    BOOL rc = FALSE;

    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_DeinitPolicy(0x%08x)\r\n",
      hPolicyAdapter
      ));

    // validate parameters
    if (hPolicyAdapter != (HANDLE)&s_SmartReflexLoadPolicyInfo)
        {
        RETAILMSG (ZONE_ERROR, (L"ERROR: SMARTREFLEX_DeinitPolicy:"
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    // clear structures
    memset(&s_SmartReflexLoadPolicyInfo, 0, sizeof(SmartReflexPolicyInfo_t));
    
    rc = TRUE;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_DeinitPolicy()\r\n"));
    return rc;
} 


//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_OpenPolicy
//
//  Called when someone request to open a communication handle to the power
//  policy adapter
//
HANDLE
SMARTREFLEX_OpenPolicy(
    HANDLE hPolicyAdapter
    )
{
    HANDLE                  rc = NULL;
    SmartReflexPolicyContext_t    *pContext;

    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_OpenPolicy(0x%08x)\r\n",
      hPolicyAdapter
      ));

    // validate parameters
    if (hPolicyAdapter != (HANDLE)&s_SmartReflexLoadPolicyInfo)
        {
        RETAILMSG (ZONE_ERROR, (L"ERROR: SMARTREFLEX_OpenPolicy:"
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    // Create a policy context handle and return handle
    pContext = (SmartReflexPolicyContext_t*)LocalAlloc(LPTR, sizeof(SmartReflexPolicyContext_t));
    if (pContext == NULL)
        {
        RETAILMSG (ZONE_WARN, (L"WARNING: SMARTREFLEX_OpenPolicy:"
            L"Policy resources allocation failed!\r\n"
            ));
        goto cleanUp;
        }

    // initialize variables
    pContext->cookie = POLICY_CONTEXT_COOKIE;

    // add to list of collection
    rc = (HANDLE)pContext;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_OpenPolicy()\r\n"));
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_ClosePolicy
//
//  device state change handler
//
BOOL
SMARTREFLEX_ClosePolicy(
    HANDLE hPolicyContext,
    UINT dev,
    UINT oldState,
    UINT newState
    )
{
    SmartReflexPolicyContext_t  *pContext = (SmartReflexPolicyContext_t*)hPolicyContext;

    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_ClosePolicy(0x%08x,0xd,0x%02x,0x%02x )\r\n",
      hPolicyContext,
      dev,
      oldState,
      newState
      ));

    // validate parameters
    if (pContext == NULL || pContext->cookie != POLICY_CONTEXT_COOKIE)
        {
        RETAILMSG (ZONE_ERROR, (L"ERROR: SMARTREFLEX_ClosePolicy:"
            L"Incorrect context parameter\r\n"
            ));
        return FALSE;
        }

    // free resources
    LocalFree(pContext);

    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_ClosePolicy()\r\n"));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_NotifyPolicy
//
//  captures messages sent to the power policy adapter
//
BOOL
SMARTREFLEX_NotifyPolicy(
    HANDLE hPolicyContext,
    DWORD msg,
    void *pParam,
    UINT size
    )
{
    BOOL rc = FALSE;
    static BOOL * pEnable = NULL;

    SmartReflexPolicyContext_t    *pContext = (SmartReflexPolicyContext_t*)hPolicyContext;
    pEnable = (BOOL*)pParam;

    RETAILMSG(ZONE_FUNCTION, (
        L"+SMARTREFLEX_NotifyPolicy(0x%08x,0x%08x,0x%08x,%d )\r\n",
        hPolicyContext,
        msg,
        pParam,
        size
        ));

    // validate parameters
    if(pContext == NULL || pContext->cookie != POLICY_CONTEXT_COOKIE)
        {
        RETAILMSG (ZONE_ERROR, (L"ERROR: SMARTREFLEX_NotifyPolicy:"
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    if(pEnable == NULL)
        {
        RETAILMSG (ZONE_WARN, (L"WARNING: SMARTREFLEX_NotifyPolicy:"
            L"Incorrect input buffer context\r\n"
            ));
        goto cleanUp;
        }

    switch (msg)
        {
        case SMARTREFLEX_LPR_MODE:
            if(*pEnable)
                {
                RETAILMSG(ZONE_INFO, (L"-SMARTREFLEX_NotifyPolicy: LPR mode ON\r\n"));
                s_SmartReflexLoadPolicyInfo.bDisplayLPROn = TRUE;
                UpdateSmartReflex();
                }
            else
                {
                RETAILMSG(ZONE_INFO, (L"-SMARTREFLEX_NotifyPolicy: LPR mode OFF\r\n"));
                s_SmartReflexLoadPolicyInfo.bDisplayLPROn = FALSE;
                UpdateSmartReflex();
                }
            break;
        }
    // exit routine
    rc = TRUE;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_NotifyPolicy()\r\n"));
    return rc;
}


//-----------------------------------------------------------------------------
// 
//  Function:  SMARTREFLEX_PreDeviceStateChange
//
//  device state change handler
//
BOOL 
SMARTREFLEX_PreDeviceStateChange(
    HANDLE hPolicyAdapter,
    UINT dev,
    UINT oldState,
    UINT newState
    )
{

    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_PreDeviceStateChange(0x%08x,0xd,0x%02x,0x%02x )\r\n",
      hPolicyAdapter,
      dev,
      oldState,
      newState
      ));

    // validate parameters
    if (hPolicyAdapter != (HANDLE)&s_SmartReflexLoadPolicyInfo) return FALSE;

    // record the new device state for the device
    if (dev >= OMAP_DEVICE_GENERIC) return TRUE;

    switch (dev)
        {
        case OMAP_DEVICE_I2C1:
        case OMAP_DEVICE_I2C2:
        case OMAP_DEVICE_I2C3:
        case OMAP_DEVICE_MMC1:
        case OMAP_DEVICE_MMC2:
        case OMAP_DEVICE_MMC3:
        case OMAP_DEVICE_USBTLL:
        case OMAP_DEVICE_HDQ:
        case OMAP_DEVICE_MCBSP1:
        case OMAP_DEVICE_MCBSP5:
        case OMAP_DEVICE_MCSPI1:
        case OMAP_DEVICE_MCSPI2:
        case OMAP_DEVICE_MCSPI3:
        case OMAP_DEVICE_MCSPI4:
        case OMAP_DEVICE_UART1:
        case OMAP_DEVICE_UART2:
        case OMAP_DEVICE_TS:
        case OMAP_DEVICE_GPTIMER10:
        case OMAP_DEVICE_GPTIMER11:
        case OMAP_DEVICE_MSPRO:
        case OMAP_DEVICE_EFUSE:
        case OMAP_DEVICE_SR1:
        case OMAP_DEVICE_SR2: 
            if (newState < (UINT)D3 && oldState >= (UINT)D3)
                {
                //Add an Active device 
                s_SmartReflexLoadPolicyInfo.dwActiveCoreDevices++;
                }
            else if (newState >= (UINT)D3 && oldState < (UINT)D3)
                {
                //Remove an Active device 
                s_SmartReflexLoadPolicyInfo.dwActiveCoreDevices--;
                }

            if(s_SmartReflexLoadPolicyInfo.dwActiveCoreDevices > 0)
                {
                UpdateSmartReflex();
                }
            else
                {
                UpdateSmartReflex();
                }
            RETAILMSG(ZONE_INFO, (L"CORE Active Devices: %d\r\n",
                s_SmartReflexLoadPolicyInfo.dwActiveCoreDevices
                ));
            break;
        }

    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_PreDeviceStateChange()\r\n"));
    return TRUE;
}

//-----------------------------------------------------------------------------
// 
//  Function:  SMARTREFLEX_PostDeviceStateChange
//
//  device state change handler
//
BOOL 
SMARTREFLEX_PostDeviceStateChange(
    HANDLE hPolicyAdapter,
    UINT dev, 
    UINT oldState, 
    UINT newState
    )
{
    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_PostDeviceStateChange(0x%08x,0xd,0x%02x,0x%02x )\r\n",
      hPolicyAdapter,
      dev,
      oldState,
      newState
      ));

    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_PostDeviceStateChange()\r\n"));
    return TRUE;
}
//-----------------------------------------------------------------------------

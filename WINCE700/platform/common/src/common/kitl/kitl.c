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
//  File:  kitl.c
//
#include <windows.h>
#include <kitlprot.h>
#include <halether.h>
#include <oal.h>

//------------------------------------------------------------------------------
//
//  Global:  g_oalKitlBuffer
//
//  This global variable is intended to be used by KITL protocol implementation.
//
UINT8 g_oalKitlBuffer[OAL_KITL_BUFFER_SIZE];

//------------------------------------------------------------------------------
//
//  Global:  g_kitlState
//
//  This global static variable store common KITL state information.
//
static struct {
    CHAR deviceId[OAL_KITL_ID_SIZE];
    OAL_KITL_ARGS args;
    OAL_KITL_DEVICE *pDevice;
} g_kitlState;

//------------------------------------------------------------------------------
//  External functions
//
BOOL OALKitlSerialInit(
    LPSTR deviceId, OAL_KITL_DEVICE *pDevice, OAL_KITL_ARGS *pArgs, 
    KITLTRANSPORT *pKitl
);

BOOL OALKitlEthInit(
    LPSTR deviceId, OAL_KITL_DEVICE *pDevice, OAL_KITL_ARGS *pArgs, 
    KITLTRANSPORT *pKitl
);


//------------------------------------------------------------------------------
//
//  Function:  OALKitlGetDevLoc
//
//  This function allows other module to obtain KITL device location.
//
BOOL OALKitlGetDevLoc(DEVICE_LOCATION *pDevLoc)
{
    if(pDevLoc)
    {
        memcpy(pDevLoc, &g_kitlState.args.devLoc, sizeof(*pDevLoc));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//------------------------------------------------------------------------------
//
//  Function:  OALKitlGetFlags
//
//  This function allows other modules to obtain KITL flags.
//
BOOL OALKitlGetFlags(UINT32 *pFlags)
{
    if(pFlags)
    {
        memcpy(pFlags, &g_kitlState.args.flags, sizeof(*pFlags));
    }
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMKitlInit
//
//  This function is called from KitlInit to initialize KITL device and
//  KITLTRANSPORT structure. Implementation verify boot args structure validity
//  and call KITL device class init function.
//
BOOL OEMKitlInit(PKITLTRANSPORT pKitl)
{
    BOOL rc = FALSE;

    KITL_RETAILMSG(ZONE_KITL_OAL, ("+OEMKitlInit(0x%x)\r\n", pKitl));

    switch (g_kitlState.pDevice->type) {
#ifdef KITL_ETHER     
    case OAL_KITL_TYPE_ETH:
        rc = OALKitlEthInit(
            g_kitlState.deviceId, g_kitlState.pDevice,
            &g_kitlState.args, pKitl
        );
        break;
#endif
#ifdef KITL_SERIAL
    case OAL_KITL_TYPE_SERIAL:
        rc = OALKitlSerialInit(
            g_kitlState.deviceId, g_kitlState.pDevice,
            &g_kitlState.args, pKitl
        );
        break;
#endif
    }
    if (rc) {
        pKitl->pfnPowerOn  = OALKitlPowerOn;
        pKitl->pfnPowerOff = OALKitlPowerOff;
    } else {
        pKitl->pfnPowerOn  = NULL;
        pKitl->pfnPowerOff = NULL;
    }

    KITL_RETAILMSG(ZONE_KITL_OAL, ("-OEMKitlInit(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALKitlInit
//
//  This function is called from OEMInit to initialize KITL. It typically
//  calls initialization routine for device KITL class and then KitlInit.
//  The kernel/KITL then call back OEMKitlInit at moment when debug/KITL
//  connection should be initialized.
//
BOOL OALKitlInit(
    LPCSTR deviceId, OAL_KITL_ARGS *pArgs, OAL_KITL_DEVICE *pDevice
) {
    BOOL rc = FALSE;

    KITL_RETAILMSG(ZONE_KITL_OAL, (
        "+OALKitlInit('%hs', 0x%x - %d/%d/0x%x, 0x%x)\r\n", deviceId, 
        pArgs->flags, pArgs->devLoc.IfcType, pArgs->devLoc.BusNumber, 
        pArgs->devLoc.LogicalLoc, pDevice
    ));

    // Display KITL parameters
    KITL_RETAILMSG(ZONE_INIT, (
        "DeviceId................. %s\r\n", deviceId
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pArgs->flags............. 0x%x\r\n", pArgs->flags
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pArgs->devLoc.IfcType.... %d\r\n",   pArgs->devLoc.IfcType
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pArgs->devLoc.LogicalLoc. 0x%x\r\n", pArgs->devLoc.LogicalLoc
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pArgs->devLoc.PhysicalLoc 0x%x\r\n", pArgs->devLoc.PhysicalLoc
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pArgs->devLoc.Pin........ %d\r\n",   pArgs->devLoc.Pin
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pArgs->ip4address........ %S\r\n",   OALKitlIPtoString(pArgs->ipAddress)
    ));

    // If KITL is disabled simply return
    if ((pArgs->flags & OAL_KITL_FLAGS_ENABLED) == 0) {
        KITL_RETAILMSG(ZONE_WARNING, ("WARN: OALKitlInit: KITL Disabled\r\n"));
        rc = TRUE;
        goto cleanUp;
    }

    // Find if we support device on given location
    g_kitlState.pDevice = OALKitlFindDevice(&pArgs->devLoc, pDevice);
    if (g_kitlState.pDevice == NULL) {
        KITL_RETAILMSG(ZONE_ERROR, (
            "ERROR: OALKitlInit: No supported KITL device at interface %d "
            "bus %d location 0x%x\r\n", pArgs->devLoc.IfcType,
            pArgs->devLoc.BusNumber, pArgs->devLoc.LogicalLoc
        ));
        goto cleanUp;
    }

    KITL_RETAILMSG(ZONE_INIT, (
        "pDevice->Name............ %S\r\n",   g_kitlState.pDevice->name
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pDevice->ifcType......... %d\r\n",   g_kitlState.pDevice->ifcType
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pDevice->id.............. 0x%x\r\n", g_kitlState.pDevice->id
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pDevice->resource........ %d\r\n",   g_kitlState.pDevice->resource
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pDevice->type............ %d\r\n",   g_kitlState.pDevice->type
    ));
    KITL_RETAILMSG(ZONE_INIT, (
        "pDevice->pDriver......... 0x%x\r\n", g_kitlState.pDevice->pDriver
    ));
    
    // Save KITL configuration 
    memcpy(g_kitlState.deviceId, deviceId, sizeof(g_kitlState.deviceId));
    memcpy(&g_kitlState.args, pArgs, sizeof(g_kitlState.args));
            
    // Start KITL in desired mode
    if (!KitlInit((pArgs->flags & OAL_KITL_FLAGS_PASSIVE) == 0)) {
        KITL_RETAILMSG(ZONE_ERROR, ("ERROR: OALKitlInit: KitlInit failed\r\n"));
        goto cleanUp;
    }
    
    rc = TRUE;
    
cleanUp:
    KITL_RETAILMSG(ZONE_KITL_OAL, ("-OALKitlInit(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALKitlPowerOff
//
//  This function is called as part of OEMPowerOff implementation. It should
//  save all information about KITL device and put it to power off mode.
//
VOID OALKitlPowerOff()
{
    KITL_RETAILMSG(ZONE_KITL_OAL, ("+OALKitlPowerOff\r\n"));
    
    switch (g_kitlState.pDevice->type) {
#ifdef KITL_ETHER
    case OAL_KITL_TYPE_ETH:
        {
            OAL_KITL_ETH_DRIVER *pDriver;
            pDriver = (OAL_KITL_ETH_DRIVER*)g_kitlState.pDevice->pDriver;
            if (pDriver && pDriver->pfnPowerOff != NULL) pDriver->pfnPowerOff();
        }            
        break;
#endif
#ifdef KITL_SERIAL
     case OAL_KITL_TYPE_SERIAL:
        {
            OAL_KITL_SERIAL_DRIVER *pDriver;
            pDriver = (OAL_KITL_SERIAL_DRIVER*)g_kitlState.pDevice->pDriver;
            if (pDriver && pDriver->pfnPowerOff != NULL) pDriver->pfnPowerOff();
        }
        break;
#endif
    }        

    KITL_RETAILMSG(ZONE_KITL_OAL, ("-OALKitlPowerOff\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OALKitlPowerOn
//
//  This function is called as part of OEMPowerOff implementation. It should
//  restore KITL device back to working state.
//
VOID OALKitlPowerOn()
{
    KITL_RETAILMSG(ZONE_KITL_OAL, ("+OALKitlPowerOn\r\n"));

    switch (g_kitlState.pDevice->type) {
#ifdef KITL_ETHER
    case OAL_KITL_TYPE_ETH:
        {
            OAL_KITL_ETH_DRIVER *pDriver;
            pDriver = (OAL_KITL_ETH_DRIVER*)g_kitlState.pDevice->pDriver;
            if (pDriver->pfnPowerOn != NULL) pDriver->pfnPowerOn();
        }            
        break;
#endif
#ifdef KITL_SERIAL
     case OAL_KITL_TYPE_SERIAL:
        {
            OAL_KITL_SERIAL_DRIVER *pDriver;
            pDriver = (OAL_KITL_SERIAL_DRIVER*)g_kitlState.pDevice->pDriver;
            if (pDriver->pfnPowerOn != NULL) pDriver->pfnPowerOn();
        }
        break;
#endif
    }        

    KITL_RETAILMSG(ZONE_KITL_OAL, ("-OALKitlPowerOn\r\n"));
}

//------------------------------------------------------------------------------

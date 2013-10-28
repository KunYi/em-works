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
//  File:  power.c
//
#include <windows.h>
#include <oal.h>


//------------------------------------------------------------------------------
//
//  Function:  OALIoBusPowerOff
//
BOOL OALIoBusPowerOff(INTERFACE_TYPE ifcType, UINT32 busNumber)
{
    BOOL rc = FALSE;

#ifdef SHIP_BUILD
    UNREFERENCED_PARAMETER(busNumber);
#endif
    OALMSG(OAL_IO&&OAL_FUNC, (
        L"+OALIoBusPowerOff(%d, %d)\r\n", ifcType, busNumber
    ));
    
    switch (ifcType) {
#ifdef OAL_IO_PCI
    case PCIBus:
        OALPCIPowerOff(busNumber);
        rc = TRUE;
        break;
#endif
    }

    OALMSG(OAL_IO&&OAL_FUNC, (L"-OALIoBusPowerOff(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoBusPowerOn
//
BOOL OALIoBusPowerOn(INTERFACE_TYPE ifcType, UINT32 busNumber)
{
    BOOL rc = FALSE;

#ifdef SHIP_BUILD
    UNREFERENCED_PARAMETER(busNumber);
#endif
    OALMSG(OAL_IO&&OAL_FUNC, (
        L"+OALIoBusPowerOn(%d, %d)\r\n", ifcType, busNumber
    ));
    
    switch (ifcType) {
#ifdef OAL_IO_PCI
    case PCIBus:
        OALPCIPowerOn(busNumber);
        rc = TRUE;
        break;
#endif
    }

    OALMSG(OAL_IO&&OAL_FUNC, (L"-OALIoBusPowerOn(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------

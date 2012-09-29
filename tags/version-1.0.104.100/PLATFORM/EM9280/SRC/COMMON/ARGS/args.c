//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include <bsp.h>

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Global Variables
UUID g_UUID;

//------------------------------------------------------------------------------
//
//  Function:  OALArgsInit
//
//  This function is to initial OAL args.
//------------------------------------------------------------------------------
VOID OALArgsInit(BSP_ARGS* pArgs)
{
    // Check the BSP Args area
    //
    if (pArgs->header.signature  != OAL_ARGS_SIGNATURE ||
        pArgs->header.oalVersion != OAL_ARGS_VERSION   ||
        pArgs->header.bspVersion != BSP_ARGS_VERSION)
    {
        // Zero out the current contents of the structure
        memset(pArgs, 0, sizeof(BSP_ARGS));
        // Setup header
        pArgs->header.signature = OAL_ARGS_SIGNATURE;
        pArgs->header.oalVersion = OAL_ARGS_VERSION;
        pArgs->header.bspVersion = BSP_ARGS_VERSION;

    }
    return;
}

//------------------------------------------------------------------------------
//
//  Function:  OALArgsQuery
//
//  This function is called from other OAL modules to return boot arguments.
//  Boot arguments are typically placed in fixed memory location and they are
//  filled by boot loader. In case that boot arguments can't be located
//  the function should return NULL. The OAL module then must use default
//  values.
//------------------------------------------------------------------------------
VOID* OALArgsQuery(UINT32 type)
{
    VOID *pData = NULL;
    BSP_ARGS *pArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;

    OALMSG(OAL_ARGS&&OAL_FUNC, (L"+OALArgsQuery(%d)\r\n", type));
    // Check if there is expected signature
    if (
        pArgs->header.signature  != OAL_ARGS_SIGNATURE ||
        pArgs->header.oalVersion != OAL_ARGS_VERSION   ||
        pArgs->header.bspVersion != BSP_ARGS_VERSION
        ) goto cleanUp;

    // Depending on required args
    switch (type) 
    {
        case OAL_ARGS_QUERY_DEVID:
            pData = &pArgs->deviceId;
            break;
   
        case OAL_ARGS_QUERY_UUID:
            RETAILMSG(1, (L"OALArgsQuery: OAL_ARGS_QUERY_UUID\r\n"));
            // Identifies the universally unique identifier (UUID) argument specified in the
            // argument structure.
            pData = &g_UUID;
            break;
    
        case OAL_ARGS_QUERY_KITL:
            pArgs->kitl.flags |= OAL_KITL_FLAGS_VMINI;
            // Has the bootloader provided a non-zero IP address and subnet mask?
            // If not, use DHCP to obtain this information.
            //
            if ((pArgs->kitl.ipAddress == 0) || (pArgs->kitl.ipMask == 0))
            {
              pArgs->kitl.flags |= OAL_KITL_FLAGS_DHCP;
            }
            if (pArgs->kitl.devLoc.LogicalLoc == 0)
            {
                pArgs->kitl.devLoc.IfcType     = Internal;
                pArgs->kitl.devLoc.BusNumber   = 0;
                pArgs->kitl.devLoc.PhysicalLoc = (PVOID)(CSP_BASE_REG_PA_USBCTRL0);
                pArgs->kitl.devLoc.LogicalLoc  = (DWORD)pArgs->kitl.devLoc.PhysicalLoc;
            }
            pData = &pArgs->kitl;
            break;
    
        case BSP_ARGS_QUERY_DBGSERIAL:
            if ((pArgs->dbgSerPhysAddr != CSP_BASE_REG_PA_UARTDBG))
            {
                pArgs->dbgSerPhysAddr = CSP_BASE_REG_PA_UARTDBG;
            }
            pData = &pArgs->dbgSerPhysAddr;
            break;
    
        case OAL_ARGS_QUERY_UPDATEMODE:
            pData = &pArgs->updateMode;
            break;
    }

cleanUp:
    OALMSG(OAL_ARGS&&OAL_FUNC, (L"-OALArgsQuery(pData = 0x%08x)\r\n", pData));
    return pData;
}

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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
#include <bsp.h>

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Global Variables

// UUID could be based on IIM fuse values or memory chip ID. For this platform,
// it will be initialized using IIM in OEMInit().
UUID g_UUID;
BSP_ARGS *g_pBSPArgs;

//------------------------------------------------------------------------------
//
//  Function:  OALArgsQuery
//
//  This function is called from other OAL modules to return boot arguments.
//  Boot arguments are typically placed in fixed memory location and they are
//  filled by boot loader. In case that boot arguments can't be located
//  the function should return NULL. The OAL module then must use default
//  values.
//
VOID* OALArgsQuery(UINT32 type)
{
    VOID *pData = NULL;
    BSP_ARGS *pArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;
    
    g_pBSPArgs = pArgs;
    
    OALMSG(OAL_ARGS&&OAL_FUNC, (L"+OALArgsQuery(%d)\r\n", type));

    // Check if there is expected signature
    if (
        pArgs->header.signature  != OAL_ARGS_SIGNATURE ||
        pArgs->header.oalVersion != OAL_ARGS_VERSION   ||
        pArgs->header.bspVersion != BSP_ARGS_VERSION
       ) goto cleanUp;

    // Depending on required args
    switch (type) {

    case OAL_ARGS_QUERY_DEVID:
        pData = &pArgs->deviceId;
        break;

    case OAL_ARGS_QUERY_KITL:
        {
            switch ((UINT32)pArgs->kitl.devLoc.PhysicalLoc)
            {
                case BSP_BASE_REG_PA_SERIALKITL:
                    pArgs->kitl.flags = OAL_KITL_FLAGS_POLL;
                    pArgs->kitl.baudRate = BSP_UART_KITL_SERIAL_BAUD;
                    pArgs->kitl.dataBits = UART_UCR2_WS_8BIT;
                    pArgs->kitl.stopBits = UART_UCR2_STPB_1STOP;
                    pArgs->kitl.parity = UART_UCR2_PREN_DISBLE;
                    pArgs->kitl.devLoc.LogicalLoc = BSP_BASE_REG_PA_SERIALKITL;
                    pArgs->kitl.devLoc.PhysicalLoc = (PVOID)BSP_BASE_REG_PA_SERIALKITL;
                    pArgs->kitl.devLoc.BusNumber   = 0;
                    pArgs->kitl.devLoc.Pin   = 0;
                    pArgs->kitl.devLoc.IfcType = Internal;
                    break;

                default:
                    pArgs->kitl.flags |= OAL_KITL_FLAGS_VMINI;

                    // Has the bootloader provided a non-zero IP address and subnet mask?
                    // If not, use DHCP to obtain this information.
                    //
                    if ((pArgs->kitl.ipAddress == 0) || (pArgs->kitl.ipMask == 0))
                    {
                        pArgs->kitl.flags |= OAL_KITL_FLAGS_DHCP;
                    }

                    // Has the bootloader provided information about which NIC it was using?
                    // If not, choose the CS8900A as the default.
                    //
                    if (pArgs->kitl.devLoc.LogicalLoc == 0)
                    {
                        pArgs->kitl.devLoc.IfcType     = Internal;
                        pArgs->kitl.devLoc.BusNumber   = 0;
                        pArgs->kitl.devLoc.PhysicalLoc = (PVOID)(BSP_BASE_REG_PA_LAN911x_IOBASE);
                        pArgs->kitl.devLoc.LogicalLoc  = (DWORD)pArgs->kitl.devLoc.PhysicalLoc;
                    }
            }
            pData = &pArgs->kitl;

        }
        break;

    // Check if this is a UUID request
    case OAL_ARGS_QUERY_UUID:
        // Return pointer to global UUID
        pData = &g_UUID;
        break;

    case OAL_ARGS_QUERY_UPDATEMODE:
        pData = &pArgs->updateMode;
        break;
    }

cleanUp:
    OALMSG(OAL_ARGS&&OAL_FUNC, (L"-OALArgsQuery(pData = 0x%08x)\r\n", pData));
    return pData;
}

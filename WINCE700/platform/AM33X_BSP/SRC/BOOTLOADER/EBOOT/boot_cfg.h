// All rights reserved ADENEO EMBEDDED 2010
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  Header: boot_cfg.h
//
#ifndef __BOOT_CFG_H
#define __BOOT_CFG_H

#include "bsp_base_regs.h"

//------------------------------------------------------------------------------
OAL_KITL_DEVICE g_bootDevices[] = {
	{
        L"Internal EMAC", Internal, AM33X_EMACSW_REGS_PA, 
        0, OAL_KITL_TYPE_ETH, &g_kitlEthLan_AM33X
    },
//	{
//        L"USBFn RNDIS ", Internal, OMAP_USBHS_REGS_PA,
//        0, OAL_KITL_TYPE_ETH, &g_kitlUsbRndis
//	},
#ifdef BUILDING_EBOOT_SD    
	{
        // For reason of adding 0x0100, see comment in COMMON_TI_V1\AM33X\soccfg\devicemap.c
        L"NK from SDCard FILE ", Internal, AM33X_MMCHS0_REGS_PA+0x0100,
        0, BOOT_SDCARD_TYPE, NULL
    }, 
#endif    
#ifndef BSP_NO_NAND_IN_SDBOOT    
	{
        L"NK from NAND", Internal, BSP_NAND_REGS_PA + 0x20, 
        0, OAL_KITL_TYPE_FLASH, NULL
    }, 
#endif        
	{
        NULL, 0, 0, 0, 0, NULL
    }
};


//------------------------------------------------------------------------------

#endif

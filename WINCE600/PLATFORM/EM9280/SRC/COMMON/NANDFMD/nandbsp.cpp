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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include <fmd.h>
#include "common_nandfmd.h"
#include "nand_hal.h"
#include "image_cfg.h"
#include "mx28_ddk.h"

extern FlashInfoExt g_FlashInfoExt;

#define BBT_BLOCK_NUM						4       // Block numbers : store BBT
#define NAND_NUM_OF_CLUSTER			1
#define NAND_NUM_OF_DEVICES			2

//
// CS&ZHL NOV-6-2011: use K9F2G08U0A for EM9280
//
#ifdef	EM9280
#define NAND_BLOCK_CNT					2048
#define NAND_NUM_OF_CS					1
#define NAND_PAGE_SIZE					2048
#else		
#define NAND_BLOCK_CNT					4096
#define NAND_NUM_OF_CS					2
#define NAND_PAGE_SIZE					4096
#endif	//EM9280

//if you choose to use dynamically detect, make buffers below large enough
BYTE g_BBTFirstBuf[(((NAND_BLOCK_CNT * NAND_NUM_OF_DEVICES * NAND_NUM_OF_CS + 7) / 8 + NAND_PAGE_SIZE - 1) / NAND_PAGE_SIZE) * NAND_PAGE_SIZE];
BYTE g_BBTSecondBuf[(((NAND_BLOCK_CNT * NAND_NUM_OF_DEVICES * NAND_NUM_OF_CS + 7) / 8 + NAND_PAGE_SIZE - 1) / NAND_PAGE_SIZE) * NAND_PAGE_SIZE];
BYTE g_SectorBuf[NAND_PAGE_SIZE];

VOID BSPNAND_GetFlashInfo(PFlashInfoExt pFlashInfo)
{
    pFlashInfo->ILSupport = FALSE;
    pFlashInfo->ClusterCount = NAND_NUM_OF_CLUSTER;
    pFlashInfo->ECCType = ECC_BCH;
    
    pFlashInfo->AutoDec.Enable = TRUE;
    pFlashInfo->AutoDec.CsSearchRange = MAX_NAND_DEVICES;
}


VOID BSPNAND_GetBufferPointer(PBBTBuffer pBBT)
{   
    pBBT->pSectorBuf = g_SectorBuf;
    pBBT->pBBTFirstBuf = g_BBTFirstBuf;
    pBBT->pBBTSecondBuf = g_BBTSecondBuf;
    pBBT->dwBBTFirstSize = sizeof(g_BBTFirstBuf);
    pBBT->dwBBTSecondSize = sizeof(g_BBTFirstBuf);
    
    //last two block is reserved for config block, temporily
    pBBT->dwBBTStartBlock = IMAGE_BBT_NAND_OFFSET;
    pBBT->dwBBTEndBlock = IMAGE_BBT_NAND_BLOCKS + IMAGE_BBT_NAND_OFFSET;
    pBBT->dwCs = 0;
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPNAND_ConfigIOMUX
//
//  This functions config certain pin for nfc use.  
//
//  Parameters:
//      CsNum
//          [in] - how many cs are used defines how this function works.  
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPNAND_ConfigIOMUX(DWORD CsNum)
{
    // Wake up PINCTRL for GPMI use (i.e. bring out of reset and clkgate).
    //HW_PINCTRL_CTRL_CLR( BM_PINCTRL_CTRL_SFTRST | BM_PINCTRL_CTRL_CLKGATE);

    // No voltage settings for GPMI_D0 - GPMI_D15
    // in the STMP378x part.

    // By default, all NAND pins are GPIO's after reset
    // Connect NAND signals by configuring 2-bit pinmux value for each pin
    // Most NAND pins are the default peripheral = 0b00.
    // Startup already checked PINCTRL present bits
    // -----------------------------------------------------------------
    // Always power up lower 8 bits of NAND Data bus.
    //HW_PINCTRL_MUXSEL0_CLR(BM_PINCTRL_MUXSEL_NAND(7, 0));   // BANK0[7:0] = 0

    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_D00,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_D01,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_D02,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_D03,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_D04,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_D05,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_D06,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_D07,DDK_IOMUX_MODE_00);

    // Select the ALE, CLE, WRN, and RDN pins of NAND.

    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_RDN,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_WRN,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_ALE,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_CLE,DDK_IOMUX_MODE_00);

    // Set the pin drive for the RDN pin to 8mA.

    DDKIomuxSetPadConfig(DDK_IOMUX_GPMI_RDN,DDK_IOMUX_PAD_DRIVE_8MA,(DDK_IOMUX_PAD_PULL)0,(DDK_IOMUX_PAD_VOLTAGE)0);

    // Power up the appropriate Chip Enable and Ready/Busy Lines.
    // Alway enable NAND0
    {
        // Power up Ready Busy for NAND0
        DDKIomuxSetPinMux(DDK_IOMUX_GPMI_READY0,DDK_IOMUX_MODE_00);

        // Power up CE0 by setting bit field to b10
        DDKIomuxSetPinMux(DDK_IOMUX_GPMI_CE0N,DDK_IOMUX_MODE_00);

    }
    if (CsNum >= 1)
    {
        // Power up Ready Busy for NAND1
        DDKIomuxSetPinMux(DDK_IOMUX_GPMI_READY1,DDK_IOMUX_MODE_00);
        // Power up CE1 by setting bit field to b00
        DDKIomuxSetPinMux(DDK_IOMUX_GPMI_CE1N,DDK_IOMUX_MODE_00);
    }

    // this is called gpmi_wpn in data spec!
    // DDKIomuxSetPinMux(DDK_IOMUX_GPMI_WPN,DDK_IOMUX_MODE_00);   
}

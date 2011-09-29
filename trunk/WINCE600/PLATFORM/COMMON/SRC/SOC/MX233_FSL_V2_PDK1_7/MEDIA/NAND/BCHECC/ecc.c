//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ecc.c
//
//
//
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include "nand_hal.h"
#include "nand_ecc.h"
#pragma warning(pop)
#include "csp.h"
#include "regsdigctl.h"
#include "regsBCH.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define ECC_CORRECTION_TIMEOUT      5000     //500 useconds.

///////////////////////////////////////////////////////////////////////////////
// Prototypes
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////

VOID ECC_ConfigLayout(WORD PageSize, WORD SpareSize)
{
    RETAILMSG(TRUE, (_T("NAND_ECCConfLayout, pagesize=%d,sisize=%d\r\n"),PageSize,SpareSize));
    if(PageSize == 2048 && SpareSize == 64)
    {
        HW_BCH_FLASH0LAYOUT0_WR(BF_BCH_FLASH0LAYOUT0_NBLOCKS(0x4) | BF_BCH_FLASH0LAYOUT0_META_SIZE(sizeof(SectorInfo)) | BF_BCH_FLASH0LAYOUT0_ECC0(ECC0_2K4K_PAGE));
        HW_BCH_FLASH0LAYOUT1_WR(BF_BCH_FLASH0LAYOUT1_PAGE_SIZE(2048+64) | BF_BCH_FLASH0LAYOUT1_ECCN(ECCN_2K4K_PAGE) | BF_BCH_FLASH0LAYOUT1_DATAN_SIZE(512));
    }
    else if(PageSize == 4096 && SpareSize == 218)
    {
        HW_BCH_FLASH0LAYOUT0_WR(BF_BCH_FLASH0LAYOUT0_NBLOCKS(0x8) | BF_BCH_FLASH0LAYOUT0_META_SIZE(sizeof(SectorInfo)) | BF_BCH_FLASH0LAYOUT0_ECC0(ECC0_2K4K_PAGE));
        HW_BCH_FLASH0LAYOUT1_WR(BF_BCH_FLASH0LAYOUT1_PAGE_SIZE(4096+218) | BF_BCH_FLASH0LAYOUT1_ECCN(ECCN_2K4K_PAGE) | BF_BCH_FLASH0LAYOUT1_DATAN_SIZE(512));
    }
    HW_BCH_LAYOUTSELECT_WR(0);
}

BOOL ECC_IsCorrectable()
{
    UINT32 u32Temp, u32StartTime;
    BOOL EccStatus = TRUE;

    u32StartTime = HW_DIGCTL_MICROSECONDS_RD();

    // Spin until ECC Complete Interrupt triggers.
    while((!(HW_BCH_CTRL_RD() & BM_BCH_CTRL_COMPLETE_IRQ)) &&
          (!((HW_DIGCTL_MICROSECONDS_RD()-u32StartTime)>ECC_CORRECTION_TIMEOUT))) ;

    // Check for timeout occurring.
    if (!(HW_BCH_CTRL_RD() & BM_BCH_CTRL_COMPLETE_IRQ))
    {
        // EccStatus = ERROR_DDI_NAND_HAL_ECC_FIX_FAILED;
        ERRORMSG(TRUE,(_T("ECC Timeout\r\n")));
        ERRORMSG(TRUE,(_T("HW_BCH_CTRL_RD=0x%x\r\n"),HW_BCH_CTRL_RD()));
        ERRORMSG(TRUE,(_T("HW_BCH_STATUS0_RD=0x%x\r\n"),HW_BCH_STATUS0_RD()));
        ERRORMSG(TRUE,(_T("HW_BCH_FLASH0LAYOUT0_RD=0x%x\r\n"),HW_BCH_FLASH0LAYOUT0_RD()));
        ERRORMSG(TRUE,(_T("HW_BCH_FLASH0LAYOUT1_RD=0x%x\r\n"),HW_BCH_FLASH0LAYOUT1_RD()));
        ERRORMSG(TRUE,(_T("HW_BCH_LAYOUTSELECT_RD=0x%x\r\n"),HW_BCH_LAYOUTSELECT_RD()));
    }
    
    // Now read the ECC status.
    u32Temp = HW_BCH_STATUS0_RD();

    if (u32Temp & BM_BCH_STATUS0_CORRECTED)
    {
        RETAILMSG(0,(_T("BM_ECC8_STATUS0_CORRECTED\r\n")));
    }

    if (u32Temp & BM_BCH_STATUS0_UNCORRECTABLE)
    {
        ERRORMSG(TRUE,(_T("BM_ECC8_STATUS0_UNCORRECTABLE,u32Temp=0x%x\r\n"),u32Temp));
        EccStatus = FALSE;
    }

    // Clear the ECC Complete IRQ.
    BW_BCH_CTRL_COMPLETE_IRQ(0);
    //HW_BCH_CTRL_SET(BM_BCH_CTRL_COMPLETE_IRQ);
    //HW_BCH_CTRL_CLR(BM_BCH_CTRL_COMPLETE_IRQ);
    return EccStatus;
}

void ECC_Init()
{
    //volatile UINT32 i;
    
    //if(HW_BCH_CTRL_RD() & (BM_BCH_CTRL_CLKGATE | BM_BCH_CTRL_SFTRST))
    //{
    //    RETAILMSG(TRUE, (_T("reset BCH module\r\n")));
    //    HW_BCH_CTRL_CLR(BM_BCH_CTRL_CLKGATE);
    //    HW_BCH_CTRL_SET(BM_BCH_CTRL_SFTRST);
    //    for(i=0;i<100*1000;i++);
    //}
    
    if(HW_BCH_CTRL_RD() & (BM_BCH_CTRL_CLKGATE | BM_BCH_CTRL_SFTRST))
    {
        RETAILMSG(TRUE, (_T("enable bch module\r\n")));
        HW_BCH_CTRL_CLR(BM_BCH_CTRL_SFTRST | BM_BCH_CTRL_CLKGATE);
    }
}

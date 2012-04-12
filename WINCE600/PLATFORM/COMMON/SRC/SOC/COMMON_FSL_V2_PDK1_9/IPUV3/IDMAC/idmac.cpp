//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  idmac.cpp
//
//  IPUv3 CSP-level IDMAC functions
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#pragma warning(pop)

#include "common_macros.h"
#include "idmac_priv.h"

#include "idmac.h"
#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
PCSP_IPU_IDMAC_REGS g_pIPUV3_IDMAC;
static PCSP_IPU_COMMON_REGS g_pIPUV3_COMMON;
static UINT32 ReadyReg[6]={0};


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: IDMACRegsInit
//
// This function allocates the data structures required for interaction
// with the IPUv3 IDMAC hardware.
//
// Parameters:
//      width
//          [in] width of the display
//      height
//          [in] height of the display
//      bpp
//          [in] bits per pixel of the display
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL IDMACRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    HANDLE hIPUBase = NULL;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPUV3_IDMAC == NULL)
    {
        //  *** Use IPU_BASE driver to retrieve IPU Base Address ***

        // First, create handle to IPU_BASE driver
        hIPUBase = IPUV3BaseOpenHandle();
        if (hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        dwIPUBaseAddr = IPUV3BaseGetBaseAddr(hIPUBase);
        if (dwIPUBaseAddr == -1)
        {
            RETAILMSG (1,
                (TEXT("%s: Failed to retrieve IPU Base addr!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        //********************************************
        // Create pointer to IDMAC registers
        //********************************************
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_IDMAC_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_IDMAC = (PCSP_IPU_IDMAC_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_IDMAC_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_IDMAC == NULL)
        {
            RETAILMSG(1,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

        //********************************************
        // Create pointer to IPU Common registers
        //********************************************
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_IPU_COMMON_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_COMMON = (PCSP_IPU_COMMON_REGS) MmMapIoSpace(phyAddr,
                                    sizeof(CSP_IPU_COMMON_REGS), FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_COMMON == NULL)
        {
            DEBUGMSG(1,
                (_T("Init:  COMMON reg MmMapIoSpace failed!\r\n")));
             goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    if (!IPUV3BaseCloseHandle(hIPUBase))
    {
        RETAILMSG(1,
            (_T("IDMAC Cleanup: Failed closing IPU Base handle!\r\n")));
    }

    // If initialization failed, be sure to clean up
    if (!rc) IDMACRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 IDMAC hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void IDMACRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap peripheral registers
    if (g_pIPUV3_IDMAC)
    {
        MmUnmapIoSpace(g_pIPUV3_IDMAC, sizeof(PCSP_IPU_IDMAC_REGS));
        g_pIPUV3_IDMAC = NULL;
    }

    // Unmap peripheral registers
    if (g_pIPUV3_COMMON)
    {
        MmUnmapIoSpace(g_pIPUV3_COMMON, sizeof(PCSP_IPU_COMMON_REGS));
        g_pIPUV3_COMMON = NULL;
    }

    IPU_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelEnable
//
// This function enables the specified IDMAC channel, setting
// the appopriate bit in the IDMAC_CH_EN registers.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being enabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelEnable(DWORD dwChannel)
{
    UINT32 oldVal, newVal, iBitval;

    if (dwChannel < 32)
    {
        // Compute shifted bit value for IDMAC_CH_EN_1
        // register field we want to set.
        iBitval = 1 << dwChannel;

        // set IDMAC_CH_EN_1 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_CH_EN_1);
            newVal = (oldVal | iBitval); // Equivalent to SETREG32 macro
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_CH_EN_1,
                oldVal, newVal) != oldVal);
    }
    else
    {
        // Compute shifted bit value for IDMAC_CH_EN_2
        // register field we want to set.
        iBitval = 1 << (dwChannel - 32);

        // set IDMAC_CH_EN_2 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_CH_EN_2);
            newVal = (oldVal | iBitval); // Equivalent to SETREG32 macro
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_CH_EN_2,
                oldVal, newVal) != oldVal);
    }
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelDisable
//
// This function disables the specified IDMAC channel, clearing
// the appopriate bit in the IDMAC_CH_EN registers.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being disabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelDisable(DWORD dwChannel)
{
    UINT32 oldVal, newVal, iBitval;

    if (dwChannel < 32)
    {
        // Compute shifted bit value for IDMAC_CH_EN_1
        // register field we want to clear.
        iBitval = 1 << dwChannel;

        // set IDMAC_CH_EN_1 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_CH_EN_1);
            newVal = oldVal & (~iBitval); // Equivalent to CLEARREG32 macro
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_CH_EN_1,
                oldVal, newVal) != oldVal);
    }
    else
    {
        // Compute shifted bit value for IDMAC_CH_EN_2
        // register field we want to clear.
        iBitval = 1 << (dwChannel - 32);

        // set IDMAC_CH_EN_2 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_CH_EN_2);
            newVal = oldVal & (~iBitval); // Equivalent to CLEARREG32 macro
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_CH_EN_2,
                oldVal, newVal) != oldVal);
    }
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelSetPriority
//
// This function sets the IDMAC Channel Priority registers,
// programming in the requested priority setting for the specified
// IDMAC channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being modified.
//
//      pri
//          [in] The new priority setting, either high or low.
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelSetPriority(DWORD dwChannel, IDMAC_CH_PRIORITY pri)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (dwChannel < 32)
    {
        // Compute bitmask and shifted bit value for IDMAC_CH_PRI_1 registers
        iMask = 1 << dwChannel;
        iBitval = pri << dwChannel;

        // set IDMAC_CH_PRI_1 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_CH_PRI_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_CH_PRI_1,
                oldVal, newVal) != oldVal);
    }
    else
    {
        // Compute bitmask and shifted bit value for IDMAC_CH_PRI_2 registers
        iMask = 1 << (dwChannel - 32);
        iBitval = pri << (dwChannel - 32);

        // set IDMAC_CH_PRI_2 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_CH_PRI_2);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_CH_PRI_2,
                oldVal, newVal) != oldVal);
    }
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelSetBandMode
//
// This function sets the IDMAC Band Mode Enable registers,
// programming in the requested band mode setting for the specified
// IDMAC channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being modified.
//
//      band_mode
//          [in] The new band mode setting, either enabled or disabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelSetBandMode(DWORD dwChannel, IDMAC_CH_BAND_MODE band_mode)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (dwChannel < 32)
    {
        // Compute bitmask and shifted bit value for IDMAC_BNDM_EN_1 registers
        iMask = 1 << dwChannel;
        iBitval = band_mode << dwChannel;

        // set IDMAC_BNDM_EN_1 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_BNDM_EN_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_BNDM_EN_1,
                oldVal, newVal) != oldVal);
    }
    else
    {
        // Compute bitmask and shifted bit value for IDMAC_BNDM_EN_2 registers
        iMask = 1 << (dwChannel - 32);
        iBitval = band_mode << (dwChannel - 32);

        // set IDMAC_BNDM_EN_2 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_BNDM_EN_2);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_BNDM_EN_2,
                oldVal, newVal) != oldVal);
    }
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelIsBusy
//
// This function reads the IDMAC_CH_BUSY register, and determines if the
// specified channel is busy.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.
//
// Returns:
//      TRUE if busy
//      FALSE if not busy
//
//-----------------------------------------------------------------------------
BOOL IDMACChannelIsBusy(DWORD dwChannel)
{
    BOOL retVal;

    if (dwChannel < 32)
    {
        retVal = EXTREG32(&g_pIPUV3_IDMAC->IDMAC_CH_BUSY_1, (1 << dwChannel), dwChannel);
    }
    else
    {
        retVal = EXTREG32(&g_pIPUV3_IDMAC->IDMAC_CH_BUSY_2, (1 << (dwChannel - 32)), (dwChannel - 32));
    }

    return retVal;
}
//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelLock
//
// This function enable/disable the corresponding IDMA channel enable feature.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.
//
//      bEnable
//          [in] If TRUE, Enable lock feature.
//                If FALSE, Disable lock feature.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelLock(DWORD dwChannel, BOOL bEnable)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (dwChannel < 32)
    {
#ifdef IPUv3EX
        // Compute bitmask and shifted bit value for IDMAC_LOCK_EN_1 registers
        iMask = 1 << dwChannel;
        iBitval = bEnable << dwChannel;

        // set IDMAC_LOCK_EN_1 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_LOCK_EN_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_LOCK_EN_1,
                oldVal, newVal) != oldVal);
#else
        ERRORMSG(1, (TEXT("IDMACChannelLock: Invalid channel Number:%d!\r\n"),dwChannel));
#endif
    }
    else
    {
        // Compute bitmask and shifted bit value for IDMAC_LOCK_EN_2 registers
        iMask = 1 << (dwChannel - 32);
        iBitval = bEnable << (dwChannel - 32);

        // set IDMAC_LOCK_EN_2 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_LOCK_EN_2);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_LOCK_EN_2,
                oldVal, newVal) != oldVal);
    }
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACWaterMark
//
// This function enable/disable the corresponding IDMA channel water mark feature.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.
//
//      bEnable
//          [in] If TRUE, Enable WM feature.
//                If FALSE, Disable WM feature.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACWaterMark(DWORD dwChannel, BOOL bEnable)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (dwChannel < 32)
    {

        // Compute bitmask and shifted bit value for IDMAC_WM_EN_1 registers
        iMask = 1 << dwChannel;
        iBitval = bEnable << dwChannel;

        // set IDMAC_WM_EN_1 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_WM_EN_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_WM_EN_1,
                oldVal, newVal) != oldVal);
    }
    else
    {
        // Compute bitmask and shifted bit value for IDMAC_WM_EN_2 registers
        iMask = 1 << (dwChannel - 32);
        iBitval = bEnable << (dwChannel - 32);

        // set IDMAC_WM_EN_2 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_WM_EN_2);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_WM_EN_2,
                oldVal, newVal) != oldVal);
    }
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelSetSepAlpha
//
// This function enable/disable the corresponding IDMA channel separate alpha channel feature.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.Only channel 14,15,23,24,27,29 can be used.
//
//      bEnable
//          [in] If TRUE, Enable separate alpha channel.
//                If FALSE, Disable separate alpha channel.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelSetSepAlpha(DWORD dwChannel, BOOL bEnable)
{
    UINT32 oldVal, newVal, iMask, iBitval;


    // Compute bitmask and shifted bit value for IDMAC_SEP_ALPHA registers
    iMask = 1 << dwChannel;
    iBitval = bEnable << dwChannel;

    // set IDMAC_SEP_ALPHA with new value
    do
    {
        oldVal = INREG32(&g_pIPUV3_IDMAC->IDMAC_SEP_ALPHA);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_IDMAC->IDMAC_SEP_ALPHA,
            oldVal, newVal) != oldVal);

}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelDBMODE
//
// This function sets the DBMODE to either double or single buffer mode.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.
//
//      bDoubleBuf
//          [in] If TRUE, set to double buffer mode.
//               If FALSE, set to single buffer mode.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelDBMODE(DWORD dwChannel, BOOL bDoubleBuf)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (dwChannel < 32)
    {
        // Compute bitmask and shifted bit value for IPU_CH_DB_MODE_SEL0 registers
        iMask = 1 << dwChannel;
        iBitval = bDoubleBuf << dwChannel;

        // set IPU_CH_DB_MODE_SEL0 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_CH_DB_MODE_SEL0);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_CH_DB_MODE_SEL0,
                oldVal, newVal) != oldVal);
    }
    else
    {
        // Compute bitmask and shifted bit value for IPU_CH_DB_MODE_SEL1 registers
        iMask = 1 << (dwChannel - 32);
        iBitval = bDoubleBuf << (dwChannel - 32);

        // set IPU_CH_DB_MODE_SEL1 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_CH_DB_MODE_SEL1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_CH_DB_MODE_SEL1,
                oldVal, newVal) != oldVal);
    }
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelTRBMODE
//
// This function sets the TRBMODE to either triple or single buffer mode.
// When the channel is configured for triple buffer mode, the double buffer mode settings 
// configured on the corresponding DB_MODE_SEL bit are overridden.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.
//
//      bTripleBuf
//          [in] If TRUE, set to triple buffer mode.
//               If FALSE, set to single buffer mode.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelTRBMODE(DWORD dwChannel, BOOL bTripleBuf)
{
#ifdef IPUv3EX
    UINT32 oldVal, newVal, iMask, iBitval;

    switch(dwChannel)
    {
        case 8:
        case 9:
        case 10:
        case 13:
        case 21:
        case 23:
        case 27:
        case 28:
            break;
            
        default:
            RETAILMSG(1,(TEXT("%s:channel %d doesn't support triple buffer mode.\r\n"), __WFUNCTION__, dwChannel));
            return;
    }

    if (dwChannel < 32)
    {
        // Compute bitmask and shifted bit value for IPU_CH_TRB_MODE_SEL0 registers
        iMask = 1 << dwChannel;
        iBitval = bTripleBuf << dwChannel;

        // set IPU_CH_TRB_MODE_SEL0 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_CH_TRB_MODE_SEL0);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_CH_TRB_MODE_SEL0,
                oldVal, newVal) != oldVal);
    }
    else
    {
        // Compute bitmask and shifted bit value for IPU_CH_TRB_MODE_SEL1 registers
        iMask = 1 << (dwChannel - 32);
        iBitval = bTripleBuf << (dwChannel - 32);

        // set IPU_CH_TRB_MODE_SEL1 with new value
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_CH_TRB_MODE_SEL1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_CH_TRB_MODE_SEL1,
                oldVal, newVal) != oldVal);
    }
    if(bTripleBuf ==  TRUE)
    {
        switch(dwChannel)
        {
            case 4:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_0, 
                           IPU_IDMAC_SUB_ADDR_0_IDMAC_SUB_ADDR_4,60);
                break;
            case 5:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_0, 
                           IPU_IDMAC_SUB_ADDR_0_IDMAC_SUB_ADDR_5,61);
                break;
            case 6:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_0, 
                           IPU_IDMAC_SUB_ADDR_0_IDMAC_SUB_ADDR_6,62);
                break;
            case 7:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_0, 
                           IPU_IDMAC_SUB_ADDR_0_IDMAC_SUB_ADDR_7,63);
                break;
            case 23:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_1, 
                           IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_23,64);
                break;
            case 24:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_1, 
                           IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_24,65);
                break;
            case 29:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_1, 
                           IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_29,66);
                break;
            case 33:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_1, 
                           IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_33,67);
                break;
            case 41:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_2, 
                           IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_41,68);
                break;
            case 51:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_2, 
                           IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_51,69);
                break;
            case 52:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_2, 
                           IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_52,70);
                break;
            case 9:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_3, 
                           IPU_IDMAC_SUB_ADDR_3_IDMAC_SUB_ADDR_9,71);
                break;
            case 10:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_3, 
                           IPU_IDMAC_SUB_ADDR_3_IDMAC_SUB_ADDR_10,72);
                break;
            case 13:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_3, 
                           IPU_IDMAC_SUB_ADDR_3_IDMAC_SUB_ADDR_13,73);
                break;
            case 27:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_3, 
                           IPU_IDMAC_SUB_ADDR_3_IDMAC_SUB_ADDR_27,74);
                break;
            case 28:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_4, 
                           IPU_IDMAC_SUB_ADDR_4_IDMAC_SUB_ADDR_28,75);
                break;
            case 8:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_4, 
                           IPU_IDMAC_SUB_ADDR_4_IDMAC_SUB_ADDR_8,76);
                break;
            case 21:
                INSREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_4, 
                           IPU_IDMAC_SUB_ADDR_4_IDMAC_SUB_ADDR_21,77);
                break;
            default:
                break;

        }
    }
#else
    //UNREFERENCED_PARAMETER(dwChannel);
    RETAILMSG(1,(TEXT("%s:IPUv3D/E doesn't support this function.Call IDMACChannelDBMODE() instead.\r\n"),__WFUNCTION__));
    IDMACChannelDBMODE(dwChannel, bTripleBuf);
#endif
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelAltCPMEMAddr
//
// This function get the alt channel cpmem index.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------

UINT8 IDMACChannelAltChIndex(DWORD dwChannel)
{
#ifdef IPUv3EX
    UINT8 retVal = 0;
    switch(dwChannel)
    {
        case 4:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_0, 
                                        IPU_IDMAC_SUB_ADDR_0_IDMAC_SUB_ADDR_4);
            break;
        case 5:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_0,
                                        IPU_IDMAC_SUB_ADDR_0_IDMAC_SUB_ADDR_5);
            break;
        case 6:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_0, 
                                        IPU_IDMAC_SUB_ADDR_0_IDMAC_SUB_ADDR_6);
            break;
        case 7:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_0, 
                                        IPU_IDMAC_SUB_ADDR_0_IDMAC_SUB_ADDR_7);
            break;
        case 23:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_1, 
                                        IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_23);
            break;
        case 24:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_1, 
                                        IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_24);
            break;
        case 29:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_1, 
                                        IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_29);
            break;
        case 33:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_1, 
                                        IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_33);
            break;
        case 41:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_2, 
                                        IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_41);
            break;
        case 51:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_2, 
                                        IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_51);
            break;
        case 52:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_2, 
                                        IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_52);
            break;
        case 9:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_3, 
                                        IPU_IDMAC_SUB_ADDR_3_IDMAC_SUB_ADDR_9);
            break;
        case 10:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_3, 
                                        IPU_IDMAC_SUB_ADDR_3_IDMAC_SUB_ADDR_10);
            break;
        case 13:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_3, 
                                        IPU_IDMAC_SUB_ADDR_3_IDMAC_SUB_ADDR_13);
            break;
        case 27:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_3, 
                                        IPU_IDMAC_SUB_ADDR_3_IDMAC_SUB_ADDR_27);
            break;
        case 28:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_4, 
                                        IPU_IDMAC_SUB_ADDR_4_IDMAC_SUB_ADDR_28);
            break;
        case 8:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_4, 
                                        IPU_IDMAC_SUB_ADDR_4_IDMAC_SUB_ADDR_8);
            break;
        case 21:
            retVal = (UINT8)EXTREG32BF(&g_pIPUV3_IDMAC->IDMAC_SUB_ADDR_4, 
                                        IPU_IDMAC_SUB_ADDR_4_IDMAC_SUB_ADDR_21);
            break;
        default:
            retVal = (UINT8)dwChannel;
            break;

    }
    return retVal;
#else
    //UNREFERENCED_PARAMETER(dwChannel);
    RETAILMSG(1,(TEXT("%s:IPUv3D/E doesn't support this function.Call IDMACChannelDBMODE() instead.\r\n"),__WFUNCTION__));
    return (UINT8)dwChannel;
#endif


}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUFReadyPush
//
// This function saves the buffer ready registers before power gating.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelBUFReadyPush()
{
    int i;
    //interlock
    while(INREG32(&g_pIPUV3_COMMON->IPU_GPR)&0x00000001)
        Sleep(1);
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000001);
    for(i=0;i<4;i++)
        ReadyReg[i]= INREG32((&g_pIPUV3_COMMON->IPU_CH_BUF0_RDY0+i));
#ifdef IPUv3EX  
    ReadyReg[4]= INREG32((&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY0));    
    ReadyReg[5]= INREG32((&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY1));    
#endif
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000000);

}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUFReadyPop
//
// This function restores the buffer ready registers after power return.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelBUFReadyPop()
{
    //interlock
    while(INREG32(&g_pIPUV3_COMMON->IPU_GPR)&0x00000001)
        Sleep(1);
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000001);

    int i;
    for(i=0;i<4;i++)
        OUTREG32((&g_pIPUV3_COMMON->IPU_CH_BUF0_RDY0+i),ReadyReg[i]);
#ifdef IPUv3EX  
    OUTREG32((&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY0),ReadyReg[4]);
    OUTREG32((&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY1),ReadyReg[5]);
#endif

    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000000);    
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUF0SetReady
//
// This function sets the specified channel to ready for buf0.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being set to ready.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelBUF0SetReady(DWORD dwChannel)
{
    //interlock
    while(INREG32(&g_pIPUV3_COMMON->IPU_GPR)&0x00000001)
        Sleep(1);
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000001);
    if (dwChannel < 32)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF0_RDY0, (1 << dwChannel));
    }
    else
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF0_RDY1, (1 << (dwChannel - 32)));
    }
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000000);
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUF1SetReady
//
// This function sets the specified channel to ready for buf1.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being set to ready.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelBUF1SetReady(DWORD dwChannel)
{
    //interlock
    while(INREG32(&g_pIPUV3_COMMON->IPU_GPR)&0x00000001)
        Sleep(1);
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000001);
    if (dwChannel < 32)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF1_RDY0, (1 << dwChannel));
    }
    else
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF1_RDY1, (1 << (dwChannel - 32)));
    }
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000000);
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUF2SetReady
//
// This function sets the specified channel to ready for buf2.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being set to ready.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelBUF2SetReady(DWORD dwChannel)
{
#ifdef IPUv3EX  
    //interlock
    while(INREG32(&g_pIPUV3_COMMON->IPU_GPR)&0x00000001)
        Sleep(1);
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000001);
    if (dwChannel < 32)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY0, (1 << dwChannel));
    }
    else
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY1, (1 << (dwChannel - 32)));
    }
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000000);
#else
    UNREFERENCED_PARAMETER(dwChannel);
    RETAILMSG(1,(TEXT("%s:IPUv3D/E doesn't support this function.\r\n"),__WFUNCTION__));
#endif

    
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUF0ClrReady
//
// This function Clear the specified channel to ready for buf0.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being set to ready.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelBUF0ClrReady(DWORD dwChannel)
{
    //interlock
    while(INREG32(&g_pIPUV3_COMMON->IPU_GPR)&0x00000001)
        Sleep(1);
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x30000001);
    if (dwChannel < 32)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF0_RDY0, (1 << dwChannel));
    }
    else
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF0_RDY1, (1 << (dwChannel - 32)));
    }
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000000);
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUF1ClrReady
//
// This function Clear the specified channel to ready for buf1.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being set to ready.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelBUF1ClrReady(DWORD dwChannel)
{
    //interlock
    while(INREG32(&g_pIPUV3_COMMON->IPU_GPR)&0x00000001)
        Sleep(1);
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0xC0000001);

    if (dwChannel < 32)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF1_RDY0, (1 << dwChannel));
    }
    else
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF1_RDY1, (1 << (dwChannel - 32)));
    }
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000000);
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUF2ClrReady
//
// This function Clear the specified channel to ready for buf2.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being set to ready.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelBUF2ClrReady(DWORD dwChannel)
{
#ifdef IPUv3EX 
    //interlock
    while(INREG32(&g_pIPUV3_COMMON->IPU_GPR)&0x00000001)
        Sleep(1);
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00300001);

    if (dwChannel < 32)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY0, (1 << dwChannel));
    }
    else
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY1, (1 << (dwChannel - 32)));
    }
    OUTREG32(&g_pIPUV3_COMMON->IPU_GPR,0x00000000);
#else
    UNREFERENCED_PARAMETER(dwChannel);
    RETAILMSG(1,(TEXT("%s:IPUv3D/E doesn't support this function.\r\n"),__WFUNCTION__));
#endif

    
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUF0IsReady
//
// This function queries the buf0 rdy bit for the specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel to query the buf0 rdy bit.
//
// Returns:
//      TRUE if BUF0 Rdy bit is set, FALSE if bit is cleared.
//
//-----------------------------------------------------------------------------
BOOL IDMACChannelBUF0IsReady(DWORD dwChannel)
{
    BOOL retVal;

    if (dwChannel < 32)
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF0_RDY0, (1 << dwChannel), dwChannel);
    }
    else
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF0_RDY1, (1 << (dwChannel - 32)), (dwChannel - 32));
    }

    return retVal;
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUF1IsReady
//
// This function queries the buf1 rdy bit for the specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel to query the buf1 rdy bit.
//
// Returns:
//      TRUE if BUF1 Rdy bit is set, FALSE if bit is cleared.
//
//-----------------------------------------------------------------------------
BOOL IDMACChannelBUF1IsReady(DWORD dwChannel)
{

    BOOL retVal;

    if (dwChannel < 32)
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF1_RDY0, (1 << dwChannel), dwChannel);
    }
    else
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF1_RDY1, (1 << (dwChannel - 32)), (dwChannel - 32));
    }

    return retVal;


    
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelBUF2IsReady
//
// This function queries the buf2 rdy bit for the specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel to query the buf2 rdy bit.
//
// Returns:
//      TRUE if BUF2 Rdy bit is set, FALSE if bit is cleared.
//
//-----------------------------------------------------------------------------
BOOL IDMACChannelBUF2IsReady(DWORD dwChannel)
{
#ifdef IPUv3EX
    BOOL retVal;

    if (dwChannel < 32)
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY0, (1 << dwChannel), dwChannel);
    }
    else
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CH_BUF2_RDY1, (1 << (dwChannel - 32)), (dwChannel - 32));
    }

    return retVal;
#else
    UNREFERENCED_PARAMETER(dwChannel);
    RETAILMSG(1,(TEXT("%s:IPUv3D/E doesn't support this function.\r\n"),__WFUNCTION__));
    return FALSE;
#endif    
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelCurrentBufIsBuf0
//
// This function queries whether buf0 is the current buffer
// for the specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel to query if
//          buf0 is the currently active buffer.
//
// Returns:
//      TRUE if BUF0 is current buffer, FALSE if bit is cleared.
//
//-----------------------------------------------------------------------------
BOOL IDMACChannelCurrentBufIsBuf0(DWORD dwChannel)
{
    BOOL retVal;

    if (dwChannel < 32)
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CUR_BUF_0, (1 << dwChannel), dwChannel);
    }
    else
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CUR_BUF_1, (1 << (dwChannel - 32)), (dwChannel - 32));
    }

    return !retVal;
}
//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelClrCurrentBuf
//
// This function resets the current buffer to buffer 0
// for the specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel whose current
//          active buffer will be reset buffer 0.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelClrCurrentBuf(DWORD dwChannel)
{
    if (dwChannel < 32)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CUR_BUF_0, (1 << dwChannel));
    }
    else
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_CUR_BUF_1, (1 << (dwChannel - 32)));
    }
#ifdef IPUv3EX
    
    if (dwChannel <16)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_TRIPLE_CUR_BUF_0, (3 << (dwChannel*2)));
    }
    else if(dwChannel <32)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_TRIPLE_CUR_BUF_1, (3 << (dwChannel*2-32)));
    }
    else if(dwChannel <48)
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_TRIPLE_CUR_BUF_2, (3 << (dwChannel*2-64)));
    }
    else
    {
        OUTREG32(&g_pIPUV3_COMMON->IPU_TRIPLE_CUR_BUF_3, (3 << (dwChannel*2-96)));
    }
    
#endif 

}
//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelCurrentBufIsBuf1
//
// This function queries whether buf1 is the current buffer
// for the specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel to query if
//          buf1 is the currently active buffer.
//
// Returns:
//      TRUE if BUF1 is current buffer, FALSE if bit is cleared.
//
//-----------------------------------------------------------------------------
BOOL IDMACChannelCurrentBufIsBuf1(DWORD dwChannel)
{
    BOOL retVal;

    if (dwChannel < 32)
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CUR_BUF_0, (1 << dwChannel), dwChannel);
    }
    else
    {
        retVal = EXTREG32(&g_pIPUV3_COMMON->IPU_CUR_BUF_1, (1 << (dwChannel - 32)), (dwChannel - 32));
    }

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelQueryTripleCurrentBuf
//
// This function queries whether buf2 is the current buffer
// for the specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel to query if
//          buf1 is the currently active buffer.
//
// Returns:
//      The number of triple current buffer. Will return -1 when the function are not supported.
//
//-----------------------------------------------------------------------------
INT8 IDMACChannelQueryTripleCurrentBuf(DWORD dwChannel)
{
#ifdef IPUv3EX
    INT8 retVal;

    if (dwChannel <16)
    {
        retVal = (INT8)EXTREG32(&g_pIPUV3_COMMON->IPU_TRIPLE_CUR_BUF_0, 
                            (3 << (dwChannel*2)),(dwChannel*2));
    }
    else if(dwChannel <32)
    {
        retVal = (INT8)EXTREG32(&g_pIPUV3_COMMON->IPU_TRIPLE_CUR_BUF_1, 
                            (3 << (dwChannel*2-32)), (dwChannel*2-32));
    }
    else if(dwChannel <48)
    {
        retVal = (INT8)EXTREG32(&g_pIPUV3_COMMON->IPU_TRIPLE_CUR_BUF_2, 
                            (3 << (dwChannel*2-64)), (dwChannel*2-64));
    }
    else
    {
        retVal = (INT8)EXTREG32(&g_pIPUV3_COMMON->IPU_TRIPLE_CUR_BUF_3, 
                            (3 << (dwChannel*2-96)), (dwChannel*2-96));
    }
    return retVal;
#else
    UNREFERENCED_PARAMETER(dwChannel);
    RETAILMSG(1,(TEXT("%s:IPUv3D/E doesn't support this function.\r\n"),__WFUNCTION__));
    return -1;
#endif    
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelGetIntStatus
//
// This function reads the interrupt status for the specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.  If the IntrType is General, this parameter
//          indicates the Left-shift for the interrupt to be queried.
//
//      IntrType
//          [in] Selects the type of interrupt to retieve status for.
//
// Returns:
//      TRUE if interrupt raising
//      FALSE if no interrupt raising
//
//-----------------------------------------------------------------------------
BOOL IDMACChannelGetIntStatus(DWORD dwChannel, IPU_INTR_TYPE IntrType)
{
    BOOL retVal;

    switch (IntrType)
    {
        case IPU_INTR_TYPE_EOF:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_1, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_2, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_NFACK:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_3, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_4, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_NFB4EOF:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_5, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_6, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_EOS:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_7, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_8, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_EOBND:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_11, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_12, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_TH:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_13, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_14, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_GENERAL:
            if (dwChannel <32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_15, (1 << dwChannel), dwChannel);
            }
            else
            {
                // Invalid interrupt value...there are only 31 general interrupts, so 
                // the value must be less than 32
                ERRORMSG (1, (TEXT("Invalid bit to clear for General Purpose IPU interrupts!\n")));
                retVal = FALSE;
            }
            break;
        default:
            retVal = FALSE;
    }

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelQueryIntEnable
//
// This function queries the mask to see if the interrupt is enabled for the specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.  If the IntrType is General, this parameter
//          indicates the Left-shift for the interrupt to be queried.
//
//      IntrType
//          [in] Selects the type of interrupt to retieve status for.
//
// Returns:
//      TRUE if interrupt is enabled
//      FALSE if no interrupt is enabled
//
//-----------------------------------------------------------------------------
BOOL IDMACChannelQueryIntEnable(DWORD dwChannel, IPU_INTR_TYPE IntrType)
{
    BOOL retVal;

    switch (IntrType)
    {
        case IPU_INTR_TYPE_EOF:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_1, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_2, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_NFACK:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_3, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_4, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_NFB4EOF:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_5, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_6, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_EOS:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_7, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_8, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_EOBND:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_11, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_12, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_TH:
            if (dwChannel < 32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_13, (1 << dwChannel), dwChannel);
            }
            else
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_14, (1 << (dwChannel - 32)), (dwChannel - 32));
            }
            break;
        case IPU_INTR_TYPE_GENERAL:
            if (dwChannel <32)
            {
                retVal = (BOOL)EXTREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_15, (1 << dwChannel), dwChannel);
            }
            else
            {
                // Invalid interrupt value...there are only 31 general interrupts, so 
                // the value must be less than 32
                ERRORMSG (1, (TEXT("Invalid bit to clear for General Purpose IPU interrupts!\n")));
                retVal = FALSE;
            }
            break;
        default:
            retVal = FALSE;
    }

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelClearIntStatus
//
// This function sets the right bit in the IPU_INT_STAT register to clear the interrupt status
// of specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.  If the IntrType is General, this parameter
//          indicates the left-shift for the interrupt to be cleared.
//
//      IntrType
//          [in] Selects the type of interrupt to clear.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelClearIntStatus(DWORD dwChannel, IPU_INTR_TYPE IntrType)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (dwChannel <32)
    {
        iMask = 1 << dwChannel;
        iBitval = 1 << dwChannel;
    }
    else
    {
        // Compute bitmask and shifted bit value for IDMAC_BNDM_EN_2 registers
        iMask = 1 << (dwChannel - 32);
        iBitval = 1 << (dwChannel - 32);
    }

    switch (IntrType)
    {
        case IPU_INTR_TYPE_EOF:
            if (dwChannel <32)
            {
                // set IPU_INTR_STAT_1 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_1);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_1,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_STAT_2 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_2);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_2,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_NFACK:
            if (dwChannel <32)
            {
                // set IPU_INTR_STAT_3 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_3);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_3,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_STAT_4 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_4);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_4,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_NFB4EOF:
            if (dwChannel <32)
            {
                // set IPU_INTR_STAT_5 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_5);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_5,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_STAT_6 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_6);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_6,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_EOS:
            if (dwChannel <32)
            {
                // set IPU_INTR_STAT_7 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_7);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_7,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_STAT_8 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_8);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_8,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_EOBND:
            if (dwChannel <32)
            {
                // set IPU_INTR_STAT_11 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_11);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_11,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_STAT_12 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_12);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_12,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_TH:
            if (dwChannel <32)
            {
                // set IPU_INTR_STAT_13 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_13);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_13,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_STAT_14 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_14);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_14,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_GENERAL:
            if (dwChannel <32)
            {
                // set IPU_INTR_STAT_15 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_15);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_15,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // Invalid interrupt value...there are only 31 general interrupts, so 
                // the value must be less than 32
                ERRORMSG (1, (TEXT("Invalid bit to clear for General Purpose IPU interrupts!\n")));
                
            }
            break;
    }
    return;
}


//-----------------------------------------------------------------------------
//
// Function:  IDMACChannelIntCntrl
//
// This function configure the right bit in the IPU_INT_CTRL register to enable or disable
// the interrupt of specified channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being queried.  If the IntrType is General, this parameter
//          indicates the mask for the interrupt to be enabled/disabled..
//
//      IntrType
//          [in] Selects the type of interrupt to enable/disable.
//
//      enable
//          [in]   TRUE:  enable interrupt
//                  FALSE:disable interrupt
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IDMACChannelIntCntrl(DWORD dwChannel, IPU_INTR_TYPE IntrType, BOOL enable)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (dwChannel <32)
    {
        iMask = 1 << dwChannel;
        if(enable)
            iBitval = 1 << dwChannel;
        else
            iBitval = 0 << dwChannel;
    }
    else
    {
        // Compute bitmask and shifted bit value for IDMAC_BNDM_EN_2 registers
        iMask = 1 << (dwChannel - 32);
        if(enable)
            iBitval = 1 << (dwChannel - 32);
        else
            iBitval = 0 << (dwChannel - 32);
    }

    switch (IntrType)
    {
        case IPU_INTR_TYPE_EOF:
            if (dwChannel <32)
            {
                // set IPU_INTR_CTRL_1 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_1);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_1,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_CTRL_2 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_2);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_2,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_NFACK:
            if (dwChannel <32)
            {
                // set IPU_INTR_CTRL_3 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_3);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_3,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_CTRL_4 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_4);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_4,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_NFB4EOF:
            if (dwChannel <32)
            {
                // set IPU_INTR_CTRL_5 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_5);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_5,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_CTRL_6 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_6);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_6,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_EOS:
            if (dwChannel <32)
            {
                // set IPU_INTR_CTRL_7 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_7);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_7,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_CTRL_8 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_8);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_8,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_EOBND:
            if (dwChannel <32)
            {
                // set IPU_INTR_CTRL_11 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_11);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_11,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_CTRL_12 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_12);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_12,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_TH:
            if (dwChannel <32)
            {
                // set IPU_INTR_CTRL_13 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_13);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_13,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // set IPU_INTR_CTRL_14 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_14);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_14,
                        oldVal, newVal) != oldVal);
            }
            break;
        case IPU_INTR_TYPE_GENERAL:
            if (dwChannel <32)
            {
                // set IPU_INTR_CTRL_15 with new value
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_15);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_15,
                        oldVal, newVal) != oldVal);
            }
            else
            {
                // Invalid interrupt value...there are only 31 general interrupts, so 
                // the value must be less than 32
                ERRORMSG (1, (TEXT("Invalid bit to clear for General Purpose IPU interrupts!\n")));
                
            }
            break;
    }
    return;
}


void IDMACDumpRegs()
{
    int i;
    UINT32* ptr = (UINT32 *)&g_pIPUV3_IDMAC->IDMAC_CONF;

    RETAILMSG (1, (TEXT("\n\nIDMAC Registers\n")));
    for (i = 0; i <= (sizeof(CSP_IPU_IDMAC_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}

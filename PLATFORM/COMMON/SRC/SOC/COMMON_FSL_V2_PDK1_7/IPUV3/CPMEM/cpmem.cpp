//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  cpmem.cpp
//
//  IPU Internal Memory access functions
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
#include "common_ipuv3ex.h"

#include "cpmem.h"
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
PCSP_IPU_MEM_CPMEM g_pIPUV3_CPMEM;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: CPMEMRegsInit
//
// This function initializes the structures needed to access
// the IPUv3 Channel Parameter Memory.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL CPMEMRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    HANDLE hIPUBase = NULL;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPUV3_CPMEM == NULL)
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

        // Map CPMEM memory region entries
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_CPMEM_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_CPMEM = (PCSP_IPU_MEM_CPMEM) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_MEM_CPMEM),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_CPMEM == NULL)
        {
            RETAILMSG(1,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
        memset(g_pIPUV3_CPMEM,0,sizeof(CSP_IPU_MEM_CPMEM));
    }

    rc = TRUE;

cleanUp:

    if (!IPUV3BaseCloseHandle(hIPUBase))
    {
        RETAILMSG(1,
            (_T("CPMEM Cleanup: Failed closing IPU Base handle!\r\n")));
    }

    // If initialization failed, be sure to clean up
    if (!rc) CPMEMRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  CPMEMRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 Channel Parameter Memory.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void CPMEMRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap peripheral registers
    if (g_pIPUV3_CPMEM)
    {
        MmUnmapIoSpace(g_pIPUV3_CPMEM, sizeof(PCSP_IPU_MEM_CPMEM));
        g_pIPUV3_CPMEM = NULL;
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: CPMEMWrite
//
// This function configures the Channel Parameter Memory for the
// specified IPU DMA channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being configured.
//
//      pCPMEMData
//          [in] Pointer to CPMEMConfigData structure containing
//          data to configure IDMAC channel.
//
//      bInterleaved
//          [in] TRUE if interleaved mode, FALSE if planar mode.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void CPMEMWrite(DWORD dwChannel, CPMEMConfigData *pCPMEMData, BOOL bInterleaved)
{
    UINT8 AlphaChMapping;
    IPU_FUNCTION_ENTRY();
    //Configure ALBM it's bind with idma channel.
    // 0 to channel 14
    // 1 to channel 15
    // 2 to channel 27
    // 3 to channel 29
    // 4 to channel 23
    // 5 to channel 24
    switch(dwChannel)
    {
    case 14:
        AlphaChMapping = 0;
        break;
    case 15:
        AlphaChMapping = 1;
        break;
    case 27:
        AlphaChMapping = 2;
        break;
    case 29:
        AlphaChMapping = 3;
        break;
    case 23:
        AlphaChMapping = 4;
        break;
    case 24:
        AlphaChMapping = 5;
        break;
    default:
        AlphaChMapping = 0;
    }
    if (!bInterleaved)
    {
        // Configure CPMEM for Planar image data
        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword0,
            CSP_BITFVAL(CPMEM_PLNR_XV, 0) |
            CSP_BITFVAL(CPMEM_PLNR_YV, 0) |
            CSP_BITFVAL(CPMEM_PLNR_XB, 0));

        // Use INSREG32BF instead of OUTREG32 for mword1_dword2 so that
        // we don't overwrite the UBO value (set using CPMEMWriteOffset)
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword1,
            CPMEM_PLNR_YB, 0);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword1,
            CPMEM_PLNR_NSB_B, 0);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword1,
            CPMEM_PLNR_CF, 0);

        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword2,
            CPMEM_PLNR_IOX, 0);

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword3,
            CSP_BITFVAL(CPMEM_PLNR_SO, pCPMEMData->iScanOrder) |
            CSP_BITFVAL(CPMEM_PLNR_BNDM, pCPMEMData->iBandMode) |
            CSP_BITFVAL(CPMEM_PLNR_BM, pCPMEMData->iBlockMode) |
            CSP_BITFVAL(CPMEM_PLNR_ROT, pCPMEMData->iRotation90) |
            CSP_BITFVAL(CPMEM_PLNR_HF, pCPMEMData->iFlipHoriz) |
            CSP_BITFVAL(CPMEM_PLNR_VF, pCPMEMData->iFlipVert) |
            CSP_BITFVAL(CPMEM_PLNR_THE, pCPMEMData->iThresholdEnable) |
            CSP_BITFVAL(CPMEM_PLNR_CAP, pCPMEMData->iCondAccessPolarity) |
            CSP_BITFVAL(CPMEM_PLNR_CAE, pCPMEMData->iCondAccessEnable) |
            CSP_BITFVAL(CPMEM_PLNR_FW_LOW, pCPMEMData->iWidth));

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword4,
            CSP_BITFVAL(CPMEM_PLNR_FW_HIGH, pCPMEMData->iWidth>>CPMEM_PLNR_FW_LOW_WID) |
            CSP_BITFVAL(CPMEM_PLNR_FH, pCPMEMData->iHeight));

        // Use INSREG32BF instead of OUTREG32 for mword1_dword2 so that
        // we don't overwrite the ILO value (set using CPMEMWriteOffset)
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_PLNR_NPB, pCPMEMData->iPixelBurst);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_PLNR_PFS, pCPMEMData->iPixelFormat);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_PLNR_ALU, pCPMEMData->iAlphaUsed);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_PLNR_ALBM, AlphaChMapping);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_PLNR_ID, pCPMEMData->iAXI_Id);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_PLNR_TH_LOW, pCPMEMData->iThreshold);

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword3,
            CSP_BITFVAL(CPMEM_PLNR_TH_HIGH, pCPMEMData->iThreshold>>CPMEM_PLNR_TH_LOW_WID) |
            CSP_BITFVAL(CPMEM_PLNR_SLY, pCPMEMData->iLineStride_Y) |
            CSP_BITFVAL(CPMEM_PLNR_WID3, pCPMEMData->pixelFormatData.component3_width));

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword4,
            CSP_BITFVAL(CPMEM_PLNR_SLUV, pCPMEMData->iLineStride_UV));
    }
    else
    {
        // Configure CPMEM for Interleaved image data
        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword0,
            CSP_BITFVAL(CPMEM_ILVD_XV, 0) |
            CSP_BITFVAL(CPMEM_ILVD_YV, 0) |
            CSP_BITFVAL(CPMEM_ILVD_XB, 0));

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword1,
            CSP_BITFVAL(CPMEM_ILVD_YB, 0) |
            CSP_BITFVAL(CPMEM_ILVD_NSB_B, 0) |
            CSP_BITFVAL(CPMEM_ILVD_CF, 0) |
            CSP_BITFVAL(CPMEM_ILVD_SX, 0) |
            CSP_BITFVAL(CPMEM_ILVD_SY_LOW, 0));

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword2,
            CSP_BITFVAL(CPMEM_ILVD_SY_HIGH, 0>>CPMEM_ILVD_SY_LOW_WID) |
            CSP_BITFVAL(CPMEM_ILVD_NS, 0) |
            CSP_BITFVAL(CPMEM_ILVD_SDX, 0) |
            CSP_BITFVAL(CPMEM_ILVD_SM, 0));

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword3,
            CSP_BITFVAL(CPMEM_ILVD_SCC, 0) |
            CSP_BITFVAL(CPMEM_ILVD_SCE, 0) |
            CSP_BITFVAL(CPMEM_ILVD_SDY, 0) |
            CSP_BITFVAL(CPMEM_ILVD_SDRX, 0) |
            CSP_BITFVAL(CPMEM_ILVD_SDRY, 0) |
            CSP_BITFVAL(CPMEM_ILVD_BPP, pCPMEMData->iBitsPerPixel) |
            CSP_BITFVAL(CPMEM_ILVD_DEC_SEL, pCPMEMData->iDecAddrSelect) |
            CSP_BITFVAL(CPMEM_ILVD_DIM, pCPMEMData->iAccessDimension) |
            CSP_BITFVAL(CPMEM_ILVD_SO, pCPMEMData->iScanOrder) |
            CSP_BITFVAL(CPMEM_ILVD_BNDM, pCPMEMData->iBandMode) |
            CSP_BITFVAL(CPMEM_ILVD_BM, pCPMEMData->iBlockMode) |
            CSP_BITFVAL(CPMEM_ILVD_ROT, pCPMEMData->iRotation90) |
            CSP_BITFVAL(CPMEM_ILVD_HF, pCPMEMData->iFlipHoriz) |
            CSP_BITFVAL(CPMEM_ILVD_VF, pCPMEMData->iFlipVert) |
            CSP_BITFVAL(CPMEM_ILVD_THE, pCPMEMData->iThresholdEnable) |
            CSP_BITFVAL(CPMEM_ILVD_CAP, pCPMEMData->iCondAccessPolarity) |
            CSP_BITFVAL(CPMEM_ILVD_CAE, pCPMEMData->iCondAccessEnable) |
            CSP_BITFVAL(CPMEM_ILVD_FW_LOW, pCPMEMData->iWidth));

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword4,
            CSP_BITFVAL(CPMEM_ILVD_FW_HIGH, pCPMEMData->iWidth>>CPMEM_ILVD_FW_LOW_WID) |
            CSP_BITFVAL(CPMEM_ILVD_FH, pCPMEMData->iHeight));

        // Use INSREG32BF instead of OUTREG32 for mword1_dword2 so that
        // we don't overwrite the ILO value (set using CPMEMWriteBuffer)
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_ILVD_NPB, pCPMEMData->iPixelBurst);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_ILVD_PFS, pCPMEMData->iPixelFormat);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_ILVD_ALU, pCPMEMData->iAlphaUsed);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_ILVD_ALBM, AlphaChMapping);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_ILVD_ID, pCPMEMData->iAXI_Id);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
            CPMEM_ILVD_TH_LOW, pCPMEMData->iThreshold);

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword3,
            CSP_BITFVAL(CPMEM_ILVD_TH_HIGH, pCPMEMData->iThreshold>>CPMEM_ILVD_TH_LOW_WID) |
            CSP_BITFVAL(CPMEM_ILVD_SL, pCPMEMData->iLineStride) |
            CSP_BITFVAL(CPMEM_ILVD_WID0, pCPMEMData->pixelFormatData.component0_width) |
            CSP_BITFVAL(CPMEM_ILVD_WID1, pCPMEMData->pixelFormatData.component1_width) |
            CSP_BITFVAL(CPMEM_ILVD_WID2, pCPMEMData->pixelFormatData.component2_width) |
            CSP_BITFVAL(CPMEM_ILVD_WID3, pCPMEMData->pixelFormatData.component3_width));

        OUTREG32(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword4,
            CSP_BITFVAL(CPMEM_ILVD_OFS0, pCPMEMData->pixelFormatData.component0_offset) |
            CSP_BITFVAL(CPMEM_ILVD_OFS1, pCPMEMData->pixelFormatData.component1_offset) |
            CSP_BITFVAL(CPMEM_ILVD_OFS2, pCPMEMData->pixelFormatData.component2_offset) |
            CSP_BITFVAL(CPMEM_ILVD_OFS3, pCPMEMData->pixelFormatData.component3_offset));
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: CPMEMWriteBuffer
//
// This function programs the address of the IPU DMA channel's source or
// destination buffer into the Channel Parameter Memory.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being configured.
//
//      bufNum
//          [in] 0 if programming buffer 0, 1 if buffer 1.
//
//      pBufAddr
//          [in] Pointer to source/destination buffer
//          data to configure IDMAC channel.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void CPMEMWriteBuffer(DWORD dwChannel, DWORD bufNum, UINT32 *pBufAddr)
{
    UINT32 dwBufAdjusted;

    IPU_FUNCTION_ENTRY();

    // physical address must be shifted by 3 before programming
    // into CPMEM eba0/eba1 fields.
    dwBufAdjusted = (UINT32)pBufAddr >> 3;

    // Offsets and word position are same for interleaved and planar
    // when programming EBA0 and EBA1 so we can use the ILVD
    // offset and width.
    if (bufNum == 0)
    {
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword0,
            CPMEM_ILVD_EBA0, dwBufAdjusted);
    }
    else
    {
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword0,
            CPMEM_ILVD_EBA1_LOW, dwBufAdjusted);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword1,
            CPMEM_ILVD_EBA1_HIGH, dwBufAdjusted>>CPMEM_ILVD_EBA1_LOW_WID);
    }

    IPU_FUNCTION_EXIT();
}
//------------------------------------------------------------------------------
//
// Function: CPMEMReadBufferAddr
//
// This function reads the address of the IPU DMA channel's source or
// destination buffer from the Channel Parameter Memory.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being configured.
//
//      bufNum
//          [in] 0 if reading from buffer 0, 1 if buffer 1.
//
// Returns:
//     The physical address of corresponding buffer.
//------------------------------------------------------------------------------
UINT32 CPMEMReadBufferAddr(DWORD dwChannel, DWORD bufNum)
{
    UINT32 dwBuf;

    IPU_FUNCTION_ENTRY();

    // Offsets and word position are same for interleaved and planar
    // when programming EBA0 and EBA1 so we can use the ILVD
    // offset and width.
    if (bufNum == 0)
    {
        dwBuf=EXTREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword0,
                CPMEM_ILVD_EBA0);
    }
    else
    {
        dwBuf=EXTREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword0,
                CPMEM_ILVD_EBA1_LOW)
               +(EXTREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword1,
                CPMEM_ILVD_EBA1_HIGH)<<CPMEM_ILVD_EBA1_LOW_WID);
    }

    // The address in CPMEM eba0/eba1 fields is shifted by 3, so it needs to be shifted back .
    dwBuf = dwBuf<<3;
    IPU_FUNCTION_EXIT();

    return dwBuf;
}


//------------------------------------------------------------------------------
//
// Function: CPMEMSwapBuffer
//
// This function exchange the buffer0 and buffer1 address in the Channel Parameter Memory.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being configured.
//
//      sequential
//          [in] True if need to adjust buffer address to sequential.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void CPMEMSwapBuffer(DWORD dwChannel,BOOL sequential)
{
    UINT32 dwBuf0,dwBuf1;

    IPU_FUNCTION_ENTRY();

    dwBuf0=EXTREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword0,
            CPMEM_ILVD_EBA0);
    dwBuf1=EXTREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword0,
            CPMEM_ILVD_EBA1_LOW)
           +(EXTREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword1,
            CPMEM_ILVD_EBA1_HIGH)<<CPMEM_ILVD_EBA1_LOW_WID);
    if((!sequential)||(dwBuf0>dwBuf1))
    {
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword0,
                CPMEM_ILVD_EBA0, dwBuf1);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword0,
                CPMEM_ILVD_EBA1_LOW, dwBuf0);
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword1,
                CPMEM_ILVD_EBA1_HIGH, dwBuf0>>CPMEM_ILVD_EBA1_LOW_WID);
        //RETAILMSG(1,(TEXT("%s: SwapED ADDRESS!\r\n"), __WFUNCTION__));
    }
    IPU_FUNCTION_EXIT();

}

//------------------------------------------------------------------------------
//
// Function: CPMEMWriteOffset
//
// This function programs the U and V buffer offsets
// into the Channel Parameter Memory.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being configured.
//
//      pOffsetData
//          [in] Pointer to CPMEMBufOffsets structure containing
//          offsets to configure IDMAC channel.
//
//      bInterleaved
//          [in] TRUE if interleaved mode, FALSE if planar mode.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void CPMEMWriteOffset(DWORD dwChannel, CPMEMBufOffsets *pOffsetData, BOOL bInterleaved)
{
    IPU_FUNCTION_ENTRY();

    // Offsets and word position are same for interleaved and planar
    // when programming ILO, so we can use the ILVD offset and width.
    
    // All address based variable in cpmem must be divided by 8
    
    INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword1,
        CPMEM_ILVD_ILO_LOW, pOffsetData->interlaceOffset >> 3);

    INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,
        CPMEM_ILVD_ILO_HIGH, (pOffsetData->interlaceOffset >> 3)>>CPMEM_ILVD_ILO_LOW_WID);

    // If planar, we must program U and V buffer offsets
    if (!bInterleaved)
    {
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword1,
            CPMEM_PLNR_UBO_LOW, pOffsetData->uOffset >> 3);

        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword2,
            CPMEM_PLNR_UBO_HIGH, (pOffsetData->uOffset >> 3)>>CPMEM_PLNR_UBO_LOW_WID);

        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword2,
            CPMEM_PLNR_VBO, pOffsetData->vOffset >> 3);
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: CPMEMWriteBandMode
//
// This function programs the Band Mode setting for the specified
// IPU DMA channel into the Channel Parameter Memory.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being configured.
//
//      dwBandMode
//          [in] Band mode setting to program for IDMAC channel.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void CPMEMWriteBandMode(DWORD dwChannel, DWORD dwBandMode)
{
    // Offsets and word position are same for interleaved and planar
    // when programming BNDM, so we can use the ILVD offset and width.
    INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword3,
        CPMEM_ILVD_BNDM, dwBandMode);
}
//------------------------------------------------------------------------------
//
// Function: CPMEMWriteXScroll
//
// This function programs the X Scroll setting for the specified
// IPU DMA channel into the Channel Parameter Memory.
// Note: This setting is only valid for input channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being configured.
//
//      dwScrollValue
//          [in] The x scroll value to program for IDMAC channel.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void CPMEMWriteXScroll(DWORD dwChannel, DWORD dwScrollValue)
{
    if(EXTREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword1_dword2,CPMEM_PLNR_PFS)<5)
    {
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword2,
            CPMEM_PLNR_IOX, dwScrollValue);
    }
    else
    {
        INSREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword1,
            CPMEM_ILVD_SX, dwScrollValue);    
    }
}

//------------------------------------------------------------------------------
//
// Function: CPMEMIsCurrentField1
//
// This function checks if current field of corresponding channel is field1.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel need check.
//
// Returns:
//      TRUE if field1,FALSE if field0.
//------------------------------------------------------------------------------
BOOL CPMEMIsCurrentField1(DWORD dwChannel)
{
    if(EXTREG32BF(&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword1,CPMEM_ILVD_CF))
        return TRUE;
    else
        return FALSE;
}

void CPMEMDumpRegs(DWORD dwChannel)
{
    int i;
    UINT32* ptr = (UINT32 *)&g_pIPUV3_CPMEM->CPMEMEntries[dwChannel].mword0_dword0;

    RETAILMSG (1, (TEXT("\n\nCPMEM %d Registers\n"),dwChannel));
    for (i = 0; i <= (sizeof(CPMEM_ENTRY) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}

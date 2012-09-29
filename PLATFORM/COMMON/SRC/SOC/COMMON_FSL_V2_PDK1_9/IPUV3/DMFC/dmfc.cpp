//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dmfc.cpp
//
//  IPU CSP-level DMFC functions(doesn't support alt channel yet)
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "dmfc_priv.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "ipu_common.h"
#include "idmac.h"
#include "dmfc.h"
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
PCSP_IPU_DMFC_REGS g_pIPUV3_DMFC;
const UINT8 gBurstSizeData[] = {128,64,32,16};
const UINT16 gFIFOSizeData[] = {1024,512,256,128,64,32,16};
static HANDLE g_hIPUBase;

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------
//
// Function: DMFCRegsInit
//
// This function allocates the data structures required for interaction
// with the IPUv3 DMFC hardware.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL DMFCRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPUV3_DMFC == NULL)
    {
        //  *** Use IPU_BASE driver to retrieve IPU Base Address ***

        // First, create handle to IPU_BASE driver
        g_hIPUBase = IPUV3BaseOpenHandle();
        if (g_hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        dwIPUBaseAddr = IPUV3BaseGetBaseAddr(g_hIPUBase);
        if (dwIPUBaseAddr == -1)
        {
            RETAILMSG (1,
                (TEXT("%s: Failed to retrieve IPU Base addr!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }
        
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_DMFC_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_DMFC = (PCSP_IPU_DMFC_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_DMFC_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_DMFC == NULL)
        {
            RETAILMSG(1,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
        

    }

    rc = TRUE;

cleanUp:
    // If initialization failed, be sure to clean up
    if (!rc) DMFCRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  DMFCRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 DMFC hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void DMFCRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap peripheral registers
    if (g_pIPUV3_DMFC)
    {
        MmUnmapIoSpace(g_pIPUV3_DMFC, sizeof(PCSP_IPU_DMFC_REGS));
        g_pIPUV3_DMFC = NULL;
    }
    if (!IPUV3BaseCloseHandle(g_hIPUBase))
    {
        RETAILMSG(1,
            (_T("DMFC Cleanup: Failed closing IPU Base handle!\r\n")));
    }

    IPU_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: DMFCEnable
//
// Enable the DMFC.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DMFCEnable(void)
{
    DEBUGMSG (1,
        (TEXT("%s: Enabling DMFC!\r\n"), __WFUNCTION__));

    if(!IPUV3EnableModule(g_hIPUBase, IPU_SUBMODULE_DMFC, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (1,
            (TEXT("%s: Failed to enable DMFC!\r\n"), __WFUNCTION__));
    }

}

//-----------------------------------------------------------------------------
//
// Function: DMFCDisable
//
// Disable the DMFC.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DMFCDisable(void)
{
    DEBUGMSG (1,
        (TEXT("%s: Disabling DMFC!\r\n"), __WFUNCTION__));

    if(!IPUV3DisableModule(g_hIPUBase, IPU_SUBMODULE_DMFC, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (1,
            (TEXT("%s: Failed to disable DMFC!\r\n"), __WFUNCTION__));
    }
}

//-----------------------------------------------------------------------------
//
// Function:  DMFCConfigure
//
// This function configures the corresponding DMFC channel.
// But it doens't support to configure IDMAC channel 21(IDMAC_CH_PRP_OUTPUT_VF)
// IDMAC 21 will be configured directly in prpclass.cpp
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel number
//
//      pConfigData
//          [in] The pointer to DMFC configuration structure.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DMFCConfigure(DWORD dwChannel, pDmfcConfigData pConfigData)
{
    UINT32 oldVal, newVal, iMask, iBitval;
    UINT8 DestPort;
    //burst size must be not larger than fifo size
    if(pConfigData->BurstSize<(pConfigData->FIFOSize-4))
    {
        DEBUGMSG(1,(_T("burst size too large!BurstSize: %d; FIFOSize: %d\r\n"),
                gBurstSizeData[pConfigData->BurstSize], 
                gFIFOSizeData[pConfigData->FIFOSize]));
        pConfigData->BurstSize = pConfigData->FIFOSize -4;
    }
    switch(dwChannel)
    {
        case IDMAC_CH_DC_READ:  //ch40              0
           OUTREG32(&g_pIPUV3_DMFC->DMFC_RD_CHAN, 
                    CSP_BITFVAL(IPU_DMFC_RD_CHAN_DMFC_PPW_C,
                                pConfigData->PixelPerWord)
                   |CSP_BITFVAL(IPU_DMFC_RD_CHAN_DMFC_WM_CLR_0,
                                pConfigData->WaterMarkClear)
                   |CSP_BITFVAL(IPU_DMFC_RD_CHAN_DMFC_WM_SET_0,
                                pConfigData->WaterMarkSet)
                   |CSP_BITFVAL(IPU_DMFC_RD_CHAN_DMFC_WM_EN_0,
                                pConfigData->WaterMarkEnable)
                   |CSP_BITFVAL(IPU_DMFC_RD_CHAN_DMFC_BURST_SIZE_0,
                                pConfigData->BurstSize));

           OUTREG32(&g_pIPUV3_DMFC->DMFC_GENERAL2, 
                    CSP_BITFVAL(IPU_DMFC_GENERAL2_DMFC_FRAME_HEIGHT_RD,
                                pConfigData->FrameHeight)
                   |CSP_BITFVAL(IPU_DMFC_GENERAL2_DMFC_FRAME_WIDTH_RD,
                                pConfigData->FrameWidth));
            break;
        case IDMAC_CH_DC_COMMAND_STREAM2: //ch43    2c
            iMask = CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_2c)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_2c)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_2c);
            iBitval = CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_2c, 
                        pConfigData->BurstSize)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_2c, 
                        pConfigData->FIFOSize)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_2c, 
                        pConfigData->StartSeg);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_WR_CHAN);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_WR_CHAN, 
                        oldVal, newVal) != oldVal);     
            
            iMask = CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_CLR_2c)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_SET_2c)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_EN_2c);
            iBitval = CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_CLR_2c, 
                        pConfigData->WaterMarkClear)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_SET_2c, 
                        pConfigData->WaterMarkSet)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_EN_2c, 
                        pConfigData->WaterMarkEnable);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_WR_CHAN_DEF);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_WR_CHAN_DEF, 
                        oldVal, newVal) != oldVal);            

            break;
        case IDMAC_CH_DC_COMMAND_STREAM1: //ch42    1c
            iMask = CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_1c)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_1c)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_1c);
            iBitval = CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_1c, 
                        pConfigData->BurstSize)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_1c, 
                        pConfigData->FIFOSize)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_1c, 
                        pConfigData->StartSeg);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_WR_CHAN);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_WR_CHAN, 
                        oldVal, newVal) != oldVal);     
            
            iMask = CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_CLR_1c)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_SET_1c)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_EN_1c);
            iBitval = CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_CLR_1c, 
                        pConfigData->WaterMarkClear)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_SET_1c, 
                        pConfigData->WaterMarkSet)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_EN_1c, 
                        pConfigData->WaterMarkEnable);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_WR_CHAN_DEF);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_WR_CHAN_DEF, 
                        oldVal, newVal) != oldVal);     
            break;
        case IDMAC_CH_DC_ASYNC_FLOW: //ch41         2
            iMask = CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_2)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_2)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_2);
            iBitval = CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_2, 
                        pConfigData->BurstSize)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_2, 
                        pConfigData->FIFOSize)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_2, 
                        pConfigData->StartSeg);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_WR_CHAN);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_WR_CHAN, 
                        oldVal, newVal) != oldVal);     
            
            iMask = CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_CLR_2)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_SET_2)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_EN_2);
            iBitval = CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_CLR_2, 
                        pConfigData->WaterMarkClear)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_SET_2, 
                        pConfigData->WaterMarkSet)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_EN_2, 
                        pConfigData->WaterMarkEnable);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_WR_CHAN_DEF);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_WR_CHAN_DEF, 
                        oldVal, newVal) != oldVal);   

            iMask = CSP_BITFMASK(IPU_DMFC_GENERAL1_WAIT4EOT_2);
            if(gFIFOSizeData[pConfigData->FIFOSize] <= pConfigData->FrameWidth)
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_2, 0 ); 
            }
            else
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_2, 1 );                 
                DEBUGMSG(1,
                    (_T("CH41 DMFC burst size too large!BurstSize: %d; FrameWidth: %d\r\n"),
                    gFIFOSizeData[pConfigData->FIFOSize], pConfigData->FrameWidth));
            }
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_GENERAL1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_GENERAL1, 
                        oldVal, newVal) != oldVal);  
                        
            break;
        case IDMAC_CH_DC_SYNC_ASYNC_FLOW: //ch28    1
            iMask = CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_1)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_1)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_1);
            iBitval = CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_1, 
                        pConfigData->BurstSize)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_1, 
                        pConfigData->FIFOSize)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_1, 
                        pConfigData->StartSeg);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_WR_CHAN);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_WR_CHAN, 
                        oldVal, newVal) != oldVal);     
            
            iMask = CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_CLR_1)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_SET_1)
                      |CSP_BITFMASK(IPU_DMFC_WR_CHAN_DEF_WM_EN_1);
            iBitval = CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_CLR_1, 
                        pConfigData->WaterMarkClear)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_SET_1, 
                        pConfigData->WaterMarkSet)
                        |CSP_BITFVAL(IPU_DMFC_WR_CHAN_DEF_WM_EN_1, 
                        pConfigData->WaterMarkEnable);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_WR_CHAN_DEF);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_WR_CHAN_DEF, 
                        oldVal, newVal) != oldVal);     

            iMask = CSP_BITFMASK(IPU_DMFC_GENERAL1_WAIT4EOT_1);
            if(gFIFOSizeData[pConfigData->FIFOSize] <= pConfigData->FrameWidth*2)
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_1, 0 ); 
            }
            else
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_1, 1 );                 
                DEBUGMSG(1,
                    (_T("CH28 DMFC burst size too large!BurstSize: %d; FrameWidth: %d\r\n"),
                    gFIFOSizeData[pConfigData->FIFOSize], pConfigData->FrameWidth));
            }
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_GENERAL1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_GENERAL1, 
                        oldVal, newVal) != oldVal); 
            break;
        case IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE: //ch29   6f
            iMask = CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_6f)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_6f)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_START_ADDR_6f);
            iBitval = CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_6f, 
                        pConfigData->BurstSize)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_6f, 
                        pConfigData->FIFOSize)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_START_ADDR_6f, 
                        pConfigData->StartSeg);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_DP_CHAN);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_DP_CHAN, 
                        oldVal, newVal) != oldVal);     
            
            iMask = CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_6f)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_6f)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_6f);
            iBitval = CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_6f, 
                        pConfigData->WaterMarkClear)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_6f, 
                        pConfigData->WaterMarkSet)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_6f, 
                        pConfigData->WaterMarkEnable);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_DP_CHAN_DEF);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_DP_CHAN_DEF, 
                        oldVal, newVal) != oldVal);     

            iMask = CSP_BITFMASK(IPU_DMFC_GENERAL1_WAIT4EOT_6F);
            if(gFIFOSizeData[pConfigData->FIFOSize] <= pConfigData->FrameWidth)
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_6F, 0 ); 
            }
            else
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_6F, 1 );                 
                DEBUGMSG(1,
                    (_T("CH29 DMFC burst size too large!BurstSize: %d; FrameWidth: %d\r\n"),
                    gFIFOSizeData[pConfigData->FIFOSize], pConfigData->FrameWidth));
            }
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_GENERAL1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_GENERAL1, 
                        oldVal, newVal) != oldVal); 
            break;
        case IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE: //ch24  6b
            iMask = CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_6b)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_6b)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_START_ADDR_6b);
            iBitval = CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_6b, 
                        pConfigData->BurstSize)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_6b, 
                        pConfigData->FIFOSize)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_START_ADDR_6b, 
                        pConfigData->StartSeg);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_DP_CHAN);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_DP_CHAN, 
                        oldVal, newVal) != oldVal);     
            
            iMask = CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_6b)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_6b)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_6b);
            iBitval = CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_6b, 
                        pConfigData->WaterMarkClear)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_6b, 
                        pConfigData->WaterMarkSet)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_6b, 
                        pConfigData->WaterMarkEnable);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_DP_CHAN_DEF);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_DP_CHAN_DEF, 
                        oldVal, newVal) != oldVal);     

            iMask = CSP_BITFMASK(IPU_DMFC_GENERAL1_WAIT4EOT_6B);
            if(gFIFOSizeData[pConfigData->FIFOSize] <= pConfigData->FrameWidth)
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_6B, 0 ); 
            }
            else
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_6B, 1 );                 
                DEBUGMSG(1,
                    (_T("CH24 DMFC burst size too large!BurstSize: %d; FrameWidth: %d\r\n"),
                    gFIFOSizeData[pConfigData->FIFOSize], pConfigData->FrameWidth));
            }
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_GENERAL1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_GENERAL1, 
                        oldVal, newVal) != oldVal); 
            break;
        case IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE: //ch27     5f
            iMask = CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_5f)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_5f)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_START_ADDR_5f);
            iBitval = CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_5f, 
                        pConfigData->BurstSize)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_5f, 
                        pConfigData->FIFOSize)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_START_ADDR_5f, 
                        pConfigData->StartSeg);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_DP_CHAN);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_DP_CHAN, 
                        oldVal, newVal) != oldVal);     
            
            iMask = CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_5f)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_5f)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_5f);
            iBitval = CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_5f, 
                        pConfigData->WaterMarkClear)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_5f, 
                        pConfigData->WaterMarkSet)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_5f, 
                        pConfigData->WaterMarkEnable);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_DP_CHAN_DEF);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_DP_CHAN_DEF, 
                        oldVal, newVal) != oldVal);     

            iMask = CSP_BITFMASK(IPU_DMFC_GENERAL1_WAIT4EOT_5F);
            if(gFIFOSizeData[pConfigData->FIFOSize] <= pConfigData->FrameWidth*2)
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_5F, 0 ); 
            }
            else
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_5F, 1 );                 
                DEBUGMSG(1,
                    (_T("CH27 DMFC burst size too large!BurstSize: %d; FrameWidth: %d\r\n"),
                    gFIFOSizeData[pConfigData->FIFOSize], pConfigData->FrameWidth));
            }
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_GENERAL1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_GENERAL1, 
                        oldVal, newVal) != oldVal); 
            break;
        case IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE: //ch23    5b
            iMask = CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_5b)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_5b)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DMFC_START_ADDR_5b);
            iBitval = CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_5b, 
                        pConfigData->BurstSize)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_5b, 
                        pConfigData->FIFOSize)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DMFC_START_ADDR_5b, 
                        pConfigData->StartSeg);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_DP_CHAN);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_DP_CHAN, 
                        oldVal, newVal) != oldVal);     
            
            iMask = CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_5b)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_5b)
                      |CSP_BITFMASK(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_5b);
            iBitval = CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_5b, 
                        pConfigData->WaterMarkClear)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_5b, 
                        pConfigData->WaterMarkSet)
                        |CSP_BITFVAL(IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_5b, 
                        pConfigData->WaterMarkEnable);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_DP_CHAN_DEF);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_DP_CHAN_DEF, 
                        oldVal, newVal) != oldVal);     

            iMask = CSP_BITFMASK(IPU_DMFC_GENERAL1_WAIT4EOT_5B);
            if(gFIFOSizeData[pConfigData->FIFOSize] <= pConfigData->FrameWidth*3)
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_5B, 0 ); 
            }
            else
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_5B, 1 );                 
                DEBUGMSG(1,
                    (_T("CH23 DMFC burst size too large!BurstSize: %d; FrameWidth: %d\r\n"),
                    gFIFOSizeData[pConfigData->FIFOSize], pConfigData->FrameWidth));
            }
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_GENERAL1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_GENERAL1, 
                        oldVal, newVal) != oldVal); 
            break;

        case IDMAC_CH_DC_OUTPUT_MASK: //ch44    9
            iMask = CSP_BITFMASK(IPU_DMFC_GENERAL1_DMFC_BURST_SIZE_9)
                      |CSP_BITFMASK(IPU_DMFC_GENERAL1_DMFC_WM_CLR_9)
                      |CSP_BITFMASK(IPU_DMFC_GENERAL1_DMFC_WM_SET_9)
                      |CSP_BITFMASK(IPU_DMFC_GENERAL1_DMFC_WM_EN_9);
            iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_DMFC_BURST_SIZE_9, 
                        pConfigData->BurstSize)
                        |CSP_BITFVAL(IPU_DMFC_GENERAL1_DMFC_WM_CLR_9, 
                        pConfigData->WaterMarkClear)
                        |CSP_BITFVAL(IPU_DMFC_GENERAL1_DMFC_WM_SET_9, 
                        pConfigData->WaterMarkSet)
                        |CSP_BITFVAL(IPU_DMFC_GENERAL1_DMFC_WM_EN_9, 
                        pConfigData->WaterMarkEnable);
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_GENERAL1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_GENERAL1, 
                        oldVal, newVal) != oldVal);     

            iMask = CSP_BITFMASK(IPU_DMFC_GENERAL1_WAIT4EOT_3);
            if(gFIFOSizeData[pConfigData->FIFOSize] <= pConfigData->FrameWidth)
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_3, 0 ); 
            }
            else
            {
                iBitval = CSP_BITFVAL(IPU_DMFC_GENERAL1_WAIT4EOT_3, 1 );                 
                DEBUGMSG(1,
                    (_T("CH44 DMFC burst size too large!BurstSize: %d; FrameWidth: %d\r\n"),
                    gFIFOSizeData[pConfigData->FIFOSize], pConfigData->FrameWidth));
            }
            do
            {
                oldVal = INREG32(&g_pIPUV3_DMFC->DMFC_GENERAL1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DMFC->DMFC_GENERAL1, 
                        oldVal, newVal) != oldVal); 
            break;
        case IDMAC_CH_PRP_OUTPUT_VF:    //ch21
            // IC->DP direct path
            switch(pConfigData->DestChannel)
            {
                case IDMAC_CH_DC_SYNC_ASYNC_FLOW: //28
                    DestPort = 0;
                    break;
                case IDMAC_CH_DC_ASYNC_FLOW:      //41
                    DestPort = 1;
                    break;
                case IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE:   //23
                    DestPort = 4;
                    break;
                case IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE:    //27
                    DestPort = 5;
                    break;
                case IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE: //24
                    DestPort = 6;
                    break;
                case IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE:  //29
                    DestPort = 7;
                    break;
                case IDMAC_INVALID_CHANNEL:   //disable ic-dmfc directpath
                    DestPort =2;
                    pConfigData->FrameHeight = 0;
                    pConfigData->FrameWidth = 0;
                    pConfigData->PixelPerWord =0;
                    break;
                default:
                    DEBUGMSG(1,
                        (_T("Incorrect Dest channel number: %d\r\n"),
                        pConfigData->DestChannel));
                    pConfigData->FrameHeight = 0;
                    pConfigData->FrameWidth = 0;
                    pConfigData->PixelPerWord =0;            
                    DestPort = 2;
                    break;
            }
            OUTREG32(&g_pIPUV3_DMFC->DMFC_IC_CTRL, 
                 CSP_BITFVAL(IPU_DMFC_IC_CTRL_DMFC_IC_FRAME_HEIGHT_RD,
                             pConfigData->FrameHeight)
                |CSP_BITFVAL(IPU_DMFC_IC_CTRL_DMFC_IC_FRAME_WIDTH_RD,
                             pConfigData->FrameWidth)
                |CSP_BITFVAL(IPU_DMFC_IC_CTRL_DMFC_IC_PPW_C,
                             pConfigData->PixelPerWord)
                |CSP_BITFVAL(IPU_DMFC_IC_CTRL_DMFC_IC_IN_PORT,
                             DestPort));
            break;
        
        default:
            return FALSE;
            break;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  DMFCFIFOIsFull
//
// This function reported if corresponding DMFC channel FIFO is full.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel number
//
// Returns:
//      Returns TRUE if FULL, otherwise FALSE;
//
//-----------------------------------------------------------------------------
BOOL DMFCFIFOIsFull(DWORD dwChannel)
{
    UINT8 iDMFCchannel;
    UINT8 ret = FALSE;
    switch(dwChannel)
    {
        case IDMAC_CH_DC_READ:  //ch40              0
            iDMFCchannel = 0;
            break;

        case IDMAC_CH_DC_COMMAND_STREAM2: //ch43    2c
            iDMFCchannel = 4;       
            break;

        case IDMAC_CH_DC_COMMAND_STREAM1: //ch42    1c
            iDMFCchannel = 3; 
            break;

        case IDMAC_CH_DC_ASYNC_FLOW: //ch41         2
            iDMFCchannel = 2; 
            break;

        case IDMAC_CH_DC_SYNC_ASYNC_FLOW: //ch28    1
            iDMFCchannel = 1; 
            break;

        case IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE: //ch29   6f
            iDMFCchannel = 8; 
            break;

        case IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE: //ch24  6b
            iDMFCchannel = 7; 
            break;

        case IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE: //ch27     5f
            iDMFCchannel = 6; 
            break;

        case IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE: //ch23    5b
            iDMFCchannel = 5; 
            break;

        case IDMAC_CH_DC_OUTPUT_MASK: //ch44    3
            iDMFCchannel = 9; 
            break;

        case IDMAC_CH_PRP_OUTPUT_VF: //ch21    IC direct to DMFC
            iDMFCchannel = 24; 
            break;

        default:
            return FALSE;
            break;
   }     
   ret = (UINT8)EXTREG32(&g_pIPUV3_DMFC->DMFC_STAT, 1 << iDMFCchannel, iDMFCchannel);
   return ret;
}

//-----------------------------------------------------------------------------
//
// Function:  DMFCFIFOIsEmpty
//
// This function reported if corresponding DMFC channel FIFO is empty.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel number
//
// Returns:
//      Returns TRUE if EMPTY, otherwise FALSE;
//
//-----------------------------------------------------------------------------
BOOL DMFCFIFOIsEmpty(DWORD dwChannel)
{
    UINT8 iDMFCchannel;
    UINT8 ret = FALSE;
    switch(dwChannel)
    {
        case IDMAC_CH_DC_READ:  //ch40              0
            iDMFCchannel = 0;
            break;

        case IDMAC_CH_DC_COMMAND_STREAM2: //ch43    2c
            iDMFCchannel = 4;       
            break;

        case IDMAC_CH_DC_COMMAND_STREAM1: //ch42    1c
            iDMFCchannel = 3; 
            break;

        case IDMAC_CH_DC_ASYNC_FLOW: //ch41         2
            iDMFCchannel = 2; 
            break;

        case IDMAC_CH_DC_SYNC_ASYNC_FLOW: //ch28    1
            iDMFCchannel = 1; 
            break;

        case IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE: //ch29   6f
            iDMFCchannel = 8; 
            break;

        case IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE: //ch24  6b
            iDMFCchannel = 7; 
            break;

        case IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE: //ch27     5f
            iDMFCchannel = 6; 
            break;

        case IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE: //ch23    5b
            iDMFCchannel = 5; 
            break;

        case IDMAC_CH_DC_OUTPUT_MASK: //ch44    3
            iDMFCchannel = 9; 
            break;

        case IDMAC_CH_PRP_OUTPUT_VF: //ch21    IC direct to DMFC
            iDMFCchannel = 13; 
            break;

        default:
            return FALSE;
            break;
   }     
   ret = (UINT8)EXTREG32(&g_pIPUV3_DMFC->DMFC_STAT, 1 << (iDMFCchannel+12), (iDMFCchannel+12));
   return ret;
}

void DMFCDumpRegs()
{
    int i;
    UINT32* ptr = (UINT32 *)&g_pIPUV3_DMFC->DMFC_RD_CHAN;

    RETAILMSG (1, (TEXT("\n\nDMFC Registers\n")));
    for (i = 0; i <= (sizeof(CSP_IPU_DMFC_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}


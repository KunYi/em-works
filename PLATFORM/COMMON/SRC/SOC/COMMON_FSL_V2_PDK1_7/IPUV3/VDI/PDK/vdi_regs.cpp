//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  vdi.cpp
//
//  IPU CSP-level VDI functions
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"
#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "ipu_common.h"
#include "idmac.h"
#include "vdi.h"
#include "vdi_priv.h"
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
PCSP_IPU_VDI_REGS g_pIPUV3_VDI;
static HANDLE g_hIPUBase;

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------
//
// Function: VDIRegsInit
//
// This function allocates the data structures required for interaction
// with the IPUv3 VDI hardware.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL VDIRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPUV3_VDI == NULL)
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
        
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_VDI_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_VDI = (PCSP_IPU_VDI_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_VDI_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_VDI == NULL)
        {
            RETAILMSG(1,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
        

    }

    rc = TRUE;

cleanUp:
    // If initialization failed, be sure to clean up
    if (!rc) VDIRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  VDIRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 VDI hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void VDIRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap peripheral registers
    if (g_pIPUV3_VDI)
    {
        MmUnmapIoSpace(g_pIPUV3_VDI, sizeof(PCSP_IPU_VDI_REGS));
        g_pIPUV3_VDI = NULL;
    }
    if (!IPUV3BaseCloseHandle(g_hIPUBase))
    {
        RETAILMSG(1,
            (_T("VDI Cleanup: Failed closing IPU Base handle!\r\n")));
    }

    IPU_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: VDIModuleEnable
//
// Enable the VDI.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDIModuleEnable(void)
{
    DEBUGMSG (1,
        (TEXT("%s: Enabling VDI!\r\n"), __WFUNCTION__));

    if(!IPUV3EnableModule(g_hIPUBase, IPU_SUBMODULE_VDI, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (1,
            (TEXT("%s: Failed to enable VDI!\r\n"), __WFUNCTION__));
    }

}

//-----------------------------------------------------------------------------
//
// Function: VDIModuleDisable
//
// Disable the VDI.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDIModuleDisable(void)
{
    DEBUGMSG (1,
        (TEXT("%s: Disabling VDI!\r\n"), __WFUNCTION__));

    if(!IPUV3DisableModule(g_hIPUBase, IPU_SUBMODULE_VDI, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (1,
            (TEXT("%s: Failed to disable VDI!\r\n"), __WFUNCTION__));
    }
}


//-----------------------------------------------------------------------------
//
// Function: VDISetFieldDimensions
//
// Configure the width and height of the frame fields.
//
// Parameters:
//      width
//          [in] Field width.
//
//      height
//          [in] Field height.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDISetFieldDimensions(DWORD width, DWORD height)
{
    OUTREG32(&g_pIPUV3_VDI->VDI_FSIZE, CSP_BITFVAL(IPU_VDI_FSIZE_VDI_FWIDTH, width) |
                                        CSP_BITFVAL(IPU_VDI_FSIZE_VDI_FHEIGHT, height));
}

//-----------------------------------------------------------------------------
//
// Function: VDISetPixelFormat
//
// Configure the input/output pixel format for the VDI to YUV420 or YUV422.
//
// Parameters:
//      pixFormat
//          [in] Input and output pixel format for VDI to use.
//              Accepted values:
//                  IPU_VDI_C_VDI_CH_422_FORMAT_422
//                  IPU_VDI_C_VDI_CH_422_FORMAT_420
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDISetPixelFormat(DWORD pixFormat)
{
    INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_CH_422, pixFormat);
}

//-----------------------------------------------------------------------------
//
// Function: VDIMotionSelect
//
// Configure the VDI Motion Calculator Block computation mode.
//
// Parameters:
//      motionSel
//          [in] Motion calculation mode.
//              Accepted values:
//                  IPU_VDI_C_VDI_MOT_SEL_ROM1
//                  IPU_VDI_C_VDI_MOT_SEL_ROM2
//                  IPU_VDI_C_VDI_MOT_SEL_FULL_MOTION
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDIMotionSelect(DWORD motionSel)
{
    INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_MOT_SEL, motionSel);
}

//-----------------------------------------------------------------------------
//
// Function: VDISetBurstSize
//
// Set the burst size for a given VDI channel.
//
// Parameters:
//      vdiChan
//          [in] Select applicable VDI channel
//
//      burstSize
//          [in] Desired burst size in # of accesses.  Valid range = {1-16}
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDISetBurstSize(VDI_CHANNEL vdiChan, DWORD burstSize)
{
    // valid range for burstSize = {1-16}
    if ((burstSize < 1) || (burstSize > 16))
    {
        DEBUGMSG (1,
            (TEXT("%s: Invalid burst size parameter - %d!  Aborting...\r\n"), __WFUNCTION__, burstSize));
        return;
    }

    switch (vdiChan)
    {
        case VDI_CHANNEL_1:
            INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_BURST_SIZE_1, burstSize - 1);
            break;
        case VDI_CHANNEL_2:
            INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_BURST_SIZE_2, burstSize - 1);
            break;
        case VDI_CHANNEL_3:
            INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_BURST_SIZE_3, burstSize - 1);
            break;
        default:
            DEBUGMSG (1,
                (TEXT("%s: Invalid VDI channel!  Aborting...\r\n"), __WFUNCTION__));
    }
}

//-----------------------------------------------------------------------------
//
// Function: VDISetWatermarkLevel
//
// Set the FIFO level at which the watermark signal will be sent to the IDMAC
//
// Parameters:
//      vdiChan
//          [in] Select applicable VDI channel
//
//      level
//          [in] Watermark level.
//              Accepted values:
//                  IPU_VDI_C_WATERMARK_FIFO_1_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_2_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_3_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_4_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_5_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_6_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_7_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_FULL
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDISetWatermarkLevel(VDI_CHANNEL vdiChan, DWORD level)
{
    switch (vdiChan)
    {
        case VDI_CHANNEL_1:
        case VDI_CHANNEL_2:
            INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_VWMI1_SET, level);
            break;
        case VDI_CHANNEL_3:
            INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_VWMI3_SET, level);
            break;
        default:
            DEBUGMSG (1,
                (TEXT("%s: Invalid VDI channel!  Aborting...\r\n"), __WFUNCTION__));
    }
}
//-----------------------------------------------------------------------------
//
// Function: VDIClearWatermarkLevel
//
// Set the FIFO level at which the watermark signal will be cleared.
//
// Parameters:
//      vdiChan
//          [in] Select applicable VDI channel
//
//      level
//          [in] Watermark level.
//              Accepted values:
//                  IPU_VDI_C_WATERMARK_FIFO_1_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_2_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_3_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_4_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_5_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_6_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_7_8TH
//                  IPU_VDI_C_WATERMARK_FIFO_FULL
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDIClearWatermarkLevel(VDI_CHANNEL vdiChan, DWORD level)
{
    switch (vdiChan)
    {
        case VDI_CHANNEL_1:
        case VDI_CHANNEL_2:
            INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_VWMI1_CLR, level);
            break;
        case VDI_CHANNEL_3:
            INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_VWMI3_CLR, level);
            break;
        default:
            DEBUGMSG (1,
                (TEXT("%s: Invalid VDI channel!  Aborting...\r\n"), __WFUNCTION__));
    }
}
//-----------------------------------------------------------------------------
//
// Function: VDISetWatermarkLevel
//
// Set the field for the VDI to treat as the top field during deinterlacing.
//
// Parameters:
//      vdiSource
//          [in] Select applicable VDI channel
//
//      topField
//          [in] Watermark level.
//              Accepted values:
//                  IPU_VDI_C_VDI_TOP_FIELD_FIELD0
//                  IPU_VDI_C_VDI_TOP_FIELD_FIELD1
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDISetTopField(VDI_INPUT_SOURCE vdiSource, DWORD topField)
{
    if (vdiSource == VDI_INPUT_SOURCE_CSI)
    {
        INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_TOP_FIELD_AUTO, topField);
    }
    else
    {
        INSREG32BF(&g_pIPUV3_VDI->VDI_C, IPU_VDI_C_VDI_TOP_FIELD_MAN, topField);
    }
}

void VDIDumpRegs()
{
#ifndef SHIP_BUILD
    UINT32* ptr = (UINT32 *)&g_pIPUV3_VDI->VDI_FSIZE;

    RETAILMSG (1, (TEXT("\n\nVDI Registers\n")));
    RETAILMSG (1, (TEXT("Address %08x  %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1) ));
#endif
}


//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dc.cpp
//
//  Display Controller (DC) low-level access functions
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
#include "common_ipuv3.h"


#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"
#include "idmac.h"
#include "dc.h"

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
static HANDLE g_hIPUBase = NULL;
PCSP_IPU_DC_REGS g_pIPUV3_DC;
PCSP_IPU_MEM_DC_TEMPLATE g_pIPUV3_DC_TEMPLATE;


//------------------------------------------------------------------------------
// Local Variables
BOOL bMaskOffsetInUse_Array[29];


//------------------------------------------------------------------------------
// Local Functions
static DWORD DCGetAvailableMaskOffsetPointer();


//------------------------------------------------------------------------------
//
// Function: DCRegsInit
//
// This function allocates the data structures required for interaction
// with the IPUv3 DC hardware.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL DCRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    // Initialize Mask/Offset array to all 0's (all FALSE)
    memset(bMaskOffsetInUse_Array, 0, sizeof(BOOL) * 29);

    if (g_pIPUV3_DC == NULL)
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

        //********************************************
        // Create pointer to DC registers
        //********************************************
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_DC_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_DC = (PCSP_IPU_DC_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_DC_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_DC == NULL)
        {
            RETAILMSG(1,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

        //********************************************
        // Create pointer to DC Template memory region
        //********************************************
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_DC_TEMPLATE_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_DC_TEMPLATE = (PCSP_IPU_MEM_DC_TEMPLATE) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_MEM_DC_TEMPLATE),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_DC_TEMPLATE == NULL)
        {
            RETAILMSG(1,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    // If initialization failed, be sure to clean up
    if (!rc) DCRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  DCRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 DC hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void DCRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    if (!IPUV3BaseCloseHandle(g_hIPUBase))
    {
        RETAILMSG(1,
            (_T("DC Cleanup: Failed closing IPU Base handle!\r\n")));
    }

    // Unmap peripheral registers
    if (g_pIPUV3_DC)
    {
        MmUnmapIoSpace(g_pIPUV3_DC, sizeof(CSP_IPU_DC_REGS));
        g_pIPUV3_DC = NULL;
    }

    if (g_pIPUV3_DC_TEMPLATE)
    {
        MmUnmapIoSpace(g_pIPUV3_DC_TEMPLATE, sizeof(CSP_IPU_MEM_DC_TEMPLATE));
        g_pIPUV3_DC_TEMPLATE = NULL;
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: DCEnable
//
// This function enables DC for normal opertion
//
// Parameters:
//      none
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCEnable(void)
{
    IPU_FUNCTION_ENTRY();

    // Enable DC

    // Call to IPU Base to turn on DC_EN in IPU_CONF reg.
    // This will also turn on IPU clocks if no other IPU
    // modules have already turned them on.
    if (!IPUV3EnableModule(g_hIPUBase, IPU_SUBMODULE_DC, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (DC_ERROR,
            (TEXT("%s: Failed to enable DC!\r\n"), __WFUNCTION__));
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: DCDisable
//
// This function disables DC
//
// Parameters:
//      none
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCDisable(void)
{
    IPU_FUNCTION_ENTRY();

    // Disable DC

    // Call to IPU Base to turn off DC_EN in IPU_CONF reg.
    if (!IPUV3DisableModule(g_hIPUBase, IPU_SUBMODULE_DC, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (DC_ERROR,
            (TEXT("%s: Failed to disable DC!\r\n"), __WFUNCTION__));
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: DCConfigureWriteChannel
//
// This function fully configures one of the specified DC_WR_CH_CONF registers.
//
// Parameters:
//      writeCh
//          [in] Selects the DC Write channel to configure (1, 2, 5, 6, 8, or 9)
//
//      pWriteChData
//          [in] Pointer to Write Channel configuration data.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCConfigureWriteChannel(DC_CHANNEL writeCh, PDCWriteChanConfData pWriteChData)
{
    IPU_FUNCTION_ENTRY();

    switch(writeCh)
    {
        case DC_CHANNEL_1: // This covers DC_CHANNEL_DC_SYNC_OR_ASYNC, since they
                           // share the same enumeration value
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_CONF_1,
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_1_W_SIZE_1, pWriteChData->dwWordSize) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_1_PROG_DI_ID_1, pWriteChData->dwDINum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_1_PROG_DISP_ID_1, pWriteChData->dwDispNum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_1_PROG_CHAN_TYP_1, pWriteChData->dwChanMode) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_1_CHAN_MASK_DEFAULT_1, pWriteChData->dwEventMask) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_1_FIELD_MODE_1, pWriteChData->dwFieldMode) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_1_PROG_START_TIME_1, pWriteChData->dwStartTime));
            break;
        case DC_CHANNEL_2: // This covers DC_CHANNEL_DC_ASYNC, since they
                           // share the same enumeration value
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_CONF_2,
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_2_W_SIZE_2, pWriteChData->dwWordSize) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_2_PROG_DI_ID_2, pWriteChData->dwDINum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_2_PROG_DISP_ID_2, pWriteChData->dwDispNum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_2_PROG_CHAN_TYP_2, pWriteChData->dwChanMode) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_2_CHAN_MASK_DEFAULT_2, pWriteChData->dwEventMask) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_2_PROG_START_TIME_2, pWriteChData->dwStartTime));
            break;
        case DC_CHANNEL_5: // This covers DC_CHANNEL_DP_PRIMARY, since they
                           // share the same enumeration value
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_CONF_5,
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_5_W_SIZE_5, pWriteChData->dwWordSize) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_5_PROG_DI_ID_5, pWriteChData->dwDINum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_5_PROG_DISP_ID_5, pWriteChData->dwDispNum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_5_PROG_CHAN_TYP_5, pWriteChData->dwChanMode) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_5_CHAN_MASK_DEFAULT_5, pWriteChData->dwEventMask) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_5_FIELD_MODE_5, pWriteChData->dwFieldMode) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_5_PROG_START_TIME_5, pWriteChData->dwStartTime));
            break;
        case DC_CHANNEL_6: // This covers DC_CHANNEL_DP_SECONDARY, since they
                           // share the same enumeration value
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_CONF_6,
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_6_W_SIZE_6, pWriteChData->dwWordSize) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_6_PROG_DI_ID_6, pWriteChData->dwDINum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_6_PROG_DISP_ID_6, pWriteChData->dwDispNum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_6_PROG_CHAN_TYP_6, pWriteChData->dwChanMode) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_6_CHAN_MASK_DEFAULT_6, pWriteChData->dwEventMask) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF_6_PROG_START_TIME_6, pWriteChData->dwStartTime));
            break;
        case DC_CHANNEL_8:
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_CONF1_8,
                CSP_BITFVAL(IPU_DC_WR_CH_CONF1_8_W_SIZE_8, pWriteChData->dwWordSize) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF1_8_MCU_DISP_ID_8, pWriteChData->dwDispNum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF1_8_CHAN_MASK_DEFAULT_8, pWriteChData->dwEventMask));
            break;
        case DC_CHANNEL_9:
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_CONF1_9,
                CSP_BITFVAL(IPU_DC_WR_CH_CONF1_9_W_SIZE_9, pWriteChData->dwWordSize) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF1_9_MCU_DISP_ID_9, pWriteChData->dwDispNum) |
                CSP_BITFVAL(IPU_DC_WR_CH_CONF1_9_CHAN_MASK_DEFAULT_9, pWriteChData->dwEventMask));
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC_CHANNEL!\r\n"), __WFUNCTION__));
            break;
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DCChangeChannelMode
//
// This function changed the DC channel mode 
// so that to enable/disable corresponding DC channel.
//
// Parameters:
//      writeCh
//          [in] Selects the DC Write channel to configure (1, 2, 5, 6)
//
//      dwChanMode
//          [in] The specific channel mode.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCChangeChannelMode(DC_CHANNEL writeCh, DWORD dwChanMode)
{
    IPU_FUNCTION_ENTRY();
    UINT32 oldVal, newVal, iMask, iBitval;

    switch(writeCh)
    {
        case DC_CHANNEL_1: // This covers DC_CHANNEL_DC_SYNC_OR_ASYNC, since they
                           // share the same enumeration value
            // Compute bitmask and shifted bit value for DC_WR_CH_CONF_1 registers
            iMask = CSP_BITFMASK(IPU_DC_WR_CH_CONF_1_PROG_CHAN_TYP_1);
            iBitval = CSP_BITFVAL(IPU_DC_WR_CH_CONF_1_PROG_CHAN_TYP_1, dwChanMode);

            // set DC_WR_CH_CONF_1 with new value
            do
            {
               oldVal = INREG32(&g_pIPUV3_DC->DC_WR_CH_CONF_1);
               newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DC->DC_WR_CH_CONF_1,
                   oldVal, newVal) != oldVal);
            break;
        case DC_CHANNEL_2: // This covers DC_CHANNEL_DC_ASYNC, since they
                           // share the same enumeration value
            // Compute bitmask and shifted bit value for DC_WR_CH_CONF_2 registers
            iMask = CSP_BITFMASK(IPU_DC_WR_CH_CONF_2_PROG_CHAN_TYP_2);
            iBitval = CSP_BITFVAL(IPU_DC_WR_CH_CONF_2_PROG_CHAN_TYP_2, dwChanMode);

            // set DC_WR_CH_CONF_2 with new value
            do
            {
              oldVal = INREG32(&g_pIPUV3_DC->DC_WR_CH_CONF_2);
              newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DC->DC_WR_CH_CONF_2,
                  oldVal, newVal) != oldVal);
            break;
        case DC_CHANNEL_5: // This covers DC_CHANNEL_DP_PRIMARY, since they
                           // share the same enumeration value
            // Compute bitmask and shifted bit value for DC_WR_CH_CONF_5 registers
            iMask = CSP_BITFMASK(IPU_DC_WR_CH_CONF_5_PROG_CHAN_TYP_5);
            iBitval = CSP_BITFVAL(IPU_DC_WR_CH_CONF_5_PROG_CHAN_TYP_5, dwChanMode);

            // set DC_WR_CH_CONF_5 with new value
            do
            {
              oldVal = INREG32(&g_pIPUV3_DC->DC_WR_CH_CONF_5);
              newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DC->DC_WR_CH_CONF_5,
                  oldVal, newVal) != oldVal);
            break;
        case DC_CHANNEL_6: // This covers DC_CHANNEL_DP_SECONDARY, since they
                           // share the same enumeration value
            // Compute bitmask and shifted bit value for DC_WR_CH_CONF_6 registers
            iMask = CSP_BITFMASK(IPU_DC_WR_CH_CONF_6_PROG_CHAN_TYP_6);
            iBitval = CSP_BITFVAL(IPU_DC_WR_CH_CONF_6_PROG_CHAN_TYP_6, dwChanMode);

            // set DC_WR_CH_CONF_6 with new value
            do
            {
              oldVal = INREG32(&g_pIPUV3_DC->DC_WR_CH_CONF_6);
              newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_DC->DC_WR_CH_CONF_6,
                  oldVal, newVal) != oldVal);
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC_CHANNEL!\r\n"), __WFUNCTION__));
            break;
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: DCSetWriteChannelAddress
//
// This function writes to one of the DC_WR_CH_ADDR registers.
//
// Parameters:
//      writeCh
//          [in] Selects the DC Write channel to configure (1, 2, 5, 6, 8, or 9)
//
//      dwAddr
//          [in] The address to program into the DC_WR_CH_ADDR register.  This
//          address specifies the start address in the display's memory
//          space for write transactions.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCSetWriteChannelAddress(DC_CHANNEL writeCh, DWORD dwAddr)
{
    IPU_FUNCTION_ENTRY();

    switch(writeCh)
    {
        case DC_CHANNEL_1:
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_ADDR_1, dwAddr);
            break;
        case DC_CHANNEL_2:
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_ADDR_2, dwAddr);
            break;
        case DC_CHANNEL_5:
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_ADDR_5, dwAddr);
            break;
        case DC_CHANNEL_6:
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_ADDR_6, dwAddr);
            break;
        case DC_CHANNEL_8:
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_CONF2_8, dwAddr);
            break;
        case DC_CHANNEL_9:
            OUTREG32(&g_pIPUV3_DC->DC_WR_CH_CONF2_9, dwAddr);
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC_CHANNEL!\r\n"), __WFUNCTION__));
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DCConfigureGeneralData
//
// This function writes to the DC_GEN register.
//
// Parameters:
//      pGenData
//          [in] Pointer to General Data info to program into DC_GEN reg.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCConfigureGeneralData(PDCGeneralData pGenData)
{
    IPU_FUNCTION_ENTRY();

    OUTREG32(&g_pIPUV3_DC->DC_GEN,
        CSP_BITFVAL(IPU_DC_GEN_SYNC_1_6, pGenData->dwCh1SyncSel) |
        CSP_BITFVAL(IPU_DC_GEN_MASK_EN, pGenData->dwMaskChanEnable) |
        CSP_BITFVAL(IPU_DC_GEN_MASK4CHAN_5, pGenData->dwMaskChanSelect) |
        CSP_BITFVAL(IPU_DC_GEN_SYNC_PRIORITY_5, pGenData->dwCh5Priority) |
        CSP_BITFVAL(IPU_DC_GEN_SYNC_PRIORITY_1, pGenData->dwCh1Priority) |
        CSP_BITFVAL(IPU_DC_GEN_DC_CH5_TYPE, pGenData->dwCh5SyncSel) |
        CSP_BITFVAL(IPU_DC_GEN_DC_BKDIV, pGenData->dwBlinkRate) |
        CSP_BITFVAL(IPU_DC_GEN_DC_BK_EN, pGenData->dwBlinkEnable));

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DCConfigureGeneralData
//
// This function writes to the DC_DISP_CONF1 registers.
//
// Parameters:
//      display
//          [in] Selects one of 4 DC displays
//
//      pDispData
//          [in] Pointer to configuration data for DC display.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCConfigureDisplay(DC_DISPLAY display, PDCDispConfData pDispData)
{
    IPU_FUNCTION_ENTRY();

    switch(display)
    {
        case DC_DISPLAY_0:
            OUTREG32(&g_pIPUV3_DC->DC_DISP_CONF1_0,
                CSP_BITFVAL(IPU_DC_DISP_CONF1_DISP_TYP, pDispData->dwDispType) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_ADDR_INCREMENT, pDispData->dwAddrIncrement) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_ADDR_BE_L_INC, pDispData->dwByteEnableAddrIncrement) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_MCU_ACC_LB_MASK, pDispData->dwAddrCompareMode) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_DISP_RD_VALUE_PTR, pDispData->dwPollingValueAndMaskSelect));
            break;
        case DC_DISPLAY_1:
            OUTREG32(&g_pIPUV3_DC->DC_DISP_CONF1_1,
                CSP_BITFVAL(IPU_DC_DISP_CONF1_DISP_TYP, pDispData->dwDispType) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_ADDR_INCREMENT, pDispData->dwAddrIncrement) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_ADDR_BE_L_INC, pDispData->dwByteEnableAddrIncrement) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_MCU_ACC_LB_MASK, pDispData->dwAddrCompareMode) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_DISP_RD_VALUE_PTR, pDispData->dwPollingValueAndMaskSelect));
            break;
        case DC_DISPLAY_2:
            OUTREG32(&g_pIPUV3_DC->DC_DISP_CONF1_2,
                CSP_BITFVAL(IPU_DC_DISP_CONF1_DISP_TYP, pDispData->dwDispType) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_ADDR_INCREMENT, pDispData->dwAddrIncrement) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_ADDR_BE_L_INC, pDispData->dwByteEnableAddrIncrement) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_MCU_ACC_LB_MASK, pDispData->dwAddrCompareMode) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_DISP_RD_VALUE_PTR, pDispData->dwPollingValueAndMaskSelect));
            break;
        case DC_DISPLAY_3:
            OUTREG32(&g_pIPUV3_DC->DC_DISP_CONF1_3,
                CSP_BITFVAL(IPU_DC_DISP_CONF1_DISP_TYP, pDispData->dwDispType) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_ADDR_INCREMENT, pDispData->dwAddrIncrement) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_ADDR_BE_L_INC, pDispData->dwByteEnableAddrIncrement) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_MCU_ACC_LB_MASK, pDispData->dwAddrCompareMode) |
                CSP_BITFVAL(IPU_DC_DISP_CONF1_DISP_RD_VALUE_PTR, pDispData->dwPollingValueAndMaskSelect));
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC_DISPLAY!\r\n"), __WFUNCTION__));
            break;
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: DCSetDisplayStride
//
// This function writes the display stride value to
// the DC_DISP_CONF_2 registers.
//
// Parameters:
//      display
//          [in] Selects one of 4 DC displays
//
//      stride
//          [in] Stride value for the display
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCSetDisplayStride(DC_DISPLAY display, DWORD stride)
{
    IPU_FUNCTION_ENTRY();

    switch(display)
    {
        case DC_DISPLAY_0:
            OUTREG32(&g_pIPUV3_DC->DC_DISP_CONF2_0, stride);
            break;
        case DC_DISPLAY_1:
            OUTREG32(&g_pIPUV3_DC->DC_DISP_CONF2_1, stride);
            break;
        case DC_DISPLAY_2:
            OUTREG32(&g_pIPUV3_DC->DC_DISP_CONF2_2, stride);
            break;
        case DC_DISPLAY_3:
            OUTREG32(&g_pIPUV3_DC->DC_DISP_CONF2_3, stride);
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC_DISPLAY!\r\n"), __WFUNCTION__));
            break;
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DCConfigureUserEventData
//
// This function writes configuration data for one of the 4
// User general event registers (DC_UGDEx_0).
//
// Parameters:
//      event
//          [in] Select one of 4 user general events to configure.
//
//      pUserEventData
//          [in] Configuration info for the user general event.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCConfigureUserEventData(DC_USER_GENERAL_DATA_EVENT event, PDCUserEventConfData pUserEventData)
{
    IPU_FUNCTION_ENTRY();

    switch(event)
    {
        case DC_USER_GENERAL_DATA_EVENT_0:
            OUTREG32(&g_pIPUV3_DC->DC_UGDE0_0,
                CSP_BITFVAL(IPU_DC_UGDE_0_ID_CODED, pUserEventData->dwEventDCChan) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_EV_PRIORITY, pUserEventData->dwEventPriority) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_EV_START, pUserEventData->dwMicrocodeStartAddr) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_ODD_START, pUserEventData->dwOddModeMicrocodeStartAddr) |
                CSP_BITFVAL(IPU_DC_UGDE_0_ODD_EN, pUserEventData->dwOddModeEnable) |
                CSP_BITFVAL(IPU_DC_UGDE_0_AUTO_RESTART, pUserEventData->dwAutorestartEnable) |
                CSP_BITFVAL(IPU_DC_UGDE_0_NF_NL, pUserEventData->dwCounterTriggerSelect));
            OUTREG32(&g_pIPUV3_DC->DC_UGDE0_1, pUserEventData->dwCounterCompare);
            OUTREG32(&g_pIPUV3_DC->DC_UGDE0_2, pUserEventData->dwCounterOffset);
            OUTREG32(&g_pIPUV3_DC->DC_UGDE0_3, pUserEventData->dwEventNumber);
            break;
        case DC_USER_GENERAL_DATA_EVENT_1:
            OUTREG32(&g_pIPUV3_DC->DC_UGDE0_1,
                CSP_BITFVAL(IPU_DC_UGDE_0_ID_CODED, pUserEventData->dwEventDCChan) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_EV_PRIORITY, pUserEventData->dwEventPriority) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_EV_START, pUserEventData->dwMicrocodeStartAddr) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_ODD_START, pUserEventData->dwOddModeMicrocodeStartAddr) |
                CSP_BITFVAL(IPU_DC_UGDE_0_ODD_EN, pUserEventData->dwOddModeEnable) |
                CSP_BITFVAL(IPU_DC_UGDE_0_AUTO_RESTART, pUserEventData->dwAutorestartEnable) |
                CSP_BITFVAL(IPU_DC_UGDE_0_NF_NL, pUserEventData->dwCounterTriggerSelect));
            OUTREG32(&g_pIPUV3_DC->DC_UGDE1_1, pUserEventData->dwCounterCompare);
            OUTREG32(&g_pIPUV3_DC->DC_UGDE1_2, pUserEventData->dwCounterOffset);
            OUTREG32(&g_pIPUV3_DC->DC_UGDE1_3, pUserEventData->dwEventNumber);
            break;
        case DC_USER_GENERAL_DATA_EVENT_2:
            OUTREG32(&g_pIPUV3_DC->DC_UGDE0_2,
                CSP_BITFVAL(IPU_DC_UGDE_0_ID_CODED, pUserEventData->dwEventDCChan) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_EV_PRIORITY, pUserEventData->dwEventPriority) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_EV_START, pUserEventData->dwMicrocodeStartAddr) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_ODD_START, pUserEventData->dwOddModeMicrocodeStartAddr) |
                CSP_BITFVAL(IPU_DC_UGDE_0_ODD_EN, pUserEventData->dwOddModeEnable) |
                CSP_BITFVAL(IPU_DC_UGDE_0_AUTO_RESTART, pUserEventData->dwAutorestartEnable) |
                CSP_BITFVAL(IPU_DC_UGDE_0_NF_NL, pUserEventData->dwCounterTriggerSelect));
            OUTREG32(&g_pIPUV3_DC->DC_UGDE2_1, pUserEventData->dwCounterCompare);
            OUTREG32(&g_pIPUV3_DC->DC_UGDE2_2, pUserEventData->dwCounterOffset);
            OUTREG32(&g_pIPUV3_DC->DC_UGDE2_3, pUserEventData->dwEventNumber);
            break;
        case DC_USER_GENERAL_DATA_EVENT_3:
            OUTREG32(&g_pIPUV3_DC->DC_UGDE0_3,
                CSP_BITFVAL(IPU_DC_UGDE_0_ID_CODED, pUserEventData->dwEventDCChan) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_EV_PRIORITY, pUserEventData->dwEventPriority) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_EV_START, pUserEventData->dwMicrocodeStartAddr) |
                CSP_BITFVAL(IPU_DC_UGDE_0_COD_ODD_START, pUserEventData->dwOddModeMicrocodeStartAddr) |
                CSP_BITFVAL(IPU_DC_UGDE_0_ODD_EN, pUserEventData->dwOddModeEnable) |
                CSP_BITFVAL(IPU_DC_UGDE_0_AUTO_RESTART, pUserEventData->dwAutorestartEnable) |
                CSP_BITFVAL(IPU_DC_UGDE_0_NF_NL, pUserEventData->dwCounterTriggerSelect));
            OUTREG32(&g_pIPUV3_DC->DC_UGDE3_1, pUserEventData->dwCounterCompare);
            OUTREG32(&g_pIPUV3_DC->DC_UGDE3_2, pUserEventData->dwCounterOffset);
            OUTREG32(&g_pIPUV3_DC->DC_UGDE3_3, pUserEventData->dwEventNumber);
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC_USER_GENERAL_DATA_EVENT!\r\n"), __WFUNCTION__));
            break;
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DCDisableUserEventData
//
// This function disables one of the 4
// User general event registers (DC_UGDEx_0).
//
// Parameters:
//      event
//          [in] Select one of 4 user general events to disable.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCDisableUserEventData(DC_USER_GENERAL_DATA_EVENT event)
{
    IPU_FUNCTION_ENTRY();

    switch(event)
    {
        case DC_USER_GENERAL_DATA_EVENT_0:
            OUTREG32(&g_pIPUV3_DC->DC_UGDE0_0, 0);
            break;
        case DC_USER_GENERAL_DATA_EVENT_1:
            OUTREG32(&g_pIPUV3_DC->DC_UGDE1_0, 0);
            break;
        case DC_USER_GENERAL_DATA_EVENT_2:
            OUTREG32(&g_pIPUV3_DC->DC_UGDE2_0, 0);
            break;
        case DC_USER_GENERAL_DATA_EVENT_3:
            OUTREG32(&g_pIPUV3_DC->DC_UGDE3_0, 0);
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC_USER_GENERAL_DATA_EVENT!\r\n"), __WFUNCTION__));
            break;
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DCConfigureMicrocode
//
// This function writes one entry into the DC template memory.
//
// Parameters:
//      pMicrocodeData
//          [in] Pointer to structure holding microcode instruction data.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCConfigureMicrocode(PMicrocodeConfigData pMicrocodeData)
{
    DWORD dwLowWord, dwHighWord;

    IPU_FUNCTION_ENTRY();

    switch (pMicrocodeData->Command)
    {
        case DC_MICROCODE_COMMAND_HLG:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_HLG_DATA_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_HLG_DATA_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_HLG_DATA_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_HLG_OPCODE, HLG_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WRG:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_WRG_DATA_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WRG_DATA_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_WRG_DATA_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_WRG_OPCODE, WRG_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_HLOA:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping) |
                CSP_BITFVAL(DC_TEMPLATE_HLOA_DATA_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_HLOA_DATA_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_HLOA_DATA_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_HLOA_AF, pMicrocodeData->dwAf) |
                CSP_BITFVAL(DC_TEMPLATE_HLOA_OPCODE, HLOA_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WROA:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping) |
                CSP_BITFVAL(DC_TEMPLATE_WROA_DATA_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WROA_DATA_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_WROA_DATA_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_WROA_AF, pMicrocodeData->dwAf) |
                CSP_BITFVAL(DC_TEMPLATE_WROA_OPCODE, WROA_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_HLOD:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping) |
                CSP_BITFVAL(DC_TEMPLATE_HLOD_DATA_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_HLOD_DATA_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_HLOD_DATA_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_HLOD_OPCODE, HLOD_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WROD:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping) |
                CSP_BITFVAL(DC_TEMPLATE_WROD_DATA_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WROD_DATA_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_WROD_DATA_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_WROD_OPCODE, WROD_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_HLOAR:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_HLOAR_AF, pMicrocodeData->dwAf) |
                CSP_BITFVAL(DC_TEMPLATE_HLOAR_OPCODE, HLOAR_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WROAR:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WROAR_AF, pMicrocodeData->dwAf) |
                CSP_BITFVAL(DC_TEMPLATE_WROAR_OPCODE, WROAR_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_HLODR:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_HLODR_OPCODE, HLODR_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WRODR:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping) |
                CSP_BITFVAL(DC_TEMPLATE_WRODR_M0, 0) |
                CSP_BITFVAL(DC_TEMPLATE_WRODR_M1, 0);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WRODR_M2, 0) |
                CSP_BITFVAL(DC_TEMPLATE_WRODR_OPCODE, WRODR_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WRBC:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WRODR_OPCODE, WRODR_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WCLK:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_WCLK_N_CLK_OPERAND_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WCLK_N_CLK_OPERAND_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_WCLK_N_CLK_OPERAND_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_WCLK_OPCODE, WCLK_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WSTSI:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping) |
                CSP_BITFVAL(DC_TEMPLATE_WSTSI_N_CLK_OPERAND_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WSTSI_N_CLK_OPERAND_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_WSTSI_N_CLK_OPERAND_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_WSTSI_OPCODE, WSTSI_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WSTSII:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping) |
                CSP_BITFVAL(DC_TEMPLATE_WSTSII_N_CLK_OPERAND_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WSTSII_N_CLK_OPERAND_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_WSTSII_N_CLK_OPERAND_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_WSTSII_OPCODE, WSTSII_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_WSTSIII:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping) |
                CSP_BITFVAL(DC_TEMPLATE_WSTSIII_N_CLK_OPERAND_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WSTSIII_N_CLK_OPERAND_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_WSTSIII_N_CLK_OPERAND_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_WSTSIII_OPCODE, WSTSIII_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_RD:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_GLUELOGIC, pMicrocodeData->dwGluelogic) |
                CSP_BITFVAL(DC_TEMPLATE_WAVEFORM, pMicrocodeData->dwWaveform) |
                CSP_BITFVAL(DC_TEMPLATE_MAPPING, pMicrocodeData->dwMapping) |
                CSP_BITFVAL(DC_TEMPLATE_RD_N_CLK_OPERAND_LOW, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_RD_N_CLK_OPERAND_HIGH, pMicrocodeData->dwOperand >> DC_TEMPLATE_RD_N_CLK_OPERAND_LOW_WID) |
                CSP_BITFVAL(DC_TEMPLATE_RD_OPCODE, RD_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_MSK:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_MSK_DM, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_NCM, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_NADM, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_EOFLDM, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_EOLM, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_EOFM, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_NFLDM, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_NLM, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_NFM, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_E3M, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_E2M, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_E1M, 0) |
                CSP_BITFVAL(DC_TEMPLATE_MSK_E0M, 0);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_WACK_OPCODE, WACK_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_HMA:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_HMA_ADDRESS, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_HMA_OPCODE, HMA_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_HMA1:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_HMA1_ADDRESS, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_HMA1_OPCODE, HMA1_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        case DC_MICROCODE_COMMAND_BMA:
            dwLowWord = CSP_BITFVAL(DC_TEMPLATE_SYNC, pMicrocodeData->dwSync) |
                CSP_BITFVAL(DC_TEMPLATE_BMA_N, pMicrocodeData->dwOperand);
            dwHighWord = CSP_BITFVAL(DC_TEMPLATE_BMA_OPCODE, BMA_OPCODE) |
                CSP_BITFVAL(DC_TEMPLATE_BMA_AF, pMicrocodeData->dwAf) |
                CSP_BITFVAL(DC_TEMPLATE_BMA_LF, pMicrocodeData->dwLf) |
                CSP_BITFVAL(DC_TEMPLATE_STOP, pMicrocodeData->dwStop);
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC Microcode instruction!\r\n"), __WFUNCTION__));
            return;
    }

    OUTREG32(&g_pIPUV3_DC_TEMPLATE->DCTemplateEntries[pMicrocodeData->dwWord][0], dwLowWord);
    OUTREG32(&g_pIPUV3_DC_TEMPLATE->DCTemplateEntries[pMicrocodeData->dwWord][1], dwHighWord);

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DCConfigureMicrocodeEvent
//
// This function writes data into the DC Routine Link registers to configure
// the priority and microcode start address for events triggered by one of
// the DC_EVENT events.
//
// Parameters:
//      writeCh
//          [in] Select DC write channel to configure microcode event for.
//
//      dcEvent
//          [in] Microcode event type.
//
//      priority
//          [in] Priority for the event.
//
//      address
//          [in] Microcode address triggered by event.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCConfigureMicrocodeEvent(DC_CHANNEL writeCh, DC_EVENT_TYPE dcEvent, DWORD priority, DWORD address)
{
    IPU_FUNCTION_ENTRY();

    switch (writeCh)
    {
        case DC_CHANNEL_0:
            switch(dcEvent)
            {
                case DC_EVENT_NEW_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_0,
                        IPU_DC_RL0_CH_COD_NL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_0,
                        IPU_DC_RL0_CH_COD_NL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_0,
                        IPU_DC_RL0_CH_COD_NF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_0,
                        IPU_DC_RL0_CH_COD_NF_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_0,
                        IPU_DC_RL1_CH_COD_NFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_0,
                        IPU_DC_RL1_CH_COD_NFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_0,
                        IPU_DC_RL1_CH_COD_EOF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_0,
                        IPU_DC_RL1_CH_COD_EOF_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_0,
                        IPU_DC_RL2_CH_COD_EOFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_0,
                        IPU_DC_RL2_CH_COD_EOFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_0,
                        IPU_DC_RL2_CH_COD_EOL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_0,
                        IPU_DC_RL2_CH_COD_EOL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_CHAN:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_0,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_0,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_ADDR:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_0,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_0,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_DATA:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_0,
                        IPU_DC_RL4_CH_COD_NEW_DATA_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_0,
                        IPU_DC_RL4_CH_COD_NEW_DATA_START_CHAN, address);
                    break;
                default:
                    DEBUGMSG (DC_ERROR,
                        (TEXT("%s: Invalid DC event type.\r\n"), __WFUNCTION__));
            }
            break;
        case DC_CHANNEL_1:
            switch(dcEvent)
            {
                case DC_EVENT_NEW_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_1,
                        IPU_DC_RL0_CH_COD_NL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_1,
                        IPU_DC_RL0_CH_COD_NL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_1,
                        IPU_DC_RL0_CH_COD_NF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_1,
                        IPU_DC_RL0_CH_COD_NF_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_1,
                        IPU_DC_RL1_CH_COD_NFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_1,
                        IPU_DC_RL1_CH_COD_NFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_1,
                        IPU_DC_RL1_CH_COD_EOF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_1,
                        IPU_DC_RL1_CH_COD_EOF_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_1,
                        IPU_DC_RL2_CH_COD_EOFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_1,
                        IPU_DC_RL2_CH_COD_EOFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_1,
                        IPU_DC_RL2_CH_COD_EOL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_1,
                        IPU_DC_RL2_CH_COD_EOL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_CHAN:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_1,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_1,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_ADDR:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_1,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_1,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_DATA:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_1,
                        IPU_DC_RL4_CH_COD_NEW_DATA_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_1,
                        IPU_DC_RL4_CH_COD_NEW_DATA_START_CHAN, address);
                    break;
                default:
                    DEBUGMSG (DC_ERROR,
                        (TEXT("%s: Invalid DC event type.\r\n"), __WFUNCTION__));
            }
            break;
        case DC_CHANNEL_2:
            switch(dcEvent)
            {
                case DC_EVENT_NEW_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_2,
                        IPU_DC_RL0_CH_COD_NL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_2,
                        IPU_DC_RL0_CH_COD_NL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_2,
                        IPU_DC_RL0_CH_COD_NF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_2,
                        IPU_DC_RL0_CH_COD_NF_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_2,
                        IPU_DC_RL1_CH_COD_NFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_2,
                        IPU_DC_RL1_CH_COD_NFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_2,
                        IPU_DC_RL1_CH_COD_EOF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_2,
                        IPU_DC_RL1_CH_COD_EOF_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_2,
                        IPU_DC_RL2_CH_COD_EOFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_2,
                        IPU_DC_RL2_CH_COD_EOFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_2,
                        IPU_DC_RL2_CH_COD_EOL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_2,
                        IPU_DC_RL2_CH_COD_EOL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_CHAN:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_2,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_2,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_ADDR:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_2,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_2,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_DATA:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_2,
                        IPU_DC_RL4_CH_COD_NEW_DATA_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_2,
                        IPU_DC_RL4_CH_COD_NEW_DATA_START_CHAN, address);
                    break;
                default:
                    DEBUGMSG (DC_ERROR,
                        (TEXT("%s: Invalid DC event type.\r\n"), __WFUNCTION__));
            }
            break;
        case DC_CHANNEL_5:
            switch(dcEvent)
            {
                case DC_EVENT_NEW_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_5,
                        IPU_DC_RL0_CH_COD_NL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_5,
                        IPU_DC_RL0_CH_COD_NL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_5,
                        IPU_DC_RL0_CH_COD_NF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_5,
                        IPU_DC_RL0_CH_COD_NF_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_5,
                        IPU_DC_RL1_CH_COD_NFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_5,
                        IPU_DC_RL1_CH_COD_NFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_5,
                        IPU_DC_RL1_CH_COD_EOF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_5,
                        IPU_DC_RL1_CH_COD_EOF_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_5,
                        IPU_DC_RL2_CH_COD_EOFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_5,
                        IPU_DC_RL2_CH_COD_EOFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_5,
                        IPU_DC_RL2_CH_COD_EOL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_5,
                        IPU_DC_RL2_CH_COD_EOL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_CHAN:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_5,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_5,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_ADDR:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_5,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_5,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_DATA:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_5,
                        IPU_DC_RL4_CH_COD_NEW_DATA_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_5,
                        IPU_DC_RL4_CH_COD_NEW_DATA_START_CHAN, address);
                    break;
                default:
                    DEBUGMSG (DC_ERROR,
                        (TEXT("%s: Invalid DC event type.\r\n"), __WFUNCTION__));
            }
            break;
        case DC_CHANNEL_6:
            switch(dcEvent)
            {
                case DC_EVENT_NEW_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_6,
                        IPU_DC_RL0_CH_COD_NL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_6,
                        IPU_DC_RL0_CH_COD_NL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_6,
                        IPU_DC_RL0_CH_COD_NF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL0_CH_6,
                        IPU_DC_RL0_CH_COD_NF_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_6,
                        IPU_DC_RL1_CH_COD_NFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_6,
                        IPU_DC_RL1_CH_COD_NFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FRAME:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_6,
                        IPU_DC_RL1_CH_COD_EOF_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_6,
                        IPU_DC_RL1_CH_COD_EOF_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_FIELD:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_6,
                        IPU_DC_RL2_CH_COD_EOFIELD_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_6,
                        IPU_DC_RL2_CH_COD_EOFIELD_START_CHAN, address);
                    break;
                case DC_EVENT_END_OF_LINE:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_6,
                        IPU_DC_RL2_CH_COD_EOL_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_6,
                        IPU_DC_RL2_CH_COD_EOL_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_CHAN:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_6,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_6,
                        IPU_DC_RL3_CH_COD_NEW_CHAN_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_ADDR:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_6,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_6,
                        IPU_DC_RL3_CH_COD_NEW_ADDR_START_CHAN, address);
                    break;
                case DC_EVENT_NEW_DATA:
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_6,
                        IPU_DC_RL4_CH_COD_NEW_DATA_PRIORITY_CHAN, priority);
                    INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_6,
                        IPU_DC_RL4_CH_COD_NEW_DATA_START_CHAN, address);
                    break;
                default:
                    DEBUGMSG (DC_ERROR,
                        (TEXT("%s: Invalid DC event type.\r\n"), __WFUNCTION__));
            }
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC Channel.  Must be 0, 1, 2, 5, or 6.\r\n"), __WFUNCTION__));
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: DCConfigureMicrocodeEventMCU
//
// This function writes data into the DC Routine Link registers to configure
// the priority and microcode start address for events triggered by one of
// the DC_EVENT events.  This function is special to DC Channels 8 and 9,
// which are triggered by fewer events, contain read and write events,
// and have 2 address regions to program.
//
// Parameters:
//      writeCh
//          [in] Select DC write channel to configure microcode event for.
//
//      dcEvent
//          [in] Microcode event type.
//
//      readWriteSel
//          [in] Select whether to program a read or write event.
//
//      region
//          [in] Select whether to program the region 1 or region 2 address.
//
//      priority
//          [in] Priority for the event.
//
//      address
//          [in] Microcode address triggered by event.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DCConfigureMicrocodeEventMCU(DC_CHANNEL writeCh, DC_EVENT_TYPE dcEvent,
    DWORD readWriteSel, DWORD region, DWORD priority, DWORD address)
{
    IPU_FUNCTION_ENTRY();

    switch (writeCh)
    {
        case DC_CHANNEL_8:
            switch(dcEvent)
            {
                case DC_EVENT_NEW_ADDR:
                    if (readWriteSel == DC_READ_WRITE_SELECT_WRITE)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_8,
                                IPU_DC_RL1_CH_COD_NEW_ADDR_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_8,
                                IPU_DC_RL1_CH_COD_NEW_ADDR_START_CHAN_W_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_8,
                                IPU_DC_RL1_CH_COD_NEW_ADDR_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_8,
                                IPU_DC_RL1_CH_COD_NEW_ADDR_START_CHAN_W_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else if (readWriteSel == DC_READ_WRITE_SELECT_READ)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_8,
                                IPU_DC_RL4_CH_COD_NEW_ADDR_START_CHAN_R_0, address);                            
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_8,
                                IPU_DC_RL4_CH_COD_NEW_ADDR_START_CHAN_R_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else
                    {
                        DEBUGMSG (DC_ERROR,
                            (TEXT("%s: Invalid read/write select value - be 0 (read) or 1 (write).\r\n"), __WFUNCTION__));
                    }
                    break;
                case DC_EVENT_NEW_CHAN:
                    if (readWriteSel == DC_READ_WRITE_SELECT_WRITE)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_8,
                                IPU_DC_RL2_CH_COD_NEW_CHAN_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_8,
                                IPU_DC_RL2_CH_COD_NEW_CHAN_START_CHAN_W_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_8,
                                IPU_DC_RL2_CH_COD_NEW_CHAN_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_8,
                                IPU_DC_RL2_CH_COD_NEW_CHAN_START_CHAN_W_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else if (readWriteSel == DC_READ_WRITE_SELECT_READ)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL5_CH_8,
                                IPU_DC_RL5_CH_COD_NEW_CHAN_START_CHAN_R_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL5_CH_8,
                                IPU_DC_RL5_CH_COD_NEW_CHAN_START_CHAN_R_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else
                    {
                        DEBUGMSG (DC_ERROR,
                            (TEXT("%s: Invalid read/write select value - be 0 (read) or 1 (write).\r\n"), __WFUNCTION__));
                    }
                    break;
                case DC_EVENT_NEW_DATA:
                    if (readWriteSel == DC_READ_WRITE_SELECT_WRITE)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_8,
                                IPU_DC_RL3_CH_COD_NEW_DATA_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_8,
                                IPU_DC_RL3_CH_COD_NEW_DATA_START_CHAN_W_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_8,
                                IPU_DC_RL3_CH_COD_NEW_DATA_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_8,
                                IPU_DC_RL3_CH_COD_NEW_DATA_START_CHAN_W_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else if (readWriteSel == DC_READ_WRITE_SELECT_READ)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL6_CH_8,
                                IPU_DC_RL6_CH_COD_NEW_DATA_START_CHAN_R_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL6_CH_8,
                                IPU_DC_RL6_CH_COD_NEW_DATA_START_CHAN_R_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else
                    {
                        DEBUGMSG (DC_ERROR,
                            (TEXT("%s: Invalid read/write select value - be 0 (read) or 1 (write).\r\n"), __WFUNCTION__));
                    }
                    break;
                default:
                    DEBUGMSG (DC_ERROR,
                        (TEXT("%s: Invalid DC event type.  For Ch's 8 and 9, must be either new chan, new addr, or new data.\r\n"), __WFUNCTION__));
            return;
            }
            break;
        case DC_CHANNEL_9:
            switch(dcEvent)
            {
                case DC_EVENT_NEW_ADDR:
                    if (readWriteSel == DC_READ_WRITE_SELECT_WRITE)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_9,
                                IPU_DC_RL1_CH_COD_NEW_ADDR_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_9,
                                IPU_DC_RL1_CH_COD_NEW_ADDR_START_CHAN_W_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_9,
                                IPU_DC_RL1_CH_COD_NEW_ADDR_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL1_CH_9,
                                IPU_DC_RL1_CH_COD_NEW_ADDR_START_CHAN_W_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else if (readWriteSel == DC_READ_WRITE_SELECT_READ)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_9,
                                IPU_DC_RL4_CH_COD_NEW_ADDR_START_CHAN_R_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL4_CH_9,
                                IPU_DC_RL4_CH_COD_NEW_ADDR_START_CHAN_R_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else
                    {
                        DEBUGMSG (DC_ERROR,
                            (TEXT("%s: Invalid read/write select value - be 0 (read) or 1 (write).\r\n"), __WFUNCTION__));
                    }
                    break;
                case DC_EVENT_NEW_CHAN:
                    if (readWriteSel == DC_READ_WRITE_SELECT_WRITE)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_9,
                                IPU_DC_RL2_CH_COD_NEW_CHAN_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_9,
                                IPU_DC_RL2_CH_COD_NEW_CHAN_START_CHAN_W_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_9,
                                IPU_DC_RL2_CH_COD_NEW_CHAN_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL2_CH_9,
                                IPU_DC_RL2_CH_COD_NEW_CHAN_START_CHAN_W_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else if (readWriteSel == DC_READ_WRITE_SELECT_READ)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL5_CH_9,
                                IPU_DC_RL5_CH_COD_NEW_CHAN_START_CHAN_R_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL5_CH_9,
                                IPU_DC_RL5_CH_COD_NEW_CHAN_START_CHAN_R_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else
                    {
                        DEBUGMSG (DC_ERROR,
                            (TEXT("%s: Invalid read/write select value - be 0 (read) or 1 (write).\r\n"), __WFUNCTION__));
                    }
                    break;
                case DC_EVENT_NEW_DATA:
                    if (readWriteSel == DC_READ_WRITE_SELECT_WRITE)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_8,
                                IPU_DC_RL3_CH_COD_NEW_DATA_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_8,
                                IPU_DC_RL3_CH_COD_NEW_DATA_START_CHAN_W_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_8,
                                IPU_DC_RL3_CH_COD_NEW_DATA_PRIORITY_CHAN_8_9, priority);
                            INSREG32BF(&g_pIPUV3_DC->DC_RL3_CH_8,
                                IPU_DC_RL3_CH_COD_NEW_DATA_START_CHAN_W_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else if (readWriteSel == DC_READ_WRITE_SELECT_READ)
                    {
                        if (region == DC_REGION_SELECT_REGION1)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL6_CH_8,
                                IPU_DC_RL6_CH_COD_NEW_DATA_START_CHAN_R_0, address);
                        }
                        else if (region == DC_REGION_SELECT_REGION2)
                        {
                            INSREG32BF(&g_pIPUV3_DC->DC_RL6_CH_8,
                                IPU_DC_RL6_CH_COD_NEW_DATA_START_CHAN_R_1, address);
                        }
                        else
                        {
                            DEBUGMSG (DC_ERROR,
                                (TEXT("%s: Invalid Region - must be 1 or 2.\r\n"), __WFUNCTION__));
                        }
                    }
                    else
                    {
                        DEBUGMSG (DC_ERROR,
                            (TEXT("%s: Invalid read/write select value - be 0 (read) or 1 (write).\r\n"), __WFUNCTION__));
                    }
                    break;
                default:
                    DEBUGMSG (DC_ERROR,
                        (TEXT("%s: Invalid DC event type.  For Ch's 8 and 9, must be either new chan, new addr, or new data.\r\n"), __WFUNCTION__));
            return;
            }
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC Channel.  Must be Ch8 or Ch9 for MCU microcode event.\r\n"), __WFUNCTION__));
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DCConfigureDataMapping
//
// This function writes data into the DC Mapping Configuration registers
// to configure the offset and mask for the 3 byte components of the IPU
// display output.
//
// Parameters:
//      dwPointerNum
//          [in] Pointer number index
//
//      mappingData
//          [in] Info to program bus mapping data into DC registers.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL DCConfigureDataMapping(DWORD dwPointerNum, PBusMappingData mappingData)
{
    DWORD dwMaskOffsetPtr2, dwMaskOffsetPtr1, dwMaskOffsetPtr0;
    DWORD dwMappingPtrRegisterNum, dwMaskOffsetRegisterRegisterNum;

    if (dwPointerNum > 29)
    {
        RETAILMSG (1, (TEXT("%s: Pointer number cannot be greater than 29.  Aborting data mapping configuration...\r\n"), __WFUNCTION__));
        return FALSE;
    }

    dwMaskOffsetPtr2 = DCGetAvailableMaskOffsetPointer();
    dwMaskOffsetPtr1 = DCGetAvailableMaskOffsetPointer();
    dwMaskOffsetPtr0 = DCGetAvailableMaskOffsetPointer();

    if ((dwMaskOffsetPtr2 == 50) || (dwMaskOffsetPtr1 == 50) || (dwMaskOffsetPtr0 == 50))
    {
        RETAILMSG (1, (TEXT("%s: Failed to find available Mask/Offset Pointer values!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    dwMappingPtrRegisterNum = (dwPointerNum/2);

    // Configure pointers to the mask and offset values for bytes 2, 1, and 0.
    if ((dwPointerNum % 2) == 0)
    {
        // Write lower part of register
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_0 + dwMappingPtrRegisterNum), IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE2_0, dwMaskOffsetPtr2);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_0 + dwMappingPtrRegisterNum), IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE1_0, dwMaskOffsetPtr1);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_0 + dwMappingPtrRegisterNum), IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE0_0, dwMaskOffsetPtr0);
    }
    else
    {
        // Write higher part of register
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_0 + dwMappingPtrRegisterNum), IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE2_1, dwMaskOffsetPtr2);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_0 + dwMappingPtrRegisterNum), IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE1_1, dwMaskOffsetPtr1);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_0 + dwMappingPtrRegisterNum), IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE0_1, dwMaskOffsetPtr0);
    }

    dwMaskOffsetRegisterRegisterNum = (dwMaskOffsetPtr2/2);

    // Configure mask and offset values for Byte 2.
    if ((dwMaskOffsetPtr2 % 2) == 0)
    {
        // Write lower part of register
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_MASK_0, mappingData->dwComponent2Mask);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_OFFSET_0, mappingData->dwComponent2Offset);
    }
    else
    {
        // Write higher part of register
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_MASK_1, mappingData->dwComponent2Mask);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_OFFSET_1, mappingData->dwComponent2Offset);
    }

    dwMaskOffsetRegisterRegisterNum = (dwMaskOffsetPtr1/2);

    // Configure mask and offset values for Byte 1.
    if ((dwMaskOffsetPtr1 % 2) == 0)
    {
        // Write lower part of register
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_MASK_0, mappingData->dwComponent1Mask);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_OFFSET_0, mappingData->dwComponent1Offset);
    }
    else
    {
        // Write higher part of register
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_MASK_1, mappingData->dwComponent1Mask);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_OFFSET_1, mappingData->dwComponent1Offset);
    }

    dwMaskOffsetRegisterRegisterNum = (dwMaskOffsetPtr0/2);

    // Configure mask and offset values for Byte 0.
    if ((dwMaskOffsetPtr0 % 2) == 0)
    {
        // Write lower part of register
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_MASK_0, mappingData->dwComponent0Mask);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_OFFSET_0, mappingData->dwComponent0Offset);
    }
    else
    {
        // Write higher part of register
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_MASK_1, mappingData->dwComponent0Mask);
        INSREG32BF((&g_pIPUV3_DC->DC_MAP_CONF_15 + dwMaskOffsetRegisterRegisterNum), IPU_DC_MAP_CONF_MD_OFFSET_1, mappingData->dwComponent0Offset);
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: DCGetStatus
//
// Returns DC_STAT status data.
//
// Parameters:
//      None.
//
// Returns:
//      DC_STAT register value.
//------------------------------------------------------------------------------
DWORD DCGetStatus()
{
    return INREG32(&g_pIPUV3_DC->DC_STAT);
}

//------------------------------------------------------------------------------
//
// Function: DCLLA
//
// Configures the Low Level access registers.
//
// Parameters:
//      pLLAData
//          [in] Pointer to LLA configuration data structure.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DCLLA(PLLAConfigData pLLAData)
{
    switch (pLLAData->dcChan)
    {
        case DC_CHANNEL_8:
            switch (pLLAData->dwRSNum)
            {
                case 0:
                    INSREG32BF(&g_pIPUV3_DC->DC_R_LLA0,
                        IPU_DC_LLA0_MCU_RS_0_0, pLLAData->dwWord);
                    break;
                case 1:
                    INSREG32BF(&g_pIPUV3_DC->DC_R_LLA0,
                        IPU_DC_LLA0_MCU_RS_1_0, pLLAData->dwWord);
                    break;
                case 2:
                    INSREG32BF(&g_pIPUV3_DC->DC_R_LLA0,
                        IPU_DC_LLA0_MCU_RS_2_0, pLLAData->dwWord);
                    break;
                case 3:
                    INSREG32BF(&g_pIPUV3_DC->DC_R_LLA0,
                        IPU_DC_LLA0_MCU_RS_3_0, pLLAData->dwWord);
                    break;
                default:
                    DEBUGMSG (DC_ERROR,
                        (TEXT("%s: Invalid RS value.  Valid range = {0-3}\r\n"), __WFUNCTION__));
                    break;
            }
            break;
        case DC_CHANNEL_9:
            switch (pLLAData->dwRSNum)
            {
                case 0:
                    INSREG32BF(&g_pIPUV3_DC->DC_R_LLA1,
                        IPU_DC_LLA1_MCU_RS_0_1, pLLAData->dwWord);
                    break;
                case 1:
                    INSREG32BF(&g_pIPUV3_DC->DC_R_LLA1,
                        IPU_DC_LLA1_MCU_RS_1_1, pLLAData->dwWord);
                    break;
                case 2:
                    INSREG32BF(&g_pIPUV3_DC->DC_R_LLA1,
                        IPU_DC_LLA1_MCU_RS_2_1, pLLAData->dwWord);
                    break;
                case 3:
                    INSREG32BF(&g_pIPUV3_DC->DC_R_LLA1,
                        IPU_DC_LLA1_MCU_RS_3_1, pLLAData->dwWord);
                    break;
                default:
                    DEBUGMSG (DC_ERROR,
                        (TEXT("%s: Invalid RS value.  Valid range = {0-3}\r\n"), __WFUNCTION__));
                    break;
            }
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: Invalid DC Channel.  Must be CH8 or CH9.\r\n"), __WFUNCTION__));
            break;
    }
}

//------------------------------------------------------------------------------
//
// Function: DCIDMACtoDCChannel
//
// Converts an IDMAC channel to a DC channel.
//
// Parameters:
//      dwIDMACChannel
//          [in] IDMAC Channel to convert to a DC channel.
//
// Returns:
//      DC_CHANNEL corresponding to IDMAC channel passed in.
//      DC_CHANNEL_INVALID if failure.
//------------------------------------------------------------------------------
DC_CHANNEL DCIDMACtoDCChannel(DWORD dwIDMACChannel)
{
    DC_CHANNEL dcChan;
    switch(dwIDMACChannel)
    {
        case IDMAC_CH_21:
            dcChan = DC_CHANNEL_5;
            break;
        case IDMAC_CH_24:
            dcChan = DC_CHANNEL_6;
            break;
        case IDMAC_CH_27:
            dcChan = DC_CHANNEL_5;
            break;
        case IDMAC_CH_28:
            dcChan = DC_CHANNEL_1;
            break;
        case IDMAC_CH_29:
            dcChan = DC_CHANNEL_6;
            break;
        case IDMAC_CH_40:
            dcChan = DC_CHANNEL_0;
            break;
        case IDMAC_CH_41:
            dcChan = DC_CHANNEL_2;
            break;
        default:
            DEBUGMSG (DC_ERROR,
                (TEXT("%s: IDMAC channel parameter does not correlate to any DC Channel\r\n"), __WFUNCTION__));
            dcChan = DC_CHANNEL_INVALID;
            break;
    }

    return dcChan;
}

//------------------------------------------------------------------------------
//
// Function: DCGetAvailableMaskOffsetPointer
//
// Traverses array to find an available register for mask/offset pointer info
//
// Parameters:
//      None.
//
// Returns:
//      Available mask/offset pointer val.  50 if none available.
//------------------------------------------------------------------------------
static DWORD DCGetAvailableMaskOffsetPointer()
{
    int i;

    for (i = 0; i < 24; i++)
    {
        if (bMaskOffsetInUse_Array[i] == FALSE)
        {
            bMaskOffsetInUse_Array[i] = TRUE;
            return i;
        }
    }

    return 50;
}


//------------------------------------------------------------------------------
//
// Function: DCWait4TripleBufEmpty
//
// Wait for corresponding DC channel triple buffer counter empty.
//
// Parameters:
//      di_sel
//          [in] di channel number
// Returns:
//      None.
//------------------------------------------------------------------------------
void DCWait4TripleBufEmpty(DI_SELECT di_sel)
{
    if(di_sel == DI_SELECT_DI0)
        while(!EXTREG32BF(&g_pIPUV3_DC->DC_STAT,IPU_DC_STAT_DC_TRIPLE_BUF_CNT_EMPTY_0)){}
    else
        while(!EXTREG32BF(&g_pIPUV3_DC->DC_STAT,IPU_DC_STAT_DC_TRIPLE_BUF_CNT_EMPTY_1)){}
    CalibrateStallCounter();
    StallExecution(50);

    return;
}


void DCDumpRegs()
{
    int i;
    UINT32* ptr = (UINT32 *)&g_pIPUV3_DC->DC_READ_CH_CONF;

    RETAILMSG (1, (TEXT("\n\nDC Registers\n")));
    for (i = 0; i <= (sizeof(CSP_IPU_DC_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}


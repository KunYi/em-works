//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  di.cpp
//
//  IPU CSP-level Display Interface functions
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
#include "di.h"


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
PCSP_IPU_DI0_REGS g_pIPUV3_DI0;
PCSP_IPU_DI1_REGS g_pIPUV3_DI1;
static PCSP_IPU_COMMON_REGS g_pIPUV3_COMMON;
UINT32 g_dwHSPCLKFreq = 0;
UINT32 g_dwDICLKFreq = 0;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: DIRegsInit
//
// This function allocates the data structures required for interaction
// with the IPUv3 DI hardware.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL DIRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if ((g_pIPUV3_DI0 == NULL) || (g_pIPUV3_DI1 == NULL))
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
        // Create pointer to DI0 registers
        //********************************************
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_DI0_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_DI0 = (PCSP_IPU_DI0_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_DI0_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_DI0 == NULL)
        {
            RETAILMSG(1,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

        //********************************************
        // Create pointer to DI1 registers
        //********************************************
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_DI1_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_DI1 = (PCSP_IPU_DI1_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_DI1_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_DI1 == NULL)
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
    // If initialization failed, be sure to clean up
    if (!rc) DIRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  DIRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 DI hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void DIRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    if (!IPUV3BaseCloseHandle(g_hIPUBase))
    {
        RETAILMSG(1,
            (_T("DI Cleanup: Failed closing IPU Base handle!\r\n")));
    }

    // Unmap peripheral registers
    if (g_pIPUV3_COMMON)
    {
        MmUnmapIoSpace(g_pIPUV3_COMMON, sizeof(CSP_IPU_COMMON_REGS));
        g_pIPUV3_COMMON = NULL;
    }

    // Unmap peripheral registers
    if (g_pIPUV3_DI0)
    {
        MmUnmapIoSpace(g_pIPUV3_DI0, sizeof(PCSP_IPU_DI0_REGS));
        g_pIPUV3_DI0 = NULL;
    }

    // Unmap peripheral registers
    if (g_pIPUV3_DI1)
    {
        MmUnmapIoSpace(g_pIPUV3_DI1, sizeof(PCSP_IPU_DI1_REGS));
        g_pIPUV3_DI1 = NULL;
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DIEnable
//
// This function enables DI for normal opertion
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to enable
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIEnable(DI_SELECT di_sel)
{
    IPU_SUBMODULE di_module = (di_sel == DI_SELECT_DI0) ? IPU_SUBMODULE_DI0 : IPU_SUBMODULE_DI1;

    IPU_FUNCTION_ENTRY();

    // Enable DI

    // Call to IPU Base to turn on DI_EN (for either DI0 or DI1) in IPU_CONF reg.
    // This will also turn on IPU clocks if no other IPU
    // modules have already turned them on.
    if (!IPUV3EnableModule(g_hIPUBase, di_module, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: Failed to enable DI!\r\n"), __WFUNCTION__));
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: DIDisable
//
// This function disables DI
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to disable
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIDisable(DI_SELECT di_sel)
{
    IPU_SUBMODULE di_module = (di_sel == DI_SELECT_DI0) ? IPU_SUBMODULE_DI0 : IPU_SUBMODULE_DI1;

    IPU_FUNCTION_ENTRY();

    // Disable DI

    // Call to IPU Base to turn off DI_EN (for either DI0 or DI1) in IPU_CONF reg.
    if (!IPUV3DisableModule(g_hIPUBase, di_module, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: Failed to disable DI!\r\n"), __WFUNCTION__));
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: DISetIPUClkFreq
//
// This function sets the IPU clock frequency, which will be used
// during DI initialization to determine timing parameters for the DI.
//
// Parameters:
//      dwIPUClkFreq
//          [in] IPU HSP clock frequency, in Hz
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DISetIPUClkFreq(UINT32 dwIPUClkFreq)
{
    g_dwHSPCLKFreq = dwIPUClkFreq;
}

//------------------------------------------------------------------------------
//
// Function: DISetDIClkFreq
//
// This function sets the DI clock frequency by configuring the divider
// in the IPU_PM register,  Before this function is called, user MUST first
// call DISetIPUClkFreq() to configure the IPU clock frequency.  That frequency
// will be used in computing the DI clock divider value.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      dwDIClkFreq
//          [in] Desired DI clock frequency, in Hz
//
// Returns:
//      none.
//------------------------------------------------------------------------------
BOOL DISetDIClkFreq(DI_SELECT di_sel, UINT32 dwDIClkFreq)
{
    DWORD dwDIDivider;

    // if HSP CLK is 0, it hasn't been configured, so we can't configure DI clk.
    if (g_dwHSPCLKFreq == 0)
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: IPU clock frequency has not yet been configured.  Configure this before calling DISetDIClkFreq!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Desired DI clk must be less than HSP clock
    if (dwDIClkFreq > g_dwHSPCLKFreq)
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: DI clock frequency must be less than IPU clock frequency - 0x%x Hz.\r\n"), __WFUNCTION__, g_dwHSPCLKFreq));
        return FALSE;
    }

    g_dwDICLKFreq = dwDIClkFreq;

    // Calculate divider value (shift by 4 because bits 3:0 of divider are fraction part)
    dwDIDivider = (g_dwHSPCLKFreq << 4) / g_dwDICLKFreq;

    if (di_sel == DI_SELECT_DI0)
    {
        INSREG32BF(&g_pIPUV3_COMMON->IPU_PM, IPU_IPU_PM_DI0_CLK_PERIOD_0, dwDIDivider);
        INSREG32BF(&g_pIPUV3_COMMON->IPU_PM, IPU_IPU_PM_DI0_CLK_PERIOD_1, dwDIDivider);
    }
    else
    {
        INSREG32BF(&g_pIPUV3_COMMON->IPU_PM, IPU_IPU_PM_DI1_CLK_PERIOD_0, dwDIDivider);
        INSREG32BF(&g_pIPUV3_COMMON->IPU_PM, IPU_IPU_PM_DI1_CLK_PERIOD_1, dwDIDivider);
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: DIConfigureGeneral
//
// This function sets the DI0 or DI1 General configuration register.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      pGenConfData
//          [in] General configuration register data
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIConfigureGeneral(DI_SELECT di_sel, PDIGeneralConfigData pGenConfData)
{
    UINT32 regVal = CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_1, pGenConfData->dwPinPolarity[0]) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_2, pGenConfData->dwPinPolarity[1]) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_3, pGenConfData->dwPinPolarity[2]) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_4, pGenConfData->dwPinPolarity[3]) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_5, pGenConfData->dwPinPolarity[4]) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_6, pGenConfData->dwPinPolarity[5]) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_7, pGenConfData->dwPinPolarity[6]) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_8, pGenConfData->dwPinPolarity[7]) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_CS0, pGenConfData->dwCS0Polarity) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_CS1, pGenConfData->dwCS1Polarity) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_ERM_VSYNC_SEL, pGenConfData->dwERMVSyncSelect) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_ERR_TREATMENT, pGenConfData->dwSyncFlowErrorTreatment) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_SYNC_COUNT_SEL, pGenConfData->dwSyncFlowCounterSelect) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_POLARITY_DISP_CLK, pGenConfData->dwClkPolarity) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_WATCHDOG_MODE, pGenConfData->dwWatchdogMode) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_CLK_EXT, pGenConfData->dwClkSource) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_VSYNC_EXT, pGenConfData->dwVSyncSource) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_MASK_SEL, pGenConfData->dwMaskSelect) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_DISP_CLOCK_INIT, pGenConfData->dwDisplayClockInitMode) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_CLOCK_STOP_MODE, pGenConfData->dwClockStopMode) |
            CSP_BITFVAL(IPU_DI_GENERAL_DI_DISP_Y_SEL, pGenConfData->dwLineCounterSelect);

    if (di_sel == DI_SELECT_DI0)
    {
        OUTREG32(&g_pIPUV3_DI0->DI0_GENERAL, regVal);
    }
    else
    {
        OUTREG32(&g_pIPUV3_DI1->DI1_GENERAL, regVal);
    }
}


//------------------------------------------------------------------------------
//
// Function: DIConfigureBaseSyncClockGen
//
// This function sets the DI0 or DI1 Base Sync Clock Gen registers.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      dwClkOffset
//          [in] Sync clock offset, in ns.
//
//      dwClkFreq
//          [in] Sync clock frequency, in Hz.
//
//      dwUpPos
//          [in] Sync clock rising edge position, in ns.
//
//      dwDownPos
//          [in] Sync clock falling edge position, in ns.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIConfigureBaseSyncClockGen(DI_SELECT di_sel, UINT32 dwClkOffset,
    UINT32 dwClkFreq, UINT32 dwUpPos, UINT32 dwDownPos)
{
    // Variables to store encoded clock timing values
    // (values to write into registers)
    UINT32 dwDIClkDivider, dwOffsetClocks, dwEncodedUpPos, dwEncodedDownPos;

    // Divider has a 4-bit (3:0) fractional part
    dwDIClkDivider = (DWORD)((LONGLONG)g_dwHSPCLKFreq * 16 / dwClkFreq);

    // The divider value must have an integer part (can't be just 
    // fractional).  Set to 1 if the value is only fractional.
    if (dwDIClkDivider < 0x10)
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: DI clock divider must have an integer part. Setting divider to 1.\r\n"), __WFUNCTION__));
        dwDIClkDivider = 0x10;
    }

    // 1000000000 (billion) / HSP CLK = HSP CLK period in ns
    // desired ns delay / HSP CLK period in ns = offset value
    // offset value = dwClkOffset / (1000000000/g_dwHSPCLKFreq)
    // offset value = dwClkOffset * g_dwHSPCLKFreq / 1000000000
    dwOffsetClocks = (DWORD)((LONGLONG)dwClkOffset * g_dwHSPCLKFreq / 1000000000);

    // Up and Down edge position values have units of HSP CLK periods
    // Multiplied by 2 because there is one fractional bit in the
    // register field.
    dwEncodedUpPos = (DWORD)((LONGLONG)dwUpPos * g_dwHSPCLKFreq * 2 / 1000000000);
    dwEncodedDownPos = (DWORD)((LONGLONG)dwDownPos * g_dwHSPCLKFreq * 2 / 1000000000);

    if (di_sel == DI_SELECT_DI0)
    {
        INSREG32BF(&g_pIPUV3_DI0->DI0_BS_CLKGEN0,
            IPU_DI_BS_CLKGEN0_DI_DISP_CLK_PERIOD, dwDIClkDivider);
        INSREG32BF(&g_pIPUV3_DI0->DI0_BS_CLKGEN0,
            IPU_DI_BS_CLKGEN0_DI_DISP_CLK_OFFSET, dwOffsetClocks);

        INSREG32BF(&g_pIPUV3_DI0->DI0_BS_CLKGEN1,
            IPU_DI_BS_CLKGEN1_DI_DISP_CLK_UP, dwEncodedUpPos);
        INSREG32BF(&g_pIPUV3_DI0->DI0_BS_CLKGEN1,
            IPU_DI_BS_CLKGEN1_DI_DISP_CLK_DOWN, dwEncodedDownPos);
    }
    else
    {
        INSREG32BF(&g_pIPUV3_DI1->DI1_BS_CLKGEN0,
            IPU_DI_BS_CLKGEN0_DI_DISP_CLK_PERIOD, dwDIClkDivider);
        INSREG32BF(&g_pIPUV3_DI1->DI1_BS_CLKGEN0,
            IPU_DI_BS_CLKGEN0_DI_DISP_CLK_OFFSET, dwOffsetClocks);

        INSREG32BF(&g_pIPUV3_DI1->DI1_BS_CLKGEN1,
            IPU_DI_BS_CLKGEN1_DI_DISP_CLK_UP, dwEncodedUpPos);
        INSREG32BF(&g_pIPUV3_DI1->DI1_BS_CLKGEN1,
            IPU_DI_BS_CLKGEN1_DI_DISP_CLK_DOWN, dwEncodedDownPos);
    }
}


//------------------------------------------------------------------------------
//
// Function: DIConfigurePointer
//
// This function configures the DI0/DI1 waveform generators (parallel mode).
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      ptrConfigData
//          [in] Pointer to pointer configuration data structure
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIConfigurePointer(DI_SELECT di_sel, PDIPointerConfigData ptrConfigData)
{
    UINT32 regVal;
    UINT32 dwAccessSize, dwComponentSize;

    // Check validity of pointer value; valid range = {0-11}
    if ((ptrConfigData->dwPointer < 0) || (ptrConfigData->dwPointer > 11))
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: Invalid ptrConfigData->dwPointer value.  Valid range = {0-11}.\r\n"), __WFUNCTION__));
    }

    // Convert from ns to DI Clk cycles
    dwAccessSize = (DWORD)((LONGLONG)ptrConfigData->dwAccessPeriod * g_dwDICLKFreq / 1000000000 - 1);
    dwComponentSize = (DWORD)((LONGLONG)ptrConfigData->dwComponentPeriod * g_dwDICLKFreq / 1000000000 - 1);

    regVal = CSP_BITFVAL(IPU_DI_DW_GEN_DI_ACCESS_SIZE, dwAccessSize) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_COMPONENT_SIZE, dwComponentSize) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_CST, ptrConfigData->dwCSPtr) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_PT_0, ptrConfigData->dwPinPtr[0]) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_PT_1, ptrConfigData->dwPinPtr[1]) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_PT_2, ptrConfigData->dwPinPtr[2]) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_PT_3, ptrConfigData->dwPinPtr[3]) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_PT_4, ptrConfigData->dwPinPtr[4]) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_PT_5, ptrConfigData->dwPinPtr[5]) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_PT_6, ptrConfigData->dwPinPtr[6]);

    if (di_sel == DI_SELECT_DI0)
    {
        OUTREG32(&g_pIPUV3_DI0->DI0_DW_GEN[ptrConfigData->dwPointer], regVal);
    }
    else
    {
        OUTREG32(&g_pIPUV3_DI1->DI1_DW_GEN[ptrConfigData->dwPointer], regVal);
    }
}


//------------------------------------------------------------------------------
//
// Function: DIConfigureSerialPointer
//
// This function configures the DI0/DI1 waveform generators (serial mode).
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      ptrConfigSerData
//          [in] Pointer to pointer configuration data structure
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIConfigureSerialPointer(DI_SELECT di_sel, PDIPointerConfigSerialData ptrConfigSerData)
{
    UINT32 regVal;
    UINT32 dwStrtPeriod, dwSerPeriod;

    // Check validity of pointer value; valid range = {0-11}
    if ((ptrConfigSerData->dwPointer < 0) || (ptrConfigSerData->dwPointer > 11))
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: Invalid ptrConfigData->dwPointer value.  Valid range = {0-11}.\r\n"), __WFUNCTION__));
    }

    // Convert from ns to DI Clk cycles
    dwStrtPeriod = (DWORD)((LONGLONG)ptrConfigSerData->dwStartPeriod * g_dwDICLKFreq / 1000000000);
    dwSerPeriod = (DWORD)((LONGLONG)ptrConfigSerData->dwSerialPeriod * g_dwDICLKFreq / 1000000000);

    regVal = CSP_BITFVAL(IPU_DI_DW_GEN_DI_SERIAL_PERIOD, dwSerPeriod) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_START_PERIOD, dwStrtPeriod) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_CST, ptrConfigSerData->dwCSPtr) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_SERIAL_VALID_BITS, ptrConfigSerData->dwSerialValidBits) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_SERIAL_RS, ptrConfigSerData->dwSerialRSPtr) |
            CSP_BITFVAL(IPU_DI_DW_GEN_DI_SERIAL_CLK, ptrConfigSerData->dwSerialClkPtr);

    if (di_sel == DI_SELECT_DI0)
    {
        OUTREG32(&g_pIPUV3_DI0->DI0_DW_GEN[ptrConfigSerData->dwPointer], regVal);
    }
    else
    {
        OUTREG32(&g_pIPUV3_DI1->DI1_DW_GEN[ptrConfigSerData->dwPointer], regVal);
    }
}

    
//------------------------------------------------------------------------------
//
// Function: DIConfigureUpDown
//
// This function configures the DI0/DI1 up/down position.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      pUpDownConfigData
//          [in] Pointer to up/down configuration data structure
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIConfigureUpDown(DI_SELECT di_sel, PDIUpDownConfigData pUpDownConfigData)
{
    // Variables to hold up and down position values after converting
    // from ns to # of IPU clocks (for writing to register)
    UINT32 dwUpPosClocks, dwDownPosClocks;

    // Check validity of pointer value; valid range = {0-11}
    if ((pUpDownConfigData->dwPointer < 0) || (pUpDownConfigData->dwPointer > 11))
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: Invalid pUpDownConfigData->dwPointer value.  Valid range = {0-11}.\r\n"), __WFUNCTION__));
    }

    // Convert from ns to DI Clk cycles
    dwUpPosClocks = (DWORD)((LONGLONG)pUpDownConfigData->dwUpPos * g_dwDICLKFreq / 1000000000);
    dwDownPosClocks = (DWORD)((LONGLONG)pUpDownConfigData->dwDownPos * g_dwDICLKFreq / 1000000000);

    if (di_sel == DI_SELECT_DI0)
    {
        switch (pUpDownConfigData->dwSet)
        {
            case 0:
                OUTREG32(&g_pIPUV3_DI0->DI0_DW_SET0[pUpDownConfigData->dwPointer],
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_UP, dwUpPosClocks) |
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_DOWN, dwDownPosClocks));
                break;
            case 1:
                OUTREG32(&g_pIPUV3_DI0->DI0_DW_SET1[pUpDownConfigData->dwPointer],
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_UP, dwUpPosClocks) |
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_DOWN, dwDownPosClocks));
                break;
            case 2:
                OUTREG32(&g_pIPUV3_DI0->DI0_DW_SET2[pUpDownConfigData->dwPointer],
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_UP, dwUpPosClocks) |
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_DOWN, dwDownPosClocks));
                break;
            case 3:
                OUTREG32(&g_pIPUV3_DI0->DI0_DW_SET3[pUpDownConfigData->dwPointer],
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_UP, dwUpPosClocks) |
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_DOWN, dwDownPosClocks));
                break;
        }
    }
    else
    {
        switch (pUpDownConfigData->dwSet)
        {
            case 0:
                OUTREG32(&g_pIPUV3_DI1->DI1_DW_SET0[pUpDownConfigData->dwPointer],
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_UP, dwUpPosClocks) |
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_DOWN, dwDownPosClocks));
                break;
            case 1:
                OUTREG32(&g_pIPUV3_DI1->DI1_DW_SET1[pUpDownConfigData->dwPointer],
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_UP, dwUpPosClocks) |
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_DOWN, dwDownPosClocks));
                break;
            case 2:
                OUTREG32(&g_pIPUV3_DI1->DI1_DW_SET2[pUpDownConfigData->dwPointer],
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_UP, dwUpPosClocks) |
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_DOWN, dwDownPosClocks));
                break;
            case 3:
                OUTREG32(&g_pIPUV3_DI1->DI1_DW_SET3[pUpDownConfigData->dwPointer],
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_UP, dwUpPosClocks) |
                    CSP_BITFVAL(IPU_DI_DW_SET_DI_DATA_CNT_DOWN, dwDownPosClocks));
                break;
        }
    }
}

//------------------------------------------------------------------------------
//
// Function: DIConfigureSync
//
// This function starts the DI0/DI1 counters running.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      pSyncConfigData
//          [in] Pointer to sync configuration data structure
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIConfigureSync(DI_SELECT di_sel, PDISyncConfigData pSyncConfigData)
{
    // Variables to hold up and down position values after converting
    // from ns to # of IPU clocks (for writing to register)
    UINT32 dwUpPosClocks, dwDownPosClocks;

    UINT32 reg0Val, reg1Val;

    // Check validity of pointer value; valid range = {1-9}
    if ((pSyncConfigData->dwPointer < 1) || (pSyncConfigData->dwPointer > 9))
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: Invalid pUpDownConfigData->dwPointer value.  Valid range = {1-9}.\r\n"), __WFUNCTION__));
    }

    //The unit is pixel clock
    dwUpPosClocks = pSyncConfigData->dwUpPos<<1;
    dwDownPosClocks = pSyncConfigData->dwDownPos<<1;

    reg0Val = CSP_BITFVAL(IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION, pSyncConfigData->dwOffsetResolution) |
            CSP_BITFVAL(IPU_DI_SW_GEN0_DI_OFFSET_VALUE, pSyncConfigData->dwOffsetValue) |
            CSP_BITFVAL(IPU_DI_SW_GEN0_DI_RUN_RESOLUTION, pSyncConfigData->dwRunResolution) |
            CSP_BITFVAL(IPU_DI_SW_GEN0_DI_RUN_VALUE_M1, pSyncConfigData->dwRunValue);

    reg1Val = CSP_BITFVAL(IPU_DI_SW_GEN1_DI_CNT_UP, dwUpPosClocks) |
            CSP_BITFVAL(IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL, pSyncConfigData->dwPolarityClearSelect) |
            CSP_BITFVAL(IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL, pSyncConfigData->dwToggleTriggerSelect) |
            CSP_BITFVAL(IPU_DI_SW_GEN1_DI_CNT_DOWN, dwDownPosClocks) |
            CSP_BITFVAL(IPU_DI_SW_GEN1_DI_CNT_CLR_SEL, pSyncConfigData->dwCounterClearSelect) |
            CSP_BITFVAL(IPU_DI_SW_GEN1_DI_CNT_AUTO_RELOAD, pSyncConfigData->bCounterAutoReload);
    
    if(pSyncConfigData->dwPointer!=9)
    {
        reg1Val |=CSP_BITFVAL(IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN, pSyncConfigData->dwPolarityGeneratorEnable);
    }
    else
    {
        reg1Val |=CSP_BITFVAL(IPU_DI_SW_GEN1_DI_GENTIME_SEL, pSyncConfigData->dwGentimeSelect);
    }     

    if (di_sel == DI_SELECT_DI0)
    {
        switch (pSyncConfigData->dwPointer)
        {
            case 1:
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN0_1, reg0Val);
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN1_1, reg1Val);
                INSREG32BF(&g_pIPUV3_DI0->DI0_STP_REP[0], IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat ); 
                break;
            case 2:
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN0_2, reg0Val);
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN1_2, reg1Val);
                INSREG32BF(&g_pIPUV3_DI0->DI0_STP_REP[0], IPU_DI_STP_REP_STEP_REPEAT_H,
                          pSyncConfigData->dwStepRepeat ); 
                break;
            case 3:
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN0_3, reg0Val);
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN1_3, reg1Val);
                INSREG32BF(&g_pIPUV3_DI0->DI0_STP_REP[1], IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat ); 
                break;
            case 4:
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN0_4, reg0Val);
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN1_4, reg1Val);
                INSREG32BF(&g_pIPUV3_DI0->DI0_STP_REP[1], IPU_DI_STP_REP_STEP_REPEAT_H,
                          pSyncConfigData->dwStepRepeat ); 
                break;
            case 5:
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN0_5, reg0Val);
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN1_5, reg1Val);
                INSREG32BF(&g_pIPUV3_DI0->DI0_STP_REP[2], IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat ); 
                break;
            case 6:
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN0_6, reg0Val);
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN1_6, reg1Val);
                INSREG32BF(&g_pIPUV3_DI0->DI0_STP_REP[2], IPU_DI_STP_REP_STEP_REPEAT_H,
                          pSyncConfigData->dwStepRepeat ); 
                break;
            case 7:
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN0_7, reg0Val);
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN1_7, reg1Val);
                INSREG32BF(&g_pIPUV3_DI0->DI0_STP_REP[3], IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat ); 
                break;
            case 8:
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN0_8, reg0Val);
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN1_8, reg1Val);
                INSREG32BF(&g_pIPUV3_DI0->DI0_STP_REP[3], IPU_DI_STP_REP_STEP_REPEAT_H,
                          pSyncConfigData->dwStepRepeat ); 
                break;
            case 9:
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN0_9, reg0Val);
                OUTREG32(&g_pIPUV3_DI0->DI0_SW_GEN1_9, reg1Val);
                INSREG32BF(&g_pIPUV3_DI0->DI0_STP_REP_9, IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat );
                break;
            default:
                DEBUGMSG (DI_ERROR,
                    (TEXT("%s: Invalid pointer.\r\n"), __WFUNCTION__));
                break;
        }
    }
    else
    {
        switch (pSyncConfigData->dwPointer)
        {
            case 1:
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN0_1, reg0Val);
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN1_1, reg1Val);
                INSREG32BF(&g_pIPUV3_DI1->DI1_STP_REP[0], IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat ); 
                break;
            case 2:
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN0_2, reg0Val);
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN1_2, reg1Val);
                INSREG32BF(&g_pIPUV3_DI1->DI1_STP_REP[0], IPU_DI_STP_REP_STEP_REPEAT_H,
                          pSyncConfigData->dwStepRepeat );
                break;
            case 3:
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN0_3, reg0Val);
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN1_3, reg1Val);
                INSREG32BF(&g_pIPUV3_DI1->DI1_STP_REP[1], IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat );
                break;
            case 4:
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN0_4, reg0Val);
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN1_4, reg1Val);
                INSREG32BF(&g_pIPUV3_DI1->DI1_STP_REP[1], IPU_DI_STP_REP_STEP_REPEAT_H,
                          pSyncConfigData->dwStepRepeat );
                break;
            case 5:
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN0_5, reg0Val);
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN1_5, reg1Val);
                INSREG32BF(&g_pIPUV3_DI1->DI1_STP_REP[2],IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat );
                break;
            case 6:
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN0_6, reg0Val);
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN1_6, reg1Val);
                INSREG32BF(&g_pIPUV3_DI1->DI1_STP_REP[2], IPU_DI_STP_REP_STEP_REPEAT_H,
                          pSyncConfigData->dwStepRepeat );
                break;
            case 7:
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN0_7, reg0Val);
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN1_7, reg1Val);
                INSREG32BF(&g_pIPUV3_DI1->DI1_STP_REP[3], IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat );
                break;
            case 8:
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN0_8, reg0Val);
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN1_8, reg1Val);
                INSREG32BF(&g_pIPUV3_DI1->DI1_STP_REP[3], IPU_DI_STP_REP_STEP_REPEAT_H,
                          pSyncConfigData->dwStepRepeat );
                break;
            case 9:
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN0_9, reg0Val);
                OUTREG32(&g_pIPUV3_DI1->DI1_SW_GEN1_9, reg1Val);
                INSREG32BF(&g_pIPUV3_DI1->DI1_STP_REP_9, IPU_DI_STP_REP_STEP_REPEAT_L,
                          pSyncConfigData->dwStepRepeat );
                
                break;
            default:
                DEBUGMSG (DI_ERROR,
                    (TEXT("%s: Invalid pointer.\r\n"), __WFUNCTION__));
                break;
        }
    }
}


//------------------------------------------------------------------------------
//
// Function: DIConfigurePolarity
//
// This function sets the polarity for the DI signals,
// by configuring the DI0/DI1 Polarity Register.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      pPolConfigData
//          [in] Pointer to polarity configuration data structure
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIConfigurePolarity(DI_SELECT di_sel, PDIPolarityConfigData pPolConfigData)
{
    if (di_sel == DI_SELECT_DI0)
    {
        switch (pPolConfigData->PolaritySetSelect)
        {
            case DI_POLARITY_SET_DRDY:
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_DRDY_POLARITY_11, pPolConfigData->PinPolarity[0]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_DRDY_POLARITY_12, pPolConfigData->PinPolarity[1]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_DRDY_POLARITY_13, pPolConfigData->PinPolarity[2]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_DRDY_POLARITY_14, pPolConfigData->PinPolarity[3]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_DRDY_POLARITY_15, pPolConfigData->PinPolarity[4]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_DRDY_POLARITY_16, pPolConfigData->PinPolarity[5]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_DRDY_POLARITY_17, pPolConfigData->PinPolarity[6]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_DRDY_DATA_POLARITY, pPolConfigData->DataPolarity);
                break;
            case DI_POLARITY_SET_CS0:
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS0_POLARITY_11, pPolConfigData->PinPolarity[0]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS0_POLARITY_12, pPolConfigData->PinPolarity[1]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS0_POLARITY_13, pPolConfigData->PinPolarity[2]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS0_POLARITY_14, pPolConfigData->PinPolarity[3]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS0_POLARITY_15, pPolConfigData->PinPolarity[4]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS0_POLARITY_16, pPolConfigData->PinPolarity[5]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS0_POLARITY_17, pPolConfigData->PinPolarity[6]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS0_DATA_POLARITY, pPolConfigData->DataPolarity);
                break;
            case DI_POLARITY_SET_CS1:
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS1_POLARITY_11, pPolConfigData->PinPolarity[0]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS1_POLARITY_12, pPolConfigData->PinPolarity[1]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS1_POLARITY_13, pPolConfigData->PinPolarity[2]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS1_POLARITY_14, pPolConfigData->PinPolarity[3]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS1_POLARITY_15, pPolConfigData->PinPolarity[4]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS1_POLARITY_16, pPolConfigData->PinPolarity[5]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS1_POLARITY_17, pPolConfigData->PinPolarity[6]);
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS1_DATA_POLARITY, pPolConfigData->DataPolarity);
                break;
            case DI_POLARITY_SET_CS0_BYTE_ENABLE:
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS0_BYTE_EN_POLARITY, pPolConfigData->CS0ByteEnablePolarity);
                break;
            case DI_POLARITY_SET_CS1_BYTE_ENABLE:
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_CS1_BYTE_EN_POLARITY, pPolConfigData->CS1ByteEnablePolarity);
                break;
            case DI_POLARITY_SET_WAIT:
                INSREG32BF(&g_pIPUV3_DI0->DI0_POL,
                    IPU_DI_POL_WAIT_POLARITY, pPolConfigData->WaitPolarity);
                break;
            default:
                DEBUGMSG (DI_ERROR,
                    (TEXT("%s: Invalid polarity set.\r\n"), __WFUNCTION__));
                break;
        }
    }
    else
    {
        switch (pPolConfigData->PolaritySetSelect)
        {
            case DI_POLARITY_SET_DRDY:
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_DRDY_POLARITY_11, pPolConfigData->PinPolarity[0]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_DRDY_POLARITY_12, pPolConfigData->PinPolarity[1]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_DRDY_POLARITY_13, pPolConfigData->PinPolarity[2]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_DRDY_POLARITY_14, pPolConfigData->PinPolarity[3]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_DRDY_POLARITY_15, pPolConfigData->PinPolarity[4]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_DRDY_POLARITY_16, pPolConfigData->PinPolarity[5]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_DRDY_POLARITY_17, pPolConfigData->PinPolarity[6]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_DRDY_DATA_POLARITY, pPolConfigData->DataPolarity);
                break;
            case DI_POLARITY_SET_CS0:
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS0_POLARITY_11, pPolConfigData->PinPolarity[0]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS0_POLARITY_12, pPolConfigData->PinPolarity[1]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS0_POLARITY_13, pPolConfigData->PinPolarity[2]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS0_POLARITY_14, pPolConfigData->PinPolarity[3]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS0_POLARITY_15, pPolConfigData->PinPolarity[4]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS0_POLARITY_16, pPolConfigData->PinPolarity[5]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS0_POLARITY_17, pPolConfigData->PinPolarity[6]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS0_DATA_POLARITY, pPolConfigData->DataPolarity);
                break;
            case DI_POLARITY_SET_CS1:
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS1_POLARITY_11, pPolConfigData->PinPolarity[0]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS1_POLARITY_12, pPolConfigData->PinPolarity[1]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS1_POLARITY_13, pPolConfigData->PinPolarity[2]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS1_POLARITY_14, pPolConfigData->PinPolarity[3]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS1_POLARITY_15, pPolConfigData->PinPolarity[4]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS1_POLARITY_16, pPolConfigData->PinPolarity[5]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS1_POLARITY_17, pPolConfigData->PinPolarity[6]);
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS1_DATA_POLARITY, pPolConfigData->DataPolarity);
                break;
            case DI_POLARITY_SET_CS0_BYTE_ENABLE:
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS0_BYTE_EN_POLARITY, pPolConfigData->CS0ByteEnablePolarity);
                break;
            case DI_POLARITY_SET_CS1_BYTE_ENABLE:
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_CS1_BYTE_EN_POLARITY, pPolConfigData->CS1ByteEnablePolarity);
                break;
            case DI_POLARITY_SET_WAIT:
                INSREG32BF(&g_pIPUV3_DI1->DI1_POL,
                    IPU_DI_POL_WAIT_POLARITY, pPolConfigData->WaitPolarity);
                break;
            default:
                DEBUGMSG (DI_ERROR,
                    (TEXT("%s: Invalid polarity set.\r\n"), __WFUNCTION__));
                break;
        }
    }
}


//------------------------------------------------------------------------------
//
// Function: DIConfigureSerial
//
// This function configures settings for serial communication
// over DI0/DI1.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      pSerConfData
//          [in] Pointer to serial configuration data structure
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIConfigureSerial(DI_SELECT di_sel, PDISerialConfigData pSerConfData)
{
    UINT32 regVal = CSP_BITFVAL(IPU_DI_SER_CONF_WAIT4SERIAL, pSerConfData->dwWait4Serial) |
            CSP_BITFVAL(IPU_DI_SER_CONF_SERIAL_CS_POLARITY, pSerConfData->dwSerCSPol) |
            CSP_BITFVAL(IPU_DI_SER_CONF_SERIAL_RS_POLARITY, pSerConfData->dwSerRSPol) |
            CSP_BITFVAL(IPU_DI_SER_CONF_SERIAL_DATA_POLARITY, pSerConfData->dwSerDataPol) |
            CSP_BITFVAL(IPU_DI_SER_CONF_SER_CLK_POLARITY, pSerConfData->dwSerClkPol) |
            CSP_BITFVAL(IPU_DI_SER_CONF_LLA_SER_ACCESS, pSerConfData->dwDirectSerLLA) |
            CSP_BITFVAL(IPU_DI_SER_CONF_SERIAL_LATCH, pSerConfData->dwDI0SerLatchDelay) |
            CSP_BITFVAL(IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_W_0, pSerConfData->dwRS0WritePtr) |
            CSP_BITFVAL(IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_W_1, pSerConfData->dwRS1WritePtr) |
            CSP_BITFVAL(IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_R_0, pSerConfData->dwRS0ReadPtr) |
            CSP_BITFVAL(IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_R_1, pSerConfData->dwRS1ReadPtr);

    if (di_sel == DI_SELECT_DI0)
    {
        OUTREG32(&g_pIPUV3_DI0->DI0_SER_CONF, regVal);
    }
    else
    {
        OUTREG32(&g_pIPUV3_DI1->DI1_SER_CONF, regVal);
    }
}

//------------------------------------------------------------------------------
//
// Function: DIVSyncCounterSelect
//
// This function sets the VSync counter select in the DI0/DI1
// Sync Assistance Gen Register.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      dwVSyncCounter
//          [in] Counter to define the VSync signal
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DIVSyncCounterSelect(DI_SELECT di_sel, DWORD dwVSyncCounter)
{
    // Check validity of pointer value; valid range = {1-9}
    if (dwVSyncCounter > 7)
    {
        DEBUGMSG (DI_ERROR,
            (TEXT("%s: Invalid dwVSyncCounter value.  Valid range = {0-7}.\r\n"), __WFUNCTION__));
    }

    if (di_sel == DI_SELECT_DI0)
    {
        INSREG32BF(&g_pIPUV3_DI0->DI0_SYNC_AS_GEN,
            IPU_DI_SYNC_AS_GEN_DI_VSYNC_SEL, dwVSyncCounter);
    }
    else
    {
        INSREG32BF(&g_pIPUV3_DI1->DI1_SYNC_AS_GEN,
            IPU_DI_SYNC_AS_GEN_DI_VSYNC_SEL, dwVSyncCounter);
    }
}


//------------------------------------------------------------------------------
//
// Function: DISetSyncStart
//
// This function sets the Sync Start value in the DI0/DI1
// Sync Assistance Gen Register.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      dwSyncStart
//          [in] Sync Start value
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DISetSyncStart(DI_SELECT di_sel, DWORD dwSyncStart)
{
    if (di_sel == DI_SELECT_DI0)
    {
        INSREG32BF(&g_pIPUV3_DI0->DI0_SYNC_AS_GEN,
            IPU_DI_SYNC_AS_GEN_DI_SYNC_START, dwSyncStart);
    }
    else
    {
        INSREG32BF(&g_pIPUV3_DI1->DI1_SYNC_AS_GEN,
            IPU_DI_SYNC_AS_GEN_DI_SYNC_START, dwSyncStart);
    }
}

//------------------------------------------------------------------------------
//
// Function: DISetActiveWindow
//
// This function sets the active window in the DI0/DI1
// Active WindowRegister.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      pAWConfData
//          [in] Active Window configuration register data
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DISetActiveWindow(DI_SELECT di_sel, PDIActiveWindowConfigData pAWConfData)
{
    UINT32 regVal_AW0, regVal_AW1;
    
    regVal_AW0 = CSP_BITFVAL(IPU_DI_AW0_AW_TRIG_SEL, pAWConfData->ActiveWindowTriggerSelector) |
            CSP_BITFVAL(IPU_DI_AW0_AW_HCOUNT_SEL, pAWConfData->HorizontalCounterSelector) |
            CSP_BITFVAL(IPU_DI_AW0_AW_HSTART, pAWConfData->HorizontalStart) |
            CSP_BITFVAL(IPU_DI_AW0_AW_HEND, pAWConfData->HorizontalEnd);            


    regVal_AW1 = CSP_BITFVAL(IPU_DI_AW1_AW_VCOUNT_SEL, pAWConfData->ActiveWindowTriggerSelector) |
            CSP_BITFVAL(IPU_DI_AW1_AW_VSTART, pAWConfData->VerticalStart) |
            CSP_BITFVAL(IPU_DI_AW1_AW_VEND, pAWConfData->VerticalEnd);            

    if (di_sel == DI_SELECT_DI0)
    {
        OUTREG32(&g_pIPUV3_DI0->DI0_AW0, regVal_AW0);
        OUTREG32(&g_pIPUV3_DI0->DI0_AW1, regVal_AW1);
    }
    else
    {
        OUTREG32(&g_pIPUV3_DI1->DI1_AW0, regVal_AW0);
        OUTREG32(&g_pIPUV3_DI1->DI1_AW1, regVal_AW1);
    }
}

//------------------------------------------------------------------------------
//
// Function: DISetScreenHeight
//
// This function sets the Screen Height value in the DI0/DI1
// Screen Configuration Register.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
//      dwScreenHeight
//          [in] Screen height value
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void DISetScreenHeight(DI_SELECT di_sel, DWORD dwScreenHeight)
{
    if (di_sel == DI_SELECT_DI0)
    {
        INSREG32BF(&g_pIPUV3_DI0->DI0_SCR_CONF,
            IPU_DI_SCR_CONF_SCREEN_HEIGHT, dwScreenHeight);
    }
    else
    {
        INSREG32BF(&g_pIPUV3_DI1->DI1_SCR_CONF,
            IPU_DI_SCR_CONF_SCREEN_HEIGHT, dwScreenHeight);
    }
}



void DIDumpRegs()
{
    int i;
    UINT32* ptr = (UINT32 *)&g_pIPUV3_DI0->DI0_GENERAL;
    RETAILMSG (1, (TEXT("\n\nDI0 Registers\n")));
    for (i = 0; i <= (sizeof(CSP_IPU_DI0_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
    
    ptr = (UINT32 *)&g_pIPUV3_DI1->DI1_GENERAL;
    RETAILMSG (1, (TEXT("\n\nDI1 Registers\n")));
    for (i = 0; i <= (sizeof(CSP_IPU_DI1_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
    
}

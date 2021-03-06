//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_ddk_iomux.c
//
//  This file contains the SoC-specific DDK interface for the IOMUX module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static PCSP_IOMUX_REGS g_pIOMUX;


//-----------------------------------------------------------------------------
// Local Functions
BOOL IomuxAlloc(void);
BOOL IomuxDealloc(void);


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetPinMux
//
//  Sets the IOMUX mux for the specified IOMUX pin.
//
//  Parameters:
//      pin
//          [in] Functional pin name used to configure IOMUX SW_MUX_CTL.
//
//      muxmode
//          [in] MUX_MODE configuration.
//
//      sion
//          [in] SION configuration.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE muxmode, 
    DDK_IOMUX_PIN_SION sion)
{
    OUTREG32(&g_pIOMUX->SW_MUX_CTL[pin], (muxmode | sion));
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxGetPinMux
//
//  Gets the IOMUX mux configuration for the specified IOMUX pin.
//
//  Parameters:
//      pin
//          [in] Functional pin name used to select the IOMUX output/input
//               path that will be returned.
//
//      pMuxmode
//          [out] MUX_MODE configuration.
//
//      pSion
//          [out] SION configuration.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxGetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE *pMuxmode, 
    DDK_IOMUX_PIN_SION *pSion)
{
    UINT32 reg;
    
    // Read the SW_MUX_CTL bits for the pin
    reg = INREG32(&g_pIOMUX->SW_MUX_CTL[pin]);

    // Return the MODE/SION path configuration for the pin
    *pMuxmode = reg & CSP_BITFMASK(IOMUX_SW_MUX_CTL_MUX_MODE);
    *pSion = reg & CSP_BITFMASK(IOMUX_SW_MUX_CTL_SION);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetPadConfig
//
//  Sets the IOMUX pad configuration for the specified IOMUX pad.
//
//  Parameters:
//      pad
//          [in] Functional pad name used to select the pad that will be 
//          configured.
//
//      slew
//          [in] Slew rate configuration.
//
//      drive
//          [in] Drive strength configuration.
//
//      opendrain
//          [in] Open drain configuration.
//
//      pull
//          [in] Pull-up/pull-down/keeper configuration.
//
//      hysteresis
//          [in] Hysteresis configuration.
//
//      voltage
//          [in] Drive voltage configuration.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW slew, 
    DDK_IOMUX_PAD_DRIVE drive, DDK_IOMUX_PAD_OPENDRAIN opendrain, 
    DDK_IOMUX_PAD_PULL pull, DDK_IOMUX_PAD_HYSTERESIS hysteresis, 
    DDK_IOMUX_PAD_VOLTAGE voltage)
{
    OUTREG32(&g_pIOMUX->SW_PAD_CTL[pad], (slew | drive | opendrain | 
        pull | hysteresis | voltage));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxGetPadConfig
//
//  Gets the IOMUX pad configration for the specified IOMUX pad.
//
//  Parameters:
//      pad
//          [in] Functional pad name used to select the pad that will be 
//          returned.
//
//      pSlew
//          [out] Slew rate configuration.
//
//      pDrive
//          [out] Drive strength configuration.
//
//      pOpendrain
//          [out] Open drain configuration.
//
//      pPull
//          [out] Pull-up/pull-down/keeper configuration.
//
//      pHysteresis
//          [in] Hysteresis configuration.
//
//      pVoltage
//          [in] Drive voltage configuration.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxGetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW *pSlew, 
    DDK_IOMUX_PAD_DRIVE *pDrive, DDK_IOMUX_PAD_OPENDRAIN *pOpendrain, 
    DDK_IOMUX_PAD_PULL *pPull, DDK_IOMUX_PAD_HYSTERESIS *pHysteresis, 
    DDK_IOMUX_PAD_VOLTAGE *pVoltage)
{
    UINT32 reg;

    // Extract the SW_PAD_CTL bits for the pad
    reg = INREG32(&g_pIOMUX->SW_PAD_CTL[pad]);

    // Return the configuration for the pad
    *pSlew = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_SRE);
    *pDrive = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_DSE);
    *pOpendrain = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_ODE);
    *pPull = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_PUS);
    *pHysteresis = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_HYS);
    *pVoltage = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSelectInput
//
//  Writes a daisy value into the IOMUX SELECT_INPUT register to select 
//  the pad that is the input to the port.
//
//  Note: Depending on the number of possible inputs to the port, 
//  the DAISY field of different SELECT_INPUT registers may get different
//  field widths and value meanings. The function caller shall be aware of 
//  the details by referring to the specific SELECT_INPUT registers 
//  definitions in the IC spec.
//
//  Parameters:
//      port
//          [in] Port to select input
//
//      daisy
//          [in] Data to be written.
//
//  Returns:
//      Returns TRUE.
//
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSelectInput(DDK_IOMUX_SELEIN port, UINT32 daisy)
{
    OUTREG32(&g_pIOMUX->SW_SELECT_INPUT[port], daisy);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: IomuxAlloc
//
//  This function allocates the data structures required for interaction
//  with the IOMUX hardware.
//
//  Parameters:
//      None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL IomuxAlloc(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;
         
    if (g_pIOMUX == NULL)
    {
       phyAddr.QuadPart = CSP_BASE_REG_PA_IOMUXC;

        // Map peripheral physical address to virtual address
        g_pIOMUX = (PCSP_IOMUX_REGS) MmMapIoSpace(phyAddr, 
           sizeof(CSP_IOMUX_REGS), FALSE);

        // Check if virtual mapping failed
        if (g_pIOMUX == NULL)
        {
            DBGCHK((_T("CSPDDK")),  FALSE);
            ERRORMSG(1, (_T("IomuxAlloc:  Virtual mapping failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;
 
cleanUp:

    if (!rc) IomuxDealloc();

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: IomuxDealloc
//
//  This function deallocates the data structures required for interaction
//  with the IOMUX hardware.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL IomuxDealloc(void)
{
    // Unmap peripheral address space
    if (g_pIOMUX != NULL)
    {
        MmUnmapIoSpace(g_pIOMUX, sizeof(CSP_IOMUX_REGS));
        g_pIOMUX = NULL;
    }

    return TRUE;
}



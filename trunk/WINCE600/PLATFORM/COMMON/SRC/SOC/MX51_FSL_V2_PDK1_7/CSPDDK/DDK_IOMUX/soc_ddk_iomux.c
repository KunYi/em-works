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
#include "mx51_to1_ddk.h"

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
static PCSP_IOMUX_TO1_REGS g_pIOMUX;


//-----------------------------------------------------------------------------
// Local Functions
BOOL IomuxAlloc(void);
BOOL IomuxDealloc(void);


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetPinMux
//
//  Sets the IOMUX configuration for the specified IOMUX pin.
//
//  Parameters:
//      pin
//          [in] Functional pin name used to select the IOMUX output/input
//               path that will be configured.
//
//      muxmode
//          [in] MUX_MODE configuration.
//
//      sion
//          [in] SION configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE muxmode, 
    DDK_IOMUX_PIN_SION sion)
{
    // No need to use interlocked access since each pad has a separate register
    OUTREG32(&g_pIOMUX->SW_MUX_CTL[pin], (muxmode | sion));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxGetPinMux
//
//  Gets the IOMUX configuration for the specified IOMUX pin.
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
//      Returns TRUE if successful, otherwise returns FALSE.
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
//      openDrain
//          [in] Open drain configuration.
//
//      pull
//          [in] Pull-up/pull-down/keeper configuration.
//
//      hysteresis
//          [in] Hysteresis configuration.
//
//      inputMode
//          [in] Input mode (CMOS/DDR) configuration.
//
//      outputVolt
//          [in] Output voltage configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW slew, 
    DDK_IOMUX_PAD_DRIVE drive, DDK_IOMUX_PAD_OPENDRAIN openDrain, 
    DDK_IOMUX_PAD_PULL pull, DDK_IOMUX_PAD_HYSTERESIS hysteresis, 
    DDK_IOMUX_PAD_INMODE inputMode, DDK_IOMUX_PAD_OUTVOLT outputVolt)
{
    // No need to use interlocked access since each pad has a separate register
    OUTREG32(&g_pIOMUX->SW_PAD_CTL[pad], (slew | drive | openDrain | pull | hysteresis | 
                  inputMode | outputVolt));
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
//      pOpenDrain
//          [out] Open drain configuration.
//
//      pPull
//          [out] Pull-up/pull-down/keeper configuration.
//
//      pInputMode
//          [out] Input mode (CMOS/DDR) configuration.
//
//      pOutputVolt
//          [out] Output voltage configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxGetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW *pSlew, 
    DDK_IOMUX_PAD_DRIVE *pDrive, DDK_IOMUX_PAD_OPENDRAIN *pOpenDrain, 
    DDK_IOMUX_PAD_PULL *pPull, DDK_IOMUX_PAD_HYSTERESIS *pHysteresis, 
    DDK_IOMUX_PAD_INMODE *pInputMode, DDK_IOMUX_PAD_OUTVOLT *pOutputVolt)
{
    UINT32 reg;

    // Extract the SW_PAD_CTL bits for the pad
    reg = INREG32(&g_pIOMUX->SW_PAD_CTL[pad]);

    // Return the configuration for the pad
    *pSlew = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_SRE);
    *pDrive = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_DSE);
    *pOpenDrain = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_ODE);
    *pPull = reg & (CSP_BITFMASK(IOMUX_SW_PAD_CTL_PKE) |
                    CSP_BITFMASK(IOMUX_SW_PAD_CTL_PUE) |
                    CSP_BITFMASK(IOMUX_SW_PAD_CTL_PUS));
    *pHysteresis = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_HYS);
    *pInputMode = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_DDR_INPUT);
    *pOutputVolt = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_HVE);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetGprBit
//
//  Writes a value into the IOMUX GPR register.  
//
//      data
//          [in] Data to be written.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetGpr0(UINT32 data)
{
    OUTREG32(&g_pIOMUX->GPR0, data);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetGprBit
//
//  Writes a value into the IOMUX GPR register.  
//
//  Parameters:
//
//      data
//          [in] Data to be written.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetGpr1(UINT32 data)
{
    OUTREG32(&g_pIOMUX->GPR1, data);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxGetGprBit
//
//  Read a value into the IOMUX GPR register.  
//
//      data
//          [in] Data to be written.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//
//-----------------------------------------------------------------------------
UINT32 DDKIomuxGetGpr0(VOID)
{
    return INREG32(&g_pIOMUX->GPR0);;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxGetGprBit
//
//  Read a value into the IOMUX GPR register.  
//
//      data
//          [in] Data to be written.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//
//-----------------------------------------------------------------------------
UINT32 DDKIomuxGetGpr1(VOID)
{
    return INREG32(&g_pIOMUX->GPR1);;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSelectInput
//
//  Writes a daisy value into the IOMUX SELECT_INPUT register to select 
//  the pad that is the input to the port.
//
//  Parameters:
//      port
//          [in] Port to select input
//
//      daisy
//          [in] Data to be written.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSelectInput(DDK_IOMUX_SELECT_INPUT port, UINT32 daisy)
{
    // No need to use interlocked access since each pad has a separate register
    OUTREG32(&g_pIOMUX->SELECT_INPUT[port], daisy);
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  IomuxAlloc
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
        g_pIOMUX = (PCSP_IOMUX_TO1_REGS) MmMapIoSpace(phyAddr, 
           sizeof(CSP_IOMUX_TO1_REGS), FALSE);

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
//  Function:  IomuxDealloc
//
//  This function deallocates the data structures required for interaction
//  with the IOMUX hardware.
//
//  Parameters:
//      pIOMUX
//          [in] Points to virtually mapped IOMUX registers when called
//          from OAL, otherwise should be set to NULL.
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
        MmUnmapIoSpace(g_pIOMUX, sizeof(CSP_IOMUX_TO1_REGS));
        g_pIOMUX = NULL;
    }

    return TRUE;
}

//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_iomux.c
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
//  Sets the IOMUX configuration for the specified IOMUX pin.
//
//  Parameters:
//      pin
//          [in] Functional pin name used to select the IOMUX output/input
//               path that will be configured.
//
//      outMux
//          [in] Output path configuration.
//
//      inMux
//          [in] Input path configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_OUT outMux, 
    DDK_IOMUX_IN inMux)
{
    UINT32 oldReg, newReg, *pReg;
    
    // Update pin muxing using interlocked access
    pReg = &g_pIOMUX->SW_MUX_CTL[IOMUX_MUX_REG(pin)];
    do
    {
        oldReg = INREG32(pReg);
        newReg = IOMUX_MUX_BFINS(pin, oldReg, outMux | inMux);
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldReg, newReg) != oldReg);

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
//      pOutMux
//          [out] Output path configuration.
//
//      pInMux
//          [out] Input path configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxGetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_OUT *pOutMux, 
    DDK_IOMUX_IN *pInMux)
{
    UINT32 reg;
    
    // Extract the SW_MUX_CTL bits for the pin
    reg = INREG32(&g_pIOMUX->SW_MUX_CTL[IOMUX_MUX_REG(pin)]);
    reg = IOMUX_MUX_BFEXT(pin, reg);    

    // Return the in/out path configuration for the pin
    *pOutMux = reg & CSP_BITFMASK(IOMUX_SW_MUX_CTL_OUT);
    *pInMux = reg & CSP_BITFMASK(IOMUX_SW_MUX_CTL_IN);

    return TRUE;
        
}


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetPadConfig
//
//  Sets the IOMUX pad configuration for the specified IOMUX pin.
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
//      mode
//          [in] CMOS/open-drain output mode configuration.
//
//      trig
//          [in] Trigger configuration.
//
//      pull
//          [in] Pull-up/pull-down/keeper configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW slew, 
    DDK_IOMUX_PAD_DRIVE drive, DDK_IOMUX_PAD_MODE mode, DDK_IOMUX_PAD_TRIG trig,
    DDK_IOMUX_PAD_PULL pull)
{
    UINT32 oldReg, newReg, *pReg;
    
    // Update pad config using interlocked access
    pReg = &g_pIOMUX->SW_PAD_CTL[IOMUX_PAD_REG(pad)];
    do
    {
        oldReg = INREG32(pReg);
        newReg = IOMUX_PAD_BFINS(pad, oldReg, slew | drive | mode | trig | pull);
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldReg, newReg) != oldReg);

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
//      pMode
//          [out] CMOS/open-drain output mode configuration.
//
//      pTrig
//          [in] Trigger configuration.
//
//      pPull
//          [out] Pull-up/pull-down/keeper configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxGetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW *pSlew, 
    DDK_IOMUX_PAD_DRIVE *pDrive, DDK_IOMUX_PAD_MODE *pMode, DDK_IOMUX_PAD_TRIG *pTrig,
    DDK_IOMUX_PAD_PULL *pPull)
{
    UINT32 reg;

    // Extract the SW_PAD_CTL bits for the pad
    reg = INREG32(&g_pIOMUX->SW_PAD_CTL[IOMUX_PAD_REG(pad)]);
    reg = IOMUX_PAD_BFEXT(pad, reg);    

    // Return the configuration for the pad
    *pSlew = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_SRE);
    *pDrive = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_DSE);
    *pMode = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_ODE);
    *pTrig = reg & CSP_BITFMASK(IOMUX_SW_PAD_CTL_HYS);
    *pPull = reg & (CSP_BITFMASK(IOMUX_SW_PAD_CTL_PKE) |
                    CSP_BITFMASK(IOMUX_SW_PAD_CTL_PUE) |
                    CSP_BITFMASK(IOMUX_SW_PAD_CTL_PUS));


    return TRUE;
        
}


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetGpr
//
//  Writes a value into the IOMUX GPR register.  The GPR is
//  used to control the muxing of signals within the SoC.
//
//  Parameters:
//      mask
//          [in] Bit mask for GPR bits to be written.
//
//      data
//          [in] Data to be written.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetGpr(UINT32 mask, UINT32 data)
{
    UINT32 oldReg, newReg, *pReg;
    
    // Make sure data bits fall within mask
    data &= mask;

    // Update pad config using interlocked access
    pReg = &g_pIOMUX->GPR;
    do
    {
        oldReg = INREG32(pReg);
        newReg = (oldReg & (~mask)) | data;
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldReg, newReg) != oldReg);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetGprBit
//
//  Writes a value into the specified IOMUX GPR bit.  These GPR bits
//  are used to control the muxing of signals within the SoC.
//
//  Parameters:
//      bit
//          [in] GPR bit to be configured.
//
//      data
//          [in] Value for the GPR bit [0 or 1] 
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetGprBit(DDK_IOMUX_GPR bit, UINT32 data)
{
    return DDKIomuxSetGpr((1U << bit), (data << bit));
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
        MmUnmapIoSpace(g_pIOMUX, sizeof(CSP_IOMUX_REGS));
        g_pIOMUX = NULL;
    }

    return TRUE;
    
}

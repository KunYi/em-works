//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  utils.c
//
//  Generic "utility" routines for the bootloader.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "loader.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables
extern PCSP_CRM_REGS g_pCRM;

//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
//
void OEMReset()
{
    
    PCSP_WDOG_REGS pReg = (PCSP_WDOG_REGS)OALPAtoUA(CSP_BASE_REG_PA_WDOG);
    pReg->WCR = pReg->WCR&(~(1<<WDOG_WCR_SRS_LSH));

}
//-----------------------------------------------------------------------------
//
//  Function:  SpinForever
//
//  Halts the bootloader.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SpinForever(void)
{
    UINT32 Selection;
    KITLOutputDebugString("SpinForever...\r\n");
    KITLOutputDebugString("Do you want to reset [Y\\N] \r\n");

    for (;;)
    {
        Selection=OEMReadDebugByte();
        if(Selection == 'Y' || Selection == 'y')
        {
            OEMReset();
        }
    }
}


//-----------------------------------------------------------------------------
//
//  Function:  OEMShowProgress
//
//  This function shows visual information, on an LED, for example, 
//  to let users know that the download is in progress. It is called as the 
//  download progresses.
//
//  Parameters:
//      dwPacketNum 
//          [in] Equal to the packet number currently downloading. Knowing 
//          the total number of packets, a download percentage can be computed.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void OEMShowProgress(DWORD dwPacketNum)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwPacketNum);
}



//-----------------------------------------------------------------------------
//
//  Function: BootTimerInit
//
//  This function initializes EPIT which is being used by 
//  OALStall() implementation.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void BootTimerInit(void)
{
    PCSP_EPIT_REG pEPIT;
    
    pEPIT = (PCSP_EPIT_REG) OALPAtoUA(CSP_BASE_REG_PA_EPIT1);
    if (pEPIT == NULL)
    {
        KITLOutputDebugString("BootTimerInit: EPIT mapping failed!\r\n");
        return;
    }

    // Disable EPIT and clear all configuration bits
    OUTREG32(&pEPIT->CR, 0);

    // Assert software reset for the timer
    OUTREG32(&pEPIT->CR, CSP_BITFMASK(EPIT_CR_SWR));

    // Wait for the software reset to complete
    while (INREG32(&pEPIT->CR) & CSP_BITFMASK(EPIT_CR_SWR));

    // Enable timer for "free-running" mode where timer rolls
    // over from 0x00000000 to 0xFFFFFFFF
    OUTREG32(&pEPIT->CR,
        CSP_BITFVAL(EPIT_CR_EN, EPIT_CR_EN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_ENMOD, EPIT_CR_ENMOD_RESUME) |
        CSP_BITFVAL(EPIT_CR_OCIEN, EPIT_CR_OCIEN_DISABLE) |
        CSP_BITFVAL(EPIT_CR_RLD, EPIT_CR_RLD_ROLLOVER) |
        CSP_BITFVAL(EPIT_CR_PRESCALAR, BSP_EPIT_PRESCALAR) |
        CSP_BITFVAL(EPIT_CR_SWR, EPIT_CR_SWR_NORESET) |
        CSP_BITFVAL(EPIT_CR_IOVW, EPIT_CR_IOVW_NOOVR) |
        CSP_BITFVAL(EPIT_CR_DBGEN, EPIT_CR_DBGEN_ACTIVE) |
        CSP_BITFVAL(EPIT_CR_WAITEN, EPIT_CR_WAITEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_DOZEN, EPIT_CR_DOZEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_STOPEN, EPIT_CR_STOPEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_OM, EPIT_CR_OM_DICONNECT) |
        CSP_BITFVAL(EPIT_CR_CLKSRC, BSP_EPIT_CLKSRC));
}


//-----------------------------------------------------------------------------
//
//  Function: OALClockSetGatingMode
//
//  This function provides the OAL a safe mechanism for setting the clock
//  gating mode of peripherals.
//
//  Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control
//           bits.
//
//      mode
//           [in] Requested clock gating mode for the peripheral.
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode)
{
        
    // Update the clock gating mode
    INSREG32(&g_pCRM->CGR_REGS.CGR[CRM_CGR_INDEX(index)], CRM_CGR_MASK(index), 
        CRM_CGR_VAL(index, mode));

}

//------------------------------------------------------------------------------
//
//  Function:  SetBootMe
//
//  Allows user to set a BOOTME packet count using the boot loader menu.
//
//  Parameters:
//      eBootCFG
//          [out] Points to bootloader configuration that will be updated with
//          the BOOTME packet count entered by the user.
//
//  Returns:
//      None.
//
//------------------------------------------------------------------------------
void SetBootMe(BOOT_CFG *pBootCFG)
{
    char szCount[16];
    WORD cwNumChars = 0;
    UINT16 InChar = 0;
    INT32  dwCharRead = OEM_DEBUG_READ_NODATA;

    KITLOutputDebugString ( "\r\nUse 0 for continuous boot me packets. \r\n");
    KITLOutputDebugString ( "Enter maximum number of boot me packets to send [0-255]: ");

    while (!((InChar == 0x0d) || (InChar == 0x0a)))
    {
        dwCharRead = OEMReadDebugByte();
        if (dwCharRead != OEM_DEBUG_COM_ERROR && dwCharRead != OEM_DEBUG_READ_NODATA)
        {
            InChar = (UINT16)dwCharRead;
            // If it's a number or a period, add it to the string
            if ((InChar >= '0' && InChar <= '9'))
            {
                if (cwNumChars < 16)
                {
                    szCount[cwNumChars++] = (char)InChar;
                    OEMWriteDebugByte((BYTE)InChar);
                }
            }
            // If it's a backspace, back up
            else if (InChar == 8)
            {
                if (cwNumChars > 0)
                {
                    cwNumChars--;
                    OEMWriteDebugByte((BYTE)InChar);
                }
            }
        }
    }

    // If it's a carriage return with an empty string, don't change anything.
    if (cwNumChars)
    {
        szCount[cwNumChars] = '\0';
        pBootCFG->numBootMe = atoi(szCount);
        if (pBootCFG->numBootMe > 255)
        {
            pBootCFG->numBootMe = 255;
        }
        else if (pBootCFG->numBootMe < 0)
        {
            pBootCFG->numBootMe = 1;
        }
    }
}


//------------------------------------------------------------------------------
//
//  Function:  SetDelay
//
//  Allows user to set a boot delay using the boot loader menu.
//
//  Parameters:
//      eBootCFG
//          [out] Points to bootloader configuration that will be updated with
//          the boot delay entered by the user.
//
//  Returns:
//      None.
//
//------------------------------------------------------------------------------
void SetDelay(BOOT_CFG *pBootCFG)
{
    char szCount[16];
    WORD cwNumChars = 0;
    UINT16 InChar = 0;
    INT32  dwCharRead = OEM_DEBUG_READ_NODATA;

    KITLOutputDebugString ( "\r\nEnter maximum number of seconds to delay [1-255]: ");

    while (!((InChar == 0x0d) || (InChar == 0x0a)))
    {
        dwCharRead = OEMReadDebugByte();
        if (dwCharRead != OEM_DEBUG_COM_ERROR && dwCharRead != OEM_DEBUG_READ_NODATA)
        {
            InChar = (UINT16)dwCharRead;

            // If it's a number or a period, add it to the string
            if ((InChar >= '0' && InChar <= '9'))
            {
                if (cwNumChars < 16)
                {
                    szCount[cwNumChars++] = (char)InChar;
                    OEMWriteDebugByte((BYTE)InChar);
                }
            }
            // If it's a backspace, back up
            else if (InChar == 8)
            {
                if (cwNumChars > 0)
                {
                    cwNumChars--;
                    OEMWriteDebugByte((BYTE)InChar);
                }
            }
        }
    }

    // If it's a carriage return with an empty string, don't change anything.
    if (cwNumChars)
    {
        szCount[cwNumChars] = '\0';
        pBootCFG->delay = atoi(szCount);
        if (pBootCFG->delay > 255)
        {
            pBootCFG->delay = 255;
        }
        else if (pBootCFG->delay < 1)
        {
            pBootCFG->delay = 1;
        }
    }
}


//------------------------------------------------------------------------------
//
//  Function:  NKCreateStaticMapping
//
//  Stub needed by fsl_usbfn_rndiskitl.lib. When linking with EBOOT.exe
//  use the stub. When linking with KITL.dll use the real function.
//
//------------------------------------------------------------------------------
VOID* NKCreateStaticMapping(DWORD phBase, DWORD size)
{
    UNREFERENCED_PARAMETER(size);
    return OALPAtoUA(phBase << 8);
}

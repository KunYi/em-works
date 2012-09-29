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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
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
extern PVOID pv_HWregRTC;
extern PVOID pv_HWregCLKCTRL;
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
void SetMAC(BOOT_CFG *pBootCfg);

//------------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
//
//  Function:  OEMReset
//
//  Reboot the whole system.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void OEMReset()
{
    // Get uncached virtual addresses for ChipReset
    HW_CLKCTRL_RESET_SET(BM_CLKCTRL_RESET_CHIP);
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
    // MAP the Hardware registers
    if(pv_HWregRTC == NULL)
    {
        pv_HWregRTC = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_RTC, FALSE);
    }
    
    if (!pv_HWregRTC)
    {
        OALMSG(OAL_ERROR, (L"ERROR: InitRTC: pv_HWregRTC null pointer!\r\n"));
        goto cleanUp;
    }
    // Remove soft reset
    HW_RTC_CTRL_CLR(BM_RTC_CTRL_SFTRST);
    // Remove clock gate
    HW_RTC_CTRL_CLR(BM_RTC_CTRL_CLKGATE);
cleanUp:
    return;
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
//VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode)
//{
//    // All clocks are enabled out of reset, so just provide a stub
//
//    // Remove-W4: Warning C4100 workaround
//    UNREFERENCED_PARAMETER(index);
//    UNREFERENCED_PARAMETER(mode);
//}




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
    INT32 dwCharRead = OEM_DEBUG_READ_NODATA;

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
    INT32 dwCharRead = OEM_DEBUG_READ_NODATA;

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


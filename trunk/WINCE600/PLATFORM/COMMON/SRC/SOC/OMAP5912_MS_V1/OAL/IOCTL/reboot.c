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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  reboot.c
//
//  This file implement OMAP5912 SoC specific OALIoCtlHalReboot function.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <omap5912.h>

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReboot
//
//  The code issue CPU reset. Because of internal ROM speed we have first
//  set DPLL module to bypass mode. This will set all clock domain back to base
//  frequency 13MHz.
//
BOOL OALIoCtlHalReboot(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    OMAP5912_CLKM_REGS *pCLKMRegs = OALPAtoUA(OMAP5912_CLKM_REGS_PA);
    
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReboot\r\n"));

    // Let do reset
    OALMSGS(TRUE, (L"*** RESET ***\r\n"));

    OALStall(1000000);

    // First we must brake clocks - bypass DPLL & set divider to 1
    CLRREG16(&pCLKMRegs->DPLL1_CTL, DPLL1_CTL_PLL_ENABLE | 3 << 1);

    // Reset
    OUTREG16(&pCLKMRegs->RSTCT1, RSTCT1_SW_RST);
    
    // Should never get to this point...
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalReboot\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------

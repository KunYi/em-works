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
//------------------------------------------------------------------------------
//
//  File: reboot.c
//
//  This file implement VRC5477 specific OALIoCtlxxxx functions.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <vrc5477.h>


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReboot
//
//
BOOL OALIoCtlHalReboot(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    // Get and save uncached virtual addresses fir ICU and GIU
    VRC5477_REGS *pVRC5477Regs = OALPAtoUA(VRC5477_REG_PA);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReboot\r\n"));

    // Let reset PCI buses
    SETREG32(&pVRC5477Regs->PCICTL0H, 1 << 31);
    SETREG32(&pVRC5477Regs->PCICTL1H, 1 << 31);

    // Warm reset the CPU
    while (TRUE) SETREG32(&pVRC5477Regs->CPUSTAT, 1 << 1);

    // Should never get to this point...
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalReboot\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------

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
// NOTE: stubs are being used - this isn't done

#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>

// Base Definitions
#include "s3c6410_base_regs.h"
#include "s3c6410_pwm.h"
#include "s3c6410_syscon.h"
#include "soc_cfg.h"
#include <pmplatform.h>

#if (CPU_REVISION != EVT0)
extern void _OEMSWReset(void);
#endif

void OEMSWReset(void)
{
//
// If the board design supports software-controllable hardware reset logic, it should be
// used.  Because this routine is specific to the S3C6410 CPU, it only uses the watchdog
// timer to assert reset.  One downside to this approach is that nRSTOUT isn't asserted
// so any board-level logic isn't reset via this method.  This routine can be overidden in
// the specific platform code to control board-level reset logic, should it exist.
//

    volatile S3C6410_SYSCON_REG *pSysConReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);

    OALMSG(TRUE, (L"[OEM] ++OEMSWReset()\r\n"));

    // HCLK_IROM, HCLK_MEM1, HCLK_MEM0, HCLK_MFC Should be Always On for power Mode (Something coupled with BUS operation)
    pSysConReg->HCLK_GATE |= ((1<<25)|(1<<22)|(1<<21)|(1<<0));

    #if (CPU_REVISION == EVT0)
    OALMSG(TRUE, (L"[OEM] + OEMSWReset() using SW_RST function\r\n"));
    // Generate Software Reset
    pSysConReg->SW_RST = 0x6410;

    #else
    OALMSG(TRUE, (L"[OEM] + OEMSWReset() using watchdog timer\r\n"));
    // Generate Software Reset using watchdog timer
    _OEMSWReset();

    #endif

    // Wait for Reset
    //
    while(1);

    // Should Never Get Here
    //
    OALMSG(TRUE, (L"[OEM] --OEMSWReset() : Do Not See Me !!!!!! \r\n"));
}

//------------------------------------------------------------------------------


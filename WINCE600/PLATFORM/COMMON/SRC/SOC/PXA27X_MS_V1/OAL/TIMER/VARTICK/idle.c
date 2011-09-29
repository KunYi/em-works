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
//  File:  idle.c
//

#include <windows.h>
#include <nkintr.h>
#include <pkfuncs.h>
#include <bulverde_base_regs.h>
#include <bulverde_memctrl.h>
#include <oal.h>

//------------------------------------------------------------------------------
// Defines 
 
//------------------------------------------------------------------------------
// External Variables 

//------------------------------------------------------------------------------
// Global Variables 
 
//------------------------------------------------------------------------------
// Local Variables 
static PBULVERSE_MEMCTRL_REG m_pMemCtrl = 0; 

//------------------------------------------------------------------------------
// Local Functions 
//
//  Function:     OALCPUIdle
//
//  This function is called by the common implementation of variable OEMIdle.
//  OEMIdle is called by the kernel when there are no threads ready to 
//  run. The CPU should be put into a reduced power mode if possible and halted. 
//  It is important to be able to resume execution quickly upon receiving an 
//  interrupt.
//
//  Interrupts are off when OALCPUIdle is called. Interrrupts are also turned off when OALCPUIdle returns.
//

void OALCPUIdle()
{
    extern void CPUEnterIdle(void);

    if (!m_pMemCtrl)
        m_pMemCtrl = (PBULVERSE_MEMCTRL_REG) OALPAtoUA(BULVERDE_BASE_REG_PA_MEMC);

    // Enable Auto power down of SDRAM & synchronous flash
    //
    m_pMemCtrl->mdrefr |= MEMCTRL_MDREFR_APD;
    
    // No need to turn interrupts on, CPUEnterIdle will do this automatically.
    // The scheduler does not handle interrupts that occur before 
    // CPUEnterIdle executes! 

    // Do not call INTERRUPTS_ON()
    
    CPUEnterIdle();

    INTERRUPTS_OFF();

    // Disable auto power down of SDRAM & synchronous flash
    // (If APD is set a latency penalty of one memory cycle is incurred for 
    //  restarting SDCLK and SDCKE between non-consecutive SDRAM/synchronous
    //  memory flash transfers. Disabling APD improves the memory throughput
    //  during normal operation)
    //
    m_pMemCtrl->mdrefr &= ~(MEMCTRL_MDREFR_APD);

}
//------------------------------------------------------------------------------

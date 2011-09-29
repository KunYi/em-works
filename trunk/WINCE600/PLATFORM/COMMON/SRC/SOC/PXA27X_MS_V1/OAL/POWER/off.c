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

#include <windows.h>
#include <oal.h>
#include <nkintr.h>
#include <bulverde_base_regs.h>
#include <xllp_defs.h>
#include <xllp_pm_sleepcontext.h>
#include "off.h"

// module level var that stores sleep params and data saved before going to sleep
//
struct OFF_SLEEP_DATA_S m_SleepData;

// External functions
//
extern VOID OEMInitDebugSerial(void);

//------------------------------------------------------------------------------
//
// Function:     OEMPowerOff
//
// Description:  Called when the system is to transition to it's lowest
//               power mode (off)
//               transition to deep-sleep mode (lowest power consumption mode)
//

void OEMPowerOff()
{
    BOOL PowerState;
    static BOOL sleepParamsInit = FALSE;
    
    // init sleep params variable
    //
    if (!sleepParamsInit) {
        InitSleepParams(&m_SleepData);
        sleepParamsInit = TRUE;
    }

    // reset global wake-up source variable
    //
    g_oalWakeSource = SYSWAKE_UNKNOWN;

    // Give chance to do board specific stuff
    // Note: this sets wake-up source register
    //
    BSPPowerOff();

    // power off KITL
    // 
    PowerState = 0;
    KITLIoctl (IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);

    // Sleep
    //
    XllpPmEnterSleep(&m_SleepData.sleepParameters);

    //At this point device returned from sleep

    // turn on the serial port
    //
    OEMInitDebugSerial ();

    // Reinitialize KITL
    //
    PowerState = 1;
    KITLIoctl (IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);

    // Do board specific stuff
    // Note: this sets g_oalWakeSource
    //
    BSPPowerOn();

}

//------------------------------------------------------------------------------
//
// Function:     InitSleepParams
//
// Description:  Initialize sleep params (need to do only once).
//                   Sets up the sleep parameters that are used by the
//                   xllp suspend/resume layer to put the system to sleep/deep-sleep
//
void InitSleepParams(struct OFF_SLEEP_DATA_S* pSleepData)
{

    struct XLLP_PM_ENTER_SLEEP_PARAMS_S*  pSlpParams = &pSleepData->sleepParameters;
         
    // Force any unsupported options to not-selected
    //
    memset (pSlpParams, 0, sizeof (struct XLLP_PM_ENTER_SLEEP_PARAMS_S));

    //Set regs Uncached addresses. Required by the xllp layer to access, save and restore regs
    //
    pSlpParams->ProcRegs.UAPwrMgrRegs      = (XLLP_UINT32_T) OALPAtoUA(BULVERDE_BASE_REG_PA_PWR);
    pSlpParams->ProcRegs.UAGPIORegs        = (XLLP_UINT32_T) OALPAtoUA(BULVERDE_BASE_REG_PA_GPIO);
    pSlpParams->ProcRegs.UAIntcRegs        = (XLLP_UINT32_T) OALPAtoUA(BULVERDE_BASE_REG_PA_INTC);
    pSlpParams->ProcRegs.UAIMControlReg    = (XLLP_UINT32_T) OALPAtoUA(BULVERDE_BASE_REG_PA_IMCONTROL);
    pSlpParams->ProcRegs.UAOSTRegs         = (XLLP_UINT32_T) OALPAtoUA(BULVERDE_BASE_REG_PA_OST);
    pSlpParams->ProcRegs.UAMEMCRegs        = (XLLP_UINT32_T) OALPAtoUA(BULVERDE_BASE_REG_PA_MEMC);
    
    //Set other values
    //
    pSlpParams->SleepDataAreaPA             = (XLLP_UINT32_T) OALVAtoPA(pSleepData);
    pSlpParams->SleepDataAreaVA             = (P_XLLP_PM_SLEEP_SAVE_DATA_T) pSleepData;
    pSlpParams->PWRMODE                     = XLLP_PM_PWRMODE_DEEPSLEEP;              //deep-sleep by default

}

//------------------------------------------------------------------------------
//
// Function:     OALIoCtlHalPresuspend
//
// Description:  
//

BOOL OALIoCtlHalPresuspend(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    return TRUE;
}

//------------------------------------------------------------------------------

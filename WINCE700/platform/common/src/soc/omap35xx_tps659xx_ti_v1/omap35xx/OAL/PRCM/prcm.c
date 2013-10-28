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
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File: prcm.c
//
#include <windows.h>
#include <nkintr.h>
#include <pkfuncs.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <initguid.h>
#include <omap35xx_guid.h>
#include <bus.h>
#include <dvfs.h>
#include <prcm_priv.h>
#include <interrupt_struct.h>

//-----------------------------------------------------------------------------
//
//  Used for debugging suspend/resume
//

extern VOID PrcmSaveRefCounts();
extern void PrcmRegsSnapshot();
extern BOOL g_PrcmDebugSuspendResume;

//-----------------------------------------------------------------------------
//
//  Used for save/restore of GPIO clock state
//

DWORD prevGpioIClkState;
DWORD prevGpioFClkState;

//-----------------------------------------------------------------------------
//
//  Function:  OALGetTTBR
//
//  function which returns the start of the MMU L1 table
//
extern DWORD OALGetTTBR();

//-----------------------------------------------------------------------------
//
//  Function:  OALInvalidateTlb
//
//  function which Invalidates the TLB
//
extern VOID OALInvalidateTlb();


//-----------------------------------------------------------------------------
//
//  Global:  g_pIntr
//
//  Reference to all interrupt related registers. Initialized in OALIntrInit
//
extern OMAP_INTR_CONTEXT const *g_pIntr;

//-----------------------------------------------------------------------------
//
//  Global:  g_pPrcmPrm
//
//  Reference to all PRCM-PRM registers. Initialized in PrcmInit
//
OMAP_PRCM_PRM              *g_pPrcmPrm;

//-----------------------------------------------------------------------------
//
//  Global:  g_pPrcmCm
//
//  Reference to all PRCM-CM registers. Initialized in PrcmInit
//
OMAP_PRCM_CM               *g_pPrcmCm;

//-----------------------------------------------------------------------------
//
//  Global:  g_PrcmPostInit
//
//  Indicates if prcm library has been fully initialized.
//  Initialized in PrcmPostInit
//
BOOL                        g_PrcmPostInit = FALSE;

//------------------------------------------------------------------------------
//
//  Global:  g_bSingleThreaded
//
//  Indicated that the OAL PRCM functions are being called while the kernel is 
//  single threaded (OEMIdle, OEMPoweroff)
//
BOOL                        g_bSingleThreaded = FALSE;

//-----------------------------------------------------------------------------
//
//  Global:  g_rgPrcmMutex
//
//  Contains a list of CRITICAL_SECTIONS used for synchronized access to
//  PRCM registers
//
CRITICAL_SECTION            g_rgPrcmMutex[Mutex_Count];

//-----------------------------------------------------------------------------
//
//  Global:  g_pCPUInfo
//
//  contains references to relevant chip/power information used in SRAM
//  functions
//
CPU_INFO                   *g_pCPUInfo = NULL;

//-----------------------------------------------------------------------------
//
//  Global:  fnCpuStart
//
//  Beginning of all SRAM routines
//
pCPUStart                   fnCpuStart = NULL;

//-----------------------------------------------------------------------------
//
//  Global:  fnOALCPUIdle
//
//  Reference to cpu idle code in SRAM
//
pCPUIdle                    fnOALCPUIdle = NULL;

//-----------------------------------------------------------------------------
//
//  Global:  fnOALUpdateCoreFreq
//
//  Reference to SRAM based DPLL3 frequency changing routine
//
pOALUpdateCoreFreq          fnOALUpdateCoreFreq = NULL;

//-----------------------------------------------------------------------------
OMAP_CM_REGS*
GetCmRegisterSet(
    UINT powerDomain
    )
{
    switch (powerDomain)
        {
        case POWERDOMAIN_WAKEUP:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_WKUP_CM;

        case POWERDOMAIN_CORE:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_CORE_CM;

        case POWERDOMAIN_PERIPHERAL:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_PER_CM;

        case POWERDOMAIN_USBHOST:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_USBHOST_CM;

        case POWERDOMAIN_EMULATION:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_EMU_CM;

        case POWERDOMAIN_MPU:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_MPU_CM;

        case POWERDOMAIN_DSS:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_DSS_CM;

        case POWERDOMAIN_NEON:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_NEON_CM;

        case POWERDOMAIN_IVA2:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_IVA2_CM;

        case POWERDOMAIN_CAMERA:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_CAM_CM;

        case POWERDOMAIN_SGX:
            return (OMAP_CM_REGS*)g_pPrcmCm->pOMAP_SGX_CM;
        }

    return NULL;
}

//-----------------------------------------------------------------------------
OMAP_PRM_REGS*
GetPrmRegisterSet(
    UINT powerDomain
    )
{
    switch (powerDomain)
        {
        case POWERDOMAIN_WAKEUP:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_WKUP_PRM;

        case POWERDOMAIN_CORE:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_CORE_PRM;

        case POWERDOMAIN_PERIPHERAL:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_PER_PRM;

        case POWERDOMAIN_USBHOST:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_USBHOST_PRM;

        case POWERDOMAIN_EMULATION:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_EMU_PRM;

        case POWERDOMAIN_MPU:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_MPU_PRM;

        case POWERDOMAIN_DSS:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_DSS_PRM;

        case POWERDOMAIN_NEON:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_NEON_PRM;

        case POWERDOMAIN_IVA2:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_IVA2_PRM;

        case POWERDOMAIN_CAMERA:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_CAM_PRM;

        case POWERDOMAIN_SGX:
            return (OMAP_PRM_REGS*)g_pPrcmPrm->pOMAP_SGX_PRM;
        }

    return NULL;
}

//-----------------------------------------------------------------------------
UINT
PrcmInterruptEnable(
    UINT mask,
    BOOL bEnable
    )
{
    UINT val;
    Lock(Mutex_Intr);

    // enable/disable prcm interrupts
    val = INREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQENABLE_MPU);
    val = (bEnable != FALSE) ? (val | mask) : (val & ~mask);
    OUTREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQENABLE_MPU, val);

    Unlock(Mutex_Intr);
    return val;
}

//-----------------------------------------------------------------------------
UINT
PrcmInterruptClearStatus(
    UINT mask
    )
{
    UINT val;

    // This routine should only be called during system boot-up or
    // from OEMIdle.  Hence, serialization within this routine
    // should not be performed.

    // clear prcm interrupt status
    val = INREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQSTATUS_MPU);
    OUTREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQSTATUS_MPU, val & mask);

    // wakeup
    OUTREG32(&g_pPrcmPrm->pOMAP_WKUP_PRM->PM_WKST_WKUP,
        INREG32(&g_pPrcmPrm->pOMAP_WKUP_PRM->PM_WKST_WKUP) | CM_CLKEN_IO
        );

    // return the status prior to clearing the status
    return val;
}

//-----------------------------------------------------------------------------
static
void
ClearXNBit(
    void *pvAddr
    )
{
    const UINT ARM_L1_NO_EXECUTE = 0x00000010;

    DWORD   idxL1MMU = ((DWORD)pvAddr) >> 20;
    DWORD  *pL1MMUTbl = (DWORD*)OALPAtoVA(OALGetTTBR(), FALSE);

    pL1MMUTbl[idxL1MMU] &= ~ARM_L1_NO_EXECUTE;
}


//-----------------------------------------------------------------------------
//
//  Function:  OALSRAMFnInit
//
//  copies power critical code into SRAM to be executed in the sanctuary of
//  SRAM
//
BOOL OALSRAMFnInit()
{
    UINT voltctrl = 0;
    pInvalidateTlb  fnInvalidateTlb;

    OALMSG(OAL_FUNC, (L"+OALSRAMFnInit()\r\n"));

    // get reference to SRAM
    fnCpuStart = OALPAtoVA(OMAP_SRAM_PA + dwOEMSRAMStartOffset, FALSE);

    // initialize cpu idle data structure
    g_pCPUInfo = (CPU_INFO*)OALPAtoVA(dwOEMMPUContextRestore, FALSE);
    g_pCPUInfo->SDRC_REGS = (UINT)g_pSDRCRegs;
    g_pCPUInfo->MPU_CM_REGS = (UINT)g_pPrcmCm->pOMAP_MPU_CM;
    g_pCPUInfo->CORE_CM_REGS = (UINT)g_pPrcmCm->pOMAP_CORE_CM;
    g_pCPUInfo->MPU_PRM_REGS = (UINT)g_pPrcmPrm->pOMAP_MPU_PRM;
    g_pCPUInfo->CORE_PRM_REGS = (UINT)g_pPrcmPrm->pOMAP_CORE_PRM;
    g_pCPUInfo->GPTIMER_REGS = (UINT)g_pTimerRegs;
    g_pCPUInfo->CLOCK_CTRL_CM_REGS = (UINT)g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM;
    g_pCPUInfo->MPU_CONTEXT_VA = (UINT)g_pCPUInfo + sizeof(CPU_INFO);
    g_pCPUInfo->MPU_CONTEXT_PA = (UINT)dwOEMMPUContextRestore + sizeof(CPU_INFO);

    // Populate fnOALCPUIdle function pointer with SRAM address of
    // OALCPUIdle function.
    fnOALCPUIdle = (pCPUIdle)((UINT)fnCpuStart + 
                                ((UINT)OALCPUIdle - (UINT)OALCPUStart)); 
    
    fnOALUpdateCoreFreq = (pOALUpdateCoreFreq)((UINT)fnCpuStart +
                                ((UINT)OALUpdateCoreFreq - (UINT)OALCPUStart));

    // Populate fnTlbValidate function pointer with SDRAM address of
    // OALInvalidateTlb function which will be called by restore.s 
    // after mpu restore.
    fnInvalidateTlb = OALPAtoVA(OALVAtoPA(OALInvalidateTlb), FALSE);

    // update the TLB Inv addr in cpuInfo
    g_pCPUInfo->TLB_INV_FUNC_ADDR = (UINT)fnInvalidateTlb;
    
    // Kernel marks all uncached adress as "no execute", we need to clear
    // it here since we're going to run some routines from non-cached memory
    ClearXNBit(fnCpuStart);
    ClearXNBit(fnOALCPUIdle);
    ClearXNBit(fnInvalidateTlb);

    // We assume OALCPUStart code is *alway* the first function in the
    // group of routines which get copied to SRAM and OALCPULast is last
    memcpy(fnCpuStart, OALCPUStart, (UINT)OALCPUEnd - (UINT)OALCPUStart);

    //  Flush the cache to ensure idle code is in SRAM
    OEMCacheRangeFlush( fnCpuStart, (UINT)OALCPUEnd - (UINT)OALCPUStart, CACHE_SYNC_ALL );

    OALMSG(OAL_FUNC, (L"-OALSRAMFnInit()\r\n"));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  OALSetSDRCRefreshCounter
//
//  Sets the SDRC Refresh counter
//
void
OALSDRCRefreshCounter(
    UINT highFreq,
    UINT lowFreq
    )
{
    g_pCPUInfo->SDRC_HIGH_RFR_FREQ = highFreq;
    g_pCPUInfo->SDRC_LOW_RFR_FREQ = lowFreq;
}

//-----------------------------------------------------------------------------
//
//  Function:  OALSavePrcmContext
//
//  Saves the prcm context
//
void
OALSavePrcmContext()
{
    // check for valid pointers
    if (g_pPrcmRestore == NULL) return;

    // save prcm restore registers

    OUTREG32(&g_pPrcmRestore->PRM_CLKSRC_CTRL,
        INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_CLKSRC_CTRL)
        );

    OUTREG32(&g_pPrcmRestore->PRM_CLKSEL,
        INREG32(&g_pPrcmPrm->pOMAP_CLOCK_CONTROL_PRM->PRM_CLKSEL)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKSEL_CORE,
        INREG32(&g_pPrcmCm->pOMAP_CORE_CM->CM_CLKSEL_CORE)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKSEL_WKUP,
        INREG32(&g_pPrcmCm->pOMAP_WKUP_CM->CM_CLKSEL_WKUP)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKEN_PLL,
        INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL)
        );

    OUTREG32(&g_pPrcmRestore->CM_AUTOIDLE_PLL,
        INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_AUTOIDLE_PLL)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL,
        INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKSEL2_PLL,
        INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL2_PLL)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKSEL3_PLL,
        INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL3_PLL)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKEN_PLL_MPU,
        INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKEN_PLL_MPU)
        );

    OUTREG32(&g_pPrcmRestore->CM_AUTOIDLE_PLL_MPU,
        INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_AUTOIDLE_PLL_MPU)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL_MPU,
        INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSEL1_PLL_MPU)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKSEL2_PLL_MPU,
        INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSEL2_PLL_MPU)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKSTCTRL_MPU,
        INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSTCTRL_MPU)
        );

    OUTREG32(&g_pPrcmRestore->CM_CLKSTCTRL_CORE,
        INREG32(&g_pPrcmCm->pOMAP_CORE_CM->CM_CLKSTCTRL_CORE)
        );
}

//-----------------------------------------------------------------------------
//
//  Function:  OALSaveSdrcContext
//
//  Saves the SDRC context
//
void
OALSaveSdrcContext()
{
    // check for valid pointers
    if (g_pSdrcRestore == NULL) return;

    OUTREG16(&g_pSdrcRestore->SYSCONFIG,
        (UINT16)INREG32(&g_pSDRCRegs->SDRC_SYSCONFIG)
        );

    OUTREG16(&g_pSdrcRestore->CS_CFG,
        (UINT16)INREG32(&g_pSDRCRegs->SDRC_CS_CFG)
        );

    OUTREG16(&g_pSdrcRestore->SHARING,
        (UINT16)INREG32(&g_pSDRCRegs->SDRC_SHARING)
        );

    OUTREG16(&g_pSdrcRestore->ERR_TYPE,
        (UINT16)INREG32(&g_pSDRCRegs->SDRC_ERR_TYPE)
        );

    OUTREG32(&g_pSdrcRestore->DLLA_CTRL,
        INREG32(&g_pSDRCRegs->SDRC_DLLA_CTRL)
        );

    OUTREG32(&g_pSdrcRestore->DLLB_CTRL,
        INREG32(&g_pSDRCRegs->SDRC_DLLB_CTRL)
        );

    OUTREG32(&g_pSdrcRestore->POWER,
        INREG32(&g_pSDRCRegs->SDRC_POWER)
        );

    OUTREG32(&g_pSdrcRestore->MCFG_0,
        INREG32(&g_pSDRCRegs->SDRC_MCFG_0)
        );

    OUTREG16(&g_pSdrcRestore->MR_0,
        (UINT16)INREG32(&g_pSDRCRegs->SDRC_MR_0)
        );

    OUTREG32(&g_pSdrcRestore->ACTIM_CTRLA_0,
        INREG32(&g_pSDRCRegs->SDRC_ACTIM_CTRLA_0)
        );

    OUTREG32(&g_pSdrcRestore->ACTIM_CTRLB_0,
        INREG32(&g_pSDRCRegs->SDRC_ACTIM_CTRLB_0)
        );

    OUTREG32(&g_pSdrcRestore->RFR_CTRL_0,
        INREG32(&g_pSDRCRegs->SDRC_RFR_CTRL_0)
        );

    OUTREG32(&g_pSdrcRestore->MCFG_1,
        INREG32(&g_pSDRCRegs->SDRC_MCFG_1)
        );

    OUTREG16(&g_pSdrcRestore->MR_1,
        INREG32(&g_pSDRCRegs->SDRC_MR_1)
        );

    OUTREG32(&g_pSdrcRestore->ACTIM_CTRLA_1,
        INREG32(&g_pSDRCRegs->SDRC_ACTIM_CTRLA_1)
        );

    OUTREG32(&g_pSdrcRestore->ACTIM_CTRLB_1,
        INREG32(&g_pSDRCRegs->SDRC_ACTIM_CTRLB_1)
        );

    OUTREG32(&g_pSdrcRestore->RFR_CTRL_1,
        INREG32(&g_pSDRCRegs->SDRC_RFR_CTRL_1)
        );

    OUTREG16(&g_pSdrcRestore->EMR1_0, 0);
    OUTREG16(&g_pSdrcRestore->EMR2_0, 0);
    OUTREG16(&g_pSdrcRestore->EMR3_0, 0);
    OUTREG16(&g_pSdrcRestore->EMR1_1, 0);
    OUTREG16(&g_pSdrcRestore->EMR2_1, 0);
    OUTREG16(&g_pSdrcRestore->EMR3_1, 0);
    OUTREG16(&g_pSdrcRestore->DCDL_1_CTRL, 0);
    OUTREG16(&g_pSdrcRestore->DCDL_2_CTRL, 0);
}

//-----------------------------------------------------------------------------
//
//  Function:  PrcmInit
//
//  Initializes the prcm module out of reset
//
void
PrcmInit(
    PrcmInitInfo   *pInfo
    )
{
    OALMSG(OAL_FUNC, (L"+PrcmInit()\r\n"));

    // initialize global pointer to the PRCM registers
    g_pPrcmPrm  = pInfo->pPrcmPrm;
    g_pPrcmCm   = pInfo->pPrcmCm;

    // initialize all internal data structures
    ResetInitialize();
    DomainInitialize();
    ClockInitialize();
    DeviceInitialize();

    OALMSG(OAL_FUNC, (L"-PrcmInit()\r\n"));
}

//-----------------------------------------------------------------------------
//
//  Function:  PrcmPostInit
//
//  Initializes the prcm module out of reset
//
void
PrcmPostInit()
{
    int i;
    OALMSG(OAL_FUNC, (L"+PrcmPostInit()\r\n"));

    // initialize synchronization objects
    for (i = 0; i < Mutex_Count; ++i)
        {
        InitializeCriticalSection(&g_rgPrcmMutex[i]);
        }

    // update flag indicating PRCM library is fully initialized
    g_PrcmPostInit = TRUE;

    OALMSG(OAL_FUNC, (L"-PrcmPostInit()\r\n"));
};

//-----------------------------------------------------------------------------
//
//  Function:  PrcmContextRestoreInit
//
//  Initializes the context restore registers
//
void
PrcmContextRestoreInit()
{
    // store the oem context restore address
    if (g_pContextRestore == NULL) return;

    OUTREG32(&g_pContextRestore->BOOT_CONFIG_ADDR, 0);
    OUTREG32(&g_pContextRestore->PUBLIC_RESTORE_ADDR, OALVAtoPA(OALCPURestoreContext));
    OUTREG32(&g_pContextRestore->SECURE_SRAM_RESTORE_ADDR, 0);
    OUTREG32(&g_pContextRestore->SDRC_MODULE_SEMAPHORE, 0);
    OUTREG32(&g_pContextRestore->OEM_CPU_INFO_DATA_PA, dwOEMMPUContextRestore);
    OUTREG32(&g_pContextRestore->OEM_CPU_INFO_DATA_VA, g_pCPUInfo);
    OUTREG32(&g_pContextRestore->PRCM_BLOCK_OFFSET, 0);
    OUTREG32(&g_pContextRestore->SDRC_BLOCK_OFFSET, 0);

    if (g_pPrcmRestore != NULL)
        {
        OUTREG32(&g_pContextRestore->PRCM_BLOCK_OFFSET,
            (UINT)g_pPrcmRestore - (UINT)g_pContextRestore
            );
        OALSavePrcmContext();
        }

    if (g_pSdrcRestore != NULL)
        {
        OUTREG32(&g_pContextRestore->SDRC_BLOCK_OFFSET,
            (UINT)g_pSdrcRestore - (UINT)g_pContextRestore
            );
        OALSaveSdrcContext();
        }
}

//-----------------------------------------------------------------------------
//
//  Function:  PrcmContextRestore
//
//  Performs the necessary steps to restore from CORE OFF
//
void
PrcmContextRestore()
{
    memcpy(fnCpuStart, OALCPUStart, (UINT)OALCPUEnd - (UINT)OALCPUStart);

    OUTREG32(&g_pPrcmCm->pOMAP_CORE_CM->CM_CLKSTCTRL_CORE,
        INREG32(&g_pPrcmRestore->CM_CLKSTCTRL_CORE)
        );

    OUTREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSTCTRL_MPU,
        INREG32(&g_pPrcmRestore->CM_CLKSTCTRL_MPU)
        );

   OUTREG32(&g_pPrcmCm->pOMAP_WKUP_CM->CM_CLKSEL_WKUP,
        INREG32(&g_pPrcmRestore->CM_CLKSEL_WKUP)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  PrcmSuspend()
//
//  Performs the necessary steps to enter full retention and then wake-up
//  from full retention.  This routine only sets up the necessary clocks
//  to try to enter full retention.  The external preconditions for full
//  retentions must be met for full retention to be entered.
//
void
PrcmSuspend()
{
    UINT tcrrExit;
    DWORD latencyState;
    UINT32 prevCoreState;
    UINT32 prevMpuState;
    UINT32 prevPerState;
    UINT32 prevMpuPerNeonDssState;
    UINT32 prevCoreCamUsbSgxState;
    int i;
    
    //--------------------------------------------------------------------------
    // perform power down sequence
    //--------------------------------------------------------------------------

    // Disable match interrupt
    OUTREG32(&g_pTimerRegs->TIER, 0);

    PrcmDeviceEnableClocksKernel(OMAP_DEVICE_GPTIMER1, FALSE);

    OUTREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->PM_PREPWSTST_MPU, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_PREPWSTST_CORE, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PREPWSTST_PER, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_NEON_PRM->PM_PREPWSTST_NEON, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_DSS_PRM->PM_PREPWSTST_DSS, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_CAM_PRM->PM_PREPWSTST_CAM, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->PM_PREPWSTST_USBHOST, 0);
    OUTREG32(&g_pPrcmPrm->pOMAP_SGX_PRM->PM_PREPWSTST_SGX, 0);

    // use the latency module to transition to a valid sleep state
    latencyState = OALWakeupLatency_GetSuspendState();
    if (OALWakeupLatency_IsChipOff(latencyState))
        {
        OALContextSave();
        }
    OALWakeupLatency_PushState(latencyState);

    // clear wakeup status
    OUTREG32(&g_pPrcmPrm->pOMAP_WKUP_PRM->PM_WKST_WKUP, CM_CLKEN_IO);

    // Enable the IO pad wakeup
    SETREG32(&g_pPrcmPrm->pOMAP_WKUP_PRM->PM_WKEN_WKUP, CM_CLKEN_IO);
    
    PrcmDeviceEnableClocksKernel(OMAP_DEVICE_GPIO1, FALSE);

    #define GPIO2_6_CLK_BITS    (0x0003e000)

    // save and disable GPIO clocks
    prevGpioIClkState = INREG32(&(OMAP_CM_REGS*)g_pPrcmCm->pOMAP_PER_CM->CM_ICLKEN_PER);
    prevGpioFClkState = INREG32(&(OMAP_CM_REGS*)g_pPrcmCm->pOMAP_PER_CM->CM_FCLKEN_PER);
    OUTREG32(&(OMAP_CM_REGS*)g_pPrcmCm->pOMAP_PER_CM->CM_ICLKEN_PER, prevGpioIClkState & ~(GPIO2_6_CLK_BITS));
    OUTREG32(&(OMAP_CM_REGS*)g_pPrcmCm->pOMAP_PER_CM->CM_FCLKEN_PER, prevGpioFClkState & ~(GPIO2_6_CLK_BITS));

#ifndef SHIP_BUILD
    if (g_PrcmDebugSuspendResume)
    {
        OALWakeupLatency_SaveSnapshot();
        PrcmSaveRefCounts();
        PrcmRegsSnapshot();
    }
#endif
        
    // Move SoC/CPU to idle mode (suspend)
    fnOALCPUIdle(g_pCPUInfo);

    // resume starts here...

    // restore GPIO clocks
    OUTREG32(&(OMAP_CM_REGS*)g_pPrcmCm->pOMAP_PER_CM->CM_FCLKEN_PER, 
        (prevGpioFClkState & GPIO2_6_CLK_BITS) | INREG32(&(OMAP_CM_REGS*)g_pPrcmCm->pOMAP_PER_CM->CM_FCLKEN_PER));
    OUTREG32(&(OMAP_CM_REGS*)g_pPrcmCm->pOMAP_PER_CM->CM_ICLKEN_PER, 
        (prevGpioIClkState & GPIO2_6_CLK_BITS) | INREG32(&(OMAP_CM_REGS*)g_pPrcmCm->pOMAP_PER_CM->CM_ICLKEN_PER));
    
    PrcmDeviceEnableClocksKernel(OMAP_DEVICE_GPIO1, TRUE);

    // Clear IO pad wakeup
    CLRREG32(&g_pPrcmPrm->pOMAP_WKUP_PRM->PM_WKEN_WKUP, CM_CLKEN_IO);

    OALWakeupLatency_PopState();

    prevMpuState = INREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->PM_PREPWSTST_MPU);
    prevCoreState = INREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_PREPWSTST_CORE);
    prevPerState = INREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PREPWSTST_PER);

    prevMpuPerNeonDssState =  (prevMpuState & 0xFF);
    prevMpuPerNeonDssState |= ((INREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PREPWSTST_PER) & 0xFF) << 8);    
    prevMpuPerNeonDssState |= ((INREG32(&g_pPrcmPrm->pOMAP_NEON_PRM->PM_PREPWSTST_NEON) & 0xFF) << 16);    
    prevMpuPerNeonDssState |= ((INREG32(&g_pPrcmPrm->pOMAP_DSS_PRM->PM_PREPWSTST_DSS) & 0xFF) << 24);    

    prevCoreCamUsbSgxState = (prevCoreState & 0xFF);
    prevCoreCamUsbSgxState |= ((INREG32(&g_pPrcmPrm->pOMAP_CAM_PRM->PM_PREPWSTST_CAM) & 0xFF) << 8);    
    prevCoreCamUsbSgxState |= ((INREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->PM_PREPWSTST_USBHOST) & 0xFF) << 16);    
    prevCoreCamUsbSgxState |= ((INREG32(&g_pPrcmPrm->pOMAP_SGX_PRM->PM_PREPWSTST_SGX) & 0xFF) << 24);    
    
    OALContextRestore(prevMpuState, prevCoreState, prevPerState);

    // Clear IO pad wakeup
    CLRREG32(&g_pPrcmPrm->pOMAP_WKUP_PRM->PM_WKEN_WKUP, CM_CLKEN_IO);

    // restart GPTIMER1
    PrcmDeviceEnableClocksKernel(OMAP_DEVICE_GPTIMER1, TRUE);
    OUTREG32(&g_pTimerRegs->TCLR, INREG32(&g_pTimerRegs->TCLR) | GPTIMER_TCLR_ST);
    for (i = 0; i < 0x100; i++)
    {
    }

    // get current TCRR value: workaround for errata 1.35
    INREG32(&g_pTimerRegs->TCRR);
    tcrrExit = INREG32(&g_pTimerRegs->TCRR);

    PrcmProcessPostMpuWakeup();

    OALLED(LED_IDX_MPU_PREV_STATE, INREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->PM_PREPWSTST_MPU));
    OALLED(LED_IDX_CORE_PREV_STATE, INREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->PM_PREPWSTST_CORE));
    OALLED(LED_IDX_PER_PREV_STATE, INREG32(&g_pPrcmPrm->pOMAP_PER_PRM->PM_PREPWSTST_PER));
    OALLED(LED_IDX_CORE_CAM_USB_SGX_PREV_STATE, prevCoreCamUsbSgxState);
    OALLED(LED_IDX_MPU_PER_NEON_DSS_PREV_STATE, prevMpuPerNeonDssState);

    // Enable match interrupt
    OUTREG32(&g_pTimerRegs->TIER, GPTIMER_TIER_MATCH);

    // ERRATA 1.31 workaround (ES 1.0 only)
    // wait for updated TCRR value
    while (tcrrExit == (INREG32(&g_pTimerRegs->TCRR)));

    // UNDONE:
    //   Need to update curridlehigh and curridlelow to track cpu loads
}

//-----------------------------------------------------------------------------

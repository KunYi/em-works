// All rights reserved ADENEO EMBEDDED 2010
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
#include "omap.h"
#include "am387x.h"
#include "am387x_oal_prcm.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"
//#include "interrupt_struct.h"
//-----------------------------------------------------------------------------
//
//  Used for debugging suspend/resume
//
#if 1

extern VOID PrcmSaveRefCounts();
extern void PrcmRegsSnapshot();
extern BOOL g_PrcmDebugSuspendResume;

//-----------------------------------------------------------------------------
//  Global:  g_pIntr
//  Reference to all interrupt related registers. Initialized in OALIntrInit
//extern OMAP_INTR_CONTEXT const *g_pIntr;


//  Used for save/restore of GPIO clock state
//DWORD prevGpioIClkState;
//DWORD prevGpioFClkState;

//-----------------------------------------------------------------------------
//  function which returns the start of the MMU L1 table
extern DWORD OALGetTTBR();

//-----------------------------------------------------------------------------
//  function which Invalidates the TLB
extern VOID OALInvalidateTlb();

//-----------------------------------------------------------------------------
//  Global:  dwOEMSRAMSaveAddr
//  Secure SRAM Save location in SDRAM
extern DWORD dwOEMSRAMSaveAddr;

//-----------------------------------------------------------------------------
//  Reference to all PRCM-registers. Initialized in PrcmInit
AM387X_PRCM_REGS        *g_pPrcmRegs;
AM387X_PLLSS_REGS		*g_pPllssRegs;

//------------------------------------------------------------------------------
//
//  Global:  g_pSysCtrlGenReg
//
//  reference to system control general register set
//
//OMAP_SYSC_GENERAL_REGS     *g_pSysCtrlGenReg = NULL;

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
void PrcmCapturePrevPowerState()
{
    // Stub, nothing to do on AM3517
}

//-----------------------------------------------------------------------------
void PrcmProfilePrevPowerState(DWORD timer_val, DWORD wakeup_delay )
{
    UNREFERENCED_PARAMETER(timer_val);
    UNREFERENCED_PARAMETER(wakeup_delay);
    // Stub, nothing to do on AM3517
}

//-----------------------------------------------------------------------------
void
PrcmInitializePrevPowerState()
{
    // Stub, nothing to do on AM3517
}

//-----------------------------------------------------------------------------
UINT PrcmInterruptEnable(UINT mask, BOOL bEnable )
{
    UINT val = 0;
    Lock(Mutex_Intr);

OALMSG(1, (L"+PrcmInterruptEnable() mask %08x, bEnable %d\r\n", mask, bEnable));
/*
    // enable/disable prcm interrupts
    val = INREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQENABLE_MPU);
    val = (bEnable != FALSE) ? (val | mask) : (val & ~mask);
    OUTREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQENABLE_MPU, val);
*/
    Unlock(Mutex_Intr);
    return val;
}

//-----------------------------------------------------------------------------
UINT PrcmInterruptClearStatus(UINT mask)
{
    UINT val = 0;

    // This routine should only be called during system boot-up or
    // from OEMIdle.  Hence, serialization within this routine
    // should not be performed.

    // clear prcm interrupt status
OALMSG(1, (L"+PrcmInterruptClearStatus() mask %08x\r\n", mask));
    // return the status prior to clearing the status
    return val;
}

static void ClearXNBit( void *pvAddr )
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
//    pInvalidateTlb  fnInvalidateTlb;

    OALMSG(OAL_FUNC, (L"+OALSRAMFnInit()\r\n"));

#if 0
#pragma warning (push)
#pragma warning (disable:4152) //disable warning that prevents using function pointers as data pointers
#endif

OALMSG(1, (L"-OALSRAMFnInit(): Check for OMAP3530 implementation if we need to port it to here\r\n"));

#if 0
#pragma warning (pop)
#endif

    OALMSG(OAL_FUNC, (L"-OALSRAMFnInit()\r\n"));

    return TRUE;
}

//-----------------------------------------------------------------------------
void OALSDRCRefreshCounter(UINT highFreq, UINT lowFreq  )
//	Sets the SDRC Refresh counter
{
OALMSG(1, (L"OALSDRCRefreshCounter(): %u %u\r\n", highFreq, lowFreq));

//    g_pCPUInfo->SDRC_HIGH_RFR_FREQ = highFreq;
//    g_pCPUInfo->SDRC_LOW_RFR_FREQ = lowFreq;
}

void OALSavePrcmContext(){}
void OALSaveSdrcContext(){}

//-----------------------------------------------------------------------------
void PrcmInit()
//	Initializes the prcm module out of reset
{
    OALMSG(OAL_FUNC, (L"+PrcmInit()\r\n"));

	g_pPrcmRegs  = OALPAtoUA(AM387X_PRCM_REGS_PA);
	g_pPllssRegs = OALPAtoUA(AM387X_PLLSS_REGS_PA);

    // initialize all internal data structures
    ResetInitialize();
    DomainInitialize();
    ClockInitialize();
    DeviceInitialize();

    OALMSG(OAL_FUNC, (L"-PrcmInit()\r\n"));
}

//-----------------------------------------------------------------------------
void PrcmPostInit()
//	Initializes the prcm module out of reset
{
    int i;
    OALMSG(1/*OAL_FUNC*/, (L"+PrcmPostInit()\r\n"));

    for (i = 0; i < Mutex_Count; ++i) // initialize synchronization objects
        InitializeCriticalSection(&g_rgPrcmMutex[i]);

    g_PrcmPostInit = TRUE; // update flag indicating PRCM library is fully initialized

    OALMSG(OAL_FUNC, (L"-PrcmPostInit()\r\n"));
};

//-----------------------------------------------------------------------------
//	Initializes the context restore registers
void PrcmContextRestoreInit(){}
//-----------------------------------------------------------------------------
//	Performs the necessary steps to restore from CORE OFF
void PrcmContextRestore(){}

void PrcmClearContextRegisters() {}

//------------------------------------------------------------------------------
//
//  Function:  PrcmSuspend()
//
//  Performs the necessary steps to enter full retention and then wake-up
//  from full retention.  This routine only sets up the necessary clocks
//  to try to enter full retention.  The external preconditions for full
//  retentions must be met for full retention to be entered.
//
void PrcmSuspend()
{
// we don't do anything here for Netra
}

//------------------------------------------------------------------------------
BOOL OALIoCtlPrcmDeviceGetDeviceManagementTable(
    UINT32 code, VOID  *pInBuffer,     UINT32 inSize, 
    VOID  *pOutBuffer, UINT32 outSize, UINT32 *pOutSize
    )
//	returns a table referencing the device clock management functions
{
    BOOL rc = FALSE;
    OMAP_DEVCLKMGMT_FNTABLE *pfnTbl;

    UNREFERENCED_PARAMETER(inSize);
    UNREFERENCED_PARAMETER(pInBuffer);
    UNREFERENCED_PARAMETER(code);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlPrcmDeviceGetDeviceManagementTable\r\n"));
    if (pOutBuffer == NULL || outSize < sizeof(OMAP_DEVCLKMGMT_FNTABLE))
        {
        goto cleanUp;
        }

    // update return information
    //
    if (pOutSize != NULL) *pOutSize = sizeof(OMAP_DEVCLKMGMT_FNTABLE);

    // update function pointers
    //
    pfnTbl = (OMAP_DEVCLKMGMT_FNTABLE*)pOutBuffer;    
    pfnTbl->pfnEnableDeviceClocks= PrcmDeviceEnableClocks;
    pfnTbl->pfnEnableDeviceIClock= PrcmDeviceEnableIClock;
    pfnTbl->pfnEnableDeviceFClock= PrcmDeviceEnableFClock;
    pfnTbl->pfnSetSourceDeviceClocks = PrcmDeviceSetSourceClocks;
    pfnTbl->pfnEnableDeviceClockAutoIdle = PrcmDeviceEnableAutoIdle;
    pfnTbl->pfnGetDeviceContextState = PrcmDeviceGetContextState;
    pfnTbl->pfnUpdateOnDeviceStateChange = OALMux_UpdateOnDeviceStateChange;
    pfnTbl->pfnGetSystemClockFrequency = PrcmClockGetSystemClockFrequency;
	pfnTbl->pfnPrcmDomainResetRelease  = PrcmDomainResetRelease;
	pfnTbl->pfnPrcmDomainReset         = PrcmDomainReset;
	pfnTbl->pfnPrcmDomainResetStatus   = PrcmDomainResetStatus; 
    pfnTbl->pfnPrcmClockGetClockRate       = PrcmClockGetClockRate;     
    rc = TRUE;

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIoCtlPrcmDeviceGetDeviceManagementTable(rc = %d)\r\n", rc));
    return rc;
}

#endif
//-----------------------------------------------------------------------------

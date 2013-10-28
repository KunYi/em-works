// All rights reserved ADENEO EMBEDDED 2010
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  am387x_oal_prcm.h
//
#ifndef _AM387X_OAL_PRCM_H
#define _AM387X_OAL_PRCM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "am387x_prcm.h"
#include <oalex.h>

//-----------------------------------------------------------------------------
typedef struct {
    UINT                    clockId;
    LONG                    refCount;
    UINT                    nLevel;
} SourceClockInfo_t;

// I don'n know what is that for yet
typedef enum {
    kVoltageProcessor1 = 0,
    kVoltageProcessor2,
    kVoltageProcessorCount
}
VoltageProcessor_e;


//-----------------------------------------------------------------------------
//
//  Function: PRCM power functions
//
void PrcmInit();
void PrcmPostInit();
void PrcmSuspend();
BOOL PrcmRestoreDomain( UINT powerDomain );
void PrcmContextRestore();
void PrcmContextRestoreInit();
void PrcmCapturePrevPowerState();
void PrcmProfilePrevPowerState(DWORD timer_val, DWORD wakeup_delay );    
void PrcmInitializePrevPowerState();
UINT PrcmInterruptClearStatus(UINT mask);
UINT PrcmInterruptProcess(UINT mask);
UINT PrcmInterruptEnable(UINT mask, BOOL bEnable);
BOOL PrcmClockSetParent(UINT clockId, UINT newParentClockId);
BOOL PrcmClockSetDivisor(UINT clockId, UINT parentClockId, UINT divisor);
BOOL PrcmClockSetSystemClockSetupTime(USHORT setupTime);
BOOL PrcmClockSetDpllFrequency( UINT dpllId, UINT m, UINT n, UINT freqSel, UINT outputDivisor);
BOOL PrcmDeviceEnableClocks( UINT clkId, BOOL bEnable);
BOOL PrcmDeviceEnableClocksKernel(UINT clkId, BOOL bEnable);
BOOL PrcmDeviceEnableIClock(UINT clkId, BOOL bEnable);
BOOL PrcmDeviceEnableFClock(UINT clkId, BOOL bEnable);
BOOL PrcmDeviceGetEnabledState(UINT clkId, BOOL *pbEnable);
BOOL PrcmDeviceEnableAutoIdle(UINT clkId,  BOOL bEnable);
BOOL PrcmDeviceGetAutoIdleState(UINT clkId, BOOL *pbEnable);
BOOL PrcmDeviceGetSourceClockInfo(UINT devId,
		IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO_OUT *pInfo);

BOOL PrcmDeviceSetSourceClocks(UINT devId, UINT count, UINT rgClocks[]);
BOOL PrcmDeviceGetContextState(UINT devId, BOOL bSet);
UINT PrcmClockGetDpllState(UINT dpllId);
BOOL PrcmClockGetParentClockInfo(UINT clockId, UINT nLevel, SourceClockInfo_t *pInfo);
BOOL PrcmClockGetParentClockRefcount(UINT clockId, UINT nLevel, LONG *pRefCount);
float PrcmClockGetSystemClockFrequency();
BOOL PrcmClockSetExternalClockRequestMode(UINT extClkReqMode);
BOOL PrcmClockSetDpllState(IOCTL_PRCM_CLOCK_SET_DPLLSTATE_IN *pInfo);

BOOL PrcmClockSetDpllAutoIdleState(UINT dpllId, UINT dpllAutoidleState);
VOID PrcmClockRestoreDpllState(UINT dpll);
UINT32 PrcmClockGetClockRate(OMAP_CLOCKID clock_id);
BOOL PrcmDomainSetClockState(UINT powerDomain, UINT clockDomain, UINT clockState);
BOOL PrcmDomainSetClockStateKernel(UINT powerDomain, UINT clockDomain, UINT clockState);
BOOL PrcmDomainSetPowerState(UINT powerDomain, UINT powerState, UINT logicState);
BOOL PrcmDomainSetSleepDependency( UINT powerDomain, UINT ffDependency, BOOL bEnable);
BOOL PrcmDomainSetWakeupDependency(UINT powerDomain, UINT ffDependency, BOOL bEnable);
BOOL PrcmDomainSetMemoryState(UINT powerDomain, UINT memoryState, UINT memoryStateMask);
BOOL PrcmDomainResetStatus(UINT powerDomain, UINT *pResetStatus, BOOL bClear);
BOOL PrcmDomainClearWakeupStatus(UINT powerDomain );
BOOL PrcmDomainReset(UINT powerDomain, UINT resetMask);
BOOL PrcmDomainResetRelease(UINT powerDomain, UINT resetMask);
void PrcmDomainUpdateRefCount(UINT powerDomain, UINT bEnable);
BOOL PrcmDomainSetPowerStateInternal(UINT powerDomain, UINT powerState, UINT logicState, BOOL bNotify);
void PrcmVoltSetControlMode(UINT voltCtrlMode, UINT voltCtrlMask);
void PrcmVoltSetControlPolarity(UINT polMode, UINT polModeMask);
void PrcmVoltSetAutoControl(UINT autoCtrlMode, UINT autoCtrlMask);
void PrcmVoltI2cInitialize(VoltageProcessor_e vp, UINT8 slaveAddr,
                      UINT8 cmdAddr, UINT8 voltAddr, BOOL bUseCmdAddr);
void PrcmVoltI2cSetHighSpeedMode(BOOL bHSMode, BOOL bRepeatStartMode, UINT8 mcode);
void PrcmVoltInitializeVoltageLevels(VoltageProcessor_e vp, UINT vddOn, UINT vddOnLP,
                                     UINT vddRetention, UINT vddOff );
void PrcmVoltSetErrorConfiguration(VoltageProcessor_e vp, UINT errorOffset, UINT errorGain);
void PrcmVoltSetSlewRange(VoltageProcessor_e vp, UINT minVStep,
                          UINT minWaitTime, UINT maxVStep, UINT maxWaitTime);
void PrcmVoltSetLimits(VoltageProcessor_e vp, UINT minVolt, UINT maxVolt, UINT timeOut);
void PrcmVoltSetVoltageLevel(VoltageProcessor_e vp, UINT vdd, UINT mask );
void PrcmVoltSetInitVddLevel(VoltageProcessor_e vp, UINT initVolt);
void PrcmVoltEnableTimeout(VoltageProcessor_e vp, BOOL bEnable );
void PrcmVoltEnableVp(VoltageProcessor_e vp, BOOL bEnable );
void PrcmVoltFlushVoltageLevels(VoltageProcessor_e vp);
BOOL PrcmVoltIdleCheck(VoltageProcessor_e vp);
UINT PrcmVoltGetVoltageRampDelay(VoltageProcessor_e vp);
void PrcmProcessPostMpuWakeup();
void PrcmDomainClearReset();

#ifdef __cplusplus
}
#endif

#endif

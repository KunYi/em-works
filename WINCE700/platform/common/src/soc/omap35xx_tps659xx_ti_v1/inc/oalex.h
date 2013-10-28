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
//  This file contains OMAP specific oal extensions.
//
#ifndef __OALEX_H
#define __OALEX_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
//
//  Valid parameters for dwOEMTargetProject
//
#define OEM_TARGET_PROJECT_CEBASE       1
#define OEM_TARGET_PROJECT_SMARTFON     2
#define OEM_TARGET_PROJECT_WPC          3

//-----------------------------------------------------------------------------
//
//  Valid parameters for dwOEMHighSecurity
//
#define OEM_HIGH_SECURITY_HS            1
#define OEM_HIGH_SECURITY_GP            2

//-----------------------------------------------------------------------------
//
//  Define:  OAL_ARG_QUERY_xxx
//
#define OAL_ARGS_QUERY_COLDBOOT         (BSP_ARGS_QUERY + 0)
#define OAL_ARGS_QUERY_HWENTROPY        (BSP_ARGS_QUERY + 1)
#define OAL_ARGS_QUERY_RNDISMAC         (BSP_ARGS_QUERY + 2)
#define OAL_ARGS_QUERY_DEVICEID         (BSP_ARGS_QUERY + 3)

//-----------------------------------------------------------------------------
//
//  minimum number of 32khz cycles to wait after SDRC DLL change
//
#define DLL_CHANGE_DELAY                (6)

//-----------------------------------------------------------------------------
//
//  maximum number of domains which are DVFS capable, set to high value for
//  future compatability
//
#define MAX_DVFS_DOMAINS                (8)

//-----------------------------------------------------------------------------
//
//  Custom cache sync value to invalidate only data cache
//
#define TI_CACHE_SYNC_INVALIDATE        0x80000000

//-----------------------------------------------------------------------------
//
//  Custom CeSetMemoryAttributes
//
#define TI_PAGE_NOCACHE                 (PAGE_NOCACHE)
#define TI_PAGE_WRITECOMBINE            (PAGE_WRITECOMBINE)
#define TI_PAGE_WRITETHROUGH            (0x80000801)
#define TI_PAGE_WRITEBACK               (0x80000802)

#ifndef PAGE_WRITETHROUGH
#define PAGE_WRITETHROUGH               (0x100000)
#endif



//-----------------------------------------------------------------------------
//
//  Extern:  g_oalAddressTable 
//
//  This table is used by kernel to establish the virtual to physical address
//  mapping. It is alos used in OALPAtoVA and OALVAtoPA functions to allow
//  get virtual address for physical and vice versa.
//
extern OAL_ADDRESS_TABLE g_oalAddressTable[];

//-----------------------------------------------------------------------------
//
//  Global:  dwOEMTargetProject
//
//  This global variable specify target BSP project. It is used to modify
//  code behavior on different project types. Variable is modified by FIXUP
//  statement in config.bib.
//
extern const volatile DWORD dwOEMTargetProject;

//-----------------------------------------------------------------------------
//
//  Global:  dwOEMHighSecurity
//
//  This global variable to distinguish HS and GP devices. It is used to modify
//  code behavior on different security types. Variable is modified by FIXUP
//  statement in config.bib.
//
extern const volatile DWORD dwOEMHighSecurity;

//-----------------------------------------------------------------------------
//
//  Globals:  g_oalProfilerEnabled
//            g_oalPerfTimerIrq  
//
//  Profiler timer enabled flag and IRQ
//
extern BOOL   g_oalProfilerEnabled;
extern UINT32 g_oalPerfTimerIrq;

//-----------------------------------------------------------------------------
// Power related IOCTL's
#define FILE_DEVICE_PRCM            (501)
#define FILE_DEVICE_OMAPPOWEREXT    (601)
#define FILE_DEVICE_SMARTREFLEX     (701) // UNDONE: smartreflex should be autonomous

//-----------------------------------------------------------------------------
//
//  Define: INTERRUPTS_STATUS
//
//  returns if ARM interrupts are enabled or not
//
BOOL
INTERRUPTS_STATUS();

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_SMARTREFLEX_CONTROL
//
//  This code is used to enable/disable smartreflex
//
#define IOCTL_SMARTREFLEX_CONTROL \
    CTL_CODE(FILE_DEVICE_SMARTREFLEX, 1001, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlSmartReflexControl(
    UINT32 code, 
    VOID  *pInBuffer,
    UINT32 inSize, 
    VOID  *pOutBuffer, 
    UINT32 outSize, 
    UINT32*pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_DEVICE_ENABLE_IN
// 
//  Generic structure used to enable/disable a particular functionality
//  for a given device
//
typedef struct {
    GUID               guid;
    UINT               devId;  
    BOOL               bEnable;
} IOCTL_PRCM_DEVICE_ENABLE_IN;

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_INTERRUPT_LATENCY_CONSTRAINT
//
//  This code is used to set a constraint on the cpu idle code to what the
//  minimal wake-up latency is.
//
#define IOCTL_INTERRUPT_LATENCY_CONSTRAINT \
    CTL_CODE(FILE_DEVICE_OMAPPOWEREXT, 1001, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlInterruptLatencyConstraint(
    UINT32 code, 
    VOID  *pInBuffer,
    UINT32 inSize, 
    VOID  *pOutBuffer, 
    UINT32 outSize, 
    UINT32*pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_DEVICE_GET_DEVICEMANAGEMENTTABLE
//
//  This code is used to retrieve the ref count of the core clocks
//
#define IOCTL_PRCM_DEVICE_GET_DEVICEMANAGEMENTTABLE    \
    CTL_CODE(FILE_DEVICE_PRCM, 1001, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    BOOL (*pfnEnableDeviceIClock)(UINT clkId, BOOL bEnable);
    BOOL (*pfnEnableDeviceFClock)(UINT clkId, BOOL bEnable);
    BOOL (*pfnEnableDeviceClockAutoIdle)(UINT clkId, BOOL bEnable);
    BOOL (*pfnSetSourceDeviceClocks)(UINT devId, UINT count, UINT rgSourceClocks[]);
    BOOL (*pfnGetDeviceContextState)(UINT devId, BOOL bSet);
    DWORD (*pfnUpdateOnDeviceStateChange)(UINT devId, UINT oldState, UINT newState, BOOL bPreSetChange);
} OMAP_DEVCLKMGMT_FNTABLE;

BOOL 
OALIoCtlPrcmDeviceGetDeviceManagementTable(
    UINT32 code, 
    VOID  *pInBuffer,
    UINT32 inSize, 
    VOID  *pOutBuffer, 
    UINT32 outSize, 
    UINT32*pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_DEVICE_SET_AUTOIDLESTATE
//
//  This code is used to enable/disable device autoidle
//
#define IOCTL_PRCM_DEVICE_SET_AUTOIDLESTATE    \
    CTL_CODE(FILE_DEVICE_PRCM, 1002, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlPrcmDeviceSetAutoIdleState(
    UINT32 code, 
    VOID  *pInBuffer,
    UINT32 inSize, 
    VOID  *pOutBuffer, 
    UINT32 outSize, 
    UINT32*pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_CLOCK_GET_SOURCECLOCKINFO
//
//  Given a device id, information about the device's source clock will be 
//  returned
//
#define IOCTL_PRCM_CLOCK_GET_SOURCECLOCKINFO    \
    CTL_CODE(FILE_DEVICE_PRCM, 1003, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Level definition:
//  1 = Source clock
//  2 = dpll clock out
//  3 = dpll
//  4 = Vdd
//
typedef struct {
    UINT                clockId;
    DWORD               clockLevel;
} IOCTL_PRCM_CLOCK_GET_SOURCECLOCKINFO_IN;

typedef struct {
    DWORD               refCount;    
    UINT                parentId;
    UINT                parentLevel;
    UINT                parentRefCount;
} IOCTL_PRCM_CLOCK_GET_SOURCECLOCKINFO_OUT;

BOOL 
OALIoCtlPrcmClockGetSourceClockInfo(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO
//
//  Given a device id, information about the device's source clock will be 
//  returned
//
#define IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO    \
    CTL_CODE(FILE_DEVICE_PRCM, 1004, METHOD_BUFFERED, FILE_ANY_ACCESS)
    
typedef struct {
    UINT                count;
    struct {
        UINT                clockId;
        UINT                nLevel;
        UINT                refCount;    
    }                   rgSourceClocks[3];
} IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO_OUT;

BOOL 
OALIoCtlPrcmDeviceGetSourceClockInfo(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_DEVICE_GET_DEVICESTATUS
//
//  retrieves the current status of the device.
//
#define IOCTL_PRCM_DEVICE_GET_DEVICESTATUS    \
    CTL_CODE(FILE_DEVICE_PRCM, 1005, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    BOOL            bEnabled;
    BOOL            bAutoIdle;
} IOCTL_PRCM_DEVICE_GET_DEVICESTATUS_OUT;
    
BOOL 
OALIoCtlPrcmDeviceGetDeviceStatus(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_CLOCK_SET_SOURCECLOCK
//
//  changes the source clock of a given functional clock.
//
#define IOCTL_PRCM_CLOCK_SET_SOURCECLOCK    \
    CTL_CODE(FILE_DEVICE_PRCM, 1006, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    UINT            clkId;
    UINT            newParentClkId;
} IOCTL_PRCM_CLOCK_SET_SOURCECLOCK_IN;
    
BOOL 
OALIoCtlPrcmClockSetSourceClock(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_CLOCK_SET_SOURCECLOCKDIVISOR
//
//  changes the source clock of a given functional clock.
//
#define IOCTL_PRCM_CLOCK_SET_SOURCECLOCKDIVISOR    \
    CTL_CODE(FILE_DEVICE_PRCM, 1007, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    UINT            clkId;
    UINT            parentClkId;
    UINT            divisor;
} IOCTL_PRCM_CLOCK_SET_SOURCECLOCKDIVISOR_IN;
    
BOOL 
OALIoCtlPrcmClockSetSourceClockDivisor(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_CLOCK_SET_DPLLSTATE
//
//  This code is used to update a dpll setting
//
#define IOCTL_PRCM_CLOCK_SET_DPLLSTATE    \
    CTL_CODE(FILE_DEVICE_PRCM, 1008, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    UINT            size;
    UINT            dpllId;
    UINT            ffMask;
    UINT            lowPowerEnabled;
    UINT            rampTime;    
    UINT            driftGuardEnabled;
    UINT            dpllMode;
    UINT            dpllAutoidleState;
} IOCTL_PRCM_CLOCK_SET_DPLLSTATE_IN;

BOOL 
OALIoCtlPrcmClockSetDpllState(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_DOMAIN_SET_WAKEUPDEP
//
//  This code is used to update a domains wake-up dependency
//
#define IOCTL_PRCM_DOMAIN_SET_WAKEUPDEP    \
    CTL_CODE(FILE_DEVICE_PRCM, 1009, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    UINT            size;
    UINT            powerDomain;
    UINT            ffWakeDep;
    BOOL            bEnable;
} IOCTL_PRCM_DOMAIN_SET_WAKEUPDEP_IN;

BOOL 
OALIoCtlPrcmDomainSetWakeupDependency(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_DOMAIN_SET_SLEEPDEP
//
//  This code is used to update a dpll setting
//
#define IOCTL_PRCM_DOMAIN_SET_SLEEPDEP    \
    CTL_CODE(FILE_DEVICE_PRCM, 1010, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    UINT            size;
    UINT            powerDomain;
    UINT            ffSleepDep;
    BOOL            bEnable;
} IOCTL_PRCM_DOMAIN_SET_SLEEPDEP_IN;

BOOL 
OALIoCtlPrcmDomainSetSleepDependency(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_DOMAIN_SET_POWERSTATE
//
//  This code is used to update a dpll setting
//
#define IOCTL_PRCM_DOMAIN_SET_POWERSTATE    \
    CTL_CODE(FILE_DEVICE_PRCM, 1011, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    UINT            size;
    UINT            powerDomain;
    UINT            powerState;
    UINT            logicState;
} IOCTL_PRCM_DOMAIN_SET_POWERSTATE_IN;

BOOL 
OALIoCtlPrcmDomainSetPowerState(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_SET_SUSPENDSTATE
//
//  Specifies to the kernel copy i2c function table
//
#define IOCTL_PRCM_SET_SUSPENDSTATE  \
    CTL_CODE(FILE_DEVICE_PRCM, 1012, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL
OALIoCtlPrcmSetSuspendState(
    UINT32 code,
    VOID *pInBuffer,
    UINT32 inSize,
    VOID *pOutBuffer,
    UINT32 outSize,
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_OPP_REQUEST
//
//  This code is used to change operating points
//
#define IOCTL_OPP_REQUEST    \
    CTL_CODE(FILE_DEVICE_PRCM, 1013, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    DWORD       size;           // mask of system power state
    DWORD       dwCount;
    DWORD       rgDomains[MAX_DVFS_DOMAINS];
    DWORD       rgOpps[MAX_DVFS_DOMAINS];
} IOCTL_OPP_REQUEST_IN;

BOOL 
OALIoCtlOppRequest(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_PRCM_DOMAIN_SET_CLOCKSTATE
//
//  This code is used to update a dpll setting
//
#define IOCTL_PRCM_DOMAIN_SET_CLOCKSTATE    \
    CTL_CODE(FILE_DEVICE_PRCM, 1014, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    UINT            size;
    UINT            powerDomain;
    UINT            clockDomain;
    UINT            clockState;
} IOCTL_PRCM_DOMAIN_SET_CLOCKSTATE_IN;

BOOL 
OALIoCtlPrcmDomainSetClockState(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_CONTEXTSAVE_GETBUFFER
//
//  This code is used to notify kernel a register set has been modified
//  and to save context before OFF mode
//
#define HAL_CONTEXTSAVE_GPIO            0x00000001
#define HAL_CONTEXTSAVE_SCM             0x00000002
#define HAL_CONTEXTSAVE_DMA             0x00000004
#define HAL_CONTEXTSAVE_GPMC            0x00000010
#define HAL_CONTEXTSAVE_INTC            0x00000020
#define HAL_CONTEXTSAVE_PRCM            0x00000040
#define HAL_CONTEXTSAVE_VRFB            0x00000080
#define HAL_CONTEXTSAVE_PINMUX          0x00000100
#define HAL_CONTEXTSAVE_SMS             0x00000200

#define IOCTL_HAL_CONTEXTSAVE_GETBUFFER \
    CTL_CODE(FILE_DEVICE_PRCM, 1015, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlHalContextSaveGetBuffer(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_POSTINIT
//
//  This code allows HAL to perform initialization after the kernel features
//  are enabled
//
BOOL 
OALIoCtlHALPostInit(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_OEM_PROFILER
//
//  This code enables/disables HAL specific profiling features
//
BOOL 
OALIoCtlHALProfiler(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_PROFILE
//
//  Deprecated IOCTL.  This handler is just to filter out the warnings.
//
BOOL 
OALIoCtlIgnore(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_IRQ2SYSINTR
//
//  This code is used to translate an IRQ to the SYSINTR
//
#define IOCTL_HAL_IRQ2SYSINTR  \
    CTL_CODE(FILE_DEVICE_HAL, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlHalIrq2Sysintr(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_GET_CPUID
//
//  This code is used to CPU CORE ID
//
#define IOCTL_HAL_GET_CPUID    \
    CTL_CODE(FILE_DEVICE_HAL, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlHalGetCpuID(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_GET_DIEID
//
//  This code is used to OMAP DIE ID
//
#define IOCTL_HAL_GET_DIEID    \
    CTL_CODE(FILE_DEVICE_HAL, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    DWORD   dwIdCode;
    DWORD   dwProdID_0;       
    DWORD   dwProdID_1;       
    DWORD   dwProdID_2;       
    DWORD   dwProdID_3;       
    DWORD   dwDieID_0;       
    DWORD   dwDieID_1;       
    DWORD   dwDieID_2;       
    DWORD   dwDieID_3;       
} IOCTL_HAL_GET_DIEID_OUT;

BOOL 
OALIoCtlHalGetDieID(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_GET_BSP_VERSION
//
//  Retrieve the BSP version
//
#define IOCTL_HAL_GET_BSP_VERSION  \
    CTL_CODE(FILE_DEVICE_HAL, 3100, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    DWORD   dwVersionMajor;
    DWORD   dwVersionMinor;       
    DWORD   dwVersionIncremental;       
} IOCTL_HAL_GET_BSP_VERSION_OUT;

BOOL 
OALIoCtlHalGetBspVersion(
    UINT32 code, 
    VOID *pInpBuffer,
    UINT32 inpSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_GET_DSP_INFO
//
//  This code is used to get the configured speed of the IVA2 (DSP) subsystem
//
#define IOCTL_HAL_GET_DSP_INFO    \
    CTL_CODE(FILE_DEVICE_HAL, 3101, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    DWORD   dwSpeedMHz;
} IOCTL_HAL_GET_DSP_INFO_OUT;

BOOL 
OALIoCtlHalGetDspInfo(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_RTC_TIME
//
//  This code is used to indicate to kernel that a periodic time event from
//  RTC chip occurred
//
#define IOCTL_HAL_RTC_TIME    \
    CTL_CODE(FILE_DEVICE_HAL, 1024, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlHalRtcTime(
    UINT32 code,
    VOID *pInBuffer,
    UINT32 inSize,
    VOID *pOutBuffer,
    UINT32 outSize,
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_RTC_ALARM
//
//  This code is used to call from TWL(Triton) chip driver when alarm occurs.
//
#define IOCTL_HAL_RTC_ALARM    \
    CTL_CODE(FILE_DEVICE_HAL, 1025, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlHalRtcAlarm(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_I2CMODE
//
//  Specifies to the kernel if a i2c is in hs mode
//
#define IOCTL_HAL_I2CHSMODE  \
    CTL_CODE(FILE_DEVICE_HAL, 1028, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlHalI2CMode(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_I2CCOPYFNTABLE
//
//  Specifies to the kernel copy i2c function table
//
#define IOCTL_HAL_I2CCOPYFNTABLE  \
    CTL_CODE(FILE_DEVICE_HAL, 1029, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL 
OALIoCtlHalI2CCopyFnTable(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetRNdisMacAddr
//
BOOL
OALIoCtlHalGetRNdisMacAddr(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalQueryFormatPartition
//
BOOL
OALIoCtlHalQueryFormatPartition(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    );

//-----------------------------------------------------------------------------
//
//  Function:  OALPerformanceTimerInit
//
void 
OALPerformanceTimerInit(
    DWORD,
    DWORD
    );

//-----------------------------------------------------------------------------
//
//  Function:  OALProfileTimerHit
//
void 
OALProfileTimerHit(
    UINT32 ra
    );


//-----------------------------------------------------------------------------
//
//  Functions: OALFlashStoreXXX
//
HANDLE
OALFlashStoreOpen(
    DWORD address
    );

BOOL
OALFlashStoreWrite(
    HANDLE hFlash,
    ULONG start,
    UCHAR *pData,
    ULONG dataSize,
    BOOL includeSectorInfo,
    BOOL offsetReservedBlocks
    );

BOOL
OALFlashStoreRead(
    HANDLE hFlash,
    ULONG start,
    UCHAR *pData,
    ULONG dataSize,
    BOOL includeSectorInfo,
    BOOL offsetReservedBlocks
    );

BOOL
OALFlashStoreErase(
    HANDLE hFlash,
    ULONG start,
    ULONG size,
    BOOL offsetReservedBlocks
    );

BOOL
OALFlashStoreBufferedRead(
    HANDLE hFlash,
    ULONG start,
    UCHAR *pData,
    ULONG size,
    BOOL offsetReservedBlocks
    );

BOOL
OALFlashStoreReadFromReservedRegion(
    HANDLE hFlash,
    LPCSTR name,
    ULONG start,
    UCHAR *pData,
    ULONG size
    );

BOOL
OALFlashStoreWriteToReservedRegion(
    HANDLE hFlash,
    LPCSTR name,
    ULONG start,
    UCHAR *pData,
    ULONG size
    );

ULONG
OALFlashStoreSectorSize(
    HANDLE hFlash
    );

ULONG
OALFlashStoreBlockSize(
    HANDLE hFlash
    );

VOID
OALFlashStoreClose(
    HANDLE hFlash
    );


//-----------------------------------------------------------------------------
//
//  Function: OALSRAMFnInit
//
BOOL OALSRAMFnInit(
    );

BOOL OALCPUSuspend(
    );

BOOL OALCPUEnd(
    );

BOOL OALCPURestoreContext(
    );

BOOL OALCPUStart(
    );

//-----------------------------------------------------------------------------
//
//  Function: OALWakeupLatency_xxx
//
BOOL
OALWakeupLatency_Initialize(
    );

DWORD
OALWakeupLatency_PushState(
    DWORD state
    );

void
OALWakeupLatency_PopState(
    );

DWORD
OALWakeupLatency_GetCurrentState(
    );

DWORD
OALWakeupLatency_GetSuspendState(
    );

BOOL
OALWakeupLatency_SetSuspendState(
    DWORD suspendState
    );

INT
OALWakeupLatency_GetDelayInTicks(
    DWORD state
    );

DWORD
OALWakeupLatency_FindStateByMaxDelayInTicks(
    INT delayTicks
    );

BOOL
OALWakeupLatency_UpdateOpp(
    DWORD *rgDomains,
    DWORD *rgOpps,
    DWORD  count
    );

BOOL
OALWakeupLatency_UpdateDomainState(
    DWORD powerDomain,
    DWORD powerState,
    DWORD logicState
    );

BOOL
OALWakeupLatency_DeviceEnabled(
    DWORD devId,
    BOOL bEnabled
    );

BOOL
OALWakeupLatency_IsChipOff(
    DWORD latencyState
    );

VOID 
OALWakeupLatency_SaveSnapshot(
    );

VOID 
OALWakeupLatency_DumpSnapshot(
    );

//-----------------------------------------------------------------------------
//
//  Function: OALMux_xxx
//
void
OALMux_InitMuxTable(
    );

DWORD
OALMux_UpdateOnDeviceStateChange(
    UINT devId,
    UINT oldState,
    UINT newState,
    BOOL bPreStateChange
    );

//-----------------------------------------------------------------------------
//
//  Function: OALSDRCRefreshCounter
//
void
OALSDRCRefreshCounter(
    UINT highFreq, 
    UINT lowFreq
    );

//-----------------------------------------------------------------------------
//
//  Function: OALPowerxxx
//
void 
OALPowerInit(
    );

void 
OALPowerPostInit(
    );

BOOL 
OALPowerVFP(
    DWORD dwCommand
    );

//-----------------------------------------------------------------------------
//
//  Function: OALPrcmIntrHandler
//
UINT32 
OALPrcmIntrHandler(
    );


//-----------------------------------------------------------------------------
//
//  Function: OAL SmartReflex interrupt handlers
//
UINT32 
OALSmartReflex1Intr(
    );

UINT32 
OALSmartReflex2Intr(
    );


//-----------------------------------------------------------------------------
//
//  Function: OALModemInit
//
void 
OALModemInit(
    );


//-----------------------------------------------------------------------------
//
//  Function: OALSetMemoryAttributes
//
BOOL 
OALSetMemoryAttributes(
    LPVOID pVirtAddr,           // Virtual address of region
    LPVOID pPhysAddrShifted,    // PhysicalAddress >> 8 (to support up to 40 bit address)
    DWORD  cbSize,              // Size of the region
    DWORD  dwAttributes         // attributes to be set
    );

//------------------------------------------------------------------------------
// Prototypes to control kitl related driver registry parameters
extern DWORD g_oalKitlEnabled;

void OEMEthernetDriverEnable(BOOL bEnable);
void OEMUsbDriverEnable(BOOL bEnable);

//------------------------------------------------------------------------------
// Prototypes to manage hardware watchdog
void 
OALWatchdogInit(OEMGLOBAL* pOemGlobal);

void OALWatchdogEnable(BOOL bEnable);

//-----------------------------------------------------------------------------
//
//
//  Define:  OUTSYSREG32
//
// Interface to update the GPMC, SDRC, MPUIC, CM register and corresponding
// shadow registers
//
#define OUTSYSREG32(RegGroup,offset,value) \
        OutShadowReg32((UINT32)RegGroup##_PA,(UINT32)(&(((RegGroup*)0)->offset)),(UINT32)value)

void 
OutShadowReg32(
    UINT32 deviceGroup, 
    UINT32 offset, 
    UINT32 value
    );

//-----------------------------------------------------------------------------
//
//  Define:  OUTSYSREG32
//
// Interface to update the GPMC, SDRC, MPUIC, CM register and corresponding
// shadow registers
//
#define SETSYSREG32(RegGroup,offset,value) \
        SetShadowReg32((UINT32)RegGroup##_PA,(UINT32)(&(((RegGroup*)0)->offset)),(UINT32)value)

void 
SetShadowReg32(
    UINT32 deviceGroup, 
    UINT32 offset, 
    UINT32 value
    );

//-----------------------------------------------------------------------------
//
//  Define:  OALContextRestore
//
// Restore the CORE context
//
VOID
OALContextRestore(
    UINT32 prevMpuState,
    UINT32 prevCoreState,
    UINT32 prevPerState
    );

//------------------------------------------------------------------------------
//
//  External: OALContextSave
//
//  Save the context of system modules just before wfi is called
//
VOID 
OALContextSave(
    );

//-----------------------------------------------------------------------------
//
//  Function:  OALContextUpdateDirtyRegister
//
//  Identifies a register set as being dirty, ie register has been modified
//  and needs to be backed-up into SDRAM for off mode
//
void 
OALContextUpdateDirtyRegister(
    UINT32  ffRegisterSet
    );

//-----------------------------------------------------------------------------
//
//  Function:  OALContextUpdateCleanRegister
//
//  clears the dirty bit for a register set
//
void 
OALContextUpdateCleanRegister(
    UINT32  ffRegisterSet
    );

void OEMCacheRangeFlush(VOID *pAddress, DWORD length, DWORD flags);

//------------------------------------------------------------------------------
//
//  Function:  OEMEnableIOPadWakeup
//
//  Enable/Disable IO PAD wakeup capability for a particular GPIO
//
//
void
OEMEnableIOPadWakeup(
    DWORD   gpio,
    BOOL    bEnable
    );

//------------------------------------------------------------------------------
//
//  Function:  OEMGetIOPadWakeupStatus
//
//  Returns the GPIO number of the corresponding IO PAD which received wakeup EVENT
//  Returns GPIO_MAX_NUM if NO wakeup event is set
//
//
DWORD
OEMGetIOPadWakeupStatus();

//------------------------------------------------------------------------------
//
//  Processor Feature Values used in IsProcessorFeaturePresent API
//

#define PF_ARM_THUMB2                   0x80000018      // thumb2 support
#define PF_ARM_T2EE                     0x80000019      // T2EE support
#define PF_ARM_VFP_V3                   0x8000001A      // VFPv3 hardware
#define PF_ARM_NEON                     0x8000001B      // NEON hardware
#define PF_ARM_UNALIGNED_ACCESS         0x8000001C      // can support unaligned access (need to call a KLIB_IOCTL to turn it on)
#define PF_ARM_VFP_SUPPORT              0x8000001D      // OS support for VFP (including OS VFP emulation, use PF_ARM_VFP_HARDWARE to query for VFP HW)
#define PF_ARM_VFP_V1                   0x8000001E      // VFPv1 hardware
#define PF_ARM_VFP_V2                   0x8000001F      // VFPv2 hardware
#define PF_ARM_VFP_ALL_ROUNDING_MODES   0x80000020      // VFP: All IEEE rounding modes are supported
#define PF_ARM_VFP_SHORT_VECTORS        0x80000021      // VFP: Support for short vector operations
#define PF_ARM_VFP_SQUARE_ROOT          0x80000022      // VFP: Square root implemented in hardware
#define PF_ARM_VFP_DIVIDE               0x80000023      // VFP: Divide operations implemented in hardware
#define PF_ARM_VFP_FP_EXCEPTIONS        0x80000024      // VFP: Support for trapped exceptions
#define PF_ARM_VFP_EXTENDED_REGISTERS   0x80000025      // VFP/Neon: 32 x 64bit register bank
#define PF_ARM_VFP_HALF_PRECISION       0x80000026      // VFP: support for half-precision floating point values
#define PF_ARM_NEON_HALF_PRECISION      0x80000027      // Neon: support for half-precision floating point values
#define PF_ARM_NEON_SINGLE_PRECISION    0x80000028      // Neon: support for float32 operations
#define PF_ARM_NEON_LOAD_STORE          0x80000029      // Neon: support for Neon SIMD loads and stores
#define PF_ARM_VFP_DENORMALS            0x8000002A      // VFP: hardware supports denormalized number arithmetic


//------------------------------------------------------------------------------
//
//  Function:  OALVFPXXX
//
void 
OALVFPEnable();

DWORD
OALVFPGetFPSID();

void 
OALVFPInitialize(OEMGLOBAL* pOemGlobal);

//-----------------------------------------------------------------------------
//
//  Function:  OALxxxNeonxxx
//

void OALInitNeonSaveArea(LPBYTE pArea);
void OALSaveNeonRegisters(LPBYTE pArea);
void OALRestoreNeonRegisters(LPBYTE pArea);

#ifdef __cplusplus
}
#endif

#endif

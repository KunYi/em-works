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
//  File: prcm_priv.h
//
#ifndef _PRCM_PRIV_H_
#define _PRCM_PRIV_H_

#include "am387x_clocks.h"

#pragma warning (push)
#pragma warning (disable:4200)

//-----------------------------------------------------------------------------
//#define MAX_IDLESTATUS_LOOP         (0x1000)
//#define cm_offset(field)            ((LONG)&(((OMAP_CM_REGS*)0)->field))
//#define prm_offset(field)           ((LONG)&(((OMAP_PRM_REGS*)0)->field))
//#define AUTOIDLE_DECL(a, x, y, z)   ClockOffsetInfo_3 a = {x, y, z}
//#define IDLESTAT_DECL(a, x, y)      ClockOffsetInfo_1 a = {x, y}
//#define ICLK_DECL(a, x, y, z)       ClockOffsetInfo_2 a = {x, y, z, FALSE}
//#define ICLK_VDECL(a, x, y, z)       ClockOffsetInfo_2 a = {x, y, z, TRUE}
//#define FCLK_DECL(a, x, y, z)       ClockOffsetInfo_2 a = {x, y, z, FALSE}
//#define FCLK_VDECL(a, x, y, z)       ClockOffsetInfo_2 a = {x, y, z, TRUE}
//#define WKEN_DECL(a, x, y)          ClockOffsetInfo_1 a = {x, y}

//-----------------------------------------------------------------------------
// common device structures

typedef struct {
    UINT                    mask;
    UINT                    offset;
} ClockOffsetInfo_1;

typedef struct {
    LONG                    refCount;    
    UINT                    mask;
    UINT                    offset;
    BOOL                    bVirtual;
} ClockOffsetInfo_2;

typedef struct {
    DWORD                   size;
    SourceClock_e           rgSourceClocks[];    
} SourceDeviceClocks_t;

typedef struct {
    PowerDomain_e   powerDomain;
    ClockDomain_e   fclk_domain;
    ClockDomain_e	iclk_domain;
	LONG			refCount;	 // reference count for device;
    LONG			CLKCTRL_REG; // offset to the register on the AM387X_PRCM_REGS struct
} DeviceLookupEntry;

typedef struct {
	PowerDomain_e	clkDomain;
	LONG			refCount;	// reference count for domain
	BOOL			ro;			// TRUE if register is RO 
	LONG			CLKSTCTRL_REG;// offset to the register on the AM387X_PRCM_REGS struct
} ClockDomainLookupEntry;

//------------------------------------------------------------------------------
// Voltage domain definition
typedef int                 VddRefCountTable[kVDD_COUNT];
//------------------------------------------------------------------------------
// DPLL domain definition
typedef struct {
    int                     vddDomain;
    LONG                    refCount;
    void * /*DpllState_t*/   *pDpllInfo; // VA: not sure whether we need that
} Dpll_Entry_t;

typedef Dpll_Entry_t        DpllMap[kDPLL_COUNT];
//------------------------------------------------------------------------------
// Root clk src definition

typedef struct {
    int     dpllDomain;
    LONG    refCount;
	double	freq;			
} DpllClkOut_Entry_t;

typedef DpllClkOut_Entry_t  DpllClkOutMap[kDPLL_CLKOUT_COUNT];
typedef enum { NONE, SEL_PRCM, SEL_PLLSS, DIV_PRCM, DIV_PLLSS} SelRegs_T;

//------------------------------------------------------------------------------
typedef struct {
    int     parentClk;
    LONG    refCount;
    BOOL    bIsDpllSrcClk;
    int		Divisor;
    int     thisClk;
	SelRegs_T sel_regT;
	LONG	SEL_REG; // offset to the register on the AM387X_PRCM_REGS or AM387X_PLLSS_PREGS struct
	UINT	shift;
	UINT	mask;
	UINT	*parents; 
} SrcClock_Entry_t;
typedef SrcClock_Entry_t    SrcClockMap[kSOURCE_CLOCK_COUNT];

//------------------------------------------------------------------------------
//  Reference to all PRCM-registers. Initialized in OALPowerInit
extern AM387X_PRCM_REGS    *g_pPrcmRegs;
extern AM387X_PLLSS_REGS  *g_pPllssRegs;
//------------------------------------------------------------------------------
//
//  Global:  g_PrcmPostInit
//
//  Indicates if prcm library has been fully initialized. 
//  Initialized in OALPowerPostInit
//
extern BOOL                 g_PrcmPostInit;

//------------------------------------------------------------------------------
//
//  Global:  g_PrcmMutex
//
//  Contains a list of CRITICAL_SECTIONS used for synchronized access to 
//  PRCM registers
//
typedef enum
{
    Mutex_DeviceClock = 0,
    Mutex_Clock,
    Mutex_Domain,
    Mutex_Voltage,
    Mutex_Power,
    Mutex_Intr,
    Mutex_Count
} PRCMMutex_Id;

extern CRITICAL_SECTION     g_rgPrcmMutex[Mutex_Count];

//------------------------------------------------------------------------------
//
//  Global:  g_bSingleThreaded
//
//  Indicated that the OAL PRCM functions are being called while the kernel is 
//  single threaded (OEMIdle, OEMPoweroff)
//
extern BOOL g_bSingleThreaded;

//------------------------------------------------------------------------------
// internal/helper routines

BOOL ClockInitialize();
BOOL ClockUpdateParentClock( int srcClkId, BOOL bEnable );
BOOL ClockEnableClkDomain(ClockDomain_e clkDomain, BOOL bEnable);
BOOL DomainInitialize();
BOOL ResetInitialize();
BOOL DeviceInitialize();
BOOL DomainGetDeviceContextState(UINT powerDomain, ClockOffsetInfo_2 *pInfo, BOOL bSet );

//------------------------------------------------------------------------------
// synchronization routines
__inline void Lock(PRCMMutex_Id mutexId)
{
    if (g_PrcmPostInit && !g_bSingleThreaded) 
        EnterCriticalSection(&g_rgPrcmMutex[mutexId]);
}

__inline void Unlock(PRCMMutex_Id mutexId)
{
    if (g_PrcmPostInit && !g_bSingleThreaded)
        LeaveCriticalSection(&g_rgPrcmMutex[mutexId]);
}

#pragma warning (pop)
//------------------------------------------------------------------------------
#endif // _PRCM_PRIV_H_

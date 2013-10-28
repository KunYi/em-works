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
//  File: prcm_domain.c
//
#include "omap.h"
#include "omap_prof.h"
#include "am387x.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//DomainMap s_DomainTable = {};

//-----------------------------------------------------------------------------
static BOOL _PrcmDomainHwUpdate(UINT powerDomain, UINT ffMask)
{
    BOOL rc = TRUE;
    OALMSG(1, (L"_PrcmDomainHwUpdate %d %08X\r\n", powerDomain, ffMask));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL PrcmDomainSetPowerStateInternal(
    UINT        powerDomain,
    UINT        powerState,
    UINT        logicState,
    BOOL        bNotify
    )
{
    OALMSG(1, (L"PrcmDomainSetPowerStateInternal %d %d %d %d\r\n",
		powerDomain, powerState, logicState, bNotify));
    return FALSE;
}

//-----------------------------------------------------------------------------
BOOL DomainGetDeviceContextState(
    UINT                powerDomain,
    ClockOffsetInfo_2  *pInfo,
    BOOL                bSet
    )
{
	UNREFERENCED_PARAMETER(pInfo);
    OALMSG(1, (L"DomainGetDeviceContextState %d %d\r\n",
		powerDomain, bSet));
	return FALSE;
}

//-----------------------------------------------------------------------------
BOOL DomainInitialize()
{
    
    OALMSG(OAL_FUNC, (L"+DomainInitialize()\r\n"));
    return TRUE;    
}

//-----------------------------------------------------------------------------
// WARNING: This function is called from OEMPowerOff - no system calls, critical 
// sections, OALMSG, etc., may be used by this function or any function that it calls.
//------------------------------------------------------------------------------
BOOL PrcmRestoreDomain(UINT powerDomain)
{
    BOOL rc = TRUE;
 	OALMSG(1/*OAL_FUNC*/, (L"+PrcmRestoreDomain (powerDomain=%d)\r\n", powerDomain));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL PrcmDomainSetWakeupDependency(
    UINT        powerDomain,
    UINT        ffDependency,
    BOOL        bEnable
    )
{
    if (!g_bSingleThreaded)
        OALMSG(1, (L"+PrcmDomainSetWakeupDependency"
            L"(powerDomain=%d, ffDependency=0x%08X, bEnable=%d)\r\n", 
            powerDomain, ffDependency, bEnable));
    
    return FALSE;
}

//-----------------------------------------------------------------------------
BOOL PrcmDomainSetSleepDependency(
    UINT        powerDomain,
    UINT        ffDependency,
    BOOL        bEnable
    )
{
    OALMSG(1, (L"+PrcmDomainSetSleepDependency"
        L"(powerDomain=%d, ffDependency=0x%08X, bEnable=%d)\r\n", 
        powerDomain, ffDependency, bEnable));
    
    return FALSE;
}

//-----------------------------------------------------------------------------
BOOL PrcmDomainSetPowerState(UINT powerDomain,
    UINT        powerState, UINT        logicState
    )
{
    
    OALMSG(1, (L"+PrcmDomainSetPowerState"
        L"(powerDomain=%d, powerState=0x%08X, logicState=%d)\r\n", 
        powerDomain, powerState, logicState));
    
    return FALSE;
}

//-----------------------------------------------------------------------------
BOOL PrcmDomainSetClockState(UINT powerDomain, UINT clockDomain, UINT clockState)
{
    if (!g_bSingleThreaded)
        OALMSG(1/*OAL_FUNC*/, (L"+PrcmDomainSetClockState"
            L"(powerDomain=%d, clockDomain=%d, clockState=0x%08X)\r\n", 
            powerDomain, clockDomain, clockState));

    return TRUE;
}

//-----------------------------------------------------------------------------
// WARNING: This function is called from OEMPowerOff - no system calls, critical 
// sections, OALMSG, etc., may be used by this function or any function that it calls.
//------------------------------------------------------------------------------
BOOL PrcmDomainSetClockStateKernel(UINT powerDomain,
    UINT clockDomain, UINT clockState)
{
    UNREFERENCED_PARAMETER(powerDomain);
    UNREFERENCED_PARAMETER(clockDomain);
    UNREFERENCED_PARAMETER(clockState);
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL PrcmDomainSetMemoryState(UINT powerDomain,
    UINT memoryState, UINT memoryStateMask )
{
    OALMSG(1, (L"+PrcmDomainSetMemoryState"
        L"(powerDomain=%d, memoryState=0x%08X, memoryStateMask=0x%08X)\r\n", 
        powerDomain, memoryState, memoryStateMask));

    return FALSE;
}

//-----------------------------------------------------------------------------
// WARNING: The PrcmDomainClearWakeupStatus function can be called from OEMIdle
// and OEMPowerOff (through PrcmSuspend). It must not use system calls, critical 
// sections, etc.
//-----------------------------------------------------------------------------
#define IN_OUT(X) OUTREG32(&X, INREG32(&X))

BOOL PrcmDomainClearWakeupStatus(UINT powerDomain)
{
    return FALSE;
}

//-----------------------------------------------------------------------------
void PrcmDomainUpdateRefCount(UINT powerDomain, UINT bEnable )
{ 
	OALMSG(1, (L"PrcmDomainUpdateRefCount %d %d\r\n",powerDomain,bEnable));

//    if (bEnable) InterlockedIncrement((LONG*)&s_DomainTable[powerDomain].refCount);
//    else         InterlockedDecrement((LONG*)&s_DomainTable[powerDomain].refCount);
}

//-----------------------------------------------------------------------------
void PrcmProcessPostMpuWakeup()
{
}

//-----------------------------------------------------------------------------
VOID PrcmDomainClearReset()
{   // Clear the Resest states
	OALMSG(1, (L"+PrcmDomainClearReset\r\n"));

//    example IN_OUT(g_pPrcmPrm->pOMAP_CORE_PRM->RM_RSTST_CORE); 
}


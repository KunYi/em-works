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
//  File: prcm_reset.c
//
#include <windows.h>
#include <nkintr.h>
#include <pkfuncs.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <initguid.h>
#include <bus.h>
#include <constants.h>
#include "prcm_priv.h"

//------------------------------------------------------------------------------
BOOL
ResetInitialize()
{
    // clear the reset flags for all the power domains
    OUTREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->RM_RSTST_CORE, 
        DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_EMU_PRM->RM_RSTST_EMU,  
        DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_PER_PRM->RM_RSTST_PER, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_DSS_PRM->RM_RSTST_DSS, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_NEON_PRM->RM_RSTST_NEON, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_SGX_PRM->RM_RSTST_SGX, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_CAM_PRM->RM_RSTST_CAM, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->RM_RSTST_USBHOST, 
      COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
      );

    OUTREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->RM_RSTST_MPU,         
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST |
        EMULATION_MPU_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_IVA2_PRM->RM_RSTST_IVA2, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST |
        EMULATION_SEQ_RST | EMULATION_VHWA_RST | EMULATION_IVA2_RST |
        IVA2_SW_RST3 | IVA2_SW_RST2 | IVA2_SW_RST1
        );

    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
PrcmDomainResetStatus(
    UINT    powerDomain,
    UINT   *pResetStatus,
    BOOL    bClear
    )
{
    // get the prm register for the domain
    //
    BOOL rc = FALSE;
    UINT resetStatus;
    OMAP_PRM_REGS *pPrmRegs = GetPrmRegisterSet(powerDomain);
    if (powerDomain == POWERDOMAIN_WAKEUP || pPrmRegs == NULL) goto cleanUp;

    // get current status 
    resetStatus = INREG32(&pPrmRegs->RM_RSTST_xxx);
    if (pResetStatus) *pResetStatus = resetStatus;
    
    // clear status if requested
    if (bClear) OUTREG32(&pPrmRegs->RM_RSTST_xxx, resetStatus);

    rc = TRUE;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
BOOL
PrcmDomainReset(
    UINT powerDomain,
    UINT resetMask
    )
{
    BOOL            rc = FALSE;
    OMAP_PRM_REGS  *pPrmRegs;
    
    // currently only support resetting of iva2 domain
    if (powerDomain != POWERDOMAIN_IVA2) goto cleanUp;

    // reset the appropriate domains
    pPrmRegs = GetPrmRegisterSet(powerDomain);
    if (pPrmRegs == NULL) goto cleanUp;

    resetMask &= RST3_IVA2 | RST2_IVA2 | RST1_IVA2;

    // UNDONE:
    // not exactly clear with the programming model for these registers
    //
    OUTREG32(&pPrmRegs->RM_RSTCTRL_xxx, resetMask);
    rc = TRUE;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
BOOL
PrcmDomainResetRelease(
    UINT powerDomain,
    UINT resetMask
    )
{
    BOOL            rc = FALSE;
    UINT            resetTemp;
    UINT            resetStatus;
    OMAP_PRM_REGS  *pPrmRegs;
    
    // currently only support resetting of iva2 domain
    if (powerDomain != POWERDOMAIN_IVA2) goto cleanUp;

    // reset the appropriate domains
    pPrmRegs = GetPrmRegisterSet(powerDomain);
    if (pPrmRegs == NULL) goto cleanUp;

    resetMask &= RST3_IVA2 | RST2_IVA2 | RST1_IVA2;
    resetTemp = /*~INREG32(&pPrmRegs->RM_RSTCTRL_xxx) |*/ resetMask;

    // UNDONE:
    // not exactly clear with the programming model for these registers
    //
    OUTREG32(&pPrmRegs->RM_RSTCTRL_xxx, ~resetTemp);
    resetStatus = resetMask << 8;    
    while (resetStatus != 0)
        {
        resetStatus &= ~INREG32(&pPrmRegs->RM_RSTST_xxx);
        }
    
    // clear the status
    OUTREG32(&pPrmRegs->RM_RSTST_xxx, resetMask << 8);
        
    rc = TRUE;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

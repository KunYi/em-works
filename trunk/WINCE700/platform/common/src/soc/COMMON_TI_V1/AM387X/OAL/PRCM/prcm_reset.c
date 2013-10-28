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
//  File: prcm_reset.c
//

#include "omap.h"
#include "omap_prof.h"
#include "am387x.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"

//------------------------------------------------------------------------------
BOOL ResetInitialize()
{
	// Don't know whether we need to clear global reset bits 
	//OUTREG32(&g_pPrcmRegs->PRM_RSTST, EXTERNALWARM_RST | SECURE_WDT_RST | MPU_WDT_RST |
	//          MPU_SECURITY_VIOL_RST | GLOBALWARM_RST | GLOBALCOLD_RST );
	
    // clear the reset flags for all the power domains
	OUTREG32(&g_pPrcmRegs->RM_DSP_RSTST, GEM_GRST | GEM_LRST );
    
    OUTREG32(&g_pPrcmRegs->RM_ALWON2_RSTST, PCI_LRST | USB2_LRST | USB1_LRST |
		             MC_RST3 | MC_M3_RST2 | MC_M3_RST1 | TPPSS_RST | TPPSS_LRST);
    
    OUTREG32(&g_pPrcmRegs->RM_HDVICP_RSTST, HDVICP_RST3 | HDVICP_RST2 | HDVICP_RST1 );

    OUTREG32(&g_pPrcmRegs->RM_ISP_RSTST, ISP_RST );

    OUTREG32(&g_pPrcmRegs->RM_HDVPSS_RSTST, HDVPSS_RST );

    OUTREG32(&g_pPrcmRegs->RM_GFX_RSTST, GFX_RST );

    OUTREG32(&g_pPrcmRegs->RM_ALWON_RSTST, SEC_M3_RST | SEC_M3_LRST );

	return TRUE;
}

//------------------------------------------------------------------------------
BOOL PrcmDomainResetStatus(UINT powerDomain, UINT *pResetStatus, BOOL    bClear)
{
	BOOL rc = FALSE;
	UINT resetStatus;

	if (pResetStatus == NULL) goto cleanUp;
	
	rc = TRUE;
	switch (powerDomain){
		case PWR_ALWAYSON2:
			resetStatus = INREG32(&g_pPrcmRegs->RM_ALWON2_RSTST);
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_ALWON2_RSTST, resetStatus);
			break;
			
		case PWR_ISP:
			resetStatus = INREG32(&g_pPrcmRegs->RM_ISP_RSTST);
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_ISP_RSTST, resetStatus);
			break;
			
		case PWR_GFX:
			resetStatus = INREG32(&g_pPrcmRegs->RM_GFX_RSTST );
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_GFX_RSTST, resetStatus);
			break;

		case PWR_DSS:
			resetStatus = INREG32(&g_pPrcmRegs->RM_HDVICP_RSTST ); // ?????????????????
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_HDVICP_RSTST, resetStatus);
			break;
	
		case PWR_ACTIVE:
			resetStatus = INREG32(&g_pPrcmRegs->RM_DSP_RSTST );
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_DSP_RSTST, resetStatus);
			break;
	
		case PWR_HVAHD:
			resetStatus = INREG32(&g_pPrcmRegs->RM_HDVPSS_RSTST ); // ???????????????????
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_HDVPSS_RSTST, resetStatus);
			break;

		default:
			rc = FALSE;
	}		
		
cleanUp:
	return rc;
}

//------------------------------------------------------------------------------
BOOL PrcmDomainReset(UINT powerDomain, UINT resetMask)
{
	BOOL			rc = TRUE;

	switch (powerDomain){
		case PWR_ALWAYSON2:
			resetMask &= PCI_LRST | USB2_LRST | USB1_LRST |
		             MC_RST3 | MC_M3_RST2 | MC_M3_RST1 | TPPSS_RST | TPPSS_LRST;
			OUTREG32(&g_pPrcmRegs->RM_ALWON2_RSTCTRL, 
				 INREG32(&g_pPrcmRegs->RM_ALWON2_RSTCTRL) | resetMask);
			break;
			
		case PWR_ISP:
			resetMask &= ISP_RST;
			OUTREG32(&g_pPrcmRegs->RM_ISP_RSTCTRL, 
				INREG32(&g_pPrcmRegs->RM_ISP_RSTCTRL) | resetMask);
			break;
			
		case PWR_GFX:
			resetMask &= GFX_RST;
			OUTREG32(&g_pPrcmRegs->RM_GFX_RSTCTRL, 
				INREG32(&g_pPrcmRegs->RM_GFX_RSTCTRL) | resetMask);
			break;

		case PWR_ACTIVE:
			resetMask &= GEM_GRST | GEM_LRST ;
			OUTREG32(&g_pPrcmRegs->RM_DSP_RSTCTRL,
				INREG32(&g_pPrcmRegs->RM_DSP_RSTCTRL) | resetMask);
			break;
	
		case PWR_DSS: 
			resetMask &= HDVICP_RST3 | HDVICP_RST2 | HDVICP_RST1;
			OUTREG32(&g_pPrcmRegs->RM_HDVICP_RSTCTRL,
				INREG32(&g_pPrcmRegs->RM_HDVICP_RSTCTRL) | resetMask);
			break;
	
		case PWR_HVAHD:
			resetMask &= HDVPSS_RST;
			OUTREG32(&g_pPrcmRegs->RM_HDVPSS_RSTCTRL,
				INREG32(&g_pPrcmRegs->RM_HDVPSS_RSTCTRL) | resetMask);
			break;

		default:
			rc = FALSE;
	}		
		
	return rc;
}

//------------------------------------------------------------------------------
BOOL PrcmDomainResetRelease(UINT powerDomain, UINT resetMask)
{
	BOOL	rc = TRUE;
	UINT32	resetStatus;
	volatile UINT32  *rstctrl = NULL;
	volatile UINT32  *rstst   = NULL;

	switch (powerDomain){
		case PWR_ALWAYSON2:
			resetMask &= PCI_LRST | USB2_LRST | USB1_LRST |
				 MC_RST3 | MC_M3_RST2 | MC_M3_RST1 | TPPSS_RST | TPPSS_LRST;
			rstctrl = &g_pPrcmRegs->RM_ALWON2_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_ALWON2_RSTST;
			break;
				
		case PWR_ISP:
			resetMask &= ISP_RST;
			rstctrl = &g_pPrcmRegs->RM_ISP_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_ISP_RSTST;
			break;
				
		case PWR_GFX:
			resetMask &= GFX_RST;
			rstctrl = &g_pPrcmRegs->RM_GFX_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_GFX_RSTST;
			break;
	
		case PWR_ACTIVE:
			resetMask &= GEM_GRST | GEM_LRST;
			rstctrl = &g_pPrcmRegs->RM_DSP_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_DSP_RSTST;
			break;
		
		case PWR_DSS:
			resetMask &= HDVICP_RST3 | HDVICP_RST2 | HDVICP_RST1;
			rstctrl = &g_pPrcmRegs->RM_HDVICP_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_HDVICP_RSTST;
			break;
		
		case PWR_HVAHD:
			resetMask &= HDVPSS_RST;
			rstctrl = &g_pPrcmRegs->RM_HDVICP_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_HDVICP_RSTST;
			break;
	
		default:
			rc = FALSE;
	}		

	if (rc == TRUE && rstctrl != NULL){
		OUTREG32(rstctrl, INREG32(rstctrl) & ~resetMask);
		resetStatus = resetMask;
		while (resetStatus != 0)
			resetStatus &= ~INREG32(rstst);
		// clear the status
		OUTREG32(rstst, resetMask);
	}			
	return rc;
}

//------------------------------------------------------------------------------
void
PrcmGlobalReset(
    )
{	
    OUTREG32(&g_pPrcmRegs->PRM_DEVICE, GLOBALWARM_RST);	
}




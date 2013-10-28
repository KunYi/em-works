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
#include "am389x.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"

//------------------------------------------------------------------------------
BOOL ResetInitialize()
{
    // clear the reset flags for all the power domains
    OUTREG32(&g_pPrcmRegs->RM_ACTIVE_RSTST, GEM_SW_RST | GEM_LRST );
    
    OUTREG32(&g_pPrcmRegs->RM_DEFAULT_RSTST, PCI_LRST | USB2_LRST |
		      USB1_LRST | DUCATI_RST3 | DUCATI_M3_RST2 | DUCATI_M3_RST1);
    
    OUTREG32(&g_pPrcmRegs->RM_IVAHD0_RSTST, IVAn_RST3 | IVAn_RST2 | IVAn_RST1 );

    OUTREG32(&g_pPrcmRegs->RM_IVAHD1_RSTST, IVAn_RST3 | IVAn_RST2 | IVAn_RST1 );

    OUTREG32(&g_pPrcmRegs->RM_IVAHD2_RSTST, IVAn_RST3 | IVAn_RST2 | IVAn_RST1 );

    OUTREG32(&g_pPrcmRegs->RM_SGX_RSTST, SGX_RST);

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
		case PWR_DEFAULT:
			resetStatus = INREG32(&g_pPrcmRegs->RM_DEFAULT_RSTST);
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_DEFAULT_RSTST, resetStatus);
			break;
			
		case PWR_ACTIVE:
			resetStatus = INREG32(&g_pPrcmRegs->RM_ACTIVE_RSTST);
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_ACTIVE_RSTST, resetStatus);
			break;
			
		case PWR_IVA0:
			resetStatus = INREG32(&g_pPrcmRegs->RM_IVAHD0_RSTST );
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_IVAHD0_RSTST, resetStatus);
			break;

		case PWR_IVA1:
			resetStatus = INREG32(&g_pPrcmRegs->RM_IVAHD1_RSTST );
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_IVAHD1_RSTST, resetStatus);
			break;
	
		case PWR_IVA2:
			resetStatus = INREG32(&g_pPrcmRegs->RM_IVAHD2_RSTST );
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_IVAHD2_RSTST, resetStatus);
			break;
	
		case PWR_SGX:
			resetStatus = INREG32(&g_pPrcmRegs->RM_SGX_RSTST );
			if (bClear) OUTREG32(&g_pPrcmRegs->RM_SGX_RSTST, resetStatus);
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
		case PWR_DEFAULT:
			resetMask &= PCI_LRST | USB2_LRST | USB1_LRST | DUCATI_RST3 | DUCATI_M3_RST2 | DUCATI_M3_RST1;
			OUTREG32(&g_pPrcmRegs->RM_DEFAULT_RSTCTRL, 
				 INREG32(&g_pPrcmRegs->RM_DEFAULT_RSTCTRL) | resetMask);
			break;
			
		case PWR_ACTIVE:
			resetMask &= GEM_SW_RST | GEM_LRST;
			OUTREG32(&g_pPrcmRegs->RM_ACTIVE_RSTCTRL, 
				INREG32(&g_pPrcmRegs->RM_ACTIVE_RSTCTRL) | resetMask);
			break;
			
		case PWR_IVA0:
			resetMask &= IVAn_RST3 | IVAn_RST2 | IVAn_RST1;
			OUTREG32(&g_pPrcmRegs->RM_IVAHD0_RSTCTRL, 
				INREG32(&g_pPrcmRegs->RM_IVAHD0_RSTCTRL) | resetMask);
			break;

		case PWR_IVA1:
			resetMask &= IVAn_RST3 | IVAn_RST2 | IVAn_RST1;
			OUTREG32(&g_pPrcmRegs->RM_IVAHD1_RSTCTRL,
				INREG32(&g_pPrcmRegs->RM_IVAHD1_RSTCTRL) | resetMask);
			break;
	
		case PWR_IVA2:
			resetMask &= IVAn_RST3 | IVAn_RST2 | IVAn_RST1;
			OUTREG32(&g_pPrcmRegs->RM_IVAHD2_RSTCTRL,
				INREG32(&g_pPrcmRegs->RM_IVAHD2_RSTCTRL) | resetMask);
			break;
	
		case PWR_SGX:
			resetMask &= SGX_RST;
			OUTREG32(&g_pPrcmRegs->RM_SGX_RSTCTRL,
				INREG32(&g_pPrcmRegs->RM_SGX_RSTCTRL) | resetMask);
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
		case PWR_DEFAULT:
			resetMask &= PCI_LRST | USB2_LRST | USB1_LRST | DUCATI_RST3 | DUCATI_M3_RST2 | DUCATI_M3_RST1;
			rstctrl = &g_pPrcmRegs->RM_DEFAULT_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_DEFAULT_RSTST;
			break;
				
		case PWR_ACTIVE:
			resetMask &= GEM_SW_RST | GEM_LRST;
			rstctrl = &g_pPrcmRegs->RM_ACTIVE_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_ACTIVE_RSTST;
			break;
				
		case PWR_IVA0:
			resetMask &= IVAn_RST3 | IVAn_RST2 | IVAn_RST1;
			rstctrl = &g_pPrcmRegs->RM_IVAHD0_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_IVAHD0_RSTST;
			break;
	
		case PWR_IVA1:
			resetMask &= IVAn_RST3 | IVAn_RST2 | IVAn_RST1;
			rstctrl = &g_pPrcmRegs->RM_IVAHD1_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_IVAHD1_RSTST;
			break;
		
		case PWR_IVA2:
			resetMask &= IVAn_RST3 | IVAn_RST2 | IVAn_RST1;
			rstctrl = &g_pPrcmRegs->RM_IVAHD2_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_IVAHD2_RSTST;
			break;
		
		case PWR_SGX:
			resetMask &= SGX_RST;
			rstctrl = &g_pPrcmRegs->RM_SGX_RSTCTRL;
			rstst   = &g_pPrcmRegs->RM_SGX_RSTST;
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



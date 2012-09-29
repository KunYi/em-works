//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  power.c
//
//  Power BSP callback functions implementation. This function are called as
//  last function before OALCPUPowerOff. The KITL was already disabled by
//  OALKitlPowerOff.
//
//-----------------------------------------------------------------------------

#include <bsp.h>
#include "mc34704.h"

//-----------------------------------------------------------------------------
// External Functions
extern void OALCPUEnterWFI(void);
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);

//-----------------------------------------------------------------------------
// External Variables
extern PCSP_SDMA_REGS g_pSDMA;
extern PCSP_CRM_REGS g_pCRM;
//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
//
//  Function:  OALCPUPowerOff
//
//  This function powers off CPU.
//
VOID OALCPUPowerOff()
{
    static UINT32 dwSdmaStat;

	//
	// CS&ZHL JUN-3-2011: check pointer validation
	//
	if(g_pSDMA == NULL)
	{
		return;
	}

	if(g_pCRM == NULL)
	{
		return;
	}

    dwSdmaStat = INREG32(&g_pSDMA->STOP_STAT);
    if (dwSdmaStat)
    {
        OUTREG32(&g_pSDMA->STOP_STAT, dwSdmaStat);
        while ((INREG32(&g_pSDMA->ONCE_STAT) & 0xF000) != 0x6000)
        {
            // OALMSGS(TRUE, (_T("ONCE_STAT = 0x%x\r\n"), INREG32(&g_pSDMA->ONCE_STAT)));
        }
    }

    // Enable ARM and EMI well bias
    INSREG32(&g_pCRM->PMCR1, 
        CSP_BITFMASK(CRM_PMCR1_CPEN) | 
        CSP_BITFMASK(CRM_PMCR1_CPEN_EMI), 
        CSP_BITFVAL(CRM_PMCR1_CPEN, CRM_PMCR1_CPEN_ENABLE) | 
        CSP_BITFVAL(CRM_PMCR1_CPEN_EMI, CRM_PMCR1_CPEN_EMI_ENABLE)); 

    // Shut down OSC24M
    INSREG32(&g_pCRM->PMCR2, 
        CSP_BITFMASK(CRM_PMCR2_OSC24M_DOWN), 
        CSP_BITFVAL(CRM_PMCR2_OSC24M_DOWN, CRM_PMCR2_OSC24M_DOWN_DOWN)); 

    // STOP mode for WFI
    INSREG32(&g_pCRM->CCTL,
        CSP_BITFMASK(CRM_CCTL_LP_CTL), 
        CSP_BITFVAL(CRM_CCTL_LP_CTL, CRM_CCTL_LP_CTL_SLEEP));

    // Need to keep the following clocks gated on for successful handshake
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_SDMA, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_SDMA, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_RTIC, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_RTIC, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_RNGB, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_CAN1, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_CAN2, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_EMI, DDK_CLOCK_GATE_MODE_ENABLED);

    // Enter wait-for-interrupt mode and the system will transition
    // to the low-power mode configured in the CCMR above.
    OALCPUEnterWFI();

    // WAIT mode for WFI
    INSREG32(&g_pCRM->CCTL,
        CSP_BITFMASK(CRM_CCTL_LP_CTL), 
        CSP_BITFVAL(CRM_CCTL_LP_CTL, CRM_CCTL_LP_CTL_WAIT));

    if (dwSdmaStat)
    {
        OUTREG32(&g_pSDMA->HSTART, dwSdmaStat);
    }
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPPowerOff
//
//  This function performs board-level power off operations.
//
VOID BSPPowerOff()
{
	//
	// CS&ZHL JUN-2-2011: iMX257PDK has a MC34704 as power managment IC, not EM9170
	//
#ifdef		IMX257PDK_MC34704
    BYTE b;
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_ENABLED);
 
    //Clear the shutdown flag in the PMIC
#if 1
    // Temprorily Disable PMIC control here in order to avoid USB Overcurrent
    // Should be recoverd after HW fix
    UNREFERENCED_PARAMETER(b);
#else
    PmicGetRegister(3,&b);
    b &= ~(1<<4);
    PmicSetRegister(3,b);

    PmicEnable(FALSE);
#endif
    
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_DISABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_DISABLED);
#endif		//IMX257PDK_MC34704
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPPowerOn
//
//  This function performs board-level power on operations.
//
VOID BSPPowerOn()
{
	//
	// CS&ZHL JUN-2-2011: iMX257PDK has a MC34704 as power managment IC, not EM9170
	//
#ifdef		IMX257PDK_MC34704
    BYTE b;
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_ENABLED);
        
    //Clear the shutdown flag in the PMIC
#if 1
    // Temprorily Disable PMIC control here in order to avoid USB Overcurrent
    // Should be recoverd after HW fix
    UNREFERENCED_PARAMETER(b);
#else
    PmicGetRegister(3,&b);
    b &= ~(1<<4);
    PmicSetRegister(3,b);

    PmicEnable(TRUE);
#endif

    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_DISABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_DISABLED);
#endif		//IMX257PDK_MC34704
}

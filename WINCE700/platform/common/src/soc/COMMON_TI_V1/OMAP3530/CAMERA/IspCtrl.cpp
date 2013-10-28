// All rights reserved ADENEO EMBEDDED 2010

#include <windows.h>
#include <ceddk.h>

#include <omap.h>
#include "omap3530.h"

#include "ISPctrl.h"
#include "params.h"
#include "util.h"

static CAM_ISP_CONFIG_REGS *pCamIspRegs = NULL;

BOOL CIspCtrl::ISPInit()
{
	PHYSICAL_ADDRESS pa;

	pa.QuadPart = (LONGLONG)OMAP_CAMISP_REGS_PA;
	pCamIspRegs = (CAM_ISP_CONFIG_REGS*)MmMapIoSpace(pa, sizeof(CAM_ISP_CONFIG_REGS), FALSE);
    if (pCamIspRegs == NULL)
    {
		ERRORMSG(TRUE,(TEXT("CIspCtrl::ISPInit : Failed to map Camera ISP registers!\r\n")));
		return FALSE;
    }   

	return TRUE;
}

VOID CIspCtrl::ISPDeInit()
{
    if (pCamIspRegs != NULL)
    {
		MmUnmapIoSpace((PVOID)pCamIspRegs, sizeof(CAM_ISP_CONFIG_REGS));
		pCamIspRegs = NULL;
	}
}

BOOL CIspCtrl::ISPEnable(BOOL bEnable)
{
	UINT32 setting = 0;
	ULONG ulTimeout = 500;       

	if (bEnable == TRUE)
	{
		// Init ISP power capability
		ISP_InReg32(&pCamIspRegs->SYSCONFIG, &setting);
		setting &= ~ISP_SYSCONFIG_AUTOIDLE;// Disable auto idle for subsystem
		setting |= (ISP_SYSCONFIG_MIdleMode_NoStandBy << ISP_SYSCONFIG_MIdleMode_SHIFT);// No standby   
		ISP_OutReg32(&pCamIspRegs->SYSCONFIG, setting);

		// Disable all interrupts and clear interrupt status 
		setting = 0;
		ISP_OutReg32(&pCamIspRegs->IRQ0ENABLE, setting);
		ISP_OutReg32(&pCamIspRegs->IRQ1ENABLE, setting);

		ISP_InReg32(&pCamIspRegs->IRQ0STATUS, &setting);
		ISP_OutReg32(&pCamIspRegs->IRQ0STATUS, setting);
		ISP_InReg32(&pCamIspRegs->IRQ1STATUS, &setting);
		ISP_OutReg32(&pCamIspRegs->IRQ1STATUS, setting);

		ISP_InReg32(&pCamIspRegs->SYSCONFIG, &setting);    
		setting |= (ISP_SYSCONFIG_SOFTRESET);   
		ISP_OutReg32(&pCamIspRegs->SYSCONFIG, setting);

		// Wait till the isp wakes out of reset 
		ISP_InReg32(&pCamIspRegs->SYSSTATUS, &setting);
		setting &= 0x1;

		while ((setting != 0x1) && (ulTimeout-- > 0))
		{     
			Sleep(10);// Reset not completed
			ISP_InReg32(&pCamIspRegs->SYSSTATUS, &setting);
			setting &= (0x1);
		}  

		if (ulTimeout <= 0)
		{
			ERRORMSG(TRUE, (TEXT("+ISPEnable: Soft reset timed out!\r\n")));
		}
	}    

	setting = 0;
	if (bEnable == TRUE)
	{
		ISP_InReg32(&pCamIspRegs->CTRL, &setting);
		// Enable using module clock and disable SBL_AUTOIDLE, PAR BRIDGE
		setting |= (ISPCTRL_CCDC_WEN_POL | ISPCTRL_CCDC_CLK_EN | ISPCTRL_CCDC_RAM_EN | (ISPCTRL_SYNC_DETECT_VSFALL << ISPCTRL_SYNC_DETECT_SHIFT) | (0 << ISPCTRL_SHIFT_SHIFT) | ISPCTRL_CCDC_FLUSH
					|ISPCTRL_SBL_WR0_RAM_EN |ISPCTRL_SBL_WR1_RAM_EN | ISPCTRL_SBL_RD_RAM_EN  | (0 << ISPCTRL_PAR_BRIDGE_SHIFT)                    
					);
#ifdef ENABLE_PACK8                            
		setting |=ISPCTRL_SHIFT_2;   
#endif //ENABLE_PACK8           
		ISP_OutReg32(&pCamIspRegs->CTRL, setting);

		// Enable IRQ0
		ISP_OutReg32(&pCamIspRegs->IRQ0ENABLE, 0xFFFFFFFF);    
	}
	else
	{   
		// Disable using module clock
		ISP_InReg32(&pCamIspRegs->CTRL, &setting);
		setting &= ~(ISPCTRL_CCDC_CLK_EN | ISPCTRL_CCDC_RAM_EN |ISPCTRL_SBL_WR0_RAM_EN |ISPCTRL_SBL_WR1_RAM_EN | ISPCTRL_SBL_RD_RAM_EN | ISPCTRL_RSZ_CLK_EN);                  
		ISP_OutReg32(&pCamIspRegs->CTRL, setting);

		// Disable IRQ0
		ISP_OutReg32(&pCamIspRegs->IRQ0ENABLE, 0);  
	}

	return TRUE;
}

DWORD CIspCtrl::GetInterruptStatus()
{
	return INREG32(&pCamIspRegs->IRQ0STATUS);
}

VOID CIspCtrl::InterruptAck(DWORD status)
{
	ISP_OutReg32(&pCamIspRegs->IRQ0STATUS, status);
}
// All rights reserved ADENEO EMBEDDED 2010

#include <windows.h>
#include <ceddk.h>

#include <omap.h>
#include "am3517.h"

#include "ISPctrl.h"

static OMAP_SYSC_GENERAL_REGS *pSysConfRegs = NULL;

BOOL CIspCtrl::ISPInit()
{
	PHYSICAL_ADDRESS pa;

	pa.QuadPart = (LONGLONG)OMAP_SYSC_GENERAL_REGS_PA;
	pSysConfRegs = (OMAP_SYSC_GENERAL_REGS*)MmMapIoSpace(pa, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE);
    if (pSysConfRegs == NULL)
    {
		ERRORMSG(TRUE,(TEXT("CIspCtrl::ISPInit : Failed to map SysConfig registers!\r\n")));
		return FALSE;
    }   

	SETREG32(&pSysConfRegs->CONTROL_IPSS_CLK_CTRL, VPFE_FUNC_CLK_EN);
	SETREG32(&pSysConfRegs->CONTROL_IPSS_CLK_CTRL, VPFE_VBUSP_CLK_EN);

	Sleep(10);

	SETREG32(&pSysConfRegs->CONTROL_IP_SW_RESET, VPFE_PCLK_SW_RST);
	Sleep(10);
	CLRREG32(&pSysConfRegs->CONTROL_IP_SW_RESET, VPFE_PCLK_SW_RST);

	Sleep(10);

	SETREG32(&pSysConfRegs->CONTROL_IP_SW_RESET, VPFE_VBUSP_SW_RST);
	Sleep(10);
	CLRREG32(&pSysConfRegs->CONTROL_IP_SW_RESET, VPFE_VBUSP_SW_RST);
	Sleep(10);

	return TRUE;
}

VOID CIspCtrl::ISPDeInit()
{
	CLRREG32(&pSysConfRegs->CONTROL_IPSS_CLK_CTRL, VPFE_FUNC_CLK_EN);
	CLRREG32(&pSysConfRegs->CONTROL_IPSS_CLK_CTRL, VPFE_VBUSP_CLK_EN);

    if (pSysConfRegs != NULL)
    {
		MmUnmapIoSpace(pSysConfRegs, sizeof(OMAP_SYSC_GENERAL_REGS));
		pSysConfRegs = NULL;
	}
}

BOOL CIspCtrl::ISPEnable(BOOL bEnable)
{
	// Nothing to do here, no ISP controller in AM3517
	return TRUE;
}

DWORD CIspCtrl::GetInterruptStatus()
{
	return 0x80000000;
}

VOID CIspCtrl::InterruptAck(DWORD status)
{
	UNREFERENCED_PARAMETER(status);

	if (pSysConfRegs != NULL)
	{
		SETREG32(&pSysConfRegs->CONTROL_LVL_INTR_CLEAR, (1 << 5));
	}
}
// All rights reserved ADENEO EMBEDDED 2010
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  SDHC controller driver implementation

#include "omap.h"
#include "am389x.h"

//OMAP_SYSC_GENERAL_REGS  *g_pSysConfRegs = NULL;
//
//static BOOL validate_pointer()
//{
//    if (g_pSysConfRegs == NULL)
//    {
//        PHYSICAL_ADDRESS pa;
//        /* Map the sys config registers */
//        pa.QuadPart = (LONGLONG)OMAP_SYSC_GENERAL_REGS_PA;
//        g_pSysConfRegs = (OMAP_SYSC_GENERAL_REGS*) MmMapIoSpace(pa,
//            sizeof (OMAP_SYSC_GENERAL_REGS),
//            FALSE);
//    }
//    return (g_pSysConfRegs != NULL) ? TRUE : FALSE;
//}

void SocResetEmac()
{
	RETAILMSG(1,(L"SocResetEmac\r\n"));
#if 0
    if (validate_pointer())
    {
		DWORD dwStart = 0;
        
		g_pSysConfRegs->CONTROL_IP_SW_RESET |= CPGMACSS_SW_RST;

		// Wait for a few milliseconds
        dwStart = GetTickCount();
		while((GetTickCount() - dwStart) < 10);

        g_pSysConfRegs->CONTROL_IP_SW_RESET &= ~CPGMACSS_SW_RST;
    }
#endif
}


void SocAckInterrupt(DWORD flag)
{
//	RETAILMSG(1,(L"SocAckInterrupt %X\r\n", flag));
    //if (validate_pointer())
    //{
    //    g_pSysConfRegs->CONTROL_LVL_INTR_CLEAR = flag & 0xF;
    //}
}


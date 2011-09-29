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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/******************************************************************************
**
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
**
**  FILENAME:   xllp_Pm_SleepContext.c
**
**  PURPOSE:    contains XLLP RTC primitives.
**
******************************************************************************/

#include <windows.h>
#include "xllp_defs.h"
#include "xllp_clkmgr.h"
#include "xllp_gpio.h"
#include "xllp_intc.h"
#include "xllp_Pm.h"
#include "xllp_ost.h"
#include "xllp_memctrl.h"
#include "xllp_Pm_SleepContext.h"

// External Functions
//
extern void OEMCacheRangeFlush(LPVOID pAddr, DWORD dwLength, DWORD dwFlags);

// These should be  turning into xllp functions real soon.
#ifdef USING_COPROCSUPPORT
extern Xllp_Store_All_WMMX_Regs(P_XLLP_UINT32_T);
extern Xllp_Restore_All_WMMX_Regs(P_XLLP_UINT32_T);
#endif // def USING_COPROCSUPPORT

//  Local declarations
void XllpPmSleepCLevelProcessing(P_XLLP_PM_ENTER_SLEEP_PARAMS_T pSleepParam);
void XllpPmWakeCLevelProcessing(P_XLLP_PM_ENTER_SLEEP_PARAMS_T,
                                P_XLLP_PM_SLEEP_SAVE_DATA_T, XLLP_UINT32_T);
void XllpPmSaveStdRegList (P_XLLP_PM_SLEEP_SAVE_DATA_T pDataSaveArea);
void XllpPmRestoreStdRegList (P_XLLP_PM_SLEEP_SAVE_DATA_T pDataSaveArea);
void XllpPmSaveAllRegLists (P_XLLP_PM_SLEEP_SAVE_DATA_T );
void XllpPmRestoreAllRegLists (P_XLLP_PM_SLEEP_SAVE_DATA_T );

// PM regs may not latch 'til read back.
//  Will not, starting with B0.
static XLLP_VUINT32_T dummyRegReadTarget;


void XllpPmWakeCLevelProcessing(P_XLLP_PM_ENTER_SLEEP_PARAMS_T pSleepParam,
                                P_XLLP_PM_SLEEP_SAVE_DATA_T  pSleepDataArea,
                                XLLP_UINT32_T ulPackedWakeupInfo)
{
    P_XLLP_PWRMGR_T  pPwrMgrRegs = (P_XLLP_PWRMGR_T) pSleepParam->ProcRegs.UAPwrMgrRegs;
    P_XLLP_GPIO_T       pGpioRegs   = (P_XLLP_GPIO_T)   pSleepParam->ProcRegs.UAGPIORegs;
    P_XLLP_OST_T        pOstRegs =      (P_XLLP_OST_T)  pSleepParam->ProcRegs.UAOSTRegs;
    P_XLLP_UINT32_T    pIMControlReg   = (P_XLLP_UINT32_T) pSleepParam->ProcRegs.UAIMControlReg; 
    
    // Restore GPLRx (values restored via GPSR and GPCR regs)
    //
    pGpioRegs->GPSR0 = pSleepDataArea->GPLR0   ;
    pGpioRegs->GPCR0 = ~(pSleepDataArea->GPLR0) ;
    pGpioRegs->GPSR1 = pSleepDataArea->GPLR1   ;
    pGpioRegs->GPCR1 = ~(pSleepDataArea->GPLR1) ;
    pGpioRegs->GPSR2 = pSleepDataArea->GPLR2   ;
    pGpioRegs->GPCR2 = ~(pSleepDataArea->GPLR2) ;
    pGpioRegs->GPSR3 = pSleepDataArea->GPLR3   ;
    pGpioRegs->GPCR3 = ~(pSleepDataArea->GPLR3) ;

    // Restore IMPMCR
    //
    *pIMControlReg = pSleepDataArea->impmcr & XLLP_IMPMCR_USEDBITS;
    
    XllpPmRestoreAllRegLists (pSleepDataArea); 

    // Currently OS uses OSCR0, OSMR0, OSMR1 and OSMR2
    // Restore these regs if PI domain was powered off during sleep
    //
    if (XLLP_PSLR_SL_PI_OFF==(pPwrMgrRegs->PSLR & XLLP_PSLR_SL_PI_MSK))
    {
        pOstRegs->oscr0 = pSleepDataArea->OSCR0;
        pOstRegs->osmr0 = pSleepDataArea->OSMR0;
        pOstRegs->osmr1 = pSleepDataArea->OSMR1;
        pOstRegs->osmr2 = pSleepDataArea->OSMR2;
        pOstRegs->oier    = pSleepDataArea->OIER;
        pOstRegs->oscr0 = pOstRegs->osmr0 - 5;
    }

    // Invalidate the checksum and zero the PSPR because the saved state is about to become obsolete.
    //
    pSleepDataArea->checksum++;
    pPwrMgrRegs->PSPR = 0;

} // XllpPmWakeCLevelProcessing()


//
// Saves context. 
// Call only after all assembly language level data saving is finished.
//
//
void XllpPmSleepCLevelProcessing(P_XLLP_PM_ENTER_SLEEP_PARAMS_T pSleepParam)
{
    P_XLLP_PM_SLEEP_SAVE_DATA_T  pSleepDataArea;
    P_XLLP_PWRMGR_T  pPwrMgrRegs    = (P_XLLP_PWRMGR_T) pSleepParam->ProcRegs.UAPwrMgrRegs;
    P_XLLP_UINT32_T pIMControlReg = (P_XLLP_UINT32_T) pSleepParam->ProcRegs.UAIMControlReg; 
    P_XLLP_OST_T pOstRegs =  (P_XLLP_OST_T)  pSleepParam->ProcRegs.UAOSTRegs;
    P_XLLP_GPIO_T pGpioRegs = (P_XLLP_GPIO_T)   pSleepParam->ProcRegs.UAGPIORegs;

    // Will need after waking.  Startup code only gets physical address
    //  of saved data area, the restoration code needs the virtual addr.
    //
    pSleepDataArea  = pSleepParam->SleepDataAreaVA;
    pSleepDataArea->pSleepDataArea  = pSleepDataArea;

    // Restoration code needs this to access UA addresses of registers
    //
    pSleepDataArea->pSleepParam = pSleepParam;

    // save internal memory power manager control register, IMPMCR
    //
    pSleepDataArea->impmcr = (*pIMControlReg);

    // Currently OS uses OSCR0, OSMR0, OSMR1 and OSMR2.
    // Save these regs if PI domain will be powered off during sleep
    //
    if (XLLP_PSLR_SL_PI_OFF == (pPwrMgrRegs->PSLR & XLLP_PSLR_SL_PI_MSK))
    {
        pSleepDataArea->OSCR0  = pOstRegs->oscr0;
        pSleepDataArea->OSMR0 = pOstRegs->osmr0;
        pSleepDataArea->OSMR1 = pOstRegs->osmr1;
        pSleepDataArea->OSMR2 = pOstRegs->osmr2;
        pSleepDataArea->OIER    = pOstRegs->oier;
    }

    // Save GPIO pin level registers
    //
    pSleepDataArea->GPLR0  =  pGpioRegs->GPLR0;
    pSleepDataArea->GPLR1  =  pGpioRegs->GPLR1;
    pSleepDataArea->GPLR2  =  pGpioRegs->GPLR2;
    pSleepDataArea->GPLR3  =  pGpioRegs->GPLR3;

    // Save word count of saved data area
    // (total number of 32-bit words stored, excluding only checksum))
    //
    pSleepDataArea->SleepAreaWordCount = 
        (sizeof(XLLP_PM_SLEEP_SAVE_DATA_T)/4)   // Checksum is done on 4-byte words
        - 1;                                                        // Don't include the checksum itself.

    // Save function address to jump to on resuming
    //
    pSleepDataArea->AwakeAddr = XllpPmRestoreAfterSleep;

    // Save proc registers 
    //
    XllpPmSaveAllRegLists (pSleepDataArea);

    // Now, set the checksum to validate data at wakeup
    // Must be done after all data saved.
    pSleepDataArea->checksum = XllpPmChecksumSleepDataVi (&pSleepDataArea->SleepAreaWordCount,
                                                pSleepDataArea->SleepAreaWordCount);

    // Make sure that the startup code can find the context.
    pPwrMgrRegs->PSPR = pSleepParam->SleepDataAreaPA;
    dummyRegReadTarget = pPwrMgrRegs->PSPR ;

    // Flush DCache, Flush ICache, Flush TLB
    OEMCacheRangeFlush(0, 0, CACHE_SYNC_WRITEBACK | CACHE_SYNC_INSTRUCTIONS | CACHE_SYNC_FLUSH_I_TLB);

} // XllpPmSleepCLevelProcessing()


void XllpPmSaveStdRegList (P_XLLP_PM_SLEEP_SAVE_DATA_T pDataSaveArea)
{
    UINT32 i = 0;
    UINT32 j = 0;
    
    P_XLLP_PWRMGR_T pPwrMgrRegs = (P_XLLP_PWRMGR_T) pDataSaveArea->pSleepParam->ProcRegs.UAPwrMgrRegs;
    P_XLLP_GPIO_T   pGpioRegs   = (P_XLLP_GPIO_T)   pDataSaveArea->pSleepParam->ProcRegs.UAGPIORegs;
    P_XLLP_INTC_T   pIntcRegs   = (P_XLLP_INTC_T)   pDataSaveArea->pSleepParam->ProcRegs.UAIntcRegs;

    // Save PGSRx regs
    // PGSRx regs will be restored, but must be modified before being saved
    //
    pDataSaveArea->StandardRegListStore[i++] =  pPwrMgrRegs->PGSR0 & XLLP_PM_PGSR0_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pPwrMgrRegs->PGSR1 & XLLP_PM_PGSR1_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pPwrMgrRegs->PGSR2 & XLLP_PM_PGSR2_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pPwrMgrRegs->PGSR3 & XLLP_PM_PGSR3_VLD_MSK;
    
    // Save GPIO regs
    //
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GPDR0 & XLLP_GPIO_GPDR0_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GPDR1 & XLLP_GPIO_GPDR1_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GPDR2 & XLLP_GPIO_GPDR2_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GPDR3 & XLLP_GPIO_GPDR3_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GAFR0_L & XLLP_GPIO_GAFR0_L_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GAFR0_U & XLLP_GPIO_GAFR0_U_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GAFR1_L & XLLP_GPIO_GAFR1_L_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GAFR1_U & XLLP_GPIO_GAFR1_U_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GAFR2_L & XLLP_GPIO_GAFR2_L_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GAFR2_U & XLLP_GPIO_GAFR2_U_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GAFR3_L & XLLP_GPIO_GAFR3_L_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GAFR3_U & XLLP_GPIO_GAFR3_U_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GRER0 & XLLP_GPIO_GRER0_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GRER1 & XLLP_GPIO_GRER1_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GRER2 & XLLP_GPIO_GRER2_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GRER3 & XLLP_GPIO_GRER3_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GFER0 & XLLP_GPIO_GFER0_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GFER1 & XLLP_GPIO_GFER1_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GFER2 & XLLP_GPIO_GFER2_VLD_MSK;
    pDataSaveArea->StandardRegListStore[i++] =  pGpioRegs->GFER3 & XLLP_GPIO_GFER3_VLD_MSK;
    
    // INTC registers
    //
    pDataSaveArea->StandardRegListStore[i++] = pIntcRegs->iclr & XLLP_INTC_ICLR_MASK;
    pDataSaveArea->StandardRegListStore[i++] = pIntcRegs->iclr2 & XLLP_INTC_ICLR2_MASK;
    pDataSaveArea->StandardRegListStore[i++] = pIntcRegs->iccr & XLLP_INTC_ICCR_MASK;
    for (j=0; j<32; j++) {
        pDataSaveArea->StandardRegListStore[i++] = pIntcRegs->ipr[j] & XLLP_INTC_IPR_MASK;    
    }
    pDataSaveArea->StandardRegListStore[i++] = pIntcRegs->ipr2[0] & XLLP_INTC_IPR2_MASK;    
    pDataSaveArea->StandardRegListStore[i++] = pIntcRegs->ipr2[1] & XLLP_INTC_IPR2_MASK;
    pDataSaveArea->StandardRegListStore[i++] = pIntcRegs->icmr    & XLLP_INTC_ICMR_MASK;
    pDataSaveArea->StandardRegListStore[i++] = pIntcRegs->icmr2   & XLLP_INTC_ICMR2_MASK;

    PREFAST_ASSERT(i<=XLLP_PM_SLEEP_STD_REGLIST_CNT);
    
}
void XllpPmRestoreStdRegList (P_XLLP_PM_SLEEP_SAVE_DATA_T pDataSaveArea)
{
    UINT32 i = 0;
    UINT32 j = 0;
    
    P_XLLP_PWRMGR_T pPwrMgrRegs = (P_XLLP_PWRMGR_T) pDataSaveArea->pSleepParam->ProcRegs.UAPwrMgrRegs;
    P_XLLP_GPIO_T   pGpioRegs   = (P_XLLP_GPIO_T)   pDataSaveArea->pSleepParam->ProcRegs.UAGPIORegs;
    P_XLLP_INTC_T   pIntcRegs   = (P_XLLP_INTC_T)   pDataSaveArea->pSleepParam->ProcRegs.UAIntcRegs;

    // Restore PGSRx regs
    //
    pPwrMgrRegs->PGSR0 = pDataSaveArea->StandardRegListStore[i++];
    pPwrMgrRegs->PGSR1 = pDataSaveArea->StandardRegListStore[i++];
    pPwrMgrRegs->PGSR2 = pDataSaveArea->StandardRegListStore[i++];
    pPwrMgrRegs->PGSR3 = pDataSaveArea->StandardRegListStore[i++];
    
    // Restore GPIO regs
    //
    pGpioRegs->GPDR0 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GPDR1 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GPDR2 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GPDR3 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GAFR0_L = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GAFR0_U = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GAFR1_L = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GAFR1_U = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GAFR2_L = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GAFR2_U = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GAFR3_L = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GAFR3_U = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GRER0 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GRER1 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GRER2 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GRER3 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GFER0 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GFER1 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GFER2 = pDataSaveArea->StandardRegListStore[i++];
    pGpioRegs->GFER3 = pDataSaveArea->StandardRegListStore[i++];  

    // INTC registers
    //
    pIntcRegs->iclr  = pDataSaveArea->StandardRegListStore[i++];
    pIntcRegs->iclr2 = pDataSaveArea->StandardRegListStore[i++];
    pIntcRegs->iccr  = pDataSaveArea->StandardRegListStore[i++];
    for (j=0; j<32; j++) {
        pIntcRegs->ipr[j] = pDataSaveArea->StandardRegListStore[i++];
    }
    pIntcRegs->ipr2[0] = pDataSaveArea->StandardRegListStore[i++];
    pIntcRegs->ipr2[1] = pDataSaveArea->StandardRegListStore[i++];
    pIntcRegs->icmr    = pDataSaveArea->StandardRegListStore[i++];
    pIntcRegs->icmr2   = pDataSaveArea->StandardRegListStore[i++];

    PREFAST_ASSERT(i<=XLLP_PM_SLEEP_STD_REGLIST_CNT);
    
}

void XllpPmSaveAllRegLists (P_XLLP_PM_SLEEP_SAVE_DATA_T pDataSaveArea)
{

    // First, store registers from standard list
    XllpPmSaveStdRegList(pDataSaveArea);

    // Also call special coprocessor save function here.
 #ifdef USING_COPROCSUPPORT
    Xllp_Store_All_WMMX_Regs(&pDataSaveArea->IWMMXTRegs[0]);
#endif  

}  // XllpPmSaveAllRegLists()


void XllpPmRestoreAllRegLists (P_XLLP_PM_SLEEP_SAVE_DATA_T pDataSaveArea)
{

    // First, restore registers from standard list
    XllpPmRestoreStdRegList(pDataSaveArea);

    // Also call special coprocessor restore function here.
 #ifdef USING_COPROCSUPPORT
    Xllp_Restore_All_WMMX_Regs(&pDataSaveArea->IWMMXTRegs[0]);
#endif //def USING_COPROCSUPPORT

}   // XllpPmRestoreAllRegLists()


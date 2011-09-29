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
#ifndef __XLLP_PM_SLEEPCONTEXT_H__
#define __XLLP_PM_SLEEPCONTEXT_H__

/******************************************************************************
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
**  FILENAME:   xllp_Pm_SleepContext.h
**
**  PURPOSE:    Contains all XLLP PM sleep specific macros, typedefs, and 
**              prototypes.
**
******************************************************************************/

//
//  NOTE: This file must be kept in sync with xllp_Pm_SleepContext.inc
//

// For sanity check in checksum; increased because of external chksm capability
#define XLLI_MAX_SLEEP_DATA_COUNT 0x400     
#define XLLP_STORE_WMX_REGS_BYTES 156
#define XLLP_IM_BANK_SIZE         0x10000
#define XLLP_PM_SLEEP_STD_REGLIST_CNT      63           // Standard reg list size
#define XLLP_IMPMCR_USEDBITS        0x00FF0FFFUL        //IMPMCR reg used bits

#define XLLP_PM_PWRMODE_SLEEP           0x00000003u   //PWRMODE register value to enter sleep mode
#define XLLP_PM_PWRMODE_DEEPSLEEP   0x00000007u   //PWRMODE register value to enter deep-sleep mode

///////////////////////////////////////////////////////////////////
//
// Uncached addresses of processor regs that are needed by sleep & resume code.
// These must be set before calling sleep code.
//
typedef struct XLLP_PM_PROC_REGS_S
{
    XLLP_UINT32_T UAPwrMgrRegs;
    XLLP_UINT32_T UAGPIORegs;
    XLLP_UINT32_T UAIntcRegs;
    XLLP_UINT32_T UAOSTRegs;
    XLLP_UINT32_T UAMEMCRegs;
    XLLP_UINT32_T UAIMControlReg;

};

/////////////////////////////////////////////////////////////////////////
//
// Definition of parameter structure needed by XllpPmEnterSleep()
//
// Note: keep this in sync with fields defined in xllp_pm_sleepcontext.inc
//
typedef struct  XLLP_PM_ENTER_SLEEP_PARAMS_S
{
    /* start of fields used by ASM language (xllp_pm_sleepcontext.inc) */
    XLLP_UINT32_T                                       SleepDataAreaPA;    // Physical addr of sleep data storage
    struct  XLLP_PM_SLEEP_SAVE_DATA_S*      SleepDataAreaVA;    // Virtual addr of sleep data storage
    XLLP_UINT32_T                                       PWRMODE;        //pwrmode register (set to enter various power modes)
    /* End of fields used by ASM language */

    //uncached base addresses of registers.
    struct XLLP_PM_PROC_REGS_S      ProcRegs;
        
}   XLLP_PM_ENTER_SLEEP_PARAMS_T, *P_XLLP_PM_ENTER_SLEEP_PARAMS_T;



typedef  void (* P_XLLP_VOID_P_SLEEP_DATA_FN_T) (P_XLLP_PM_SLEEP_SAVE_DATA_T);

/////////////////////////////////////////////////////////////////////////
//
//  The C-level definition of the sleep data save area structure.
//
// Note: keep this in sync with fields defined in xllp_pm_sleepcontext.inc
//
typedef struct  XLLP_PM_SLEEP_SAVE_DATA_S
{
    /* start of fields used by ASM language (xllp_pm_sleepcontext.inc) */

    XLLP_UINT32_T checksum            ; // Must be first.
    
    // Word count must be second.
    // Needed for checksum after reset. (total number of 32-bit words stored, excluding only checksum)
    XLLP_UINT32_T SleepAreaWordCount ; 
    P_XLLP_VOID_P_SLEEP_DATA_FN_T   AwakeAddr; 
    struct  XLLP_PM_SLEEP_SAVE_DATA_S* pSleepDataArea;
    P_XLLP_PM_ENTER_SLEEP_PARAMS_T  pSleepParam;

    // Most ARM* registers stored on the stacks of their processor modes.
    //
    XLLP_UINT32_T ENTRY_CPSR;      
    XLLP_UINT32_T ENTRY_SP;        
    XLLP_UINT32_T ENTRY_SPSR;      
    XLLP_UINT32_T SYS_SP;          
    XLLP_UINT32_T FIQ_SP;          
    XLLP_UINT32_T ABT_SP;          
    XLLP_UINT32_T IRQ_SP;          
    XLLP_UINT32_T UND_SP;          
    XLLP_UINT32_T SVC_SP;          

    // Coprocessor-accessed values (Saved Coprocessor regs other than CP0+1 (includes MMU values))
    //
    XLLP_UINT32_T Cp15_ACR_MMU;
    XLLP_UINT32_T Cp15_AUXCR_MMU;
    XLLP_UINT32_T Cp15_TTBR_MMU;
    XLLP_UINT32_T Cp15_DACR_MMU;
    XLLP_UINT32_T Cp15_PID_MMU;
    XLLP_UINT32_T Cp15_CPAR;

    /* End of fields used by ASM language */

    // IMPMCR saved register
    //
    XLLP_UINT32_T    impmcr ;
    
    // OS timer registers saved and restored if PI domain is powered down
    //
    XLLP_UINT32_T OSCR0;
    XLLP_UINT32_T OSMR0;
    XLLP_UINT32_T OSMR1;
    XLLP_UINT32_T OSMR2;
    XLLP_UINT32_T OIER;
    
    // GPIO Pin level registers
    //
    XLLP_UINT32_T GPLR0;
    XLLP_UINT32_T GPLR1;
    XLLP_UINT32_T GPLR2;
    XLLP_UINT32_T GPLR3;

#ifdef USING_COPROCSUPPORT
    XLLP_UINT32_T  IWMMXTRegs[XLLP_STORE_WMX_REGS_BYTES/4];
#endif

    // Storage for register lists from standard peripheral register list
    XLLP_VUINT32_T StandardRegListStore [XLLP_PM_SLEEP_STD_REGLIST_CNT];

} XLLP_PM_SLEEP_SAVE_DATA_T, *P_XLLP_PM_SLEEP_SAVE_DATA_T;



////////////////////////////////////////////////
//
//  Function prototypes
//

#ifdef __cplusplus
extern "C" {
#endif

extern void XllpPmEnterSleep (P_XLLP_PM_ENTER_SLEEP_PARAMS_T);

extern void XllpPmRestoreAfterSleep (P_XLLP_PM_SLEEP_SAVE_DATA_T);
extern XLLP_UINT32_T XllpPmChecksumSleepDataVi (P_XLLP_UINT32_T, 
                                                XLLP_UINT32_T);
#ifdef __cplusplus
}
#endif


#endif  // __XLLP_PM_SLEEPCONTEXT_H__

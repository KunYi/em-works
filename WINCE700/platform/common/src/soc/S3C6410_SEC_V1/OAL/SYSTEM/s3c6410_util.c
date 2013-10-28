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
// Copyright (c) Samsung Electronics. Co. Ltd.  All rights reserved.

#include <windows.h>

#include <oal.h>
#include <s3c6410.h>
#include "soc_cfg.h"

typedef struct _pllcon_
{
    volatile UINT32  e: 1;
    volatile UINT32   : 5;
    volatile UINT32  m:10;
    volatile UINT32   : 2;
    volatile UINT32  p: 6;
    volatile UINT32   : 5;
    volatile UINT32  s: 3;
} PLL_CON_BIT;

typedef struct _clkdiv0_
{
    volatile UINT32 mfc  : 4;
    volatile UINT32 jpeg : 4;
    volatile UINT32 cam  : 4;
    volatile UINT32 secur: 2;
    volatile UINT32      : 2;
    volatile UINT32 pclk : 4;
    volatile UINT32 hclkx2:3;
    volatile UINT32 hclk : 1;
    volatile UINT32      : 3;
    volatile UINT32 mpll : 1;
    volatile UINT32 arm  : 4;
} CLK_DIV0_BIT;


#define PLL_MVAL(t) (((t)>>16)&0x3FF)
#define PLL_PVAL(t) (((t)>>8)&0x3F)
#define PLL_SVAL(t) ((t)&0x7)
#define PCLKDIV(t)      ((((t)>>12)&0xF)+1)
#define HCLKX2DIV(t)    ((((t)>>9)&0x7)+1)
#define HCLKDIV(t)      ((((t)>>8)&0x1)+1)
#define ARMDIV(t)       (((t)&0x7)+1)
//----------------------------------------------------------------------------------------------
//
// Function:     System_GetPCLK
// Description:  Get PCLK value Using FINCLK and MPS PLL Diver, and Clock Divider Register falue
//  
UINT32 System_GetPCLK()
{
    UINT32 uPCLK;
    UINT32 uPLLCLK;

    volatile S3C6410_SYSCON_REG *pSysConReg;    
    pSysConReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);

    if(System_VCheckSyncMode())
    {
        uPLLCLK = System_GetAPLLCLK();
    }
    else
    {
        uPLLCLK = System_GetMPLLCLK();        
    }
    uPCLK = uPLLCLK/HCLKX2DIV(pSysConReg->CLK_DIV0)/(PCLKDIV(pSysConReg->CLK_DIV0));
    
    return uPCLK;
}

UINT32 System_GetHCLK()
{
    UINT32 HCLK;
    UINT32 PLLCLK;

    volatile S3C6410_SYSCON_REG *pSysConReg;    
    pSysConReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);

    if(System_VCheckSyncMode())
    {
        PLLCLK = System_GetAPLLCLK();
    }
    else
    {
        PLLCLK = System_GetMPLLCLK();        
    }
    HCLK = PLLCLK/HCLKX2DIV(pSysConReg->CLK_DIV0)/HCLKDIV(pSysConReg->CLK_DIV0);
    
    return HCLK;
}

UINT32 System_GetARMCLK()
{
    UINT32 PLLCLK;
    UINT32 ARMCLK;

    volatile S3C6410_SYSCON_REG *pSysConReg;    
    pSysConReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);

    PLLCLK = System_GetAPLLCLK();    
    ARMCLK = PLLCLK/ARMDIV(pSysConReg->CLK_DIV0);
    
    return ARMCLK;
}

UINT32 System_GetAPLLCLK()
{
    volatile S3C6410_SYSCON_REG *pSysConReg;    
    UINT32 APLLCLK;
    
    pSysConReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);

    if(pSysConReg->CLK_SRC & 0x1)   // APLL CLOCK SELECT
    {
        APLLCLK = ((FIN_CLK>>PLL_SVAL(pSysConReg->APLL_CON))/PLL_PVAL(pSysConReg->APLL_CON)*PLL_MVAL(pSysConReg->APLL_CON));
    }
    else
    {
        APLLCLK = FIN_CLK;
    }

    return APLLCLK;
}

UINT32 System_GetMPLLCLK()
{
    volatile S3C6410_SYSCON_REG *pSysConReg;    
    UINT32 MPLLCLK;
        
    pSysConReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);    
    
    MPLLCLK = ((FIN_CLK>>PLL_SVAL(pSysConReg->MPLL_CON))/PLL_PVAL(pSysConReg->MPLL_CON)*PLL_MVAL(pSysConReg->MPLL_CON));

    return MPLLCLK;
}

UINT32 System_VCheckSyncMode()
{
    volatile S3C6410_SYSCON_REG *pSysConReg;        
    
    pSysConReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);        
    if(pSysConReg->OTHERS & 0xC0)
    {
        return TRUE;    // SyncMode
    }
    else
    {
        return FALSE;   // ASyncMode
    }
    
}
 
void FillClockInfo(ClockInfo *stCI)
{
    if(stCI)
    {
        stCI->APLLCLK = System_GetAPLLCLK();
        stCI->MPLLCLK = System_GetMPLLCLK();
        stCI->ARMCLK = System_GetARMCLK();
        stCI->HCLK = System_GetHCLK();
        stCI->PCLK = System_GetPCLK();
    }
}
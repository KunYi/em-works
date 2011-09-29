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
**  COPYRIGHT (C) 2002-2003 Intel Corporation.
**
**  This software as well as the software described in it is furnished under
**  license and may only be used or copied in accordance with the terms of the
**  license. The information in this file is furnished for informational use
**  only, is subject to change without notice, and should not be construed as
**  a commitment by Intel Corporation. Intel Corporation assumes no
**  responsibility or liability for any errors or inaccuracies that may appear
**  in this document or any software that may be provided in association with
**  this document. 
**  Except as permitted by such license, no part of this document may be 
**  reproduced, stored in a retrieval system, or transmitted in any form or by
**  any means without the express written consent of Intel Corporation. 
**
**  FILENAME:   xllp_ost.c
**
**  PURPOSE:    contains XLLP OST primitives.
**
******************************************************************************/

#include "xllp_defs.h"
#include "xllp_ost.h"


//
// Forward references
//
static void XllpOstDelayTicks(P_XLLP_OST_T pOstRegs, XLLP_UINT32_T ticks);


//
// XllpOstConfigureTimer
//
// Configures the specified OST match register to cause the (compatible)
// operating system timer to interrupt when the OST count equals that
// assigned to the specified match register.
// 
// Upon completion of this routine, the OST match register is armed 
// and enabled.
//
// Inputs:
//   pOSTHandle     -  handle to structure containing the OST and INTC base register
//                     addresses.
//   matchreg       -  an enumeration specifiying the OST Match register to 
//                     be configured (match registers 0-3, only)
//   matchvalue     -  the unsigned 32-bit value to assign to the specified
//                     match register
//
void XllpOstConfigureTimer 
     (P_XLLP_OST_HANDLE_T pOSTHandle, XLLP_OST_MATCHREG matchreg, XLLP_UINT32_T matchvalue)
{

    P_XLLP_OST_T        pOSTRegs = pOSTHandle->pOSTRegs;
    P_XLLP_INTC_T       pINTCRegs = pOSTHandle->pINTCRegs;
    P_XLLP_VUINT32_T    pOSTMatchReg;
    XLLP_UINT32_T       ossrbit,
                        oierbit,
                        icmrbit;

    switch (matchreg)
    {
        default:
        case MatchReg0:
            ossrbit = XLLP_OSSR_M0;
            oierbit = XLLP_OIER_E0;
            icmrbit = XLLP_INTC_OSMR0;
            pOSTMatchReg = &(pOSTRegs->osmr0);
            break;

        case MatchReg1:
            ossrbit = XLLP_OSSR_M1;
            oierbit = XLLP_OIER_E1;
            icmrbit = XLLP_INTC_OSMR1;
            pOSTMatchReg = &(pOSTRegs->osmr1);
            break;

        case MatchReg2:
            ossrbit = XLLP_OSSR_M2;
            oierbit = XLLP_OIER_E2;
            icmrbit = XLLP_INTC_OSMR2;
            pOSTMatchReg = &(pOSTRegs->osmr2);
            break;

        case MatchReg3:
            ossrbit = XLLP_OSSR_M3;
            oierbit = XLLP_OIER_E3;
            icmrbit = XLLP_INTC_OSMR3;
            pOSTMatchReg = &(pOSTRegs->osmr3);
            break;

		case MatchReg4:
            ossrbit = XLLP_OSSR_M4;
            oierbit = XLLP_OIER_E4;
            icmrbit = XLLP_INTC_OSMRXX_4;
            pOSTMatchReg = &(pOSTRegs->osmr4);
            break;

		case MatchReg5:
            ossrbit = XLLP_OSSR_M5;
            oierbit = XLLP_OIER_E5;
            icmrbit = XLLP_INTC_OSMRXX_4;
            pOSTMatchReg = &(pOSTRegs->osmr5);
            break;
    }

    //
    // Disable interrupts on the specified Match register
    //
    pOSTRegs->oier &= ~(oierbit | XLLP_OIER_RESERVED_BITS);

    //
    // Clear any interrupt on the specified Match register
    //
    pOSTRegs->ossr = ossrbit; 

    // 
	// Set up the match register to expire when the oscr0 reaches
    // the next match interval.
	//
    *pOSTMatchReg = matchvalue; 

    //
    // Enable the Match register interrupt on 
    //
    pOSTRegs->oier|= oierbit;

    //
    // Enable the Match interrupt at the interrupt controller
    //
    pINTCRegs->icmr |= icmrbit; 

    return;

}



//
// XllpOstConfigureMatchReg
// 
// XllpOstConfigureMatchReg assigns the sum of the specified matchincrement value
// and the (compatible) Operating System timer to the specified match register. 
// An interrupt will occur when the compatible OST counter equals that assigned
// to the specified match register.  
// 
// Upon completion of this routine, the OST match register is armed 
// and enabled.
//
// This is a variation of the XllpOstConfigureTimer().  The enhancement being that the
// match increment value is computed here, saving the caller from doing it.
//
// Inputs:
//   pOSTHandle     -  handle to structure containing the OST and INTC base register
//                     addresses.
//   matchreg       -  an enumeration specifiying the OST Match register to 
//                     be configured (match registers 0-3, only)
//   matchincrement -  this value and the compatible OS timer (OSCR0) are added.  The
//                     resulting value is assigned to the specified OST Match register. 
//

void XllpOstConfigureMatchReg
  (P_XLLP_OST_HANDLE_T pOSTHandle, XLLP_OST_MATCHREG matchreg, XLLP_UINT32_T matchincrement)
{
    P_XLLP_OST_T        pOSTRegs = pOSTHandle->pOSTRegs;
    XLLP_UINT32_T       matchvalue;

    //
    // Compute the new match value to load
    //
    matchvalue = pOSTRegs->oscr0 + matchincrement;

    //
    // Configure the timer to interrupt at that match value
    //
    XllpOstConfigureTimer (pOSTHandle, matchreg, matchvalue);

}


// 
// XllpOstDelayMicroSeconds
//
// Provides a delay in microseconds using the OST OSCR0 clock.
//
// Inputs:
//   pOstRegs       -  handle to structure containing the OST a base register
//                     addresses.
//   microseconds   -  the unsigned 32-bit value containing the  number of
//                     microseconds to delay
//
void XllpOstDelayMicroSeconds 
     (P_XLLP_OST_T pOstRegs, XLLP_UINT32_T microseconds)
{
    XLLP_UINT32_T    ticks;

    ticks = microseconds * XLLP_OST_TICKS_US;  // approx. 3 ticks per microsecond. 
    XllpOstDelayTicks (pOstRegs, ticks);

}


// 
// XllpOstDelayMilliSeconds
//
// Provides a delay using the OST OSCR0 clock.
//
// Inputs:
//   pOstRegs       -  handle to structure containing the OST a base register
//                     addresses.
//   milliseconds   -  the unsigned 32-bit value containing the number of
//                     milliseconds to delay
//
void XllpOstDelayMilliSeconds 
     (P_XLLP_OST_T pOstRegs, XLLP_UINT32_T milliseconds)
{
    XLLP_UINT32_T    ticks;

    ticks = milliseconds * XLLP_OST_TICKS_MS;
    XllpOstDelayTicks (pOstRegs, ticks);
    return;
}

  
// 
// XllpOstDelayTicks
//
// Delay the number of ticks specified using the OST OSCR0 clock.
//
// Inputs:
//   pOstRegs       -  handle to structure containing the OST a base register
//                     addresses.
//   ticks          -  the unsigned 32-bit value containing the number of
//                     ticks to delay
//
static void XllpOstDelayTicks 
     (P_XLLP_OST_T pOstRegs, XLLP_UINT32_T ticks)
{    
    XLLP_UINT32_T    expireTime,
                    time;

    time = pOstRegs->oscr0;
	expireTime = time + ticks;

    //
    // Check if we wrapped on the expireTime
    // and delay first part until wrap
    //
	if (expireTime < time) 
    {
		while (time < pOstRegs->oscr0);
	}
	while (pOstRegs->oscr0 <= expireTime);
    return;
}
//-----------------------------------------------------------------------------
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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  intr.c
//
//  This file contains Debug board-specific interrupt code.
//
//-----------------------------------------------------------------------------
#include <cmnintrin.h>
#include "bsp.h"

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrInit
//
BOOL BSPIntrInit()
{
    OALMSGS(OAL_INTR&&OAL_FUNC, (L"+BSPIntrInit\r\n"));

    OALMSGS(OAL_INTR&&OAL_FUNC, (L"-BSPIntrInit\r\n"));

    return TRUE;
}

//------------------------------------------------------------------------------

BOOL BSPIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
    BOOL rc = FALSE;

    UNREFERENCED_PARAMETER(pDevLoc);
    UNREFERENCED_PARAMETER(pCount);
    UNREFERENCED_PARAMETER(pIrqs);
    OALMSGS(OAL_INTR&&OAL_FUNC, (L"+BSPIntrRequestIrq\r\n"));

    OALMSGS(OAL_INTR&&OAL_FUNC, (L"-BSPIntrRequestIrq(rc = %d)\r\n", rc));
    
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrEnableIrq
//
//  This function is called from OALIntrEnableIrq to enable interrupt on
//  board-level interrupt controller.
//
UINT32 BSPIntrEnableIrq(UINT32 irq)
{
    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrEnableIrq\r\n", irq));

    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrEnableIrq(irq = %d)\r\n", irq));

    return irq;
}

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrDisableIrq
//
//  This function is called from OALIntrDisableIrq to disable interrupt on
//  board-level interrupt controller.
//
UINT32 BSPIntrDisableIrq(UINT32 irq)
{
    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDisableIrq\r\n"));

    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDisableIrq(irq = %d)\r\n", irq));

    return irq;
}

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrDoneIrq
//
//  This function is called from OALIntrDoneIrq to finish interrupt on
//  board-level interrupt controller.
//
UINT32 BSPIntrDoneIrq(UINT32 irq)
{
    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDoneIrq\r\n", irq));

    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDoneIrq(irq = %d)\r\n", irq));
    
    return irq;
}

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrActiveIrq
//
//  This function is called from interrupt handler to give BSP chance to
//  translate IRQ in case of board-level interrupt controller.
//
UINT32 BSPIntrActiveIrq(UINT32 irq)
{
    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrActiveIrq\r\n", irq));

    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrActiveIrq(%d)\r\n", irq));

    return irq;
}
//------------------------------------------------------------------------------

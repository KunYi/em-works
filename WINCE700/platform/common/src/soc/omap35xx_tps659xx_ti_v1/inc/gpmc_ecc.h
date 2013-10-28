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
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/

//-----------------------------------------------------------------------------
//
//  File:  gpmc_ecc.h
//
#ifndef __GPMC_ECC_H
#define __GPMC_ECC_H

//------------------------------------------------------------------------------
//  ECC Macros
#define ECC_P1_128_E(val)       ((val) & 0xFF)          /*Bit 0 to 7 */
#define ECC_P1_128_O(val)       ((val >> 16) & 0xFF)    /* Bit 16 to Bit 23*/
#define ECC_P512_2048_E(val)    ((val >> 8) & 0x0F)     /* Bit 8 to 11 */
#define ECC_P512_2048_O(val)    ((val >> 24) & 0x0F)    /* Bit 24 to Bit 27 */

//-----------------------------------------------------------------------------
// defines a set of gpmc based ecc routines

#ifdef __cplusplus
extern "C" {
#endif

VOID
ECC_Init(
    OMAP_GPMC_REGS *pGpmcRegs,
    UINT configMask
    );

VOID
ECC_Result(
    OMAP_GPMC_REGS *pGpmcRegs,
    BYTE *pEcc,
    int size
    );

VOID
ECC_Reset(
    OMAP_GPMC_REGS *pGpmcRegs
    );

BOOL 
ECC_CorrectData(
    OMAP_GPMC_REGS *pGpmcRegs,
    BYTE *pData,                // Data buffer
    int sizeData,               // Count of bytes in data buffer
    BYTE const *pEccOld,        // Pointer to the ECC on flash
    BYTE const *pEccNew         // Pointer to the ECC the caller calculated
    );

#ifdef __cplusplus
}
#endif

#endif // __GPMC_ECC_H
//-----------------------------------------------------------------------------

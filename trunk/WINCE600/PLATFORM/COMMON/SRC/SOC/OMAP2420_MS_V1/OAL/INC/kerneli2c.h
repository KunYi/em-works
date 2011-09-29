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
#ifndef __KERNELI2C_H_
#define __KERNELI2C_H_

// public\common\src\arm\ti\omap2420\drvers\inc\i2ctrans.h
#include "i2ctrans.h"
// included to get I2CTRANS definitions

#ifdef __cplusplus
extern "C" {
#endif

#define KERNELI2C_ERR_INVALIDOPCODE     ((DWORD)-10)
    /* an op code is invalid */
#define KERNELI2C_ERR_INVALIDOPLENGTH   ((DWORD)-20)
    /* an op length is zero */
#define KERNELI2C_ERR_OPBUFFERINVALID   ((DWORD)-30)
    /* an op buffer index/length places at least part of the op outside range of transaction buffer */
#define KERNELI2C_ERR_INVALIDADDR       ((DWORD)-40)
    /* i2c address or address size invalid */

/* done in OEMInit() */
void KERNELI2C_OEMInit(void);

/* done in OALHalPostInit() */
void KERNELI2C_HalPostInit(void);

/* ONLY ISRs should use this call (with inISR set to true) */
void KERNELI2C_NonPreemptibleSubmit(BOOL inISR, DWORD i2cAddr, DWORD addrSize, I2CTRANS *pTrans);

/* ONLY KERNEL preemptible routines should use this call */
void KERNELI2C_PreemptibleSubmit(DWORD i2cAddr, DWORD addrSize, I2CTRANS *pTrans);


#ifdef __cplusplus
};
#endif

#endif

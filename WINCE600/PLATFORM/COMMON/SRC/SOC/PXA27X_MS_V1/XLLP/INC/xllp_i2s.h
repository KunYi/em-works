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
**  COPYRIGHT (C) 2001, 2002 Intel Corporation.
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
**  FILENAME:       xllp_i2s.h
**
**  PURPOSE: contains all I2S specific macros, typedefs, and prototypes.
**           Declares no storage.
**                  
**
******************************************************************************/
#include "xllp_defs.h"
#include "xllp_bcr.h"
#include "xllp_gpio.h"
#include "xllp_clkmgr.h"
#include "xllp_i2c.h"

#ifndef __XLLP_I2S_H__
#define __XLLP_I2S_H__


//  SA -  Serial Audio
/**
  I2S Register Definitions
**/

typedef struct 
{
    XLLP_VUINT32_T SACR0;			/* Global Control Register */
    XLLP_VUINT32_T SACR1;			/* SA i2s/MSB-justified control register*/
    XLLP_UINT32_T RESERVED0;	    /* addr. offset 0x008-0x00C */
    XLLP_VUINT32_T SASR0;			/* SA i2s/MSB-justified interface and FIFO status register */
    XLLP_UINT32_T RESERVED1;		/* addr. offset 0x010-0x014 */
    XLLP_VUINT32_T SAIMR;			/* SA Interrupt Mask Register */
    XLLP_VUINT32_T SAICR;			/* SA Interrupt Clear Register */
    XLLP_UINT32_T RESERVED3[17];	/* addr. offset 0x01C-0x05c */
    XLLP_VUINT32_T SADIV;            /* Audio Clock divider reg */
    XLLP_UINT32_T RESERVED4[7];		/* addr. offset 0x064-0x07c */
    XLLP_VUINT32_T SADR;				/* SA Data register */

} XLLP_I2S_T, *P_XLLP_I2S_T;


/* SACR0 Pin Names  */
#define XLLP_SACR0_ENB			XLLP_BIT_0
#define XLLP_SACR0_BCKD			XLLP_BIT_2
#define XLLP_SACR0_RST			XLLP_BIT_3
#define XLLP_SACR0_EFWR			XLLP_BIT_4
#define XLLP_SACR0_STRF			XLLP_BIT_5

#define XLLP_SACR0_CPUDMA_RWNORM	0x0
#define XLLP_SACR0_CPUDMA_RWTRAN	0x2
#define XLLP_SACR0_CPUDMA_RWRECV	0x3
#define XLLP_SACR0_TFTH				0xF0

#define XLLP_SACR0_TF_MNSIZE_ALL	0

typedef enum 
{
    MXSIZE_8 = 0xD,
    MXSIZE_16 = 0xC,
    MXSIZE_32 = 0x8
} XLLP_SACR0_TF_MXSIZE;         /* Bulverde has four GPIO register banks */

#define XLLP_SACR0_RFTH				0xF00

typedef enum 
{
    MNSIZE_8 = 0x1,
    MNSIZE_16 = 0x3,
    MNSIZE_32 = 0x7
} XLLP_SACR0_RF_MNSIZE;         /* Bulverde has four GPIO register banks */

#define XLLP_SACR0_RF_MXSIZE_ALL	0xE

typedef enum 
{
    SAMP_FREQ8  = 0x48,
    SAMP_FREQ11 = 0x34,
    SAMP_FREQ16 = 0x24,
    SAMP_FREQ22 = 0x1A,
    SAMP_FREQ44 = 0x0D,
    SAMP_FREQ48 = 0x0C
    
} XLLP_SADIV_VAL;         /* Bulverde has four GPIO register banks */



/* SACR1 Pin Names  */
#define XLLP_SACR1_AMSL			XLLP_BIT_0
#define XLLP_SACR1_DREC			XLLP_BIT_3
#define XLLP_SACR1_DRPL			XLLP_BIT_4
#define XLLP_SACR1_ENLBF		XLLP_BIT_5

/* SASR0 Pin Names  */
#define XLLP_SASR0_TNF			XLLP_BIT_0
#define XLLP_SASR0_RNE			XLLP_BIT_1
#define XLLP_SASR0_BSY			XLLP_BIT_2
#define XLLP_SASR0_TFS			XLLP_BIT_3
#define XLLP_SASR0_RFS			XLLP_BIT_4
#define XLLP_SASR0_TUR			XLLP_BIT_5
#define XLLP_SASR0_ROR			XLLP_BIT_6
#define XLLP_SASR0_I2SOFF		XLLP_BIT_7
#define XLLP_SACR0_TFL			0xF0
#define XLLP_SACR0_RFL			0xF00

/* SADIV Pin Names  */
#define XLLP_SADIV_SADIV		0x7F

/* SAICR Pin Names  */
#define XLLP_SAICR_TUR			XLLP_BIT_5
#define XLLP_SAICR_ROR			XLLP_BIT_6


/* SAIMR Pin Names  */
#define XLLP_SAIMR_TFS			XLLP_BIT_3
#define XLLP_SAIMR_RFS			XLLP_BIT_4
#define XLLP_SAIMR_TUR			XLLP_BIT_5
#define XLLP_SAIMR_ROR			XLLP_BIT_6

/* SADR Pin Names  */
#define XLLP_SADR_RDS			0xFF00
#define XLLP_SADR_LDS			0x00FF

// functions
XLLP_BOOL_T XllpI2sMInit(P_XLLP_I2S_T i2s_regs, P_XLLP_I2C_T i2c_regs, P_XLLP_CLKMGR_T clkMgr, 
						 P_XLLP_GPIO_T gpio, P_XLLP_BCR_T Bcr);
XLLP_BOOL_T XllpI2sSInit(P_XLLP_I2S_T i2s_regs, P_XLLP_I2C_T i2c_regs, P_XLLP_CLKMGR_T clkMgr, 
						 P_XLLP_GPIO_T gpio, P_XLLP_BCR_T Bcr);


#endif // end of .h file
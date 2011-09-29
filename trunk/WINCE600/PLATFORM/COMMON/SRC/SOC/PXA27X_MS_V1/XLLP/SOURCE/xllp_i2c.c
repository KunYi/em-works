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
**  COPYRIGHT (C) 2000, 2001 Intel Corporation.
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
**  FILENAME:       xllp_i2c.c
**
**  PURPOSE:        Xllp i2c file
**
**
******************************************************************************/
#include "xllp_i2c.h"
#include "xllp_intc.h"
#define DEBUG 1

/* 
 * Initialization to use I2C bus
 *
 * PARAMETERS:
 *			P_XLLP_I2C_T I2C_regs structure for i2c regs
 *			 P_XLLP_CLKMGR_T clkaddr - address of clkmanager
 *          XLLP_UINT32_T dev_id - Default slave device id for Bulverde
 *
 * RETURNS: True - failure
 *			false - success			
 */
  /*  Enable I2C Interface Unit - 
   *   
   *      XLLP_ICR_GCD  - Disable General Call (will be master)
   *      XLLP_ICR_UIE    - Enable I2C unit
   *      XLLP_ICR_SCLEA - Enable I2C Clock Generator 
   *      
   */

XLLP_BOOL_T XllpI2cInit(P_XLLP_I2C_T I2C_regs, P_XLLP_GPIO_T gpio, P_XLLP_CLKMGR_T clkMgr, XLLP_UINT32_T dev_id)
{
	I2C_regs->ICR = 0;

	clkMgr->cken |=  XLLP_CLKEN_I2C;

	gpio->GPDR3 |= (XLLP_GPIO_BIT_SCL | XLLP_GPIO_BIT_SDA);

	gpio->GAFR3_U |=  ( XLLP_GPIO_AF_BIT_SCL | XLLP_GPIO_AF_BIT_SDA);

	/* Setup I2C slave address */
	I2C_regs->ISAR =  dev_id;

	I2C_regs->ICR = XLLP_ICR_SCLEA;
	I2C_regs->ICR |= XLLP_ICR_UIE;

	return(XLLP_FALSE);
}

/* 
 * Initialization to use PWRI2C bus
 *
 * PARAMETERS:
 *			P_XLLP_I2C_T I2C_regs structure for i2c regs
 *			 P_XLLP_CLKMGR_T clkaddr - address of clkmanager
 *          XLLP_UINT32_T dev_id - Default slave device id for Bulverde
 *
 * RETURNS: True - failure
 *			false - success			
 */
 /*  Enable PWRI2C Interface Unit - 
 *   
 *      
 */

XLLP_BOOL_T XllpPI2cInit(P_XLLP_PI2C_T I2C_regs, P_XLLP_GPIO_T gpio, P_XLLP_CLKMGR_T clkMgr, XLLP_UINT32_T dev_id)
{

	clkMgr->cken |=  XLLP_CLKEN_PWRI2C;

	I2C_regs->PCFR |= 0x40;
	
	I2C_regs->ICR = 0;
	/* Setup I2C slave address */
	I2C_regs->ISAR =  dev_id;

	I2C_regs->ICR = XLLP_ICR_SCLEA;
	I2C_regs->ICR |= XLLP_ICR_UIE;

	return(XLLP_FALSE);
}


/* 
 * Wait for Receive empty status
 *
 * RETURNS: 0 success
 *          1 failure
 */
XLLP_BOOL_T XllpI2cRxFull(P_XLLP_I2C_T I2C_regs, XLLP_OST_T *pOSTRegs, XLLP_INT32_T timeout)
{
	XLLP_UINT32_T temp;

	while (timeout--)
	{
		temp = I2C_regs->ISR;
		if ((temp & XLLP_ISR_IRF) == XLLP_ISR_IRF)
		{
			I2C_regs->ISR = temp | XLLP_ISR_IRF;
			return XLLP_FALSE;
		}
		// delay 1 ms here
		XllpOstDelayMilliSeconds(pOSTRegs, 1);
	}

  return XLLP_TRUE;
}

/* Wait for transmit empty status
 *
 * RETURNS: 0 success
 *          1 failure
 */
XLLP_BOOL_T XllpI2cTxEmpty(P_XLLP_I2C_T I2C_regs, XLLP_OST_T *pOSTRegs, XLLP_INT32_T timeout)
{
	XLLP_UINT32_T temp;

	while (timeout--)
	{
		temp = I2C_regs->ISR;
		if((temp & XLLP_ISR_ITE) == XLLP_ISR_ITE)
		{
			I2C_regs->ISR = temp | XLLP_ISR_ITE;
			if ((temp & XLLP_ISR_ALD) == XLLP_ISR_ALD)
			{
				I2C_regs->ISR |= XLLP_ISR_ALD;
			}
			return XLLP_FALSE;
		}
		// delay 1 ms here
		XllpOstDelayMilliSeconds(pOSTRegs, 1);
	}
	return XLLP_TRUE;
}


XLLP_BOOL_T XllpI2CWrite(P_XLLP_I2C_T I2C_regs, XLLP_OST_T *pOSTRegs, XLLP_UINT8_T slaveAddr, const XLLP_UINT8_T * bytesBuf, XLLP_UINT32_T bytesCount, XLLP_BOOL_T bSendStop)
{
	XLLP_UINT32_T reg;
	int timer = 0;
	
	I2C_regs->IDBR = (slaveAddr << 1) & ~XLLP_IDBR_MODE;
	reg = I2C_regs->ICR;
	reg |= (XLLP_ICR_START | XLLP_ICR_TB);
	reg &= ~(XLLP_ICR_STOP | XLLP_ICR_ALDIE);
	I2C_regs->ICR = reg;

	if (XllpI2cTxEmpty(I2C_regs, pOSTRegs,20) == XLLP_TRUE)
	{
		return XLLP_TRUE;
	}

	// Send all the bytes
	while (bytesCount--)
	{
		I2C_regs->IDBR = (XLLP_UINT32_T)(*bytesBuf++);
		
		reg = I2C_regs->ICR;
		
		reg &= ~XLLP_ICR_START;
		reg |= (XLLP_ICR_ALDIE | XLLP_ICR_TB);
		
		if ((bytesCount == 0) && bSendStop)
			reg |= XLLP_ICR_STOP;
		else
			reg &= ~XLLP_ICR_STOP;

		I2C_regs->ICR = reg;

		if (XllpI2cTxEmpty(I2C_regs, pOSTRegs, 250) == XLLP_TRUE)
		{
			return XLLP_TRUE;
		}
	}

	// Clear the STOP bit always
	I2C_regs->ICR &= ~XLLP_ICR_STOP;
	return XLLP_FALSE;
}

XLLP_BOOL_T XllpI2CRead(P_XLLP_I2C_T I2C_regs, XLLP_OST_T *pOSTRegs, XLLP_UINT8_T slaveAddr, XLLP_UINT8_T * bytesBuf, XLLP_UINT32_T bytesCount, XLLP_BOOL_T bSendStop)
{
	XLLP_UINT32_T reg;

	I2C_regs->IDBR = (slaveAddr << 1) | XLLP_IDBR_MODE;

	reg = I2C_regs->ICR;
	reg |= (XLLP_ICR_START | XLLP_ICR_TB);
	reg &= ~(XLLP_ICR_STOP | XLLP_ICR_ALDIE);
	I2C_regs->ICR = reg;

	if (XllpI2cTxEmpty(I2C_regs, pOSTRegs,20) == XLLP_TRUE)
	{
		return XLLP_TRUE;
	}

	while (bytesCount--)
	{
		reg = I2C_regs->ICR;
		reg &= ~XLLP_ICR_START;
		reg |= XLLP_ICR_ALDIE | XLLP_ICR_TB;
		if (bytesCount == 0)
		{
			reg |= XLLP_ICR_ACKNACK;
			if (bSendStop)
				reg |= XLLP_ICR_STOP;
			else
				reg &= ~XLLP_ICR_STOP;
		} else
		{
			reg &= ~XLLP_ICR_ACKNACK;
		}
		I2C_regs->ICR = reg;

		if (XllpI2cRxFull(I2C_regs, pOSTRegs, 60) == XLLP_TRUE)
		{	
			return XLLP_TRUE;
		}
		reg = I2C_regs->IDBR & 0xFF;
		*bytesBuf++ = (XLLP_UINT8_T)reg;
	}

	I2C_regs->ICR &= ~(XLLP_ICR_STOP | XLLP_ICR_ACKNACK);

	return XLLP_FALSE;
}

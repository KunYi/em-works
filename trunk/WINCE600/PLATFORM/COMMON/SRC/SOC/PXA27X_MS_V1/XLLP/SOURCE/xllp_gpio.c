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
**  COPYRIGHT (C) 2002 Intel Corporation.
**
**  This software as well as the software described in it is furnished under
**  license and may only be used or copied in accordance with the terms of the
**  license. The information in this file is furnished for informational use
**  only, is subject to change without notice, and should not be consXLLP_TRUEd as
**  a commitment by Intel Corporation. Intel Corporation assumes no
**  responsibility or liability for any errors or inaccuracies that may appear
**  in this document or any software that may be provided in association with
**  this document. 
**  Except as permitted by such license, no part of this document may be 
**  reproduced, stored in a retrieval system, or transmitted in any form or by
**  any means without the express written consent of Intel Corporation. 
**
**  FILENAME:   xllp_gpio.c
**
**  PURPOSE:    contains XLLP GPIO primitives.
**  
******************************************************************************/


#include "xllp_gpio.h"
#include "xllp_serialization.h"

/**
 * General-purpose I/O access routines 
 * Registers:
 *	GPLR_0,1,2,3:	line-level registers
 *	GPDR_0,1,2,3:	data-direction registers
 *	GPSR_0,1,2,3:	set registers
 *	GPCR_0,1,2,3:	clear registers
 *	GRER_0,1,2,3:	rising-edge detect registers
 *	GFER_0,1,2,3:	falling-edge detect registers
 *	GEDR_0,1,2,3:	edge-detect status register
 *	GAFR0_L: alternate function select registers
 *  GAFR0_U: alternate function select registers
 *	GAFR1_L: alternate function select registers
 *  GAFR1_U: alternate function select registers
 *	GAFR2_L: alternate function select registers
 *  GAFR2_U: alternate function select registers
 *	GAFR3_L: alternate function select registers
 *  GAFR3_U: alternate function select registers
 */


/**
 * Levels
 */
XLLP_UINT32_T XllpGpioGetState  
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin)
/**
 * Read GPIO pin level from the register specified by aGpioPin
 * GPLR Read Only Register
 */
	{
	XLLP_UINT32_T aGpioPinMask;

	aGpioPinMask = 0x1 << (aGpioPin & 0x1F);

	if(aGpioPin > 95)
		return (pGPIO->GPLR3 & aGpioPinMask);
	else if(aGpioPin > 63) 
		return (pGPIO->GPLR2 & aGpioPinMask);
	else if(aGpioPin > 31) 
		return (pGPIO->GPLR1 & aGpioPinMask);
	else return (pGPIO->GPLR0 & aGpioPinMask);
	}

/**
 * Data direction
 */

/**
 * Read GPIO pin direction DDR
 * return 0=input, 1=output
 */
XLLP_UINT32_T XllpGpioGetDirection 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin)
    {
	XLLP_UINT32_T aGpioPinMask;

	aGpioPinMask = 0x1 << (aGpioPin & 0x1F);

	if(aGpioPin > 95)
		return (pGPIO->GPDR3 & aGpioPinMask);
	else if(aGpioPin > 63) 
		return (pGPIO->GPDR2 & aGpioPinMask);
	else if(aGpioPin > 31) 
		return (pGPIO->GPDR1 & aGpioPinMask);
	else return (pGPIO->GPDR0 & aGpioPinMask);
    }

/**
 * Modify GP DDR For Input Direction
 */
/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 */
void XllpGpioSetDirectionIn 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
    {
	XLLP_UINT32_T LockID;
	XLLP_UINT32_T aGpioPinMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0, aMask1, aMask2, aMask3;
	XLLP_BOOL_T aSet0, aSet1, aSet2, aSet3;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0=aMask1=aMask2=aMask3=0;
	aSet0=aSet1=aSet2=aSet3=XLLP_FALSE;

	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinMask = 0x1u << (aGpioPinArray[i] & 0x1F);
		if(aGpioPinArray[i] > 95)
		{
			aMask3 |= aGpioPinMask;
			aSet3=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 63)
		{
			aMask2 |= aGpioPinMask;
			aSet2=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 31)
		{ 
			aMask1 |= aGpioPinMask;
			aSet1=XLLP_TRUE;
		}
		else
		{
			aMask0 |= aGpioPinMask;
			aSet0=XLLP_TRUE;
		}
			
	}
	
	if(aSet3)
	{
		LockID = XllpLock(GPDR3);
		pGPIO->GPDR3=((pGPIO->GPDR3&~aMask3)&~XLLP_GPIO_PIN_RESERVED_BITS);
		XllpUnlock(LockID);
	}
	if(aSet2)
	{
		LockID = XllpLock(GPDR2);
		pGPIO->GPDR2=((pGPIO->GPDR2)&~aMask2);
		XllpUnlock(LockID);
	}
	if(aSet1)
	{
		LockID = XllpLock(GPDR1);
		pGPIO->GPDR1=((pGPIO->GPDR1)&~aMask1);
		XllpUnlock(LockID);
	}
	if(aSet0)
	{
		LockID = XllpLock(GPDR0);
		pGPIO->GPDR0=((pGPIO->GPDR0)&~aMask0);
		XllpUnlock(LockID);
	}
    }

/**
 * Modify GP DDR For Output Direction
 */
/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 */
void XllpGpioSetDirectionOut 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
    {
	XLLP_UINT32_T LockID;
	XLLP_UINT32_T aGpioPinMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0, aMask1, aMask2, aMask3;
	XLLP_BOOL_T aSet0, aSet1, aSet2, aSet3;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0=aMask1=aMask2=aMask3=0;
	aSet0=aSet1=aSet2=aSet3=XLLP_FALSE;
	
	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinMask = 0x1u << (aGpioPinArray[i] & 0x1F);
		if(aGpioPinArray[i] > 95)
		{
			aMask3 |= aGpioPinMask;
			aSet3=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 63)
		{
			aMask2 |= aGpioPinMask;
			aSet2=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 31)
		{
			aMask1 |= aGpioPinMask;
			aSet1=XLLP_TRUE;
		}
		else
		{
			aMask0 |= aGpioPinMask;
			aSet0=XLLP_TRUE;
		}
	}
	if(aSet3)
	{
		LockID = XllpLock(GPDR3);
		pGPIO->GPDR3=((pGPIO->GPDR3| aMask3)&~XLLP_GPIO_PIN_RESERVED_BITS);
		XllpUnlock(LockID);
	}
	if(aSet2)
	{
		LockID = XllpLock(GPDR2);
		pGPIO->GPDR2=((pGPIO->GPDR2)| aMask2);
		XllpUnlock(LockID);
	}
	if(aSet1)
	{
		LockID = XllpLock(GPDR1);
		pGPIO->GPDR1=((pGPIO->GPDR1)| aMask1);
		XllpUnlock(LockID);
	}
	if(aSet0)
	{
		LockID = XllpLock(GPDR0);
		pGPIO->GPDR0=((pGPIO->GPDR0)| aMask0);
		XllpUnlock(LockID);
	}
    }

/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 * Set GPIO output pin to level'1'
 * Write-0 ignored
 */
void XllpGpioSetOutputState1 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
	{
	XLLP_UINT32_T aGpioPinMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0, aMask1, aMask2, aMask3;
	XLLP_BOOL_T aSet0, aSet1, aSet2, aSet3;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0=aMask1=aMask2=aMask3=0;
	aSet0=aSet1=aSet2=aSet3=XLLP_FALSE;

	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinMask = 0x1u << (aGpioPinArray[i] & 0x1F);
		if(aGpioPinArray[i] > 95)
		{
			aMask3 |= aGpioPinMask;
			aSet3=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 63)
		{
			aMask2 |= aGpioPinMask;
			aSet2=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 31)
		{
			aMask1 |= aGpioPinMask;
			aSet1=XLLP_TRUE;
		}
		else
		{
			aMask0 |= aGpioPinMask;
			aSet0=XLLP_TRUE;
		}
	}
	if(aSet3)
		pGPIO->GPSR3=aMask3;
	if(aSet2)
		pGPIO->GPSR2=aMask2;
	if(aSet1)
		pGPIO->GPSR1=aMask1;
	if(aSet0)
		pGPIO->GPSR0=aMask0;
	}

/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 * Set GPIO output pin to level'0'
 * Write-0 ignored
 */
void XllpGpioSetOutput0 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
	{
	XLLP_UINT32_T aGpioPinMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0, aMask1, aMask2, aMask3;
	XLLP_BOOL_T aSet0, aSet1, aSet2, aSet3;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0=aMask1=aMask2=aMask3=0;
	aSet0=aSet1=aSet2=aSet3=XLLP_FALSE;

	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinMask = 0x1u << (aGpioPinArray[i] & 0x1F);
		if(aGpioPinArray[i] > 95)
		{
			aMask3 |= aGpioPinMask;
			aSet3=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 63)
		{
			aMask2 |= aGpioPinMask;
			aSet2=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 31)
		{
			aMask1 |= aGpioPinMask;
			aSet1=XLLP_TRUE;
		}
		else
		{
			aMask0 |= aGpioPinMask;
			aSet0=XLLP_TRUE;
		}
	}
	if(aSet3)
		pGPIO->GPCR3=aMask3;
	if(aSet2)
		pGPIO->GPCR2=aMask2;
	if(aSet1)
		pGPIO->GPCR1=aMask1;
	if(aSet0)
		pGPIO->GPCR0=aMask0;
	}

/**
 * Read GRER for a Gpio pin specified by aGpioPin 
 */
XLLP_UINT32_T XllpGpioGetRisingDetectEnable 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin)
/**
 * Read GRER: 
 * return 1=edge detect enable, 0=edge detect disable
 */
	{
	
	XLLP_UINT32_T aGpioPinMask;

	aGpioPinMask = 0x1 << (aGpioPin & 0x1F);

	if(aGpioPin > 95)
		return (pGPIO->GRER3& aGpioPinMask);
	else if(aGpioPin > 63) 
		return (pGPIO->GRER2 & aGpioPinMask);
	else if(aGpioPin > 31) 
		return (pGPIO->GRER1 & aGpioPinMask);
	else return (pGPIO->GRER0 & aGpioPinMask);
	}

/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 * Disable Rising Edge Detect
 */
void XllpGpioSetRisingDetectDisable 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
	{
	XLLP_UINT32_T LockID;
	XLLP_UINT32_T aGpioPinMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0, aMask1, aMask2, aMask3;
	XLLP_BOOL_T aSet0, aSet1, aSet2, aSet3;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0=aMask1=aMask2=aMask3=0;
	aSet0=aSet1=aSet2=aSet3=XLLP_FALSE;

	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinMask = 0x1u << (aGpioPinArray[i] & 0x1F);
		if(aGpioPinArray[i] > 95)
		{
			aMask3 |= aGpioPinMask;
			aSet3=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 63)
		{
			aMask2 |= aGpioPinMask;
			aSet2=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 31)
		{
			aMask1 |= aGpioPinMask;
			aSet1=XLLP_TRUE;
		}
		else
		{
			aMask0 |= aGpioPinMask;
			aSet0=XLLP_TRUE;
		}
	}
	if(aSet3)
	{
		LockID = XllpLock(GRER3);
		pGPIO->GRER3=((pGPIO->GRER3&~aMask3)&~XLLP_GPIO_PIN_RESERVED_BITS);
		XllpUnlock(LockID);
	}
	if(aSet2)
	{
		LockID = XllpLock(GRER2);
		pGPIO->GRER2=((pGPIO->GRER2)&~aMask2);
		XllpUnlock(LockID);
	}
	if(aSet1)
	{
		LockID = XllpLock(GRER1);
		pGPIO->GRER1=((pGPIO->GRER1)&~aMask1);
		XllpUnlock(LockID);
	}
	if(aSet0)
	{
		LockID = XllpLock(GRER0);
		pGPIO->GRER0=((pGPIO->GRER0)&~aMask0);
		XllpUnlock(LockID);
	}
	}

/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 * Enable Rising Edge Detect
 */
void XllpGpioSetRisingDetectEnable 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
	{
	XLLP_UINT32_T LockID;
	XLLP_UINT32_T aGpioPinMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0, aMask1, aMask2, aMask3;
	XLLP_BOOL_T aSet0, aSet1, aSet2, aSet3;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0=aMask1=aMask2=aMask3=0;
	aSet0=aSet1=aSet2=aSet3=XLLP_FALSE;

	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinMask = 0x1u << (aGpioPinArray[i] & 0x1F);
		if(aGpioPinArray[i] > 95)
		{
			aMask3 |= aGpioPinMask;
			aSet3=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 63)
		{
			aMask2 |= aGpioPinMask;
			aSet2=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 31)
		{
			aMask1 |= aGpioPinMask;
			aSet1=XLLP_TRUE;
		}
		else
		{
			aMask0 |= aGpioPinMask;
			aSet0=XLLP_TRUE;
		}
	}
	if(aSet3)
	{
		LockID = XllpLock(GRER3);
		pGPIO->GRER3=((pGPIO->GRER3|aMask3)&~XLLP_GPIO_PIN_RESERVED_BITS);
		XllpUnlock(LockID);
	}
	if(aSet2)
	{
		LockID = XllpLock(GRER2);
		pGPIO->GRER2=((pGPIO->GRER2)|aMask2);
		XllpUnlock(LockID);
	}
	if(aSet1)
	{
		LockID = XllpLock(GRER1);
		pGPIO->GRER1=((pGPIO->GRER1)|aMask1);
		XllpUnlock(LockID);
	}
	if(aSet0)
	{
		LockID = XllpLock(GRER0);
		pGPIO->GRER0=((pGPIO->GRER0)|aMask0);
		XllpUnlock(LockID);
	}
	}

/**
 * Read GFER for a Gpio pin specified by aGpioPin 
 */
XLLP_UINT32_T XllpGpioGetFallingDetectEnable 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin)
/**
 * Read GFER: 
 * return 1=falling edge detect enable, 0=falling edge detect disable
 */
	{
	XLLP_UINT32_T aGpioPinMask;

	aGpioPinMask = 0x1 << (aGpioPin & 0x1F);

	if(aGpioPin > 95)
		return (pGPIO->GFER3 & aGpioPinMask);
	else if(aGpioPin > 63) 
		return (pGPIO->GFER2 & aGpioPinMask);
	else if(aGpioPin > 31) 
		return (pGPIO->GFER1 & aGpioPinMask);
	else return (pGPIO->GFER0 & aGpioPinMask);
	}

/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 * Disable Falling Edge Detect
 */
void XllpGpioSetFallingEdgeDetectDisable 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
	{
	XLLP_UINT32_T LockID;
	XLLP_UINT32_T aGpioPinMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0, aMask1, aMask2, aMask3;
	XLLP_BOOL_T aSet0, aSet1, aSet2, aSet3;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0=aMask1=aMask2=aMask3=0;
	aSet0=aSet1=aSet2=aSet3=XLLP_FALSE;

	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinMask = 0x1u << (aGpioPinArray[i] & 0x1F);
		if(aGpioPinArray[i] > 95)
		{
			aMask3 |= aGpioPinMask;
			aSet3=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 63)
		{
			aMask2 |= aGpioPinMask;
			aSet2=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 31)
		{
			aMask1 |= aGpioPinMask;
			aSet1=XLLP_TRUE;
		}
		else
		{
			aMask0 |= aGpioPinMask;
			aSet0=XLLP_TRUE;
		}
	}
	if(aSet3)
	{
		LockID = XllpLock(GFER3);
		pGPIO->GFER3=((pGPIO->GFER3&~aMask3)&~XLLP_GPIO_PIN_RESERVED_BITS);
		XllpUnlock(LockID);
	}
	if(aSet2)
	{
		LockID = XllpLock(GFER2);
		pGPIO->GFER2=((pGPIO->GFER2)&~aMask2);
		XllpUnlock(LockID);
	}
	if(aSet1)
	{
		LockID = XllpLock(GFER1);
		pGPIO->GFER1=((pGPIO->GFER1)&~aMask1);
		XllpUnlock(LockID);
	}
	if(aSet0)
	{
		LockID = XllpLock(GFER0);
		pGPIO->GFER0=((pGPIO->GFER0)&~aMask0);
		XllpUnlock(LockID);
	}
	}

/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 * Enable Falling Edge Detect
 */
void XllpGpioSetFallingEdgeDetectEnable 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
	{
	XLLP_UINT32_T LockID;
	XLLP_UINT32_T aGpioPinMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0, aMask1, aMask2, aMask3;
	XLLP_BOOL_T aSet0, aSet1, aSet2, aSet3;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0=aMask1=aMask2=aMask3=0;
	aSet0=aSet1=aSet2=aSet3=XLLP_FALSE;
	
	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinMask = 0x1u << (aGpioPinArray[i] & 0x1F);
		if(aGpioPinArray[i] > 95)
		{
			aMask3 |= aGpioPinMask;
			aSet3=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 63)
		{
			aMask2 |= aGpioPinMask;
			aSet2=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 31)
		{
			aMask1 |= aGpioPinMask;
			aSet1=XLLP_TRUE;
		}
		else
		{
			aMask0 |= aGpioPinMask;
			aSet0=XLLP_TRUE;
		}
	}
	if(aSet3)
	{
		LockID = XllpLock(GFER3);
		pGPIO->GFER3=((pGPIO->GFER3|aMask3)&~XLLP_GPIO_PIN_RESERVED_BITS);
		XllpUnlock(LockID);
	}
	if(aSet2)
	{
		LockID = XllpLock(GFER2);
		pGPIO->GFER2=((pGPIO->GFER2)|aMask2);
		XllpUnlock(LockID);
	}
	if(aSet1)
	{
		LockID = XllpLock(GFER1);
		pGPIO->GFER1=((pGPIO->GFER1)|aMask1);
		XllpUnlock(LockID);
	}
	if(aSet0)
	{
		LockID = XllpLock(GFER0);
		pGPIO->GFER0=((pGPIO->GFER0)|aMask0);
		XllpUnlock(LockID);
	}
	}

/*
 * Get the edge-detect status of a Gpio pin.  
 */
XLLP_UINT32_T XllpGpioGetEdgeDetectStatus 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin)
/**
 * Read edge-detect status (GEDR): 
 * return 1=edge occured, 0=no edge occur
 */
	{
	XLLP_UINT32_T aGpioPinMask;

	aGpioPinMask = 0x1 << (aGpioPin & 0x1F);

	if(aGpioPin > 95)
		return (pGPIO->GEDR3& aGpioPinMask); 
	else if(aGpioPin > 63) 
		return (pGPIO->GEDR2 & aGpioPinMask);
	else if(aGpioPin > 31) 
		return (pGPIO->GEDR1 & aGpioPinMask);
	else return (pGPIO->GEDR0 & aGpioPinMask);
	}

/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 * clear edge-detect status bits specified bu aGpioPinArray[]
 */
void XllpGpioClearEdgeDetectStatus 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
	{
	XLLP_UINT32_T aGpioPinMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0, aMask1, aMask2, aMask3;
	XLLP_BOOL_T aSet0, aSet1, aSet2, aSet3;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0=aMask1=aMask2=aMask3=0;
	aSet0=aSet1=aSet2=aSet3=XLLP_FALSE;
	
	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinMask = 0x1u << (aGpioPinArray[i] & 0x1F);
		if(aGpioPinArray[i] > 95)
		{
			aMask3 |= aGpioPinMask;
			aSet3=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 63)
		{
			aMask2 |= aGpioPinMask;
			aSet2=XLLP_TRUE;
		}
		else if(aGpioPinArray[i] > 31)
		{
			aMask1 |= aGpioPinMask;
			aSet1=XLLP_TRUE;
		}
		else
		{
			aMask0 |= aGpioPinMask;
			aSet0=XLLP_TRUE;
		}
	}
	if(aSet3)
		pGPIO->GEDR3= aMask3;
	if(aSet2)
		pGPIO->GEDR2= aMask2;
	if(aSet1)
		pGPIO->GEDR1= aMask1;
	if(aSet0)
		pGPIO->GEDR0= aMask0;
	}

/**
 * Alternative functions
 */

/**
 * Read alternate function value for Gpio pin specified by aGpioPin
 * aGpioPin=0 to 120
 */
XLLP_UINT32_T XllpGpioGetAlternateFn 
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin)
	{
	XLLP_UINT32_T aGpioPinAFMask;

	aGpioPinAFMask = 0x3u << ((aGpioPin & 0xF)*2);

	if (aGpioPin>111)
		return (pGPIO->GAFR3_U & aGpioPinAFMask);
	else if (aGpioPin>95)
		return (pGPIO->GAFR3_L & aGpioPinAFMask);
	else if (aGpioPin>79)
		return (pGPIO->GAFR2_U & aGpioPinAFMask);
	else if (aGpioPin>63)
		return (pGPIO->GAFR2_L & aGpioPinAFMask);
	else if (aGpioPin>47)
		return (pGPIO->GAFR1_U & aGpioPinAFMask);
	else if (aGpioPin>31)
		return (pGPIO->GAFR1_L & aGpioPinAFMask);
	else if (aGpioPin>15)
		return (pGPIO->GAFR0_U & aGpioPinAFMask);
	else return (pGPIO->GAFR0_L & aGpioPinAFMask);
	}
/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 *
 * aAfValueArray[]=array of GPIO pins alternate function values
 * aAfValueArray[0] = size of array
 *
 * Set GPIO pins alternate function values
 *
 * IMPORTANT:THE ORDER OF aAfValueArray[] HAS TO MATCH THE ORDER OF aGpioPinArray[] 
 */
void XllpGpioSetAlternateFn
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[], XLLP_UINT32_T aAfValueArray[])
	{
	XLLP_UINT32_T LockID;
	XLLP_UINT32_T aGpioPinAFMask;
	XLLP_UINT32_T aGpioPinAFValue;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0_U, aMask0_L, aMask1_U, aMask1_L; 
	XLLP_UINT32_T aMask2_U, aMask2_L, aMask3_U, aMask3_L;
	XLLP_UINT32_T aAFnV0_U, aAFnV0_L, aAFnV1_U, aAFnV1_L; 
	XLLP_UINT32_T aAFnV2_U, aAFnV2_L, aAFnV3_U, aAFnV3_L;
	XLLP_BOOL_T aSet0_U, aSet0_L, aSet1_U, aSet1_L; 
	XLLP_BOOL_T aSet2_U, aSet2_L, aSet3_U, aSet3_L;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0_U=aMask0_L=aMask1_U=aMask1_L=0;
	aMask2_U=aMask2_L=aMask3_U=aMask3_L=0;
	aAFnV0_U=aAFnV0_L=aAFnV1_U=aAFnV1_L=0;
	aAFnV2_U=aAFnV2_L=aAFnV3_U=aAFnV3_L=0;
	aSet0_U=aSet0_L=aSet1_U=aSet1_L=XLLP_FALSE;
	aSet2_U=aSet2_L=aSet3_U=aSet3_L=XLLP_FALSE;

	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinAFMask = 0x3u << ((aGpioPinArray[i] & 0xF)*2);
		aGpioPinAFValue = aAfValueArray[i] << ((aGpioPinArray[i] & 0xF)*2);
		if (aGpioPinArray[i]>111)
		{
			aAFnV3_U |= aGpioPinAFValue;
			aMask3_U |= aGpioPinAFMask;
			aSet3_U=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>95)
		{
			aAFnV3_L |= aGpioPinAFValue;
			aMask3_L |= aGpioPinAFMask;
			aSet3_L=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>79)
		{
			aAFnV2_U |= aGpioPinAFValue;
			aMask2_U |= aGpioPinAFMask;
			aSet2_U=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>63)
		{
			aAFnV2_L |= aGpioPinAFValue;
			aMask2_L |= aGpioPinAFMask;
			aSet2_L=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>47)
		{
			aAFnV1_U |= aGpioPinAFValue;
			aMask1_U |= aGpioPinAFMask;
			aSet1_U=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>31)
		{
			aAFnV1_L |= aGpioPinAFValue;
			aMask1_L |= aGpioPinAFMask;
			aSet1_L=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>15)
		{
			aAFnV0_U |= aGpioPinAFValue;
			aMask0_U |= aGpioPinAFMask;
			aSet0_U=XLLP_TRUE;
		}
		else 
		{
			aAFnV0_L |= aGpioPinAFValue;
			aMask0_L |= aGpioPinAFMask;
			aSet0_L=XLLP_TRUE;
		}
	}
	if(aSet3_U)
	{
		LockID = XllpLock(GAFR3_U);
		pGPIO->GAFR3_U=(((pGPIO->GAFR3_U&~aMask3_U)|aAFnV3_U)&~XLLP_GPIO_ALT_RESERVED_BITS);
		XllpUnlock(LockID);
	}
	if(aSet3_L)
	{
		LockID = XllpLock(GAFR3_L);
		pGPIO->GAFR3_L=((pGPIO->GAFR3_L&~aMask3_L)|aAFnV3_L);
		XllpUnlock(LockID);
	}
	if(aSet2_U)
	{
		LockID = XllpLock(GAFR2_U);
		pGPIO->GAFR2_U=((pGPIO->GAFR2_U&~aMask2_U)|aAFnV2_U);
		XllpUnlock(LockID);
	}
	if(aSet2_L)
	{
		LockID = XllpLock(GAFR2_L);
		pGPIO->GAFR2_L=((pGPIO->GAFR2_L&~aMask2_L)|aAFnV2_L);
		XllpUnlock(LockID);
	}
	if(aSet1_U)
	{
		LockID = XllpLock(GAFR1_U);
		pGPIO->GAFR1_U=((pGPIO->GAFR1_U&~aMask1_U)|aAFnV1_U);
		XllpUnlock(LockID);
	}
	if(aSet1_L)
	{
		LockID = XllpLock(GAFR1_L);
		pGPIO->GAFR1_L=((pGPIO->GAFR1_L&~aMask1_L)|aAFnV1_L);
		XllpUnlock(LockID);
	}
	if(aSet0_U)
	{
		LockID = XllpLock(GAFR0_U);
		pGPIO->GAFR0_U=((pGPIO->GAFR0_U&~aMask0_U)|aAFnV0_U);
		XllpUnlock(LockID);
	}
	if(aSet0_L)
	{
		LockID = XllpLock(GAFR0_L);
		pGPIO->GAFR0_L=((pGPIO->GAFR0_L&~aMask0_L)|aAFnV0_L);
		XllpUnlock(LockID);
	}
	}

/*
 * aGpioPinArray[]=array of GPIO pins
 * aGpioPinArray[0] = size of array
 *
 * Clear GPIO pin alternate function value
 */
void XllpGpioClearAlternateFn
     (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[])
	{
	XLLP_UINT32_T LockID;
	XLLP_UINT32_T aGpioPinAFMask;
	XLLP_UINT32_T aSizeArray;
	XLLP_UINT32_T aMask0_U, aMask0_L, aMask1_U, aMask1_L; 
	XLLP_UINT32_T aMask2_U, aMask2_L, aMask3_U, aMask3_L;
	XLLP_BOOL_T aSet0_U, aSet0_L, aSet1_U, aSet1_L; 
	XLLP_BOOL_T aSet2_U, aSet2_L, aSet3_U, aSet3_L;
	XLLP_UINT32_T i;

	//determine size of array
	aSizeArray = aGpioPinArray[0];
	aMask0_U=aMask0_L=aMask1_U=aMask1_L=0;
	aMask2_U=aMask2_L=aMask3_U=aMask3_L=0;
	aSet0_U=aSet0_L=aSet1_U=aSet1_L=XLLP_FALSE;
	aSet2_U=aSet2_L=aSet3_U=aSet3_L=XLLP_FALSE;

	for(i=1; i<=aSizeArray; i++)
	{
		aGpioPinAFMask = 0x3u << ((aGpioPinArray[i] & 0xF)*2);
		if (aGpioPinArray[i]>111)
		{
			aMask3_U |= aGpioPinAFMask;
			aSet3_U=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>95)
		{
			aMask3_L |= aGpioPinAFMask;
			aSet3_L=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>79)
		{
			aMask2_U |= aGpioPinAFMask;
			aSet2_U=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>63)
		{
			aMask2_L |= aGpioPinAFMask;
			aSet2_L=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>47)
		{
			aMask1_U |= aGpioPinAFMask;
			aSet1_U=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>31)
		{
			aMask1_L |= aGpioPinAFMask;
			aSet1_L=XLLP_TRUE;
		}
		else if (aGpioPinArray[i]>15)
		{
			aMask0_U |= aGpioPinAFMask;
			aSet0_U=XLLP_TRUE;
		}
		else
		{
			aMask0_L |= aGpioPinAFMask;
			aSet0_L=XLLP_TRUE;
		}
	}
	if(aSet3_U)
	{
		LockID = XllpLock(GAFR3_U);
		pGPIO->GAFR3_U=((pGPIO->GAFR3_U&~aMask3_U)&~XLLP_GPIO_ALT_RESERVED_BITS);
		XllpUnlock(LockID);
	}
	if(aSet3_L)
	{
		LockID = XllpLock(GAFR3_L);
		pGPIO->GAFR3_L=(pGPIO->GAFR3_L&~aMask3_L);
		XllpUnlock(LockID);
	}
	if(aSet2_U)
	{
		LockID = XllpLock(GAFR2_U);
		pGPIO->GAFR2_U=(pGPIO->GAFR2_U&~aMask2_U);
		XllpUnlock(LockID);
	}
	if(aSet2_L)
	{
		LockID = XllpLock(GAFR2_L);
		pGPIO->GAFR2_L=(pGPIO->GAFR2_L&~aMask2_L);
		XllpUnlock(LockID);
	}
	if(aSet1_U)
	{
		LockID = XllpLock(GAFR1_U);
		pGPIO->GAFR1_U=(pGPIO->GAFR1_U&~aMask1_U);
		XllpUnlock(LockID);
	}
	if(aSet1_L)
	{
		LockID = XllpLock(GAFR1_L);
		pGPIO->GAFR1_L=(pGPIO->GAFR1_L&~aMask1_L);
		XllpUnlock(LockID);
	}
	if(aSet0_U)
	{
		LockID = XllpLock(GAFR0_U);
		pGPIO->GAFR0_U=(pGPIO->GAFR0_U&~aMask0_U);
		XllpUnlock(LockID);
	}
	if(aSet0_L)
	{
		LockID = XllpLock(GAFR0_L);
		pGPIO->GAFR0_L=(pGPIO->GAFR0_L&~aMask0_L);
		XllpUnlock(LockID);
	}
	}

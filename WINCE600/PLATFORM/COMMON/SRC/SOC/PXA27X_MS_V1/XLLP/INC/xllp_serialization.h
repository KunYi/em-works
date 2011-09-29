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
#ifndef XLLP_SERIALIZATION
#define XLLP_SERIALIZATION
typedef enum XLLP_PROTECTED_REGISTERS
{
/*
//  Mainstone Board Level Registers
*/
	MISC_WR,	/* Miscellaneous Write Register 1 */
	MISC_WR2,	/* Miscellaneous Write Register 2 */
	INT_MSK_EN, /* Platform Interrupt Mask/Enable Register */
	INT_SET_CLR, /* Platform Interrupt Set/Clear Register */
/*
//  GPIO
*/
	GPDR0,		/* Data Direction Reg. Bank 0 */
	GPDR1,      /* Data Direction Reg. Bank 1 */
	GPDR2,      /* Data Direction Reg. Bank 2 */
	GPSR0,      /* Pin Output Set Reg. Bank 0 */
	GPSR1,      /* Pin Output Set Reg. Bank 1 */
	GPSR2,      /* Pin Output Set Reg. Bank 2 */
	GPCR0,      /* Pin Output Clr Reg. Bank 0 */
	GPCR1,      /* Pin Output Clr Reg. Bank 1 */
	GPCR2,      /* Pin Output Clr Reg. Bank 2 */
	GRER0,      /* Ris. Edge Detect Enable Reg. Bank 0 */
	GRER1,      /* Ris. Edge Detect Enable Reg. Bank 1 */
	GRER2,      /* Ris. Edge Detect Enable Reg. Bank 2 */
	GFER0,      /* Fal. Edge Detect Enable Reg. Bank 0 */
	GFER1,      /* Fal. Edge Detect Enable Reg. Bank 1 */
	GFER2,      /* Fal. Edge Detect Enable Reg. Bank 2 */
	GEDR0,      /* Edge Detect Status Reg. Bank 0 */
	GEDR1,      /* Edge Detect Status Reg. Bank 1 */
	GEDR2,      /* Edge Detect Status Reg. Bank 2 */
	GAFR0_L,    /* Alt. Function Select Reg.[  0:15 ] */
	GAFR0_U,    /* Alt. Function Select Reg.[ 16:31 ] */
	GAFR1_L,    /* Alt. Function Select Reg.[ 32:47 ] */
	GAFR1_U,    /* Alt. Function Select Reg.[ 48:63 ] */
	GAFR2_L,    /* Alt. Function Select Reg.[ 64:79 ] */
	GAFR2_U,    /* Alt. Function Select Reg.[ 80:95 ] */
	GAFR3_L,    /* Alt. Function Select Reg.[ 96:111] */
	GAFR3_U,    /* Alt. Function Select Reg.[112:120] */
	GPLR3,      /* Level Detect Reg. Bank 3 */
	GPDR3,      /* Data Direction Reg. Bank 3 */
	GPSR3,      /* Pin Output Set Reg. Bank 3 */
	GPCR3,      /* Pin Output Clr Reg. Bank 3 */
	GRER3,      /* Ris. Edge Detect Enable Reg. Bank 3 */
	GFER3,      /* Fal. Edge Detect Enable Reg. Bank 3 */
	GEDR3,      /* Edge Detect Status Reg. Bank 3 */
/*
//  CLKMAN
*/
	CCCR,       /* Core Clock Configuration Register */
	CKEN,       /* Clock Enable Register */
	OSCC,       /* Oscillator Configuration Register */
/*
//  OST
*/
	OSMR0,	/* OS timer match register 0 */
	OSMR1,  /* OS timer match register 1 */
	OSMR2,  /* OS timer match register 2 */
	OSMR3,  /* OS timer match register 3 */
	OSCR0,  /* OS timer counter register 0(compatible) */
	OSSR,   /* OS timer status register */
	OWER,   /* OS timer watchdog enable register */
	OIER,   /* OS timer interrupt enable register */
	OSCR4,	/* OS timer counter register 4 */
	OSCR5,	/* OS timer counter register  5 */
	OSCR6,	/* OS timer counter register  6 */
	OSCR7,	/* OS timer counter register  7 */
	OSCR8,	/* OS timer counter register  8 */
	OSCR9,	/* OS timer counter register  9 */
	OSCR10,	/* OS timer counter register  10 */
	OSCR11,	/* OS timer counter register  11 */
	OSMR4,	/* OS timer match register 4 */
	OSMR5,	/* OS timer match register 5 */
	OSMR6,	/* OS timer match register 6 */
	OSMR7,	/* OS timer match register 7 */
	OSMR8,	/* OS timer match register 8 */
	OSMR9,	/* OS timer match register 9 */
	OSMR10,	/* OS timer match register 10 */
	OSMR11,	/* OS timer match register 11 */
	OMCR4,	/* OS timer match control register 4 */
	OMCR5,	/* OS timer match control register 5 */
	OMCR6,	/* OS timer match control register 6 */
	OMCR7,	/* OS timer match control register 7 */
	OMCR8,	/* OS timer match control register 8 */
	OMCR9,	/* OS timer match control register 9 */
	OMCR10,	/* OS timer match control register 10 */
	OMCR11,	/* OS timer match control register 11 */
/*
//  RTC
*/
    RCNR,     	/*  RTC count register */
    RTAR,     	/*  RTC alarm register */
    RTSR,     	/*  RTC status register */
    RTTR,     	/*  RTC timer trim register */
    RDCR,     	/*  RTC day counter register */
    RYCR,		/*  RTC year counter register */
    RDAR1,    	/*  RTC day alarm register 1 */
    RYAR1,    	/*  RTC year alarm register 1 */
    RDAR2,    	/*  RTC day alarm register 2 */
    RYAR2,    	/*  RTC year alarm register 2 */
    SWCR,		/*  RTC stopwatch counter */
    SWAR1,    	/*  RTC stopwatch alarm register 1 */
    SWAR2,    	/*  RTC stopwatch alarm register 2 */
    PICR,	  	/*  RTC periodic interrupt counter register */
    PIAR,	  	/*  RTC periodic interrupt alarm register */

/*
//  I2C, PI2C
*/
    I2C,	  	/*  I2C register */
    PI2C,	  	/*  Power I2C register */

//  Keep this the last one in the enum list
    XLLP_PROTECTED_REGISTER_COUNT
}XLLP_PROTECTED_REGISTER;

XLLP_UINT32_T XllpLock(XLLP_PROTECTED_REGISTER Xllp_RegisterID);

void XllpUnlock(XLLP_PROTECTED_REGISTER Xllp_RegisterID);
#endif
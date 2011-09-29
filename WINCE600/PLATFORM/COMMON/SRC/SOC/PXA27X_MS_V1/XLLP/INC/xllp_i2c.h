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
**  FILENAME:       xllp_i2c.h
**  
**  PURPOSE: contains all I2C specific macros, typedefs, and prototypes.
**           Declares no storage.
**  
**  
******************************************************************************/
#include "xllp_intc.h"
#include "xllp_clkmgr.h"
#include "xllp_gpio.h"
#include "xllp_defs.h"
#include "xllp_ost.h"

#ifndef __XLLP_I2C_H
#define __XLLP_I2C_H


/**************************
 * I2C Bus Interface Unit *
 **************************/
// note offset at 0x4030_16xx for standard i2c

typedef struct 
{
    XLLP_VUINT32_T IBMR;         /* Bus monitor register */
	XLLP_UINT32_T RESERVED1;	/* addr. offset 0x84-0x88 */
	XLLP_VUINT32_T IDBR;			/* Data buffer Register */
  	XLLP_UINT32_T RESERVED2;	/* addr. offset 0x8C-0x90*/
  	XLLP_VUINT32_T ICR;			/* Global Control Register */
  	XLLP_UINT32_T RESERVED3;	/* addr. offset 0x94-0x98 */
  	XLLP_VUINT32_T ISR;			/* Status Register*/
    XLLP_UINT32_T RESERVED4;	/* addr. offset 0x9C-0xA0 */
  	XLLP_VUINT32_T ISAR;			/* Slave address register */

} XLLP_I2C_T, *P_XLLP_I2C_T;

// note offset at 0x40F0_01xx for power i2c
typedef struct 
{
  XLLP_VUINT32_T 	PMCR;      // Pwr Mgr Control Reg
  XLLP_VUINT32_T 	PSSR;      // Pwr Mgr Sleep Status Reg
  XLLP_VUINT32_T 	PSPR;      // Pwr Mgr Scratch Pad Reg 
  XLLP_VUINT32_T 	PWER;      // Pwr Mgr Wake-Up Enable Reg
  XLLP_VUINT32_T 	PRER;      // Pwr Mgr Rising-Edge Detect Enable Reg
  XLLP_VUINT32_T 	PFER;      // Pwr Mgr Falling-Edge Detect Enable Reg
  XLLP_VUINT32_T 	PEDR;      // Pwr Mgr Edge-Detect Status Reg 
  XLLP_VUINT32_T 	PCFR;      // Pwr Mgr General Configuration Reg
                // Power Manager GPIO Sleep State Registers:
  XLLP_VUINT32_T 	PGSR0;     // GPIO<31:0>
  XLLP_VUINT32_T 	PGSR1;     // GPIO<63:32>
  XLLP_VUINT32_T 	PGSR2;     // GPIO<95:64>
  XLLP_VUINT32_T 	PGSR3;     // GPIO<118:96>
  XLLP_VUINT32_T 	RCSR;      // Reset Controller Status Reg
  XLLP_VUINT32_T 	PSLR;      // Pwr Mgr Sleep Configuration Reg
  XLLP_VUINT32_T 	PSTR;      // Pwr Mgr Standby Configuration Reg
  XLLP_VUINT32_T 	PSNR;      // Pwr Mgr Sense Configuration Reg
  XLLP_VUINT32_T 	PVCR;      // Pwr Mgr Voltage Change Control Reg
  XLLP_VUINT32_T    rsvd1[2];
               // Next register (PUCR) controlled by USIM driver
  XLLP_VUINT32_T    PUCR;      // Pwr Mgr USIM Card Control Reg
  XLLP_VUINT32_T    PKWR;      // Pwr Mgr Keyboard Wake-Up Enable Reg
  XLLP_VUINT32_T    PKSR;      // Pwr Mgr Keyboard Level-Detect Status Reg
  XLLP_VUINT32_T    rsvd2[10];
  XLLP_VUINT32_T    PCMDn[32];   // Pwr Mgr I2C Command Reg File
  XLLP_VUINT32_T    rsvd3[32];

  XLLP_VUINT32_T	IBMR;         /* Bus monitor register */
  XLLP_UINT32_T		RESERVED1;	/* addr. offset 0x84-0x88 */
  XLLP_VUINT32_T	IDBR;			/* Data buffer Register */
  XLLP_UINT32_T		RESERVED2;	/* addr. offset 0x8C-0x90*/
  XLLP_VUINT32_T	ICR;			/* Global Control Register */
  XLLP_UINT32_T		RESERVED3;	/* addr. offset 0x94-0x98 */
  XLLP_VUINT32_T	ISR;			/* Status Register*/
  XLLP_UINT32_T		RESERVED4;	/* addr. offset 0x9C-0xA0 */
  XLLP_VUINT32_T	ISAR;			/* Slave address register */

} XLLP_PI2C_T, *P_XLLP_PI2C_T;


/*  bus monitor register */
#define	XLLP_IBMR_SDA		XLLP_BIT_0  /* reflects the status of SDA pin */
#define	XLLP_IBMR_SCL		XLLP_BIT_1 /* reflects the status of SCL pin */

/* data buffer regiter mask */
#define	XLLP_IDBR_ADDR		0xFF;  /*buffer for I2C bus send/receive data */
#define	XLLP_IDBR_MODE		XLLP_BIT_0	
/* Control Register */
#define	XLLP_ICR_START		XLLP_BIT_0 /* 1:send a Start condition to the I2C when in master mode */
#define	XLLP_ICR_STOP		XLLP_BIT_1 /* 1:send a Stop condition after next data byte transferred on I2C bus in master mode */
#define	XLLP_ICR_ACKNACK	XLLP_BIT_2  /* Ack/Nack control: 1:Nack, 0:Ack (negative or positive pulse) */
#define	XLLP_ICR_TB			XLLP_BIT_3  /* 1:send/receive byte, 0:cleared by I2C unit when done */
#define	XLLP_ICR_MA			XLLP_BIT_4  /* 1:I2C sends STOP w/out data permission, 0:ICR bit used only */
#define	XLLP_ICR_SCLEA		XLLP_BIT_5  /* I2C clock output: 1:Enabled, 0:Disabled. ICCR configured before ! */
#define	XLLP_ICR_UIE		XLLP_BIT_6 /* I2C unit: 1:Enabled, 0:Disabled */
#define	XLLP_ICR_GCD		XLLP_BIT_7  /* General Call: 1:Disabled, 0:Enabled */
#define	XLLP_ICR_ITEIE		XLLP_BIT_8  /* 1: IDBR Transmit Empty Interrupt Enable */
#define	XLLP_ICR_DRFIE		XLLP_BIT_9  /* 1: IDBR Receive Full Interrupt Enable */
#define	XLLP_ICR_BEIE		XLLP_BIT_10  /* 1: Bus Error Interrupt Enable */
#define	XLLP_ICR_SSDIE		XLLP_BIT_11 /* 1: Slave Stop Detected Interrupt Enable */
#define	XLLP_ICR_ALDIE		XLLP_BIT_12  /* 1: Arbitration Loss Detected Interrupt Enable */
#define	XLLP_ICR_SADIE		XLLP_BIT_13  /* 1: Slave Address Detected Interrupt Enable */
#define	XLLP_ICR_UR			XLLP_BIT_14  /* 1: I2C unit reset */
#define XLLP_ICR_FM   		XLLP_BIT_15 /* 1: Fast mode - 400 KBit/sec. operation. Default is 100 KBit/sec */

/* Status Register */
#define	XLLP_ISR_RWM		XLLP_BIT_0  /* 1: I2C in master receive = slave transmit mode */
#define	XLLP_ISR_ACKNACK	XLLP_BIT_1  /* 1: I2C received/sent a Nack, 0: Ack */
#define	XLLP_ISR_UB			XLLP_BIT_2  /* 1: Processor's I2C unit busy */
#define	XLLP_ISR_IBB		XLLP_BIT_3  /* 1: I2C bus busy. Processor's I2C unit not involved */
#define	XLLP_ISR_SSD		XLLP_BIT_4  /* 1: Slave Stop detected (when in slave mode: receive or transmit) */
#define	XLLP_ISR_ALD		XLLP_BIT_5  /* 1: Arbitration Loss Detected */
#define	XLLP_ISR_ITE		XLLP_BIT_6  /* 1: Transfer finished on I2C bus. If enabled in ICR, interrupt signaled */
#define	XLLP_ISR_IRF		XLLP_BIT_7  /* 1: IDBR received new byte from I2C bus. If ICR, interrupt signaled */
#define	XLLP_ISR_GCAD		XLLP_BIT_8  /* 1: I2C unit received a General Call address */
#define	XLLP_ISR_SAD		XLLP_BIT_9  /* 1: I2C unit detected a 7-bit address matching the general call or ISAR */
#define	XLLP_ISR_BED		XLLP_BIT_10  /* Bit set by unit when a Bus Error detected */

/*  slave address mask */
#define	XLLP_ISAR_ADDR		0x7F;	/*  7-bit(6:0) add to which I2C unit responses to in slave/receive mode*/

#define XLLP_ICR_INIT_VALUE (XLLP_ICR_UIE|XLLP_ICR_SCLEA)

#define XLLP_I2C_NO_STOP     0    /* Don't issue stop bit */
#define XLLP_I2C_STOP        1    /* Issue stop bit */

/* Processor I2C Device ID */
#define XLLP_I2C_SLAVE_ID 0x4E  /* 0100_111x */ /* The Phillips spec says it must be a value between 0001_000xB and 1110_111xB */

#ifdef __cplusplus
extern "C" {
#endif
XLLP_BOOL_T XllpI2cInit(P_XLLP_I2C_T I2C_regs, P_XLLP_GPIO_T gpio, P_XLLP_CLKMGR_T clkMgr, XLLP_UINT32_T dev_id);
XLLP_BOOL_T XllpPI2cInit(P_XLLP_PI2C_T I2C_regs, P_XLLP_GPIO_T gpio, P_XLLP_CLKMGR_T clkMgr, XLLP_UINT32_T dev_id);
XLLP_BOOL_T XllpI2cRxFull(P_XLLP_I2C_T i2c_regs, XLLP_OST_T *pOSTRegs, XLLP_INT32_T timeout);
XLLP_BOOL_T XllpI2cTxEmpty(P_XLLP_I2C_T i2c_regs, XLLP_OST_T *pOSTRegs, XLLP_INT32_T timeout);
XLLP_BOOL_T XllpI2CWrite(P_XLLP_I2C_T I2C_regs, XLLP_OST_T *pOSTRegs, XLLP_UINT8_T slaveAddr, const XLLP_UINT8_T * bytesBuf, XLLP_UINT32_T bytesCount, XLLP_BOOL_T bSendStop);
XLLP_BOOL_T XllpI2CRead(P_XLLP_I2C_T I2C_regs, XLLP_OST_T *pOSTRegs, XLLP_UINT8_T slaveAddr, XLLP_UINT8_T * bytesBuf, XLLP_UINT32_T bytesCount, XLLP_BOOL_T bSendStop);
#ifdef __cplusplus
};
#endif

#endif /* XLLP_I2C_H */

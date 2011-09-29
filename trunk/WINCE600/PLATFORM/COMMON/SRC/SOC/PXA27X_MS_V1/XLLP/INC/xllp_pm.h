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
#ifndef __XLLP_PM_H__
#define __XLLP_PM_H__
/******************************************************************************
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
**
**  FILENAME:   xllp_Pm.h
**
**  PURPOSE:    Contains definitions for the Power Management related registers
**                  in the APPS
**              This file is for the B0 stepping of the processor code-named 
**                  Bulverde.
**
******************************************************************************/


//
// Power Manager Register definitions
//   - Not including Power Manager I2C registers.
//
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

} XLLP_PWRMGR_T, *P_XLLP_PWRMGR_T;


/////////////////////////////////////////////////
// Power Manager register field masks and definitions

// Reserved and valid masks

#define XLLP_PM_PMCR_RESERVED_BITS 0xFFFFFFC0u
#define XLLP_PM_PMCR_VLD_MSK (~(XLLP_PM_PMCR_RESERVED_BITS))

#define XLLP_PM_PSSR_RESERVED_BITS 0xFFFFFF80u
#define XLLP_PM_PSSR_VLD_MSK (~(XLLP_PM_PSSR_RESERVED_BITS))

#define XLLP_PM_PSPR_RESERVED_BITS 0x00000000u
#define XLLP_PM_PSPR_VLD_MSK (~(XLLP_PM_PSPR_RESERVED_BITS))

#define XLLP_PM_PWER_RESERVED_BITS 0x206001E4u
#define XLLP_PM_PWER_VLD_MSK (~(XLLP_PM_PWER_RESERVED_BITS))

#define XLLP_PM_PRER_RESERVED_BITS 0xFEFF01E4u
#define XLLP_PM_PRER_VLD_MSK (~(XLLP_PM_PRER_RESERVED_BITS))

#define XLLP_PM_PFER_RESERVED_BITS 0xFEFF01E4u
#define XLLP_PM_PFER_VLD_MSK (~(XLLP_PM_PFER_RESERVED_BITS))

#define XLLP_PM_PEDR_RESERVED_BITS 0x20ED01E4u
#define XLLP_PM_PEDR_VLD_MSK (~(XLLP_PM_PEDR_RESERVED_BITS))

#define XLLP_PM_PCFR_RESERVED_BITS 0xFFFF2328u
#define XLLP_PM_PCFR_VLD_MSK (~(XLLP_PM_PCFR_RESERVED_BITS))

#define XLLP_PM_PGSR0_RESERVED_BITS 0x000001E4u
#define XLLP_PM_PGSR0_VLD_MSK (~(XLLP_PM_PGSR0_RESERVED_BITS))

#define XLLP_PM_PGSR1_RESERVED_BITS 0x00000000u
#define XLLP_PM_PGSR1_VLD_MSK (~(XLLP_PM_PGSR1_RESERVED_BITS))

#define XLLP_PM_PGSR2_RESERVED_BITS 0x00000000u
#define XLLP_PM_PGSR2_VLD_MSK (~(XLLP_PM_PGSR2_RESERVED_BITS))

#define XLLP_PM_PGSR3_RESERVED_BITS 0xFF800000u
#define XLLP_PM_PGSR3_VLD_MSK (~(XLLP_PM_PGSR3_RESERVED_BITS))

#define XLLP_PM_RCSR_RESERVED_BITS 0x0000000Fu
#define XLLP_PM_RCSR_VLD_MSK (~(XLLP_PM_RCSR_RESERVED_BITS))

#define XLLP_PM_PSLR_RESERVED_BITS 0x002FF0F3u
#define XLLP_PM_PSLR_VLD_MSK (~(XLLP_PM_PSLR_RESERVED_BITS))

#define XLLP_PM_PSTR_RESERVED_BITS 0xFFFFF0F3u
#define XLLP_PM_PSTR_VLD_MSK (~(XLLP_PM_PSTR_RESERVED_BITS))

#define XLLP_PM_PSNR_RESERVED_BITS 0xFFFFF0FFu
#define XLLP_PM_PSNR_VLD_MSK (~(XLLP_PM_PSNR_RESERVED_BITS))

#define XLLP_PM_PVCR_RESERVED_BITS 0xFE0FB000u
#define XLLP_PM_PVCR_VLD_MSK (~(XLLP_PM_PVCR_RESERVED_BITS))

#define XLLP_PM_PUCR_RESERVED_BITS 0xFFFFFFD2u
#define XLLP_PM_PUCR_VLD_MSK (~(XLLP_PM_PUCR_RESERVED_BITS))

#define XLLP_PM_PKWR_RESERVED_BITS 0xFFF00000u
#define XLLP_PM_PKWR_VLD_MSK (~(XLLP_PM_PKWR_RESERVED_BITS))

#define XLLP_PM_PKSR_RESERVED_BITS 0xFFF00000u
#define XLLP_PM_PKSR_VLD_MSK (~(XLLP_PM_PKSR_RESERVED_BITS))


//  PSLR: Sleep Configuration register
// pwr_i2c control

#define   XLLP_PSLR_SL_PI_MSK       0x0000000CUL // Also a reserved value, for now
#define   XLLP_PSLR_SL_PI_OFF       0x00000000UL
#define   XLLP_PSLR_SL_PI_RUN       0x00000008UL // Run during sleep. Not for Deep S.
#define   XLLP_PSLR_SL_PI_RETAIN    0x00000004UL // Retain state during Sleeps

#endif  // __XLLP_PM_H__

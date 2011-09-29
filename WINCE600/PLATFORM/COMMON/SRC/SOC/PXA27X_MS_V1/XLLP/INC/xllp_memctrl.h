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
**  FILENAME:       xllp_memctrl.h
**
**  PURPOSE:        This header file contains the definitions for the Intel(r)
**                  XScale(tm) Microarchitecture memory controller.
**
**
******************************************************************************/
#ifndef _xllp_memctrl_h
#define _xllp_memctrl_h

// Bulverde Memory Controller Register Definitions.
typedef struct MEMORY_CONTROL_REGISTER_S {
    XLLP_VUINT32_T MDCNFG;       // SDRAM Configuration Register 0
    XLLP_VUINT32_T MDREFR;       // SDRAM Refresh Control Register
    XLLP_VUINT32_T MSC0;         // Static Memory Control Register 0
    XLLP_VUINT32_T MSC1;         // Static Memory Control Register 1
    XLLP_VUINT32_T MSC2;         // Static Memory Control Register 2
    XLLP_VUINT32_T MECR;         // Expansion Memory Bus Configuration Register
    XLLP_VUINT32_T RESERVED1;    // Reserved.
    XLLP_VUINT32_T SXCNFG;       // Synchronous Static Memory Control Register
    XLLP_VUINT32_T RESERVED2[2]; // Reserved.
    XLLP_VUINT32_T MCMEM0;       // Card Interface Common Memory Space Socket 0
    XLLP_VUINT32_T MCMEM1;       // Card Interface Common Memory Space Socket 1
    XLLP_VUINT32_T MCATT0;       // Card Interface Attribute Space Socket 0
    XLLP_VUINT32_T MCATT1;       // Card Interface Attribute Space Socket 1
    XLLP_VUINT32_T MCIO0;        // Card Interface I/O Space Socket 0
    XLLP_VUINT32_T MCIO1;        // Card Interface I/O Space Socket 1
    XLLP_VUINT32_T MDMRS;        // MRS value to be written to SDRAM
    XLLP_VUINT32_T BOOT_DEF;     // Read-Only Boot Time Register
    XLLP_VUINT32_T ARB_CNTTL;    // Arbiter Control Register
    XLLP_VUINT32_T BSCNTR0;      // System Memory Buffer Strength Control
                                 // Register 0
    XLLP_VUINT32_T BSCNTR1;      // System Memory Buffer Strength Control
                                 //  Register 1
    XLLP_VUINT32_T LCDBSCNTR;    // LCD Buffer Strength Control Register
    XLLP_VUINT32_T MDMRSLP;      // Special Low Power SDRAM Mode Register Set
                                 // Configuration Register 
    XLLP_VUINT32_T BSCNTR2;      // System Memory Buffer Strength Control
                                 // Register 2
    XLLP_VUINT32_T BSCNTR3;      // System Memory Buffer Strength Control
                                 // Register 3
    XLLP_VUINT32_T SA1110;       // SA1110 Compatibility Mode for Static Memory
} XLLP_MEMORY_CONTROL_REGISTER_T;

/*
*******************************************************************************
*    Expansion Memory (PCMCIA and Compact Flash) Configuration Register bits.
*    Refer to the Bulverde EAS for details.
*******************************************************************************
*/
#define XLLP_MECR_NOS    (0x1U << 0)    // Card Interface Number of Sockets
#define XLLP_MECR_CIT    (0x1U << 1)    // Card Interface Card Is There

/*
*******************************************************************************
*    MCMEM0 and MCMEM1 Register Bitmap settings.
*    Refer to the Bulverde EAS for details.
*******************************************************************************
*/
#define XLLP_MCMEM0_SET  (0x007U << 0)
#define XLLP_MCMEM0_ASST (0x06U << 7)
#define XLLP_MCMEM0_HOLD (0x05U << 14)

#define XLLP_MCMEM1_SET  (0x007U << 0)
#define XLLP_MCMEM1_ASST (0x06U << 7)
#define XLLP_MCMEM1_HOLD (0x05U << 14)

/*
*******************************************************************************
*    MCATT0 and MCATT1 Register Bitmap settings.
*    Refer to the Bulverde EAS for details.
*******************************************************************************
*/
#define XLLP_MCATT0_SET  (0x07U << 0)
#define XLLP_MCATT0_ASST (0x0FU << 7)
#define XLLP_MCATT0_HOLD (0x07U << 14)

#define XLLP_MCATT1_SET  (0x07U << 0)
#define XLLP_MCATT1_ASST (0x0FU << 7)
#define XLLP_MCATT1_HOLD (0x07U << 14)

/*
*******************************************************************************
*    MCIO0 and MCIO1 Register Bitmap settings.
*    Refer to the Bulverde EAS for details.
*    Assume Memory Clock is 100 Mhz = 10 ns cycle
*******************************************************************************
*/
#define XLLP_MCIO0_SET  (0x0FU << 0)
#define XLLP_MCIO0_ASST (0x06U << 7)
#define XLLP_MCIO0_HOLD (0x05U << 14)

#define XLLP_MCIO1_SET  (0x0FU << 0)
#define XLLP_MCIO1_ASST (0x06U << 7)
#define XLLP_MCIO1_HOLD (0x05U << 14)

/*
*******************************************************************************
*    BOOT_DEF Bit Definitions.
*    Refer to the Bulverde EAS for details.
*******************************************************************************
*/
#define XLLP_BOOT_SEL    (0x1U << 0)    // Boot Select bit
#define XLLP_PKG_TYPE    (0x1U << 3)    // Package Type

#endif // _xllp_memctrl_h

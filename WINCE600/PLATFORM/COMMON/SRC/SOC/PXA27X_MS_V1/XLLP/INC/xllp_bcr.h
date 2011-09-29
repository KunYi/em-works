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
**  FILENAME:       xllp_bcr.h
**
**  PURPOSE: Contains all Board Control Register (aka Board Level Register)
**              specific macros, typedefs, and prototypes.
**           Includes System Configuration Register (not memory mapped) constant 
**              definitions  
**           Declares no storage.
**                  
**
******************************************************************************/

#ifndef __XLLP_BCR_H__
#define __XLLP_BCR_H__

#include "xllp_defs.h"



/**
  BCR Register Definitions
**/

typedef struct
{
    XLLP_VUINT32_T RESV1[4];            /* 0x00 */
    XLLP_VUINT32_T HLDR1;               /* 0x10 Hex LED Data Register 1  (LEDDATA1) */
    XLLP_VUINT32_T HLDR2;               /* 0x14 Hex LED Data Register 2 (LEDDATA2)*/
    XLLP_VUINT32_T RESV2[10];           /* 0x18-0x3F */
    XLLP_VUINT32_T LCR;                 /* 0x40 Led Control register */
    XLLP_VUINT32_T RESV3[7];            /* 0x44-5F*/
    XLLP_VUINT32_T GPSR;                /* 0x60 General Purpose Switch register */
    XLLP_VUINT32_T RESV4[7];            /* 0x64-7F*/
    XLLP_VUINT32_T MISCWR1;             /* 0x80 Miscellaneous Write Register 1 */
    XLLP_VUINT32_T MISCWR2;             /* 0x84 Miscellaneous Write Register 2 */
    XLLP_VUINT32_T MISCWR3;             /* 0x88 Miscellaneous Write Register 3 */
    XLLP_VUINT32_T RESV5[1];            /* 0x8C-8F */
    XLLP_VUINT32_T MISCRR1;             /* 0x90 Miscellaneous Read Register */
    XLLP_VUINT32_T RESV6[11];           /* 0x94-BF*/
    XLLP_VUINT32_T PIMER1;              /* 0xC0 Platform Interrupt Mask/Enable Register 1 */
    XLLP_VUINT32_T RESV7[3];            /* 0xC4-CF*/
    XLLP_VUINT32_T PSCR1;               /* 0xD0 Platform Interrupt Set/Clear Register 1 */
    XLLP_VUINT32_T RESV8[3];            /* 0xD4-DF*/
    XLLP_VUINT32_T PCMCIAS0SCR;         /* 0xE0 PCMCIA  Socket 0 Status / Control register */
    XLLP_VUINT32_T PCMCIAS1SCR;         /* 0xE4 PCMCIA  Socket 1 Status / Control register */
    XLLP_VUINT32_T RESV9[2];            /* 0xE8-EF*/
    XLLP_VUINT32_T REVID;               /* 0xF0 FPGA code revision ID (16 LSB: X.YZ) */
    XLLP_VUINT32_T SCRATCH[3];          /* 0xF4-FF (For debug) Maintained through sleep and deep sleep */
   
} XLLP_BCR_T, *P_XLLP_BCR_T;


/* Hex LED Digit0 to Digit7*/

#define XLLP_BCR_HEXLED_DIGIT0  0xfu        /* Least significant Bit*/
#define XLLP_BCR_HEXLED_DIGIT1  0xf0u
#define XLLP_BCR_HEXLED_DIGIT2  0xf00u
#define XLLP_BCR_HEXLED_DIGIT3  0xf000u
#define XLLP_BCR_HEXLED_DIGIT4  0xf0000u
#define XLLP_BCR_HEXLED_DIGIT5  0xf00000u
#define XLLP_BCR_HEXLED_DIGIT6  0xf000000u
#define XLLP_BCR_HEXLED_DIGIT7  0xf0000000u  /*Most significant Bit*/

#define XLLP_BCR_HEXLED1_RESERVED_BITS 0x00000000u
#define XLLP_BCR_HEXLED1_MASK (~(XLLP_BCR_HEXLED1_RESERVED_BITS))

/* Hex LED DOTS and decimal Points */

#define XLLP_BCR_HEXLED_HEX0L1      (XLLP_BIT_0)            
#define XLLP_BCR_HEXLED_HEX0L2      (XLLP_BIT_1)
#define XLLP_BCR_HEXLED_HEX0L3      (XLLP_BIT_2)
#define XLLP_BCR_HEXLED_HEX1L1      (XLLP_BIT_3)
#define XLLP_BCR_HEXLED_HEX1L2      (XLLP_BIT_4)
#define XLLP_BCR_HEXLED_HEX1L3      (XLLP_BIT_5)
#define XLLP_BCR_HEXLED_DIGIT0DP    (XLLP_BIT_8)
#define XLLP_BCR_HEXLED_DIGIT1DP    (XLLP_BIT_9)   
#define XLLP_BCR_HEXLED_DIGIT2DP    (XLLP_BIT_10)
#define XLLP_BCR_HEXLED_DIGIT3DP    (XLLP_BIT_11)
#define XLLP_BCR_HEXLED_DIGIT4DP    (XLLP_BIT_12)   
#define XLLP_BCR_HEXLED_DIGIT5DP    (XLLP_BIT_13)
#define XLLP_BCR_HEXLED_DIGIT6DP    (XLLP_BIT_14)   
#define XLLP_BCR_HEXLED_DIGIT7DP    (XLLP_BIT_15)

/* BCR Reserved Bit Fields */
#define XLLP_BCR_HEXLED_RESERVED_BITS  (XLLP_BIT_6 | XLLP_BIT_7)
#define XLLP_BCR_HEXLED2_RESERVED_BITS ((0xffff0000u)|XLLP_BCR_HEXLED_RESERVED_BITS)
#define XLLP_BCR_HEXLED2_MASK (~(XLLP_BCR_HEXLED2_RESERVED_BITS))

/* Hex Led Control Register*/
#define XLLP_BCR_DISCLED0   (XLLP_BIT_0)
#define XLLP_BCR_DISCLED1   (XLLP_BIT_1)
#define XLLP_BCR_DISCLED2   (XLLP_BIT_2)
#define XLLP_BCR_DISCLED3   (XLLP_BIT_3)
#define XLLP_BCR_DISCLED4   (XLLP_BIT_4)
#define XLLP_BCR_DISCLED5   (XLLP_BIT_5)
#define XLLP_BCR_DISCLED6   (XLLP_BIT_6)
#define XLLP_BCR_DISCLED7   (XLLP_BIT_7)

#define XLLP_BCR_HEXLED_BLANK0  (XLLP_BIT_8)
#define XLLP_BCR_HEXLED_BLANK1  (XLLP_BIT_9)
#define XLLP_BCR_HEXLED_BLANK2  (XLLP_BIT_10)
#define XLLP_BCR_HEXLED_BLANK3  (XLLP_BIT_11)
#define XLLP_BCR_HEXLED_BLANK4  (XLLP_BIT_12)
#define XLLP_BCR_HEXLED_BLANK5  (XLLP_BIT_13)
#define XLLP_BCR_HEXLED_BLANK6  (XLLP_BIT_14)
#define XLLP_BCR_HEXLED_BLANK7  (XLLP_BIT_15)

#define XLLP_BCR_LEDCTRL_RESERVED_BITS 0xFFFF0000u
#define XLLP_BCR_LEDCTRL_MASK (~(XLLP_BCR_LEDCTRL_RESERVED_BITS))

/* General Purpose Switch register */
#define XLLP_BCR_GPSR_HEXSWT0   (0xfu)
#define XLLP_BCR_GPSR_HEXSWT1   (0xfu<<4)
#define XLLP_BCR_GPSR_GPSWT0    (XLLP_BIT_8)
#define XLLP_BCR_GPSR_GPSWT1    (XLLP_BIT_9)
#define XLLP_BCR_GPSR_GPSWT2    (XLLP_BIT_10)
#define XLLP_BCR_GPSR_GPSWT3    (XLLP_BIT_11)
#define XLLP_BCR_GPSR_GPSWT4    (XLLP_BIT_12)
#define XLLP_BCR_GPSR_GPSWT5    (XLLP_BIT_13)
#define XLLP_BCR_GPSR_GPSWT6    (XLLP_BIT_14)
#define XLLP_BCR_GPSR_GPSWT7    (XLLP_BIT_15)

#define XLLP_BCR_GPSR_RESERVED_BITS 0xFFFF0000u
#define XLLP_BCR_GPSR_MASK (~(XLLP_BCR_GPSR_RESERVED_BITS))

/* XLLP Miscellaneous Write Register 1 */
#define XLLP_BCR_MISCWR1_SYSRESET   (XLLP_BIT_0)
#define XLLP_BCR_MISCWR1_MTR_ON     (XLLP_BIT_1)
#define XLLP_BCR_MISCWR1_PDC_CTL    (XLLP_BIT_2)
#define XLLP_BCR_MISCWR1_GREENLED   (XLLP_BIT_3)
#define XLLP_BCR_MISCWR1_IRDA_FIR   (XLLP_BIT_4)
#define XLLP_BCR_MISCWR1_IRDA_MD    (XLLP_BIT_5 | XLLP_BIT_6)
#define XLLP_BCR_MISCWR1_BTDTR      (XLLP_BIT_7)
#define XLLP_BCR_MISCWR1_nBT_OFF    (XLLP_BIT_8)
#define XLLP_BCR_MISCWR1_BB_SEL     (XLLP_BIT_9)
#define XLLP_BCR_MISCWR1_MS_SEL     (XLLP_BIT_10)
#define XLLP_BCR_MISCWR1_MMC_ON     (XLLP_BIT_11)
#define XLLP_BCR_MISCWR1_MS_ON      (XLLP_BIT_12)
#define XLLP_BCR_MISCWR1_LCD_CTL    (XLLP_BIT_13)
#define XLLP_BCR_MISCWR1_CAMERA_SEL (XLLP_BIT_14)
#define XLLP_BCR_MISCWR1_CAMERA_ON  (XLLP_BIT_15)

#define XLLP_BCR_MISCWR1_RESERVED_BITS 0xFFFF0000u
#define XLLP_BCR_MISCWR1_MASK (~(XLLP_BCR_MISCWR1_RESERVED_BITS))

/* XLLP Miscellaneous Write Register 2 */
#define XLLP_BCR_MISCWR2_RADIO_WAKE     (XLLP_BIT_0)
#define XLLP_BCR_MISCWR2_RADIO_PWR      (XLLP_BIT_1)
#define XLLP_BCR_MISCWR2_LINE1_SPKROFF  (XLLP_BIT_2)
#define XLLP_BCR_MISCWR2_LINE2_SPKROFF  (XLLP_BIT_3)
#define XLLP_BCR_MISCWR2_NUSBC_SC       (XLLP_BIT_4)
#define XLLP_BCR_MISCWR2_USB_OTG_RST    (XLLP_BIT_5)
#define XLLP_BCR_MISCWR2_USB_OTG_SEL    (XLLP_BIT_6)
#define XLLP_BCR_MISCWR2_GRAPHICS_SEL   (XLLP_BIT_7)
#define XLLP_BCR_MISCWR2_nLEGACY_SEL    (XLLP_BIT_8)

#define XLLP_BCR_MISCWR2_RESERVED_BITS 0xFFFFFE00u
#define XLLP_BCR_MISCWR2_MASK (~(XLLP_BCR_MISCWR2_RESERVED_BITS))


/* XLLP Miscellaneous Write Register 3 */

#define XLLP_BCR_MISCWR3_COMMS_SW_RESET (XLLP_BIT_0)
#define XLLP_BCR_MISCWR3_GPIO_RESET     (XLLP_BIT_1)
#define XLLP_BCR_MISCWR3_GPIO_RESET_EN  (XLLP_BIT_2)

#define XLLP_BCR_MISCWR3_RESERVED_BITS 0xFFFFFFF8u
#define XLLP_BCR_MISCWR3_MASK (~(XLLP_BCR_MISCWR3_RESERVED_BITS))


/* XLLP Miscellaneous Read Register 1 */

#define XLLP_BCR_MISCRR1_MMC_WP     (XLLP_BIT_0)
#define XLLP_BCR_MISCRR1_BTDCD      (XLLP_BIT_1)
#define XLLP_BCR_MISCRR1_BTRI       (XLLP_BIT_2)
#define XLLP_BCR_MISCRR1_BTDSR      (XLLP_BIT_3)
#define XLLP_BCR_MISCRR1_TS_BUSY    (XLLP_BIT_4)
#define XLLP_BCR_MISCRR1_USB_CBL    (XLLP_BIT_5)
#define XLLP_BCR_MISCRR1_nUSIM_CD   (XLLP_BIT_6)
#define XLLP_BCR_MISCRR1_nMMC_CD    (XLLP_BIT_7)
#define XLLP_BCR_MISCRR1_nMEMSTK_CD (XLLP_BIT_8)
#define XLLP_BCR_MISCRR1_nPENIRQ    (XLLP_BIT_9)
#define XLLP_BCR_MISCRR1_POLL_FLAG  (XLLP_BIT_10)

#define XLLP_BCR_MISCRR1_RESERVED_BITS 0xFFFFFC00u
#define XLLP_BCR_MISCRR1_MASK (~(XLLP_BCR_MISCRR1_RESERVED_BITS))

/* Platform Interrupt Mask/Enable Register 1 */

#define XLLP_BCR_INTMASK_ENABLE_R1_MMC          (XLLP_BIT_0)
#define XLLP_BCR_INTMASK_ENABLE_R1_USIM         (XLLP_BIT_1)
#define XLLP_BCR_INTMASK_ENABLE_R1_USBC         (XLLP_BIT_2)
#define XLLP_BCR_INTMASK_ENABLE_R1_ETHERNET     (XLLP_BIT_3)
#define XLLP_BCR_INTMASK_ENABLE_R1_AC97         (XLLP_BIT_4)
#define XLLP_BCR_INTMASK_ENABLE_R1_PENIRQ       (XLLP_BIT_5)
#define XLLP_BCR_INTMASK_ENABLE_R1_MSINS        (XLLP_BIT_6)
#define XLLP_BCR_INTMASK_ENABLE_R1_nEXBRD_INT   (XLLP_BIT_7)
#define XLLP_BCR_INTMASK_ENABLE_R1_S0_CD        (XLLP_BIT_9)
#define XLLP_BCR_INTMASK_ENABLE_R1_S0_STSCHG    (XLLP_BIT_10)
#define XLLP_BCR_INTMASK_ENABLE_R1_S0_IRQ       (XLLP_BIT_11)
#define XLLP_BCR_INTMASK_ENABLE_R1_S1_CD        (XLLP_BIT_13)
#define XLLP_BCR_INTMASK_ENABLE_R1_S1_STSCHG    (XLLP_BIT_14)
#define XLLP_BCR_INTMASK_ENABLE_R1_S1_IRQ       (XLLP_BIT_15)
#define XLLP_BCR_INTMASK_ENABLE_R1_PMC_IRQ      (XLLP_BIT_16)
#define XLLP_BCR_INTMASK_ENABLE_R1_BT_DTR_INT   (XLLP_BIT_17)
#define XLLP_BCR_INTMASK_ENABLE_R1_BT_RI_INT    (XLLP_BIT_18)
#define XLLP_BCR_INTMASK_ENABLE_R1_INT_Mx       (XLLP_BIT_19)

#define XLLP_BCR_INTMASK_ENABLE_R1_RESERVED_BITS 0xFFF01100u
#define XLLP_BCR_INTMASK_ENABLE_MASK (~(XLLP_BCR_INTMASK_ENABLE_R1_RESERVED_BITS))

/* Platform Interrupt Set/Clear Register 1 */

#define XLLP_BCR_INTSET_CLEAR_R1_MMC          (XLLP_BIT_0)
#define XLLP_BCR_INTSET_CLEAR_R1_USIM         (XLLP_BIT_1)
#define XLLP_BCR_INTSET_CLEAR_R1_USBC         (XLLP_BIT_2)
#define XLLP_BCR_INTSET_CLEAR_R1_ETHERNET     (XLLP_BIT_3)
#define XLLP_BCR_INTSET_CLEAR_R1_AC97         (XLLP_BIT_4)
#define XLLP_BCR_INTSET_CLEAR_R1_PENIRQ       (XLLP_BIT_5)
#define XLLP_BCR_INTSET_CLEAR_R1_MSINS        (XLLP_BIT_6)
#define XLLP_BCR_INTSET_CLEAR_R1_nEXBRD_INT   (XLLP_BIT_7)
#define XLLP_BCR_INTSET_CLEAR_R1_S0_CD        (XLLP_BIT_9)
#define XLLP_BCR_INTSET_CLEAR_R1_S0_STSCHG    (XLLP_BIT_10)
#define XLLP_BCR_INTSET_CLEAR_R1_S0_IRQ       (XLLP_BIT_11)
#define XLLP_BCR_INTSET_CLEAR_R1_S1_CD        (XLLP_BIT_13)
#define XLLP_BCR_INTSET_CLEAR_R1_S1_STSCHG    (XLLP_BIT_14)
#define XLLP_BCR_INTSET_CLEAR_R1_S1_IRQ       (XLLP_BIT_15)
#define XLLP_BCR_INTSET_CLEAR_R1_PMC_IRQ      (XLLP_BIT_16)
#define XLLP_BCR_INTSET_CLEAR_R1_BT_DTR_INT   (XLLP_BIT_17)
#define XLLP_BCR_INTSET_CLEAR_R1_BT_RI_INT    (XLLP_BIT_18)
#define XLLP_BCR_INTSET_CLEAR_R1_INT_Mx       (XLLP_BIT_19)

#define XLLP_BCR_INTSET_CLEAR_R1_RESERVED_BITS 0xFFF01100u
#define XLLP_BCR_INTSET_CLEAR_MASK (~(XLLP_BCR_INTSET_CLEAR_R1_RESERVED_BITS))

/* PCMCIA Socket0 Status/Control Register*/

#define XLLP_BCR_PCMCIA_SCR_S0_A0VPP        (XLLP_BIT_0)
#define XLLP_BCR_PCMCIA_SCR_S0_A1VPP        (XLLP_BIT_1)
#define XLLP_BCR_PCMCIA_SCR_S0_A0VCC        (XLLP_BIT_2)
#define XLLP_BCR_PCMCIA_SCR_S0_A1VCC        (XLLP_BIT_3)
#define XLLP_BCR_PCMCIA_SCR_S0_5V           (XLLP_BIT_0 | XLLP_BIT_1 | XLLP_BIT_2)
#define XLLP_BCR_PCMCIA_SCR_S0_3_3V         (XLLP_BIT_0 | XLLP_BIT_1 | XLLP_BIT_3)
#define XLLP_BCR_PCMCIA_SCR_S0_PWR          (XLLP_BIT_0 | XLLP_BIT_1 | XLLP_BIT_2 | XLLP_BIT_3)
#define XLLP_BCR_PCMCIA_SCR_S0_RESET        (XLLP_BIT_4)
#define XLLP_BCR_PCMCIA_SCR_S0_nCD          (XLLP_BIT_5)
#define XLLP_BCR_PCMCIA_SCR_S0_nVS1         (XLLP_BIT_6)
#define XLLP_BCR_PCMCIA_SCR_S0_nVS2         (XLLP_BIT_7)
#define XLLP_BCR_PCMCIA_SCR_S0_nVS          (XLLP_BIT_6 | XLLP_BIT_7)
#define XLLP_BCR_PCMCIA_SCR_S0_nSTSCHG_BVD1 (XLLP_BIT_8)
#define XLLP_BCR_PCMCIA_SCR_S0_nSPKR_BVD2   (XLLP_BIT_9)
#define XLLP_BCR_PCMCIA_SCR_S0_nIRQ         (XLLP_BIT_10)

#define XLLP_BCR_PCMCIA_SCR_S0_RESERVED_BITS    (0xF800u)
#define XLLP_BCR_PCMCIA_SCR_S0_MASK (~(XLLP_BCR_PCMCIA_SCR_S0_RESERVED_BITS))

/* PCMCIA Socket0 Status/Control Register*/

#define XLLP_BCR_PCMCIA_SCR_S1_A0VPP        (XLLP_BIT_0)
#define XLLP_BCR_PCMCIA_SCR_S1_A1VPP        (XLLP_BIT_1)
#define XLLP_BCR_PCMCIA_SCR_S1_A0VCC        (XLLP_BIT_2)
#define XLLP_BCR_PCMCIA_SCR_S1_A1VCC        (XLLP_BIT_3)
#define XLLP_BCR_PCMCIA_SCR_S1_5V           (XLLP_BIT_0 | XLLP_BIT_1 | XLLP_BIT_2)
#define XLLP_BCR_PCMCIA_SCR_S1_3_3V         (XLLP_BIT_0 | XLLP_BIT_1 | XLLP_BIT_3)
#define XLLP_BCR_PCMCIA_SCR_S1_PWR          (XLLP_BIT_0 | XLLP_BIT_1 | XLLP_BIT_2 | XLLP_BIT_3)
#define XLLP_BCR_PCMCIA_SCR_S1_RESET        (XLLP_BIT_4)
#define XLLP_BCR_PCMCIA_SCR_S1_nCD          (XLLP_BIT_5)
#define XLLP_BCR_PCMCIA_SCR_S1_nVS1         (XLLP_BIT_6)
#define XLLP_BCR_PCMCIA_SCR_S1_nVS2         (XLLP_BIT_7)
#define XLLP_BCR_PCMCIA_SCR_S1_nVS          (XLLP_BIT_6 | XLLP_BIT_7)
#define XLLP_BCR_PCMCIA_SCR_S1_nSTSCHG_BVD1 (XLLP_BIT_8)
#define XLLP_BCR_PCMCIA_SCR_S1_nSPKR_BVD2   (XLLP_BIT_9)
#define XLLP_BCR_PCMCIA_SCR_S1_nIRQ         (XLLP_BIT_10)

#define XLLP_BCR_PCMCIA_SCR_S1_RESERVED_BITS    (0xF800u)
#define XLLP_BCR_PCMCIA_SCR_S1_MASK (~(XLLP_BCR_PCMCIA_SCR_S1_RESERVED_BITS))

/* XLLP REVID (FPGA code revision ID) register */

#define XLLP_BCR_REVID_Z                (0x0Fu << 0)
#define XLLP_BCR_REVID_Y                (0x0Fu << 4)
#define XLLP_BCR_REVID_X                (0xFFu << 8)

#define XLLP_BCR_REVID_RESERVED_BITS    (0xFFFF0000u)
#define XLLP_BCR_REVID_MASK (~(XLLP_BCR_REVID_RESERVED_BITS))

/* XLLP Scratch (debug sleep storage) registers */

#define XLLP_BCR_SCRATCH_RESERVED_BITS  (0x00000000u)
#define XLLP_BCR_SCRATCH_MASK (~(XLLP_BCR_SCRATCH_RESERVED_BITS))

/*----------------------------------------------*/
/* XLLP SCR registers: not memory mapped        */

/* XLLP SCR  */

#define XLLP_BCR_SCR_DCID             (0x0Fu)
#define XLLP_BCR_SCR_DCREV            (XLLP_BIT_5 | XLLP_BIT_6 | XLLP_BIT_7)
#define XLLP_BCR_SCR_nBB_PRES         (XLLP_BIT_8)
#define XLLP_BCR_SCR_BBREV            (XLLP_BIT_9 | XLLP_BIT_10 | XLLP_BIT_11)
#define XLLP_BCR_SCR_nEXBID_PRES      (XLLP_BIT_12)
#define XLLP_BCR_SCR_EXBID            (XLLP_BIT_13 | XLLP_BIT_14)
#define XLLP_BCR_SCR_SWAP_FLASH       (XLLP_BIT_15)

#define XLLP_BCR_SCR_RESERVED_BITS    (0xFFFF0000u)
#define XLLP_BCR_SCR_MASK (~(XLLP_BCR_SCR_RESERVED_BITS))

/* XLLP SCR2  */

#define XLLP_BCR_SCR2_LCDID           (0x3Fu)
#define XLLP_BCR_SCR2_ORIENT          (XLLP_BIT_6)
#define XLLP_BCR_SCR2_AUDIOID         (XLLP_BIT_7 | XLLP_BIT_8 | XLLP_BIT_9)
#define XLLP_BCR_SCR2_nGFX_PRES       (XLLP_BIT_10)
#define XLLP_BCR_SCR2_nBB_PRES        (XLLP_BIT_11) // Questionable.  To be verified.

#define XLLP_BCR_SCR2_RESERVED_BITS    (0xFFFFFF00u)
#define XLLP_BCR_SCR2_MASK (~(XLLP_BCR2_SCR_RESERVED_BITS))


/* XLLP Board Control Register Primitive Functions */
/*
void XllpBcrWriteToHexLeds(P_XLLP_BCR_T,XLLP_UINT32_T);
void XllpBcrWriteDIGITxDPToHexLeds(P_XLLP_BCR_T,XLLP_UINT32_T);
void XllpBcrWriteDiscLeds(P_XLLP_BCR_T,XLLP_UINT32_T);
XLLP_UINT32_T XllpBcrReadRotSwt(P_XLLP_BCR);
XLLP_UINT32_T XllpBcrReadGpSwt(P_XLLP_BCR);
XLLP_UINT32_T XllpBcrReadMiscReadReg1(P_XLLP_BCR);
void XllpBcrModifyMiscWriteReg1(P_XLLP_BCR, XLLP_UINT32_T,XLLP_UINT32_T);
void XllpBcrModifyMiscWriteReg2(P_XLLP_BCR, XLLP_UINT32_T,XLLP_UINT32_T);
void XllpBcrPlatformInterruptMask_Enable(P_XLLP_BCR, XLLP_UINT32_T,XLLP_UINT32_T);
void XllpBcrPlatformInterruptSet_Clear(P_XLLP_BCR, XLLP_UINT32_T,XLLP_UINT32_T);
void XllpBcrWritePcmciaS0StatusControlReg(P_XLLP_BCR, XLLP_UINT32_T);
void XllpBcrWritePcmciaS1StatusControlReg(P_XLLP_BCR, XLLP_UINT32_T);
XLLP_UINT32_T XllpBcrReadPcmciaS0StatusControlReg(P_XLLP_BCR);
XLLP_UINT32_T XllpBcrReadPcmciaS1StatusControlReg(P_XLLP_BCR);
*/


#endif //__XLLP_BCR_H__

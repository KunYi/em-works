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
**  FILENAME:       xllp_gpio.h
**
**  PURPOSE: contains all GPIO specific macros, typedefs, and prototypes.
**           Declares no storage.
**                  
**
******************************************************************************/

#ifndef __GPIO_H__
#define __GPIO_H__

#include "xllp_defs.h"

/**
  GPIO Register Definitions
**/
typedef struct 
{
    XLLP_VUINT32_T GPLR0;             /* Level Detect Reg. Bank 0 */
    XLLP_VUINT32_T GPLR1;             /* Level Detect Reg. Bank 1 */
    XLLP_VUINT32_T GPLR2;             /* Level Detect Reg. Bank 2 */
    XLLP_VUINT32_T GPDR0;            /* Data Direction Reg. Bank 0 */
    XLLP_VUINT32_T GPDR1;            /* Data Direction Reg. Bank 1 */
    XLLP_VUINT32_T GPDR2;            /* Data Direction Reg. Bank 2 */
    XLLP_VUINT32_T GPSR0;            /* Pin Output Set Reg. Bank 0 */
    XLLP_VUINT32_T GPSR1;            /* Pin Output Set Reg. Bank 1 */
    XLLP_VUINT32_T GPSR2;            /* Pin Output Set Reg. Bank 2 */
    XLLP_VUINT32_T GPCR0;            /* Pin Output Clr Reg. Bank 0 */
    XLLP_VUINT32_T GPCR1;            /* Pin Output Clr Reg. Bank 1 */
    XLLP_VUINT32_T GPCR2;            /* Pin Output Clr Reg. Bank 2 */
    XLLP_VUINT32_T GRER0;   /* Ris. Edge Detect Enable Reg. Bank 0 */
    XLLP_VUINT32_T GRER1;   /* Ris. Edge Detect Enable Reg. Bank 1 */
    XLLP_VUINT32_T GRER2;   /* Ris. Edge Detect Enable Reg. Bank 2 */
    XLLP_VUINT32_T GFER0;   /* Fal. Edge Detect Enable Reg. Bank 0 */
    XLLP_VUINT32_T GFER1;   /* Fal. Edge Detect Enable Reg. Bank 1 */
    XLLP_VUINT32_T GFER2;   /* Fal. Edge Detect Enable Reg. Bank 2 */
    XLLP_VUINT32_T GEDR0;       /* Edge Detect Status Reg. Bank 0 */
    XLLP_VUINT32_T GEDR1;       /* Edge Detect Status Reg. Bank 1 */
    XLLP_VUINT32_T GEDR2;       /* Edge Detect Status Reg. Bank 2 */
    XLLP_VUINT32_T GAFR0_L;  /* Alt. Function Select Reg.[  0:15 ] */
    XLLP_VUINT32_T GAFR0_U;  /* Alt. Function Select Reg.[ 16:31 ] */
    XLLP_VUINT32_T GAFR1_L;  /* Alt. Function Select Reg.[ 32:47 ] */
    XLLP_VUINT32_T GAFR1_U;  /* Alt. Function Select Reg.[ 48:63 ] */
    XLLP_VUINT32_T GAFR2_L;  /* Alt. Function Select Reg.[ 64:79 ] */
    XLLP_VUINT32_T GAFR2_U;  /* Alt. Function Select Reg.[ 80:95 ] */
    XLLP_VUINT32_T GAFR3_L;  /* Alt. Function Select Reg.[ 96:111] */
    XLLP_VUINT32_T GAFR3_U;  /* Alt. Function Select Reg.[112:120] */
    XLLP_VUINT32_T  RESERVED1[35];    /* addr. offset 0x074-0x0fc */
    XLLP_VUINT32_T GPLR3;             /* Level Detect Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED2[2];      /* addr. offset 0x104-0x108 */
    XLLP_VUINT32_T GPDR3;            /* Data Direction Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED3[2];      /* addr. offset 0x110-0x114 */
    XLLP_VUINT32_T GPSR3;            /* Pin Output Set Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED4[2];      /* addr. offset 0x11c-0x120 */
    XLLP_VUINT32_T GPCR3;            /* Pin Output Clr Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED5[2];      /* addr. offset 0x128-0x12c */
    XLLP_VUINT32_T GRER3;   /* Ris. Edge Detect Enable Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED6[2];      /* addr. offset 0x134-0x138 */
    XLLP_VUINT32_T GFER3;   /* Fal. Edge Detect Enable Reg. Bank 3 */
    XLLP_VUINT32_T  RESERVED7[2];      /* addr. offset 0x140-0x144 */
    XLLP_VUINT32_T GEDR3;       /* Edge Detect Status Reg. Bank 3 */

} XLLP_GPIO_T, *P_XLLP_GPIO_T;


/* Begin of GPIO Pin Values  */
#define XLLP_GPIO_0                       0
#define XLLP_GPIO_1                       1
#define XLLP_GPIO_SYS_EN                  2
#define XLLP_GPIO_PWR_SCL                 3
#define XLLP_GPIO_PWR_SDA                 4
#define XLLP_GPIO_PWR_CAP0                5
#define XLLP_GPIO_PWR_CAP1                6
#define XLLP_GPIO_PWR_CAP2                7
#define XLLP_GPIO_PWR_CAP3                8
#define XLLP_GPIO_CLK_PIO                 9
#define XLLP_GPIO_CLK_TOUT               10
#define XLLP_GPIO_SSPRXD2				 11
#define XLLP_GPIO_CIF_DD7		         12
#define XLLP_GPIO_SSPTXD2                13
#define XLLP_GPIO_L_VSYNC                14
#define XLLP_GPIO_nCS1                   15
#define XLLP_GPIO_PWM_OUT0               16
#define XLLP_GPIO_PWM_OUT1               17
#define XLLP_GPIO_CIF_DD6                17
#define XLLP_GPIO_RDY                    18
#define XLLP_GPIO_L_CS                   19
#define XLLP_GPIO_MBREQ                  20
#define XLLP_GPIO_MBGNT                  21
#define XLLP_GPIO_SSPCLK2	             22
#define XLLP_GPIO_CIF_MCLK               23
#define XLLP_GPIO_CIF_FV                 24
#define XLLP_GPIO_CIF_LV                 25
#define XLLP_GPIO_CIF_PCLK               26
#define XLLP_GPIO_CIF_DD0                27
#define XLLP_GPIO_AC97BITCLK			 28
#define XLLP_GPIO_I2SBITCLK				 28  
#define XLLP_GPIO_AC97_SDATA_IN_0		 29
#define XLLP_GPIO_I2S_SDATA_IN			 29
#define XLLP_GPIO_I2S_SDATA_OUT			 30
#define XLLP_GPIO_AC97_SDATA_OUT         30 
#define XLLP_GPIO_I2S_SYNC				 31
#define XLLP_GPIO_AC97_SYNC				 31
#define XLLP_GPIO_MMCLK                  32
#define XLLP_GPIO_MSSCLK                 32 
#define XLLP_GPIO_nCS5                   33
#define XLLP_GPIO_FFRXD                  34
#define XLLP_GPIO_USB_P2_2               34
#define XLLP_GPIO_FFCTS                  35
#define XLLP_GPIO_USB_P2_1               35
#define XLLP_GPIO_FFDCD                  36
#define XLLP_GPIO_USB_P2_4               36
#define XLLP_GPIO_FFDSR                  37
#define XLLP_GPIO_FFRI	                 38
#define XLLP_GPIO_USB_P2_3	             38
#define XLLP_GPIO_FFTXD                  39
#define XLLP_GPIO_USB_P2_6               39
#define XLLP_GPIO_FFDTR                  40
#define XLLP_GPIO_USB_P2_5               40
#define XLLP_GPIO_FFRTS                  41
#define XLLP_GPIO_BTRXD                  42
#define XLLP_GPIO_BTTXD                  43
#define XLLP_GPIO_BTCTS                  44
#define XLLP_GPIO_BTRTS                  45
#define XLLP_GPIO_ICP_RXD                46
#define XLLP_GPIO_STD_RXD				 46
#define XLLP_GPIO_ICP_TXD                47
#define XLLP_GPIO_STD_TXD                47
#define XLLP_GPIO_BB_OB_DAT1             48
#define XLLP_GPIO_PCMCIA_nPOE            48
#define XLLP_GPIO_nPWE                   49
#define XLLP_GPIO_BB_OB_DAT2             50
#define XLLP_GPIO_PCMCIA_nPIOR			 50
#define XLLP_GPIO_BB_OB_DAT3             51
#define XLLP_GPIO_PCMCIA_nPIOW			 51
#define XLLP_GPIO_BB_OB_CLK              52
#define XLLP_GPIO_BB_OB_STB              53
#define XLLP_GPIO_BB_OB_WAIT             54
#define XLLP_GPIO_PCMCIA_nPCE2			 54
#define XLLP_GPIO_BB_IB_DAT1             55
#define XLLP_GPIO_PCMCIA_nPREG			 55
#define XLLP_GPIO_BB_IB_DAT2             56
#define XLLP_GPIO_PCMCIA_nPWAIT			 56
#define XLLP_GPIO_BB_IB_DAT3             57
#define XLLP_GPIO_PCMCIA_nIOIS16		 57
#define XLLP_GPIO_L_DD0                  58
#define XLLP_GPIO_L_DD1                  59
#define XLLP_GPIO_L_DD2                  60
#define XLLP_GPIO_L_DD3                  61 
#define XLLP_GPIO_L_DD4                  62
#define XLLP_GPIO_L_DD5                  63
#define XLLP_GPIO_L_DD6                  64
#define XLLP_GPIO_L_DD7                  65
#define XLLP_GPIO_L_DD8                  66
#define XLLP_GPIO_L_DD9                  67
#define XLLP_GPIO_L_DD10                 68
#define XLLP_GPIO_L_DD11                 69
#define XLLP_GPIO_L_DD12                 70
#define XLLP_GPIO_L_DD13                 71
#define XLLP_GPIO_L_DD14                 72
#define XLLP_GPIO_L_DD15                 73
#define XLLP_GPIO_L_FCLK                 74
#define XLLP_GPIO_L_LCLK                 75
#define XLLP_GPIO_L_PCLK                 76
#define XLLP_GPIO_L_BIAS                 77
#define XLLP_GPIO_nCS2                   78
#define XLLP_GPIO_PCMCIA_PSKTSEL         79
#define XLLP_GPIO_nCS4                   80
#define XLLP_GPIO_BB_OB_DAT0             81
#define XLLP_GPIO_BB_IB_DAT0             82
#define XLLP_GPIO_BB_IB_CLK              83
#define XLLP_GPIO_BB_IB_STB              84
#define XLLP_GPIO_BB_IB_WAIT             85
#define XLLP_GPIO_PCMCIA_nPCE1           85
#define XLLP_GPIO_L_DD16                 86
#define XLLP_GPIO_PCMCIA_nPCE1_1         86
#define XLLP_GPIO_L_DD17                 87
#define XLLP_GPIO_PCMCIA_nPCE1_2         87
#define XLLP_GPIO_USBHPWR0               88
#define XLLP_GPIO_SSPFRM2                88
#define XLLP_GPIO_USBHPEN0               89
#define XLLP_GPIO_URST                   90
#define XLLP_GPIO_CIF_DD4                90
#define XLLP_GPIO_UCLK                   91
#define XLLP_GPIO_CIF_DD5                91
#define XLLP_GPIO_MMDAT0                 92
#define XLLP_GPIO_MSBS					 92
#define XLLP_GPIO_KP_DKIN0               93
#define XLLP_GPIO_KP_DKIN1               94
#define XLLP_GPIO_KP_MKIN6               95
#define XLLP_GPIO_KP_MKOUT6              96
#define XLLP_GPIO_KP_MKIN3               97
#define XLLP_GPIO_KP_MKIN4               98
#define XLLP_GPIO_KP_MKIN5               99
#define XLLP_GPIO_KP_MKIN0              100
#define XLLP_GPIO_KP_MKIN1              101
#define XLLP_GPIO_KP_MKIN2              102
#define XLLP_GPIO_KP_MKOUT0             103
#define XLLP_GPIO_KP_MKOUT1             104
#define XLLP_GPIO_KP_MKOUT2             105
#define XLLP_GPIO_KP_MKOUT3             106
#define XLLP_GPIO_KP_MKOUT4             107
#define XLLP_GPIO_KP_MKOUT5             108
#define XLLP_GPIO_MMDAT1                109
#define XLLP_GPIO_MSSDIO			    109
#define XLLP_GPIO_MMDAT2                110
#define XLLP_GPIO_MMDAT3                111
#define XLLP_GPIO_MMCMD                 112
#define XLLP_GPIO_MSINS					112
#define XLLP_GPIO_AC97_RESET_n          113
#define XLLP_GPIO_I2S_SYSCLK			113
#define XLLP_GPIO_UVS0                  114
#define XLLP_GPIO_CIF_DD1               114
#define XLLP_GPIO_U_EN                  115
#define XLLP_GPIO_CIF_DD3               115
#define XLLP_GPIO_U_DET                 116
#define XLLP_GPIO_CIF_DD2               116
#define XLLP_GPIO_SCL                   117
#define XLLP_GPIO_SDA                   118
/* End of GPIO Pin Values  */
 
/******* Begin of GPIO Pin Bit Position ********/
/* GPIO Pin Bank 0 */
#define XLLP_GPIO_BIT_0						( XLLP_BIT_0 )
#define XLLP_GPIO_BIT_1						( XLLP_BIT_1 )
#define XLLP_GPIO_BIT_SYS_EN				( XLLP_BIT_2 )
#define XLLP_GPIO_BIT_PWR_SCL				( XLLP_BIT_3 )
#define XLLP_GPIO_BIT_PWR_SDA				( XLLP_BIT_4 )
#define XLLP_GPIO_BIT_PWR_CAP0				( XLLP_BIT_5 )
#define XLLP_GPIO_BIT_PWR_CAP1				( XLLP_BIT_6 )
#define XLLP_GPIO_BIT_PWR_CAP2				( XLLP_BIT_7 )
#define XLLP_GPIO_BIT_PWR_CAP3				( XLLP_BIT_8 )
#define XLLP_GPIO_BIT_CLK_PIO				( XLLP_BIT_9 )
#define XLLP_GPIO_BIT_CLK_TOUT				( XLLP_BIT_10 )
#define XLLP_GPIO_BIT_SSPRXD2				( XLLP_BIT_11 )
#define XLLP_GPIO_BIT_CIF_DD7				( XLLP_BIT_12 )
#define XLLP_GPIO_BIT_SSPTXD2				( XLLP_BIT_13 )
#define XLLP_GPIO_BIT_L_VSYNC				( XLLP_BIT_14 )
#define XLLP_GPIO_BIT_nCS1					( XLLP_BIT_15 )
#define XLLP_GPIO_BIT_PWM_OUT0				( XLLP_BIT_16 )
#define XLLP_GPIO_BIT_PWM_OUT1				( XLLP_BIT_17 )
#define XLLP_GPIO_BIT_CIF_DD6				( XLLP_BIT_17 )
#define XLLP_GPIO_BIT_RDY					( XLLP_BIT_18 )
#define XLLP_GPIO_BIT_L_CS					( XLLP_BIT_19 )
#define XLLP_GPIO_BIT_MBREQ                 ( XLLP_BIT_20 )
#define XLLP_GPIO_BIT_MBGNT                 ( XLLP_BIT_21 )
#define XLLP_GPIO_BIT_SSPCLK2	            ( XLLP_BIT_22 )
#define XLLP_GPIO_BIT_CIF_MCLK              ( XLLP_BIT_23 )
#define XLLP_GPIO_BIT_CIF_FV                ( XLLP_BIT_24 )
#define XLLP_GPIO_BIT_CIF_LV                ( XLLP_BIT_25 )
#define XLLP_GPIO_BIT_CIF_PCLK              ( XLLP_BIT_26 )
#define XLLP_GPIO_BIT_CIF_DD0               ( XLLP_BIT_27 )
#define XLLP_GPIO_BIT_AC97BITCLK			( XLLP_BIT_28 )
#define XLLP_GPIO_BIT_I2SBITCLK				( XLLP_BIT_28 )
#define XLLP_GPIO_BIT_AC97_SDATA_IN_0		( XLLP_BIT_29 )
#define XLLP_GPIO_BIT_I2S_SDATA_IN			( XLLP_BIT_29 )
#define XXLP_GPIO_BIT_SSP2_SSPRXD2          ( XLLP_BIT_29 )
#define XLLP_GPIO_BIT_I2S_SDATA_OUT			( XLLP_BIT_30 )
#define XLLP_GPIO_BIT_AC97_SDATA_OUT		( XLLP_BIT_30 )
#define XLLP_GPIO_BIT_I2S_SYNC				( XLLP_BIT_31 )
#define XLLP_GPIO_BIT_AC97_SYNC				( XLLP_BIT_31 )

/* GPIO Pin Bank 1 */
#define XLLP_GPIO_BIT_MMCLK					( XLLP_BIT_0 )
#define XLLP_GPIO_BIT_MSSCLK				( XLLP_BIT_0 )
#define XLLP_GPIO_BIT_nCS5					( XLLP_BIT_1 )
#define XLLP_GPIO_BIT_FFRXD					( XLLP_BIT_2 )
#define XLLP_GPIO_BIT_USB_P2_2              ( XLLP_BIT_2 )
#define XLLP_GPIO_BIT_FFCTS					( XLLP_BIT_3 )
#define XLLP_GPIO_BIT_USB_P2_1              ( XLLP_BIT_3 )
#define XLLP_GPIO_BIT_FFDCD					( XLLP_BIT_4 )
#define XLLP_GPIO_BIT_USB_P2_4              ( XLLP_BIT_4 )
#define XLLP_GPIO_BIT_FFDSR					( XLLP_BIT_5 )
#define XLLP_GPIO_BIT_USB_P2_8				( XLLP_BIT_5 )
#define XLLP_GPIO_BIT_FFRI					( XLLP_BIT_6 )
#define XLLP_GPIO_BIT_USB_P2_3	            ( XLLP_BIT_6 )
#define XLLP_GPIO_BIT_FFTXD					( XLLP_BIT_7 )
#define XLLP_GPIO_BIT_USB_P2_6              ( XLLP_BIT_7 )
#define XLLP_GPIO_BIT_FFDTR					( XLLP_BIT_8 )
#define XLLP_GPIO_BIT_USB_P2_5              ( XLLP_BIT_8 )
#define XLLP_GPIO_BIT_FFRTS					( XLLP_BIT_9 )
#define XLLP_GPIO_BIT_USB_P2_7              ( XLLP_BIT_9 )
#define XLLP_GPIO_BIT_BTRXD					( XLLP_BIT_10 )
#define XLLP_GPIO_BIT_BTTXD					( XLLP_BIT_11 )
#define XLLP_GPIO_BIT_BTCTS					( XLLP_BIT_12 )
#define XLLP_GPIO_BIT_BTRTS					( XLLP_BIT_13 )
#define XLLP_GPIO_BIT_ICP_RXD				( XLLP_BIT_14 )
#define XLLP_GPIO_BIT_STD_RXD				( XLLP_BIT_14 )
#define XLLP_GPIO_BIT_ICP_TXD				( XLLP_BIT_15 )
#define XLLP_GPIO_BIT_STD_TXD				( XLLP_BIT_15 )
#define XLLP_GPIO_BIT_BB_OB_DAT1			( XLLP_BIT_16 )
#define XLLP_GPIO_BIT_PCMCIA_nPOE			( XLLP_BIT_16 )
#define XLLP_GPIO_BIT_nPWE					( XLLP_BIT_17 )
#define XLLP_GPIO_BIT_BB_OB_DAT2			( XLLP_BIT_18 )
#define XLLP_GPIO_BIT_PCMCIA_nPIOR			( XLLP_BIT_18 )
#define XLLP_GPIO_BIT_BB_OB_DAT3			( XLLP_BIT_19 )
#define XLLP_GPIO_BIT_PCMCIA_nPIOW			( XLLP_BIT_19 )
#define XLLP_GPIO_BIT_BB_OB_CLK				( XLLP_BIT_20 )
#define XLLP_GPIO_BIT_BB_OB_STB				( XLLP_BIT_21 )
#define XLLP_GPIO_BIT_BB_OB_WAIT			( XLLP_BIT_22 )
#define XLLP_GPIO_BIT_PCMCIA_nPCE2			( XLLP_BIT_22 )
#define XLLP_GPIO_BIT_BB_IB_DAT1			( XLLP_BIT_23 )
#define XLLP_GPIO_BIT_PCMCIA_nPREG			( XLLP_BIT_23 )
#define XLLP_GPIO_BIT_BB_IB_DAT2			( XLLP_BIT_24 )
#define XLLP_GPIO_BIT_PCMCIA_nPWAIT			( XLLP_BIT_24 )
#define XLLP_GPIO_BIT_BB_IB_DAT3			( XLLP_BIT_25 )
#define XLLP_GPIO_BIT_PCMCIA_nIOIS16		( XLLP_BIT_25 )
#define XLLP_GPIO_BIT_L_DD0					( XLLP_BIT_26 )
#define XLLP_GPIO_BIT_L_DD1					( XLLP_BIT_27 )
#define XLLP_GPIO_BIT_L_DD2					( XLLP_BIT_28 )
#define XLLP_GPIO_BIT_L_DD3					( XLLP_BIT_29 )
#define XLLP_GPIO_BIT_L_DD4					( XLLP_BIT_30 )
#define XLLP_GPIO_BIT_L_DD5					( XLLP_BIT_31 )

/* GPIO Pin Bank 2 */
#define XLLP_GPIO_BIT_L_DD6					( XLLP_BIT_0 )
#define XLLP_GPIO_BIT_L_DD7					( XLLP_BIT_1 )
#define XLLP_GPIO_BIT_L_DD8					( XLLP_BIT_2 )
#define XLLP_GPIO_BIT_L_DD9					( XLLP_BIT_3 )
#define XLLP_GPIO_BIT_L_DD10				( XLLP_BIT_4 )
#define XLLP_GPIO_BIT_L_DD11				( XLLP_BIT_5 )
#define XLLP_GPIO_BIT_L_DD12				( XLLP_BIT_6 ) 
#define XLLP_GPIO_BIT_L_DD13				( XLLP_BIT_7 )
#define XLLP_GPIO_BIT_L_DD14				( XLLP_BIT_8 )
#define XLLP_GPIO_BIT_L_DD15				( XLLP_BIT_9 )
#define XLLP_GPIO_BIT_L_FCLK				( XLLP_BIT_10 )
#define XLLP_GPIO_BIT_L_LCLK				( XLLP_BIT_11 )
#define XLLP_GPIO_BIT_L_PCLK				( XLLP_BIT_12 )
#define XLLP_GPIO_BIT_L_BIAS				( XLLP_BIT_13 )
#define XLLP_GPIO_BIT_nCS2					( XLLP_BIT_14 )
#define XLLP_GPIO_BIT_PCMCIA_PSKTSEL		( XLLP_BIT_15 )
#define XLLP_GPIO_BIT_nCS4					( XLLP_BIT_16 )
#define XLLP_GPIO_BIT_BB_OB_DAT0			( XLLP_BIT_17 )
#define XLLP_GPIO_BIT_BB_IB_DAT0			( XLLP_BIT_18 )
#define XLLP_GPIO_BIT_BB_IB_CLK				( XLLP_BIT_19 )
#define XLLP_GPIO_BIT_BB_IB_STB				( XLLP_BIT_20 )
#define XLLP_GPIO_BIT_BB_IB_WAIT			( XLLP_BIT_21 )
#define XLLP_GPIO_BIT_PCMCIA_nPCE1			( XLLP_BIT_21 )
#define XLLP_GPIO_BIT_L_DD16				( XLLP_BIT_22 )
#define XLLP_GPIO_BIT_PCMCIA_nPCE1_1        ( XLLP_BIT_22 )
#define XLLP_GPIO_BIT_L_DD17				( XLLP_BIT_23 )
#define XLLP_GPIO_BIT_PCMCIA_nPCE1_2        ( XLLP_BIT_23 )
#define XLLP_GPIO_BIT_USBHPWR0				( XLLP_BIT_24 )
#define XLLP_GPIO_BIT_SSPFRM2               ( XLLP_BIT_24 )
#define XLLP_GPIO_BIT_USBHPEN0				( XLLP_BIT_25 )
#define XLLP_GPIO_BIT_URST					( XLLP_BIT_26 )
#define XLLP_GPIO_BIT_CIF_DD4               ( XLLP_BIT_26 )
#define XLLP_GPIO_BIT_UCLK					( XLLP_BIT_27 )
#define XLLP_GPIO_BIT_CIF_DD5               ( XLLP_BIT_27 )
#define XLLP_GPIO_BIT_MMDAT0				( XLLP_BIT_28 )
#define XLLP_GPIO_BIT_MSBS					( XLLP_BIT_28 )
#define XLLP_GPIO_BIT_KP_DKIN0				( XLLP_BIT_29 )
#define XLLP_GPIO_BIT_KP_DKIN1				( XLLP_BIT_30 )
#define XLLP_GPIO_BIT_KP_MKIN6              ( XLLP_BIT_31 )


/* GPIO Pin Bank 3 */
#define XLLP_GPIO_BIT_KP_MKOUT6             ( XLLP_BIT_0 )
#define XLLP_GPIO_BIT_KP_MKIN3              ( XLLP_BIT_1 )
#define XLLP_GPIO_BIT_KP_MKIN4              ( XLLP_BIT_2 )
#define XLLP_GPIO_BIT_KP_MKIN5              ( XLLP_BIT_3 )
#define XLLP_GPIO_BIT_KP_MKIN0				( XLLP_BIT_4 )
#define XLLP_GPIO_BIT_KP_MKIN1				( XLLP_BIT_5 )
#define XLLP_GPIO_BIT_KP_MKIN2				( XLLP_BIT_6 )
#define XLLP_GPIO_BIT_KP_MKOUT0				( XLLP_BIT_7 )
#define XLLP_GPIO_BIT_KP_MKOUT1				( XLLP_BIT_8 )
#define XLLP_GPIO_BIT_KP_MKOUT2				( XLLP_BIT_9 )
#define XLLP_GPIO_BIT_KP_MKOUT3				( XLLP_BIT_10 )
#define XLLP_GPIO_BIT_KP_MKOUT4				( XLLP_BIT_11 )
#define XLLP_GPIO_BIT_KP_MKOUT5				( XLLP_BIT_12 )
#define XLLP_GPIO_BIT_MMDAT1				( XLLP_BIT_13 )
#define XLLP_GPIO_BIT_MSSDIO				( XLLP_BIT_13 )
#define XLLP_GPIO_BIT_MMDAT2				( XLLP_BIT_14 )
#define XLLP_GPIO_BIT_MMDAT3				( XLLP_BIT_15 )
#define XLLP_GPIO_BIT_MMCMD					( XLLP_BIT_16 )
#define XLLP_GPIO_BIT_MSINS					( XLLP_BIT_16 )
#define XLLP_GPIO_BIT_AC97_RESET_n			( XLLP_BIT_17 )
#define XLLP_GPIO_BIT_I2S_SYSCLK			( XLLP_BIT_17 )
#define XLLP_GPIO_BIT_UVS0					( XLLP_BIT_18 )
#define XLLP_GPIO_BIT_CIF_DD1               ( XLLP_BIT_18 )
#define XLLP_GPIO_BIT_U_EN                  ( XLLP_BIT_19 )
#define XLLP_GPIO_BIT_CIF_DD3               ( XLLP_BIT_19 )
#define XLLP_GPIO_BIT_U_DET                 ( XLLP_BIT_20 )
#define XLLP_GPIO_BIT_CIF_DD2               ( XLLP_BIT_20 )
#define XLLP_GPIO_BIT_SCL					( XLLP_BIT_21 )
#define XLLP_GPIO_BIT_SDA					( XLLP_BIT_22 )

/******* End of GPIO Pin Bit Position ********/

/*=================================================================*/
/* BEGIN of alternate function values for each GPIO  -- in BIT-WISE */
//
//
/* Pin  11  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPRXD2			(0x2u  << 22)
#define XLLP_GPIO_AF_BIT_SSPRXD2_MASK		(0x3u  << 22)

/* Pin  12  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_DD7			(0x2u  << 24)
#define XLLP_GPIO_AF_BIT_CIF_DD7_MASK		(0x3u  << 24)

/* Pin  13  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPTXD2			(XLLP_BIT_26)
#define XLLP_GPIO_AF_BIT_SSPTXD2_MASK		(0x3u  << 26)

/* Pin  14  alternate functions */
#define XLLP_GPIO_AF_BIT_L_VSYNC			(XLLP_BIT_28)
#define XLLP_GPIO_AF_BIT_L_VSYNC_MASK		(0x3u  << 28)

/* Pin  15  alternate functions */
#define XLLP_GPIO_AF_BIT_nCS1				(0x2u  << 30)
#define XLLP_GPIO_AF_BIT_nCS1_MASK			(0x3u  << 30)

/* Pin  16  alternate functions */
#define XLLP_GPIO_AF_BIT_PWM_OUT0			(0x2u  << 0)
#define XLLP_GPIO_AF_BIT_PWM_OUT0_MASK		(0x3u  << 0)

/* Pin  17  alternate functions */
#define XLLP_GPIO_AF_BIT_PWM_OUT1			(0x2u  << 2)
#define XLLP_GPIO_AF_BIT_PWM_OUT1_MASK		(0x3u  << 2)

/* Pin  17  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_DD6			(0x2u  << 2)
#define XLLP_GPIO_AF_BIT_CIF_DD6_MASK		(0x3u  << 2)

/* Pin  18  alternate functions */
#define XLLP_GPIO_AF_BIT_RDY				(XLLP_BIT_4)
#define XLLP_GPIO_AF_BIT_RDY_MASK			(0x3u  << 4)

/* Pin  19  alternate functions */
#define XLLP_GPIO_AF_BIT_L_CS				(0x2u  << 6)
#define XLLP_GPIO_AF_BIT_L_CS_MASK			(0x3u  << 6)

/* Pin  20  alternate functions */
#define XLLP_GPIO_AF_BIT_MBREQ				(0x2u  << 8)
#define XLLP_GPIO_AF_BIT_MBREQ_MASK			(0x3u  << 8)

/* Pin  21  alternate functions */
#define XLLP_GPIO_AF_BIT_MBGNT				(0x3u  << 10)
#define XLLP_GPIO_AF_BIT_MBGNT_MASK			(0x3u  << 10)

/* Pin  22  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPCLK2			(0x3u  << 12)
#define XLLP_GPIO_AF_BIT_SSPCLK2_MASK		(0x3u  << 12)

/* Pin  23  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_MCLK			(XLLP_BIT_14)
#define XLLP_GPIO_AF_BIT_CIF_MCLK_MASK		(0x3u  << 14)

/* Pin  24  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_FV				(XLLP_BIT_16)
#define XLLP_GPIO_AF_BIT_CIF_FV_MASK		(0x3u  << 16)

/* Pin  25  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_LV				(XLLP_BIT_18)
#define XLLP_GPIO_AF_BIT_CIF_LV_MASK		(0x3u  << 18)

/* Pin  26  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_PCLK			(0x2u  << 20)
#define XLLP_GPIO_AF_BIT_CIF_PCLK_MASK		(0x3u  << 20)

/* Pin  27  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_DD0			(0x3u  << 22)
#define XLLP_GPIO_AF_BIT_CIF_DD0_MASK		(0x3u  << 22)

/* Pin  28  alternate functions */
#define XLLP_GPIO_AF_BIT_AC97_BITCLK		(XLLP_BIT_24)
#define XLLP_GPIO_AF_BIT_AC97_BITCLK_MASK	(0x3u  << 24)
#define XLLP_GPIO_AF_BIT_I2SBITCLK_IN		(0x2u  << 24)
#define XLLP_GPIO_AF_BIT_I2SBITCLK_IN_MASK	(0x3u  << 24)
#define XLLP_GPIO_AF_BIT_I2SBITCLK_OUT		(XLLP_BIT_24)
#define XLLP_GPIO_AF_BIT_I2SBITCLK_OUT_MASK	(0x3u  << 24)

/* Pin  29  alternate functions */
#define XLLP_GPIO_AF_BIT_AC97_SDATA_IN_0		(XLLP_BIT_26)
#define XLLP_GPIO_AF_BIT_AC97_SDATA_IN_0_MASK	(0x3u  << 26)
#define XLLP_GPIO_AF_BIT_I2S_SDATA_IN			(0x2u  << 26)
#define XLLP_GPIO_AF_BIT_I2S_SDATA_IN_MASK		(0x3u  << 26)
#define XLLP_GPIO_AF_BIT_SSP2_SSPRXD2           (0x1u << 26)
#define XLLP_GPIO_AF_BIT_SSP2_SSPRXD2_MASK      (0x3u << 26)

/* Pin  30  alternate functions */
#define XLLP_GPIO_AF_BIT_I2S_SDATA_OUT			(XLLP_BIT_28)
#define XLLP_GPIO_AF_BIT_I2S_SDATA_OUT_MASK		(0x3u  << 28)
#define XLLP_GPIO_AF_BIT_AC97_SDATA_OUT			(0x2u  << 28)
#define XLLP_GPIO_AF_BIT_AC97_SDATA_OUT_MASK	(0x3u  << 28)

/* Pin  31  alternate functions */
#define XLLP_GPIO_AF_BIT_I2S_SYNC			(XLLP_BIT_30)
#define XLLP_GPIO_AF_BIT_I2S_SYNC_MASK		(0x3u  << 30)
#define XLLP_GPIO_AF_BIT_AC97_SYNC			(0x2u  << 30)
#define XLLP_GPIO_AF_BIT_AC97_SYNC_MASK		(0x3u  << 30)

/* Pin  32  alternate functions */
#define XLLP_GPIO_AF_BIT_MSSCLK				(XLLP_BIT_0)
#define XLLP_GPIO_AF_BIT_MSSCLK_MASK		(0x3u  << 0)		
#define XLLP_GPIO_AF_BIT_MMCLK				(0x2u  << 0)
#define XLLP_GPIO_AF_BIT_MMCLK_MASK			(0x3u  << 0)

/* Pin  33  alternate functions */
#define XLLP_GPIO_AF_BIT_nCS5				(0x2u  << 2)
#define XLLP_GPIO_AF_BIT_nCS5_MASK			(0x3u  << 2)

/* Pin  34  alternate functions */
#define XLLP_GPIO_AF_BIT_FFRXD				(XLLP_BIT_4)
#define XLLP_GPIO_AF_BIT_FFRXD_MASK			(0x3u  << 4)

/* Pin  34  alternate functions */
#define XLLP_GPIO_AF_BIT_USB_P2_2			(XLLP_BIT_4)
#define XLLP_GPIO_AF_BIT_USB_P2_2_MASK		(0x3u  << 4)

/* Pin  35  alternate functions */
#define XLLP_GPIO_AF_BIT_FFCTS				(XLLP_BIT_6)
#define XLLP_GPIO_AF_BIT_FFCTS_MASK			(0x3u  << 6)

/* Pin  35  alternate functions */
#define XLLP_GPIO_AF_BIT_USB_P2_1			(0x2u  << 6)
#define XLLP_GPIO_AF_BIT_USB_P2_1_MASK		(0x3u  << 6)

/* Pin  36  alternate functions */
#define XLLP_GPIO_AF_BIT_FFDCD				(XLLP_BIT_8)
#define XLLP_GPIO_AF_BIT_FFDCD_MASK			(0x3u  << 8)

/* Pin  36  alternate functions */
#define XLLP_GPIO_AF_BIT_USB_P2_4			(XLLP_BIT_8)
#define XLLP_GPIO_AF_BIT_USB_P2_4_MASK		(0x3u  << 8)

/* Pin  36  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPSCLK2			(0x2u  << 8)
#define XLLP_GPIO_AF_BIT_SSPSCLK2_MASK		(0x3u  << 8)

/* Pin  37  alternate functions */
#define XLLP_GPIO_AF_BIT_FFDSR				(XLLP_BIT_10)
#define XLLP_GPIO_AF_BIT_FFDSR_MASK			(0x3u  << 10)

/* Pin  37  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPSFRM2			(0x2u  << 10)
#define XLLP_GPIO_AF_BIT_SSPSFRM2_MASK		(0x3u  << 10)

/* Pin  37  alternate functions */
#define XLLP_GPIO_AF_BIT_USB_P2_8			(0x1u  << 10)
#define XLLP_GPIO_AF_BIT_USB_P2_8_MASK		(0x3u  << 10)

/* Pin  38  alternate functions */
#define XLLP_GPIO_AF_BIT_FFRI				(XLLP_BIT_12)
#define XLLP_GPIO_AF_BIT_FFRI_MASK			(0x3u  << 12)

/* Pin  38  alternate functions */
#define XLLP_GPIO_AF_BIT_USB_P2_3			(0x3u  << 12)
#define XLLP_GPIO_AF_BIT_USB_P2_3_MASK		(0x3u  << 12)

/* Pin  39  alternate functions */
#define XLLP_GPIO_AF_BIT_FFTXD				(0x2u  << 14)
#define XLLP_GPIO_AF_BIT_FFTXD_MASK			(0x3u  << 14)

/* Pin  39  alternate functions */
#define XLLP_GPIO_AF_BIT_USB_P2_6			(XLLP_BIT_14)
#define XLLP_GPIO_AF_BIT_USB_P2_6_MASK		(0x3u  << 14)

/* Pin  40  alternate functions */
#define XLLP_GPIO_AF_BIT_FFDTR				(0x2u  << 16)
#define XLLP_GPIO_AF_BIT_FFDTR_MASK			(0x3u  << 16)

/* Pin  40  alternate functions */
#define XLLP_GPIO_AF_BIT_USB_P2_5			(0x3u  << 16)
#define XLLP_GPIO_AF_BIT_USB_P2_5_MASK		(0x3u  << 16)

/* Pin  41  alternate functions */
#define XLLP_GPIO_AF_BIT_FFRTS				(0x2u  << 18)
#define XLLP_GPIO_AF_BIT_FFRTS_MASK			(0x3u  << 18)

#define XLLP_GPIO_AF_BIT_USB_P2_7			(0x2u  << 18)
#define XLLP_GPIO_AF_BIT_USB_P2_7_MASK		(0x3u  << 18)

/* Pin  42  alternate functions */
#define XLLP_GPIO_AF_BIT_BTRXD				(XLLP_BIT_20)
#define XLLP_GPIO_AF_BIT_BTRXD_MASK			(0x3u  << 20)

/* Pin  43  alternate functions */
#define XLLP_GPIO_AF_BIT_BTTXD				(0x2u  << 22)
#define XLLP_GPIO_AF_BIT_BTTXD_MASK			(0x3u  << 22)

/* Pin  44  alternate functions */
#define XLLP_GPIO_AF_BIT_BTCTS				(XLLP_BIT_24)
#define XLLP_GPIO_AF_BIT_BTCTS_MASK			(0x3u  << 24)

/* Pin  45  alternate functions */
#define XLLP_GPIO_AF_BIT_BTRTS				(0x2u  << 26)
#define XLLP_GPIO_AF_BIT_BTRTS_MASK			(0x3u  << 26)

/* Pin  46  alternate functions */
#define XLLP_GPIO_AF_BIT_ICP_RXD			(XLLP_BIT_28)
#define XLLP_GPIO_AF_BIT_ICP_RXD_MASK		(0x3u  << 28)
#define XLLP_GPIO_AF_BIT_STD_RXD			(0x2u  << 28)
#define XLLP_GPIO_AF_BIT_STD_RXD_MASK		(0x3u  << 28)

/* Pin  47  alternate functions */
#define XLLP_GPIO_AF_BIT_ICP_TXD			(0x2u  << 30)
#define XLLP_GPIO_AF_BIT_ICP_TXD_MASK		(0x3u  << 30)
#define XLLP_GPIO_AF_BIT_STD_TXD			(XLLP_BIT_30)
#define XLLP_GPIO_AF_BIT_STD_TXD_MASK		(0x3u  << 30)

/* Pin  48  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_OB_DAT1			(XLLP_BIT_0)
#define XLLP_GPIO_AF_BIT_BB_OB_DAT1_MASK	(0x3u  << 0)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPOE		(0x2u  << 0)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPOE_MASK	(0x3u  << 0)

/* Pin  49  alternate functions */
#define XLLP_GPIO_AF_BIT_nPWE				(0x2u  << 2)
#define XLLP_GPIO_AF_BIT_nPWE_MASK			(0x3u  << 2)

/* Pin  50  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_OB_DAT2			(XLLP_BIT_4)
#define XLLP_GPIO_AF_BIT_BB_OB_DAT2_MASK	(0x3u  << 4)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPIOR		(0x2u  << 4)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPIOR_MASK	(0x3u  << 4)

/* Pin  51  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_OB_DAT3			(XLLP_BIT_6)
#define XLLP_GPIO_AF_BIT_BB_OB_DAT3_MASK	(0x3u  << 6)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPIOW		(0x2u  << 6)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPIOW_MASK	(0x3u  << 6)

/* Pin  52  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_OB_CLK			(XLLP_BIT_8)
#define XLLP_GPIO_AF_BIT_BB_OB_CLK_MASK		(0x3u  << 8)

/* Pin  52  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPSCLK3			(0x2u  << 8)
#define XLLP_GPIO_AF_BIT_SSPSCLK3_MASK		(0x3u  << 8)

/* Pin  53  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_OB_STB			(XLLP_BIT_10)
#define XLLP_GPIO_AF_BIT_BB_OB_STB_MASK		(0x3u  << 10)

/* Pin  54  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_OB_WAIT			(0x2u  << 12)
#define XLLP_GPIO_AF_BIT_BB_OB_WAIT_MASK	(0x3u  << 12)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPCE2		(0x2u  << 12)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPCE2_MASK  (0x3u  << 12)

/* Pin  55  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_IB_DAT1			(0x2u  << 14)
#define XLLP_GPIO_AF_BIT_BB_IB_DAT1_MASK	(0x3u  << 14)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPREG		(0x2u  << 14)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPREG_MASK	(0x3u  << 14)

/* Pin  56  alternate functions */
#define XLLP_GPIO_AF_BIT_PCMCIA_nPWAIT		(XLLP_BIT_16)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPWAIT_MASK	(0x3u  << 16)
#define XLLP_GPIO_AF_BIT_BB_IB_DAT2			(0x2u  << 16)
#define XLLP_GPIO_AF_BIT_BB_IB_DAT2_MASK	(0x3u  << 16)

/* Pin  57  alternate functions */
#define XLLP_GPIO_AF_BIT_PCMCIA_nIOIS16			(XLLP_BIT_18)
#define XLLP_GPIO_AF_BIT_PCMCIA_nIOIS16_MASK	(0x3u  << 18)
#define XLLP_GPIO_AF_BIT_BB_IB_DAT3				(0x2u  << 18)
#define XLLP_GPIO_AF_BIT_BB_IB_DAT3_MASK		(0x3u  << 18)

/* Pin  58  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD0				(0x2u  << 20)
#define XLLP_GPIO_AF_BIT_L_DD0_MASK			(0x3u  << 20)

/* Pin  59  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD1				(0x2u  << 22)
#define XLLP_GPIO_AF_BIT_L_DD1_MASK			(0x3u  << 22)

/* Pin  60  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD2				(0x2u  << 24)
#define XLLP_GPIO_AF_BIT_L_DD2_MASK			(0x3u  << 24)

/* Pin  61  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD3				(0x2u  << 26)
#define XLLP_GPIO_AF_BIT_L_DD3_MASK			(0x3u  << 26)

/* Pin  62  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD4				(0x2u  << 28)
#define XLLP_GPIO_AF_BIT_L_DD4_MASK			(0x3u  << 28)

/* Pin  63  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD5				(0x2u  << 30)
#define XLLP_GPIO_AF_BIT_L_DD5_MASK			(0x3u  << 30)

/* Pin  64  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD6				(0x2u  << 0)
#define XLLP_GPIO_AF_BIT_L_DD6_MASK			(0x3u  << 0)

/* Pin  65  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD7				(0x2u  << 2)
#define XLLP_GPIO_AF_BIT_L_DD7_MASK			(0x3u  << 2)

/* Pin  66  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD8				(0x2u  << 4)
#define XLLP_GPIO_AF_BIT_L_DD8_MASK			(0x3u  << 4)

/* Pin  67  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD9				(0x2u  << 6)
#define XLLP_GPIO_AF_BIT_L_DD9_MASK			(0x3u  << 6)

/* Pin  68  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD10				(0x2u  << 8)
#define XLLP_GPIO_AF_BIT_L_DD10_MASK		(0x3u  << 8)

/* Pin  69  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD11				(0x2u  << 10)
#define XLLP_GPIO_AF_BIT_L_DD11_MASK		(0x3u  << 10)

/* Pin  70  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD12				(0x2u  << 12)
#define XLLP_GPIO_AF_BIT_L_DD12_MASK		(0x3u  << 12)

/* Pin  71  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD13				(0x2u  << 14)
#define XLLP_GPIO_AF_BIT_L_DD13_MASK		(0x3u  << 14)

/* Pin  72  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD14				(0x2u  << 16)
#define XLLP_GPIO_AF_BIT_L_DD14_MASK		(0x3u  << 16)

/* Pin  73  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD15				(0x2u  << 18)
#define XLLP_GPIO_AF_BIT_L_DD15_MASK		(0x3u  << 18)

/* Pin  74  alternate functions */
#define XLLP_GPIO_AF_BIT_L_FCLK_RD			(0x2u  << 20)
#define XLLP_GPIO_AF_BIT_L_FCLK_RD_MASK		(0x3u  << 20)

/* Pin  75  alternate functions */
#define XLLP_GPIO_AF_BIT_L_LCLK_A0			(0x2u  << 22)
#define XLLP_GPIO_AF_BIT_L_LCLK_A0_MASK		(0x3u  << 22)

/* Pin  76  alternate functions */
#define XLLP_GPIO_AF_BIT_L_PCLK_WR			(0x2u  << 24)
#define XLLP_GPIO_AF_BIT_L_PCLK_WR_MASK		(0x3u  << 24)

/* Pin  77  alternate functions */
#define XLLP_GPIO_AF_BIT_L_BIAS				(0x2u  << 26)
#define XLLP_GPIO_AF_BIT_L_BIAS_MASK		(0x3u  << 26)

/* Pin  78  alternate functions */
#define XLLP_GPIO_AF_BIT_nCS2				(0x2u  << 28)
#define XLLP_GPIO_AF_BIT_nCS2_MASK			(0x3u  << 28)

/* Pin  79  alternate functions */
#define XLLP_GPIO_AF_BIT_PCMCIA_PSKTSEL			(XLLP_BIT_30)
#define XLLP_GPIO_AF_BIT_PCMCIA_PSKTSEL_MASK	(0x3u  << 30)

/* Pin  80  alternate functions */
#define XLLP_GPIO_AF_BIT_nCS4				(0x2u  << 0)
#define XLLP_GPIO_AF_BIT_nCS4_MASK			(0x3u  << 0)

/* Pin  81  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_OB_DAT0			(0x2u  << 2)
#define XLLP_GPIO_AF_BIT_BB_OB_DAT0_MASK	(0x3u  << 2)

/* Pin  81  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPTXD3			(XLLP_BIT_2)
#define XLLP_GPIO_AF_BIT_SSPTXD3_MASK		(0x3u  << 2)

/* Pin  82  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_IB_DAT0			(0x2u  << 4)
#define XLLP_GPIO_AF_BIT_BB_IB_DAT0_MASK	(0x3u  << 4)

/* Pin  82  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPRXD3			(XLLP_BIT_4)
#define XLLP_GPIO_AF_BIT_SSPRXD3_MASK		(0x3u  << 4)

/* Pin  83  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_IB_CLK			(0x2u  << 6)
#define XLLP_GPIO_AF_BIT_BB_IB_CLK_MASK		(0x3u  << 6)

/* Pin  83  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPSFRM3			(XLLP_BIT_6)
#define XLLP_GPIO_AF_BIT_SSPSFRM3_MASK		(0x3u  << 6)

/* Pin  84  alternate functions */
#define XLLP_GPIO_AF_BIT_BB_IB_STB			(0x2u  << 8)
#define XLLP_GPIO_AF_BIT_BB_IB_STB_MASK		(0x3u  << 8)

/* Pin  85  alternate functions */
#define XLLP_GPIO_AF_BIT_PCMCIA_nPCE1	    (XLLP_BIT_10)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPCE1_MASK  (0x3u  << 10)
#define XLLP_GPIO_AF_BIT_BB_IB_WAIT			(0x2u  << 10)
#define XLLP_GPIO_AF_BIT_BB_IB_WAIT_MASK	(0x3u  << 10)

/* Pin  86  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD16				(0x2u  << 12)
#define XLLP_GPIO_AF_BIT_L_DD16_MASK		(0x3u  << 12)

/* Pin  86  alternate functions */
#define XLLP_GPIO_AF_BIT_PCMCIA_nPCE1_1		 (XLLP_BIT_12)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPCE1_1_MASK (0x3u  << 12)

/* Pin  87  alternate functions */
#define XLLP_GPIO_AF_BIT_L_DD17				(0x2u  << 14)
#define XLLP_GPIO_AF_BIT_L_DD17_MASK		(0x3u  << 14)

/* Pin  87  alternate functions */
#define XLLP_GPIO_AF_BIT_PCMCIA_nPCE1_2		 (XLLP_BIT_14)
#define XLLP_GPIO_AF_BIT_PCMCIA_nPCE1_2_MASK (0x3u  << 14)

/* Pin  88  alternate functions */
#define XLLP_GPIO_AF_BIT_USBHPWR0			(XLLP_BIT_16)
#define XLLP_GPIO_AF_BIT_USBHPWR0_MASK		(0x3u  << 16)

/* Pin  88  alternate functions */
#define XLLP_GPIO_AF_BIT_SSPFRM2			(0x3u  << 16)
#define XLLP_GPIO_AF_BIT_SSPFRM2_MASK		(0x3u  << 16)

/* Pin  89  alternate functions */
#define XLLP_GPIO_AF_BIT_USBHPEN0			(0x2u  << 18)
#define XLLP_GPIO_AF_BIT_USBHPEN0_MASK		(0x3u  << 18)

/* Pin  90  alternate functions */
#define XLLP_GPIO_AF_BIT_URST				(0x2u  << 20)
#define XLLP_GPIO_AF_BIT_URST_MASK			(0x3u  << 20)

/* Pin  90  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_DD4			(0x3u  << 20)
#define XLLP_GPIO_AF_BIT_CIF_DD4_MASK		(0x3u  << 20)

/* Pin  91  alternate functions */
#define XLLP_GPIO_AF_BIT_UCLK				(0x2u  << 22)
#define XLLP_GPIO_AF_BIT_UCLK_MASK			(0x3u  << 22)

/* Pin  91  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_DD5			(0x3u  << 22)
#define XLLP_GPIO_AF_BIT_CIF_DD5_MASK		(0x3u  << 22)

/* Pin  92  alternate functions */
#define XLLP_GPIO_AF_BIT_MMDAT0				(XLLP_BIT_24)
#define XLLP_GPIO_AF_BIT_MMDAT0_MASK		(0x3u  << 24)
#define XLLP_GPIO_AF_BIT_MSBS				(0x2u  << 24)
#define XLLP_GPIO_AF_BIT_MSBS_MASK			(0x3u  << 24)

/* Pin  93  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_DKIN0			(XLLP_BIT_26)
#define XLLP_GPIO_AF_BIT_KP_DKIN0_MASK		(0x3u  << 26)

/* Pin  94  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_DKIN1			(XLLP_BIT_28)
#define XLLP_GPIO_AF_BIT_KP_DKIN1_MASK		(0x3u  << 28)

/* Pin  95  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKIN6			(0x3u  << 30)
#define XLLP_GPIO_AF_BIT_KP_MKIN6_MASK		(0x3u  << 30)

/* Pin  96  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKOUT6			(0x3u  << 0)
#define XLLP_GPIO_AF_BIT_KP_MKOUT6_MASK		(0x3u  << 0)

/* Pin  97  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKIN3			(0x3u  << 2)
#define XLLP_GPIO_AF_BIT_KP_MKIN3_MASK		(0x3u  << 2)

/* Pin  98  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKIN4			(0x3u  << 4)
#define XLLP_GPIO_AF_BIT_KP_MKIN4_MASK		(0x3u  << 4)

/* Pin  99  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKIN5			(0x3u  << 6)
#define XLLP_GPIO_AF_BIT_KP_MKIN5_MASK		(0x3u  << 6)

/* Pin  100  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKIN0			(XLLP_BIT_8)
#define XLLP_GPIO_AF_BIT_KP_MKIN0_MASK		(0x3u  << 8)

/* Pin  101  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKIN1			(XLLP_BIT_10)
#define XLLP_GPIO_AF_BIT_KP_MKIN1_MASK		(0x3u  << 10)

/* Pin  102  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKIN2			(XLLP_BIT_12)
#define XLLP_GPIO_AF_BIT_KP_MKIN2_MASK		(0x3u  << 12)

/* Pin  103  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKOUT0			(0x2u  << 14)
#define XLLP_GPIO_AF_BIT_KP_MKOUT0_MASK		(0x3u  << 14)

/* Pin  104  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKOUT1			(0x2u  << 16)
#define XLLP_GPIO_AF_BIT_KP_MKOUT1_MASK		(0x3u  << 16)

/* Pin  105  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKOUT2			(0x2u  << 18)
#define XLLP_GPIO_AF_BIT_KP_MKOUT2_MASK		(0x3u  << 18)

/* Pin  106  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKOUT3			(0x2u  << 20)
#define XLLP_GPIO_AF_BIT_KP_MKOUT3_MASK		(0x3u  << 20)

/* Pin  107  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKOUT4			(0x2u  << 22)
#define XLLP_GPIO_AF_BIT_KP_MKOUT4_MASK		(0x3u  << 22)

/* Pin  108  alternate functions */
#define XLLP_GPIO_AF_BIT_KP_MKOUT5			(0x2u  << 24)
#define XLLP_GPIO_AF_BIT_KP_MKOUT_MASK		(0x3u  << 24)

/* Pin  109  alternate functions */
#define XLLP_GPIO_AF_BIT_MSSDIO				(0x2u  << 26)
#define XLLP_GPIO_AF_BIT_MSSDIO_MASK		(0x3u  << 26)
#define XLLP_GPIO_AF_BIT_MMDAT1				(XLLP_BIT_26)
#define XLLP_GPIO_AF_BIT_MMDAT1_MASK		(0x3u  << 26)

/* Pin  110  alternate functions */
#define XLLP_GPIO_AF_BIT_MMDAT2				(XLLP_BIT_28)
#define XLLP_GPIO_AF_BIT_MMDAT2_MASK		(0x3u  << 28)

/* Pin  111  alternate functions */
#define XLLP_GPIO_AF_BIT_MMDAT3				(XLLP_BIT_30)
#define XLLP_GPIO_AF_BIT_MMDAT3_MASK		(0x3u  << 30)

/* Pin  112  alternate functions */
#define XLLP_GPIO_AF_BIT_MMCMD				(XLLP_BIT_0)
#define XLLP_GPIO_AF_BIT_MMCMD_MASK			(0x3u  << 0)
#define XLLP_GPIO_AF_BIT_MSINS				(0x2u  << 0)
#define XLLP_GPIO_AF_BIT_MSINS_MASK			(0x3u  << 0)

/* Pin  113  alternate functions */
#define XLLP_GPIO_AF_BIT_I2S_SYSCLK			(XLLP_BIT_2)
#define XLLP_GPIO_AF_BIT_I2S_SYSCLK_MASK	(0x3u  << 2)
#define XLLP_GPIO_AF_BIT_AC97_nRESET		(0x2u  << 2)
#define XLLP_GPIO_AF_BIT_AC97_nRESET_MASK	(0x3u  << 2)

/* Pin  114  alternate functions */
#define XLLP_GPIO_AF_BIT_UVS0				(0x2u  << 4)
#define XLLP_GPIO_AF_BIT_UVS0_MASK			(0x3u  << 4)

/* Pin  114  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_DD1			(XLLP_BIT_4)
#define XLLP_GPIO_AF_BIT_CIF_DD1_MASK		(0x3u  << 4)

/* Pin  115  alternate functions */
#define XLLP_GPIO_AF_BIT_U_EN				(XLLP_BIT_6)
#define XLLP_GPIO_AF_BIT_U_EN_MASK			(0x3u  << 6)

/* Pin  115  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_DD3			(0x2u  << 6)
#define XLLP_GPIO_AF_BIT_CIF_DD3_MASK		(0x3u  << 6)

/* Pin  116  alternate functions */
#define XLLP_GPIO_AF_BIT_U_DET				(0x3u  << 8)
#define XLLP_GPIO_AF_BIT_U_DET_MASK			(0x3u  << 8)

/* Pin  116  alternate functions */
#define XLLP_GPIO_AF_BIT_CIF_DD2			(XLLP_BIT_8)
#define XLLP_GPIO_AF_BIT_CIF_DD2_MASK		(0x3u  << 8)

/* Pin  117  alternate functions */
#define XLLP_GPIO_AF_BIT_SCL				(XLLP_BIT_10)
#define XLLP_GPIO_AF_BIT_SCL_MASK			(0x3u  << 10)

/* Pin  118  alternate functions */
#define XLLP_GPIO_AF_BIT_SDA				(XLLP_BIT_12)
#define XLLP_GPIO_AF_BIT_SDA_MASK			(0x3u  << 12)

/* Pin  119  alternate functions */
#define XLLP_GPIO_AF_BIT_USBHPWR2			(XLLP_BIT_14)
#define XLLP_GPIO_AF_BIT_USBHPWR2_MASK		(0x3u  << 14)

/* Pin  120  alternate functions */
#define XLLP_GPIO_AF_BIT_USBHPEN2			(0x2u  << 16)
#define XLLP_GPIO_AF_BIT_USBHPEN2_MASK		(0x3u  << 16)


/* END of alternate function values for each GPIO  -- in BIT-WISE */
/*=================================================================*/

/* GPIO Register Fields */

#define XLLP_GPIO_PIN_RESERVED_BITS  0xfe000000u
#define XLLP_GPIO_ALT_RESERVED_BITS  0xfffc0000u
#define XLLP_GPIO_ALT_FUNC_MASK      3u

/************************************************************************/
/* BEGIN of alternate function values for each GPIO*/
//
//      ~~~ALTERNATE FUNCTIONS~~~
//	

/* Pin  11  alternate functions */
#define XLLP_GPIO_AF_SSPRXD2			XLLP_GPIO_ALT_FN_2

/* Pin  12  alternate functions */
#define XLLP_GPIO_AF_CIF_DD7			XLLP_GPIO_ALT_FN_2

/* Pin  13  alternate functions */
#define XLLP_GPIO_AF_SSPTXD2			XLLP_GPIO_ALT_FN_1

/* Pin  14  alternate functions */
#define XLLP_GPIO_AF_L_VSYNC			XLLP_GPIO_ALT_FN_1

/* Pin  15  alternate functions */
#define XLLP_GPIO_AF_nCS1				XLLP_GPIO_ALT_FN_2

/* Pin  16  alternate functions */
#define XLLP_GPIO_AF_PWM_OUT0			XLLP_GPIO_ALT_FN_2

/* Pin  17  alternate functions */
#define XLLP_GPIO_AF_PWM_OUT1			XLLP_GPIO_ALT_FN_2

/* Pin  17  alternate functions */
#define XLLP_GPIO_AF_CIF_DD6			XLLP_GPIO_ALT_FN_2

/* Pin  18  alternate functions */
#define XLLP_GPIO_AF_RDY				XLLP_GPIO_ALT_FN_1

/* Pin  19  alternate functions */
#define XLLP_GPIO_AF_L_CS				XLLP_GPIO_ALT_FN_2

/* Pin  20  alternate functions */
#define XLLP_GPIO_AF_MBREQ				XLLP_GPIO_ALT_FN_2

/* Pin  21  alternate functions */
#define XLLP_GPIO_AF_MBGNT				XLLP_GPIO_ALT_FN_3

/* Pin  22  alternate functions */
#define XLLP_GPIO_AF_SSPCLK2			XLLP_GPIO_ALT_FN_3

/* Pin  23  alternate functions */
#define XLLP_GPIO_AF_CIF_MCLK			XLLP_GPIO_ALT_FN_1

/* Pin  24  alternate functions */
#define XLLP_GPIO_AF_CIF_FV				XLLP_GPIO_ALT_FN_1

/* Pin  25  alternate functions */
#define XLLP_GPIO_AF_CIF_LV				XLLP_GPIO_ALT_FN_1

/* Pin  26  alternate functions */
#define XLLP_GPIO_AF_CIF_PCLK			XLLP_GPIO_ALT_FN_2

/* Pin  27  alternate functions */
#define XLLP_GPIO_AF_CIF_DD0			XLLP_GPIO_ALT_FN_3

/* Pin  28  alternate functions */
#define XLLP_GPIO_AF_AC97_BITCLK		XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_I2SBITCLK_IN		XLLP_GPIO_ALT_FN_2
#define XLLP_GPIO_AF_I2SBITCLK_OUT		XLLP_GPIO_ALT_FN_1

/* Pin  29  alternate functions */
#define XLLP_GPIO_AF_AC97_SDATA_IN_0	XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_I2S_SDATA_IN		XLLP_GPIO_ALT_FN_2

/* Pin  30  alternate functions */
#define XLLP_GPIO_AF_I2S_SDATA_OUT		XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_AC97_SDATA_OUT		XLLP_GPIO_ALT_FN_2

/* Pin  31  alternate functions */
#define XLLP_GPIO_AF_I2S_SYNC			XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_AC97_SYNC			XLLP_GPIO_ALT_FN_2

/* Pin  32  alternate functions */
#define XLLP_GPIO_AF_MSSCLK				XLLP_GPIO_ALT_FN_1  
#define XLLP_GPIO_AF_MMCLK				XLLP_GPIO_ALT_FN_2 

/* Pin  33  alternate functions */
#define XLLP_GPIO_AF_nCS5				XLLP_GPIO_ALT_FN_2

/* Pin  34  alternate functions */
#define XLLP_GPIO_AF_FFRXD				XLLP_GPIO_ALT_FN_1

/* Pin  34  alternate functions */
#define XLLP_GPIO_AF_USB_P2_2			XLLP_GPIO_ALT_FN_1

/* Pin  35  alternate functions */
#define XLLP_GPIO_AF_FFCTS				XLLP_GPIO_ALT_FN_1

/* Pin  35  alternate functions */
#define XLLP_GPIO_AF_USB_P2_1			XLLP_GPIO_ALT_FN_1

/* Pin  36  alternate functions */
#define XLLP_GPIO_AF_FFDCD				XLLP_GPIO_ALT_FN_1

/* Pin  36  alternate functions */
#define XLLP_GPIO_AF_USB_P2_4			XLLP_GPIO_ALT_FN_1

/* Pin  36  alternate functions */
#define XLLP_GPIO_AF_SSPSCLK2			XLLP_GPIO_ALT_FN_2

/* Pin  37  alternate functions */
#define XLLP_GPIO_AF_FFDSR				XLLP_GPIO_ALT_FN_1

/* Pin  37  alternate functions */
#define XLLP_GPIO_AF_SSPSFRM2			XLLP_GPIO_ALT_FN_2

/* Pin  38  alternate functions */
#define XLLP_GPIO_AF_FFRI				XLLP_GPIO_ALT_FN_1

/* Pin  38  alternate functions */
#define XLLP_GPIO_AF_USB_P2_3			XLLP_GPIO_ALT_FN_3

/* Pin  38  alternate functions */
#define XLLP_GPIO_AF_SSPTXD2			XLLP_GPIO_ALT_FN_1

/* Pin  39  alternate functions */
#define XLLP_GPIO_AF_FFTXD				XLLP_GPIO_ALT_FN_2

/* Pin  39  alternate functions */
#define XLLP_GPIO_AF_USB_P2_6			XLLP_GPIO_ALT_FN_1

/* Pin  40  alternate functions */
#define XLLP_GPIO_AF_FFDTR				XLLP_GPIO_ALT_FN_2

/* Pin  40  alternate functions */
#define XLLP_GPIO_AF_USB_P2_5			XLLP_GPIO_ALT_FN_3

/* Pin  41  alternate functions */
#define XLLP_GPIO_AF_FFRTS				XLLP_GPIO_ALT_FN_2

/* Pin  42  alternate functions */
#define XLLP_GPIO_AF_BTRXD				XLLP_GPIO_ALT_FN_1

/* Pin  43  alternate functions */
#define XLLP_GPIO_AF_BTTXD				XLLP_GPIO_ALT_FN_2

/* Pin  44  alternate functions */
#define XLLP_GPIO_AF_BTCTS				XLLP_GPIO_ALT_FN_1

/* Pin  45  alternate functions */
#define XLLP_GPIO_AF_BTRTS				XLLP_GPIO_ALT_FN_2

/* Pin  46  alternate functions */
#define XLLP_GPIO_AF_ICP_RXD			XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_STD_RXD			XLLP_GPIO_ALT_FN_2

/* Pin  47  alternate functions */
#define XLLP_GPIO_AF_ICP_TXD			XLLP_GPIO_ALT_FN_2
#define XLLP_GPIO_AF_STD_TXD			XLLP_GPIO_ALT_FN_1

/* Pin  48  alternate functions */
#define XLLP_GPIO_AF_BB_OB_DAT1			XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_PCMCIA_nPOE		XLLP_GPIO_ALT_FN_2

/* Pin  49  alternate functions */
#define XLLP_GPIO_AF_nPWE				XLLP_GPIO_ALT_FN_2

/* Pin  50  alternate functions */
#define XLLP_GPIO_AF_BB_OB_DAT2			XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_PCMCIA_nPIOR		XLLP_GPIO_ALT_FN_2

/* Pin  51  alternate functions */
#define XLLP_GPIO_AF_BB_OB_DAT3			XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_PCMCIA_nPIOW		XLLP_GPIO_ALT_FN_2

/* Pin  52  alternate functions */
#define XLLP_GPIO_AF_BB_OB_CLK			XLLP_GPIO_ALT_FN_1

/* Pin  52  alternate functions */
#define XLLP_GPIO_AF_SSPSCLK3			XLLP_GPIO_ALT_FN_2

/* Pin  53  alternate functions */
#define XLLP_GPIO_AF_BB_OB_STB			XLLP_GPIO_ALT_FN_1

/* Pin  54  alternate functions */
#define XLLP_GPIO_AF_BB_OB_WAIT			XLLP_GPIO_ALT_FN_2
#define XLLP_GPIO_AF_PCMCIA_nPCE2		XLLP_GPIO_ALT_FN_2

/* Pin  55  alternate functions */
#define XLLP_GPIO_AF_BB_IB_DAT1			XLLP_GPIO_ALT_FN_2
#define XLLP_GPIO_AF_PCMCIA_nPREG		XLLP_GPIO_ALT_FN_2

/* Pin  56  alternate functions */
#define XLLP_GPIO_AF_PCMCIA_nPWAIT		XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_BB_IB_DAT2			XLLP_GPIO_ALT_FN_2

/* Pin  57  alternate functions */
#define XLLP_GPIO_AF_PCMCIA_nIOIS16		XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_BB_IB_DAT3			XLLP_GPIO_ALT_FN_2

/* Pin  58  alternate functions */
#define XLLP_GPIO_AF_L_DD0				XLLP_GPIO_ALT_FN_2

/* Pin  59  alternate functions */
#define XLLP_GPIO_AF_L_DD1				XLLP_GPIO_ALT_FN_2

/* Pin  60  alternate functions */
#define XLLP_GPIO_AF_L_DD2				XLLP_GPIO_ALT_FN_2

/* Pin  61  alternate functions */
#define XLLP_GPIO_AF_L_DD3				XLLP_GPIO_ALT_FN_2

/* Pin  62  alternate functions */
#define XLLP_GPIO_AF_L_DD4				XLLP_GPIO_ALT_FN_2

/* Pin  63  alternate functions */
#define XLLP_GPIO_AF_L_DD5				XLLP_GPIO_ALT_FN_2

/* Pin  64  alternate functions */
#define XLLP_GPIO_AF_L_DD6				XLLP_GPIO_ALT_FN_2

/* Pin  65  alternate functions */
#define XLLP_GPIO_AF_L_DD7				XLLP_GPIO_ALT_FN_2

/* Pin  66  alternate functions */
#define XLLP_GPIO_AF_L_DD8				XLLP_GPIO_ALT_FN_2

/* Pin  67  alternate functions */
#define XLLP_GPIO_AF_L_DD9				XLLP_GPIO_ALT_FN_2

/* Pin  68  alternate functions */
#define XLLP_GPIO_AF_L_DD10				XLLP_GPIO_ALT_FN_2

/* Pin  69  alternate functions */
#define XLLP_GPIO_AF_L_DD11				XLLP_GPIO_ALT_FN_2

/* Pin  70  alternate functions */
#define XLLP_GPIO_AF_L_DD12				XLLP_GPIO_ALT_FN_2

/* Pin  71  alternate functions */
#define XLLP_GPIO_AF_L_DD13				XLLP_GPIO_ALT_FN_2

/* Pin  72  alternate functions */
#define XLLP_GPIO_AF_L_DD14				XLLP_GPIO_ALT_FN_2

/* Pin  73  alternate functions */
#define XLLP_GPIO_AF_L_DD15				XLLP_GPIO_ALT_FN_2

/* Pin  74  alternate functions */
#define XLLP_GPIO_AF_L_FCLK_RD			XLLP_GPIO_ALT_FN_2

/* Pin  75  alternate functions */
#define XLLP_GPIO_AF_L_LCLK_A0			XLLP_GPIO_ALT_FN_2

/* Pin  76  alternate functions */
#define XLLP_GPIO_AF_L_PCLK_WR			XLLP_GPIO_ALT_FN_2

/* Pin  77  alternate functions */
#define XLLP_GPIO_AF_L_BIAS				XLLP_GPIO_ALT_FN_2

/* Pin  78  alternate functions */
#define XLLP_GPIO_AF_nCS2				XLLP_GPIO_ALT_FN_2

/* Pin  79  alternate functions */
#define XLLP_GPIO_AF_PCMCIA_PSKTSEL		XLLP_GPIO_ALT_FN_1

/* Pin  80  alternate functions */
#define XLLP_GPIO_AF_nCS4				XLLP_GPIO_ALT_FN_2

/* Pin  81  alternate functions */
#define XLLP_GPIO_AF_BB_OB_DAT0			XLLP_GPIO_ALT_FN_2

/* Pin  81  alternate functions */
#define XLLP_GPIO_AF_SSPTXD3			XLLP_GPIO_ALT_FN_1

/* Pin  82  alternate functions */
#define XLLP_GPIO_AF_BB_IB_DAT0			XLLP_GPIO_ALT_FN_2

/* Pin  82  alternate functions */
#define XLLP_GPIO_AF_SSPRXD3			XLLP_GPIO_ALT_FN_1

/* Pin  83  alternate functions */
#define XLLP_GPIO_AF_BB_IB_CLK			XLLP_GPIO_ALT_FN_2

/* Pin  83  alternate functions */
#define XLLP_GPIO_AF_SSPSFRM3			XLLP_GPIO_ALT_FN_1

/* Pin  84  alternate functions */
#define XLLP_GPIO_AF_BB_IB_STB			XLLP_GPIO_ALT_FN_2

/* Pin  85  alternate functions */
#define XLLP_GPIO_AF_PCMCIA_nPCE1		XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_BB_IB_WAIT			XLLP_GPIO_ALT_FN_2

/* Pin  86  alternate functions */
#define XLLP_GPIO_AF_L_DD16				XLLP_GPIO_ALT_FN_2

/* Pin  86  alternate functions */
#define XLLP_GPIO_AF_PCMCIA_nPCE1_1		XLLP_GPIO_ALT_FN_1

/* Pin  87  alternate functions */
#define XLLP_GPIO_AF_L_DD17				XLLP_GPIO_ALT_FN_2

/* Pin  87  alternate functions */
#define XLLP_GPIO_AF_PCMCIA_nPCE1_2		XLLP_GPIO_ALT_FN_1

/* Pin  88  alternate functions */
#define XLLP_GPIO_AF_USBHPWR0			XLLP_GPIO_ALT_FN_1

/* Pin  88  alternate functions */
#define XLLP_GPIO_AF_SSPFRM2			XLLP_GPIO_ALT_FN_3

/* Pin  89  alternate functions */
#define XLLP_GPIO_AF_USBHPEN0			XLLP_GPIO_ALT_FN_2

/* Pin  90  alternate functions */
#define XLLP_GPIO_AF_URST				XLLP_GPIO_ALT_FN_2

/* Pin  90  alternate functions */
#define XLLP_GPIO_AF_CIF_DD4			XLLP_GPIO_ALT_FN_3

/* Pin  91  alternate functions */
#define XLLP_GPIO_AF_UCLK				XLLP_GPIO_ALT_FN_2

/* Pin  91  alternate functions */
#define XLLP_GPIO_AF_CIF_DD5			XLLP_GPIO_ALT_FN_3

/* Pin  92  alternate functions */
#define XLLP_GPIO_AF_MMDAT0				XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_MSBS				XLLP_GPIO_ALT_FN_2

/* Pin  93  alternate functions */
#define XLLP_GPIO_AF_KP_DKIN0			XLLP_GPIO_ALT_FN_1

/* Pin  94  alternate functions */
#define XLLP_GPIO_AF_KP_DKIN1			XLLP_GPIO_ALT_FN_1

/* Pin  95  alternate functions */
#define XLLP_GPIO_AF_KP_MKIN6			XLLP_GPIO_ALT_FN_3

/* Pin  96  alternate functions */
#define XLLP_GPIO_AF_KP_MKOUT6			XLLP_GPIO_ALT_FN_3

/* Pin  97  alternate functions */
#define XLLP_GPIO_AF_KP_MKIN3			XLLP_GPIO_ALT_FN_3

/* Pin  98  alternate functions */
#define XLLP_GPIO_AF_KP_MKIN4			XLLP_GPIO_ALT_FN_3

/* Pin  99  alternate functions */
#define XLLP_GPIO_AF_KP_MKIN5			XLLP_GPIO_ALT_FN_3

/* Pin  100  alternate functions */
#define XLLP_GPIO_AF_KP_MKIN0			XLLP_GPIO_ALT_FN_1

/* Pin  101  alternate functions */
#define XLLP_GPIO_AF_KP_MKIN1			XLLP_GPIO_ALT_FN_1

/* Pin  102  alternate functions */
#define XLLP_GPIO_AF_KP_MKIN2			XLLP_GPIO_ALT_FN_1

/* Pin  103  alternate functions */
#define XLLP_GPIO_AF_KP_MKOUT0			XLLP_GPIO_ALT_FN_2

/* Pin  104  alternate functions */
#define XLLP_GPIO_AF_KP_MKOUT1			XLLP_GPIO_ALT_FN_2

/* Pin  105  alternate functions */
#define XLLP_GPIO_AF_KP_MKOUT2			XLLP_GPIO_ALT_FN_2

/* Pin  106  alternate functions */
#define XLLP_GPIO_AF_KP_MKOUT3			XLLP_GPIO_ALT_FN_2

/* Pin  107  alternate functions */
#define XLLP_GPIO_AF_KP_MKOUT4			XLLP_GPIO_ALT_FN_2

/* Pin  108  alternate functions */
#define XLLP_GPIO_AF_KP_MKOUT5			XLLP_GPIO_ALT_FN_2

/* Pin  109  alternate functions */
#define XLLP_GPIO_AF_MSSDIO				XLLP_GPIO_ALT_FN_2
#define XLLP_GPIO_AF_MMDAT1				XLLP_GPIO_ALT_FN_1

/* Pin  110  alternate functions */
#define XLLP_GPIO_AF_MMDAT2				XLLP_GPIO_ALT_FN_1

/* Pin  111  alternate functions */
#define XLLP_GPIO_AF_MMDAT3				XLLP_GPIO_ALT_FN_1

/* Pin  112  alternate functions */
#define XLLP_GPIO_AF_MMCMD				XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_MSINS				XLLP_GPIO_ALT_FN_2

/* Pin  113  alternate functions */
#define XLLP_GPIO_AF_I2S_SYSCLK			XLLP_GPIO_ALT_FN_1
#define XLLP_GPIO_AF_AC97_nRESET		XLLP_GPIO_ALT_FN_2

/* Pin  114  alternate functions */
#define XLLP_GPIO_AF_UVS0				XLLP_GPIO_ALT_FN_2

/* Pin  114  alternate functions */
#define XLLP_GPIO_AF_CIF_DD1			XLLP_GPIO_ALT_FN_1

/* Pin  115  alternate functions */
#define XLLP_GPIO_AF_U_EN				XLLP_GPIO_ALT_FN_1

/* Pin  115  alternate functions */
#define XLLP_GPIO_AF_CIF_DD3			XLLP_GPIO_ALT_FN_2

/* Pin  116  alternate functions */
#define XLLP_GPIO_AF_U_DET				XLLP_GPIO_ALT_FN_3

/* Pin  116  alternate functions */
#define XLLP_GPIO_AF_CIF_DD2			XLLP_GPIO_ALT_FN_1

/* Pin  117  alternate functions */
#define XLLP_GPIO_AF_SCL				XLLP_GPIO_ALT_FN_1

/* Pin  118  alternate functions */
#define XLLP_GPIO_AF_SDA				XLLP_GPIO_ALT_FN_1

/* Pin  119  alternate functions */
#define XLLP_GPIO_AF_USBHPWR2			XLLP_GPIO_ALT_FN_1

/* Pin  120  alternate functions */
#define XLLP_GPIO_AF_USBHPEN2			XLLP_GPIO_ALT_FN_2


/* END of alternate function values for each GPIO */
/************************************************************************/
// GPIO register reserved and valid bit masks

#define XLLP_GPIO_GPLR0_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPLR0_VLD_MSK (~(XLLP_GPIO_GPLR0_RESERVED_BITS))
#define XLLP_GPIO_GPLR1_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPLR1_VLD_MSK (~(XLLP_GPIO_GPLR1_RESERVED_BITS))
#define XLLP_GPIO_GPLR2_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPLR2_VLD_MSK (~(XLLP_GPIO_GPLR2_RESERVED_BITS))
#define XLLP_GPIO_GPLR3_RESERVED_BITS 0xFE000000u
#define XLLP_GPIO_GPLR3_VLD_MSK (~(XLLP_GPIO_GPLR3_RESERVED_BITS))

#define XLLP_GPIO_GPDR0_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPDR0_VLD_MSK (~(XLLP_GPIO_GPDR0_RESERVED_BITS))
#define XLLP_GPIO_GPDR1_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPDR1_VLD_MSK (~(XLLP_GPIO_GPDR1_RESERVED_BITS))
#define XLLP_GPIO_GPDR2_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPDR2_VLD_MSK (~(XLLP_GPIO_GPDR2_RESERVED_BITS))
#define XLLP_GPIO_GPDR3_RESERVED_BITS 0xFE000000u
#define XLLP_GPIO_GPDR3_VLD_MSK (~(XLLP_GPIO_GPDR3_RESERVED_BITS))

#define XLLP_GPIO_GPSR0_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPSR0_VLD_MSK (~(XLLP_GPIO_GPSR0_RESERVED_BITS))
#define XLLP_GPIO_GPSR1_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPSR1_VLD_MSK (~(XLLP_GPIO_GPSR1_RESERVED_BITS))
#define XLLP_GPIO_GPSR2_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPSR2_VLD_MSK (~(XLLP_GPIO_GPSR2_RESERVED_BITS))
#define XLLP_GPIO_GPSR3_RESERVED_BITS 0xFE000000u
#define XLLP_GPIO_GPSR3_VLD_MSK (~(XLLP_GPIO_GPSR3_RESERVED_BITS))

#define XLLP_GPIO_GPCR0_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPCR0_VLD_MSK (~(XLLP_GPIO_GPCR0_RESERVED_BITS))
#define XLLP_GPIO_GPCR1_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPCR1_VLD_MSK (~(XLLP_GPIO_GPCR1_RESERVED_BITS))
#define XLLP_GPIO_GPCR2_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GPCR2_VLD_MSK (~(XLLP_GPIO_GPCR2_RESERVED_BITS))
#define XLLP_GPIO_GPCR3_RESERVED_BITS 0xFE000000u
#define XLLP_GPIO_GPCR3_VLD_MSK (~(XLLP_GPIO_GPCR3_RESERVED_BITS))

#define XLLP_GPIO_GRER0_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GRER0_VLD_MSK (~(XLLP_GPIO_GRER0_RESERVED_BITS))
#define XLLP_GPIO_GRER1_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GRER1_VLD_MSK (~(XLLP_GPIO_GRER1_RESERVED_BITS))
#define XLLP_GPIO_GRER2_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GRER2_VLD_MSK (~(XLLP_GPIO_GRER2_RESERVED_BITS))
#define XLLP_GPIO_GRER3_RESERVED_BITS 0xFE000000u
#define XLLP_GPIO_GRER3_VLD_MSK (~(XLLP_GPIO_GRER3_RESERVED_BITS))

#define XLLP_GPIO_GFER0_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GFER0_VLD_MSK (~(XLLP_GPIO_GFER0_RESERVED_BITS))
#define XLLP_GPIO_GFER1_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GFER1_VLD_MSK (~(XLLP_GPIO_GFER1_RESERVED_BITS))
#define XLLP_GPIO_GFER2_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GFER2_VLD_MSK (~(XLLP_GPIO_GFER2_RESERVED_BITS))
#define XLLP_GPIO_GFER3_RESERVED_BITS 0xFE000000u
#define XLLP_GPIO_GFER3_VLD_MSK (~(XLLP_GPIO_GFER3_RESERVED_BITS))

#define XLLP_GPIO_GEDR0_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GEDR0_VLD_MSK (~(XLLP_GPIO_GEDR0_RESERVED_BITS))
#define XLLP_GPIO_GEDR1_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GEDR1_VLD_MSK (~(XLLP_GPIO_GEDR1_RESERVED_BITS))
#define XLLP_GPIO_GEDR2_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GEDR2_VLD_MSK (~(XLLP_GPIO_GEDR2_RESERVED_BITS))
#define XLLP_GPIO_GEDR3_RESERVED_BITS 0xFE000000u
#define XLLP_GPIO_GEDR3_VLD_MSK (~(XLLP_GPIO_GEDR3_RESERVED_BITS))

#define XLLP_GPIO_GAFR0_L_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GAFR0_L_VLD_MSK (~(XLLP_GPIO_GAFR0_L_RESERVED_BITS))
#define XLLP_GPIO_GAFR0_U_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GAFR0_U_VLD_MSK (~(XLLP_GPIO_GAFR0_U_RESERVED_BITS))
#define XLLP_GPIO_GAFR1_L_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GAFR1_L_VLD_MSK (~(XLLP_GPIO_GAFR1_L_RESERVED_BITS))
#define XLLP_GPIO_GAFR1_U_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GAFR1_U_VLD_MSK (~(XLLP_GPIO_GAFR1_U_RESERVED_BITS))
#define XLLP_GPIO_GAFR2_L_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GAFR2_L_VLD_MSK (~(XLLP_GPIO_GAFR2_L_RESERVED_BITS))
#define XLLP_GPIO_GAFR2_U_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GAFR2_U_VLD_MSK (~(XLLP_GPIO_GAFR2_U_RESERVED_BITS))
#define XLLP_GPIO_GAFR3_L_RESERVED_BITS 0x00000000u
#define XLLP_GPIO_GAFR3_L_VLD_MSK (~(XLLP_GPIO_GAFR3_L_RESERVED_BITS))
#define XLLP_GPIO_GAFR3_U_RESERVED_BITS 0xFFFC0000u
#define XLLP_GPIO_GAFR3_U_VLD_MSK (~(XLLP_GPIO_GAFR3_U_RESERVED_BITS))

// END of GPIO register reserved and valid bit masks
/************************************************************************/

/* XLLP GPIO API typedefs */
/* Bulverde has four GPIO register banks */
/* For non-alternate function registers */

typedef enum 
{
    XLLP_GPIO_BANK_0,
    XLLP_GPIO_BANK_1,
    XLLP_GPIO_BANK_2,
    XLLP_GPIO_BANK_3
} XLLP_GPIO_REGISTER_BANK_T;  
      

/* Bulverde has eight half GPIO register banks */
/* For only alternate function registers */
typedef enum 
{
    XLLP_GPIO_HALF_BANK_0_L,
	XLLP_GPIO_HALF_BANK_0_U,
	XLLP_GPIO_HALF_BANK_1_L,
	XLLP_GPIO_HALF_BANK_1_U,
	XLLP_GPIO_HALF_BANK_2_L,
	XLLP_GPIO_HALF_BANK_2_U,
	XLLP_GPIO_HALF_BANK_3_L,
	XLLP_GPIO_HALF_BANK_3_U
} XLLP_GPIO_REGISTER_HALF_BANK_T;        


typedef enum
{
    XLLP_GPIO_ALT_FN_0 = 0x0,
    XLLP_GPIO_ALT_FN_1 = 0x1,
    XLLP_GPIO_ALT_FN_2 = 0x2,
    XLLP_GPIO_ALT_FN_3 = 0x3
} XLLP_GPIO_ALT_FUNC_T;

#ifdef __cplusplus
extern "C" {
#endif

XLLP_UINT32_T XllpGpioGetState (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin);
XLLP_UINT32_T XllpGpioGetDirection (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin);
void XllpGpioSetDirectionIn (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
void XllpGpioSetDirectionOut (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
void XllpGpioSetOutputState1 (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
void XllpGpioSetOutput0 (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
XLLP_UINT32_T XllpGpioGetRisingDetectEnable (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin);
void XllpGpioSetRisingDetectDisable (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
void XllpGpioSetRisingDetectEnable (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
XLLP_UINT32_T XllpGpioGetFallingDetectEnable (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin);
void XllpGpioSetFallingEdgeDetectDisable (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
void XllpGpioSetFallingEdgeDetectEnable (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
XLLP_UINT32_T XllpGpioGetEdgeDetectStatus (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin);
void XllpGpioClearEdgeDetectStatus (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
XLLP_UINT32_T XllpGpioGetAlternateFn (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPin);
void XllpGpioSetAlternateFn (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[], XLLP_UINT32_T aAfValueArray[]);
void XllpGpioClearAlternateFn (P_XLLP_GPIO_T pGPIO, XLLP_UINT32_T aGpioPinArray[]);
/* XLLP GPIO Function Prototypes - None */

#ifdef __cplusplus
};
#endif


#endif //__GPIO_H__

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
**  FILENAME:       xllp_ac97acodec.h
**
**  PURPOSE: contains all AC97 specific macros, typedefs, and prototypes.
**           Declares no storage.
**                  
**
******************************************************************************/
#ifndef __XLLP_AC97ACODEC_H__
#define __XLLP_AC97ACODEC_H__

#include "xllp_defs.h"
#include "xllp_acodec.h"


/*
*******************************************************************************
    AC'97 Controller Registers Structure and Bit Definition
*******************************************************************************
*/

#define XLLP_AC97_CODEC_REGS_NUM        0x40

typedef struct
{  //   Register symbol     // Usage

    XLLP_VUINT32_T POCR;           // PCM Out Control Register
    XLLP_VUINT32_T PICR;           // PCM In Control Register
    XLLP_VUINT32_T MCCR;           // Mic In Control Register
    XLLP_VUINT32_T GCR;            // Global Control Register
    XLLP_VUINT32_T POSR;           // PCM Out Status Register
    XLLP_VUINT32_T PISR;           // PCM In Status Register
    XLLP_VUINT32_T MCSR;           // Mic In Status Register
    XLLP_VUINT32_T GSR;            // Global Status Register
    XLLP_VUINT32_T CAR;            // CODEC Access Register
    XLLP_VUINT32_T reserved1 [0x7];      // 0x4050-0024 through 0x4050-003C
    XLLP_VUINT32_T PCDR;           // PCM FIFO Data Register
    XLLP_VUINT32_T reserved2 [0x7];      // 0x4050-0044 through 0x4050-005C
    XLLP_VUINT32_T MCDR;           // Mic-in FIFO Data Register
    XLLP_VUINT32_T reserved3 [0x27];   // 0x4050-0064 through 0x4050-00FC
    XLLP_VUINT32_T MOCR;           // MODEM Out Control Register
    XLLP_VUINT32_T reserved4;
    XLLP_VUINT32_T MICR;           // MODEM In Control Register
    XLLP_VUINT32_T reserved5;
    XLLP_VUINT32_T MOSR;           // MODEM Out Status Register
    XLLP_VUINT32_T reserved6;
    XLLP_VUINT32_T MISR;           // MODEM In Status Register
    XLLP_VUINT32_T reserved7 [0x9];      // 0x4050-011C through 0x4050-013C
    XLLP_VUINT32_T MODR;           // MODEM FIFO Data Register
    XLLP_VUINT32_T reserved8 [0x2F];   // 0x4050-0144 through 0x4050-01FC
                            // Primary Audio CODEC registers access
    XLLP_VUINT32_T CodecRegsPrimaryAud   [XLLP_AC97_CODEC_REGS_NUM];
                            // Secondary Audio CODEC registers access
    XLLP_VUINT32_T CodecRegsSecondaryAud [XLLP_AC97_CODEC_REGS_NUM];
                            // Primary MODEM CODEC registers access
    XLLP_VUINT32_T CodecRegsPrimaryMdm   [XLLP_AC97_CODEC_REGS_NUM];
                            // Secondary MODEM CODEC registers access
    XLLP_VUINT32_T CodecRegsSecondaryMdm [XLLP_AC97_CODEC_REGS_NUM];

}  XLLP_AC97ACODEC_T, *P_XLLP_AC97ACODEC_T ;


/* Constants for the Global Control Register and Global Status Register */

// AC97 Global Control Register bit mask constants

#define XLLP_AC97_GCR_GIE_MSK          (1u << 0 )
#define XLLP_AC97_GCR_COLD_RESET_MSK   (1u << 1 )
#define XLLP_AC97_GCR_WARM_RESET_MSK   (1u << 2 )
#define XLLP_AC97_GCR_LINK_OFF_MSK     (1u << 3 )
#define XLLP_AC97_GCR_PCRSM_IEN_MSK    (1u << 4 )
#define XLLP_AC97_GCR_SCRSM_IEN_MSK    (1u << 5 )
#define XLLP_AC97_GCR_PCRDY_IEN_MSK    (1u << 8 )
#define XLLP_AC97_GCR_SCRDY_IEN_MSK    (1u << 9 )
#define XLLP_AC97_GCR_SDONE_IE_MSK     (1u << 18)
#define XLLP_AC97_GCR_CDONE_IE_MSK     (1u << 19)
#define XLLP_AC97_GCR_nDMAEN_MSK       (1u << 24)

// Global Status Register bit mask constants

#define XLLP_AC97_GSR_GSCI_MSK       (1u << 0 )
#define XLLP_AC97_GSR_MIINT_MSK      (1u << 1 )
#define XLLP_AC97_GSR_MOINT_MSK      (1u << 2 )
#define XLLP_AC97_GSR_ACOFFD_MSK     (1u << 3 )
#define XLLP_AC97_GSR_PIINT_MSK      (1u << 5 )
#define XLLP_AC97_GSR_POINT_MSK      (1u << 6 )
#define XLLP_AC97_GSR_MINT_MSK       (1u << 7 )
#define XLLP_AC97_GSR_PCRDY_MSK      (1u << 8 )
#define XLLP_AC97_GSR_SCRDY_MSK      (1u << 9 )
#define XLLP_AC97_GSR_PCRSM_MSK      (1u << 10)
#define XLLP_AC97_GSR_SCRSM_MSK      (1u << 11)
#define XLLP_AC97_GSR_SLT12_BITS_MSK (7u << 12)
#define XLLP_AC97_GSR_RCS_ERR_MSK    (1u << 15)
#define XLLP_AC97_GSR_SDONE_MSK      (1u << 18)
#define XLLP_AC97_GSR_CDONE_MSK      (1u << 19)


// Bit mask and values for CAIP bit in CAR register.
#define XLLP_AC97_CAR_CAIP_MSK       (0x1<<0)
#define XLLP_AC97_CAR_CAIP_LOCKED    (0x1<<0)
#define XLLP_AC97_CAR_CAIP_CLEAR     (0<<0)

/* Constants for FIFO status reporting and control */

// One bit location is used to report FIFO error conditions and clear 
//  interrupts on those conditions in the various non-global status registers.  

// XLLP_AC97_FIFOSTAT_FIFOE is used in:
                                                // POSR
                                                // PISR
                                                // MCSR
                                                // MOSR
                                                // MISR

#define XLLP_AC97_FIFOSTAT_FIFOE  (1u << 4)
#define XLLP_AC97_FIFOSTAT_EOC	  (1u << 3)
#define XLLP_AC97_FIFOSTAT_FSR	  (1u << 2)

// A different bit location is used to enable or disable interrupts based on 
//  FIFO error conditions in the various non-global control registers.  

// XLLP_AC97_FIFOCTRL_FEIE is used in:
                                                // POCR
                                                // PICR
                                                // MCCR
                                                // MOCR
                                                // MICR

#define XLLP_AC97_FIFOCTRL_FEIE  (1u << 3)
#define XLLP_AC97_FIFOCTRL_FSRIE (1u << 1)

/*
*******************************************************************************
    AC'97 Codec Registers Location and Bit Definition
*******************************************************************************
*/

/* */

    // Includes symbolic values for certain proprietary register asssignments
    //   in AC'97 devices that might be used with XLLP_AC97.

    // Valid for subset of R 2.1 specification.
    // Leading "e" in comment means it is an "expanded" register definition as
    //  found in one or more of the Appendices A-D of the R 2.1 specification.
    //  Appendix identifier will immediately follow the "e", such as "eA"
    // R/O indicates read-only
    // Registers not supported by the assumed controller will be commented out.

#define    XLLP_AC97_CR_RESET_ID             0x00  // RESET CODEC TO DEFAULT, get ID info
#define    XLLP_AC97_CR_MASTER_VOLUME        0x02  // LINE OUT VOLUME
#define    XLLP_AC97_CR_HEADPHONE_VOLUME     0x04  //
#define    XLLP_AC97_CR_MASTER_VOLUME_MONO   0x06  // 
#define    XLLP_AC97_CR_MASTER_TONE_R_L      0x08  // 
#define    XLLP_AC97_CR_PC_BEEP_VOLUME       0x0A  // 
#define    XLLP_AC97_CR_PHONE_VOLUME         0x0C  // 
#define    XLLP_AC97_CR_MIC_VOLUME           0x0E  //   MICROPHONE VOLUME/ AGC
#define    XLLP_AC97_CR_LINE_IN_VOLUME       0x10  //   LINE IN VOLUME
#define    XLLP_AC97_CR_CD_VOLUME            0x12  // 
#define    XLLP_AC97_CR_VIDEO_VOLUME         0x14  // 
#define    XLLP_AC97_CR_AUX_VOLUME           0x16  // 
#define    XLLP_AC97_CR_PCM_OUT_VOLUME       0x18  //
#define    XLLP_AC97_CR_RECORD_SELECT        0x1A  //   SELECT LINE IN OR MICROPHONE
#define    XLLP_AC97_CR_RECORD_GAIN          0x1C  //
#define    XLLP_AC97_CR_RECORD_GAIN_MIC      0x1E  //
#define    XLLP_AC97_CR_GENERAL_PURPOSE      0x20  //
#define    XLLP_AC97_CR_CONTROL_3D           0x22  //
#define    XLLP_AC97_CR_POWERDOWN_CTRL_STAT  0x26  //   POWER MANAGEMENT
#define    XLLP_AC97_CR_E_AUDIO_ID           0x28  // eA Extended audio sprt info, R/O
#define    XLLP_AC97_CR_E_AUDIO_CTRL_STAT    0x2A  // eA Extended audio stat + control

//
// Audio Sample Rate Control Registers, 0x2C - 0x34
//
                                           // eA PCM Front DAC rate control
#define    XLLP_AC97_CR_E_ASR_PCM_FRNT_DAC_RT 0x2C  //  (output slots 3, 4, 6)
#define    XLLP_AC97_CR_E_ASR_PCM_LR_ADC_RT   0x32  // eA PCM L+R ADC rate control (3+4)
#define    XLLP_AC97_CR_E_ASR_MIC_ADC_RT      0x34  // eA PCM Mic ADC rate control (5)


#define    XLLP_AC97_CR_E_MDM_GPIO_PIN_STAT     0x54
//
// 5Ah-7Ah: Vendor Reserved
//
//
// 7Ch-7Eh: Vendor ID registers.  Optional but standardized for Plug'n'Play
//
#define    XLLP_AC97_CR_VENDOR_ID1         0x7C  
#define    XLLP_AC97_CR_VENDOR_ID2         0x7E  

#define    XLLP_AC97_CR_MAX               XLLP_AC97_CR_VENDOR_ID2

#define    XLLP_AC97_CR_END_OF_LIST       (XLLP_AC97_CR_MAX +  2)



/* Other Constants */

// For accessing the Codec mixer registers, each increment of one 32-bit word
//  in processor space increments the addressed mixer register by two.
// This does not cause any ambiguities because only even mixer register 
//  addresses are currently supported (AC '97 spec, R 2.2)
#define XLLP_AC97_CODEC_REGS_PER_WORD   	2

/* Default timeout and holdtime settings */

// timeout in reading and writing codec registers through AC link
#define XLLP_AC97_RW_TIMEOUT_DEF		200	//unit is us

// timeout in waiting for codec's ready signal during setup process
#define XLLP_AC97_SETUP_TIMEOUT_DEF		500	//unit is us

// timeout in waiting for locking the link successfully 
#define XLLP_AC97_LOCK_TIMEOUT_DEF   		300	//unit is us

// timeout in shutting down the link
#define XLLP_AC97_LINKOFF_TIMEOUT_DEF   	500	//unit is us

// holdtime for keeping nReset signal active(low) in AC link
#define XLLP_AC97_COLD_HOLDTIME			100	//unit is us

/*
*******************************************************************************
  XLLP AC97 data structure used in function interface  
*******************************************************************************
*/

typedef enum
{
    XLLP_AC97_ACODEC_PRIMARY = 0,
    XLLP_AC97_ACODEC_SECONDARY = 1
} XLLP_AC97_ACODEC_SEL_T ; 


typedef struct
{
    XLLP_BOOL_T  codecReady ;
} XLLP_AC97_ACODEC_STAT_T ;




/*
*******************************************************************************
  XLLP ACODEC AC97 Functions 
*******************************************************************************
*/

extern XLLP_ACODEC_ERROR_T XllpAc97ACodecRead  (XLLP_ACODEC_CONTEXT_T *pDevContext, XLLP_UINT16_T regAddr,  XLLP_UINT16_T *pRegValue);
extern XLLP_ACODEC_ERROR_T XllpAc97ACodecWrite (XLLP_ACODEC_CONTEXT_T *pDevContext, XLLP_UINT16_T regAddr, XLLP_UINT16_T regValue);
extern XLLP_ACODEC_ERROR_T XllpAc97ACodecInit (XLLP_ACODEC_CONTEXT_T *pDevContext);
extern XLLP_ACODEC_ERROR_T XllpAc97ACodecDeInit (XLLP_ACODEC_CONTEXT_T *pDevContext);

#endif //__XLLP_AC97ACODEC_H__
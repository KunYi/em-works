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
**  FILENAME:       xllp_ac97.h
**
**  PURPOSE: contains all AC97 specific macros, typedefs, and prototypes.
**           Declares no storage.
**                  
**
******************************************************************************/

#ifndef __XLLP_AC97_H__
#define __XLLP_AC97_H__

#include "xllp_defs.h"
#include "xllp_gpio.h"
#include "xllp_clkmgr.h"
#include "xllp_intc.h"
#include "xllp_ost.h"



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

}  XLLP_AC97_T, *P_XLLP_AC97_T ;


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

#define    XLLP_AC97_CR_RESET_ID        0x00  // RESET CODEC TO DEFAULT, get ID info
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
//#define  XLLP_AC97_CR_RESERVED_0x24        0x24
#define    XLLP_AC97_CR_POWERDOWN_CTRL_STAT  0x26  //   POWER MANAGEMENT
#define    XLLP_AC97_CR_E_AUDIO_ID           0x28  // eA Extended audio sprt info, R/O
#define    XLLP_AC97_CR_E_AUDIO_CTRL_STAT    0x2A  // eA Extended audio stat + control

//
// Audio Sample Rate Control Registers, 0x2C - 0x34
//
                                           // eA PCM Front DAC rate control
#define    XLLP_AC97_CR_E_ASR_PCM_FRNT_DAC_RT 0x2C  //  (output slots 3, 4, 6)
//#define  XLLP_AC97_CR_E_ASR_PCM_SURR_DAC_RT 0x2E  // eA PCM Surround DAC rate control
//#define  XLLP_AC97_CR_E_ASR_PCM_LFE_DAC_RT  0x30  // eA PCM Surround DAC rate control
#define    XLLP_AC97_CR_E_ASR_PCM_LR_ADC_RT   0x32  // eA PCM L+R ADC rate control (3+4)
#define    XLLP_AC97_CR_E_ASR_MIC_ADC_RT      0x34  // eA PCM Mic ADC rate control (5)

//  0x36 - 0x38: eA Control registers for the four optional PCM channels
//                 (LFE, CNT, LSR, RSR)
//#define  XLLP_AC97_CR_E_6CH_VOL_C_LFE       0x36  // eA
//#define  XLLP_AC97_CR_E_6CH_VOL_LR_SURR     0x38  // eA
//#define  XLLP_AC97_CR_E_RESERVED_0x3A       0x3A

//
// Rev 2.0 Modem registers
//

//#define XLLP_AC97_CR_E_MDEM_ID                0x3C  // eB
//#define XLLP_AC97_CR_E_MDM_CTRL_STAT          0x3E  // eB
//#define XLLP_AC97_CR_E_MDM_LN1_DAC_ADC_RT     0x40  // eB
//#define XLLP_AC97_CR_E_MDM_LN2_DAC_ADC_RT     0x42  // eB
//#define XLLP_AC97_CR_E_MDM_HNDST_DAC_ADC_RT   0x44  // eB
//#define XLLP_AC97_CR_E_MDM_LN1_DAC_ADC_LVL    0x46  // eB 
//#define XLLP_AC97_CR_E_MDM_LN2_DAC_ADC_LVL    0x48  // eB
//#define XLLP_AC97_CR_E_MDM_HNDST_DAC_ADC_LVL  0x4A  // eB
//#define XLLP_AC97_CR_E_MDM_GPIO_PIN_CNFG      0x4C  // eB
//#define XLLP_AC97_CR_E_MDM_GPIO_PIN_POL_TYPE  0x4E  // eB
//#define XLLP_AC97_CR_E_MDM_GPIO_PIN_STICKY    0x50  // eB
//#define XLLP_AC97_CR_E_MDM_GPIO_PIN_WAKEUP    0x52  // eB
//#define XLLP_AC97_CR_E_MDM_GPIO_PIN_STAT      0x54  // eB
//#define XLLP_AC97_CR_E_MDM_MISC_AFE_CTRL_STAT 0x56  // eB
//#define XLLP_AC97_CR_E_RESERVED_0x58          0x58

//
// 5Ah-7Ah: Vendor Reserved
//

// UCB1400, Philips proprietary.
#define    XLLP_AC97_CR_U14_IO_CTRL_STAT  0x5A  // I/O pin level [0..9]. R/W.
#define    XLLP_AC97_CR_U14_IO_DIRN       0x5C  // Sets In(0) or Out(1) for I/O pins
#define    XLLP_AC97_CR_U14_POS_INT_ENAB  0x5E  // Enables intrpt sgnl on rising edge
#define    XLLP_AC97_CR_U14_NEG_INT_ENAB  0x60  // Enables intrpt sgnl on falling edge
#define    XLLP_AC97_CR_U14_INT_CLR_STAT  0x62  // Reports intrpt status, W clears stat
#define    XLLP_AC97_CR_U14_TS_CTRL       0x64  // Touch Screen Control
#define    XLLP_AC97_CR_U14_ADC_CTRL      0x66  // ADC Control
#define    XLLP_AC97_CR_U14_ADC_DATA      0x68  // ADC Data
#define    XLLP_AC97_CR_U14_FTR_CTRL_STAT1  0x6A  // Feature control + status reg 1
#define    XLLP_AC97_CR_U14_FTR_CTRL_STAT2  0x6C  // Feature control + status reg 2
#define    XLLP_AC97_CR_U14_TEST_CTRL       0x6E  // Only in Vendor Specific Test Mode

//
// 7Ch-7Eh: Vendor ID registers.  Optional but standardized for Plug'n'Play
//
#define    XLLP_AC97_CR_VENDOR_ID1         0x7C  
#define    XLLP_AC97_CR_VENDOR_ID2         0x7E  

#define    XLLP_AC97_CR_MAX               XLLP_AC97_CR_VENDOR_ID2

#define    XLLP_AC97_CR_END_OF_LIST       (XLLP_AC97_CR_MAX +  2)


#define    XLLP_AC97_U14_RR_LOUDNESS ( 0x1 << 5 )    // Loudness (bass boost) supported
#define    XLLP_AC97_U14_RR_20BITDAC ( 0x1 << 7 )    // supports 20 bit DAC
#define XLLP_AC97_U14_RR_20BITADC ( 0x1 << 9 )    // supports 20 bit ADC

// Master Volume Register (MVR) definitions

#define XLLP_AC97_U14_MVR_MR_SHIFT    0               // Volume Right, 6 bits wide
// Bits 6,7 Reserved
#define XLLP_AC97_U14_MVR_ML_SHIFT    8               // Volume Left, 6 bits wide
// Bit 14 Reserved
#define XLLP_AC97_U14_MVR_MM          ( 0x1 << 15 )   // Master Mute

// MIC Volume Register (MCVR) definitions

// Bits 0-5 Reserved
#define XLLP_AC97_U14_MCVR_20DB       ( 0x1 << 6 )    // MIC Volume boosted by 20 dB
// Bits 7-15 Reserved

// Record Select Register (RSR) definitions

#define XLLP_AC97_U14_RSR_SR_SHIFT    0
#define XLLP_AC97_U14_RSR_SR_CL       ( 0x0 << XLLP_AC97_U14_RSR_SR_SHIFT ) // copy from left
#define XLLP_AC97_U14_RSR_SR_LINE     ( 0x100 << XLLP_AC97_U14_RSR_SR_SHIFT )

#define XLLP_AC97_U14_RSR_SL_SHIFT    8
#define XLLP_AC97_U14_RSR_SL_MIC      ( 0x0 << XLLP_AC97_U14_RSR_SL_SHIFT )
#define XLLP_AC97_U14_RSR_SL_LINE     ( 0x100 << XLLP_AC97_U14_RSR_SL_SHIFT )

// Record Gain Register (RGR) definitions

#define XLLP_AC97_U14_RGR_GR_SHIFT    0               // Gain Right, 4 bits wide
// Bits 4-7 Reserved
#define XLLP_AC97_U14_RGR_GL_SHIFT    8               // Gain Left, 4 bits wide
// Bits 12-14 Reserved
#define XLLP_AC97_U14_RGR_RM          ( 0x1 << 15 )   // Record Mute

// General Purpose Register (GPR) definitions

// Bits 0-6 Reserved
#define XLLP_AC97_U14_GPR_LPBK        ( 0x1 << 7 )    // ADC/DAC Loopback Mode
// Bits 8-15 Reserved

// Powerdown Control/Status Register (PCSR) definitions

#define XLLP_AC97_U14_PCSR_ADCR   ( 0x1 << 0 )    // ADC ready to transmit data
#define XLLP_AC97_U14_PCSR_DAC    ( 0x1 << 1 )    // DAC ready to accept data
// Bit 2 Reserved
#define XLLP_AC97_U14_PCSR_REF    ( 0x1 << 3 )    // Vref is up to nominal level
// Bits 4-7 Reserved
#define XLLP_AC97_U14_PCSR_PR0    ( 0x1 << 8 )    // ADC & input path powerdown
#define XLLP_AC97_U14_PCSR_PR1    ( 0x1 << 9 )    // DAC & ouput path powerdown
// Bit 10 Reserved
#define XLLP_AC97_U14_PCSR_PR3    ( 0x1 << 11 )   // Vref powerdown
#define XLLP_AC97_U14_PCSR_PR4    ( 0x1 << 12 )   // Digital interface powerdown
#define XLLP_AC97_U14_PCSR_PR5    ( 0x1 << 13 )   // Internal Clock disable
// Bits 14,15 Reserved

// Extended Audio ID Register (EAIDR) definitions

#define XLLP_AC97_U14_EAIDR_VRA   ( 0x1 << 0 )    // Variable Rate PCM Audio supported
// Bits 1-13 Reserved
#define XLLP_AC97_U14_EAIDR_ID    ( 0x11 << 14 )  // 2 bits wide, Always 0.  UCB1400 is a primary codec

// Extended Audio Status and Control Register (EASCR) definitions

#define XLLP_AC97_U14_EASCR_VRA   ( 0x1 << 0 )    // Enable Variable Rate Audio mode
// Bits 1-15 Reserved

// Audio DAC & ADC Sample Rate Control Register (ADR & AAR) definitions

#define XLLP_AC97_U14_DR_8000     0x1F40  //  8000 samples/sec
#define XLLP_AC97_U14_DR_11025    0x2B11  // 11025 samples/sec
#define XLLP_AC97_U14_DR_16000    0x3E80  // 16000 samples/sec
#define XLLP_AC97_U14_DR_22050    0x5622  // 22050 samples/sec
#define XLLP_AC97_U14_DR_32000    0x7D00  // 32000 samples/sec
#define XLLP_AC97_U14_DR_44100    0xAC44  // 44100 samples/sec
#define XLLP_AC97_U14_DR_48000    0xBB80  // 48000 samples/sec

// I/O Data Register (IODR) and I/O Data Direction (IODIRR) definitions
#define XLLP_AC97_U14_IO0 ( 0x1 << 0 )
#define XLLP_AC97_U14_IO1 ( 0x1 << 1 )
#define XLLP_AC97_U14_IO2 ( 0x1 << 2 )
#define XLLP_AC97_U14_IO3 ( 0x1 << 3 )
#define XLLP_AC97_U14_IO4 ( 0x1 << 4 )
#define XLLP_AC97_U14_IO5 ( 0x1 << 5 )
#define XLLP_AC97_U14_IO6 ( 0x1 << 6 )
#define XLLP_AC97_U14_IO7 ( 0x1 << 7 )
#define XLLP_AC97_U14_IO8 ( 0x1 << 8 )
#define XLLP_AC97_U14_IO9 ( 0x1 << 9 )
// Bits 10-15 Reserved

// Positive INT Enable Register (PIER) definitions

#define XLLP_AC97_U14_PIER_ION0   ( 0x1 << 0 )    // enable falling edge interrupt for I/O pin 0
#define XLLP_AC97_U14_PIER_ION1   ( 0x1 << 1 )    // enable falling edge interrupt for I/O pin 1
#define XLLP_AC97_U14_PIER_ION2   ( 0x1 << 2 )    // enable falling edge interrupt for I/O pin 2
#define XLLP_AC97_U14_PIER_ION3   ( 0x1 << 3 )    // enable falling edge interrupt for I/O pin 3

#define XLLP_AC97_U14_PIER_ION4   ( 0x1 << 4 )    // enable falling edge interrupt for I/O pin 4
#define XLLP_AC97_U14_PIER_ION5   ( 0x1 << 5 )    // enable falling edge interrupt for I/O pin 5
#define XLLP_AC97_U14_PIER_ION6   ( 0x1 << 6 )    // enable falling edge interrupt for I/O pin 6
#define XLLP_AC97_U14_PIER_ION7   ( 0x1 << 7 )    // enable falling edge interrupt for I/O pin 7

#define XLLP_AC97_U14_PIER_ION8   ( 0x1 << 8 )    // enable falling edge interrupt for I/O pin 8
#define XLLP_AC97_U14_PIER_ION9   ( 0x1 << 9 )    // enable falling edge interrupt for I/O pin 9
#define XLLP_AC97_U14_PIER_D10    ( 0x1 << 10 )   // Reserved
#define XLLP_AC97_U14_PIER_ADCP   ( 0x1 << 11 )   // enable falling edge interrupt for ADC Ready

#define XLLP_AC97_U14_PIER_TPXP   ( 0x1 << 12 )   // enable falling edge interrupt for TSPX
#define XLLP_AC97_U14_PIER_TMXP   ( 0x1 << 13 )   // enable falling edge interrupt for TSMX
#define XLLP_AC97_U14_PIER_D14    ( 0x1 << 14 )   // Reserved
#define XLLP_AC97_U14_PIER_OVLP   ( 0x1 << 15 )   // enable falling edge interrupt for OVFL

// Negative INT Enable Register (NIER) definitions

#define XLLP_AC97_U14_NIER_ION0   ( 0x1 << 0 )    // enable falling edge interrupt for I/O pin 0
#define XLLP_AC97_U14_NIER_ION1   ( 0x1 << 1 )    // enable falling edge interrupt for I/O pin 1
#define XLLP_AC97_U14_NIER_ION2   ( 0x1 << 2 )    // enable falling edge interrupt for I/O pin 2
#define XLLP_AC97_U14_NIER_ION3   ( 0x1 << 3 )    // enable falling edge interrupt for I/O pin 3

#define XLLP_AC97_U14_NIER_ION4   ( 0x1 << 4 )    // enable falling edge interrupt for I/O pin 4
#define XLLP_AC97_U14_NIER_ION5   ( 0x1 << 5 )    // enable falling edge interrupt for I/O pin 5
#define XLLP_AC97_U14_NIER_ION6   ( 0x1 << 6 )    // enable falling edge interrupt for I/O pin 6
#define XLLP_AC97_U14_NIER_ION7   ( 0x1 << 7 )    // enable falling edge interrupt for I/O pin 7

#define XLLP_AC97_U14_NIER_ION8   ( 0x1 << 8 )    // enable falling edge interrupt for I/O pin 8
#define XLLP_AC97_U14_NIER_ION9   ( 0x1 << 9 )    // enable falling edge interrupt for I/O pin 9
#define XLLP_AC97_U14_NIER_D10    ( 0x1 << 10 )   // Reserved
#define XLLP_AC97_U14_NIER_ADCN   ( 0x1 << 11 )   // enable falling edge interrupt for ADC Ready

#define XLLP_AC97_U14_NIER_TPXN   ( 0x1 << 12 )   // enable falling edge interrupt for TSPX
#define XLLP_AC97_U14_NIER_TMXN   ( 0x1 << 13 )   // enable falling edge interrupt for TSMX
#define XLLP_AC97_U14_NIER_D14    ( 0x1 << 14 )   // Reserved
#define XLLP_AC97_U14_NIER_OVLN   ( 0x1 << 15 )   // enable falling edge interrupt for OVFL

// INT Clear/Status Register (ICSR) definitions

#define XLLP_AC97_U14_ICSR_IOS0   ( 0x1 << 0 )    // use to check or clear the int status for IO Bit 0
#define XLLP_AC97_U14_ICSR_IOS1   ( 0x1 << 1 )    // use to check or clear the int status for IO Bit 1
#define XLLP_AC97_U14_ICSR_IOS2   ( 0x1 << 2 )    // use to check or clear the int status for IO Bit 2
#define XLLP_AC97_U14_ICSR_IOS3   ( 0x1 << 3 )    // use to check or clear the int status for IO Bit 3

#define XLLP_AC97_U14_ICSR_IOS4   ( 0x1 << 4 )    // use to check or clear the int status for IO Bit 4
#define XLLP_AC97_U14_ICSR_IOS5   ( 0x1 << 5 )    // use to check or clear the int status for IO Bit 5
#define XLLP_AC97_U14_ICSR_IOS6   ( 0x1 << 6 )    // use to check or clear the int status for IO Bit 6
#define XLLP_AC97_U14_ICSR_IOS7   ( 0x1 << 7 )    // use to check or clear the int status for IO Bit 7

#define XLLP_AC97_U14_ICSR_IOS8   ( 0x1 << 8 )    // use to check or clear the int status for IO Bit 8
#define XLLP_AC97_U14_ICSR_IOS9   ( 0x1 << 9 )    // use to check or clear the int status for IO Bit 9
#define XLLP_AC97_U14_ICSR_D10    ( 0x1 << 10 )   // Reserved
#define XLLP_AC97_U14_ICSR_ADCS   ( 0x1 << 11 )   // use to check or clear the int status for ADC ready

#define XLLP_AC97_U14_ICSR_TSPX   ( 0x1 << 12 )   // use to check or clear the int status for TSPX
#define XLLP_AC97_U14_ICSR_TSMX   ( 0x1 << 13 )   // use to check or clear the int status for TSMX
#define XLLP_AC97_U14_ICSR_D14    ( 0x1 << 14 )   // Reserved
#define XLLP_AC97_U14_ICSR_OVLS   ( 0x1 << 15 )   // use to check or clear the int status for OVFL

// Touch Screen Control Register (TSCR) defintions

#define XLLP_AC97_U14_TSCR_TSMX_POW   ( 0x1 << 0 )    // TSMX pin is powered
#define XLLP_AC97_U14_TSCR_TSPX_POW   ( 0x1 << 1 )    // TSPX pin is powered
#define XLLP_AC97_U14_TSCR_TSMY_POW   ( 0x1 << 2 )    // TSMY pin is powered
#define XLLP_AC97_U14_TSCR_TSPY_POW   ( 0x1 << 3 )    // TSPY pin is powered

#define XLLP_AC97_U14_TSCR_TSMX_GND   ( 0x1 << 4 )    // TSMX pin is grounded
#define XLLP_AC97_U14_TSCR_TSPX_GND   ( 0x1 << 5 )    // TSPX pin is grounded
#define XLLP_AC97_U14_TSCR_TSMY_GND   ( 0x1 << 6 )    // TSMY pin is grounded
#define XLLP_AC97_U14_TSCR_TSPY_GND   ( 0x1 << 7 )    // TSPY pin is grounded

#define XLLP_AC97_U14_TSCR_INTMO      ( 0x0 << 8 )    // Interrupt Mode
#define XLLP_AC97_U14_TSCR_PREMO      ( 0x1 << 8 )    // Pressure Measurement Mode
#define XLLP_AC97_U14_TSCR_POSMO      ( 0x2 << 8 )    // Position Measurement Mode
#define XLLP_AC97_U14_TSCR_HYSD       ( 0x1 << 10 )   // Hysteresis deactivated
#define XLLP_AC97_U14_TSCR_BIAS       ( 0x1 << 11 )   // Bias circuitry activated

#define XLLP_AC97_U14_TSCR_PX         ( 0x1 << 12 )   // Inverted state of TSPX pin
#define XLLP_AC97_U14_TSCR_MX         ( 0x1 << 13 )   // Inverted state of TSMX pin
#define XLLP_AC97_U14_TSCR_D14        ( 0x1 << 14 )   // Reserved
#define XLLP_AC97_U14_TSCR_D15        ( 0x1 << 15 )   // Reserved

// ADC Control Register (ADCCR) definitions

#define XLLP_AC97_U14_ADCCR_ASE       ( 0x1 << 0 ) // ADC is armed by AS bit and started by rising edge on ADCSYNC pin
#define XLLP_AC97_U14_ADCCR_D1        ( 0x1 << 1 ) // Reserved

#define XLLP_AC97_U14_ADCCR_AI_SHIFT  2
#define XLLP_AC97_U14_ADCCR_AI_TSPX   ( 0x0 << XLLP_AC97_U14_ADCCR_AI_SHIFT )   // ADC source is TSPX
#define XLLP_AC97_U14_ADCCR_AI_TSMX   ( 0x1 << XLLP_AC97_U14_ADCCR_AI_SHIFT )   // ADC source is TSMX
#define XLLP_AC97_U14_ADCCR_AI_TSPY   ( 0x2 << XLLP_AC97_U14_ADCCR_AI_SHIFT )   // ADC source is TSPY
#define XLLP_AC97_U14_ADCCR_AI_TSMY   ( 0x3 << XLLP_AC97_U14_ADCCR_AI_SHIFT )   // ADC source is TSMY
#define XLLP_AC97_U14_ADCCR_AI_AD0    ( 0x4 << XLLP_AC97_U14_ADCCR_AI_SHIFT )   // ADC source is AD0
#define XLLP_AC97_U14_ADCCR_AI_AD1    ( 0x5 << XLLP_AC97_U14_ADCCR_AI_SHIFT )   // ADC source is AD1
#define XLLP_AC97_U14_ADCCR_AI_AD2    ( 0x6 << XLLP_AC97_U14_ADCCR_AI_SHIFT )   // ADC source is AD2
#define XLLP_AC97_U14_ADCCR_AI_AD3    ( 0x7 << XLLP_AC97_U14_ADCCR_AI_SHIFT )   // ADC source is AD3

#define XLLP_AC97_U14_ADCCR_D5        ( 0x1 << 5 )    // Reserved
#define XLLP_AC97_U14_ADCCR_D6        ( 0x1 << 6 )    // Reserved
#define XLLP_AC97_U14_ADCCR_AS        ( 0x1 << 7 )    // Start the ADC conversion seq.

#define XLLP_AC97_U14_ADCCR_D8        ( 0x1 << 8 )    // Reserved
#define XLLP_AC97_U14_ADCCR_D9        ( 0x1 << 9 )    // Reserved
#define XLLP_AC97_U14_ADCCR_D10       ( 0x1 << 10 )   // Reserved
#define XLLP_AC97_U14_ADCCR_D11       ( 0x1 << 11 )   // Reserved

#define XLLP_AC97_U14_ADCCR_D12       ( 0x1 << 12 )   // Reserved
#define XLLP_AC97_U14_ADCCR_D13       ( 0x1 << 13 )   // Reserved
#define XLLP_AC97_U14_ADCCR_D14       ( 0x1 << 14 )   // Reserved
#define XLLP_AC97_U14_ADCCR_AE        ( 0x1 << 15 )   // ADC is activated

// ADC Data Register (ADCDR) definitions

#define XLLP_AC97_U14_ADCDR_MASK      0x3FF           // ADC data register data mask
#define XLLP_AC97_U14_ADCDR_ADV       ( 0x1 << 15 )   // Conversion complete

// Feature Control/Status Register 1 (FCSR1) definitions

#define XLLP_AC97_U14_FCSR1_OVFL      ( 0x1 << 0 )    // ADC overflow status
// bit 1 is reserved
#define XLLP_AC97_U14_FCSR1_GIEN      ( 0x1 << 2 )    // Enable interrupt/wakeup signaling
#define XLLP_AC97_U14_FCSR1_HIPS      ( 0x1 << 3 )    // Activate ADC High Pass Filter
#define XLLP_AC97_U14_FCSR1_DC        ( 0x1 << 4 )    // DC filter is enabled
#define XLLP_AC97_U14_FCSR1_DE        ( 0x1 << 5 )    // De-emphasis is enabled
#define XLLP_AC97_U14_FCSR1_XTM       ( 0x1 << 6 )    // Crystal Oscillator Powerdown Mode

#define XLLP_AC97_U14_FCSR1_M_SHIFT   7
#define XLLP_AC97_U14_FCSR1_M_FLAT    ( 0x00 << XLLP_AC97_U14_FCSR1_M_SHIFT )   // Flat mode
#define XLLP_AC97_U14_FCSR1_M_MIN1    ( 0x1 << XLLP_AC97_U14_FCSR1_M_SHIFT )    // Minimum mode
#define XLLP_AC97_U14_FCSR1_M_MIN2    ( 0x2 << XLLP_AC97_U14_FCSR1_M_SHIFT )    // Minimum mode
#define XLLP_AC97_U14_FCSR1_M_MAX     ( 0x3 << XLLP_AC97_U14_FCSR1_M_SHIFT )    // Maximum mode

#define XLLP_AC97_U14_FCSR1_TR_SHIFT  9   // 2 bits wide, Treble Boost

#define XLLP_AC97_U14_FCSR1_BB_SHIFT  11  // 4 bits wide, Bass Boost
// Bit 15 Reserved

// Feature Control/Status Register 2 (FCSR2) definitions

#define XLLP_AC97_U14_FCSR2_EV_SHIFT   0
#define XLLP_AC97_U14_FCSR2_EV_MASK    ( 0x7 << XLLP_AC97_U14_FCSR2_EV_SHIFT ) // Mask for reading or clearing EV */
#define XLLP_AC97_U14_FCSR2_EV_NORMOP  ( 0x0 << XLLP_AC97_U14_FCSR2_EV_SHIFT ) // Set EV for normal operation
#define XLLP_AC97_U14_FCSR2_EV_ACLPBK  ( 0x1 << XLLP_AC97_U14_FCSR2_EV_SHIFT ) // Set EV for ACLink loopback
#define XLLP_AC97_U14_FCSR2_EV_BSLPBK  ( 0x2 << XLLP_AC97_U14_FCSR2_EV_SHIFT ) // Set EV for bitstr loopback
#define XLLP_AC97_U14_FCSR2_EV_DACEVAL ( 0x3 << XLLP_AC97_U14_FCSR2_EV_SHIFT ) // Set EV for DAC bitstr eval
#define XLLP_AC97_U14_FCSR2_EV_ADCEVAL ( 0x4 << XLLP_AC97_U14_FCSR2_EV_SHIFT ) // Set EV for ADC bitstr eval
#define XLLP_AC97_U14_FCSR2_EV_CLKEVAL ( 0x5 << XLLP_AC97_U14_FCSR2_EV_SHIFT ) // Set EV for Clocks eval mode
#define XLLP_AC97_U14_FCSR2_EV_ADC10EV ( 0x6 << XLLP_AC97_U14_FCSR2_EV_SHIFT ) // Set EV for 10 bit ADC eval mode
#define XLLP_AC97_U14_FCSR2_EV_NORMOP1 ( 0x7 << XLLP_AC97_U14_FCSR2_EV_SHIFT ) // Set EV for normal operation
// Bit 3 Reserved

#define XLLP_AC97_U14_FCSR2_SLP_SHIFT     4
#define XLLP_AC97_U14_FCSR2_SLP_MASK      ( 0x3 << XLLP_AC97_U14_FCSR2_SLP_SHIFT )  // Mask for reading or clearing SLP
#define XLLP_AC97_U14_FCSR2_SLP_NSLP      ( 0x0 << XLLP_AC97_U14_FCSR2_SLP_SHIFT )  // No Smart Low Power Mode
#define XLLP_AC97_U14_FCSR2_SLP_SLPC      ( 0x1 << XLLP_AC97_U14_FCSR2_SLP_SHIFT )  // Smart Low Power Codec
#define XLLP_AC97_U14_FCSR2_SLP_SLPPLL    ( 0x2 << XLLP_AC97_U14_FCSR2_SLP_SHIFT )  // Smart Low Power PLL
#define XLLP_AC97_U14_FCSR2_SLP_SLPALL    ( 0x3 << XLLP_AC97_U14_FCSR2_SLP_SHIFT )  // Smart Low Power Codec & PLL
// Bits 6-15 Reserved

// Test Control Register (TCR) definitions
#define XLLP_AC97_U14_TCR_IDDQ    ( 0x1 << 0 )    // IDDQ testing
#define XLLP_AC97_U14_TCR_ROM     ( 0x1 << 1 )    // ROM testing
#define XLLP_AC97_U14_TCR_RAM     ( 0x1 << 2 )    // RAM testing
#define XLLP_AC97_U14_TCR_VOH     ( 0x1 << 3 )    // VOH testing
#define XLLP_AC97_U14_TCR_VOL     ( 0x1 << 4 )    // VOL testing
#define XLLP_AC97_U14_TCR_TRI     ( 0x1 << 5 )    // Tri-Sate testing
// Bits 7-15 Reserved

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
    XLLP_AC97_CODEC_PRIMARY = 0,
    XLLP_AC97_CODEC_SECONDARY = 1
} XLLP_AC97_CODEC_SEL_T ; 

typedef struct
{
    P_XLLP_GPIO_T 	pGpioReg;
    P_XLLP_CLKMGR_T pClockReg;
    P_XLLP_AC97_T 	pAc97Reg;
    P_XLLP_OST_T 	pOstRegs;
    P_XLLP_INTC_T 	pIntcReg;
    XLLP_UINT32_T 	maxSetupTimeOutUs;
    XLLP_BOOL_T 	useSecondaryCodec;
} XLLP_AC97_CONTEXT_T, *P_XLLP_AC97_CONTEXT_T ;


typedef struct
{
    XLLP_BOOL_T  codecReady ;
} XLLP_AC97_STAT_T ;


/*
*******************************************************************************
  XLLP AC97 Error 
*******************************************************************************
*/

typedef enum
{
    XLLP_AC97_NO_ERROR = 0,
    XLLP_AC97_CODEC_ACCESS_TIMEOUT = 1,
    XLLP_AC97_CODEC_NOT_READY = 2,
    XLLP_AC97_LINK_SHUTDOWN_FAIL = 3,
    XLLP_AC97_INTERNAL_ERROR = 4,
    XLLP_AC97_LINK_LOCK_FAIL = 5
} XLLP_AC97_ERROR_T ;


/*
*******************************************************************************
  XLLP AC97 Functions 
*******************************************************************************
*/

XLLP_AC97_ERROR_T  XllpAc97Init(P_XLLP_AC97_CONTEXT_T pAc97ctxt);
XLLP_AC97_ERROR_T  XllpAc97DeInit(P_XLLP_AC97_CONTEXT_T pAc97ctxt);
XLLP_AC97_ERROR_T  XllpAc97ColdReset(P_XLLP_AC97_CONTEXT_T pAc97ctxt);  
XLLP_AC97_ERROR_T  XllpAc97ShutdownAclink(P_XLLP_AC97_T pAc97Reg, P_XLLP_OST_T pOstRegs);

XLLP_AC97_ERROR_T  XllpAc97Read(XLLP_UINT16_T offset, P_XLLP_UINT16_T pData, 
                P_XLLP_AC97_T pAc97Reg, P_XLLP_OST_T pOstRegs, 
                XLLP_UINT32_T maxRWTimeOutUs, XLLP_AC97_CODEC_SEL_T codecSel); 
XLLP_AC97_ERROR_T  XllpAc97Write(XLLP_UINT16_T offset, XLLP_UINT16_T data, 
                P_XLLP_AC97_T pAc97Reg, P_XLLP_OST_T pOstRegs, XLLP_UINT32_T maxRWTimeOutUs, 
                XLLP_AC97_CODEC_SEL_T codecSel); 
void  XllpAc97GetStatus(XLLP_AC97_STAT_T *pStat, P_XLLP_AC97_T pAc97Reg, XLLP_AC97_CODEC_SEL_T codecSel);

#endif //__XLLP_AC97_H__
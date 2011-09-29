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
/*++

$Workfile: AC97.H $

$Date: 3/21/02 11:10a $

Abstract:
    Header file for AC 97 specific functions

Functions:

Notes:


/*++

** Copyright 2000-2001 Intel Corporation. All Rights Reserved.
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

--*/



#ifndef __XSC_AC97_H__
#define __XSC_AC97_H__

// Index of Codec Registers on AC-link.
// Note:  Only items having a "Usage" comment are actively used by the driver.      

//      Register Name           Index           // Usage
//-----------------------------------------------------------------------
#define RESET                   0X00            // RESET CODEC TO DEFAULT                   
#define MASTER_VOLUME           0X02            // LINE OUT VOLUME
#define HEADPHONE_VOLUME        0X04
#define MASTER_VOLUME_MONO      0X06
#define MASTER_TONE_R_L         0X08
#define PC_BEEP_VOLUME          0X0A
#define PHONE_VOLUME            0X0C
#define MIC_VOLUME              0X0E            // MICROPHONE VOLUME/ AGC
#define LINE_IN_VOLUME          0X10            // LINE IN VOLUME
#define CD_VOLUME               0X12
#define VIDEO_VOLUME            0X14
#define AUX_VOLUME              0X16
#define PCM_OUT_VOL             0X18
#define RECORD_SELECT           0X1A            // SELECT LINE IN OR MICROPHONE
#define RECORD_GAIN             0X1C
#define RECORD_GAIN_MIC         0X1E            //undefined for UCB1400
#define GENERAL_PURPOSE         0X20
#define CONTROL_3D              0X22
#define RESERVED                0X24
#define POWERDOWN_CTRL_STAT     0X26            // POWER MANAGEMENT
#define EXTENDED_AUDIO_ID       0X28
#define EXTENDED_AUDIO_CTRL     0X2A            // BIT 0 must be set to 1 for variable rate audio
#define AUDIO_DAC_RATE          0X2C            // 16 bit unsigned is sample rate in hertz
#define AUDIO_ADC_RATE          0X32            // 16 bit unsigned is sample rate in hertz
#define FEATURE_CSR1            0x6A            // UCB1400 specific bass, treble and other items
#define FEATURE_CSR2            0x6C            // UCB1400 specific bass, treble and other items
#define VENDOR_ID1              0x7c            // UCB1400 id first 16 bits
#define VENDOR_ID2              0x7e            // UCB1400 id second 16 bits

#define VRA_ENABLED_MASK        0x1             // VRA bit set to 1 enables sample rate conversion

#define EQ_MASK                 0xFE00          //1111111000000000
#define BASS_MASK               0XE000          //1110000000000000
#define TREB_MASK               0x1800          //0001100000000000
#define MODE_MASK               0x0600          //0000011000000000
#define MID_BASS_MAX_TREB       0x9E00          //1001111000000000

#define BASS_BIT_POSTN          (0x1u<<11)
#define TREB_BIT_POSTN          (0x1u<<00)
#define MODE_BIT_POSTN          (0x1u<<07)

//power management bits
#define PWR_ADC                 (0x1)
#define PWR_DAC                 (0x1<<1)
#define PWR_ANL                 (0x1<<2)
#define PWR_REF                 (0x1<<3)
#define PWR_PR0                 (0x1<<8) //write this bit to power down the adc
#define PWR_PR1                 (0x1<<9) //write this bit to power down the dac
#define PWR_PR0_ADC             (0x1<<8) //write this bit to power down the adc
#define PWR_PR1_DAC             (0x1<<9) //write this bit to power down the dac
#define PWR_PR2                 (0x1<<10)
#define PWR_PR3                 (0x1<<11)
#define PWR_PR4                 (0x1<<12)
#define PWR_PR5                 (0x1<<13)
#define PWR_PR6                 (0x1<<14)
#define PWR_EAPD                (0x1<<15)

//smart power bits
#define PWR_SLP0                (0x1<<4)
#define PWR_SLP1                (0x1<<5)
//#define PWR_SMART_NONE
#define PWR_SMART_CODEC         PWR_SLP0
#define PWR_SMART_PLL           PWR_SLP1
#define PWR_SMART_BOTH          (PWR_SLP0 & PWR_SLP1)

// Volume Specific Bits
#define MUTE_VOL                0X8000

#define RECORD_GAIN_MAX         0X0f0f
#define RECORD_GAIN_MIN         0X0
#define MUTE_MASK               0x8000

#define AUDIO_LINE_INPUT_STEREO 0X4040
#define AUDIO_MIC_INPUT_MONO     0X0

// Codec Register Masks.

#define MASTER_VOLUME_R_MASK    0x0000003F      // 5 bits required. 6 optional.
#define MASTER_VOLUME_SHIFT     8               // Left volume bit2 13:8

// AC-link Specifics.

#define READ_CODEC              ( 1<<7 )        // AC-link. Command Address Port "read" bit.
#define WRITE_CODEC             ( 0x00 )        // AC-link. Command Address Port "write" bit.
                                                //   Bit 7 is zero for writes.
// Useful Constants
#define DWORD_ZEROS     ((DWORD)0x00000000)

//UCB1400 STUFF
#define REV_2A 0x2a0
#define REV_1B 0x2a
#define HPEN_BIT_POS              (6U)  //bit 6 is headphone enable
#define HPEN_MASK                 (0xf << HPEN_BIT_POS)  // bit 6

//CODEC TYPES
#define GENERIC_AC97 0
#define UCB14001B 1
#define UCB14002A 2


// Serial Audio Control Register (SACR0) bit masks.
#define SACR0_ENB_MASK      (1<<0)      // Pins function as SAC
#define SACR0_BCKD_MASK     (1<<2)      // BIT_CLK diretion; 1 ==> Output.
#define SACR0_RST_MASK      (1<<3)      // Reset is active to other SAC registers.
#define SACR0_SAC_ENB       1           // Enable pins as SAC controller.
#define SACR0_BCKD_INPUT    0           // BIT_CLK pin set to input.
#define SACR0_RESET_ACTIVE  1           // Reset other SAC registers(not SACR0) & FIFO
#define SACR0_RESET_CLEAR   0           // Deassert Reset.
// Serial Audio Control Register (SACR2) bit masks.
#define SACR2_WKUP_MASK     (1<<2)      // Wake up codec by activating SYNC.
#define SACR2_DRPL_MASK     (1<<4)      // AC-link Replay; 1==> Disabled.
#define SACR2_RESET_MASK    (1<<6)      // Cold Codec Reset.
#define SACR2_DRPL_DISABLE  1           // 1==> AC-link replay is diabled.
#define SACR2_WKUP_ENABLE   1           // 1==> Activate SYNC signal.
#define SACR2_RESET_ACTIVE      0       // Cold Reset; 0 ==> Active.
#define SACR2_RESET_NOTACTIVE   1       // Cold Reset; 0 ==> Active.
// Serial Audio Status Register (SASR1) bit masks.
#define SASR1_TUR_MASK      (1<<5)          // AC-link Command Address and Data Transmit
#define SASR1_ROR_MASK      (1<<6)          // AC-link Command Address and Data Transmit
#define SASR1_CADT_MASK     (1<<16)         // AC-link Command Address and Data Transmit
                                        // 1 ==> Done
#define SASR1_SADR_MASK     (1<<17)         // AC-link Command Address and Data are Received.
                                        // 1 ==> Done
#define SASR1_RSTO_MASK     (1<<18)         // Read Status Timeout.
                                        // 1 ==> Timeout detected 4 Frames after Read Command.      
#define SASR1_CRDY_MASK     (1<<20)         // Codec Ready. 1 ==> Ready for normal operation.
// Serial Audio Status Clear Register (SASCR)
#define SASCR_DTS_CLEAR     1           // Clear the status sent bit in SASR1.
#define SASCR_TUR_CLEAR     1           // Clear the status sent bit in SASR1.
#define SASCR_ROR_CLEAR     1           // Clear the status sent bit in SASR1.
// Serial Audio DMA Receive/Transmit Control Registers ( SADRCS, SADTCS)
#define SADRTCS_DEN_MASK        (1<<0)      // Enable DMA
//#define   SADRTCS_DEN_MASK_tst    (1<<0)      // $$ Enable DMA
#define SADRTCS_DIE_MASK        (1<<1)      // Enable DMA Interrupts for Done A or B.
#define SADRTCS_DONE_A_MASK     (1<<3)      // The "A" transfer is done.
#define SADRTCS_DONE_B_MASK     (1<<5)      // The "B" transfer is done.
#define SADRTCS_START_A_MASK    (1<<4)      // Start "A" transfer
#define SADRTCS_START_B_MASK    (1<<6)      // Start "B" transfer
#define SADRTCS_BIU_MASK        (1<<7)      // A or B actively transferring data.

// System Bus Control Register (SKCR) bit masks.
#define SKCR_SeLAC_MASK     (1<<8)          // AC-link Command Address and Data Transmit
#define SKCR_SeLAC_ACLINK   1           // Enable AC-link.
// System Power Control Register (SKPCR) bit masks.
#define SKPCR_ACCLNKEN_MASK (1<<1)          // Audio Controller AC Link Clock Enable.
#define SKPCR_ACCLKEN       1           // 1==> AC-link clock enable.
// SA-1111 CPLD. (Section 4.4, Sa-1111 Microprocessor Developent Module, User's Guide Dec 99)
// AUD_CTL, Audio Control Register.
#define AUD_CTL_SELECT_AC97         0       //  0==> AC97
#define SA1111_PLD_SEL1341_MASK (1<<0)

  // Bits Values
#define     AC_Link_Active  1
#define     I2S_Active      0

// SA1111 Interrupt Controller ( 0 - 31)
//#define   INT_AUD_DTS_MASK    (1<<)

// SA1111 Interrupt Controller ( 0 - 31)
#define INT_AUD_DTS_MASK    (1 << 40-32)    // Data Sent to codec register.
#define INT_AUD_RDD_MASK    (1 << 41-32)    // Data received from codec



//
//  Virtual Base Addresses
//

#define NOCOMPILE_RSTREG_PHYSICAL    0x90030000
#define NOCOMPILE_RSTREG_VIRTUAL     0xA9030000


//
// This appears in XSC1.h, but is not compiled.
//

// Input for the function
//  void SA_StartAC97DMA(void * p, void * ba, int len, int ie)

/*
typedef enum {
     PLAY_XMIT,
     RECORD_RCV
} AC97_TRANSFER;
*/
// Buffer sizes in units of 16-bit Stereo samples.
//      Assume AUDIO_DMA_PAGE_SIZE is multiple of 4 bytes.
#define OUT_SAMPLE_SIZE_AC97            4       // Bytes in Stereo 16-Bits
#define AUDIO_DMA_16BIT_BUF_SIZE        (AUDIO_DMA_PAGE_SIZE/2)
#define AUDIO_MORE_THAN_MAX             (1<<30)


//#ifdef NOCOMPILE
/***
struct rsrrBits {
    unsigned swr            :1;
    unsigned rsvd0          :31;
};

struct rcsrBits {
    unsigned hwr            :1;
    unsigned swr            :1;
    unsigned wdr            :1;
    unsigned smr            :1;
    unsigned rsvd0          :28;
};

struct rstreg{
    struct rsrrBits         rsrr;
    struct rcsrBits         rcsr;
    unsigned                tucr;
};
***/

//#endif


#endif // __XSC_AC97_H__

//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: hw_pwm.h
//------------------------------------------------------------------------------
#ifndef __HW_PWM__
#define __HW_PWM__

#ifdef __cplusplus
extern "C" {
#endif
// Defines

// First supported input frequency
#define PWM_FREQ_FIRST PWM_FREQ_24_MHZ

//Last supported input frequency
#define PWM_FREQ_LAST PWM_FREQ_23_4375_KHZ

//First supported PWM channel
#define PWM_CHANNEL_FIRST PWM_CHANNEL_0

//Last supported PWM channel
#define PWM_CHANNEL_LAST PWM_CHANNEL_4

//Bitwise OR of all the supported channel bits
#define PWM_CHANNEL_ALL_BITS \
    (PWM_CHANNEL_0_BIT | \
     PWM_CHANNEL_1_BIT | \
     PWM_CHANNEL_2_BIT | \
     PWM_CHANNEL_3_BIT | \
     PWM_CHANNEL_4_BIT)

#define PWM_MAX_FREQUENCY 1200000000
#define PWM_MIN_FREQUENCY 36

//! The input frequency to the PWM block
#define PWM_INPUT_FREQ_TIMES_100 (24000000UL * 100UL)
//-----------------------------------------------------------------------------
// Types
// Indicates a target clock frequency for a PWM channel
// These values correspond to the CDIV settings in the HW_PWM_PERIOD regs
typedef enum _PWM_INPUTFREQ
{
    //! \brief CDIV=1
    PWM_FREQ_24_MHZ,
    //! \brief CDIV=2
    PWM_FREQ_12_MHZ,
    //! \brief CDIV=4
    PWM_FREQ_6_MHZ,
    //! \brief CDIV=8
    PWM_FREQ_3_MHZ,
    //! \brief CDIV=16
    PWM_FREQ_1500_KHZ,
    //! \brief CDIV=64
    PWM_FREQ_375_KHZ,
    //! \brief CDIV=256 - 93.75 KHz
    PWM_FREQ_93_75_KHZ,
    //! \brief CDIV=1024 - 23.4375 KHz
    PWM_FREQ_23_4375_KHZ,
} PWM_INPUTFREQ;

// PWM channel bit specifier
typedef enum _PWM_CHANNELBIT
{
    //! \brief TBD
    PWM_CHANNEL_0_BIT=0x1,
    //! \brief TBD
    PWM_CHANNEL_1_BIT=0x2,
    //! \brief TBD
    PWM_CHANNEL_2_BIT=0x4,
    //! \brief TBD
    PWM_CHANNEL_3_BIT=0x8,
    //! \brief TBD
    PWM_CHANNEL_4_BIT=0x10,
} PWM_CHANNELBIT;

//PWM channel specifier
typedef enum _PWM_CHANNEL
{
    //! \brief TBD
    PWM_CHANNEL_0,
    //! \brief TBD
    PWM_CHANNEL_1,
    //! \brief TBD
    PWM_CHANNEL_2,
    //! \brief TBD
    PWM_CHANNEL_3,
    //! \brief TBD
    PWM_CHANNEL_4,
} PWM_CHANNEL;

//Used to specify active and inactive states
typedef enum _PWM_STATE
{
    PWM_STATE_HI_Z=0,
    PWM_STATE_LOW=2,
    PWM_STATE_HIGH=3
} PWM_STATE;

/* Table that represents the prescalar divider that will be
 * applied to the period clocks.  Unfourtunately the hardware
 * does not map the register values to corresponding 2^x values.
 * Thus we must have this silly table that maps register values
 * to the actual divider.
 */
const UINT8 reg2div[(PWM_FREQ_LAST + 1)] = {
    0, // 2^0
    1, // 2^1
    1, // 2^2
    1, // 2^3
    1, // 2^4
    2, // 2^6
    2, // 2^8
    2  // 2^10
};
BOOL PwmInitialize(void);
void PwmDeinit(void);
BOOL PWMSetClkGate(BOOL bClkGate);
UINT32 PWMGetChannelPresentMask(VOID);
BOOL PWMChSetMultiChipMode(PWM_CHANNEL Channel,BOOL bMultiChipMode);
BOOL PWMChSetConfig(PWM_CHANNEL Channel,
                    PWM_STATE ActiveState,
                    PWM_STATE InactiveState,
                    UINT32 u32HzTimes100,
                    UINT32 u32PercentActiveDutyCycle);

BOOL   PWMChGetConfig(PWM_CHANNEL Channel,
                      UINT8 *pu8MultiChipMode,
                      PWM_INPUTFREQ *pInputFreq,
                      PWM_STATE *pActiveState,
                      PWM_STATE *pInactiveState,
                      UINT16 *pu16PeriodClocksMinusOne,
                      UINT16 *pu16ClocksBeforeActive,
                      UINT16 *pu16ClocksBeforeInactive);

BOOL PWMChSetDutyCycle(PWM_CHANNEL Channel,
                       PWM_STATE ActiveState,
                       PWM_STATE InactiveState,
                       UINT32 u32PercentActiveDutyCycle);

BOOL PWMChOutputEnable(UINT32 u32ChannelMask, BOOL bEnable);

UINT32 PWMGetChannelOutputEnabledMask(VOID);

//BOOL PWMChSetAnalogFeedback(PWM_CHANNEL Channel, BOOL bEnable);
BOOL   PWMchSetIOMux(UINT32 u32Channel,DDK_IOMUX_PIN_MUXMODE muxmode);

BOOL PWMSetClockGating(BOOL bGating);
#ifdef __cplusplus
}
#endif

#endif //__HW_PWM__

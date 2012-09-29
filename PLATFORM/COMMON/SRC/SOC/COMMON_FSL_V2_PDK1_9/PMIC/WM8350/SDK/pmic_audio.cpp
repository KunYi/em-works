//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @version $Id: pmic_audio.cpp 693 2007-07-04 01:36:24Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//-----------------------------------------------------------------------------
/*!
 * @file pmic_audio.cpp
 * @brief Implementation of the PMIC Audio driver APIs.
 *
 * The PMIC Audio driver and this API were developed to support the
 * audio playback, recording, and mixing capabilities of the power
 * management ICs that are available from Freescale Semiconductor, Inc.
 * In particular, both the and MC13783 and SC5512 power management
 * ICs are currently supported. All of the common features of each
 * power management IC are available as well as access to all device-
 * specific features. However, attempting to use a device-specific
 * feature on a platform on which it is not supported will simply
 * return an error status.
 *
 * The following operating modes are supported:
 *
 * @verbatim
       Operating Mode               MC1378     SC5512     WM8350
       ---------------------------- ------    --------    ------
       Stereo DAC Playback           Yes        Yes        Yes
       Stereo DAC Input Mixing       Yes        Yes        Yes
       Voice CODEC Playback          Yes        Yes        No
       Voice CODEC Input Mixing      Yes        No         No
       Voice CODEC Mono Recording    Yes        Yes        No
       Voice CODEC Stereo Recording  Yes        No         No
       Microphone Bias Control       Yes        Yes        Yes
       Output Amplifier Control      Yes        Yes        Yes
       Output Mixing Control         Yes        Yes        Yes
       Input Amplifier Control       Yes        Yes        Yes
       Master/Slave Mode Select      Yes        Yes        Yes
       Anti Pop Bias Circuit Control Yes        Yes        Yes
   @endverbatim
 *
 * Note that the Voice CODEC may also be referred to as the Telephone
 * CODEC in the PMIC DTS documentation. Also note that, while the power
 * management ICs do provide similar audio capabilities, each PMIC may
 * support additional configuration settings and features. Therefore, it
 * is highly recommended that the appropriate power management IC DTS
 * documents be used in conjunction with this API interface. 
 *
 * @ingroup PMIC_WM8350_AUDIO
 */

/*===========================================================================*/

#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
#include <mmsystem.h>

#include <pmic_audio.h> /* PMIC audio driver API definitions.               */
#include <pmic_adc.h>   /* PMIC analog-to-digital converter interface defs. */
#include <pmic_lla.h>   /* PMIC low-level interface definitions.            */
#include "WMPmic.h"

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

extern DBGPARAM dpCurSettings;

#endif  // DEBUG

/*!
 *  Our handle to the audio device.
 */
WM_DEVICE_HANDLE g_hWMAudioDevice = WM_HANDLE_INVALID;

/*!
 * Define the minimum sampling rate (in Hz) that is supported by the
 * Stereo DAC.
 */
const unsigned MIN_STDAC_SAMPLING_RATE_HZ = 8000;

/*!
 * Define the maximum sampling rate (in Hz) that is supported by the
 * Stereo DAC.
 */
const unsigned MAX_STDAC_SAMPLING_RATE_HZ = 48000;

/*! @def SET_BITS
 * Set a register field to a given value.
 */
#define SET_BITS(reg, field, value)    (((value) << reg.field.offset) & \
                                        reg.field.mask)
/*! @def GET_BITS
 * Get the current value of a given register field.
 */
#define GET_BITS(reg, field, value)    (((value) & reg.field.mask) >> \
                                        reg.field.offset)

/*!
 * @brief Define the possible states for a device handle.
 *
 * This enumeration is used to track the current state of each device handle.
 */
typedef enum
{
    HANDLE_FREE,  /*!< Handle is available for use. */
    HANDLE_IN_USE /*!< Handle is currently in use.  */
} HANDLE_STATE;

/*!
 * WinCE volume scheme: 16 bits of Left and 16 bits of Right (0xRRRRLLLL)
 */
#define RIGHT_VOLUME_MASK   0xFFFF0000
#define RIGHT_VOLUME_SHIFT  16
#define LEFT_VOLUME_MASK    0x0000FFFF
#define LEFT_VOLUME_SHIFT   0
#define RIGHT_VOL( _vol )   ((unsigned short)(((_vol) & RIGHT_VOLUME_MASK) >> RIGHT_VOLUME_SHIFT))
#define LEFT_VOL( _vol )    ((unsigned short)(((_vol) & LEFT_VOLUME_MASK) >> LEFT_VOLUME_SHIFT))

/*!
 * @brief This structure is used to track the state of a microphone input.
 */
typedef struct
{
    PMIC_AUDIO_INPUT_PORT      mic;      /*!< Microphone input port.      */
    PMIC_AUDIO_INPUT_MIC_STATE micOnOff; /*!< Microphone On/Off state.    */
    PMIC_AUDIO_MIC_AMP_MODE    ampMode;  /*!< Input amplifier mode.       */
    PMIC_AUDIO_MIC_GAIN        gain;     /*!< Input amplifier gain level. */
} PMIC_MICROPHONE_STATE;

/*!
 * @brief Tracks whether a headset is currently attached or not.
 */
typedef enum {
    NO_HEADSET, /*!< No headset currently attached. */
    HEADSET_ON  /*!< Headset has been attached.     */
} HEADSET_STATUS;

/*! Constant NULL value for initializing/reseting the audio handles. */
static const PMIC_AUDIO_HANDLE AUDIO_HANDLE_NULL = (PMIC_AUDIO_HANDLE)NULL;

/*!
 * @brief This structure maintains the current state of the Stereo DAC.
 */
typedef struct
{
    PMIC_AUDIO_HANDLE               handle;       /*!< Handle used to access
                                                      the Stereo DAC.         */
    HANDLE_STATE                    handleState;  /*!< Current handle state.   */
    PMIC_AUDIO_DATA_BUS             busID;        /*!< Data bus used to access
                                                      the Stereo DAC.         */
    PMIC_AUDIO_BUS_PROTOCOL         protocol;     /*!< Data bus protocol.      */
    PMIC_AUDIO_BUS_MODE             masterSlave;  /*!< Master/Slave mode
                                                      select.                 */
    PMIC_AUDIO_NUMSLOTS             numSlots;     /*!< Number of timeslots
                                                      used.                   */
    PMIC_AUDIO_CALLBACK             callback;     /*!< Event notification
                                                      callback function
                                                      pointer.                */
    PMIC_AUDIO_EVENTS               eventMask;    /*!< Event notification mask.*/
    PMIC_AUDIO_CLOCK_IN_SOURCE      clockIn;      /*!< Stereo DAC clock input
                                                      source select.          */
    PMIC_AUDIO_SAMPLING_RATE        samplingRate; /*!< Stereo DAC sampling rate
                                                      select.                 */
    PMIC_AUDIO_FREQ                 clockFreq;    /*!< Stereo DAC clock input
                                                      frequency.              */
    PMIC_AUDIO_CLOCK_INVERT         invert;       /*!< Stereo DAC clock signal
                                                      invert select.          */
    PMIC_AUDIO_STDAC_TIMESLOTS      timeslot;     /*!< Stereo DAC data
                                                      timeslots select.       */
    PMIC_AUDIO_STDAC_CONFIG         config;       /*!< Stereo DAC configuration
                                                      options.                */
} PMIC_AUDIO_STDAC_STATE;

/*!
 * @brief This variable maintains the current state of the Stereo DAC.
 *
 * This variable tracks the current state of the Stereo DAC audio hardware
 * along with any information that is required by the device driver to
 * manage the hardware (e.g., callback functions and event notification
 * masks).
 *
 * The initial values represent the reset/power on state of the Stereo DAC.
 */
static PMIC_AUDIO_STDAC_STATE s_hifiCodec = {
    (PMIC_AUDIO_HANDLE)NULL,       /* handle       */
    HANDLE_FREE,                   /* handleState  */
    AUDIO_DATA_BUS_1,              /* busID        */
    NORMAL_MSB_JUSTIFIED_MODE,     /* protocol     */
    BUS_SLAVE_MODE,                /* masterSlave  */
    USE_2_TIMESLOTS,               /* numSlots     */
    (PMIC_AUDIO_CALLBACK)NULL,     /* callback     */
    (PMIC_AUDIO_EVENTS)NULL,       /* eventMask    */
    CLOCK_IN_SYSCLK,               /* clockIn      */
    STDAC_RATE_44_1_KHZ,           /* samplingRate */
    STDAC_CLI_19_2MHZ,             /* clockFreq    */
    NO_INVERT,                     /* invert       */
    USE_TS0_TS1,                   /* timeslot     */
    (PMIC_AUDIO_STDAC_CONFIG)0     /* config       */
};

/*!
 * @brief This structure maintains the current state of the Voice CODEC.
 */
typedef struct
{
    PMIC_AUDIO_HANDLE               handle;       /*!< Handle used to access
                                                       the Voice CODEC.       */
    HANDLE_STATE                    handleState;  /*!< Current handle state.  */
    PMIC_AUDIO_DATA_BUS             busID;        /*!< Data bus used to access
                                                       the Voice CODEC.       */
    PMIC_AUDIO_BUS_PROTOCOL         protocol;     /*!< Data bus protocol.     */
    PMIC_AUDIO_BUS_MODE             masterSlave;  /*!< Master/Slave mode
                                                       select.                */
    PMIC_AUDIO_NUMSLOTS             numSlots;     /*!< Number of timeslots
                                                       used.                  */
    PMIC_AUDIO_CALLBACK             callback;     /*!< Event notification
                                                       callback function
                                                       pointer.               */
    PMIC_AUDIO_EVENTS               eventMask;    /*!< Event notification
                                                       mask.                  */
    PMIC_AUDIO_CLOCK_IN_SOURCE      clockIn;      /*!< Voice CODEC clock input
                                                       source select.         */
    PMIC_AUDIO_SAMPLING_RATE        samplingRate; /*!< Voice CODEC sampling
                                                       rate select.           */
    PMIC_AUDIO_FREQ                 clockFreq;    /*!< CODEC clock input
                                                       frequency.             */
    PMIC_AUDIO_CLOCK_INVERT         invert;       /*!< Voice CODEC clock
                                                       signal invert select.  */
    PMIC_AUDIO_VCODEC_TIMESLOT      timeslot;     /*!< Voice CODEC data
                                                       timeslot select.       */
    PMIC_AUDIO_VCODEC_TIMESLOT      secondaryTXtimeslot; /*!< Voice CODEC
                                                              secondary TX
                                                              timeslot.       */
    PMIC_AUDIO_VCODEC_CONFIG        config;          /*!< Voice CODEC
                                                          configuration
                                                          options.            */
    PMIC_AUDIO_INPUT_CONFIG         inputConfig;     /*!< Voice CODEC
                                                          recording
                                                          configuration.      */
    PMIC_MICROPHONE_STATE           leftChannelMic;  /*!< Left channel
                                                          microphone
                                                          configuration.      */
    PMIC_MICROPHONE_STATE           rightChannelMic; /*!< Right channel
                                                          microphone
                                                          configuration.      */
} PMIC_AUDIO_VCODEC_STATE;

/*!
 * @brief This variable maintains the current state of the Voice CODEC.
 *
 * This variable tracks the current state of the Voice CODEC audio hardware
 * along with any information that is required by the device driver to
 * manage the hardware (e.g., callback functions and event notification
 * masks).
 *
 * The initial values represent the reset/power on state of the Voice CODEC.
 */
static PMIC_AUDIO_VCODEC_STATE s_voiceCodec = {
    (PMIC_AUDIO_HANDLE)NULL,                         /* handle             */
    HANDLE_FREE,                                     /* handleState        */
    AUDIO_DATA_BUS_2,                                /* busID              */
    NETWORK_MODE,                                    /* protocol           */
    BUS_SLAVE_MODE,                                  /* masterSlave        */
    USE_4_TIMESLOTS,                                 /* numSlots           */
    (PMIC_AUDIO_CALLBACK)NULL,                       /* callback           */
    (PMIC_AUDIO_EVENTS)NULL,                         /* eventMask          */
    CLOCK_IN_SYSCLK,                                 /* clockIn            */
    STDAC_RATE_8_KHZ,                                /* samplingRate       */
    STDAC_CLI_19_2MHZ,                               /* clockFreq          */
    NO_INVERT,                                       /* invert             */
    USE_TS0,                                         /* timeslot           */
    USE_TS2,                                         /* secondary timeslot */
    PMIC_AUDIO_VCODEC_CONFIG(INPUT_HIGHPASS_FILTER |
                             OUTPUT_HIGHPASS_FILTER),/* config             */
    (PMIC_AUDIO_INPUT_CONFIG)NULL,                   /* inputConfig        */
                                                     /* leftChannelMic     */
    { NO_MIC,                                        /*    mic             */
      MICROPHONE_OFF,                                /*    micOnOff        */
      CURRENT_TO_VOLTAGE,                            /*    ampMode         */
      MIC_GAIN_0DB                                   /*    gain            */
    },
                                                     /* rightChannelMic    */
    { NO_MIC,                                        /*    mic             */
      MICROPHONE_OFF,                                /*    micOnOff        */
      AMP_OFF,                                       /*    ampMode         */
      MIC_GAIN_0DB                                   /*    gain            */
    }
};

/*!
 * @brief This maintains the current state of the External Stereo Input.
 */
typedef struct
{
    PMIC_AUDIO_HANDLE         handle;      /*!< Handle used to access the
                                                External Stereo Inputs.     */
    HANDLE_STATE              handleState; /*!< Current handle state.       */
    PMIC_AUDIO_CALLBACK       callback;    /*!< Event notification callback
                                                function pointer.           */
    PMIC_AUDIO_EVENTS         eventMask;   /*!< Event notification mask.    */
    PMIC_AUDIO_STEREO_IN_GAIN inputGain;   /*!< External Stereo Input
                                                amplifier gain level.       */
} PMIC_AUDIO_EXT_STEREO_IN_STATE;

/*!
 * @brief This maintains the current state of the External Stereo Input.
 *
 * This variable tracks the current state of the External Stereo Input audio
 * hardware along with any information that is required by the device driver
 * to manage the hardware (e.g., callback functions and event notification
 * masks).
 *
 * The initial values represent the reset/power on state of the External
 * Stereo Input.
 */
static PMIC_AUDIO_EXT_STEREO_IN_STATE extStereoIn = {
    (PMIC_AUDIO_HANDLE)NULL,   /* handle      */
    HANDLE_FREE,               /* handleState */
    (PMIC_AUDIO_CALLBACK)NULL, /* callback    */
    (PMIC_AUDIO_EVENTS)NULL,   /* eventMask   */
    STEREO_IN_GAIN_0DB         /* inputGain   */
};

/*!
 * @brief This maintains the current state of the Audio Output Section.
 */
typedef struct
{
    PMIC_AUDIO_OUTPUT_PORT              outputPorts;       /*!< Current audio
                                                             output port.     */
    PMIC_WINDOWS_GAIN                   outputStDACPGAGain; /*!< Output PGA gain
                                                               level.         */
    PMIC_WINDOWS_GAIN                   outputVCodecPGAGain; /*!< Output PGA gain
                                                                level.        */
    PMIC_WINDOWS_GAIN                   outputExtStereoInPGAGain; /*!< Output PGA
                                                                     gain level.
                                                                              */
    PMIC_AUDIO_GAIN                     out1Gain;       /*!< OUT1 gain level */
    PMIC_AUDIO_GAIN                     out2Gain;       /*!< OUT2 gain level */
    PMIC_AUDIO_OUTPUT_BALANCE_GAIN      balanceLeftGain;  /*!< Left channel
                                                             balance gain
                                                             level.           */
    PMIC_AUDIO_OUTPUT_BALANCE_GAIN      balanceRightGain; /*!< Right channel
                                                             balance gain
                                                             level.           */
    PMIC_AUDIO_MONO_ADDER_OUTPUT_GAIN   monoAdderGain;    /*!< Mono adder gain
                                                             level.           */
    PMIC_AUDIO_OUTPUT_CONFIG            config;           /*!< Audio output
                                                             section config
                                                             options.         */
} PMIC_AUDIO_AUDIO_OUTPUT_STATE;

/*!
 * @brief This variable maintains the current state of the Audio Output Section.
 *
 * This variable tracks the current state of the Audio Output Section.
 *
 * The initial values represent the reset/power on state of the Audio
 * Output Section.
 */
static PMIC_AUDIO_AUDIO_OUTPUT_STATE audioOutput = {
    (PMIC_AUDIO_OUTPUT_PORT)0,      /* outputPorts              */
    0x99999999,                     /* outputStDACPGAGain       */
    0x99999999,                     /* outputVCodecPGAGain      */
    0x99999999,                     /* outputExtStereoInPGAGain */
    PMIC_GAIN_DB( -6 ),             /* out1Gain - HP            */
    PMIC_GAIN_DB( 0 ),              /* out2Gain - SPK           */
    BAL_GAIN_0DB,                   /* balanceLeftGain          */
    BAL_GAIN_0DB,                   /* balanceRightGain         */
    MONOADD_GAIN_0DB,               /* monoAdderGain            */
    (PMIC_AUDIO_OUTPUT_CONFIG)0     /* config                   */
};

/*! The current headset status. */
static HEADSET_STATUS headsetState = NO_HEADSET;

/*! Define a 1 ms wait interval that is needed to ensure that certain
 *  hardware operations are successfully completed.
 */
static const unsigned long delay_1ms = 1;

/*! Define a handle to a mutex object for implementing mutual exclusion.
 *
 *  Note that because this source file will be part of a DLL, we cannot
 *  just call CreateMutex() here to initialize the mutex handle. Attempting
 *  to do so will result in a LNK4210 warning when the linker tries to
 *  create the DLL. This warning indicates that no CRT (C runtime) code
 *  is available to handle non-trivial static initializers (such as those
 *  that involve a function call). Therefore, we must defer the CreateMutex()
 *  call until the DLL is actually loaded by the Device Manager and
 *  activated. This will result in a call to pmic_audio_driver_init() where
 *  we can finally make our CreateMutex() call.
 *
 *  The mutex handle is released and the mutex destroyed when the Device
 *  Manager deactivates the device and pmic_audio_driver_deinit() is called.
 */
static HANDLE mutex = NULL;

/*! Declare a global flag that is only set to TRUE when the powerdown handler
 *  is running. This allows us to bypass any timed delay calls that would
 *  have normally been made. Because the powerdown handler runs at priority 0
 *  with interrupts disabled, it cannot do anything that would cause it to
 *  block or otherwise wait.
 */
static BOOL g_audioPowerdown = FALSE;

/*! We need to have a globally accessible handle to the timed delay event
 *  object so that the powerdown handler can forcibly cancel an in-progress
 *  timer. All in-progress timed delays must be immediately cancelled so
 *  that the thread can wake up and quickly release the critical section
 *  (which is guarded by the mutex handle that was declared above).
 *  Otherwise, the powerdown handler will block when it too tries to
 *  acquire the critical section.
 */
static HANDLE hTimedDelay = NULL;

/* Prototypes for all static audio driver functions. */

//static PMIC_STATUS pmic_read_reg(const pmic_control_register regID,
//                                 const unsigned *regValue);
//static PMIC_STATUS pmic_write_reg(const pmic_control_register regID,
//                                  const unsigned regValue,
//                                  const unsigned regMask);
static inline void down_interruptible(HANDLE hMutex);
static inline void up(HANDLE hMutex);
//static PMIC_STATUS pmic_adc_convert(const unsigned short  channel,
//                                    const unsigned short *adcResult);
//static PMIC_STATUS pmic_audio_mic_boost_enable(void);
//static PMIC_STATUS pmic_audio_mic_boost_disable(void);
static PMIC_STATUS pmic_audio_close_handle(const PMIC_AUDIO_HANDLE handle);
static PMIC_STATUS pmic_audio_reset_device(const PMIC_AUDIO_HANDLE handle);

/*************************************************************************
 * Audio device access APIs.
 *************************************************************************
 */

/*!
 * @name General Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC Audio
 * hardware.
 */
/*@{*/

/*!
 * @brief Request exclusive access to the PMIC Audio hardware.
 *
 * Attempt to open and gain exclusive access to a key PMIC audio hardware
 * component (e.g., the Stereo DAC or the Voice CODEC). Depending upon the
 * type of audio operation that is desired and the nature of the audio data
 * stream, the Stereo DAC and/or the Voice CODEC will be a required hardware
 * component and needs to be acquired by calling this function.
 *
 * If the open request is successful, then a numeric handle is returned
 * and this handle must be used in all subsequent function calls to complete
 * the configuration of either the Stereo DAC or the Voice CODEC and along
 * with any other associated audio hardware components that will be needed.
 *
 * The same handle must also be used in the close call when use of the PMIC
 * audio hardware is no longer required.
 *
 * The open request will fail if the requested audio hardware component has
 * already been acquired by a previous open call but not yet closed.
 *
 * @param[out]  handle          Device handle to be used for subsequent PMIC
 *                              audio API calls.
 * @param[in]   device          The required PMIC audio hardware component.
 *
 * @retval      PMIC_SUCCESS         If the open request was successful
 * @retval      PMIC_PARAMETER_ERROR If the handle argument is NULL.
 * @retval      PMIC_ERROR           If the audio hardware component is
 *                                   unavailable.
 */
PMIC_STATUS PmicAudioOpen(
    PMIC_AUDIO_HANDLE *const handle,
    const PMIC_AUDIO_SOURCE  device)
{
    PMIC_STATUS retval = PMIC_ERROR;
    WM_STATUS   status;

    if (handle == (PMIC_AUDIO_HANDLE *)NULL)
    {
        /* Do not dereference a NULL pointer. */
        return PMIC_PARAMETER_ERROR;
    }

    /* We only need to acquire a mutex here because the interrupt handler
     * never modifies the device handle or device handle state. Therefore,
     * we don't need to worry about conflicts with the interrupt handler
     * or the need to execute in an interrupt context.
     *
     * But we do need a critical section here to avoid problems in case
     * multiple calls to pmic_audio_open() are made since we can only allow
     * one of them to succeed.
     */
    down_interruptible(mutex);

    /*
     * Check the current device handle state and acquire the handle if
     * it is available.
     */
    if ((device == HIFI_DAC) && (s_hifiCodec.handleState == HANDLE_FREE))
    {
        status = WMAudioOpenDevice( &g_hWMAudioDevice );
        if ( WM_SUCCESS( status ) )
        {
            s_hifiCodec.handle      = (PMIC_AUDIO_HANDLE)(&s_hifiCodec);
            s_hifiCodec.handleState = HANDLE_IN_USE;
            *handle = s_hifiCodec.handle;

            retval = PMIC_SUCCESS;
        }
        else
        {
            *handle = AUDIO_HANDLE_NULL;
            retval = WMStatusToPmicStatus( status );
        }
    }
    else if ((device == HIFI_ADC) && (s_voiceCodec.handleState == HANDLE_FREE))
    {
        if (g_hWMAudioDevice != WM_HANDLE_INVALID)
        {
        
            s_voiceCodec.handle      = (PMIC_AUDIO_HANDLE)(&s_voiceCodec);
            s_voiceCodec.handleState = HANDLE_IN_USE;
            *handle = s_voiceCodec.handle;

            retval = PMIC_SUCCESS;
        }
        else
        {
            *handle = AUDIO_HANDLE_NULL;
        }
    }
    else
    {
        *handle = AUDIO_HANDLE_NULL;
    }

    /* Exit the critical section. */
    up(mutex);

    return retval;
}

/*!
 * @brief Terminate further access to the PMIC audio hardware.
 *
 * Terminate further access to the PMIC audio hardware that was previously
 * acquired by calling pmic_audio_open(). This now allows another thread to
 * successfully call pmic_audio_open() to gain access.
 *
 * Note that we will shutdown/reset the Voice CODEC or Stereo DAC as well as
 * any associated audio input/output components that are no longer required.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the close request was successful.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 */
PMIC_STATUS PmicAudioClose(const PMIC_AUDIO_HANDLE handle)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;

    /* We need a critical section here to maintain a consistent state. */
    down_interruptible(mutex);

    /* We can now call pmic_audio_close_handle() to actually do the work. */
    retval = pmic_audio_close_handle(handle);

    /* Exit the critical section. */
    up(mutex);

    return retval;
}

/*!
 * @brief Configure the data bus protocol to be used.
 *
 * Provide the parameters needed to properly configure the audio data bus
 * protocol so that data can be read/written to either the Stereo DAC or
 * the Voice CODEC.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   busID           Select data bus to be used.
 * @param[in]   protocol        Select the data bus protocol.
 * @param[in]   masterSlave     Select the data bus timing mode.
 * @param[in]   numSlots        Define the number of timeslots (only if in
 *                              master mode).
 *
 * @retval      PMIC_SUCCESS         If the protocol was successful configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or the protocol parameters
 *                                   are invalid.
 */
PMIC_STATUS PmicAudioSetProtocol(
    const PMIC_AUDIO_HANDLE       handle,
    const PMIC_AUDIO_DATA_BUS     busID,
    const PMIC_AUDIO_BUS_PROTOCOL protocol,
    const PMIC_AUDIO_BUS_MODE     masterSlave,
    const PMIC_AUDIO_NUMSLOTS     numSlots)
{
    PMIC_STATUS         retval = PMIC_PARAMETER_ERROR;
    WM_STATUS           status;
    BOOL                isMaster;
    WM_AUDIOIF_FORMAT   format;
    WM_AUDIOIF_FLAGS    flags = 0;

    UNREFERENCED_PARAMETER( busID );

    /*
     * Parameter validation.
     */
    if ( ( s_hifiCodec.handle != handle ) || ( HANDLE_IN_USE != s_hifiCodec.handleState ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: invalid handle 0x%X\r\n"),
                  _T(__FUNCTION__),
                  handle
                ));
        retval = PMIC_PARAMETER_ERROR;
        goto paramError;
    }

    if ( (NETWORK_MODE == protocol) || (USE_2_TIMESLOTS != numSlots) )
    {
        /*
         * ## TBD - support TDM.
         */
        DEBUGMSG(ZONE_ERROR,
                 (_T("%s: TDM mode is not supported yet)\n"),
                 __FUNCTION__));
        retval = PMIC_NOT_SUPPORTED;
        goto paramError;
    }

    switch ( protocol )
    {
    case I2S_MODE:
        format = WM_AUDIOIF_I2S;
        break;

    case NORMAL_MSB_JUSTIFIED_MODE:
        format = WM_AUDIOIF_LEFT_JUSTIFY;
        break;

    default:
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: protocol %d not supported\r\n"),
                  _T(__FUNCTION__),
                  protocol
                ));
        retval = PMIC_NOT_SUPPORTED;
        goto paramError;
    }

    if ( BUS_MASTER_MODE == masterSlave )
        isMaster = TRUE;
    else
        isMaster = FALSE;

    /*
     * Enter a critical section so that we can ensure only one
     * state change request is completed at a time.
     */
    down_interruptible(mutex);

    status = WMAudioConfigureInterface( g_hWMAudioDevice,
                                        WM_AUDIOIF_HIFI,
                                        isMaster,
                                        format,
                                        WM_AUDIOIF_16BIT,
                                        flags
                                      );

    if ( WM_SUCCESS( status ) )
    {
        s_hifiCodec.busID       = busID;
        s_hifiCodec.protocol    = protocol;
        s_hifiCodec.masterSlave = masterSlave;
        s_hifiCodec.numSlots    = numSlots;

        retval = PMIC_SUCCESS;
    }
    else
    {
        retval = WMStatusToPmicStatus( status );
    }

    /* Exit critical section. */
    up(mutex);

paramError:
    return retval;
}

/*!
 * @brief Retrieve the current data bus protocol configuration.
 *
 * Retrieve the parameters that define the current audio data bus protocol.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  busID           The data bus being used.
 * @param[out]  protocol        The data bus protocol being used.
 * @param[out]  masterSlave     The data bus timing mode being used.
 * @param[out]  numSlots        The number of timeslots being used (if in
 *                              master mode).
 *
 * @retval      PMIC_SUCCESS         If the protocol was successful retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 */
PMIC_STATUS PmicAudioGetProtocol(
    const PMIC_AUDIO_HANDLE        handle,
    PMIC_AUDIO_DATA_BUS *const     busID,
    PMIC_AUDIO_BUS_PROTOCOL *const protocol,
    PMIC_AUDIO_BUS_MODE *const     masterSlave,
    PMIC_AUDIO_NUMSLOTS *const     numSlots)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;

    if ((busID != (PMIC_AUDIO_DATA_BUS *)NULL)        &&
        (protocol != (PMIC_AUDIO_BUS_PROTOCOL *)NULL) &&
        (masterSlave != (PMIC_AUDIO_BUS_MODE *)NULL)  &&
        (numSlots != (PMIC_AUDIO_NUMSLOTS *)NULL))
    {
        /* Enter a critical section so that we return a consistent state. */
        down_interruptible(mutex);

        if ((handle == s_hifiCodec.handle) &&
            (s_hifiCodec.handleState == HANDLE_IN_USE))
        {
            *busID       = s_hifiCodec.busID;
            *protocol    = s_hifiCodec.protocol;
            *masterSlave = s_hifiCodec.masterSlave;
            *numSlots    = s_hifiCodec.numSlots;

            retval = PMIC_SUCCESS;
        }
        else if ((handle == s_voiceCodec.handle) &&
                 (s_voiceCodec.handleState == HANDLE_IN_USE))
        {
            *busID       = s_voiceCodec.busID;
            *protocol    = s_voiceCodec.protocol;
            *masterSlave = s_voiceCodec.masterSlave;
            *numSlots    = s_voiceCodec.numSlots;

            retval = PMIC_SUCCESS;
        }

        /* Exit critical section. */
        up(mutex);
    }

    return retval;
}

/*!
 * @brief Enable the Stereo DAC or the Voice CODEC.
 *
 * Explicitly enable the Stereo DAC or the Voice CODEC to begin audio
 * playback or recording as required. This should only be done after
 * successfully configuring all of the associated audio components (e.g.,
 * microphones, amplifiers, etc.).
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the device was successful enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 * @retval      PMIC_ERROR           If the device could not be enabled.
 */
PMIC_STATUS PmicAudioEnable( const PMIC_AUDIO_HANDLE handle )
{
    PMIC_STATUS retval         = PMIC_PARAMETER_ERROR;
    WM_STATUS   status;
    WM_STREAM_ID stream;

    /*
     * Parameter validation.
     */
    if ( ( s_hifiCodec.handle == handle ) && ( HANDLE_IN_USE == s_hifiCodec.handleState ) )
    {
        stream = WM_STREAM_HIFI_OUT;
    }
    else if ( ( s_voiceCodec.handle == handle ) && ( HANDLE_IN_USE == s_voiceCodec.handleState ) )
    {
        stream = WM_STREAM_HIFI_IN;
    }
    else
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: invalid handle 0x%X\r\n"),
                  _T(__FUNCTION__),
                  handle
                ));
        retval = PMIC_PARAMETER_ERROR;
        goto paramError;
    }

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    status = WMAudioEnableStream( g_hWMAudioDevice, stream );
    if ( !WM_SUCCESS( status ) )
        goto error;

    //WMDumpRegs( g_hWMAudioDevice );

    /* Exit critical section. */
    up(mutex);

    return PMIC_SUCCESS;

error:
    /* Exit critical section. */
    up(mutex);

    retval = WMStatusToPmicStatus( status );

paramError:
    return retval;

}

/*!
 * @brief Disable the Stereo DAC or the Voice CODEC.
 *
 * Explicitly disable the Stereo DAC or the Voice CODEC to end audio
 * playback or recording as required.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the device was successful disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 * @retval      PMIC_ERROR           If the device could not be disabled.
 */
PMIC_STATUS PmicAudioDisable( const PMIC_AUDIO_HANDLE handle )
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;
    WM_STATUS   status;
    WM_STREAM_ID stream;

    /*
     * Parameter validation.
     */
    if ( ( s_hifiCodec.handle == handle ) && ( HANDLE_IN_USE == s_hifiCodec.handleState ) )
    {
        stream = WM_STREAM_HIFI_OUT;
    }
    else if ( ( s_voiceCodec.handle == handle ) && ( HANDLE_IN_USE == s_voiceCodec.handleState ) )
    {
        stream = WM_STREAM_HIFI_IN;
    }
    else
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: invalid handle 0x%X\r\n"),
                  _T(__FUNCTION__),
                  handle
                ));
        retval = PMIC_PARAMETER_ERROR;
        goto paramError;
    }

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    status = WMAudioDisableStream( g_hWMAudioDevice, stream );
    if ( WM_SUCCESS( status ) )
        retval = PMIC_SUCCESS;
    else
        retval = WMStatusToPmicStatus( status );

    //WMDumpRegs( g_hWMAudioDevice );

    /* Exit critical section. */
    up(mutex);

paramError:
    return retval;
}

/*!
 * @brief Mute the Stereo DAC or the Voice CODEC.
 *
 * Applies the mute to the Stereo DAC or the Voice CODEC.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the device was successful enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 * @retval      PMIC_ERROR           If the device could not be enabled.
 */
PMIC_STATUS PmicAudioMute(
    const PMIC_AUDIO_HANDLE handle)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;
    WM_STATUS   status;

    /*
     * Parameter validation.
     */
    if ( ( s_hifiCodec.handle != handle ) || ( HANDLE_IN_USE != s_hifiCodec.handleState ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: invalid handle 0x%X\r\n"),
                  _T(__FUNCTION__),
                  handle
                ));
        retval = PMIC_PARAMETER_ERROR;
        goto paramError;
    }

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    status = WMAudioMuteSignal( g_hWMAudioDevice, WM_AUDIO_HIFI_DAC, TRUE );
    if ( WM_SUCCESS( status ) )
        retval = PMIC_SUCCESS;
    else
        retval = WMStatusToPmicStatus( status );

    //WMDumpRegs( g_hWMAudioDevice );

    /* Exit critical section. */
    up(mutex);

paramError:
    return retval;
}

/*!
 * @brief Umute the Stereo DAC or the Voice CODEC.
 *
 * Removes the mute from the Stereo DAC or the Voice CODEC.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the device was successful disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 * @retval      PMIC_ERROR           If the device could not be disabled.
 */
PMIC_STATUS PmicAudioUnmute(
    const PMIC_AUDIO_HANDLE handle)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;
    WM_STATUS   status;

    /*
     * Parameter validation.
     */
    if ( ( s_hifiCodec.handle != handle ) || ( HANDLE_IN_USE != s_hifiCodec.handleState ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: invalid handle 0x%X\r\n"),
                  _T(__FUNCTION__),
                  handle
                ));
        retval = PMIC_PARAMETER_ERROR;
        goto paramError;
    }

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    status = WMAudioMuteSignal( g_hWMAudioDevice, WM_AUDIO_HIFI_DAC, FALSE );
    if ( WM_SUCCESS( status ) )
        retval = PMIC_SUCCESS;
    else
        retval = WMStatusToPmicStatus( status );

    //WMDumpRegs( g_hWMAudioDevice );

    /* Exit critical section. */
    up(mutex);

paramError:
    return retval;
}

/*!
 * @brief Sets up the audio in the given power state.
 *
 * This enables or disables the references and amplifiers within the codec
 * in a controlled manner to prevent pops caused by sudden changes in voltage
 * on the outputs.
 *
 * @param[in]   state           Power state to move to.
 *
 * @retval      PMIC_SUCCESS         If the device was successful disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 * @retval      PMIC_ERROR           If the device could not be disabled.
 */
PMIC_STATUS PmicAudioSetPowerState(
    PMIC_AUDIO_POWER_STATE state)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;
    WM_STATUS   status;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    switch ( state )
    {
        case POWER_OFF:
            status = WMAudioDisableReferences( g_hWMAudioDevice );
            break;

        case POWER_STANDBY:
            status = WMAudioEnableReferences( g_hWMAudioDevice, WM_AUDIO_POWER_STANDBY );
            break;

        case POWER_ACTIVE:
            status = WMAudioEnableReferences( g_hWMAudioDevice, WM_AUDIO_POWER_ACTIVE );
            break;

        default:
            retval = PMIC_PARAMETER_ERROR;
            goto paramError;
    }
    if ( WM_SUCCESS( status ) )
        retval = PMIC_SUCCESS;
    else
        retval = WMStatusToPmicStatus( status );

    //WMDumpRegs( g_hWMAudioDevice );

paramError:
    /* Exit critical section. */
    up(mutex);

    return retval;
}

/*!
 * @brief Reset the selected audio hardware control registers to their
 *        power on state.
 *
 * This resets all of the audio hardware control registers currently
 * associated with the device handle back to their power on states. For
 * example, if the handle is associated with the Stereo DAC and a
 * specific output port and output amplifiers, then this function will
 * reset all of those components to their initial power on state.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the reset operation was successful.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 * @retval      PMIC_ERROR           If the reset was unsuccessful.
 */
PMIC_STATUS PmicAudioReset( const PMIC_AUDIO_HANDLE handle )
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    retval = pmic_audio_reset_device(handle);

    /* Exit the critical section. */
    up(mutex);

    return retval;
}

/*!
 * @brief Reset all audio hardware control registers to their power on state.
 *
 * This resets all of the audio hardware control registers back to their
 * power on states. Use this function with care since it also invalidates
 * (i.e., automatically closes) all currently opened device handles.
 *
 * @retval      PMIC_SUCCESS         If the reset operation was successful.
 * @retval      PMIC_ERROR           If the reset was unsuccessful.
 */
PMIC_STATUS PmicAudioResetAll(void)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    retval = pmic_audio_reset_device(NULL);

    /* Exit the critical section. */
    up(mutex);

    return retval;
}

/*!
 * @brief Set the Audio callback function.
 *
 * Register a callback function that will be used to signal PMIC audio
 * events. For example, the audio driver should register a callback
 * function in order to be notified of headset connect/disconnect events.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   func            A pointer to the callback function.
 * @param[in]   eventMask       A mask selecting events to be notified.
 *
 * @retval      PMIC_SUCCESS         If the callback was successfully
 *                                   registered.
 * @retval      PMIC_PARAMETER_ERROR If the handle or the eventMask is invalid.
 */
PMIC_STATUS PmicAudioSetCallback(
    const PMIC_AUDIO_HANDLE   handle,
    const PMIC_AUDIO_CALLBACK func,
    const PMIC_AUDIO_EVENTS   eventMask)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( func );
    UNREFERENCED_PARAMETER( eventMask );

    PMIC_STATUS retval = PMIC_NOT_SUPPORTED;

    /* This function is deprecated for WinCE because audio driver interrupts
     * are now handled directly by the higher-level WAVEDEV driver.
     */

    return retval;
}

/*!
 * @brief Deregisters the existing audio callback function.
 *
 * Deregister the callback function that was previously registered by calling
 * pmic_audio_set_callback().
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the callback was successfully
 *                                   deregistered.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 */
PMIC_STATUS PmicAudioClearCallback(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    PMIC_STATUS retval = PMIC_NOT_SUPPORTED;

    /* This function is deprecated for WinCE because audio driver interrupts
     * are now handled directly by the higher-level WAVEDEV driver.
     */

    return retval;
}

/*!
 * @brief Get the current audio callback function settings.
 *
 * Get the current callback function and event mask.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  func            The current callback function.
 * @param[out]  eventMask       The current event selection mask.
 *
 * @retval      PMIC_SUCCESS         If the callback information was
 *                                   successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 */
PMIC_STATUS PmicAudioGetCallback(
    const PMIC_AUDIO_HANDLE    handle,
    PMIC_AUDIO_CALLBACK *const func,
    PMIC_AUDIO_EVENTS *const   eventMask)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;

    /* We only need to acquire the mutex here because we will not be updating
     * anything that may affect the interrupt handler. We just need to ensure
     * that the callback fields are not changed while we are in the critical
     * section by calling either pmic_audio_set_callback() or
     * pmic_audio_clear_callback().
     */
    down_interruptible(mutex);

    if ((func != (PMIC_AUDIO_CALLBACK *)NULL) &&
        (eventMask != (PMIC_AUDIO_EVENTS *)NULL))
    {
        if ((handle == s_hifiCodec.handle) &&
            (s_hifiCodec.handleState == HANDLE_IN_USE))
        {
            *func      = s_hifiCodec.callback;
            *eventMask = s_hifiCodec.eventMask;

            retval = PMIC_SUCCESS;
        }
        else if ((handle == s_voiceCodec.handle) &&
                 (s_voiceCodec.handleState == HANDLE_IN_USE))
        {
            *func      = s_voiceCodec.callback;
            *eventMask = s_voiceCodec.eventMask;

            retval = PMIC_SUCCESS;
        }
        else if ((handle == extStereoIn.handle) &&
                 (extStereoIn.handleState == HANDLE_IN_USE))
        {
            *func      = extStereoIn.callback;
            *eventMask = extStereoIn.eventMask;

            retval = PMIC_SUCCESS;
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return retval;
}

/*!
 * @brief Enable the anti-pop circuitry to avoid extra noise when inserting
 *        or removing a external device (e.g., a headset).
 *
 * Enable the use of the built-in anti-pop circuitry to prevent noise from
 * being generated when an external audio device is inserted or removed
 * from an audio plug. A slow ramp speed may be needed to avoid extra noise.
 *
 * @param[in]   rampSpeed       The desired anti-pop circuitry ramp speed.
 *
 * @retval      PMIC_SUCCESS         If the anti-pop circuitry was successfully
 *                                   enabled.
 * @retval      PMIC_ERROR           If the anti-pop circuitry could not be
 *                                   enabled.
 */
PMIC_STATUS PmicAudioAntipopEnable(
    const PMIC_AUDIO_ANTI_POP_RAMP_SPEED rampSpeed)
{
    UNREFERENCED_PARAMETER( rampSpeed );

    // ### TBD
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Disable the anti-pop circuitry.
 *
 * Disable the use of the built-in anti-pop circuitry to prevent noise from
 * being generated when an external audio device is inserted or removed
 * from an audio plug.
 *
 * @retval      PMIC_SUCCESS         If the anti-pop circuitry was successfully
 *                                   disabled.
 * @retval      PMIC_ERROR           If the anti-pop circuitry could not be
 *                                   disabled.
 */
PMIC_STATUS PmicAudioAntipopDisable(void)
{
    // ### TBD
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Performs a reset of the Voice CODEC/Stereo DAC digital filter.
 *
 * The digital filter should be reset whenever the clock or sampling rate
 * configuration has been changed.
 *
 * @retval      PMIC_SUCCESS         If the digital filter was successfully
 *                                   reset.
 * @retval      PMIC_ERROR           If the digital filter could not be reset.
 */
PMIC_STATUS PmicAudioDigitalFilterReset(const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    // ### TBD
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the most recent PTT button voltage reading.
 *
 * This function returns the most recent reading for the PTT button voltage.
 * The value may be used during the processing of the PTT_BUTTON_RANGE event
 * as part of the headset ID detection process.
 *
 * Note that the MC13783 PMIC does not support the Push-to-Talk button.
 *
 * @retval      PMIC_SUCCESS         If the most recent PTT button voltage was
 *                                   returned.
 * @retval      PMIC_PARAMETER_ERROR If a NULL pointer argument was given.
 */
PMIC_STATUS PmicAudioGetPttButtonLevel(unsigned int *const level)
{
    UNREFERENCED_PARAMETER( level );

    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Provide a hexadecimal dump of all PMIC audio registers (DEBUG only)
 *
 * This function is intended strictly for debugging purposes only and will
 * print the current values of the following PMIC registers:
 *
 * - AUDIO_CODEC
 * - HIFI_CODEC
 * - RX0
 * - RX1
 * - AUDIO_TX
 * - SSI_NETWORK
 *
 * The register fields will also be decoded.
 *
 * Note that we don't dump any of the arbitration bits because we cannot
 * access the true arbitration bit settings when reading the registers
 * from the secondary SPI bus.
 */
void PmicAudioDumpRegisters(void)
{
#ifdef DEBUG

    //unsigned int reg_value = 0;

    DEBUGMSG(ZONE_INFO, (_T(">>> PMIC Audio Register Dump:\n")));

    //
    // ### TBD
    //

#endif /* DEBUG */
}

/*@}*/

/*!
 * @brief Set the clock source and operating characteristics.
 *
 * Define the clock source and operating characteristics. This
 * must be done before the CODEC is enabled. The CODEC will be
 * in a disabled state when this function returns.
 *
 * Note that we automatically perform a digital filter reset operation
 * when we change the clock configuration.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   clockIn         Select the clock signal source.
 * @param[in]   clockFreq       Select the clock signal frequency.
 * @param[in]   samplingRate    Select the audio data sampling rate.
 * @param[in]   invert          Enable inversion of the frame sync and/or
 *                              bit clock inputs.
 *
 * @retval      PMIC_SUCCESS         If the CODEC clock settings were
 *                                   successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or clock configuration was
 *                                   invalid.
 * @retval      PMIC_ERROR           If the CODEC clock configuration
 *                                   could not be set.
 */
PMIC_STATUS PmicAudioSetClock(
    const PMIC_AUDIO_HANDLE             handle,
    const PMIC_AUDIO_CLOCK_IN_SOURCE    clockIn,
    const PMIC_AUDIO_FREQ               clockFreq,
    const PMIC_AUDIO_SAMPLING_RATE      samplingRate,
    const PMIC_AUDIO_CLOCK_INVERT       invert)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;
    WM_STATUS           status;
    WM_AUDIO_INTERFACE  audioIF = WM_AUDIOIF_HIFI;
    WM_AUDIO_CLKINPUT   clkInput = WM_AUDIO_MCLK;
    WM_STREAM_ID        stream = WM_STREAM_HIFI_OUT;

    /*
     * Parameter validation.
     */
    if ( ( s_hifiCodec.handle != handle ) || ( HANDLE_IN_USE != s_hifiCodec.handleState ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: invalid handle 0x%X\r\n"),
                  _T(__FUNCTION__),
                  handle
                ));
        retval = PMIC_PARAMETER_ERROR;
        goto paramError;
    }
    switch ( clockIn )
    {
        case CLOCK_IN_DEFAULT:
        case CLOCK_IN_MCLK:
        case CLOCK_IN_SYSCLK:
            clkInput = WM_AUDIO_MCLK;
            break;
        case CLOCK_IN_32KREF:
            clkInput = WM_AUDIO_32KREF;
            break;
        case CLOCK_IN_DACLRC:
            clkInput = WM_AUDIO_DACLRCLK;
            break;
        case CLOCK_IN_ADCLRC:
            clkInput = WM_AUDIO_ADCLRCLK;
            break;
        default:
            DEBUGMSG( ZONE_ERROR, (
                      _T("%s: invalid clock source %d\r\n"),
                      _T(__FUNCTION__),
                      clockIn
                    ));
            retval = PMIC_PARAMETER_ERROR;
            goto paramError;
    }

    /* ### TBD */
    if ( invert )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: clock inversion not yet implemented\r\n"),
                  _T(__FUNCTION__),
                  handle
                ));
        retval = PMIC_NOT_SUPPORTED;
        goto paramError;
    }

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ( s_hifiCodec.handle == handle )
        stream = WM_STREAM_HIFI_OUT;
    else if ( s_voiceCodec.handle == handle )
        stream = WM_STREAM_HIFI_IN;

    status = WMAudioConfigureClocking( g_hWMAudioDevice,
                                       audioIF,
                                       clkInput,
                                       clockFreq
                                     );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: WMAudioConfigureClocking failed: 0x%0X\r\n"),
                  _T(__FUNCTION__),
                  status
                ));
        retval = WMStatusToPmicStatus( status );
        goto error;
    }

    status = WMAudioSetSampleRate( g_hWMAudioDevice,
                                   stream,
                                   samplingRate
                                 );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: WMAudioSetSampleRate failed: 0x%0X\r\n"),
                  _T(__FUNCTION__),
                  status
                ));
        retval = WMStatusToPmicStatus( status );
        goto error;
    }

    if ( s_hifiCodec.handle == handle )
    {
        s_hifiCodec.clockIn      = clockIn;
        s_hifiCodec.clockFreq    = clockFreq;
        s_hifiCodec.samplingRate = samplingRate;
        s_hifiCodec.invert       = invert;
    }

    retval = PMIC_SUCCESS;

    /* Exit the critical section. */
error:
    up(mutex);

paramError:
    return retval;
}

/*************************************************************************
 * General Voice CODEC configuration.
 *************************************************************************
 */

/*!
 * @name General Voice CODEC Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC Voice
 * CODEC hardware.
 */
/*@{*/

/*!
 * @brief Set the clock source and operating characteristics.
 *
 * Define the clock source and operating characteristics. This
 * must be done before the Voice CODEC is enabled. The Voice CODEC will be
 * in a disabled state when this function returns.
 *
 * Note that we automatically perform a digital filter reset operation
 * when we change the clock configuration.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   clockIn         Select the clock signal source.
 * @param[in]   clockFreq       Select the clock signal frequency.
 * @param[in]   samplingRate    Select the audio data sampling rate.
 * @param[in]   invert          Enable inversion of the frame sync and/or
 *                              bit clock inputs.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC clock settings were
 *                                   successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or clock configuration was
 *                                   invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC clock configuration
 *                                   could not be set.
 */
PMIC_STATUS PmicAudioVcodecSetClock(
    const PMIC_AUDIO_HANDLE                   handle,
    const PMIC_AUDIO_CLOCK_IN_SOURCE          clockIn,
    const PMIC_AUDIO_FREQ                     clockFreq,
    const PMIC_AUDIO_SAMPLING_RATE             samplingRate,
    const PMIC_AUDIO_CLOCK_INVERT             invert)
{
    PMIC_STATUS    retval;

    retval = PmicAudioSetClock( handle, clockIn, clockFreq, samplingRate, invert );
    if (retval == PMIC_SUCCESS)
    {
        s_voiceCodec.clockIn      = clockIn;
        s_voiceCodec.clockFreq    = clockFreq;
        s_voiceCodec.samplingRate = samplingRate;
        s_voiceCodec.invert       = invert;
    }

    return retval;
}

/*!
 * @brief Get the Voice CODEC clock source and operating characteristics.
 *
 * Get the current Voice CODEC clock source and operating characteristics.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  clockIn         The clock signal source.
 * @param[out]  clockFreq       The clock signal frequency.
 * @param[out]  samplingRate    The audio data sampling rate.
 * @param[out]  invert          Inversion of the frame sync and/or
 *                              bit clock inputs is enabled/disabled.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC clock settings were
 *                                   successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC clock configuration
 *                                   could not be retrieved.
 */
PMIC_STATUS PmicAudioVcodecGetClock(
    const PMIC_AUDIO_HANDLE                handle,
    PMIC_AUDIO_CLOCK_IN_SOURCE *const      clockIn,
    PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ *const clockFreq,
    PMIC_AUDIO_VCODEC_SAMPLING_RATE *const samplingRate,
    PMIC_AUDIO_CLOCK_INVERT *const         invert)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( clockIn );
    UNREFERENCED_PARAMETER( clockFreq );
    UNREFERENCED_PARAMETER( samplingRate );
    UNREFERENCED_PARAMETER( invert );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Set the Voice CODEC primary audio channel timeslot.
 *
 * Set the Voice CODEC primary audio channel timeslot. This function must be
 * used if the default timeslot for the primary audio channel is to be changed.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   timeslot        Select the primary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC primary audio channel
 *                                   timeslot was successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or audio channel timeslot
 *                                   was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC primary audio channel
 *                                   timeslot could not be set.
 */
PMIC_STATUS PmicAudioVcodecSetRxtxTimeslot(
    const PMIC_AUDIO_HANDLE          handle,
    const PMIC_AUDIO_VCODEC_TIMESLOT timeslot)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( timeslot );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current Voice CODEC primary audio channel timeslot.
 *
 * Get the current Voice CODEC primary audio channel timeslot.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  timeslot        The primary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC primary audio channel
 *                                   timeslot was successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC primary audio channel
 *                                   timeslot could not be retrieved.
 */
PMIC_STATUS PmicAudioVcodecGetRxtxTimeslot(
    const PMIC_AUDIO_HANDLE           handle,
    PMIC_AUDIO_VCODEC_TIMESLOT *const timeslot)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( timeslot );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Set the Voice CODEC secondary recording audio channel timeslot.
 *
 * Set the Voice CODEC secondary audio channel timeslot. This function must be
 * used if the default timeslot for the secondary audio channel is to be
 * changed. The secondary audio channel timeslot is used to transmit the audio
 * data that was recorded by the Voice CODEC from the secondary audio input
 * channel.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   timeslot        Select the secondary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC secondary audio channel
 *                                   timeslot was successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or audio channel timeslot
 *                                   was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC secondary audio channel
 *                                   timeslot could not be set.
 */
PMIC_STATUS PmicAudioVcodecSetSecondaryTxslot(
    const PMIC_AUDIO_HANDLE          handle,
    const PMIC_AUDIO_VCODEC_TIMESLOT timeslot)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( timeslot );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the Voice CODEC secondary recording audio channel timeslot.
 *
 * Get the Voice CODEC secondary audio channel timeslot.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  timeslot        The secondary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC secondary audio channel
 *                                   timeslot was successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC secondary audio channel
 *                                   timeslot could not be retrieved.
 */
PMIC_STATUS PmicAudioVcodecGetSecondaryTxslot(
    const PMIC_AUDIO_HANDLE           handle,
    PMIC_AUDIO_VCODEC_TIMESLOT *const timeslot)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( timeslot );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Set/Enable the Voice CODEC options.
 *
 * Set or enable various Voice CODEC options. The available options include
 * the use of dithering, highpass digital filters, and loopback modes.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   config          The Voice CODEC options to enable.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC options were
 *                                   successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or Voice CODEC options
 *                                   were invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC options could not be
 *                                   successfully set/enabled.
 */
PMIC_STATUS PmicAudioVcodecSetConfig(
    const PMIC_AUDIO_HANDLE        handle,
    const PMIC_AUDIO_VCODEC_CONFIG config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Clear/Disable the Voice CODEC options.
 *
 * Clear or disable various Voice CODEC options.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   config          The Voice CODEC options to be cleared/disabled.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC options were
 *                                   successfully cleared/disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or the Voice CODEC options
 *                                   were invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC options could not be
 *                                   cleared/disabled.
 */
PMIC_STATUS PmicAudioVcodecClearConfig(
    const PMIC_AUDIO_HANDLE        handle,
    const PMIC_AUDIO_VCODEC_CONFIG config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current Voice CODEC options.
 *
 * Get the current Voice CODEC options.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  config          The current set of Voice CODEC options.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC options were
 *                                   successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC options could not be
 *                                   retrieved.
 */
PMIC_STATUS PmicAudioVcodecGetConfig(
    const PMIC_AUDIO_HANDLE         handle,
    PMIC_AUDIO_VCODEC_CONFIG *const config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Enable the Voice CODEC bypass audio pathway.
 *
 * Enables the Voice CODEC bypass pathway for audio data. This allows direct
 * output of the voltages on the TX data bus line to the output amplifiers
 * (bypassing the digital-to-analog converters within the Voice CODEC).
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC bypass was successfully
 *                                   enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC bypass could not be
 *                                   enabled.
 */
PMIC_STATUS PmicAudioVcodecEnableBypass(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Disable the Voice CODEC bypass audio pathway.
 *
 * Disables the Voice CODEC bypass pathway for audio data. This means that
 * the TX data bus line will deliver digital data to the digital-to-analog
 * converters within the Voice CODEC.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC bypass was successfully
 *                                   disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC bypass could not be
 *                                   disabled.
 */
PMIC_STATUS PmicAudioVcodecDisableBypass(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    // No Voice codec on the WM8350
    return PMIC_NOT_SUPPORTED;
}

/*@}*/

/*************************************************************************
 * General Stereo DAC configuration.
 *************************************************************************
 */

/*!
 * @name General Stereo DAC Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC Stereo
 * DAC hardware.
 */
/*@{*/

/*!
 * @brief Set the Stereo DAC clock source and operating characteristics.
 *
 * Define the Stereo DAC clock source and operating characteristics. This
 * must be done before the Stereo DAC is enabled. The Stereo DAC will be
 * in a disabled state when this function returns.
 *
 * Note that we automatically perform a digital filter reset operation
 * when we change the clock configuration.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   clockIn         Select the clock signal source.
 * @param[in]   clockFreq       Select the clock signal frequency.
 * @param[in]   samplingRate    Select the audio data sampling rate.
 * @param[in]   invert          Enable inversion of the frame sync and/or
 *                              bit clock inputs.
 *
 * @retval      PMIC_SUCCESS         If the Stereo DAC clock settings were
 *                                   successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or clock configuration was
 *                                   invalid.
 * @retval      PMIC_ERROR           If the Stereo DAC clock configuration
 *                                   could not be set.
 */
PMIC_STATUS PmicAudioStdacSetClock(
    const PMIC_AUDIO_HANDLE             handle,
    const PMIC_AUDIO_CLOCK_IN_SOURCE    clockIn,
    const PMIC_AUDIO_FREQ               clockFreq,
    const PMIC_AUDIO_SAMPLING_RATE      samplingRate,
    const PMIC_AUDIO_CLOCK_INVERT       invert)
{
    PMIC_STATUS    retval;

    retval = PmicAudioSetClock( handle, clockIn, clockFreq, samplingRate, invert );
    if (retval == PMIC_SUCCESS)
    {
        s_hifiCodec.clockIn      = clockIn;
        s_hifiCodec.clockFreq    = clockFreq;
        s_hifiCodec.samplingRate = samplingRate;
        s_hifiCodec.invert       = invert;
    }

    return retval;
}

/*!
 * @brief Get the Stereo DAC clock source and operating characteristics.
 *
 * Get the current Stereo DAC clock source and operating characteristics.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  clockIn         The clock signal source.
 * @param[out]  clockFreq       The clock signal frequency.
 * @param[out]  samplingRate    The audio data sampling rate.
 * @param[out]  invert          Inversion of the frame sync and/or
 *                              bit clock inputs is enabled/disabled.
 *
 * @retval      PMIC_SUCCESS         If the Stereo DAC clock settings were
 *                                   successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle invalid.
 * @retval      PMIC_ERROR           If the Stereo DAC clock configuration
 *                                   could not be retrieved.
 */
PMIC_STATUS PmicAudioStdacGetClock(
    const PMIC_AUDIO_HANDLE            handle,
    PMIC_AUDIO_CLOCK_IN_SOURCE  *const clockIn,
    PMIC_AUDIO_FREQ             *const clockFreq,
    PMIC_AUDIO_SAMPLING_RATE    *const samplingRate,
    PMIC_AUDIO_CLOCK_INVERT     *const invert)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( clockIn );
    UNREFERENCED_PARAMETER( clockFreq );
    UNREFERENCED_PARAMETER( samplingRate );
    UNREFERENCED_PARAMETER( invert );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Set the Stereo DAC primary audio channel timeslot.
 *
 * Set the Stereo DAC primary audio channel timeslot. This function must be
 * used if the default timeslot for the primary audio channel is to be changed.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   timeslot        Select the primary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Stereo DAC primary audio channel
 *                                   timeslot was successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or audio channel timeslot
 *                                   was invalid.
 * @retval      PMIC_ERROR           If the Stereo DAC primary audio channel
 *                                   timeslot could not be set.
 */
PMIC_STATUS PmicAudioStdacSetRxTimeslot(
    const PMIC_AUDIO_HANDLE          handle,
    const PMIC_AUDIO_STDAC_TIMESLOTS timeslot)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( timeslot );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current Stereo DAC primary audio channel timeslot.
 *
 * Get the current Stereo DAC primary audio channel timeslot.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  timeslot        The primary audio channel timeslot.
 *
 * @retval      PMIC_SUCCESS         If the Stereo DAC primary audio channel
 *                                   timeslot was successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Stereo DAC primary audio channel
 *                                   timeslot could not be retrieved.
 */
PMIC_STATUS PmicAudioStdacGetRxTimeslot(
    const PMIC_AUDIO_HANDLE           handle,
    PMIC_AUDIO_STDAC_TIMESLOTS *const timeslot)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( timeslot );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Set/Enable the Stereo DAC options.
 *
 * Set or enable various Stereo DAC options. The available options include
 * resetting the digital filter and enabling the bus master clock outputs.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   config          The Stereo DAC options to enable.
 *
 * @retval      PMIC_SUCCESS         If the Stereo DAC options were
 *                                   successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or Stereo DAC options
 *                                   were invalid.
 * @retval      PMIC_ERROR           If the Stereo DAC options could not be
 *                                   successfully set/enabled.
 */
PMIC_STATUS PmicAudioStdacSetConfig(
    const PMIC_AUDIO_HANDLE       handle,
    const PMIC_AUDIO_STDAC_CONFIG config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Clear/Disable the Stereo DAC options.
 *
 * Clear or disable various Stereo DAC options.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   config          The Stereo DAC options to be cleared/disabled.
 *
 * @retval      PMIC_SUCCESS         If the Stereo DAC options were
 *                                   successfully cleared/disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or the Stereo DAC options
 *                                   were invalid.
 * @retval      PMIC_ERROR           If the Stereo DAC options could not be
 *                                   cleared/disabled.
 */
PMIC_STATUS PmicAudioStdacClearConfig(
    const PMIC_AUDIO_HANDLE       handle,
    const PMIC_AUDIO_STDAC_CONFIG config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current Stereo DAC options.
 *
 * Get the current Stereo DAC options.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  config          The current set of Stereo DAC options.
 *
 * @retval      PMIC_SUCCESS         If the Stereo DAC options were
 *                                   successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Stereo DAC options could not be
 *                                   retrieved.
 */
PMIC_STATUS PmicAudioStdacGetConfig(
    const PMIC_AUDIO_HANDLE        handle,
    PMIC_AUDIO_STDAC_CONFIG *const config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*@}*/

/*************************************************************************
 * Audio input section configuration.
 *************************************************************************
 */

/*!
 * @name Audio Input Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC audio
 * input hardware.
 */
/*@{*/

/*!
 * @brief Set/Enable the audio input section options.
 *
 * Set or enable various audio input section options. The only available
 * option right now is to enable the automatic disabling of the microphone
 * input amplifiers when a microphone/headset is inserted or removed.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   config          The audio input section options to enable.
 *
 * @retval      PMIC_SUCCESS         If the audio input section options were
 *                                   successfully configured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or audio input section
 *                                   options were invalid.
 * @retval      PMIC_ERROR           If the audio input section options could
 *                                   not be successfully set/enabled.
 */
PMIC_STATUS PmicAudioInputSetConfig(
    const PMIC_AUDIO_HANDLE       handle,
    const PMIC_AUDIO_INPUT_CONFIG config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Clear/Disable the audio input section options.
 *
 * Clear or disable various audio input section options.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   config          The audio input section options to be
 *                              cleared/disabled.
 *
 * @retval      PMIC_SUCCESS         If the audio input section options were
 *                                   successfully cleared/disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or the audio input section
 *                                   options were invalid.
 * @retval      PMIC_ERROR           If the audio input section options could
 *                                   not be cleared/disabled.
 */
PMIC_STATUS PmicAudioInputClearConfig(
    const PMIC_AUDIO_HANDLE       handle,
    const PMIC_AUDIO_INPUT_CONFIG config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current audio input section options.
 *
 * Get the current audio input section options.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  config          The current set of audio input section options.
 *
 * @retval      PMIC_SUCCESS         If the audio input section options were
 *                                   successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the audio input section options could
 *                                   not be retrieved.
 */
PMIC_STATUS PmicAudioInputGetConfig(
    const PMIC_AUDIO_HANDLE        handle,
    PMIC_AUDIO_INPUT_CONFIG *const config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*@}*/

/*************************************************************************
 * Audio recording using the Voice CODEC.
 *************************************************************************
 */

/*!
 * @name Audio Recording Using the Voice CODEC Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC Voice CODEC
 * to perform audio recording.
 */
/*@{*/

/*!
 * @brief Select the microphone inputs to be used for Voice CODEC recording.
 *
 * Select left (MC13783-only) and right microphone inputs for Voice CODEC
 * recording. It is possible to disable or not use a particular microphone
 * input channel by specifying NO_MIC as a parameter.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   leftChannel     Select the left microphone input channel.
 * @param[in]   rightChannel    Select the right microphone input channel.
 *
 * @retval      PMIC_SUCCESS         If the microphone input channels were
 *                                   successfully enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or microphone input ports
 *                                   were invalid.
 * @retval      PMIC_ERROR           If the microphone input channels could
 *                                   not be successfully enabled.
 */
PMIC_STATUS PmicAudioVcodecSetMic(
    const PMIC_AUDIO_HANDLE     handle,
    const PMIC_AUDIO_INPUT_PORT leftChannel,
    const PMIC_AUDIO_INPUT_PORT rightChannel)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( leftChannel );
    UNREFERENCED_PARAMETER( rightChannel );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current microphone inputs being used for Voice CODEC
 *        recording.
 *
 * Get the left (MC13783-only) and right microphone inputs currently being
 * used for Voice CODEC recording.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  leftChannel     The left microphone input channel.
 * @param[out]  rightChannel    The right microphone input channel.
 *
 * @retval      PMIC_SUCCESS         If the microphone input channels were
 *                                   successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the microphone input channels could
 *                                   not be retrieved.
 */
PMIC_STATUS PmicAudioVcodecGetMic(
    const PMIC_AUDIO_HANDLE      handle,
    PMIC_AUDIO_INPUT_PORT *const leftChannel,
    PMIC_AUDIO_INPUT_PORT *const rightChannel)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( leftChannel );
    UNREFERENCED_PARAMETER( rightChannel );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Enable/disable the microphone input.
 *
 * This function enables/disables the current microphone input channel. The
 * input amplifier is automatically turned off when the microphone input is
 * disabled.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[in]   leftChannel     The left microphone input channel state.
 * @param[in]   rightChannel    the right microphone input channel state.
 *
 * @retval      PMIC_SUCCESS         If the microphone input channels were
 *                                   successfully reconfigured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or microphone input states
 *                                   were invalid.
 * @retval      PMIC_ERROR           If the microphone input channels could
 *                                   not be reconfigured.
 */
PMIC_STATUS PmicAudioVcodecSetMicOnOff(
    const PMIC_AUDIO_HANDLE          handle,
    const PMIC_AUDIO_INPUT_MIC_STATE leftChannel,
    const PMIC_AUDIO_INPUT_MIC_STATE rightChannel)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( leftChannel );
    UNREFERENCED_PARAMETER( rightChannel );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Return the current state of the microphone inputs.
 *
 * This function returns the current state (on/off) of the microphone
 * input channels.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 * @param[out]  leftChannel     The current left microphone input channel
 *                              state.
 * @param[out]  rightChannel    the current right microphone input channel
 *                              state.
 *
 * @retval      PMIC_SUCCESS         If the microphone input channel states
 *                                   were successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the microphone input channel states
 *                                   could not be retrieved.
 */
PMIC_STATUS PmicAudioVcodecGetMicOnOff(
    const PMIC_AUDIO_HANDLE           handle,
    PMIC_AUDIO_INPUT_MIC_STATE *const leftChannel,
    PMIC_AUDIO_INPUT_MIC_STATE *const rightChannel)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( leftChannel );
    UNREFERENCED_PARAMETER( rightChannel );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Set the microphone input amplifier mode and gain level.
 *
 * This function sets the current microphone input amplifier operating mode
 * and gain level.
 *
 * @param[in]   handle           Device handle from pmic_audio_open() call.
 * @param[in]   leftChannelMode  The left microphone input amplifier mode.
 * @param[in]   leftChannelGain  The left microphone input amplifier gain level.
 * @param[in]   rightChannelMode The right microphone input amplifier mode.
 * @param[in]   rightChannelGain The right microphone input amplifier gain
 *                               level.
 *
 * @retval      PMIC_SUCCESS         If the microphone input amplifiers were
 *                                   successfully reconfigured.
 * @retval      PMIC_PARAMETER_ERROR If the handle or microphone input amplifier
 *                                   modes or gain levels were invalid.
 * @retval      PMIC_ERROR           If the microphone input amplifiers could
 *                                   not be reconfigured.
 */
PMIC_STATUS PmicAudioVcodecSetRecordGain(
    const PMIC_AUDIO_HANDLE       handle,
    const PMIC_AUDIO_MIC_AMP_MODE leftChannelMode,
    const PMIC_AUDIO_MIC_GAIN     leftChannelGain,
    const PMIC_AUDIO_MIC_AMP_MODE rightChannelMode,
    const PMIC_AUDIO_MIC_GAIN     rightChannelGain)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( leftChannelMode );
    UNREFERENCED_PARAMETER( leftChannelGain );
    UNREFERENCED_PARAMETER( rightChannelMode );
    UNREFERENCED_PARAMETER( rightChannelGain );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current microphone input amplifier mode and gain level.
 *
 * This function gets the current microphone input amplifier operating mode
 * and gain level.
 *
 * @param[in]   handle           Device handle from pmic_audio_open() call.
 * @param[out]  leftChannelMode  The left microphone input amplifier mode.
 * @param[out]  leftChannelGain  The left microphone input amplifier gain level.
 * @param[out]  rightChannelMode The right microphone input amplifier mode.
 * @param[out]  rightChannelGain The right microphone input amplifier gain
 *                               level.
 *
 * @retval      PMIC_SUCCESS         If the microphone input amplifier modes
 *                                   and gain levels were successfully
 *                                   retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the microphone input amplifier modes
 *                                   and gain levels could not be retrieved.
 */
PMIC_STATUS PmicAudioVcodecGetRecordGain(
    const PMIC_AUDIO_HANDLE        handle,
    PMIC_AUDIO_MIC_AMP_MODE *const leftChannelMode,
    PMIC_AUDIO_MIC_GAIN *const     leftChannelGain,
    PMIC_AUDIO_MIC_AMP_MODE *const rightChannelMode,
    PMIC_AUDIO_MIC_GAIN *const     rightChannelGain)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( leftChannelMode );
    UNREFERENCED_PARAMETER( leftChannelGain );
    UNREFERENCED_PARAMETER( rightChannelMode );
    UNREFERENCED_PARAMETER( rightChannelGain );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Enable a microphone bias circuit.
 *
 * This function enables one of the available microphone bias circuits.
 *
 * @param[in]   handle           Device handle from pmic_audio_open() call.
 * @param[in]   biasCircuit      The microphone bias circuit to be enabled.
 *
 * @retval      PMIC_SUCCESS         If the microphone bias circuit was
 *                                   successfully enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or selected microphone bias
 *                                   circuit was invalid.
 * @retval      PMIC_ERROR           If the microphone bias circuit could not
 *                                   be enabled.
 */
PMIC_STATUS PmicAudioVcodecEnableMicbias(
    const PMIC_AUDIO_HANDLE   handle,
    const PMIC_AUDIO_MIC_BIAS biasCircuit)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( biasCircuit );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Disable a microphone bias circuit.
 *
 * This function disables one of the available microphone bias circuits.
 *
 * @param[in]   handle           Device handle from pmic_audio_open() call.
 * @param[in]   biasCircuit      The microphone bias circuit to be disabled.
 *
 * @retval      PMIC_SUCCESS         If the microphone bias circuit was
 *                                   successfully disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or selected microphone bias
 *                                   circuit was invalid.
 * @retval      PMIC_ERROR           If the microphone bias circuit could not
 *                                   be disabled.
 */
PMIC_STATUS PmicAudioVcodecDisableMicbias(
    const PMIC_AUDIO_HANDLE   handle,
    const PMIC_AUDIO_MIC_BIAS biasCircuit)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( biasCircuit );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*@}*/

/*************************************************************************
 * Audio Playback Using the Voice CODEC.
 *************************************************************************
 */

/*!
 * @name Audio Playback Using the Voice CODEC Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC Voice CODEC
 * to perform audio playback.
 */
/*@{*/

/*!
 * @brief Configure and enable the Voice CODEC mixer.
 *
 * This function configures and enables the Voice CODEC mixer.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   rxSecondaryTimeslot The timeslot used for the secondary audio
 *                                  channel.
 * @param[in]   gainIn              The secondary audio channel gain level.
 * @param[in]   gainOut             The mixer output gain level.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC mixer was successfully
 *                                   configured and enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or mixer configuration
 *                                   was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC mixer could not be
 *                                   reconfigured or enabled.
 */
PMIC_STATUS PmicAudioVcodecEnableMixer(
    const PMIC_AUDIO_HANDLE              handle,
    const PMIC_AUDIO_VCODEC_TIMESLOT     rxSecondaryTimeslot,
    const PMIC_AUDIO_VCODEC_MIX_IN_GAIN  gainIn,
    const PMIC_AUDIO_VCODEC_MIX_OUT_GAIN gainOut)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( rxSecondaryTimeslot );
    UNREFERENCED_PARAMETER( gainIn );
    UNREFERENCED_PARAMETER( gainOut );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Disable the Voice CODEC mixer.
 *
 * This function disables the Voice CODEC mixer.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the Voice CODEC mixer was successfully
 *                                   disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Voice CODEC mixer could not be
 *                                   disabled.
 */
PMIC_STATUS PmicAudioVcodecDisableMixer(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    //
    // The WM8350 doesn't have a voice codec
    //
    return PMIC_NOT_SUPPORTED;
}

/*@}*/

/*************************************************************************
 * Audio Playback Using the Stereo DAC.
 *************************************************************************
 */

/*!
 * @name Audio Playback Using the Stereo DAC Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC Stereo DAC
 * to perform audio playback.
 */
/*@{*/

/*!
 * @brief Configure and enable the Stereo DAC mixer.
 *
 * This function configures and enables the Stereo DAC mixer.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   rxSecondaryTimeslot The timeslot used for the secondary audio
 *                                  channel.
 * @param[in]   gainIn              The secondary audio channel gain level.
 * @param[in]   gainOut             The mixer output gain level.
 *
 * @retval      PMIC_SUCCESS         If the Stereo DAC mixer was successfully
 *                                   configured and enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or mixer configuration
 *                                   was invalid.
 * @retval      PMIC_ERROR           If the Stereo DAC mixer could not be
 *                                   reconfigured or enabled.
 */
PMIC_STATUS PmicAudioStdacEnableMixer(
    const PMIC_AUDIO_HANDLE             handle,
    const PMIC_AUDIO_STDAC_TIMESLOTS    rxSecondaryTimeslot,
    const PMIC_AUDIO_STDAC_MIX_IN_GAIN  gainIn,
    const PMIC_AUDIO_STDAC_MIX_OUT_GAIN gainOut)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( rxSecondaryTimeslot );
    UNREFERENCED_PARAMETER( gainIn );
    UNREFERENCED_PARAMETER( gainOut );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Disable the Stereo DAC mixer.
 *
 * This function disables the Stereo DAC mixer.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the Stereo DAC mixer was successfully
 *                                   disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the Stereo DAC mixer could not be
 *                                   disabled.
 */
PMIC_STATUS PmicAudioStdacDisableMixer(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*@}*/

/*************************************************************************
 * Audio Output Control
 *************************************************************************
 */

/*!
 * @name Audio Output Section Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC audio output
 * section to support playback.
 */
/*@{*/

/*!
 * @brief Select the audio output ports.
 *
 * This function selects the audio output ports to be used. This also enables
 * the appropriate output amplifiers.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   port                The audio output ports to be used.
 *
 * @retval      PMIC_SUCCESS         If the audio output ports were successfully
 *                                   acquired.
 * @retval      PMIC_PARAMETER_ERROR If the handle or output ports were
 *                                   invalid.
 * @retval      PMIC_ERROR           If the audio output ports could not be
 *                                   acquired.
 */
PMIC_STATUS PmicAudioOutputSetPort(
    const PMIC_AUDIO_HANDLE      handle,
    const PMIC_AUDIO_OUTPUT_PORT ports)
{
    PMIC_STATUS         retval;
    WM_STATUS           status;
    //WM_AUDIO_CHANNELS   channels;
    PMIC_AUDIO_OUTPUT_PORT  newPorts;

    UNREFERENCED_PARAMETER( handle );

    if ( ports & (MONO_ALERT | MONO_EXTOUT | MONO_CDCOUT | STEREO_LEFT_LOW_POWER ) )
    {
        retval = PMIC_NOT_SUPPORTED;
        goto paramError;
    }

    newPorts = (PMIC_AUDIO_OUTPUT_PORT)(ports &~audioOutput.outputPorts);

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ( newPorts & (OUT1_LEFT|OUT1_RIGHT) )
    {
        status = WMAudioEnableSignal( g_hWMAudioDevice, WM_AUDIO_OUT1 );
        if ( !WM_SUCCESS( status ) )
            goto error;
    }
    if ( newPorts & (OUT2_LEFT|OUT2_RIGHT) )
    {
        status = WMAudioEnableSignal( g_hWMAudioDevice, WM_AUDIO_OUT2 );
        if ( !WM_SUCCESS( status ) )
            goto error;
    }

    if ( ports & (OUT1_LEFT|OUT1_RIGHT) )
    {
        status = WMAudioMuteSignal( g_hWMAudioDevice, WM_AUDIO_OUT1, FALSE );
        if ( WMS_UNSUPPORTED == status )
        {
            /* This signal uses a special value for mute */
            status = WMAudioSetSignalVolumesAdv( g_hWMAudioDevice,
                                                 WM_AUDIO_OUT1,
                                                 audioOutput.out1Gain,
                                                 audioOutput.out1Gain
                                               );
        }
        if ( !WM_SUCCESS( status ) )
            goto error;
    }
    if ( ports & (OUT2_LEFT|OUT2_RIGHT) )
    {
        status = WMAudioMuteSignal( g_hWMAudioDevice, WM_AUDIO_OUT2, FALSE );
        if ( WMS_UNSUPPORTED == status )
        {
            /* This signal uses a special value for mute */
            status = WMAudioSetSignalVolumesAdv( g_hWMAudioDevice,
                                                 WM_AUDIO_OUT2,
                                                 audioOutput.out2Gain,
                                                 audioOutput.out2Gain
                                               );
        }
        if ( !WM_SUCCESS( status ) )
            goto error;
    }

    audioOutput.outputPorts = (PMIC_AUDIO_OUTPUT_PORT)(audioOutput.outputPorts | ports);

    up( mutex );

    return PMIC_SUCCESS;

error:
    up( mutex );

    retval = WMStatusToPmicStatus( status );

paramError:
    return retval;
}

/*!
 * @brief Deselect/disable the audio output ports.
 *
 * This function disables the audio output ports that were previously enabled
 * by calling PmicAudioOutputSetPort().
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   port                The audio output ports to be disabled.
 *
 * @retval      PMIC_SUCCESS         If the audio output ports were successfully
 *                                   disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or output ports were
 *                                   invalid.
 * @retval      PMIC_ERROR           If the audio output ports could not be
 *                                   disabled.
 */
PMIC_STATUS PmicAudioOutputClearPort(
    const PMIC_AUDIO_HANDLE      handle,
    const PMIC_AUDIO_OUTPUT_PORT ports)
{
    PMIC_STATUS retval;
    WM_STATUS   status;

    UNREFERENCED_PARAMETER( handle );

    if ( ports & (MONO_ALERT | MONO_EXTOUT | MONO_CDCOUT | STEREO_LEFT_LOW_POWER ) )
    {
        retval = PMIC_NOT_SUPPORTED;
        goto paramError;
    }

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ( ports & (OUT1_LEFT|OUT1_RIGHT) )
    {
        status = WMAudioMuteSignal( g_hWMAudioDevice, WM_AUDIO_OUT1, TRUE );
        if ( !WM_SUCCESS( status ) )
            goto error;
    }
    if ( ports & (OUT2_LEFT|OUT2_RIGHT) )
    {
        status = WMAudioMuteSignal( g_hWMAudioDevice, WM_AUDIO_OUT2, TRUE );
        if ( !WM_SUCCESS( status ) )
            goto error;
    }

    audioOutput.outputPorts =
        PMIC_AUDIO_OUTPUT_PORT(audioOutput.outputPorts & ~ports);

    up( mutex );

    return PMIC_SUCCESS;

error:
    up( mutex );

    retval = WMStatusToPmicStatus( status );

paramError:
    return retval;
}

/*!
 * @brief Get the current audio output ports.
 *
 * This function retrieves the audio output ports that are currently being
 * used.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[out]  pPorts              The audio output ports currently being used.
 *
 * @retval      PMIC_SUCCESS         If the audio output ports were successfully
 *                                   retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the audio output ports could not be
 *                                   retrieved.
 */
PMIC_STATUS PmicAudioOutputGetPort(
    const PMIC_AUDIO_HANDLE       handle,
    PMIC_AUDIO_OUTPUT_PORT *const pPorts)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((((handle == s_hifiCodec.handle)       &&
          (s_hifiCodec.handleState == HANDLE_IN_USE))  ||
         ((handle == s_voiceCodec.handle)      &&
          (s_voiceCodec.handleState == HANDLE_IN_USE)) ||
         ((handle == extStereoIn.handle) &&
          (extStereoIn.handleState == HANDLE_IN_USE))) &&
        (pPorts != (PMIC_AUDIO_OUTPUT_PORT *)NULL))
    {
        *pPorts = audioOutput.outputPorts;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
}

/*!
 * @brief Set the gain level for the external stereo inputs.
 *
 * This function sets the gain levels for the external stereo inputs.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   gain                The external stereo input gain level.
 *
 * @retval      PMIC_SUCCESS         If the gain level was successfully set.
 * @retval      PMIC_PARAMETER_ERROR If the handle or gain level was invalid.
 * @retval      PMIC_ERROR           If the gain level could not be set.
 */
PMIC_STATUS PmicAudioOutputSetStereoInGain(
    const PMIC_AUDIO_HANDLE         handle,
    const PMIC_AUDIO_STEREO_IN_GAIN gain)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( gain );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current gain level for the external stereo inputs.
 *
 * This function retrieves the current gain levels for the external stereo
 * inputs.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[out]  gain                The current external stereo input gain
 *                                  level.
 *
 * @retval      PMIC_SUCCESS         If the gain level was successfully
 *                                   retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the gain level could not be retrieved.
 */
PMIC_STATUS PmicAudioOutputGetStereoInGain(
    const PMIC_AUDIO_HANDLE          handle,
    PMIC_AUDIO_STEREO_IN_GAIN *const gain)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( gain );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Set the output PGA gain level.
 *
 * This function sets the audio output PGA gain level.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   gain                The output PGA gain level.
 *
 * @retval      PMIC_SUCCESS         If the gain level was successfully set.
 * @retval      PMIC_PARAMETER_ERROR If the handle or gain level was invalid.
 * @retval      PMIC_ERROR           If the gain level could not be set.
 */
PMIC_STATUS PmicAudioOutputSetPgaGain(
    const PMIC_AUDIO_HANDLE          handle,
    const PMIC_AUDIO_OUTPUT_PGA_GAIN gain)
{
    WM_STATUS       status;
    PMIC_STATUS     retval;
    WM_AUDIO_SIGNAL signal;
    unsigned short  leftVol = LEFT_VOL( gain );
    unsigned short  rightVol = RIGHT_VOL( gain );

    /*
     * Parameter validation.
     */
    if ( ( s_hifiCodec.handle != handle ) || ( HANDLE_IN_USE != s_hifiCodec.handleState ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: invalid handle 0x%X\r\n"),
                  _T(__FUNCTION__),
                  handle
                ));
        retval = PMIC_PARAMETER_ERROR;
        goto paramError;
    }

    if ( s_hifiCodec.handle == handle )
        signal = WM_AUDIO_HIFI_DAC;
    else if ( s_voiceCodec.handle == handle )
        signal = WM_AUDIO_HIFI_ADC;
    else
    {
        retval = PMIC_PARAMETER_ERROR;
        goto paramError;
    }

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    status = WMAudioSetSignalVolumes( g_hWMAudioDevice,
                                      signal,
                                      leftVol,
                                      rightVol
                                    );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: WMAudioSetSignalVolumes failed: 0x%0X\r\n"),
                  _T(__FUNCTION__),
                  status
                ));
        retval = WMStatusToPmicStatus( status );
        goto error;
    }

    if (handle == s_hifiCodec.handle)
    {
        audioOutput.outputStDACPGAGain = gain;
    }
    else if (handle == s_voiceCodec.handle)
    {
        audioOutput.outputVCodecPGAGain = gain;
    }

    retval = PMIC_SUCCESS;

    /* Exit the critical section. */
error:
    up(mutex);

paramError:
    return retval;
}

/*!
 * @brief Get the output PGA gain level.
 *
 * This function retrieves the current audio output PGA gain level.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[out]  pGain               The current output PGA gain level.
 *
 * @retval      PMIC_SUCCESS         If the gain level was successfully
 *                                   retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the gain level could not be retrieved.
 */
PMIC_STATUS PmicAudioOutputGetPgaGain(
    const PMIC_AUDIO_HANDLE           handle,
    PMIC_AUDIO_OUTPUT_PGA_GAIN *const pGain)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if (pGain == (PMIC_AUDIO_OUTPUT_PGA_GAIN *)NULL)
    {
        retval = PMIC_PARAMETER_ERROR;
    }
    else if (((handle == s_hifiCodec.handle) && (s_hifiCodec.handleState == HANDLE_IN_USE)))
    {
        *pGain = audioOutput.outputStDACPGAGain;
        retval = PMIC_SUCCESS;
    }
    else if ((handle == s_voiceCodec.handle) && (s_voiceCodec.handleState == HANDLE_IN_USE))
    {
        *pGain = audioOutput.outputVCodecPGAGain;
        retval = PMIC_SUCCESS;
    }
    else if ((handle == extStereoIn.handle) &&
             (extStereoIn.handleState == HANDLE_IN_USE))
    {
        *pGain = audioOutput.outputExtStereoInPGAGain;
        retval = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return retval;
}

/*!
 * @brief Enable the output mixer.
 *
 * This function enables the output mixer for the audio stream that
 * corresponds to the current handle (i.e., the Voice CODEC, Stereo DAC, or
 * the external stereo inputs).
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the mixer was successfully enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the mixer could not be enabled.
 */
PMIC_STATUS PmicAudioOutputEnableMixer(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Disable the output mixer.
 *
 * This function disables the output mixer for the audio stream that
 * corresponds to the current handle (i.e., the Voice CODEC, Stereo DAC, or
 * the external stereo inputs).
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the mixer was successfully disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the mixer could not be disabled.
 */
PMIC_STATUS PmicAudioOutputDisableMixer(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Configure and enable the output balance amplifiers.
 *
 * This function configures and enables the output balance amplifiers.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   leftGain            The desired left channel gain level.
 * @param[in]   rightGain           The desired right channel gain level.
 *
 * @retval      PMIC_SUCCESS         If the output balance amplifiers were
 *                                   successfully configured and enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or gain levels were invalid.
 * @retval      PMIC_ERROR           If the output balance amplifiers could not
 *                                   be reconfigured or enabled.
 */
PMIC_STATUS PmicAudioOutputSetBalance(
    const PMIC_AUDIO_HANDLE              handle,
    const PMIC_AUDIO_OUTPUT_BALANCE_GAIN leftGain,
    const PMIC_AUDIO_OUTPUT_BALANCE_GAIN rightGain)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( leftGain );
    UNREFERENCED_PARAMETER( rightGain );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current output balance amplifier gain levels.
 *
 * This function retrieves the current output balance amplifier gain levels.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[out]  leftGain            The current left channel gain level.
 * @param[out]  rightGain           The current right channel gain level.
 *
 * @retval      PMIC_SUCCESS         If the output balance amplifier gain levels
 *                                   were successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the output balance amplifier gain levels
 *                                   could be retrieved.
 */
PMIC_STATUS PmicAudioOutputGetBalance(
    const PMIC_AUDIO_HANDLE               handle,
    PMIC_AUDIO_OUTPUT_BALANCE_GAIN *const leftGain,
    PMIC_AUDIO_OUTPUT_BALANCE_GAIN *const rightGain)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( leftGain );
    UNREFERENCED_PARAMETER( rightGain );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Configure and enable the output mono adder.
 *
 * This function configures and enables the output mono adder.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   mode                The desired mono adder operating mode.
 *
 * @retval      PMIC_SUCCESS         If the mono adder was successfully
 *                                   configured and enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle or mono adder mode was
 *                                   invalid.
 * @retval      PMIC_ERROR           If the mono adder could not be reconfigured
 *                                   or enabled.
 */
PMIC_STATUS PmicAudioOutputEnableMonoAdder(
    const PMIC_AUDIO_HANDLE    handle,
    const PMIC_AUDIO_MONO_ADDER_MODE mode)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( mode );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Disable the output mono adder.
 *
 * This function disables the output mono adder.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the mono adder was successfully
 *                                   disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the mono adder could not be disabled.
 */
PMIC_STATUS PmicAudioOutputDisableMonoAdder(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Configure the mono adder output gain level.
 *
 * This function configures the mono adder output amplifier gain level.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   gain                The desired output gain level.
 *
 * @retval      PMIC_SUCCESS         If the mono adder output amplifier gain
 *                                   level was successfully set.
 * @retval      PMIC_PARAMETER_ERROR If the handle or gain level was invalid.
 * @retval      PMIC_ERROR           If the mono adder output amplifier gain
 *                                   level could not be reconfigured.
 */
PMIC_STATUS PmicAudioOutputSetMonoAdderGain(
    const PMIC_AUDIO_HANDLE                 handle,
    const PMIC_AUDIO_MONO_ADDER_OUTPUT_GAIN gain)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( gain );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current mono adder output gain level.
 *
 * This function retrieves the current mono adder output amplifier gain level.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[out]  gain                The current output gain level.
 *
 * @retval      PMIC_SUCCESS         If the mono adder output amplifier gain
 *                                   level was successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the mono adder output amplifier gain
 *                                   level could not be retrieved.
 */
PMIC_STATUS PmicAudioOutputGetMonoAdderGain(
    const PMIC_AUDIO_HANDLE                  handle,
    PMIC_AUDIO_MONO_ADDER_OUTPUT_GAIN *const gain)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( gain );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Set various audio output section options.
 *
 * This function sets one or more audio output section configuration
 * options. The currently supported options include whether to disable
 * the non-inverting mono speaker output, enabling the loudspeaker common
 * bias circuit, enabling detection of headset insertion/removal, and
 * whether to automatically disable the headset amplifiers when a headset
 * insertion/removal has been detected.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   config              The desired audio output section
 *                                  configuration options to be set.
 *
 * @retval      PMIC_SUCCESS         If the desired configuration options were
 *                                   all successfully set.
 * @retval      PMIC_PARAMETER_ERROR If the handle or configuration options
 *                                   were invalid.
 * @retval      PMIC_ERROR           If the desired configuration options
 *                                   could not be set.
 */
PMIC_STATUS PmicAudioOutputSetConfig(
    const PMIC_AUDIO_HANDLE        handle,
    const PMIC_AUDIO_OUTPUT_CONFIG config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Clear various audio output section options.
 *
 * This function clears one or more audio output section configuration
 * options.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[in]   config              The desired audio output section
 *                                  configuration options to be cleared.
 *
 * @retval      PMIC_SUCCESS         If the desired configuration options were
 *                                   all successfully cleared.
 * @retval      PMIC_PARAMETER_ERROR If the handle or configuration options
 *                                   were invalid.
 * @retval      PMIC_ERROR           If the desired configuration options
 *                                   could not be cleared.
 */
PMIC_STATUS PmicAudioOutputClearConfig(
    const PMIC_AUDIO_HANDLE        handle,
    const PMIC_AUDIO_OUTPUT_CONFIG config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Get the current audio output section options.
 *
 * This function retrieves the current audio output section configuration
 * option settings.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[out]  config              The current audio output section
 *                                  configuration option settings.
 *
 * @retval      PMIC_SUCCESS         If the current configuration options were
 *                                   successfully retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the current configuration options
 *                                   could not be retrieved.
 */
PMIC_STATUS PmicAudioOutputGetConfig(
    const PMIC_AUDIO_HANDLE         handle,
    PMIC_AUDIO_OUTPUT_CONFIG *const config)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( config );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Enable the phantom ground circuit that is used to help identify
 *        the type of headset that has been inserted.
 *
 * This function enables the phantom ground circuit that is used to help
 * identify the type of headset (e.g., stereo or mono) that has been inserted.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the phantom ground circuit was
 *                                   successfully enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the phantom ground circuit could not
 *                                   be enabled.
 */
PMIC_STATUS PmicAudioOutputEnablePhantomGround(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Disable the phantom ground circuit that is used to help identify
 *        the type of headset that has been inserted.
 *
 * This function disables the phantom ground circuit that is used to help
 * identify the type of headset (e.g., stereo or mono) that has been inserted.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the phantom ground circuit was
 *                                   successfully disabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the phantom ground circuit could not
 *                                   be disabled.
 */
PMIC_STATUS PmicAudioOutputDisablePhantomGround(
    const PMIC_AUDIO_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Suspend the current thread for a timed interval.
 *
 * This function suspends the current thread for at least the period defined
 * by the argument (in milliseconds). Note that the actual delay interval
 * will typically be a little bit longer than that which is specified because
 * there is some additional processing that must be done to resume the thread.
 * Therefore, this function should only be used in situations where a high
 * resolution timer is not required. Fortunately, all of the audio functions
 * are of this nature.
 *
 * @param[in]   delay_ms        The desired delay interval in milliseconds.
 *
 * @retval      PMIC_SUCCESS    The delay has been successfully completed.
 * @retval      PMIC_ERROR      The delay was prematurely aborted.
 */
PMIC_STATUS PmicAudioTimedDelay(const unsigned long delay_ms)
{
    PMIC_STATUS retval = PMIC_SUCCESS;

    if (g_audioPowerdown)
    {
        /*! Use polling and the timeGetTime() API to implement the timed
         *  delay since we cannot use interrupts when performing a suspend
         *  or resume operation.
         *
         *  Note that timeGetTime() returns the current system time in
         *  milliseconds. This is good enough for implementing our timed
         *  delay interval.
         */
        DWORD waitEnd = timeGetTime() + (DWORD)delay_ms;

        /*! Just keep reading the system clock until the desired time
         *  interval has elapsed.
         */
        while (timeGetTime() < waitEnd)
            ; // Polling loop.
    }
    else
    {
        hTimedDelay = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hTimedDelay == NULL)
        {
            /*! Failed to create the required timed delay event object, we must
             *  immediately abort.
             */
            retval = PMIC_ERROR;
        }
        else
        {
            /*! Start by creating a timer event that will be set at the end of
             *  the desired delay interval.
             *
             *  Note that we must explicitly cast the handle to the timedDelay
             *  event object to be an LPTIMECALLBACK function pointer to avoid a
             *  compiler error. The timeSetEvent() function will actually
             *  interpret the value properly as a handle to an event object and
             *  not as a callback function pointer because we've also set the
             *  TIME_CALLBACK_EVENT_SET flag.
             */
            if (timeSetEvent((UINT)delay_ms, 1, (LPTIMECALLBACK)hTimedDelay,
                             NULL,
                             TIME_ONESHOT | TIME_CALLBACK_EVENT_SET) == NULL)
            {
                retval = PMIC_ERROR;
            }
            else
            {
                /*! Now suspend the thread until the timer event is
                 *  signalled.
                 */
                if (WaitForSingleObject(hTimedDelay, INFINITE) != WAIT_OBJECT_0)
                {
                    /*! Timed delay was prematurely aborted for some reason. */
                    retval = PMIC_ERROR;
                }
            }

            /*! Clean up by releasing the event handle. */
            CloseHandle(hTimedDelay);
            hTimedDelay = NULL;
        }
    }

    return retval;
}

/*!
 * @brief Called by the upper level audio driver when we are about powerdown.
 *
 * This function must be called by the power manager when we are about to
 * suspend or powerdown. PmicAudioPowerUp() must be called when we are ready
 * to resume normal operation.
 *
 */
void PmicAudioPowerDown(void)
{
    /* Set this flag so that other parts of the PMIC audio driver can bypass
     * any operations that should not be done during powerdown.
     */
    g_audioPowerdown = TRUE;

    /* Immediately terminate the current timed delay because we're about
     * to powerdown. This is required because the waiting thread is holding
     * a critical section that will also be needed by the powerdown handler.
     */
    if (hTimedDelay != NULL)
    {
        PulseEvent(hTimedDelay);
    }
}

/*!
 * @brief Called by the upper level audio driver when we are about powerup
 *        again following a previous call to PmicAudioPowerDown().
 *
 * This function must be called by the power manager when we are about to
 * resume or powerup again.
 *
 */
void PmicAudioPowerUp(void)
{
    /* Clear this flag now that we're returning to normal operation. */
    g_audioPowerdown = FALSE;
}

/*@}*/

/**************************************************************************
 * Static functions.
 **************************************************************************
 */

/*!
 * @name Audio Driver Internal Support Functions
 * These non-exported internal functions are used to support the functionality
 * of the exported audio APIs.
 */
/*@{*/

/*!
 * @brief Enter a critical section.
 *
 * This function must be called when attempting to enter a critical section.
 * It will suspend the current thread until the mutex is available. When the
 * mutex does become available, one of the currently waiting threads will
 * be awakened and will own the mutex until up() is called.
 *
 * @param[in]   hMutex          A handle to a mutex object.
 */
static inline void down_interruptible(HANDLE hMutex)
{
    WaitForSingleObject(hMutex, INFINITE);
}

/*!
 * @brief Leave a critical section.
 *
 * This function must be called when leaving a critical section that was
 * previously entered by calling down_interruptible().
 *
 * @param[in]   hMutex          A handle to a mutex object.
 */
static inline void up(HANDLE hMutex)
{
    ReleaseMutex(hMutex);
}

/*!
 * @brief Obtain a single sample from a specific ADC channel.
 *
 * @param[in]   channel         The ADC channel to be sampled.
 * @param[in]   adcResult       Pointer to a buffer for the ADC sample value.
 */
//static PMIC_STATUS pmic_adc_convert(const unsigned short channel,
//                                    const unsigned short *adcResult)
//{
//    return PmicADCGetSingleChannelOneSample((UINT16) channel,
//                                            (UINT16 *)adcResult);
//}

/*!
 * @brief Enables the 5.6V boost for the microphone bias 2 circuit.
 *
 * This function enables the switching regulator SW3 and configures it to
 * provide the 5.6V boost that is required for driving the microphone bias 2
 * circuit when using a 5-pole jack configuration (which is the case for the
 * Sphinx board).
 *
 * @retval      PMIC_SUCCESS         The 5.6V boost was successfully enabled.
 * @retval      PMIC_ERROR           Failed to enable the 5.6V boost.
 */
//static PMIC_STATUS pmic_audio_mic_boost_enable(void)
//{
//    PMIC_STATUS retval = PMIC_NOT_SUPPORTED;
//
//    /* This feature is not supported by the MC13783 PMIC. */
//
//    return retval;
//}

/*!
 * @brief Disables the 5.6V boost for the microphone bias 2 circuit.
 *
 * This function disables the switching regulator SW3 to turn off the 5.6V
 * boost for the microphone bias 2 circuit.
 *
 * @retval      PMIC_SUCCESS         The 5.6V boost was successfully disabled.
 * @retval      PMIC_ERROR           Failed to disable the 5.6V boost.
 */
//static PMIC_STATUS pmic_audio_mic_boost_disable(void)
//{
//    PMIC_STATUS retval = PMIC_NOT_SUPPORTED;
//
//    /* This feature is not supported by the MC13783 PMIC. */
//
//    return retval;
//}

/*!
 * @brief Free a device handle previously acquired by calling pmic_audio_open().
 *
 * Terminate further access to the PMIC audio hardware that was previously
 * acquired by calling pmic_audio_open(). This now allows another thread to
 * successfully call pmic_audio_open() to gain access.
 *
 * Note that we will shutdown/reset the Voice CODEC or Stereo DAC as well as
 * any associated audio input/output components that are no longer required.
 *
 * Also note that this function should only be called with the mutex already
 * acquired.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the close request was successful.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 */
static PMIC_STATUS pmic_audio_close_handle(const PMIC_AUDIO_HANDLE handle)
{
    PMIC_STATUS retval = PMIC_PARAMETER_ERROR;

    /* Match up the handle to the audio device and then close it. */
    if ((handle == s_hifiCodec.handle) &&
        (s_hifiCodec.handleState == HANDLE_IN_USE))
    {
        /* Also shutdown the Stereo DAC hardware. The simplest way to
         * do this is to simply call pmic_audio_reset_device() which will
         * restore the ST_DAC register to it's initial power-on state.
         *
         * This will also shutdown the audio output section if no one
         * else is still using it.
         */
        retval = pmic_audio_reset_device(s_hifiCodec.handle);

        if (retval == PMIC_SUCCESS)
        {
            s_hifiCodec.handle      = AUDIO_HANDLE_NULL;
            s_hifiCodec.handleState = HANDLE_FREE;
        }
    }

    return retval;
}

/*!
 * @brief Reset the selected audio hardware control registers to their
 *        power on state.
 *
 * This resets all of the audio hardware control registers currently
 * associated with the device handle back to their power on states. For
 * example, if the handle is associated with the Stereo DAC and a
 * specific output port and output amplifiers, then this function will
 * reset all of those components to their initial power on state.
 *
 * This function can only be called if the mutex has already been acquired.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the reset operation was successful.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 * @retval      PMIC_ERROR           If the reset was unsuccessful.
 */
static PMIC_STATUS pmic_audio_reset_device(
    const PMIC_AUDIO_HANDLE handle)
{
    WM_STATUS       status;
    UNREFERENCED_PARAMETER( handle );

    status = WMAudioReset( g_hWMAudioDevice );
    if ( WM_SUCCESS( status ) )
        return PMIC_SUCCESS;
    else
        return WMStatusToPmicStatus( status );
}

/**************************************************************************
 * Special device driver initialization/deinitialization functions.
 **************************************************************************
 */

/*!
 * @brief Non-trivial initialization of the PMIC audio driver global variables.
 *
 * This function must be called as part of DLL_PROCESS_ATTACH so that all non-
 * trivial initialization of any global variables can be done. This specifically
 * includes initialization of global variables that involve a function call. We
 * cannot make the function call directly in the declaration of the global
 * variable because that would require the use of CRT (C runtime) code to
 * actually make the function call. Since this code is not available when
 * creating a DLL, we must defer all such function calls to a separate device
 * driver initialization function such as the one we have here.
 *
 * Note that since DLL_PROCESS_ATTACH is guaranteed to be handled in a
 * serialized manner, we will only create a valid mutex object once.
 */
PMIC_STATUS PmicAudioDriverInit(void)
{
    /*! Create and initialize the mutex object to an unsignalled state. */
    if (mutex == NULL)
    {
        mutex = CreateMutex(NULL, FALSE, NULL);

        /*! Do not try to reset the PMIC audio hardware here because this
         *  function will be called for all processes that attach to the
         *  PMIC SDK DLL (this includes audio, touch, etc.) and we really
         *  don't want the audio hardware to be reset unless it is by the
         *  audio driver. Therefore, we'll just wait for the WAVEDEV audio
         *  driver to explicitly reset the PMIC audio hardware.
         */
    }

    return (mutex == NULL) ? PMIC_ERROR : PMIC_SUCCESS;
}

/*!
 * @brief Deinitialization of the PMIC audio driver global variables.
 *
 * This function must be called as part of DLL_PROCESS_DETACH.
 */
PMIC_STATUS PmicAudioDriverDeinit(void)
{
    if (mutex != NULL)
    {
        CloseHandle(mutex);
        mutex = NULL;

        /*! Do not try to reset the PMIC audio hardware here because this
         *  function will be called for all processes that attached to the
         *  PMIC SDK DLL (this includes audio, touch, etc.) and we really
         *  don't want the audio hardware to be reset unless it is by the
         *  audio driver. Therefore, we'll just depend on the WAVEDEV audio
         *  driver to explicitly reset the PMIC audio hardware.
         */
    }

    return PMIC_SUCCESS;
}

/*@}*/

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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
/*---------------------------------------------------------------------------
* Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
*--------------------------------------------------------------------------*/

#ifndef __PMIC_AUDIO_H__
#define __PMIC_AUDIO_H__

/*!
 * @defgroup PMIC_AUDIO PMIC Audio Driver
 * @ingroup PMIC_DRVRS
 */

/*!
 * @file pmic_audio.h
 * @brief External definitions for the PMIC Audio Client driver.
 *
 * The PMIC Audio driver and this API were developed to support the
 * audio playback, recording, and mixing capabilities of the power
 * management ICs that are available from Freescale Semiconductor, Inc.
 * In particular, both the MC13783 and SC55112 power management
 * ICs are currently supported. All of the common features of each
 * power management IC are available as well as access to all device-
 * specific features. However, attempting to use a device-specific
 * feature on a platform on which it is not supported will simply
 * return an error status.
 *
 * The following table shows which audio-related capabilities are supported
 * by each power management IC:
 *
 * @verbatim
       Operating Mode                 MC13783   SC55112
       -----------------------------  -------   -------
       Stereo DAC Playback             Yes        Yes
       Stereo DAC Input Mixing         Yes        Yes
       Voice CODEC Playback            Yes        Yes
       Voice CODEC Input Mixing        Yes        No
       Voice CODEC Mono Recording      Yes        Yes
       Voice CODEC Stereo Recording    Yes        No
       Sampling Rate and Clock Select  Yes        Yes
       Input Amplifier Control         Yes        Yes
       Microphone Bias Control         Yes        Yes
       Output Amplifier Control        Yes        Yes
       Output Mixing Control           Yes        Yes
       Data Bus Protocol Select        Yes        Yes
       Master/Slave Mode Select        Yes        Yes
       Anti Pop Bias Circuit Control   Yes        Yes
       Microphone/Headset Detect       Yes        Yes
       PTT Detect                      No         Yes
   @endverbatim
 *
 * Note that the Voice CODEC may also be referred to as the Telephone
 * CODEC in the power management IC DTS documentation. Also note that,
 * while the power management ICs do provide similar audio capabilities,
 * each PMIC may support additional configuration settings and features.
 * Therefore, it is highly recommended that the appropriate power
 * management IC DTS documents be used in conjunction with this API
 * interface. The following documents were used in the development
 * and testing of this API:
 *
 * [1] MC13783 Detailed Technical Specification (Level 3),
 *     Rev 1.3 - 04/09/17. Freescale Semiconductor, Inc.
 *
 * [2] iDEN Phoenix Platform Level 3 Detailed Technical Specification,
 *     Rev 2.2 11-03-2005. Freescale Semiconductor, Inc.
 *
 * @ingroup PMIC_AUDIO
 */

#include "pmic_status.h"

/***************************************************************************
 *                       TYPEDEFS AND ENUMERATIONS                         *
 ***************************************************************************/

/*!
 * @name General Setup and Audio Device Access Typedefs and Enumerations
 * Typedefs and enumerations that are used for initial access to the
 * PMIC Audio hardware.
 */
/*@{*/

/*!
 * @typedef PMIC_AUDIO_HANDLE
 * @brief Define typedef for a handle to the PMIC Audio hardware.
 *
 * Define a "handle" that is returned when the PMIC Audio hardware
 * is opened. This handle grants exclusive access to the PMIC Audio
 * hardware and must be used in all subsequent function calls. When access
 * to the PMIC Audio hardware is no longer required, then a close
 * operation must be done with this handle. The handle is no longer valid
 * if the close operation was successful.
 */
typedef long PMIC_AUDIO_HANDLE;

/*!
 * @enum PMIC_AUDIO_EVENTS
 * @brief Identify the audio events that have been detected and should be
 * handled.
 *
 * This enumeration defines all of the possible PMIC Audio events. Multiple
 * events may be selected when defining a mask and multiple events may be
 * signalled together.
 *
 * Note that the MICROPHONE_DETECT and MICROPHONE_REMOVED events may also be
 * used to signal the operation of a serial or parallel microphone switch
 * when used with a combined headset+microphone device. In that case the
 * HEADSET_DETECT state must also be checked to determine if it's only the
 * microphone switch being operated or whether the microphone has truly been
 * inserted/removed (along with the headset).
 */
typedef enum
{
    HEADSET_DETECTED         = 1,    /*!< Detected headset insertion.      */
    HEADSET_STEREO           = 2,    /*!< Detected stereo headset device.  */
    HEADSET_MONO             = 4,    /*!< Detected mono headset device.    */
    HEADSET_THERMAL_SHUTDOWN = 8,    /*!< Detected output amplifier
                                          shutdown due to thermal
                                          limits <b>(MC13783-only)</b>.    */
    HEADSET_SHORT_CIRCUIT    = 16,   /*!< Detected output amplifier
                                          short circuit condition
                                          <b>(MC13783-only)</b>.           */
    HEADSET_REMOVED          = 32,   /*!< Detected headset removal.        */
    MICROPHONE_DETECTED      = 64,   /*!< Detected microphone insertion.   */
    MICROPHONE_REMOVED       = 128,  /*!< Detected microphone removal.     */
    PTT_BUTTON_PRESS         = 256,  /*!< Detected PTT button down
                                          <b>(SC55112)</b>.                */
    PTT_BUTTON_RANGE         = 512,  /*!< Detected PTT button within
                                          voltage range
                                          <b>(SC55112)</b>.                */
    PTT_SHORT_OR_INVALID     = 1024  /*!< Detected PTT button outside
                                          of voltage range or invalid
                                          device <b>(SC55112)</b>.         */
} PMIC_AUDIO_EVENTS;

/*!
 * @typedef PMIC_AUDIO_CALLBACK
 * @brief Typedef for PMIC Audio event notification callback function.
 *
 * Define a typedef for the PMIC Audio event notification callback
 * function. The signalled events are passed to the function as the first
 * argument. The callback function should then process whatever events it
 * can and then return the set of unhandled events (if any).
 */
typedef PMIC_AUDIO_EVENTS (*PMIC_AUDIO_CALLBACK)(
    const PMIC_AUDIO_EVENTS event);

/*!
 * @enum PMIC_AUDIO_SOURCE
 * @brief Select an audio signal processing component.
 *
 * This enumeration defines all of the possible PMIC audio signal handling
 * components which can be acquired by calling pmic_audio_open().
 *
 * Note that the EXTERNAL_STEREO_IN selection is used to simply gain access
 * to the stereo input pins. The stereo input signal can then be routed
 * directly to the output amplifiers. In this case, no signal processing is
 * done by either the Voice CODEC or the Stereo DAC.
 */
typedef enum
{
    STEREO_DAC,           /*!< Open connection to Stereo DAC.             */
    VOICE_CODEC,          /*!< Open connection to Voice CODEC.            */
    EXTERNAL_STEREO_IN    /*!< Open connection to external stereo inputs. */
} PMIC_AUDIO_SOURCE;

/*@}*/

/*!
 * @name Data Bus Setup and Configuration Typedefs and Enumerations
 * Typedefs and enumerations that are used to define and configure
 * the data bus protocol in order to communicate with the Stereo DAC
 * or the Voice CODEC.
 */
/*@{*/

 /*
 * Note that the same data bus is used to transfer audio data to/from
 * the Voice CODEC and the Stereo DAC. However, in this case, the data bus
 * must be configured for network mode with different timeslots assigned to
 * the Voice CODEC and the Stereo DAC. Also, the sampling rates must be
 * identical for both the Voice CODEC and the Stereo DAC in order to avoid
 * a data bus timing conflict and audio signal distortion.
 */

/*!
 * @enum PMIC_AUDIO_BUS_PROTOCOL
 * @brief Select the data bus protocol to be used.
 *
 * This enumeration defines all of the possible PMIC audio data bus protocols
 * that may be selected.
 */
typedef enum
{
    NORMAL_MSB_JUSTIFIED_MODE,      /*!< Transmit and receive audio data
                                         in normal MSB-justified mode.        */
    NETWORK_MODE,                   /*!< Transmit and receive audio data
                                         in network mode.                     */
    I2S_MODE,                       /*!< Transmit and receive audio data
                                         in I2S mode.                         */
    SPD_IF_MODE                     /*!< Transmit and receive audio data
                                         in SPD/IF mode <b>(MC13783-only)</b>.*/
} PMIC_AUDIO_BUS_PROTOCOL;

/*!
 * @enum PMIC_AUDIO_BUS_MODE
 * @brief Select the data bus mode to be used.
 *
 * This enumeration defines all of the possible PMIC audio data bus modes
 * that may be selected. When configured in BUS_MASTER_MODE, the PMIC is
 * responsible for supplying the data bus clock signals. Alternatively,
 * when configured in BUS_SLAVE_MODE, the PMIC will use the data bus clock
 * signals that are supplied by the bus master.
 */
typedef enum
{
    BUS_MASTER_MODE = 0,            /*!< Operate as bus master.       */
    BUS_SLAVE_MODE  = 1             /*!< Operate as bus slave.        */
} PMIC_AUDIO_BUS_MODE;

/*!
 * @enum PMIC_AUDIO_CLOCK_IN_SOURCE
 * @brief Select the clock signal source.
 *
 * This enumeration defines all of the possible PMIC audio clock signal
 * sources that may be selected. One of these clock signal sources must
 * be selected in order to use either the Voice CODEC or the Stereo DAC.
 *
 * When configured in BUS_MASTER_MODE, the PMIC's onboard PLL circuits
 * will also be driven by the selected clock input signal.
 */
typedef enum
{
    CLOCK_IN_DEFAULT,    /*!< Just use default (power-up) clock input.      */
    CLOCK_IN_CLIA,       /*!< Use the CLIA clock source (Stereo DAC
                              default) <b>(MC13783-only)</b>.               */
    CLOCK_IN_CLIB,       /*!< Use the CLIB clock source (Voice CODEC
                              default) <b>(MC13783-only)</b>.               */
    CLOCK_IN_CLKIN,      /*!< Use the CLKIN clock source
                              <b>(SC55112)</b>.                             */
    CLOCK_IN_MCLK,       /*!< Disable the internal PLL and use the MCLK
                              clock source (Stereo DAC only)
                              <b>(SC55112)</b>.                             */
    CLOCK_IN_FSYNC,      /*!< Internal PLL input from external framesync
                              (Stereo DAC only).                            */
    CLOCK_IN_BITCLK      /*!< Internal PLL input from external bitclock
                              (Stereo DAC only).                            */
} PMIC_AUDIO_CLOCK_IN_SOURCE;

/*!
 * @enum PMIC_AUDIO_CLOCK_INVERT
 * @brief Select whether to invert the frame sync or bit clock signals.
 *
 * This enumeration enables or disables the inversion of the incoming
 * frame sync or bit clock signals.
 */
typedef enum
{
    NO_INVERT        = 0,       /*!< Do not invert the clock signals.    */
    INVERT_BITCLOCK  = 1,       /*!< Invert the BCLK input signal.       */
    INVERT_FRAMESYNC = 2        /*!< Invert the FSYNC input signal.      */
} PMIC_AUDIO_CLOCK_INVERT;

/*!
 * @enum PMIC_AUDIO_NUMSLOTS
 * @brief Select whether to invert the frame sync or bit clock signals.
 *
 * This enumeration defines all of the possible number of timeslots that may
 * be selected when the PMIC is configured as the data bus master. One of these
 * options must be selected if the Stereo DAC is to provide the data bus
 * clock signals.
 *
 * Note that the Voice CODEC currently only allows USE_4_TIMESLOTS when
 * operating in data bus master mode.
 */
typedef enum
{
    USE_2_TIMESLOTS,               /*!< Configure for 2 timeslots. */
    USE_4_TIMESLOTS,               /*!< Configure for 4 timeslots. */
    USE_8_TIMESLOTS                /*!< Configure for 8 timeslots. */
} PMIC_AUDIO_NUMSLOTS;

/*!
 * @enum PMIC_AUDIO_STDAC_SAMPLING_RATE
 * @brief Select the audio data sampling rate for the Stereo DAC.
 *
 * This enumeration defines all of the possible sampling rates currently
 * supported by the Stereo DAC. One of these sampling rates must be selected
 * and it must match that of the audio stream or else signal distortion will
 * occur.
 */
typedef enum
{
    STDAC_RATE_8_KHZ,               /*!< Use 8 kHz sampling rate.      */
    STDAC_RATE_11_025_KHZ,          /*!< Use 11.025 kHz sampling rate. */
    STDAC_RATE_12_KHZ,              /*!< Use 12 kHz sampling rate.     */
    STDAC_RATE_16_KHZ,              /*!< Use 16 kHz sampling rate.     */
    STDAC_RATE_22_050_KHZ,          /*!< Use 22.050 kHz sampling rate. */
    STDAC_RATE_24_KHZ,              /*!< Use 24 kHz sampling rate.     */
    STDAC_RATE_32_KHZ,              /*!< Use 32 kHz sampling rate.     */
    STDAC_RATE_44_1_KHZ,            /*!< Use 44.1 kHz sampling rate.   */
    STDAC_RATE_48_KHZ,              /*!< Use 48 kHz sampling rate.     */
    STDAC_RATE_64_KHZ,              /*!< Use 64 kHz sampling rate
                                         <b>(MC13783-only)</b>.        */
    STDAC_RATE_96_KHZ               /*!< Use 96 kHz sampling rate.
                                         <b>(MC13783-only)</b>.        */
} PMIC_AUDIO_STDAC_SAMPLING_RATE;

/*!
 * @enum PMIC_AUDIO_VCODEC_SAMPLING_RATE
 * @brief Select the audio data sampling rate for the Voice CODEC.
 *
 * This enumeration defines all of the possible sampling rates currently
 * supported by the Voice CODEC. One of these sampling rates must be selected
 * and it must match that of the audio stream or else signal distortion will
 * occur.
 */
typedef enum
{
    VCODEC_RATE_8_KHZ,               /*!< Use 8 kHz sampling rate.      */
    VCODEC_RATE_16_KHZ,              /*!< Use 16 kHz sampling rate.     */
} PMIC_AUDIO_VCODEC_SAMPLING_RATE;

/*!
 * @enum PMIC_AUDIO_ANTI_POP_RAMP_SPEED
 * @brief Select the anti-pop circuitry's ramp up speed.
 *
 * This enumeration defines all of the possible ramp up speeds for the
 * anti-pop circuitry. A slow ramp up speed may be required in order to
 * avoid the popping noise that is typically generated during the insertion
 * or removal of a headset or microphone.
 */
typedef enum
{
    ANTI_POP_RAMP_FAST,              /*!< Select fast ramp up.          */
    ANTI_POP_RAMP_SLOW               /*!< Select slow ramp up.          */
} PMIC_AUDIO_ANTI_POP_RAMP_SPEED;

/*@}*/

/*!
 * @name General Voice CODEC Configuration Typedefs and Enumerations
 * Typedefs and enumerations that are used to define and configure
 * the basic operating options for the Voice CODEC.
 */
/*@{*/

/*!
 * @enum PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ
 * @brief Select the Voice CODEC input clock frequency.
 *
 * This enumeration defines all of the supported Voice CODEC input clock
 * frequencies. One of these frequencies must be selected in order to
 * properly configure the Voice CODEC to operate at the required sampling
 * rate.
 */
typedef enum
{
    VCODEC_CLI_13MHZ,                   /*!< Clock frequency is 13MHz.    */
    VCODEC_CLI_15_36MHZ,                /*!< Clock frequency is 15.36MHz. */
    VCODEC_CLI_16_8MHZ,                 /*!< Clock frequency is 16.8MHz
                                             <b>(MC13783-only)</b>.       */
    VCODEC_CLI_26MHZ,                   /*!< Clock frequency is 26MHz.    */
    VCODEC_CLI_33_6MHZ,                 /*!< Clock frequency is 33.6MHz.  */
} PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ;

/*!
 * @enum PMIC_AUDIO_VCODEC_CONFIG
 * @brief Select the Voice CODEC configuration options.
 *
 * This enumeration is used to enable/disable each of the Voice CODEC options.
 * This includes the use of highpass digital filters and audio signal
 * loopback modes.
 *
 * Note that resetting the digital filters is now handled by the
 * pmic_audio_digital_filter_reset() API.
 */
typedef enum
{
    DITHERING                   = 1,    /*!< Enable/disable dithering.      */
    INPUT_HIGHPASS_FILTER       = 2,    /*!< Enable/disable the input high
                                             pass digital filter.           */
    OUTPUT_HIGHPASS_FILTER      = 4,    /*!< Enable/disable the output high
                                             pass digital filter.           */
    ANALOG_LOOPBACK             = 8,    /*!< Enable/disable the analog
                                             loopback path
                                             <b>(MC13783-only)</b>.         */
    DIGITAL_LOOPBACK            = 16,   /*!< Enable/disable the digital
                                             loopback path.                 */
    VCODEC_MASTER_CLOCK_OUTPUTS = 32,   /*!< Enable/disable the bus master
                                             clock outputs.                 */
    TRISTATE_TS                 = 64    /*!< Enable/disable FSYNC, BITCLK,
                                             and TX tristate.               */
} PMIC_AUDIO_VCODEC_CONFIG;

/*@}*/

/*!
 * @name General Stereo DAC Configuration Typedefs and Enumerations
 * Typedefs and enumerations that are used to define and configure
 * the basic operating options for the Stereo DAC.
 */
/*@{*/

/*!
 * @enum PMIC_AUDIO_STDAC_CLOCK_IN_FREQ
 * @brief Select the Stereo DAC input clock frequency.
 *
 * This enumeration defines all of the supported Stereo DAC input clock
 * frequencies. One of these frequencies must be selected in order to
 * properly configure the Stereo DAC to operate at the required sampling
 * rate.
 */
typedef enum
{
    STDAC_CLI_3_6864MHZ,        /*!< Clock frequency is 3.36864MHz
                                     <b>(MC13783-only)</b>.              */
    STDAC_CLI_12MHZ,            /*!< Clock frequency is 12MHz.
                                     <b>(MC13783-only)</b>.              */
    STDAC_CLI_13MHZ,            /*!< Clock frequency is 13MHz.           */
    STDAC_CLI_15_36MHZ,         /*!< Clock frequency is 15.36MHz.        */
    STDAC_CLI_16_8MHZ,          /*!< Clock frequency is 16.8MHz
                                     <b>(MC13783-only)</b>.              */
    STDAC_CLI_26MHZ,            /*!< Clock frequency is 26MHz.           */
    STDAC_CLI_33_6MHZ,          /*!< Clock frequency is 33.6MHz.         */
    STDAC_MCLK_PLL_DISABLED,    /*!< Use MCLK and disable internal PLL.  */
    STDAC_FSYNC_IN_PLL,         /*!< Use FSYNC as internal PLL input.    */
    STDAC_BCLK_IN_PLL           /*!< Use BCLK as internal PLL input.     */
} PMIC_AUDIO_STDAC_CLOCK_IN_FREQ;

/*!
 * @enum PMIC_AUDIO_STDAC_CONFIG
 * @brief Select the Stereo DAC configuration options.
 *
 * This enumeration is used to enable/disable each of the Stereo DAC options.
 */
typedef enum
{
    STDAC_MASTER_CLOCK_OUTPUTS = 1  /*!< Enable/disable the bus master clock
                                         outputs.                             */
} PMIC_AUDIO_STDAC_CONFIG;

/*@}*/

/*!
 * @name Voice CODEC Audio Port Mixing Typedefs and Enumerations
 * Typedefs and enumerations that are used for setting up the audio mixer
 * within the Voice CODEC.
 */
/*@{*/

/*!
 * @enum PMIC_AUDIO_VCODEC_TIMESLOT
 * @brief Select the Stereo DAC configuration options.
 *
 * This enumeration is used to select the timeslot for both the primary and
 * secondary (for MC13783-only) audio channels to the Voice CODEC.
 */
typedef enum
{
    USE_TS0,                     /*!< Use timeslot 0 for audio signal source
                                      <b>(MC13783-only)</b>.                 */
    USE_TS1,                     /*!< Use timeslot 1 for audio signal source
                                      <b>(MC13783-only)</b>.                 */
    USE_TS2,                     /*!< Use timeslot 2 for audio signal source
                                      <b>(MC13783-only)</b>.                 */
    USE_TS3                      /*!< Use timeslot 3 for audio signal source
                                      <b>(MC13783-only)</b>.                 */
} PMIC_AUDIO_VCODEC_TIMESLOT;

/*!
 * @enum PMIC_AUDIO_VCODEC_MIX_IN_GAIN
 * @brief Select the secondary channel input gain for the Voice CODEC mixer.
 *
 * This enumeration selects the secondary channel input gain for the Voice
 * CODEC mixer.
 */
typedef enum
{
    VCODEC_NO_MIX,                /*!< No audio mixing <b>(MC13783-only)</b>. */
    VCODEC_MIX_IN_0DB,            /*!< Mix with 0dB secondary channel gain
                                       <b>(MC13783-only)</b>.                 */
    VCODEC_MIX_IN_MINUS_6DB,      /*!< Mix with -6dB secondary channel gain
                                       <b>(MC13783-only)</b>.                 */
    VCODEC_MIX_IN_MINUS_12DB,     /*!< Mix with -12dB secondary channel gain
                                       <b>(MC13783-only)</b>.                 */
} PMIC_AUDIO_VCODEC_MIX_IN_GAIN;

/*!
 * @enum PMIC_AUDIO_VCODEC_MIX_OUT_GAIN
 * @brief Select the output gain for the Voice CODEC mixer.
 *
 * This enumeration selects the output gain for the Voice CODEC mixer.
 */
typedef enum
{
    VCODEC_MIX_OUT_0DB,           /*!< Select 0dB mixer output gain
                                       <b>(MC13783-only)</b>.                 */
    VCODEC_MIX_OUT_MINUS_6DB,     /*!< Select -6dB mixer output gain
                                       <b>(MC13783-only)</b>.                 */
} PMIC_AUDIO_VCODEC_MIX_OUT_GAIN;

/*@}*/

/*!
 * @name Stereo DAC Audio Port Mixing Typedefs and Enumerations
 * Typedefs and enumerations that are used for setting up the audio mixer
 * within the Stereo DAC.
 */
/*@{*/

/*!
 * @enum PMIC_AUDIO_STDAC_TIMESLOTS
 * @brief Select the timeslots used to transmit the left and right audio
 *        channels to the Stereo DAC.
 *
 * This enumeration is used to select the timeslots used to transmit the
 * data corresponding to the left and right audio channels to the Stereo
 * DAC.
 */
typedef enum
{
    USE_TS0_TS1,                 /*!< Use timeslots 0 and 1 for left and
                                      right channels, respectively.      */
    USE_TS2_TS3,                 /*!< Use timeslots 2 and 3 for left and
                                      right channels, respectively
                                      <b>(MC13783-only)</b>.             */
    USE_TS4_TS5,                 /*!< Use timeslots 4 and 5 for left and
                                      right channels, respectively
                                      <b>(MC13783-only)</b>.             */
    USE_TS6_TS7                  /*!< Use timeslots 6 and 7 for left and
                                      right channels, respectively
                                      <b>(MC13783-only)</b>.             */
} PMIC_AUDIO_STDAC_TIMESLOTS;

/*!
 * @enum PMIC_AUDIO_STDAC_MIX_IN_GAIN
 * @brief Select the secondary channel input gain for the Stereo DAC mixer.
 *
 * This enumeration is used to select the secondary channel input gain for
 * the Stereo DAC mixer.
 */
typedef enum
{
    STDAC_NO_MIX,                      /*!< No mixing, keep separate left
                                            and right audio channels.         */
    STDAC_MIX_IN_0DB,                  /*!< Mix left and right audio channels
                                            together with 0dB secondary
                                            channel gain.                     */
    STDAC_MIX_IN_MINUS_6DB,            /*!< Mix left and right audio channels
                                            together with -6dB secondary
                                            channel gain.                     */
    STDAC_MIX_IN_MINUS_12DB            /*!< Mix left and right audio channels
                                            together with -12dB secondary
                                            channel gain
                        <b>(MC13783-only)</b>.            */
} PMIC_AUDIO_STDAC_MIX_IN_GAIN;

/*!
 * @enum PMIC_AUDIO_STDAC_MIX_OUT_GAIN
 * @brief Select the output gain for the Stereo DAC mixer.
 *
 * This enumeration is used to select the output gain for the Stereo DAC
 * mixer.
 */
typedef enum
{
    STDAC_MIX_OUT_0DB,                 /*!< Select 0dB mixer output gain.     */
    STDAC_MIX_OUT_MINUS_6DB,           /*!< Select -6dB mixer output gain
                                            <b>(MC13783-only)</b>.              */
} PMIC_AUDIO_STDAC_MIX_OUT_GAIN;

/*@}*/

/*!
 * @name Microphone Input Typedefs and Enumerations
 * Typedefs and enumerations that are used for selecting and setting up
 * one or more or microphone inputs for recording.
 */
/*@{*/

/*!
 * @enum PMIC_AUDIO_MIC_BIAS
 * @brief Select the microphone bias circuit to be enabled/disabled.
 *
 * This enumeration lists all of the available microphone bias circuits that
 * may be enabled or disabled.
 */
typedef enum
{
    NO_BIAS   = 0,        /*!< No microphone bias circuit selected.      */
    MIC_BIAS1 = 1,        /*!< Enable/disable microphone bias 1 circuit. */
    MIC_BIAS2 = 2,        /*!< Enable/disable microphone bias 2 circuit. */
} PMIC_AUDIO_MIC_BIAS;

/*!
 * @enum PMIC_AUDIO_INPUT_PORT
 * @brief Select an audio input port for recording.
 *
 * This enumeration lists all of the available audio input ports that may
 * be selected for a recording operation.
 */
typedef enum
{
    NO_MIC,               /*!< No microphone input selected.               */
    MIC1_LEFT,            /*!< Enable left/mono channel microphone input
                               <b>(MC13783-only)</b>.                      */
    MIC1_RIGHT_MIC_MONO,  /*!< Enable right channel microphone input.      */
    MIC2_AUX,             /*!< Enable auxiliary microphone input.          */
    TXIN_EXT,             /*!< Enable external mono input.                 */
    RXIN_STEREO           /*!< Enable external stero input
                               <b>(MC13783-only)</b>.                      */
} PMIC_AUDIO_INPUT_PORT;

/*!
 * @enum PMIC_AUDIO_INPUT_MIC_STATE
 * @brief Control whether the input microphone is on/off.
 *
 * This enumeration allows the currently selected input microphone amplifier
 * to be turned on/off.
 */
typedef enum
{
    MICROPHONE_ON,        /*!< Turn microphone input on for recording. */
    MICROPHONE_OFF        /*!< Turn microphone input off (mute).       */
} PMIC_AUDIO_INPUT_MIC_STATE;

/*!
 * @enum PMIC_AUDIO_INPUT_CONFIG
 * @brief Enable/disable the audio input options.
 *
 * This enumeration allows for enabling/disabling any of the audio input
 * section options.
 */
typedef enum
{
    MIC_AMP_AUTO_DISABLE = 1,     /*!< Enable/disable automatic disabling of
                                       microphone input amplifiers following
                                       headset insertion/removal
                                       <b>(SC55112)</b>.                     */
    MIC2_BIAS_DETECT = 2          /*!< Enable/disable detection of the Bias2
                                       circuit out of regulation condition
                                       <b>(MC13783-only)</b>                 */
} PMIC_AUDIO_INPUT_CONFIG;

/*!
 * @enum PMIC_AUDIO_MIC_AMP_MODE
 * @brief Select the operating mode for the microphone amplifiers.
 *
 * This enumeration is used to select the operating mode for the microphone
 * amplifier.
 */
typedef enum
{
    AMP_OFF,                      /*!< Disable input amplifier.   */
    VOLTAGE_TO_VOLTAGE,           /*!< Operate input amplifier in
                                       voltage-to-voltage mode
                                       <b>(MC13783-only)</b>.     */
    CURRENT_TO_VOLTAGE            /*!< Operate input amplifier in
                                       current-to-voltage mode    */
} PMIC_AUDIO_MIC_AMP_MODE;

/*!
 * @enum PMIC_AUDIO_MIC_GAIN
 * @brief Select the microphone amplifier gain level.
 *
 * This enumeration lists all of the available microphone amplifier gain
 * levels.
 */
typedef enum
{
    MIC_GAIN_MINUS_8DB,          /*!< Select -8dB microphone amplifier gain
                                      <b>(MC13783-only)</b>.                 */
    MIC_GAIN_MINUS_7DB,          /*!< Select -7dB microphone amplifier gain
                                      <b>(MC13783-only)</b>.                 */
    MIC_GAIN_MINUS_6DB,          /*!< Select -6dB microphone amplifier gain
                                      <b>(MC13783-only)</b>.                 */
    MIC_GAIN_MINUS_5DB,          /*!< Select -5dB microphone amplifier gain
                                      <b>(MC13783-only)</b>.                 */
    MIC_GAIN_MINUS_4DB,          /*!< Select -4dB microphone amplifier gain
                                      <b>(MC13783-only)</b>.                 */
    MIC_GAIN_MINUS_3DB,          /*!< Select -3dB microphone amplifier gain
                                      <b>(MC13783-only)</b>.                 */
    MIC_GAIN_MINUS_2DB,          /*!< Select -2dB microphone amplifier gain
                                      <b>(MC13783-only)</b>.                 */
    MIC_GAIN_MINUS_1DB,          /*!< Select -1dB microphone amplifier gain
                                      <b>(MC13783-only)</b>.                 */
    MIC_GAIN_0DB,                /*!< Select 0dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_1DB,           /*!< Select 1dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_2DB,           /*!< Select 2dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_3DB,           /*!< Select 3dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_4DB,           /*!< Select 4dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_5DB,           /*!< Select 5dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_6DB,           /*!< Select 6dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_7DB,           /*!< Select 7dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_8DB,           /*!< Select 8dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_9DB,           /*!< Select 9dB microphone amplifier gain.  */
    MIC_GAIN_PLUS_10DB,          /*!< Select 10dB microphone amplifier gain. */
    MIC_GAIN_PLUS_11DB,          /*!< Select 11dB microphone amplifier gain. */
    MIC_GAIN_PLUS_12DB,          /*!< Select 12dB microphone amplifier gain. */
    MIC_GAIN_PLUS_13DB,          /*!< Select 13dB microphone amplifier gain. */
    MIC_GAIN_PLUS_14DB,          /*!< Select 14dB microphone amplifier gain. */
    MIC_GAIN_PLUS_15DB,          /*!< Select 15dB microphone amplifier gain. */
    MIC_GAIN_PLUS_16DB,          /*!< Select 16dB microphone amplifier gain. */
    MIC_GAIN_PLUS_17DB,          /*!< Select 17dB microphone amplifier gain. */
    MIC_GAIN_PLUS_18DB,          /*!< Select 18dB microphone amplifier gain. */
    MIC_GAIN_PLUS_19DB,          /*!< Select 19dB microphone amplifier gain. */
    MIC_GAIN_PLUS_20DB,          /*!< Select 20dB microphone amplifier gain. */
    MIC_GAIN_PLUS_21DB,          /*!< Select 21dB microphone amplifier gain. */
    MIC_GAIN_PLUS_22DB,          /*!< Select 22dB microphone amplifier gain. */
    MIC_GAIN_PLUS_23DB,          /*!< Select 23dB microphone amplifier gain. */
    MIC_GAIN_PLUS_24DB,          /*!< Select 24dB microphone amplifier gain
                                      <b>(SC55112)</b>.                      */
    MIC_GAIN_PLUS_25DB,          /*!< Select 25dB microphone amplifier gain
                                      <b>(SC55112)</b>.                      */
    MIC_GAIN_PLUS_26DB,          /*!< Select 26dB microphone amplifier gain
                                      <b>(SC55112)</b>.                      */
    MIC_GAIN_PLUS_27DB,          /*!< Select 27dB microphone amplifier gain
                                      <b>(SC55112)</b>.                      */
    MIC_GAIN_PLUS_28DB,          /*!< Select 28dB microphone amplifier gain
                                      <b>(SC55112)</b>.                      */
    MIC_GAIN_PLUS_29DB,          /*!< Select 29dB microphone amplifier gain
                                      <b>(SC55112)</b>.                      */
    MIC_GAIN_PLUS_30DB,          /*!< Select 30dB microphone amplifier gain
                                      <b>(SC55112)</b>.                      */
    MIC_GAIN_PLUS_31DB           /*!< Select 31dB microphone amplifier gain
                                      <b>(SC55112)</b>.                      */
} PMIC_AUDIO_MIC_GAIN;

/*@}*/

/*!
 * @name Audio Output Section Typedefs and Enumerations
 * Typedefs and enumerations that are used for selecting and setting up
 * one or more or audio output ports for playback.
 */
/*@{*/

/*!
 * @enum PMIC_AUDIO_OUTPUT_PORT
 * @brief Select the audio output port.
 *
 * This enumeration lists all of the available audio output ports. One or
 * more may be selected as desired to handle the output audio stream from
 * either the Voice CODEC or the Stereo DAC.
 */
typedef enum
{
    MONO_SPEAKER          = 1,   /*!< Select mono output speaker.            */
    MONO_LOUDSPEAKER      = 2,   /*!< Select mono loudspeaker
                                      <b>(MC13783-only)</b>.                 */
    MONO_ALERT            = 4,   /*!< Select mono alert output
                                      <b>(SC55112)</b>.                      */
    MONO_EXTOUT           = 8,   /*!< Select mono external output
                                      <b>(SC55112)</b>.                      */
    MONO_CDCOUT           = 16,  /*!< Select dedicated Voice CODEC output
                                      <b>(MC13783-only)</b>.                 */
    STEREO_LEFT_LOW_POWER = 32,  /*!< Select stereo left channel low power
                                      output <b>(MC13783-only)</b>.          */
    STEREO_HEADSET_LEFT   = 64,  /*!< Select stereo headset left channel.    */
    STEREO_HEADSET_RIGHT  = 128, /*!< Select stereo headset right channel.   */
    STEREO_OUT_LEFT       = 256, /*!< Select stereo external left channel
                                      output <b>(MC13783-only)</b>.          */
    STEREO_OUT_RIGHT      = 512  /*!< Select stereo external right channel
                                      output <b>(MC13783-only)</b>.          */
} PMIC_AUDIO_OUTPUT_PORT;

/*!
 * @enum PMIC_AUDIO_OUTPUT_CONFIG
 * @brief Enable/disable the audio output section options.
 *
 * This enumeration is used to enable/disable any of the audio output section
 * options.
 */
typedef enum
{
    MONO_SPEAKER_INVERT_OUT_ONLY    = 1, /*!< Enable/disable the non-inverted
                                              mono speaker output
                                              <b>(SC55112)</b>.               */
    MONO_LOUDSPEAKER_COMMON_BIAS    = 2, /*!< Enable/disable the loudspeaker
                                              output amplifier common bias
                                              <b>(MC13783-only)</b>.          */
    HEADSET_DETECT_ENABLE           = 4, /*!< Enable/disable headset
                                              insertion/removal detection
                                              <b>(MC13783-only)</b>.          */
    STEREO_HEADSET_AMP_AUTO_DISABLE = 8  /*!< Enable/disable automatic
                                              disabling of the stereo headset
                                              output amplifiers following
                                              headset insertion/removal.      */
} PMIC_AUDIO_OUTPUT_CONFIG;

/*!
 * @enum PMIC_AUDIO_STEREO_IN_GAIN
 * @brief Select the amplifier gain for the external stereo inputs.
 *
 * This enumeration is used to select the amplifier gain level to be used for
 * the external stereo inputs.
 */
typedef enum
{
    STEREO_IN_GAIN_0DB,         /*!< Select 0dB external stereo signal
                                     input gain.                        */
    STEREO_IN_GAIN_PLUS_18DB    /*!< Select 18dB external stereo signal
                                     input gain <b>(MC13783-only)</b>.  */
} PMIC_AUDIO_STEREO_IN_GAIN;

/*!
 * @enum PMIC_AUDIO_OUTPUT_PGA_GAIN
 * @brief Select the output PGA amplifier gain level.
 *
 * This enumeration is used to select the output PGA amplifier gain level.
 */
typedef enum
{
    OUTPGA_GAIN_MINUS_33DB,     /*!< Select -33dB output PGA gain
                                     <b>(MC13783-only)</b>.        */
    OUTPGA_GAIN_MINUS_30DB,     /*!< Select -30dB output PGA gain
                                     <b>(MC13783-only)</b>.        */
    OUTPGA_GAIN_MINUS_27DB,     /*!< Select -27dB output PGA gain
                                     <b>(MC13783-only)</b>.        */
    OUTPGA_GAIN_MINUS_24DB,     /*!< Select -24dB output PGA gain. */
    OUTPGA_GAIN_MINUS_21DB,     /*!< Select -21dB output PGA gain. */
    OUTPGA_GAIN_MINUS_18DB,     /*!< Select -18dB output PGA gain. */
    OUTPGA_GAIN_MINUS_15DB,     /*!< Select -15dB output PGA gain. */
    OUTPGA_GAIN_MINUS_12DB,     /*!< Select -12dB output PGA gain. */
    OUTPGA_GAIN_MINUS_9DB,      /*!< Select -9dB output PGA gain.  */
    OUTPGA_GAIN_MINUS_6DB,      /*!< Select -6dB output PGA gain.  */
    OUTPGA_GAIN_MINUS_3DB,      /*!< Select -3dB output PGA gain.  */
    OUTPGA_GAIN_0DB,            /*!< Select 0dB output PGA gain.   */
    OUTPGA_GAIN_PLUS_3DB,       /*!< Select 3dB output PGA gain.   */
    OUTPGA_GAIN_PLUS_6DB,       /*!< Select 6dB output PGA gain.   */
    OUTPGA_GAIN_PLUS_9DB,       /*!< Select 9dB output PGA gain.
                                     <b>(SC55112)</b>.             */
    OUTPGA_GAIN_PLUS_12DB,      /*!< Select 12dB output PGA gain
                                     <b>(SC55112)</b>.             */
    OUTPGA_GAIN_PLUS_15DB,      /*!< Select 15dB output PGA gain
                                     <b>(SC55112)</b>.             */
    OUTPGA_GAIN_PLUS_18DB,      /*!< Select 18dB output PGA gain
                                     <b>(SC55112)</b>.             */
    OUTPGA_GAIN_PLUS_21DB       /*!< Select 21dB output PGA gain
                                     <b>(SC55112)</b>.             */
} PMIC_AUDIO_OUTPUT_PGA_GAIN;

/*!
 * @enum PMIC_AUDIO_OUTPUT_BALANCE_GAIN
 * @brief Select the left/right channel balance gain level.
 *
 * This enumeration is used to select the balance gain level that is to be
 * separately applied to the left and right audio channels.
 */
typedef enum
{
    BAL_GAIN_MINUS_21DB,        /*!< Select -21dB channel balance
                                     gain <b>(MC13783-only)</b>.      */
    BAL_GAIN_MINUS_18DB,        /*!< Select -18dB channel balance
                                     gain <b>(MC13783-only)</b>.      */
    BAL_GAIN_MINUS_15DB,        /*!< Select -15dB channel balance
                                     gain <b>(MC13783-only)</b>.      */
    BAL_GAIN_MINUS_12DB,        /*!< Select -12dB channel balance
                                     gain <b>(MC13783-only)</b>.      */
    BAL_GAIN_MINUS_9DB,         /*!< Select -9dB channel balance
                                     gain <b>(MC13783-only)</b>.      */
    BAL_GAIN_MINUS_6DB,         /*!< Select -6dB channel balance
                                     gain <b>(MC13783-only)</b>.      */
    BAL_GAIN_MINUS_3DB,         /*!< Select -3dB channel balance
                                     gain <b>(MC13783-only)</b>.      */
    BAL_GAIN_0DB                /*!< Select 0dB channel balance gain. */
} PMIC_AUDIO_OUTPUT_BALANCE_GAIN;

/*!
 * @enum PMIC_AUDIO_MONO_ADDER_MODE
 * @brief Select the output mono adder operating mode.
 *
 * This enumeration is used to select the operating mode for the mono adder
 * in the audio output section.
 */
typedef enum
{
    MONO_ADDER_OFF,             /*!< Disable mono adder (keep separate
                                     left and right channels).           */
    MONO_ADD_LEFT_RIGHT,        /*!< Add left and right channels.        */
    MONO_ADD_OPPOSITE_PHASE,    /*!< Add left and right channels but
                                     with outputs in opposite phase
                                     <b>(MC13783-only)</b>.              */
    STEREO_OPPOSITE_PHASE       /*!< Keep separate left and right
                                     channels but invert phase of
                                     left channel <b>(MC13783-only)</b>. */
} PMIC_AUDIO_MONO_ADDER_MODE;

/*!
 * @enum PMIC_AUDIO_MONO_ADDER_OUTPUT_GAIN
 * @brief Select the mono adder output amplifier gain level.
 *
 * This enumeration is used to select the output amplifier gain level for
 * the mono adder.
 */
typedef enum
{
    MONOADD_GAIN_MINUS_6DB,     /*!< Select -6dB mono adder output gain
                                     <b>(SC55112)</b>.          */
    MONOADD_GAIN_MINUS_3DB,     /*!< Select -3dB mono adder output gain
                                     <b>(SC55112)</b>.          */
    MONOADD_GAIN_0DB            /*!< Select 0dB mono adder output gain. */
} PMIC_AUDIO_MONO_ADDER_OUTPUT_GAIN;

/*@}*/

/***************************************************************************
 *                       PMIC-SPECIFIC DEFINITIONS                         *
 ***************************************************************************/

/*!
 * @name Definition of PMIC-specific Capabilities
 * Constants that are used to define PMIC-specific capabilities.
 */
/*@{*/

/*!
 * Define the minimum Stereo DAC sampling rate (Hz).
 */
extern const unsigned MIN_STDAC_SAMPLING_RATE_HZ;
/*!
 * Define the maximum Stereo DAC sampling rate (Hz).
 */
extern const unsigned MAX_STDAC_SAMPLING_RATE_HZ;

/*@}*/

/***************************************************************************
 *                          PMIC API DEFINITIONS                           *
 ***************************************************************************/

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
 *                              Connectivity API calls.
 * @param[in]   device          The required PMIC audio hardware component.
 *
 * @retval      PMIC_SUCCESS         If the open request was successful
 * @retval      PMIC_PARAMETER_ERROR If the handle argument is NULL.
 * @retval      PMIC_ERROR           If the audio hardware component is
 *                                   unavailable.
 */
PMIC_STATUS PmicAudioOpen(
    PMIC_AUDIO_HANDLE *const handle,
    const PMIC_AUDIO_SOURCE  device);

/*!
 * @brief Terminate further access to the PMIC audio hardware.
 *
 * Terminate further access to the PMIC audio hardware that was previously
 * acquired by calling pmic_audio_open(). This now allows another thread to
 * successfully call pmic_audio_open() to gain access.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the close request was successful.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 */
PMIC_STATUS PmicAudioClose(
    const PMIC_AUDIO_HANDLE handle);

/*!
 * @brief Configure the data bus protocol to be used.
 *
 * Provide the parameters needed to properly configure the audio data bus
 * protocol so that data can be read/written to either the Stereo DAC or
 * the Voice CODEC.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
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
    const PMIC_AUDIO_BUS_PROTOCOL protocol,
    const PMIC_AUDIO_BUS_MODE     masterSlave,
    const PMIC_AUDIO_NUMSLOTS     numSlots);

/*!
 * @brief Retrieve the current data bus protocol configuration.
 *
 * Retrieve the parameters that define the current audio data bus protocol.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
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
    PMIC_AUDIO_BUS_PROTOCOL *const protocol,
    PMIC_AUDIO_BUS_MODE     *const masterSlave,
    PMIC_AUDIO_NUMSLOTS     *const numSlots);

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
PMIC_STATUS PmicAudioEnable(
    const PMIC_AUDIO_HANDLE handle);

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
PMIC_STATUS PmicAudioDisable(
    const PMIC_AUDIO_HANDLE handle);

/*!
 * @brief Reset the selected audio hardware control registers to their
 *        power on state.
 *
 * This resets all of the audio hardware control registers currently
 * associated with the device handle back to their power on states. For
 * example, if the handle is associated with the Stereo DAC and a
 * specific output port and output amplifiers, then this function will
 * reset all of those components to their power on state.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the reset operation was successful.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 * @retval      PMIC_ERROR           If the reset was unsuccessful.
 */
PMIC_STATUS PmicAudioReset(
    const PMIC_AUDIO_HANDLE handle);

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
PMIC_STATUS PmicAudioResetAll(void);

/*!
 * @brief Set the Audio callback function.
 *
 * Register a callback function that will be used to signal PMIC audio
 * events. For example, the OSS audio driver should register a callback
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
    const PMIC_AUDIO_EVENTS   eventMask);

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
    const PMIC_AUDIO_HANDLE handle);

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
    PMIC_AUDIO_EVENTS   *const eventMask);

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
    const PMIC_AUDIO_ANTI_POP_RAMP_SPEED rampSpeed);

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
PMIC_STATUS PmicAudioAntipopDisable(void);

/*!
 * @brief Performs a reset of the Voice CODEC/Stereo DAC digital filter.
 *
 * This function performs a reset of the digital filter using the back-to-back
 * SPI write procedure as documented in the SC55112 PMIC DTS document.
 *
 * @retval      PMIC_SUCCESS         If the digital filter was successfully
 *                                   reset.
 * @retval      PMIC_ERROR           If the digital filter could not be reset.
 */
PMIC_STATUS PmicAudioDigitalFilterReset(const PMIC_AUDIO_HANDLE handle);

/*!
 * @brief Get the most recent PTT button voltage reading.
 *
 * This function returns the most recent reading for the PTT button voltage.
 * The value may be used during the processing of the PTT_BUTTON_RANGE event
 * as part of the headset ID detection process.
 *
 * @retval      PMIC_SUCCESS         If the most recent PTT button voltage was
 *                                   returned.
 * @retval      PMIC_PARAMETER_ERROR If a NULL pointer argument was given.
 */
PMIC_STATUS PmicAudioGetPttButtonLevel(unsigned int *const level);

#ifdef DEBUG

/*!
 * @brief Provide a hexadecimal dump of all PMIC audio registers (DEBUG only).
 *
 * This function is intended strictly for debugging purposes only (i.e.,
 * the DEBUG macro must be defined) and will print the current values of the
 * following PMIC registers:
 *
 * - AUD_CODEC (Voice CODEC state)
 * - ST_DAC (Stereo DAC state)
 * - RX_AUD_AMPS (audio input section state)
 * - TX_AUD_AMPS (audio output section state)
 *
 * The register fields will also be decoded.
 */
void PmicAudioDumpRegisters(void);

#endif /* DEBUG */

/*@}*/

/*!
 * @name General Voice CODEC Setup and Configuration APIs
 * Functions for general setup and configuration of the PMIC Voice
 * CODEC hardware.
 */
/*@{*/

/*!
 * @brief Set the Voice CODEC clock source and operating characteristics.
 *
 * Define the Voice CODEC clock source and operating characteristics. This
 * must be done before the Voice CODEC is enabled.
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
    const PMIC_AUDIO_HANDLE               handle,
    const PMIC_AUDIO_CLOCK_IN_SOURCE      clockIn,
    const PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ clockFreq,
    const PMIC_AUDIO_VCODEC_SAMPLING_RATE samplingRate,
    const PMIC_AUDIO_CLOCK_INVERT         invert);

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
    PMIC_AUDIO_CLOCK_IN_SOURCE      *const clockIn,
    PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ *const clockFreq,
    PMIC_AUDIO_VCODEC_SAMPLING_RATE *const samplingRate,
    PMIC_AUDIO_CLOCK_INVERT         *const invert);

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
    const PMIC_AUDIO_VCODEC_TIMESLOT timeslot);

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
    PMIC_AUDIO_VCODEC_TIMESLOT *const timeslot);

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
    const PMIC_AUDIO_VCODEC_TIMESLOT timeslot);

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
    PMIC_AUDIO_VCODEC_TIMESLOT *const timeslot);

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
    const PMIC_AUDIO_VCODEC_CONFIG config);

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
    const PMIC_AUDIO_VCODEC_CONFIG config);

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
    PMIC_AUDIO_VCODEC_CONFIG *const config);

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
    const PMIC_AUDIO_HANDLE handle);

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
    const PMIC_AUDIO_HANDLE handle);

/*@}*/

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
 * must be done before the Stereo DAC is enabled.
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
    const PMIC_AUDIO_HANDLE              handle,
    const PMIC_AUDIO_CLOCK_IN_SOURCE     clockIn,
    const PMIC_AUDIO_STDAC_CLOCK_IN_FREQ clockFreq,
    const PMIC_AUDIO_STDAC_SAMPLING_RATE samplingRate,
    const PMIC_AUDIO_CLOCK_INVERT        invert);

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
    const PMIC_AUDIO_HANDLE               handle,
    PMIC_AUDIO_CLOCK_IN_SOURCE     *const clockIn,
    PMIC_AUDIO_STDAC_SAMPLING_RATE *const samplingRate,
    PMIC_AUDIO_STDAC_CLOCK_IN_FREQ *const clockFreq,
    PMIC_AUDIO_CLOCK_INVERT        *const invert);

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
    const PMIC_AUDIO_STDAC_TIMESLOTS timeslot);

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
    PMIC_AUDIO_STDAC_TIMESLOTS *const timeslot);

/*!
 * @brief Set/Enable the Stereo DAC options.
 *
 * Set or enable various Stereo DAC options. The available options include
 * enabling/disabling the bus master clock outputs.
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
    const PMIC_AUDIO_STDAC_CONFIG config);

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
    const PMIC_AUDIO_STDAC_CONFIG config);

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
    PMIC_AUDIO_STDAC_CONFIG *const config);

/*@}*/

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
    const PMIC_AUDIO_INPUT_CONFIG config);

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
    const PMIC_AUDIO_INPUT_CONFIG config);

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
    PMIC_AUDIO_INPUT_CONFIG *const config);

/*@}*/

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
    const PMIC_AUDIO_INPUT_PORT rightChannel);

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
    PMIC_AUDIO_INPUT_PORT *const rightChannel);

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
    const PMIC_AUDIO_INPUT_MIC_STATE rightChannel);

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
    PMIC_AUDIO_INPUT_MIC_STATE *const rightChannel);

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
    const PMIC_AUDIO_MIC_GAIN     rightChannelGain);

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
    PMIC_AUDIO_MIC_GAIN     *const leftChannelGain,
    PMIC_AUDIO_MIC_AMP_MODE *const rightChannelMode,
    PMIC_AUDIO_MIC_GAIN     *const rightChannelGain);

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
    const PMIC_AUDIO_MIC_BIAS biasCircuit);

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
    const PMIC_AUDIO_MIC_BIAS biasCircuit);

/*@}*/

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
    const PMIC_AUDIO_VCODEC_MIX_OUT_GAIN gainOut);

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
    const PMIC_AUDIO_HANDLE handle);

/*@}*/

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
    const PMIC_AUDIO_STDAC_MIX_OUT_GAIN gainOut);

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
    const PMIC_AUDIO_HANDLE handle);

/*@}*/

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
    const PMIC_AUDIO_OUTPUT_PORT port);

/*!
 * @brief Deselect/disable the audio output ports.
 *
 * This function disables the audio output ports that were previously enabled
 * by calling pmic_audio_output_set_port().
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
    const PMIC_AUDIO_OUTPUT_PORT port);

/*!
 * @brief Get the current audio output ports.
 *
 * This function retrieves the audio output ports that are currently being
 * used.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[out]  port                The audio output ports currently being used.
 *
 * @retval      PMIC_SUCCESS         If the audio output ports were successfully
 *                                   retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the audio output ports could not be
 *                                   retrieved.
 */
PMIC_STATUS PmicAudioOutputGetPort(
    const PMIC_AUDIO_HANDLE       handle,
    PMIC_AUDIO_OUTPUT_PORT *const port);

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
    const PMIC_AUDIO_STEREO_IN_GAIN gain);

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
    PMIC_AUDIO_STEREO_IN_GAIN *const gain);

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
    const PMIC_AUDIO_OUTPUT_PGA_GAIN gain);

/*!
 * @brief Get the output PGA gain level.
 *
 * This function retrieves the current audio output PGA gain level.
 *
 * @param[in]   handle              Device handle from pmic_audio_open() call.
 * @param[out]  gain                The current output PGA gain level.
 *
 * @retval      PMIC_SUCCESS         If the gain level was successfully
 *                                   retrieved.
 * @retval      PMIC_PARAMETER_ERROR If the handle was invalid.
 * @retval      PMIC_ERROR           If the gain level could not be retrieved.
 */
PMIC_STATUS PmicAudioOutputGetPgaGain(
    const PMIC_AUDIO_HANDLE           handle,
    PMIC_AUDIO_OUTPUT_PGA_GAIN *const gain);

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
    const PMIC_AUDIO_HANDLE handle);

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
    const PMIC_AUDIO_HANDLE handle);

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
    const PMIC_AUDIO_OUTPUT_BALANCE_GAIN rightGain);

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
    PMIC_AUDIO_OUTPUT_BALANCE_GAIN *const rightGain);

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
    const PMIC_AUDIO_MONO_ADDER_MODE mode);

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
    const PMIC_AUDIO_HANDLE handle);

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
    const PMIC_AUDIO_MONO_ADDER_OUTPUT_GAIN gain);

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
    PMIC_AUDIO_MONO_ADDER_OUTPUT_GAIN *const gain);

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
    const PMIC_AUDIO_OUTPUT_CONFIG config);

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
    const PMIC_AUDIO_OUTPUT_CONFIG config);

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
    PMIC_AUDIO_OUTPUT_CONFIG *const config);

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
    const PMIC_AUDIO_HANDLE handle);

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
    const PMIC_AUDIO_HANDLE handle);

PMIC_STATUS PmicAudioTimedDelay(const unsigned long delay_ms);

/*@}*/

/*!
 * @brief Initialize the PMIC audio driver.
 *
 * This function initializes all global variables, mutex handles, and the
 * PMIC hardware state when the PMIC device driver DLL is first loaded.
 *
 * @retval      PMIC_SUCCESS         If the driver initialization was successful
 * @retval      PMIC_ERROR           If the driver initialization failed.
 */
PMIC_STATUS PmicAudioDriverInit(void);

/*!
 * @brief Deinitialize the PMIC audio driver.
 *
 * This function deinitializes all global variables, mutex handles, and the
 * PMIC hardware state when the PMIC device driver DLL is unloaded.
 *
 * @retval      PMIC_SUCCESS         If the driver deinitialization was
 *                                   successful.
 * @retval      PMIC_ERROR           If the driver deinitialization failed.
 */
PMIC_STATUS PmicAudioDriverDeinit(void);

void PmicAudioPowerDown(void);
void PmicAudioPowerUp(void);

#endif /* __PMIC_AUDIO_H__ */

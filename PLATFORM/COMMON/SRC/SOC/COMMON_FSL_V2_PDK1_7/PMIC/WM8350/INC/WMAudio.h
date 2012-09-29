/*******************************************************************************
 *
 * Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
 *
 * This software as well as any related documentation may only be used or
 * copied in accordance with the terms of the Wolfson Microelectronics plc
 * agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
 *
 * The information in this file is furnished for informational use only,
 * is subject to change without notice, and should not be construed as a
 * commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
 * assumes no responsibility or liability for any errors or inaccuracies that
 * may appear in the software or any related documentation.
 *
 * Except as permitted by the agreement(s), no part of the software or any
 * related documentation may be reproduced, stored in a retrieval system, or
 * transmitted in any form or by any means without the express written
 * consent of Wolfson Microelectronics plc.
 *                                                                         *//**
 * @file   WMAudio.h
 * @brief  Audio functions for Wolfson devices.
 *
 * @version $Id: WMAudio.h 643 2007-06-15 22:11:56Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMAUDIO_H__
#define __WMAUDIO_H__

/*
 * Include files
 */
#include "WMStatus.h"
#include "WMTypes.h"

/*
 * Definitions
 */
#define WM_AUDIO        TRUE

/**
 * Stream IDs
 */
typedef unsigned short WM_STREAM_ID;

#define WM_STREAM_HIFI_OUT          1                   /**< HiFi playback */
#define WM_STREAM_HIFI_IN           2                   /**< HiFi record */
#define WM_STREAM_INVALID_STREAM    ((WM_STREAM_ID) -1)

/*
 * Various tests on the streams.
 */
#define WM_IS_VALID_STREAM( _stream )                       \
    ( WM_IS_INPUT_STREAM( _stream )                     ||  \
      WM_IS_OUTPUT_STREAM( _stream ) )
#define WM_IS_INPUT_STREAM( _stream )                       \
    ( WM_STREAM_HIFI_IN == _stream )
#define WM_IS_OUTPUT_STREAM( _stream )                      \
    ( WM_STREAM_HIFI_OUT == _stream )
#define WM_IS_HIFI_STREAM( _stream )                        \
    ( WM_STREAM_HIFI_IN == _stream                      ||  \
      WM_STREAM_HIFI_OUT == _stream )

/**
 * A stream handle type, returned from WMAudioOpenStream and passed into
 * all subsequent stream function calls.
 */
typedef WM_HANDLE   WM_STREAM_HANDLE;

/**
 * Frequencies and sample rates
 */
typedef unsigned int WM_AUDIO_FREQUENCY;
typedef WM_AUDIO_FREQUENCY WM_SAMPLE_RATE;
#define WM_SAMPLE_RATE_UNKNOWN      0
#define WM_SAMPLE_RATE_8K           8000    /* 0x1F40 */
#define WM_SAMPLE_RATE_11K025       11025   /* 0x2B11 */
#define WM_SAMPLE_RATE_12K          12000   /* 0x2EE0 */
#define WM_SAMPLE_RATE_16K          16000   /* 0x3E80 */
#define WM_SAMPLE_RATE_22K05        22050   /* 0x5622 */
#define WM_SAMPLE_RATE_24K          24000   /* 0x5DC0 */
#define WM_SAMPLE_RATE_32K          32000   /* 0x7D00 */
#define WM_SAMPLE_RATE_44K1         44100   /* 0xAC44 */
#define WM_SAMPLE_RATE_48K          48000   /* 0xBB80 */
#define WM_SAMPLE_RATE_88K2         88200   /* 0x15888 */
#define WM_SAMPLE_RATE_96K          96000   /* 0x17700 */

#define KHZ                         1000
#define MHZ                         1000*KHZ

/**
 * Audio signals for routing and gain control.
 */
#define WM_AUDIO_INPUT_BASE                0x0000    /* This value CANNOT change */
#define WM_AUDIO_ANALOGUE_INPUT_BASE    WM_AUDIO_INPUT_BASE
#define WM_AUDIO_DIGITAL_OUTPUT_BASE    0x20    /* This value CANNOT change */
#define WM_AUDIO_ANALOGUE_OUTPUT_BASE    0x30    /* This value CANNOT change */
#define WM_AUDIO_DIGITAL_INPUT_BASE        0x50    /* This value CANNOT change */
#define WM_AUDIO_MIXER_MUX_BASE            0x60    /* This value CANNOT change */
typedef enum tagWM_AUDIO_SIGNAL
{
    /* Analogue inputs - i.e. signal sources */
    WM_AUDIO_IN1                    = WM_AUDIO_ANALOGUE_INPUT_BASE+1,
    WM_AUDIO_IN2                    = WM_AUDIO_ANALOGUE_INPUT_BASE+2,
    WM_AUDIO_IN3                    = WM_AUDIO_ANALOGUE_INPUT_BASE+3,
    /* The xx_END values are the only ones allowed to change */
    WM_AUDIO_ANALOGUE_INPUT_END     = WM_AUDIO_IN3,

    /* Digital outputs - i.e. signal sources */
    WM_AUDIO_HIFI_DAC               = WM_AUDIO_DIGITAL_OUTPUT_BASE,
    WM_AUDIO_STEREO_DAC             = WM_AUDIO_HIFI_DAC,                    /* Synonym kept for backwards compatibility */
    /* The xx_END values are the only ones allowed to change */
    WM_AUDIO_DIGITAL_OUTPUT_END     = WM_AUDIO_HIFI_DAC,

    /* Analogue outputs - i.e. signal destinations */
    WM_AUDIO_OUT1                   = WM_AUDIO_ANALOGUE_OUTPUT_BASE+1,
    WM_AUDIO_OUT2                   = WM_AUDIO_ANALOGUE_OUTPUT_BASE+2,
    WM_AUDIO_OUT3                   = WM_AUDIO_ANALOGUE_OUTPUT_BASE+3,
    WM_AUDIO_OUT4                   = WM_AUDIO_ANALOGUE_OUTPUT_BASE+4,
    /* The xx_END values are the only ones allowed to change */
    WM_AUDIO_ANALOGUE_OUTPUT_END    = WM_AUDIO_OUT4,

   /* Digital input - i.e. signal destination */
    WM_AUDIO_HIFI_ADC               = WM_AUDIO_DIGITAL_INPUT_BASE,          /* Standard ADC gain range of 0dB to +22.5dB in 1.5dB steps */
    WM_AUDIO_STEREO_ADC             = WM_AUDIO_HIFI_ADC,                    /* Synonym kept for backwards compatibility */
    /* The xx_END values are the only ones allowed to change */
    WM_AUDIO_DIGITAL_INPUT_END      = WM_AUDIO_HIFI_ADC,

    /* Mixers and Muxes */
    WM_AUDIO_INPUT_MIXER            = WM_AUDIO_MIXER_MUX_BASE,
    WM_AUDIO_OUTPUT_MIXER           = WM_AUDIO_MIXER_MUX_BASE+1,
    WM_AUDIO_HEADPHONE_MIXER        = WM_AUDIO_MIXER_MUX_BASE+2,
    WM_AUDIO_HEADPHONE_MIXER_MONO   = WM_AUDIO_MIXER_MUX_BASE+3,
    WM_AUDIO_PHONE_MIXER            = WM_AUDIO_MIXER_MUX_BASE+4,
    WM_AUDIO_SPEAKER_MIXER          = WM_AUDIO_MIXER_MUX_BASE+5,
    WM_AUDIO_MONO_MIXER             = WM_AUDIO_MIXER_MUX_BASE+6,
    WM_AUDIO_FRONT_MIXER            = WM_AUDIO_MIXER_MUX_BASE+7,
    /* The xx_END values are the only ones allowed to change */
    WM_AUDIO_MIXER_MUX_END          = WM_AUDIO_FRONT_MIXER,

    /* A value for Don't care / Don't change signals */
    WM_AUDIO_IGNORE                 = 0xFFFE,

    /* A value for unknown signals */
    WM_AUDIO_SIGNAL_UNKNOWN         = 0xFFFF
} WM_AUDIO_SIGNAL;

/* Ensure that signal IDs do not overlap */
#if (WM_AUDIO_ANALOGUE_INPUT_BASE  >= WM_AUDIO_DIGITAL_OUTPUT_BASE  ||    \
     WM_AUDIO_DIGITAL_OUTPUT_BASE  >= WM_AUDIO_ANALOGUE_OUTPUT_BASE ||    \
     WM_AUDIO_ANALOGUE_OUTPUT_BASE >= WM_AUDIO_DIGITAL_INPUT_BASE   ||    \
     WM_AUDIO_DIGITAL_INPUT_BASE   >= WM_AUDIO_MIXER_MUX_BASE )

#error The Signal IDs overlap please correct

#endif /* Signal overlap check */


/* Check to see if the given signal is an ignorable signal */
#define WM_SIGNAL_IS_IGNORE( _signal ) (WM_AUDIO_IGNORE == (_signal) )

/* Check to see if the signal is a recordable signal */
#define WM_SIGNAL_IS_RECORDABLE( _signal )                                        \
                            ( (WM_AUDIO_ANALOGUE_INPUT_BASE <= (_signal) &&        \
                               WM_AUDIO_ANALOGUE_INPUT_END >= (_signal) ) ||    \
                              (WM_AUDIO_MIXER_MUX_BASE <= (_signal) &&            \
                               WM_AUDIO_MIXER_MUX_END >= (_signal) )            \
                            )

/* Check to see if the signal is a playable  output signal */
#define WM_SIGNAL_IS_OUTPUT( _signal )                                            \
                            (  WM_AUDIO_ANALOGUE_OUTPUT_BASE <= (_signal) &&    \
                               WM_AUDIO_ANALOGUE_OUTPUT_END >= (_signal)        \
                            )

/* Check to see if the signal is a valid signal */
#define WM_SIGNAL_IS_VALID( _signal )                                            \
                          ( (WM_AUDIO_ANALOGUE_INPUT_BASE <= (_signal) &&        \
                             WM_AUDIO_ANALOGUE_INPUT_END >= (_signal) ) ||        \
                            (WM_AUDIO_DIGITAL_INPUT_BASE <= (_signal) &&        \
                             WM_AUDIO_DIGITAL_INPUT_END >= (_signal) ) ||       \
                            (WM_AUDIO_MIXER_MUX_BASE <= (_signal) &&            \
                             WM_AUDIO_MIXER_MUX_END >= (_signal) ) ||             \
                            (WM_AUDIO_ANALOGUE_OUTPUT_BASE <= (_signal) &&      \
                             WM_AUDIO_ANALOGUE_OUTPUT_END >= (_signal) ) ||     \
                            (WM_AUDIO_DIGITAL_OUTPUT_BASE <= (_signal) &&       \
                             WM_AUDIO_DIGITAL_OUTPUT_END >= (_signal) ) ||      \
                            (WM_AUDIO_IGNORE == (_signal) )                        \
                          )

/**
 * Audio channel defines
 */
typedef unsigned int WM_AUDIO_CHANNELS;
/* The  audio channels that are available / understood */
#define WM_CHANNEL_NONE                 0x00
#define WM_CHANNEL_MONO                 0x01
#define WM_CHANNEL_LEFT                 0x02
#define WM_CHANNEL_RIGHT                0x04
#define WM_CHANNEL_STEREO               ( WM_CHANNEL_LEFT | WM_CHANNEL_RIGHT )
#define WM_CHANNEL_ALL                  0xFF

/**
 * Signal level definitions for the advanced functions which give precise
 * level control.
 */
#define WM_SIGNAL_LEVEL_1DB             16
#define WM_SIGNAL_LEVEL( _dB )          ((int)((_dB)*WM_SIGNAL_LEVEL_1DB))
#define WM_SIGNAL_LEVEL_AC97( _level )  ((int)((_level)*(int)(WM_SIGNAL_LEVEL_1DB)*1.5))
#define WM_SIGNAL_LEVEL_TO_DB( _level ) ((_level)/WM_SIGNAL_LEVEL_1DB)

/**
 * Interface definitions.
 *
 * An audio interface carries both ADC and DAC data.
 */
typedef enum tagWM_AUDIO_INTERFACE
{
    WM_AUDIOIF_NONE = 0,
    WM_AUDIOIF_HIFI = 1,
} WM_AUDIO_INTERFACE;

/** Interface format */
typedef enum tagWM_AUDIOIF_FORMAT
{
    WM_AUDIOIF_I2S = 1,
    WM_AUDIOIF_LEFT_JUSTIFY,
    WM_AUDIOIF_RIGHT_JUSTIFY,
    WM_AUDIOIF_DSP_LATE,    /* DSP mode A */
    WM_AUDIOIF_DSP_EARLY    /* DSP mode B */
} WM_AUDIOIF_FORMAT;

/** Interface word length */
typedef enum tagWM_AUDIOIF_WIDTH
{
    WM_AUDIOIF_16BIT = 16,
    WM_AUDIOIF_20BIT = 20,
    WM_AUDIOIF_24BIT = 24,
    WM_AUDIOIF_32BIT = 32
} WM_AUDIOIF_WIDTH;

/** Source for the raw clock input */
typedef enum WM_AUDIO_CLKINPUT
{
    WM_AUDIO_MCLK = 1,          /**< Clock from the MCLK pin */
    WM_AUDIO_32KREF,            /**< The 32kHz RTC clock */
    WM_AUDIO_ADCLRCLK,          /**< The ADC LRCLK */
    WM_AUDIO_DACLRCLK,          /**< The DAC LRCLK */
} WM_AUDIO_CLKINPUT;

/** Source for the internal SYSCLK */
typedef enum WM_AUDIO_CLKSRC
{
    WM_AUDIO_CLK_IN = 0,        /**< The raw input clock signal */
    WM_AUDIO_CLK_FLL,           /**< Generate SYSCLK from the FLL */
} WM_AUDIO_CLKSRC;

typedef enum WM_AUDIO_POWER
{
    WM_AUDIO_POWER_STANDBY = 1,
    WM_AUDIO_POWER_ACTIVE
} WM_AUDIO_POWER;

/** Various miscellaneous configuration options. */
typedef unsigned int WM_AUDIOIF_FLAGS;

#define WM_AUDIOIF_FLAGS_NONE           0x0000  /**< No flags set */
#define WM_AUDIOIF_INVERT_CLOCK         0x0001  /**< Invert BITCLK */
#define WM_AUDIOIF_INVERT_LR_CLOCK      0x0002  /**< Invert LRCLK */
#define WM_AUDIOIF_LR_SWAP              0x0004  /**< Swap left and right channels */
#define WM_AUDIOIF_MONO                 0x0008  /**< Configure as mono */
#define WM_AUDIOIF_TRISTATE             0x0010  /**< Tristate the interface */
#define WM_AUDIOIF_ENABLE               0x0020  /**< Enable the interface (don't tristate) */

/**
 * The value which defines the maximum amplitude of the waves generated by
 * WMAudioPlaySineWave and WMAudioPlaySquareWave.
 */
#define WM_AUDIO_MAX_AMPLITUDE      32768

/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WMAudioInit                                                    *//**
 *
 * @brief  Initialises the audio subsystem.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS             success.
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioInit( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMAudioShutdown                                               *//**
 *
 * @brief  Shuts down the audio subsystem.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS             success.
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioShutdown( WM_DEVICE_HANDLE hDevice );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioPowerUp
 *
 * Called to power up the audio-specific sections of the chip.
 *
 * Parameters:
 *      hDevice         handle to the device (from WMOpenDevice)
 *      powerSections   The sections to power up.
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
//WM_STATUS WMAudioPowerUp( WM_DEVICE_HANDLE   hDevice,
//                          WM_POWERFLAG       powerSections
//                        );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioPowerDown
 *
 * Called to power down the audio-specific sections of the chip.  Note if the
 * sections are still in use by another driver they will not actually be
 * powered down.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   powerSections   The sections to power down.
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
//WM_STATUS WMAudioPowerDown( WM_DEVICE_HANDLE hDevice,
//                            WM_POWERFLAG     powerSections
//                          );

/*******************************************************************************
 * Function: WMAudioReset                                                   *//**
 *
 * @brief   Resets the audio subsystem.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS             success.
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioReset( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMAudioConfigureInterface                                     *//**
 *
 * @brief   Configures the streaming format for the given interface.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   audioIF         the interface to configure
 * @param   isMaster        whether the codec masters the link
 * @param   format          the streaming format to use
 * @param   width           the number of bits per sample
 * @param   flags           extra configuration flags
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioConfigureInterface( WM_DEVICE_HANDLE       hDevice,
                                     WM_AUDIO_INTERFACE     audioIF,
                                     BOOL                   isMaster,
                                     WM_AUDIOIF_FORMAT      format,
                                     WM_AUDIOIF_WIDTH       width,
                                     WM_AUDIOIF_FLAGS       flags
                                   );

/*******************************************************************************
 * Function: WMAudioGetInterfaceConfig                                     *//**
 *
 * @brief   Returns the streaming format for the given interface.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   audioIF         the interface to query
 * @param   pIsMaster       returns whether the codec masters the link
 * @param   pFormat         returns the streaming format to use
 * @param   pWidth          returns the number of bits per sample
 * @param   pFlags          returns extra configuration flags
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioGetInterfaceConfig( WM_DEVICE_HANDLE       hDevice,
                                     WM_AUDIO_INTERFACE     audioIF,
                                     BOOL                   *pIsMaster,
                                     WM_AUDIOIF_FORMAT      *pFormat,
                                     WM_AUDIOIF_WIDTH       *pWidth,
                                     WM_AUDIOIF_FLAGS       *pFlags
                                   );

/*******************************************************************************
 * Function: WMAudioEnableInterface                                        *//**
 *
 * @brief   Enables the given interface (removes tristate).
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   audioIF         the interface to configure
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioEnableInterface( WM_DEVICE_HANDLE      hDevice,
                                  WM_AUDIO_INTERFACE    audioIF
                                );

/*******************************************************************************
 * Function: WMAudioDisableInterface                                        *//**
 *
 * @brief   Tristates the given interface.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   audioIF         the interface to disable
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioDisableInterface( WM_DEVICE_HANDLE     hDevice,
                                   WM_AUDIO_INTERFACE   audioIF
                                 );

/*******************************************************************************
 * Function: WMAudioIsInterfaceSupported                                   *//**
 *
 * @brief  Queries whether the given audio interface is supported.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   audioIF         The interface to check.
 *
 * @retval  TRUE            supported
 * @retval  FALSE           not supported
 ******************************************************************************/
BOOL WMAudioIsInterfaceSupported( WM_DEVICE_HANDLE      hDevice,
                                  WM_AUDIO_INTERFACE    audioIF
                                );

/*******************************************************************************
 * Function: WMAudioIsStreamSupported                                      *//**
 *
 * @brief  Queries whether the given audio stream is supported.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   stream          The stream to check.
 *
 * @retval  TRUE            supported
 * @retval  FALSE           not supported
 ******************************************************************************/
BOOL WMAudioIsStreamSupported( WM_DEVICE_HANDLE hDevice,
                               WM_STREAM_ID     stream
                             );

/*******************************************************************************
 * Function: WMAudioEnableStream                                           *//**
 *
 * @brief   Powers up and enables the given stream
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   stream          The stream to enable
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioEnableStream( WM_DEVICE_HANDLE hDevice,
                               WM_STREAM_ID     stream
                             );

/*******************************************************************************
 * Function: WMAudioDisableStream                                           *//**
 *
 * @brief   Powers down and disables the given stream
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   stream          The stream to disable
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioDisableStream( WM_DEVICE_HANDLE    hDevice,
                                WM_STREAM_ID        stream
                              );

/*******************************************************************************
 * Function: WMAudioConfigureClocking                                      *//**
 *
 * @brief  Configures the clocking scheme for the given stream in master mode.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   audioIF         The interface to configure.
 * @param   clockInput      The clock source.
 * @param   clockFreq       The frequency of the clock source.
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioConfigureClocking( WM_DEVICE_HANDLE    hDevice,
                                    WM_AUDIO_INTERFACE  audioIF,
                                    WM_AUDIO_CLKINPUT   clockInput,
                                    unsigned int        clockFreq
                                  );

/*******************************************************************************
 * Function: WMAudioSetSampleRate                                          *//**
 *
 * @brief  Sets the sample rate for the given stream.
 *
 * This function will configure the FLL/PLL as appropriate to generate the
 * requested sample rate off the clock source specified.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   stream          The stream for which to set the sample rate
 * @param   sampleRate      The new sample rate.
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioSetSampleRate( WM_DEVICE_HANDLE hDevice,
                                WM_STREAM_ID     stream,
                                WM_SAMPLE_RATE   sampleRate
                              );

/*******************************************************************************
 * Function: WMAudioGetSampleRate                                                              *//**
 *
 * @brief  Returns the sample rate for the given stream.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   stream          The stream to query.
 * @param   pSampleRate     Receives the sample rate.
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioGetSampleRate( WM_DEVICE_HANDLE hDevice,
                                WM_STREAM_ID     stream,
                                WM_SAMPLE_RATE   *pSampleRate
                              );

/*******************************************************************************
 * Function: WMAudioIsSampleRateSupported                                  *//**
 *
 * @brief   Returns whether the sample rate is supported by the given stream.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   stream          The stream for which to check the sample rate
 * @param   sampleRate      The sample rate to check.
 *
 * @retval  WMS_RETURN_TRUE     sample rate is supported.
 * @retval  WMS_RETURN_FALSE    sample rate is not supported.
 * @retval  WMS_UNSUPPORTED     stream is not supported on this device.
 ******************************************************************************/
WM_STATUS WMAudioIsSampleRateSupported( WM_DEVICE_HANDLE hDevice,
                                        WM_STREAM_ID     ifStream,
                                        WM_SAMPLE_RATE   sampleRate
                                      );

/*******************************************************************************
 * Function: WMAudioEnableReferences                                       *//**
 *
 * @brief   Powers up the audio references.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   powerLevel      The power level (active or standby).
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioEnableReferences( WM_DEVICE_HANDLE hDevice,
                                   WM_AUDIO_POWER   powerLevel
                                 );

/*******************************************************************************
 * Function: WMAudioDisableReferences                                      *//**
 *
 * @brief   Powers down the audio references.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioDisableReferences( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMAudioEnableSignal                                           *//**
 *
 * @brief   Powers up and enables the given signal
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   signal          The signal to enable
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioEnableSignal( WM_DEVICE_HANDLE hDevice,
                               WM_AUDIO_SIGNAL  signal
                             );

/*******************************************************************************
 * Function: WMAudioDisableSignal                                          *//**
 *
 * @brief   Disables  and powers down the given signal
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   signal          The signal to disable
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioDisableSignal( WM_DEVICE_HANDLE    hDevice,
                                WM_AUDIO_SIGNAL     signal
                              );

/*******************************************************************************
 * Function: WMAudioRouteSignal                                            *//**
 *
 * @brief   Routes the given signal through the given mixed signal.
 *
 * This function updates the mixer to send the given source signal through to
 * the destination.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   source          The signal input to the mixer.
 * @param   output          The signal output from the mixer.
 * @param   dB              The gain or attenuation to apply in dB.
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 *****************************************************************************/
WM_STATUS WMAudioRouteSignal( WM_DEVICE_HANDLE  hDevice,
                              WM_AUDIO_SIGNAL   source,
                              WM_AUDIO_SIGNAL   output,
                              int               volDb
                            );

/*******************************************************************************
 * Function: WMAudioMuteSignal                                             *//**
 *
 * @brief   Mutes or unmutes the signal.
 *
 * @param   signal          The signal to mute or unmute.
 * @param   mute            Mute if TRUE, unmute if FALSE.
 *
 * @retval  WMS_SUCCESS             success
 * @retval  WMS_INVALID_PARAMETER   invalid parameter
 * @retval  WMS_HW_ERROR            error communicating with device
 ******************************************************************************/
WM_STATUS WMAudioMuteSignal( WM_DEVICE_HANDLE   hDevice,
                             WM_AUDIO_SIGNAL    signal,
                             BOOL               mute
                           );

/*******************************************************************************
 * Function:    WMAudioIsSignalMuted                                       *//**
 *
 * @brief   Checks to see whether the signal is muted.
 *
 * @param   signal          The signal to check.
 *
 * @retval  WMS_RETURN_TRUE         the signal is muted.
 * @retval  WMS_RETURN_FALSE        the signal not muted.
 * @retval  WMS_UNSUPPORTED         the signal is not available on this device.
 * @retval  WMS_HW_ERROR            error communicating with device
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioIsSignalMuted( WM_DEVICE_HANDLE    hDevice,
                                WM_AUDIO_SIGNAL     signal
                              );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioIsSignalSupported
 *
 * Check to see if the signal is supported on this device.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to check.
 *
 * Returns:     WM_BOOL
 *      TRUE     - signal is supported
 *      FALSE    - signal is not supported
 *---------------------------------------------------------------------------*/
BOOL WMAudioIsSignalSupported( WM_DEVICE_HANDLE  hDevice,
                               WM_AUDIO_SIGNAL   signal
                             );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioSignalHasVolume
 *
 * Check to see if the signal is supported and has volume control on this device.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to check.
 *
 * Returns:     WM_BOOL
 *      TRUE     - signal volume control is supported
 *      FALSE    - signal volume control is not supported
 *---------------------------------------------------------------------------*/
BOOL WMAudioSignalHasVolume( WM_DEVICE_HANDLE  hDevice,
                             WM_AUDIO_SIGNAL   signal
                           );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioSignalHasMute
 *
 * Check to see if the signal is supported and has a mute on this device.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to check.
 *
 * Returns:     WM_BOOL
 *      TRUE     - signal mute is supported
 *      FALSE    - signal mute is not supported
 *---------------------------------------------------------------------------*/
BOOL WMAudioSignalHasMute( WM_DEVICE_HANDLE  hDevice,
                           WM_AUDIO_SIGNAL   signal
                         );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioSignalHasNoSeparateMute
 *
 * Check to see if the signal is supported and is muted with a special volume
 * on this device (as opposed to using a bitfield).
 *
 * If this is the case, the only way to unmute is to set the volume to a new
 * value, since there is no previous volume information held in the register.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to check.
 *
 * Returns:     WM_BOOL
 *      TRUE     - signal mute is via a special volume value
 *      FALSE    - signal is not supported or mute uses a separate bitfield
 *---------------------------------------------------------------------------*/
BOOL WMAudioSignalHasNoSeparateMute( WM_DEVICE_HANDLE  hDevice,
                                     WM_AUDIO_SIGNAL   signal
                                   );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioIsSignalStereo
 *
 * Check to see if the signal is supported and is a stereo signal.
 * Note this does not tell you that the signal is mono - it might not be
 * supported.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to check.
 *
 * Returns:     WM_BOOL
 *      TRUE             - signal is stereo
 *      FALSE            - signal is mono or not supported
 *---------------------------------------------------------------------------*/
BOOL WMAudioIsSignalStereo( WM_DEVICE_HANDLE  hDevice,
                            WM_AUDIO_SIGNAL   signal
                          );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioIsSignalPanned
 *
 * Check to see if the given signal's left and right channels are set to the
 * same gain.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to check.
 *
 * Returns:     WM_BOOL
 *      TRUE             - Left and Right gains are the different
 *      FALSE            - Left and Right gains are the same
 *---------------------------------------------------------------------------*/
BOOL WMAudioIsSignalPanned( WM_DEVICE_HANDLE hDevice,
                            WM_AUDIO_SIGNAL  signal
                          );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioIsSignalMono
 *
 * Check to see if the signal is supported and is a mono signal.
 * Note this does not tell you that the signal is stereo - it might not be
 * supported.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to check.
 *
 * Returns:     WM_BOOL
 *      TRUE             - signal is mono
 *      FALSE            - signal is stereo or not supported
 *---------------------------------------------------------------------------*/
BOOL WMAudioIsSignalMono( WM_DEVICE_HANDLE  hDevice,
                          WM_AUDIO_SIGNAL   signal
                        );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioGetSignalChannels
 *
 * Returns the channels which this signal supports.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to check.
 *        pChannels        Receives the channels.
 *
 * Returns:     WMSTATUS
 *      WMS_SUCCESS                - Succeeded.
 *         WMS_UNSUPPORTED            - Signal not supported on this device.
 *         WMS_NO_SUPPORTED_DEVICE - Device support not present
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioGetSignalChannels( WM_DEVICE_HANDLE  hDevice,
                                    WM_AUDIO_SIGNAL   signal,
                                    WM_AUDIO_CHANNELS *pChannels
                                  );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioMuteSignal
 *
 * Called to mute the signal.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to mute.
 *      mute            Mute if TRUE, unmute if FALSE.
 *
 * Note: This function will set and clear all the mute bits for the signal.
 *
 * Returns:     WMSTATUS
 *       See WMStatus.h.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioMuteSignal( WM_DEVICE_HANDLE   hDevice,
                             WM_AUDIO_SIGNAL    signal,
                             BOOL               mute
                           );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioIsSignalMuted
 *
 * Check to see if the signal is muted.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to check.
 *
 * Returns:     WMSTATUS
 *      WMS_RETURN_TRUE if the signal is muted.
 *      WMS_RETURN_FALSE if the signal not muted.
 *      WMS_UNSUPPORTED if the signal is not available on this device.
 *      WMS_NO_SUPPORTED_DEVICE if device support is not present.
 *      See WMStatus.h for all other values and meanings.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioIsSignalMuted( WM_DEVICE_HANDLE  hDevice,
                                WM_AUDIO_SIGNAL   signal
                             );

/*-----------------------------------------------------------------------------
 * Function: WMSignalName
 *
 * This function returns a textual representation of the signal - useful
 * for reporting errors.
 *
 * Parameters:
 *      signal      signal
 *
 * Returns:      const char *
 *      Textual representation of signal.
 ----------------------------------------------------------------------------*/
const TCHAR *WMSignalName( WM_AUDIO_SIGNAL signal );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioSetSignalVolumeDb
 *
 * Set the signal level relative to full-scale.
 *
 * Note: the underlying device may not use dB steps.  In this case, the applied
 * level will be rounded to the nearest available step.  For example, if the device
 * has a 1.5dB step size (typical of AC'97 devices), a level of 4 or 5 would both
 * result in a 4.5dBFS signal.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to set.
 *      dbVol           The relative amplification or attenuation to apply,
 *                      in decibels.
 *      channels        One or more of the WM_CHANNEL_XXX constants.
 *
 *
 * Returns:     WMSTATUS
 *      WMS_SUCCESS                 - success
 *      WMS_UNSUPPORTED             - signal is not supported, or doesn't
 *                                    support volume control
 *      WMS_NO_SUPPORTED_DEVICE     - device support not present
 *      See WMStatus.h for all other values and meanings.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioSetSignalVolumeDb( WM_DEVICE_HANDLE     hDevice,
                                    WM_AUDIO_SIGNAL      signal,
                                    int                  dbVol,
                                    WM_AUDIO_CHANNELS    channels
                                  );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioSetSignalVolumesDb
 *
 * Set the signal level relative to full-scale.
 *
 * Note: the underlying device may not use dB steps.  In this case, the applied
 * level will be rounded to the nearest available step.  For example, if the device
 * has a 1.5dB step size (typical of AC'97 devices), a level of 4 or 5 would both
 * result in a 4.5dBFS signal.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to set.
 *      leftVol         The relative amplification or attenuation to apply
 *                      to the left channel, in decibels.
 *      rightVol        The relative amplification or attenuation to apply
 *                      to the right channel, in decibels.
 *
 *
 * Returns:     WMSTATUS
 *      WMS_SUCCESS                 - success
 *      WMS_UNSUPPORTED             - signal is not supported, or doesn't
 *                                    support volume control
 *      WMS_NO_SUPPORTED_DEVICE     - device support not present
 *      See WMStatus.h for all other values and meanings.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioSetSignalVolumesDb( WM_DEVICE_HANDLE     hDevice,
                                     WM_AUDIO_SIGNAL      signal,
                                     int                  leftVol,
                                     int                  rightVol
                                   );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioGetSignalVolumeDb
 *
 * Returns the signal level relative to full-scale, rounded to the nearest dB
 * relative to a full-scale signal.
 *
 * Note: If multiple channels are specified (e.g. WM_CHANNEL_STEREO) this function
 * assumes all channels are at the same level and returns the level of the first
 * channel it finds.  To check for different levels on different channels, call this
 * function once for each channel (e.g. once with WM_CHANNEL_LEFT and once with
 * WM_CHANNEL_RIGHT).
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to get.
 *      pVol            Receives the relative signal level in decibels.
 *      channel         One of the WM_CHANNEL_XXX constants.
 *
 * Returns:     WMSTATUS
 *      WMS_SUCCESS                 - success
 *      WMS_UNSUPPORTED             - signal is not supported, or doesn't
 *                                    support volume control
 *      WMS_NO_SUPPORTED_DEVICE     - device support not present
 *      See WMStatus.h for all other values and meanings.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioGetSignalVolumeDb( WM_DEVICE_HANDLE     hDevice,
                                    WM_AUDIO_SIGNAL      signal,
                                    int                  *pVol,
                                    WM_AUDIO_CHANNELS    channel
                                  );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioSetSignalVolumeAdv
 *
 * Set the signal level relative to full-scale, specifying the level in 1/16dB
 * steps relative to full scale.  The WM_SIGNAL_LEVEL macro can convert from
 * a dB value to the corresponding 1/16dB value.  E.g. WM_SIGNAL_LEVEL( 1.5 )
 * gives 0x18, which corresponds to 1.5dBFS.
 *
 * Note: the underlying device will probably not be able to set this level
 * precisely.  The applied level will be rounded to the nearest available
 * step.  For example, if the device has a 1.5dB step size (typical of AC'97
 * devices), a level of WM_SIGNAL_LEVEL(4) (0x40), WM_SIGNAL_LEVEL(4.125) (0x42),
 * WM_SIGNAL_LEVEL(4.5) (0x48) or WM_SIGNAL_LEVEL( 5 ) (0x50) would all
 * result in a 4.5dBFS signal.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to set.
 *      baseVol         The relative amplification or attenuation to apply,
 *                      in 1/16 dB steps.
 *      channels        One or more of the WM_CHANNEL_XXX constants.
 *
 * Returns:     WMSTATUS
 *      WMS_SUCCESS                 - success
 *      WMS_UNSUPPORTED             - signal is not supported, or doesn't
 *                                    support volume control
 *      WMS_NO_SUPPORTED_DEVICE     - device support not present
 *      See WMStatus.h for all other values and meanings.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioSetSignalVolumeAdv( WM_DEVICE_HANDLE     hDevice,
                                     WM_AUDIO_SIGNAL      signal,
                                     int                  baseVol,
                                     WM_AUDIO_CHANNELS    channels
                                   );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioSetSignalVolumesAdv
 *
 * Set the signal level relative to full-scale, specifying the level in 1/16dB
 * steps relative to full scale.  The WM_SIGNAL_LEVEL macro can convert from
 * a dB value to the corresponding 1/16dB value.  E.g. WM_SIGNAL_LEVEL( 1.5 )
 * gives 0x18, which corresponds to 1.5dBFS.
 *
 * Note: the underlying device will probably not be able to set this level
 * precisely.  The applied level will be rounded to the nearest available
 * step.  For example, if the device has a 1.5dB step size (typical of AC'97
 * devices), a level of WM_SIGNAL_LEVEL(4) (0x40), WM_SIGNAL_LEVEL(4.125) (0x42),
 * WM_SIGNAL_LEVEL(4.5) (0x48) or WM_SIGNAL_LEVEL( 5 ) (0x50) would all
 * result in a 4.5dBFS signal.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to set.
 *      leftVol         The relative amplification or attenuation to apply,
 *                      in 1/16 dB steps.
 *      rightVol        The relative amplification or attenuation to apply,
 *                      in 1/16 dB steps.
 *
 * Returns:     WMSTATUS
 *      WMS_SUCCESS                 - success
 *      WMS_UNSUPPORTED             - signal is not supported, or doesn't
 *                                    support volume control
 *      WMS_NO_SUPPORTED_DEVICE     - device support not present
 *      See WMStatus.h for all other values and meanings.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioSetSignalVolumesAdv( WM_DEVICE_HANDLE   hDevice,
                                      WM_AUDIO_SIGNAL    signal,
                                      int                leftVol,
                                      int                rightVol
                                    );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioGetSignalVolumeAdv
 *
 * Returns the signal level relative to full-scale, using 1/16dB steps.
 *
 * Note: If multiple channels are specified (e.g. WM_CHANNEL_STEREO) this function
 * assumes all channels are at the same level and returns the level of the first
 * channel it finds.  To check for different levels on different channels, call this
 * function once for each channel (e.g. once with WM_CHANNEL_LEFT and once with
 * WM_CHANNEL_RIGHT).
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to get.
 *      pVol            Receives the relative signal level in 1/16 dB steps.
 *      channel         One of the WM_CHANNEL_XXX constants.
 *
 * Returns:     WMSTATUS
 *      WMS_SUCCESS                 - success
 *      WMS_UNSUPPORTED             - signal is not supported, or doesn't
 *                                    support volume control
 *      WMS_NO_SUPPORTED_DEVICE     - device support not present
 *      See WMStatus.h for all other values and meanings.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioGetSignalVolumeAdv( WM_DEVICE_HANDLE     hDevice,
                                     WM_AUDIO_SIGNAL      signal,
                                     int                  *pVol,
                                     WM_AUDIO_CHANNELS    channel
                                   );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioVolToDbAdv
 *
 * Called to calculate the dB volume for a given perceived volume level.
 * Experiments have shown that a 10dB increase in sound level corresponds
 * approximately to a perceived doubling of loudness.  We take 0x10000 as
 * 0dBFS, and scale from there (so 0xFFFF is loudest, 0x8000 is -10dBFS,
 * 0x4000 is -20dBFS, etc).
 *
 * Note: Linearly reducing the perceived volume does _not_ result in a linear
 *       attenuation of the output signal.
 *
 * Parameters:
 *      hDevice             device handle
 *      perceivedVolume     16-bit volume where 0xFFFF is 0dBFS
 *
 * Returns:     int
 *      corresponding volume in 1/16dB steps
 *---------------------------------------------------------------------------*/
int WMAudioVolToDbAdv( WM_DEVICE_HANDLE hDevice, unsigned short perceivedVolume );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioSetSignalVolume
 *
 * Set the perceived volume for the given signal.
 * Experiments have shown that a 10dB increase in sound level corresponds
 * approximately to a perceived doubling of loudness.  We take 0x10000 as
 * 0dBFS, and scale from there (so 0xFFFF is loudest, 0x8000 is -10dBFS,
 * 0x4000 is -20dBFS, etc).
 *
 * Note: Linearly reducing the perceived volume does _not_ result in a linear
 *       attenuation of the output signal.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to set.
 *      volume          The level to apply, where 0xFFFF is 0dBFS.
 *      channels        One or more of the WM_CHANNEL_XXX constants.
 *
 *
 * Returns:     WMSTATUS
 *      WMS_SUCCESS                 - success
 *      WMS_UNSUPPORTED             - signal is not supported, or doesn't
 *                                    support volume control
 *      WMS_NO_SUPPORTED_DEVICE     - device support not present
 *      See WMStatus.h for all other values and meanings.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioSetSignalVolume( WM_DEVICE_HANDLE     hDevice,
                                  WM_AUDIO_SIGNAL      signal,
                                  unsigned short       volume,
                                  WM_AUDIO_CHANNELS    channels
                                );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioSetSignalVolumes
 *
 * Set the perceived volumes for the given stereo signal.
 * Experiments have shown that a 10dB increase in sound level corresponds
 * approximately to a perceived doubling of loudness.  We take 0x10000 as
 * 0dBFS, and scale from there (so 0xFFFF is loudest, 0x8000 is -10dBFS,
 * 0x4000 is -20dBFS, etc).
 *
 * Note: Linearly reducing the perceived volume does _not_ result in a linear
 *       attenuation of the output signal.
 *
 * Parameters:
 *      hDevice         The handle to the device (from WMOpenDevice).
 *      signal          The signal to set.
 *      leftVol         The level to apply to the left channel, where 0xFFFF
 *                      is 0dBFS.
 *      rightVol        The level to apply to the right channel, where 0xFFFF
 *                      is 0dBFS.
 *
 * Returns:     WMSTATUS
 *      WMS_SUCCESS                 - success
 *      WMS_UNSUPPORTED             - signal is not supported, or doesn't
 *                                    support volume control
 *      WMS_NO_SUPPORTED_DEVICE     - device support not present
 *      See WMStatus.h for all other values and meanings.
 *---------------------------------------------------------------------------*/
WM_STATUS WMAudioSetSignalVolumes( WM_DEVICE_HANDLE     hDevice,
                                   WM_AUDIO_SIGNAL      signal,
                                   unsigned short       leftVol,
                                   unsigned short       rightVol
                                );


#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif  /* __WMAUDIO_H__ */
/*------------------------------ END OF FILE ---------------------------------*/

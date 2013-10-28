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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
/*!
 * @file pmic_audio.c
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
       Operating Mode               MC1378     SC5512
       ---------------------------- ------    --------
       Stereo DAC Playback           Yes        Yes
       Stereo DAC Input Mixing       Yes        Yes
       Voice CODEC Playback          Yes        Yes
       Voice CODEC Input Mixing      Yes        No
       Voice CODEC Mono Recording    Yes        Yes
       Voice CODEC Stereo Recording  Yes        No
       Microphone Bias Control       Yes        Yes
       Output Amplifier Control      Yes        Yes
       Output Mixing Control         Yes        Yes
       Input Amplifier Control       Yes        Yes
       Master/Slave Mode Select      Yes        Yes
       Anti Pop Bias Circuit Control Yes        Yes
   @endverbatim
 *
 * Note that the Voice CODEC may also be referred to as the Telephone
 * CODEC in the PMIC DTS documentation. Also note that, while the power
 * management ICs do provide similar audio capabilities, each PMIC may
 * support additional configuration settings and features. Therefore, it
 * is highly recommended that the appropriate power management IC DTS
 * documents be used in conjunction with this API interface. The following
 * documents were used in the development of this API:
 *
 * [1] MC13783 Detailed Technical Specification (Level 3),
 *     Rev 1.3 - 04/09/17. Freescale Semiconductor, Inc.
 *
 * [2] iDEN Phoenix Platform Level 3 Detailed Technical Specification,
 *     Rev 2.2.1 5-12-2005. Motorola, Inc.
 *
 * @ingroup PMIC_MC13783_AUDIO
 */

/*===========================================================================*/

#include <windows.h>    /* Needed for all standard WinCE definitions. */
#include <mmsystem.h>

#include <pmic_audio.h> /* PMIC audio driver API definitions.               */
#include <pmic_adc.h>   /* PMIC analog-to-digital converter interface defs. */
#include <pmic_lla.h>   /* PMIC low-level interface definitions.            */
#include <regs.h>       /* MC13783 audio control register definitions.      */

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

typedef enum
{
    REG_IMR,
    REG_RX0         = MC13783_AUD_RX0_ADDR,
    REG_RX1         = MC13783_AUD_RX1_ADDR,
    REG_AUDIO_TX    = MC13783_AUD_TX_ADDR,
    REG_SSI_NETWORK = MC13783_SSI_NW_ADDR,
    REG_AUDIO_CODEC = MC13783_AUD_CDC_ADDR,
    REG_STEREO_DAC  = MC13783_AUD_STR_DAC_ADDR
} pmic_control_register;

/*===========================================================================*/

/*!
 * Define the minimum sampling rate (in Hz) that is supported by the
 * Stereo DAC.
 */
const unsigned MIN_STDAC_SAMPLING_RATE_HZ = 8000;

/*!
 * Define the maximum sampling rate (in Hz) that is supported by the
 * Stereo DAC.
 */
const unsigned MAX_STDAC_SAMPLING_RATE_HZ = 96000;

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

/*!
 * @brief This structure is used to define a specific hardware register field.
 *
 * All hardware register fields are defined using an offset to the LSB
 * and a mask. The offset is used to right shift a register value before
 * applying the mask to actually obtain the value of the field.
 */
typedef struct
{
    unsigned char offset;/*!< Offset of LSB of register field.          */
    unsigned int  mask;  /*!< Mask value used to isolate register field.*/
} REGFIELD;

/*!
 * @brief This structure lists all fields of the AUDIO_CODEC hardware register.
 */
typedef struct
{
    REGFIELD CDCSSISEL;    /*!< Codec SSI Bus Select                   */
    REGFIELD CDCCLKSEL;    /*!< Codec Clock Input Select               */
    REGFIELD CDCSM;        /*!< Codec Slave/Master Select              */
    REGFIELD CDCBCLINV;    /*!< Codec Bitclock Inversion               */
    REGFIELD CDCFSINV;     /*!< Codec Framesync Inversion              */
    REGFIELD CDCFS;        /*!< Bus Protocol Selection                 */
    REGFIELD CDCCLK;       /*!< Codec Clock Setting                    */
    REGFIELD CDCFS8K16K;   /*!< Codec Sampling Rate Select             */
    REGFIELD CDCEN;        /*!< Codec Enable                           */
    REGFIELD CDCCLKEN;     /*!< Codec Master Clock Output Enable       */
    REGFIELD CDCTS;        /*!< Codec SSI Tristate                     */
    REGFIELD CDCDITH;      /*!< Codec Dithering                        */
    REGFIELD CDCRESET;     /*!< Codec Digital Filter Reset             */
    REGFIELD CDCBYP;       /*!< Codec Bypass                           */
    REGFIELD CDCALM;       /*!< Codec Analog Loopback                  */
    REGFIELD CDCDLM;       /*!< Codec Digital Loopback                 */
    REGFIELD AUDIHPF;      /*!< Transmit Highpass Filter Enable        */
    REGFIELD AUDOHPF;      /*!< Receive Highpass Filter Enable         */
} REGISTER_AUDIO_CODEC;

/*!
 * @brief This variable is used to access the AUDIO_CODEC hardware register.
 *
 * This variable defines how to access all of the fields within the
 * AUDIO_CODEC hardware register. The initial values consist of the offset
 * and mask values needed to access each of the register fields.
 */
static const REGISTER_AUDIO_CODEC regAUDIO_CODEC = {
    {  0, 0x000001 }, /* CDCSSISEL    */
    {  1, 0x000002 }, /* CDCCLKSEL    */
    {  2, 0x000004 }, /* CDCSM        */
    {  3, 0x000008 }, /* CDCBCLINV    */
    {  4, 0x000010 }, /* CDCFSINV     */
    {  5, 0x000060 }, /* CDCFS        */
    {  7, 0x000380 }, /* CDCCLK       */
    { 10, 0x000400 }, /* CDCFS8K16K   */
    { 11, 0x000800 }, /* CDCEN        */
    { 12, 0x001000 }, /* CDCCLKEN     */
    { 13, 0x002000 }, /* CDCTS        */
    { 14, 0x004000 }, /* CDCDITH      */
    { 15, 0x008000 }, /* CDCRESET     */
    { 16, 0x010000 }, /* CDCBYP       */
    { 17, 0x020000 }, /* CDCALM       */
    { 18, 0x040000 }, /* CDCDLM       */
    { 19, 0x080000 }, /* AUDIHPF      */
    { 20, 0x100000 }  /* AUDOHPF      */
};

/*!
 * @brief This structure lists all fields of the STEREO_DAC hardware register.
 */
typedef struct
{
    REGFIELD STDCSSISEL;        /*!< Stereo DAC SSI Bus Select            */
    REGFIELD STDCCLKSEL;        /*!< Stereo DAC Clock Input Select        */
    REGFIELD STDCSM;            /*!< Stereo DAC Slave/Master Select       */
    REGFIELD STDCBCLINV;        /*!< Stereo DAC Bitclock Inversion        */
    REGFIELD STDCFSINV;         /*!< Stereo DAC Framesync Inversion       */
    REGFIELD STDCFS;            /*!< Stereo DAC Bus Protocol Selection    */
    REGFIELD STDCCLK;           /*!< Stereo DAC Clock Setting             */
    REGFIELD STDCFSDLYB;        /*!< Stereo DAC Framesync Delay Bar       */
    REGFIELD STDCEN;            /*!< Stereo DAC Enable                    */
    REGFIELD STDCCLKEN;         /*!< Stereo DAC Clocking Enable           */
    REGFIELD STDCRESET;         /*!< Stereo DAC Digital Filter Reset      */
    REGFIELD SPDIF;             /*!< Stereo DAC SSI SPDIF Mode            */
    REGFIELD SR;                /*!< Stereo DAC Sampling Rate Select      */
} REGISTER_STEREO_DAC;

/*!
 * @brief This variable is used to access the STEREO_DAC hardware register.
 *
 * This variable defines how to access all of the fields within the
 * STEREO_DAC hardware register. The initial values consist of the offset
 * and mask values needed to access each of the register fields.
 */
static const REGISTER_STEREO_DAC regSTEREO_DAC = {
    {  0, 0x000001 }, /* STDCSSISEL        */
    {  1, 0x000002 }, /* STDCCLKSEL        */
    {  2, 0x000004 }, /* STDCSM            */
    {  3, 0x000008 }, /* STDCBCLINV        */
    {  4, 0x000010 }, /* STDCFSINV         */
    {  5, 0x000060 }, /* STDCFS            */
    {  7, 0x000380 }, /* STDCCLK           */
    { 10, 0x000400 }, /* STDCFSDLYB        */
    { 11, 0x000800 }, /* STDCEN            */
    { 12, 0x001000 }, /* STDCCLKEN         */
    { 15, 0x008000 }, /* STDCRESET         */
    { 16, 0x010000 }, /* SPDIF             */
    { 17, 0x1e0000 }  /* SR                */
};

/*!
 * @brief This structure lists all fields of the RX0 hardware register.
 */
typedef struct
{
    REGFIELD VAUDIOON;      /*!< Forces VAUDIO in Active On Mode              */
    REGFIELD BIASEN;        /*!< Audio Bias Enable                            */
    REGFIELD BIASSPEED;     /*!< Turn On Ramp Speed of the Audio Bias         */
    REGFIELD ASPEN;         /*!< Amplifier Asp Enable                         */
    REGFIELD ASPSEL;        /*!< Asp Input Selector                           */
    REGFIELD ALSPEN;        /*!< Amplifier Alsp Enable                        */
    REGFIELD ALSPREF;       /*!< Bias Alsp at Common Audio Reference          */
    REGFIELD ALSPSEL;       /*!< Alsp Input Selector                          */
    REGFIELD LSPLEN;        /*!< Output LSPL Enable                           */
    REGFIELD AHSREN;        /*!< Amplifier AhsR Enable                        */
    REGFIELD AHSLEN;        /*!< Amplifier AhsL Enable                        */
    REGFIELD AHSSEL;        /*!< Ahsr and Ahsl Input Selector                 */
    REGFIELD HSPGDIS;       /*!< Phantom Ground Disable                       */
    REGFIELD HSDETEN;       /*!< Headset Detect Enable                        */
    REGFIELD HSDETAUTOB;    /*!< Amplifier State Determined by Headset Detect */
    REGFIELD ARXOUTREN;     /*!< Output RXOUTR Enable                         */
    REGFIELD ARXOUTLEN;     /*!< Output RXOUTL Enable                         */
    REGFIELD ARXOUTSEL;     /*!< Arxout Input Selector                        */
    REGFIELD CDCOUTEN;      /*!< Output CDCOUT Enable                         */
    REGFIELD HSLDETEN;      /*!< Headset Left Channel Detect Enable           */
    REGFIELD ADDCDC;        /*!< Adder Channel Codec Selection                */
    REGFIELD ADDSTDC;       /*!< Adder Channel Stereo DAC Selection           */
    REGFIELD ADDRXIN;       /*!< Adder Channel Line In Selection              */
} REGISTER_RX0;

/*!
 * @brief This variable is used to access the RX0 hardware register.
 *
 * This variable defines how to access all of the fields within the
 * RX0 hardware register. The initial values consist of the offset
 * and mask values needed to access each of the register fields.
 */
static const REGISTER_RX0 regRX0 = {
    {  0, 0x000001 }, /* VAUDIOON      */
    {  1, 0x000002 }, /* BIASEN        */
    {  2, 0x000004 }, /* BIASSPEED     */
    {  3, 0x000008 }, /* ASPEN         */
    {  4, 0x000010 }, /* ASPSEL        */
    {  5, 0x000020 }, /* ALSPEN        */
    {  6, 0x000040 }, /* ALSPREF       */
    {  7, 0x000080 }, /* ALSPSEL       */
    {  8, 0x000100 }, /* LSPLEN        */
    {  9, 0x000200 }, /* AHSREN        */
    { 10, 0x000400 }, /* AHSLEN        */
    { 11, 0x000800 }, /* AHSSEL        */
    { 12, 0x001000 }, /* HSPGDIS       */
    { 13, 0x002000 }, /* HSDETEN       */
    { 14, 0x004000 }, /* HSDETAUTOB    */
    { 15, 0x008000 }, /* ARXOUTREN     */
    { 16, 0x010000 }, /* ARXOUTLEN     */
    { 17, 0x020000 }, /* ARXOUTSEL     */
    { 18, 0x040000 }, /* CDCOUTEN      */
    { 19, 0x080000 }, /* HSLDETEN      */
    { 21, 0x200000 }, /* ADDCDC        */
    { 22, 0x400000 }, /* ADDSTDC       */
    { 23, 0x800000 }  /* ADDRXIN       */
};

/*!
 * @brief This structure lists all fields of the RX1 hardware register.
 */
typedef struct
{
    REGFIELD PGARXEN;       /*!< Codec Receive PGA Enable                    */
    REGFIELD PGARX;         /*!< Codec Receive Gain Setting                  */
    REGFIELD PGASTEN;       /*!< Stereo DAC PGA Enable                       */
    REGFIELD PGAST;         /*!< Stereo DAC Gain Setting                     */
    REGFIELD ARXINEN;       /*!< Amplifier Arx Enable                        */
    REGFIELD ARXIN;         /*!< Amplifier Arx Additional Gain Setting       */
    REGFIELD PGARXIN;       /*!< PGArxin Gain Setting                        */
    REGFIELD MONO;          /*!< Mono Adder Setting                          */
    REGFIELD BAL;           /*!< Balance Control                             */
    REGFIELD BALLR;         /*!< Left/Right Channel Balance                  */
} REGISTER_RX1;

/*!
 * @brief This variable is used to access the RX1 hardware register.
 *
 * This variable defines how to access all of the fields within the
 * RX0 hardware register. The initial values consist of the offset
 * and mask values needed to access each of the register fields.
 */
static const REGISTER_RX1 regRX1 = {
    {  0, 0x000001 }, /* PGARXEN       */
    {  1, 0x00001e }, /* PGARX         */
    {  5, 0x000020 }, /* PGASTEN       */
    {  6, 0x0003c0 }, /* PGAST         */
    { 10, 0x000400 }, /* ARXINEN       */
    { 11, 0x000800 }, /* ARXIN         */
    { 12, 0x00f000 }, /* PGARXIN       */
    { 16, 0x030000 }, /* MONO          */
    { 18, 0x1c0000 }, /* BAL           */
    { 21, 0x200000 }  /* BALLR         */
};

/*!
 * @brief This structure lists all fields of the AUDIO_TX hardware register.
 *
 */
typedef struct
{
    REGFIELD MC1BEN;             /*!< Microphone Bias 1 Enable                */
    REGFIELD MC2BEN;             /*!< Microphone Bias 2 Enable                */
    REGFIELD MC2BDETDBNC;        /*!< Microphone Bias Detect Debounce Setting */
    REGFIELD MC2BDETEN;          /*!< Microphone Bias 2 Detect Enable         */
    REGFIELD AMC1REN;            /*!< Amplifier Amc1R Enable                  */
    REGFIELD AMC1RITOV;          /*!< Amplifier Amc1R I-to-V Mode Enable      */
    REGFIELD AMC1LEN;            /*!< Amplifier Amc1L Enable                  */
    REGFIELD AMC1LITOV;          /*!< Amplifier Amc1L I-to-V Mode Enable      */
    REGFIELD AMC2EN;             /*!< Amplifier Amc2 Enable                   */
    REGFIELD AMC2ITOV;           /*!< Amplifier Amc2 I-to-V Mode Enable       */
    REGFIELD ATXINEN;            /*!< Amplifier Atxin Enable                  */
#if 0 /* This field is currently reserved but not used. */
    REGFIELD ATXOUTEN;           /*!< Reserved for Output TXOUT Enable        */
#endif
    REGFIELD RXINREC;            /*!< RXINR/RXINL to Voice CODEC ADC Routing  */
    REGFIELD PGATXR;             /*!< Transmit Gain Setting Right Channel     */
    REGFIELD PGATXL;             /*!< Transmit Gain Setting Left Channel      */
} REGISTER_AUDIO_TX;

/*!
 * @brief This variable is used to access the AUDIO_TX hardware register.
 *
 * This variable defines how to access all of the fields within the
 * AUDIO_TX hardware register. The initial values consist of the offset
 * and mask values needed to access each of the register fields.
 */
static const REGISTER_AUDIO_TX regAUDIO_TX = {
    {  0, 0x000001 }, /* MC1BEN             */
    {  1, 0x000002 }, /* MC2BEN             */
    {  2, 0x000004 }, /* MC2BDETDBNC        */
    {  3, 0x000008 }, /* MC2BDETEN          */
                      /* Reserved           */
    {  5, 0x000020 }, /* AMC1REN            */
    {  6, 0x000040 }, /* AMC1RITOV          */
    {  7, 0x000080 }, /* AMC1LEN            */
    {  8, 0x000100 }, /* AMC1LITOV          */
    {  9, 0x000200 }, /* AMC2EN             */
    { 10, 0x000400 }, /* AMC2ITOV           */
    { 11, 0x000800 }, /* ATXINEN            */
    { 13, 0x002000 }, /* RXINREC            */
    { 14, 0x07c000 }, /* PGATXR             */
    { 19, 0xf80000 }  /* PGATXL             */
};

/*!
 * @brief This structure lists all fields of the SSI_NETWORK hardware register.
 *
 */
typedef struct
{
    REGFIELD CDCTXRXSLOT;        /*!< Codec Timeslot Assignment               */
    REGFIELD CDCTXSECSLOT;       /*!< Codec Secondary Transmit Timeslot       */
    REGFIELD CDCRXSECSLOT;       /*!< Codec Secondary Receive Timeslot        */
    REGFIELD CDCRXSECGAIN;       /*!< Codec Secondary Receive Channel Gain    */
    REGFIELD CDCSUMGAIN;         /*!< Codec Summed Receive Gain Setting       */
    REGFIELD CDCFSDLY;           /*!< Codec Framesync Delay                   */
    REGFIELD STDCSLOTS;          /*!< Stereo DAC Number of Timeslots Select   */
    REGFIELD STDCRXSLOT;         /*!< Stereo DAC Number of Timeslots Select   */
    REGFIELD STDCRXSECSLOT;      /*!< Stereo DAC Secondary Receive Timeslot   */
    REGFIELD STDCRXSECGAIN;      /*!< Stereo DAC Secondary Channel Gain       */
    REGFIELD STDCSUMGAIN;        /*!< Stereo DAC Summed Receive Signal Gain   */
} REGISTER_SSI_NETWORK;

/*!
 * @brief This variable is used to access the SSI_NETWORK hardware register.
 *
 * This variable defines how to access all of the fields within the
 * SSI_NETWORK hardware register. The initial values consist of the offset
 * and mask values needed to access each of the register fields.
 */
static const REGISTER_SSI_NETWORK regSSI_NETWORK = {
    {  2, 0x00000c }, /* CDCTXRXSLOT        */
    {  4, 0x000030 }, /* CDCTXSECSLOT       */
    {  6, 0x0000c0 }, /* CDCRXSECSLOT       */
    {  8, 0x000300 }, /* CDCRXSECGAIN       */
    { 10, 0x000400 }, /* CDCSUMGAIN         */
    { 11, 0x000800 }, /* CDCFSDLY           */
    { 12, 0x003000 }, /* STDCSLOTS          */
    { 14, 0x00c000 }, /* STDCRXSLOT         */
    { 16, 0x030000 }, /* STDCRXSECSLOT      */
    { 18, 0x0c0000 }, /* STDCRXSECGAIN      */
    { 20, 0x100000 }  /* STDCSUMGAIN        */
};

/*! Define a mask to access the entire hardware register. */
static const unsigned int REG_FULLMASK = 0xffffff;

/*! Reset value for the AUD_CODEC register. */
static const unsigned int RESET_AUDIO_CODEC = 0x180027;

/*! Reset value for the ST_DAC register. */
static const unsigned int RESET_STEREO_DAC = 0x0e0004;

/*! Reset value for the RX0 register. */
static const unsigned int RESET_RX0 = 0x001000;

/*! Reset value for the RX1 register. */
static const unsigned int RESET_RX1 = 0x00d35a;

/*! Reset value for the AUDIO_TX register. */
static const unsigned int RESET_AUDIO_TX = 0x420000;

/*! Reset value for the SSI_NETWORK register. */
static const unsigned int RESET_SSI_NETWORK = 0x013060;

/*! Constant NULL value for initializing/reseting the audio handles. */
static const PMIC_AUDIO_HANDLE AUDIO_HANDLE_NULL = (PMIC_AUDIO_HANDLE)NULL;

/*!
 * @brief This structure maintains the current state of the Stereo DAC.
 */
typedef struct
{
    PMIC_AUDIO_HANDLE              handle;       /*!< Handle used to access
                                                      the Stereo DAC.         */
    HANDLE_STATE                   handleState;  /*!< Current handle state.   */
    PMIC_AUDIO_BUS_PROTOCOL        protocol;     /*!< Data bus protocol.      */
    PMIC_AUDIO_BUS_MODE            masterSlave;  /*!< Master/Slave mode
                                                      select.                 */
    PMIC_AUDIO_NUMSLOTS            numSlots;     /*!< Number of timeslots
                                                      used.                   */
    PMIC_AUDIO_CALLBACK            callback;     /*!< Event notification
                                                      callback function
                                                      pointer.                */
    PMIC_AUDIO_EVENTS              eventMask;    /*!< Event notification mask.*/
    PMIC_AUDIO_CLOCK_IN_SOURCE     clockIn;      /*!< Stereo DAC clock input
                                                      source select.          */
    PMIC_AUDIO_STDAC_SAMPLING_RATE samplingRate; /*!< Stereo DAC sampling rate
                                                      select.                 */
    PMIC_AUDIO_STDAC_CLOCK_IN_FREQ clockFreq;    /*!< Stereo DAC clock input
                                                      frequency.              */
    PMIC_AUDIO_CLOCK_INVERT        invert;       /*!< Stereo DAC clock signal
                                                      invert select.          */
    PMIC_AUDIO_STDAC_TIMESLOTS     timeslot;     /*!< Stereo DAC data
                                                      timeslots select.       */
    PMIC_AUDIO_STDAC_CONFIG        config;       /*!< Stereo DAC configuration
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
static PMIC_AUDIO_STDAC_STATE stDAC = {
    (PMIC_AUDIO_HANDLE)NULL,       /* handle       */
    HANDLE_FREE,                   /* handleState  */
    NORMAL_MSB_JUSTIFIED_MODE,     /* protocol     */
    BUS_SLAVE_MODE,                /* masterSlave  */
    USE_2_TIMESLOTS,               /* numSlots     */
    (PMIC_AUDIO_CALLBACK)NULL,     /* callback     */
    (PMIC_AUDIO_EVENTS)NULL,       /* eventMask    */
    CLOCK_IN_CLIA,                 /* clockIn      */
    STDAC_RATE_44_1_KHZ,           /* samplingRate */
    STDAC_CLI_26MHZ,               /* clockFreq    */  // STDAC_CLI_13MHZ
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
    PMIC_AUDIO_VCODEC_SAMPLING_RATE samplingRate; /*!< Voice CODEC sampling
                                                       rate select.           */
    PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ clockFreq;    /*!< Voice CODEC clock input
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
static PMIC_AUDIO_VCODEC_STATE vCodec = {
    (PMIC_AUDIO_HANDLE)NULL,                         /* handle             */
    HANDLE_FREE,                                     /* handleState        */
    NETWORK_MODE,                                    /* protocol           */
    BUS_SLAVE_MODE,                                  /* masterSlave        */
    USE_4_TIMESLOTS,                                 /* numSlots           */
    (PMIC_AUDIO_CALLBACK)NULL,                       /* callback           */
    (PMIC_AUDIO_EVENTS)NULL,                         /* eventMask          */
    CLOCK_IN_CLIB,                                   /* clockIn            */
    VCODEC_RATE_8_KHZ,                               /* samplingRate       */
    VCODEC_CLI_13MHZ,                                /* clockFreq          */
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
    PMIC_AUDIO_OUTPUT_PORT            outputPort;       /*!< Current audio
                                                             output port.     */
    PMIC_AUDIO_OUTPUT_PGA_GAIN        outputStDACPGAGain; /*!< Output PGA gain
                                                               level.         */
    PMIC_AUDIO_OUTPUT_PGA_GAIN        outputVCodecPGAGain; /*!< Output PGA gain
                                                                level.        */
    PMIC_AUDIO_OUTPUT_PGA_GAIN        outputExtStereoInPGAGain; /*!< Output PGA
                                                                     gain level.
                                                                              */
    PMIC_AUDIO_OUTPUT_BALANCE_GAIN    balanceLeftGain;  /*!< Left channel
                                                             balance gain
                                                             level.           */
    PMIC_AUDIO_OUTPUT_BALANCE_GAIN    balanceRightGain; /*!< Right channel
                                                             balance gain
                                                             level.           */
    PMIC_AUDIO_MONO_ADDER_OUTPUT_GAIN monoAdderGain;    /*!< Mono adder gain
                                                             level.           */
    PMIC_AUDIO_OUTPUT_CONFIG          config;           /*!< Audio output
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
    (PMIC_AUDIO_OUTPUT_PORT)NULL, /* outputPort               */
    OUTPGA_GAIN_0DB,              /* outputStDACPGAGain       */
    OUTPGA_GAIN_0DB,              /* outputVCodecPGAGain      */
    OUTPGA_GAIN_0DB,              /* outputExtStereoInPGAGain */
    BAL_GAIN_0DB,                 /* balanceLeftGain          */
    BAL_GAIN_0DB,                 /* balanceRightGain         */
    MONOADD_GAIN_0DB,             /* monoAdderGain            */
    (PMIC_AUDIO_OUTPUT_CONFIG)0   /* config                   */
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

static PMIC_STATUS pmic_read_reg(const pmic_control_register regID,
                                 const unsigned *regValue);
static PMIC_STATUS pmic_write_reg(const pmic_control_register regID,
                                  const unsigned regValue,
                                  const unsigned regMask);
static inline void down_interruptible(HANDLE hMutex);
static inline void up(HANDLE hMutex);
static PMIC_STATUS pmic_adc_convert(const unsigned short  channel,
                                    const unsigned short *adcResult);
static PMIC_STATUS pmic_audio_mic_boost_enable(void);
static PMIC_STATUS pmic_audio_mic_boost_disable(void);
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
    PMIC_STATUS rc = PMIC_ERROR;

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

    /* Check the current device handle state and acquire the handle if
     * it is available.
     */
    if ((device == STEREO_DAC) && (stDAC.handleState == HANDLE_FREE))
    {
        stDAC.handle      = (PMIC_AUDIO_HANDLE)(&stDAC);
        stDAC.handleState = HANDLE_IN_USE;
        *handle = stDAC.handle;

        rc = PMIC_SUCCESS;
    }
    else if ((device == VOICE_CODEC) && (vCodec.handleState == HANDLE_FREE))
    {
        vCodec.handle      = (PMIC_AUDIO_HANDLE)(&vCodec);
        vCodec.handleState = HANDLE_IN_USE;
        *handle = vCodec.handle;

        rc = PMIC_SUCCESS;
    }
    else if ((device == EXTERNAL_STEREO_IN) &&
             (extStereoIn.handleState == HANDLE_FREE))
    {
        extStereoIn.handle      = (PMIC_AUDIO_HANDLE)(&extStereoIn);
        extStereoIn.handleState = HANDLE_IN_USE;
        *handle = extStereoIn.handle;

        rc = PMIC_SUCCESS;

    } else {

        *handle = AUDIO_HANDLE_NULL;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* We need a critical section here to maintain a consistent state. */
    down_interruptible(mutex);

    /* We can now call pmic_audio_close_handle() to actually do the work. */
    rc = pmic_audio_close_handle(handle);

    /* Exit the critical section. */
    up(mutex);

    return rc;
}

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
    const PMIC_AUDIO_NUMSLOTS     numSlots)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    const unsigned int ST_DAC_MASK1 = regSTEREO_DAC.STDCSSISEL.mask |
                                      regSTEREO_DAC.STDCSM.mask     |
                                      regSTEREO_DAC.STDCCLKEN.mask;
    const unsigned int ST_DAC_MASK2 = regSSI_NETWORK.STDCSLOTS.mask;
    const unsigned int VCODEC_MASK  = regAUDIO_CODEC.CDCSSISEL.mask |
                                      regAUDIO_CODEC.CDCSM.mask     |
                                      regAUDIO_CODEC.CDCCLKEN.mask;
    unsigned int reg_value  = 0;
    unsigned int reg_value2 = 0;

    /* Enter a critical section so that we can ensure only one
     * state change request is completed at a time.
     */
    down_interruptible(mutex);

    if ((handle == stDAC.handle) &&
        (stDAC.handleState == HANDLE_IN_USE))
    {
        /* Make sure that the MC13783 PMIC supports the requested
         * Stereo DAC data bus protocol.
         */
        if ((protocol == NETWORK_MODE) &&
            !((numSlots == USE_2_TIMESLOTS) ||
              (numSlots == USE_4_TIMESLOTS) ||
              (numSlots == USE_8_TIMESLOTS)))
        {
            /* The Stereo DAC only supports 2, 4, or 8 time slots when in
             * Network Mode.
             */
            DEBUGMSG(ZONE_ERROR,
                     (_T("%s: Invalid numSlots parameter (Stereo DAC only ")
                     _T("supports 2,4, or 8 timeslots in Network Mode)\n"),
                     __FILE__));
            rc = PMIC_NOT_SUPPORTED;
        }
        else if (((protocol == NORMAL_MSB_JUSTIFIED_MODE) ||
                  (protocol == I2S_MODE)) &&
                 (numSlots != USE_2_TIMESLOTS))
        {
            /* The Stereo DAC only supports the use of 2 time slots when in
             * Normal Mode or I2S Mode.
             */
            DEBUGMSG(ZONE_ERROR,
                     (_T("%s: Invalid numSlots parameter (Stereo DAC only ")
                     _T("supports 2 timeslots in Normal or I2S Mode)\n"),
                     __FILE__));
            rc = PMIC_NOT_SUPPORTED;
        }
        else if ((masterSlave == BUS_MASTER_MODE) &&
                 ((stDAC.clockIn == CLOCK_IN_FSYNC) ||
                  (stDAC.clockIn == CLOCK_IN_BITCLK)))
        {
            /* Invalid clock inputs for master mode. */
            DEBUGMSG(ZONE_ERROR,
                     (_T("%s: Invalid Stereo DAC clock selection for ")
                     _T("Master Mode\n"),
                     __FILE__));
            rc = PMIC_PARAMETER_ERROR;
        }
        else if ((masterSlave == BUS_SLAVE_MODE) &&
                 (stDAC.clockIn == CLOCK_IN_DEFAULT))
        {
            /* Invalid clock inputs for slave mode. */
            DEBUGMSG(ZONE_ERROR,
                     (_T("%s: Invalid Stereo DAC clock selection for ")
                     _T("Slave Mode\n"),
                     __FILE__));
            rc = PMIC_PARAMETER_ERROR;
        }
        else if (protocol == SPD_IF_MODE)
        {
            /* SPD/IF mode is no longer supported. */
            DEBUGMSG(ZONE_ERROR,
                     (_T("%s: The SPD/IF Stereo DAC mode is no longer ")
                     _T("supported\n"), __FILE__));
            rc = PMIC_NOT_SUPPORTED;
        }
        else
        {
            if (rc != PMIC_ERROR)
            {
                /* We can now proceed with the Stereo DAC configuration. */
                reg_value = SET_BITS(regSTEREO_DAC, STDCSSISEL, 0)    |
                            SET_BITS(regSTEREO_DAC, STDCFS, protocol)     |
                            SET_BITS(regSTEREO_DAC, STDCSM, masterSlave);

                if (masterSlave == BUS_MASTER_MODE)
                {
                    /* Enable clock outputs when in master mode. */
                    reg_value |= SET_BITS(regSTEREO_DAC, STDCCLKEN, 1);
                }

                if (numSlots == USE_2_TIMESLOTS)
                {
                    /* Use 2 time slots (left, right). */
                    reg_value2 |= SET_BITS(regSSI_NETWORK, STDCSLOTS, 3);
                }
                else if (numSlots == USE_4_TIMESLOTS)
                {
                    /* Use 4 time slots (left, right, 2 other). */
                    reg_value2 |= SET_BITS(regSSI_NETWORK, STDCSLOTS, 2);
                }
                else if ((stDAC.samplingRate == STDAC_RATE_64_KHZ) ||
                         (stDAC.samplingRate == STDAC_RATE_96_KHZ))
                {
                    /* Use 8 time slots (must use this mode for 64 kHz or
                     * 96 kHz sampling rates).
                     */
                    reg_value2 |= SET_BITS(regSSI_NETWORK, STDCSLOTS, 0);
                }
                else
                {
                    /* Use 8 time slots (left, right, 6 other) for all
                     * sampling rates other than 64 kHz or 96 kHz.
                     */
                    reg_value2 |= SET_BITS(regSSI_NETWORK, STDCSLOTS, 1);
                }

                if ((pmic_write_reg(REG_STEREO_DAC, reg_value,
                                    ST_DAC_MASK1 | regSTEREO_DAC.STDCFS.mask) ==
                     PMIC_SUCCESS) &&
                    (pmic_write_reg(REG_SSI_NETWORK, reg_value2, ST_DAC_MASK2) ==
                     PMIC_SUCCESS))
                {
                    stDAC.protocol    = protocol;
                    stDAC.masterSlave = masterSlave;
                    stDAC.numSlots    = numSlots;

                    rc = PMIC_SUCCESS;
                }
                else
                {
                    rc = PMIC_ERROR;
                }
            }
        }
    }
    else if ((handle == vCodec.handle) &&
             (vCodec.handleState == HANDLE_IN_USE))
    {
        /* The MC13783 Voice CODEC only supports a network or I2S mode with 4
         * timeslots.
         */
        if (!((protocol == NETWORK_MODE) || (protocol == I2S_MODE)))
        {
            DEBUGMSG(ZONE_ERROR,
                     (_T("%s: Voice CODEC only supports Network or I2S ")
                     _T("modes\n"), __FILE__));
            rc = PMIC_NOT_SUPPORTED;
        }
        else if (numSlots != USE_4_TIMESLOTS)
        {
            DEBUGMSG(ZONE_ERROR,
                     (_T("%s: Voice CODEC only supports using 4 timeslots\n"),
                     __FILE__));
            rc = PMIC_NOT_SUPPORTED;
        }
        else
        {
            if (rc != PMIC_ERROR)
            {
                reg_value = SET_BITS(regAUDIO_CODEC, CDCSSISEL, 0)  |
                            SET_BITS(regAUDIO_CODEC, CDCFS, protocol)   |
                            SET_BITS(regAUDIO_CODEC, CDCSM, masterSlave);

                if (masterSlave == BUS_MASTER_MODE)
                {
                    reg_value |= SET_BITS(regAUDIO_CODEC, CDCCLKEN, 1);
                }

                rc = pmic_write_reg(REG_AUDIO_CODEC, reg_value,
                                    VCODEC_MASK | regAUDIO_CODEC.CDCFS.mask);

                if (rc == PMIC_SUCCESS)
                {
                    vCodec.protocol    = protocol;
                    vCodec.masterSlave = masterSlave;
                    vCodec.numSlots    = numSlots;
                }
            }
        }
    }

    /* Exit critical section. */
    up(mutex);

    return rc;
}

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
    PMIC_AUDIO_BUS_MODE *const     masterSlave,
    PMIC_AUDIO_NUMSLOTS *const     numSlots)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    if ((protocol != (PMIC_AUDIO_BUS_PROTOCOL *)NULL) &&
        (masterSlave != (PMIC_AUDIO_BUS_MODE *)NULL)  &&
        (numSlots != (PMIC_AUDIO_NUMSLOTS *)NULL))
    {
        /* Enter a critical section so that we return a consistent state. */
        down_interruptible(mutex);

        if ((handle == stDAC.handle) &&
            (stDAC.handleState == HANDLE_IN_USE))
        {
            *protocol    = stDAC.protocol;
            *masterSlave = stDAC.masterSlave;
            *numSlots    = stDAC.numSlots;

            rc = PMIC_SUCCESS;
        }
        else if ((handle == vCodec.handle) &&
                 (vCodec.handleState == HANDLE_IN_USE))
        {
            *protocol    = vCodec.protocol;
            *masterSlave = vCodec.masterSlave;
            *numSlots    = vCodec.numSlots;

            rc = PMIC_SUCCESS;
        }

        /* Exit critical section. */
        up(mutex);
    }

    return rc;
}

/*!
 * @brief Enable the Stereo DAC or the Voice CODEC.
 *
 * Explicitly enable the Stereo DAC or the Voice CODEC to begin audio
 * playback or recording as required. This should only be done after
 * successfully configuring all of the associated audio components (e.g.,
 * microphones, amplifiers, etc.).
 *
 * Note that the timed delays used in this function are necessary to
 * ensure reliable operation of the Voice CODEC and Stereo DAC. The
 * Stereo DAC seems to be particularly sensitive and it has been observed
 * to fail to generate the required master mode clock signals if it is
 * not allowed enough time to initialize properly.
 *
 * @param[in]   handle          Device handle from pmic_audio_open() call.
 *
 * @retval      PMIC_SUCCESS         If the device was successful enabled.
 * @retval      PMIC_PARAMETER_ERROR If the handle is invalid.
 * @retval      PMIC_ERROR           If the device could not be enabled.
 */
PMIC_STATUS PmicAudioEnable(
    const PMIC_AUDIO_HANDLE handle)
{
    const unsigned int AUDIO_BIAS_ENABLE = regRX0.BIASEN.mask;
    const unsigned int STDAC_ENABLE      = regSTEREO_DAC.STDCEN.mask   |
                                           regSTEREO_DAC.STDCRESET.mask;
    const unsigned int VCODEC_ENABLE     = regAUDIO_CODEC.CDCEN.mask   |
                                           regAUDIO_CODEC.CDCRESET.mask;
    unsigned int reg_value = 0;
    PMIC_STATUS rc         = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE))
    {
        /* Only set the audio bias enable if it is not already set. Since there
         * is a significant delay associated with the audio bias ramp up, it is
         * worth the effort to check first to see whether the audio bias has
         * already been enabled. If so, then we can skip this step and its
         * associated delay.
         */
        pmic_read_reg(REG_RX0, &reg_value);
        if (!(reg_value & AUDIO_BIAS_ENABLE))
        {
            /* Must first set the audio bias bit to power up the audio
             * circuits.
             */
            pmic_write_reg(REG_RX0, AUDIO_BIAS_ENABLE, AUDIO_BIAS_ENABLE);

            /* Pause for 150 ms to let the audio circuits ramp up. We must use
             * TASK_UNINTERRUPTIBLE here because we want to delay for at least
             * 150 ms.
             */
            PmicAudioTimedDelay(150 * delay_1ms);
        }

        /* Also check if the Stereo DAC is already enabled and skip the register
         * write and Stereo DAC enable delay if possible.
         */
        pmic_read_reg(REG_STEREO_DAC, &reg_value);
        if (reg_value & STDAC_ENABLE)
        {
            /* The Stereo DAC is already enabled but we still need to reset
             * the digital filter.
             */
            rc = PmicAudioDigitalFilterReset(handle);
        }
        else
        {
            /* We must try to enable the Stereo DAC. */
            rc = pmic_write_reg(REG_STEREO_DAC, STDAC_ENABLE, STDAC_ENABLE);

            pmic_read_reg(REG_STEREO_DAC, &reg_value);
            if (!(reg_value & STDAC_ENABLE))
            {
                DEBUGMSG(ZONE_ERROR, (_T("Failed to enable the Stereo DAC\n")));
                rc = PMIC_ERROR;
            }
            else
            {
                /* Pause for another 10 ms to let the hardware stabilize after
                 * enabling. We must use TASK_UNINTERRUPTIBLE here because we
                 * want to delay for at least 10 ms.
                 */
                PmicAudioTimedDelay(10 * delay_1ms);
            }
        }
    }
    else if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        /* Only set the audio bias enable if it is not already set. Since there
         * is a significant delay associated with the audio bias ramp up, it is
         * worth the effort to check first to see whether the audio bias has
         * already been enabled. If so, then we can skip this step and its
         * associated delay.
         */
        pmic_read_reg(REG_RX0, &reg_value);
        if (!(reg_value & AUDIO_BIAS_ENABLE))
        {
            /* Must first set the audio bias bit to power up the audio
             * circuits.
             */
            pmic_write_reg(REG_RX0, AUDIO_BIAS_ENABLE, AUDIO_BIAS_ENABLE);

            /* Pause for 150 ms to let the audio circuits ramp up. We must use
             * TASK_UNINTERRUPTIBLE here because we want to delay for at least
             * 150 ms.
             */
            PmicAudioTimedDelay(150 * delay_1ms);
        }

        /* Also check if the Voice CODEC is already enabled and skip the
         * register write and Voice CODEC enable delay if possible.
         */
        pmic_read_reg(REG_AUDIO_CODEC, &reg_value);
        if (reg_value & VCODEC_ENABLE)
        {
            /* The Voice CODEC is already enabled but we still need to reset
             * the digital filter.
             */
            rc = PmicAudioDigitalFilterReset(handle);
        }
        else
        {
            /* We must try to enable the Voice CODEC. */
            rc = pmic_write_reg(REG_AUDIO_CODEC, VCODEC_ENABLE, VCODEC_ENABLE);

            pmic_read_reg(REG_AUDIO_CODEC, &reg_value);
            if (!(reg_value & SET_BITS(regAUDIO_CODEC, CDCEN, 1)))
            {
                DEBUGMSG(ZONE_ERROR, (_T("Failed to enable the Voice ")
                                      _T("CODEC\n")));
                rc = PMIC_ERROR;
            }
            else
            {
                /* Pause for another 10 ms to let the hardware stabilize after
                 * enabling. We must use TASK_UNINTERRUPTIBLE here because we
                 * want to delay for at least 10 ms.
                 */
                PmicAudioTimedDelay(10 * delay_1ms);
            }
        }
    }

    /* Exit critical section. */
    up(mutex);

    return rc;
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
PMIC_STATUS PmicAudioDisable(
    const PMIC_AUDIO_HANDLE handle)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int AUDIO_BIAS_ENABLE = regRX0.BIASEN.mask;
    const unsigned int STDAC_DISABLE     = regSTEREO_DAC.STDCEN.mask;
    const unsigned int VCODEC_DISABLE    = regAUDIO_CODEC.CDCEN.mask;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE))
    {

        rc = pmic_write_reg(REG_STEREO_DAC, 0, STDAC_DISABLE);
    }
    else if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        rc = pmic_write_reg(REG_AUDIO_CODEC, 0, VCODEC_DISABLE);
    }

    /* We can also power down all of the audio circuits to minimize power
     * usage if no device handles are currently active.
     */
    if ((stDAC.handleState == HANDLE_FREE)     &&
        (vCodec.handleState == HANDLE_FREE)    &&
        (extStereoIn.handleState == HANDLE_FREE))
    {
        /* Yes, we can power down all of the audio circuits. */
        pmic_write_reg(REG_RX0, 0, AUDIO_BIAS_ENABLE);
    }

    /* Exit critical section. */
    up(mutex);

    return rc;
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
PMIC_STATUS PmicAudioReset(
    const PMIC_AUDIO_HANDLE handle)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    rc = pmic_audio_reset_device(handle);

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_SUCCESS;

    /* We need a critical section here to maintain a consistent state. */
    down_interruptible(mutex);

    /* First close all opened device handles, also deregisters callbacks. */
    pmic_audio_close_handle(stDAC.handle);
    pmic_audio_close_handle(vCodec.handle);
    pmic_audio_close_handle(extStereoIn.handle);

    if (pmic_write_reg(REG_SSI_NETWORK, RESET_SSI_NETWORK, REG_FULLMASK) !=
        PMIC_SUCCESS)
    {
        rc = PMIC_ERROR;
    }
    else
    {
        stDAC.timeslot             = USE_TS0_TS1;
        vCodec.timeslot            = USE_TS0;
        vCodec.secondaryTXtimeslot = USE_TS1;
    }

    if (pmic_write_reg(REG_STEREO_DAC, RESET_STEREO_DAC, REG_FULLMASK) !=
        PMIC_SUCCESS)
    {
        rc = PMIC_ERROR;
    }
    else
    {
        /* Also reset the driver state information to match. Note that we
         * keep the device handle and event callback settings unchanged
         * since these don't affect the actual hardware and we rely on
         * the user to explicitly close the handle or deregister callbacks
         */
        stDAC.protocol     = NORMAL_MSB_JUSTIFIED_MODE;
        stDAC.masterSlave  = BUS_SLAVE_MODE;
        stDAC.numSlots     = USE_2_TIMESLOTS;
        stDAC.clockIn      = CLOCK_IN_CLIA;
        stDAC.samplingRate = STDAC_RATE_44_1_KHZ;
        stDAC.clockFreq    = STDAC_CLI_26MHZ; //STDAC_CLI_13MHZ
        stDAC.invert       = NO_INVERT;
        stDAC.config       = (PMIC_AUDIO_STDAC_CONFIG)0;
    }

    if (pmic_write_reg(REG_AUDIO_CODEC, RESET_AUDIO_CODEC, REG_FULLMASK) !=
        PMIC_SUCCESS)
    {
        rc = PMIC_ERROR;
    }
    else
    {
        /* Also reset the driver state information to match. Note that we
         * keep the device handle and event callback settings unchanged
         * since these don't affect the actual hardware and we rely on
         * the user to explicitly close the handle or deregister callbacks
         */
        vCodec.protocol     = NETWORK_MODE;
        vCodec.masterSlave  = BUS_SLAVE_MODE;
        vCodec.numSlots     = USE_4_TIMESLOTS;
        vCodec.clockIn      = CLOCK_IN_CLIB;
        vCodec.samplingRate = VCODEC_RATE_8_KHZ;
        vCodec.clockFreq    = VCODEC_CLI_13MHZ;
        vCodec.invert       = NO_INVERT;
        vCodec.config       = PMIC_AUDIO_VCODEC_CONFIG(INPUT_HIGHPASS_FILTER |
                                                       OUTPUT_HIGHPASS_FILTER);
    }

    if (pmic_write_reg(REG_RX0, RESET_RX0, REG_FULLMASK) != PMIC_SUCCESS)
    {
        rc = PMIC_ERROR;
    }
    else
    {
        /* Also reset the driver state information to match. */
        audioOutput.outputPort               = (PMIC_AUDIO_OUTPUT_PORT)NULL;
        audioOutput.outputStDACPGAGain       = OUTPGA_GAIN_0DB;
        audioOutput.outputVCodecPGAGain      = OUTPGA_GAIN_0DB;
        audioOutput.outputExtStereoInPGAGain = OUTPGA_GAIN_0DB;
        audioOutput.config                   = (PMIC_AUDIO_OUTPUT_CONFIG)0;
    }

    if (pmic_write_reg(REG_RX1, RESET_RX1, REG_FULLMASK) != PMIC_SUCCESS)
    {
        rc = PMIC_ERROR;
    }
    else
    {
        audioOutput.monoAdderGain    = MONOADD_GAIN_0DB;
        audioOutput.balanceLeftGain  = BAL_GAIN_0DB;
        audioOutput.balanceRightGain = BAL_GAIN_0DB;
    }

    if (pmic_write_reg(REG_AUDIO_TX, RESET_AUDIO_TX, REG_FULLMASK) !=
        PMIC_SUCCESS)
    {
        rc = PMIC_ERROR;
    }
    else
    {
        /* Also reset the driver state information to match. Note that we
         * reset the vCodec fields since all of the input/recording
         * devices are only connected to the Voice CODEC and are managed
         * as part of the Voice CODEC state.
         */
        vCodec.inputConfig              = (PMIC_AUDIO_INPUT_CONFIG)NULL;
        vCodec.leftChannelMic.mic       = NO_MIC;
        vCodec.leftChannelMic.micOnOff  = MICROPHONE_OFF;
        vCodec.leftChannelMic.ampMode   = CURRENT_TO_VOLTAGE;
        vCodec.leftChannelMic.gain      = MIC_GAIN_0DB;
        vCodec.rightChannelMic.mic      = NO_MIC;
        vCodec.rightChannelMic.micOnOff = MICROPHONE_OFF;
        vCodec.rightChannelMic.ampMode  = AMP_OFF;
        vCodec.rightChannelMic.gain     = MIC_GAIN_0DB;
    }

    /* Finally, also reset any global state variables. */
    headsetState     = NO_HEADSET;

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_NOT_SUPPORTED;

    /* This function is deprecated for WinCE because audio driver interrupts
     * are now handled directly by the higher-level WAVEDEV driver.
     */

    return rc;
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
    PMIC_STATUS rc = PMIC_NOT_SUPPORTED;

    /* This function is deprecated for WinCE because audio driver interrupts
     * are now handled directly by the higher-level WAVEDEV driver.
     */

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

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
        if ((handle == stDAC.handle) &&
            (stDAC.handleState == HANDLE_IN_USE))
        {
            *func      = stDAC.callback;
            *eventMask = stDAC.eventMask;

            rc = PMIC_SUCCESS;
        }
        else if ((handle == vCodec.handle) &&
                 (vCodec.handleState == HANDLE_IN_USE))
        {
            *func      = vCodec.callback;
            *eventMask = vCodec.eventMask;

            rc = PMIC_SUCCESS;
        }
        else if ((handle == extStereoIn.handle) &&
                 (extStereoIn.handleState == HANDLE_IN_USE))
        {
            *func      = extStereoIn.callback;
            *eventMask = extStereoIn.eventMask;

            rc = PMIC_SUCCESS;
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_ERROR;
    unsigned int reg_write      = SET_BITS(regRX0, BIASEN, 1);
    const unsigned int reg_mask = SET_BITS(regRX0, BIASEN, 1)   |
                                  SET_BITS(regRX0, BIASSPEED, 1);

    /* No critical section required here since we are not updating any
     * global data.
     */

    /* For the MC13783 PMIC, the anti-pop circuitry is enabled by simply
     * setting the BIASEN bit in the RX0 register. We also set the ramp
     * speed using the BIASSPEED bit in the RX0 register.
     */
    if (rampSpeed == ANTI_POP_RAMP_FAST)
    {
         reg_write |= SET_BITS(regRX0, BIASSPEED, 1);
    }
    rc = pmic_write_reg(REG_RX0, reg_write, reg_mask);

    return rc;
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
    PMIC_STATUS rc = PMIC_ERROR;
    const unsigned int reg_mask = SET_BITS(regRX0, BIASEN, 1);

    /* No critical section required here since we are not updating any
     * global data.
     */

    /* For the MC13783 PMIC, the anti-pop circuitry is disabled by simply
     * clearing the BIASEN bit in the RX0 register.
     */
    rc = pmic_write_reg(REG_RX0, 0, reg_mask);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_value           = 0;
    unsigned int reg_write           = 0;
    unsigned int reg_mask            = 0;
    const unsigned int STDAC_ENABLE  = regSTEREO_DAC.STDCEN.mask;
    const unsigned int VCODEC_ENABLE = regAUDIO_CODEC.CDCEN.mask;

    /* We need a critical section here to avoid repeating the digital
     * filter reset operation while the previous reset operation is
     * still in progress.
     */
    down_interruptible(mutex);

    if ((handle == stDAC.handle) &&
        (stDAC.handleState == HANDLE_IN_USE))
    {
        /* Also check if the Stereo DAC is already enabled and skip the filter
         * reset if it is not enabled. The digital filter will automatically
         * be reset the next time we enable the Stereo DAC.
         */
        pmic_read_reg(REG_STEREO_DAC, &reg_value);
        if (reg_value & STDAC_ENABLE)
        {
            reg_write = SET_BITS(regSTEREO_DAC, STDCRESET, 1);
            reg_mask  = regSTEREO_DAC.STDCRESET.mask;

            if ((pmic_write_reg(REG_STEREO_DAC, reg_write, reg_mask) ==
                        PMIC_SUCCESS))
            {
                rc = PMIC_SUCCESS;
            }
            else
            {
                rc = PMIC_ERROR;
            }
        }
        else
        {
            /* Cannot reset the digital filter at this time because the
             * Stereo DAC is not enabled.
             */
            rc = PMIC_ERROR;
        }
    }
    else if ((handle == vCodec.handle) &&
             (vCodec.handleState == HANDLE_IN_USE))
    {
        /* Also check if the Voice CODEC is already enabled and skip the filter
         * reset if it is not enabled. The digital filter will automatically
         * be reset the next time we enable the Voice CODEC.
         */
        pmic_read_reg(REG_AUDIO_CODEC, &reg_value);
        if (reg_value & VCODEC_ENABLE)
        {
            reg_write = SET_BITS(regAUDIO_CODEC, CDCRESET, 1);
            reg_mask  = regAUDIO_CODEC.CDCRESET.mask;

            if ((pmic_write_reg(REG_AUDIO_CODEC, reg_write, reg_mask) !=
                        PMIC_SUCCESS))
            {
                rc = PMIC_SUCCESS;
            }
            else
            {
                rc = PMIC_ERROR;
            }
        }
        else
        {
            /* Cannot reset the digital filter at this time because the
             * Voice CODEC is not enabled.
             */
            rc = PMIC_ERROR;
        }
    }

    if (rc == PMIC_SUCCESS)
    {
        /* Pause for 10 ms to let the hardware complete the filter reset.
         * We must use TASK_UNINTERRUPTIBLE here because we want to delay
         * for at least 10 ms.
         */
        PmicAudioTimedDelay(10 * delay_1ms);
    }

    up(mutex);

    return rc;
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
    return PMIC_NOT_SUPPORTED;
}

/*!
 * @brief Provide a hexadecimal dump of all PMIC audio registers (DEBUG only)
 *
 * This function is intended strictly for debugging purposes only and will
 * print the current values of the following PMIC registers:
 *
 * - AUDIO_CODEC
 * - STEREO_DAC
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

    unsigned int reg_value = 0;

    DEBUGMSG(ZONE_INFO, (_T(">>> PMIC Audio Register Dump:\n")));

    /* Dump the AUDIO_CODEC (Voice CODEC) register. */
    if (pmic_read_reg(REG_AUDIO_CODEC, &reg_value) == PMIC_SUCCESS)
    {
        DEBUGMSG(ZONE_INFO, (_T("\n  AUDIO_CODEC = 0x%08x\n"), reg_value));

        /* Also decode the register fields. */
        DEBUGMSG(ZONE_INFO, (_T("             CDCSSISEL = %d (Digital ")
                 _T("Audio Bus %d)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCSSISEL, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCSSISEL, reg_value)) ? 1 : 0));
        DEBUGMSG(ZONE_INFO, (_T("             CDCCLKSEL = %d (CLI%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCCLKSEL, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCCLKSEL, reg_value)) ?
                 _T("B") : _T("A")));
        DEBUGMSG(ZONE_INFO, (_T("                 CDCSM = %d (%s Mode)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCSM, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCSM, reg_value)) ?
                 _T("Slave") : _T("Master")));
        DEBUGMSG(ZONE_INFO, (_T("             CDCBCLINV = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCBCLINV, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCBCLINV, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("              CDCFSINV = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCFSINV, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCFSINV, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                 CDCFS = 0x%x ("),
                 GET_BITS(regAUDIO_CODEC, CDCFS, reg_value)));
        switch (GET_BITS(regAUDIO_CODEC, CDCFS, reg_value))
        {
            case 1:
                DEBUGMSG(ZONE_INFO, (_T("Network Mode")));
                break;
            case 2:
                DEBUGMSG(ZONE_INFO, (_T("I2S Mode")));
                break;
            default:
                DEBUGMSG(ZONE_INFO, (_T("Not Supported")));
                break;
        }
        DEBUGMSG(ZONE_INFO, (_T(")\n")));
        DEBUGMSG(ZONE_INFO, (_T("                CDCCLK = 0x%x ("),
                 GET_BITS(regAUDIO_CODEC, CDCCLK, reg_value)));
        switch (GET_BITS(regAUDIO_CODEC, CDCCLK, reg_value))
        {
            case 0:
                DEBUGMSG(ZONE_INFO, (_T("CLI = 13.0 MHz")));
                break;
            case 1:
                DEBUGMSG(ZONE_INFO, (_T("CLI = 15.36 MHz")));
                break;
            case 2:
                DEBUGMSG(ZONE_INFO, (_T("CLI = 16.8 MHz")));
                break;
            case 4:
                DEBUGMSG(ZONE_INFO, (_T("CLI = 26 MHz")));
                break;
            case 7:
                DEBUGMSG(ZONE_INFO, (_T("CLI = 33 MHz")));
                break;
            default:
                DEBUGMSG(ZONE_INFO, (_T("INVALID VALUE")));
        }
        DEBUGMSG(ZONE_INFO, (_T(")\n")));
        DEBUGMSG(ZONE_INFO, (_T("            CDCFS8K16K = %d (%d kHz ")
                 _T("sampling rate)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCFS8K16K, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCFS8K16K, reg_value)) ?
                 16 : 8));
        DEBUGMSG(ZONE_INFO, (_T("                 CDCEN = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCEN, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("              CDCCLKEN = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCCLKEN, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCCLKEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                 CDCTS = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCTS, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCTS, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("               CDCDITH = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCDITH, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCDITH, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("              CDCRESET = %d (Should always ")
                 _T("read back as 0)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCRESET, reg_value)));
        DEBUGMSG(ZONE_INFO, (_T("                CDCBYP = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCBYP, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCBYP, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                CDCALM = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCALM, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCALM, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                CDCDLM = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, CDCDLM, reg_value),
                 (GET_BITS(regAUDIO_CODEC, CDCDLM, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("               AUDIHPF = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, AUDIHPF, reg_value),
                 (GET_BITS(regAUDIO_CODEC, AUDIHPF, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("               AUDOHPF = %d (%s)\n"),
                 GET_BITS(regAUDIO_CODEC, AUDOHPF, reg_value),
                 (GET_BITS(regAUDIO_CODEC, AUDOHPF, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (_T("!!! ERROR: Failed to read ")
                 _T("AUDIO_CODEC register !!!\n")));
    }

    /* Dump the STEREO_DAC (Stereo DAC) register. */
    if (pmic_read_reg(REG_STEREO_DAC, &reg_value) == PMIC_SUCCESS)
    {
        DEBUGMSG(ZONE_INFO, (_T("\n  STEREO_DAC = 0x%08x\n"), reg_value));

        /* Also decode the register fields. */
        DEBUGMSG(ZONE_INFO, (_T("            STDCSSISEL = %d (Digital ")
                 _T("Audio Bus %d)\n"),
                 GET_BITS(regSTEREO_DAC, STDCSSISEL, reg_value),
                 (GET_BITS(regSTEREO_DAC, STDCSSISEL, reg_value)) ? 1 : 0));
        DEBUGMSG(ZONE_INFO, (_T("            STDCCLKSEL = %d (CLI%s)\n"),
                 GET_BITS(regSTEREO_DAC, STDCCLKSEL, reg_value),
                 (GET_BITS(regSTEREO_DAC, STDCCLKSEL, reg_value)) ?
                 _T("B") : _T("A")));
        DEBUGMSG(ZONE_INFO, (_T("                STDCSM = %d (%s Mode)\n"),
                 GET_BITS(regSTEREO_DAC, STDCSM, reg_value),
                 (GET_BITS(regSTEREO_DAC, STDCSM, reg_value)) ?
                 _T("Slave") : _T("Master")));
        DEBUGMSG(ZONE_INFO, (_T("            STDCBCLINV = %d (%s)\n"),
                 GET_BITS(regSTEREO_DAC, STDCBCLINV, reg_value),
                 (GET_BITS(regSTEREO_DAC, STDCBCLINV, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("             STDCFSINV = %d (%s)\n"),
                 GET_BITS(regSTEREO_DAC, STDCFSINV, reg_value),
                 (GET_BITS(regSTEREO_DAC, STDCFSINV, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                STDCFS = 0x%x ("),
                 GET_BITS(regSTEREO_DAC, STDCFS, reg_value)));
        switch (GET_BITS(regSTEREO_DAC, STDCFS, reg_value))
        {
            case 0:
                DEBUGMSG(ZONE_INFO, (_T("Normal Mode")));
                break;
            case 1:
                DEBUGMSG(ZONE_INFO, (_T("Network Mode")));
                break;
            case 2:
                DEBUGMSG(ZONE_INFO, (_T("I2S Mode")));
                break;
            default:
                DEBUGMSG(ZONE_INFO, (_T("Not Supported")));
                break;
        }
        DEBUGMSG(ZONE_INFO, (_T(")\n")));
        DEBUGMSG(ZONE_INFO, (_T("               STDCCLK = 0x%x ("),
                 GET_BITS(regSTEREO_DAC, STDCCLK, reg_value)));
        switch (GET_BITS(regSTEREO_DAC, STDCCLK, reg_value))
        {
            case 0:
                DEBUGMSG(ZONE_INFO, (_T("CLI = 13.0 MHz")));
                break;
            case 1:
                DEBUGMSG(ZONE_INFO, (_T("CLI = 15.36 MHz")));
                break;
            case 2:
                DEBUGMSG(ZONE_INFO, (_T("CLI = 16.8 MHz")));
                break;
            case 4:
                DEBUGMSG(ZONE_INFO, (_T("CLI = 26 MHz")));
                break;
            case 5:
                DEBUGMSG(ZONE_INFO,
                    ((GET_BITS(regSTEREO_DAC, STDCSM, reg_value)) ?
                    _T("CLI = MCL, PLL disabled") :  /* Slave Mode  */
                    _T("CLI = 12 MHz")));            /* Master Mode */
                break;
            case 6:
                DEBUGMSG(ZONE_INFO,
                    ((GET_BITS(regSTEREO_DAC, STDCSM, reg_value)) ?
                    _T("FSYNC pin") :              /* Slave Mode  */
                    _T("CLI = 3.6864 MHz")));      /* Master Mode */
                break;
            case 7:
                DEBUGMSG(ZONE_INFO,
                    ((GET_BITS(regSTEREO_DAC, STDCSM, reg_value)) ?
                    _T("BitCLK pin") :             /* Slave Mode  */
                    _T("CLI = 33.6 MHz")));        /* Master Mode */
                break;
            default:
                DEBUGMSG(ZONE_INFO, (_T("INVALID VALUE")));
        }
        DEBUGMSG(ZONE_INFO, (_T(")\n")));
        DEBUGMSG(ZONE_INFO, (_T("            STDCFSDLYB = %d (%s)\n"),
                 GET_BITS(regSTEREO_DAC, STDCFSDLYB, reg_value),
                 (GET_BITS(regSTEREO_DAC, STDCFSDLYB, reg_value)) ?
                 _T("Disabled") : _T("Enabled")));
        DEBUGMSG(ZONE_INFO, (_T("                STDCEN = %d (%s)\n"),
                 GET_BITS(regSTEREO_DAC, STDCEN, reg_value),
                 (GET_BITS(regSTEREO_DAC, STDCEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("             STDCCLKEN = %d (%s)\n"),
                 GET_BITS(regSTEREO_DAC, STDCCLKEN, reg_value),
                 (GET_BITS(regSTEREO_DAC, STDCCLKEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("             STDCRESET = %d (Should ")
                 _T("always read back as 0)\n"),
                 GET_BITS(regSTEREO_DAC, STDCRESET, reg_value)));
        DEBUGMSG(ZONE_INFO, (_T("                 SPDIF = %d (Mode ")
                 _T("no longer available)\n"),
                 GET_BITS(regSTEREO_DAC, SPDIF, reg_value)));
        DEBUGMSG(ZONE_INFO, (_T("                    SR = 0x%x ("),
                 GET_BITS(regSTEREO_DAC, SR, reg_value)));
        switch (GET_BITS(regSTEREO_DAC, SR, reg_value))
        {
            case 0:
                DEBUGMSG(ZONE_INFO, (_T("8 kHz")));
                break;
            case 1:
                DEBUGMSG(ZONE_INFO, (_T("11.025 kHz")));
                break;
            case 2:
                DEBUGMSG(ZONE_INFO, (_T("12 kHz")));
                break;
            case 3:
                DEBUGMSG(ZONE_INFO, (_T("16 kHz")));
                break;
            case 4:
                DEBUGMSG(ZONE_INFO, (_T("22.05 kHz")));
                break;
            case 5:
                DEBUGMSG(ZONE_INFO, (_T("24 kHz")));
                break;
            case 6:
                DEBUGMSG(ZONE_INFO, (_T("32 kHz")));
                break;
            case 7:
                DEBUGMSG(ZONE_INFO, (_T("44.1 kHz")));
                break;
            case 8:
                DEBUGMSG(ZONE_INFO, (_T("48 kHz")));
                break;
            case 9:
                DEBUGMSG(ZONE_INFO, (_T("64 kHz")));
                break;
            case 10:
                DEBUGMSG(ZONE_INFO, (_T("96 kHz")));
                break;
            default:
                DEBUGMSG(ZONE_INFO, (_T("INVALID VALUE")));
        }
        DEBUGMSG(ZONE_INFO, (_T(")\n")));
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (_T("!!! ERROR: Failed to read ")
                 _T("STEREO_DAC register !!!\n")));
    }

    /* Dump the RX0 (audio output section) register. */
    if (pmic_read_reg(REG_RX0, &reg_value) == PMIC_SUCCESS)
    {
        DEBUGMSG(ZONE_INFO, (_T("\n  RX0 = 0x%08x\n"), reg_value));

        /* Also decode the register fields. */
        DEBUGMSG(ZONE_INFO, (_T("              VAUDIOON = %d (%s)\n"),
                 GET_BITS(regRX0, VAUDIOON, reg_value),
                 (GET_BITS(regRX0, VAUDIOON, reg_value)) ?
                 _T("VAUDIO Forced in On Mode") :
                 _T("Use VAUDIOEN, VAUDIOMODE, VAUDIOSTBY")));
        DEBUGMSG(ZONE_INFO, (_T("                BIASEN = %d (%s)\n"),
                 GET_BITS(regRX0, BIASEN, reg_value),
                 (GET_BITS(regRX0, BIASEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("             BIASSPEED = %d (%s)\n"),
                 GET_BITS(regRX0, BIASSPEED, reg_value),
                 (GET_BITS(regRX0, BIASSPEED, reg_value)) ?
                 _T("10 ms") : _T("100 ms - 150 ms")));
        DEBUGMSG(ZONE_INFO, (_T("                 ASPEN = %d (%s)\n"),
                 GET_BITS(regRX0, ASPEN, reg_value),
                 (GET_BITS(regRX0, ASPEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                ASPSEL = %d (%s)\n"),
                 GET_BITS(regRX0, ASPSEL, reg_value),
                 (GET_BITS(regRX0, ASPSEL, reg_value)) ?
                 _T("Right Channel") : _T("Voice CODEC")));
        DEBUGMSG(ZONE_INFO, (_T("                ALSPEN = %d (%s)\n"),
                 GET_BITS(regRX0, ALSPEN, reg_value),
                 (GET_BITS(regRX0, ALSPEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("               ALSPREF = %d (%s)\n"),
                 GET_BITS(regRX0, ALSPREF, reg_value),
                 (GET_BITS(regRX0, ALSPREF, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("               ALSPSEL = %d (%s)\n"),
                 GET_BITS(regRX0, ALSPSEL, reg_value),
                 (GET_BITS(regRX0, ALSPSEL, reg_value)) ?
                 _T("Right Channel") : _T("Voice CODEC")));
        DEBUGMSG(ZONE_INFO, (_T("                LSPLEN = %d (%s)\n"),
                 GET_BITS(regRX0, LSPLEN, reg_value),
                 (GET_BITS(regRX0, LSPLEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                AHSREN = %d (%s)\n"),
                 GET_BITS(regRX0, AHSREN, reg_value),
                 (GET_BITS(regRX0, AHSREN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                AHSLEN = %d (%s)\n"),
                 GET_BITS(regRX0, AHSLEN, reg_value),
                 (GET_BITS(regRX0, AHSLEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                AHSSEL = %d (%s)\n"),
                 GET_BITS(regRX0, AHSSEL, reg_value),
                 (GET_BITS(regRX0, AHSSEL, reg_value)) ?
                 _T("Right+Left Channels") : _T("Voice CODEC")));
        DEBUGMSG(ZONE_INFO, (_T("               HSPGDIS = %d (")
                 _T("Phantom Ground %s)\n"),
                 GET_BITS(regRX0, HSPGDIS, reg_value),
                 (GET_BITS(regRX0, HSPGDIS, reg_value)) ?
                 _T("Disabled") : _T("Enabled")));
        DEBUGMSG(ZONE_INFO, (_T("               HSDETEN = %d (%s)\n"),
                 GET_BITS(regRX0, HSDETEN, reg_value),
                 (GET_BITS(regRX0, HSDETEN, reg_value)) ?
                 _T("Headset Detect Enabled") : _T("Headset Detect Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("            HSDETAUTOB = %d (Function %s)\n"),
                 GET_BITS(regRX0, HSDETAUTOB, reg_value),
                 (GET_BITS(regRX0, HSDETAUTOB, reg_value)) ?
                 _T("Disabled") : _T("Enabled")));
        DEBUGMSG(ZONE_INFO, (_T("             ARXOUTREN = %d (RXOUTR %s)\n"),
                 GET_BITS(regRX0, ARXOUTREN, reg_value),
                 (GET_BITS(regRX0, ARXOUTREN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("             ARXOUTLEN = %d (RXOUTL %s)\n"),
                 GET_BITS(regRX0, ARXOUTREN, reg_value),
                 (GET_BITS(regRX0, ARXOUTLEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("             ARXOUTSEL = %d (%s)\n"),
                 GET_BITS(regRX0, ARXOUTSEL, reg_value),
                 (GET_BITS(regRX0, ARXOUTSEL, reg_value)) ?
                 _T("Right+Left Channels") : _T("Voice CODEC")));
        DEBUGMSG(ZONE_INFO, (_T("              CDCOUTEN = %d (")
                 _T("Voice CODEC Output %s)\n"),
                 GET_BITS(regRX0, CDCOUTEN, reg_value),
                 (GET_BITS(regRX0, CDCOUTEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("              HSLDETEN = %d (%s)\n"),
                 GET_BITS(regRX0, HSLDETEN, reg_value),
                 (GET_BITS(regRX0, HSLDETEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                ADDCDC = %d (%s)\n"),
                 GET_BITS(regRX0, ADDCDC, reg_value),
                 (GET_BITS(regRX0, ADDCDC, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("               ADDSTDC = %d (%s)\n"),
                 GET_BITS(regRX0, ADDSTDC, reg_value),
                 (GET_BITS(regRX0, ADDSTDC, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("               ADDRXIN = %d (%s)\n"),
                 GET_BITS(regRX0, ADDRXIN, reg_value),
                 (GET_BITS(regRX0, ADDRXIN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (_T("!!! ERROR: Failed to read ")
                 _T("RX0 register !!!\n")));
    }

    /* Dump the RX1 (audio output section) register. */
    if (pmic_read_reg(REG_RX1, &reg_value) == PMIC_SUCCESS)
    {
        DEBUGMSG(ZONE_INFO, (_T("\n  RX1 = 0x%08x\n"), reg_value));

        /* Also decode the register fields. */
        DEBUGMSG(ZONE_INFO, (_T("               PGARXEN = %d (%s)\n"),
                 GET_BITS(regRX1, PGARXEN, reg_value),
                 (GET_BITS(regRX1, PGARXEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                 PGARX = %d ("),
                 GET_BITS(regRX1, PGARX, reg_value)));
        if (GET_BITS(regRX1, PGARX, reg_value) <= 2 )
        {
            DEBUGMSG(ZONE_INFO, (_T("-33")));
        }
        else
        {
            DEBUGMSG(ZONE_INFO, (_T("%d"),
                3 * (GET_BITS(regRX1, PGARX, reg_value) - 3) - 30));
        }
        DEBUGMSG(ZONE_INFO, (_T("dB)\n")));
        DEBUGMSG(ZONE_INFO, (_T("               PGASTEN = %d (%s)\n"),
                 GET_BITS(regRX1, PGASTEN, reg_value),
                 (GET_BITS(regRX1, PGASTEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                 PGAST = %d ("),
                 GET_BITS(regRX1, PGAST, reg_value)));
        if (GET_BITS(regRX1, PGAST, reg_value) <= 2 )
        {
            DEBUGMSG(ZONE_INFO, (_T("-33")));
        }
        else
        {
            DEBUGMSG(ZONE_INFO, (_T("%d"),
                3 * (GET_BITS(regRX1, PGAST, reg_value) - 3) - 30));
        }
        DEBUGMSG(ZONE_INFO, (_T("dB)\n")));
        DEBUGMSG(ZONE_INFO, (_T("               ARXINEN = %d (%s)\n"),
                 GET_BITS(regRX1, ARXINEN, reg_value),
                 (GET_BITS(regRX1, ARXINEN, reg_value)) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                 ARXIN = %d (%s)\n"),
                 GET_BITS(regRX1, ARXIN, reg_value),
                 (GET_BITS(regRX1, ARXIN, reg_value)) ?
                 _T("+18dB") : _T("+0dB")));
        DEBUGMSG(ZONE_INFO, (_T("               PGARXIN = %d ("),
                 GET_BITS(regRX1, PGARXIN, reg_value)));
        if (GET_BITS(regRX1, PGARXIN, reg_value) <= 2 )
        {
            DEBUGMSG(ZONE_INFO, (_T("-33")));
        }
        else
        {
            DEBUGMSG(ZONE_INFO, (_T("%d"),
                3 * (GET_BITS(regRX1, PGARXIN, reg_value) - 3) - 30));
        }
        DEBUGMSG(ZONE_INFO, (_T("dB)\n")));
        DEBUGMSG(ZONE_INFO, (_T("                  MONO = %d ("),
                 GET_BITS(regRX1, MONO, reg_value)));
        switch (GET_BITS(regRX1, MONO, reg_value))
        {
            case 0:
                DEBUGMSG(ZONE_INFO, (_T("Independent Left/Right Channels")));
                break;
            case 1:
                DEBUGMSG(ZONE_INFO, (_T("Stereo Opposite")));
                break;
            case 2:
                DEBUGMSG(ZONE_INFO, (_T("Stereo to Mono Conversion")));
                break;
            case 3:
                DEBUGMSG(ZONE_INFO, (_T("Mono Opposite")));
                break;
            default:
                DEBUGMSG(ZONE_INFO, (_T("INVALID VALUE")));
                break;
        }
        DEBUGMSG(ZONE_INFO, (_T(")\n")));
        DEBUGMSG(ZONE_INFO, (_T("                   BAL = %d (%ddB)\n"),
                 GET_BITS(regRX1, BAL, reg_value),
                 -3 * GET_BITS(regRX1, BAL, reg_value)));
        DEBUGMSG(ZONE_INFO, (_T("                 BALLR = %d (%s Channel)\n"),
                 GET_BITS(regRX1, BALLR, reg_value),
                 GET_BITS(regRX1, BALLR, reg_value) ? _T("Left") : _T("Right")));
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (_T("!!! ERROR: Failed to read ")
                 _T("RX1 register !!!\n")));
    }

    /* Dump the AUDIO_TX (audio input section) register. */
    if (pmic_read_reg(REG_AUDIO_TX, &reg_value) == PMIC_SUCCESS)
    {
        DEBUGMSG(ZONE_INFO, (_T("\n  AUDIO_TX = 0x%08x\n"), reg_value));

        /* Also decode the register fields. */
        DEBUGMSG(ZONE_INFO, (_T("                MC1BEN = %d (%s)\n"),
                 GET_BITS(regAUDIO_TX, MC1BEN, reg_value),
                 GET_BITS(regAUDIO_TX, MC1BEN, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                MC2BEN = %d (%s)\n"),
                 GET_BITS(regAUDIO_TX, MC2BEN, reg_value),
                 GET_BITS(regAUDIO_TX, MC2BEN, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("           MC2BDETDBNC = %d (%s Debounce)\n"),
                 GET_BITS(regAUDIO_TX, MC2BDETDBNC, reg_value),
                 GET_BITS(regAUDIO_TX, MC2BDETDBNC, reg_value) ?
                 _T("100ms") : _T("30ms")));
        DEBUGMSG(ZONE_INFO, (_T("             MC2BDETEN = %d (%s)\n"),
                 GET_BITS(regAUDIO_TX, MC2BDETEN, reg_value),
                 GET_BITS(regAUDIO_TX, MC2BDETEN, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("               AMC1REN = %d (%s)\n"),
                 GET_BITS(regAUDIO_TX, AMC1REN, reg_value),
                 GET_BITS(regAUDIO_TX, AMC1REN, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("             AMC1RITOV = %d (%s Mode)\n"),
                 GET_BITS(regAUDIO_TX, AMC1RITOV, reg_value),
                 GET_BITS(regAUDIO_TX, AMC1RITOV, reg_value) ?
                 _T("I-to-V") : _T("V-to-V")));
        DEBUGMSG(ZONE_INFO, (_T("               AMC1LEN = %d (%s)\n"),
                 GET_BITS(regAUDIO_TX, AMC1LEN, reg_value),
                 GET_BITS(regAUDIO_TX, AMC1LEN, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("             AMC1LITOV = %d (%s Mode)\n"),
                 GET_BITS(regAUDIO_TX, AMC1LITOV, reg_value),
                 GET_BITS(regAUDIO_TX, AMC1LITOV, reg_value) ?
                 _T("I-to-V") : _T("V-to-V")));
        DEBUGMSG(ZONE_INFO, (_T("                AMC2EN = %d (%s)\n"),
                 GET_BITS(regAUDIO_TX, AMC2EN, reg_value),
                 GET_BITS(regAUDIO_TX, AMC2EN, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("              AMC2ITOV = %d (%s Mode)\n"),
                 GET_BITS(regAUDIO_TX, AMC2ITOV, reg_value),
                 GET_BITS(regAUDIO_TX, AMC2ITOV, reg_value) ?
                 _T("I-to-V") : _T("V-to-V")));
        DEBUGMSG(ZONE_INFO, (_T("               ATXINEN = %d (%s)\n"),
                 GET_BITS(regAUDIO_TX, ATXINEN, reg_value),
                 GET_BITS(regAUDIO_TX, ATXINEN, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
#if 0 /* This field is reserved but currently not used. */
        DEBUGMSG(ZONE_INFO, (_T("              ATXOUTEN = %d (%s)\n"),
                 GET_BITS(regAUDIO_TX, ATXOUTEN, reg_value),
                 GET_BITS(regAUDIO_TX, ATXOUTEN, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
#endif
        DEBUGMSG(ZONE_INFO, (_T("               RXINREC = %d (%s)\n"),
                 GET_BITS(regAUDIO_TX, RXINREC, reg_value),
                 GET_BITS(regAUDIO_TX, RXINREC, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("                PGATXR = %d (%ddB)\n"),
                 GET_BITS(regAUDIO_TX, PGATXR, reg_value),
                 GET_BITS(regAUDIO_TX, PGATXR, reg_value) - 8));
        DEBUGMSG(ZONE_INFO, (_T("                PGATXL = %d (%ddB)\n"),
                 GET_BITS(regAUDIO_TX, PGATXL, reg_value),
                 GET_BITS(regAUDIO_TX, PGATXL, reg_value) - 8));
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (_T("!!! ERROR: Failed to read ")
                 _T("AUDIO_TX register !!!\n")));
    }

    /* Dump the SSI_NETWORK (audio input section) register. */
    if (pmic_read_reg(REG_SSI_NETWORK, &reg_value) == PMIC_SUCCESS)
    {
        DEBUGMSG(ZONE_INFO, (_T("\n  SSI_NETWORK = 0x%08x\n"), reg_value));

        DEBUGMSG(ZONE_INFO, (_T("           CDCTXRXSLOT = %d (use time ")
                 _T("slot TS%d)\n"),
                 GET_BITS(regSSI_NETWORK, CDCTXRXSLOT, reg_value),
                 GET_BITS(regSSI_NETWORK, CDCTXRXSLOT, reg_value)));
        DEBUGMSG(ZONE_INFO, (_T("          CDCTXSECSLOT = %d (use time ")
                 _T("slot TS%d)\n"),
                 GET_BITS(regSSI_NETWORK, CDCTXSECSLOT, reg_value),
                 GET_BITS(regSSI_NETWORK, CDCTXSECSLOT, reg_value)));
        DEBUGMSG(ZONE_INFO, (_T("          CDCRXSECSLOT = %d (use time ")
                 _T("slot TS%d)\n"),
                 GET_BITS(regSSI_NETWORK, CDCRXSECSLOT, reg_value),
                 GET_BITS(regSSI_NETWORK, CDCRXSECSLOT, reg_value)));
        DEBUGMSG(ZONE_INFO, (_T("          CDCRXSECGAIN = %d ("),
                 GET_BITS(regSSI_NETWORK, CDCRXSECGAIN, reg_value)));
        switch (GET_BITS(regSSI_NETWORK, CDCRXSECGAIN, reg_value))
        {
            case 0:
                DEBUGMSG(ZONE_INFO, (_T("No mixing")));
                break;
            case 1:
                DEBUGMSG(ZONE_INFO, (_T("0dB")));
                break;
            case 2:
                DEBUGMSG(ZONE_INFO, (_T("-6dB")));
                break;
            case 3:
                DEBUGMSG(ZONE_INFO, (_T("-12dB")));
                break;
            default:
                DEBUGMSG(ZONE_INFO, (_T("INVALID VALUE")));
                break;
        }
        DEBUGMSG(ZONE_INFO, (_T(")\n")));
        DEBUGMSG(ZONE_INFO, (_T("            CDCSUMGAIN = %d (%sdB)\n"),
                 GET_BITS(regSSI_NETWORK, CDCSUMGAIN, reg_value),
                 GET_BITS(regSSI_NETWORK, CDCSUMGAIN, reg_value) ?
                 _T("-6") : _T("0")));
        DEBUGMSG(ZONE_INFO, (_T("              CDCFSDLY = %d (%s)\n"),
                 GET_BITS(regSSI_NETWORK, CDCFSDLY, reg_value),
                 GET_BITS(regSSI_NETWORK, CDCFSDLY, reg_value) ?
                 _T("Enabled") : _T("Disabled")));
        DEBUGMSG(ZONE_INFO, (_T("             STDCSLOTS = %d ("),
                 GET_BITS(regSSI_NETWORK, STDCSLOTS, reg_value)));
        switch (GET_BITS(regSSI_NETWORK, STDCSLOTS, reg_value))
        {
            case 0:
                DEBUGMSG(ZONE_INFO, (_T("8 (for 64/96 kHz modes only)")));
                break;
            case 1:
                DEBUGMSG(ZONE_INFO, (_T("8 (Left, Right, 6 other)")));
                break;
            case 2:
                DEBUGMSG(ZONE_INFO, (_T("4 (Left, Right, 2 other)")));
                break;
            case 3:
                DEBUGMSG(ZONE_INFO, (_T("2 (Left, Right)")));
                break;
            default:
                DEBUGMSG(ZONE_INFO, (_T("INVALID VALUE")));
                break;
        }
        DEBUGMSG(ZONE_INFO, (_T(" timeslots)\n")));
        DEBUGMSG(ZONE_INFO, (_T("            STDCRXSLOT = %d (use time ")
                 _T("slots TS%d and TS%d)\n"),
                 GET_BITS(regSSI_NETWORK, STDCRXSLOT, reg_value),
                 2 * GET_BITS(regSSI_NETWORK, STDCRXSLOT, reg_value),
                 2 * GET_BITS(regSSI_NETWORK, STDCRXSLOT, reg_value) + 1));
        DEBUGMSG(ZONE_INFO, (_T("            STDCRXSECSLOT = %d (use time ")
                 _T("slots TS%d and TS%d)\n"),
                 GET_BITS(regSSI_NETWORK, STDCRXSECSLOT, reg_value),
                 2 * GET_BITS(regSSI_NETWORK, STDCRXSECSLOT, reg_value),
                 2 * GET_BITS(regSSI_NETWORK, STDCRXSECSLOT, reg_value) + 1));
        DEBUGMSG(ZONE_INFO, (_T("            STDCRXSECGAIN = %d ("),
                 GET_BITS(regSSI_NETWORK, STDCRXSECGAIN, reg_value)));
        switch (GET_BITS(regSSI_NETWORK, STDCRXSECGAIN, reg_value))
        {
            case 0:
                DEBUGMSG(ZONE_INFO, (_T("No Mixing")));
                break;
            case 1:
                DEBUGMSG(ZONE_INFO, (_T("0dB")));
                break;
            case 2:
                DEBUGMSG(ZONE_INFO, (_T("-6dB")));
                break;
            case 3:
                DEBUGMSG(ZONE_INFO, (_T("-12dB")));
                break;
            default:
                DEBUGMSG(ZONE_INFO, (_T("INVALID VALUE")));
                break;
        }
        DEBUGMSG(ZONE_INFO, (_T(")\n")));
        DEBUGMSG(ZONE_INFO, (_T("           STDCSUMGAIN = %d (%sdB)\n"),
                 GET_BITS(regSSI_NETWORK, STDCSUMGAIN, reg_value),
                 GET_BITS(regSSI_NETWORK, STDCSUMGAIN, reg_value) ?
                 _T("-6") : _T("0")));
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (_T("!!! ERROR: Failed to read ")
                 _T("SSI_NETWORK register !!!\n")));
    }

#endif /* DEBUG */
}

/*@}*/

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
 * @brief Set the Voice CODEC clock source and operating characteristics.
 *
 * Define the Voice CODEC clock source and operating characteristics. This
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
    const PMIC_AUDIO_HANDLE               handle,
    const PMIC_AUDIO_CLOCK_IN_SOURCE      clockIn,
    const PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ clockFreq,
    const PMIC_AUDIO_VCODEC_SAMPLING_RATE samplingRate,
    const PMIC_AUDIO_CLOCK_INVERT         invert)
{
    const unsigned int VCODEC_DISABLE = SET_BITS(regAUDIO_CODEC, CDCEN, 1);
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    /* Validate all of the calling parameters. */
    if ((handle == vCodec.handle)              &&
        (vCodec.handleState == HANDLE_IN_USE)  &&
        ((clockIn == CLOCK_IN_DEFAULT) ||
         (clockIn == CLOCK_IN_CLIA)    ||
         (clockIn == CLOCK_IN_CLIB))      &&
        ((clockFreq == VCODEC_CLI_13MHZ)    ||
         (clockFreq == VCODEC_CLI_15_36MHZ) ||
         (clockFreq == VCODEC_CLI_16_8MHZ)  ||
         (clockFreq == VCODEC_CLI_26MHZ)    ||
         (clockFreq == VCODEC_CLI_33_6MHZ))    &&
        ((samplingRate == VCODEC_RATE_8_KHZ) ||
         (samplingRate == VCODEC_RATE_16_KHZ)))
    {
        /* Start by disabling the Voice CODEC and any existing clock output. */
        pmic_write_reg(REG_AUDIO_CODEC, 0, VCODEC_DISABLE);

        /* Pause briefly to ensure that the Voice CODEC is completely
         * disabled before changing the clock configuration.
         */
        PmicAudioTimedDelay(10 * delay_1ms);

        /* Select the clock input source. */
        reg_mask = SET_BITS(regAUDIO_CODEC, CDCCLKSEL, 1);
        if ((clockIn == CLOCK_IN_DEFAULT) || (clockIn == CLOCK_IN_CLIB))
        {
            reg_write = SET_BITS(regAUDIO_CODEC, CDCCLKSEL, 1);
        }

        /* Set the CDC_CLK bits to match the input clock frequency. */
        reg_mask |= SET_BITS(regAUDIO_CODEC, CDCCLK, 7);
        if (clockFreq == VCODEC_CLI_15_36MHZ)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCCLK, 1);
        }
        else if (clockFreq == VCODEC_CLI_16_8MHZ)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCCLK, 2);
        }
        else if (clockFreq == VCODEC_CLI_26MHZ)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCCLK, 4);
        }
        else if (clockFreq == VCODEC_CLI_33_6MHZ)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCCLK, 7);
        }

        /* Set the desired sampling rate. */
        reg_mask |= SET_BITS(regAUDIO_CODEC, CDCFS8K16K, 1);
        if (samplingRate == VCODEC_RATE_16_KHZ)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCFS8K16K, 1);
        }

        /* Set the optional clock inversion.
         *
         * Note that the clock and framesync inversion have to be set
         * separately. Therefore, we'll set the clock inversion first
         * here if needed and then set the framesync inversion later
         * if also required.
         */
        reg_mask |= SET_BITS(regAUDIO_CODEC, CDCBCLINV, 1);
        if (invert & INVERT_BITCLOCK)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCBCLINV, 1);
        }

        /* Write the configuration to the AUDIO_CODEC register. */
        rc = pmic_write_reg(REG_AUDIO_CODEC, reg_write, reg_mask);

        /* Set the optional frame sync inversion.
         *
         * Note that this cannot be done at the same time that the clock
         * inversion is being set.
         */
        if (rc == PMIC_SUCCESS)
        {
            reg_mask = SET_BITS(regAUDIO_CODEC, CDCFSINV, 1);
            if (invert & INVERT_FRAMESYNC)
            {
                reg_write = SET_BITS(regAUDIO_CODEC, CDCFSINV, 1);
            }
            rc = pmic_write_reg(REG_AUDIO_CODEC, reg_write, reg_mask);
        }

        if (rc == PMIC_SUCCESS)
        {
            vCodec.clockIn      = clockIn;
            vCodec.clockFreq    = clockFreq;
            vCodec.samplingRate = samplingRate;
            vCodec.invert       = invert;
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure that we return a consistent state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)                                 &&
        (vCodec.handleState == HANDLE_IN_USE)                     &&
        (clockIn != (PMIC_AUDIO_CLOCK_IN_SOURCE *)NULL)           &&
        (clockFreq != (PMIC_AUDIO_VCODEC_CLOCK_IN_FREQ *)NULL)    &&
        (samplingRate != (PMIC_AUDIO_VCODEC_SAMPLING_RATE *)NULL) &&
        (invert != (PMIC_AUDIO_CLOCK_INVERT *)NULL))
    {
        *clockIn      = vCodec.clockIn;
        *clockFreq    = vCodec.clockFreq;
        *samplingRate = vCodec.samplingRate;
        *invert       = vCodec.invert;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    const unsigned int reg_mask = SET_BITS(regSSI_NETWORK, CDCTXRXSLOT, 3);

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)                         &&
        (vCodec.handleState == HANDLE_IN_USE)             &&
        ((timeslot == USE_TS0) || (timeslot == USE_TS1) ||
         (timeslot == USE_TS2) || (timeslot == USE_TS3)))
    {
        reg_write = SET_BITS(regSSI_NETWORK, CDCTXRXSLOT, timeslot);

        rc = pmic_write_reg(REG_SSI_NETWORK, reg_write, reg_mask);

        if (rc == PMIC_SUCCESS)
        {
            vCodec.timeslot = timeslot;
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)                         &&
        (vCodec.handleState == HANDLE_IN_USE)             &&
        (timeslot != (PMIC_AUDIO_VCODEC_TIMESLOT *)NULL))
    {
        *timeslot = vCodec.timeslot;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    const unsigned int reg_mask = SET_BITS(regSSI_NETWORK, CDCTXSECSLOT, 3);
    unsigned int reg_write = 0;
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        reg_write = SET_BITS(regSSI_NETWORK, CDCTXSECSLOT, timeslot);

        rc = pmic_write_reg(REG_SSI_NETWORK, reg_write, reg_mask);

        if (rc == PMIC_SUCCESS)
        {
            vCodec.timeslot = timeslot;
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)                         &&
        (vCodec.handleState == HANDLE_IN_USE)             &&
        (timeslot != (PMIC_AUDIO_VCODEC_TIMESLOT *)NULL))
    {
        rc = PMIC_NOT_SUPPORTED;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)             &&
        (vCodec.handleState == HANDLE_IN_USE))
    {
        if (config & DITHERING)
        {
            reg_write = SET_BITS(regAUDIO_CODEC, CDCDITH, 1);
            reg_mask  = SET_BITS(regAUDIO_CODEC, CDCDITH, 1);
        }

        if (config & INPUT_HIGHPASS_FILTER)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, AUDIHPF, 1);
            reg_mask  |= SET_BITS(regAUDIO_CODEC, AUDIHPF, 1);
        }

        if (config & OUTPUT_HIGHPASS_FILTER)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, AUDOHPF, 1);
            reg_mask  |= SET_BITS(regAUDIO_CODEC, AUDOHPF, 1);
        }

        if (config & DIGITAL_LOOPBACK)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCDLM, 1);
            reg_mask  |= SET_BITS(regAUDIO_CODEC, CDCDLM, 1);
        }

        if (config & ANALOG_LOOPBACK)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCALM, 1);
            reg_mask  |= SET_BITS(regAUDIO_CODEC, CDCALM, 1);
        }

        if (config & VCODEC_MASTER_CLOCK_OUTPUTS)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCCLKEN, 1);
            reg_mask  |= SET_BITS(regAUDIO_CODEC, CDCCLKEN, 1);
        }

        if (config & TRISTATE_TS)
        {
            reg_write |= SET_BITS(regAUDIO_CODEC, CDCTS, 1);
            reg_mask  |= SET_BITS(regAUDIO_CODEC, CDCTS, 1);
        }

        if (reg_mask == 0)
        {
            /* We should not reach this point without having to configure
             * anything so we flag it as an error.
             */
            rc = PMIC_ERROR;
        }
        else
        {
            rc = pmic_write_reg(REG_AUDIO_CODEC, reg_write, reg_mask);
        }

        if (rc == PMIC_SUCCESS)
        {
            vCodec.config = PMIC_AUDIO_VCODEC_CONFIG(vCodec.config | config);
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)             &&
        (vCodec.handleState == HANDLE_IN_USE))
    {
        if (config & DITHERING)
        {
            reg_mask = SET_BITS(regAUDIO_CODEC, CDCDITH, 1);
        }

        if (config & INPUT_HIGHPASS_FILTER)
        {
            reg_mask |= SET_BITS(regAUDIO_CODEC, AUDIHPF, 1);
        }

        if (config & OUTPUT_HIGHPASS_FILTER)
        {
            reg_mask |= SET_BITS(regAUDIO_CODEC, AUDOHPF, 1);
        }

        if (config & DIGITAL_LOOPBACK)
        {
            reg_mask |= SET_BITS(regAUDIO_CODEC, CDCDLM, 1);
        }

        if (config & ANALOG_LOOPBACK)
        {
            reg_mask |= SET_BITS(regAUDIO_CODEC, CDCALM, 1);
        }

        if (config & VCODEC_MASTER_CLOCK_OUTPUTS)
        {
            reg_mask |= SET_BITS(regAUDIO_CODEC, CDCCLKEN, 1);
        }

        if (config & TRISTATE_TS)
        {
            reg_mask |= SET_BITS(regAUDIO_CODEC, CDCTS, 1);
        }

        if (reg_mask == 0)
        {
            /* We can reach this point without having to write anything
             * to the AUD_CODEC register since there is a possible no-op
             * for the VCODEC_RESET_DIGITAL_FILTER option. Therefore, we
             * just assume success and proceed.
             */
            rc = PMIC_SUCCESS;
        }
        else
        {
            rc = pmic_write_reg(REG_AUDIO_CODEC, reg_write, reg_mask);
        }

        if (rc == PMIC_SUCCESS)
        {
            vCodec.config = PMIC_AUDIO_VCODEC_CONFIG(vCodec.config & ~config);
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)                    &&
        (vCodec.handleState == HANDLE_IN_USE)        &&
        (config != (PMIC_AUDIO_VCODEC_CONFIG *)NULL))
    {
        *config = vCodec.config;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_write = SET_BITS(regAUDIO_CODEC, CDCBYP, 1);
    const unsigned int reg_mask  = SET_BITS(regAUDIO_CODEC, CDCBYP, 1);

    /* No critical section required here since we are not updating any
     * global data.
     */

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        rc = pmic_write_reg(REG_AUDIO_CODEC, reg_write, reg_mask);
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_write = 0;
    const unsigned int reg_mask  = SET_BITS(regAUDIO_CODEC, CDCBYP, 1);

    /* No critical section required here since we are not updating any
     * global data.
     */

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        rc = pmic_write_reg(REG_AUDIO_CODEC, reg_write, reg_mask);
    }

    return rc;
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
    const PMIC_AUDIO_HANDLE              handle,
    const PMIC_AUDIO_CLOCK_IN_SOURCE     clockIn,
    const PMIC_AUDIO_STDAC_CLOCK_IN_FREQ clockFreq,
    const PMIC_AUDIO_STDAC_SAMPLING_RATE samplingRate,
    const PMIC_AUDIO_CLOCK_INVERT        invert)
{
    const unsigned int STDAC_DISABLE = SET_BITS(regSTEREO_DAC, STDCEN, 1);
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    /* Validate all of the calling parameters. */
    if ((handle == stDAC.handle)              &&
        (stDAC.handleState == HANDLE_IN_USE)  &&
        ((clockIn == CLOCK_IN_DEFAULT) ||
         (clockIn == CLOCK_IN_CLIA)    ||
         (clockIn == CLOCK_IN_CLIB)    ||
         (clockIn == CLOCK_IN_FSYNC)   ||
         (clockIn == CLOCK_IN_BITCLK))        &&
        ((clockFreq == STDAC_CLI_13MHZ)          ||
         (clockFreq == STDAC_CLI_15_36MHZ)       ||
         (clockFreq == STDAC_CLI_16_8MHZ)        ||
         (clockFreq == STDAC_CLI_26MHZ)          ||
         (clockFreq == STDAC_CLI_33_6MHZ)        ||
         (clockFreq == STDAC_MCLK_PLL_DISABLED)  ||
         (clockFreq == STDAC_FSYNC_IN_PLL)       ||
         (clockFreq == STDAC_BCLK_IN_PLL)) &&
        ((samplingRate >= STDAC_RATE_8_KHZ) &&
         (samplingRate <= STDAC_RATE_96_KHZ)))
    {
        /* Also verify that the clock input source and frequency are valid. */
        if ((((clockIn == CLOCK_IN_CLIA) || (clockIn == CLOCK_IN_CLIB)) &&
             (clockFreq != STDAC_CLI_13MHZ)    &&
             (clockFreq != STDAC_CLI_15_36MHZ) &&
             (clockFreq != STDAC_CLI_16_8MHZ)  &&
             (clockFreq != STDAC_CLI_26MHZ)    &&
             (clockFreq != STDAC_CLI_33_6MHZ))       ||
            ((clockIn == CLOCK_IN_FSYNC) &&
             (clockFreq != STDAC_FSYNC_IN_PLL))      ||
            ((clockIn == CLOCK_IN_BITCLK) &&
             (clockFreq != STDAC_BCLK_IN_PLL)))
        {
            rc = PMIC_PARAMETER_ERROR;
        }
        else
        {
            /* Start by disabling the Stereo DAC and any existing clock
             * output.
             */
            pmic_write_reg(REG_STEREO_DAC, 0, STDAC_DISABLE);

            /* Pause briefly to ensure that the Stereo DAC is completely
             * disabled before changing the clock configuration.
             */
            PmicAudioTimedDelay(10 * delay_1ms);

            /* Set the STDCCLK bits to match the input clock source and
             * frequency.
             */
            reg_mask |= regSTEREO_DAC.STDCCLK.mask;

            /* Note that STDAC_CLI_13MHZ corresponds to STDCCLK = 0. */
            if (clockFreq == STDAC_CLI_3_6864MHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCCLK, 6);
            }
            else if (clockFreq == STDAC_CLI_12MHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCCLK, 5);
            }
            else if (clockFreq == STDAC_CLI_15_36MHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCCLK, 1);
            }
            else if (clockFreq == STDAC_CLI_16_8MHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCCLK, 2);
            }
            else if (clockFreq == STDAC_CLI_26MHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCCLK, 4);
            }
            else if (clockFreq == STDAC_CLI_33_6MHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCCLK, 7);
            }
            else if (clockFreq == STDAC_MCLK_PLL_DISABLED)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCCLK, 5);
            }
            else if (clockFreq == STDAC_FSYNC_IN_PLL)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCCLK, 6);
            }
            else if (clockFreq == STDAC_BCLK_IN_PLL)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCCLK, 7);
            }

            /* Set the desired sampling rate. */
            reg_mask |= SET_BITS(regSTEREO_DAC, SR, 15);

            /* Note that SR = 0 for an 8 kHz sampling rate. */
            if (samplingRate == STDAC_RATE_11_025_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 1);
            }
            else if (samplingRate == STDAC_RATE_12_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 2);
            }
            else if (samplingRate == STDAC_RATE_16_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 3);
            }
            else if (samplingRate == STDAC_RATE_22_050_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 4);
            }
            else if (samplingRate == STDAC_RATE_24_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 5);
            }
            else if (samplingRate == STDAC_RATE_32_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 6);
            }
            else if (samplingRate == STDAC_RATE_44_1_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 7);
            }
            else if (samplingRate == STDAC_RATE_48_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 8);
            }
            else if (samplingRate == STDAC_RATE_64_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 9);
            }
            else if (samplingRate == STDAC_RATE_96_KHZ)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, SR, 10);
            }

            /* Set the optional clock inversion. Note that the bit clock
             * and framesync inversion must be set separately. So we'll
             * set the bitclock inversion here if necessary and set the
             * framesync inversion if required later.
             */
            reg_mask |= SET_BITS(regSTEREO_DAC, STDCBCLINV, 1);
            if (invert & INVERT_BITCLOCK)
            {
                reg_write |= SET_BITS(regSTEREO_DAC, STDCBCLINV, 1);
            }

            /* Write the configuration to the STEREO_DAC register. */
            rc = pmic_write_reg(REG_STEREO_DAC, reg_write, reg_mask);

            /* Set the optional frame sync inversion. This must be done
             * separately from setting the bit clock inversion which is
             * why we wait until now to do it.
             */
            if (rc == PMIC_SUCCESS)
            {
                reg_mask = SET_BITS(regSTEREO_DAC, STDCFSINV, 1);
                if (invert & INVERT_FRAMESYNC)
                {
                    reg_write = SET_BITS(regSTEREO_DAC, STDCFSINV, 1);
                }
                rc = pmic_write_reg(REG_STEREO_DAC, reg_write, reg_mask);
            }

            if (rc == PMIC_SUCCESS)
            {
                stDAC.clockIn      = clockIn;
                stDAC.clockFreq    = clockFreq;
                stDAC.samplingRate = samplingRate;
                stDAC.invert       = invert;
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    const PMIC_AUDIO_HANDLE               handle,
    PMIC_AUDIO_CLOCK_IN_SOURCE *const     clockIn,
    PMIC_AUDIO_STDAC_SAMPLING_RATE *const samplingRate,
    PMIC_AUDIO_STDAC_CLOCK_IN_FREQ *const clockFreq,
    PMIC_AUDIO_CLOCK_INVERT *const        invert)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == stDAC.handle)                                 &&
        (stDAC.handleState == HANDLE_IN_USE)                     &&
        (clockIn != (PMIC_AUDIO_CLOCK_IN_SOURCE *)NULL)          &&
        (samplingRate != (PMIC_AUDIO_STDAC_SAMPLING_RATE *)NULL) &&
        (clockFreq != (PMIC_AUDIO_STDAC_CLOCK_IN_FREQ *)NULL)    &&
        (invert != (PMIC_AUDIO_CLOCK_INVERT *)NULL))
    {
        *clockIn      = stDAC.clockIn;
        *samplingRate = stDAC.samplingRate;
        *clockFreq    = stDAC.clockFreq;
        *invert       = stDAC.invert;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    const unsigned int reg_mask = SET_BITS(regSSI_NETWORK, STDCRXSLOT, 3);
    unsigned int reg_write      = 0;
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE) &&
        (timeslot >= USE_TS0_TS1) && (timeslot <= USE_TS6_TS7))
    {
        /* Note that STDCRXSLOT = 0 for timeslots 0 and 1. */
        if (timeslot == USE_TS2_TS3)
        {
            reg_write = SET_BITS(regSSI_NETWORK, STDCRXSLOT, 1);
        }
        else if (timeslot == USE_TS4_TS5)
        {
            reg_write = SET_BITS(regSSI_NETWORK, STDCRXSLOT, 2);
        }
        else if (timeslot == USE_TS6_TS7)
        {
            reg_write = SET_BITS(regSSI_NETWORK, STDCRXSLOT, 3);
        }

        rc = pmic_write_reg(REG_SSI_NETWORK, reg_write, reg_mask);

        if (rc == PMIC_SUCCESS)
        {
            stDAC.timeslot = timeslot;
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == stDAC.handle)                          &&
        (stDAC.handleState == HANDLE_IN_USE)              &&
        (timeslot != (PMIC_AUDIO_STDAC_TIMESLOTS *)NULL))
    {
        *timeslot = stDAC.timeslot;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE))
    {
        if (config & STDAC_MASTER_CLOCK_OUTPUTS)
        {
            reg_write |= SET_BITS(regSTEREO_DAC, STDCCLKEN, 1);
            reg_mask  |= SET_BITS(regSTEREO_DAC, STDCCLKEN, 1);
        }

        rc = pmic_write_reg(REG_STEREO_DAC, reg_write, reg_mask);

        if (rc == PMIC_SUCCESS)
        {
            stDAC.config = PMIC_AUDIO_STDAC_CONFIG(stDAC.config | config);
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE))
    {
        /* Note that clearing the digital filter reset bit is a no-op
         * since this bit is self clearing.
         */

        if (config & STDAC_MASTER_CLOCK_OUTPUTS)
        {
            reg_mask |= SET_BITS(regSTEREO_DAC, STDCCLKEN, 1);
        }

        if (reg_mask != 0)
        {
            rc = pmic_write_reg(REG_STEREO_DAC, reg_write, reg_mask);

            if (rc == PMIC_SUCCESS)
            {
                stDAC.config = PMIC_AUDIO_STDAC_CONFIG(stDAC.config & ~config);
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == stDAC.handle)                     &&
        (stDAC.handleState == HANDLE_IN_USE)         &&
        (config != (PMIC_AUDIO_STDAC_CONFIG *)NULL))
    {
        *config = stDAC.config;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        if ((config & MIC_AMP_AUTO_DISABLE) ||
            (config != MIC2_BIAS_DETECT))
        {
            rc = PMIC_NOT_SUPPORTED;
        }
        else
        {
            /* For the MC13783 PMIC, MC2BDETEN = 1 enables the detection of
             * an out of regulation condition for the microphone bias 2
             * circuit. This condition can be used with microphones that
             * have an in-line switch to generate a hardware interrupt
             * when the switch is pressed.
             */
            reg_write = SET_BITS(regAUDIO_TX, MC2BDETEN, 1);
            reg_mask  = regAUDIO_TX.MC2BDETEN.mask;

            rc = pmic_write_reg(REG_AUDIO_TX, reg_write, reg_mask);
            if (rc == PMIC_SUCCESS)
            {
                vCodec.inputConfig =
                    PMIC_AUDIO_INPUT_CONFIG(vCodec.inputConfig | config);
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        if ((config & MIC_AMP_AUTO_DISABLE) ||
            (config != MIC2_BIAS_DETECT))
        {
            rc = PMIC_NOT_SUPPORTED;
        }
        else
        {
            /* For the MC13783 PMIC, MC2BDETEN = 0 disables the detection of
             * an out of regulation condition for the microphone bias 2
             * circuit. This will prevent the detection of switch presses
             * for microphones that have an in-line switch.
             */
            reg_mask = regAUDIO_TX.MC2BDETEN.mask;

            rc = pmic_write_reg(REG_AUDIO_TX, reg_write, reg_mask);

            if (rc == PMIC_SUCCESS)
            {
                vCodec.inputConfig =
                    PMIC_AUDIO_INPUT_CONFIG(vCodec.inputConfig & ~config);
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)                    &&
        (vCodec.handleState == HANDLE_IN_USE)        &&
        (config != (PMIC_AUDIO_INPUT_CONFIG *)NULL))
    {
        *config = vCodec.inputConfig;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        /* Validate the left/right microphone channel selections.
         *
         * The left microphone channel can either be disabled or connected
         * to the Amc1L input amplifier.
         *
         * The right microphone channel can either be disabled or connected
         * to the Amc1R, Amc2, or Atxin amplifiers.
         */
        if (!((leftChannel == NO_MIC) || (leftChannel == MIC1_LEFT))        ||
            !((rightChannel == NO_MIC)              ||
              (rightChannel == MIC1_RIGHT_MIC_MONO) ||
              (rightChannel == MIC2_AUX)            ||
              (rightChannel == TXIN_EXT))                                   ||
            ((leftChannel == RXIN_STEREO) && (rightChannel != RXIN_STEREO)) ||
            ((leftChannel != RXIN_STEREO) && (rightChannel == RXIN_STEREO)))
        {
            rc = PMIC_PARAMETER_ERROR;
        }
        else
        {
            reg_mask = regAUDIO_TX.AMC1LEN.mask;
            if (leftChannel == MIC1_LEFT)
            {
                /* Enable the left channel microphone input using the
                 * Amc1L amplifier.
                 */
                reg_write = SET_BITS(regAUDIO_TX, AMC1LEN, 1);
            }

            reg_mask |= regAUDIO_TX.AMC1REN.mask |
                        regAUDIO_TX.AMC2EN.mask  |
                        regAUDIO_TX.ATXINEN.mask |
                        regAUDIO_TX.RXINREC.mask;
            if (rightChannel == MIC1_RIGHT_MIC_MONO)
            {
                /* Enable the right channel microphone input using the
                 * Amc1R amplifier.
                 *
                 * We also make sure to disable the other right channel
                 * microphone input amplifiers to avoid any unwanted noise.
                 */
                reg_write |= SET_BITS(regAUDIO_TX, AMC1REN, 1);
            }
            else if (rightChannel == MIC2_AUX)
            {
                /* Enable the right channel microphone input using the
                 * Amc2 amplifier.
                 *
                 * We also make sure to disable the other right channel
                 * microphone input amplifiers to avoid any unwanted noise.
                 */
                reg_write |= SET_BITS(regAUDIO_TX, AMC2EN, 1);
            }
            else if (rightChannel == TXIN_EXT)
            {
                /* Enable the right channel microphone input using the
                 * Atxin amplifier.
                 *
                 * We also make sure to disable the other right channel
                 * microphone input amplifiers to avoid any unwanted noise.
                 */
                reg_write |= SET_BITS(regAUDIO_TX, ATXINEN, 1);
            }
            else if (rightChannel == RXIN_STEREO)
            {
                /* Enable the right (and left) channel external unamplified
                 * input.
                 */
                reg_write |= SET_BITS(regAUDIO_TX, RXINREC, 1);
            }

            rc = pmic_write_reg(REG_AUDIO_TX, reg_write, reg_mask);

            if (rc == PMIC_SUCCESS)
            {
                vCodec.leftChannelMic.mic  = leftChannel;
                vCodec.rightChannelMic.mic = rightChannel;
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)                        &&
        (vCodec.handleState == HANDLE_IN_USE)            &&
        (leftChannel != (PMIC_AUDIO_INPUT_PORT *)NULL)   &&
        (rightChannel != (PMIC_AUDIO_INPUT_PORT *)NULL))
    {
        *leftChannel  = vCodec.leftChannelMic.mic;
        *rightChannel = vCodec.rightChannelMic.mic;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        reg_mask = regAUDIO_TX.AMC1LEN.mask;
        if (leftChannel == MICROPHONE_ON)
        {
            reg_write = SET_BITS(regAUDIO_TX, AMC1LEN, 1);
        }

        reg_mask |= regAUDIO_TX.AMC1REN.mask |
                    regAUDIO_TX.AMC2EN.mask  |
                    regAUDIO_TX.ATXINEN.mask;
        if (rightChannel == MICROPHONE_ON)
        {
            if (vCodec.rightChannelMic.mic == MIC1_RIGHT_MIC_MONO)
            {
                reg_write |= SET_BITS(regAUDIO_TX, AMC1REN, 1);
            }
            else if (vCodec.rightChannelMic.mic == MIC2_AUX)
            {
                reg_write |= SET_BITS(regAUDIO_TX, AMC2EN, 1);
            }
            else if (vCodec.rightChannelMic.mic == TXIN_EXT)
            {
                reg_write |= SET_BITS(regAUDIO_TX, ATXINEN, 1);
            }
        }

        rc = pmic_write_reg(REG_AUDIO_TX, reg_write, reg_mask);

        if (rc == PMIC_SUCCESS)
        {
            vCodec.leftChannelMic.micOnOff  = leftChannel;
            vCodec.rightChannelMic.micOnOff = rightChannel;
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)                             &&
        (vCodec.handleState == HANDLE_IN_USE)                 &&
        (leftChannel != (PMIC_AUDIO_INPUT_MIC_STATE *)NULL)   &&
        (rightChannel != (PMIC_AUDIO_INPUT_MIC_STATE *)NULL))
    {
        *leftChannel  = vCodec.leftChannelMic.micOnOff;
        *rightChannel = vCodec.rightChannelMic.micOnOff;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        /* Validate the amplifier modes based upon the following:
         *
         *   - Microphone inputs must have been previously selected.
         *   - For the right microphone input channel, only the Amc1R and
         *     Amc2 amplifiers can operate in the I-to-V mode.
         */
        if (((vCodec.leftChannelMic.mic == NO_MIC) &&
             (leftChannelMode != AMP_OFF))                            ||
            ((vCodec.rightChannelMic.mic == NO_MIC) &&
             (rightChannelMode != AMP_OFF))                           ||
            ((rightChannelMode == CURRENT_TO_VOLTAGE) &&
             !((vCodec.rightChannelMic.mic == MIC1_RIGHT_MIC_MONO) ||
               (vCodec.rightChannelMic.mic == MIC2_AUX))))
        {
            rc = PMIC_PARAMETER_ERROR;
        }
        else
        {
            if (leftChannelMode == CURRENT_TO_VOLTAGE)
            {
                reg_write = SET_BITS(regAUDIO_TX, AMC1LITOV, 1);
                reg_mask  = SET_BITS(regAUDIO_TX, AMC1LITOV, 1);
            }
            else if (leftChannelMode == VOLTAGE_TO_VOLTAGE)
            {
                reg_mask = SET_BITS(regAUDIO_TX, AMC1LITOV, 1);
            }

            if (rightChannelMode == CURRENT_TO_VOLTAGE)
            {
                if (vCodec.rightChannelMic.mic == MIC1_RIGHT_MIC_MONO)
                {
                    reg_write |= SET_BITS(regAUDIO_TX, AMC1RITOV, 1);
                    reg_mask  |= SET_BITS(regAUDIO_TX, AMC1RITOV, 1);
                }
                else if (vCodec.rightChannelMic.mic == MIC2_AUX)
                {
                    reg_write |= SET_BITS(regAUDIO_TX, AMC2ITOV, 1);
                    reg_mask  |= SET_BITS(regAUDIO_TX, AMC2ITOV, 1);
                }

                /* Note that the Atxin amplifier cannot operate in an I-to-V
                 * mode and we've already tested for that invalid option above.
                 */
            }
            else if (rightChannelMode == VOLTAGE_TO_VOLTAGE)
            {
                if (vCodec.rightChannelMic.mic == MIC1_RIGHT_MIC_MONO)
                {
                    reg_mask |= SET_BITS(regAUDIO_TX, AMC1RITOV, 1);
                }
                else if (vCodec.rightChannelMic.mic == MIC2_AUX)
                {
                    reg_mask |= SET_BITS(regAUDIO_TX, AMC2ITOV, 1);
                }

                /* Note that this is a no-op for the Atxin amplifier since
                 * it can only operate in a V-to-V mode.
                 */
            }

            /* We intentionally subtract MIC_GAIN_MINUS_8DB (even though it is
             * currently defined as zero in the PMIC_AUDIO_MIC_GAIN enumeration)
             * for the sake of safety and future code robustness.
             *
             * Doing the subtraction will allow the code to still work properly
             * even if we should add additional gain levels below -8dB to the
             * enumeration in the future to support other PMICs. Furthermore,
             * any decent optimizing compiler should remove a subtract by zero
             * operation so this should not impose any sort of a performance
             * penalty at all.
             */
            reg_write |= SET_BITS(regAUDIO_TX, PGATXL, leftChannelGain -
                                  MIC_GAIN_MINUS_8DB) |
                         SET_BITS(regAUDIO_TX, PGATXR, rightChannelGain -
                                  MIC_GAIN_MINUS_8DB);
            reg_mask  |= regAUDIO_TX.PGATXL.mask | regAUDIO_TX.PGATXR.mask;

            rc = pmic_write_reg(REG_AUDIO_TX, reg_write, reg_mask);

            if (rc == PMIC_SUCCESS)
            {
                vCodec.leftChannelMic.ampMode  = leftChannelMode;
                vCodec.leftChannelMic.gain     = leftChannelGain;
                vCodec.rightChannelMic.ampMode = rightChannelMode;
                vCodec.rightChannelMic.gain    = rightChannelGain;
            }
        }
    }

    /* Exit critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == vCodec.handle)                             &&
        (vCodec.handleState == HANDLE_IN_USE)                 &&
        (leftChannelMode != (PMIC_AUDIO_MIC_AMP_MODE *)NULL)  &&
        (leftChannelGain != (PMIC_AUDIO_MIC_GAIN *)NULL)      &&
        (rightChannelMode != (PMIC_AUDIO_MIC_AMP_MODE *)NULL) &&
        (rightChannelGain != (PMIC_AUDIO_MIC_GAIN *)NULL))
    {
        *leftChannelMode  = vCodec.leftChannelMic.ampMode;
        *leftChannelGain  = vCodec.leftChannelMic.gain;
        *rightChannelMode = vCodec.rightChannelMic.ampMode;
        *rightChannelGain = vCodec.rightChannelMic.gain;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        if (biasCircuit & MIC_BIAS1)
        {
            reg_write = SET_BITS(regAUDIO_TX, MC1BEN, 1);
            reg_mask  = SET_BITS(regAUDIO_TX, MC1BEN, 1);
        }

        if (biasCircuit & MIC_BIAS2)
        {
            reg_write |= SET_BITS(regAUDIO_TX, MC2BEN, 1);
            reg_mask  |= SET_BITS(regAUDIO_TX, MC2BEN, 1);
        }

        if (reg_mask != 0)
        {
            rc = pmic_write_reg(REG_AUDIO_TX, reg_write, reg_mask);
        }
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        if (biasCircuit & MIC_BIAS1)
        {
            reg_mask = SET_BITS(regAUDIO_TX, MC1BEN, 1);
        }

        if (biasCircuit & MIC_BIAS2)
        {
            reg_mask |= SET_BITS(regAUDIO_TX, MC2BEN, 1);
        }

        if (reg_mask != 0)
        {
            rc = pmic_write_reg(REG_AUDIO_TX, reg_write, reg_mask);
        }
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        if ((vCodec.timeslot != rxSecondaryTimeslot) &&
            (gainIn != VCODEC_NO_MIX))
        {
            reg_write = SET_BITS(regSSI_NETWORK, CDCRXSECSLOT,
                                 rxSecondaryTimeslot)                  |
                        SET_BITS(regSSI_NETWORK, CDCRXSECGAIN, gainIn) |
                        SET_BITS(regSSI_NETWORK, CDCSUMGAIN, gainOut);
            reg_mask  = regSSI_NETWORK.CDCRXSECSLOT.mask |
                        regSSI_NETWORK.CDCRXSECGAIN.mask |
                        regSSI_NETWORK.CDCSUMGAIN.mask;

            rc = pmic_write_reg(REG_SSI_NETWORK, reg_write, reg_mask);
        }
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_write = 0;
    const unsigned int reg_mask  = regSSI_NETWORK.CDCRXSECGAIN.mask |
                                   regSSI_NETWORK.CDCSUMGAIN.mask;

    if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        /* Just set CDCRXSECGAIN = 0 and CDCSUMGAIN = 0 in the SSI Network
         * register to disable mixing.
         */
        rc = pmic_write_reg(REG_SSI_NETWORK, reg_write, reg_mask);
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE) &&
        (gainIn <= STDAC_MIX_IN_MINUS_12DB) &&
        (gainIn != STDAC_NO_MIX)            &&
        (gainOut <= STDAC_MIX_OUT_MINUS_6DB))
    {
        /* Enable the Stereo DAC mixer by setting the input and
         * output gains.
         */
        reg_mask = SET_BITS(regSSI_NETWORK, STDCRXSECGAIN, 3) |
                   SET_BITS(regSSI_NETWORK, STDCSUMGAIN, 1);

        if (gainIn == STDAC_MIX_IN_0DB)
        {
            reg_write = SET_BITS(regSSI_NETWORK, STDCRXSECGAIN, 1);
        }
        else if (gainIn == STDAC_MIX_IN_MINUS_6DB)
        {
            reg_write = SET_BITS(regSSI_NETWORK, STDCRXSECGAIN, 2);
        }
        else if (gainIn == STDAC_MIX_IN_MINUS_12DB)
        {
            reg_write = SET_BITS(regSSI_NETWORK, STDCRXSECGAIN, 3);
        }

        /* Note that STDCSUMGAIN = 0 for a 0dB output gain. */
        if (gainOut == STDAC_MIX_OUT_MINUS_6DB)
        {
            reg_write |= SET_BITS(regSSI_NETWORK, STDCSUMGAIN, 1);
        }

        rc = pmic_write_reg(REG_SSI_NETWORK, reg_write, reg_mask);
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_write = 0;
    const unsigned int reg_mask  = regSSI_NETWORK.STDCRXSECGAIN.mask |
                                   regSSI_NETWORK.STDCSUMGAIN.mask;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE))
    {
        /* Just set STDCRXSECGAIN = 0 and STDCSUMGAIN = 0 in the SSI Network
         * register to disable mixing.
         */
        rc = pmic_write_reg(REG_SSI_NETWORK, reg_write, reg_mask);
    }

    return rc;
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
    const PMIC_AUDIO_OUTPUT_PORT port)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if (((handle == stDAC.handle)       &&
         (stDAC.handleState == HANDLE_IN_USE))  ||
        ((handle == vCodec.handle)      &&
         (vCodec.handleState == HANDLE_IN_USE)) ||
        ((handle == extStereoIn.handle) &&
         (extStereoIn.handleState == HANDLE_IN_USE)))
    {
        if ((port & MONO_ALERT) || (port & MONO_EXTOUT))
        {
            rc = PMIC_NOT_SUPPORTED;
        }
        else if (!(((port & STEREO_LEFT_LOW_POWER) &&
                    (handle == vCodec.handle))        ||
                   ((port & MONO_CDCOUT)           &&
                    (handle != vCodec.handle))        ||
                   ((port & MONO_SPEAKER)          &&
                    (port & MONO_LOUDSPEAKER))))
        {
            if (port & MONO_SPEAKER)
            {
                reg_write |= SET_BITS(regRX0, ASPEN, 1);
                if ((handle == stDAC.handle) || (handle == extStereoIn.handle))
                {
                    reg_write |= SET_BITS(regRX0, ASPSEL, 1);
                }
                reg_mask |= regRX0.ASPSEL.mask | regRX0.ASPEN.mask;
            }
            if (port & MONO_LOUDSPEAKER)
            {
                reg_write |= SET_BITS(regRX0, ALSPEN, 1);
                if ((handle == stDAC.handle) || (handle == extStereoIn.handle))
                {
                    reg_write |= SET_BITS(regRX0, ALSPSEL, 1);
                }
                reg_mask |= regRX0.ALSPSEL.mask | regRX0.ALSPEN.mask;
            }
            if (port & MONO_CDCOUT)
            {
                reg_write |= SET_BITS(regRX0, CDCOUTEN, 1);
                reg_mask  |= regRX0.CDCOUTEN.mask;
            }
            if (port & STEREO_LEFT_LOW_POWER)
            {
                reg_write |= SET_BITS(regRX0, LSPLEN, 1);
                reg_mask  |= regRX0.LSPLEN.mask;
            }
            if (port & STEREO_HEADSET_LEFT)
            {
                reg_write |= SET_BITS(regRX0, AHSLEN, 1);
                if ((handle == stDAC.handle) || (handle == extStereoIn.handle))
                {
                    reg_write |= SET_BITS(regRX0, AHSSEL, 1);
                }
                reg_mask |= regRX0.AHSSEL.mask | regRX0.AHSLEN.mask;
            }
            if (port & STEREO_HEADSET_RIGHT)
            {
                reg_write |= SET_BITS(regRX0, AHSREN, 1);
                if ((handle == stDAC.handle) || (handle == extStereoIn.handle))
                {
                    reg_write |= SET_BITS(regRX0, AHSSEL, 1);
                }
                reg_mask |= regRX0.AHSSEL.mask | regRX0.AHSREN.mask;
            }
            if (port & STEREO_OUT_LEFT)
            {
                reg_write |= SET_BITS(regRX0, ARXOUTLEN, 1);
                if ((handle == stDAC.handle) || (handle == extStereoIn.handle))
                {
                    reg_write |= SET_BITS(regRX0, ARXOUTSEL, 1);
                }
                reg_mask |= regRX0.ARXOUTSEL.mask | regRX0.ARXOUTLEN.mask;
            }
            if (port & STEREO_OUT_RIGHT)
            {
                reg_write |= SET_BITS(regRX0, ARXOUTREN, 1);
                if ((handle == stDAC.handle) || (handle == extStereoIn.handle))
                {
                    reg_write |= SET_BITS(regRX0, ARXOUTSEL, 1);
                }
                reg_mask |= regRX0.ARXOUTSEL.mask | regRX0.ARXOUTREN.mask;
            }

            rc = pmic_write_reg(REG_RX0, reg_write, reg_mask);

            if (rc == PMIC_SUCCESS)
            {
                /* Special handling for the phantom ground circuit is
                 * recommended if either ALEFT/ARIGHT amplifiers are enabled.
                 */
                if (port & (STEREO_HEADSET_LEFT | STEREO_HEADSET_RIGHT))
                {
                    /* We should also enable the phantom ground circuit when
                     * using either ALEFT/ARIGHT to improve the quality of the
                     * audio output.
                     */
                    PmicAudioOutputEnablePhantomGround(handle);
                }
                else
                {
                    /* Otherwise disable the phantom ground circuit when not
                     * using either ALEFT/ARIGHT to reduce power consumption.
                     */
                    PmicAudioOutputDisablePhantomGround(handle);
                }

                audioOutput.outputPort = port;
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    const PMIC_AUDIO_OUTPUT_PORT port)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_write = 0;
    unsigned int reg_mask        = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if (((handle == stDAC.handle)       &&
         (stDAC.handleState == HANDLE_IN_USE))  ||
        ((handle == vCodec.handle)      &&
         (vCodec.handleState == HANDLE_IN_USE)) ||
        ((handle == extStereoIn.handle) &&
         (extStereoIn.handleState == HANDLE_IN_USE)))
    {
        if ((port & MONO_ALERT) || (port & MONO_EXTOUT))
        {
            rc = PMIC_NOT_SUPPORTED;
        }
        else if (!(((port & STEREO_LEFT_LOW_POWER) &&
                    (handle == vCodec.handle))        ||
                   ((port & MONO_CDCOUT)           &&
                    (handle != vCodec.handle))        ||
                   ((port & MONO_SPEAKER)          &&
                    (port & MONO_LOUDSPEAKER))))
        {
            if (port & MONO_SPEAKER)
            {
                reg_mask |= regRX0.ASPEN.mask;
            }
            if (port & MONO_LOUDSPEAKER)
            {
                reg_mask |= regRX0.ALSPEN.mask;
            }
            if (port & MONO_CDCOUT)
            {
                reg_mask |= regRX0.CDCOUTEN.mask;
            }
            if (port & STEREO_LEFT_LOW_POWER)
            {
                reg_mask |= regRX0.LSPLEN.mask;
            }
            if ((port & STEREO_HEADSET_LEFT) && (handle == stDAC.handle && extStereoIn.handleState != HANDLE_IN_USE))   //KIMG for FM
            {
                //KIMG for FM
                reg_mask |= regRX0.AHSLEN.mask;
            }
            if ((port & STEREO_HEADSET_RIGHT) && (handle == stDAC.handle && extStereoIn.handleState != HANDLE_IN_USE))  //KIMG for FM
            {
                reg_mask |= regRX0.AHSREN.mask;
            }
            if (port & STEREO_OUT_LEFT)
            {
                reg_mask |= regRX0.ARXOUTLEN.mask;
            }
            if (port & STEREO_OUT_RIGHT)
            {
                reg_mask |= regRX0.ARXOUTREN.mask;
            }

            rc = pmic_write_reg(REG_RX0, reg_write, reg_mask);

            if (rc == PMIC_SUCCESS)
            {
                if ((port & STEREO_HEADSET_LEFT) &&
                    (port & STEREO_HEADSET_RIGHT))
                {
                    /* Disable the phantom ground since we've now disabled
                     * both of the stereo headset outputs.
                     */
                    PmicAudioOutputDisablePhantomGround(handle);
                }

                audioOutput.outputPort =
                    PMIC_AUDIO_OUTPUT_PORT(audioOutput.outputPort & ~port);
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
}

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
    PMIC_AUDIO_OUTPUT_PORT *const port)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((((handle == stDAC.handle)       &&
          (stDAC.handleState == HANDLE_IN_USE))  ||
         ((handle == vCodec.handle)      &&
          (vCodec.handleState == HANDLE_IN_USE)) ||
         ((handle == extStereoIn.handle) &&
          (extStereoIn.handleState == HANDLE_IN_USE))) &&
        (port != (PMIC_AUDIO_OUTPUT_PORT *)NULL))
    {
        *port = audioOutput.outputPort;

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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == extStereoIn.handle) &&
        (extStereoIn.handleState == HANDLE_IN_USE))
    {
        if (gain == STEREO_IN_GAIN_0DB)
        {
            reg_mask = regRX1.ARXIN.mask;
        }
        else if (gain == STEREO_IN_GAIN_PLUS_18DB)
        {
            reg_write = SET_BITS(regRX1, ARXIN, 1);
            reg_mask  = regRX1.ARXIN.mask;
        }

        if (reg_mask != 0)
        {
            rc = pmic_write_reg(REG_RX1, reg_write, reg_mask);

            if (rc == PMIC_SUCCESS)
            {
                extStereoIn.inputGain = gain;
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((handle == extStereoIn.handle) &&
        (extStereoIn.handleState == HANDLE_IN_USE) &&
        (gain != (PMIC_AUDIO_STEREO_IN_GAIN *)NULL))
    {
        *gain = extStereoIn.inputGain;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if (((handle == stDAC.handle)       &&
         (stDAC.handleState == HANDLE_IN_USE))  ||
        ((handle == vCodec.handle)      &&
         (vCodec.handleState == HANDLE_IN_USE)) ||
        ((handle == extStereoIn.handle) &&
         (extStereoIn.handleState == HANDLE_IN_USE)))
    {
        if ((gain < OUTPGA_GAIN_MINUS_33DB) ||
            (gain > OUTPGA_GAIN_PLUS_6DB))
        {
            rc = PMIC_NOT_SUPPORTED;
        }
        else
        {
            if (handle == stDAC.handle || handle == extStereoIn.handle)
            {
                /* Note that we add 2 to the RX1 register value for the
                 * PGAST (Stereo DAC PGA gain) field because settings of
                 * 0, 1, and 2 all represent a gain of -33 dB. Therefore,
                 * by just adding 2, we can get an easy conversion from
                 * the PMIC_AUDIO_OUTPUT_PGA_GAIN enumeration values to
                 * the corresponding register settings.
                 */
                reg_write = SET_BITS(regRX1, PGAST,
                                     gain - OUTPGA_GAIN_MINUS_33DB + 2);
                reg_mask  = regRX1.PGAST.mask;

                // Workaround for ENGR42530. Adjusting system volume could not affect FM radio volume.
                reg_write |= SET_BITS(regRX1, PGARXIN,
                                     gain - OUTPGA_GAIN_MINUS_33DB + 2);
                reg_mask  |= regRX1.PGARXIN.mask;
            }
            else if (handle == vCodec.handle)
            {
                reg_write = SET_BITS(regRX1, PGARX,
                                     gain - OUTPGA_GAIN_MINUS_33DB);
                reg_mask  = regRX1.PGARX.mask;
            }

            rc = pmic_write_reg(REG_RX1, reg_write, reg_mask);

            if (rc == PMIC_SUCCESS)
            {
                if (handle == stDAC.handle)
                {
                    audioOutput.outputStDACPGAGain = gain;
                }
                else if (handle == vCodec.handle)
                {
                    audioOutput.outputVCodecPGAGain = gain;
                }
                else if (handle == extStereoIn.handle)
                {
                    audioOutput.outputExtStereoInPGAGain = gain;
                }
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
}

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
    PMIC_AUDIO_OUTPUT_PGA_GAIN *const gain)
{
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if (gain == (PMIC_AUDIO_OUTPUT_PGA_GAIN *)NULL)
    {
        rc = PMIC_PARAMETER_ERROR;
    }
    else if (((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE)))
    {
        *gain = audioOutput.outputStDACPGAGain;
        rc = PMIC_SUCCESS;
    }
    else if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        *gain = audioOutput.outputVCodecPGAGain;
        rc = PMIC_SUCCESS;
    }
    else if ((handle == extStereoIn.handle) &&
             (extStereoIn.handleState == HANDLE_IN_USE))
    {
        *gain = audioOutput.outputExtStereoInPGAGain;
        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write0 = 0;
    unsigned int reg_mask0  = 0;
    unsigned int reg_write1 = 0;
    unsigned int reg_mask1  = 0;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE))
    {
        reg_write0 = SET_BITS(regRX0, ADDSTDC, 1);
        reg_mask0  = regRX0.ADDSTDC.mask;
        reg_write1 = SET_BITS(regRX1, PGASTEN, 1);
        reg_mask1  = regRX1.PGASTEN.mask;
    }
    else if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        reg_write0 = SET_BITS(regRX0, ADDCDC, 1);
        reg_mask0  = regRX0.ADDCDC.mask;
        reg_write1 = SET_BITS(regRX1, PGARXEN, 1);
        reg_mask1  = regRX1.PGARXEN.mask;
    }
    else if ((handle == extStereoIn.handle) &&
             (extStereoIn.handleState == HANDLE_IN_USE))
    {
        reg_write0 = SET_BITS(regRX0, ADDRXIN, 1);
        reg_mask0  = regRX0.ADDRXIN.mask;
        reg_write1 = SET_BITS(regRX1, ARXINEN, 1);
        reg_mask1  = regRX1.ARXINEN.mask;
    }

    if ((reg_mask0 != 0)                                                 &&
        (pmic_write_reg(REG_RX0, reg_write0, reg_mask0) == PMIC_SUCCESS) &&
        (pmic_write_reg(REG_RX1, reg_write1, reg_mask1) == PMIC_SUCCESS))
    {
        rc = PMIC_SUCCESS;
    }
    else
    {
        rc = PMIC_ERROR;
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_write = 0;
    unsigned int reg_mask        = 0;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE))
    {
        reg_mask = regRX0.ADDSTDC.mask;
    }
    else if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        reg_mask = regRX0.ADDCDC.mask;
    }
    else if ((handle == extStereoIn.handle) &&
             (extStereoIn.handleState == HANDLE_IN_USE))
    {
        reg_mask = regRX0.ADDRXIN.mask;
    }

    if (reg_mask != 0)
    {
        rc = pmic_write_reg(REG_RX0, reg_write, reg_mask);
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if (((handle == stDAC.handle)       &&
         (stDAC.handleState == HANDLE_IN_USE))  ||
        ((handle == vCodec.handle)      &&
         (vCodec.handleState == HANDLE_IN_USE)) ||
        ((handle == extStereoIn.handle) &&
         (extStereoIn.handleState == HANDLE_IN_USE)))
    {
        /* For the MC13783 PMIC, we can only adjust the balance for either the
         * left or right channel.
         *
         * Therefore, we require that either the left or right channel be
         * defined with a 0dB gain level and then we can configure the other
         * channel's balance relative to it.
         */
        if (leftGain == BAL_GAIN_0DB)
        {
            /* The left channel is our 0dB reference, so let's adjust
             * the right channel balance.
             *
             * We subtract the rightGain value from the BAL_GAIN_MINUS_21DB
             * enumeration value in order to quickly get the correct register
             * value. We could have simply reversed the order of the definitions
             * in the PMIC_AUDIO_OUTPUT_BALANCE_GAIN enumeration in pmic_audio.h
             * but it is better to have a lower-to-higher order of gain
             * definitions to maintain consistency with the other enumerations.
             */
            reg_write = SET_BITS(regRX1, BAL, BAL_GAIN_MINUS_21DB - rightGain);
            reg_mask  = regRX1.BAL.mask | regRX1.BALLR.mask;
        }
        else if (rightGain == BAL_GAIN_0DB)
        {
            /* The right channel is our 0dB reference, so let's adjust
             * the left channel balance.
             *
             * See also the previous comment about why we subtract from the
             * BAL_GAIN_MINUS_21DB enumeration value.
             */
            reg_write = SET_BITS(regRX1, BAL, BAL_GAIN_MINUS_21DB - leftGain) |
                        SET_BITS(regRX1, BALLR, 1);
            reg_mask  = regRX1.BAL.mask | regRX1.BALLR.mask;
        }

        if (reg_mask != 0)
        {
            rc = pmic_write_reg(REG_RX1, reg_write, reg_mask);

            if (rc == PMIC_SUCCESS)
            {
                audioOutput.balanceLeftGain  = leftGain;
                audioOutput.balanceRightGain = rightGain;
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((((handle == stDAC.handle)       &&
          (stDAC.handleState == HANDLE_IN_USE))  ||
         ((handle == vCodec.handle)      &&
          (vCodec.handleState == HANDLE_IN_USE)) ||
         ((handle == extStereoIn.handle) &&
          (extStereoIn.handleState == HANDLE_IN_USE)))        &&
        ((leftGain != (PMIC_AUDIO_OUTPUT_BALANCE_GAIN *)NULL) &&
        (rightGain != (PMIC_AUDIO_OUTPUT_BALANCE_GAIN *)NULL)))
    {
        *leftGain  = audioOutput.balanceLeftGain;
        *rightGain = audioOutput.balanceRightGain;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_mask = SET_BITS(regRX1, MONO, 3);
    unsigned int reg_write      = 0;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if (((handle == stDAC.handle)       &&
         (stDAC.handleState == HANDLE_IN_USE))  ||
        ((handle == vCodec.handle)      &&
         (vCodec.handleState == HANDLE_IN_USE)) ||
        ((handle == extStereoIn.handle) &&
         (extStereoIn.handleState == HANDLE_IN_USE)))
    {
        if (mode == MONO_ADDER_OFF)
        {
            rc = PMIC_ERROR;
        }
        else if (mode == MONO_ADD_LEFT_RIGHT)
        {
            reg_write = SET_BITS(regRX1, MONO, 2);
            rc = pmic_write_reg(REG_RX1, reg_write, reg_mask);
        }
        else if (mode == MONO_ADD_OPPOSITE_PHASE)
        {
            reg_write = SET_BITS(regRX1, MONO, 3);
            rc = pmic_write_reg(REG_RX1, reg_write, reg_mask);
        }
        else if (mode == STEREO_OPPOSITE_PHASE)
        {
            reg_write = SET_BITS(regRX1, MONO, 1);
            rc = pmic_write_reg(REG_RX1, reg_write, reg_mask);
        }
        else
        {
            rc = PMIC_NOT_SUPPORTED;
        }
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_write = 0;
    const unsigned int reg_mask  = regRX1.MONO.mask;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if (((handle == stDAC.handle)       &&
         (stDAC.handleState == HANDLE_IN_USE))  ||
        ((handle == vCodec.handle)      &&
         (vCodec.handleState == HANDLE_IN_USE)) ||
        ((handle == extStereoIn.handle) &&
         (extStereoIn.handleState == HANDLE_IN_USE)))
    {
        rc = pmic_write_reg(REG_RX1, reg_write, reg_mask);
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if (((handle == stDAC.handle)       &&
         (stDAC.handleState == HANDLE_IN_USE))  ||
        ((handle == vCodec.handle)      &&
         (vCodec.handleState == HANDLE_IN_USE)) ||
        ((handle == extStereoIn.handle) &&
         (extStereoIn.handleState == HANDLE_IN_USE)))
    {
        if (gain == MONOADD_GAIN_0DB)
        {
            /* This is just a no-op for the MC13783 PMIC since there is really
             * no mono adder gain control.
             */
            rc = PMIC_SUCCESS;
        }
        else
        {
            /* The MC13783 PMIC does not support any gain settings for the
             * mono adder. Therefore, anything other than a 0dB gain setting
             * should return PMIC_NOT_SUPPORTED.
             */
            rc = PMIC_NOT_SUPPORTED;
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((((handle == stDAC.handle)       &&
          (stDAC.handleState == HANDLE_IN_USE))  ||
         ((handle == vCodec.handle)      &&
          (vCodec.handleState == HANDLE_IN_USE)) ||
         ((handle == extStereoIn.handle) &&
          (extStereoIn.handleState == HANDLE_IN_USE))) &&
        (gain != (PMIC_AUDIO_MONO_ADDER_OUTPUT_GAIN *)NULL))
    {
        *gain = audioOutput.monoAdderGain;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if (((handle == stDAC.handle)       &&
         (stDAC.handleState == HANDLE_IN_USE))  ||
        ((handle == vCodec.handle)      &&
         (vCodec.handleState == HANDLE_IN_USE)) ||
        ((handle == extStereoIn.handle) &&
         (extStereoIn.handleState == HANDLE_IN_USE)))
    {
        if (config & MONO_SPEAKER_INVERT_OUT_ONLY)
        {
            rc = PMIC_NOT_SUPPORTED;
        }
        else
        {
            if (config & MONO_LOUDSPEAKER_COMMON_BIAS)
            {
                reg_write = SET_BITS(regRX0, ALSPREF, 1);
                reg_mask  = regRX0.ALSPREF.mask;
            }

            if (config & HEADSET_DETECT_ENABLE)
            {
                reg_write |= SET_BITS(regRX0, HSDETEN, 1);
                reg_mask  |= regRX0.HSDETEN.mask;
            }

            if (config & STEREO_HEADSET_AMP_AUTO_DISABLE)
            {
                /* For the MC13783 PMIC, HSDETAUTOB = 0 enables the automatic
                 * amplifier disable function based upon the headset detect
                 * circuit.
                 */
                reg_mask |= regRX0.HSDETAUTOB.mask;
            }

            if (reg_mask != 0)
            {
                rc = pmic_write_reg(REG_RX0, reg_write, reg_mask);

                if (rc == PMIC_SUCCESS)
                {
                    audioOutput.config =
                        PMIC_AUDIO_OUTPUT_CONFIG(audioOutput.config | config);
                }
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if (((handle == stDAC.handle)       &&
         (stDAC.handleState == HANDLE_IN_USE))  ||
        ((handle == vCodec.handle)      &&
         (vCodec.handleState == HANDLE_IN_USE)) ||
        ((handle == extStereoIn.handle) &&
         (extStereoIn.handleState == HANDLE_IN_USE)))
    {
        if (config & MONO_SPEAKER_INVERT_OUT_ONLY)
        {
            rc = PMIC_NOT_SUPPORTED;
        }
        else
        {
            if (config & MONO_LOUDSPEAKER_COMMON_BIAS)
            {
                reg_write = SET_BITS(regRX0, ALSPREF, 1);
                reg_mask  = regRX0.ALSPREF.mask;
            }

            if (config & HEADSET_DETECT_ENABLE)
            {
                reg_write |= SET_BITS(regRX0, HSDETEN, 1);
                reg_mask  |= regRX0.HSDETEN.mask;
            }

            if (config & STEREO_HEADSET_AMP_AUTO_DISABLE)
            {
                /* For the MC13783 PMIC, HSDETAUTOB = 1 disables the automatic
                 * amplifier disable function based upon the headset detect
                 * circuit.
                 */
                reg_write |= SET_BITS(regRX0, HSDETAUTOB, 1);
                reg_mask  |= regRX0.HSDETAUTOB.mask;
            }

            if (reg_mask != 0)
            {
                rc = pmic_write_reg(REG_RX0, reg_write, reg_mask);

                if (rc == PMIC_SUCCESS)
                {
                    audioOutput.config =
                        PMIC_AUDIO_OUTPUT_CONFIG(audioOutput.config & ~config);
                }
            }
        }
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Use a critical section to ensure a consistent hardware state. */
    down_interruptible(mutex);

    if ((((handle == stDAC.handle)       &&
          (stDAC.handleState == HANDLE_IN_USE))  ||
         ((handle == vCodec.handle)      &&
          (vCodec.handleState == HANDLE_IN_USE)) ||
         ((handle == extStereoIn.handle) &&
          (extStereoIn.handleState == HANDLE_IN_USE))) &&
        (config != (PMIC_AUDIO_OUTPUT_CONFIG *)NULL))
    {
        *config = audioOutput.config;

        rc = PMIC_SUCCESS;
    }

    /* Exit the critical section. */
    up(mutex);

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_mask = regRX0.HSPGDIS.mask;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if (((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE)) ||
        ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE)))
    {
        rc = pmic_write_reg(REG_RX0, 0, reg_mask);
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    const unsigned int reg_write = regRX0.HSPGDIS.mask;

    /* No critical section required here since we are not updating any
     * global data.
     */

    if (((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE)) ||
        ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE)))
    {
        rc = pmic_write_reg(REG_RX0, reg_write, reg_write);
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_SUCCESS;

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
            rc = PMIC_ERROR;
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
                rc = PMIC_ERROR;
            }
            else
            {
                /*! Now suspend the thread until the timer event is
                 *  signalled.
                 */
                if (WaitForSingleObject(hTimedDelay, INFINITE) != WAIT_OBJECT_0)
                {
                    /*! Timed delay was prematurely aborted for some reason. */
                    rc = PMIC_ERROR;
                }
            }

            /*! Clean up by releasing the event handle. */
            CloseHandle(hTimedDelay);
            hTimedDelay = NULL;
        }
    }

    return rc;
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
 * @brief Read the current value of a specific PMIC control register.
 *
 * This function returns the current value of a specific PMIC control register.
 *
 * @param[in]   regID           The PMIC control register to be modified.
 * @param[in]   regValue        A pointer to where a copy of the current
 *                              register value will be saved.
 *
 * @retval      PMIC_SUCCESS    The register has been successfully read.
 * @retval      PMIC_ERROR      An error occured.
 */
static PMIC_STATUS pmic_read_reg(const pmic_control_register regID,
                                 const unsigned *regValue)
{
    return PmicRegisterRead((unsigned char)regID, (UINT32 *)regValue);
}

/*!
 * @brief Write a value to a specific PMIC control register.
 *
 * This function writes a new value to a specific PMIC control register. An
 * associated mask value is used to determine exactly which bits within the
 * control register are modified.
 *
 * @param[in]   regID           The PMIC control register to be modified.
 * @param[in]   regValue        The new register value to written.
 * @param[in]   regMask         The register value mask to be used.
 *
 * @retval      PMIC_SUCCESS    The register has been successfully modified.
 * @retval      PMIC_ERROR      An error occured.
 */
static PMIC_STATUS pmic_write_reg(const pmic_control_register regID,
                                  const unsigned regValue,
                                  const unsigned regMask)
{
    return PmicRegisterWrite((unsigned char)regID, (UINT32) regValue,
                             (UINT32)regMask);
}

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
static PMIC_STATUS pmic_adc_convert(const unsigned short channel,
                                    const unsigned short *adcResult)
{
    return PmicADCGetSingleChannelOneSample((UINT16) channel,
                                            (UINT16 *)adcResult);
}

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
static PMIC_STATUS pmic_audio_mic_boost_enable(void)
{
    PMIC_STATUS rc = PMIC_NOT_SUPPORTED;

    /* This feature is not supported by the MC13783 PMIC. */

    return rc;
}

/*!
 * @brief Disables the 5.6V boost for the microphone bias 2 circuit.
 *
 * This function disables the switching regulator SW3 to turn off the 5.6V
 * boost for the microphone bias 2 circuit.
 *
 * @retval      PMIC_SUCCESS         The 5.6V boost was successfully disabled.
 * @retval      PMIC_ERROR           Failed to disable the 5.6V boost.
 */
static PMIC_STATUS pmic_audio_mic_boost_disable(void)
{
    PMIC_STATUS rc = PMIC_NOT_SUPPORTED;

    /* This feature is not supported by the MC13783 PMIC. */

    return rc;
}

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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;

    /* Match up the handle to the audio device and then close it. */
    if ((handle == stDAC.handle) &&
        (stDAC.handleState == HANDLE_IN_USE))
    {
        /* Also shutdown the Stereo DAC hardware. The simplest way to
         * do this is to simply call pmic_audio_reset_device() which will
         * restore the ST_DAC register to it's initial power-on state.
         *
         * This will also shutdown the audio output section if no one
         * else is still using it.
         */
        rc = pmic_audio_reset_device(stDAC.handle);

        if (rc == PMIC_SUCCESS)
        {
            stDAC.handle      = AUDIO_HANDLE_NULL;
            stDAC.handleState = HANDLE_FREE;
        }
    }
    else if ((handle == vCodec.handle) &&
             (vCodec.handleState == HANDLE_IN_USE))
    {
        /* Also shutdown the Voice CODEC and audio input hardware. The
         * simplest way to do this is to simply call pmic_audio_reset_device()
         * which will restore the AUD_CODEC register to it's initial
         * power-on state.
         *
         * This will also shutdown the audio output section if no one
         * else is still using it.
         */
        rc = pmic_audio_reset_device(vCodec.handle);

        if (rc == PMIC_SUCCESS)
        {
            vCodec.handle      = AUDIO_HANDLE_NULL;
            vCodec.handleState = HANDLE_FREE;
        }
    }
    else if ((handle == extStereoIn.handle) &&
             (extStereoIn.handleState == HANDLE_IN_USE))
    {
        /* Call pmic_audio_reset_device() here to shutdown the audio output
         * section if no one else is still using it.
         */
        rc = pmic_audio_reset_device(extStereoIn.handle);

        if (rc == PMIC_SUCCESS)
        {
            extStereoIn.handle      = AUDIO_HANDLE_NULL;
            extStereoIn.handleState = HANDLE_FREE;
        }
    }

    return rc;
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
    PMIC_STATUS rc = PMIC_PARAMETER_ERROR;
    unsigned int reg_write = 0;
    unsigned int reg_mask  = 0;

    if ((handle == stDAC.handle) && (stDAC.handleState == HANDLE_IN_USE))
    {
        if ((vCodec.handleState == HANDLE_FREE) &&
            (extStereoIn.handleState == HANDLE_FREE))
        {
            /* Also shutdown the audio output section since nobody else
             * is using it.
             */
            pmic_write_reg(REG_RX0, RESET_RX0, REG_FULLMASK);
            pmic_write_reg(REG_RX1, RESET_RX1, REG_FULLMASK);
            pmic_write_reg(REG_SSI_NETWORK, RESET_SSI_NETWORK, REG_FULLMASK);
        }
        else
        {
            /* We have to be more selective and only shut down the Stereo DAC
             * specific items and leave the other settings intact because
             * either the Voice CODEC and/or the External Stereo Input is still
             * being used.
             */
            reg_write = 0;
            reg_mask  = regRX0.ADDSTDC.mask;
            pmic_write_reg(REG_RX0, reg_write, reg_mask);

            reg_write = SET_BITS(regRX1, PGAST, 0xd);
            reg_mask  = regRX1.PGASTEN.mask | regRX1.PGAST.mask;
            pmic_write_reg(REG_RX1, reg_write, reg_mask);

            reg_write = SET_BITS(regSSI_NETWORK, STDCSLOTS, 3)    |
                        SET_BITS(regSSI_NETWORK, STDCRXSECSLOT, 1);
            reg_mask  = regSSI_NETWORK.STDCSLOTS.mask     |
                        regSSI_NETWORK.STDCRXSLOT.mask    |
                        regSSI_NETWORK.STDCRXSECSLOT.mask |
                        regSSI_NETWORK.STDCRXSECGAIN.mask |
                        regSSI_NETWORK.STDCSUMGAIN.mask;
            pmic_write_reg(REG_SSI_NETWORK, reg_write, reg_mask);
        }

        rc = pmic_write_reg(REG_STEREO_DAC, RESET_STEREO_DAC, REG_FULLMASK);

        if (rc == PMIC_SUCCESS)
        {
            /* Also reset the driver state information to match. Note that we
             * keep the device handle and event callback settings unchanged
             * since these don't affect the actual hardware and we rely on
             * the user to explicitly close the handle or deregister callbacks
             */
            stDAC.protocol     = NORMAL_MSB_JUSTIFIED_MODE;
            stDAC.masterSlave  = BUS_SLAVE_MODE;
            stDAC.numSlots     = USE_2_TIMESLOTS;
            stDAC.clockIn      = CLOCK_IN_CLKIN;
            stDAC.samplingRate = STDAC_RATE_44_1_KHZ;
            stDAC.clockFreq    = STDAC_CLI_26MHZ; //STDAC_CLI_13MHZ
            stDAC.invert       = NO_INVERT;
            stDAC.timeslot     = USE_TS0_TS1;
            stDAC.config       = (PMIC_AUDIO_STDAC_CONFIG)0;
        }
    }
    else if ((handle == vCodec.handle) && (vCodec.handleState == HANDLE_IN_USE))
    {
        /* Disable the audio input section when disabling the Voice CODEC. */
        pmic_write_reg(REG_AUDIO_TX, RESET_AUDIO_TX, REG_FULLMASK);

        if ((stDAC.handleState == HANDLE_FREE) &&
            (extStereoIn.handleState == HANDLE_FREE))
        {
            /* Also shutdown the audio output section since nobody else
             * is using it.
             */
            pmic_write_reg(REG_RX0, RESET_RX0, REG_FULLMASK);
            pmic_write_reg(REG_RX1, RESET_RX1, REG_FULLMASK);
            pmic_write_reg(REG_SSI_NETWORK, RESET_SSI_NETWORK, REG_FULLMASK);
        }
        else
        {
            /* We have to be more selective and only shut down the Voice CODEC
             * specific items and leave the other settings intact because
             * either the Stereo DAC and/or the External Stereo Input is still
             * being used.
             */
            reg_write = 0;
            reg_mask  = regRX0.CDCOUTEN.mask | regRX0.ADDCDC.mask;
            pmic_write_reg(REG_RX0, reg_write, reg_mask);

            reg_write = SET_BITS(regRX1, PGARX, 0xd);
            reg_mask  = regRX1.PGARXEN.mask | regRX1.PGARX.mask;
            pmic_write_reg(REG_RX1, reg_write, reg_mask);

            reg_write = SET_BITS(regSSI_NETWORK, CDCTXSECSLOT, 2) |
                        SET_BITS(regSSI_NETWORK, CDCRXSECSLOT, 1);
            reg_mask  = regSSI_NETWORK.CDCTXRXSLOT.mask  |
                        regSSI_NETWORK.CDCTXSECSLOT.mask |
                        regSSI_NETWORK.CDCRXSECSLOT.mask |
                        regSSI_NETWORK.CDCRXSECGAIN.mask |
                        regSSI_NETWORK.CDCSUMGAIN.mask   |
                        regSSI_NETWORK.CDCFSDLY.mask;
            pmic_write_reg(REG_SSI_NETWORK, reg_write, reg_mask);
        }

        rc = pmic_write_reg(REG_AUDIO_CODEC, RESET_AUDIO_CODEC, REG_FULLMASK);

        if (rc == PMIC_SUCCESS)
        {
            /* Also reset the driver state information to match. Note that we
             * keep the device handle and event callback settings unchanged
             * since these don't affect the actual hardware and we rely on
             * the user to explicitly close the handle or deregister callbacks
             */
            vCodec.protocol     = NETWORK_MODE;
            vCodec.masterSlave  = BUS_SLAVE_MODE;
            vCodec.numSlots     = USE_4_TIMESLOTS;
            vCodec.clockIn      = CLOCK_IN_CLKIN;
            vCodec.samplingRate = VCODEC_RATE_8_KHZ;
            vCodec.clockFreq    = VCODEC_CLI_13MHZ;
            vCodec.invert       = NO_INVERT;
            vCodec.timeslot     = USE_TS0;
            vCodec.config       =
                PMIC_AUDIO_VCODEC_CONFIG(INPUT_HIGHPASS_FILTER |
                                         OUTPUT_HIGHPASS_FILTER);
        }
    }
    else if ((handle == extStereoIn.handle) &&
             (extStereoIn.handleState == HANDLE_IN_USE))
    {
        if ((stDAC.handleState == HANDLE_FREE) &&
            (extStereoIn.handleState == HANDLE_FREE))
        {
            /* Also shutdown the audio output section since nobody else
             * is using it.
             */
            pmic_write_reg(REG_RX0, RESET_RX0, REG_FULLMASK);
            pmic_write_reg(REG_RX1, RESET_RX1, REG_FULLMASK);
            pmic_write_reg(REG_SSI_NETWORK, RESET_SSI_NETWORK, REG_FULLMASK);
        }
        else
        {
            /* We have to be more selective and only shut down the Stereo DAC
             * specific items and leave the other settings intact because
             * either the Voice CODEC and/or the External Stereo Input is still
             * being used.
             */
            reg_mask = regRX1.ARXINEN.mask;
            pmic_write_reg(REG_RX1, reg_write, reg_mask);
        }

        /* All of the required configuration changes for resetting the External
         * Stereo Input state have already been done as part of the register
         * RX1 changes above. There are no other registers that affect the
         * External Stereo Input.
         */
        rc = PMIC_SUCCESS;
    }

    return rc;
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

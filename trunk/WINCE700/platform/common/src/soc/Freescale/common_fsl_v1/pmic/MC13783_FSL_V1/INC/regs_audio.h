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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2005, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs_audio.h
//
//  This header file defines audio registers of MC13783.
//
//------------------------------------------------------------------------------

#ifndef __MC13783_REGS_AUDIO_H__
#define __MC13783_REGS_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define MC13783_AUD_RX0_VAUDIOON_LSH            0
#define MC13783_AUD_RX0_BIASEN_LSH            1
#define MC13783_AUD_RX0_BIASSPEED_LSH            2
#define MC13783_AUD_RX0_ASPEN_LSH            3
#define MC13783_AUD_RX0_ASPSEL_LSH            4
#define MC13783_AUD_RX0_ALSPEN_LSH            5
#define MC13783_AUD_RX0_ALSPREF_LSH            6
#define MC13783_AUD_RX0_ALSPSEL_LSH            7
#define MC13783_AUD_RX0_LSPLEN_LSH            8
#define MC13783_AUD_RX0_AHSREN_LSH            9
#define MC13783_AUD_RX0_AHSLEN_LSH            10
#define MC13783_AUD_RX0_AHSSEL_LSH            11
#define MC13783_AUD_RX0_HSPGDIS_LSH            12
#define MC13783_AUD_RX0_HSDETEN_LSH            13
#define MC13783_AUD_RX0_HSDETAUTO_LSH            14
#define MC13783_AUD_RX0_ARXOUTREN_LSH            15
#define MC13783_AUD_RX0_ARXOUTLEN_LSH            16
#define MC13783_AUD_RX0_ARXOUTSEL_LSH            17
#define MC13783_AUD_RX0_CDCOUTEN_LSH            18
#define MC13783_AUD_RX0_ADDCDC_LSH            21
#define MC13783_AUD_RX0_ADDSTDC_LSH            22
#define MC13783_AUD_RX0_ADDRXIN_LSH            23

#define MC13783_AUD_RX1_PGARXEN_LSH            0
#define MC13783_AUD_RX1_PGARX_LSH            1
#define MC13783_AUD_RX1_PGASTEN_LSH            5
#define MC13783_AUD_RX1_PGAST_LSH            6
#define MC13783_AUD_RX1_ARXINEN_LSH            10
#define MC13783_AUD_RX1_ARXIN_LSH            11
#define MC13783_AUD_RX1_PGARXIN_LSH            12
#define MC13783_AUD_RX1_MONO_LSH            16
#define MC13783_AUD_RX1_BAL_LSH            18
#define MC13783_AUD_RX1_BALLR_LSH            21

#define MC13783_AUD_TX_MC1BEN_LSH            0
#define MC13783_AUD_TX_MC2BEN_LSH            1
#define MC13783_AUD_TX_MC2BDETEN_LSH            3
#define MC13783_AUD_TX_AMC1REN_LSH            5
#define MC13783_AUD_TX_AMC1RITOV_LSH            6
#define MC13783_AUD_TX_AMC1LEN_LSH            7
#define MC13783_AUD_TX_AMC1LITOV_LSH            8
#define MC13783_AUD_TX_AMC2EN_LSH            9
#define MC13783_AUD_TX_AMC2ITOV_LSH            10
#define MC13783_AUD_TX_ATXINEN_LSH            11
#define MC13783_AUD_TX_ATXOUTEN_LSH            12
#define MC13783_AUD_TX_PGATXR_LSH            14
#define MC13783_AUD_TX_PGATXL_LSH            19

#define MC13783_SSI_NW_CDCTXRXSLOT_LSH            2
#define MC13783_SSI_NW_CDCTXSECSLOT_LSH            4
#define MC13783_SSI_NW_CDCRXSECSLOT_LSH            6
#define MC13783_SSI_NW_CDCRXSECGAIN_LSH            8
#define MC13783_SSI_NW_CDCSUMGAIN_LSH            10
#define MC13783_SSI_NW_STDCSLOT_LSH            12
#define MC13783_SSI_NW_STDCRXSLOT_LSH            14
#define MC13783_SSI_NW_STDCRXSECSLOT_LSH            16
#define MC13783_SSI_NW_STDCRXSECGAIN_LSH            18
#define MC13783_SSI_NW_STDCSUMGAIN_LSH            20

#define MC13783_AUD_CDC_CDCSSISEL_LSH            0
#define MC13783_AUD_CDC_CDCCLKSEL_LSH            1
#define MC13783_AUD_CDC_CDCSM_LSH            2
#define MC13783_AUD_CDC_CDCBCLINV_LSH            3
#define MC13783_AUD_CDC_CDCFSINV_LSH            4
#define MC13783_AUD_CDC_CDCFSOFFSET_LSH            5
#define MC13783_AUD_CDC_CDCFSLONG_LSH            6
#define MC13783_AUD_CDC_CDCCLK_LSH            7
#define MC13783_AUD_CDC_CDCFS8K16K_LSH            10
#define MC13783_AUD_CDC_CDCEN_LSH            11
#define MC13783_AUD_CDC_CDCCLKEN_LSH            12
#define MC13783_AUD_CDC_CDCTS_LSH            13
#define MC13783_AUD_CDC_CDCDITH_LSH            14
#define MC13783_AUD_CDC_CDCRESET_LSH            15
#define MC13783_AUD_CDC_CDCBYP_LSH            16
#define MC13783_AUD_CDC_CDCALM_LSH            17
#define MC13783_AUD_CDC_CDCDLM_LSH            18
#define MC13783_AUD_CDC_AUDIHPF_LSH            19
#define MC13783_AUD_CDC_AUDOHPF_LSH            20

#define MC13783_AUD_STR_DAC_STDCSSISEL_LSH            0
#define MC13783_AUD_STR_DAC_STDCCLKSEL_LSH            1
#define MC13783_AUD_STR_DAC_STDCSM_LSH            2
#define MC13783_AUD_STR_DAC_STDCBCLINV_LSH            3
#define MC13783_AUD_STR_DAC_STDCFSINV_LSH            4
#define MC13783_AUD_STR_DAC_STDCFS_LSH            5
#define MC13783_AUD_STR_DAC_STDCCLK_LSH            7
#define MC13783_AUD_STR_DAC_STDCEN_LSH            11
#define MC13783_AUD_STR_DAC_STDCCLKEN_LSH            12
#define MC13783_AUD_STR_DAC_STDCRESET_LSH            15
#define MC13783_AUD_STR_DAC_SPDIF_LSH            16
#define MC13783_AUD_STR_DAC_SR_LSH            17


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define MC13783_AUD_RX0_VAUDIOON_WID            1
#define MC13783_AUD_RX0_BIASEN_WID            1
#define MC13783_AUD_RX0_BIASSPEED_WID            1
#define MC13783_AUD_RX0_ASPEN_WID            1
#define MC13783_AUD_RX0_ASPSEL_WID            1
#define MC13783_AUD_RX0_ALSPEN_WID            1
#define MC13783_AUD_RX0_ALSPREF_WID            1
#define MC13783_AUD_RX0_ALSPSEL_WID            1
#define MC13783_AUD_RX0_LSPLEN_WID            1
#define MC13783_AUD_RX0_AHSREN_WID            1
#define MC13783_AUD_RX0_AHSLEN_WID            1
#define MC13783_AUD_RX0_AHSSEL_WID            1
#define MC13783_AUD_RX0_HSPGDIS_WID            1
#define MC13783_AUD_RX0_HSDETEN_WID            1
#define MC13783_AUD_RX0_HSDETAUTO_WID            1
#define MC13783_AUD_RX0_ARXOUTREN_WID            1
#define MC13783_AUD_RX0_ARXOUTLEN_WID            1
#define MC13783_AUD_RX0_ARXOUTSEL_WID            1
#define MC13783_AUD_RX0_CDCOUTEN_WID            1
#define MC13783_AUD_RX0_ADDCDC_WID            1
#define MC13783_AUD_RX0_ADDSTDC_WID            1
#define MC13783_AUD_RX0_ADDRXIN_WID            1

#define MC13783_AUD_RX1_PGARXEN_WID            1
#define MC13783_AUD_RX1_PGARX_WID            4
#define MC13783_AUD_RX1_PGASTEN_WID            1
#define MC13783_AUD_RX1_PGAST_WID            4
#define MC13783_AUD_RX1_ARXINEN_WID            1
#define MC13783_AUD_RX1_ARXIN_WID            1
#define MC13783_AUD_RX1_PGARXIN_WID            4
#define MC13783_AUD_RX1_MONO_WID            2
#define MC13783_AUD_RX1_BAL_WID            3
#define MC13783_AUD_RX1_BALLR_WID            1

#define MC13783_AUD_TX_MC1BEN_WID            1
#define MC13783_AUD_TX_MC2BEN_WID            1
#define MC13783_AUD_TX_MC2BDETEN_WID            1
#define MC13783_AUD_TX_AMC1REN_WID            1
#define MC13783_AUD_TX_AMC1RITOV_WID            1
#define MC13783_AUD_TX_AMC1LEN_WID            1
#define MC13783_AUD_TX_AMC1LITOV_WID            1
#define MC13783_AUD_TX_AMC2EN_WID           1
#define MC13783_AUD_TX_AMC2ITOV_WID            1
#define MC13783_AUD_TX_ATXINEN_WID            1
#define MC13783_AUD_TX_ATXOUTEN_WID            1
#define MC13783_AUD_TX_PGATXR_WID            5
#define MC13783_AUD_TX_PGATXL_WID            5

#define MC13783_SSI_NW_CDCTXRXSLOT_WID            2
#define MC13783_SSI_NW_CDCTXSECSLOT_WID            2
#define MC13783_SSI_NW_CDCRXSECSLOT_WID            2
#define MC13783_SSI_NW_CDCRXSECGAIN_WID            2
#define MC13783_SSI_NW_CDCSUMGAIN_WID            1
#define MC13783_SSI_NW_STDCSLOT_WID            2
#define MC13783_SSI_NW_STDCRXSLOT_WID            2
#define MC13783_SSI_NW_STDCRXSECSLOT_WID            2
#define MC13783_SSI_NW_STDCRXSECGAIN_WID            2
#define MC13783_SSI_NW_STDCSUMGAIN_WID            1

#define MC13783_AUD_CDC_CDCSSISEL_WID            1
#define MC13783_AUD_CDC_CDCCLKSEL_WID            1
#define MC13783_AUD_CDC_CDCSM_WID            1
#define MC13783_AUD_CDC_CDCBCLINV_WID            1
#define MC13783_AUD_CDC_CDCFSINV_WID            1
#define MC13783_AUD_CDC_CDCFSOFFSET_WID            1
#define MC13783_AUD_CDC_CDCFSLONG_WID            1
#define MC13783_AUD_CDC_CDCCLK_WID            3
#define MC13783_AUD_CDC_CDCFS8K16K_WID            1
#define MC13783_AUD_CDC_CDCEN_WID            1
#define MC13783_AUD_CDC_CDCCLKEN_WID            1
#define MC13783_AUD_CDC_CDCTS_WID            1
#define MC13783_AUD_CDC_CDCDITH_WID            1
#define MC13783_AUD_CDC_CDCRESET_WID            1
#define MC13783_AUD_CDC_CDCBYP_WID            1
#define MC13783_AUD_CDC_CDCALM_WID            1
#define MC13783_AUD_CDC_CDCDLM_WID            1
#define MC13783_AUD_CDC_AUDIHPF_WID            1
#define MC13783_AUD_CDC_AUDOHPF_WID            1

#define MC13783_AUD_STR_DAC_STDCSSISEL_WID            1
#define MC13783_AUD_STR_DAC_STDCCLKSEL_WID            1
#define MC13783_AUD_STR_DAC_STDCSM_WID            1
#define MC13783_AUD_STR_DAC_STDCBCLINV_WID            1
#define MC13783_AUD_STR_DAC_STDCFSINV_WID            1
#define MC13783_AUD_STR_DAC_STDCFS_WID            2
#define MC13783_AUD_STR_DAC_STDCCLK_WID            3
#define MC13783_AUD_STR_DAC_STDCEN_WID            1
#define MC13783_AUD_STR_DAC_STDCCLKEN_WID            1
#define MC13783_AUD_STR_DAC_STDCRESET_WID            1
#define MC13783_AUD_STR_DAC_SPDIF_WID            1
#define MC13783_AUD_STR_DAC_SR_WID            4


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// AUD_RX0
#define MC13783_AUD_RX0_VAUDIOON_DISABLE            0 //no effect
#define MC13783_AUD_RX0_VAUDIOON_FORCE            1 //force vaudio in active on mode

#define MC13783_AUD_RX0_BIASEN_DISABLE            0 //audio bias disable
#define MC13783_AUD_RX0_BIASEN_ENABLE            1 //audio bias enable

#define MC13783_AUD_RX0_BIASSPEED_DISABLE            0 //ramp speed disable
#define MC13783_AUD_RX0_BIASSPEED_ENABLE            1 //ramp speed enable

#define MC13783_AUD_RX0_ASPEN_DISABLE            0 //amplifier asp disable
#define MC13783_AUD_RX0_ASPEN_ENABLE            1 //amplifier asp enable

#define MC13783_AUD_RX0_ASPSEL_CODEC            0 //select codec
#define MC13783_AUD_RX0_ASPSEL_RIGHT            1 //select right

#define MC13783_AUD_RX0_ALSPEN_DISABLE            0 //amplifier alsp disable
#define MC13783_AUD_RX0_ALSPEN_ENABLE            1 //amplifier alsp enable

#define MC13783_AUD_RX0_ALSPREF_NOBIAS            0 //bias alsp at common audio reference disable
#define MC13783_AUD_RX0_ALSPREF_BIAS            1 //bias alsp at common audio reference enable

#define MC13783_AUD_RX0_ALSPSEL_CODEC            0 //select codec
#define MC13783_AUD_RX0_ALSPSEL_RIGHT            1 //select right

#define MC13783_AUD_RX0_LSPLEN_DISABLE            0 //output lspl disable
#define MC13783_AUD_RX0_LSPLEN_ENABLE            1 //output lspl enable

#define MC13783_AUD_RX0_AHSREN_DISABLE            0 //amplifier ahsr disable
#define MC13783_AUD_RX0_AHSREN_ENABLE            1 //amplifier ahsr enable

#define MC13783_AUD_RX0_AHSLEN_DISABLE            0 //amplifier ahsl disable
#define MC13783_AUD_RX0_AHSLEN_ENABLE            1 //amplifier ahsl enable

#define MC13783_AUD_RX0_AHSSEL_CODEC            0 //ahsr and ahsl input select codec
#define MC13783_AUD_RX0_AHSSEL_LEFTRIGHT            1 //ahsr and ahsl input select left/right

#define MC13783_AUD_RX0_HSPGDIS_ENABLE            0 //phantom ground enable
#define MC13783_AUD_RX0_HSPGDIS_DISABLE            1 //phantom ground disable

#define MC13783_AUD_RX0_HSDETEN_DISABLE            0 //headset detect disable
#define MC13783_AUD_RX0_HSDETEN_ENABLE            1 //headset detect enable

#define MC13783_AUD_RX0_HSDETAUTO_DISABLE            1 //amplifier not enabled by headset detect
#define MC13783_AUD_RX0_HSDETAUTO_ENABLE            0 //amplifier enabled by headset detect

#define MC13783_AUD_RX0_ARXOUTREN_DISABLE            0 //output rxoutr disable
#define MC13783_AUD_RX0_ARXOUTREN_ENABLE            1 //output rxoutr enable

#define MC13783_AUD_RX0_ARXOUTLEN_DISABLE            0 //output rxoutl disable
#define MC13783_AUD_RX0_ARXOUTLEN_ENABLE            1 //output rxoutl enable

#define MC13783_AUD_RX0_ARXOUTSEL_CODEC            0 //arout input select codec
#define MC13783_AUD_RX0_ARXOUTSEL_LEFTRIGHT            1 //arxout input select left/right

#define MC13783_AUD_RX0_CDCOUTEN_DISABLE            0 //output cdcout disable
#define MC13783_AUD_RX0_CDCOUTEN_ENABLE            1 //output cdcout enable

#define MC13783_AUD_RX0_ADDCDC_NOSELECT            0 //adder channel codec not selected
#define MC13783_AUD_RX0_ADDCDC_SELECT            1 //adder channel codec select

#define MC13783_AUD_RX0_ADDSTDC_NOSELECT            0 //adder channel stereo dac not selected
#define MC13783_AUD_RX0_ADDSTDC_SELECT            1 //adder channel stereo dac select

#define MC13783_AUD_RX0_ADDRXIN_NOSELECT            0 //adder channel line in not selected
#define MC13783_AUD_RX0_ADDRXIN_SELECT            1 //adder channel line in select

// AUD_RX1
#define MC13783_AUD_RX1_PGARXEN_DISABLE            0 //codec receive pga disable
#define MC13783_AUD_RX1_PGARXEN_ENABLE            1 //codec receive pga enable

#define MC13783_AUD_RX1_PGASTEN_DISABLE            0 //stereo dac pga disable
#define MC13783_AUD_RX1_PGASTEN_ENABLE            1 //stereo dac pga enable

#define MC13783_AUD_RX1_ARXINEN_DISABLE            0 //amplifier arx disable
#define MC13783_AUD_RX1_ARXINEN_ENABLE            1 //amplifier arx enable

#define MC13783_AUD_RX1_ARXIN_NOGAIN            0 //amplifier arx no additional gain
#define MC13783_AUD_RX1_ARXIN_ADDGAIN            1 //amplifier arx additional gain

#define MC13783_AUD_RX1_MONO_STEREO           0 // Stereo
#define MC13783_AUD_RX1_MONO_STEREO_OPP       1 // Stereo opposite phase
#define MC13783_AUD_RX1_MONO_STEREO2MONO      2 // Stereo to mono conversion
#define MC13783_AUD_RX1_MONO_MONO_OPP         3 // Mono opposite

#define MC13783_AUD_RX1_BALLR_RIGHT            0 //right channel balance
#define MC13783_AUD_RX1_BALLR_LEFT            1 //left channel balance

//AUD_TX
#define MC13783_AUD_TX_MC1BEN_DISABLE            0 //microphone bias 1 disable
#define MC13783_AUD_TX_MC1BEN_ENABLE            1 //microphone bias 1 enable

#define MC13783_AUD_TX_MC2BEN_DISABLE           0 //microphone bias 2 disable
#define MC13783_AUD_TX_MC2BEN_ENABLE           1 //microphone bias 2 enable

#define MC13783_AUD_TX_MC2BDETEN_DISABLE            0 //microphone bias 2 detect disable
#define MC13783_AUD_TX_MC2BDETEN_ENABLE            1 //microphone bias 2 detect enable

#define MC13783_AUD_TX_AMC1REN_DISABLE            0 //amplifier amc1r disable
#define MC13783_AUD_TX_AMC1REN_ENABLE            1 //amplifier amc1r enable

#define MC13783_AUD_TX_AMC1LEN_DISABLE            0 //amplifier amc1l disable
#define MC13783_AUD_TX_AMC1LEN_ENABLE            1 //amplifier amc1l enable

#define MC13783_AUD_TX_AMC2EN_DISABLE           0 //amplifier amc2 disable
#define MC13783_AUD_TX_AMC2EN_ENABLE           1 //amplifier amc2 enable

#define MC13783_AUD_TX_AMC_MODE_VTOV            0 //amplifier voltage to voltage mode
#define MC13783_AUD_TX_AMC_MODE_ITOV            1 //amplifier current to voltage mode

#define MC13783_AUD_TX_ATXINEN_DISABLE            0 //amplifier atxin disable
#define MC13783_AUD_TX_ATXINEN_ENABLE            1 //amplifier atxin enable

#define MC13783_AUD_TX_ATXOUTEN_DISABLE            0 //output txout disable
#define MC13783_AUD_TX_ATXOUTEN_ENABLE            1 //output txout enable

//SSI_NW
#define MC13783_SSI_NW_SUMGAIN_0DB            0 //Summed receive signal gain setting 0dB
#define MC13783_SSI_NW_SUMGAIN_MINUS6DB            1 //summed receive signal gain setting -6dB

#define MC13783_SSI_NW_RXSECGAIN_NOMIX            0 //no mixing
#define MC13783_SSI_NW_RXSECGAIN_0DB            1 //0dB
#define MC13783_SSI_NW_RXSECGAIN_MINUS6DB            2 //-6dB
#define MC13783_SSI_NW_RXSECGAIN_MINUS12DB            3 //-12dB

//AUD_CDC
#define MC13783_AUD_CDC_CDCSSISEL_PATH1            0 //codec ssi bus select rx1, bcl1, fs1
#define MC13783_AUD_CDC_CDCSSISEL_PATH2            1 //codec ssi bus select rx2, bcl2, fs2

#define MC13783_AUD_CDC_CDCCLKSEL_CLIA            0 //codec clock input select CLIA
#define MC13783_AUD_CDC_CDCCLKSEL_CLIB            1 //codec clock input select CLIB

#define MC13783_AUD_CDC_CDCSM_MASTER            0 //codec master select
#define MC13783_AUD_CDC_CDCSM_SLAVE            1 //codec slave select

#define MC13783_AUD_CDC_CDCBCLINV_NOINVERT            0 //codec bitclock inversion disable
#define MC13783_AUD_CDC_CDCBCLINV_INVERT            1 //codec bitclock inversion enable

#define MC13783_AUD_CDC_CDCFSINV_NOINVERT            0 //codec framesync inversion disable
#define MC13783_AUD_CDC_CDCFSINV_INVERT            1 //codec framesync inversion enable

#define MC13783_AUD_CDC_CDCFSOFFSET_MINUS1            0 //codec framesync offset select -1
#define MC13783_AUD_CDC_CDCFSOFFSET_ZERO            1 //codec framesync offset select 0

#define MC13783_AUD_CDC_CDCFSLONG_SHORT            0 //codec long framesync select short
#define MC13783_AUD_CDC_CDCFSLONG_LONG            1 //codec long framesync select long

#define MC13783_AUD_CDC_CDCFS8K16K_8K            0 //codec framesync select 8k
#define MC13783_AUD_CDC_CDCFS8K16K_16K            1 //codec framesync select 16k

#define MC13783_AUD_CDC_CDCEN_DISABLE            0 //codec disable
#define MC13783_AUD_CDC_CDCEN_ENABLE            1 //codec enable

#define MC13783_AUD_CDC_CDCCLKEN_DISABLE            0 //codec clocking disable
#define MC13783_AUD_CDC_CDCCLKEN_ENABLE            1 //codec clocking enable

#define MC13783_AUD_CDC_CDCTS_NONTRISTATE            0 //codec ssi non tristate
#define MC13783_AUD_CDC_CDCTS_TRISTATE            1 //codec ssi FS, TX and BCL are tristate

#define MC13783_AUD_CDC_CDCDITH_ENABLE            0 //codec dithering enable
#define MC13783_AUD_CDC_CDCDITH_DISABLE            1 //codec dithering disable

#define MC13783_AUD_CDC_CDCRESET_NORESET            0 //codec filter no reset
#define MC13783_AUD_CDC_CDCRESET_RESET            1 //codec filter reset

#define MC13783_AUD_CDC_CDCBYP_NOBYPASS            0 //codec no bypass
#define MC13783_AUD_CDC_CDCBYP_BYPASS            1 //codec bypass

#define MC13783_AUD_CDC_CDCALM_DISABLE            0 //codec analog loopback disable
#define MC13783_AUD_CDC_CDCALM_ENABLE            1 //codec analog loopback enable

#define MC13783_AUD_CDC_CDCDLM_DISABLE            0 //codec digital loopback disable
#define MC13783_AUD_CDC_CDCDLM_ENABLE            1 //codec digital loopback enable

#define MC13783_AUD_CDC_AUDIHPF_DISABLE            0 //transmit high pass filter disable
#define MC13783_AUD_CDC_AUDIHPF_ENABLE            1 //transmit high pass filter enable

#define MC13783_AUD_CDC_AUDOHPF_DISABLE            0 //receive high pass filter disable
#define MC13783_AUD_CDC_AUDOHPF_ENABLE            1 //receive high pass filter enable


//AUD_STR_DAC
#define MC13783_AUD_STR_DAC_STDCSSISEL_PATH1            0 //stereo dac ssi bus select rx1, fs1, bcl1
#define MC13783_AUD_STR_DAC_STDCSSISEL_PATH2            1 //stereo dac ssi bus select rx2, fs2, bcl2

#define MC13783_AUD_STR_DAC_STDCCLKSEL_CLIA            0 //stereo dac clock input select CLIA
#define MC13783_AUD_STR_DAC_STDCCLKSEL_CLIB            1 //stereo dac clock input select CLIB

#define MC13783_AUD_STR_DAC_STDCSM_MASTER            0 //stereo dac master mode select
#define MC13783_AUD_STR_DAC_STDCSM_SLAVE            1 //stereo dac slave mode select

#define MC13783_AUD_STR_DAC_STDCBCLINV_NOINVERT            0 //stereo dac bitclock inversion disable
#define MC13783_AUD_STR_DAC_STDCBCLINV_INVERT            1 //stereo dac bitclock inversion enable

#define MC13783_AUD_STR_DAC_STDCFSINV_NOINVERT            0 //stereo dac framesync inversion disable
#define MC13783_AUD_STR_DAC_STDCFSINV_INVERT            1 //stereo dac framesync inversion enable

#define MC13783_AUD_STR_DAC_STDCFS_NORMAL                 0 // Normal mode timing
#define MC13783_AUD_STR_DAC_STDCFS_NETWORK                1 // Network mode timing
#define MC13783_AUD_STR_DAC_STDCFS_I2S                    2 // I2S mode timing

#define MC13783_AUD_STR_DAC_STDCCLK_13MHZ                 0 // CLI = 13 MHz (master)
#define MC13783_AUD_STR_DAC_STDCCLK_15_36MHZ              1 // CLI = 15.36 MHz (master)
#define MC13783_AUD_STR_DAC_STDCCLK_16_8MHZ               2 // CLI = 16.8 MHz (master)
#define MC13783_AUD_STR_DAC_STDCCLK_26MHZ                 4 // CLI = 26 MHz (master)
#define MC13783_AUD_STR_DAC_STDCCLK_12MHZ                 5 // CLI = 12 MHz (master)
#define MC13783_AUD_STR_DAC_STDCCLK_3_6864MHZ             6 // CLI = 3.6864 MHz (master)
#define MC13783_AUD_STR_DAC_STDCCLK_33_6MHZ               7 // CLI = 33.6 MHz (master)
#define MC13783_AUD_STR_DAC_STDCCLK_MCL                   5 // CLI = MCL, PLL off (slave)
#define MC13783_AUD_STR_DAC_STDCCLK_FS                    6 // FS (slave)
#define MC13783_AUD_STR_DAC_STDCCLK_BCL                   7 // BCL (slave)

#define MC13783_AUD_STR_DAC_STDCEN_DISABLE            0 //stereo dac disable
#define MC13783_AUD_STR_DAC_STDCEN_ENABLE            1 //stereo dac enable

#define MC13783_AUD_STR_DAC_STDCCLKEN_DISABLE            0 //stereo dac clocking disable
#define MC13783_AUD_STR_DAC_STDCCLKEN_ENABLE            1 //stereo dac clocking enable

#define MC13783_AUD_STR_DAC_STDCRESET_NORESET            0 //stereo dac filter no reset
#define MC13783_AUD_STR_DAC_STDCRESET_RESET            1 //stereo dac filter reset

#define MC13783_AUD_STR_DAC_SPDIF_DISABLE            0 //stereo dac ssi spdif mode disable
#define MC13783_AUD_STR_DAC_SPDIF_ENABLE            1 //stereo dac ssi spdif mode enable

#define MC13783_AUD_STR_DAC_SR_8000                       0 // FS = 8 KHz
#define MC13783_AUD_STR_DAC_SR_11025                      1 // FS = 11.025 KHz
#define MC13783_AUD_STR_DAC_SR_12000                      2 // FS = 12 KHz
#define MC13783_AUD_STR_DAC_SR_16000                      3 // FS = 16 KHz
#define MC13783_AUD_STR_DAC_SR_22050                      4 // FS = 22.050 KHz
#define MC13783_AUD_STR_DAC_SR_24000                      5 // FS = 24 KHz
#define MC13783_AUD_STR_DAC_SR_32000                      6 // FS = 32 KHz
#define MC13783_AUD_STR_DAC_SR_44100                      7 // FS = 44.1 KHz
#define MC13783_AUD_STR_DAC_SR_48000                      8 // FS = 48 KHz
#define MC13783_AUD_STR_DAC_SR_64000                      9 // FS = 64 KHz
#define MC13783_AUD_STR_DAC_SR_96000                      10 // FS = 96 KHz

#ifdef __cplusplus
}
#endif

#endif // __MC13783_REGS_AUDIO_H__

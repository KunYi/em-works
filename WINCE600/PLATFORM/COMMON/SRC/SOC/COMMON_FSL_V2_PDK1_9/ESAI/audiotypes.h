//------------------------------------------------------------------------------
//
//  Copyright (C) 2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: audiotypes.h
//
//  Definitions that could possibly used in BSP functions.
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Types

#pragma once

typedef enum
{
    AUDIO_BUS_DAC,
    AUDIO_BUS_ADC
} AUDIO_BUS;

typedef enum
{
    AUDIO_PROTOCOL_NETWORK = 0,
    AUDIO_PROTOCOL_I2S = 1,
    AUDIO_PROTOCOL_LEFT_ALIGNED = 2,
} AUDIO_PROTOCOL;

typedef enum
{
    AUDMUX_PORT1 = 0,
    AUDMUX_PORT2 = 1,
    AUDMUX_PORT3 = 2,
} AUDMUX_INTERNAL_PORT;

typedef enum
{
    AUDMUX_PORT4 = 3,
    AUDMUX_PORT5 = 4,
    AUDMUX_PORT6 = 5,
    AUDMUX_PORT7 = 6,
} AUDMUX_EXTERNAL_PORT;

//------------------------------------------------------------------------------
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: rotaryhw.h
//------------------------------------------------------------------------------
#ifndef __ROTARYHW_H
#define __ROTARYHW_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Definitions  (Rotary )
//------------------------------------------------------------------------------

#define ROT_DEBOUNCE_DIVIDER            0
#define ROT_DEBOUNCE_DIVIDER_MAX        63


#define ROT_DEBOUNCE_OVERSAMPLE         2
#define ROT_DEBOUNCE_OVERSAMPLE_MAX     3


#define ROT_COUNTER_RELATIVE_ON         1
#define ROT_COUNTER_RELATIVE_OFF        !ROT_COUNTER_RELATIVE_ON


#define ROT_INPUT_POLARITY_INVERSE      1
#define ROT_INPUT_POLARITY_NO_CHANGE    !ROT_INPUT_POLARITY_INVERSE

typedef struct _ROTARYINFO
{
    BOOL ROTARYINIT;
    HANDLE hHandle;
} ROTARYINFO, *PROTARYINFO;

// Enumeration of Rotary decoder input sources of SELECTA or B
typedef enum _ROT_INPUT_SOURCE
{
    ROT_SELECT_NEVER_TICK=0,
    ROT_SELECT_PWM0,
    ROT_SELECT_PWM1,
    ROT_SELECT_PWM2,
    ROT_SELECT_PWM3,
    ROT_SELECT_PWM4,
    ROT_SELECT_ROTARYA,
    ROT_SELECT_ROTARYB,
    ROT_SELECT_MAX
} ROT_INPUT_SOURCE;

// Structure for RotaryInit
typedef struct _ROT_SETUP
{
    // RELATIVE flag for up or down counter operation
    BOOL Relative;
    // POLARITY_A flag to set the polarity for SelectA input
    BOOL PolA;
    // POLARITY_B flag to set the polarity for SelectB input
    BOOL PolB;
    // SELECT_A bitfield (3 bits) for selecting an input source
    ROT_INPUT_SOURCE SelectA;
    // SELECT_A bitfield (3 bits) for selecting an input source
    ROT_INPUT_SOURCE SelectB;
    // DIVIDER bitfield (6 bits) for the clock rate divider
    UINT8 Divider;
    // OVERSAMPLE bitfield (2 bits) for the oversampling rate
    UINT8 Oversample;
} ROT_SETUP;

//------------------------------------------------------------------------------
// Prototypes  (Rotary )
//------------------------------------------------------------------------------

// Initializes the rotary decoder hardware to the normal operation.
BOOL RotaryInit(ROT_SETUP *stRotSetup);
// Deinitializes the rotary decoder hardware to the normal operation.
BOOL RotaryDeinit();
// Read the rotary decoder up/down counter.
INT16 RotaryGetUpdownCount (VOID);
//Enable or disable the clock gate of the timer and rotary block.
VOID RotarySetClkGate(BOOL Value);
//This function clears the  SFTRST bit if the rotary block's is SFTRST bit //  is set
VOID RotarySetSftRst();
//The function reenable timer  for timer interrupt
void RotarySetTimer();


#if __cplusplus
}
#endif

#endif //__ROTARYHW_H

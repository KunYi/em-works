//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// Header: touch.h
//
//------------------------------------------------------------------------------

#ifndef __TOUCH_H__
#define __TOUCH_H__

//--------------------------------------------------------------
// Defines
//--------------------------------------------------------------

typedef struct {
    int x;
    int y;
    BOOL valid;
} T_POSITION;

// ADC Driver
#define ADC_DRIVER_NAME     TEXT("ADC1:")

// Low sampling rate in ms.
#define TOUCHPANEL_SAMPLE_RATE_LOW                  5

// High sampling rate in ms.
#define TOUCHPANEL_SAMPLE_RATE_HIGH                 3





#endif //

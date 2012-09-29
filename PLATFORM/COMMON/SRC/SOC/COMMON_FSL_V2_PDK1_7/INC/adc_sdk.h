//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  adc.h
//
//   Header file for ADC driver.
//
//------------------------------------------------------------------------------

#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

typedef enum {
    INAUX0,
    INAUX1,
    INAUX2,
    WIPER,
    CHAN_NUM
} CHAN_ID;

typedef enum {
    EXTREF,
    INTREF
}POS_REF;

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Functions

BOOL AdcInit(void);
void AdcDeinit(void);
BOOL AdcConfigureChannel(CHAN_ID id, DWORD settlingTime, POS_REF pRef, DWORD numSamples);
BOOL AdcGetSamples(CHAN_ID id, UINT16* pBuf, DWORD nbSamples);

#ifdef __cplusplus
}
#endif

#endif // __ADC_H__

//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// adc.h
//
// Definition for structures use in ADC driver
//
//-----------------------------------------------------------------------------

#ifndef __ADC_H__
#define __ADC_H__

typedef struct {
    PCSP_ADC_REGS pADCRegs;
    BOOL    fEnableWakeUp;
    CRITICAL_SECTION cs;
} T_DEVICE_CONTEXT;

typedef struct {
    T_DEVICE_CONTEXT* pDevCtxt;
} T_OPEN_CONTEXT;

#endif //__ADC_H__

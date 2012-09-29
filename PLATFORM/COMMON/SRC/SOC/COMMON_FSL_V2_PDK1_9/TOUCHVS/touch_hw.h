//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  touch_hw.cpp
//
//  Provides the header file for PDD code for the DDSI touch driver routines.
//
//-----------------------------------------------------------------------------
#ifndef __TOUCH_HW_H__
#define __TOUCH_HW_H__

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-------------------------------------------------------------------------
// Defines
#define _CH_XP_                             LRADC_CH2
#define _CH_YP_                             LRADC_CH3
#define _CH_XM_                             LRADC_CH4
#define _CH_YM_                             LRADC_CH5
#define _CH_SAMPLE_X_                   2
#define _CH_SAMPLE_Y_                   3
#define _CH_NORMALIZE_                  6

#define _LIMIT_CONFIRM_RETIRES_         2
#define _DEBOUNCE_SAMPLES_                      4

#define HW_TP_DDSI_WAIT_SHARE_MUTEX()           TRUE
#define HW_TP_DDSI_FREE_SHARE_MUTEX()


#define _LIMIT_CONFIRM_RETIRES_                 2
#define _DEBOUNCE_SAMPLES_                      4

//-----------------------------------------------------------------------------
// Types
typedef struct _ST_TP_SAMPLE_POINT_ {
    USHORT x;
    USHORT y;
} _ST_TP_SAMPLE_POINT, *_PST_TP_SAMPLE_POINT;

typedef struct _ST_TP_SAMPLES_ {
    _ST_TP_SAMPLE_POINT Confirmed;
    _ST_TP_SAMPLE_POINT Current;
    _ST_TP_SAMPLE_POINT Previous;
} _ST_TP_SAMPLES, *_PST_TP_SAMPLES;

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions
extern BOOL                                         TouchConfig(void);
extern void                                         TouchDeAllocateResource(void);
extern BOOL                                         TouchAllocateResource(void);
extern void                                         TouchInterruptClear(void);
extern _EN_TP_INTERRUPT                             TouchInterruptCapture(_EN_TP_INTERRUPT irqCurrent);
extern void                                         TouchRegisterDump(void);
extern TOUCH_PANEL_SAMPLE_FLAGS                     TouchGetSample(INT *x, INT *y);
extern void                                         TouchCaturePenDown(void);

#endif  // __TOUCH_HW_H__.

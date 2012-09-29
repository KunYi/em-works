//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  touch_ddsi.h
//
//  Provides the header file for the DDSI touch driver routines.
//
//-----------------------------------------------------------------------------
#ifndef __TOUCH_DDSI_H__
#define __TOUCH_DDSI_H__

//
// There are two expected stylus interrupt states: sylus down (the stylus is currently up) and
// stylus up or a timer event (the stylus is down and will either be lifted or the timer will
// fire and gwes will capture the point for drawing).
//
typedef enum {
    _TP_INTERRUPT_PEN_DOWN_,   /*0*/
    _TP_INTERRUPT_PEN_UP_OR_TIMER_, /*1*/
    _TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_X_=_TP_INTERRUPT_PEN_UP_OR_TIMER_, /*1*/
    _TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_Y_, /*2*/
    _TP_INTERRUPT_PEN_UP_,
    _TP_INTERRUPT_BAD_
} _EN_TP_INTERRUPT;

#endif  // __TOUCH_DDSI_H__.

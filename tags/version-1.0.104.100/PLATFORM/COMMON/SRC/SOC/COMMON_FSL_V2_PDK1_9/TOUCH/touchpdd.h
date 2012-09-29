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
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  Touchpdd.h
//
//   Header file for touch panel driver.
//
//------------------------------------------------------------------------------
#ifndef __TOUCHPDD_H__
#define __TOUCHPDD_H__

//------------------------------------------------------------------------------
// Defines

// These macros define the touch panel sample rate and sample number.
#define TOUCH_SAMPLE_RATE_LOW           100    // Low sample rate
#define TOUCH_SAMPLE_RATE_HIGH          200    // High sample rate

//------------------------------------------------------------------------------
// Types

typedef enum TouchPowerStatus
{
    TouchPowerOff,
    TouchPowerOn
} TouchPowerStatus_c;

//------------------------------------------------------------------------------
// Functions

#endif //__TOUCHPDD_H__

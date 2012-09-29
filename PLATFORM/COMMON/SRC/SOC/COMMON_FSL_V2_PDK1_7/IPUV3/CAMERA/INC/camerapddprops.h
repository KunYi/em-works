//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#ifndef _CAMERAPDDPROPS_H
#define _CAMERAPDDPROPS_H

#define MAX_SUPPORTED_PINS        3
#define MAX_PINNAME_LENGTH       10
#define MAX_VIDEO_FORMAT         10

#define NUM_VIDEOPROCAMP_ITEMS   10
#define NUM_CAMERACONTROL_ITEMS   8
#define NUM_PROPERTY_ITEMS       NUM_VIDEOPROCAMP_ITEMS + NUM_CAMERACONTROL_ITEMS
#define NUM_SUPPORTED_PINS        3


static const WCHAR g_wszPinNames[MAX_SUPPORTED_PINS][MAX_PINNAME_LENGTH] = { L"Capture"
                                                                            ,L"Still"
                                                                            ,L"Preview"
                                                                            };

static const WCHAR g_wszPinDeviceNames[MAX_SUPPORTED_PINS][MAX_PINNAME_LENGTH] = {L"PIN1:",
                                                                                  L"PIN1:",
                                                                                  L"PIN1:"};

#endif

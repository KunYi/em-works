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
//  Copyright (C) 2004-2007,2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
#define NUM_PIN_BUFFER            3

// There is a known issue in CE6 that causes Data Aborts in AllocPhysMem when allocating
// too much pages in a given section, which happens in the driver when operating multiple
// allocation/deallocation in a row. This works around the issue by allocating CSI buffers
// only once with the maximum size required by the largest resolution supported.
#define SENNA_FIX_MEM_ALLOC_ISSUE 1
#define MAX_IMAGE_SIZE              (1280*960*2)

static const WCHAR g_wszPinNames[MAX_SUPPORTED_PINS][MAX_PINNAME_LENGTH] = { L"Capture"
                                                                            ,L"Still"
                                                                            ,L"Preview"
                                                                            };

static const WCHAR g_wszPinDeviceNames[MAX_SUPPORTED_PINS][MAX_PINNAME_LENGTH] = {L"PIN1:",
                                                                                  L"PIN1:",
                                                                                  L"PIN1:"};

#endif

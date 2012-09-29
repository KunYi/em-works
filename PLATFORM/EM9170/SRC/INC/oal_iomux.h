//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------//
//  File: oal_iomux.h
//
//  This header file defines the custom IOMUX OAL module. This module implements
//  function used to configure and manage the SoC IOMUX.
//
//------------------------------------------------------------------------------
#ifndef __OAL_IOMUX_H
#define __OAL_IOMUX_H

#if __cplusplus
extern "C" {
#endif

#define OAL_IOMUX_SET_MUX(pIOMUX, pin, muxmode, sion) \
    OUTREG32(&pIOMUX->SW_MUX_CTL[pin], (muxmode | sion))

#define OAL_IOMUX_SET_PAD(pIOMUX, pad, slew, drive, opendrain, pull, hysteresis, voltage) \
    OUTREG32(&pIOMUX->SW_PAD_CTL[pad], (slew | drive | opendrain | pull | hysteresis | voltage))

#define OAL_IOMUX_SET_INPUT(pIOMUX, port, daisy) \
    OUTREG32(&pIOMUX->SW_SELECT_INPUT[port], daisy)

#if __cplusplus
}
#endif

#endif //__OAL_IOMUX_H

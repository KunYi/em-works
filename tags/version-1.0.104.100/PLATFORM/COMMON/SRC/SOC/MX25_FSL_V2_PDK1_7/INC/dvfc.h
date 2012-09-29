//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// Header: dvfc.h
//
// Provides definitions for RNGB module.
//
//------------------------------------------------------------------------------
#ifndef __SOC_DVFC_H
#define __SOC_DVFC_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define DVFC_FID                            _T("DVF1:")

#define DVFC_IOCTL_REQUEST_SETPOINT         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3030, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Request setpoint (predictive dvfs api)
#define DVFC_IOCTL_RELEASE_SETPOINT         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3031, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Release setpoint (predictive dvfs api)
#define DVFC_IOCTL_QUERY_SETPOINT           CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3032, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Get setpoint count(predictive dvfs api)

#define DVFC_SETPOINT_TURBO                 0
#define DVFC_SETPOINT_HIGH                  1
#define DVFC_SETPOINT_MEDIUM                2
#define DVFC_SETPOINT_LOW                   3

#define DVFC_NUM_SETPOINTS                  4


//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------
HANDLE DvfcOpenHandle(LPCWSTR lpDevName);
BOOL DvfcCloseHandle(HANDLE hDvfc);
BOOL DvfcRequestSetpoint(HANDLE hDvfc, DWORD dwSetpoint, PDWORD pdwReferCount);
BOOL DvfcReleaseSetpoint(HANDLE hDvfc, DWORD dwSetpoint, PDWORD pdwReferCount);
BOOL DvfcQuerySetpoint(HANDLE hDvfc, PDWORD pdwReferCountBuf, DWORD BufferSize, PDWORD pdwActualOut);


#ifdef __cplusplus
}
#endif

#endif // __SOC_DVFC_H

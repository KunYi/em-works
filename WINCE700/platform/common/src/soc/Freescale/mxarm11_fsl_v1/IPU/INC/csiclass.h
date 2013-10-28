//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CsiClass.h
//
//  Common definitions for csi module
//
//-----------------------------------------------------------------------------
#include "IpuModuleInterfaceClass.h"

#ifndef __CSICLASS_H__
#define __CSICLASS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types

// The data format from the sensor
typedef enum csiSensorOutputFormatEnum
{
    csiSensorOutputFormat_RGB,
    csiSensorOutputFormat_YUV444,
    csiSensorOutputFormat_YUV422,
    csiSensorOutputFormat_Bayer,
} csiSensorOutputFormat;

// The image resolution from the sensor
typedef enum csiSensorOutputResolutionEnum
{
    csiSensorOutputResolution_VGA,
    csiSensorOutputResolution_QVGA,
    csiSensorOutputResolution_CIF,
    csiSensorOutputResolution_QCIF,
    csiSensorOutputResolution_QQVGA,
    csiSensorOutputResolution_SVGA,
    csiSensorOutputResolution_XGA,
    csiSensorOutputResolution_SXGA,
    csiSensorOutputResolution_UXGA,
} csiSensorOutputResolution;


class CsiClass : public IpuModuleInterfaceClass
{
    public:
        CsiClass();
        ~CsiClass();
        void CsiEnable(IC_CHANNEL);
        void CsiEnable(void);
        void CsiDisable(IC_CHANNEL);
        void CsiDisable(void);
        BOOL CsiConfigureSensor(csiSensorOutputFormat, csiSensorOutputResolution);
        void CsiZoom(DWORD);
        void CameraSensorDeconfig();
        void CsiDetectEOF(void);
        
    private:
        void CsiInit(void);
        void CsiDeinit(void);

        HANDLE m_hIPUBase;

        BOOL m_bEncCsiEnable;
        BOOL m_bVfCsiEnable;
        BOOL m_bDisableCsi;
};

//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif  // __CSICLASS_H__

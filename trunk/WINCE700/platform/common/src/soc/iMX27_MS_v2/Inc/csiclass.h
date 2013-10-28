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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  File:  CsiClass.h
//
//  Common definitions for csi module
//
//-----------------------------------------------------------------------------

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
    csiSensorOutputResolution_SXGA,
    csiSensorOutputResolution_PAL,
    csiSensorOutputResolution_NTSC
} csiSensorOutputResolution;

class CsiClass 
{
    public:
        CsiClass();
        ~CsiClass();
        void CsiEnable(void);
        void CsiDisable(void);

        BOOL CsiConfigureSensor(csiSensorOutputFormat, csiSensorOutputResolution);
        void CsiChangeFrameRate(DWORD);
        void CsiZoom(DWORD);
        
    private:
        PCSP_CSI_REGS m_pCSI;
        BOOL m_bIsCsiEnabled;
	 
        void CsiInit(void);
        void CsiDeinit(void);
};

//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif  // __CSICLASS_H__

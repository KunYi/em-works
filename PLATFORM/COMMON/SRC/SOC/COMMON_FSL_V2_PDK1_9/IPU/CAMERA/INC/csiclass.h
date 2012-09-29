//-----------------------------------------------------------------------------
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
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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
#ifndef __CSICLASS_H__
#define __CSICLASS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
// For Ringo TVIN +:only for tvin used
//------------------------------------------------------------------------------
//     Lines    Field/VBlk                      Line
//      PAL       NTSC F          V                    Description
//      22        19            Field 1 - First Vertical Blanking(Top)
//      288      240            Field 1 - Active Video
//      2          3            Field 1 - Second Vertical Blanking(Bottom)
//      23        20            Field 2 - First Vertical Blanking(Top)
//      288      240            Field 2 - Active Video
//      2          3            Field 2 - Second Vertical Blanking(Bottom)
//      625      525
//------------------------------------------------------------------------------
#define NBL_PAL  24*2 //numbers of blanking lines
#define NBL_NTSC 22
#define SKIP_NTSC 12

// Set CSI Test Mode:only work in HSP_CLK mode
#define CSI_TEST_MODE 0
// Sensor Clk mode
// Sensor Clk Source:SENSB_SENS_CLK clock or HSP_CLK clock after division.
#define CSI_SENSB_SENS_CLK 0

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
    csiSensorOutputResolution_VGA,   //640*480
    csiSensorOutputResolution_QVGA,  //320*240
    csiSensorOutputResolution_CIF,   //352*288
    csiSensorOutputResolution_QCIF,  //176*144
    csiSensorOutputResolution_QQVGA, //160*120
    csiSensorOutputResolution_SVGA,  //800*600
    csiSensorOutputResolution_XGA,   //1024*768
    csiSensorOutputResolution_SXGA,  //1280*1024
    csiSensorOutputResolution_UXGA,  //1600*1200
    csiSensorOutputResolution_WVGA,  //800*480
    csiSensorOutputResolution_PAL,   //720*576, scan lines is 625
    csiSensorOutputResolution_NTSC   //720*480, scan lines is 525
} csiSensorOutputResolution;  

// The image resolution from the sensor
typedef enum csiSensorOutputResolutionWidthEnum
{
    VGA_Width = 640,    //640*480
    QVGA_Width = 320,   //320*240
    CIF_Width = 352,    //352*288
    QCIF_Width = 176,   //176*144
    QQVGA_Width = 160,  //160*120
    SVGA_Width = 800,   //800*600
    XGA_Width = 1024,   //1024*768
    SXGA_Width = 1280,  //1280*1024
    UXGA_Width = 1600,  //1600*1200
    WVGA_Width = 800,   //800*480
    PAL_Width = 720,    //720*576, scan lines is 625
    NTSC_Width = 720    //720*480, scan lines is 525
} csiSensorOutputResolutionWidth;  

// The image resolution from the sensor
typedef enum csiSensorOutputResolutionHeightEnum
{
    VGA_Height = 480,    //640*480
    QVGA_Height = 240,   //320*240
    CIF_Height = 288,    //352*288
    QCIF_Height = 144,   //176*144
    QQVGA_Height = 120,  //160*120
    SVGA_Height = 600,   //800*600
    XGA_Height = 768,    //1024*768
    SXGA_Height = 1024,  //1280*1024
    UXGA_Height = 1200,  //1600*1200
    WVGA_Height = 480,   //800*480
    PAL_Height = 576,    //720*576, scan lines is 625
    NTSC_Height = 506    //720*480, scan lines is 525
} csiSensorOutputResolutionHeight;  


class CsiClass : public IpuModuleInterfaceClass
{
    public:
        CsiClass();
        ~CsiClass();
        void CsiEnable(IC_CHANNEL);
        void CsiEnable(void);
        void CsiDisable(IC_CHANNEL);
        void CsiDisable(void);
        BOOL CsiConfigure(csiSensorOutputFormat*, csiSensorOutputResolution*);
        BOOL CsiConfigureSensor();
        void CsiZoom(DWORD);
        void CameraSensorDeconfig();
        BOOL CsiDetectEOF(void);

        // camera type
        int m_iCamType; // check bspcsi.h->typedef enum csiSensorId,-1 error type
        
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

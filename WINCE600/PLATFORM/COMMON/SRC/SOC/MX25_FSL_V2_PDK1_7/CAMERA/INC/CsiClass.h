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
// Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
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

#include "common_macros.h"
#include "csp.h"

    //------------------------------------------------------------------------------
// Defines


#define CSI_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define CSI_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

//------------------------------------------------------------------------------
// Types

// The data source for CSI
typedef enum csiConnectedDataSourceEnum
{
    csiConnectedDataSource_PARALLEL = 0,
    csiConnectedDataSource_MIPI,
}csiConnectedDataSource;
// The data format from the sensor
typedef enum csiSensorOutputFormatEnum
{
    csiSensorOutputFormat_RGB888,
    csiSensorOutputFormat_YUV444,
    csiSensorOutputFormat_YUV422_UYVY,
    csiSensorOutputFormat_YUV422_YUY2,
    csiSensorOutputFormat_YUV420,
    csiSensorOutputFormat_Bayer,
    csiSensorOutputFormat_RGB565,
    csiSensorOutputFormat_RGB555,
    csiSensorOutputFormat_RGB444,
    csiSensorOutputFormat_JPEG,
} csiSensorOutputFormat;

// The image resolution from the sensor
typedef enum csiSensorOutputResolutionEnum
{
    csiSensorOutputResolution_D1_PAL,//720*576
    csiSensorOutputResolution_D1_NTSC,//720*480
    csiSensorOutputResolution_VGA,   //640*480
    csiSensorOutputResolution_QVGA,  //320*240
    csiSensorOutputResolution_CIF,   //352*288
    csiSensorOutputResolution_QCIF,  //176*144
    csiSensorOutputResolution_QQCIF, //88*72
    csiSensorOutputResolution_QQVGA, //160*120
    csiSensorOutputResolution_SVGA,  //800*600
    csiSensorOutputResolution_XGA,   //1024*768
    csiSensorOutputResolution_1024_800,//1024*800 
    csiSensorOutputResolution_1280_960,//1280*960
    csiSensorOutputResolution_SXGA,  //1280*1024
    csiSensorOutputResolution_UXGA,  //1600*1200
    csiSensorOutputResolution_QXGA,  //2048*1536
} csiSensorOutputResolution;  

// The image resolution from the sensor
typedef enum csiSensorOutputResolutionWidthEnum
{
    VGA_Width = 640,    //640*480
    QVGA_Width = 320,   //320*240
    CIF_Width = 352,    //352*288
    QCIF_Width = 176,   //176*144
    QQCIF_Width = 88,   //88*72
    QQVGA_Width = 160,  //160*120
    SVGA_Width = 800,   //800*600
    XGA_Width = 1024,   //1024*768
    SXGA_1024_800_Width = 1024, //1024*800
    SXGA_1280_960_Width = 1280, //1280*960
    SXGA_Width = 1280,  //1280*1024
    UXGA_Width = 1600,  //1600*1200
    QXGA_Width = 2048,  //2048*1536
    D1_Width = 720, //720*576 720*480
} csiSensorOutputResolutionWidth;  

// The image resolution from the sensor
typedef enum csiSensorOutputResolutionHeightEnum
{
    VGA_Height = 480,    //640*480
    QVGA_Height = 240,   //320*240
    CIF_Height = 288,    //352*288
    QCIF_Height = 144,   //176*144
    QQCIF_Height = 72,   //88*72
    QQVGA_Height = 120,  //160*120
    SVGA_Height = 600,   //800*600
    XGA_Height = 768,    //1024*768
    SXGA_1024_800_Height = 800, //1024*800
    SXGA_1280_960_Height = 960, //1280*960
    SXGA_Height = 1024,  //1280*1024
    UXGA_Height = 1200,  //1600*1200
    QXGA_Height = 1536,  //2048*1536
    D1_PAL_Height = 576, //720*576
    D1_NTSC_Height = 480, //720*480
} csiSensorOutputResolutionHeight;  

// The number of sensors connected and their associated data widths
typedef enum csiSensorInputDataConfigEnum
{
    Two8BitSensors = 0,
    One8BitSensor = 1,
    One10BitSensor = 2,
    One16BitSensor = 3
} csiSensorInputDataConfig;

class CsiClass 
{
    public:
        CsiClass();
        ~CsiClass();
        void CsiEnable(void);
        void CsiDisable(void);

        BOOL    CsiConfigure(csiSensorOutputFormat, csiSensorOutputResolution);
        BOOL    CsiConfigureSensor(DWORD dwFramerate);
        void    CsiChangeFrameRate(DWORD);
        DWORD   CsiGetFrameCount(void);
        void    CsiZoom(DWORD);
        BOOL    CsiAllocateBuffers(ULONG numBuffers, ULONG bufSize);
        BOOL    CsiDeleteBuffers();
        BOOL    CsiStartChannel(void);
        BOOL    CsiStopChannel(void);
        BOOL    CsiISRLoop(UINT32 timeout);

        BOOL    RegisterBuffer(ULONG bufSize, LPVOID pVirtualAddress, UINT32 pPhysicalAddress);
        BOOL    UnregisterBuffer(LPVOID pVirtualAddress, UINT32 pPhysicalAddress);
        BOOL    Enqueue(void);
        UINT32* GetBufFilled(void);
        void    PrintBufferInfo(void);

        void    BSPEnableCamera(void);
        void    BSPDisableCamera(void);
        void    BSPGetSensorFormat(DWORD *pSensorFormat);
        void    BSPGetSensorResolution(DWORD *pSensorResolution);

        HANDLE  m_hCSIEOFEvent;

    private:
        PCSP_CSI_REGS               m_pCSI;
        BOOL                        m_bIsCsiEnabled;
        CamBufferManager*           m_pBufferManager;
        HANDLE                      m_hExitCsiISRThread;
        HANDLE                      m_hCsiISRThread;
        HANDLE                      m_hCsiIntrEvent;
        DWORD                       m_dwCsiSysIntr;
        DWORD                       m_dwCurrentDMA;
        DWORD                       m_dwFramerate;

        void                        BSPResetCamera(void);
        void                        BSPSetupCamera(DWORD dwFramerate);
        void                        BSPDeleteCamera(void);
        BOOL                        BSPCSIIOMUXConfig(void);
        void                        BSPSetDigitalZoom (DWORD zoom);
        BOOL                        BSPSensorSetClockGatingMode(BOOL startClocks);
        BOOL                        BSPGetDefaultCameraFromRegistry(void);
        BOOL                        BSPCameraSetOutputResolution(DWORD dwFramerate,csiSensorOutputResolution outputResolution);
        BOOL                        BSPCameraSetOutputFormat(csiSensorOutputFormat outputFormat);
        csiSensorInputDataConfig    BSPCSIGetInputDataConfig(void);
        void                        dumpCsiRegisters(void);

        void CsiInit(void);
        void CsiDeinit(void);
};

//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif  // __CSICLASS_H__

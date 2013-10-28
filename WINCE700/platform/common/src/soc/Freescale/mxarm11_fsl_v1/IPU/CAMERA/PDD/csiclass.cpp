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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CsiClass.cpp
//
//  Implementation of CMOS Sensor Interface Product Device Driver
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <Devload.h>
#include <NKIntr.h>
#include <ceddk.h>

#include "mxarm11.h"
#include "cameradbg.h"
#include "ipu.h"
#include "CsiClass.h"

//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPGetDefaultCameraFromRegistry();
extern BOOL BSPCSIIOMUXConfig();
extern void BSPSetupCamera();
extern void BSPDeleteCamera();
extern void BSPEnableCamera();
extern void BSPDisableCamera();
extern void BSPResetCamera();
extern UINT32 BSPGetSensorClockRatio();
extern void BSPSetDigitalZoom(DWORD);
extern BOOL BSPCameraSetOutputResolution(csiSensorOutputResolution);
extern BOOL BSPCameraSetOutputFormat(csiSensorOutputFormat);

//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define CSI_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define CSI_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: CsiClass
//
// CsiClass constructor.  Calls CsiInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
CsiClass::CsiClass()
{
    m_bEncCsiEnable = FALSE;
    m_bVfCsiEnable = FALSE;
    m_bDisableCsi = FALSE;

    // open handle to the IPU_BASE driver in order to enable IC module
    m_hIPUBase = CreateFile(TEXT("IPU1:"),          // "special" file name
        GENERIC_READ|GENERIC_WRITE,   // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                   // security attributes (=NULL)
        OPEN_EXISTING,            // creation disposition
        FILE_FLAG_RANDOM_ACCESS,  // flags and attributes
        NULL);                  // template file (ignored)
    if (m_hIPUBase == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
    }

    CsiInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~CsiClass
//
// CsiClass destructor.  Calls CsiDeinit to deinitialize module.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
CsiClass::~CsiClass()
{
    CsiDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: CsiInit
//
// This function initializes the Camera Sensor Interface and
// Image Converter modules.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
void CsiClass::CsiInit()
{
    CSI_FUNCTION_ENTRY();

    // Request IOMUX pins
    
    // Configure IOMUX for I2C pins
    if (!BSPCSIIOMUXConfig())
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Error configuring IOMUX for CSI.\r\n"), __WFUNCTION__));
        return;
    }

    // Get camera-in-use from registry key. Otherwise use default
    if (!BSPGetDefaultCameraFromRegistry())
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Interrupt initialization failed! (IPU Error Interrupt)\r\n"), __WFUNCTION__));
        goto Error;
    }

    CSI_FUNCTION_EXIT();

    return;

Error:
    CsiDeinit();
    return;
}

//-----------------------------------------------------------------------------
//
// Function: CsiDeinit
//
// This function deinitializes the Camera Sensor Interface module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiDeinit()
{
    CSI_FUNCTION_ENTRY();

    CsiDisable(IC_CHANNEL_ENC);
    CsiDisable(IC_CHANNEL_VF);

    BSPDeleteCamera();

    CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: CsiEnable
//
// Enable the Camera Sensor Interface.
//
// Parameters:
//      channel
//          [in] Identifies the preprocessing channel (encoding or viewfinding)
//          that is requesting that the CSI be enabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiEnable(IC_CHANNEL channel)
{
    CSI_FUNCTION_ENTRY();

    if (channel == IC_CHANNEL_ENC)
    {
        m_bEncCsiEnable = TRUE;

        // If VF has already enabled CSI, we are done
        if (m_bVfCsiEnable)
        {
            return;
        }
    }
    else if (channel == IC_CHANNEL_VF)
    {
        m_bVfCsiEnable = TRUE;

        // If ENC has already enabled CSI, we are done
        if (m_bEncCsiEnable)
        {
            return;
        }
    }

    CsiEnable();

    CSI_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: CsiEnable
//
// Enable the Camera Sensor Interface.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiEnable(void)
{
    DWORD dwBytesTransferred;

    CSI_FUNCTION_ENTRY();

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Enabling CSI!\r\n"), __WFUNCTION__));
   
    if (!DeviceIoControl(m_hIPUBase,        // file handle to the driver
             IPU_IOCTL_ENABLE_CSI,          // I/O control code
             NULL,                          // in buffer
             0,                             // in buffer size
             NULL,                          // out buffer
             0,                             // out buffer size
             &dwBytesTransferred,           // number of bytes returned
             NULL))                         // ignored (=NULL)
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to enable CSI!\r\n"), __WFUNCTION__));
    }

    CSI_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: CsiDisable
//
// Disable the Camera Sensor Interface.
//
// Parameters:
//      channel
//          [in] Identifies the preprocessing channel (encoding or viewfinding)
//          that is requesting that the CSI be enabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiDisable(IC_CHANNEL channel)
{
    CSI_FUNCTION_ENTRY();

    if (channel == IC_CHANNEL_ENC)
    {
        m_bEncCsiEnable = FALSE;

        // If VF has already enabled CSI, we are done
        if (m_bVfCsiEnable)
        {
            return;
        }
    }
    else if (channel == IC_CHANNEL_VF)
    {
        m_bVfCsiEnable = FALSE;

        // If ENC has already enabled CSI, we are done
        if (m_bEncCsiEnable)
        {
            return;
        }
    }

    // For power manager: suspend-resume crash ++
    CsiDetectEOF();
    
    CsiDisable();

    CSI_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: CsiDisable
//
// Disable the Camera Sensor Interface.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiDisable(void)
{
    DWORD dwBytesTransferred;

    CSI_FUNCTION_ENTRY();

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Disabling CSI!\r\n"), __WFUNCTION__));

    if (!DeviceIoControl(m_hIPUBase,        // file handle to the driver
             IPU_IOCTL_DISABLE_CSI,         // I/O control code
             NULL,                          // in buffer
             0,                             // in buffer size
             NULL,                          // out buffer
             0,                             // out buffer size
             &dwBytesTransferred,           // number of bytes returned
             NULL))                         // ignored (=NULL)
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to disable CSI!\r\n"), __WFUNCTION__));
    }

    BSPDisableCamera();

    CSI_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: CiConfigureSensor
//
// This function configures the camera sensor and preprocessing module.
//
// Parameters:
//      sensorConfig
//          [in] pCsiSensorConfig_t structure describing how
//          to configure the sensor
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CsiClass::CsiConfigureSensor(csiSensorOutputFormat outFormat, csiSensorOutputResolution outResolution)
{
    static BOOL isCameraInitialized = FALSE;
    UINT32 sensClkDivider;
    
    CSI_FUNCTION_ENTRY();

    // If CSI not enabled, temporarily enable it
    if (!(EXTREG32BF(&m_pIPU->IPU_CONF, IPU_IPU_CONF_CSI_EN)))
    {
        CsiEnable();
        m_bDisableCsi = TRUE;
    }

    // Only set up camera once.
    if(!isCameraInitialized)
    {
        BSPEnableCamera();

        // Get camera divider ratio
        sensClkDivider = BSPGetSensorClockRatio();
        RETAILMSG (0,(TEXT("BSPGetSensorClockRatio: sensClkDivider: %x"),sensClkDivider));
/*
        INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
            IPU_CSI_SENS_CONF_DIV_RATIO, sensClkDivider);
*/

        INSREG32BF(&m_pIPU->CSI_SENS_CONF,
            IPU_CSI_SENS_CONF_SENS_CLK_SRC, IPU_CSI_SENS_CONF_SENS_CLK_SRC_HSP_CLK);

        INSREG32BF(&m_pIPU->CSI_SENS_CONF,
            IPU_CSI_SENS_CONF_DIV_RATIO, sensClkDivider);

        BSPResetCamera();

        BSPSetupCamera();

        isCameraInitialized = TRUE;
    }

// TODO: Reenable
/*
    // Set the output resolution for the BSP-specific camera module.
    if (!BSPCameraSetOutputResolution(outResolution)) 
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: Invalid camera sensor output resolution.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Set the output format for the BSP-specific camera module.
    if (!BSPCameraSetOutputFormat(outFormat)) 
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: Invalid camera sensor output format.\r\n"), __WFUNCTION__));
        return FALSE;
    }
*/
    // TODO: See if this works on Virtio
    // Set data format from the sensor.
    INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
        IPU_CSI_SENS_CONF_EXT_VSYNC, 
        IPU_CSI_SENS_CONF_EXT_VSYNC_EXTERNAL);

    // Set IC configuration parameters and sensor protocol based on the output format.
    switch (outFormat)
    {
        case csiSensorOutputFormat_YUV422:
            // Set data format from the sensor.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_YUV422);
        
            // Set timing and data protocol to Progressive CCIR mode.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, IPU_CSI_SENS_CONF_SENS_PRTCL, 
                IPU_CSI_SENS_CONF_SENS_PRTCL_GATED_CLOCK_MODE);
//                IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_PROGRESSIVE_MODE);

            // Set up CCIR code registers
            INSREG32BF(&m_pIPU->CSI_CCIR_CODE_3, 
                IPU_CSI_CCIR_CODE_3_CCIR_PRECOM, 0xFF0000);

            INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_1ST, 6);

            INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                IPU_CSI_CCIR_CODE_1_END_FLD0_ACTV, 4);

            INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                IPU_CSI_CCIR_CODE_1_STRT_FLD0_ACTV, 0);
/*
            INSREG32BF(&m_pIPU->CSI_OUT_FRM_CTRL, 
                IPU_CSI_OUT_FRM_CTRL_HSC, 9);

            INSREG32BF(&m_pIPU->CSI_OUT_FRM_CTRL, 
                IPU_CSI_OUT_FRM_CTRL_VSC, 0x64);
*/
            break;
        case csiSensorOutputFormat_YUV444:

            // Set data format from the sensor.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_YUV444);

            // Set timing and data protocol to Progressive CCIR mode.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, IPU_CSI_SENS_CONF_SENS_PRTCL, 
                IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_PROGRESSIVE_MODE);

            // Set up CCIR code registers
            INSREG32BF(&m_pIPU->CSI_CCIR_CODE_3, 
                IPU_CSI_CCIR_CODE_3_CCIR_PRECOM, 0xFF0000);

            INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_1ST, 6);

            INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                IPU_CSI_CCIR_CODE_1_END_FLD0_ACTV, 4);

            INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                IPU_CSI_CCIR_CODE_1_STRT_FLD0_ACTV, 0);

            INSREG32BF(&m_pIPU->CSI_OUT_FRM_CTRL, 
                IPU_CSI_OUT_FRM_CTRL_HSC, 9);

            INSREG32BF(&m_pIPU->CSI_OUT_FRM_CTRL, 
                IPU_CSI_OUT_FRM_CTRL_VSC, 0x64);

            break;
        case csiSensorOutputFormat_RGB:

            // Set data format from the sensor.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_RGB);

            // Set timing and data protocol to non-CCIR non-gated mode.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, IPU_CSI_SENS_CONF_SENS_PRTCL, 
                IPU_CSI_SENS_CONF_SENS_PRTCL_NONGATED_CLOCK_MODE);
//                IPU_CSI_SENS_CONF_SENS_PRTCL_GATED_CLOCK_MODE);

            break;
        case csiSensorOutputFormat_Bayer:

            // Set data format from the sensor.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_BAYER);

            // Set timing and data protocol to non-CCIR non-gated mode.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, IPU_CSI_SENS_CONF_SENS_PRTCL,
                IPU_CSI_SENS_CONF_SENS_PRTCL_GATED_CLOCK_MODE);

            break;
        default:

            break;
    }

    // Data width should be 8 bits per color, regardless of the format
    INSREG32BF(&m_pIPU->CSI_SENS_CONF, IPU_CSI_SENS_CONF_DATA_WIDTH, 
                IPU_CSI_SENS_CONF_DATA_WIDTH_8BIT);


    // Set up IC width and height configuration parameters and 
    // CSI sensor and actual frame size from the output resolution.
    // Note: Subtract one from frame height and width values 
    // before writing to CSI registers.
    switch (outResolution)
    {
        case csiSensorOutputResolution_UXGA:
            // Set up CSI sensor frame and actual frame for UXGA resolution.
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, 1599 ); 
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT,1199);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, 1599 );
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT,1199 );
            break;

        case csiSensorOutputResolution_SXGA:

            // Set up CSI sensor frame and actual frame for SXGA resolution.
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, 1279);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, 1023);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, 1279);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, 1023);

            break;

        case csiSensorOutputResolution_XGA:

            // Set up CSI sensor frame and actual frame for XGA resolution.
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, 1279);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, 959);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, 1279);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, 959);

            break;

        case csiSensorOutputResolution_SVGA:

            // Set up CSI sensor frame and actual frame for SXGA resolution.
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, 799);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, 599);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, 799);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, 599);

            break;

        case csiSensorOutputResolution_VGA:

            // Set up CSI sensor frame and actual frame for VGA resolution
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, 639);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, 479);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, 639);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, 479);

            break;

        case csiSensorOutputResolution_QVGA:

            // Set up CSI sensor frame and actual frame for QVGA resolution
            INSREG32(&m_pIPU->CSI_SENS_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH),  
                CSP_BITFVAL(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, 319));
            INSREG32(&m_pIPU->CSI_SENS_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT),  
                CSP_BITFVAL(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT,239 ));

            INSREG32(&m_pIPU->CSI_ACT_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH),  
                CSP_BITFVAL(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, 319));
            INSREG32(&m_pIPU->CSI_ACT_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT),  
                CSP_BITFVAL(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, 239));

            break;

        case csiSensorOutputResolution_CIF:

            // Set up CSI sensor frame and actual frame for CIF resolution
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, 351);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, 287);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, 351);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, 287);

            break;

        case csiSensorOutputResolution_QCIF:

            // Set up CSI sensor frame and actual frame for QCIF resolution
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, 175);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE,
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, 143);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, 175);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, 143);

            break;

        case csiSensorOutputResolution_QQVGA:

            // Set up CSI sensor frame and actual frame for QQVGA resolution
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, 159);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, 119);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, 159);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, 119);

            break;

        default:
            break;
    }

    // Disable the CSI, which had been temporarily enabled.
    if (m_bDisableCsi)
    {
        CsiDisable();
        m_bDisableCsi = FALSE;
    }

    CSI_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CsiZoom
//
// This function sets the camera zoom value.
//
// Parameters:
//      zoomVal
//          [in] zoom value.  
//                If 2, zoom by 2x.  If 1, zoom by 1x.  
//                All other values are invalid.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiZoom(DWORD zoomVal)
{
    CSI_FUNCTION_ENTRY();

    // If CSI not enabled, temporarily enable it
    if (!(EXTREG32BF(&m_pIPU->IPU_CONF, IPU_IPU_CONF_CSI_EN)))
    {
        CsiEnable();
        m_bDisableCsi = TRUE;
    }

    BSPSetDigitalZoom(zoomVal);

    // Disable the CSI, which had been temporarily enabled.
    if (m_bDisableCsi)
    {
        CsiDisable();
        m_bDisableCsi = FALSE;
    }

    CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: CsiDetectEOF
//
// This function Detect CSI_EOF bit of IPU_INT_STAT_3 register.
//
// Parameters:
//      zoomVal
//          None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiDetectEOF(void)
{

    INT32 uTempReg, uTempReg1, uCount=0;

    uTempReg1 = (INREG32(&m_pIPU->IPU_INT_STAT_3)) & 0x40;
    RETAILMSG (0,(TEXT("%s: uTempReg1:%x .\r\n"), __WFUNCTION__,uTempReg1));

    OUTREG32(&m_pIPU->IPU_INT_STAT_3, 0x40);

    uTempReg = (INREG32(&m_pIPU->IPU_INT_STAT_3)) & 0x40;


    
    while (uTempReg == 0)
    {
        if (uCount <= 100)
        {
            //..give up the remainder of time slice
            Sleep(1);
            uCount++;

            //.. need to check after the sleep delay
            uTempReg = (INREG32(&m_pIPU->IPU_INT_STAT_3)) & 0x40;
            RETAILMSG (0,(TEXT("%s: uTempReg:%x , uCount: %x . \r\n"), __WFUNCTION__,uTempReg, uCount));
        }
        else
        {
            //.. there is something wrong ....break out
            RETAILMSG(1,(TEXT("%s: CSI_EOF always is 1. \r\n"), __WFUNCTION__));
            break;
        }
    }

}


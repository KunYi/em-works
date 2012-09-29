//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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
#pragma warning(disable: 4100 4189 4101) 
#include <windows.h>
#include <Devload.h>
#include <NKIntr.h>
#include <ceddk.h>

#include "common_ipu.h"
#include "common_macros.h"
#include "cameradbg.h"
#include "ipu.h"
#include "IpuModuleInterfaceClass.h"
#include "CsiClass.h"


//------------------------------------------------------------------------------
// External Functions
extern int BSPGetDefaultCameraFromRegistry();
extern BOOL BSPCSIIOMUXConfig();
extern void BSPSetupCamera();
extern void BSPDeleteCamera();
extern void BSPEnableCamera();
extern void BSPDisableCamera(BOOL powerstate);
extern void BSPResetCamera();
extern UINT32 BSPGetSensorClockRatio();
extern void BSPSetDigitalZoom(DWORD);
extern BOOL BSPCameraSetOutputResolution(csiSensorOutputResolution);
extern BOOL BSPCameraSetOutputFormat(csiSensorOutputFormat);
extern void BSPGetSensorFormat(DWORD * pSensorFormat);
extern void BSPGetSensorResolution(DWORD * pSensorResolution);
    
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
    
    m_bEncCsiEnable = FALSE;
    m_bVfCsiEnable = FALSE;
    m_bDisableCsi = FALSE;
    m_iCamType = -1;

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
        ERRORMSG (TRUE,
            (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        return;
    }

    // Request IOMUX pins
    
    // Configure IOMUX for I2C pins
    if (!BSPCSIIOMUXConfig())
    {
        ERRORMSG (TRUE, 
            (TEXT("%s: Error configuring IOMUX for CSI.\r\n"), __WFUNCTION__));
        return;
    }

    // Get camera-in-use from registry key. Otherwise use default
    m_iCamType = BSPGetDefaultCameraFromRegistry();
    if ( m_iCamType == -1)
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
            CSI_FUNCTION_EXIT();
            return;
        }
    }
    else if (channel == IC_CHANNEL_VF)
    {
        m_bVfCsiEnable = TRUE;

        // If ENC has already enabled CSI, we are done
        if (m_bEncCsiEnable)
        {
            CSI_FUNCTION_EXIT();
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
        ERRORMSG (TRUE,
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
            CSI_FUNCTION_EXIT();
            return;
        }
    }
    else if (channel == IC_CHANNEL_VF)
    {
        m_bVfCsiEnable = FALSE;

        // If ENC has already enabled CSI, we are done
        if (m_bEncCsiEnable)
        {
            CSI_FUNCTION_EXIT();
            return;
        }
    }

    // For power manager:if disable CSI not detect CSI_EOF = 1,
    // it may be lead OS crash when operating suspend/resume.
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
        ERRORMSG (TRUE,
            (TEXT("%s: Failed to disable CSI!\r\n"), __WFUNCTION__));
    }

    CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: CsiConfigure
//
// This function configures and preprocessing module.
//
// Parameters:
//      outFormat  
//          [in] Sensor output format
//      outResolution
//          [in] Sensor output resolution:VGA,QVGA,QQVGA,CIF,QIF
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CsiClass::CsiConfigure(csiSensorOutputFormat* outFormat, csiSensorOutputResolution* outResolution)
{  
    DWORD sensorFormat, sensorResolution;
    
    CSI_FUNCTION_ENTRY();

    // If CSI not enabled, temporarily enable it
    if (!(EXTREG32BF(&m_pIPU->IPU_CONF, IPU_IPU_CONF_CSI_EN)))
    {
        CsiEnable();
        m_bDisableCsi = TRUE;
    }

    #if CSI_TEST_MODE
        // Turn on test pattern generation from the CSI
        INSREG32BF(&m_pIPU->CSI_TST_CTRL, 
            IPU_CSI_TST_CTRL_TEST_GEN_MODE, 
            IPU_CSI_TST_CTRL_TEST_GEN_MODE_ACTIVE);
        
        // Set test pattern in CSI
        INSREG32BF(&m_pIPU->CSI_TST_CTRL, 
            IPU_CSI_TST_CTRL_PG_R_VALUE, 
            0x00);
        INSREG32BF(&m_pIPU->CSI_TST_CTRL, 
            IPU_CSI_TST_CTRL_PG_G_VALUE, 
            0xff);
        INSREG32BF(&m_pIPU->CSI_TST_CTRL, 
            IPU_CSI_TST_CTRL_PG_B_VALUE, 
            0xFF);
    #else
        INSREG32BF(&m_pIPU->CSI_TST_CTRL, 
            IPU_CSI_TST_CTRL_TEST_GEN_MODE, 
            IPU_CSI_TST_CTRL_TEST_GEN_MODE_INACTIVE);
    #endif

    // Set VSYNC
    switch (m_iCamType)
    {
        case 4://csiTVinId_ADV7180   
             //! Not used, ADV7180 has a dedicated clock: 14318180; 
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_EXT_VSYNC, 
                IPU_CSI_SENS_CONF_EXT_VSYNC_INTERNAL);
            break;
             
        case 3://csiSensorId_OV2640
        default:
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_EXT_VSYNC, 
                IPU_CSI_SENS_CONF_EXT_VSYNC_EXTERNAL);
        
            break;
    }

    // TODO: See if this works on Virtio
    // Set data format from the sensor.
    
    // Get senor output format
    #if CSI_TEST_MODE
    *outFormat = csiSensorOutputFormat_RGB;
    #else
    BSPGetSensorFormat(&sensorFormat);
    *outFormat = (csiSensorOutputFormat) sensorFormat;     
    #endif
    // Set IC configuration parameters and sensor protocol based on the output format.
    switch (*outFormat)
    {
        case csiSensorOutputFormat_YUV422:
            // Set data format from the sensor.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_YUV422);

            switch (m_iCamType)
            {
                case 4://csiTVinId_ADV7180
                    // Set timing and data protocol to Progressive CCIR mode.
                    INSREG32BF(&m_pIPU->CSI_SENS_CONF, IPU_CSI_SENS_CONF_SENS_PRTCL, 
                               IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_PROGRESSIVE_MODE);

                    // if CONF_SENS_PRTCL is GATED_MODE,not use following config
                    // Set up CCIR code registers
                    INSREG32BF(&m_pIPU->CSI_CCIR_CODE_3, 
                        IPU_CSI_CCIR_CODE_3_CCIR_PRECOM, 0xFF0000);
                    
                    INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_END_FLD0_BLNK_1ST, 0x0);//0x6);//

                    INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_1ST, 0x6);//0x2);//

                    INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_END_FLD0_BLNK_2ND, 0x0);//0x6);//

                    INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_2ND, 0x0);//0x2);//

                    INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_END_FLD0_ACTV, 0x4);            

                    INSREG32BF(&m_pIPU->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_STRT_FLD0_ACTV, 0x0);     
                    break;
                    
                case 3://csiSensorId_OV2640
                    default:
                    // Set timing and data protocol to Progressive CCIR mode.
                    INSREG32BF(&m_pIPU->CSI_SENS_CONF, IPU_CSI_SENS_CONF_SENS_PRTCL, 
                              IPU_CSI_SENS_CONF_SENS_PRTCL_GATED_CLOCK_MODE);
                    break;
            }
            
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_VSYNC_POL, 0x0);
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_HSYNC_POL, 0x0);
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DATA_POL, 0x0);
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DISP_PIX_CLK_POL, 0x0);

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

            #if CSI_TEST_MODE
            // Set timing and data protocol to non-CCIR non-gated mode.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, IPU_CSI_SENS_CONF_SENS_PRTCL, 
                IPU_CSI_SENS_CONF_SENS_PRTCL_NONGATED_CLOCK_MODE);
            #else
            // Set timing and data protocol to non-CCIR gated mode.
            INSREG32BF(&m_pIPU->CSI_SENS_CONF, IPU_CSI_SENS_CONF_SENS_PRTCL, 
                IPU_CSI_SENS_CONF_SENS_PRTCL_GATED_CLOCK_MODE);
            #endif

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
    // Get sensor output resolution
    BSPGetSensorResolution(&sensorResolution);
    *outResolution = (csiSensorOutputResolution) sensorResolution;
    switch (*outResolution)
    {
        case csiSensorOutputResolution_UXGA:
            // Set up CSI sensor frame and actual frame for UXGA resolution.
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, UXGA_Width - 1); 
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, UXGA_Height - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, UXGA_Width - 1 );
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, UXGA_Height - 1 );
            break;

        case csiSensorOutputResolution_SXGA:

            // Set up CSI sensor frame and actual frame for SXGA resolution.
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, SXGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, SXGA_Height - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, SXGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, SXGA_Height - 1);

            break;

        case csiSensorOutputResolution_XGA:

            // Set up CSI sensor frame and actual frame for XGA resolution.
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, XGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, XGA_Height - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, XGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, XGA_Height - 1);

            break;

        case csiSensorOutputResolution_SVGA:

            // Set up CSI sensor frame and actual frame for SVGA resolution.
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, SVGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, SVGA_Height - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, SVGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, SVGA_Height - 1);

            break;

        case csiSensorOutputResolution_VGA:

            // Set up CSI sensor frame and actual frame for VGA resolution
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, VGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, VGA_Height - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, VGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, VGA_Height - 1);

            break;

        case csiSensorOutputResolution_QVGA:

            // Set up CSI sensor frame and actual frame for QVGA resolution
            INSREG32(&m_pIPU->CSI_SENS_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH),  
                CSP_BITFVAL(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, QVGA_Width - 1));
            INSREG32(&m_pIPU->CSI_SENS_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT),  
                CSP_BITFVAL(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, QVGA_Height - 1));

            INSREG32(&m_pIPU->CSI_ACT_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH),  
                CSP_BITFVAL(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, QVGA_Width - 1));
            INSREG32(&m_pIPU->CSI_ACT_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT),  
                CSP_BITFVAL(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, QVGA_Height - 1));

            break;

        case csiSensorOutputResolution_CIF:

            // Set up CSI sensor frame and actual frame for CIF resolution
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, CIF_Width - 1);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, CIF_Height - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, CIF_Width - 1);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, CIF_Height - 1);

            break;

        case csiSensorOutputResolution_QCIF:

            // Set up CSI sensor frame and actual frame for QCIF resolution
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, QCIF_Width - 1);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE,
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, QCIF_Height - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, QCIF_Width - 1);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, QCIF_Height - 1);

            break;

        case csiSensorOutputResolution_QQVGA:

            // Set up CSI sensor frame and actual frame for QQVGA resolution
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, QQVGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, QQVGA_Height - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, QQVGA_Width - 1);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, QQVGA_Height - 1);

            break;
            
        // only For Ringo TVIN +:only for tvin used
        case csiSensorOutputResolution_PAL:
            
            // Set up CSI sensor frame and actual frame for 720x576 resolution
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, PAL_Width - 1);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, PAL_Height/2 + NBL_PAL - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, PAL_Width -1);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, PAL_Height/2  - 1);

            //If first run NTSC format, it will set the register
            INSREG32BF(&m_pIPU->CSI_OUT_FRM_CTRL,
                IPU_CSI_OUT_FRM_CTRL_VSC, 0);

            break;

        // only For Ringo TVIN +:only for tvin used
        case csiSensorOutputResolution_NTSC:
            
             // Set up CSI sensor frame and actual frame for 720x480 resolution
             // the NTSC_Height not set to 480,because it need remove the white line in front of each field
             // and the value is assured the input prp size is bigger than prp output size
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, NTSC_Width - 1);
            INSREG32BF(&m_pIPU->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, NTSC_Height/2 + NBL_NTSC - 1);

            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, NTSC_Width - 1);
            INSREG32BF(&m_pIPU->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, NTSC_Height/2 - 1);

            INSREG32BF(&m_pIPU->CSI_OUT_FRM_CTRL,
                IPU_CSI_OUT_FRM_CTRL_VSC, SKIP_NTSC);
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
// Function: CiConfigureSensor
//
// This function configures the camera sensor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CsiClass::CsiConfigureSensor()
{   
    UINT32 sensClkDivider;
    CSI_FUNCTION_ENTRY();

    // Enable Sensor/TVin power
    BSPEnableCamera();

    // Set Clock source
    switch (m_iCamType)
    {
        case 4://csiTVinId_ADV7180   
             //! Not used, ADV7180 has a dedicated clock: 14318180; 
            break;
             
        case 3://csiSensorId_OV2640
        default:
            // Set Sensor Clk source
            #if CSI_SENSB_SENS_CLK
                INSREG32BF(&m_pIPU->CSI_SENS_CONF,
                    IPU_CSI_SENS_CONF_SENS_CLK_SRC, IPU_CSI_SENS_CONF_SENS_CLK_SRC_IPP_IND_SENSB_SENS_CLK);
            #else//HSP_CLK division
                // Get camera divider ratio for HSP_CLK
                sensClkDivider = BSPGetSensorClockRatio();
                
                INSREG32BF(&m_pIPU->CSI_SENS_CONF,
                    IPU_CSI_SENS_CONF_SENS_CLK_SRC, IPU_CSI_SENS_CONF_SENS_CLK_SRC_HSP_CLK);
            
                INSREG32BF(&m_pIPU->CSI_SENS_CONF,
                    IPU_CSI_SENS_CONF_DIV_RATIO, sensClkDivider );
            #endif
        
            break;
    }

    // Reset Sensor/TVin
    BSPResetCamera();

    // Set Sensor/Tvin Registers 
    BSPSetupCamera();

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
// This function check the bit of CSI_EOF in status register.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE for detected EOF,FALSE for no EOF.
//
//---------------------------------------------------------
BOOL CsiClass::CsiDetectEOF(void)
{
    INT32 uTempReg,uCount=0;

    // Clear Interrupt Status Bits:CSI_EOF_EN
    INT32 uTempReg1 = (INREG32(&m_pIPU->IPU_INT_STAT_3)) & 0x40;
    RETAILMSG(0,(TEXT("%s: CSI_EOF_EN->uTempReg1:%x .\r\n"), __WFUNCTION__,uTempReg1));

    OUTREG32(&m_pIPU->IPU_INT_STAT_3, 0x40);

    // Check the CSI_EOF
    uCount = 0;
    uTempReg = (INREG32(&m_pIPU->IPU_INT_STAT_3)) & 0x40;
    RETAILMSG(0,(TEXT("%s: uTempReg:%x .\r\n"), __WFUNCTION__,uTempReg));
    
    while (uTempReg == 0)
    {
        if (uCount < 100)
        {
            //..give up the remainder of time slice
            Sleep(1);
            uCount++;

            //.. need to check after the sleep delay
            uTempReg = (INREG32(&m_pIPU->IPU_INT_STAT_3)) & 0x40;
            RETAILMSG(0,(TEXT("%s: uTempReg:%x , uCount: %d . \r\n"), __WFUNCTION__,uTempReg & 0x40, uCount));
        }
        else
        {
            //.. there is something wrong ....break out
            RETAILMSG(1,(TEXT("%s: CSI_EOF always is 0. \r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    return TRUE;

}


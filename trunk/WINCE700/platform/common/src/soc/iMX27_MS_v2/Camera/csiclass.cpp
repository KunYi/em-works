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
//  Copyright (C) 2003, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  csi.cpp
//
//  Implementation of CMOS Sensor Interface Product Device Driver
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <Devload.h>
#include <NKIntr.h>
#include <ceddk.h>
#include "csp.h"

#include "CameraDebug.h"
#include "CsiClass.h"

//------------------------------------------------------------------------------
// External Functions

extern BOOL BSPGetDefaultCameraFromRegistry();
extern BOOL BSPCSIGPIOConfig(BOOL);
extern void BSPSetupCamera();
extern void BSPDeleteCamera();
extern void BSPEnableCamera();
extern void BSPDisableCamera();
extern void BSPResetCamera();
extern UINT32 BSPGetSensorClockRatio();
extern void BSPSetDigitalZoom(BOOL);
extern BOOL BSPCameraSetOutputResolution(csiSensorOutputResolution);
extern BOOL BSPCameraSetOutputFormat(csiSensorOutputFormat);
extern BOOL BSPCameraIsTVin();

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
    m_bIsCsiEnabled = FALSE;

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
    PHYSICAL_ADDRESS phyAddr;

    phyAddr.QuadPart = CSP_BASE_REG_PA_CSI;

    // Map peripheral physical address to virtual address
    m_pCSI = (PCSP_CSI_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_CSI_REGS), FALSE); 
        
    // Check if virtual mapping failed
    if (m_pCSI == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("BSPCSIGPIOConfig:  MmMapIoSpace failed!\r\n")));
        goto Error;
    }

    if (!BSPCSIGPIOConfig(TRUE))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error configuring GPIO for CSI.\r\n"), __WFUNCTION__));
        return;
    }

    // Get camera-in-use from registry key. Otherwise use default
    if (!BSPGetDefaultCameraFromRegistry())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Get camera from registry failed!\r\n"), __WFUNCTION__));
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

    if (!BSPCSIGPIOConfig(FALSE))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error configuring GPIO for CSI.\r\n"), __WFUNCTION__));
        return;
    }

    CsiDisable();

    if (m_pCSI)
    {
        MmUnmapIoSpace(m_pCSI, sizeof(CSP_CSI_REGS));
        m_pCSI = NULL;
    }

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
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiEnable(void)
{
    CSI_FUNCTION_ENTRY();

    if (TRUE == m_bIsCsiEnabled)
        return;

    BSPEnableCamera();

    // Set the bit to enable the CSI.
    INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_CSI_SVR, CSI_CSICR3_CSI_SVR_SVRMODE);
    INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_MCLKEN, CSI_CSICR1_MCLKEN_ENABLE);

    //Here we should restore all the register setting.
    
    m_bIsCsiEnabled = TRUE;

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
    CSI_FUNCTION_ENTRY();

    if (FALSE == m_bIsCsiEnabled)
        return;

    INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_CSI_SVR, CSI_CSICR3_CSI_SVR_ANYMODE);
    INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_MCLKEN, CSI_CSICR1_MCLKEN_DISABLE);
  
    BSPDisableCamera();

    //Here we should save all the registers setting. 
    
    m_bIsCsiEnabled = FALSE;
   
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
//    UINT32 sensClkDivider;
    
    CSI_FUNCTION_ENTRY();

    // Only set up camera once.
    if(!isCameraInitialized)
    {
      // Get camera divider ratio
      // sensClkDivider = BSPGetSensorClockRatio();

      // Configure CSI interface.
      CsiEnable();  

      m_pCSI->CSICR1 |= CSP_BITFVAL(CSI_CSICR1_REDGE,CSI_CSICR1_REDGE_RISING) |
                     CSP_BITFVAL(CSI_CSICR1_GCLK_MODE,CSI_CSICR1_GCLK_MODE_GATED) |
                     CSP_BITFVAL(CSI_CSICR1_PACK_DIR, CSI_CSICR1_PACK_DIR_MSB) | 
                     CSP_BITFVAL(CSI_CSICR1_FCC, CSI_CSICR1_FCC_SYNC) |                     
                     CSP_BITFVAL(CSI_CSICR1_HSYNC_POL,CSI_CSICR1_HSYNC_POL_HIGN) |
                     CSP_BITFVAL(CSI_CSICR1_MCLKDIV,CSI_CSICR1_MCLKDIV_VALUE(4)) |
                     CSP_BITFVAL(CSI_CSICR1_SOF_POL,CSI_CSICR1_SOF_POL_RISING) |    
                     CSP_BITFVAL(CSI_CSICR1_RXFF_LEVEL,CSI_CSICR1_RXFF_LEVEL_16WORDS) | 
                     CSP_BITFVAL(CSI_CSICR1_PRP_IF_EN,CSI_CSICR1_PRP_IF_EN_ENABLE) |
                     CSP_BITFVAL(CSI_CSICR1_EXT_VSYNC,CSI_CSICR1_EXT_VSYNC_EXTERNAL);

      if (TRUE == BSPCameraIsTVin())
      {
        m_pCSI->CSICR1 |= CSP_BITFVAL(CSI_CSICR1_CCIR_EN,CSI_CSICR1_CCIR_EN_CCIR) |
                       CSP_BITFVAL(CSI_CSICR1_CCIR_MODE,CSI_CSICR1_CCIR_MODE_INTERLACE) ;
      }
      else
      {
        m_pCSI->CSICR1 |= CSP_BITFVAL(CSI_CSICR1_CCIR_EN,CSI_CSICR1_CCIR_EN_TRADITIONAL) |
                       CSP_BITFVAL(CSI_CSICR1_SOF_INTEN,CSI_CSICR1_SOF_INTEN_ENABLE) |
                       CSP_BITFVAL(CSI_CSICR1_RXFF_INTEN,CSI_CSICR1_RXFF_INTEN_ENABLE) |
                       CSP_BITFVAL(CSI_CSICR1_RF_OR_INTEN,CSI_CSICR1_RF_OR_INTEN_ENABLE) |
                       CSP_BITFVAL(CSI_CSICR1_CCIR_MODE,CSI_CSICR1_CCIR_MODE_PROGRESSIVE) ;
      }

      // Reset CMOS Sensor.
      BSPResetCamera();

      // Configure CMOS Sensor.CMOS sensor only be configured once.
      BSPSetupCamera();

      isCameraInitialized = TRUE;
    }


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

    // Set IC configuration parameters and sensor protocol based on the output format.
    switch (outFormat)
    {
        case csiSensorOutputFormat_YUV422:
            m_pCSI->CSICR1 |= CSP_BITFMASK(CSI_CSICR1_PACK_DIR);
            break;

        case csiSensorOutputFormat_YUV444:
            break;

        case csiSensorOutputFormat_RGB:
            m_pCSI->CSICR1 &= ~ CSP_BITFVAL(CSI_CSICR1_CCIR_EN, CSI_CSICR1_CCIR_EN_TRADITIONAL);
            // Disable this bit for RGB, Half Byte swap in PrP config is removed
            m_pCSI->CSICR1 &= ~ CSP_BITFMASK(CSI_CSICR1_PACK_DIR);
            break;

        case csiSensorOutputFormat_Bayer:
            break;

        default:
            break;            
    }

    // Set up IC width and height configuration parameters and 
    // CSI sensor and actual frame size from the output resolution.
    // Note: Subtract one from frame height and width values 
    // before writing to CSI registers.
    switch (outResolution)
    {
        case csiSensorOutputResolution_SXGA:
            break;

        case csiSensorOutputResolution_VGA:
            INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_RXFF_LEVEL, CSI_CSICR1_RXFF_LEVEL_16WORDS);
            break;

        case csiSensorOutputResolution_QVGA:
            INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_RXFF_LEVEL, CSI_CSICR1_RXFF_LEVEL_16WORDS);
            break;

        case csiSensorOutputResolution_CIF:
            INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_RXFF_LEVEL, CSI_CSICR1_RXFF_LEVEL_16WORDS);
            break;

        case csiSensorOutputResolution_QCIF:
            INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_RXFF_LEVEL, CSI_CSICR1_RXFF_LEVEL_8WORDS);
            break;

        case csiSensorOutputResolution_QQVGA:
            INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_RXFF_LEVEL, CSI_CSICR1_RXFF_LEVEL_16WORDS);
            break;

        case csiSensorOutputResolution_PAL:
            INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_RXFF_LEVEL, CSI_CSICR1_RXFF_LEVEL_8WORDS);
            break;

        case csiSensorOutputResolution_NTSC:
            INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_RXFF_LEVEL, CSI_CSICR1_RXFF_LEVEL_8WORDS);
            break;
    
        default:
            break;
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

    if (zoomVal == 2)
        BSPSetDigitalZoom(TRUE);
    else if (zoomVal == 1)
        BSPSetDigitalZoom(FALSE);
    else
        DEBUGMSG(ZONE_ERROR, (_T("%s: Invalid zoom value.  Must be 1 or 2.\r\n"), __WFUNCTION__));

    CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: CsiChangeFrameRate
//
// Change Frame Rate via change Camera Sensor Interface Clock.
//
// Parameters:
//      rate. if 15, Rate = 15; 30, Rate=30
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiChangeFrameRate(DWORD rate)
{
    CSI_FUNCTION_ENTRY();

    // Disable the module first.
    INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_CSI_SVR, CSI_CSICR3_CSI_SVR_ANYMODE);
    INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_MCLKEN, CSI_CSICR1_MCLKEN_DISABLE);

    if ( rate == 30 )
    {
         m_pCSI->CSICR1 &= ~CSP_BITFMASK(CSI_CSICR1_MCLKDIV);
         m_pCSI->CSICR1 |= CSP_BITFVAL(CSI_CSICR1_MCLKDIV,CSI_CSICR1_MCLKDIV_VALUE(2));  
    }else{
         m_pCSI->CSICR1 &= ~CSP_BITFMASK(CSI_CSICR1_MCLKDIV);
         m_pCSI->CSICR1 |= CSP_BITFVAL(CSI_CSICR1_MCLKDIV,CSI_CSICR1_MCLKDIV_VALUE(4));  
    }

    // Enable the moudle again.
    INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_CSI_SVR, CSI_CSICR3_CSI_SVR_SVRMODE);
    INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_MCLKEN, CSI_CSICR1_MCLKEN_ENABLE);

    CSI_FUNCTION_EXIT();
}

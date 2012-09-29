//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//

//  File:  bspcsi.c
//
//  Provides BSP-specific configuration routines for the CSI peripheral.
//
//------------------------------------------------------------------------------
#include <windows.h>
#pragma warning(push)
#pragma warning(disable: 4005 4702)
#include "bsp.h"
#include "cameradbg.h"
#pragma warning(pop)
#include "CameraPDDProps.h"
#include "CamBufferManager.h"
#include "bspcsi.h"
#include "CsiClass.h"
#include "CameraOV2640.h"
#include "i2cbus.h"

#include <ceddk.h>
#include <devload.h>
#include <NKIntr.h>

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines
#define BSP_CSI_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define BSP_CSI_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#define CAMERA_SENSOR_MODULE    csiSensorId_OV2640 // OmniVision as default Sensor
 
//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

// Camera in Used - Default iMagic
csiSensorId gSensorInUse = CAMERA_SENSOR_MODULE;

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function:  BSPCSIIOMUXConfig
//
// This function makes the DDK call to configure the IOMUX
// pins required for the CSI.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL CsiClass::BSPCSIIOMUXConfig(void)
{
    BSP_CSI_FUNCTION_ENTRY();

    BOOL retVal = TRUE;

    // CMOS_RST pin 
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_A20,        DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_PWDN pin 
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_A19,        DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_D2
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D2,     DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_D3
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D3,     DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_D4
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D4,     DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_D5
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D5,     DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_D6
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D6,     DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_D7
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D7,     DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_D8
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D8,     DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_D9
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D9,     DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_MCLK
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_MCLK,   DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_VSYNC
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_VSYNC,  DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_HSYNC
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_HSYNC,  DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }
    // CSI_PIXCLK
    if(!DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_PIXCLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR))
    {
        retVal = FALSE;
    }

    // Configure Pads
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D2,      DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D3,      DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D4,      DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D5,      DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D6,      DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D7,      DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D8,      DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D9,      DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_HSYNC,   DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_MCLK,    DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_PIXCLK,  DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_VSYNC,   DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_A20, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_1V8);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_A19, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_1V8);


    // Configure Outputs
    DDKGpioSetConfig(CAMERA_RESET_GPIO_PORT,CAMERA_RESET_GPIO_PIN,DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioSetConfig(CAMERA_ENABLE_GPIO_PORT,CAMERA_ENABLE_GPIO_PIN,DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);

    BSP_CSI_FUNCTION_EXIT();

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: BSPSetupCamera
//
// This function initializes the camera sensor module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CsiClass::BSPSetupCamera(DWORD dwFramerate)
{
    BSP_CSI_FUNCTION_ENTRY();

    switch (gSensorInUse)
    {
        case csiSensorId_OV2640:
        default:
             DEBUGMSG(ZONE_INIT, (TEXT("setupCamera: OmniVision 2640 sensor initialization\r\n")));
             CameraOV2640Init((SensorOutputFrameRate)dwFramerate);        
             break;        
    }

    BSP_CSI_FUNCTION_EXIT();

    return;
}

//------------------------------------------------------------------------------
//
// Function: BSPDeleteCamera
//
// This function deinitializes the camera sensor module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CsiClass::BSPDeleteCamera(void)
{
    BSP_CSI_FUNCTION_ENTRY();

    switch (gSensorInUse)
    {
        case csiSensorId_OV2640:
        default:
             CameraOV2640Deinit();
             DEBUGMSG(ZONE_INIT,(TEXT("DeleteCamera: OmniVision sensor deinitialization\r\n")));
             break;     
    }

    BSP_CSI_FUNCTION_EXIT();

    return;
}


//------------------------------------------------------------------------------
//
// Function: BSPEnableCamera
//
// This function enable the camera sensor module(set camera power).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CsiClass::BSPEnableCamera(void)
{
    BSP_CSI_FUNCTION_ENTRY();

    switch(gSensorInUse)
    {
        case csiSensorId_OV2640:
            {
                //Power-Down disable
                DDKGpioWriteDataPin(CAMERA_ENABLE_GPIO_PORT, CAMERA_ENABLE_GPIO_PIN, 0);
                Sleep(100);
            }
            break;
    }

    BSP_CSI_FUNCTION_EXIT();

    return;
}

//------------------------------------------------------------------------------
//
// Function: BSPDisableCamera
//
// This function Disable the camera sensor module,turn on/off module clock.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CsiClass::BSPDisableCamera(void)
{
    BSP_CSI_FUNCTION_ENTRY();

    switch(gSensorInUse)
    {
        case csiSensorId_OV2640:
        default:
            {
                //Power-Down enable
                DDKGpioWriteDataPin(CAMERA_ENABLE_GPIO_PORT, CAMERA_ENABLE_GPIO_PIN, 1);
                Sleep(100);
            }
            break;
    }

    BSP_CSI_FUNCTION_EXIT();

    return;
}

//------------------------------------------------------------------------------
//
// Function: BSPResetCamera
//
// This function reset the camera sensor module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CsiClass::BSPResetCamera(void)
{
    BSP_CSI_FUNCTION_ENTRY();

    //Reset camera (CMOS_RST)
    DDKGpioWriteDataPin(CAMERA_RESET_GPIO_PORT, CAMERA_RESET_GPIO_PIN, 0);
    Sleep(100);
    DDKGpioWriteDataPin(CAMERA_RESET_GPIO_PORT, CAMERA_RESET_GPIO_PIN, 1);
    Sleep(100);

    BSP_CSI_FUNCTION_EXIT();
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: BSPGetDefaultCameraFromRegistry
//
// This function reads the default camera sensor from the
// registry.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//------------------------------------------------------------------------------
BOOL CsiClass::BSPGetDefaultCameraFromRegistry(void)
{
    UINT32 error;
    HKEY hKey;
    UINT32 dwSize;
    csiSensorId sensorId;

    BSP_CSI_FUNCTION_ENTRY();

    // Open CSI registry path
    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT(CSI_REG_PATH), 0 , 0, &hKey);
    if (error != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("Failed to open camera reg path:%s [Error:0x%x]\r\n"),CSI_REG_PATH,error));
        //return (FALSE);
        return -1;
    }

    // Get Default Camera
    dwSize = sizeof(csiSensorId);
    error = RegQueryValueEx(hKey, TEXT(CSI_REG_CAMERAID_KEYWORD), NULL, NULL,(LPBYTE)&sensorId, (LPDWORD)&dwSize);
    if (error == ERROR_SUCCESS)
    {
        // Make sure it's valid CameraID
        if(sensorId >= 0 && sensorId < CSI_SENSOR_NUMBER_SUPPORTED)
        {
            gSensorInUse = sensorId;
        }
        else
        {
            DEBUGMSG(ZONE_ERROR,(TEXT("Invalid Camera ID, set to default:%d\r\n"),CAMERA_SENSOR_MODULE));
        }
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Failed to get the default CameraID [Error:0x%x]\r\n"),error));
    }

    // Close registry key
    RegCloseKey(hKey);

    BSP_CSI_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: BSPSetDigitalZoom
//
// This function sets the zoom value on the camera sensor.
//
// Parameters:
//      zoom
//          [in] If 2, zoom set to 2x; if 1, zoom set to 1x
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CsiClass::BSPSetDigitalZoom (DWORD zoom)
{
    BSP_CSI_FUNCTION_ENTRY();

    switch (gSensorInUse)
    {
      case csiSensorId_OV2640:
      default:
           CameraOV2640SetDigitalZoom(zoom);       
           break;
    }        
    BSP_CSI_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: BSPCameraSetOutputResolution
//
// This function sets the output resolution on the camera sensor.
//
// Parameters:
//      outputMode
//          [in] Resolution type.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//------------------------------------------------------------------------------
BOOL CsiClass::BSPCameraSetOutputResolution(DWORD dwFramerate,csiSensorOutputResolution outputResolution)
{
    BSP_CSI_FUNCTION_ENTRY();

    switch (gSensorInUse)
    {
      case csiSensorId_OV2640:
      default:
           CameraOV2640SetOutputResolution(dwFramerate,outputResolution);
           break;
    }        

    BSP_CSI_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: BSPCameraSetOutputFormat
//
// This function sets the output format on the camera sensor.
//
// Parameters:
//      outputFormat
//          [in] Format specified to output sensor data.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//------------------------------------------------------------------------------
BOOL CsiClass::BSPCameraSetOutputFormat(csiSensorOutputFormat outputFormat)
{
    cameraOutputFormat cameraOutputFormat;
    
    BSP_CSI_FUNCTION_ENTRY();

    switch (outputFormat)
    {
      case csiSensorOutputFormat_Bayer:
           return FALSE;

      case csiSensorOutputFormat_YUV422_UYVY:
           DEBUGMSG(ZONE_FUNCTION,(TEXT("Set sensor output format is csiSensorOutputFormat_YUV422_UYVY\r\n")));
           cameraOutputFormat = cameraOutputFormat_YUV422_UYVY;
           break;

      case csiSensorOutputFormat_YUV422_YUY2:
           DEBUGMSG(ZONE_FUNCTION,(TEXT("Set sensor output format is csiSensorOutputFormat_YUV422_YUY2\r\n")));
           cameraOutputFormat = cameraOutputFormat_YUV422_YUY2;
           break;

      case csiSensorOutputFormat_YUV420:
           DEBUGMSG(ZONE_FUNCTION,(TEXT("Set sensor output format is cameraOutputFormat_YUV420\r\n")));
           cameraOutputFormat = cameraOutputFormat_YUV420;
           break;

      case csiSensorOutputFormat_RGB555:
           DEBUGMSG(ZONE_FUNCTION,(TEXT("Set sensor output format is cameraOutputFormat_RGB555\r\n")));
           cameraOutputFormat = cameraOutputFormat_RGB555;
           break;
           
      case csiSensorOutputFormat_RGB565:
      default:
           DEBUGMSG(ZONE_FUNCTION,(TEXT("Set sensor output format is cameraOutputFormat_RGB565\r\n")));
           cameraOutputFormat = cameraOutputFormat_RGB565;
           break;
    }

    switch (gSensorInUse)
    {
      case csiSensorId_OV2640:
      default:
           CameraOV2640SetOutputFormat(cameraOutputFormat);
           break;
    }        
    
    BSP_CSI_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: BSPSensorSetClockGatingMode
//
// This function calls to the CRM module to
// set the clock gating mode, turning on or off
// clocks to the Camera Sesnor.
//
// Parameters:
//      startClocks
//          [in] If TRUE, turn clocks to GPT on.
//                If FALSE, turn clocks to GPT off
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL CsiClass::BSPSensorSetClockGatingMode(BOOL startClocks)
{
    BSP_CSI_FUNCTION_ENTRY();
    UINT32 freq = 0;

    if (startClocks)
    {
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER_CSI, &freq);
        DEBUGMSG(ZONE_FUNCTION,(TEXT("BSPEnableCamera: DDK_CLOCK_SIGNAL_PER_CSI = %d"),freq));
        
        // Set right divider to the USB PLL to obtain 24 MHz
        DDKClockConfigBaud(DDK_CLOCK_SIGNAL_PER_CSI, DDK_CLOCK_BAUD_SOURCE_USBPLL, 10-1);

        // Turn CSI and sensor clocks on
        if(!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_CSI, DDK_CLOCK_GATE_MODE_ENABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPSensorSetClockGatingMode: Enable CSI clock failed.\r\n")));
            return FALSE;
        }
        if(!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_CSI, DDK_CLOCK_GATE_MODE_ENABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPSensorSetClockGatingMode: Enable CSI clock failed.\r\n")));
            return FALSE;
        }
        if(!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_CSI, DDK_CLOCK_GATE_MODE_ENABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPSensorSetClockGatingMode: Enable CSI clock failed.\r\n")));
            return FALSE;
        }

        DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER_CSI, &freq);
        DEBUGMSG(ZONE_FUNCTION,(TEXT("BSPEnableCamera: DDK_CLOCK_SIGNAL_PER_CSI = %d"),freq));
    }
    else
    {
        // Turn CSI and sensor clocks off
        if(!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_CSI, DDK_CLOCK_GATE_MODE_DISABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPSensorSetClockGatingMode: Disable CSI clock failed.\r\n")));
            return FALSE;
        }
        if(!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_CSI, DDK_CLOCK_GATE_MODE_DISABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPSensorSetClockGatingMode: Disable CSI clock failed.\r\n")));
            return FALSE;
        }
        if(!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_CSI, DDK_CLOCK_GATE_MODE_DISABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPSensorSetClockGatingMode: Disable CSI clock failed.\r\n")));
            return FALSE;
        }
    }

    BSP_CSI_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: BSPCSIGetInputDataConfig
//
// This function returns the input data configuration
//
// Parameters:
//      None.
//
// Returns:
//      The Input Data Sensor configuration for this platform      
//
//------------------------------------------------------------------------------
csiSensorInputDataConfig CsiClass::BSPCSIGetInputDataConfig(void)
{
    switch (gSensorInUse)
    {
      case csiSensorId_OV2640:
      default:
           return One8BitSensor;
           break;
    }
}

//------------------------------------------------------------------------------
//
// Function: BSPGetSensorFormat
//
// This function returns the sensor data format.
//
// Parameters:
//      pSensorFormat
//          [out] Pointer to DWORD to hold return value of sensor format.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CsiClass::BSPGetSensorFormat(DWORD *pSensorFormat)
{
    BSP_CSI_FUNCTION_ENTRY();

    *pSensorFormat = csiSensorOutputFormat_RGB565;

    BSP_CSI_FUNCTION_EXIT();    
}

//------------------------------------------------------------------------------
//
// Function: BSPGetSensorResolution
//
// This function returns the sensor data format.
//
// Parameters:
//      pSensorFormat
//          [out] Pointer to DWORD to hold return value of sensor resolution.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CsiClass::BSPGetSensorResolution(DWORD *pSensorResolution)
{
    BSP_CSI_FUNCTION_ENTRY();

    switch (gSensorInUse)
    {
      case csiSensorId_OV2640:
           default:
           *pSensorResolution = csiSensorOutputResolution_SVGA; 
           break;
    }  

    BSP_CSI_FUNCTION_EXIT();  

}

//----------------------------------------------------------------------------
//
// Function: BSPSensorFlip
//
// Sets the Sensor Flipping features.
//
// Parameters:
//      doFlip
//          [in] Sensor Vertical flipping will be set or not
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPSensorFlip(BOOL doFlip)
{
    //Sensor Mirroring feature will not be used, so ignor it
    CameraOV2640MirFlip(FALSE, doFlip);
}

//-----------------------------------------------------------------------------
//
// Function: I2CWriteOneByte
//
// This function writes a single byte byData to the register stated in byReg.
//
// Parameters:
//      hI2C
//          [in] File handle to I2C Bus Interface.
//
//      byAddr
//          [in] I2C Slave device address.
//
//      byReg
//          [in] Register Index.
//
//      byData
//          [in] Data to write to byReg.
//
//      lpiResult
//          [in] Pointer of the result. The I2C Bus will store the
//            result of the operation in location pointed to by
//            lpiResult.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID I2CWriteOneByte(HANDLE hI2C, BYTE byAddr, BYTE byReg, BYTE byData, LPINT lpiResult)
{
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPacket;
    BYTE byOutData[2];

    BSP_CSI_FUNCTION_ENTRY();

    byOutData[0] = byReg;
    byOutData[1] = byData;
    I2CPacket.wLen = sizeof(byOutData);
    I2CPacket.pbyBuf = (PBYTE) &byOutData;

    I2CPacket.byRW = I2C_RW_WRITE;
    I2CPacket.byAddr = byAddr;
    I2CPacket.lpiResult = lpiResult;

    I2CXferBlock.pI2CPackets = &I2CPacket;
    I2CXferBlock.iNumPackets = 1;

    I2CTransfer(hI2C, &I2CXferBlock);

    BSP_CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: I2CReadOneByte
//
// This function read a single byte data from the register stated in byReg.
//
// Parameters:
//      hI2C
//          [in] File handle to I2C Bus Interface.
//
//      byAddr
//          [in] I2C Slave device address.
//
//      byReg
//          [in] Register Index.
//
//      lpiResult
//          [in] Pointer of the result. The I2C Bus will store the
//            result of the operation in location pointed to by
//            lpiResult.
//
// Returns:
//      The single byte content stored in byReg.
//
//-----------------------------------------------------------------------------
BYTE I2CReadOneByte(HANDLE hI2C, BYTE byAddr, BYTE byReg, LPINT lpiResult)
{
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPacket[2];
    BYTE byOutData;
    BYTE byInData;

    BSP_CSI_FUNCTION_ENTRY();

    byOutData = byReg;

    I2CPacket[0].pbyBuf = (PBYTE) &byOutData;
    I2CPacket[0].wLen = sizeof(byOutData);

    I2CPacket[0].byRW = I2C_RW_WRITE;
    I2CPacket[0].byAddr = byAddr;
    I2CPacket[0].lpiResult = lpiResult;


    I2CPacket[1].pbyBuf = (PBYTE) &byInData;
    I2CPacket[1].wLen = sizeof(byInData);

    I2CPacket[1].byRW = I2C_RW_READ;
    I2CPacket[1].byAddr = byAddr;
    I2CPacket[1].lpiResult = lpiResult;

    I2CXferBlock.pI2CPackets = I2CPacket;
    I2CXferBlock.iNumPackets = 2;

    I2CTransfer(hI2C, &I2CXferBlock);

    BSP_CSI_FUNCTION_EXIT();

    return byInData;
}

//-----------------------------------------------------------------------------
//
// Function: I2CWriteTwoBytes
//
// This function writes a two byte data to the register stated in byReg. The
// function will write byData1 first, followed by byData2. If byData1 is not
// written properly, the entire function will terminate without attempting to
// write byData2.
//
// Parameters:
//      hI2C
//          [in] File handle to I2C Bus Interface.
//
//      byAddr
//          [in] I2C Slave device address.
//
//      byReg
//          [in] Register Index.
//
//      byData1
//          [in] 1st Data to write to byReg.
//
//      byData2
//          [in] 2nd Data to write to byReg
//
//      lpiResult
//          [in] Pointer of the result. The I2C Bus will store the
//            result of the operation in location pointed to by
//            lpiResult.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID I2CWriteTwoBytes(HANDLE hI2C, BYTE byAddr, BYTE byReg, BYTE byData1, BYTE byData2, LPINT lpiResult)
{
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPacket;
    BYTE byOutData[3];

    BSP_CSI_FUNCTION_ENTRY();

    byOutData[0] = byReg;
    byOutData[1] = byData1;
    byOutData[2] = byData2;

    I2CPacket.wLen = sizeof(byOutData);
    I2CPacket.pbyBuf = (PBYTE) &byOutData;

    I2CPacket.byRW = I2C_RW_WRITE;
    I2CPacket.byAddr = byAddr;
    I2CPacket.lpiResult = lpiResult;

    I2CXferBlock.pI2CPackets = &I2CPacket;
    I2CXferBlock.iNumPackets = 1;

    I2CTransfer(hI2C, &I2CXferBlock);

    BSP_CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: I2CReadTwoBytes
//
// This function reads two bytes of data from the register stated in byReg.
//
// Parameters:
//      hI2C
//          [in] File handle to I2C Bus Interface.
//
//      byAddr
//          [in] I2C Slave device address.
//
//      byReg
//          [in] Register Index.
//
//      lpiResult
//          [in] Pointer of the result. The I2C Bus will store the
//          result of the operation in location pointed to by
//          lpiResult.
//
// Returns:
//      The two byte content stored in byReg.
//
//-----------------------------------------------------------------------------
WORD I2CReadTwoBytes(HANDLE hI2C, BYTE byAddr, BYTE byReg, LPINT lpiResult)
{
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPacket[2];
    BYTE byOutData;
    BYTE byInData[2];

    BSP_CSI_FUNCTION_ENTRY();

    byOutData = byReg;

    I2CPacket[0].pbyBuf = (PBYTE) &byOutData;
    I2CPacket[0].wLen = sizeof(byOutData);

    I2CPacket[0].byRW = I2C_RW_WRITE;
    I2CPacket[0].byAddr = byAddr;
    I2CPacket[0].lpiResult = lpiResult;


    I2CPacket[1].pbyBuf = (PBYTE) &byInData;
    I2CPacket[1].wLen = sizeof(byInData);

    I2CPacket[1].byRW = I2C_RW_READ;
    I2CPacket[1].byAddr = byAddr;
    I2CPacket[1].lpiResult = lpiResult;

    I2CXferBlock.pI2CPackets = I2CPacket;
    I2CXferBlock.iNumPackets = 2;

    I2CTransfer(hI2C, &I2CXferBlock);

    BSP_CSI_FUNCTION_EXIT();

    return ((0xFF & byInData[1]) | (((WORD) byInData[0]) << 8));
}

//-----------------------------------------------------------------------------
//
// Function: I2CReadThreeBytes
//
// This function reads three bytes of data from the register stated in byReg.
//
// Parameters:
//      hI2C
//          [in] File handle to I2C Bus Interface.
//
//      byAddr
//          [in] I2C Slave device address.
//
//      byReg
//          [in] Register Index.
//
//      lpiResult
//          [in] Pointer of the result. The I2C Bus will store the
//          result of the operation in location pointed to by
//          lpiResult.
//
// Returns:
//      The three byte content stored in byReg.
//
//-----------------------------------------------------------------------------
BOOL I2CReadThreeBytes(HANDLE hI2C, BYTE byAddr, BYTE byReg, LPINT lpiResult ,PBYTE byInData)
{
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPacket[3];
    BYTE byOutData;
    BOOL bRet = FALSE;

    BSP_CSI_FUNCTION_ENTRY();

    byOutData = byReg;

    I2CPacket[0].pbyBuf = (PBYTE) &byOutData;
    I2CPacket[0].wLen = sizeof(byOutData);

    I2CPacket[0].byRW = I2C_RW_WRITE;
    I2CPacket[0].byAddr = byAddr;
    I2CPacket[0].lpiResult = lpiResult;

    I2CPacket[1].pbyBuf = (PBYTE) &byInData;
    I2CPacket[1].wLen = 2;
 
    I2CPacket[1].byRW = I2C_RW_READ;
    I2CPacket[1].byAddr = byAddr;

    I2CPacket[2].pbyBuf = (PBYTE) &byInData[2];
    I2CPacket[2].wLen = 1;
 
    I2CPacket[2].byRW = I2C_RW_READ;
    I2CPacket[2].byAddr = byAddr;
 
    I2CXferBlock.pI2CPackets = I2CPacket;
    I2CXferBlock.iNumPackets = 3;

    bRet = I2CTransfer(hI2C, &I2CXferBlock);    

    BSP_CSI_FUNCTION_EXIT();

    return bRet;
}



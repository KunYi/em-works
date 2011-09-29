//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CameraOV2640.cpp    
//
//  Definitions for OminVisionChip camera module specific
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)
#include <windows.h>
#pragma warning(pop)

#include "cameradbg.h"
#include "bsp.h"
#include "i2cbus.h"
#include "CameraPDDProps.h"
#include "CamBufferManager.h"
#include "CsiClass.h"
#include "bspcsi.h"
#include "CameraOV2640.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables
extern csiSensorId gSensorInUse;

//------------------------------------------------------------------------------
// Defines
#define OV2640_I2C_ADDRESS        0x30  //ov2640, default 30
#define OV2640_I2C_SPEED          400000 //default 400KHZ

#define CAMERA_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define CAMERA_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
HANDLE hI2C = NULL ;

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: CameraOV2640Init
//
// Initializes the OV2640 camera sensor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CameraOV2640Init(SensorOutputFrameRate SensorFramerate)
{
    DWORD dwFrequency;
    BYTE bySlaveAddr, byCsiAddr;
    
    CAMERA_FUNCTION_ENTRY();

    byCsiAddr = CSI_I2C_ADDRESS;

    switch (gSensorInUse)
    {
     case csiSensorId_OV2640:
           dwFrequency = OV2640_I2C_SPEED;
           bySlaveAddr = OV2640_I2C_ADDRESS;
           break;               

     default:
           DEBUGMSG(ZONE_ERROR, (TEXT("%s: We don't support this kind of sensor!\r\n"), __WFUNCTION__));
           return;
    }        

    if(hI2C == NULL)
    {
        hI2C = I2COpenHandle(I2C1_FID);
        if (hI2C == INVALID_HANDLE_VALUE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: CreateFile for I2C failed!\r\n"), __WFUNCTION__));
        }

        if (!I2CSetMasterMode(hI2C))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Set I2C Master mode failed!\r\n"), __WFUNCTION__));
        }
        
        // Initialize the device internal fields
        if (!I2CSetFrequency(hI2C, dwFrequency))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Set I2C frequency failed!\r\n"), __WFUNCTION__));
        }
    }

    switch(SensorFramerate)
    {
        case SensorOutputFrameRate30fps:    
            CameraOV2640SetSVGA30fps();
            break;

        case SensorOutputFrameRateUXGA15fps:
            CameraOV2640SetUXGA15fps();
            break;

        default:
            ERRORMSG(TRUE,(TEXT("Set error sensor output framerate!\r\n")));
    }
}


//------------------------------------------------------------------------------
//
// Function: CameraOV2640SetUXGA15fps
//
// Set sensor output frame rate is 15fps based on UXGA.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CameraOV2640SetUXGA15fps()
{
    INT iResult;
    BYTE bySlaveAddr;

    CAMERA_FUNCTION_ENTRY();
    switch (gSensorInUse)
    {
     case csiSensorId_OV2640:
           bySlaveAddr = OV2640_I2C_ADDRESS;
           break;               

     default:
           ERRORMSG(TRUE, (TEXT("%s: We don't support this kind of sensor!\r\n"), __WFUNCTION__));
           return;
    }   

    DEBUGMSG(ZONE_FUNCTION,(TEXT("CameraOV2640SetUXGA15fps: Set sensor registers\r\n")));
    // ;2640 UXGA 15fps RGB565
    {   
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x01, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x12, 0x80, &iResult);  
        
        Sleep(10);
         
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x2c, 0xff, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x2e, 0xdf, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x01, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3c, 0x32, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x11, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x09, 0x02, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x04, 0x28, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x13, 0xe5, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x14, 0x48, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x2c, 0x0c, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x33, 0x78, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3a, 0x33, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3b, 0xfb, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3e, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x43, 0x11, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x16, 0x10, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x39, 0x02, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x35, 0x58, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x22, 0x0a, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x37, 0x40, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x23, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x34, 0xa0, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x36, 0x1a, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x06, 0x02, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x07, 0xc0, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x0d, 0xb7, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x0e, 0x01, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x4c, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x4a, 0x81, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x21, 0x99, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x24, 0x40, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x25, 0x38, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x26, 0x82, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x63, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x46, 0x3f, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x61, 0x70, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x62, 0x80, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7c, 0x05, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x20, 0x80, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x28, 0x30, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x6c, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x6d, 0x80, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x6e, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x70, 0x02, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x71, 0x94, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x73, 0xc1, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3d, 0x34, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x57, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x0c, 0x04, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x4f, 0xbb, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x9c, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe5, 0x7f, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xf9, 0xc0, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x41, 0x24, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x14, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x76, 0xff, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x33, 0xa0, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x42, 0x20, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x43, 0x18, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x4c, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x87, 0xd0, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x88, 0x3f, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xd7, 0x03, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xd9, 0x10, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x82, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc8, 0x08, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc9, 0x80, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7c, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7c, 0x03, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x48, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x48, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7c, 0x08, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x20, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x10, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x0e, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x90, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x0e, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x1a, &iResult);                           
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x31, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x5a, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x69, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x75, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x7e, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x88, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x8f, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x96, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xa3, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xaf, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xc4, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xd7, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xe8, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x20, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x92, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x06, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0xe3, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x05, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x05, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x04, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x96, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x08, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x19, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x02, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x0c, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x24, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x30, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x28, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x26, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x02, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x98, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x80, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc3, 0xed, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xa4, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xa8, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc5, 0x11, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc6, 0x51, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xbf, 0x80, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc7, 0x10, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb6, 0x66, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb8, 0xA5, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb7, 0x64, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb9, 0x7C, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb3, 0xaf, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb4, 0x97, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb5, 0xFF, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb0, 0xC5, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb1, 0x94, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb2, 0x0f, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc4, 0x5c, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x1d, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x57, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x90, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x2c, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x05, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x82, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc3, 0xed, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7f, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xda, 0x08, &iResult);         
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe5, 0x1f, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe1, 0x77, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xdd, 0x7f, &iResult);  
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x05, 0x00, &iResult);  
    }   

    CAMERA_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: CameraOV2640SetSVGA30fps
//
// Set sensor output frame rate is 30fps.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CameraOV2640SetSVGA30fps()
{
    INT iResult;
    BYTE bySlaveAddr;

    CAMERA_FUNCTION_ENTRY();
    switch (gSensorInUse)
    {
     case csiSensorId_OV2640:
           bySlaveAddr = OV2640_I2C_ADDRESS;
           break;               

     default:
           ERRORMSG(TRUE, (TEXT("%s: We don't support this kind of sensor!\r\n"), __WFUNCTION__));
           return;
    }   

    DEBUGMSG(ZONE_FUNCTION,(TEXT("CameraOV2640SetSVGA30fps: Set sensor registers\r\n")));
    // ;2640 SVGA 30fps RGB565
    {   
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x01, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x12, 0x80, &iResult); 
    
        Sleep(10);
    
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x2c, 0xff, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x2e, 0xdf, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x01, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3c, 0x32, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x11, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x09, 0x00, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x04, 0x28, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x13, 0xe5, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x14, 0x48, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x2c, 0x0c, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x33, 0x78, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3a, 0x33, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3b, 0xfB, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3e, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x43, 0x11, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x16, 0x10, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x39, 0x92, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x35, 0xda, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x22, 0x1a, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x37, 0xc3, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x23, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x34, 0xc0, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x36, 0x1a, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x06, 0x88, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x07, 0xc0, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x0d, 0x87, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x0e, 0x41, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x4c, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x48, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5B, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x42, 0x03, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x4a, 0x81, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x21, 0x99, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x24, 0x40, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x25, 0x38, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x26, 0x82, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x63, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x46, 0x22, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x0c, 0x3c, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x61, 0x70, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x62, 0x80, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7c, 0x05, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x20, 0x80, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x28, 0x30, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x6c, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x6d, 0x80, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x6e, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x70, 0x02, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x71, 0x94, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x73, 0xc1, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x12, 0x40, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x17, 0x11, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x18, 0x43, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x19, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x1a, 0x4b, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x32, 0x09, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x37, 0xc0, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x4f, 0xca, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0xa8, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x23, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x6d, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x3d, 0x38, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe5, 0x7f, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xf9, 0xc0, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x41, 0x24, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x14, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x76, 0xff, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x33, 0xa0, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x42, 0x20, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x43, 0x18, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x4c, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x87, 0xd5, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x88, 0x3f, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xd7, 0x03, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xd9, 0x10, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x82, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc8, 0x08, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc9, 0x80, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7c, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7c, 0x03, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x48, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x48, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7c, 0x08, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x20, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x10, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7d, 0x0e, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x90, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x0e, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x1a, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x31, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x5a, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x69, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x75, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x7e, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x88, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x8f, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x96, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xa3, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xaf, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xc4, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xd7, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0xe8, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x91, 0x20, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x92, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x06, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0xe3, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x05, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x05, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x04, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x93, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x96, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x08, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x19, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x02, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x0c, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x24, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x30, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x28, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x26, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x02, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x98, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x80, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x97, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc3, 0xed, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xa4, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xa8, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc5, 0x11, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc6, 0x51, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xbf, 0x80, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc7, 0x10, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb6, 0x66, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb8, 0xA5, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb7, 0x64, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb9, 0x7C, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb3, 0xaf, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb4, 0x97, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb5, 0xFF, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb0, 0xC5, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb1, 0x94, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xb2, 0x0f, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc4, 0x5c, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0x64, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x4B, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0xC8, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x96, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0xC8, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x96, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x82, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xc3, 0xed, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x7f, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xda, 0x08, &iResult); 
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe5, 0x1f, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe1, 0x67, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0xdd, 0x7f, &iResult);
        I2CWriteOneByte(hI2C, bySlaveAddr, 0x05, 0x00, &iResult);        
    }   
    
    CAMERA_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: CameraOV2640Deinit
//
// Deinitializes the OV2640 camera sensor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CameraOV2640Deinit(void)
{
    CAMERA_FUNCTION_ENTRY();

    CloseHandle(hI2C);

    CAMERA_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: CameraOV2640SetOutputFormat
//
// Set interface to configure camera output format.
//
// Parameters:
//      outputFormat
//          [in] structure indicating the sensor output format:
//              csiSensorOutputFormat_YUV422 - YUV422
//              csiSensorOutputFormat_YUV422 - YUV420
//              csiSensorOutputFormat_RGB565 - RGB565
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CameraOV2640SetOutputFormat(cameraOutputFormat outputFormat)
{
    INT iResult;
    BYTE bySlaveAddr;

    CAMERA_FUNCTION_ENTRY();
    switch (gSensorInUse)
    {
     case csiSensorId_OV2640:
           bySlaveAddr = OV2640_I2C_ADDRESS;
           break;               

     default:
           ERRORMSG(TRUE, (TEXT("%s: We don't support this kind of sensor!\r\n"), __WFUNCTION__));
           return;
    } 

    switch(outputFormat)
    {
        case cameraOutputFormat_RGB555:
            break;

        case cameraOutputFormat_RGB565:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CameraOV2640SetOutputFormat: Set RGB565\r\n")));
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xda, 0x08, &iResult);
            break;
            
        case cameraOutputFormat_YUV420:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CameraOV2640SetOutputFormat: Set YUV420\r\n")));
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult); 
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xda, 0x00, &iResult);
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xd7, 0x1b, &iResult); 
            break;

        case cameraOutputFormat_YUV422_YUY2:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CameraOV2640SetOutputFormat: Set YUV422_YUY2\r\n")));
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult); 
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xda, 0x01, &iResult); 
            break;

        case  cameraOutputFormat_YUV422_UYVY:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CameraOV2640SetOutputFormat: Set YUV422_UYVY\r\n")));
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult); 
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xda, 0x00, &iResult);
            I2CWriteOneByte(hI2C, bySlaveAddr, 0xd7, 0x01, &iResult); 
            break;
    }
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(outputFormat);

    CAMERA_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: CameraOV2640SetOutputResolution
//
// Set interface to configure camera output resolution.
//
// Parameters:
//      outputResolution
//          [in]  Structure indicating the camera output resolution:
//              csiSensorOutputResolution outputResolution :
//              csiSensorOutputResolution_1280*960 
//              csiSensorOutputResolution_1024*800 
//              csiSensorOutputResolution_SVGA - SVGA 800*600
//              csiSensorOutputResolution_VGA - VGA 640*480     
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CameraOV2640SetOutputResolution(DWORD dwFramerate,csiSensorOutputResolution outputResolution)
{
    INT iResult;
    BYTE bySlaveAddr;

    CAMERA_FUNCTION_ENTRY();
    switch (gSensorInUse)
    {
     case csiSensorId_OV2640:
           bySlaveAddr = OV2640_I2C_ADDRESS;
           break;               

     default:
           ERRORMSG(TRUE, (TEXT("%s: We don't support this kind of sensor!\r\n"), __WFUNCTION__));
           return;
    } 

    // Make sure other resolution MCLK is 24, 1280*960 is 24/2    
    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x01, &iResult);
    I2CWriteOneByte(hI2C, bySlaveAddr, 0x11, 0x00, &iResult);
    switch(outputResolution)
    {
        case csiSensorOutputResolution_1280_960:
            switch((SensorOutputFrameRate)dwFramerate)
            {
                case SensorOutputFrameRate30fps:
                    ERRORMSG(0,(TEXT("Cannot scale down SVGA to 1280*960\r\n")));
                break;
                
                case SensorOutputFrameRateUXGA15fps://UXGA to 1280*960
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x01, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x11, 0x01, &iResult);//Set MCLK/2
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);    
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x40, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0xf0, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x01, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x82, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                break;
            }
            break;
            
        case csiSensorOutputResolution_1024_800:
            switch((SensorOutputFrameRate)dwFramerate)
            {
                case SensorOutputFrameRate30fps:
                    ERRORMSG(0,(TEXT("Cannot scale down SVGA to 1024*800\r\n")));
                break;
                
                case SensorOutputFrameRateUXGA15fps://UXGA to 1024*800
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x01, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                break;
            }
            break;
            
        case csiSensorOutputResolution_SVGA:
            switch((SensorOutputFrameRate)dwFramerate)
            {
                case SensorOutputFrameRate30fps://SVGA to SVGA
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0x64, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x4B, &iResult); 
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x1D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0xC8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0xC8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;

                case SensorOutputFrameRateUXGA15fps://UXGA to SVGA
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x89, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;
            }
        break;
        
        case csiSensorOutputResolution_VGA: 
            switch((SensorOutputFrameRate)dwFramerate)
            {
                case SensorOutputFrameRate30fps://SVGA to VGA
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult); //SVGA to VGA       
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0x64, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x4B, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0xC8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0xA0, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x78, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult); 
                    break;
  
                case SensorOutputFrameRateUXGA15fps://UXGA to VGA
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x89, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0xa0, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x78, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;
            }
        break;

        case csiSensorOutputResolution_QVGA:
            switch((SensorOutputFrameRate)dwFramerate)
            {
                case SensorOutputFrameRate30fps://SVGA to QVGA
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0x64, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x4B, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x89, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0xC8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x50, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x3C, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;
   
                case SensorOutputFrameRateUXGA15fps://UXGA to QVGA
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x92, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x50, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x3c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;
            }
        break;

        case csiSensorOutputResolution_QQVGA:
            switch((SensorOutputFrameRate)dwFramerate)
            {
                case SensorOutputFrameRate30fps://SVGA to QQVGA
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0x64, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x4B, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x92, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0xC8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x28, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x1E, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;

                case SensorOutputFrameRateUXGA15fps://UXGA to QQVGA
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x9b, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x28, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x1e, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;
            }
        break;

        case csiSensorOutputResolution_CIF:
            switch((SensorOutputFrameRate)dwFramerate)
            {
                case SensorOutputFrameRate30fps://SVGA to CIF
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0x64, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x4B, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x89, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0xC8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x58, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x48, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;

                case SensorOutputFrameRateUXGA15fps://UXGA to CIF
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x92, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x58, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x48, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;
            }
        break;

        case csiSensorOutputResolution_QCIF:
            switch((SensorOutputFrameRate)dwFramerate)
            {
                case SensorOutputFrameRate30fps://SVGA to QCIF
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0x64, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x4B, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x92, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0xC8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x2C, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x24, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;

                case SensorOutputFrameRateUXGA15fps://UXGA to QCIF
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x9b, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x24, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;
            }
        break;

        case csiSensorOutputResolution_QQCIF:
            switch((SensorOutputFrameRate)dwFramerate)
            {
                case SensorOutputFrameRate30fps://SVGA to QQCIF
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0x64, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x4B, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0x9B, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0xC8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x16, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x12, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;

                case SensorOutputFrameRateUXGA15fps://UXGA to QQCIF
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x00, &iResult);        
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x04, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc0, 0xc8, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xc1, 0x96, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x8c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x86, 0x3D, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x50, 0xa4, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x51, 0x90, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x52, 0x2c, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x53, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x54, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x55, 0x88, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5a, 0x16, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5b, 0x12, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0x5c, 0x00, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xd3, 0x02, &iResult);
                    I2CWriteOneByte(hI2C, bySlaveAddr, 0xe0, 0x00, &iResult);
                    break;
            }
        break;

        default:
            break;
    }   
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(outputResolution);

    CAMERA_FUNCTION_EXIT();
}


//----------------------------------------------------------------------------
//
// Function: CameraOV2640SetDigitalZoom
//
// Sets the camera digital zoom level .
//
// Parameters:
//      zoom
//          [in] Camera zoom value that will be set
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CameraOV2640SetDigitalZoom(DWORD ZoomLevel)
{    
    CAMERA_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ZoomLevel);

    CAMERA_FUNCTION_EXIT();
}


//----------------------------------------------------------------------------
//
// Function: CameraOV2640MirFlip
//
// Sets the camera Mirroring and Flipping features.
//
// Parameters:
//      doRot
//          [in] Camera Horizontal Mirroring will be set or not
//      doFlip
//          [in] Camera Vertical flipping will be set or not
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CameraOV2640MirFlip(BOOL doMirror, BOOL doFlip)
{    
    INT iResult;
    BYTE bySlaveAddr;

    CAMERA_FUNCTION_ENTRY();

    switch (gSensorInUse)
    {
     case csiSensorId_OV2640:
           bySlaveAddr = OV2640_I2C_ADDRESS;
           break;               

     default:
           DEBUGMSG(ZONE_ERROR, (TEXT("%s: We don't support this kind of sensor!\r\n"), __WFUNCTION__));
           return;
    }   
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(doMirror);
    UNREFERENCED_PARAMETER(doFlip);

    I2CWriteOneByte(hI2C, bySlaveAddr, 0xff, 0x01, &iResult);

    if(doMirror)
    {
        if (doFlip)
            I2CWriteOneByte(hI2C, bySlaveAddr, 0x04,0xf0, &iResult);
        else
            I2CWriteOneByte(hI2C, bySlaveAddr, 0x04,0xa0, &iResult);
    }
    else
    {
        if (doFlip)
            I2CWriteOneByte(hI2C, bySlaveAddr, 0x04,0x70, &iResult);
        else
            I2CWriteOneByte(hI2C, bySlaveAddr, 0x04,0x20, &iResult); //default value
    }
    
    CAMERA_FUNCTION_EXIT();

}


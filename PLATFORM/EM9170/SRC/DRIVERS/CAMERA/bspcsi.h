//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//

//  File:  bspcsi.h
//
//  Provides prototypes for BSP-specific CSI routines to communicate via I2C.
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Defines

#define CSI_SENSOR_NUMBER_SUPPORTED   1
#define CSI_I2C_ADDRESS               0x20


// CSI Registry Path
#define CSI_REG_PATH                  "Drivers\\BuiltIn\\Camera"
#define CSI_REG_CAMERAID_KEYWORD      "CameraId"

// Define the right platform for custom CamApp
#define CAMAPP_FOR_SENNA

//------------------------------------------------------------------------------
// Types
typedef enum csiSensorId {
    csiSensorId_OV2640 = 0,
}csiSensorId_c;


typedef enum cameraOutputFormatEnum {
    cameraOutputFormat_YUV422_YUY2,
    cameraOutputFormat_YUV422_UYVY,
    cameraOutputFormat_YUV420,
    cameraOutputFormat_RGB565,
    cameraOutputFormat_RGB555,
} cameraOutputFormat;

//------------------------------------------------------------------------------
// Functions

VOID I2CWriteOneByte(HANDLE, BYTE, BYTE, BYTE, LPINT);
BYTE I2CReadOneByte(HANDLE, BYTE, BYTE, LPINT);
VOID I2CWriteTwoBytes(HANDLE, BYTE, BYTE, BYTE, BYTE, LPINT);
WORD I2CReadTwoBytes(HANDLE, BYTE, BYTE, LPINT);
BOOL I2CReadThreeBytes(HANDLE, BYTE, BYTE, LPINT, PBYTE);

//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CameraOV2640.h
//
//  Definitions for OV2640Chip Camera Sensor.
//
//------------------------------------------------------------------------------

#ifndef __CAMERAOV2640_H__
#define __CAMERAOV2640_H__

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types
typedef enum SensorOutPutFrameRateEnum
{
    SensorOutputFrameRateUXGA15fps = 15,
    SensorOutputFrameRate30fps = 30,
} SensorOutputFrameRate;

//------------------------------------------------------------------------------
// Functions
void CameraOV2640Init(SensorOutputFrameRate);
void CameraOV2640SetSVGA30fps(void);
void CameraOV2640SetUXGA15fps(void);
void CameraOV2640Deinit(void);
void CameraOV2640SetOutputResolution(DWORD ,csiSensorOutputResolution);
void CameraOV2640SetOutputFormat(cameraOutputFormat);
void CameraOV2640SetDigitalZoom(DWORD);
void CameraOV2640MirFlip(BOOL doMirror, BOOL doFlip);


#endif   // __CAMERAOV2640_H__


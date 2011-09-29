//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
/*
* NAME:         GPSCTL.h
*
* DESCRIPTION:  Contains all exports of the millisecond timer.
*
* NOTES:   The GPSCTL is sample driver for creating millisecond timer and 
*          is implemented according to best practices recommended by Microsoft.
******************************************************************************/
/// \file Trigger inclusion in doxygen documentation

#pragma once

#include <tchar.h>

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GPSCTL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GPS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GPSCTL_EXPORTS
#define GPS_API __declspec(dllexport)
#else
#define GPS_API 
//__declspec(dllimport)
#endif

#if !defined(SUPPORT_POWER_YYY_ENTRY_POINTS)
    // By default let's not support XXX_PowerUp/Down in order to specify driver as pageable
    // -> see "Best practices for Optimizing Device Drivers" in Platform Builder docu.
    #define SUPPORT_POWER_YYY_ENTRY_POINTS 1
#endif


#if defined(__cplusplus)
extern "C" {
#endif

/// Initializes driver.
/// Initializes driver, gets called during RegisterService or by Services.exe upon startup.
///
/// \see See Microsoft's documentation on device driver entry points.
GPS_API DWORD GPS_Init(LPCTSTR pContext, LPCVOID lpvBusContext);

/// Deinitializes device driver.
/// Deinitializes device driver.
///
/// \see See Microsoft's documentation on device driver entry points.
GPS_API BOOL GPS_Deinit(DWORD context);

/// IOControl call.
/// Exposes the different IOControls supported by the module.
///
/// \see See Microsoft's documentation on device driver entry points.
GPS_API BOOL GPS_IOControl(DWORD context, DWORD code,
    PBYTE pBufIn, DWORD lenIn,
    PBYTE pBufOut, DWORD lenOut,
    PDWORD pActualOut);

#if SUPPORT_POWER_YYY_ENTRY_POINTS

/// Placeholder for powerdown functionality.
/// Placeholder for powerdown functionality. Note that one may not call
/// any Win32 APIs during this call
///
/// \remarks There's no actual power functionality in this device driver.
/// \see See Microsoft's documentation on device driver entry points.
GPS_API void GPS_PowerDown(DWORD context);

/// Placeholder for powerup functionality.
/// Placeholder for powerup functionality. Note that one may not call any
/// Win32 APIs during this call
///
/// \remarks There's no actual power functionality in this device driver.
/// \see See Microsoft's documentation on device driver entry points.
GPS_API void GPS_PowerUp(DWORD context);

#endif // SUPPORT_POWER_YYY_ENTRY_POINTS

/// Opens the stream interface.
/// Opens the stream interface of the device driver.
///
/// \see See Microsoft's documentation on device driver entry points.
GPS_API DWORD GPS_Open(DWORD context, DWORD accessCode, DWORD shareMode);

/// Closes the stream interface.
/// Closes the stream interface of the device driver.
///
/// \see See Microsoft's documentation on device driver entry points.
GPS_API BOOL GPS_Close(DWORD context);

/// Read from stream.
/// Reads data from stream.
///
/// \see See Microsoft's documentation on device driver entry points.
GPS_API DWORD GPS_Read(DWORD context, LPVOID pBuf, DWORD len);

/// Moves position in stream.
/// Moves the current position in stream.
///
/// \remarks There's no actual seek functionality in this device driver.
/// \see See Microsoft's documentation on device driver entry points.
GPS_API DWORD GPS_Seek(DWORD context, long pos, DWORD type);

/// Write to stream.
/// Writes the data specified to stream.
///
/// \see See Microsoft's documentation on device driver entry points.
GPS_API DWORD GPS_Write(DWORD context, LPCVOID pBufIn, DWORD lenIn);

#if defined(__cplusplus)
}
#endif


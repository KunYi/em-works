//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  tve_sdk.cpp
//
//  This module provides wrapper functions for accessing
//  the stream interface for the TVE driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>

#pragma warning(pop)

#include "tve_sdk.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: TVEOpenHandle
//
// This method creates a handle to the TVE stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to TVE driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE TVEOpenHandle(void)
{
    HANDLE hTVE;

    hTVE = CreateFile(TEXT("TVE1:"),        // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to TVE
    if (hTVE == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(1, (TEXT("CreateFile TVE failed!\r\n")));
    }

    return (HANDLE) hTVE;
}


//------------------------------------------------------------------------------
//
// Function: TVECloseHandle
//
// This method closes a handle to the TVE stream driver.
//
// Parameters:
//      hTVE
//          [in] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TVECloseHandle(HANDLE hTVE)
{
    BOOL retVal = TRUE;

    // if we don't have handle to TVE driver
    if (hTVE != NULL)
    {
        if (!CloseHandle(hTVE))
        {
            retVal = FALSE;
        }
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: TVEInit
//
// This method initializes the TVE stream driver.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
// Returns:
//      Returna TRUE if sucessful.
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TVEInit(HANDLE hTVE)
{
    BOOL retVal = TRUE;;
    
    // issue the IOCTL to set TVE closed caption fields
    if (!DeviceIoControl(hTVE,     // file handle to the driver
        TVE_IOCTL_INIT,            // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("TVE_IOCTL_INIT failed!\r\n")));
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: TVEEnable
//
// This method enables a handle to the TVE stream driver.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
// Returns:
//      Returna TRUE if sucessful.
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TVEEnable(HANDLE hTVE)
{
    BOOL retVal = TRUE;;
    
    // issue the IOCTL to enable TVE
    if (!DeviceIoControl(hTVE,     // file handle to the driver
        TVE_IOCTL_ENABLE,          // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("TVE_IOCTL_ENABLE failed!\r\n")));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: TVEDisable
//
// This method disables a handle to the TVE stream driver.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
// Returns:
//      Returna TRUE if sucessful.
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TVEDisable(HANDLE hTVE)
{
    BOOL retVal = TRUE;;
    
    // issue the IOCTL to enable TVE
    if (!DeviceIoControl(hTVE,     // file handle to the driver
        TVE_IOCTL_DISABLE,         // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("TVE_IOCTL_DISABLE failed!\r\n")));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: TVESetClosedCaptionFields
//
// This method sets two data to the TVE Closed-Caption two field registers.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
//      pCCInfo
//          [in] Pointer to an TVECCInfo object holding info needed
//          to set the TVE Closed Caption fields.
//          value.
//
//
// Returns:
//      Returns TRUE if sucessful. 
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------

BOOL TVESetClosedCaptionFields(HANDLE hTVE, TVECCInfo *pCCInfo)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to set TVE closed caption fields
    if (!DeviceIoControl(hTVE,     // file handle to the driver
        TVE_IOCTL_SET_CC_FIELDS,   // I/O control code
        pCCInfo,                   // in buffer
        sizeof(TVECCInfo),         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("TVE_IOCTL_SET_CC_FIELDS failed!\r\n")));
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: TVESetCgmsWssPAL
//
// This method sets the TVECgmsInfo based on the passed parameters.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
//      pCgmsWssInfo
//          [in] Pointer to an TVECgmsWssPAL625Info object holding info needed
//          to set the TVE CGMS-A WSS fields.
//          value.
//
//
// Returns:
//      Returns TRUE if sucessful. 
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TVESetCgmsWssPAL(HANDLE hTVE, TVECgmsWssPAL625Info *pCgmsWssInfo)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to set TVE CGMS-A WSS PAL field
    if (!DeviceIoControl(hTVE,          // file handle to the driver
        TVE_IOCTL_SET_CGMS_WSS_PAL,     // I/O control code
        pCgmsWssInfo,                   // in buffer
        sizeof(TVECgmsWssPAL625Info),   // in buffer size
        NULL,                           // out buffer
        0,                              // out buffer size
        0,                              // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("TVE_IOCTL_SET_CGMS_WSS_PAL failed!\r\n")));
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: TVESetCgmsWssNTSC
//
// This method sets the TVECgmsInfo based on the passed parameters.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
//      pCgmsInfo
//          [in] Pointer to an TVECgmsWssNTSC525Info object holding info needed
//          to set the TVE CGMS-A fields.
//          value.
//
//
// Returns:
//      Returns TRUE if sucessful. 
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TVESetCgmsWssNTSC(HANDLE hTVE, TVECgmsWssNTSC525Info *pCgmsInfo)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to set TVE CGMS-A WSS NTSC fields
    if (!DeviceIoControl(hTVE,          // file handle to the driver
        TVE_IOCTL_SET_CGMS_WSS_NTSC,    // I/O control code
        pCgmsInfo,                      // in buffer
        sizeof(TVECgmsWssNTSC525Info),  // in buffer size
        NULL,                           // out buffer
        0,                              // out buffer size
        0,                              // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("TVE_IOCTL_SET_CGMS_WSS_NTSC failed!\r\n")));
    }

    return retVal;
}



//------------------------------------------------------------------------------
//
// Function: TVESetOutputMode
//
// This method sets the TVE output mode.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
//      pOutputModeInfo
//          [in] Pointer to a TVEOutputModeInfo object holding info needed
//          to set the TV output mode.
//
//
// Returns:
//      Returns TRUE if sucessful. 
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TVESetOutputMode(HANDLE hTVE, TVEOutputModeInfo *pOutputModeInfo)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to set TVE output mode
    if (!DeviceIoControl(hTVE,          // file handle to the driver
        TVE_IOCTL_SET_OUTPUT_MODE,      // I/O control code
        pOutputModeInfo,                // in buffer
        sizeof(TVEOutputModeInfo),      // in buffer size
        NULL,                           // out buffer
        0,                              // out buffer size
        0,                              // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("TVE_IOCTL_SET_OUTPUT_MODE failed!\r\n")));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: TVESetOutputStdType
//
// This method sets the TVE output standard type.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
//      TVEOutputStdInfo
//          [in] Pointer to a TVEOutputStdInfo object holding info needed
//          to set the TV standard type.
//
//
// Returns:
//      Returns TRUE if sucessful. 
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TVESetOutputStdType(HANDLE hTVE, TVEOutputStdInfo *pOutputStdInfo)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to set TVE output standard type
    if (!DeviceIoControl(hTVE,          // file handle to the driver
        TVE_IOCTL_SET_OUTPUT_STD_TYPE,  // I/O control code
        pOutputStdInfo,                 // in buffer
        sizeof(TVEOutputStdInfo),       // in buffer size
        NULL,                           // out buffer
        0,                              // out buffer size
        0,                              // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("TVE_IOCTL_SET_OUTPUT_STD_TYPE failed!\r\n")));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: TVESetOutputResSizeType
//
// This method sets the TVE output resolution size type.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
//      TVEOutputResSizeInfo
//          [in] Pointer to a TVEOutputResSizeInfo object holding info needed
//          to set the TV output resolution size type.
//
//
// Returns:
//      Returns TRUE if sucessful. 
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TVESetOutputResSizeType(HANDLE hTVE, TVEOutputResSizeInfo *pOutputResSizeInfo)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to set TVE output resolution size type
    if (!DeviceIoControl(hTVE,               // file handle to the driver
        TVE_IOCTL_SET_OUTPUT_RES_SIZE_TYPE,  // I/O control code
        pOutputResSizeInfo,                  // in buffer
        sizeof(TVEOutputResSizeInfo),        // in buffer size
        NULL,                                // out buffer
        0,                                   // out buffer size
        0,                                   // number of bytes returned
        NULL))                               // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("TVE_IOCTL_SET_OUTPUT_RES_SIZE_TYPE failed!\r\n")));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: TVEGetOutputMode
//
// This method gets the TVE output mode.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
//
// Returns:
//      Returns TVE output mode if sucessful. 
//      Returns 0 if failure.
//
//------------------------------------------------------------------------------
TVE_TV_OUT_MODE TVEGetOutputMode(HANDLE hTVE) 
{
    
    DWORD outputMode = 0;
    
    // issue the IOCTL to set TVE output resolution size
    if (!DeviceIoControl(hTVE,          // file handle to the driver
        TVE_IOCTL_GET_OUTPUT_MODE,      // I/O control code
        NULL,                           // in buffer
        0,                              // in buffer size
        &outputMode,                    // out buffer
        sizeof(DWORD),                  // out buffer size
        0,                              // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(1, (TEXT("TVE_IOCTL_GET_OUTPUT_MODE failed!\r\n")));
    }

    return (TVE_TV_OUT_MODE)outputMode;
}


//------------------------------------------------------------------------------
//
// Function: TVEGetOutputStdType
//
// This method gets the TVE output standard type.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
//
// Returns:
//      Returns TVE output standard type if sucessful. 
//      Returns 0 if failure.
//
//------------------------------------------------------------------------------
TVE_TV_STAND TVEGetOutputStdType(HANDLE hTVE)
{
    
    DWORD stdType = 0;
    
    // issue the IOCTL to set TVE output resolution size
    if (!DeviceIoControl(hTVE,          // file handle to the driver
        TVE_IOCTL_GET_OUTPUT_STD_TYPE,  // I/O control code
        NULL,                           // in buffer
        0,                              // in buffer size
        &stdType,                       // out buffer
        sizeof(DWORD),                  // out buffer size
        0,                              // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(1, (TEXT("TVE_IOCTL_GET_OUTPUT_STD_TYPE failed!\r\n")));
    }

    return (TVE_TV_STAND)stdType;
}

//------------------------------------------------------------------------------
//
// Function: TVEGetOutputResSizeType
//
// This method gets the TVE output resolution size type.
//
// Parameters:
//      hTVE
//          [in] Handle to TVE driver.
//
//
// Returns:
//      Returns TVE output resolution size type if sucessful. 
//      Returns 0 if failure.
//
//------------------------------------------------------------------------------
TVE_TV_RES_SIZE TVEGetOutputResSizeType(HANDLE hTVE)
{
    
    DWORD resSizeType = 0;
    
    // issue the IOCTL to set TVE output resolution size type
    if (!DeviceIoControl(hTVE,               // file handle to the driver
        TVE_IOCTL_GET_OUTPUT_RES_SIZE_TYPE,  // I/O control code
        NULL,                                // in buffer
        0,                                   // in buffer size
        &resSizeType,                        // out buffer
        sizeof(DWORD),                       // out buffer size
        0,                                   // number of bytes returned
        NULL))                               // ignored (=NULL)
    {
        ERRORMSG(1, (TEXT("TVE_IOCTL_GET_OUTPUT_RES_SIZE_TYPE failed!\r\n")));
    }

    return (TVE_TV_RES_SIZE)resSizeType;
}


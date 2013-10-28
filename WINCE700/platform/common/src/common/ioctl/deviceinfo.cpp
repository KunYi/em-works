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
//  File:  deviceinfo.c
//
//  This file implements the IOCTL_HAL_GET_DEVICE_INFO handler.
//
#include <windows.h>
#include <bldver.h>
#include <oal.h>

const PLATFORMVERSION HALPlatformVersion[] = {{CE_MAJOR_VER, CE_MINOR_VER}};

#if defined( project_smartfon )
const WCHAR g_oalIoCtlProjectName[] = L"SmartPhone";
#elif defined( project_cebase )
const WCHAR g_oalIoCtlProjectName[] = L"CEBase";
#else
const WCHAR g_oalIoCtlProjectName[] = L"Unknown WinCE Project";
#endif

// each platform that needs to support SPI_GETPLATFORMMANUFACTURER and SPI_GETPLATFORMNAME
// must initialize the following global variables to a not-NULL, unicode string in OEMInit()
extern "C" LPCWSTR g_oalIoCtlPlatformManufacturer = NULL;
extern "C" LPCWSTR g_oalIoCtlPlatformName = NULL;

// each platform that needs to support SPI_GETGUIDPATTERN with other than the common GUID pattern below
// must initialize the following global variable to a platform-specific GUID pattern in OEMInit()
extern "C" GUID g_oalIoCtlPlatformGuidPattern = {0x600cc7d0, 0xde3a, 0x4713, {
                                                 0xa5, 0xb0, 0x56, 0xe, 0x6c, 0x36, 0x4e, 0xde
                                                 }};

extern "C" BOOL OEMIoControl(DWORD code, VOID *pInBuffer, DWORD inSize, VOID *pOutBuffer, DWORD outSize, DWORD *pOutSize);

// CheckBufferSize validates that availableSize is greater than or equal to requiredSize.
// It returns true if availableSize is large enough and false otherwise, setting LastError to
// ERROR_INSUFFICIENT_BUFFER.  If pRequiredSize is non-NULL, CheckBufferSize will populate it
// with the value of requiredSize (regardless of whether availableSize is large enough or not).
// The requiredSize then propagates back to the caller who can re-call with a sufficiently large
// availableSize.
static BOOL CheckBufferSize(const VOID* pOutBuffer, const UINT32 & availableSize, UINT32* pRequiredSize, const UINT32 & requiredSize, const WCHAR* spiName);

// ErrorNotSupported sets LastError to ERROR_NOT_SUPPORTED and prints a debug message.
static void ErrorNotSupported(const WCHAR* spiName);

// internal helper functions
static BOOL GetPlatformType(VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName);
static BOOL GetOEMInfo(VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName);
static BOOL GetPlatformVersion(VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName);
static BOOL GetPlatformManufacturer(VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName);
static BOOL GetProjectName(VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName);
static BOOL GetPlatformName(VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName);
static BOOL GetBootMeName(VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName);
static BOOL GetUUID(VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName);
static BOOL GetGUIDPattern(VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName);

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlGetDeviceInfo
//
//  Implements the IOCTL_HAL_GET_DEVICE_INFO handler
//
/*  This IOCTL reads a UINT32 SPI value from pInpBuffer (a preallocated block of size inpSize),
    It reads information from the device based on the value of pInpBuffer.  It returns information
    in pOutBuffer (a preallocated block of size outSize), and returns the size of the filled
    pOutBuffer in pOutSize.  If pOutBuffer is not large enough and pOutSize is valid, *pOutSize
    is set to the required amount of bytes so the caller can call again with an allocated block
    of the appropriate size. */

BOOL OALIoCtlHalGetDeviceInfo( 
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    BOOL rc = FALSE;
    
    // This variable contains the text name of the SPI we're using - at this point we don't know
    // value but we will fill it in later
    const WCHAR* spiName = L"";

    UNREFERENCED_PARAMETER(code);    

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetDeviceInfo(...)\r\n"));

    // Validate inputs
    if ((((DWORD)pInpBuffer) & 3) || //check for byte alignment of input buffer
        (pInpBuffer == NULL) || (inpSize != sizeof(UINT32))) 
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalGetDeviceInfo: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    // Process according to input request
    switch (*(static_cast<UINT32*>(pInpBuffer))) {
    case SPI_GETPLATFORMTYPE:
        rc = GetPlatformType(pOutBuffer, outSize, pOutSize, &spiName);
        break;
    case SPI_GETOEMINFO:
        rc = GetOEMInfo(pOutBuffer, outSize, pOutSize, &spiName);
        break;
    case SPI_GETPLATFORMVERSION:
        rc = GetPlatformVersion(pOutBuffer, outSize, pOutSize, &spiName);
        break;
    case SPI_GETPLATFORMMANUFACTURER: 
        rc = GetPlatformManufacturer(pOutBuffer, outSize, pOutSize, &spiName);
        break;
    case SPI_GETPLATFORMNAME:
        rc = GetPlatformName(pOutBuffer, outSize, pOutSize, &spiName);
        break;
    case SPI_GETPROJECTNAME:
        rc = GetProjectName(pOutBuffer, outSize, pOutSize, &spiName);
        break;
    case SPI_GETBOOTMENAME:
        rc = GetBootMeName(pOutBuffer, outSize, pOutSize, &spiName);
        break;
    case SPI_GETUUID:
        rc = GetUUID(pOutBuffer, outSize, pOutSize, &spiName);
        break;
    case SPI_GETGUIDPATTERN:
        rc = GetGUIDPattern(pOutBuffer, outSize, pOutSize, &spiName);
        break;
    default:
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalGetDeviceInfo: Invalid request\r\n"
        ));
        break;
    }

cleanUp:
    // Indicate status
    OALMSG(OAL_FUNC&&OAL_IOCTL, (
        L"-OALIoCtlHalGetDeviceInfo(SPI = %s, rc = %d)\r\n", spiName, rc
    ));
    return rc;
}

static BOOL GetPlatformType(
    VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName
) {

    // Note: on Smartphone platforms the platform type 
    // string may contain NULLs and is terminated with a 
    // double NULL.
    BOOL rc;
    (*spiName) = L"SPI_GETPLATFORMTYPE";

#if defined( project_smartfon )
    RETAILMSG(1, (TEXT("Warning: you are requesting IOCTL_HAL_GET_DEVICE_INFO::SPI_GETPLATFORMTYPE, which has been deprecated.  Use IOCTL_HAL_GET_DEVICE_INFO::SPI_GETPROJECTNAME instead.\r\n")));
    UINT32 projectNameSPICode = SPI_GETPROJECTNAME;
    rc = OEMIoControl(IOCTL_HAL_GET_DEVICE_INFO, static_cast<VOID*>(&projectNameSPICode), sizeof(projectNameSPICode), pOutBuffer, outSize, reinterpret_cast<DWORD*>(pOutSize));
#else
    RETAILMSG(1, (TEXT("Warning: you are requesting IOCTL_HAL_GET_DEVICE_INFO::SPI_GETPLATFORMTYPE, which has been deprecated.  Use IOCTL_HAL_GET_DEVICE_INFO::SPI_GETPLATFORMNAME instead.\r\n")));
    UINT32 platformNameSPICode = SPI_GETPLATFORMNAME;
    rc = OEMIoControl(IOCTL_HAL_GET_DEVICE_INFO, static_cast<VOID*>(&platformNameSPICode), sizeof(platformNameSPICode), pOutBuffer, outSize, reinterpret_cast<DWORD*>(pOutSize));
#endif

    return rc;
}

static BOOL GetOEMInfo(
    VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName
) {
    BOOL rc;
    (*spiName) = L"SPI_GETOEMINFO";
    rc = (CheckBufferSize(pOutBuffer, outSize, pOutSize, (NKwcslen(g_oalIoCtlPlatformOEM)+1)*sizeof(WCHAR), *spiName));
    if(rc)
    {
        // Copy OEM info to caller's buffer
        NKwcscpy((WCHAR*)pOutBuffer, g_oalIoCtlPlatformOEM);
        rc = TRUE;
    }
    return rc;
}

// GetPlatformVersion is left in the OAL so OAL/Kernel code currently using the
// IOCTL IOCTL_HAL_GET_DEVICE_INFO with SPI code SPI_GETPLATFORMVERSION can query
// for the platform version. All other applications which can link against coredll.dll should use the 
// interface SystemParametersInfo(SPI_GETPLATFORMVERSION, ...)
static BOOL GetPlatformVersion(
    VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName
) {
    BOOL rc;
    (*spiName) = L"SPI_GETPLATFORMVERSION";
    rc = (CheckBufferSize(pOutBuffer, outSize, pOutSize, sizeof(PLATFORMVERSION), *spiName));
    if(rc)
    {
        // Copy platform version to caller's buffer
        memcpy(pOutBuffer, HALPlatformVersion, sizeof(HALPlatformVersion));
        rc = TRUE;
    }
    return rc;
}

static BOOL GetPlatformManufacturer(
    VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName
) {
    UINT32 length;
    BOOL rc;
    (*spiName) = L"SPI_GETPLATFORMMANUFACTURER";

    // validate if this action code is supported
    if (g_oalIoCtlPlatformManufacturer == NULL)
    {
        ErrorNotSupported(*spiName);
        rc = FALSE;
    }
    else
    {
        // Validate output buffer size
        length = (NKwcslen(g_oalIoCtlPlatformManufacturer) + 1) * sizeof(WCHAR);
        rc = (CheckBufferSize(pOutBuffer, outSize, pOutSize, length, *spiName));
        if(rc)
        {
            // Copy platform version to caller's buffer
            memcpy(pOutBuffer, g_oalIoCtlPlatformManufacturer, length);
            rc = TRUE;
        }
    }
    return rc;
}

static BOOL GetProjectName(
    VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName
) {
    BOOL rc;
    (*spiName) = L"SPI_GETPROJECTNAME";
    rc = (CheckBufferSize(pOutBuffer, outSize, pOutSize, (NKwcslen(g_oalIoCtlProjectName)+1)*sizeof(WCHAR), *spiName));
    if(rc)
    {
        // Copy project name to caller's buffer
        NKwcscpy((WCHAR*)pOutBuffer, g_oalIoCtlProjectName);
        rc = TRUE;
    }
    return rc;
}

static BOOL GetPlatformName(
    VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName
) {
    UINT32 length;
    BOOL rc;
    (*spiName) = L"SPI_GETPLATFORMNAME";
 
    // validate if this action code is supported
    if (g_oalIoCtlPlatformName == NULL)
    {
        ErrorNotSupported(*spiName);
        rc = FALSE;
    }
    else
    {
        // Validate output buffer size
        length = (NKwcslen(g_oalIoCtlPlatformName) + 1) * sizeof(WCHAR);
        rc = (CheckBufferSize(pOutBuffer, outSize, pOutSize, length, *spiName));
        if(rc)
        {
            // Copy platform version to caller's buffer
            memcpy(pOutBuffer, g_oalIoCtlPlatformName, length);
            rc = TRUE;
        }
    }        
    return rc;
}

static BOOL GetBootMeName(
    VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName
) {
    BOOL rc;
    (*spiName) = L"SPI_GETBOOTMENAME";
    const char* pBootMeName = (static_cast<char*>(OALArgsQuery(OAL_ARGS_QUERY_DEVID)));
    if(!pBootMeName)
    {
        ErrorNotSupported(*spiName);
        rc = FALSE;
    }
    else 
    {
        // Convert to Unicode so we know what size we're returning to the caller
        // MultiByteToWideChar(...) etc. may not be available at this point
        WCHAR pWBootMeName[OAL_KITL_ID_SIZE+1];
        UINT32 i;
        for(i = 0; (i < OAL_KITL_ID_SIZE) && (*pBootMeName != '\0'); i++)
        {
            pWBootMeName[i] = (*(pBootMeName++));
        }
        pWBootMeName[i] = 0;
        rc = (CheckBufferSize(pOutBuffer, outSize, pOutSize, (NKwcslen(pWBootMeName)+1)*sizeof(WCHAR), *spiName));
        if(rc)
        {
            // Copy Unicode bootme name to caller's buffer
            NKwcscpy((static_cast<WCHAR*>(pOutBuffer)), pWBootMeName);
        }
    }
    return rc;
}

static BOOL GetUUID(
    VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName
) {
    BOOL rc;
    (*spiName) = L"SPI_GETUUID";

    GUID *pUuid;

    // Does BSP specific UUID exist?
    pUuid = static_cast<GUID*>(OALArgsQuery(OAL_ARGS_QUERY_UUID));
    if (!pUuid)
    {
        ErrorNotSupported(*spiName);
        rc = FALSE;
    }
    else 
    {   
        rc = (CheckBufferSize(pOutBuffer, outSize, pOutSize, sizeof(GUID), *spiName));
        if(rc)
        {
            // Return the UUID in the caller's allocated space
            memcpy(pOutBuffer, pUuid, sizeof(GUID));
        }
    }

    return rc;
}

static BOOL GetGUIDPattern(
    VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize, const WCHAR** spiName
) {
    BOOL rc;
    (*spiName) = L"SPI_GETGUIDPATTERN";

    rc = (CheckBufferSize(pOutBuffer, outSize, pOutSize, sizeof(g_oalIoCtlPlatformGuidPattern), *spiName));
    if(rc)
    {
        memcpy(pOutBuffer, &g_oalIoCtlPlatformGuidPattern, sizeof(g_oalIoCtlPlatformGuidPattern));
        rc = TRUE;
    }
    return rc;
}
static BOOL CheckBufferSize(
    const VOID* pOutBuffer, const UINT32 & availableSize, UINT32* pRequiredSize, const UINT32 & requiredSize, const WCHAR* spiName
) {
#ifdef SHIP_BUILD
    UNREFERENCED_PARAMETER(spiName);
#endif
        // If the output buffer is an invalid pointer but the caller is giving us a nonzero size, something is very wrong
        if (pOutBuffer == NULL && availableSize > 0)
        {
            NKSetLastError(ERROR_INVALID_PARAMETER);
            OALMSG(OAL_ERROR, ((
                L"ERROR: OALIoCtlHalGetDeviceinfo::%s: Invalid parameter: nonzero buffer size supplied with invalid buffer\r\n"), spiName));
            return FALSE;
        }
        if (pRequiredSize != NULL)
        {
            // indicate to caller how much data we want to return
            *pRequiredSize = requiredSize; 
        }
        // If there isn't an output buffer or it is too small, return false.
        // At this point, if pRequiredSize is valid it has been filled in, so the
        // caller can call the function again with a buffer of pRequiredSize and expect success.
        if (pOutBuffer == NULL || availableSize < requiredSize)
        {
            NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            OALMSG(OAL_WARN&&OAL_VERBOSE, ((
                L"WARN: OALIoCtlHalGetDeviceinfo::%s: Buffer too small\r\n"), spiName));
            return FALSE;
        }
    // if we made it here then we have sufficient buffer space
    return TRUE;
}

static void ErrorNotSupported(
    const WCHAR* spiName
) {
#ifdef SHIP_BUILD
    UNREFERENCED_PARAMETER(spiName);
#endif
    NKSetLastError(ERROR_NOT_SUPPORTED);
    OALMSG(OAL_ERROR, 
    ((L"ERROR: OALIoCtlHalGetDeviceInfo: Device doesn't support IOCTL_HAL_GET_DEVICE_INFO::%s\r\n"), spiName)
    );
}

//------------------------------------------------------------------------------

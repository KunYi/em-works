//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  asrc_sdk.cpp
//
//  The implementation of ASRC driver SDK.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "asrc_sdk.h"


//------------------------------------------------------------------------------
//
// Function: ASRCOpenHandle
//
// This method creates a handle to the ASRC stream driver.
//
// Parameters:
//      pPairIndex: point to keep pair index returned
//
// Returns:
//      Handle to ASRC driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE ASRCOpenHandle(DWORD* pPairIndex)
{
    HANDLE hASRC;
    BOOL bRet;
    DWORD dwBytesReturned;

    if (pPairIndex == NULL)
        return INVALID_HANDLE_VALUE;

    hASRC = CreateFile( TEXT("ARC1:"), 
        GENERIC_WRITE|GENERIC_READ,
        FILE_SHARE_READ|FILE_SHARE_WRITE, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL); 

    if (hASRC == INVALID_HANDLE_VALUE)
    {
        return hASRC;
    }

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_REQUEST_PAIR, 
        NULL, 
        0, 
        LPVOID(pPairIndex), 
        sizeof(DWORD), 
        &dwBytesReturned, 
        NULL
    );

    if( !bRet ){
        CloseHandle(hASRC);
        return INVALID_HANDLE_VALUE;
    }

    return hASRC;
}

//------------------------------------------------------------------------------
//
// Function: ASRCCloseHandle
//
// This method closes a handle to the ASRC stream driver.
//
// Parameters:
//      hASRC:  Handle to close.
//      dwPairIndex:  pair index returend in ASRCOpenHandle function
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL ASRCCloseHandle(HANDLE hASRC, DWORD dwPairIndex)
{
    if (hASRC != NULL)
    {
        DeviceIoControl(
            hASRC, 
            ASRC_IOCTL_RELEASE_PAIR, 
            LPVOID(&dwPairIndex), 
            sizeof(DWORD), 
            NULL, 
            0, 
            NULL, 
            NULL
    );    
        if (!CloseHandle(hASRC))
        {
          return FALSE;
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: ASRCOpenPair
//
// This method open the asrc pair for convertion.
//
// Parameters:
//      hASRC:  Handle to close.
//      pOpenParam:  point to open param
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL ASRCOpenPair(HANDLE hASRC,PASRC_OPEN_PARAM pOpenParam )
{
    BOOL bRet;
    
    if ((hASRC == INVALID_HANDLE_VALUE) || (pOpenParam== NULL))
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_OPEN_PAIR, 
        LPVOID(pOpenParam), 
        sizeof(ASRC_OPEN_PARAM), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;
}

//------------------------------------------------------------------------------
//
// Function: ASRCClosePair
//
// This method closes the asrc pair used for convertion.
//
// Parameters:
//      hASRC:  Handle to close.
//      dwPairIndex:  pair index returend in ASRCOpenHandle function
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL ASRCClosePair(HANDLE hASRC,DWORD dwPairIndex )
{
    BOOL bRet;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_CLOSE_PAIR, 
        LPVOID(&dwPairIndex), 
        sizeof(DWORD), 
        NULL, 
        0, 
        NULL, 
        NULL
    );
    return bRet;    
}

//This function is not used
BOOL ASRCPrepareInputBuffer(HANDLE hASRC, PASRCHDR pHdrIn)
{
    BOOL bRet;

    if ((hASRC == NULL) || (pHdrIn == NULL))
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_PREPARE_INPUT_BUFFER, 
        LPVOID(pHdrIn), 
        sizeof(ASRCHDR), 
        NULL, 
        0, 
        NULL, 
        NULL
    );
    return bRet;
}

//This function is not used
BOOL ASRCPrepareOutputBuffer(HANDLE hASRC, PASRCHDR pHdrOut)
{
    BOOL bRet;

    if ((hASRC == NULL) || (pHdrOut == NULL))
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_PREPARE_OUTPUT_BUFFER, 
        LPVOID(pHdrOut), 
        sizeof(ASRCHDR), 
        NULL, 
        0, 
        NULL, 
        NULL
    );
    return bRet;
}

//This function is not used
BOOL ASRCUnprepareInputBuffer(HANDLE hASRC, PASRCHDR pHdrIn)
{
    BOOL bRet;

    if ((hASRC == NULL) || (pHdrIn == NULL))
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_UNPREPARE_INPUT_BUFFER, 
        LPVOID(pHdrIn), 
        sizeof(ASRCHDR), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;
}

//This function is not used
BOOL ASRCUnprepareOutputBuffer(HANDLE hASRC, PASRCHDR pHdrOut)
{
    BOOL bRet;

    if ((hASRC == NULL) || (pHdrOut == NULL))
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_UNPREPARE_OUTPUT_BUFFER, 
        LPVOID(pHdrOut), 
        sizeof(ASRCHDR), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;
}


//------------------------------------------------------------------------------
//
// Function: ASRCAddInputBuffer
//
// This method adds input buffer(mem->asrc).
//
// Parameters:
//      hASRC:  Handle to close.
//      pHdrIn:  point to input buffer header
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL ASRCAddInputBuffer(HANDLE hASRC, PASRCHDR pHdrIn)
{
    BOOL bRet;

    if ((hASRC == NULL) || (pHdrIn == NULL))
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_ADD_INPUT_BUFFER, 
        LPVOID(pHdrIn), 
        sizeof(ASRCHDR), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;
}

//------------------------------------------------------------------------------
//
// Function: ASRCAddOutputBuffer
//
// This method adds output buffer(asrc->mem).
//
// Parameters:
//      hASRC:  Handle to close.
//      pHdrIn:  point to output buffer header
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL ASRCAddOutputBuffer(HANDLE hASRC, PASRCHDR pHdrOut)
{
    BOOL bRet;

    if ((hASRC == NULL) || (pHdrOut == NULL))
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_ADD_OUTPUT_BUFFER, 
        LPVOID(pHdrOut), 
        sizeof(ASRCHDR), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;
}    

//------------------------------------------------------------------------------
//
// Function: ASRCStart
//
// This method starts the convertion.
//
// Parameters:
//      hASRC:  Handle to close.
//      dwPairIndex:  pair index returend in ASRCOpenHandle function
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL ASRCStart(HANDLE hASRC, DWORD dwPairIndex)
{
    BOOL bRet;

    if (hASRC == NULL)
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_START_CONVERT, 
        LPVOID(&dwPairIndex), 
        sizeof(DWORD), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;

}

//------------------------------------------------------------------------------
//
// Function: ASRCStop
//
// This method stops the convertion.
//
// Parameters:
//      hASRC:  Handle to close.
//      dwPairIndex:  pair index returend in ASRCOpenHandle function
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL ASRCStop(HANDLE hASRC, DWORD dwPairIndex)
{
    BOOL bRet;

    if (hASRC == NULL)
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_STOP_CONVERT, 
        LPVOID(&dwPairIndex), 
        sizeof(DWORD), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;

}

//This function is not used now
BOOL ASRCReset(HANDLE hASRC, DWORD dwPairIndex)
{
    BOOL bRet;

    if (hASRC == NULL)
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_RESET_PAIR, 
        LPVOID(&dwPairIndex), 
        sizeof(DWORD), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;

}

//------------------------------------------------------------------------------
//
// Function: ASRCConfig
//
// This method stops the convertion.
//
// Parameters:
//      hASRC:  Handle to close.
//      pConfigParam:  point to config param
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL ASRCConfig(HANDLE hASRC, PASRC_CONFIG_PARAM pConfigParam)
{
    BOOL bRet;

    if ((hASRC == NULL) || (pConfigParam == NULL))
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_SET_CONFIG, 
        LPVOID(pConfigParam), 
        sizeof(ASRC_CONFIG_PARAM), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;   

}

//------------------------------------------------------------------------------
//
// Function: ASRCGetCapability
//
// This method querys the capability of asrc device.
//
// Parameters:
//      hASRC:  Handle to close.
//      pCapParam:  point to cap param
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL ASRCGetCapability(HANDLE hASRC, PASRC_CAP_PARAM pCapParam)
{
    BOOL bRet;

    if ((hASRC == NULL) || (pCapParam == NULL))
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_GET_CAPALITY, 
        NULL,
        0,
        LPVOID(pCapParam), 
        sizeof(ASRC_CAP_PARAM), 
        NULL, 
        NULL
    );

    return bRet;  
}

BOOL ASRCSuspend(HANDLE hASRC, DWORD dwPairIndex)
{
    BOOL bRet;

    if (hASRC == NULL)
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_SUSPEND_CONVERT, 
        LPVOID(&dwPairIndex), 
        sizeof(DWORD), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;

}

BOOL ASRCResume(HANDLE hASRC, DWORD dwPairIndex)
{
    BOOL bRet;

    if (hASRC == NULL)
        return FALSE;

    bRet = DeviceIoControl(
        hASRC, 
        ASRC_IOCTL_RESUME_CONVERT, 
        LPVOID(&dwPairIndex), 
        sizeof(DWORD), 
        NULL, 
        0, 
        NULL, 
        NULL
    );

    return bRet;

}

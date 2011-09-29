//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#include <oal_log.h>
#pragma warning(pop)

#include "ioctl_pmic.h"

extern CRITICAL_SECTION g_oalPmicMutex;

//-----------------------------------------------------------------------------
//
//  Function: OALPmicIoctlCspiLock
//
//  Obtains a lock for the CSPI hardware so that the OAL and PMIC core driver
//  can safely share access.
//
//  Parameters:
//      code       - unused
//      pInpBuffer - unused
//      inpSize    - unused
//      pOutBuffer - unused
//      outSize    - unused
//      pOutSize   - unused
//
//  Returns:
//      TRUE - successfully acquired CSPI lock
//
//-----------------------------------------------------------------------------

BOOL OALPmicIoctlCspiLock(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize)
{
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    // Ensure that the PMIC driver has exclusive access to the CSPI bus
    // until the current CSPI transaction has been completed.
    EnterCriticalSection(&g_oalPmicMutex);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: OALPmicIoctlCspiUnlock
//
//  Release the lock previously obtained by OALPmicIoctlCspiLock.
//
//  Parameters:
//      code       - unused
//      pInpBuffer - unused
//      inpSize    - unused
//      pOutBuffer - unused
//      outSize    - unused
//      pOutSize   - unused
//
//  Returns:
//      TRUE - successfully released CSPI lock
//
//-----------------------------------------------------------------------------

BOOL OALPmicIoctlCspiUnlock(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize)
{
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    // The current CSPI transaction has been completed. Other PMIC device
    // drivers or the OAL may now access the CSPI bus.
    LeaveCriticalSection(&g_oalPmicMutex);

    return TRUE;
}

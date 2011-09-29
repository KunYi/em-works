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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File: backlight.c
//
//  Backlight driver source code
//
#include <windows.h>
#include <pm.h>
#include <ceddkex.h>
#include <ceddk.h>
#include <omap2420.h>
#include <i2c.h>

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifdef DEBUG

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INIT           DEBUGZONE(3)
#define ZONE_I2C            DEBUGZONE(6)
#define ZONE_POWER          DEBUGZONE(7)

DBGPARAM dpCurSettings = {
    L"Backlight", {
        L"Errors",      L"Warnings",    L"Function",    L"Init",
        L"Info",        L"IOCTL",       L"I2C Setup",   L"Power",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0xC000
};
#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define BKL_DEVICE_COOKIE       'bklD'
#define BKL_INSTANCE_COOKIE     'bklI'

#define I2C_DEVICE_NAME         L"I2C1:"
#define I2C_BACKLIGHT_ADDRESS   0x20
#define I2C_BACKLIGHT_ADDRSIZE  7

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    LONG instances;
    CEDEVICE_POWER_STATE ps;
} BKL_DEVICE;

typedef struct {
    DWORD cookie;
    BKL_DEVICE *pDevice;
} BKL_INSTANCE;

const GUID DEVICE_IFC_I2C_GUID;

//------------------------------------------------------------------------------
//  Local Functions

BOOL  BKL_Deinit(DWORD context);

//------------------------------------------------------------------------------
//  Global variables


BOOL BKL_SetState(BOOL bEnable)
{
    HANDLE hI2C = NULL ;
    BOOL retVal = FALSE;
    I2CTRANS sI2C_Transaction;

    DEBUGMSG(ZONE_FUNCTION, (L"+BKL_SetState(%d)\r\n", bEnable));

    // open I2C bus
    hI2C = I2COpen(I2C_DEVICE_NAME);

    if (hI2C == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_SetState: "
            L"Failed open I2C device\r\n"
        ));
        goto cleanUp;
    }

    if (!I2CSetSlaveAddress( hI2C, I2C_BACKLIGHT_ADDRSIZE, I2C_BACKLIGHT_ADDRESS) )
    {

        DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_SetState: "
            L"Failed to set backlight I2C address\r\n"
        ));
        goto cleanUp;
    }

    // Initialize I2C Transaction Structure
    ZeroMemory(&sI2C_Transaction, sizeof(sI2C_Transaction));

    // Set up an I2C Transaction to read the initial values
    // of the lines controlled by this I2C Device
    sI2C_Transaction.mClk_HL_Divisor  = I2C_CLOCK_DEFAULT;
    sI2C_Transaction.mOpCode[0]       = I2C_OPCODE_READ;
    sI2C_Transaction.mTransLen[0]     = 1;
    sI2C_Transaction.mBufferOffset[0] = 0;

    // Perform the I2C Read
    I2CTransact(hI2C, &sI2C_Transaction);

    if (sI2C_Transaction.mErrorCode != 0)
    {
        DEBUGMSG(ZONE_ERROR, (L"BKL_SetState() - "
            L"ERROR: I2C Read Transaction Failed with Error 0x%08X\r\n", sI2C_Transaction.mErrorCode
        ));
    }
    else
    {
        // Set or clear the appropriate bit.
        if (bEnable)
        {
            sI2C_Transaction.mBuffer[0] |=  (1 << 5);
        }
        else
        {
            sI2C_Transaction.mBuffer[0] &= ~(1 << 5);
        }

        // Set up an I2C Transaction to write the new value
        // and verify that it was written correctly.
        sI2C_Transaction.mOpCode[0]       = I2C_OPCODE_WRITE;
        sI2C_Transaction.mTransLen[0]     = 1;
        sI2C_Transaction.mBufferOffset[0] = 0;
        sI2C_Transaction.mOpCode[1]       = I2C_OPCODE_READ;
        sI2C_Transaction.mTransLen[1]     = 1;
        sI2C_Transaction.mBufferOffset[1] = 0;

        // Perform the Write and Verification Read
        I2CTransact(hI2C, &sI2C_Transaction);

        if (sI2C_Transaction.mErrorCode != 0)
        {
            DEBUGMSG(ZONE_ERROR, (L"BKL_SetState() - "
                L"ERROR: I2C Write/Verify Transaction Failed with Error 0x%08X\r\n", sI2C_Transaction.mErrorCode
            ));
        }
        else
        {
            retVal = TRUE;
        }
    }

cleanUp:
    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (L"-BKL_SetState(retVal = %d)\r\n", retVal));

    if (hI2C)
        I2CClose(hI2C);

    return retVal;
}



//------------------------------------------------------------------------------
//
//  Function:  BKL_Init
//
//  Called by device manager to initialize device.
//
DWORD BKL_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    DWORD rc = (DWORD)NULL;
    BKL_DEVICE *pDevice = NULL;
    DEBUGMSG(ZONE_FUNCTION, (
        L"+BKL_Init(%s, 0x%08x)\r\n", szContext, pBusContext
    ));

    // Create device structure
    pDevice = (BKL_DEVICE *)LocalAlloc(LPTR, sizeof(BKL_DEVICE));
    if (pDevice == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_Init: "
            L"Failed allocate BKL device structure\r\n"
        ));
        goto cleanUp;
    }

    // Set cookie
    pDevice->cookie = BKL_DEVICE_COOKIE;
    pDevice->ps = D0;

    // Switch LCD backlight on
    BKL_SetState(TRUE);

    // Return non-null value
    rc = (DWORD)pDevice;

cleanUp:
    if (rc == 0) BKL_Deinit((DWORD)pDevice);
    DEBUGMSG(ZONE_FUNCTION, (L"-BKL_Init(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BKL_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL BKL_Deinit(DWORD context)
{
    BOOL rc = FALSE;
    BKL_DEVICE *pDevice = (BKL_DEVICE*)context;

    DEBUGMSG(ZONE_FUNCTION, (L"+BKL_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if (pDevice == NULL || pDevice->cookie != BKL_DEVICE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: BKL_Deinit: "
            L"Incorrect context parameter\r\n"
        ));
        goto cleanUp;
    }

    // Check for open instances
    if (pDevice->instances > 0) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: BKL_Deinit: "
            L"Deinit with active instance (%d instances active)\r\n",
            pDevice->instances
        ));
        goto cleanUp;
    }

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-BKL_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BKL_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD BKL_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
    DWORD rc = (DWORD)NULL;
    BKL_DEVICE *pDevice = (BKL_DEVICE*)context;
    BKL_INSTANCE *pInstance = NULL;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+BKL_Open(0x%08x, 0x%08x, 0x%08x\r\n", context, accessCode, shareMode
    ));

    // Check if we get correct context
    if (pDevice == NULL || pDevice->cookie != BKL_DEVICE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: BKL_Open: "
            L"Incorrect context parameter\r\n"
        ));
        goto cleanUp;
    }

    // Create device structure
    pInstance = (BKL_INSTANCE*)LocalAlloc(LPTR, sizeof(BKL_INSTANCE));
    if (pInstance == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_Open: "
            L"Failed allocate BKL instance structure\r\n"
        ));
        goto cleanUp;
    }

    // Set cookie
    pInstance->cookie = BKL_INSTANCE_COOKIE;

    // Save device reference
    pInstance->pDevice = pDevice;

    // Increment number of open instances
    InterlockedIncrement(&pDevice->instances);

    // sanity check number of instances
    ASSERT(pDevice->instances > 0);

    // Done...
    rc = (DWORD)pInstance;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-BKL_Open(rc = 0x%08x)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BKL_Close
//
//  This function closes the device context.
//
BOOL BKL_Close(DWORD context)
{
    BOOL rc = FALSE;
    BKL_DEVICE *pDevice;
    BKL_INSTANCE *pInstance = (BKL_INSTANCE*)context;

    DEBUGMSG(ZONE_FUNCTION, (L"+BKL_Close(0x%08x)\r\n", context));

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != BKL_INSTANCE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: BKL_Read: "
            L"Incorrect context paramer\r\n"
        ));
        goto cleanUp;
    }

    // Get device context
    pDevice = pInstance->pDevice;

    // sanity check number of instances
    ASSERT(pDevice->instances > 0);

    // Decrement number of open instances
    InterlockedDecrement(&pDevice->instances);

    // Free instance structure
    LocalFree(pInstance);

    // Done...
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-BKL_Close(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BKL_IOControl
//
//  This function sends a command to a device.
//
BOOL BKL_IOControl(
    DWORD context, DWORD code, BYTE *pInBuffer, DWORD inSize, BYTE *pOutBuffer,
    DWORD outSize, DWORD *pOutSize
) {
    BOOL rc = FALSE;
    BKL_INSTANCE *pInstance = (BKL_INSTANCE*)context;
    BKL_DEVICE *pDevice;
    POWER_CAPABILITIES powerCap;
    CEDEVICE_POWER_STATE powerState;


    DEBUGMSG(ZONE_FUNCTION, (L"+BKL_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
    ));

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != BKL_INSTANCE_COOKIE) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_IOControl: "
            L"Incorrect context paramer\r\n"
        ));
        goto cleanUp;
    }

    // Device structure
    pDevice = pInstance->pDevice;

    switch (code) {
    // Return device specific power capabilities.
    case IOCTL_POWER_CAPABILITIES:
        // Check input parameters
        if (pOutSize != NULL) *pOutSize = sizeof(POWER_CAPABILITIES);
        if (pOutBuffer == NULL || outSize < sizeof(POWER_CAPABILITIES)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        memset(&powerCap, 0, sizeof(powerCap));
        powerCap.DeviceDx = DX_MASK(D0) | DX_MASK(D4);
        if (!CeSafeCopyMemory(pOutBuffer, &powerCap, sizeof(powerCap))) {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        rc = TRUE;
        break;

    // Request a change from one device power state to another.
    case IOCTL_POWER_SET:
        DEBUGMSG(ZONE_POWER, (L"Backlight: IOCTL_POWER_SET %d\r\n", *(CEDEVICE_POWER_STATE*)pOutBuffer ));
        // Check input parameters
        if (pOutSize != NULL)
        {
            *pOutSize = sizeof(CEDEVICE_POWER_STATE);
        }
        if (pOutBuffer == NULL || outSize < sizeof(CEDEVICE_POWER_STATE) ||
            !CeSafeCopyMemory(&powerState, pOutBuffer, sizeof(powerState)) )
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        // Check for any valid power state.
        if (!VALID_DX(powerState))
        {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_IOControl: "
                L"Invalid power state 0x%08x\r\n", powerState));
            break;
        }
        switch (powerState)
        {
        case D0:
            DEBUGMSG(ZONE_POWER, (L"-BKL_IOControl: Backlight ON\r\n"));
            // Switch LCD backlight on
            BKL_SetState(TRUE);
            pDevice->ps = D0;
            break;
         case D1:
         case D2:
         case D3:
         case D4:
            DEBUGMSG(ZONE_POWER, (L"-BKL_IOControl: Backlight OFF\r\n"));
            // Switch LCD backlight off
            BKL_SetState(FALSE);
            pDevice->ps = D4;
            break;
        }
        if (!CeSafeCopyMemory(pOutBuffer, &pDevice->ps, sizeof(pDevice->ps))) {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        rc = TRUE;
        break;

    // This is to satisfy the CETK test cases
    case IOCTL_POWER_QUERY:
        rc = TRUE;
        break;

    // Return the current device power state.
    case IOCTL_POWER_GET:
        // Check input parameters
        if (pOutSize != NULL) *pOutSize = sizeof(CEDEVICE_POWER_STATE);
        if (
            pOutBuffer == NULL || outSize < sizeof(CEDEVICE_POWER_STATE) ||
            !CeSafeCopyMemory(pOutBuffer, &pDevice->ps, sizeof(pDevice->ps))
        ) {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        rc = TRUE;
        break;
    }

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-BKL_IOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  DllMain
//
//  Standard Windows DLL entry point.
//
BOOL __stdcall DllMain(HANDLE hDLL, DWORD reason, VOID *pReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER(hDLL);
        DisableThreadLibraryCalls((HMODULE)hDLL);
        break;
    }
    return TRUE;
}

//------------------------------------------------------------------------------


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
//------------------------------------------------------------------------------
//
//  File: i2c.c
//
#pragma optimize("", off)       // debug

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <pm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <i2c.h>
#include <buses.h>
#include <omap2420.h>

//Debug the Power IOCTL's
#define PM_TRACE 0

//------------------------------------------------------------------------------
//  Local Definitions

#define I2C_DEVICE_COOKIE       'i2cD'
#define I2C_INSTANCE_COOKIE     'i2cI'

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    DWORD dwIndex;
    LONG instances;
    HANDLE hMutex;
    DWORD timeout;
} I2C_DEVICE;

typedef struct {
    DWORD cookie;
    I2C_DEVICE *pDevice;
    DWORD addrSize;
    DWORD address;
} I2C_INSTANCE;

//------------------------------------------------------------------------------
//  Local Functions

BOOL I2C_Deinit(DWORD context);
BOOL I2C_SetSlaveAddress(DWORD context, DWORD size, DWORD address);
VOID I2C_ClockOn(I2C_DEVICE *pDevice);
VOID I2C_ClockOff(I2C_DEVICE *pDevice);

//------------------------------------------------------------------------------
//  Global variables

extern DBGPARAM dpCurSettings;

const GUID DEVICE_IFC_I2C_GUID;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"Index", PARAM_DWORD, TRUE, offset(I2C_DEVICE, dwIndex),
        fieldsize(I2C_DEVICE, dwIndex), NULL
    }        
};

//------------------------------------------------------------------------------
//
//  Function:  I2C_Init
//
//  Called by device manager to initialize device.
//
DWORD I2C_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    DWORD rc = (DWORD)NULL;
    I2C_DEVICE *pDevice = NULL;

    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (
        L"+I2C_Init(%s, 0x%08x)\r\n", szContext, pBusContext
    ));

    // Create device structure
    pDevice = (I2C_DEVICE *)LocalAlloc(LPTR, sizeof(I2C_DEVICE));
    if (pDevice == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: I2C_Init: "
            L"Failed allocate I2C controller structure\r\n"
        ));
        goto cleanUp;
    }

    // Set cookie
    pDevice->cookie = I2C_DEVICE_COOKIE;

    // Initalize mutex - used to support priority inversion accross different processes
    pDevice->hMutex = CreateMutex(NULL,FALSE,TEXT("I2CMUTEX"));
    if (!pDevice->hMutex)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: I2C_Init: "
            L"Failed creation of I2C mutex handle\r\n"
        ));
        goto cleanUp;
    }

    // Read device parameters
    if (GetDeviceRegistryParams(
        szContext, pDevice, dimof(g_deviceRegParams), g_deviceRegParams
    ) != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: I2C_Init: "
            L"Failed read I2C driver registry parameters\r\n"
        ));
        goto cleanUp;
    }
 
    // Return non-null value
    rc = (DWORD)pDevice;
    
cleanUp:
    if (rc == 0) I2C_Deinit((DWORD)pDevice);
    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (L"-I2C_Init(rc = %d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL I2C_Deinit(DWORD context)
{
    BOOL rc = FALSE;
    I2C_DEVICE *pDevice = (I2C_DEVICE*)context;

    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (L"+I2C_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if (pDevice == NULL || pDevice->cookie != I2C_DEVICE_COOKIE) 
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: I2C_Deinit: "
            L"Incorrect context paramer\r\n"
        ));
        goto cleanUp;
    }

    // Check for open instances
    if (pDevice->instances > 0) 
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: I2C_Deinit: "
            L"Deinit with active instance (%d instances active)\r\n",
            pDevice->instances
        ));
        goto cleanUp;
    }

    // Delete mutex handle
    CloseHandle(pDevice->hMutex);

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (L"-I2C_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD I2C_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
    DWORD rc = (DWORD)NULL;
    I2C_DEVICE *pDevice = (I2C_DEVICE*)context;
    I2C_INSTANCE *pInstance = NULL;

    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (
        L"+I2C_Open(0x%08x, 0x%08x, 0x%08x\r\n", context, accessCode, shareMode
    ));

    // Check if we get correct context
    if (pDevice == NULL || pDevice->cookie != I2C_DEVICE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: I2C_Open: "
            L"Incorrect context parameter\r\n"
        ));
        goto cleanUp;
    }

    // Create device structure
    pInstance = (I2C_INSTANCE*)LocalAlloc(LPTR, sizeof(I2C_INSTANCE));
    if (pInstance == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: I2C_Open: "
            L"Failed allocate I2C instance structure\r\n"
        ));
        goto cleanUp;
    }

    // Set cookie
    pInstance->cookie = I2C_INSTANCE_COOKIE;

    // Save device reference
    pInstance->pDevice = pDevice;

    // Increment number of open instances
    InterlockedIncrement(&pDevice->instances);

    // sanity check number of instances
    ASSERT(pDevice->instances > 0);

    // Done...
    rc = (DWORD)pInstance;

cleanUp:
    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (L"-I2C_Open(rc = 0x%08x)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Close
//
//  This function closes the device context.
//
BOOL I2C_Close(DWORD context)
{
    BOOL rc = FALSE;
    I2C_DEVICE *pDevice;
    I2C_INSTANCE *pInstance = (I2C_INSTANCE*)context;

    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (L"+I2C_Close(0x%08x)\r\n", context));

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != I2C_INSTANCE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: I2C_Close: "
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
    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (L"-I2C_Close(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Transact
//
//  This function does an atomic set of I2C reads/writes which make up a transaction
BOOL I2C_Transact(DWORD context, I2CTRANS *pTrans)
{
    I2C_DEVICE *pDevice;
    I2C_INSTANCE *pInstance = (I2C_INSTANCE*)context;
    BOOL rc = FALSE;
    DWORD dwKernelRet;
    DWORD dwParam[2];

    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (
        L"+I2C_Transact(0x%08x, 0x%08x\r\n", context, pTrans
        ));

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != I2C_INSTANCE_COOKIE)
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: I2C_Read: "
            L"Incorrect context paramer\r\n"));
        goto clean;
    }

    pDevice = pInstance->pDevice;

    WaitForSingleObject(pDevice->hMutex,INFINITE);

    /* ok. we are the only ones doing a transaction with the kernel now 
       besides the kernel itself */
    dwParam[0] = pInstance->address;
    dwParam[1] = pInstance->addrSize;
    dwKernelRet = 0;
    if (!KernelIoControl(IOCTL_KERNELI2C_SUBMIT,
                         &dwParam,
                         sizeof(DWORD)*2,
                         pTrans,
                         sizeof(I2CTRANS),
                         &dwKernelRet))
        dwKernelRet = 0;
    if (dwKernelRet)
    {
        /* dwKernelRet holds the size of the data that came back! */
        rc = TRUE;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (L"***I2C ERROR: "
            L"KernelIoControl returned invalid I2C transaction return size.\r\n"));
        pTrans->mErrorCode = (DWORD)-1;
    }

    ReleaseMutex(pDevice->hMutex);

clean:
    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (L"-I2C_Transact()\r\n"));
    return rc;
}

DWORD I2C_Read(DWORD context, PVOID pBuffer, DWORD size)
{
    SetLastError(-1);
    return (DWORD)-1;
}

DWORD I2C_Write(DWORD context, PVOID pBuffer, DWORD size)
{
    SetLastError(-1);
    return (DWORD)-1;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_IOControl
//
//  This function sends a command to a device.
//
BOOL I2C_IOControl(
    DWORD context, DWORD dwCode, BYTE *pInBuffer, DWORD inSize, BYTE *pOutBuffer,
    DWORD outSize, DWORD *pOutSize
) {
    BOOL bRetVal = FALSE;
    I2C_DEVICE *pDevice = NULL;
    I2C_INSTANCE *pInstance = (I2C_INSTANCE*)context;
    DEVICE_IFC_I2C ifc;
    I2C_SET_SLAVE_ADDRESS *pAddress;
    I2CTRANS *pTrans;

    DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (
        L"+I2C_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, dwCode, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
    ));

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != I2C_INSTANCE_COOKIE)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: I2C_IOControl: "
            L"Incorrect context paramer\r\n" ));
        goto clean;
    }

    //Get Device
    pDevice = pInstance->pDevice;

    switch (dwCode) {
    case IOCTL_DDK_GET_DRIVER_IFC:
        // We can give interface only to our peer in device process
        if (GetCurrentProcessId() != (DWORD)GetCallerProcess()) {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: I2C_IOControl: "
                L"IOCTL_DDK_GET_DRIVER_IFC can be called only from "
                L"device process (caller process id 0x%08x)\r\n",
                GetCallerProcess()
            ));
            SetLastError(ERROR_ACCESS_DENIED);
            goto clean;
        }
        // Check input parameters
        if (pInBuffer == NULL || inSize < sizeof(GUID)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        if (IsEqualGUID(pInBuffer, &DEVICE_IFC_I2C_GUID)) {
            if (pOutSize != NULL) *pOutSize = sizeof(DEVICE_IFC_I2C);
            if (pOutBuffer == NULL || outSize < sizeof(DEVICE_IFC_I2C)) {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            ifc.context = context;
            ifc.pfnSetSlaveAddress = I2C_SetSlaveAddress;
            ifc.pfnTransact = I2C_Transact;
            if (!CeSafeCopyMemory(pOutBuffer, &ifc, sizeof(DEVICE_IFC_I2C))) {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            bRetVal = TRUE;
            break;
        }
        SetLastError(ERROR_INVALID_PARAMETER);
        break;

    case IOCTL_I2C_SET_SLAVE_ADDRESS:
        if (pOutSize != NULL)
        {
            *pOutSize = sizeof(I2C_SET_SLAVE_ADDRESS);
        }
        if (pInBuffer == NULL || inSize < sizeof(I2C_SET_SLAVE_ADDRESS)) 
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        pAddress = (I2C_SET_SLAVE_ADDRESS*)pInBuffer;
        bRetVal = I2C_SetSlaveAddress(context, pAddress->size, pAddress->address);
        break;

    case IOCTL_I2C_TRANSACT:
        //Issue a transaction using the outbuffer as input/output
        if (pOutBuffer == NULL || outSize < sizeof(I2CTRANS)) 
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        pTrans = (I2CTRANS*)pOutBuffer;
        bRetVal = I2C_Transact(context, pTrans);
        *pOutSize = sizeof(I2CTRANS);
        break;


    default:
        // power control ioctls taken out.  self-manages power
       // ERRORMSG(1, (TEXT("I2C: Unknown IOCTL_xxx(0x%0.8X) \r\n"), dwCode));
        break;
    }

clean:
    //DEBUGMSG(ZONE_I2C&&ZONE_FUNCTION, (L"-I2C_IOControl(rc = %d)\r\n", rc));
    return bRetVal;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_PowerUp
//
//  This function restores power to a device.
//
VOID I2C_PowerUp(DWORD context)
{
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_PowerDown
//
//  This function suspends power to the device.
//
void I2C_PowerDown(DWORD context)
{
}

//------------------------------------------------------------------------------

BOOL I2C_SetSlaveAddress(DWORD context, DWORD size, DWORD address)
{
    BOOL rc = FALSE;
    I2C_INSTANCE *pInstance = (I2C_INSTANCE*)context;

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != I2C_INSTANCE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: I2C_SetSlaveAddress: "
            L"Incorrect context paramer\r\n"
        ));
        goto cleanUp;
    }

    if (size != 7 && size != 10) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: I2C_SetSlaveAddress: "
            L"Incorrect address size %d (valid values are 7 & 10 bits\r\n", size
        ));
        goto cleanUp;
    }
    pInstance->addrSize = size;
    pInstance->address = address;

    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

#pragma optimize("", on)        //debug

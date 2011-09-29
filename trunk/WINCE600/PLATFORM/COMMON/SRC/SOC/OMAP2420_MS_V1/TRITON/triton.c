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
#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap2420.h>
#include <i2c.h>
#include <twl.h>
#include <bsp_menelaus.h>
#include <bsp_i2c_cfg.h>
#include "triton.h"

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"Triton (TWL)", {
        L"Errors",      L"Warnings",    L"Function",    L"Init",
        L"Info",        L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"RTC"
    },
    0x8003
};

#endif

//------------------------------------------------------------------------------
//  Global variables

static const GUID DEVICE_IFC_TWL_GUID;
static const GUID DEVICE_IFC_I2C_GUID;
//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"Irq", PARAM_DWORD, TRUE, offset(Device_t, irq),
        fieldsize(Device_t, irq), NULL
    }, {
        L"Priority256", PARAM_DWORD, FALSE, offset(Device_t, priority256),
        fieldsize(Device_t, priority256), (VOID*)100
    }, {        
        L"I2CAddress", PARAM_DWORD, FALSE, offset(Device_t, i2cAddress),
        fieldsize(Device_t, i2cAddress), (VOID*)0x2D
    }        
};

//------------------------------------------------------------------------------
// Function : GetTritonIrqStatus
//
// Return status register
//
BOOL GetTritonIrqStatus(VOID *pContext, UINT16 *pStatus)
{
    Device_t *pDevice = (Device_t*)pContext;
    I2CTRANS trans;

    ZeroMemory(&trans,sizeof(trans));
    
    trans.mClk_HL_Divisor = I2C_CLOCK_100Khz;
    
    /* write out the register address to read from */
    trans.mOpCode[0] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[0] = 0;
    trans.mTransLen[0] = 1;
    trans.mBuffer[0] = MENELAUS_INTSTATUS1_OFFSET;
    /* now read the byte at that location */
    trans.mOpCode[1] = I2C_OPCODE_READ;
    trans.mBufferOffset[1] = 0;
    trans.mTransLen[1] = 1;
    /* write out the second address to read from */
    trans.mOpCode[2] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[2] = 1;
    trans.mTransLen[2] = 1;
    trans.mBuffer[1] = MENELAUS_INTSTATUS2_OFFSET;
    /* now read back the byte at that location */
    trans.mOpCode[3] = I2C_OPCODE_READ;
    trans.mBufferOffset[3] = 1;
    trans.mTransLen[3] = 1;

    I2CTransact(pDevice->hI2C, &trans);

    if (trans.mErrorCode)
        return FALSE;

    *pStatus = (((UINT16)trans.mBuffer[1])<<8) | trans.mBuffer[0];
	
	return TRUE;
}

//------------------------------------------------------------------------------
// Function : GetTritonIrqMask
//
// Return mask register
//
BOOL GetTritonIrqMask(VOID *pContext, UINT16 *pStatus)
{
    Device_t *pDevice = (Device_t*)pContext;
    I2CTRANS trans;

    ZeroMemory(&trans,sizeof(trans));
    
    trans.mClk_HL_Divisor = I2C_CLOCK_100Khz;
    
    // write out the register address to read from 
    trans.mOpCode[0] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[0] = 0;
    trans.mTransLen[0] = 1;
    trans.mBuffer[0] = MENELAUS_INTMASK1_OFFSET;
    // now read the byte at that location 
    trans.mOpCode[1] = I2C_OPCODE_READ;
    trans.mBufferOffset[1] = 0;
    trans.mTransLen[1] = 1;
    // write out the second address to read from 
    trans.mOpCode[2] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[2] = 1;
    trans.mTransLen[2] = 1;
    trans.mBuffer[1] = MENELAUS_INTMASK2_OFFSET;
    // now read back the byte at that location 
    trans.mOpCode[3] = I2C_OPCODE_READ;
    trans.mBufferOffset[3] = 1;
    trans.mTransLen[3] = 1;

    I2CTransact(pDevice->hI2C, &trans);

    *pStatus = (((UINT16)trans.mBuffer[1])<<8) | trans.mBuffer[0];

    return (trans.mErrorCode == 0);
}

BOOL SetTritonIrqMask(VOID *pContext, UINT16 status)
{
    Device_t *pDevice = (Device_t*)pContext;
    I2CTRANS trans;
    UCHAR mask;

    ZeroMemory(&trans,sizeof(trans));
    trans.mClk_HL_Divisor = I2C_CLOCK_100Khz;
    
    mask = status & 0xff;
    if (mask) {
        // read current mask setting
        // write out the register address to read from
        trans.mOpCode[0] = I2C_OPCODE_WRITE;
        trans.mBufferOffset[0] = 0;
        trans.mTransLen[0] = 1;
        trans.mBuffer[0] = MENELAUS_INTMASK1_OFFSET;
        // now read the byte at that location 
        trans.mOpCode[1] = I2C_OPCODE_READ;
        trans.mBufferOffset[1] = 0;
        trans.mTransLen[1] = 1;

        I2CTransact(pDevice->hI2C, &trans);

        // mask the pending interrupt
        mask |= trans.mBuffer[0];
        trans.mOpCode[0] = I2C_OPCODE_WRITE;
        trans.mBufferOffset[0] = 0;
        trans.mTransLen[0] = 2;
        trans.mBuffer[0] = MENELAUS_INTMASK1_OFFSET;
        trans.mBuffer[1] = mask;

        I2CTransact(pDevice->hI2C, &trans);
    }

    mask = (status >> 8) & 0xff;
    if (mask) {
        // read current mask setting
        // write out the register address to read from
        trans.mOpCode[0] = I2C_OPCODE_WRITE;
        trans.mBufferOffset[0] = 0;
        trans.mTransLen[0] = 1;
        trans.mBuffer[0] = MENELAUS_INTMASK2_OFFSET;
        // now read the byte at that location
        trans.mOpCode[1] = I2C_OPCODE_READ;
        trans.mBufferOffset[1] = 0;
        trans.mTransLen[1] = 1;

        I2CTransact(pDevice->hI2C, &trans);

        // mask the pending interrupt
        mask |= trans.mBuffer[0];
        trans.mOpCode[0] = I2C_OPCODE_WRITE;
        trans.mBufferOffset[0] = 0;
        trans.mTransLen[0] = 2;
        trans.mBuffer[0] = MENELAUS_INTMASK2_OFFSET;
        trans.mBuffer[1] = mask;

        I2CTransact(pDevice->hI2C, &trans);
    }

    return (trans.mErrorCode == 0);
}

//------------------------------------------------------------------------------
//
BOOL
InitializeTritonInterrupts(
    Device_t *pDevice
    )
{
    // Disable all interrupts
    return SetTritonIrqMask(pDevice, 0xffff);
}

//------------------------------------------------------------------------------
//
//  Function:  TWL_Init
//
//  Called by device manager to initialize device.
//
DWORD
TWL_Init(
    LPCWSTR szContext, 
    LPCVOID pBusContext
    )
{
    DWORD rc = (DWORD)NULL;
    Device_t *pDevice = NULL;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+TWL_Init(%s, 0x%08x)\r\n", szContext, pBusContext
        ));

    // Create device structure
    pDevice = (Device_t *)LocalAlloc(LPTR, sizeof(Device_t));
    if (pDevice == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_Init: "
            L"Failed allocate TWL controller structure\r\n"
            ));
        goto cleanUp;
        }

    // Set cookie
    pDevice->cookie = TWL_DEVICE_COOKIE;

    // Initalize critical section
    InitializeCriticalSection(&pDevice->cs);

    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams
            ) != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_Init: "
            L"Failed read TWL driver registry parameters\r\n"
            ));
        goto cleanUp;
        }

    // Open parent bus
    pDevice->hI2C = I2COpen(I2C_DEVICE_NAME);
    if (pDevice->hI2C == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_Init: "
            L"Failed open I2C bus driver\r\n"
            ));
        goto cleanUp;
    }

    // Set Triton I2C address
    if (!I2CSetSlaveAddress(pDevice->hI2C, 7, I2C_MENELAUS_ADDRESS)) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_Init: "
            L"Failed set I2C bus slave address\r\n"
            ));
        goto cleanUp;
    }

    // Disable all interrupts
    if (InitializeTritonInterrupts(pDevice) == FALSE) goto cleanUp;

    pDevice->irq = IRQ_SYS_NIRQ;
    // Map interrupt
    if (!KernelIoControl(
            IOCTL_HAL_REQUEST_SYSINTR, &pDevice->irq, sizeof(pDevice->irq),
            &pDevice->sysIntr, sizeof(pDevice->sysIntr), NULL) ) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_Init: "
            L"Failed map Triton interrupt (%d)\r\n", pDevice->irq
            ));
        goto cleanUp;
    }

    // Create interrupt event
    pDevice->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pDevice->hIntrEvent == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_Init: "
            L"Failed create interrupt event\r\n"
            ));
        goto cleanUp;
    }

    // Initialize interrupt
    if (!InterruptInitialize(pDevice->sysIntr, pDevice->hIntrEvent, NULL, 0)) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: TWL_Init: "
            L"InterruptInitialize failed\r\n"
            ));
        goto cleanUp;
    }

    // Start interrupt service thread
    pDevice->intrThreadExit = FALSE;
    pDevice->hIntrThread = CreateThread(NULL, 0, TWL_IntrThread, pDevice, 0,NULL);
    if (!pDevice->hIntrThread) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: TWL_Init: "
            L"Failed create interrupt thread\r\n"
            ));
        goto cleanUp;
    }

    // Set thread priority
    CeSetThreadPriority(pDevice->hIntrThread, pDevice->priority256);

    // Return non-null value
    rc = (DWORD)pDevice;
    
cleanUp:
    if (rc == 0) TWL_Deinit((DWORD)pDevice);
    DEBUGMSG(ZONE_FUNCTION, (L"-TWL_Init(rc = %d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  TWL_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL
TWL_Deinit(
    DWORD context
    )
{
    BOOL rc = FALSE;
    Device_t *pDevice = (Device_t*)context;


    DEBUGMSG(ZONE_FUNCTION, (L"+TWL_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != TWL_DEVICE_COOKIE))
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: TWL_Deinit: "
            L"Incorrect context paramer\r\n"
            ));
        goto cleanUp;
        }

    // Close interrupt thread
    if (pDevice->hIntrThread != NULL)
        {
        // Signal stop to thread
        pDevice->intrThreadExit = TRUE;
        // Set event to wake it
        SetEvent(pDevice->hIntrEvent);
        // Wait until thread exits
        WaitForSingleObject(pDevice->hIntrThread, INFINITE);
        // Close handle
        CloseHandle(pDevice->hIntrThread);
        }

    // Disable interrupt
    if (pDevice->sysIntr != 0)
        {
        InterruptDisable(pDevice->sysIntr);
        KernelIoControl(
            IOCTL_HAL_RELEASE_SYSINTR, &pDevice->sysIntr,
            sizeof(pDevice->sysIntr), NULL, 0, NULL
            );
        }

    // Close interrupt handler
    if (pDevice->hIntrEvent != NULL) CloseHandle(pDevice->hIntrEvent);

    // Close I2C bus
    if (pDevice->hI2C != NULL) I2CClose(pDevice->hI2C);
    if (pDevice->hICX != NULL) CloseHandle(pDevice->hICX);

    // Delete critical section
    DeleteCriticalSection(&pDevice->cs);

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-TWL_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  TWL_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD
TWL_Open(
    DWORD context, 
    DWORD accessCode, 
    DWORD shareMode
    )
{
    return context;
}

//------------------------------------------------------------------------------
//
//  Function:  TWL_Close
//
//  This function closes the device context.
//
BOOL
TWL_Close(
    DWORD context
    )
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  TWL_IOControl
//
//  This function sends a command to a device.
//
BOOL
TWL_IOControl(
    DWORD context, 
    DWORD code, 
    UCHAR *pInBuffer, 
    DWORD inSize, 
    UCHAR *pOutBuffer,
    DWORD outSize, 
    DWORD *pOutSize
    )
{
    BOOL rc = FALSE;
    Device_t *pDevice = (Device_t*)context;
    DEVICE_IFC_TWL ifc;
    DWORD address, size;


    DEBUGMSG(ZONE_FUNCTION, (
        L"+TWL_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        ));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != TWL_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_IOControl: "
            L"Incorrect context paramer\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case IOCTL_DDK_GET_DRIVER_IFC:
            // We can give interface only to our peer in device process
            if (GetCurrentProcessId() != (DWORD)GetCallerProcess())
                {
                DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_IOControl: "
                    L"IOCTL_DDK_GET_DRIVER_IFC can be called only from "
                    L"device process (caller process id 0x%08x)\r\n",
                    GetCallerProcess()
                    ));
                SetLastError(ERROR_ACCESS_DENIED);
                break;
                }
            // Check input parameters
            if ((pInBuffer == NULL) || (inSize < sizeof(GUID)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            if (IsEqualGUID(pInBuffer, &DEVICE_IFC_TWL_GUID))
                {
                if (pOutSize != NULL) *pOutSize = sizeof(DEVICE_IFC_TWL);
                if (pOutBuffer == NULL || outSize < sizeof(DEVICE_IFC_TWL))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                    }
                ifc.context = context;
                ifc.pfnReadRegs = TWL_ReadRegs;
                ifc.pfnWriteRegs = TWL_WriteRegs;
                ifc.pfnSetIntrEvent = TWL_SetIntrEvent;
                ifc.pfnIntrEnable = TWL_IntrEnable;
                ifc.pfnIntrDisable = TWL_IntrDisable;
                if (!CeSafeCopyMemory(pOutBuffer, &ifc, sizeof(DEVICE_IFC_TWL)))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                    }
                rc = TRUE;
                break;
                }
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        case IOCTL_TWL_READREGS:
            if ((pInBuffer == NULL) || 
                (inSize < sizeof(IOCTL_TWL_READREGS_IN)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            address = ((IOCTL_TWL_READREGS_IN*)pInBuffer)->address;
            size = ((IOCTL_TWL_READREGS_IN*)pInBuffer)->size;
            if (pOutSize != NULL) *pOutSize = size;
            if ((pOutBuffer == NULL) || (outSize < size))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            rc = ReadRegs(pDevice, address, pOutBuffer, size);
            break;
        case IOCTL_TWL_WRITEREGS:
            if ((pInBuffer == NULL) || 
                (inSize < sizeof(IOCTL_TWL_WRITEREGS_IN)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            address = ((IOCTL_TWL_WRITEREGS_IN*)pInBuffer)->address;
            size = ((IOCTL_TWL_WRITEREGS_IN*)pInBuffer)->size;
            if (inSize < (sizeof(IOCTL_TWL_WRITEREGS_IN) + size))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            ((IOCTL_TWL_WRITEREGS_IN*)pInBuffer)++;
            rc = WriteRegs(pDevice, address, pInBuffer, size);
            break;
        }

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-TWL_IOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  TWL_InterruptThread
//
//  This function acts as the IST for the external irq.
//
DWORD
TWL_IntrThread(
    VOID *pContext
    )
{
    Device_t *pDevice = (Device_t*)pContext;
    DWORD id;
    UINT16 status;
    UINT16 currMask;

    // Loop until we are stopped...
    while (!pDevice->intrThreadExit) {
        // Wait for event
        WaitForSingleObject(pDevice->hIntrEvent, INFINITE);
        if (pDevice->intrThreadExit) break;

        // Get interrupt status register
        if (GetTritonIrqStatus(pDevice, &status) && GetTritonIrqMask(pDevice, &currMask)) {
            // Diable all interrupts
            SetTritonIrqMask(pDevice, status);

            // Process each unmasked interrupt
            status &= ~currMask;
            id = 0;
            while (status != 0) {
                if ((status & 0x0001) != 0) {
                    if (pDevice->hSetIntrEvent[id] != NULL) {
                        SetEvent(pDevice->hSetIntrEvent[id]);
                    }

                    // If this is a RTC alarm interrupt, let oal handle the interrupt.
                    if (id == TWL_INTR_RTCALM) {
                        KernelIoControl(IOCTL_HAL_RTC_ALARM, NULL, 0, NULL, 0, NULL);
                    }
                }
                status >>= 1;
                id++;
            }
        }
        // Set fake interrupt event
        if (pDevice->hSetIntrEvent[16] != NULL) {
            // Use pulse event there in case that event is already
            // signaled as associated with some interrupt...
            PulseEvent(pDevice->hSetIntrEvent[16]);
        }
        
        InterruptDone(pDevice->sysIntr);
    }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
static
BOOL
TWL_ReadRegs(
    DWORD context, 
    DWORD address,
    UCHAR *pBuffer,
    DWORD size
    )
{
    BOOL rc = FALSE;
    Device_t *pDevice = (Device_t*)context;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != TWL_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_ReadRegs: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    rc = ReadRegs(pDevice, address, pBuffer, size);
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
BOOL
TWL_WriteRegs(
    DWORD context, 
    DWORD address,
    const UCHAR *pBuffer,
    DWORD size
    )
{
    BOOL rc = FALSE;
    Device_t *pDevice = (Device_t*)context;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != TWL_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_WriteRegs: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    rc = WriteRegs(pDevice, address, pBuffer, size);
    
cleanUp:
    return rc;
}


//------------------------------------------------------------------------------
// This function allows child drivers to register event to triton driver.
// When interrupt happends, triton driver will trigger the related event.
//
static
BOOL
TWL_SetIntrEvent(
    DWORD context,
    DWORD intrId,
    HANDLE hEvent
    )
{
    BOOL rc = FALSE;
    Device_t *pDevice = (Device_t*)context;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != TWL_DEVICE_COOKIE)) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_SetIntrEvent: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
    }

    if ((intrId > 16) && (intrId != (-1))) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_SetIntrEvent: "
            L"Incorrect interrupt Id %d\r\n", intrId
            ));
        goto cleanUp;
    }

    // Common interrupt is stored in last position
    if (intrId == (-1)) intrId = 16;

    // If handle isn't NULL we set new association, 
    // otherwise we delete it....
    if (hEvent != NULL) {
        if (pDevice->hSetIntrEvent[intrId] != NULL) {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_SetIntrEvent: "
                L"Interrupt Id %d already associated with event\r\n"
                ));
            goto cleanUp;
        }
        rc = DuplicateHandle(
            GetCurrentProcess(), hEvent, GetCurrentProcess(),
            &pDevice->hSetIntrEvent[intrId], 0, FALSE, DUPLICATE_SAME_ACCESS
            );

        if (!rc) {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_SetIntrEvent: "
                L"Event handler duplication failed\r\n"
                ));
            goto cleanUp;
        }
    }
    else {
        if (pDevice->hSetIntrEvent[intrId] == NULL) {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_SetIntrEvent: "
                L"Interrupt Id %d isn't associated with event\r\n"
                ));
            goto cleanUp;
        }
        rc = CloseHandle(pDevice->hSetIntrEvent[intrId]);
        pDevice->hSetIntrEvent[intrId] = NULL;
    }
            
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
// Child driver can call this function to enable the sub interrupt.
//
static
BOOL 
TWL_IntrEnable(
    DWORD context, 
    DWORD intrId
    )
{
    BOOL rc = FALSE;
    Device_t *pDevice = (Device_t*)context;
    BYTE   mask;
    BYTE   offset;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != TWL_DEVICE_COOKIE)) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_IntrEnable: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
    }

    if (intrId > 16)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_IntrEnable: "
            L"Incorrect interrupt Id %d\r\n", intrId
            ));
        goto cleanUp;
        }

    // We have take critical section there to avoid concurrent
    // enable register modification

    if (intrId < 8) {
        offset = MENELAUS_INTMASK1_OFFSET;
    }
    else {
        offset = MENELAUS_INTMASK2_OFFSET;
        intrId -= 8;
    }

    EnterCriticalSection(&pDevice->cs);

    // Get actual mask
    if (!ReadRegs(pDevice, offset, &mask, sizeof(mask))) {
        goto cleanUp;
    }

    // Enable interrupt
    mask &= ~(1 << intrId);

    // Write it back
    if (!WriteRegs(pDevice, offset, &mask, sizeof(mask))) {
        goto cleanUp;
    }

    rc = TRUE;
    
cleanUp:
    LeaveCriticalSection(&pDevice->cs);
    return rc;
}

//------------------------------------------------------------------------------
// Child driver can call this fucntion to disable the sub interrupt.
//
static
BOOL 
TWL_IntrDisable(
    DWORD context, 
    DWORD intrId
    )
{
    BOOL rc = FALSE;
    Device_t *pDevice = (Device_t*)context;
    BYTE   mask;
    BYTE   offset;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != TWL_DEVICE_COOKIE)) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_IntrDisable: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
    }

    if (intrId > 16)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: TWL_IntrDisable: "
            L"Incorrect interrupt Id %d\r\n", intrId
            ));
        goto cleanUp;
        }
    
    if (intrId < 8)
        {
        offset = MENELAUS_INTMASK1_OFFSET;
        }
    else
        {
        offset = MENELAUS_INTMASK2_OFFSET;
        intrId -= 8;
        }

    // We have take critical section there to avoid concurrent
    // enable register modification
    EnterCriticalSection(&pDevice->cs);

    // Get actual mask
    if (!ReadRegs(pDevice, offset, &mask, sizeof(mask))) {
        goto cleanUp;
    }

    // Disable interrupt
    mask |= (1 << intrId);

    // Write it back
    if (!WriteRegs(pDevice, offset, &mask, sizeof(mask))) {
        goto cleanUp;
    }

    rc = TRUE;
    
cleanUp:
    LeaveCriticalSection(&pDevice->cs);
    return rc;
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

BOOL
ReadRegs(
    Device_t *pDevice,
    DWORD address,
    UCHAR *pBuffer,
    DWORD size
    )
{
    I2CTRANS trans;

    ZeroMemory(&trans,sizeof(trans));

    trans.mClk_HL_Divisor = I2C_CLOCK_100Khz;
    /* first write register address */
    trans.mOpCode[0] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[0] = 0;
    trans.mTransLen[0] = 1;
    trans.mBuffer[0] = (BYTE)address;
    /* then read back data from that address */
    trans.mOpCode[1] = I2C_OPCODE_READ;
    trans.mBufferOffset[1] = 0;
    trans.mTransLen[1] = 1;

    I2CTransact(pDevice->hI2C, &trans);

    *pBuffer = trans.mBuffer[0];

    return (trans.mErrorCode == 0);
}

//------------------------------------------------------------------------------

BOOL
WriteRegs(
    Device_t *pDevice,
    DWORD address,
    const UCHAR *pBuffer,
    DWORD size
    )
{
    I2CTRANS trans;

    ZeroMemory(&trans,sizeof(trans));
    
    trans.mClk_HL_Divisor = I2C_CLOCK_100Khz;

    /* just write the register # then the data in one shot */
    trans.mOpCode[0] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[0] = 0;
    trans.mTransLen[0] = 2;
    trans.mBuffer[0] = (BYTE)address;
    trans.mBuffer[1] = ((UCHAR*)pBuffer)[0];

    I2CTransact(pDevice->hI2C, &trans);

    return (trans.mErrorCode == 0);
}


//------------------------------------------------------------------------------
//
//  Function:  DllMain
//
//  Standard Windows DLL entry point.
//
BOOL
__stdcall
DllMain(
    HANDLE hDLL,
    DWORD reason,
    VOID *pReserved
    )
{
    switch (reason)
        {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER(hDLL);
            DisableThreadLibraryCalls((HMODULE)hDLL);
            break;
        }
    return TRUE;
}

//------------------------------------------------------------------------------


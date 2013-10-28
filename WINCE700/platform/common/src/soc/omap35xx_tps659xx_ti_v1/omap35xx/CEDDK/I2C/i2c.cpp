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
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File:  i2c.cpp
//
//  This file contains DDK library implementation for platform specific
//  i2c operations.
//  
#include <windows.h>
#include <types.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <i2c.h>
#include <oal_i2c.h>

#define I2C_CEDDK_COOKIE        ('i2cH')
#define ValidateI2C()           ((_bI2CFnTableInit == FALSE) ? I2CInitialize() : TRUE)

//-----------------------------------------------------------------------------
struct I2CContextEx_t : public I2CContext_t
{
    UINT                cookie;
};


//-----------------------------------------------------------------------------
static OAL_IFC_I2C      _i2cFnTable;
static BOOL             _bI2CFnTableInit = FALSE;


//-----------------------------------------------------------------------------
static
BOOL
I2CInitialize()
{
    if (_bI2CFnTableInit == FALSE)
        {
        // get clock ref counter table from kernel
        if (KernelIoControl(IOCTL_HAL_I2CCOPYFNTABLE, (void*)&_i2cFnTable,
                sizeof(OAL_IFC_I2C), NULL, 0, NULL))
            {
            _bI2CFnTableInit = TRUE;
            }
        }
    return _bI2CFnTableInit;
}



//-----------------------------------------------------------------------------
UINT 
I2CGetDeviceIdFromMembase(
    UINT memBase
    )
{
    switch (memBase)
        {
        case OMAP_I2C1_REGS_PA:
            return OMAP_DEVICE_I2C1;
            break;

        case OMAP_I2C2_REGS_PA:
            return OMAP_DEVICE_I2C2;
            break;

        case OMAP_I2C3_REGS_PA:
            return OMAP_DEVICE_I2C3;
            break;
        }

    return OMAP_DEVICE_NONE;
}
    

//-----------------------------------------------------------------------------
HANDLE 
I2COpen(
    UINT devId
    )
{
    I2CContextEx_t *pContext = NULL;

    if (ValidateI2C() == FALSE) return NULL;
    
    switch (devId)
        {
        case OMAP_DEVICE_I2C1:
        case OMAP_DEVICE_I2C2:
        case OMAP_DEVICE_I2C3:
            pContext = new I2CContextEx_t();
            if (pContext != NULL)
                {
                pContext->cookie = I2C_CEDDK_COOKIE;
                _i2cFnTable.fnI2COpen(devId, pContext);
                }
            break;
        }

    return pContext;
}


//-----------------------------------------------------------------------------
void 
I2CSetSlaveAddress(
    HANDLE      hContext,
    DWORD       slaveAddress
    )
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;

    if (pContext->cookie == I2C_CEDDK_COOKIE)
        {
        pContext->slaveAddress = slaveAddress;
        }
}

//-----------------------------------------------------------------------------
void 
I2CSetSubAddressMode(
    HANDLE      hContext,
    DWORD       subAddressMode
    )
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;

    if (pContext->cookie == I2C_CEDDK_COOKIE)
        {
        pContext->subAddressMode = subAddressMode;
        }
}

//-----------------------------------------------------------------------------
void 
I2CSetBaudIndex(
    HANDLE      hContext,
    DWORD       baudIndex
    )
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;

    if (pContext->cookie == I2C_CEDDK_COOKIE)
        {
        pContext->baudIndex = baudIndex;
        }
}

//-----------------------------------------------------------------------------
void 
I2CSetTimeout(
    HANDLE      hContext,
    DWORD       timeOut
    )
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;

    if (pContext->cookie == I2C_CEDDK_COOKIE)
        {
        pContext->timeOut = timeOut;
        }
}


//-----------------------------------------------------------------------------
void
I2CClose(
    HANDLE hContext
    )
{
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;
    if (pContext->cookie == I2C_CEDDK_COOKIE)
        {
        delete pContext;
        }
}

//-----------------------------------------------------------------------------
DWORD
I2CRead(
    HANDLE hContext, 
    DWORD subaddr,
    VOID *pBuffer, 
    DWORD size
    )
{
    DWORD result = -1;
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;
    if (pContext->cookie == I2C_CEDDK_COOKIE && ValidateI2C())
        {
#if (_WINCEOSVER<600)
        DWORD   kOldMode = SetKMode(TRUE);
#endif        

        result = _i2cFnTable.fnI2CRead(pContext, subaddr, pBuffer, size);
        
#if (_WINCEOSVER<600)
        SetKMode(kOldMode);
#endif        
        }
    return result;
}
    
//-----------------------------------------------------------------------------
DWORD 
I2CWrite(
    HANDLE hContext, 
    DWORD subaddr,
    const VOID *pBuffer, 
    DWORD size
    )
{
    DWORD result = -1;
    I2CContextEx_t *pContext = (I2CContextEx_t*)hContext;
    if (pContext->cookie == I2C_CEDDK_COOKIE && ValidateI2C())
        {
#if (_WINCEOSVER<600)
        DWORD   kOldMode = SetKMode(TRUE);
#endif        

        result = _i2cFnTable.fnI2CWrite(pContext, subaddr, (VOID*)pBuffer, size);
        
#if (_WINCEOSVER<600)
        SetKMode(kOldMode);
#endif        
        }
    return result;
}


//-----------------------------------------------------------------------------

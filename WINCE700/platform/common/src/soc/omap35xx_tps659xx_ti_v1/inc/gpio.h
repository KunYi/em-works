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
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  gpio.h
//
//  This header defines interface for GPIO device driver. This driver control
//  GPIO pins on hardware. It allows abstract GPIO interface and break up
//  physicall and logical pins. To avoid overhead involved the driver exposes
//  interface which allows obtain funtion pointers to base set/clr/get etc.
//  functions.
//
#ifndef __GPIO_H
#define __GPIO_H

#ifdef __cplusplus
extern "C" {
#endif
#include "gpio_const.h"
//------------------------------------------------------------------------------
//
//  Define:  GPIO_DEVICE_NAME
//
#define GPIO_DEVICE_NAME        L"GIO1:"

//------------------------------------------------------------------------------
//
//  Define:  gpio specific debug zones
//
#ifdef GPIO_DEBUG_ZONES
#  undef ZONE_ERROR
#  undef ZONE_WARN
#  undef ZONE_FUNCTION
#  undef ZONE_INFO

#  define ZONE_ERROR          DEBUGZONE(0)
#  define ZONE_WARN           DEBUGZONE(1)
#  define ZONE_FUNCTION       DEBUGZONE(2)
#  define ZONE_INFO           DEBUGZONE(3)
extern DBGPARAM dpCurSettings;
#endif

//------------------------------------------------------------------------------
//
//  Define:  GPIO_DIR_xxx/GPIO_INT_xxx
//
#define GPIO_DIR_OUTPUT         (0 << 0)
#define GPIO_DIR_INPUT          (1 << 0)
#define GPIO_INT_LOW_HIGH       (1 << 1)
#define GPIO_INT_HIGH_LOW       (1 << 2)
#define GPIO_INT_LOW            (1 << 3)
#define GPIO_INT_HIGH           (1 << 4)
#define GPIO_DEBOUNCE_ENABLE    (1 << 5)

#define GPIO_PULLUP_DISABLE     0
#define GPIO_PULLUP_ENABLE      1
#define GPIO_PULLDOWN_DISABLE   0
#define GPIO_PULLDOWN_ENABLE    1

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_GPIO_GUID
//
DEFINE_GUID(
    DEVICE_IFC_GPIO_GUID, 0xa0272611, 0xdea0, 0x4678,
    0xae, 0x62, 0x65, 0x61, 0x5b, 0x7d, 0x53, 0xaa
);

//------------------------------------------------------------------------------
//
//  Type: function prototype
//
//  Predefines a set of function callbacks used to manage gpio's exposed by
//  multiple silicon
//
typedef BOOL (*fnGpioInit)(LPCTSTR szContext, HANDLE *phContext, UINT *pGpioCount);
typedef BOOL (*fnGpioDeinit)(HANDLE hContext);
typedef BOOL (*fnGpioSetMode)(HANDLE hContext, UINT id, UINT mode);
typedef BOOL (*fnGpioGetMode)(HANDLE hContext, UINT id, UINT *pMode);
typedef BOOL (*fnGpioPullup)(HANDLE hContext, UINT id, UINT enable);
typedef BOOL (*fnGpioPulldown)(HANDLE hContext, UINT id, UINT enable);
typedef BOOL (*fnGpioInterruptInitialize)(HANDLE context, UINT intrID,HANDLE hEvent);
typedef BOOL (*fnGpioInterruptDone)(HANDLE hContext, UINT id);
typedef BOOL (*fnGpioInterruptDisable)(HANDLE hContext, UINT id);
typedef BOOL (*fnGpioSetBit)(HANDLE hContext, UINT id);
typedef BOOL (*fnGpioClrBit)(HANDLE hContext, UINT id);
typedef BOOL (*fnGpioGetBit)(HANDLE hContext, UINT id, UINT *pValue);
typedef void (*fnGpioPowerUp)(HANDLE hContext);
typedef void (*fnGpioPowerDown)(HANDLE hContext);
typedef BOOL (*fnGpioIoControl)(HANDLE hContext, UINT code, 
                                UCHAR *pinVal, UINT inSize, UCHAR *poutVal, 
                                UINT outSize, UINT *pOutSize);

//------------------------------------------------------------------------------
//  Type: GPIO_TABLE
//
//  Predefines structure containing gpio call back routines exposed for a
//  silicon
//
typedef struct {
    fnGpioInit      Init;
    fnGpioDeinit    Deinit;
    fnGpioSetMode   SetMode;
    fnGpioGetMode   GetMode;
    fnGpioPullup    Pullup;
    fnGpioPulldown  Pulldown;
    fnGpioInterruptInitialize   InterruptInitialize;
    fnGpioInterruptDone         InterruptDone;
    fnGpioInterruptDisable      InterruptDisable;
    fnGpioSetBit    SetBit;
    fnGpioClrBit    ClrBit;
    fnGpioGetBit    GetBit;
    fnGpioPowerUp   PowerUp;
    fnGpioPowerDown PowerDown;
    fnGpioIoControl IoControl;
} GPIO_TABLE;

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_IFC_GPIO
//
//  This structure is used to obtain GPIO interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    VOID  (*pfnSetBit)(DWORD context, DWORD id);
    VOID  (*pfnClrBit)(DWORD context, DWORD id);
    DWORD (*pfnGetBit)(DWORD context, DWORD id);
    VOID  (*pfnSetMode)(DWORD context, DWORD id, DWORD mode);
    DWORD (*pfnGetMode)(DWORD context, DWORD id);
    DWORD (*pfnPullup)(DWORD context, DWORD id, DWORD enable);
    DWORD (*pfnPulldown)(DWORD context, DWORD id, DWORD enable);
    DWORD (*pfnInterruptInitialize)(DWORD context, DWORD id, DWORD intrID, HANDLE hEvent);
    DWORD (*pfnInterruptDone)(DWORD context, DWORD id ,DWORD intrID);
    DWORD (*pfnInterruptDisable)(DWORD context, DWORD id ,DWORD intrID);
    DWORD (*pfnGetSystemIrq)(DWORD context, DWORD id);
} DEVICE_IFC_GPIO;

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_GPIO
//
//  This structure is used to store GPIO device context.
//
typedef struct {
    DEVICE_IFC_GPIO ifc;
    HANDLE hDevice;
} DEVICE_CONTEXT_GPIO;

//------------------------------------------------------------------------------

#define IOCTL_GPIO_SETBIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_CLRBIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_UPDATEBIT    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0302, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GETBIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0303, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_SETMODE      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0304, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GETMODE      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0305, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GETIRQ       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0306, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_SET_POWER_STATE  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0307, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GET_POWER_STATE  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0308, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_SET_DEBOUNCE_TIME  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0309, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GET_DEBOUNCE_TIME  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0310, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_INIT_INTERRUPT  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0311, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_ACK_INTERRUPT  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0312, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_DISABLE_INTERRUPT  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0313, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------

typedef struct {
    UINT                    gpioId;
    CEDEVICE_POWER_STATE    state;
} IOCTL_GPIO_POWER_STATE_IN;

typedef struct {
    CEDEVICE_POWER_STATE    gpioState;
    CEDEVICE_POWER_STATE    bankState;
} IOCTL_GPIO_GET_POWER_STATE_OUT;

typedef struct {
    UINT                    gpioId;
    UINT                    debounceTime;
} IOCTL_GPIO_SET_DEBOUNCE_TIME_IN;

typedef struct {
    UINT                uGpioID;
    DWORD               dwSysIntrID;
} IOCTL_GPIO_INTERRUPT_INFO,  *PIOCTL_GPIO_INTERRUPT_INFO;

typedef struct {
    UINT                uGpioID;
    DWORD               dwSysIntrID;
    HANDLE              hEvent;
} IOCTL_GPIO_INIT_INTERRUPT_INFO,  *PIOCTL_GPIO_INIT_INTERRUPT_INFO;


//------------------------------------------------------------------------------

__inline HANDLE
GPIOOpen(
    )
{
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    DEVICE_CONTEXT_GPIO *pContext = NULL;

    hDevice = CreateFile(GPIO_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
        {
        DEBUGMSG(1, (L"GPIOOpen CreateFile failed\r\n"));
        goto cleanUp;
        }

    // Allocate memory for our handler...
    pContext = (DEVICE_CONTEXT_GPIO*)LocalAlloc(
        LPTR, sizeof(DEVICE_CONTEXT_GPIO)
        );
    if (pContext == NULL)
        {
        DEBUGMSG(1, (L"GPIOOpen LocalAlloc failed\r\n"));
        CloseHandle(hDevice);
        goto cleanUp;
        }

    // Get function pointers.  If not possible (b/c of cross process calls), use IOCTLs instead
    if (!DeviceIoControl(
            hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_GPIO_GUID,
            sizeof(DEVICE_IFC_GPIO_GUID), &pContext->ifc, 
            sizeof(DEVICE_IFC_GPIO), NULL, NULL))
        {
        //  Need to use IOCTLs instead of direct function ptrs
        pContext->ifc.context = 0;
        DEBUGMSG(1, (L"GPIOOpen DeviceIoControl failed\r\n"));
        CloseHandle(hDevice);
        LocalFree(pContext);
        pContext = NULL;
        goto cleanUp;
        }

    // Save device handle
    pContext->hDevice = hDevice;

cleanUp:
    return pContext;
}

__inline VOID
GPIOClose(
    HANDLE hContext
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    if (pContext != NULL)
    {
        CloseHandle(pContext->hDevice);
        LocalFree(pContext);
    }
}

__inline VOID
GPIOSetBit(
    HANDLE hContext, 
    DWORD id
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return;
    }

    if( pContext->ifc.context )
    {
        pContext->ifc.pfnSetBit(pContext->ifc.context, id);
    }
    else
    {
        DeviceIoControl(pContext->hDevice,
                        IOCTL_GPIO_SETBIT,
                        &id,
                        sizeof(id),
                        NULL,
                        0,
                        NULL,
                        NULL );
    }
}

__inline VOID
GPIOClrBit(
    HANDLE hContext,
    DWORD id
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return;
    }

    if( pContext->ifc.context )
    {
    pContext->ifc.pfnClrBit(pContext->ifc.context, id);
}
    else
    {
        DeviceIoControl(pContext->hDevice,
                        IOCTL_GPIO_CLRBIT,
                        &id,
                        sizeof(id),
                        NULL,
                        0,
                        NULL,
                        NULL );
    }
}

__inline DWORD
GPIOGetBit(
    HANDLE hContext, 
    DWORD id
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return 0;
    }

    if( pContext->ifc.context )
    {
    return pContext->ifc.pfnGetBit(pContext->ifc.context, id);
}
    else
    {
        DWORD   dwValue = 0;

        DeviceIoControl(pContext->hDevice,
                        IOCTL_GPIO_GETBIT,
                        &id,
                        sizeof(id),
                        &dwValue,
                        sizeof(dwValue),
                        NULL,
                        NULL );

        return dwValue;
    }
}

__inline VOID
GPIOSetMode(
    HANDLE hContext, 
    DWORD id, 
    DWORD mode
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return;
    }

    if( pContext->ifc.context )
    {
    pContext->ifc.pfnSetMode(pContext->ifc.context, id, mode);
}
    else
    {
        DWORD   dwValue[2];

        dwValue[0] = id;
        dwValue[1] = mode;

        DeviceIoControl(pContext->hDevice,
                        IOCTL_GPIO_SETMODE,
                        &dwValue,
                        sizeof(dwValue),
                        NULL,
                        0,
                        NULL,
                        NULL );
    }
}

__inline DWORD
GPIOGetMode(
    HANDLE hContext, 
    DWORD id
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return 0;
    }

    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnGetMode(pContext->ifc.context, id);
    }
    else
    {
        DWORD   dwValue = 0;

        DeviceIoControl(pContext->hDevice,
                        IOCTL_GPIO_GETMODE,
                        &id,
                        sizeof(id),
                        &dwValue,
                        sizeof(dwValue),
                        NULL,
                        NULL );

        return dwValue;
    }
}



__inline DWORD
GPIOPullup(
    HANDLE hContext, 
    DWORD id,
    DWORD enable
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return 0;
    }

    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnPullup(pContext->ifc.context, id, enable);
    }
    else
    {
        return 0;
    }
}

__inline DWORD
GPIOPulldown(
    HANDLE hContext, 
    DWORD id,
    DWORD enable
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return 0;
    }

    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnPulldown(pContext->ifc.context, id, enable);
    }
    else
    {
        return 0;
    }
}

__inline DWORD
GPIOInterruptInitialize(
    HANDLE hContext,
    DWORD id,
    DWORD intrID,
    HANDLE hEvent
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return 0;
    }
    
    if (pContext->ifc.context)
    {
        return pContext->ifc.pfnInterruptInitialize(pContext->ifc.context, id,intrID , hEvent);
    }
    else
    {
        DWORD   dwRC;
        IOCTL_GPIO_INIT_INTERRUPT_INFO sInitIntrInfo = {id, intrID, hEvent};

        dwRC = DeviceIoControl(pContext->hDevice,
                        IOCTL_GPIO_INIT_INTERRUPT,
                        &sInitIntrInfo,
                        sizeof(sInitIntrInfo),
                        NULL,
                        0,
                        NULL,
                        NULL );

        return dwRC;
    }
}


__inline DWORD
GPIOInterruptDone(
    HANDLE hContext, 
    DWORD id,
    DWORD intrID
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return 0;
    }

    if (pContext->ifc.context)
    {
        return pContext->ifc.pfnInterruptDone(pContext->ifc.context, id, intrID );
    }
    else
    {
        DWORD   dwRC;
        IOCTL_GPIO_INTERRUPT_INFO sIntrInfo = {id, intrID};

        dwRC = DeviceIoControl(pContext->hDevice,
                        IOCTL_GPIO_ACK_INTERRUPT,
                        &sIntrInfo,
                        sizeof(sIntrInfo),
                        NULL,
                        0,
                        NULL,
                        NULL );

        return dwRC;
    }
}

__inline DWORD
GPIOInterruptDisable(
    HANDLE hContext, 
    DWORD id,
    DWORD intrID
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
    {
        return 0;
    }

    if (pContext->ifc.context)
    {
        return pContext->ifc.pfnInterruptDisable(pContext->ifc.context, id, intrID );
    }
    else
    {
        DWORD   dwRC;
        IOCTL_GPIO_INTERRUPT_INFO sIntrInfo = {id, intrID};

        dwRC = DeviceIoControl(pContext->hDevice,
                        IOCTL_GPIO_DISABLE_INTERRUPT,
                        &sIntrInfo,
                        sizeof(sIntrInfo),
                        NULL,
                        0,
                        NULL,
                        NULL );

        return dwRC;
    }
}




//***************************************END ADDITION********************************************


__inline DWORD
GPIOGetSystemIrq(
    HANDLE hContext, 
    DWORD id
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if ( pContext == NULL )
    {
        return 0;
    }

    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnGetSystemIrq(pContext->ifc.context, id);
    }
    else
    {
        DWORD   dwValue = 0;

        DeviceIoControl(pContext->hDevice,
                        IOCTL_GPIO_GETIRQ,
                        &id,
                        sizeof(id),
                        &dwValue,
                        sizeof(dwValue),
                        NULL,
                        NULL );

        return dwValue;
    }
}

__inline DWORD
GPIOIoControl(
    HANDLE hContext,
    DWORD  code, 
    UCHAR *pInBuffer, 
    DWORD  inSize, 
    UCHAR *pOutBuffer,
    DWORD  outSize, 
    DWORD *pOutSize,
    LPOVERLAPPED pOverlap 
    )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if ( pContext == NULL )
    {
        return 0;
    }

    return DeviceIoControl(pContext->hDevice, code, pInBuffer,
                inSize, pOutBuffer, outSize, pOutSize, pOverlap);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

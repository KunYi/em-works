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

//------------------------------------------------------------------------------
//
//  Define:  GPIO_xxx
//
//  Following defines logical GPIO pins. Mapping to physical GPIO pin is
//  defined in bsp_gpio_cfg.h file.
//
#define GPIO_OTG_TRANS                  0
#define GPIO_BATTERY_CHARGING           1
#define GPIO_KEYPAD_BACKLIGHT           2
#define GPIO_DISPLAY_BACKLIGHT          3
#define GPIO_HEADSET_DETECT             4
#define GPIO_NAND_READY                 5
#define GPIO_ROLLER_TRIG                6
#define GPIO_ROLLER_DIR                 7
#define GPIO_BATTERY_CHG_EN             8
#define GPIO_BATTERY_CHG_HI             9
#define GPIO_VIBRATOR_EN		10
#define GPIO_PNDTBUS_DATA		11
#define GPIO_PNDTBUS_CLK		12

#define GPIO_COUNT                      13

//------------------------------------------------------------------------------
//
//  Global:  g_gpioMap
//
//  Global array mapping logical to physical GPIO.
//
extern const DWORD g_gpioMap[GPIO_COUNT];

//------------------------------------------------------------------------------
//
//  Define:  GPIO_DIR_xxx/GPIO_INT_xxx
//
//  Following defines config values for GPIO pins.
//
#define GPIO_DIR_INPUT          0x01
#define GPIO_DIR_OUTPUT         0x00
#define GPIO_INT_LOW_HIGH       0x02
#define GPIO_INT_HIGH_LOW       0x00

//------------------------------------------------------------------------------
//
//  Define:  GPIO_DEVICE_NAME
//
#define GPIO_DEVICE_NAME        L"GIO1:"

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
//  Type:  DEVICE_IFC_GPIO
//
//  This structure is used to obtain GPIO interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    VOID  (*pfnSetBit)(DWORD context, DWORD id);
    VOID  (*pfnClrBit)(DWORD context, DWORD id);
    VOID  (*pfnUpdateBit)(DWORD context, DWORD id, DWORD value);
    DWORD (*pfnGetBit)(DWORD context, DWORD id);
    VOID  (*pfnSetMode)(DWORD context, DWORD id, DWORD mode);
    DWORD (*pfnGetMode)(DWORD context, DWORD id);
    DWORD (*pfnGetIrq)(DWORD context, DWORD id);
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

//------------------------------------------------------------------------------

__inline HANDLE GPIOOpen()
{
    HANDLE hDevice;
    DEVICE_CONTEXT_GPIO *pContext = NULL;

    hDevice = CreateFile(GPIO_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto clean;

    // Allocate memory for our handler...
    if ((pContext = (DEVICE_CONTEXT_GPIO *)LocalAlloc(
        LPTR, sizeof(DEVICE_CONTEXT_GPIO)
    )) == NULL) {
        CloseHandle(hDevice);
        goto clean;
    }

    // Get function pointers, fail when IOCTL isn't supported...
    if (!DeviceIoControl(
        hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_GPIO_GUID,
        sizeof(DEVICE_IFC_GPIO_GUID), &pContext->ifc, sizeof(DEVICE_IFC_GPIO),
        NULL, NULL
    )) {
        CloseHandle(hDevice);
        LocalFree(pContext);
        pContext = NULL;
        goto clean;
    }

    // Save device handle
    pContext->hDevice = hDevice;

clean:
    return pContext;
}

__inline VOID GPIOClose(HANDLE hContext)
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    CloseHandle(pContext->hDevice);
    LocalFree(pContext);
}

__inline VOID GPIOSetBit(HANDLE hContext, DWORD id)
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    pContext->ifc.pfnSetBit(pContext->ifc.context, id);
}

__inline VOID GPIOClrBit(HANDLE hContext, DWORD id)
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    pContext->ifc.pfnClrBit(pContext->ifc.context, id);
}

__inline VOID GPIOUpdateBit(HANDLE hContext, DWORD id, DWORD value)
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    pContext->ifc.pfnUpdateBit(pContext->ifc.context, id, value);
}

__inline DWORD GPIOGetBit(HANDLE hContext, DWORD id)
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    return pContext->ifc.pfnGetBit(pContext->ifc.context, id);
}

__inline VOID GPIOSetMode(HANDLE hContext, DWORD id, DWORD mode)
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    pContext->ifc.pfnSetMode(pContext->ifc.context, id, mode);
}

__inline DWORD GPIOGetMode(HANDLE hContext, DWORD id)
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    return pContext->ifc.pfnGetMode(pContext->ifc.context, id);
}

__inline DWORD GPIOGetIrq(HANDLE hContext, DWORD id)
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    return pContext->ifc.pfnGetIrq(pContext->ifc.context, id);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

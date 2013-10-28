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
//  File: omap35xx_gpio.cpp
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <gpio.h>
#include <initguid.h>
#include <bus.h>

//------------------------------------------------------------------------------
//  Local Definitions

#define GPIO_DEVICE_COOKIE      'gioO'
#define GPIO_BITS_PER_BANK      (0x1F)
#define MAX_GPIO_COUNT          (32 * OMAP_GPIO_BANK_COUNT)

#define GPIO_BANK(x)            (x >> 5)
#define GPIO_BIT(x)             (x & GPIO_BITS_PER_BANK)

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    HANDLE hParentBus;
    DWORD powerEnabled[OMAP_GPIO_BANK_COUNT];
    DWORD memBase[OMAP_GPIO_BANK_COUNT];
    DWORD memLen[OMAP_GPIO_BANK_COUNT];
    CRITICAL_SECTION pCs[OMAP_GPIO_BANK_COUNT];
    OMAP_GPIO_REGS *ppGpioRegs[OMAP_GPIO_BANK_COUNT]; //We have 6 GPIO banks
} GpioDevice_t;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
    L"OmapGpio", PARAM_MULTIDWORD, TRUE, offset(GpioDevice_t, memBase),
    fieldsize(GpioDevice_t, memBase), NULL
    }, {
    L"OmapGpioLen", PARAM_MULTIDWORD, TRUE, offset(GpioDevice_t, memLen),
    fieldsize(GpioDevice_t, memLen), NULL
    }
};

static OMAP_DEVICE s_rgGpioClocks[] = {
    OMAP_DEVICE_GPIO1,
    OMAP_DEVICE_GPIO2,
    OMAP_DEVICE_GPIO3,
    OMAP_DEVICE_GPIO4,
    OMAP_DEVICE_GPIO5,
    OMAP_DEVICE_GPIO6,
};

//------------------------------------------------------------------------------
//  Local Functions

// Init function
static BOOL OmapGpioInit(LPCTSTR szContext, HANDLE *phContext, UINT *pGpioCount);
static BOOL OmapGpioDeinit(HANDLE hContext);
static BOOL OmapGpioSetMode(HANDLE hContext, UINT id, UINT mode);
static BOOL OmapGpioGetMode(HANDLE hContext, UINT id, UINT *pMode);
static BOOL OmapGpioPullup(HANDLE hcontext, UINT id, UINT enable);
static BOOL OmapGpioPulldown(HANDLE hcontext, UINT id, UINT enable);
static BOOL OmapGpioInterruptInitialize(HANDLE hcontext,  UINT intrId, HANDLE hEvent);
static BOOL OmapGpioInterruptDone(HANDLE hcontext,    UINT intrId);
static BOOL OmapGpioInterruptDisable(HANDLE hcontext,    UINT intrId);
static BOOL OmapGpioSetBit(HANDLE hContext, UINT id);
static BOOL OmapGpioClrBit(HANDLE hContext, UINT id);
static BOOL OmapGpioGetBit(HANDLE hContext, UINT id, UINT *pValue);
static void OmapGpioPowerUp(HANDLE hContext);
static void OmapGpioPowerDown(HANDLE hContext);
static BOOL OmapGpioIoControl(HANDLE hContext, UINT code,
                               UCHAR *pinVal, UINT inSize, UCHAR *poutVal,
                               UINT outSize, UINT *pOutSize);

//------------------------------------------------------------------------------
//  exported function table
GPIO_TABLE Omap35xx_Gpio = {
    OmapGpioInit,
    OmapGpioDeinit,
    OmapGpioSetMode,
    OmapGpioGetMode,
    OmapGpioPullup,
    OmapGpioPulldown,
    OmapGpioInterruptInitialize,
    OmapGpioInterruptDone,
    OmapGpioInterruptDisable,
    OmapGpioSetBit,
    OmapGpioClrBit,
    OmapGpioGetBit,
    OmapGpioPowerUp,
    OmapGpioPowerDown,
    OmapGpioIoControl
};

//------------------------------------------------------------------------------
//
//  Function:  SetGpioBankPowerState
//
//  Called by client to initialize device.
//
static
void
SetGpioBankPowerState(
    GpioDevice_t *pDevice,
    UINT id,
    CEDEVICE_POWER_STATE state
    )
{
    // determine GPIO bank
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    UINT prevPowerState = pDevice->powerEnabled[bank];

    if (state < D3)
        {
        pDevice->powerEnabled[bank] |= bit;
        }
    else
        {
        pDevice->powerEnabled[bank] &= ~bit;
        }

    // check if power needs to be enabled/disabled for the gpio bank
    if (!prevPowerState != !pDevice->powerEnabled[bank])
        {
        if (pDevice->powerEnabled[bank] == 0)
            {
            BusClockRelease(pDevice->hParentBus, s_rgGpioClocks[bank]);
            }
        else
            {
            BusClockRequest(pDevice->hParentBus, s_rgGpioClocks[bank]);
            }
        }
}

//------------------------------------------------------------------------------
//
//  Function:  InternalSetGpioBankPowerState
//
//  Called by client to initialize device.
//
void
InternalSetGpioBankPowerState(
    GpioDevice_t *pDevice,
    UINT id,
    CEDEVICE_POWER_STATE state
    )
{
    // determine GPIO bank
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    UINT prevPowerState = pDevice->powerEnabled[bank];

    // check if power is enabled by client
    if (pDevice->powerEnabled[bank] != 0) return;

    if (state < D3)
        {
        BusClockRequest(pDevice->hParentBus, s_rgGpioClocks[bank]);
        }
    else
        {
        BusClockRelease(pDevice->hParentBus, s_rgGpioClocks[bank]);
        }
}


//------------------------------------------------------------------------------
//
//  Function:  OmapGpioInit
//
//  Called by client to initialize device.
//
BOOL
OmapGpioInit(
    LPCTSTR szContext,
    HANDLE *phContext,
    UINT   *pGpioCount
    )
{
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = NULL;
    PHYSICAL_ADDRESS pa;
    DWORD size;
    UINT8 i = 0;


    DEBUGMSG(ZONE_FUNCTION, (
        L"+OmapGpioInit(%s)\r\n", szContext
        ));

    // Create device structure
    pDevice = (GpioDevice_t *)LocalAlloc(LPTR, sizeof(GpioDevice_t));
    if (pDevice == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInit: "
            L"Failed allocate GPIO driver structure\r\n"
            ));
        goto cleanUp;
        }

    memset(pDevice, 0, sizeof(GpioDevice_t));

    // Set cookie
    pDevice->cookie = GPIO_DEVICE_COOKIE;

    // Initialize critical sections
    for (i = 0; i < OMAP_GPIO_BANK_COUNT; i++)
        InitializeCriticalSection(&pDevice->pCs[i]);

    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams
            ) != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInit: "
            L"Failed read GPIO driver registry parameters\r\n"
            ));
        goto cleanUp;
        }

    for (i = 0; i < OMAP_GPIO_BANK_COUNT; i++)
        {
        // Map GPIO registers
        pa.QuadPart = pDevice->memBase[i];
        size = pDevice->memLen[i];
        pDevice->ppGpioRegs[i] = (OMAP_GPIO_REGS*)MmMapIoSpace(pa, size, FALSE);
        if (pDevice->ppGpioRegs[i] == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInit: "
                 L"Failed map GIO%d controller registers\r\n",i
                ));
            goto cleanUp;
            }
        }

    // Open parent bus
    pDevice->hParentBus = CreateBusAccessHandle(szContext);
    if (pDevice->hParentBus == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInit: "
            L"Failed open parent bus driver\r\n"
            ));
        goto cleanUp;
        }

    // indicate gpio registers need to be saved for OFF mode
    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

    // Return non-null value
    rc = TRUE;
    *phContext = (HANDLE)pDevice;
    *pGpioCount = MAX_GPIO_COUNT;

cleanUp:
    if (rc == FALSE) OmapGpioDeinit((HANDLE)pDevice);
    DEBUGMSG(ZONE_FUNCTION, (L"-OmapGpioInit()\r\n"));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OmapGpioDeinit
//
//  Called by device manager to uninitialize device.
//
BOOL
OmapGpioDeinit(
    HANDLE context
    )
{
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;
    UINT8 i = 0;

    DEBUGMSG(ZONE_FUNCTION, (L"+OmapGpioDeinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: OmapGpioDeinit: "
            L"Incorrect context parameter\r\n"
            ));
            goto cleanUp;
        }

    // Delete critical sections
    for (i = 0; i < OMAP_GPIO_BANK_COUNT; i++)
        DeleteCriticalSection(&pDevice->pCs[i]);

    // Unmap module registers
    for (i = 0 ; i < OMAP_GPIO_BANK_COUNT; i++)
    {
       if (pDevice->ppGpioRegs[i] != NULL)
        {
            MmUnmapIoSpace((VOID*)pDevice->ppGpioRegs[i], pDevice->memLen[i]);
        }
    }

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-OmapGpioDeinit()\r\n"));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: OmapGpioSetMode
//
BOOL
OmapGpioSetMode(
    HANDLE context,
    UINT id,
    UINT mode
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioSetMode: "
            L"Incorrect context\r\n"
            ));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        UINT32 mask = 1 << (bit);
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];
        OMAP_GPIO_REGS *pGpio = pDevice->ppGpioRegs[bank];

        EnterCriticalSection(pCs);
        InternalSetGpioBankPowerState(pDevice, id, D0);

        // set gpio direction
        if ((mode & GPIO_DIR_INPUT) != 0)
            {
            SETREG32(&pGpio->OE, mask);
            }
        else
            {
            CLRREG32(&pGpio->OE, mask);
            }

        // set debounce mode
        if ((mode & GPIO_DEBOUNCE_ENABLE) != 0)
            {
            SETREG32(&pGpio->DEBOUNCENABLE, mask);
            }
        else
            {
            CLRREG32(&pGpio->DEBOUNCENABLE, mask);
            }

        // set edge/level detect mode
        if ((mode & GPIO_INT_LOW) != 0)
            {
            SETREG32(&pGpio->LEVELDETECT0, mask);
            }
        else
            {
            CLRREG32(&pGpio->LEVELDETECT0, mask);
            }

        if ((mode & GPIO_INT_HIGH) != 0)
            {
            SETREG32(&pGpio->LEVELDETECT1, mask);
            }
        else
            {
            CLRREG32(&pGpio->LEVELDETECT1, mask);
            }

        if ((mode & GPIO_INT_LOW_HIGH) != 0)
            {
            SETREG32(&pGpio->RISINGDETECT, mask);
            }
        else
            {
            CLRREG32(&pGpio->RISINGDETECT, mask);
            }

        if ((mode & GPIO_INT_HIGH_LOW) != 0)
            {
            SETREG32(&pGpio->FALLINGDETECT, mask);
            }
        else
            {
            CLRREG32(&pGpio->FALLINGDETECT, mask);
            }

        InternalSetGpioBankPowerState(pDevice, id, D4);
        LeaveCriticalSection(pCs);

        // indicate gpio registers need to be saved for OFF mode
        HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

        rc = TRUE;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioGetMode
//
BOOL
OmapGpioGetMode(
    HANDLE context,
    UINT id,
    UINT *pMode
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioGetMode: "
            L"Incorrect context\r\n"
            ));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        *pMode = 0;
        OMAP_GPIO_REGS *pGpio = pDevice->ppGpioRegs[bank];
        UINT32 mask = 1 << (bit);
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];

        EnterCriticalSection(pCs);
        InternalSetGpioBankPowerState(pDevice, id, D0);
        Sleep(2);

        // get edge mode
        if ((INREG32(&pGpio->OE) & mask) != 0)
            {
            *pMode |= GPIO_DIR_INPUT;
            }
        else
            {
            *pMode |= GPIO_DIR_OUTPUT;
            }

        // get debounce mode
        if ((INREG32(&pGpio->DEBOUNCENABLE) & mask) != 0)
            {
            *pMode |= GPIO_DEBOUNCE_ENABLE;
            }

        // get edge/level detect mode
        if ((INREG32(&pGpio->LEVELDETECT0) & mask) != 0)
            {
            *pMode |= GPIO_INT_LOW;
            }

        if ((INREG32(&pGpio->LEVELDETECT1) & mask) != 0)
            {
            *pMode |= GPIO_INT_HIGH;
            }

        if ((INREG32(&pGpio->RISINGDETECT) & mask) != 0)
            {
            *pMode |= GPIO_INT_LOW_HIGH;
            }

        if ((INREG32(&pGpio->FALLINGDETECT) & mask) != 0)
            {
            *pMode |= GPIO_INT_HIGH_LOW;
            }

        InternalSetGpioBankPowerState(pDevice, id, D4);
        LeaveCriticalSection(pCs);

        rc = TRUE;
        }

cleanUp:
    return rc;
}




//------------------------------------------------------------------------------
//
//  Function: OmapGpioPullup
//
BOOL
OmapGpioPullup(
    HANDLE context,
    UINT id,
    UINT enable
    )
{
    BOOL    rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioSetBit: Incorrect context\r\n"));
        }
    else
        {
        if (id < MAX_GPIO_COUNT)
            {
            volatile UINT *p = &pDevice->ppGpioRegs[bank]->DATAOUT;
            CRITICAL_SECTION *pCs = &pDevice->pCs[bank];
            UINT val;

            EnterCriticalSection(pCs);
            InternalSetGpioBankPowerState(pDevice, id, D0);
            if (enable)
                {
                val = 0 << (bit);
                }
            else
                {
                val = 1 << (bit);
                }
            SETREG32(p, val);
            InternalSetGpioBankPowerState(pDevice, id, D4);
            LeaveCriticalSection(pCs);

            // indicate gpio registers need to be saved for OFF mode
            HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

            rc = TRUE;
            }
        }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: OmapGpioPulldown
//
BOOL
OmapGpioPulldown(
    HANDLE context,
    UINT id,
    UINT enable
    )
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioSetIntrEvent
//
BOOL
OmapGpioInterruptInitialize(
    HANDLE context,
    UINT intrId,
    HANDLE hEvent
    )
{
    BOOL    rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioSetBit: Incorrect context\r\n"));
    }
    else
    {
        // Make sure hEvent is not NULL. We can't disassociate system interrupt
        // from an event once the association is set as the T2 interrupts do. because
        // there is not API call to do so in CE.
        if (hEvent != NULL)
        {
            rc = InterruptInitialize(intrId, hEvent, NULL, 0);
        }
    }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioIntrEnable
//
BOOL
OmapGpioInterruptDone(
    HANDLE context,
    UINT intrId
    )
{
    BOOL    rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioSetBit: Incorrect context\r\n"));
    }
    else
    {
        // Simply call the kernel to enable the interrupt via InterruptDone.
        InterruptDone((DWORD)intrId);
        rc = TRUE;
    }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: OmapGpioIntrDisable
//
BOOL
OmapGpioInterruptDisable(
    HANDLE context,
    UINT intrId
    )
{
    BOOL    rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioSetBit: Incorrect context\r\n"));
    }
    else
    {
        // Simply call the kernel to disable the interrupt via InterruptDisable.
        InterruptDisable((DWORD)intrId);
        rc = TRUE;
    }

    return rc;
}



//***************************************END ADDITION********************************************

//------------------------------------------------------------------------------
//
//  Function: OmapGpioSetBit - Set the value of the GPIO output pin
//
BOOL
OmapGpioSetBit(
    HANDLE context,
    UINT id
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioSetBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        volatile UINT *p = &pDevice->ppGpioRegs[bank]->DATAOUT;
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];

        EnterCriticalSection(pCs);
        InternalSetGpioBankPowerState(pDevice, id, D0);
        SETREG32(p, 1 << (bit));
        InternalSetGpioBankPowerState(pDevice, id, D4);
        LeaveCriticalSection(pCs);

        // indicate gpio registers need to be saved for OFF mode
        HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

        rc = TRUE;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioClrBit
//
BOOL
OmapGpioClrBit(
    HANDLE context,
    UINT id
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioClrBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        volatile UINT *p = &pDevice->ppGpioRegs[bank]->DATAOUT;
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];

        EnterCriticalSection(pCs);
        InternalSetGpioBankPowerState(pDevice, id, D0);
        CLRREG32(p, 1 << (bit));
        InternalSetGpioBankPowerState(pDevice, id, D4);
        LeaveCriticalSection(pCs);

        // indicate gpio registers need to be saved for OFF mode
        HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

        rc = TRUE;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioGetBit
//
BOOL
OmapGpioGetBit(
    HANDLE context,
    UINT id,
    UINT *pValue
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioGetBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        volatile UINT *p = &pDevice->ppGpioRegs[bank]->DATAIN;
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];
        EnterCriticalSection(pCs);
        InternalSetGpioBankPowerState(pDevice, id, D0);
        *pValue = (INREG32(p) >> (bit)) & 0x01;
        InternalSetGpioBankPowerState(pDevice, id, D4);
        LeaveCriticalSection(pCs);
        rc = TRUE;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OmapGpioIoControl
//
//  This function sends a command to a device.
//
BOOL
OmapGpioIoControl(
    HANDLE  context,
    UINT    code,
    UCHAR  *pInBuffer,
    UINT    inSize,
    UCHAR  *pOutBuffer,
    UINT    outSize,
    UINT   *pOutSize
    )
{
    UINT bit;
    UINT bank;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+OmapGpioIOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        ));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioIOControl: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case IOCTL_GPIO_SET_POWER_STATE:
            {
                if ((pInBuffer == NULL) || inSize != sizeof(IOCTL_GPIO_POWER_STATE_IN))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    RETAILMSG(ZONE_ERROR, (L"ERROR: OmapGpioIOControl: "
                            L"IOCTL_GPIO_SET_POWER_STATE - Invalid parameter\r\n"
                            ));
                    break;
                    }

                IOCTL_GPIO_POWER_STATE_IN *pPowerIn;
                pPowerIn = (IOCTL_GPIO_POWER_STATE_IN*)pInBuffer;

                bank = GPIO_BANK(pPowerIn->gpioId);

                CRITICAL_SECTION *pCs = &pDevice->pCs[bank];
                EnterCriticalSection(pCs);
                SetGpioBankPowerState(pDevice, pPowerIn->gpioId, pPowerIn->state);
                LeaveCriticalSection(pCs);
                rc = TRUE;
            }
            break;

        case IOCTL_GPIO_GET_POWER_STATE:
            {
                if ((pInBuffer == NULL) || (pOutBuffer == NULL) ||
                    inSize != sizeof(IOCTL_GPIO_POWER_STATE_IN) ||
                    outSize != sizeof(IOCTL_GPIO_GET_POWER_STATE_OUT))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    RETAILMSG(ZONE_ERROR, (L"ERROR: OmapGpioIOControl: "
                            L"IOCTL_GPIO_SET_POWER_STATE - Invalid parameter\r\n"
                            ));
                    break;
                    }

                IOCTL_GPIO_POWER_STATE_IN *pPowerIn;
                IOCTL_GPIO_GET_POWER_STATE_OUT *pPowerOut;

                pPowerIn = (IOCTL_GPIO_POWER_STATE_IN*)pInBuffer;
                pPowerOut = (IOCTL_GPIO_GET_POWER_STATE_OUT*)pOutBuffer;

                bit = GPIO_BIT(pPowerIn->gpioId);
                bank = GPIO_BANK(pPowerIn->gpioId);

                // get power state for gpio
                CRITICAL_SECTION *pCs = &pDevice->pCs[bank];
                EnterCriticalSection(pCs);
                pPowerOut->gpioState = (pDevice->powerEnabled[bank] & (1 << bit)) ? D0 : D4;
                pPowerOut->bankState = (pDevice->powerEnabled[bank]) ? D0 : D4;
                LeaveCriticalSection(pCs);

                rc = TRUE;
            }

        case IOCTL_GPIO_SET_DEBOUNCE_TIME:
            {
                if ((pInBuffer == NULL) ||
                    (inSize < sizeof(IOCTL_GPIO_SET_DEBOUNCE_TIME_IN)))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                    }

                IOCTL_GPIO_SET_DEBOUNCE_TIME_IN *pDebounce;

                pDebounce = (IOCTL_GPIO_SET_DEBOUNCE_TIME_IN*)pInBuffer;

                if (pDebounce->gpioId < MAX_GPIO_COUNT)
                    {
                    CRITICAL_SECTION *pCs;
                    bank = GPIO_BANK(pDebounce->gpioId);
                    pCs = &pDevice->pCs[bank];
                    EnterCriticalSection(pCs);
                    InternalSetGpioBankPowerState(pDevice, pDebounce->gpioId, D0);
                    OUTREG32(&pDevice->ppGpioRegs[bank]->DEBOUNCINGTIME,
                        pDebounce->debounceTime);
                    InternalSetGpioBankPowerState(pDevice, pDebounce->gpioId, D4);
                    LeaveCriticalSection(pCs);

                    // indicate gpio registers need to be saved for OFF mode
                    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

                    rc = TRUE;
                    }
            }
            break;

        case IOCTL_GPIO_GET_DEBOUNCE_TIME:
            {
                if ((pInBuffer == NULL) || (pOutBuffer == NULL) ||
                    (inSize < sizeof(UINT)) ||
                    (outSize < sizeof(UINT)))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                    }

                UINT *pId;
                UINT *pOut;

                pId = (UINT*)pInBuffer;
                pOut = (UINT*)pOutBuffer;

                if (*pId < MAX_GPIO_COUNT)
                    {

                    bank = GPIO_BANK(*pId);

                    CRITICAL_SECTION *pCs = &pDevice->pCs[bank];
                    EnterCriticalSection(pCs);
                    InternalSetGpioBankPowerState(pDevice, *pId, D0);
                    *pOut = INREG32(&pDevice->ppGpioRegs[bank]->DEBOUNCINGTIME);
                    InternalSetGpioBankPowerState(pDevice, *pId, D4);
                    LeaveCriticalSection(pCs);

                    // indicate gpio registers need to be saved for OFF mode
                    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

                    rc = TRUE;
                    }
            }
            break;
        }

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-OmapGpioIOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OmapGpioPowerUp
//
//  This function restores power to a device.
//
VOID
OmapGpioPowerUp(
    HANDLE hContext
    )
{
}

//------------------------------------------------------------------------------
//
//  Function:  OmapGpioPowerDown
//
//  This function suspends power to the device.
//
VOID
OmapGpioPowerDown(
    HANDLE hContext
    )
{
}

//------------------------------------------------------------------------------

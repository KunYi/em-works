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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_gpio.c
//
//  This file contains a DDK interface for the GPIO module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "mxarm11.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
extern PCSP_GPIO_REGS g_pGPIO[];


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioSetConfig
//
//  Sets the GPIO configration (direction and interrupt) for the specified pin.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      pin
//          [in] GPIO pin [0-31].
//
//      dir
//          [in] Direction for the pin.
//
//      intr
//          [in] Interrupt configuration for the pin.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioSetConfig(DDK_GPIO_PORT port, UINT32 pin, DDK_GPIO_DIR dir, 
    DDK_GPIO_INTR intr)
{
    BOOL rc = FALSE;
    UINT32 oldReg, newReg, icrShift, *pReg;
    
    if (!g_pGPIO[port])
    {            
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }

    if (pin >= GPIO_PINS_PER_PORT)
    {
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }
            
    // Safely update GPIO direction register
    pReg = &g_pGPIO[port]->GDIR;
    do
    {
        oldReg = INREG32((LPLONG) pReg);
        newReg = (oldReg & (~(1U << pin))) | (dir << pin);
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldReg, newReg) != oldReg);        

    if (intr != DDK_GPIO_INTR_NONE)
    {
        // Safely update GPIO direction register
        if (pin <= 15)
        {
            pReg = &g_pGPIO[port]->ICR1;
            icrShift = pin << 1;
        }
        else
        {
            pReg = &g_pGPIO[port]->ICR2;
            icrShift = (pin - 16) << 1;
        }
        do
        {
            oldReg = INREG32(pReg);
            newReg = (oldReg & (~(3U << icrShift))) | (intr << icrShift);
        } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                            oldReg, newReg) != oldReg);        
    }
    
    rc = TRUE;

cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioBindIrq
//
//  Binds the specified GPIO line with an IRQ that is registered with the
//  OAL to receive interrupts.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      pin
//          [in] GPIO pin [0-31].
//
//      irq
//          [in] Specifies the harware IRQ that will be translated into 
//          a registered SYSINTR within OEMInterruptHandler when a 
//          the configured interrupt condition for the GPIO line occurs.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioBindIrq(DDK_GPIO_PORT port, UINT32 pin, DWORD irq)
{
    BOOL rc = FALSE;
    GPIO_IRQ_MAP_PARMS irqMap;
                
    irqMap.irq = irq;
    irqMap.port = port;
    irqMap.pin = pin;

    if (!KernelIoControl(IOCTL_HAL_REQUEST_GPIO_IRQ_MAP, &irqMap, 
        sizeof(irqMap), NULL, 0, NULL))
    {
        ERRORMSG(1, (_T("IOCTL_HAL_REQUEST_GPIO_IRQ_MAP failed!\r\n")));
        goto cleanUp;
    }
    
    rc = TRUE;

cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioUnbindIrq
//
//  Unbinds the specified GPIO line from an IRQ that is registered with the
//  OAL to receive interrupts.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      pin
//          [in] GPIO pin [0-31].
//
//      irq
//          [in] Specifies the harware IRQ that will be translated into 
//          a registered SYSINTR within OEMInterruptHandler when a 
//          the configured interrupt condition for the GPIO line occurs.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioUnbindIrq(DDK_GPIO_PORT port, UINT32 pin, DWORD irq)
{
    BOOL rc = FALSE;
    GPIO_IRQ_MAP_PARMS irqMap;    
    
    irqMap.irq = irq;
    irqMap.port = port;
    irqMap.pin = pin;

    if (!KernelIoControl(IOCTL_HAL_RELEASE_GPIO_IRQ_MAP, &irqMap, 
        sizeof(irqMap), NULL, 0, NULL))
    {
        ERRORMSG(1, (_T("IOCTL_HAL_REQUEST_GPIO_IRQ_MAP failed!\r\n")));
        goto cleanUp;
    }
    
    rc = TRUE;

cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioWriteData
//
//  Writes the GPIO port data to the specified pins.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      portMask
//          [in] Bit mask for data port pins to be written.
//
//      data
//          [in] Data to be written.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//
//-----------------------------------------------------------------------------
BOOL DDKGpioWriteData(DDK_GPIO_PORT port, UINT32 portMask, UINT32 data)
{
    BOOL rc = FALSE;
    UINT32 oldReg, newReg, *pReg;
    
    if (!g_pGPIO[port])
    {            
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }
     
    // Make sure data bits fall within mask
    data &= portMask;
    
    // Safely update GPIO direction register
    pReg = &g_pGPIO[port]->DR;
    do
    {
        oldReg = INREG32(pReg);
        newReg = (oldReg & (~portMask)) | data;
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldReg, newReg) != oldReg);        

    rc = TRUE;

cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioWriteDataPin
//
//  Writes the GPIO port data to the specified pin.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      pin
//          [in] GPIO pin [0-31].
//
//      data
//           [in] Data to be written [0 or 1].
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioWriteDataPin(DDK_GPIO_PORT port, UINT32 pin, UINT32 data)
{
    if (pin >= GPIO_PINS_PER_PORT)
    {
        DBGCHK((_T("CSPDDK")), FALSE);
        return FALSE;
    }

    return DDKGpioWriteData(port, (1U << pin), (data << pin));
}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioReadData
//
//  Reads the GPIO port data for the specified pins.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      portMask
//          [in] Bit mask for data port pins to be read.
//
//      pData
//          [out] Points to buffer for data read.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioReadData(DDK_GPIO_PORT port, UINT32 portMask, UINT32 *pData)
{
    BOOL rc = FALSE;
    
    if (!g_pGPIO[port])
    {            
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }
    
    *pData = portMask & INREG32(&g_pGPIO[port]->PSR);
    
    rc = TRUE;

cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioReadDataPin
//
//  Reads the GPIO port data from the specified pin.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      pin
//          [in] GPIO pin [0-31].
//
//      pData
//          [out] Points to buffer for data read.  Data will be shifted to LSB.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioReadDataPin(DDK_GPIO_PORT port, UINT32 pin, UINT32 *pData)
{
    BOOL rc = FALSE;
    
    if (pin >= GPIO_PINS_PER_PORT)
    {
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }
    
   if (!DDKGpioReadData(port, (1U << pin), pData))
   {
        goto cleanUp;
   }

   *pData >>= pin;

   rc = TRUE;
    
cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioReadIntr
//
//  Reads the GPIO port interrupt status for the specified pins.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      portMask
//          [in] Bit mask for data port pins to be read.
//
//      pStatus
//          [out] Points to buffer for interrupt status.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioReadIntr(DDK_GPIO_PORT port, UINT32 portMask, UINT32 *pStatus)
{
    BOOL rc = FALSE;
    
    if (!g_pGPIO[port])
    {            
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }
    
    *pStatus = portMask & INREG32(&g_pGPIO[port]->ISR);
    
    rc = TRUE;

cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioReadIntrPin
//
//  Reads the GPIO port interrupt status from the specified pin.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      pin
//          [in] GPIO pin [0-31].
//
//      pStatus
//          [out] Points to buffer for interrupt status.  Status will be 
//          shifted to LSB.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioReadIntrPin(DDK_GPIO_PORT port, UINT32 pin, UINT32 *pStatus)
{
    BOOL rc = FALSE;
    
    if (pin >= GPIO_PINS_PER_PORT)
    {
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }
    
   if (!DDKGpioReadIntr(port, (1U << pin), pStatus))
   {
        goto cleanUp;
   }

   *pStatus >>= pin;

   rc = TRUE;
    
cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioClearIntrPin
//
//  Clears the GPIO interrupt status for the specified pin.
//
//  Parameters:
//      port 
//          [in] GPIO module instance.
//
//      pin
//          [in] GPIO pin [0-31].
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioClearIntrPin(DDK_GPIO_PORT port, UINT32 pin)
{
    BOOL rc = FALSE;
    
    if (!g_pGPIO[port])
    {            
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }

    if (pin >= GPIO_PINS_PER_PORT)
    {
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }
    
    // GPIO ISR is write-1-clear
    OUTREG32(&g_pGPIO[port]->ISR, (1 << pin));

    rc = TRUE;

cleanUp:

    return rc;
    
}

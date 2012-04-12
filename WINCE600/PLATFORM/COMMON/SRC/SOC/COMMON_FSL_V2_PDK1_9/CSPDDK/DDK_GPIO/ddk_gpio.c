//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "common_macros.h"
#include "common_gpio.h"
#include "common_ddk.h"
#include "common_ioctl.h"

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
extern BOOL g_SupportBothEdgeIntr;
extern DDK_GPIO_PORT g_MaxPort;
extern PCSP_GPIO_REGS g_pGPIO[];
extern PHYSICAL_ADDRESS g_PhyBaseAddress[];

//-----------------------------------------------------------------------------
// Local Variables

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
    
    if (port > g_MaxPort || !g_pGPIO[port])
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

    // Call SOC interrupt configuration function
    if (intr != DDK_GPIO_INTR_NONE)
    {   
        // For non both edge intr
        if (intr != DDK_GPIO_INTR_BOTH_EDGE)
        {    
            // Check if IP supports both-edge interrupt
            if(g_SupportBothEdgeIntr)
            {
                // Disable edge select detection
                pReg = &g_pGPIO[port]->EDGE_SEL;
                do
                {
                    oldReg = INREG32(pReg);
                    newReg = oldReg & (~(1U << pin));
                } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                                    oldReg, newReg) != oldReg);
            }

            // Safely update GPIO interrupt configuration register
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
        else
        {
            if(g_SupportBothEdgeIntr)
            {
                // Enable edge select detection which disregards ICR settings
                pReg = &g_pGPIO[port]->EDGE_SEL;
                do
                {
                    oldReg = INREG32(pReg);
                    newReg = oldReg | (1U << pin);
                } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                                oldReg, newReg) != oldReg);
            }
            else
            {
                DBGCHK((_T("CSPDDK")), FALSE);
                goto cleanUp;
            }
        }
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
    
    if (port > g_MaxPort || !g_pGPIO[port])
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
    if (port > g_MaxPort || pin >= GPIO_PINS_PER_PORT)
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
    
    if (port > g_MaxPort || !g_pGPIO[port])
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
    
    if (port > g_MaxPort || pin >= GPIO_PINS_PER_PORT)
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
    
    if (port > g_MaxPort || !g_pGPIO[port])
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
    
    if (port > g_MaxPort || pin >= GPIO_PINS_PER_PORT)
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
    
    if (port > g_MaxPort || !g_pGPIO[port])
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

//-----------------------------------------------------------------------------
//
// Function:  GpioDealloc
//
// This function deallocates the data structures required for interaction
// with the GPIO hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL GpioDealloc(void)
{
    int i;

    for(i=0; i <= g_MaxPort; i++)
    {
        // Unmap peripheral address space
        if (g_pGPIO[i] != NULL)
        {
            MmUnmapIoSpace(g_pGPIO[i], sizeof(CSP_GPIO_REGS));
            g_pGPIO[i] = NULL;
        }
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  GpioAlloc
//
// This function allocates the data structures required for interaction
// with the GPIO hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL GpioAlloc(void)
{
    BOOL rc = FALSE;
    int i;

    for(i = 0; i <= g_MaxPort; i++)
    {
        if (g_pGPIO[i] == NULL)
        {
            // Map peripheral physical address to virtual address
            g_pGPIO[i] =
                (PCSP_GPIO_REGS) MmMapIoSpace(g_PhyBaseAddress[i], sizeof(CSP_GPIO_REGS),
                                          FALSE);

            // Check if virtual mapping failed
            if (g_pGPIO[i] == NULL)
            {
                DBGCHK((_T("CSPDDK")),  FALSE);
                ERRORMSG(1, (_T("GpioAlloc:  MmMapIoSpace failed!\r\n")));
                goto cleanUp;
            }
        }
    }

    rc = TRUE;

cleanUp:

    if (!rc) GpioDealloc();

    return rc;
}

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
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: ddk_gpio.c
//
// This file contains the SoC-specific DDK interface for the GPIO module.
//
//-----------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define GPIO_MUTEX_NAME     _T("GPIO_MUTEX")

#define DDK_GPIO_LOCK()     WaitForSingleObject(g_Mutex, INFINITE)
#define DDK_GPIO_UNLOCK()   ReleaseMutex(g_Mutex)

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static PCSP_GPIO_REGS g_pGPIO = NULL;
static HANDLE g_Mutex = NULL;

//-----------------------------------------------------------------------------
// Local Functions
BOOL GpioAlloc(void);
BOOL GpioDealloc(void);

//-----------------------------------------------------------------------------
//
// Function: GpioAlloc
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
    PHYSICAL_ADDRESS phyAddr;
         
    if (g_pGPIO == NULL) {
        phyAddr.QuadPart = CSP_BASE_REG_PA_GPIO;

        // Map peripheral physical address to virtual address
        g_pGPIO = (PCSP_GPIO_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_GPIO_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pGPIO == NULL) {
            DBGCHK((_T("CSPDDK")), FALSE);
            ERRORMSG(1, (_T("GpioAlloc: MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    g_Mutex = CreateMutex(NULL, FALSE, GPIO_MUTEX_NAME);
    if (g_Mutex == NULL) {
        DBGCHK((_T("CSPDDK")), FALSE);
        ERRORMSG(1, (_T("GpioAlloc: CreateMutex failed!\r\n")));
        goto cleanUp;
    }
    
    rc = TRUE;
 
cleanUp:

    if (!rc) GpioDealloc();

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: GpioDealloc
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
    // Unmap peripheral address space
    if (g_pGPIO != NULL) {
        MmUnmapIoSpace(g_pGPIO, sizeof(CSP_GPIO_REGS));
        g_pGPIO = NULL;
    }

    CloseHandle(g_Mutex);
    
    return TRUE;    
}

//-----------------------------------------------------------------------------
//
// Function: GpioConfigPriFunc
//
// This function Configures a pin set to it's PRI function.
// Disabling will cause pins to be set as input with pullup enabled.
//
// Parameters:
//      pGpioCfg
//          [in] Pointer to configuration.
//      bEnable
//          [in] Enable or disable configuration.
//
// Returns:
//      TRUE if request was successful, otherwise returns false.
//
//-----------------------------------------------------------------------------
static BOOL GpioConfigPriFunc(DDK_GPIO_CFG *pGpioCfg, BOOL bEnable)
{
	UINT32 mask;
	GPIO_PORT port;
	UINT32 puen;

	if (!pGpioCfg)
		return FALSE;

	mask = pGpioCfg->PriConfig.PinMap;
	if (mask == 0) {
        DBGCHK((_T("CSPDDK")), FALSE);
		ERRORMSG(1, (_T("GpioConfigPriFunc: no pins specified!\r\n")));
		return FALSE;
	}
	port = pGpioCfg->PriConfig.Port;
	puen = g_pGPIO->PORT[port].PUEN;

    DDK_GPIO_LOCK();
    
	if (bEnable) {
		// Configure Primary pins
		g_pGPIO->PORT[port].GIUS &= ~mask;
		g_pGPIO->PORT[port].GPR &= ~mask;
        
		// Ensure only selected pins PUEN are changed
		puen &= ~mask;
		puen |= (pGpioCfg->PriConfig.PuenMap & mask);
		g_pGPIO->PORT[port].PUEN = puen;
	} else {
		// Configure as input pins, pullup enabled
		g_pGPIO->PORT[port].DDIR &= ~mask;	
		g_pGPIO->PORT[port].PUEN |= mask;
		g_pGPIO->PORT[port].GIUS |= mask;
	}

    DDK_GPIO_UNLOCK();
    
	return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: GpioConfigAltFunc
//
// This function Configures a pin set to it's ALT function.
// Disabling will cause pins to be set as input with pullup enabled.
//
// Parameters:
//      pGpioCfg
//          [in] Pointer to configuration.
//      bEnable
//          [in] Enable or disable configuration.
//
// Returns:
//      TRUE if request was successful, otherwise returns false.
//
//-----------------------------------------------------------------------------
static BOOL GpioConfigAltFunc(DDK_GPIO_CFG *pGpioCfg, BOOL bEnable)
{
	UINT32 mask;
	GPIO_PORT port;
	UINT32 puen;

	if (!pGpioCfg)
		return FALSE;

	mask = pGpioCfg->AltConfig.PinMap;
	if (mask == 0) {
        DBGCHK((_T("CSPDDK")), FALSE);
		ERRORMSG(1, (_T("GpioConfigAltFunc: no pins specified!\r\n")));
		return FALSE;
	}
	port = pGpioCfg->AltConfig.Port;
	puen = g_pGPIO->PORT[port].PUEN;

    DDK_GPIO_LOCK();
    
	if (bEnable) {
		// Configure as Alt pins
		g_pGPIO->PORT[port].GIUS &= ~mask;
		g_pGPIO->PORT[port].GPR |= mask;

        // Ensure only selected pins PUEN are changed
		puen &= ~mask;
		puen |= (pGpioCfg->AltConfig.PuenMap & mask);
		g_pGPIO->PORT[port].PUEN = puen;
	} else {
		// Configure as input pins, pullup enabled
		g_pGPIO->PORT[port].GIUS |= mask;
		g_pGPIO->PORT[port].DDIR &= ~mask;	
		g_pGPIO->PORT[port].PUEN |= mask;
	}

    DDK_GPIO_UNLOCK();
    
	return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: GpioConfigModuleIOFunc
//
// This function Configures a pin set to it's Module IO function.
// Disabling will cause pins to be set as input with pullup enabled.
//
// Parameters:
//      pGpioCfg
//          [in] Pointer to configuration.
//      bEnable
//          [in] Enable or disable configuration.
//
// Returns:
//      TRUE if request was successful, otherwise returns false.
//
//-----------------------------------------------------------------------------
static BOOL GpioConfigModuleIOFunc(DDK_GPIO_CFG *pGpioCfg, BOOL bEnable)
{
	GPIO_PORT port;
	GPIO_INPUT_DEST_TYPE InputDest;
	GPIO_OUTPUT_SOURCE_TYPE OutputSource;
	UINT32 inputMask;
	UINT32 outputMask;
	UINT32 mask;
	UINT32 pin;
	UINT32 puen;
	UINT32 ocr;
	UINT32 iconf;

	if (!pGpioCfg)
		return FALSE;

	inputMask = pGpioCfg->ModuleIOConfig.InputPinMap;
	outputMask = pGpioCfg->ModuleIOConfig.OutputPinMap;
	if ((inputMask == 0 && outputMask == 0) || (inputMask & outputMask) != 0) {
        DBGCHK((_T("CSPDDK")), FALSE);
		ERRORMSG(1, (_T("GpioConfigModuleIOFunc: invalid pins! input %x output %x\r\n"),
            inputMask, outputMask));
		return FALSE;
	}
	
	OutputSource = pGpioCfg->ModuleIOConfig.OutputSource;
	if (outputMask != 0 && OutputSource >= GPIO_OUTPUT_SOURCE_MAX) {
        DBGCHK((_T("CSPDDK")), FALSE);
		ERRORMSG(1, (_T("GpioConfigModuleIOFunc: invalid output! mask %x source %d\r\n"),
            outputMask, OutputSource));
		return FALSE;
	}
		
	InputDest = pGpioCfg->ModuleIOConfig.InputDest;
	if(inputMask != 0 && InputDest != GPIO_INPUT_DEST_AOUT 
        && InputDest != GPIO_INPUT_DEST_BOUT ) {
        DBGCHK((_T("CSPDDK")), FALSE);
		ERRORMSG(1, (_T("GpioConfigModuleIOFunc: invalid input! inputMask %x InputDest %d\r\n"),
            inputMask, InputDest));
		return FALSE;
	}    

	port = pGpioCfg->ModuleIOConfig.Port;
	puen = g_pGPIO->PORT[port].PUEN;
	mask = inputMask | outputMask;

    DDK_GPIO_LOCK();

	g_pGPIO->PORT[port].GIUS |= (inputMask | outputMask);
    
	if (bEnable) {
		// Configure module IO pins
		g_pGPIO->PORT[port].DDIR &= ~inputMask;
		g_pGPIO->PORT[port].DDIR |= outputMask;
		
		puen &= ~mask;
		puen |= (pGpioCfg->ModuleIOConfig.PuenMap & mask);
		g_pGPIO->PORT[port].PUEN = puen;
	} else {
		// Configure all as input pins, pullup enabled
		g_pGPIO->PORT[port].DDIR &= ~mask;
		g_pGPIO->PORT[port].PUEN |= mask;
	}

	if (((inputMask | outputMask) & 0x0000FFFF) != 0) {
		if(InputDest == GPIO_INPUT_DEST_AOUT)
			iconf = g_pGPIO->PORT[port].ICONFA1;
		else if(InputDest == GPIO_INPUT_DEST_BOUT)
			iconf = g_pGPIO->PORT[port].ICONFB1;
		ocr = g_pGPIO->PORT[port].OCR1;

		for (mask = 1, pin = 0; pin < 16; pin++, mask <<= 1) { 
			if ((mask & inputMask) != 0) {
				if (bEnable == TRUE)
					iconf &= ~(3 << (pin * 2));
				else
					iconf |= (3 << (pin * 2));
			} else if ((mask & outputMask) != 0) {
				ocr &= ~(3 << (pin * 2));
				if (bEnable == TRUE)
					ocr |= (OutputSource << (pin * 2));
			}
		}
		
		if (InputDest == GPIO_INPUT_DEST_AOUT)
			g_pGPIO->PORT[port].ICONFA1 = iconf;
		else if(InputDest == GPIO_INPUT_DEST_BOUT)
			g_pGPIO->PORT[port].ICONFB1 = iconf;
		g_pGPIO->PORT[port].OCR1 = ocr;
	}
    
	if (((inputMask | outputMask) & 0xFFFF0000) != 0) {
		if(InputDest == GPIO_INPUT_DEST_AOUT)
			iconf = g_pGPIO->PORT[port].ICONFA2;
		else if(InputDest == GPIO_INPUT_DEST_BOUT)
			iconf = g_pGPIO->PORT[port].ICONFB2;
		ocr = g_pGPIO->PORT[port].OCR2;

		for (mask = (1 << 16), pin = 0; pin < 16; pin++, mask <<= 1) { 
			if ((mask & inputMask) != 0) {
				if(bEnable == TRUE)
					iconf &= ~(3 << (pin * 2));
				else
					iconf |= (3 << (pin * 2));
			} else if ((mask & outputMask) != 0) {
				ocr &= ~(3 << (pin * 2));
				if(bEnable == TRUE)
					ocr |= (OutputSource << (pin * 2));
			}
		}
		if(InputDest == GPIO_INPUT_DEST_AOUT)
			g_pGPIO->PORT[port].ICONFA2 = iconf;
		else if(InputDest == GPIO_INPUT_DEST_BOUT)
			g_pGPIO->PORT[port].ICONFB2 = iconf;
		g_pGPIO->PORT[port].OCR2 = ocr;
	}

    DDK_GPIO_UNLOCK();
    
	return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: GpioConfigIOFunc
//
// This function Configures a pin set to it's simple IO function.
// Disabling will cause pins to be set as input with pullup enabled.
//
// Parameters:
//      pGpioCfg
//          [in] Pointer to configuration.
//      bEnable
//          [in] Enable or disable configuration.
//
// Returns:
//      TRUE if request was successful, otherwise returns false.
//
//-----------------------------------------------------------------------------
static BOOL GpioConfigIOFunc(DDK_GPIO_CFG *pGpioCfg, BOOL bEnable)
{
	GPIO_PORT port;
	UINT32 inputMask;
	UINT32 outputMask;
	UINT32 pin;
	UINT32 mask;
	UINT32 puen;
	UINT32 ocr;

	if (!pGpioCfg)
		return FALSE;

	inputMask = pGpioCfg->IOConfig.InputPinMap;
	outputMask = pGpioCfg->IOConfig.OutputPinMap;
	if (inputMask == 0 && outputMask == 0 || (inputMask & outputMask) != 0) {
        DBGCHK((_T("CSPDDK")), FALSE);
		ERRORMSG(1, (_T("GpioConfigIOFunc: invalid pins! input %x output %x\r\n"),
            inputMask, outputMask));
		return FALSE;
	}    
    
	port = pGpioCfg->IOConfig.Port;
	puen = g_pGPIO->PORT[port].PUEN;
	mask = inputMask | outputMask;

    DDK_GPIO_LOCK();
    
	g_pGPIO->PORT[port].GIUS |= mask;

	if (bEnable) {
		// Configure IO pins via pGPIO function.
		g_pGPIO->PORT[port].DDIR &= ~inputMask;
		g_pGPIO->PORT[port].DDIR |= outputMask;
		
		puen &= ~mask;
		puen |= (pGpioCfg->IOConfig.PuenMap & mask);
		g_pGPIO->PORT[port].PUEN = puen;
	} else {
		// Configure all as input pins, pullup enabled.
		g_pGPIO->PORT[port].DDIR &= ~mask;	
		g_pGPIO->PORT[port].PUEN |= mask;
	}

	if ((outputMask & 0x0000FFFF) != 0) {
		ocr = g_pGPIO->PORT[port].OCR1;

		for (mask = 1, pin = 0; pin < 16; pin++, mask <<= 1) { 
			if ((mask & outputMask) != 0) {		   
				ocr	&= ~(3 << (pin * 2));
				if (bEnable == TRUE)
					ocr |= (GPIO_OUTPUT_SOURCE_DATA << (pin * 2));
			}
		}
        
		g_pGPIO->PORT[port].OCR1 = ocr;
	}
    
	if ((outputMask & 0xFFFF0000) != 0) {
		ocr = g_pGPIO->PORT[port].OCR2;

		for (mask = (1 << 16), pin = 0; pin < 16; pin++, mask <<= 1) { 
			if ((mask & outputMask) != 0) { 
			    ocr	&= ~(3 << (pin * 2));
			    if (bEnable == TRUE)
				    ocr |= (GPIO_OUTPUT_SOURCE_DATA << (pin * 2));
		    }
		}
        
		g_pGPIO->PORT[port].OCR2 = ocr;
	}

    DDK_GPIO_UNLOCK();
    
	return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: GpioConfigIntrFunc
//
// This function Configures a pin set to it's interrupt function.
// Disabling will cause pins to be set as input with pullup enabled.
//
// Parameters:
//      pGpioCfg
//          [in] Pointer to configuration.
//      bEnable
//          [in] Enable or disable configuration.
//
// Returns:
//      TRUE if request was successful, otherwise returns false.
//
//-----------------------------------------------------------------------------
static BOOL GpioConfigIntrFunc(DDK_GPIO_CFG *pGpioCfg, BOOL bEnable)
{
	GPIO_PORT port;
	GPIO_INT_TYPE intType;
	UINT32 mask;
	UINT32 pin;
	UINT32 intMask;
	UINT32 puen;
	UINT32 icr;

	if (!pGpioCfg)
		return FALSE;

	intMask = pGpioCfg->IntrConfig.PinMap;
	intType = pGpioCfg->IntrConfig.IntType;
	if (intMask == 0 || intType >= GPIO_INT_TYPE_MAX) {
        DBGCHK((_T("CSPDDK")), FALSE);
		ERRORMSG(1, (_T("GpioConfigIntrFunc: invalid pins! input %x type %d\r\n"),
            intMask, intType));
		return FALSE;
	}

    DDK_GPIO_LOCK();

	// Input, interrupt disabled.
	port = pGpioCfg->IntrConfig.Port;
	g_pGPIO->PORT[port].IMR &= ~intMask;
	g_pGPIO->PORT[port].GIUS |= intMask;
	g_pGPIO->PORT[port].DDIR &= ~intMask;
	
	if ((intMask & 0x0000FFFF) != 0) {
		icr = g_pGPIO->PORT[port].ICR1;
		for (pin = 0, mask = 1; pin < 16; pin++, mask <<= 1) {
			if (mask & intMask) {
				icr &= ~(3 << (pin * 2));
				if (bEnable == TRUE)
					icr |= (intType << (pin * 2));
			}
		}
		g_pGPIO->PORT[port].ICR1 = icr;
	}
    
	if ((intMask & 0xFFFF0000) != 0) {
		icr = g_pGPIO->PORT[port].ICR2;
		for (pin = 0, mask = (1 << 16); pin < 16; pin++, mask <<= 1) {
			if (mask & intMask) {
				icr &= ~(3 << (pin * 2));
				if (bEnable == TRUE)
                    icr |= (intType << (pin * 2));
			}
		}
		g_pGPIO->PORT[port].ICR2 = icr;
	}
	
	// Pullup enabled if interrupt is negative level/edge triggered 
	// and pullup disabled otherwise.
	puen = g_pGPIO->PORT[port].PUEN;
	if (bEnable && (intType == GPIO_INT_TYPE_POSEDGE || intType == GPIO_INT_TYPE_POSLEVEL))
		puen &= ~intMask;
	else
		puen |= intMask;
	g_pGPIO->PORT[port].PUEN = puen;

	// Clear any pedign interrupt status
    g_pGPIO->PORT[port].ISR |= intMask;

    DDK_GPIO_UNLOCK();
        
	return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: DDKGpioEnable
//
// This function provides a mechanism for configuring a GPIO pin mapping for
// a particular peripheral.
//
// Parameters:
//      pGpioCfg
//          [in] Pointer to configuration.
//
// Returns:
//      TRUE if request was successful, otherwise returns false.
//
//------------------------------------------------------------------------------
BOOL DDKGpioEnable(DDK_GPIO_CFG *pGpioCfg)
{
    BOOL rc = FALSE;

    // Call function itself
    switch(pGpioCfg->ConfigType) {
        case GPIO_CFG_PRI:
            rc = GpioConfigPriFunc(pGpioCfg, TRUE);
            break;

        case GPIO_CFG_ALT:
            rc = GpioConfigAltFunc(pGpioCfg, TRUE);
            break;

        case GPIO_CFG_MODULEIO:
            rc = GpioConfigModuleIOFunc(pGpioCfg, TRUE);
            break;

        case GPIO_CFG_INT:
            rc = GpioConfigIntrFunc(pGpioCfg, TRUE);
            break;

        case GPIO_CFG_IO:
            rc = GpioConfigIOFunc(pGpioCfg, TRUE);
            break;

        default:
            DBGCHK((_T("CSPDDK")), FALSE);
            ERRORMSG(1, (_T("DDKEnableGpio: invalid config type!\r\n")));
            break;
    }

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: DDKGpioDisable
//
// This function provides a mechanism for resetting a GPIO pin mapping for
// a particular peripheral.
//
// Parameters:
//      pGpioCfg
//          [in] Pointer to configuration.
//
// Returns:
//      TRUE if request was successful, otherwise returns false.
//
//------------------------------------------------------------------------------
BOOL DDKGpioDisable(DDK_GPIO_CFG *pGpioCfg)
{
    BOOL rc = FALSE;

    // Call function itself
    switch (pGpioCfg->ConfigType) {
        case GPIO_CFG_PRI:
            rc = GpioConfigPriFunc(pGpioCfg, FALSE);
            break;

        case GPIO_CFG_ALT:
            rc = GpioConfigAltFunc(pGpioCfg, FALSE);
            break;

        case GPIO_CFG_MODULEIO:
            rc = GpioConfigModuleIOFunc(pGpioCfg, FALSE);
            break;

        case GPIO_CFG_INT:
            rc = GpioConfigIntrFunc(pGpioCfg, FALSE);
            break;

        case GPIO_CFG_IO:
            rc = GpioConfigIOFunc(pGpioCfg, FALSE);
            break;

        default:
            DBGCHK((_T("CSPDDK")), FALSE);
            ERRORMSG(1, (_T("OALIoCtlHalDisableGpio: invalid config type!\r\n")));
            break;
    }

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: DDKGpioWriteData
//
// Writes the GPIO port data to the specified pins.
//
// Parameters:
//      port 
//          [in] GPIO port.
//      portMask
//          [in] Bit mask for data port pins to be written.
//      data
//          [in] Data to be written.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioWriteData(GPIO_PORT port, UINT32 portMask, UINT32 data)
{
    UINT32 oldReg, newReg;
         
    // Make sure data bits fall within mask
    data &= portMask;
    
    // Safely update GPIO data register
    do {
        oldReg = g_pGPIO->PORT[port].DR;
        newReg = (oldReg & (~portMask)) | data;
    } while (InterlockedTestExchange((LPLONG)&g_pGPIO->PORT[port].DR, 
        oldReg, newReg) != oldReg);        
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DDKGpioWriteDataPin
//
// Writes the GPIO port data to the specified pin.
//
// Parameters:
//      port 
//          [in] GPIO port.
//      pin
//          [in] GPIO pin [0-31].
//      data
//          [in] Data to be written [0 or 1].
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioWriteDataPin(GPIO_PORT port, UINT32 pin, UINT32 data)
{
    if (pin >= GPIO_PINS_PER_PORT) {
        DBGCHK((_T("CSPDDK")), FALSE);
        return FALSE;
    }

    if ((data != 0) && (data != 1)) {
        DBGCHK((_T("CSPDDK")), FALSE);
        return FALSE;
    }
        
    return DDKGpioWriteData(port, GPIO_PIN_MASK(pin), (data << pin));
}

//-----------------------------------------------------------------------------
//
// Function: DDKGpioReadData
//
// Reads the GPIO port data for the specified pins.
//
// Parameters:
//      port 
//          [in] GPIO port.
//      portMask
//          [in] Bit mask for data port pins to be read.
//      pData
//          [out] Points to buffer for data read.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioReadData(GPIO_PORT port, UINT32 portMask, UINT32 *pData)
{
    *pData = portMask & g_pGPIO->PORT[port].SSR;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DDKGpioReadDataPin
//
// Reads the GPIO port data from the specified pin.
//
// Parameters:
//      port 
//          [in] GPIO port.
//      pin
//          [in] GPIO pin [0-31].
//      pData
//          [out] Points to buffer for data read.  Data will be shifted to LSB.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioReadDataPin(GPIO_PORT port, UINT32 pin, UINT32 *pData)
{    
    if (pin >= GPIO_PINS_PER_PORT) {
        DBGCHK((_T("CSPDDK")), FALSE);
        return FALSE;
    }
    
    DDKGpioReadData(port, GPIO_PIN_MASK(pin), pData);

    *pData >>= pin;
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DDKGpioReadIntr
//
// Reads the GPIO port interrupt status for the specified pins.
//
// Parameters:
//      port 
//          [in] GPIO port.
//      portMask
//          [in] Bit mask for data port pins to be read.
//      pStatus
//          [out] Points to buffer for interrupt status.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioReadIntr(GPIO_PORT port, UINT32 portMask, UINT32 *pStatus)
{
    *pStatus = portMask & g_pGPIO->PORT[port].ISR;
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DDKGpioReadIntrPin
//
// Reads the GPIO port interrupt status from the specified pin.
//
// Parameters:
//      port 
//          [in] GPIO port.
//      pin
//          [in] GPIO pin [0-31].
//      pStatus
//          [out] Points to buffer for interrupt status.  Status will be 
//          shifted to LSB.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioReadIntrPin(GPIO_PORT port, UINT32 pin, UINT32 *pStatus)
{   
    if (pin >= GPIO_PINS_PER_PORT) {
        DBGCHK((_T("CSPDDK")), FALSE);
        return FALSE;
    }
    
    DDKGpioReadIntr(port, GPIO_PIN_MASK(pin), pStatus);

    *pStatus >>= pin;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DDKGpioClearIntrPin
//
// Clears the GPIO interrupt status for the specified pin.
//
// Parameters:
//      port 
//          [in] GPIO port.
//      pin
//          [in] GPIO pin [0-31].
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioClearIntrPin(GPIO_PORT port, UINT32 pin)
{
    if (pin >= GPIO_PINS_PER_PORT) {
        DBGCHK((_T("CSPDDK")), FALSE);
        return FALSE;
    }
    
    // GPIO ISR is write-1-clear
    g_pGPIO->PORT[port].ISR = GPIO_PIN_MASK(pin);

    return TRUE;    
}

//------------------------------------------------------------------------------
//
// Function: DDKSetGpioInterruptState
//
// Unmasks/Masks the interrupt for the specified pin.
//
// Parameters:
//      port
//          [in] GPIO port.
//      pin
//          [in] GPIO pin [0-31].
//      bEnable
//          [in] TURE for Unmask, FALSE for Mask.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL DDKGpioSetIntrPin(GPIO_PORT port, UINT32 pin, BOOL bEnable)
{
    if (pin >= GPIO_PINS_PER_PORT) {
        DBGCHK((_T("CSPDDK")), FALSE);
        return FALSE;
    }

    DDK_GPIO_LOCK();
    
    if (bEnable)
        g_pGPIO->PORT[port].IMR |= GPIO_PIN_MASK(pin);
    else
        g_pGPIO->PORT[port].IMR &= ~GPIO_PIN_MASK(pin);

    // GPIO ISR is write-1-clear
    g_pGPIO->PORT[port].ISR = GPIO_PIN_MASK(pin);

    DDK_GPIO_UNLOCK();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: DDKGpioSetIntrType
//
// Sets the current type setting reading for pin specified by signal. The pin
// should have already been initialised as an interrupt pin during GPIO 
// initialisation.
//
// Parameters:
//      port
//          [in] GPIO port.
//      pin
//          [in] GPIO pin [0-31].
//      type
//          [in] Interrupt type setting.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------  
BOOL DDKGpioSetIntrType(GPIO_PORT port, UINT32 pin, 
    GPIO_INT_TYPE type)
{
    REG32 *pICR;
    UINT32 imr;

    if (pin >= GPIO_PINS_PER_PORT) {
        DBGCHK((_T("CSPDDK")), FALSE);
        return FALSE;
    }

    DDK_GPIO_LOCK();

    imr = g_pGPIO->PORT[port].IMR;
    g_pGPIO->PORT[port].IMR &= ~GPIO_PIN_MASK(pin);
    g_pGPIO->PORT[port].ISR |= GPIO_PIN_MASK(pin);

    if (pin < 16) {
        pICR = &g_pGPIO->PORT[port].ICR1;
    } else if(pin > 15 && pin < 32) {
        pin -= 16;
        pICR = &g_pGPIO->PORT[port].ICR2;
    }

    *pICR &= ~(3 << (pin * 2));
    switch (type) {
        case GPIO_INT_TYPE_POSEDGE:
            *pICR |= GPIO_INT_TYPE_POSEDGE << (pin * 2);
            break;

        case GPIO_INT_TYPE_NEGEDGE:
            *pICR |= GPIO_INT_TYPE_NEGEDGE << (pin * 2);
            break;

        case GPIO_INT_TYPE_POSLEVEL:
            *pICR |= GPIO_INT_TYPE_POSLEVEL << (pin * 2);
            break;

        case GPIO_INT_TYPE_NEGLEVEL:
            *pICR |= GPIO_INT_TYPE_NEGLEVEL << (pin * 2);
            break;
    }

    g_pGPIO->PORT[port].IMR = imr;

    DDK_GPIO_UNLOCK();

    return TRUE;
}                                        


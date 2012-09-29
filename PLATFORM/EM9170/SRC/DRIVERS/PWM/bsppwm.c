//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//-----------------------------------------------------------------------------
//
//  File:  bsppwm.c
//
//   This file implements the BSP specific functions for PWM.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)
#include "bsp.h"
#include "pwm.h"

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

// This will define the choice of clock source in PWMCR register
#define PWM_CLKSRC_VAL    PWM_PWMCR_CLKSRC_IPG_CLK_32K

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: BSPGetClockSource
//
// This function returns the BSP-specific clock
// source selection value for the PWM.
//
// Parameters:
//      None
//
// Returns:
//      The clock source for the PWM.
//
//------------------------------------------------------------------------------
UINT32 BSPGetPWMClockSource(void)
{
#ifdef	EM9170
	return PWM_PWMCR_CLKSRC_IPG_CLK;			// -> 66.5MHz
#else
    return PWM_CLKSRC_VAL;
#endif	//EM9170
}

//------------------------------------------------------------------------------
//
// Function: BSPPwmSetClockGatingMode
//
// This function enable or disable CRM clock for PWM.
//
// Parameters:
//      pContext
//          [in] pointer to the device context
//      startClocks
//          [in] boolean variable to enable or disable CRM clock
//
// Returns:
//      Return TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL BSPPwmSetClockGatingMode(PVOID pDeviceContext, BOOL startClocks)
{
    PPWM_DEVICE_CONTEXT pPwm = (PPWM_DEVICE_CONTEXT) pDeviceContext;
    DWORD dwClockIndex = 0;

    switch(pPwm->dwIndex)
    {
        case 1:
            dwClockIndex = DDK_CLOCK_GATE_INDEX_PWM1;
            break;
        case 2:
            dwClockIndex = DDK_CLOCK_GATE_INDEX_PWM2;
            break;
        case 3:
            dwClockIndex = DDK_CLOCK_GATE_INDEX_PWM3;
            break;
        case 4:
            dwClockIndex = DDK_CLOCK_GATE_INDEX_PWM4;
            break;
        default:
            dwClockIndex = 0;
            return FALSE;
            break;
    }

    if (startClocks)
    {
        DDKClockSetGatingMode(dwClockIndex, DDK_CLOCK_GATE_MODE_ENABLED);
    }
    else
    {
        DDKClockSetGatingMode(dwClockIndex, DDK_CLOCK_GATE_MODE_DISABLED);
    }
    return TRUE;

}

//
// CS&ZHL AUG-17-2011: added for EM9170
//
DWORD	 BSPPWMGetRefClk(void)
{
    UINT32								freq;
	DWORD								dwClockSource;							

	dwClockSource = BSPGetPWMClockSource();
	if(dwClockSource == PWM_PWMCR_CLKSRC_IPG_CLK)
	{
		DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER_PWM, &freq);
	}
	else
	{
		freq = 32768;
	}

	return freq;
}

//------------------------------------------------------------------------------
//
// Function: BSPPwmIomuxSetPin
//
// This function configures gpio for PWM.
//
// Parameters:
//      Index: Index of the controller
//
// Returns:
//      TRUE if successfull, FALSE if not
//------------------------------------------------------------------------------
BOOL BSPPwmIomuxSetPin(DWORD Index)
{
    BOOL bRet = TRUE;
    //None of these pins are allocated for the PWM on the iMX25 PDk, therefore they are commented out 
    //except for PWM 1 and 3 which are needed for testing.
    switch (Index)
    {
    case 1:
#ifdef	EM9170
		{
			PHYSICAL_ADDRESS phyAddr;
			PEM9K_CPLD_REGS	pISA = NULL;							// handle to access keypad port in ISA bus

			//disable ISA bus 
			phyAddr.QuadPart = CSP_BASE_MEM_PA_CS5;			//map ISA memory space
			pISA = (PEM9K_CPLD_REGS) MmMapIoSpace(phyAddr, sizeof(EM9K_CPLD_REGS), FALSE);
			if (pISA == NULL)
			{
				RETAILMSG(1, (TEXT("%s: ISA memory space mapping failed!\r\n"), __WFUNCTION__));
				return FALSE;
			}
			OUTREG8(&pISA->PWMCtrlReg, EM9K_CPLD_PWMCTRL_EN);		//PWM output enable 
			MmUnmapIoSpace(pISA, sizeof(EM9K_CPLD_REGS));					//unmap ISA memory space
		}
#endif		//EM9170
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_PWM, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        break;
    case 2:
#ifdef	EM9170
		//
		// CS&ZHL AUG-15-2011: use as PWM2 / GPIO13
		//
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_SS0, DDK_IOMUX_PIN_MUXMODE_ALT2, DDK_IOMUX_PIN_SION_REGULAR);
#endif	//EM9170
        break;
    case 3:
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_TX_CLK, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
        break;
    case 4:
        //DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_C, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
        break;
    default:
        bRet = FALSE;
        break;
    }
    
    return bRet;
}

//------------------------------------------------------------------------------
//
// Function: BSPPwmGetBaseAddress
//
// This function returns the base address of the PWM controller
//
// Parameters:
//      dwIndex
//          [IN] index of the PWM
//
// Returns:
//      Base address of PWM if success. 0 if failure
//
//------------------------------------------------------------------------------
DWORD BSPPwmGetBaseAddress(DWORD dwIndex)
{
    DWORD dwBaseAddress = 0;

    switch(dwIndex)
    {
        case 1:
            dwBaseAddress = CSP_BASE_REG_PA_PWM1;
            break;
        case 2:
            dwBaseAddress = CSP_BASE_REG_PA_PWM2;
            break;
        case 3:
            dwBaseAddress = CSP_BASE_REG_PA_PWM3;
            break;
        case 4:
            dwBaseAddress = CSP_BASE_REG_PA_PWM4;
            break;
        default:
            dwBaseAddress = 0;
            break;
    }
    return dwBaseAddress;
}

//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//-----------------------------------------------------------------------------
//
//  File:  pwmpdd.c
//
//  Implementation of PWM Driver Platform Device Driver
//  This file implements the device specific functions for PWM.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
//#include <nkintr.h>
#include <devload.h>
#include "csp.h"
#include "pwmdefs.h"
#include "pwm.h"


//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------
extern UINT32 BSPGetPWMClockSource(void);
extern DWORD BSPPWMGetRefClk(void);					// CS&ZHL AUG-16-2011: get PWM's ipg_clk
extern BOOL BSPPwmIomuxSetPin(DWORD);
extern BOOL BSPPwmSetClockGatingMode(PVOID pContext, BOOL);
extern DWORD BSPPwmGetBaseAddress(DWORD dwIndex);

BOOL PwmGetRegistryValues(PPWM_DEVICE_CONTEXT pPwm, LPCTSTR pContext);

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define PWMPRESCALER_VAL			0			// divide by 1
#define PWMWATERMARK_VAL			0			// FIFO empty flag is set when there are
																	// more than or equal to 1 empty slots in FIFO
#define PWMOUTCONFIG_VAL			0			// output pin is set at rollover and cleared at comparsion
#define PWMSAMPLEREPEAT_VAL		0			// use each sample once
#define UPPER_LIMIT							65535  // upper limit of 16bits register
//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
static DWORD g_dwPrimitiveNum[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 49};

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------
static void Reset(PVOID);

// CS&ZHL AUG-17-2011: return = PreScaler
DWORD PwmInfo2PwmSample(PPWMINFO pInfo, pPwmSample_t pSample)
{
	DWORD		dwSourceClock;
	DWORD		dwDivider;
	DWORD		dwPreScaler, dwPeriod, dwSample;
	DWORD		i1;
	DWORD		dwNumPrimitive;					

	dwSourceClock = BSPPWMGetRefClk();
	//RETAILMSG(1, (TEXT("PwmInfo2PwmSample::SourceClock = %d\r\n"), dwSourceClock));
	dwNumPrimitive = sizeof(g_dwPrimitiveNum) / sizeof(DWORD);

	//
	// CS&ZHL AUG-16-2011: compute config parameters...
	//
	dwPreScaler = 1;
	dwDivider = dwSourceClock / pInfo->dwFreq;
	while(dwDivider > 65535)
	{
		for(i1 = 0; i1 < dwNumPrimitive; i1++)
		{
			if(!(dwDivider % g_dwPrimitiveNum[i1]))
			{
				break;
			}
		}

		if( i1 < dwNumPrimitive)
		{	// find a primitive number
			dwDivider /= g_dwPrimitiveNum[i1];
			dwPreScaler *= g_dwPrimitiveNum[i1];
		}
		else
		{	// 
			dwDivider /= 2;
			dwPreScaler *= 2;
		}
		
		if(dwPreScaler > 4096)
		{
			dwPreScaler = 4096;
			dwDivider = dwSourceClock / (pInfo->dwFreq * dwPreScaler);
			break;
		}
	}

	dwPeriod   = dwDivider;
	dwSample = (dwPeriod * pInfo->dwDuty) / 100;
	pSample->period  = dwPeriod - 2;		//CS&ZHL AUG-17-2011: fPWMO = fPCLK / (PERIOD +2)
	pSample->sample = dwSample;
	pSample->duration = pInfo->dwDuration;
	//LQK Sep-9-2011
	//pInfo->dwPwmctrl[1:0]->pSample->pwmctrl[19:18]-> REG32 PWM_CR[19:18]
	pSample->pwmctrl = pInfo->dwPwmctrl<<PWM_CR_POUTC_LSH;
	RETAILMSG(1, (TEXT("PwmInfo2PwmSample::pwmctrl = %0x\r\n"), pSample->pwmctrl));
	//RETAILMSG(1, (TEXT("PwmInfo2PwmSample::PreScaler = %d, Period = %d, Sample = %d\r\n"), dwPreScaler, dwPeriod, dwSample));

	return dwPreScaler;
}

//------------------------------------------------------------------------------
//
// Function: PwmStart
//
// This function is used to start the Pwm. It will set the enable
// bit in pwm control register. This function should be called to
// enable the pwm after configuring the pwm.
//
// Parameters:
//      pDeviceContext 
//          [IN] pointer to the device context.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PwmStart(PVOID pDeviceContext)
{
    PPWM_DEVICE_CONTEXT pPwm = (PPWM_DEVICE_CONTEXT) pDeviceContext;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PwmStart\r\n")));

    // Enable the pwm
    INSREG32(&pPwm->pPwmRegs->PWM_CR, CSP_BITFMASK(PWM_CR_EN), CSP_BITFVAL(PWM_CR_EN, PWM_CR_EN_ENABLE));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmStart\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: PwmStop
//
// This function stops the Pwm. It will
// clear the enable bit in Pwm control register.
//
// Parameters:
//     pDeviceContext 
//          [IN] pointer to the device context.
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PwmStop(PVOID pDeviceContext)
{
    PPWM_DEVICE_CONTEXT pPwm = (PPWM_DEVICE_CONTEXT) pDeviceContext;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PwmStop\r\n")));
    // Disable the pwm
	OUTREG32( &pPwm->pPwmRegs->PWM_CR, INREG32(&pPwm->pPwmRegs->PWM_CR)|0xc0000 );
    INSREG32(&pPwm->pPwmRegs->PWM_CR, CSP_BITFMASK(PWM_CR_EN),CSP_BITFVAL(PWM_CR_EN, PWM_CR_EN_DISABLE));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmStop\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: PwmPlaySample
//
// This function is used to play the pwm sample
//
// Parameters:
//      pDeviceContext 
//          [IN] pointer to the device context
//
//      SampleBuffer
//          [in] pwm sample conform to pwmSample_t structure
//
//      SampleSize
//          [in] Size of the pwmSample_t structure
// Returns:
//      TRUE - If success.
//      FALSE - If failed.
//
//------------------------------------------------------------------------------
BOOL PwmPlaySample(PVOID pDeviceContext, LPCVOID SampleBuffer, int SampleSize )
{
    PPWM_DEVICE_CONTEXT pPwm = (PPWM_DEVICE_CONTEXT) pDeviceContext;
    pPwmSample_t PwmSampleBuffer;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PwmPlaySample\r\n")));
    
    PwmSampleBuffer = (pPwmSample_t)SampleBuffer;
    // check for the limits of the sample and period
    if(PwmSampleBuffer->sample > UPPER_LIMIT || PwmSampleBuffer->period > UPPER_LIMIT)
        return FALSE;
    if(SampleSize != sizeof(PwmSample_t))
        return FALSE;
    EnterCriticalSection(&pPwm->hPwmLock);

	//
	// CS&ZHL AUG-17-2011: setup PWM control parameters if freq is changed
	//
	if(pPwm->dwLastPreScaler != pPwm->dwPreScaler)
	{
		// change precsaler
		Reset((PVOID)pPwm);
		pPwm->dwLastPreScaler = pPwm->dwPreScaler;
	}

	OUTREG32(&pPwm->pPwmRegs->PWM_SAR, PwmSampleBuffer->sample);
    OUTREG32(&pPwm->pPwmRegs->PWM_PR, PwmSampleBuffer->period);

	//RETAILMSG(1, (TEXT("pPwmRegs->PWM_CR = 0x%x\r\n"), INREG32(&pPwm->pPwmRegs->PWM_CR)));
	//lqk Sep-9-2011: Pwm output configuation
	//INSREG32(&pPwm->pPwmRegs->PWM_CR, CSP_BITFMASK(PWM_CR_POUTC), CSP_BITFVAL(PWM_CR_POUTC, PWMOUTCONFIG_VAL));
	//OUTREG32( &pPwm->pPwmRegs->PWM_CR, INREG32(&pPwm->pPwmRegs->PWM_CR)|0xc0000 );
	//RETAILMSG(1, (TEXT("pPwmRegs->PWM_CR = 0x%x\r\n"), INREG32(&pPwm->pPwmRegs->PWM_CR)));
    PwmStart(pDeviceContext);
    
	/*
	{
		DWORD dwStartTime, dwElapsedTime;

		dwStartTime = GetTickCount();
		do{
			 dwElapsedTime = GetTickCount() - dwStartTime;
		}while(dwElapsedTime < PwmSampleBuffer->duration);
    }
    PwmStop(pDeviceContext);
	*/
	//
	// CS&ZHL AUG-17-2011: duration = 0: inifinite time
	//
	if(PwmSampleBuffer->duration)
	{
		Sleep(PwmSampleBuffer->duration);
		PwmStop(pDeviceContext);
	}
    
    LeaveCriticalSection(&pPwm->hPwmLock);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmPlaySample\r\n")));
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PwmDeinit
//
// Frees up the register space and deletes critical
// section for deinitialization.
//
// Parameters:
//      pDeviceContext 
//          [IN] pointer to the device context
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PwmDeinit(PVOID pDeviceContext)
{
    PPWM_DEVICE_CONTEXT pPwm = (PPWM_DEVICE_CONTEXT) pDeviceContext;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PwmRelease\r\n")));;

    if(pPwm != NULL)
    {
        // free the virtual space allocated for PWM memory map    
        if (pPwm->pPwmRegs != NULL)
        {
            MmUnmapIoSpace(pPwm->pPwmRegs, sizeof(CSP_PWM_REG));
            pPwm->pPwmRegs = NULL;
        }

        //DeAlloc the Device context
        LocalFree(pPwm);

        // Delete the critical section
        DeleteCriticalSection(&pPwm->hPwmLock);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmRelease\r\n")));;
    return;
}

//------------------------------------------------------------------------------
//
// Function: PwmInitialize
//
// This function will allocate mapping for PWM registers, initialize critical section,
// reset the PWM, and initialize of PWM control register
//
// Parameters:
//      pContext 
//          [IN] Null string containing path to device driver registry
//
// Returns:
//      A pointer on PWM context if success. 0 if init fail.
//
//------------------------------------------------------------------------------
DWORD PwmInitialize(LPCTSTR pContext)
{
    PHYSICAL_ADDRESS phyAddr;
    PPWM_DEVICE_CONTEXT pPwm = 0;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PwmInitialize\r\n")));

    //allocate memory for device context
    pPwm = (PPWM_DEVICE_CONTEXT) LocalAlloc(LPTR, sizeof(PWM_DEVICE_CONTEXT));

    if(PwmGetRegistryValues(pPwm, pContext) == 0)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("PwmInitialize:  Could not read registry\r\n")));
        goto Error;
    }

    phyAddr.HighPart = 0;
    phyAddr.LowPart = BSPPwmGetBaseAddress(pPwm->dwIndex);
    pPwm->pPwmRegs = (PCSP_PWM_REG) MmMapIoSpace(phyAddr, sizeof(CSP_PWM_REG), FALSE);

    // check if Map Virtual Address failed
    if (pPwm->pPwmRegs == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("PwmInitialize:  MmMapIoSpace failed!\r\n")));
        goto Error;
    }

	// CS&ZHL AUG-17-2011: default settings:
	pPwm->dwClockSource = PWM_PWMCR_CLKSRC_IPG_CLK;
	pPwm->dwPreScaler = 1;
	pPwm->dwLastPreScaler = 1;		// = 1.. 4096
	pPwm->dwLastPeriod = 0;
      
    // Initialize PWM critical section
    InitializeCriticalSection(&pPwm->hPwmLock);

    BSPPwmSetClockGatingMode(pPwm, TRUE);
    
    Reset(pPwm);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmInitialize\r\n")));;
    return (DWORD) pPwm;

Error:
    PwmDeinit(pPwm);
    return 0;
}

//------------------------------------------------------------------------------
//
// Function: PwmReset
//
// This function will reset PWM.
//
// Parameters:
//      pDeviceContext 
//          [IN] pointer to the device context.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PwmReset(PVOID pDeviceContext)
{
    PPWM_DEVICE_CONTEXT pPwm = (PPWM_DEVICE_CONTEXT) pDeviceContext;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PwmReset\r\n")));

    EnterCriticalSection(&pPwm->hPwmLock);
    Reset(pDeviceContext);
    LeaveCriticalSection(&pPwm->hPwmLock);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmReset\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: PwmReadRegister
//
// This function is used to read the current values of all PWM registers
//
// Parameters:
//      pDeviceContext 
//          [in] pointer to the device context
//
//      readBuf
//          [in] pwm sample conform to pwmSample_t structure
//
//      Count
//          [in] number of uint32 to be read
//
// Returns:
//      TRUE - If success.
//      FALSE - If the Pwm is not allocated.
//
//------------------------------------------------------------------------------
BOOL PwmReadRegister(PVOID pDeviceContext, UINT32* readBuf, DWORD Count)
{
    PPWM_DEVICE_CONTEXT pPwm = (PPWM_DEVICE_CONTEXT) pDeviceContext;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PwmReadRegister\r\n")));

    EnterCriticalSection(&pPwm->hPwmLock);

    if(Count < (6 * sizeof(UINT32))) 
        return FALSE;
    readBuf[0] = INREG32(&pPwm->pPwmRegs->PWM_CR);
    readBuf[1] = INREG32(&pPwm->pPwmRegs->PWM_SR);
    readBuf[2] = INREG32(&pPwm->pPwmRegs->PWM_IR);
    readBuf[3] = INREG32(&pPwm->pPwmRegs->PWM_SAR);
    readBuf[4] = INREG32(&pPwm->pPwmRegs->PWM_PR);
    readBuf[5] = INREG32(&pPwm->pPwmRegs->PWM_CNR);

    LeaveCriticalSection(&pPwm->hPwmLock);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmReadRegister\r\n")));
    return TRUE;

}

//------------------------------------------------------------------------------
//
// Function: Reset
//
// This function is used to trigger a reset in PWM Control Register,
// and load preset value in Control Register
//
// Parameters:
//      pDeviceContext 
//          [IN] pointer to the device context
//
// Returns:
//      none.
//
//------------------------------------------------------------------------------
static void Reset(PVOID pDeviceContext)
{
    PPWM_DEVICE_CONTEXT pPwm = (PPWM_DEVICE_CONTEXT) pDeviceContext;
    UINT32 clkSrc;

    // Disable PWM and clear all configuration bits
    OUTREG32(&pPwm->pPwmRegs->PWM_CR, 0);
    // Assert software reset for the timer
    INSREG32BF(&pPwm->pPwmRegs->PWM_CR, PWM_CR_SWR, PWM_CR_SWR_RESET);
    // Wait for the software reset to complete
    while(EXTREG32BF(&pPwm->pPwmRegs->PWM_CR, PWM_CR_SWR));

    // Get BSP-specific clock source
    clkSrc = BSPGetPWMClockSource();

	/*
    OUTREG32(&pPwm->pPwmRegs->PWM_CR,
        (CSP_BITFVAL(PWM_CR_EN, PWM_CR_EN_DISABLE)) |                        // disables the PWM
        (CSP_BITFVAL(PWM_CR_REPEAT, PWMSAMPLEREPEAT_VAL)) |       // use sample with no repeat
        (CSP_BITFVAL(PWM_CR_SWR, PWM_CR_SWR_NORESET)) |                // no SW reset
        (CSP_BITFVAL(PWM_CR_PRESCALER, PWMPRESCALER_VAL)) |        // prescaler value of 1
        (CSP_BITFVAL(PWM_CR_CLKSRC, clkSrc)) |                                             // bsp set clock source
        (CSP_BITFVAL(PWM_CR_POUTC, PWMOUTCONFIG_VAL)) |                // PWM output configuration
        (CSP_BITFVAL(PWM_CR_HCTR, PWM_CR_HCTR_DISABLE)) |             // no half-word data swap
        (CSP_BITFVAL(PWM_CR_BCTR, PWM_CR_BCTR_DISABLE)) |              // no byte data swap
        (CSP_BITFVAL(PWM_CR_DBGEN, PWM_CR_DBGEN_INACTIVE)) |      // pwm inactive in debug mode
        (CSP_BITFVAL(PWM_CR_WAITEN, PWM_CR_WAITEN_INACTIVE)) |  // pwm inactive in wait mode
        (CSP_BITFVAL(PWM_CR_DOZEN, PWM_CR_DOZEN_INACTIVE)) |      // pwm inactive in doze mode
        (CSP_BITFVAL(PWM_CR_STOPEN, PWM_CR_STOPEN_INACTIVE)) |   // pwm inactive in stop mode
        (CSP_BITFVAL(PWM_CR_FWM, PWMWATERMARK_VAL)));                 // FIFO empty flag set condition
	*/
	//
	// CS&ZHL AUG-17-2011: keep PWM all the time
	//
    OUTREG32(&pPwm->pPwmRegs->PWM_CR,
        (CSP_BITFVAL(PWM_CR_EN, PWM_CR_EN_DISABLE)) |                        // disables the PWM
        (CSP_BITFVAL(PWM_CR_REPEAT, PWMSAMPLEREPEAT_VAL)) |       // use sample with no repeat
        (CSP_BITFVAL(PWM_CR_SWR, PWM_CR_SWR_NORESET)) |                // no SW reset
        (CSP_BITFVAL(PWM_CR_PRESCALER, (pPwm->dwPreScaler - 1))) |        // prescaler value, default = 1
        (CSP_BITFVAL(PWM_CR_CLKSRC, clkSrc)) |                                             // bsp set clock source
        (CSP_BITFVAL(PWM_CR_POUTC, PWMOUTCONFIG_VAL)) |                // PWM output configuration
        (CSP_BITFVAL(PWM_CR_HCTR, PWM_CR_HCTR_DISABLE)) |             // no half-word data swap
        (CSP_BITFVAL(PWM_CR_BCTR, PWM_CR_BCTR_DISABLE)) |              // no byte data swap
        (CSP_BITFVAL(PWM_CR_DBGEN, PWM_CR_DBGEN_ACTIVE)) |      // pwm active in debug mode
        (CSP_BITFVAL(PWM_CR_WAITEN, PWM_CR_WAITEN_ACTIVE)) |  // pwm active in wait mode
        (CSP_BITFVAL(PWM_CR_DOZEN, PWM_CR_DOZEN_ACTIVE)) |      // pwm active in doze mode
        (CSP_BITFVAL(PWM_CR_STOPEN, PWM_CR_STOPEN_ACTIVE)) |   // pwm active in stop mode
        (CSP_BITFVAL(PWM_CR_FWM, PWMWATERMARK_VAL)));                 // FIFO empty flag set condition

    // disable all interrupts
    OUTREG32(&pPwm->pPwmRegs->PWM_IR,
        (CSP_BITFVAL(PWM_IR_FIE, PWM_IR_FIE_DISABLE)) |
        (CSP_BITFVAL(PWM_IR_RIE, PWM_IR_RIE_DISABLE)) |
        (CSP_BITFVAL(PWM_IR_CIE, PWM_IR_CIE_DISABLE)));


    // Clear Interrupt Status Bits
    INSREG32(&pPwm->pPwmRegs->PWM_SR, CSP_BITFMASK(PWM_SR_CMP),        CSP_BITFVAL(PWM_SR_CMP, PWM_SR_CMP_STATUS_CLEAR));

}

//------------------------------------------------------------------------------
//
// Function: SetPwmPower
//
// This function is to initiailize the power management for PWM driver.
//
// Parameters:
//      power
//          [in] pointer to POWER_CAPABILITIES structure
//
// Returns:
//      none.
//
//------------------------------------------------------------------------------
void SetPwmPower(PPOWER_CAPABILITIES power)
{
    power->DeviceDx = 0x15;    // D0, D2 & D4
    
    // supported power & latency info
    power->Power[D0] = (DWORD) PwrDeviceUnspecified;     // mW  
    power->Power[D2] = (DWORD) PwrDeviceUnspecified;     // mW
    power->Power[D4] = (DWORD) PwrDeviceUnspecified;     // mW
    
    power->Latency[D0] = 0;   // mSec
    power->Latency[D2] = 0;   // mSec
    power->Latency[D4] = 0;   // mSec
    
    power->WakeFromDx = 0;    // no device wake
    power->InrushDx = 0;      // no inrush
    return;

}

//------------------------------------------------------------------------------
//
// Function: PwmGetRegistryValues(LPCTSTR pContext)
//
// This function fills the device context with important information from the registry.
//
// Parameters:
//      pPwm
//          [in] PWM device context structure.
//
//      pContext
//          [in] Null terminated string that is the path to the registry values
//
// Returns:
//       TRUE indicates success, FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL PwmGetRegistryValues(PPWM_DEVICE_CONTEXT pPwm, LPCTSTR pContext)
{
    HKEY hKey = 0;
    DWORD dwSize = sizeof(DWORD);

    hKey = OpenDeviceKey(pContext);

    if(hKey == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(1, (TEXT("PwmGetRegistryValues: Error can not open reg key. Error = 0d%d\r\n"), GetLastError()));
        return FALSE;
    }

    if(RegQueryValueEx(hKey, REG_INDEX_STRING, NULL, NULL, (LPBYTE) &(pPwm->dwIndex), &dwSize) != ERROR_SUCCESS)
    {
        ERRORMSG(1, (TEXT("PwmGetRegistryValues: Error, can not retrieve index from registry. Error = %d\r\n"), GetLastError()));
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: BOOL PwmOpen(DWORD dwIndex)
//
// This function calls the function to set the pin muxes
//
// Parameters:
//      dwIndex
//          [in] Index of device
//
// Returns:
//       TRUE indicates success, FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL PwmOpen(DWORD dwIndex)
{
    return BSPPwmIomuxSetPin(dwIndex);
}

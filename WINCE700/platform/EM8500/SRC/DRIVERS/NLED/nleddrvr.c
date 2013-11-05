//===================================================================
//
//	Module Name:	NLED.DLL
//
//	File Name:		nleddrv.c
//
//	Description:	Control of the notification LED(s)
//
//
// Copyright (c) Texas Instruments Incorporated 2010. All Rights Reserved.
//
//------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
// header files
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable : 4115 4214)
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include "am33x.h"
#include <nled.h>
#include <led_drvr.h>
#include <initguid.h>
#include <soc_cfg.h>
#include <sdk_i2c.h>
#pragma warning(pop)

//-----------------------------------------------------------------------------
//
// Local Macros
//
//-----------------------------------------------------------------------------

#define ENABLE_DEBUG_MESSAGES                   TRUE

// number of NLEDs supported by this driver, must be 1..4
#define NLEDS_NUMBER_LEDS                       4

// number of NLEDs reported by driver, allows hidden NLEDs
#define NLEDS_NUMBER_LEDS_REPORTED              4

// I2C address of GPIO expander used to control the LEDs
#define IOEXP_I2C_ADDR                          0x20

// I2C bus number used for GPIO expander 
#define IOEXP_I2C_BUS  							2    /* I2C1 */

#define IOEXP_CTRL_REG							3
#define IOEXP_OUTIO_REG							1

#if NLEDS_NUMBER_LEDS <= 0
	#error "NLED Driver configured for no notification LEDs (NLEDS_NUMBER_LEDS)"
#endif

//-----------------------------------------------------------------------------
//
// private functions
//
//-----------------------------------------------------------------------------

static BOOL I2CGpioInitPins(void);
static BOOL I2CGpioSetPins(UINT8 *pData);
static BOOL I2CGpioGetPins(UINT8 *pData);

//-----------------------------------------------------------------------------
//
// private data
//
//-----------------------------------------------------------------------------

// Array to hold current Blink Params
static volatile struct NLED_SETTINGS_INFO BlinkParams[NLEDS_NUMBER_LEDS];

// Array to hold current LED settings
static volatile int NLedCurrentState[NLEDS_NUMBER_LEDS];

// Array to hold event handles for waking each LED Thread
static HANDLE hLedHandle[NLEDS_NUMBER_LEDS];
static HANDLE hNewThread[NLEDS_NUMBER_LEDS];
// flag to indicate thread should exit
// Shared by all threads
static BOOL g_bExitThread = FALSE;

// Helper function used to set state of NLED control bits
static void NLedDriverSetLedState(UINT LedNum, int LedState)
{
    UINT8 data[2];
    
    if (LedNum >= NLEDS_NUMBER_LEDS)
	{
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: NLedDriverSetLedState: invalid NLED number: %d\r\n"), LedNum));
        return;
	}

    // Get state of LEDs
    I2CGpioGetPins(&(data[0]));

	switch (LedState)
	{
		case 1:
            // turn LED on	
            data[0] |= 1 << (LedNum + 4);
			break;
		case 0:
            // turn LED off
            data[0] &= ~(1 << (LedNum + 4));
			break;
	}
	
	I2CGpioSetPins(&(data[0]));
}

int NLedControlThread(
	UINT	NLedID
	)
{
	int	i;

	for(;;)
	{
		StateChangeDueToApiCall:

		if (g_bExitThread == TRUE)
			break;

		// check unchanged NLED state
		if ( NLedCurrentState[NLedID] == BlinkParams[NLedID].OffOnBlink )
		{
			// handle blinking state
			if ( NLedCurrentState[NLedID] == 2 )
			{
				// Do meta cycle on blinks (just do regular blink cycle if Meta is 0)
				//DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED%d MetaCycleOn %d periods\r\n"), NLedID, BlinkParams[NLedID].MetaCycleOn));
				for (i = 0; i < (BlinkParams[NLedID].MetaCycleOn == 0 ? 1 : BlinkParams[NLedID].MetaCycleOn); i++)
				{
					// turn NLED on
					NLedDriverSetLedState(NLedID, 1);

					// wait for on time (or change from API call)
					if ( WaitForSingleObject(hLedHandle[NLedID], (BlinkParams[NLedID].OnTime / 1000)) == WAIT_OBJECT_0 )
						goto StateChangeDueToApiCall;

					// turn NLED off
					NLedDriverSetLedState(NLedID, 0);

					// wait for off time (or change from API call)
					if ( WaitForSingleObject(hLedHandle[NLedID], ((BlinkParams[NLedID].TotalCycleTime - BlinkParams[NLedID].OnTime) / 1000)) == WAIT_OBJECT_0 )
						goto StateChangeDueToApiCall;
				}

				// check for meta off specified, wait for that time period (or change from API call)
				//DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED%d MetaCycleOff %d periods\r\n"), NLedID, BlinkParams[NLedID].MetaCycleOff));
				if ( BlinkParams[NLedID].MetaCycleOff > 0 )
				{
					if ( WaitForSingleObject(hLedHandle[NLedID], (((BlinkParams[NLedID].OffTime + BlinkParams[NLedID].OnTime) / 1000) * BlinkParams[NLedID].MetaCycleOff)) == WAIT_OBJECT_0 )
						goto StateChangeDueToApiCall;
				}
			}
			else
			{
				// LED state unchanged, do nothing for on or off state except wait for API change
				WaitForSingleObject(hLedHandle[NLedID], INFINITE);
				#if ENABLE_DEBUG_MESSAGES
					DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED # %d driver thread awakened:\r\n"), NLedID));
				#endif
			}
		}
		else
		{
			// LED state changed, update NLED for new state
			#if ENABLE_DEBUG_MESSAGES
				DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED # %d driver thread: Mode change to %d\r\n"), NLedID, BlinkParams[NLedID].OffOnBlink));
			#endif
			if ( BlinkParams[NLedID].OffOnBlink == 0 )
			{
				NLedDriverSetLedState(NLedID, 0);
				NLedCurrentState[NLedID] = 0;
			}
			if ( BlinkParams[NLedID].OffOnBlink == 1 )
			{
				NLedDriverSetLedState(NLedID, 1);
				NLedCurrentState[NLedID] = 1;
			}
			if ( BlinkParams[NLedID].OffOnBlink == 2 )
				NLedCurrentState[NLedID] = 2;
		}
	}

	DEBUGMSG(ZONE_INIT, (TEXT("NLedControlThread exiting!!\r\n"), NLedID));

	return 0;
}


BOOL
WINAPI
NLedDriverGetDeviceInfo(
	INT		nInfoId,
	PVOID	pOutput
	)
{
	if ( nInfoId == NLED_COUNT_INFO_ID )
	{
		struct NLED_COUNT_INFO * p = (struct NLED_COUNT_INFO*)pOutput;

        if (p == NULL)
	        goto ReturnError;

#if 0
        if (IsBadWritePtr(pOutput, sizeof(struct NLED_COUNT_INFO)))
	        goto ReturnError;
#endif

		// Fill in number of leds
		p->cLeds = NLEDS_NUMBER_LEDS_REPORTED;
		DEBUGMSG(ZONE_INIT, (TEXT("NLEDDRV: NLedDriverGetDeviceInfo(NLED_COUNT_INFO_ID...) returning %d NLEDs\n"), NLEDS_NUMBER_LEDS_REPORTED));
		return TRUE;
	}

	if ( nInfoId == NLED_SUPPORTS_INFO_ID )
	{
		struct NLED_SUPPORTS_INFO * p = (struct NLED_SUPPORTS_INFO *)pOutput;

        if (p == NULL)
	        goto ReturnError;

#if 0
        if (IsBadWritePtr(pOutput, sizeof(struct NLED_SUPPORTS_INFO)))
	        goto ReturnError;
#endif

		if ( p->LedNum >= NLEDS_NUMBER_LEDS )
			goto ReturnError;

		// Fill in LED capabilities
		p->lCycleAdjust = 1000;			// Granularity of cycle time adjustments (microseconds)
		p->fAdjustTotalCycleTime = TRUE;	// LED has an adjustable total cycle time
		p->fAdjustOnTime = TRUE;			// @FIELD	LED has separate on time
		p->fAdjustOffTime = TRUE;			// @FIELD	LED has separate off time
		p->fMetaCycleOn = TRUE;				// @FIELD	LED can do blink n, pause, blink n, ...
		p->fMetaCycleOff = TRUE;			// @FIELD	LED can do blink n, pause n, blink n, ...

		// override individual LED capabilities
		#if NLED_DRIVER_SUPPORTS_VIBRATE
			if (p->LedNum == (NLEDS_NUMBER_LEDS - 1))
			{
				// vibrate must be last NLED, reports special lCycleAdjust value
				p->lCycleAdjust = -1;				// Well that was obvious!
				p->fAdjustTotalCycleTime = FALSE;	// LED has an adjustable total cycle time
				p->fAdjustOnTime = FALSE;			// @FIELD	LED has separate on time
				p->fAdjustOffTime = FALSE;			// @FIELD	LED has separate off time
				p->fMetaCycleOn = FALSE;			// @FIELD	LED can do blink n, pause, blink n, ...
				p->fMetaCycleOff = FALSE;			// @FIELD	LED can do blink n, pause n, blink n, ...
			}
		#endif
		return TRUE;
	}
	else if ( nInfoId == NLED_SETTINGS_INFO_ID )
	{
		struct NLED_SETTINGS_INFO * p = (struct NLED_SETTINGS_INFO *)pOutput;

        if (p == NULL)
	        goto ReturnError;

#if 0
        if (IsBadWritePtr(pOutput, sizeof(struct NLED_SETTINGS_INFO)))
	        goto ReturnError;
#endif

		if ( p->LedNum >= NLEDS_NUMBER_LEDS )
			goto ReturnError;

		// Fill in current LED settings

		// Get any individual current settings
		p->OffOnBlink = BlinkParams[p->LedNum].OffOnBlink;
		p->TotalCycleTime = BlinkParams[p->LedNum].TotalCycleTime;
		p->OnTime = BlinkParams[p->LedNum].OnTime;
		p->OffTime = BlinkParams[p->LedNum].OffTime;
		p->MetaCycleOn = BlinkParams[p->LedNum].MetaCycleOn;
		p->MetaCycleOff = BlinkParams[p->LedNum].MetaCycleOff;

		return TRUE;
	}

ReturnError:

	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
}


BOOL
WINAPI
NLedDriverSetDevice(
	INT		nInfoId,
	PVOID	pInput
	)
{
	struct NLED_SETTINGS_INFO * p = (struct NLED_SETTINGS_INFO *)pInput;

	if ( nInfoId == NLED_SETTINGS_INFO_ID )
	{
        if (pInput == NULL)
	        goto ReturnError;

#if 0  // IsBadXXXPtr is deprecated
        if (IsBadReadPtr(pInput, sizeof(struct NLED_SETTINGS_INFO)))
	        goto ReturnError;
#endif

		if ( p->LedNum >= NLEDS_NUMBER_LEDS )
			goto ReturnError;

		// check for invalid parameters
		if ( p->OffOnBlink < 0 || p->OffOnBlink > 2 )
        {
		#if ENABLE_DEBUG_MESSAGES
			DEBUGMSG(ZONE_ERROR, (TEXT("NLED: NLedDriverSetDevice: NLED %x, Invalid parameter\r\n"), p->LedNum));
		#endif
			goto ReturnError;
        }

		// for blink state, check for valid times (why were integers used?)
		if ( p->OffOnBlink == 2 )
			if (p->TotalCycleTime < p->OnTime ||
				p->TotalCycleTime < p->OffTime ||
				p->MetaCycleOn < 0 ||
				p->MetaCycleOff < 0 ||
				p->TotalCycleTime < 0 ||
				p->OnTime < 0 ||
				p->OffTime < 0 ||
				p->TotalCycleTime < p->OnTime + p->OffTime
			)
				goto ReturnError;

		// check for any changed NLED settings
		if ( BlinkParams[p->LedNum].OffOnBlink != p->OffOnBlink ||
			 BlinkParams[p->LedNum].TotalCycleTime != p->TotalCycleTime ||
			 BlinkParams[p->LedNum].OnTime != p->OnTime ||
			 BlinkParams[p->LedNum].OffTime != p->OffTime ||
			 BlinkParams[p->LedNum].MetaCycleOn != p->MetaCycleOn ||
			 BlinkParams[p->LedNum].MetaCycleOff != p->MetaCycleOff
		)
		{
			// Update NLED settings
			BlinkParams[p->LedNum].OffOnBlink = p->OffOnBlink;
			BlinkParams[p->LedNum].TotalCycleTime = p->TotalCycleTime;
			BlinkParams[p->LedNum].OnTime = p->OnTime;
			BlinkParams[p->LedNum].OffTime = p->OffTime;
			BlinkParams[p->LedNum].MetaCycleOn = p->MetaCycleOn;
			BlinkParams[p->LedNum].MetaCycleOff = p->MetaCycleOff;

			// wake up appropriate NLED thread
			#if ENABLE_DEBUG_MESSAGES
				DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED # %d change signaled\r\n"), p->LedNum));
			#endif

			SetEvent(hLedHandle[p->LedNum]);
		}

		return TRUE;
	}

ReturnError:

	SetLastError(ERROR_INVALID_PARAMETER);

	return FALSE;
}


// Note: This function is called by the power handler, it must not make
// any system calls other than the few that are specifically allowed
// (such as SetInterruptEvent()).
VOID
NLedDriverPowerDown(
   BOOL power_down
   )
{
	UINT NledNum;

	if ( power_down )
	{
		// shut off all NLEDs
		for (NledNum = 0; NledNum < NLEDS_NUMBER_LEDS; NledNum++)
		{
			NLedDriverSetLedState(NledNum, 0);
			NLedCurrentState[NledNum] = 0;
		}
	}
	else
	{
		for (NledNum = 0; NledNum < NLEDS_NUMBER_LEDS; NledNum++)
		{
			// On Power Up (Resume) turn on any LEDs that should be "ON".
			//	the individual LED control threads will put the Blinking
			//	LEDs back in their proper pre suspend state while "OFF" LEDs
			//	will stay off.
			if ( BlinkParams[NledNum].OffOnBlink == 1 )
			{
				NLedDriverSetLedState(NledNum, 1);
				NLedCurrentState[NledNum] = 1;
			}
		}
	}
}


BOOL
NLedDriverInitialize(
	VOID
	)
{
	BOOL bResult = TRUE;

	int i;

	// initialize the NLED state array and blink parameter structure
	for (i = 0; i < NLEDS_NUMBER_LEDS; i++)
	{
		BlinkParams[i].LedNum = i;
		BlinkParams[i].OffOnBlink = 0;
		BlinkParams[i].TotalCycleTime = 0;
		BlinkParams[i].OnTime = 0;
		BlinkParams[i].OffTime = 0;
		BlinkParams[i].MetaCycleOn = 0;
		BlinkParams[i].MetaCycleOff = 0;
		NLedCurrentState[i] = 0;
	}

	for ( i = 0; i < NLEDS_NUMBER_LEDS; i++ )
	{
		hLedHandle[i] = NULL;
		hNewThread[i] = NULL;
	}
		
	for ( i = 0; i < NLEDS_NUMBER_LEDS; i++ )
	{
		hLedHandle[i] = CreateEvent(0, FALSE, FALSE, NULL);
		hNewThread[i] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)NLedControlThread, (LPVOID)i, 0, NULL);

		if ( hNewThread[i] == NULL || hLedHandle[i] == NULL)
        {
			DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: NLedDriverInitialize: Could not create event and/or start thread.\r\n")));
        	bResult = FALSE;
        }
		else
		{
			DEBUGMSG(ZONE_INIT, (TEXT("NLedDriverInitialize: NLED # %d driver thread: (0x%X) started.\r\n"), i, hNewThread[i]));
		}
	}
	
	// Initialise GPIOs
	bResult = I2CGpioInitPins();
	return bResult;
}

BOOL
NLedDriverDeInitialize(
	VOID
	)
{
	int i;

    DEBUGMSG(ZONE_INIT, (TEXT("NLEDDRV:  NLedDriverDeInitialize() Unloading driver...\r\n")));

	// Stop all threads, close handles
	g_bExitThread = TRUE;
	for ( i = 0; i < NLEDS_NUMBER_LEDS; i++ )
	{
		if (hLedHandle[i])
		    SetEvent(hLedHandle[i]);

        if (hNewThread[i])
		{
    		WaitForSingleObject(hNewThread[i], INFINITE);
    		CloseHandle(hNewThread[i]);
		}

		if (hLedHandle[i])
    		CloseHandle(hLedHandle[i]);

		hNewThread[i] = NULL;
		hLedHandle[i] = NULL;
	}
	
	return TRUE;
}

//-----------------------------------------------------------------------------
static BOOL WriteI2C(HANDLE hI2C, UINT16 subaddr, VOID* pBuffer, DWORD count,DWORD *pWritten)
{
    BOOL bRet = TRUE;    

    if (hI2C==NULL) 
        RETAILMSG(1,(L"NLED::WriteAIC::hI2C is NULL reg=%d val=%d\r\n",subaddr,*(UINT8 *)pBuffer));

	if ((*pWritten = I2CWrite(hI2C, subaddr, pBuffer, count)) != count)
	{
		bRet = FALSE;
	}

	return bRet;
}

//-----------------------------------------------------------------------------
static BOOL ReadI2C(HANDLE hI2C, UINT16 subaddr, VOID* pBuffer, DWORD count, DWORD *pRead)
{
    BOOL bRet = TRUE;	 
    
    if (hI2C==NULL) 
    	RETAILMSG(1,(L"NLED::ReadAIC::hI2C is NULL reg=%d \r\n",subaddr));
    
    if ((*pRead = I2CRead(hI2C, subaddr, pBuffer, sizeof(UINT8))) != sizeof(UINT8))
    {
    	bRet = FALSE;
    }
    
    return bRet;

}

//-----------------------------------------------------------------------------
static HANDLE OpenI2C(LPCWSTR devicename)
{
    HANDLE hI2C;

    DWORD I2Cbus = IOEXP_I2C_BUS;
    
    DEBUGMSG( ZONE_FUNCTION,
    	(L"+NLED::OpenI2C\r\n"));
     
    // Open I2C bus
    hI2C = (HANDLE)I2COpen(SOCGetI2CDeviceByBus(I2Cbus));
    if (hI2C == INVALID_HANDLE_VALUE)
    {
        hI2C = NULL;
    }

    return hI2C;
}

//-----------------------------------------------------------------------------
static VOID CloseI2C(HANDLE hI2C)
{
    if ((hI2C != NULL) && (hI2C != INVALID_HANDLE_VALUE))
    {
        I2CClose(hI2C);
    }
}


static DWORD ReadExp(UINT8 reg, UINT8 *data, UINT32 length)
{
    TCHAR *device = NULL;
    HANDLE hI2C = NULL;
    UINT32 addr;
    DWORD readCount = 0;

    addr = IOEXP_I2C_ADDR;
    device = L"I2C1:";

    hI2C = OpenI2C(device);
    if (hI2C == NULL)
    {
        DEBUGMSG( ZONE_ERROR,
            (L"ERROR: Failed to open I2C device %s\r\n", device));
        return 0;
    }

    // set slave address
    if (!I2CSetSlaveAddress(hI2C, addr))
    {
        DEBUGMSG( ZONE_ERROR,
            (L"ERROR: Failed to set I2C slave address\r\n"));
        return 0;
    }

    if (!ReadI2C(hI2C, (UINT16)reg, data, length, &readCount)){
        DEBUGMSG( ZONE_ERROR,
            (L"ERROR: Failed to read I2C device %s\r\n", device));
    }

    if (hI2C)
        CloseI2C(hI2C);

    return readCount;
}

static BOOL WriteExp(UINT8 reg, UINT8 *data, DWORD length)
{
    TCHAR *device = NULL;
    HANDLE hI2C = NULL;
    UINT32 addr;
    DWORD written = 0;
    BOOL RetVal;

    addr = IOEXP_I2C_ADDR;
    device = L"I2C1:";

    hI2C = OpenI2C(device);
    if (hI2C == NULL)
    {
        DEBUGMSG( ZONE_ERROR,
            (L"ERROR: Failed to open I2C device %s \r\n", device));
        return FALSE;
    }

    if (!I2CSetSlaveAddress(hI2C, addr))
    {
        DEBUGMSG( ZONE_ERROR,
            (L"ERROR: Failed to set I2C slave address\r\n"));
        return FALSE;
    }

    RetVal = WriteI2C(hI2C, (UINT16)reg, data, length, &written);
    if ((RetVal == FALSE) || (written != length))
    {
        DEBUGMSG( ZONE_ERROR,
            (L"ERROR: Write Error - I2C device %s (Ret=%d, wrote %dB)\r\n", device, RetVal, written));
    }

    if (hI2C)
        CloseI2C(hI2C);

    return RetVal;
}


static BOOL I2CGpioGetPins(UINT8 *pData)
{
    // Read 1 bytes from output register
    return ReadExp(IOEXP_OUTIO_REG, pData, 1);
}

static BOOL I2CGpioSetPins(UINT8 *pData)
{
    // Write 1 bytes to register 1 (output register)
    return WriteExp(IOEXP_OUTIO_REG, pData, 1);
}

static BOOL I2CGpioInitPins(void)
{
    UINT8 data;
	
    /* configure as pin 4-7 for output pins */
	data = 0x0f;	
	WriteExp(IOEXP_CTRL_REG, &data, 1);

    // Get state of LEDs
    ReadExp(IOEXP_OUTIO_REG, &data, 1);    

	/* Initial state of LED, all LEDs are off */
    data &= 0x0F;
	WriteExp(IOEXP_OUTIO_REG, &data, 1);
	
    return TRUE;
}

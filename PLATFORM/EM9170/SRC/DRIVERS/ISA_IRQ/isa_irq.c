#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 4245)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <devload.h>
#pragma warning(pop)

#include "bsp.h"
#include "bsp_drivers.h"
#include "isa_irq.h"

BOOL WINAPI
DllEntry(
             HANDLE  hInstDll,
             DWORD   dwReason,
             LPVOID  lpvReserved
             )
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch ( dwReason ) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstDll);
        DEBUGMSG(ZONE_INIT, (TEXT("ISA_IRQ : DLL_PROCESS_ATTACH\r\n")));
        break;

    case DLL_PROCESS_DETACH:
        // should be signaling thread here
        DEBUGMSG(ZONE_INIT, (TEXT("ISA_IRQ : DLL_PROCESS_DETACH\r\n")));
        break;
    }

    return (TRUE);
}


DWORD ISAGetIRQ(DWORD dwIndex)
{
    DWORD dwSysIntr = (DWORD) -1;
    
    switch(dwIndex)
    {
    case 1:
        //dwSysIntr = IRQ_EXT_INT5;			//pin: GPIO_F -> GPIO1_5
        dwSysIntr = IRQ_GPIO1_PIN5;			//pin: GPIO_F -> GPIO1_5
        break;
        
    case 2:
        //dwSysIntr = IRQ_EXT_INT4;			//pin: GPIO_E -> GPIO1_4
        dwSysIntr = IRQ_GPIO1_PIN4;			//pin: GPIO_F -> GPIO1_4
        break;
                   
    default:
        break;
    }

	return dwSysIntr;
}


void IRQEventHandler( HANDLE hOpenContext )
{
  	IRQINSTANCE*	pIRQInstance = (IRQINSTANCE*)hOpenContext;
	UINT32				dwTmp = 0;

    if ( pIRQInstance->KillThread ||!pIRQInstance->hIRQEvent ) 
	{
        SetEvent( pIRQInstance->hKillISTThread );
        ExitThread(0);
    }

	//SetEvent( pIRQInstance->hExternIRQEvent );
	switch(pIRQInstance->DeviceArrayIndex)
	{
	case 1:		// config pin GPIO_F as input with pull-up resistor
		DDKGpioReadDataPin(DDK_GPIO_PORT1,  5,  &dwTmp);
		if(dwTmp)
		{
			SetEvent( pIRQInstance->hExternIRQEvent );
			//RETAILMSG(1, (TEXT("->ISA_IRQ1\r\n")));
		}
		else		
		{
			// GPIO_F is in low level state, send EOI internally
			DDKGpioClearIntrPin(DDK_GPIO_PORT1, 5);
			InterruptDone(pIRQInstance->SysIntr);		// send EOI
		}
		break;

	case 2:		// config pin GPIO_E as input with pull-up resistor
		DDKGpioReadDataPin(DDK_GPIO_PORT1,  4,  &dwTmp);
		if(dwTmp)
		{
			SetEvent( pIRQInstance->hExternIRQEvent );
			//RETAILMSG(1, (TEXT("->ISA_IRQ2\r\n")));
		}
		else		
		{
			// GPIO_E is in low level state, send EOI internally
			DDKGpioClearIntrPin(DDK_GPIO_PORT1, 4);
			InterruptDone(pIRQInstance->SysIntr);		// send EOI
		}
		break;

	default:
		break;
	}
}

/*
 @doc INTERNAL
 *
 *	Not exported to users.
 *
 @rdesc This thread technically returns a status, but in practice, doesn't return
 *		while the device is open.
 */
static DWORD WINAPI IRQ_ISTThread( HANDLE hContext )				    /* @parm [IN] Pointer to main data structure. */
{
  	IRQINSTANCE*		pIRQInstance = (IRQINSTANCE*)hContext;
    ULONG					WaitReturn;

    /* Wait for the event that any serial port action creates.
     */
    while ( !pIRQInstance->KillThread ) 
	{
        WaitReturn = WaitForSingleObject( pIRQInstance->hIRQEvent, INFINITE);

		// IST Handler
        IRQEventHandler( hContext );

		//send EOI
		//InterruptDone(pIRQInstance->SysIntr);
	}

    return 0;
}

// ****************************************************************
//
//	@doc INTERNAL
//	@func		BOOL | Stop thread, disable interrupt.
//
//	@parm 		HANDLE | hOpenContext
//
//	@rdesc		TRUE if success, FALSE if failed.
//
BOOL  StopISTThread( HANDLE hOpenContext  )
{
	IRQINSTANCE*		pIRQInstance = (IRQINSTANCE*)hOpenContext;

    HANDLE              pThisThread = GetCurrentThread( );
    ULONG               priority256;

    /* If we have an interrupt handler thread, kill it */
    if ( pIRQInstance->hISTThread ) 
	{
        /* Set the priority of the dispatch thread to be equal to this one,
         * so that it shuts down before we free its memory. If this routine
         * has been called from SerialDllEntry then RxCharBuffer is set to
         * NULL and the dispatch thread is already dead, so just skip the
         * code which kills the thread.
         */
        priority256 = CeGetThreadPriority( pThisThread);
        CeSetThreadPriority(pIRQInstance->hISTThread, priority256);        

        /* Signal the Dispatch thread to die. */
        pIRQInstance->KillThread = 1;
        SetEvent(pIRQInstance->hIRQEvent);

        WaitForSingleObject(pIRQInstance->hKillISTThread, 3000);
        Sleep(10);

        CloseHandle(pIRQInstance->hISTThread);
        pIRQInstance->hISTThread = NULL;        
    }
	
	InterruptDone(pIRQInstance->SysIntr);
	InterruptDisable(pIRQInstance->SysIntr);

    return TRUE;
}

BOOL InitIST(HANDLE hOpenContext)
{
	IRQINSTANCE*    pIRQInstance = (IRQINSTANCE*)hOpenContext;

	// create interrupt event
	pIRQInstance->hIRQEvent = CreateEvent( NULL,
										   FALSE,		//	Not manual reset
                                           FALSE,		//	Not signalled
                                           NULL
                                         );
	if(pIRQInstance->hIRQEvent==NULL)
	{
		return FALSE;
	}

	// create kill IST thread event
	pIRQInstance->hKillISTThread = CreateEvent( 0, FALSE, FALSE, NULL );
	if(pIRQInstance->hKillISTThread==NULL)
	{
		return FALSE;
	}

	// create extern IRQ event
	pIRQInstance->hExternIRQEvent = CreateEvent( NULL,
										        FALSE,		//	Not manual reset
                                                FALSE,		//	Not signalled
                                                NULL
                                               );
	if(pIRQInstance->hExternIRQEvent==NULL)
	{
		return FALSE;
	}

	// bind SYSINTR to event
   	if(!InterruptInitialize( pIRQInstance->SysIntr, pIRQInstance->hIRQEvent, NULL, 0))
	{
		return FALSE;
	}

	//CS&ZHL JUN-18-07: follow 'com_mmd2\mdd.c' to do this
    InterruptDone(pIRQInstance->SysIntr);

	pIRQInstance->KillThread = 0;
	pIRQInstance->hISTThread = NULL;

	// create IST thread
    pIRQInstance->hISTThread=CreateThread((LPSECURITY_ATTRIBUTES)NULL,
									       0,
                                           (LPTHREAD_START_ROUTINE) IRQ_ISTThread,
                                           hOpenContext,
                                           0,
                                           NULL);
    if(!pIRQInstance->hISTThread)
	{
        return FALSE;
	}
	
	return TRUE;
}

BOOL DeinitIST(HANDLE hOpenContext)
{
	IRQINSTANCE *pIRQInstance;

    pIRQInstance  =  (IRQINSTANCE*)hOpenContext;

	// ToDo: better thread termination code
	StopISTThread( hOpenContext );

	// ToDo: ReleaseSysIntr
    return TRUE;
}


BOOL IRQ_IOControl( HANDLE hOpenContext, DWORD dwCode,
					PBYTE pBufIn, DWORD dwLenIn,
					PBYTE pBufOut, DWORD dwLenOut,
					PDWORD pdwActualOut)
{
	BOOL		bRet = TRUE;
	IRQINSTANCE *pIRQInstance;
	DWORD		dwWaitReturn;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);

    pIRQInstance = (IRQINSTANCE*)hOpenContext;
	
	switch(dwCode)
	{
	case IOCTL_WAIT_FOR_IRQ:
		if(!pBufIn || (dwLenIn < sizeof(DWORD)))
		{
			bRet = FALSE;
			break;
		}

		if(!pBufOut || (dwLenOut < sizeof(DWORD)))
		{
			bRet = FALSE;
			break;
		}

		dwWaitReturn = WaitForSingleObject( pIRQInstance->hExternIRQEvent, *((PDWORD)pBufIn) );
		if(  dwWaitReturn == WAIT_OBJECT_0 )    //The state of the specified object is signaled
		{
			*((PDWORD)pBufOut) = 1;
		}
		else															// The state of WAIT_TIMEOUT and WAIT_FAILED
		{
			*((PDWORD)pBufOut) = 0;
		}

		if(pdwActualOut != NULL)
		{
			*pdwActualOut = (DWORD)(sizeof(DWORD));
		}
		break;

	case IOCTL_SEND_EOI:
		// clear GPIO interrupt status 
		switch(pIRQInstance->DeviceArrayIndex)
		{
		case 1:		// config pin GPIO_F as input with pull-up resistor
			DDKGpioClearIntrPin(DDK_GPIO_PORT1, 5);
			break;

		case 2:		// config pin GPIO_E as input with pull-up resistor
			DDKGpioClearIntrPin(DDK_GPIO_PORT1, 4);
			break;
		}
		//
		// send EOI to permit following interrrupt requests
		//
		InterruptDone(pIRQInstance->SysIntr);
		break;

	default:
		// unsupported IOControl
		bRet = FALSE;
	}

	return bRet;
}

HANDLE IRQ_Init( DWORD dwContext )
{
    HKEY					hKey;
    LONG				regError;
	DWORD				dwDataSize;
	DWORD				dwTmp;
    IRQINSTANCE*	pIRQInstance = NULL;

	RETAILMSG(1, (TEXT("->IRQ_Init\r\n")));
    // Allocate irq structure.
    pIRQInstance  =  (IRQINSTANCE*)LocalAlloc(LPTR, sizeof(IRQINSTANCE));

    // Check that LocalAlloc did stuff ok too.
    if ( !pIRQInstance ) 
	{
		RETAILMSG(1, (TEXT("IRQ_Init::alloc failed\r\n")));
        return(NULL);
    }

	//
	// CS&ZHL APR-25-2008: read DeviceIndex from registry
	//
	hKey = OpenDeviceKey((LPCTSTR)dwContext);
    if ( !hKey ) 
	{
		RETAILMSG(1, (TEXT("IRQ_Init::can not open registry\r\n")));
        LocalFree(pIRQInstance);
        return(NULL);
    }

	//
	// CS&ZHL JLY-15-2009: check if the driver is required to install
	//
	dwDataSize = sizeof(DWORD);
    regError = RegQueryValueEx(
                              hKey, 
                              _T("TrueInstall"), 
                              NULL, 
                              NULL,
                              (LPBYTE)(&dwTmp), 
                              &dwDataSize);
    if ( regError != ERROR_SUCCESS ) 
	{
		RETAILMSG(1, (TEXT("IRQ_Init::failed to read TrueInstall.\r\n")));
		RegCloseKey (hKey);
        LocalFree(pIRQInstance);
        return (NULL);
    }
	if(!dwTmp)
	{
		RETAILMSG(1, (TEXT("IRQ_Init::quit IRQ driver installation.\r\n")));
		RegCloseKey (hKey);
        LocalFree(pIRQInstance);
        return (NULL);
	}

	// read device index
	dwDataSize = sizeof(DWORD);
    regError = RegQueryValueEx(
                              hKey, 
                              _T("DeviceArrayIndex"), 
                              NULL, 
                              NULL,
                              (LPBYTE)(&pIRQInstance->DeviceArrayIndex), 
                              &dwDataSize);
    if ( regError != ERROR_SUCCESS ) 
	{
		RETAILMSG(1, (TEXT("IRQ_Init::read DeviceArrayIndex failed.\r\n")));
		RegCloseKey (hKey);
        LocalFree(pIRQInstance);
        return (NULL);
    }

    RegCloseKey (hKey);

	// get logical IRQ number
	pIRQInstance->dwDeviceID = ISAGetIRQ(pIRQInstance->DeviceArrayIndex);
	//
	// Request sysintr for the device
	//
	pIRQInstance->SysIntr = (DWORD)SYSINTR_UNDEFINED;
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, 
						(PVOID)&pIRQInstance->dwDeviceID, 
						sizeof(pIRQInstance->dwDeviceID), 
						(PVOID)&pIRQInstance->SysIntr, 
						sizeof(pIRQInstance->SysIntr), NULL))
	{
		RETAILMSG(1, (TEXT("ERROR: Failed to request sysintr.\r\n")));
		pIRQInstance->SysIntr = (DWORD)SYSINTR_UNDEFINED;

		// release the instance
		LocalFree( pIRQInstance );
		return (NULL);
	}

	RETAILMSG(1, (TEXT("<-IRQ_Init:: IrqNo=%d, SysIntr=%d\r\n"), pIRQInstance->dwDeviceID, pIRQInstance->SysIntr));
    return (HANDLE) pIRQInstance;
}

BOOL IRQ_Deinit( DWORD hDeviceContext )
{
    IRQINSTANCE*    pIRQInstance = (IRQINSTANCE*)hDeviceContext;

    //CS&ZHL 
	LocalFree( pIRQInstance );

    return TRUE;
}

HANDLE IRQ_Open( HANDLE hDeviceContext,
				 DWORD AccessCode,
				 DWORD ShareMode )
{
	IRQINSTANCE*	pIRQInstance;

    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    pIRQInstance  = (IRQINSTANCE*)hDeviceContext;

	if(pIRQInstance->IsOpened)
	{
		// already opened, return failure
		return (HANDLE)NULL;
	}
	pIRQInstance->IsOpened=TRUE;

	//
	// First, start Interrupt Service Thread
	//
	if(	!InitIST(hDeviceContext) )
	{
		pIRQInstance->IsOpened = FALSE;
		return NULL;
	}

	//
	// And then, configuring hardware pins if necessary
	//
	switch(pIRQInstance->DeviceArrayIndex)
	{
	case 1:		// config pin GPIO_F as input with pull-down resistor
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_F,						// pin name
										DDK_IOMUX_PIN_MUXMODE_ALT0,			// select GPIO1[5] 
										DDK_IOMUX_PIN_SION_REGULAR);			// no SION option on this pin

		DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_F,						
										 DDK_IOMUX_PAD_SLEW_SLOW,				// -> the same electrical config as PDK1_7
										 DDK_IOMUX_PAD_DRIVE_HIGH,
										 DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
										 DDK_IOMUX_PAD_PULL_DOWN_100K,
										 DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
										 DDK_IOMUX_PAD_VOLTAGE_3V3);
		
		//DDKGpioSetConfig(DDK_GPIO_PORT1,						//gpio group number = DDK_GPIO_PORT1..DDK_GPIO_PORT4
		//							5,													//gpio number within a gpio group = 0..31
		//							DDK_GPIO_DIR_IN,						//gpio direction = DDK_GPIO_DIR_IN, DDK_GPIO_DIR_OUT
		//							DDK_GPIO_INTR_NONE);				//gpio interrupt = level, edge,none, etc 
		// try to use GPIO interrupt: rise edge active
		DDKGpioSetConfig(DDK_GPIO_PORT1,						//gpio group number = DDK_GPIO_PORT1..DDK_GPIO_PORT4
									5,													//gpio number within a gpio group = 0..31
									DDK_GPIO_DIR_IN,						//gpio direction = DDK_GPIO_DIR_IN, DDK_GPIO_DIR_OUT
									DDK_GPIO_INTR_RISE_EDGE);		//gpio interrupt = level, edge,none, etc 
    
		DDKGpioClearIntrPin(DDK_GPIO_PORT1, 5);
		break;

	case 2:		// config pin GPIO_E as input with pull-down resistor
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_E,						// pin name
										DDK_IOMUX_PIN_MUXMODE_ALT0,			// select GPIO1[4] 
										DDK_IOMUX_PIN_SION_REGULAR);			// no SION option on this pin

		DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_E,						
										 DDK_IOMUX_PAD_SLEW_SLOW,
										 DDK_IOMUX_PAD_DRIVE_HIGH,
										 DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
										 DDK_IOMUX_PAD_PULL_DOWN_100K,
										 DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
										 DDK_IOMUX_PAD_VOLTAGE_3V3);
		
		//DDKGpioSetConfig(DDK_GPIO_PORT1,						//gpio group number = DDK_GPIO_PORT1..DDK_GPIO_PORT4
		//							4,													//gpio number within a gpio group = 0..31
		//							DDK_GPIO_DIR_IN,						//gpio direction = DDK_GPIO_DIR_IN, DDK_GPIO_DIR_OUT
		//							DDK_GPIO_INTR_NONE);				//gpio interrupt = level, edge,none, etc 
		// try to use GPIO interrupt: rise edge active
		DDKGpioSetConfig(DDK_GPIO_PORT1,						//gpio group number = DDK_GPIO_PORT1..DDK_GPIO_PORT4
									4,													//gpio number within a gpio group = 0..31
									DDK_GPIO_DIR_IN,						//gpio direction = DDK_GPIO_DIR_IN, DDK_GPIO_DIR_OUT
									DDK_GPIO_INTR_RISE_EDGE);		//gpio interrupt = level, edge,none, etc 
    
		DDKGpioClearIntrPin(DDK_GPIO_PORT1, 4);
		break;

	default:
		break;
	}

	return pIRQInstance;
}

BOOL IRQ_Close( HANDLE hOpenContext )
{
	IRQINSTANCE*	pIRQInstance = (IRQINSTANCE*)hOpenContext;

	if(!pIRQInstance->IsOpened)
	{
		return FALSE;
	}
	pIRQInstance->IsOpened = FALSE;

	switch(pIRQInstance->DeviceArrayIndex)
	{
	case 1:		// config pin GPIO_F as input with pull-up resistor
		DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_F,						
										 DDK_IOMUX_PAD_SLEW_SLOW,				// -> the same electrical config as PDK1_7
										 DDK_IOMUX_PAD_DRIVE_HIGH,
										 DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
										 DDK_IOMUX_PAD_PULL_UP_100K,
										 DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
										 DDK_IOMUX_PAD_VOLTAGE_3V3);

		DDKGpioSetConfig(DDK_GPIO_PORT1,						//gpio group number = DDK_GPIO_PORT1..DDK_GPIO_PORT4
									5,													//gpio number within a gpio group = 0..31
									DDK_GPIO_DIR_IN,						//gpio direction = DDK_GPIO_DIR_IN, DDK_GPIO_DIR_OUT
									DDK_GPIO_INTR_NONE);				//gpio interrupt = level, edge,none, etc 
		//clear interrupt flag if set
		DDKGpioClearIntrPin(DDK_GPIO_PORT1, 5);
		break;

	case 2:		// config pin GPIO_E as input with pull-up resistor
		DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_E,						
										 DDK_IOMUX_PAD_SLEW_SLOW,
										 DDK_IOMUX_PAD_DRIVE_HIGH,
										 DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
										 DDK_IOMUX_PAD_PULL_UP_100K,
										 DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
										 DDK_IOMUX_PAD_VOLTAGE_3V3);

		DDKGpioSetConfig(DDK_GPIO_PORT1,						//gpio group number = DDK_GPIO_PORT1..DDK_GPIO_PORT4
									4,													//gpio number within a gpio group = 0..31
									DDK_GPIO_DIR_IN,						//gpio direction = DDK_GPIO_DIR_IN, DDK_GPIO_DIR_OUT
									DDK_GPIO_INTR_NONE);				//gpio interrupt = level, edge,none, etc 
		// clear interrupt flag
		DDKGpioClearIntrPin(DDK_GPIO_PORT1, 4);
		break;

	default:
		break;
	}

	// uninstall IST then
	DeinitIST( hOpenContext );

	return TRUE;
}

ULONG IRQ_Read( HANDLE dwOpenContext,
			    LPVOID pBuf,
			    DWORD len )
{
    UNREFERENCED_PARAMETER(dwOpenContext);
    UNREFERENCED_PARAMETER(pBuf);
    UNREFERENCED_PARAMETER(len);
	return 0;
}

ULONG IRQ_Write( HANDLE hOpenContext,
				 LPCVOID pSourceBytes,
				 DWORD NumberOfBytes )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pSourceBytes);
    UNREFERENCED_PARAMETER(NumberOfBytes);
    return 0;
}

ULONG IRQ_Seek( DWORD hOpenContext,
			    long Amount,
				WORD Type )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(Amount);
    UNREFERENCED_PARAMETER(Type);
	return 0;
}

void IRQ_PowerDown( DWORD hDeviceContext )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
}

void IRQ_PowerUp( DWORD hDeviceContext )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
}


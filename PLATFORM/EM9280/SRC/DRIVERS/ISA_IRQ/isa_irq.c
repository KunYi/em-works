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


DWORD GetLogicalIrqNum(DWORD dwIndex)
{
    DWORD dwIrqNum;
    
    switch(dwIndex)
    {
    case 1:
		dwIrqNum = IRQ_GPIO2_PIN3;			//GPIO2_3 -> EM9280_GPIO24 
        break;
        
    case 2:
		dwIrqNum = IRQ_GPIO2_PIN20;			//GPIO2_20 -> EM9280_GPIO25 
        break;
                   
    case 3:
		dwIrqNum = IRQ_GPIO3_PIN30;			//GPIO3_30 -> EM9280_GPIO26
        break;
                   
    case 4:
		dwIrqNum = IRQ_GPIO4_PIN20;			//GPIO4_20 -> EM9280_GPIO27
        break;
                   
    default:
		dwIrqNum = (DWORD) -1;
    }

	return dwIrqNum;
}


DWORD GetIrqGpioPin(DWORD dwIndex)
{
    DWORD dwGpioPin;
    
    switch(dwIndex)
    {
    case 1:
		dwGpioPin = DDK_IOMUX_GPIO2_3;		//GPIO2_3 -> EM9280_GPIO24 
        break;
        
    case 2:
		dwGpioPin = DDK_IOMUX_GPIO2_20;		//GPIO2_20 -> EM9280_GPIO25 
        break;
                   
    case 3:
		dwGpioPin = DDK_IOMUX_GPIO3_30;		//GPIO3_30 -> EM9280_GPIO26
        break;
                   
    case 4:
		dwGpioPin = DDK_IOMUX_GPIO4_20;		//GPIO4_20 -> EM9280_GPIO27
        break;
                   
    default:
		dwGpioPin = DDK_IOMUX_INVALID_PIN;
    }

	return dwGpioPin;
}


void IRQEventHandler( HANDLE hOpenContext )
{
  	IRQINSTANCE*	pIRQInstance = (IRQINSTANCE*)hOpenContext;
	UINT32			u32State = 0;

    if ( pIRQInstance->KillThread ||!pIRQInstance->hIRQEvent ) 
	{
        SetEvent( pIRQInstance->hKillISTThread );
        ExitThread(0);
    }

	// read state of GPIO pin
	DDKGpioReadIntrPin((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin, &u32State);
	RETAILMSG(1, (TEXT("IRQEventHandler::0x%x\r\n"), u32State ));

	// issue irq event on rising edge
	if(u32State)		
	{
		SetEvent( pIRQInstance->hExternIRQEvent );
	}

	// clear GPIO interrupt status 
	DDKGpioClearIntrPin((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin);
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
  	IRQINSTANCE*	pIRQInstance = (IRQINSTANCE*)hContext;
    ULONG			WaitReturn;

    /* Wait for the event that any serial port action creates.
     */
    while ( !pIRQInstance->KillThread ) 
	{
        WaitReturn = WaitForSingleObject( pIRQInstance->hIRQEvent, INFINITE);

		// IST Handler
        IRQEventHandler( hContext );

		//send EOI if required
		if(!pIRQInstance->bSendEOI)
		{
			InterruptDone(pIRQInstance->dwSysIntr);
		}
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
	
	InterruptDone(pIRQInstance->dwSysIntr);
	InterruptDisable(pIRQInstance->dwSysIntr);

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
   	if(!InterruptInitialize( pIRQInstance->dwSysIntr, pIRQInstance->hIRQEvent, NULL, 0))
	{
		return FALSE;
	}

	//CS&ZHL JUN-18-07: follow 'com_mmd2\mdd.c' to do this
    InterruptDone(pIRQInstance->dwSysIntr);

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
		*((PDWORD)pBufOut) = dwWaitReturn;		// = WAIT_OBJECT_0, WAIT_TIMEOUT and WAIT_FAILED

		if(pdwActualOut != NULL)
		{
			*pdwActualOut = (DWORD)(sizeof(DWORD));
		}
		break;

	case IOCTL_SEND_EOI:
		// clear GPIO interrupt status 
		DDKGpioClearIntrPin((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin);
		//
		// send EOI to permit following interrrupt requests
		//
		InterruptDone(pIRQInstance->dwSysIntr);
		break;

	default:
		// unsupported IOControl
		bRet = FALSE;
	}

	return bRet;
}

HANDLE IRQ_Init( DWORD dwContext )
{
    HKEY			hKey;
    LONG			regError;
	DWORD			dwDataSize;
	DWORD			dwTmp;
    IRQINSTANCE*	pIRQInstance = NULL;

	RETAILMSG(1, (TEXT("->IRQ_Init\r\n")));
    // Allocate irq structure.
    pIRQInstance = (IRQINSTANCE*)LocalAlloc(LPTR, sizeof(IRQINSTANCE));

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

	// read send EOI flag
	dwDataSize = sizeof(DWORD);
    regError = RegQueryValueEx(
                              hKey, 
                              _T("SendEOI"), 
                              NULL, 
                              NULL,
                              (LPBYTE)(&dwTmp), 
                              &dwDataSize);
    if ( regError != ERROR_SUCCESS ) 
	{
		dwTmp = 0;
		RETAILMSG(1, (TEXT("IRQ_Init::SendEOI registry unavailable, use default = %d.\r\n"), dwTmp));
	}
	if(!dwTmp)
	{
		pIRQInstance->bSendEOI = FALSE;
	}
	else
	{
		pIRQInstance->bSendEOI = TRUE;
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
    RegCloseKey (hKey);

    if ( regError != ERROR_SUCCESS ) 
	{
		RETAILMSG(1, (TEXT("IRQ_Init::read DeviceArrayIndex failed.\r\n")));
        LocalFree(pIRQInstance);
        return (NULL);
    }

	// get logical IRQ number and Gpio pin
	pIRQInstance->dwDeviceID = GetLogicalIrqNum(pIRQInstance->DeviceArrayIndex);
	pIRQInstance->dwIrqGpioPin = GetIrqGpioPin(pIRQInstance->DeviceArrayIndex);

	//
	// Request sysintr for the device
	//
	pIRQInstance->dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, 
						(PVOID)&pIRQInstance->dwDeviceID, 
						sizeof(pIRQInstance->dwDeviceID), 
						(PVOID)&pIRQInstance->dwSysIntr, 
						sizeof(pIRQInstance->dwSysIntr), NULL))
	{
		RETAILMSG(1, (TEXT("ERROR: Failed to request sysintr.\r\n")));
		pIRQInstance->dwSysIntr = (DWORD)SYSINTR_UNDEFINED;

		// release the instance
		LocalFree( pIRQInstance );
		return (NULL);
	}

	RETAILMSG(1, (TEXT("<-IRQ_Init:: IrqNo=%d, SysIntr=%d\r\n"), pIRQInstance->dwDeviceID, pIRQInstance->dwSysIntr));
    return (HANDLE) pIRQInstance;
}

BOOL IRQ_Deinit( DWORD hDeviceContext )
{
    IRQINSTANCE* pIRQInstance = (IRQINSTANCE*)hDeviceContext;

	if(pIRQInstance->dwSysIntr != SYSINTR_UNDEFINED)
	{
		KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, 
						&pIRQInstance->dwSysIntr, sizeof(pIRQInstance->dwSysIntr), 
						NULL, 0, 0);
	}

	LocalFree( pIRQInstance );
    return TRUE;
}

HANDLE IRQ_Open( HANDLE hDeviceContext,
				 DWORD AccessCode,
				 DWORD ShareMode )
{
	IRQINSTANCE*	pIRQInstance;
	DDK_GPIO_CFG	intrCfg;

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
	// And then, setup GPIO interrupt function
	// 
    DDKIomuxSetPinMux((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin, DDK_IOMUX_MODE_GPIO);
	DDKGpioEnableDataPin((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin, 0);		// output disable
    DDKIomuxSetPadConfig((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);  

	// config GPIO interrupt c
	intrCfg.DDK_PIN_IO           = DDK_GPIO_INPUT;
	intrCfg.DDK_PIN_IRQ_CAPABLE  = DDK_GPIO_IRQ_CAPABLE;
	intrCfg.DDK_PIN_IRQ_ENABLE   = DDK_GPIO_IRQ_DISABLED;				// DDK_GPIO_IRQ_ENABLED;
	intrCfg.DDK_PIN_IRQ_LEVEL    = DDK_GPIO_IRQ_EDGE;					// DDK_GPIO_IRQ_LEVEL;
	intrCfg.DDK_PIN_IRQ_POLARITY = DDK_GPIO_IRQ_POLARITY_HI;			// interrupt trigger on rising edge
	if(!DDKGpioConfig((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin, intrCfg))
	{
		RETAILMSG(1,(TEXT("IRQ_Open: config %d# pin failed\r\n"), (DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin));
		return NULL;
	}
	DDKGpioClearIntrPin((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin);

	// Finally, enable GPIO interrupt
	DDKGpioIntrruptEnable((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin);

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

	// clear interrupt flag
	DDKGpioClearIntrPin((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin);

	// disable interrupt
	DDKGpioIntrruptDisable((DDK_IOMUX_PIN)pIRQInstance->dwIrqGpioPin);

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


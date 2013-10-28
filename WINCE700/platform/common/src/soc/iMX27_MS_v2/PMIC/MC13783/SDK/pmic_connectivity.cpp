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
// File:pmic_connectivity.cpp
//
// This file contains the SDK interface that is used by applications and other
// drivers to access connectivity feature of the MC13783.
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include "csparm_macros.h"
#include "regs.h"
#include "regs_connectivity.h"
#include "pmic_lla.h"
#include "pmic_ioctl.h"
#include "pmic_connectivity.h"
#include "pmic_convity_priv.h"


#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR		0
#define ZONEID_WARN			1
#define ZONEID_INIT			2
#define ZONEID_FUNC			3
#define ZONEID_INFO			4

// Debug zone masks
#define ZONEMASK_ERROR		 (1 << ZONEID_ERROR)
#define ZONEMASK_WARN		(1 << ZONEID_WARN)
#define ZONEMASK_INIT		(1 << ZONEID_INIT)
#define ZONEMASK_FUNC		(1 << ZONEID_FUNC)
#define ZONEMASK_INFO		(1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR			 DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN			DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT			DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC			DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO			DEBUGZONE(ZONEID_INFO)

extern DBGPARAM dpCurSettings;

#endif// DEBUG


//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables
extern HANDLE hPMI;

//-----------------------------------------------------------------------------
// Defines
#define CONVITY_INTR_USBI						0
#define CONVITY_INTR_IDI						1
#define CONVITY_INTR_SE1I						2
#define CONVITY_INTR_CKDETI						3

#define NUM_CONVITY_INTR						4

#define MC13783_INT_SEN0_CONVITYBITS_LSH			16
#define MC13783_INT_SEN0_CONVITYBITS_WID			7

#define MC13783_INT_SEN0_CONVITYBITS_USB4V4S		(1 << 0)
#define MC13783_INT_SEN0_CONVITYBITS_USB2V0S		(1 << 1)
#define MC13783_INT_SEN0_CONVITYBITS_USB0V8S		(1 << 2)
#define MC13783_INT_SEN0_CONVITYBITS_IDFLOATS		(1 << 3)
#define MC13783_INT_SEN0_CONVITYBITS_IDGNDS		(1 << 4)
#define MC13783_INT_SEN0_CONVITYBITS_SE1S			(1 << 5)
#define MC13783_INT_SEN0_CONVITYBITS_CKDETS		(1 << 6)

#define ENTRY_MSG		DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__))
#define EXIT_MSG		DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__))

//-----------------------------------------------------------------------------
// Types

// This enumeration is used to track the current state of each device handle.
typedef enum
{
    HANDLE_FREE,  // Handle is available for use.
    HANDLE_IN_USE // Handle is currently in use.
} HANDLE_STATE;


// This structure maintains the current state of the connectivity driver. This
// includes both the PMIC hardware state as well as the device handle and
// callback states.

typedef struct
{
    PMIC_CONVITY_HANDLE                       handle;           // Device handle.      
    HANDLE_STATE                              handleState;      // Device handle
                                                                // state.              
    PMIC_CONVITY_MODE                         mode;             // Device mode.        
    PMIC_CONVITY_CALLBACK                     callback;         // Event callback
                                                                // function pointer.   
    PMIC_CONVITY_EVENTS                       eventMask;        // Event mask.         
    PMIC_CONVITY_USB_SPEED                    usbSpeed;         // USB connection
                                                                // speed.              
    PMIC_CONVITY_USB_MODE                     usbMode;          // USB connection
                                                                // mode.               
    PMIC_CONVITY_USB_POWER_IN                 usbPowerIn;       // USB transceiver
                                                                // power source.       
    PMIC_CONVITY_USB_POWER_OUT                usbPowerOut;      // USB transceiver
                                                                // power output
                                                                // level.              
    PMIC_CONVITY_USB_TRANSCEIVER_MODE         usbXcvrMode;      // USB transceiver
                                                                // mode.               
    unsigned int                              usbDlpDuration;   // USB Data Line
                                                                // Pulsing duration.   
    PMIC_CONVITY_USB_OTG_CONFIG               usbOtgCfg;        // USB OTG
                                                                // configuration
                                                                // options.            
    PMIC_CONVITY_RS232_INTERNAL               rs232CfgInternal; // RS-232 internal
                                                                // connections.        
    PMIC_CONVITY_RS232_EXTERNAL               rs232CfgExternal; // RS-232 external
                                                                // connections.        
	BOOL                                      rs232TxTristated; // RS-232 TX state     

	PMIC_CONVITY_CEA936_DETECTION_CONFIG      cea936DetectCfg;  // CEA-936 device 
																// detection circuitry 
} PMIC_CONVITY_STATE_STRUCT;


//-----------------------------------------------------------------------------
// Global Variables

// This structure defines the reset/power on state for the Connectivity driver.
static const PMIC_CONVITY_STATE_STRUCT reset =
{
	0,
	HANDLE_FREE,
	USB,
	NULL,
	(PMIC_CONVITY_EVENTS)0,
	USB_FULL_SPEED,
	USB_PERIPHERAL,
	USB_POWER_INTERNAL,
	USB_POWER_3V3,
	USB_TRANSCEIVER_OFF,
	0,
	(PMIC_CONVITY_USB_OTG_CONFIG)(USB_VBUS_CURRENT_LIMIT_HIGH),
	RS232_TX_USE0VM_RX_UDATVP,
	RS232_TX_UDM_RX_UDP,
	FALSE,
	ACCESSORY_ID_DP150KPU
};

// This structure maintains the current state of the Connectivity driver.
static PMIC_CONVITY_STATE_STRUCT convity;


// We use a mutex to ensure that no race condition arises in register states (inside ioctl handlers)
// as well as convity data structure between the API calls and interrupt service thread.

static CRITICAL_SECTION mutex;

static PMIC_PARAM_CONVITY_OP param;

static HANDLE hThread = NULL;
static bool threadContinue = 0;

// data structures that help handle all the connectivity interrupts in a loop.
static HANDLE hEventTab[] = {NULL, NULL, NULL, NULL};
static WCHAR* evtNames[] = 
{
	TEXT("EVENT_CONVITY_USBI"), 
	TEXT("EVENT_CONVITY_IDI"), 
	TEXT("EVENT_CONVITY_SE1I"),
	TEXT("EVENT_CONVITY_CKDETI")
};
static PMIC_INT_ID interruptTab[] = 
{
	PMIC_MC13783_INT_USBI, 
	PMIC_MC13783_INT_IDI, 
	PMIC_MC13783_INT_SE1I,
	PMIC_MC13783_INT_CKDETI
};


//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions


PMIC_STATUS PmicConvityInit(void)
{
	ENTRY_MSG;

	// Initialize the driver state to default state.
	convity = reset;

	// Initialize mutex.
	InitializeCriticalSection(&mutex);

	EXIT_MSG;
	return PMIC_SUCCESS;
}

void PmicConvityDeinit(void)
{
	ENTRY_MSG;
	// Free mutex.
	DeleteCriticalSection(&mutex);
	EXIT_MSG;
}


// Read the sense bits relevant to Connectivity from MC13783 interrupt sense
// 0 register.
static UINT32 GetSenseBits()
{
	UINT32 addr, temp;
	addr = MC13783_INT_SEN0_ADDR;

	if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
			&temp, sizeof(temp), NULL, NULL))
		return 0;

	return CSP_BITFEXT(temp, MC13783_INT_SEN0_CONVITYBITS);
}

// Look at the interrupt, new and previous sense bits and find out the occurred events
static PMIC_CONVITY_EVENTS GetSignalledEvents(int intrIndex, UINT32 oldSenseBits,
		UINT32 newSenseBits)
{
	UINT32 signals = 0;

	switch (intrIndex)
	{
	case CONVITY_INTR_USBI:
		// Check if 4.4V sense bit has changed.
		if ((oldSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB4V4S) ^
			(newSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB4V4S))
		{
			signals |= (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB4V4S) ?
				USB_DETECT_4V4_RISE : USB_DETECT_4V4_FALL;
		}
		// Check if 2.0V sense bit has changed.
		if ((oldSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB2V0S) ^
			(newSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB2V0S))
		{
			signals |= (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB2V0S) ?
				USB_DETECT_2V0_RISE : USB_DETECT_2V0_FALL;
		}
		// Check if 0.8V sense bit has changed.
		if ((oldSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB0V8S) ^
			(newSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB0V8S))
		{
			signals |= (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB0V8S) ?
				USB_DETECT_0V8_RISE : USB_DETECT_0V8_FALL;
		}
		break;

	case CONVITY_INTR_IDI:
		//  Look at the current value of IDFLOATS and IDGNDS bits and determine the event.
		//			IDFLOATS		IDGNDS				Meaning
		//			============================================
		//				0				0				Non-USB accessory is attached.
		//				0				1				A type plug (USB host) is attached.
		//				1				0				B type plug (USB peripheral, OTG device 
		//												or no device) is attached.
		//				1				1				Factory mode.
		if (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_IDFLOATS)
		{
			signals |= (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_IDGNDS) ?
				USB_DETECT_FACTORY_MODE : USB_DETECT_MINI_B;
		}
		else
		{
			signals |= (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_IDGNDS) ?
				USB_DETECT_MINI_A : USB_DETECT_NON_USB_ACCESORY;
		}
		break;

	case CONVITY_INTR_SE1I:
		// Look at SE1S to find out if it is rise or fall event.
		signals |= (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_SE1S) ?
				USB_DETECT_SE1_RISE : USB_DETECT_SE1_FALL;
		break;

	case CONVITY_INTR_CKDETI:
		// CKDETI occurs only when CKDETS changed from 0 to 1. (CKDETS is L2H only).
		signals |= USB_DETECT_CKDETECT;
		break;

	default:
		break;
	}
	
	return (PMIC_CONVITY_EVENTS)signals;
}

//  Thread that waits for interrupt events to be signalled. Upon occurrence of an interrupt,
//  it finds out the event bits to inform the callback function.
static BOOL PmicConvityIsrThreadProc(LPVOID lpParam)
{
	// This variable keeps the previous known value of the sense bits. Upon occurrence
	// of an interrupt, the current values of sense bits and this (previous) value is used
	// to find out which event has occurred. Needed for USBI related sense bits only.
	static UINT32 senseBits = 0;
	int iRetVal;
	int iCount = 0;
	UINT32 newSenseBits;
	UINT32 events;


	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	senseBits = GetSenseBits();
	while(threadContinue != 0)
	{
		iRetVal = WaitForMultipleObjects(NUM_CONVITY_INTR, hEventTab, FALSE, INFINITE);
		if ((iRetVal >= WAIT_OBJECT_0) && (iRetVal <= WAIT_OBJECT_0 + NUM_CONVITY_INTR - 1))
		{
			iRetVal -= WAIT_OBJECT_0;
			newSenseBits = GetSenseBits();
			events = GetSignalledEvents(iRetVal, senseBits, newSenseBits);
			events &= convity.eventMask;

			if ((convity.callback != NULL) && events)
				(*convity.callback)(events);

			// save the new sense bits in the global variable senseBits.
			senseBits = newSenseBits;

			// inform the system that interrupt handling is complete.
			PmicInterruptHandlingComplete(interruptTab[iRetVal]);
		}
	}

	ExitThread(TRUE);

	return TRUE;
}

// cleans up callback related data structures.
static PMIC_STATUS cleanup()
{
	int i;

	threadContinue = 0;
	for (i = 0; i < NUM_CONVITY_INTR; i++)
	{
		if (hEventTab[i])
		{
			PmicInterruptDeregister(interruptTab[i]);
			hEventTab[i] = NULL;
		}
	}
	hThread = NULL;
	convity.callback = (PMIC_CONVITY_CALLBACK)NULL;
	convity.eventMask = (PMIC_CONVITY_EVENTS)0;

	return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
// Function: PmicConvityOpen
//
// Attempt to open and gain exclusive access to the PMIC connectivity
// hardware. An initial operating mode must also be specified.
//
// If the open request is successful, then a numeric handle is returned
// and this handle must be used in all subsequent function calls. The
// same handle must also be used in the pmic_convity_close() call when use
// of the PMIC connectivity hardware is no longer required.
//
// Parameters:
//            handle          device handle from open() call
//            mode            initial connectivity operating mode
//
// Returns:
//           PMIC_SUCCESS    if the open request was successful
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityOpen(PMIC_CONVITY_HANDLE *const handle,
                            const PMIC_CONVITY_MODE    mode)
{
    PMIC_STATUS rc = PMIC_ERROR;

	ENTRY_MSG;
    if (handle == (PMIC_CONVITY_HANDLE *)NULL)
    {
        // Do not dereference a NULL pointer. 
        return PMIC_ERROR;
    }

    // We need a critical section here to avoid problems in case
    // multiple calls to PmicConvityOpen() are made since we can only
    // allow one of them to succeed.
     
	EnterCriticalSection(&mutex);

    // Check the current device handle state and acquire the handle if
    // it is available.
     
    if (convity.handleState != HANDLE_FREE)
    {
        // Cannot open the PMIC connectivity hardware at this time or an invalid
        // mode was requested.
         
        *handle = reset.handle;
    }
    else
    {
        // Let's begin by acquiring the connectivity device handle. 
        convity.handle = (PMIC_CONVITY_HANDLE)(&convity);
        convity.handleState = HANDLE_IN_USE;

        // Then we can try to set the desired operating mode. 
		param.op = OP_SET_MODE;
		param.PARAMS.ifMode = mode;

		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_OP, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);
		
        if (rc == PMIC_SUCCESS)
        {
            // Successfully set the desired operating mode, now return the
            // handle to the caller.
             
            *handle = convity.handle;
	        convity.mode = mode;
        }
        else
        {
            // Failed to set the desired mode, return the handle to an unused
            // state.
             
            convity.handle      = reset.handle;
            convity.handleState = reset.handleState;

            *handle = reset.handle;
        }
    }

    // Exit the critical section. 
    LeaveCriticalSection(&mutex);
	
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityClose
//
// Terminate further access to the PMIC connectivity hardware. Also allows
// another process to call PmicConvityOpen() to gain access.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
// Returns:
//           PMIC_SUCCESS    if the open request was successful
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityClose(const PMIC_CONVITY_HANDLE handle)
{
    PMIC_STATUS rc = PMIC_ERROR;

	ENTRY_MSG;
    // Begin a critical section here to avoid the possibility of race
    // conditions if multiple threads happen to call this function and
    // PmicConvityOpen() at the same time.
     
    EnterCriticalSection(&mutex);

    // Confirm that the device handle matches the one assigned in the
    // PmicConvityOpen() call and then close the connection.
     
    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
        rc = PMIC_SUCCESS;

        // Deregister for all existing callbacks if necessary and make sure
        // that the event handling settings are consistent following the
        // close operation.
         
        if (convity.callback != reset.callback)
        {
            // Deregister the existing callback function and all registered
            // events before we completely close the handle.
             
            rc = cleanup();
        }

        if (rc == PMIC_SUCCESS)
        {
            // Mark the connectivity device handle as being closed. 
            convity.handle      = reset.handle;
            convity.handleState = reset.handleState;
        }
    }

    // Exit the critical section. 
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}
//------------------------------------------------------------------------------
// Function: PmicConvitySetMode
//
// Change the current operating mode of the PMIC connectivity hardware.
// The available connectivity operating modes is hardware dependent and
// consists of one or more of the following: USB (including USB On-the-Go),
// RS-232, and CEA-936. Requesting an operating mode that is not supported
// by the PMIC hardware will return PMIC_NOT_SUPPORTED.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            mode            desired operating mode
// Returns:
//           PMIC_SUCCESS    if the requested mode was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvitySetMode(const PMIC_CONVITY_HANDLE handle,
                               const PMIC_CONVITY_MODE   mode)
{
    PMIC_STATUS rc = PMIC_ERROR;

	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = OP_SET_MODE;
		param.PARAMS.ifMode = mode;
		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_OP, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);
	    if (rc == PMIC_SUCCESS)
		{
			convity.mode = mode;
		}
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}
//------------------------------------------------------------------------------
// Function: PmicConvityGetMode
//
//  Get the current operating mode for the PMIC connectivity hardware.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            mode            the current PMIC connectivity operating mode
// Returns:
//           PMIC_SUCCESS    if the operation was successful
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityGetMode(const PMIC_CONVITY_HANDLE handle,
                               PMIC_CONVITY_MODE *const  mode)
{
    PMIC_STATUS rc = PMIC_ERROR;

	ENTRY_MSG;
    // Use a critical section to maintain a consistent state. 
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle)             &&
        (convity.handleState == HANDLE_IN_USE) &&
        (mode != (PMIC_CONVITY_MODE *)NULL))
    {
        *mode = convity.mode;

        rc = PMIC_SUCCESS;
    }

    // Exit the critical section. 
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}
//------------------------------------------------------------------------------
// Function: PmicConvityReset
//
//  Restore all registers to the initial power-on/reset state.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
// Returns:
//           PMIC_SUCCESS    if the reset was successful
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityReset(const PMIC_CONVITY_HANDLE handle)
{
    PMIC_STATUS rc = PMIC_ERROR;

	ENTRY_MSG;
    // Use a critical section to maintain a consistent state. 
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
        // Reset the PMIC Connectivity register to it's power on state. 
		param.op = OP_RESET;
		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_OP, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);

        if (rc == PMIC_SUCCESS)
        {
            // Also reset the device driver state data structure. 
            convity = reset;

            // need to restore the handle and state.
            convity.handle = handle;
            convity.handleState = HANDLE_IN_USE;
        }
    }

    // Exit the critical section. 
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvitySetCallback
//
// Register a callback function that will be used to signal PMIC connectivity
// events. For example, the USB subsystem should register a callback function
// in order to be notified of device connect/disconnect events. Note, however,
// that non-USB events may also be signalled depending upon the PMIC hardware
// capabilities. Therefore, the callback function must be able to properly
// handle all of the possible events if support for non-USB peripherals is
// also to be included.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            func            a pointer to the callback function
//            eventMask       a mask selecting events to be notified
// Returns:
//           PMIC_SUCCESS    if the callback was successful registered
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvitySetCallback(const PMIC_CONVITY_HANDLE   handle,
                                   const PMIC_CONVITY_CALLBACK func,
                                   const PMIC_CONVITY_EVENTS   eventMask)
{
    PMIC_STATUS rc = PMIC_ERROR;
	int i;
	
	ENTRY_MSG;
    //We need to start a critical section here to ensure a consistent state
    //in case simultaneous calls to pmic_convity_set_callback() are made. In
    //that case, we must serialize the calls to ensure that the "callback"
    //and "eventMask" state variables are always consistent.

    //Note that we don't actually need to acquire the spinlock until later
    //when we are finally ready to update the "callback" and "eventMask"
    //state variables which are shared with the interrupt handler.

    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
        //Return an error if either the callback function or event mask
        //is not properly defined.

        //It is also considered an error if a callback function has already
        //been defined. If you wish to register for a new set of events,
        //then you must first call pmic_convity_clear_callback() to
        //deregister the existing callback function and list of events
        //before trying to register a new callback function.

        if ((func == NULL) || (eventMask == 0) || (convity.callback != NULL) ||
			(threadContinue != 0))
        {
            rc = PMIC_ERROR;
        }
        else
        {
			// create all the events; start the thread.
			//all the events are received. Only those for which there is a callback
			// registered, are intimated.

			for (i = 0; i < NUM_CONVITY_INTR; i++)
			{
                // create event for PMIC interrupt signaling
				//	pEventAttributes = NULL (must be NULL)
				//	bManualReset = FALSE => resets automatically to nonsignaled
				//							state after waiting thread released
				//	bInitialState = FALSE => initial state is non-signaled
				//	lpName = => object created with a name

				hEventTab[i] = CreateEvent(NULL, FALSE, FALSE, evtNames[i]);

				// check if CreateEvent failed
				if (hEventTab[i] == NULL)
				{
					DEBUGMSG(ZONE_ERROR, (TEXT("%s(): CreateEvent failed!\r\n"),
						__WFUNCTION__));
					cleanup();
					goto ExitLoop;
				}
	
				// Register for interrupts.
				if (PmicInterruptRegister(interruptTab[i],
                                          evtNames[i])!= PMIC_SUCCESS)
				{
					DEBUGMSG(ZONE_ERROR, (TEXT("%s(): PmicInterruptRegister failed!\r\n"),
						__WFUNCTION__));
					cleanup();
					goto ExitLoop;
				}

				// Make sure interrupt is unmasked
				if (PmicInterruptEnable(interruptTab[i]) != PMIC_SUCCESS)
				{
					DEBUGMSG(ZONE_ERROR, (TEXT("%s(): PmicInterruptEnable failed!\r\n"),
						__WFUNCTION__));
					cleanup();
					goto ExitLoop;
				}
			}
 
			// Since same thread takes care of all the events,
			// create one thread
			
			threadContinue = 1;
			hThread = CreateThread(NULL, 0,
						(LPTHREAD_START_ROUTINE)PmicConvityIsrThreadProc,
						(LPVOID)0, 0, NULL);
			if (!hThread)
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("%s(): CreateThread failed!\r\n"),
					__WFUNCTION__));
				cleanup();
				goto ExitLoop;	
			}

            // Successfully registered for all events.
			rc = PMIC_SUCCESS;
            convity.callback  = func;
            convity.eventMask = eventMask;
        }
    }

ExitLoop:
    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityClearCallback
//
// Clears the current callback function. If this function returns successfully
// then all future Connectivity events will only be handled by the default
// handler within the Connectivity driver.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
// Returns:
//           PMIC_SUCCESS    if the callback was successful cleared
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityClearCallback(const PMIC_CONVITY_HANDLE handle)
{
    PMIC_STATUS rc = PMIC_ERROR;

	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
        rc = cleanup();
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityGetCallback
//
// Get the current callback function and event mask.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            func            the current callback function
//            eventMask       the current event selection mask
// Returns:
//           PMIC_SUCCESS    if the callback information was successful
//                           retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityGetCallback(const PMIC_CONVITY_HANDLE        handle,
                                   PMIC_CONVITY_CALLBACK     *const func,
                                   PMIC_CONVITY_EVENTS       *const eventMask)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle)              &&
        (convity.handleState == HANDLE_IN_USE)  &&
        (func != (PMIC_CONVITY_CALLBACK *)NULL) &&
        (eventMask != (PMIC_CONVITY_EVENTS *)NULL))
    {
        *func      = convity.callback;
        *eventMask = convity.eventMask;

        rc = PMIC_SUCCESS;
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityGetEventStatus
// 
// Get the connectivity Event status
//
//  Parameters:
//            handle          device handle from PmicConvityOpen() call
//            events          the currently signalled events
//
//  Returns:
//           PMIC_SUCCESS    if the transceiver speed was successfully set
//------------------------------------------------------------------------------

PMIC_STATUS PmicConvityGetEventStatus(const PMIC_CONVITY_HANDLE handle,
                                    PMIC_CONVITY_EVENTS *const events)
{
    PMIC_STATUS rc = PMIC_ERROR;
    UINT32 newSenseBits, eventSet;

    ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle)              &&
        (convity.handleState == HANDLE_IN_USE)  &&
        (events != (PMIC_CONVITY_EVENTS *)NULL))
    {
        newSenseBits = GetSenseBits();
        eventSet = 0;
        if (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB4V4S)
            eventSet |= USB_DETECT_4V4_RISE;
        if (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB2V0S)
            eventSet |= USB_DETECT_2V0_RISE;
        if (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_USB0V8S)
            eventSet |= USB_DETECT_0V8_RISE;
        eventSet |= GetSignalledEvents(CONVITY_INTR_IDI, 0, newSenseBits);
        if (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_SE1S)
            eventSet |= USB_DETECT_SE1_RISE;
        if (newSenseBits & MC13783_INT_SEN0_CONVITYBITS_CKDETS)
            eventSet |= USB_DETECT_CKDETECT;
		*events = (PMIC_CONVITY_EVENTS)eventSet;
        rc = PMIC_SUCCESS;
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
    EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Set the USB transceiver speed.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            speed           the desired USB transceiver speed
//            mode            the USB transceiver mode
// Returns:
//           PMIC_SUCCESS    if the transceiver speed was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbSetSpeed(const PMIC_CONVITY_HANDLE    handle,
                                   const PMIC_CONVITY_USB_SPEED speed,
                                   const PMIC_CONVITY_USB_MODE  mode)
{
    PMIC_STATUS rc = PMIC_ERROR;

	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = 0;
		param.PARAMS.USB_SPEED.usbSpeed = speed;
		param.PARAMS.USB_SPEED.usbMode = mode;
		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_USB_SETSPEED, &param,
                        sizeof(param), &rc, sizeof(rc), NULL, NULL);
        if (rc == PMIC_SUCCESS)
        {
			convity.usbSpeed = speed;
			convity.usbMode  = mode;
		}
    }
    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbGetSpeed
//
// Get the USB transceiver speed.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            speed           the current USB transceiver speed
//            mode            the current USB transceiver mode
// Returns:
//           PMIC_SUCCESS    if the transceiver speed was successfully obtained
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbGetSpeed(const PMIC_CONVITY_HANDLE     handle,
                                   PMIC_CONVITY_USB_SPEED *const speed,
                                   PMIC_CONVITY_USB_MODE *const  mode)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle)                &&
        (convity.handleState == HANDLE_IN_USE)    &&
        (speed != (PMIC_CONVITY_USB_SPEED *)NULL) &&
        (mode != (PMIC_CONVITY_USB_MODE *)NULL))
    {
        *speed = convity.usbSpeed;
        *mode  = convity.usbMode;

        rc = PMIC_SUCCESS;
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbSetPowerSource
//
// Set the USB transceiver's power supply configuration.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            pwrin           USB transceiver regulator input power source
//            pwrout          USB transceiver regulator output power level
// Returns:
//           PMIC_SUCCESS    if the USB transceiver's power supply
//                           configuration was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbSetPowerSource(const PMIC_CONVITY_HANDLE        handle,
                                         const PMIC_CONVITY_USB_POWER_IN  pwrin,
                                         const PMIC_CONVITY_USB_POWER_OUT pwrout)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = 0;
		param.PARAMS.USB_PWR.pwrin = pwrin;
		param.PARAMS.USB_PWR.pwrout = pwrout;
		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_USB_SETPWR, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);
		if (rc == PMIC_SUCCESS)
		{
			convity.usbPowerIn  = pwrin;
			convity.usbPowerOut = pwrout;
		}
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbGetPowerSource
//
// Get the USB transceiver's current power supply configuration.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            pwrin           USB transceiver regulator input power source
//            pwrout          USB transceiver regulator output power level
// Returns:
//           PMIC_SUCCESS    if the USB transceiver's power supply
//                           configuration was successfully retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbGetPowerSource(const PMIC_CONVITY_HANDLE         handle,
                                         PMIC_CONVITY_USB_POWER_IN  *const pwrin,
                                         PMIC_CONVITY_USB_POWER_OUT *const pwrout)
{
    PMIC_STATUS rc = PMIC_ERROR;

	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle)                   &&
        (convity.handleState == HANDLE_IN_USE)       &&
        (pwrin != (PMIC_CONVITY_USB_POWER_IN *)NULL) &&
        (pwrout != (PMIC_CONVITY_USB_POWER_OUT *)NULL))
    {
        *pwrin  = convity.usbPowerIn;
        *pwrout = convity.usbPowerOut;

        rc = PMIC_SUCCESS;
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbSetXcvr
//
// Set the USB transceiver's operating mode.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            mode            desired operating mode
// Returns:
//           PMIC_SUCCESS    if the USB transceiver's operating mode
//                           was successfully configured
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbSetXcvr(const PMIC_CONVITY_HANDLE             handle,
                                  const PMIC_CONVITY_USB_TRANSCEIVER_MODE mode)
{
    PMIC_STATUS rc = PMIC_ERROR;

	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = 0;
		param.PARAMS.usbXcvrMode = mode;
		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_USB_SETXCVR, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);
        if (rc == PMIC_SUCCESS)
        {
            convity.usbXcvrMode = mode;
        }
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbGetXcvr
//
// Get the USB transceiver's current operating mode.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            mode            current operating mode
// Returns:
//           PMIC_SUCCESS    if the USB transceiver's operating mode
//                           was successfully retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbGetXcvr(const PMIC_CONVITY_HANDLE              handle,
                                  PMIC_CONVITY_USB_TRANSCEIVER_MODE *const mode)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle)             &&
        (convity.handleState == HANDLE_IN_USE) &&
        (mode != (PMIC_CONVITY_USB_TRANSCEIVER_MODE *)NULL))
    {
        *mode = convity.usbXcvrMode;

        rc = PMIC_SUCCESS;
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgSetDlpDuration
//
// Set the Data Line Pulse duration (in milliseconds) for the USB OTG
// Session Request Protocol.
// For MC13783, the get/set DLP duration APIs are not supported.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            duration        the data line pulse duration (ms)
// Returns:
//           PMIC_SUCCESS    if the pulse duration was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgSetDlpDuration(const PMIC_CONVITY_HANDLE handle,
                                            const unsigned int      duration)
{
    PMIC_STATUS rc = PMIC_NOT_SUPPORTED;
	ENTRY_MSG;

    // The DLP duration is not supported by the MC13783 PMIC hardware.
    // No critical section is required.
    if ((handle != convity.handle) || (convity.handleState != HANDLE_IN_USE))
    {
        // Must return error indication for invalid handle parameter to be
        // consistent with other APIs.
        rc = PMIC_ERROR;
    }
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgGetDlpDuration
//
// Get the current Data Line Pulse duration (in milliseconds) for the USB
// OTG Session Request Protocol.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            duration        the data line pulse duration (ms)
// Returns:
//           PMIC_SUCCESS    if the pulse duration was successfully obtained
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgGetDlpDuration(const PMIC_CONVITY_HANDLE handle,
                                            unsigned int *const     duration)
{
    PMIC_STATUS rc = PMIC_NOT_SUPPORTED;
	
	ENTRY_MSG;
    // The DLP duration is not supported by the MC13783 PMIC hardware.
    // No critical section is required.
    if ((handle != convity.handle) || (convity.handleState != HANDLE_IN_USE))
    {
        // Must return error indication for invalid handle parameter to be
        // consistent with other APIs.
        rc = PMIC_ERROR;
    }
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgBeginHnp
//
// Explicitly start the USB OTG Host Negotiation Protocol (HNP) process.
// This simply involves pulling the D+ line high for the "A" device
// and disconnecting all pull-up and pull-down resistors for the "B"
// device.
// Note that the pmic_convity_usb_otg_end_hnp() function must be called
// after a suitable time has elapsed to complete the HNP process.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            deviceType      the USB device type (either A or B)
// Returns:
//           PMIC_SUCCESS    if the HNP was successfully started
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgBeginHnp(const PMIC_CONVITY_HANDLE          handle,
                                      const PMIC_CONVITY_USB_DEVICE_TYPE deviceType)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = 0;
		param.PARAMS.usbOtgType = deviceType;
 		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_USB_BGNHNP, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgEndHnp
//
// Explicitly complete the USB OTG Host Negotiation Protocol (HNP) process.
// This just involves disconnecting the pull-up resistor on D+ for the "A"
// device and turning on the pull-up resistor on D+ for the "B" device.
//
// Note that this function should only be called after a suitable time has
// elapsed after calling pmic_convity_usb_otg_begin_hnp().
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            deviceType      the USB device type (either A or B)
// Returns:
//           PMIC_SUCCESS    if the HNP was successfully ended
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgEndHnp(const PMIC_CONVITY_HANDLE          handle,
                                    const PMIC_CONVITY_USB_DEVICE_TYPE deviceType)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = 0;
		param.PARAMS.usbOtgType = deviceType;
 		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_USB_ENDHNP, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgSetConfig
//
// Set the USB On-The-Go (OTG) configuration.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             desired USB OTG configuration
// Returns:
//           PMIC_SUCCESS    if the OTG configuration was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgSetConfig(const PMIC_CONVITY_HANDLE      handle,
                                       const PMIC_CONVITY_USB_OTG_CONFIG cfg)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = 0;
		param.PARAMS.usbOtgCfg = cfg;
 		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_USB_SETCFG, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);

		if (rc == PMIC_SUCCESS)
		{
			if ((cfg & USB_VBUS_CURRENT_LIMIT_HIGH) ||
				(cfg & USB_VBUS_CURRENT_LIMIT_LOW))
			{
				// Make sure that the VBUS current limit state is
				// correctly set to either USB_VBUS_CURRENT_LIMIT_HIGH
				// or USB_VBUS_CURRENT_LIMIT_LOW but never both at the
				// same time.

				// We guarantee this by first clearing both of the
				// status bits and then resetting the correct one.

				convity.usbOtgCfg = (PMIC_CONVITY_USB_OTG_CONFIG)
					(convity.usbOtgCfg & (~(USB_VBUS_CURRENT_LIMIT_HIGH |
									   USB_VBUS_CURRENT_LIMIT_LOW)));

			}
			convity.usbOtgCfg =
                        (PMIC_CONVITY_USB_OTG_CONFIG) (convity.usbOtgCfg | cfg);
		}
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgClearConfig
//
// Clears the USB On-The-Go (OTG) configuration. Multiple configuration settings
// may be OR'd together in a single call. However, selecting conflicting
// settings (e.g., multiple VBUS current limits) will result in undefined
// behavior.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             USB OTG configuration settings to be cleared.
// Returns:
//           PMIC_SUCCESS         If the OTG configuration was successfully
//                                cleared.
//           PMIC_ERROR           If the handle is invalid.
//           PMIC_NOT_SUPPORTED   If the desired USB OTG configuration is
//                                not supported by the PMIC hardware.
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgClearConfig(const PMIC_CONVITY_HANDLE handle,
                                         const PMIC_CONVITY_USB_OTG_CONFIG cfg)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = 0;
		param.PARAMS.usbOtgCfg = cfg;
 		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_USB_CLRCFG, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);

		if (rc == PMIC_SUCCESS)
		{
			convity.usbOtgCfg =
                      (PMIC_CONVITY_USB_OTG_CONFIG)(convity.usbOtgCfg & (~cfg));
		}
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgGetConfig
//
// Get the current USB On-The-Go (OTG) configuration.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             the current USB OTG configuration
// Returns:
//           PMIC_SUCCESS         if the OTG configuration was successfully
//                                retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgGetConfig(const PMIC_CONVITY_HANDLE       handle,
                                       PMIC_CONVITY_USB_OTG_CONFIG *const cfg)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle)             &&
        (convity.handleState == HANDLE_IN_USE) &&
        (cfg != (PMIC_CONVITY_USB_OTG_CONFIG *)NULL))
    {
        *cfg = convity.usbOtgCfg;

        rc = PMIC_SUCCESS;
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityRs232SetConfig
//
// Set the connectivity interface to the selected RS-232 operating
// configuration.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfgInternal     RS-232 transceiver internal connections
//            cfgExternal     RS-232 transceiver external connections
//            txTristated     RS-232 transceiver TX state
// Returns:
//           PMIC_SUCCESS     if the requested mode was set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityRs232SetConfig(const PMIC_CONVITY_HANDLE         handle,
                                      const PMIC_CONVITY_RS232_INTERNAL cfgInternal,
                                      const PMIC_CONVITY_RS232_EXTERNAL cfgExternal,
	                                  const BOOL                        txTristated)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = OP_RS232_SET_CFG;
		param.PARAMS.RS232_CFG.cfgInternal = cfgInternal;
		param.PARAMS.RS232_CFG.cfgExternal = cfgExternal;
		param.PARAMS.RS232_CFG.txTristated = txTristated;

 		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_RS232_OP, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);

		if (rc == PMIC_SUCCESS)
		{
			convity.rs232CfgInternal = cfgInternal;
			convity.rs232CfgExternal = cfgExternal;
			convity.rs232TxTristated = txTristated;
		}
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityRs232GetConfig
//
// Get the connectivity interface's current RS-232 operating configuration.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfgInternal     RS-232 transceiver internal connections
//            cfgExternal     RS-232 transceiver external connections
//            txTristated     RS-232 transceiver TX state
// Returns:
//           PMIC_SUCCESS     if the requested mode was retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityRs232GetConfig(const PMIC_CONVITY_HANDLE          handle,
                                      PMIC_CONVITY_RS232_INTERNAL *const cfgInternal,
                                      PMIC_CONVITY_RS232_EXTERNAL *const cfgExternal,
	                                  BOOL                        *const txTristated)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle)                           &&
        (convity.handleState == HANDLE_IN_USE)               &&
        (cfgInternal != (PMIC_CONVITY_RS232_INTERNAL *)NULL) &&
        (cfgExternal != (PMIC_CONVITY_RS232_EXTERNAL *)NULL) &&
		(txTristated != (BOOL *)NULL))
    {
        *cfgInternal = convity.rs232CfgInternal;
        *cfgExternal = convity.rs232CfgExternal;
		*txTristated = convity.rs232TxTristated;

        rc = PMIC_SUCCESS;
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityCea936SetDetectionConfig
//
// Set-up the circuitry helping in accessory type identification.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             detection related circuitry set-up,
//                            bit-wise OR allowed.
// Returns:
//           PMIC_SUCCESS     if the requested set-up was done successfully.
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityCea936SetDetectionConfig(const PMIC_CONVITY_HANDLE handle, 
	                             const PMIC_CONVITY_CEA936_DETECTION_CONFIG  cfg)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE))
    {
		param.op = OP_CEA936_SET_DETECT;
		param.PARAMS.cea936DetectCfg = cfg;

 		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_CEA936_OP, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);

		if (rc == PMIC_SUCCESS)
		{
			convity.cea936DetectCfg = cfg;
		}
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityCea936GetDetectionConfig
//
// Get the current set-up of circuitry helping in accessory type identification.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             detection related circuitry set-up,
//                            bit-wise OR allowed.
// Returns:
//           PMIC_SUCCESS     if the requested set-up was successfully retrieved.
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityCea936GetDetectionConfig(const PMIC_CONVITY_HANDLE handle,
	                             PMIC_CONVITY_CEA936_DETECTION_CONFIG *const cfg)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle)                           &&
        (convity.handleState == HANDLE_IN_USE)               &&
        (cfg != (PMIC_CONVITY_CEA936_DETECTION_CONFIG *)NULL))
    {
        *cfg = convity.cea936DetectCfg;

        rc = PMIC_SUCCESS;
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}

//------------------------------------------------------------------------------
// Function: PmicConvityCea936ExitSignal
//
// Signal the attached device to exit the current CEA-936 operating mode.
// Returns an error if the current operating mode is not CEA-936.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            signal          type of exit signal to be sent
// Returns:
//           PMIC_SUCCESS     if exit signal was sent
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityCea936ExitSignal(const PMIC_CONVITY_HANDLE        handle,
                                   const PMIC_CONVITY_CEA936_EXIT_SIGNAL signal)
{
    PMIC_STATUS rc = PMIC_ERROR;
	
	ENTRY_MSG;
    // Use a critical section to maintain a consistent state.
    EnterCriticalSection(&mutex);

    if ((handle == convity.handle) &&
        (convity.handleState == HANDLE_IN_USE) &&
		((convity.mode == CEA936_MONO) || (convity.mode == CEA936_STEREO) || 
		(convity.mode == CEA936_TEST_RIGHT) || (convity.mode == CEA936_TEST_LEFT)))
    {
		param.op = OP_CEA936_EXIT;
		param.PARAMS.cea936ExitSignal = signal;

 		DeviceIoControl(hPMI, PMIC_IOCTL_CONVT_CEA936_OP, &param, sizeof(param),
			&rc, sizeof(rc), NULL, NULL);
    }

    // Exit the critical section.
    LeaveCriticalSection(&mutex);
	EXIT_MSG;

    return rc;
}


//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{
//!
//! All rights reserved ADENEO 2007
//!
//! \brief		Stream driver for the Keypad embedded on the AT91SAM9262EK EvalBoard
//
//! \file keypad.cpp
//
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9261EK_v170_rc4/PLATFORM/AT91SAM9261EK/SRC/DRIVERS/KeyPad/keypad.cpp $
//!   $Author: pblanchard $
//!   $Revision: 1219 $
//!   $Date: 2007-08-03 16:59:11 +0200 (ven., 03 août 2007) $
//! \endif
//! 
//! The keypad driver is a DLL loaded by DeviceManager, named KPD.
//! It is a stream driver (KPD1), with some dummy functions (Open, Close, Read, Write, Seek, IOControl).
//! It is not loaded by GWES, using the Layout Manager.
//! It is written in C++. (class KeyPad)
//! It works from an IST, sending keyboard events with PostKeybdMessage().
//! It can be unloaded using "s unmountdrv KPD1:" and reloaded using "s activateDevice BuiltIn\KEYPAD"
//!
//-----------------------------------------------------------------------------
//! \addtogroup	Keypad
//! @{
//!

// System include
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <devload.h>

// Board include
#include "bsp.h"
#include "bsp_drivers.h"

// Local include
#include "keypad.hpp"


//------------------------------------------------------------------------------
// Defines
//
#define MIN(x,y)((x > y) ? y : x)

//------------------------------------------------------------------------------
// Externs
//
extern KeyPad* CustomKeyPadInit ();	

//------------------------------------------------------------------------------
// Global Variables
//


//------------------------------------------------------------------------------
// Local Variables
//

//------------------------------------------------------------------------------
// Local Functions
//

extern "C"
BOOL WINAPI DllMain(HANDLE hinstDll, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
             DisableThreadLibraryCalls((HMODULE) hinstDll);
            break;
    }

    return TRUE;
}

KeyPad::KeyPad()
{ 
	m_dwAutoRepeatKeysPerSec = KBD_AUTO_REPEAT_KEYS_PER_SEC_DEFAULT; 
	m_dwAutoRepeatInitialDelay = KBD_AUTO_REPEAT_INITIAL_DELAY_DEFAULT; 
	m_hInterruptEvent = NULL; 
	m_dwOldKeypadState = 0; 
	m_dwSysintr = (DWORD)SYSINTR_UNDEFINED;
	m_dwLastKeyPressedDate = 0;
	m_dwLastKeyPressed = 0;
}
    

BOOL KeyPad::KeyPadPowerOff(void)
{
	RETAILMSG(1,(TEXT("KeyPadPowerOff class\r\n")));

	return TRUE;
}


BOOL KeyPad::KeyPadPowerOn(void)
{   
	RETAILMSG(1,(TEXT("KeyPadPowerOn class\r\n")));

	return TRUE;
}

void KeyPad::SendKeyEvent (DWORD dwKeyID, DWORD dwKeystate)
{
	UCHAR VKey;
	UINT uiScanCode;
	DWORD dwFlags = 0;

	if (dwKeyID >= m_dwNbKeypadKeys)
	{
		ERRORMSG(1,(TEXT("Key id is out of bouds (%d)"), dwKeyID));
		return;
	}

	VKey = m_KeypadKeyToVKey[dwKeyID];
	uiScanCode = MapVirtualKey(VKey, 0);
	
	RETAILMSG(1,(TEXT("Key %d has been %s (VKEY %d / SCANCODE %d)\r\n"),dwKeyID,dwKeystate ?  L"Pressed" : L"Released",VKey,uiScanCode));
	
	if (dwKeystate == 0)	
	{
		dwFlags |= KEYEVENTF_KEYUP; 
	}
	if (uiScanCode & 0xFF00)
	{
		dwFlags |= KEYEVENTF_EXTENDEDKEY;
	}

	keybd_event(VKey, (BYTE)(uiScanCode & 0xFF), dwFlags, 0); 
}

BOOL KeyPad::IsrThreadProc(void)
{	
	DWORD dwTimeout = m_dwPollingTimeOut;

	while( !m_bExitIST )
	{
		DWORD dwRet = WaitForSingleObject(m_hInterruptEvent, dwTimeout);
		DWORD dwKeyChangedMask;

		switch( dwRet )
		{
			// When an IT occure
			case WAIT_OBJECT_0:
				// Acknowledge the interrupt controller, re-enable the interrupt
				if (m_dwSysintr != SYSINTR_UNDEFINED)
				{
					InterruptDone(m_dwSysintr);
				}
				break;

			case WAIT_TIMEOUT:
				//DEBUGMSG(1, (_T("KPD> Timeout\r\n")));
				break;

			default:
				break;
		}

		// Get key code
		m_dwCurrentKeypadState = GetKeypadState();

		// Check keypad key changed
		dwKeyChangedMask = m_dwCurrentKeypadState ^ m_dwOldKeypadState;

		//
		if (dwKeyChangedMask)
		{
			for (DWORD i = 0  ;  i < m_dwNbKeypadKeys  ;  i++)
			{
				// The keypad key state has changed since the previous read
				if (dwKeyChangedMask & (1 << i))
				{
				
					SendKeyEvent(i, m_dwCurrentKeypadState & (1 << i));
					if (m_dwCurrentKeypadState & (1 << i))
					{
						m_dwLastKeyPressed = i;
						m_dwLastKeyPressedDate = GetTickCount();					
					}
				}			
			}
		}
		else
		{
			if (m_dwCurrentKeypadState & (1 << m_dwLastKeyPressed))
			{
				DWORD dwElapsedTime = GetTickCount() - m_dwLastKeyPressedDate;
				// No status change ...Key still pressed ... Let's try the repeat
				if (dwElapsedTime > m_dwRepeatTimeout)
				{
					m_dwRepeatTimeout = 1000/m_dwAutoRepeatKeysPerSec;
					m_dwLastKeyPressedDate = GetTickCount();
					SendKeyEvent(m_dwLastKeyPressed,1);
				}
				
			}

		}

		// Store the keypad keys states
		m_dwOldKeypadState = m_dwCurrentKeypadState;


		// Compute new time out
		if (m_dwCurrentKeypadState == 0)
		{
			// 
			dwTimeout = MIN(m_dwPollingTimeOut, INFINITE);

			m_dwRepeatTimeout = m_dwAutoRepeatInitialDelay;
		}
		else
		{
			dwTimeout = MIN(m_dwPollingTimeOut, m_dwRepeatTimeout);			
		}

	}

	return (TRUE);
}


DWORD KeyPad::KeyPadIsrThread(KeyPad *pKP)
{
    pKP->IsrThreadProc();

    return(0);
}


BOOL KeyPad::IsrThreadStart(void)
{
   m_bExitIST = FALSE;
   m_hISTthreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyPadIsrThread, this, 0, NULL);

   return(TRUE);
}

// Free interrupt and Delete event
BOOL KeyPad::Deinitialize() 
{
	// Delete thread
	m_bExitIST = TRUE;
    SetEvent(m_hInterruptEvent);

	DWORD dwRet = WaitForSingleObject(m_hISTthreadHandle, 250);
	switch( dwRet )
	{
		// When an IT occure
		case WAIT_OBJECT_0:
			DEBUGMSG(1, (_T("KPD> IST thread killed\r\n")));
			break;
		case WAIT_TIMEOUT:
			DEBUGMSG(1, (_T("KPD> Timeout - Failed to kill IST\r\n")));
			break;
		default:
			break;
	}
    

	if (m_dwSysintr != SYSINTR_UNDEFINED)
	{
		// Disable the interrupt
		InterruptDisable( m_dwSysintr );
	}

	// Delete event
    if (m_hInterruptEvent)
	{
		if (!CloseHandle(m_hInterruptEvent))
		{
			DEBUGMSG(TRUE, (TEXT("KPD> Failed to delete interrupt event\r\n")));
		}
	}

	//Power off the keypad
	KeyPadPowerOff();


	// Perform custom deinitialization
	CustomDeinitialize ();
	
	return TRUE;
}

BOOL KeyPad::Initialize(LPCTSTR pContext)
{
	BOOL bRet = TRUE;
	
	m_hInterruptEvent = NULL;
	
	
	// Perform custom initialisation
	bRet = CustomInitialize(pContext);

	if (!bRet)
	{
		return bRet;
	}


	// 
	m_hInterruptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	//
	if (m_dwSysintr == SYSINTR_UNDEFINED)
	{
        DEBUGMSG(1, (TEXT("Initialize: No sysintr available\r\n")));            
	}
	else
	{
		bRet = InterruptInitialize(m_dwSysintr, m_hInterruptEvent, NULL, 0);

		if (bRet == FALSE)
		{
	        ERRORMSG(1, (TEXT("Initialize: Faile to initialize interrupt with sysintr (%d)\r\n"), m_dwSysintr));            
			return bRet;
		}
	}

	return TRUE;
}

BOOL KeyPad::IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
	BOOL bResult = FALSE;

    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);

    switch(dwCode) 
	{
	case IOCTL_KBD_SET_MODIFIERS: 
		break;
		
    case IOCTL_KBD_SET_AUTOREPEAT:
        DEBUGMSG(1, (_T("IOControl: IOCTL_HID_SET_AUTOREPEAT\r\n")));            
        if ( (dwLenIn != sizeof(KBDI_AUTOREPEAT_INFO)) || (pBufIn == NULL) ) 
		{
            DEBUGMSG(1, (_T("IOControl: ERROR_INVALID_PARAMETER\r\n")));            
        }
        else 
		{
            // These global writes are not protected since the Layout Manager
            // serializes these IOCTLs.
            KBDI_AUTOREPEAT_INFO *pAutoRepeat = (KBDI_AUTOREPEAT_INFO*) pBufIn;
            
            INT32 iInitialDelay = pAutoRepeat->CurrentInitialDelay;
            INT32 iRepeatRate = pAutoRepeat->CurrentRepeatRate;
            
            iInitialDelay = max(KBD_AUTO_REPEAT_INITIAL_DELAY_MIN, iInitialDelay);
            iInitialDelay = min(KBD_AUTO_REPEAT_INITIAL_DELAY_MAX, iInitialDelay);
			
            if (iRepeatRate) { // Do not alter 0
                iRepeatRate = max(KBD_AUTO_REPEAT_KEYS_PER_SEC_MIN, iRepeatRate);
                iRepeatRate = min(KBD_AUTO_REPEAT_KEYS_PER_SEC_MAX, iRepeatRate);
            }
            
            m_dwRepeatTimeout = m_dwAutoRepeatInitialDelay = iInitialDelay;
            m_dwAutoRepeatKeysPerSec  = iRepeatRate;
			
            DEBUGMSG(1, (_T("Keypad : AutoRepeat intial delay = %u\r\n"),iInitialDelay));
            DEBUGMSG(1, (_T("Keypad : AutoRepeat keys/sec = %u\r\n"),iRepeatRate));
        }
        break;
    }

	return bResult;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// CS&ZHL JLY-17-2008: make C routines of KBD_XXX driver from C++ class member routines
//
////////////////////////////////////////////////////////////////////////////////////////////
extern "C"
DWORD Init(LPCTSTR pContext)
{
    DWORD dwRet = 0;
    
	KeyPad *pKeyPad = CustomKeyPadInit();
	
	if (pKeyPad == NULL)
	{
        DEBUGMSG(TRUE, (TEXT("KPD> Failed to allocate keypad controller object (Error=0x%x)\r\n"), GetLastError()));
        goto EXIT;
    }

    // Initialize the keypad controller object.
    if (!pKeyPad->Initialize(pContext))
    {
        DEBUGMSG(TRUE, (TEXT("KPD> Failed to initialize keypad control object\r\n")));
        goto EXIT;
    }

    // Create the keypad interrupt service thread (handles key presses).
    if (!pKeyPad->IsrThreadStart())
    {
        DEBUGMSG(TRUE, (TEXT("KPD> Failed to initialize keypad interrupt service thread\r\n")));
        goto EXIT;
    }

	//
	// CS&ZHL JLY-17-2008 commented: return the KeyPad class instance
	//
    dwRet = (DWORD)pKeyPad;
    
EXIT:
    if (dwRet == 0 && pKeyPad != NULL) {
        pKeyPad->Deinitialize();
    }

    return dwRet;
}

extern "C"
BOOL Deinit(DWORD hDeviceContext) 
{
    ASSERT(hDeviceContext != NULL);
    return ((KeyPad *) hDeviceContext)->Deinitialize();
}

extern "C"
void PowerDown(DWORD hDeviceContext)
{
	ASSERT(hDeviceContext != NULL);
	((KeyPad *) hDeviceContext)->KeyPadPowerOff();
	RETAILMSG(1,(TEXT("KeyPadPowerOff\r\n")));
}

extern "C"
void PowerUp(DWORD hDeviceContext)
{
	ASSERT(hDeviceContext != NULL);
	((KeyPad *) hDeviceContext)->KeyPadPowerOn();
	RETAILMSG(1,(TEXT("KeyPadPowerOn\r\n")));
}

extern "C" 
BOOL IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{ 
	ASSERT(hOpenContext != NULL);
	return ((KeyPad *) hOpenContext)->IOControl(dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
}



//-----------------------------------------------------------------------------
/// Here for compatibility
extern "C" 
{
	DWORD Open(DWORD hDeviceContext, DWORD dwAccess, DWORD dwShareMode) 
	{ 
		UNREFERENCED_PARAMETER(dwAccess);
		UNREFERENCED_PARAMETER(dwShareMode);

		return hDeviceContext; 
	}

	BOOL Close(DWORD hOpenContext) 
	{ 
		UNREFERENCED_PARAMETER(hOpenContext);

		return TRUE; 
	}

	DWORD Read(DWORD hOpenContext, LPVOID pBuf, DWORD Len) 
	{ 
		UNREFERENCED_PARAMETER(hOpenContext);
		UNREFERENCED_PARAMETER(pBuf);
		UNREFERENCED_PARAMETER(Len);

		return (DWORD)-1; 
	}

	DWORD Write(DWORD hOpenContext, LPCVOID pBuf, DWORD Len) 
	{ 
		UNREFERENCED_PARAMETER(hOpenContext);
		UNREFERENCED_PARAMETER(pBuf);
		UNREFERENCED_PARAMETER(Len);

		return (DWORD)-1; 
	}

	DWORD Seek(DWORD hOpenContext, long pos, DWORD type) 
	{ 
		UNREFERENCED_PARAMETER(hOpenContext);
		UNREFERENCED_PARAMETER(pos);
		UNREFERENCED_PARAMETER(type);

		return (DWORD)(-1); 
	}

} // end of extern "C"

//! @}

//! @}
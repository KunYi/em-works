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
	m_dwSysintr = (DWORD)SYSINTR_UNDEFINED;
	//m_dwOldKeypadState = 0; 
	//m_dwLastKeyPressedDate = 0;
	//m_dwLastKeyPressed = 0;
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

//----------------------------------------------------------------------------
// dwKeyState = 0: key is up; = 1: key is down
//----------------------------------------------------------------------------
void KeyPad::SendKeyEvent (DWORD dwKeyID, DWORD dwKeystate)
{
	UCHAR		VKey;
	UINT			uiScanCode;
	DWORD		dwFlags = 0;

	if (dwKeyID >= m_dwNbKeypadKeys)
	{
		ERRORMSG(1,(TEXT("Key id is out of bouds (%d)"), dwKeyID));
		return;
	}

	VKey = m_KeypadKeyToVKey[dwKeyID];
	uiScanCode = MapVirtualKey(VKey, 0);
	
	RETAILMSG(1,(TEXT("Key %d has been %s (VKEY 0x%x / SCANCODE 0x%x)\r\n"),dwKeyID,dwKeystate ?  L"Pressed" : L"Released",VKey,uiScanCode));
	
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

// start keypad scanning and send keyboard event if available
BOOL KeyPad::KeyPadScanProc(void)
{
	BOOL		bRet = TRUE;
	DWORD		dwReturn;

	while(!m_bTerminateThread)
	{
		dwReturn = WaitForSingleObject(m_hKeypadScanEvent, INFINITE);

		if ( m_bTerminateThread || !m_hKeypadScanEvent ) 
		{
			RETAILMSG(1,(TEXT("Exit KeyPadScanProc thread\r\n")));
			SetEvent( m_hKillThreadAckEvent );
			ExitThread(0);
		}

		bRet = CustomKeypadScan();
	}

	return bRet;
}

BOOL KeyPad::IsrThreadProc(void)
{
	DWORD dwRet =  0;

	while( !m_bExitIST )
	{
		dwRet = WaitForSingleObject(m_hInterruptEvent, INFINITE);

		if ( m_bExitIST || !m_hInterruptEvent ) 
		{
			RETAILMSG(1,(TEXT("Exit IsrThreadProc thread\r\n")));
			SetEvent( m_hKillThreadAckEvent );
			ExitThread(0);
		}

		// inform scan thread to start work
		SetEvent(m_hKeypadScanEvent);
		InterruptDone(m_dwSysintr);
	}

	return TRUE;
}


//---------------------------------------------------------
// This is the interrupt thread
//---------------------------------------------------------
DWORD KeyPad::KeyPadIsrThread(KeyPad *pKP)
{
	// should NOT return until terminated by DeInit
    pKP->IsrThreadProc();

    return(0);
}

//---------------------------------------------------------
// This is the kaypad scan thread
//---------------------------------------------------------
DWORD KeyPad::KeyPadScanThread(KeyPad *pKP)
{
	// should NOT return until terminated by DeInit
    pKP->KeyPadScanProc();

    return(0);
}


BOOL KeyPad::ThreadStart(void)
{
	m_bTerminateThread = FALSE;
	m_hKeypadScanThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyPadScanThread, this, 0, NULL);

	m_bExitIST = FALSE;
	m_hISTthreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyPadIsrThread, this, 0, NULL);

	return(TRUE);
}

// Free interrupt and Delete event
BOOL KeyPad::Deinitialize() 
{
    HANDLE	pThisThread = GetCurrentThread( );
    DWORD		priority256;

    /* If we have an interrupt handler thread, kill it */
    if ( m_hISTthreadHandle ) 
	{
        /* Set the priority of the dispatch thread to be equal to this one,
         * so that it shuts down before we free its memory. If this routine
         * has been called from SerialDllEntry then RxCharBuffer is set to
         * NULL and the dispatch thread is already dead, so just skip the
         * code which kills the thread.
         */
        priority256 = CeGetThreadPriority( pThisThread);
        CeSetThreadPriority(m_hISTthreadHandle, priority256);        

        /* Signal the Dispatch thread to die. */
		m_bExitIST = TRUE;
		SetEvent(m_hInterruptEvent);

        WaitForSingleObject(m_hKillThreadAckEvent, 3000);
        Sleep(10);

        CloseHandle(m_hISTthreadHandle);
        m_hISTthreadHandle = NULL;        
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

	// kill the scan thread if it available
	if(m_hKeypadScanThreadHandle)
	{
        /* Set the priority of the dispatch thread to be equal to this one,
         * so that it shuts down before we free its memory. If this routine
         * has been called from SerialDllEntry then RxCharBuffer is set to
         * NULL and the dispatch thread is already dead, so just skip the
         * code which kills the thread.
         */
        priority256 = CeGetThreadPriority( pThisThread);
        CeSetThreadPriority(m_hKeypadScanThreadHandle, priority256);        

        /* Signal the Dispatch thread to die. */
		m_bTerminateThread = TRUE;
		SetEvent(m_hKeypadScanEvent);

        WaitForSingleObject(m_hKillThreadAckEvent, 3000);
        Sleep(10);

        CloseHandle(m_hKeypadScanThreadHandle);
        m_hKeypadScanThreadHandle = NULL;        
	}

	// Delete event
    if (m_hKeypadScanEvent)
	{
		if (!CloseHandle(m_hKeypadScanEvent))
		{
			DEBUGMSG(TRUE, (TEXT("KPD> Failed to scan keypad event\r\n")));
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

	// create events
	m_hKeypadScanEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hInterruptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hKillThreadAckEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// bind interrupt number and interrupt event
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
    if (!pKeyPad->ThreadStart())
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
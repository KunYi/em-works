//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{
//!
// All rights reserved ADENEO 2007
//!
//! \file 	custom_keypad.cpp
//!
//! \brief		Stream driver for the Keypad embedded on the AT91SAM9261EK EvalBoard
//! 
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9261EK_v170_rc4/PLATFORM/AT91SAM9261EK/SRC/DRIVERS/KeyPad/custom_keypad.cpp $
//!   $Author: pblanchard $
//!   $Revision: 1219 $
//!   $Date: 2007-08-03 16:59:11 +0200 (ven., 03 août 2007) $
//! \endif
//
// The keypad driver is a DLL loaded by DeviceManager, named KPD.
// It is a stream driver (KPD1), with some dummy functions (Open, Close, Read, Write, Seek, IOControl).
// It is not loaded by GWES, using the Layout Manager.
// It is written in C++. (class KeyPad)
// It works from an IST, sending keyboard events with PostKeybdMessage().
// It can be unloaded using "s unmountdrv KPD1:" and reloaded using "s activateDevice BuiltIn\KEYPAD"
//
//-----------------------------------------------------------------------------

//! \addtogroup	Keypad
//! @{
//
// System include
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 4245)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <devload.h>
#pragma warning(pop)

// Board include
#include "bsp.h"
#include "bsp_drivers.h"

// Local include
#include "custom_keypad.hpp"

//------------------------------------------------------------------------------
// Defines
//
#define KEYPAD_POLLING_TIMEOUT_NAME		(TEXT("PollingTimeout"))
#define KEYPAD_NUMBER_OF_KOUT					(TEXT("NbOfKOUT"))
#define KEYPAD_KEY_NAME_FORMAT				(TEXT("K4X%d%d%d"))
#define KEYPAD_KIN_PORT_ADDRESS				(TEXT("KINPortOffset"))
#define KEYPAD_KOUT_PORT_ADDRESS			(TEXT("KOUTPortOffset"))


#define POLLING_TIME_OUT_DEFAULT		15 // in milliseconds
#define KEYPRESSURE_TIME				 2
//------------------------------------------------------------------------------
// Externs
//
extern "C" PVOID VirtualAllocCopyPhysical(unsigned size,char *str,PVOID pPhysicalAddress);

//------------------------------------------------------------------------------
// Global Variables
//
static DWORD g_dwSysIntr_WakeUp ;//wake up sysintr

//------------------------------------------------------------------------------
// Local Variables
//

//------------------------------------------------------------------------------
// Local Functions
//
KeyPad* CustomKeyPadInit ()
{
	return new CustomKeyPad();
}

CustomKeyPad::CustomKeyPad()
{
	m_dwKeysStates = NULL;
	m_pISA = NULL;

	//
	// CS&ZHL SEP-08-2009: default setting => 4x5 keypad
	//
	m_dwNbOfKOUT = 5;

	// Clear
	m_dwKINPortOffset = 0;
	m_dwKOUTPortOffset = 0;
}

BOOL CustomKeyPad::RegQueryDword (HKEY hKey, LPWSTR lpKeyName, DWORD *pdwValue)
{
	BOOL bRet = FALSE;
	LONG lResult = 0;
	DWORD dwSize = sizeof(DWORD);

	lResult = RegQueryValueEx(hKey, lpKeyName, NULL, NULL, (LPBYTE)pdwValue, &dwSize);

	if (lResult == ERROR_SUCCESS)
	{
		bRet = TRUE;
	}

	return bRet;
}


// Get Keypad keys assignement from registry
BOOL CustomKeyPad::GetKeysAssignement(HKEY hKey)
{
	WCHAR	pStr[255];
	DWORD	dwKeyIndex = 0;
	DWORD	dwVKeyValue = 0;
	BOOL	bNoError = FALSE;
	DWORD	dwKINIndex;
	DWORD	dwKOUTIndex;

	//
	// CS&ZHL SEP-08-2009: get number of KOUT
	//
	bNoError = RegQueryDword (hKey, KEYPAD_NUMBER_OF_KOUT, &m_dwNbOfKOUT);
	if(!bNoError)
	{
		RETAILMSG(1, (TEXT("CustomKeyPad::GetISAPortAssignement: Failed to get NbOfKOUT\r\n")));
		return bNoError;
	}

	// Clear buffer
	memset(pStr, 0x00, sizeof(pStr));

	// Detect the numbers of keys, and get the associated VKEY for each key
	//for(dwKOUTIndex = 0; dwKOUTIndex < NUM_MATRIX_KEY_OUT; dwKOUTIndex++)
	for(dwKOUTIndex = 0; dwKOUTIndex < m_dwNbOfKOUT; dwKOUTIndex++)
	{
		for(dwKINIndex = 0; dwKINIndex < NUM_MATRIX_KEY_IN; dwKINIndex++)
		{
			// Generate key
			//swprintf((wchar_t *)pStr, KEYPAD_KEY_NAME_FORMAT, dwKOUTIndex, dwKINIndex);
			swprintf((wchar_t *)pStr, KEYPAD_KEY_NAME_FORMAT, m_dwNbOfKOUT, dwKOUTIndex, dwKINIndex);

			// Get the Key VKEY
			bNoError = RegQueryDword (hKey, (wchar_t *)pStr, &dwVKeyValue);
			if(!bNoError)
			{
				//RETAILMSG(1,(TEXT("CustomKeyPad::GetKeysAssignement: No such a KEY%d%d!\r\n"), 
				//	dwKOUTIndex, dwKINIndex));
				RETAILMSG(1,(TEXT("CustomKeyPad::GetKeysAssignement: No such a K4X%d%d%d!\r\n"), 
					m_dwNbOfKOUT, dwKOUTIndex, dwKINIndex));
				break;
			}
			//RETAILMSG(1,(TEXT("CustomKeyPad::GetKeysAssignement: KEY%d%d => VKey = 0x%X\r\n"), 
			//	dwKOUTIndex, dwKINIndex, dwVKeyValue));
			RETAILMSG(1,(TEXT("CustomKeyPad::GetKeysAssignement: K4X%d%d%d => VKey = 0x%X\r\n"), 
				m_dwNbOfKOUT, dwKOUTIndex, dwKINIndex, dwVKeyValue));
			
			// save the VK code 
			m_KeypadKeyToVKey[dwKeyIndex] = (UCHAR)dwVKeyValue;
			m_dwNbKeypadKeys = dwKeyIndex + 1;
			dwKeyIndex++;
		}

		if(!bNoError)
		{
			break;
		}
	}

	return bNoError;
}

// Get ISA Port assignement from registry
BOOL CustomKeyPad::GetISAPortAssignement(HKEY hKey)
{
	BOOL		bNoError = TRUE;

	// Get the KIN Port Address
	bNoError = RegQueryDword (hKey, KEYPAD_KIN_PORT_ADDRESS, &m_dwKINPortOffset);
	if(!bNoError)
	{
		RETAILMSG(1, (TEXT("CustomKeyPad::GetISAPortAssignement: Failed to assign KINPortOffset\r\n")));
		return bNoError;
	}
	RETAILMSG(1,(TEXT("CustomKeyPad::GetGPIOAssignement: KINPortOffset = 0x%x\r\n"), m_dwKINPortOffset));
	
	// Get the KOUT Port Address
	bNoError = RegQueryDword (hKey, KEYPAD_KOUT_PORT_ADDRESS, &m_dwKOUTPortOffset);
	if(!bNoError)
	{
		RETAILMSG(1, (TEXT("CustomKeyPad::GetISAPortAssignement: Failed to assign KOUTPortOffset\r\n")));
		return bNoError;
	}
	RETAILMSG(1,(TEXT("CustomKeyPad::GetGPIOAssignement: KOUTPortOffset = 0x%x\r\n"), m_dwKOUTPortOffset));

	return bNoError;
}

// Set KIN/KOUT port
 BOOL CustomKeyPad::ISAPortInit(void)
{
    PHYSICAL_ADDRESS phyAddr;

	RETAILMSG(1,(TEXT("-> ISAPortInit\r\n")));
	//
    // CS&ZHL AUG-9-2011: set KPP pins as inputs
	//gpio group number = DDK_GPIO_PORT1..DDK_GPIO_PORT4
	//gpio number within a gpio group = 0..31
	//gpio direction = DDK_GPIO_DIR_IN, DDK_GPIO_DIR_OUT
	//gpio interrupt = level, edge,none, etc 
	//
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL2, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);		// -> GPIO3_3
    DDKGpioSetConfig(DDK_GPIO_PORT3, 3,   DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);				

	DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL3, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);		// -> GPIO3_4
    DDKGpioSetConfig(DDK_GPIO_PORT3, 4,   DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);				

	DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D8,      DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);		// -> GPIO1_7
    DDKGpioSetConfig(DDK_GPIO_PORT1, 7,   DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);				

	DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D9,      DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);		// -> GPIO4_21
    DDKGpioSetConfig(DDK_GPIO_PORT4, 21, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);				

	DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW0, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);	// -> GPIO2_29
    DDKGpioSetConfig(DDK_GPIO_PORT2, 29, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);				

	DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW1, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);	// -> GPIO2_30
    DDKGpioSetConfig(DDK_GPIO_PORT2, 30, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);				

	DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW2, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);	// -> GPIO2_31
    DDKGpioSetConfig(DDK_GPIO_PORT2, 31, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);				

	DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW3, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);	// -> GPIO3_0
    DDKGpioSetConfig(DDK_GPIO_PORT3, 0,   DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);				

    // CONSTANT SETTINGS:   
    // low/high output voltage to NA
    // test_ts to Disabled
    // dse test to regular
    // strength mode to 4_level
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // CONFIGURED SETTINGS:
    // Hyst. Enable to Enabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to CFG(100Kohm PU)
    // Drive Strength to CFG(High)
    // Slew Rate to CFG(FAST)
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW0,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_UP_100K,				//DDK_IOMUX_PAD_PULL_NONE,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW1,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_UP_100K,				//DDK_IOMUX_PAD_PULL_NONE,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW2,
                         DDK_IOMUX_PAD_SLEW_SLOW, 
                         DDK_IOMUX_PAD_DRIVE_NORMAL, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K,				//DDK_IOMUX_PAD_PULL_NONE, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW3, 
                         DDK_IOMUX_PAD_SLEW_SLOW, 
                         DDK_IOMUX_PAD_DRIVE_NORMAL, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K,				//DDK_IOMUX_PAD_PULL_NONE, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_COL2, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K,				//DDK_IOMUX_PAD_PULL_NONE, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_COL3, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K,				//DDK_IOMUX_PAD_PULL_NONE, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D8, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K,				//DDK_IOMUX_PAD_PULL_NONE, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D9, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K,				//DDK_IOMUX_PAD_PULL_NONE, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

	//map ISA memory space
	phyAddr.QuadPart = CSP_BASE_MEM_PA_CS5;
    m_pISA = (PEM9K_CPLD_REGS) MmMapIoSpace(phyAddr, sizeof(EM9K_CPLD_REGS), FALSE);
	if (m_pISA == NULL)
	{
		RETAILMSG(1, (TEXT("%s: ISA memory space mapping failed!\r\n"), __WFUNCTION__));
		return FALSE;
	}

	// enable ISA bus
	OUTREG8(&m_pISA->ISACtrlReg, EM9K_CPLD_ISACTRL_ISAEN);
	// enable CPLD output
	OUTREG8(&m_pISA->ResetReg, 0x55);

	// set all KOUT = "1"
	KOUT(0xFF);
	RETAILMSG(1,(TEXT("<- ISAPortInit\r\n")));

	return TRUE;
}


//
// /note This function perform the following steps
//       1) Get registry entry if needed
//		 2) Assign the m_dwSysintr value (optional) for key pressure detection
//			 - if not used m_dwSysintr must be SYSINTR_UNDIFINED
//		 3) Assign the m_dwPollingTimeOut (optional) to
//			 - if not used m_dwPollingTimeOut must be INFINITE
//		 4) Allocate and fill the m_KeypadKeyToVKey variable with the VKEY assignement
//		 5) Do not mised to set m_dwNbKeypadKeys to the value of m_KeypadKeyToVKey elements
//
BOOL CustomKeyPad::CustomInitialize(LPCTSTR pContext)
{
	BOOL   bRet = TRUE;
	HKEY   hKey;
	DWORD  dwValue;
	BOOL   bStatus;

	// Load KeypadKeys assignement
	hKey = OpenDeviceKey (pContext);
	if (hKey == NULL )
	{
		ERRORMSG(1,(TEXT("CustomKeyPad::Initialize Unable to open device registry entry\r\n")));
		bRet = FALSE;
	}

	// Get Key assignement from registry
	if (bRet)
	{
		bRet = GetKeysAssignement(hKey);
	}

	if( bRet )
	{
		// Clear the keys states table
		m_dwKeysStates = new DWORD[m_dwNbKeypadKeys];
		memset(m_dwKeysStates, 0x00, m_dwNbKeypadKeys * sizeof(DWORD));

		bRet = GetISAPortAssignement(hKey);
	}

	if( bRet )
	{
		//
		// Set KOUT -> 5'b11111
		//
		ISAPortInit();
	}

	//
	// CS&ZHL JLY-17-2008: get timeout value from registry  
	//
	if (bRet)
	{
		// No Sysintr need
		m_dwSysintr = (DWORD)SYSINTR_UNDEFINED;
		
		//
		bStatus = RegQueryDword (hKey, KEYPAD_POLLING_TIMEOUT_NAME, &dwValue);
		if (bStatus)
		{
			m_dwPollingTimeOut = dwValue;
		}
		else
		{
			m_dwPollingTimeOut = POLLING_TIME_OUT_DEFAULT;
		}

	}

	// Close registry key
	if ((hKey != NULL) && (RegCloseKey(hKey) != ERROR_SUCCESS))
	{
		bRet = FALSE;
	}

	return bRet;
}


BOOL CustomKeyPad::CustomDeinitialize() 
{
	if (m_dwKeysStates != NULL)
	{
		delete m_dwKeysStates;
	}

	return TRUE;
}


BOOL CustomKeyPad::KeyPadPowerOff(void)
{
	RETAILMSG(1,(TEXT("KeyPadPowerOff customclass\r\n")));
	InterruptDone(g_dwSysIntr_WakeUp);
	return TRUE;
}

BOOL CustomKeyPad::KeyPadPowerOn(void)
{
	RETAILMSG(1,(TEXT("KeyPadPowerOn customclass\r\n")));
	InterruptDone(g_dwSysIntr_WakeUp);
	return TRUE;
}

void		CustomKeyPad::KOUT(BYTE ucValue)
{
	BYTE ub1;

	if(m_pISA == NULL)
	{
		RETAILMSG(1,(TEXT("m_pISA == NULL, exit KOUT(..)\r\n")));
		return;
	}


	//RETAILMSG(1,(TEXT("check ISA config...\r\n")));
	ub1 = INREG8(&(m_pISA->StateReg));
	//ub1 = m_pISA->StateReg;
	//RETAILMSG(1,(TEXT("write KOUT...\r\n")));
	if( ub1 & EM9K_CPLD_STATE_CS0_EN)
	{
		OUTREG8(&(m_pISA->ISA_CS1[m_dwKOUTPortOffset]), ucValue);
	}
	else
	{
		OUTREG8(&(m_pISA->ISA_CS0[m_dwKOUTPortOffset]), ucValue);
	}
}

BYTE		CustomKeyPad::KIN()
{
	BYTE		ucKIN;
	
	if(m_pISA == NULL)
	{
		RETAILMSG(1,(TEXT("m_pISA == NULL, exit KIN( )\r\n")));
		return 0xFF;
	}

	if(INREG8(&(m_pISA->StateReg)) & EM9K_CPLD_STATE_CS0_EN)
	{
		ucKIN = INREG8(&(m_pISA->ISA_CS1[m_dwKINPortOffset]));
	}
	else
	{
		ucKIN = INREG8(&(m_pISA->ISA_CS0[m_dwKINPortOffset]));
	}

	return ucKIN;
}

// Get the keypad keys state (1 bit for each key)
// To avoid mecanical rebounds, the Key must be seen X time pressed before indicating as pressed to the driver
DWORD CustomKeyPad::GetKeypadState(void)
{
	DWORD	dwIndex;
	DWORD	dwKeyPadKeysState;
	DWORD	dwGPIOState1, dwGPIOState2;
	DWORD	dwKINIndex;
	DWORD	dwKOUTIndex;

	//
	// CS&ZHL JLY-17-2008 comment: This is key scanning routine! dwPadKeysState.x = 1 => the key pressed
	//
  	dwKeyPadKeysState = 0;

	// start scanning
	for(dwKOUTIndex = 0; dwKOUTIndex < m_dwNbOfKOUT; dwKOUTIndex++)
	{
		// set current KOUTindex as "0" -> active
		//*((volatile UCHAR*)m_dwKOUTPortAddress) = ~(1 << dwKOUTIndex);
		KOUT((BYTE)((~(1 << dwKOUTIndex)) & 0xFF));

		// read each KIN
		for(dwKINIndex = 0; dwKINIndex < NUM_MATRIX_KEY_IN; dwKINIndex++)
		{
			// convert out dwIndex
			dwIndex = dwKOUTIndex * NUM_MATRIX_KEY_IN + dwKINIndex;

			// Get the Pio Status
			// "1" -> the key is released, "0" -> the key is pressed
			//dwGPIOState1 = (DWORD)(*((volatile UCHAR*)m_dwKINPortAddress));
			//dwGPIOState2 = (DWORD)(*((volatile UCHAR*)m_dwKINPortAddress));
			dwGPIOState1 = (DWORD)KIN();
			dwGPIOState2 = (DWORD)KIN();
			if(dwGPIOState1 != dwGPIOState2)
			{
				continue;
			}

			if( !(dwGPIOState1 & (1 << dwKINIndex)) )
			{
				// The key is pressed, so increase the keystate conter
				m_dwKeysStates[dwIndex] += 1;

				if (m_dwKeysStates[dwIndex] >= KEYPRESSURE_TIME)
				{
					dwKeyPadKeysState |= (1 << dwIndex);
				}
			}
			else
			{
				m_dwKeysStates[dwIndex] = 0;
			}
		}

		// set current KOUTindex as "1" -> inactive
		//*((volatile UCHAR*)m_dwKOUTPortAddress) = 0xff;
		KOUT(0xFF);
	}

	return dwKeyPadKeysState;
}

//! @}
//! @}

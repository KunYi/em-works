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
#define KEYPAD_POLLING_TIMEOUT_NAME	(TEXT("PollingTimeout"))
#define KEYPAD_KEY_NAME_FORMAT			(TEXT("K4X%d%d%d"))

#define POLLING_TIME_OUT_DEFAULT			15	// in milliseconds
#define KEYPRESSURE_TIME							2

#define KEY_DEBOUNCE_PERIOD      40					// msec
#define KEY_WAIT_UP_PERIOD			100				// msec

#define KPP_COLUMN_INUSE			4																	// -> COL2, COL3, COL6, COL7
#define KPP_ROW_INUSE					4																	// -> ROW0, ROW1, ROW2, ROW3
#define KEY_NUMBER						(KPP_COLUMN_INUSE*KPP_ROW_INUSE)
#define KPP_COLUMN_MASK				0xCC
#define KPP_ROW_MASK					0x0F

#define KEYPAD_STATE_UP				0
#define KEYPAD_STATE_DOWN			1
#define KEYPAD_STATE_WAIT			2

//------------------------------------------------------------------------------
// External Functions
extern UINT32 CSPKppGetBaseRegAddr(VOID);
extern UINT32 CSPKppGetIrq(VOID);
extern UINT32 CSPKppGetBaseRegAddr(VOID);

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

//------------------------------------------------------------------------------
//
// Function: BSPKppSetClockGatingMode
//
// Turn on/off clocks to the keypad port module.
//
// Parameters:
//      startClocks
//          [in] If TRUE, turn clocks to KPP on.
//                If FALSE, turn clocks to KPP off
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL BSPKppSetClockGatingMode(BOOL startClocks)
{

    if (startClocks)
    {
        // Turn KPP clocks on
        if (!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_KPP, 
            DDK_CLOCK_GATE_MODE_ENABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed to set CRM clock gating mode!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }
    else
    {
        // Turn KPP clocks off
        if (!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_KPP, 
            DDK_CLOCK_GATE_MODE_DISABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed to set CRM clock gating mode!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    return TRUE;
}


//--------------------------------------------------------
// class member functions
//--------------------------------------------------------

CustomKeyPad::CustomKeyPad()
{
	m_dwKeysStates = NULL;
	m_pKPP = NULL;

	m_uColMaskTab[0] = 0x04;			// -> COL2
	m_uColMaskTab[1] = 0x08;			// -> COL3
	m_uColMaskTab[2] = 0x40;			// -> COL6
	m_uColMaskTab[3] = 0x80;			// -> COL7
	m_uRowMaskTab[0] = 0x01;		// -> ROW0
	m_uRowMaskTab[1] = 0x02;		// -> ROW1
	m_uRowMaskTab[2] = 0x04;		// -> ROW2
	m_uRowMaskTab[3] = 0x08;		// -> ROW3

	m_dwScanState = KEYPAD_STATE_UP;	
	
	//
	// CS&ZHL SEP-08-2009: default setting => 4x4 keypad
	//
	m_dwNbOfKOUT = 4;
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
	WCHAR		pStr[255];
	DWORD		dwKeyIndex = 0;
	DWORD		dwVKeyValue = 0;
	BOOL		bNoError = FALSE;
	DWORD		dwKINIndex;
	DWORD		dwKOUTIndex;

	//
	// CS&ZHL SEP-08-2009: get number of KOUT
	//
	//bNoError = RegQueryDword (hKey, KEYPAD_NUMBER_OF_KOUT, &m_dwNbOfKOUT);
	//if(!bNoError)
	//{
	//	RETAILMSG(1, (TEXT("CustomKeyPad::GetISAPortAssignement: Failed to get NbOfKOUT\r\n")));
	//	return bNoError;
	//}

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
				RETAILMSG(1, (TEXT("CustomKeyPad::GetKeysAssignement: No such a K4X%d%d%d!\r\n"), 
					m_dwNbOfKOUT, dwKOUTIndex, dwKINIndex));
				break;
			}
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


//------------------------------------------------------------------------------
//
// Function: BSPKppRegInit
//
// Initializes the keypad port registers.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE or FALSE.
//
//------------------------------------------------------------------------------
BOOL CustomKeyPad::BSPKppRegInit()
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
	OUTREG8(&pISA->ISACtrlReg, 0);									//ISA bus output disable 
	MmUnmapIoSpace(pISA, sizeof(EM9K_CPLD_REGS));	//unmap ISA memory space

    //map KPP memory space
    if(m_pKPP == NULL)
    {
		phyAddr.QuadPart = CSPKppGetBaseRegAddr();
        m_pKPP = (PCSP_KPP_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_KPP_REGS), FALSE);
		if (m_pKPP == NULL)
		{
			RETAILMSG(1, (TEXT("%s: KPP memory space mapping failed!\r\n"), __WFUNCTION__));
			return FALSE;
		}
    }

	DEBUGMSG(ZONE_INIT, (TEXT("[KPP]%s:  m_pKPP=0x%x\r\n"), __WFUNCTION__, m_pKPP));

	//
    // CS&ZHL AUG-4-2011: EM9170 use KPP_ROW0, KPP_ROW1, KPP_ROW2, and KPP_ROW3 as KIN0 - KIN3
	//                                    EM9170 use KPP_COL2, KPP_COL3, KPP_COL6, and KPP_COL7 as KOUT0 - KOUT3
	//
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL2, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL3, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D8, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);			// -> KPP_COL6
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D9, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);			// -> KPP_COL7
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW0, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW1, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW2, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW3, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    
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
                         DDK_IOMUX_PAD_PULL_UP_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW1,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_UP_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW2,
                         DDK_IOMUX_PAD_SLEW_SLOW, 
                         DDK_IOMUX_PAD_DRIVE_NORMAL, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW3, 
                         DDK_IOMUX_PAD_SLEW_SLOW, 
                         DDK_IOMUX_PAD_DRIVE_NORMAL, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_COL2, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_COL3, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D8, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D9, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);


    // CONSTANT SETTINGS:   
    // low/high output voltage to NA
    // test_ts to Disabled
    // dse test to regular
    // strength mode to 4_level
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to Pull
    // CONFIGURED SETTINGS:
    // Hyst. Enable to Enabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull Up / Down Config. to CFG(100Kohm PU)
    // Open Drain Enable to CFG(Disabled)
    // Drive Strength to CFG(High)
    // Slew Rate to CFG(FAST)
    //reg32_write(IOMUXC_SW_PAD_CTL_PAD_KEY_COL3, 0x01a5);



    // Enable KPP clocks to access KPP registers
    BSPKppSetClockGatingMode(TRUE);

    // Enable no. of rows in keypad (KRE = 1)
    // Configure columns as open-drain (KCO = 1)
    INSREG16(&m_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KRE),
        CSP_BITFVAL(KPP_KPCR_KRE, KPP_ROW_MASK));
    INSREG16(&m_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KCO),
        CSP_BITFVAL(KPP_KPCR_KCO, KPP_COLUMN_MASK));

    // Write 0's to all columns
    INSREG16(&m_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KCD),
        CSP_BITFVAL(KPP_KPDR_KCD, 0));

    // Configure rows as input, columns in use as output
    INSREG16(&m_pKPP->KDDR, CSP_BITFMASK(KPP_KDDR_KCDD),
        CSP_BITFVAL(KPP_KDDR_KCDD, KPP_COLUMN_MASK));
    INSREG16(&m_pKPP->KDDR, CSP_BITFMASK(KPP_KDDR_KRDD),
        CSP_BITFVAL(KPP_KDDR_KRDD, 0));

    // Clear KPKD and KPSR_KPKR status flag (w1c)
    // Clear synchronizer chain - KDSC (w1c)
    // Enable keypad interrupt - Set KDIE, -> Keypad key depress interrupt enable
    // clear KRIE (avoid false release events) -> Keypad release interrupt disable
    OUTREG16(&m_pKPP->KPSR,
        (CSP_BITFVAL(KPP_KPSR_KPP_EN, KPP_KPSR_KPP_EN_ENABLE) |
        CSP_BITFVAL(KPP_KPSR_KPKD, KPP_KPSR_KPKD_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KPKR, KPP_KPSR_KPKR_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KDSC, KPP_KPSR_KDSC_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KDIE, KPP_KPSR_KDIE_INT_ENABLE) |
        CSP_BITFVAL(KPP_KPSR_KRIE, KPP_KPSR_KRIE_INT_DISABLE)));

    DEBUGMSG(ZONE_PDD, (TEXT("End of Init method - ctrl: %x  status: %x  direction: %x data: %x\r\n"),
                    m_pKPP->KPCR, m_pKPP->KPSR, m_pKPP->KDDR, m_pKPP->KPDR));

    // Disable KPP clocks for Power Management
    //BSPKppSetClockGatingMode(FALSE);

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
	BOOL		bRet = TRUE;
	HKEY			hKey;
	DWORD		dwValue = 0;
	BOOL		bStatus;

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
	}

	if( bRet )
	{
		//
		// initialize iMX257 KeyPad Port -> hardware init
		//
		bRet = BSPKppRegInit();
	}

	//
	// CS&ZHL JLY-17-2008: request sysIntr and get timeout value from registry  
	//
	if (bRet)
	{
		DWORD		dwIrq_Keybd = CSPKppGetIrq();
	
		// request systen interrupt number
		m_dwSysintr = (DWORD)SYSINTR_UNDEFINED;
		if(!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, 
							&dwIrq_Keybd, sizeof(DWORD), 
							&m_dwSysintr, sizeof(DWORD), 
							0))
		{
			RETAILMSG(1, (TEXT("ERROR: Failed to obtain sysintr value for keyboard interrupt.\r\n")));
			m_dwSysintr = (DWORD)SYSINTR_UNDEFINED;
		}
		
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
    // Disable interrupts while processing.
    INSREG16(&m_pKPP->KPSR, CSP_BITFMASK(KPP_KPSR_KDIE),
        CSP_BITFVAL(KPP_KPSR_KDIE, KPP_KPSR_KDIE_INT_DISABLE));
    INSREG16(&m_pKPP->KPSR, CSP_BITFMASK(KPP_KPSR_KRIE),
        CSP_BITFVAL(KPP_KPSR_KRIE, KPP_KPSR_KRIE_INT_DISABLE));

	// Disable KPP clocks for Power Management
    BSPKppSetClockGatingMode(FALSE);

	if(m_dwSysintr != (DWORD)SYSINTR_UNDEFINED)
	{
		//release SysIntr of KPP;
		if(!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, 
							&m_dwSysintr, sizeof(DWORD), 
							NULL, 0, 
							NULL))
		{
			RETAILMSG(1, (TEXT("CustomDeinitialize::Failed to release sysintr value for keyboard interrupt.\r\n")));
		}
	}

	if (m_dwKeysStates != NULL)
	{
		delete m_dwKeysStates;
	}

	//map KPP memory space
    if(m_pKPP != NULL)
    {
		MmUnmapIoSpace(m_pKPP, sizeof(CSP_KPP_REGS));	//unmap KPP memory space
	}

	return TRUE;
}


BOOL CustomKeyPad::KeyPadPowerOff(void)
{
	RETAILMSG(1,(TEXT("KeyPadPowerOff customclass\r\n")));
	//InterruptDone(g_dwSysIntr_WakeUp);
	return TRUE;
}

BOOL CustomKeyPad::KeyPadPowerOn(void)
{
	RETAILMSG(1,(TEXT("KeyPadPowerOn customclass\r\n")));
	//InterruptDone(g_dwSysIntr_WakeUp);
	return TRUE;
}

//----------------------------------------------------------------------------------------------------------------------
// real code to implementation of physical keypad scan
//----------------------------------------------------------------------------------------------------------------------
BOOL CustomKeyPad::CustomKeypadScan(void)
{
    UINT16		tempKPSR;								// KPSR value read at start of scan sequence    
    UINT8		iRowMask;
    UINT8		iColMask;
    UINT8		iCol;
    UINT8		iRow;
    UINT8		rowData = 0;
	UINT8		rowCurrentData = 0;
	DWORD		dwKeyIndex = (DWORD)-1;
	BOOL		bKeyPressed = FALSE;

    // Enable KPP clocks to access registers
    // during key scan sequence
    BSPKppSetClockGatingMode(TRUE);

	// Read keypad status register
    tempKPSR = INREG16(&m_pKPP->KPSR);

    DEBUGMSG(ZONE_PDD, (TEXT("Before scan, tempKPSR 0x%04x, KPSR 0x%04x. \r\n"), 
        tempKPSR, INREG16(&m_pKPP->KPSR)));

    // Disable interrupts while processing.
    INSREG16(&m_pKPP->KPSR, CSP_BITFMASK(KPP_KPSR_KDIE),
        CSP_BITFVAL(KPP_KPSR_KDIE, KPP_KPSR_KDIE_INT_DISABLE));
    INSREG16(&m_pKPP->KPSR, CSP_BITFMASK(KPP_KPSR_KRIE),
        CSP_BITFVAL(KPP_KPSR_KRIE, KPP_KPSR_KRIE_INT_DISABLE));

	//
	// step1: check if a key pressed
	//
    // Write '1' to all columns
    INSREG16(&m_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KCD), CSP_BITFVAL(KPP_KPDR_KCD, KPP_COLUMN_MASK));

    // Configure column as totem-pole outputs (for quick discharging of keypad capacitance).
    INSREG16(&m_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KCO), CSP_BITFVAL(KPP_KPCR_KCO, ~KPP_COLUMN_MASK));
    // Configure columns as open drain
    INSREG16(&m_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KCO), CSP_BITFVAL(KPP_KPCR_KCO, KPP_COLUMN_MASK));

    // Scan key map for changes
    for(iCol = 0; iCol < KPP_COLUMN_INUSE; iCol++)
    {
		iColMask = m_uColMaskTab[iCol];
        // Write '0' for this column.
        INSREG16(&m_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KCD), CSP_BITFVAL(KPP_KPDR_KCD, ~iColMask));
        // Wait required to allow row outputs to propagate
        Sleep(0);

        // Get current key status & handle accordingly
        rowData = (UINT8) (KPP_ROW_MASK & EXTREG16(&m_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KRD), KPP_KPDR_KRD_LSH));
		// read twice
        rowData = (UINT8) (KPP_ROW_MASK & EXTREG16(&m_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KRD), KPP_KPDR_KRD_LSH));
		if( rowData != KPP_ROW_MASK)
		{
			// some key pressed
			break;
		}
	}
	if(iCol >= KPP_COLUMN_INUSE)
	{
		// no key pressed, end the scan processing
		goto scan_end1;
	}

	//
	// step2: apply a debounce time and then try to get the key pressed
	//
	Sleep(KEY_DEBOUNCE_PERIOD);
	// read again
	rowCurrentData = (UINT8) (KPP_ROW_MASK & EXTREG16(&m_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KRD), KPP_KPDR_KRD_LSH));
	if(rowData == rowCurrentData)
	{
		//try to find which key pressed
		for(iRow = 0; iRow < KPP_ROW_INUSE; iRow++)
		{
			iRowMask = m_uRowMaskTab[iRow];
			dwKeyIndex = iCol * KPP_ROW_INUSE + iRow;
			if( rowData == (~iRowMask & KPP_ROW_MASK))
			{
				// find the key, and inform upper layer
				SendKeyEvent (dwKeyIndex, 1);			// the key is down
				bKeyPressed = TRUE;
				break;
			}
		}
	}

	//
	// step3: wait key release, and send key release event if available
	//
	DWORD dwRepeatTimeout = 1000 / m_dwAutoRepeatKeysPerSec;
	DWORD dwStartCount = GetTickCount();
	DWORD dwLastKeyEventCount = GetTickCount();
	for( ; ; )
	{
		Sleep(KEY_WAIT_UP_PERIOD);
        // Get current key status & handle accordingly
        rowData = (UINT8) (KPP_ROW_MASK & EXTREG16(&m_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KRD), KPP_KPDR_KRD_LSH));
		if(rowData == KPP_ROW_MASK)		//key is released
		{
			// Write 0's to all columns
			INSREG16(&m_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KCD), CSP_BITFVAL(KPP_KPDR_KCD, 0));

			// send release event if available
			if(bKeyPressed)
			{
				SendKeyEvent (dwKeyIndex, 0);			// the key is up
				bKeyPressed = FALSE;
			}
			break;
		}
		else
		{
			// can send repeat event if required
			if( (GetTickCount() - dwStartCount) < m_dwAutoRepeatInitialDelay)
			{
				continue;
			}
			else
			{
				if(bKeyPressed && ((GetTickCount() - dwLastKeyEventCount) > dwRepeatTimeout))
				{
					SendKeyEvent (dwKeyIndex, 1);			// the key is down
					dwLastKeyEventCount = GetTickCount();
				}
			}
		}
	} 

	//
	// step4: resume interrupt, end of scanning
	//
scan_end1:
    // Enable no. of rows in keypad (KRE = 1)
    // Configure columns as open-drain (KCO = 1)
    INSREG16(&m_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KRE),
        CSP_BITFVAL(KPP_KPCR_KRE, KPP_ROW_MASK));
    INSREG16(&m_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KCO),
        CSP_BITFVAL(KPP_KPCR_KCO, KPP_COLUMN_MASK));

    // Write 0's to all columns
    INSREG16(&m_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KCD),
        CSP_BITFVAL(KPP_KPDR_KCD, 0));

    // Configure rows as input, columns in use as output
    INSREG16(&m_pKPP->KDDR, CSP_BITFMASK(KPP_KDDR_KCDD),
        CSP_BITFVAL(KPP_KDDR_KCDD, KPP_COLUMN_MASK));
    INSREG16(&m_pKPP->KDDR, CSP_BITFMASK(KPP_KDDR_KRDD),
        CSP_BITFVAL(KPP_KDDR_KRDD, 0));

    // Disable key release interrupts and re-enable key depress interrupts.
    OUTREG16(&m_pKPP->KPSR,
        (CSP_BITFVAL(KPP_KPSR_KPP_EN, KPP_KPSR_KPP_EN_ENABLE) |
        CSP_BITFVAL(KPP_KPSR_KDIE, KPP_KPSR_KDIE_INT_ENABLE) |
        CSP_BITFVAL(KPP_KPSR_KRIE, KPP_KPSR_KRIE_INT_DISABLE) |
        CSP_BITFVAL(KPP_KPSR_KPKR, KPP_KPSR_KPKR_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KPKD, KPP_KPSR_KPKD_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KDSC, KPP_KPSR_KDSC_CLEAR)));

	// Disable KPP clocks for Power Management
    //BSPKppSetClockGatingMode(FALSE);
	return TRUE;
}

//! @}
//! @}

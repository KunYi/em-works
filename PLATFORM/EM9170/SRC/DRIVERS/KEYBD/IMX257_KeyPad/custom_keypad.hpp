//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{
//!
//! All rights reserved ADENEO 2007
//!
//! \brief		Stream driver for the Keypad embedded on the AT91SAM9262EK EvalBoard
//
//! \file 	custom_keypad.hpp
//!
//! \if subversion
//!   $URL: $
//!   $Author: $
//!   $Revision: $
//!   $Date: $
//! \endif
//! 
//-----------------------------------------------------------------------------

//! \addtogroup	Keypad
//! @{
//!
#ifndef __CUSTOM_KEYPAD_H__
#define __CUSTOM_KEYPAD_H__


////////////////////////////////////////////////////////////////////////////////
//                                                                      Includes
////////////////////////////////////////////////////////////////////////////////
// Local include
#include "keypad.hpp"

//
// Max_num_keys <= 32, we define a 4x4 matrix keypad which consists of 16 keys
//
#define NUM_MATRIX_KEY_IN			4
#define NUM_MATRIX_KEY_OUT		4

class CustomKeyPad: public KeyPad
{
public:

	CustomKeyPad();
	
	// To be implemented or overrided
	// 
	BOOL CustomInitialize(LPCTSTR pContext);
	BOOL CustomDeinitialize(void);
	
	// Power Management
	BOOL KeyPadPowerOff(void);
	BOOL KeyPadPowerOn(void);

	// Get the keypad keys state (1 bit for each key)
	DWORD GetKeypadState(void);

	// real code to implementation of physical keypad scan
	BOOL CustomKeypadScan(void);

private:
	BOOL RegQueryDword (HKEY hKey, LPWSTR lpKeyName, DWORD *pdwValue);
	BOOL GetKeysAssignement(HKEY hKey);

	BOOL BSPKppRegInit();

	DWORD*				m_dwKeysStates;
	PCSP_KPP_REGS	m_pKPP;						// handle to access keypad port in iMX257

	//
	// CS&ZHL SEP-08-2009: add var for number of KOUTs
	//
	DWORD		m_dwNbOfKOUT;
	UINT8		m_uColMaskTab[4];			// = {0x04, 0x08, 0x40, 0x80};
	UINT8		m_uRowMaskTab[4];		// = {0x01, 0x02, 0x04, 0x08};
	DWORD		m_dwScanState;				// = 0: Up, = 1: Down; = 2: Wait Release
};

#endif // __CUSTOM_KEYPAD_H__

//! @}

//! @}

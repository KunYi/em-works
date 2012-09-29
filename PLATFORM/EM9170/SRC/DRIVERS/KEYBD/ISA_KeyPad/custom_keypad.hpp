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
// Max_num_keys <= 32, we define a 4x4 or 4x5 matrix keypad which consists of 16 or 20 keys
//
#define NUM_MATRIX_KEY_IN		4
//#define NUM_MATRIX_KEY_OUT		4

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

private:
	BOOL RegQueryDword (HKEY hKey, LPWSTR lpKeyName, DWORD *pdwValue);
	
	BOOL GetKeysAssignement(HKEY hKey);
	BOOL GetISAPortAssignement(HKEY hKey);

	BOOL ISAPortInit(void);

	void		KOUT(BYTE ucValue);
	BYTE		KIN();

	// Variables for Matrix key pad
	DWORD m_dwKINPortOffset;
	DWORD m_dwKOUTPortOffset;

	DWORD						*m_dwKeysStates;
	PEM9K_CPLD_REGS	m_pISA;						// handle to access keypad port in ISA bus

	//
	// CS&ZHL SEP-08-2009: add var for number of KOUTs
	//
	DWORD m_dwNbOfKOUT;
};

#endif // __CUSTOM_KEYPAD_H__

//! @}

//! @}

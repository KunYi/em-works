//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{
//!
//! All rights reserved ADENEO 2007
//!
//! \brief		Stream driver for the Keypad embedded on the AT91SAM9262EK EvalBoard
//
//! \file	keypad.hpp
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

#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#define KEYPAD_MAX_KEY_NUMBER					32
#define KEYPAD_SR_MASK							(0xFFFFFFFF)

#define KBD_AUTO_REPEAT_INITIAL_DELAY_DEFAULT		500
#define KBD_AUTO_REPEAT_INITIAL_DELAY_MIN				250
#define KBD_AUTO_REPEAT_INITIAL_DELAY_MAX				1000

#define KBD_AUTO_REPEAT_KEYS_PER_SEC_DEFAULT		20
#define KBD_AUTO_REPEAT_KEYS_PER_SEC_MIN				2
#define KBD_AUTO_REPEAT_KEYS_PER_SEC_MAX				30



class KeyPad
{
public:
    KeyPad() ;
    
	// create both interrupt thread and keypad scan thread
    BOOL ThreadStart(void);

	// ack interrupt event 
    BOOL IsrThreadProc(void);

	// start keypad scanning and send keyboard event if available
	BOOL KeyPadScanProc(void);

	BOOL IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

	// a cpp shell to call IsrThreadProc()
	static DWORD KeyPadIsrThread(KeyPad *pKP);

	// a cpp shell to call KeyPadScanProc()
	static DWORD KeyPadScanThread(KeyPad *pKP);

	// Driver initialisation (IRQ request, ...)
	BOOL Initialize(LPCTSTR pContext);
	BOOL Deinitialize(void);

	
protected:
	UCHAR   m_KeypadKeyToVKey[KEYPAD_MAX_KEY_NUMBER];
	DWORD  m_dwNbKeypadKeys;
	DWORD  m_dwSysintr;
	DWORD  m_dwPollingTimeOut;
	
	void SendKeyEvent(DWORD dwKeyID, DWORD dwKeystate);
	
	DWORD m_dwAutoRepeatInitialDelay;
	DWORD m_dwAutoRepeatKeysPerSec;
	DWORD m_dwRepeatTimeout;

private:
	// Keys backup
	//DWORD m_dwLastKeyPressed;
	//DWORD m_dwLastKeyPressedDate;
	//DWORD m_dwOldKeypadState;
	//DWORD m_dwCurrentKeypadState;

	BOOL		m_bExitIST;
	HANDLE	m_hInterruptEvent;
	HANDLE	m_hISTthreadHandle;

	BOOL		m_bTerminateThread;
	HANDLE	m_hKeypadScanEvent;
	HANDLE	m_hKeypadScanThreadHandle;

	HANDLE	m_hKillThreadAckEvent;		// ACK event from being killed thread
	
// To be implemented or overrided
public:
	// 
	virtual BOOL CustomInitialize(LPCTSTR pContext)=0;
	virtual BOOL CustomDeinitialize(void)=0;
	
	// Power Management
    virtual BOOL KeyPadPowerOff(void);
    virtual BOOL KeyPadPowerOn(void);

	// Get the keypad keys state (1 bit for each key)
	//virtual DWORD GetKeypadState(void)=0;

	// real code to implementation of physical keypad scan
	virtual BOOL CustomKeypadScan(void)=0;
};


#define VK_0    0x030
#define VK_1    0x031
#define VK_2    0x032
#define VK_3    0x033
#define VK_4    0x034
#define VK_5    0x035
#define VK_6    0x036
#define VK_7    0x037
#define VK_8    0x038
#define VK_9    0x039
#define VK_A    0x041
#define VK_B    0x042
#define VK_C    0x043
#define VK_D    0x044
#define VK_E    0x045
#define VK_F    0x046
#define VK_G    0x047
#define VK_H    0x048
#define VK_I    0x049
#define VK_J    0x04A
#define VK_K    0x04B
#define VK_L    0x04C
#define VK_M    0x04D
#define VK_N    0x04E
#define VK_O    0x04F
#define VK_P    0x050
#define VK_Q    0x051
#define VK_R    0x052
#define VK_S    0x053
#define VK_T    0x054
#define VK_U    0x055
#define VK_V    0x056
#define VK_W    0x057
#define VK_X    0x058
#define VK_Y    0x059
#define VK_Z    0x05A

#endif // __KEYPAD_H__

//! @}

//! @}
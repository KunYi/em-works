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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*

  Copyright(c) 1998,1999 SIC/Hitachi,Ltd.
  Copyright(c) 1998,1999 3RD Rail Engineering.

	Module Name:

		ps2p465.cpp

	Revision History:

		26th April 1999		Released
		16th June  1999		Revised
		1999-Oct-19 cea		Change "magic" number delay to call to BusyWait
							in DRVLIB.LIB. This is a CPU clock frequency
							dependent wait function.
*/

#include <windows.h>
#include <ceddk.h>
//#include "mobytel.h"
#include <pkfuncs.h>

#include "ps2p465.hpp"
#include "keybddbg.h"

/* Wait before timing out a write to the HD64465.*/
#define WRITE_TIMEOUT       (50000)

//static unsigned int Delay_30uS;
//static unsigned int Delay_90uS;
//static unsigned int Delay_180uS;
//static unsigned int Delay_2000uS;

//
// Commands sent to the keyboard.
//
static const UINT8 cmdKeybdSetMode  = 0xF0;
static const UINT8 cmdKeybdReset    = 0xFF;
static const UINT8 cmdKeybdLights   = 0xED;

// Keyboard modes
static const UINT8 cmdKeybdModeAT   = 0x02;

//
// Commands sent to the mouse.
//
static const UINT8 cmdMouseReadId     = 0xF2;
static const UINT8 cmdMouseEnable     = 0xF4;
static const UINT8 cmdMouseSampleRate = 0xF3;
static const UINT8 cmdMouseDisable    = 0xF5;
static const UINT8 cmdMouseReset      = 0xFF;

static const UINT8 response8042IntelliMouseId	= 0x03;

//	Pointer to memory registers.
PVBYTE		v_pIoRegs;

UINT32		MouseInterruptControl;
UINT32		KeyboardInterruptControl;

extern DWORD dwHD64465Base;

void Ps2P465::SetModeIntelliMouse( 	void 	)
{
	// IntelliMouse(R) is uniquely identified by issuing the specific series of Set Report Rate commands:
	// 200Hz (0xC8), then 100Hz (0x64), then 80Hz (0x50).  The Set Report Rate commands are valid and we 
	// therefore have to set the report rate back to the default 100Hz (this is done be MouseId() ).
    REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
    MouseCommandPut(cmdMouseSampleRate);
    REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
    MouseCommandPut(0xC8);
    REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
    MouseCommandPut(cmdMouseSampleRate);
    REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
    MouseCommandPut(0x64);
    REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
    MouseCommandPut(cmdMouseSampleRate);
    REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
    MouseCommandPut(0x50);
}	


UINT8 Ps2P465:: MouseId( 	void )
{
	UINT8	ui8MouseId=0;
    BOOL    bReturn;
    
    /* Enable the Mouse input again. */
    REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
    bReturn=MouseCommandPut(cmdMouseReadId);
    REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
    bReturn=MousePollRead(&ui8MouseId) ;
    RETAILMSG(1,(TEXT("ps2p465.cpp Mouse READ ID returned: %02X\r\n"),ui8MouseId));
    return ui8MouseId;
    
    
}

/*++

Ps2P465::
MouseTest:


This routine tests for a mouse present on the HD64465 PS/2 mouse (AUX) port.

It's not a very good method. If the mouse interrupts are active
this function will fail. This function should only be called from
the initializer prior to the interrupt enable.

Returns true if a mouse is found.

--*/
BOOL
Ps2P465::
MouseTest(
	void
	)
{
	BOOL	bRet = FALSE;
//	UINT8	ui8Data;

//NKDbgPrintfW(L"ps2p465.cpp Mouse test entry\r\n");

	if (MouseInterruptControl) {
//NKDbgPrintfW(L"ps2p465.cpp Mouse test MouseInterruptControl non-zerp\r\n");
		bRet = TRUE;
	}
	else {
		
		bRet=MouseCommandPut(cmdMouseDisable);	// Send Disable command
        //ASSERT(bRet);
        // Read mouse ID to determine if IntelliMouse is present.
        SetModeIntelliMouse();
        if( response8042IntelliMouseId == MouseId() )
        {
            m_bIntelliMouseFound = true;
        }
        
        //NKDbgPrintfW(L"ps2p465.cpp Mouse command SET SAMPLE RATE\r\n");
        /* Enable the Mouse input again. */
        REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
        MouseCommandPut(cmdMouseSampleRate);
        //NKDbgPrintfW(L"ps2p465.cpp Mouse data for SET SAMPLE RATE\r\n");
        /* Enable the Mouse input again. */
        REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
        MouseCommandPut(0x28);

		/* Enable the Mouse input again. */
		REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
		bRet= MouseCommandPut(cmdMouseEnable);
    }

	/* Enable the Mouse input again. */
	REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
	if ( !bRet )
		{
//NKDbgPrintfW(L"ps2p465.cpp Mouse not found: %02X\r\n",ui8Data);
		}

//NKDbgPrintfW(L"ps2p465.cpp Mouse test exit\r\n");
	return bRet;
}


/*++

Ps2P465::
MouseReset:

Sends the mouse reset command to the mouse and reads the response.

Returns true if the success response is read.


--*/
BOOL
Ps2P465::
MouseReset(
	void
	)
{
	BOOL	bRet = FALSE;
//	UINT8	ui8Data;

/*
 * Mouse reset removed. Always returns TRUE.
 */
	bRet = TRUE;

	return bRet;
}



/*++

Ps2P465::
MouseDataRead:

Reads data from the HD64465 output port.


--*/
BOOL
Ps2P465::
MouseDataRead(
	UINT8	*pui8Data
	)
{
	
	*pui8Data = (UINT8)(REG16(v_pIoRegs, MOUSE_CSR));
//	REG16(v_pIoRegs, MOUSE_ISR) = MOUSE_RDRF;
	return TRUE;
}



/*++

Ps2P465::
MousePollRead:

This routine reads data from the HD64465 PS/2 mouse (AUX) port.
It's not a very good method. If the mouse interrupts are active
this function will fail. This function should only be called from
the initializer prior to the interrupt enable.

--*/
BOOL
Ps2P465::
MousePollRead(
	UINT8	*pui8
	)
{
	BOOL	bRet = FALSE;
	UINT8	ui8Status;

	UINT32	cSleeps = 100;	//	Rather arbitrary.


  if(MouseInterruptControl == 0) {
	MouseInterruptControl = 2;
	while ( cSleeps )
		{
		ui8Status = (UINT8)(REG16(v_pIoRegs, MOUSE_ISR));
		if ( ui8Status & MOUSE_RDRF )
			{
			break;
			}
		else
			{
			Sleep(10);
			cSleeps--;
			}
		}

	//	May as well read it even if we failed.
	*pui8 = (UINT8)(REG16(v_pIoRegs, MOUSE_CSR));
	REG16(v_pIoRegs, MOUSE_ISR) = MOUSE_RDRF;

	if ( cSleeps )
		{
		bRet = TRUE;
		}
	else
		{
		ERRORMSG(1,(TEXT("Ps2P465::MousePollRead: too long waiting for mouse response.\r\n")));
		}
	MouseInterruptControl = 0;
  }
	return bRet;
}


/*++

Ps2P465::
MouseInterruptEnable:

Enables HD64465 auxiliary interrupts.


--*/
BOOL
Ps2P465::
MouseInterruptEnable(
	void
	)
{
	UINT16 k;

    while (MouseInterruptControl == 2) Sleep(20);
	MouseInterruptControl = 1;
//  Ack the interrupt.
	k = (REG16(v_pIoRegs, MOUSE_CSR));
//  Write 1 to clear RDRF bit
	REG16(v_pIoRegs, MOUSE_ISR) = MOUSE_RDRF;

//  Set the register properly to receive
	REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;


	return TRUE;
}



/*++

Ps2P465::
KeybdInterruptEnable:

Enables HD64465 keyboard interrupts.


--*/
BOOL
Ps2P465::
KeybdInterruptEnable(
	void
	)
{
	UINT16 k;

	while(KeyboardInterruptControl == 2) Sleep(20);
	KeyboardInterruptControl = 1;
//  Ack the interrupt.
	k = (REG16(v_pIoRegs, KB_CSR));
//  Write 1 to clear RDRF bit
	REG16(v_pIoRegs, KB_ISR) = KB_RDRF;

//  Set the register properly to receive
	REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN;

	return TRUE;
}



/*++

Ps2P465::
KeyboardInterfaceTest:

Writes the keyboard interface test command to the HD64465 command register
and reads the response.

Returns true if the success response is read.


--*/
BOOL
Ps2P465::
KeyboardInterfaceTest(
	void
	)
{
	BOOL	bRet = FALSE;
	UINT8	ui8Data;


//NKDbgPrintfW(L"ps2p465.cpp Keyboard test entry\r\n");
	if (KeyboardInterruptControl) {
		bRet = TRUE;
//NKDbgPrintfW(L"ps2p465.cpp Keyboard test KeyboardInterruptControl non-zero\r\n");
	}
	else {

		bRet = TRUE;		// This is here for debug (** cea 6/15/99)

//NKDbgPrintfW(L"ps2p465.cpp Keyboard command RESET\r\n");
		if ( !KeyboardCommandPut(cmdKeybdReset) )	// Send RESET command
		{
			goto leave;
		}

//NKDbgPrintfW(L"ps2p465.cpp Keyboard waiting for RESET BAT code\r\n");

		/* Enable the Keyboard input again. */
		REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN;
		if ( !KeyboardPollRead(&ui8Data) )			// Read BAT byte
		{
			goto leave;
		}

//NKDbgPrintfW(L"ps2p465.cpp Keyboard BAT returned: %02X\r\n",ui8Data);

		bRet = TRUE;

	}

leave:
	/* Enable the Keyboard input again. */
	REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN;
	if ( !bRet )
		{
//NKDbgPrintfW(L"ps2p465.cpp Keyboard not found: %02X\r\n",ui8Data);
		}

//NKDbgPrintfW(L"ps2p465.cpp Keyboard test exit\r\n");
	return bRet;
}



/*++

Ps2P465::
KeyboardReset:

Sends the keyboard reset command to the keyboard and reads the response.

Returns true if the success response is read.


--*/
BOOL
Ps2P465::
KeyboardReset(
	void
	)
{
	BOOL	bRet = FALSE;
//	UINT8	ui8Data;

/*
 * Keyboard reset removed. Always returns flase.
 */
	return bRet;
}



/*++

Ps2P465::
KeyboardLights:

Sets the keyboard indicator lights.


--*/
void
Ps2P465::
KeyboardLights(
	unsigned int	fLights
	)
{
#if 0		// temporarily disabled due to timing problems with the HD64465 keyboard interface
//NKDbgPrintfW(L"ps2p465.cpp KeyboardLights(%02X)\r\n",fLights);
	if ( !KeyboardCommandPut(cmdKeybdLights) )
		{
//NKDbgPrintfW(L"ps2p465.cpp KeyboardLights command put failed\r\n");
		goto leave;
		}

	if ( !KeyboardCommandPut(fLights) )
		{
//NKDbgPrintfW(L"ps2p465.cpp KeyboardLights data put failed\r\n");
		goto leave;
		}

leave:
//NKDbgPrintfW(L"ps2p465.cpp KeyboardLights exit\r\n");
	return;
#endif
}


/*++

Ps2P465::
KeybdDataRead:

Reads data from HD64465 output port.


--*/
BOOL
Ps2P465::
KeybdDataRead(
	UINT8	*pui8Data
	)
{

	*pui8Data = (UINT8)(REG16(v_pIoRegs, KB_CSR));
//	REG16(v_pIoRegs, KB_ISR) = KB_RDRF;
	return TRUE;
}


/*++

Ps2P465::
KeyboardPollRead:

SPINS waiting for the output buffer to go full and then reads the data
from the output buffer.  Returns true if the buffer goes full, false if
the buffer never goes full.  Remember that output means output of the
HD64465, not the PC.

--*/
BOOL
Ps2P465::
KeyboardPollRead(
	UINT8	*pui8
	)
{
	BOOL	bRet = FALSE;
	UINT8	ui8Status;

	int		cSleeps = 100;	//	Rather arbitrary.

  if(KeyboardInterruptControl == 0) {
	KeyboardInterruptControl = 2;
	while ( cSleeps )
		{
		ui8Status = (UINT8)(REG16(v_pIoRegs, KB_ISR));
		
		if ( ui8Status & KB_RDRF )
			{
			break;
			}
		else
			{
			Sleep(10);
			cSleeps--;
			}
		}

	//	May as well read it even if we failed.
	*pui8 = (UINT8)(REG16(v_pIoRegs, KB_CSR));
	REG16(v_pIoRegs, KB_ISR) = KB_RDRF;

	if ( cSleeps )
		{
		bRet = TRUE;
		}
	else
		{
		ERRORMSG(1,(TEXT("Ps2p465::KeyboardPollRead: too long waiting for keyboard response.\r\n")));
		}
	KeyboardInterruptControl = 0;
  }
	return bRet;
}

/*++

Ps2P465::
KeyboardCommandPut:

This function sends a byte one bit ay a time to the HD64465 keyboard interface.
Then waits for the acknowledge (0xFA) byte.

--*/
BOOL
Ps2P465::
KeyboardCommandPut(
	UINT8	ui8Cmd
	)
{
	UINT8	ui8Data;

	UINT32 i, j, parity;
	UINT32 tm;
	UINT16 k;


	/* Wait for 90 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_90uS );  // This allows the CLOCK and DATA lines to be high
							 // before the HOST sends a command to the device

	/* KBCIE = 0, KBCOE = 1, KBDOE = 0, KBCD = 0, KBDD = 0 */
	/* Disable input shift register, Set clock line low, Set DATA line tri-state */
	REG16(v_pIoRegs, KB_CSR) = 0x4000;
  
	/* Wait for 90 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_90uS );

	/* KBCIE = 0, KBCOE = 1, KBDOE = 1, KBCD = 0, KBDD = 0 */
	/* Disable input shift register, Set CLOCK line low, Set DATA line low */
	REG16(v_pIoRegs, KB_CSR) = 0x6000;
	REG16(v_pIoRegs, KB_ISR) = KB_RDRF;		// clear any waiting data

	/* Wait for 90 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_180uS );

	/* KBCIE = 0, KBCOE = 0, KBDOE = 1, KBCD = 1, KBDD = 0 */
	/* Disable input shift register, Set CLOCK line tri-state, Set DATA line low */
	REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN+0x3000;

	// Poll KBCK pin. (bit 10). Whenever it goes low from high, write  one bit.

	// Data to send: 8 data, 1 parity, 1 stop.
	parity = 0;
	for(i=0; i < 10; i++) {


		// Poll KBCK pin (bit 10) to go low from high.
		tm = 0;
		k = 0;
		while( (!(k & KB_CLK_RD)) && (tm < WRITE_TIMEOUT) ) {	// Wait while CLOCK is low
			k = REG16(v_pIoRegs, KB_CSR);
			tm++;
		}
		if(tm == WRITE_TIMEOUT) goto error_exit;

		tm = 0;
		k = KB_CSR;
		while( (k & KB_CLK_RD) && (tm < WRITE_TIMEOUT) ) {	// Wait while CLOCK is high
			k = REG16(v_pIoRegs, KB_CSR);
			tm++;
		}
		if(tm == WRITE_TIMEOUT) goto error_exit;

		if(i<8) {
			// Send next data bit. LSB first to Keyboard Data Drive (bit 11).
			// Writing 0x3800 means writing 1 to Keyboard Data Drive bit, 
			// Writing 0x3000 means writing 0 to Keyboard Data Drive bit.
			j = (ui8Cmd & (1 << i));
			if(j) {
				parity ++;
				REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN+0x3800;
			}
			else {
				REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN+0x3000;
			}
		}
		else {
			if(i == 8) { // Send the parity bit. make parity + data = odd no. of 1's
				if ((parity % 2) == 0) {
					REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN+0x3800;
				}
				else {
					REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN+0x3000;
				}
			}
			if (i == 9) { // Send the Stop bit
				REG16(v_pIoRegs, KB_CSR) = 0x6800; // set CLOCK low to hold off ACK
			}
		}
	}

	/* Wait for 90 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_90uS );
	REG16(v_pIoRegs, KB_CSR) = 0x3800;		// set CLOCK high

	/* Wait for 30 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_30uS );

	k = REG16(v_pIoRegs, KB_CSR);
	REG16(v_pIoRegs, KB_ISR) = KB_RDRF;		// clear any waiting data
	if (KeyboardInterruptControl) {

/* We should try to receive the ACK from the keyboard here but when 
 * the interrupt is enabled the hardware just will not cooperate.
 *
 * So inplace of doing the correct this we just waste 2,000 microseconds
 * and hope that the ACK came in. If the ACK is late it will be processed
 * as a scan code. This causes some other problems.
 */
		REG16(v_pIoRegs, KB_CSR) = 0;			// Disable keyboard input

		/* Wait for 2000 us. */
        DDKDriverSleep(2000);
		//BusyWait( Delay_2000uS );
//NKDbgPrintfW(L"ps2p465.cpp KeyboardCommandPut(%02X), delay while ACK comes in.\r\n",ui8Cmd);
	}
	else {

		REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN;
		KeyboardPollRead(&ui8Data);

//NKDbgPrintfW(L"ps2p465.cpp KeyboardCommandPut(%02X), data: %02X\r\n",ui8Cmd,ui8Data);

	}

	/* Enable the keyboard input again. */
	REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN;

	return TRUE;

error_exit:
	// Still Enable the interrupts. So that if the keyboard is 
	// plugged in later on, it'll work.

//NKDbgPrintfW(L"ps2p465.cpp KeyboardCommandPut(%02X), Error.\r\n",ui8Cmd);

	REG16(v_pIoRegs, KB_ISR) = KB_RDRF;
	REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN;

	return FALSE;

}

/*++

Ps2P465::
MouseCommandPut:

This function sends a byte one bit ay a time to the HD64465 Mouse interface.
Then waits for the acknowledge (0xFA) byte.

--*/
BOOL
Ps2P465::
MouseCommandPut(
	UINT8	ui8Cmd
	)
{
	UINT8	ui8Data;

	UINT32 i, j, parity;
	UINT32 tm;
	UINT16 k;

	/* Wait for 90 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_90uS );  // This allows the CLOCK and DATA lines to be high
							 // before the HOST sends a command to the device

	/* MSCIE = 0, MSCOE = 1, MSDOE = 0, MSCD = 0, MSDD = 0 */
	/* Disable input shift register, Set clock line low, Set DATA line tri-state */
	REG16(v_pIoRegs, MOUSE_CSR) = 0x4000;
  
	/* Wait for 90 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_90uS );

	/* MSCIE = 0, MSCOE = 1, MSDOE = 1, MSCD = 0, MSDD = 0 */
	/* Disable input shift register, Set CLOCK line low, Set DATA line low */
	REG16(v_pIoRegs, MOUSE_CSR) = 0x6000;
	REG16(v_pIoRegs, MOUSE_ISR) = MOUSE_RDRF;	// clear any waiting data

	/* Wait for 90 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_180uS );

	/* MSCIE = 0, MSCOE = 0, MSDOE = 1, MSCD = 1, MSDD = 0 */
	/* Disable input shift register, Set CLOCK line tri-state, Set DATA line low */
	REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN+0x3000;

	// Poll MSCK pin. (bit 10). Whenever it goes low from high, write  one bit.

	// Data to send: 8 data, 1 parity, 1 stop.
	parity = 0;
	for(i=0; i < 10; i++) {


		// Poll MSCK pin (bit 10) to go low from high.
		tm = 0;
		k = REG16(v_pIoRegs, MOUSE_CSR);  // Davli k = 0;
		while( (!(k & MOUSE_CLK_RD)) && (tm < WRITE_TIMEOUT) ) {	// Wait while CLOCK is low
			k = REG16(v_pIoRegs, MOUSE_CSR);
			tm++;
		}
		if(tm == WRITE_TIMEOUT) goto error_exit;

		tm = 0;
		k = MOUSE_CSR;
		while( (k & MOUSE_CLK_RD) && (tm < WRITE_TIMEOUT) ) {	// Wait while CLOCK is high
			k = REG16(v_pIoRegs, MOUSE_CSR);
			tm++;
		}
		if(tm == WRITE_TIMEOUT) goto error_exit;

		if(i<8) {
			// Send next data bit. LSB first to Mouse Data Drive (bit 11).
			// Writing 0x3800 means writing 1 to Mouse Data Drive bit, 
			// Writing 0x3000 means writing 0 to Mouse Data Drive bit.
			j = (ui8Cmd & (1 << i));
			if(j) {
				parity ++;
				REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN+0x3800;
			}
			else {
				REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN+0x3000;
			}
		}
		else {
			if(i == 8) { // Send the parity bit. make parity + data = odd no. of 1's
				if ((parity % 2) == 0) {
					REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN+0x3800;
				}
				else {
					REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN+0x3000;
				}
			}
			if (i == 9) { // Send the Stop bit
				REG16(v_pIoRegs, MOUSE_CSR) = 0x6800; // set CLOCK low to hold off ACK
			}
		}
	}

	/* Wait for 90 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_90uS );
	REG16(v_pIoRegs, MOUSE_CSR) = 0x3800;		// set CLOCK tri-state DATA high

	/* Wait for 30 us. */
    DDKDriverSleep(90);
	//BusyWait( Delay_30uS );
	k = REG16(v_pIoRegs, MOUSE_CSR);
	REG16(v_pIoRegs, MOUSE_ISR) = MOUSE_RDRF;	// clear any waiting data

	if (MouseInterruptControl) {

/* We should try to receive the ACK from the mouse here but when 
 * the interrupt is enabled the hardware just will not cooperate.
 *
 * So inplace of doing the correct this we just waste 2,000 microseconds
 * and hope that the ACK came in. If the ACK is late it will be processed
 * as a data packet byte. This causes some other problems.
 */
		REG16(v_pIoRegs, MOUSE_CSR) = 0;			// Disable Mouse input

		/* Wait for 2000 us. */
        DDKDriverSleep(90);
		//BusyWait( Delay_2000uS );
//NKDbgPrintfW(L"ps2p465.cpp MouseCommandPut(%02X), delay while ACK comes in.\r\n",ui8Cmd);
	}
	else {

		REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;
		MousePollRead(&ui8Data);

//NKDbgPrintfW(L"ps2p465.cpp MouseCommandPut(%02X), data: %02X\r\n",ui8Cmd,ui8Data);

	}
	/* Enable the Mouse input again. */
	REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;

	return TRUE;

error_exit:
	// Still Enable the interrupts. So that if the mouse is plugged in later
	// on, it'll work.

//NKDbgPrintfW(L"ps2p465.cpp MouseCommandPut(%02X), Error.\r\n",ui8Cmd);

	REG16(v_pIoRegs, MOUSE_ISR) = MOUSE_RDRF;
	REG16(v_pIoRegs, MOUSE_CSR) = MOUSE_CLK_EN;

	return FALSE;

}

/*++

Ps2P465::
Initialize:

Initializes the Ps2P465 object.


--*/
BOOL
Ps2P465::
Initialize(
	PVBYTE	iopBase
	)
{

    m_iopBase = iopBase;

    m_bMouseFound = FALSE;
    m_bIntelliMouseFound = false;
    m_fEnableWake = FALSE;
    MouseInterruptControl = 1;
    KeyboardInterruptControl = 1;

    if ( KeyboardCommandPut(cmdKeybdSetMode) ) 
        {
        KeyboardCommandPut(cmdKeybdModeAT);
        }

    if ( MouseTest() )
        {
        m_bMouseFound = TRUE;
        MouseCommandPut(cmdMouseEnable);
        }


    return TRUE;
}


BOOL
KeybdDriverInitializeAddresses(
	DWORD dwBase, DWORD dwLen
	)
{

	//Delay_30uS   = AdjustMicroSecondsToLoopCount( 30 );
	//Delay_90uS   = AdjustMicroSecondsToLoopCount( 90 );
	//Delay_180uS  = AdjustMicroSecondsToLoopCount( 180 );
	//Delay_2000uS = AdjustMicroSecondsToLoopCount( 2000 );
    CalibrateStallCounter();

    //  Map HD64465 PS/2 interface base address
	v_pIoRegs = (PVBYTE)VirtualAlloc(0, 0x1000, MEM_RESERVE, PAGE_NOACCESS);
	if ( v_pIoRegs == NULL )
		{
		ERRORMSG(1,
			(TEXT("KeybdDriverInitializeAddresses: PS2 VirtualAlloc failed!\r\n")));
		goto error_return;
		}

	if ( !VirtualCopy((PVOID)v_pIoRegs, (PVOID) dwBase, dwLen, PAGE_READWRITE|PAGE_NOCACHE) )
		{
		ERRORMSG(1,
			(TEXT("KeybdDriverInitializeAddresses: PS2 VirtualCopy failed!\r\n")));
		goto error_return;
		}

    // Set the register properly to receive
	REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN;

    // Clear any initial interrupts
	REG16(v_pIoRegs, KB_ISR) = KB_RDRF;

    // Set the register properly to receive after clearing interrupt
	REG16(v_pIoRegs, KB_CSR) = KB_CLK_EN;

	return TRUE;

error_return:
	if ( v_pIoRegs )
		VirtualFree((PVOID)v_pIoRegs, 0, MEM_RELEASE);
	v_pIoRegs = 0;
	return FALSE;

}

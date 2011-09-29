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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

    @doc	EXTERNAL DRIVERS

    @module	keypadist.cpp |

This file implements the interrupt service thread part of the platform
independent code of the keyboard driver.  This is provided as a sample to
platform driver writers and is expected to be able to be used without
major modification on most hardware platforms.

*/

#include <windows.h>
#include <nkintr.h>
#include "keypad.hpp"
#include <winuserm.h>

//	Auto repeat #defines and state variables.
#define AR_WAIT_FOR_ANY		0
#define AR_INITIAL_DELAY	1
#define AR_AUTOREPEATING	2
#define AR_AUTO_PARTIAL     3   // Auto repeat was interrupted by real PDD event.

#define AUTO_REPEAT_INITIAL_DELAY_MIN		500
#define AUTO_REPEAT_INITIAL_DELAY_MAX		250
#define AUTO_REPEAT_INITIAL_DELAY_DEFAULT	500

#define AUTO_REPEAT_KEYS_PER_SEC_MIN		2
#define AUTO_REPEAT_KEYS_PER_SEC_MAX		4
#define AUTO_REPEAT_KEYS_PER_SEC_DEFAULT	2




static int              v_AutoRepeatState = AR_WAIT_FOR_ANY;
static DWORD            v_AutoRepeatInitialDelay = AUTO_REPEAT_INITIAL_DELAY_DEFAULT;
static DWORD            v_AutoRepeatKeysPerSec = AUTO_REPEAT_KEYS_PER_SEC_DEFAULT;
static UINT32           v_AutoRepeatVKey;
static UINT32           v_AutoRepeatScanCode;
static KEY_STATE_FLAGS  v_AutoRepeatKeyStateFlags;



static VOID KeybdEvent(DWORD dwVk, DWORD dwSc, KEY_STATE_FLAGS flags)
{
    DWORD dwFlags = KeyStateIsDown(flags) ? 0 : KEYEVENTF_KEYUP;
    keybd_event((BYTE) dwVk, (BYTE) dwFlags, dwFlags, 0);
}


/*++

KeybdDriverThread:

Keyboard driver interrupt service thread.


Return Value:

Never returns.

--*/
BOOL
KeyPadIstLoop(
            HANDLE  hevInterrupt
            )
{
    UINT32          VKeyBuf[16];            //	hardcoded w/ PDD.
    UINT32          ScanCodeBuf[16];        //	hardcoded w/ PDD.
    KEY_STATE_FLAGS KeyStateFlagsBuf[16];   //  hardcoded w/ PDD.

    UINT32          RemapVKeyBuf[16];
    UINT32          RemapScanCodeBuf[16];
    KEY_STATE_FLAGS RemapKeyStateFlagsBuf[16];

    int             cKeyEvents;
    int             iKeyEventIdx;

    int             cRemapEvents;
    int             iRemapIdx;

    DWORD           AutoRepeatKeysPerSec;
    long            AutoRepeatTimeout;
    BOOL            fSendAutoRepeatKey;
    DWORD           MRKeyTimeForPolling = 0;    //Get rid of compiler uninitialized variable warning.

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    AutoRepeatTimeout = INFINITE;

    wait_for_keybd_interrupt:
    fSendAutoRepeatKey = FALSE;

//	Grab a copy once w/o critical section.
    AutoRepeatKeysPerSec = v_AutoRepeatKeysPerSec;

//	0 keys per second => auto repeat disabled.
    if ( AutoRepeatKeysPerSec == 0 )
        v_AutoRepeatState = AR_WAIT_FOR_ANY;

    if ( v_AutoRepeatState == AR_WAIT_FOR_ANY )
    {
        AutoRepeatTimeout = INFINITE;
    }
    else if ( v_AutoRepeatState == AR_INITIAL_DELAY )
    {
        AutoRepeatTimeout = v_AutoRepeatInitialDelay;
    }
    else if ( v_AutoRepeatState == AR_AUTO_PARTIAL )
    {
        long lTemp = AutoRepeatTimeout - (GetTickCount() - MRKeyTimeForPolling);
        AutoRepeatTimeout = (lTemp > 0) ? lTemp : 0;
    }
    else
    {
        AutoRepeatTimeout = 1000/AutoRepeatKeysPerSec;
    }

    MRKeyTimeForPolling = GetTickCount();
    if ( WaitForSingleObject(hevInterrupt, AutoRepeatTimeout) == WAIT_TIMEOUT )
    {

/*	On power off, the v_AutoRepeatState is set to AR_WAIT_FOR_ANY, but we
cannot reset the WaitForSingleObject timeout value.  This means that when
we power on, we could (and probably will) come back from the wait with a
wait timeout status.  In this case, we do nothing and come back around to
wait again with the proper timeout.  */

        if ( v_AutoRepeatState == AR_WAIT_FOR_ANY )
            ;   //	do nothing
        else if ( v_AutoRepeatState == AR_INITIAL_DELAY )
        {
            fSendAutoRepeatKey = TRUE;
            v_AutoRepeatState = AR_AUTOREPEATING;
        }
        else if ( v_AutoRepeatState == AR_AUTO_PARTIAL )
        {
            fSendAutoRepeatKey = TRUE;
            v_AutoRepeatState = AR_AUTOREPEATING;
        }
        else
        {
            fSendAutoRepeatKey = TRUE;
        }
    }
    else
    {
        //	We got a keyboard interrupt but there may or may not be key events.
        cKeyEvents = KeypdPdd_GetEventEx(VKeyBuf, ScanCodeBuf, KeyStateFlagsBuf);

        if ( cKeyEvents )
        {
            for ( iKeyEventIdx = 0; iKeyEventIdx < cKeyEvents; iKeyEventIdx++ )
            {
                if ( KeyStateIsDown(KeyStateFlagsBuf[iKeyEventIdx]) )
                {
                    MRKeyTimeForPolling = GetTickCount();
                    v_AutoRepeatState = AR_INITIAL_DELAY;
                    v_AutoRepeatVKey = VKeyBuf[iKeyEventIdx];
                    v_AutoRepeatScanCode = ScanCodeBuf[iKeyEventIdx];
                    v_AutoRepeatKeyStateFlags = KeyStateFlagsBuf[iKeyEventIdx];

                    cRemapEvents = KeybdDriverRemapVKeyDownEx(
                                                             VKeyBuf[iKeyEventIdx],
                                                             ScanCodeBuf[iKeyEventIdx],
                                                             KeyStateFlagsBuf[iKeyEventIdx],
                                                             RemapVKeyBuf,
                                                             RemapScanCodeBuf,
                                                             RemapKeyStateFlagsBuf
                                                             );

                    for ( iRemapIdx = 0; iRemapIdx < cRemapEvents; iRemapIdx++ )
                    {
//						RETAILMSG(1,
//							(TEXT("D %d %x %x\r\n"),
//								iRemapIdx, RemapVKeyBuf[iRemapIdx], RemapKeyStateFlagsBuf[iRemapIdx]));

/*						NKDbgPrintfW(TEXT("D %d %x %x\r\n"),iRemapIdx, 
                                                            RemapVKeyBuf[iRemapIdx], 
                                                            RemapKeyStateFlagsBuf[iRemapIdx]);*/
                        KeybdEvent(
                            RemapVKeyBuf[iRemapIdx],
                            RemapScanCodeBuf[iRemapIdx],
                            RemapKeyStateFlagsBuf[iRemapIdx]);

                        if ((RemapVKeyBuf[iRemapIdx] == VK_TVOLUMEUP) || (RemapVKeyBuf[iRemapIdx] == VK_TVOLUMEDOWN) ||
                            (RemapVKeyBuf[iRemapIdx] == VK_UP) || (RemapVKeyBuf[iRemapIdx] == VK_DOWN))
                        {
                            KeybdEvent(
                                RemapVKeyBuf[iRemapIdx],
                                RemapScanCodeBuf[iRemapIdx],
                                0);
                            v_AutoRepeatState = AR_WAIT_FOR_ANY;
                        }
                    }
                }
                else
                {
//					RETAILMSG(1,(TEXT("reset keystate to up\r\n")));
//					NKDbgPrintfW(TEXT("reset keystate to up\r\n"));

                    v_AutoRepeatState = AR_WAIT_FOR_ANY;

                    cRemapEvents = KeybdDriverRemapVKeyUpEx(
                                                           VKeyBuf[iKeyEventIdx],
                                                           ScanCodeBuf[iKeyEventIdx],
                                                           KeyStateFlagsBuf[iKeyEventIdx],
                                                           RemapVKeyBuf,
                                                           RemapScanCodeBuf,
                                                           RemapKeyStateFlagsBuf
                                                           );

                    for ( iRemapIdx = 0; iRemapIdx < cRemapEvents; iRemapIdx++ )
                    {
//					RETAILMSG(1, (TEXT("U %d %x %x\r\n"), j, RemapVKeyBuf[j], RemapKeyStateFlagsBuf[j]));
//					NKDbgPrintfW(TEXT("U %d %x %x\r\n"), iRemapIdx, RemapVKeyBuf[iRemapIdx], RemapKeyStateFlagsBuf[iRemapIdx]);
                        KeybdEvent(
                            RemapVKeyBuf[iRemapIdx],
                            RemapScanCodeBuf[iRemapIdx],
                            RemapKeyStateFlagsBuf[iRemapIdx]);
                    }
                }
            }
        }
        else
        {
//	We did not get a timeout from the wait or key events.  We must be periodically polling.
//	This means that we will need to do the timing here.
            if ( ( v_AutoRepeatState == AR_INITIAL_DELAY ) ||
                 ( v_AutoRepeatState == AR_AUTO_PARTIAL ) ||
                 ( v_AutoRepeatState == AR_AUTOREPEATING ) )
            {
                v_AutoRepeatState = AR_AUTO_PARTIAL;

            }
        }
//  Ack the interrupt.
        InterruptDone(SYSINTR_KEYPAD);
    }

    if ( fSendAutoRepeatKey )
    {
        cRemapEvents = KeybdDriverRemapVKeyDownEx(
                                                 v_AutoRepeatVKey,
                                                 v_AutoRepeatScanCode,
                                                 v_AutoRepeatKeyStateFlags,
                                                 RemapVKeyBuf,
                                                 RemapScanCodeBuf,
                                                 RemapKeyStateFlagsBuf
                                                 );

        for ( iRemapIdx = 0; iRemapIdx < cRemapEvents; iRemapIdx++ )
        {
//			RETAILMSG(1,
//				(TEXT("D %d %x %x\r\n"),
//					j, RemapVKeyBuf[j], RemapKeyStateFlagsBuf[j]));
//			NKDbgPrintfW(TEXT("DAR %d %x %x\r\n"),iRemapIdx, RemapVKeyBuf[iRemapIdx], RemapKeyStateFlagsBuf[iRemapIdx]);

            KeybdEvent(
                RemapVKeyBuf[iRemapIdx],
                RemapScanCodeBuf[iRemapIdx],
                RemapKeyStateFlagsBuf[iRemapIdx]);
        }
    }

    goto wait_for_keybd_interrupt;

    ERRORMSG(1, (TEXT("Keyboard driver thread terminating.\r\n")));
    return (0);
}



/*++

@doc EXTERNAL DRIVERS

@func

System power state change notification.

@comm This routine is called in a kernel context and may not make any
system calls whatsoever.  It may read and write its own memory and that's
about it.

@comm Resets the auto-repeat state and calls the <f KeybdPdd_PowerHandler> routine.

--*/
void KeybdDriverPowerHandler(
                            BOOL    bOff    // @parm TRUE, the system is powering off; FALSE, the system is powering up.
                            )
{
    v_AutoRepeatState = AR_WAIT_FOR_ANY;
    KeypdPdd_PowerHandler(bOff);
    return;
}


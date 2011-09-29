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
/* Copyright 1999 Intel Corporation. All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.
** Title to the Material remains with Intel Corporation or its suppliers and licensors.
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
*/

#ifndef __KEYPAD_CTRL_H__
#define __KEYPAD_CTRL_H__

// TODO - temp.
#define SYSINTR_KEYPAD          (SYSINTR_FIRMWARE+4)

class KeyPad
{

    HANDLE  m_hevInterrupt;

public:

    KeyPad() { m_hevInterrupt = NULL; }
    ~KeyPad() { if (m_hevInterrupt) CloseHandle(m_hevInterrupt); }

    BOOL Initialize(void);
    BOOL KeyPadDataRead(UINT8 *pui8Data);
    BOOL KeyPadPowerOff(void);
    BOOL KeyPadPowerOn(void);

    BOOL IsrThreadStart(void);
    BOOL IsrThreadProc(void);

    friend void        KeypdPdd_PowerHandler(BOOL bOff);

    friend int WINAPI  KeypdPdd_GetEventEx(UINT32 VKeyBuf[16],
                                           UINT32 ScanCodeBuf[16],
                                           KEY_STATE_FLAGS KeyStateFlagsBuf[16]);
};


#define         DISABLE    0
#define         ENABLE     1


UINT32  ScanCodeToVKeyEx(
                        UINT32          ScanCode,
                        KEY_STATE_FLAGS KeyStateFlags,
                        UINT32          VKeyBuf[16],
                        UINT32          ScanCodeBuf[16],
                        KEY_STATE_FLAGS KeyStateFlagsBuf[16]
                        );

int     KeybdDriverRemapVKeyDownEx(
                                  UINT32          VirtualKey,
                                  UINT32          ScanCode,
                                  KEY_STATE_FLAGS KeyStateFlags,
                                  UINT32          RemapVKeyBuf[16],
                                  UINT32          RemapScanCodeBuf[16],
                                  KEY_STATE_FLAGS RemapKeyStateFlagsBuf[16]
                                  );

int     KeybdDriverRemapVKeyUpEx(
                                UINT32          VirtualKey,
                                UINT32          ScanCode,
                                KEY_STATE_FLAGS KeyStateFlags,
                                UINT32          RemapVKeyBuf[16],
                                UINT32          RemapScanCodeBuf[16],
                                KEY_STATE_FLAGS RemapKeyStateFlagsBuf[16]
                                ) ;

void    KeybdDriverPowerHandler(BOOL bOff);

#endif // __KEYPAD_CTRL_H__.


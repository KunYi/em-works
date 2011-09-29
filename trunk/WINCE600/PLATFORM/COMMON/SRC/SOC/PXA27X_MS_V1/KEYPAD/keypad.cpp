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
/* Copyright 1999-2001 Intel Corporation. All Rights Reserved.
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


#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <bulverde.h>
#include <xllp_gpio.h>
#include "keypad.hpp"

//------------------------------------------------------------------------------
// Defines
//

//------------------------------------------------------------------------------
// Externs
//
extern BOOL KeyPadIstLoop(HANDLE hevIntrKeybd);

//------------------------------------------------------------------------------
// Global Variables
//
KeyPad *g_pKPC = NULL;

const DWORD gIntrKeyboard = SYSINTR_KEYPAD;

volatile BULVERDE_CLKMGR_REG *g_pClockRegs  = NULL;
volatile BULVERDE_GPIO_REG   *g_pGPIORegs   = NULL;
volatile BULVERDE_KEYPAD_REG *g_pKeyPadRegs = NULL;
volatile BULVERDE_OST_REG    *g_pOSTRegs    = NULL;

//------------------------------------------------------------------------------
// Local Variables
//

//------------------------------------------------------------------------------
// Local Functions
//

extern "C"
BOOL WINAPI DllMain(HANDLE hinstDll, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            g_pKPC = NULL;
            g_pClockRegs = NULL;
            g_pGPIORegs = NULL;
            g_pKeyPadRegs = NULL;
            DisableThreadLibraryCalls((HMODULE) hinstDll);
            break;
    }

    return TRUE;
}


BOOL KeyPad::KeyPadDataRead(UINT8 *pui8Data)
{
    BOOL bRet = FALSE;

    // Disable the keypad interrupt.
    XllpSetUpKeyPadInterrupts((XLLP_KEYPAD_REGS *)g_pKeyPadRegs, (XLLP_BOOL_T) DISABLE);

    // Read the keypad scan code.
    bRet = (XllpReadScanCode((XLLP_KEYPAD_REGS *)g_pKeyPadRegs, pui8Data) == XLLP_TRUE ? TRUE : FALSE);

    // Enable the keypad interrupt.
    XllpSetUpKeyPadInterrupts((XLLP_KEYPAD_REGS *)g_pKeyPadRegs, (XLLP_BOOL_T) ENABLE);

    return(bRet);
}


void FreeKeyPadRegs(void)
{
    if (g_pKeyPadRegs != NULL)
    {
        VirtualFree((void *)g_pKeyPadRegs, 0, MEM_RELEASE);
        g_pKeyPadRegs = NULL;
    }

    if (g_pGPIORegs != NULL)
    {
        VirtualFree((void *)g_pGPIORegs, 0, MEM_RELEASE);
        g_pGPIORegs = NULL;
    }

    if (g_pClockRegs != NULL)
    {
        VirtualFree((void *)g_pClockRegs, 0, MEM_RELEASE);
        g_pClockRegs = NULL;
    }

    if (g_pOSTRegs != NULL)
    {
        VirtualFree((void *)g_pOSTRegs, 0, MEM_RELEASE);
        g_pOSTRegs = NULL;
    }
}


BOOL AllocKeyPadRegs(void)
{
    PHYSICAL_ADDRESS PA;

    if (g_pKeyPadRegs == NULL)
    {
        PA.QuadPart = BULVERDE_BASE_REG_PA_KEYPAD;
        g_pKeyPadRegs = (volatile BULVERDE_KEYPAD_REG *) MmMapIoSpace(PA, 0x400, FALSE);
    }

    if (g_pGPIORegs == NULL)
    {
        PA.QuadPart = BULVERDE_BASE_REG_PA_GPIO;
        g_pGPIORegs = (volatile BULVERDE_GPIO_REG *) MmMapIoSpace(PA, 0x400, FALSE);
    }

    if (g_pClockRegs == NULL)
    {
        PA.QuadPart = BULVERDE_BASE_REG_PA_CLKMGR;
        g_pClockRegs = (volatile BULVERDE_CLKMGR_REG *) MmMapIoSpace(PA, 0x400, FALSE);
    }

    if (g_pOSTRegs == NULL)
    {
        PA.QuadPart = BULVERDE_BASE_REG_PA_OST;
        g_pOSTRegs = (volatile BULVERDE_OST_REG *) MmMapIoSpace(PA, 0x400, FALSE);
    }

    if (!g_pKeyPadRegs || !g_pGPIORegs || !g_pClockRegs || !g_pOSTRegs)
    {
        FreeKeyPadRegs();
        return(FALSE);
    }

    return(TRUE);

}


BOOL KeyPad::Initialize(void)
{
    BOOL bRet = FALSE;

    // Allocate and map keypad registers.
    if (AllocKeyPadRegs())
    {
        // Power-on the keypad controller.
        bRet = KeyPadPowerOn();
    }

    return(bRet);
}


BOOL KeyPad::KeyPadPowerOff(void)
{
    if (!g_pClockRegs) return(FALSE);

    g_pClockRegs->cken &= ~(0x1 << 19);

    return(TRUE);
}


BOOL KeyPad::KeyPadPowerOn(void)
{
    BOOL bRet = FALSE;

    if (g_pClockRegs && g_pKeyPadRegs && g_pGPIORegs)
    {
        g_pClockRegs->cken |= 0x1 << 19;

        if (g_pOSTRegs) XllpOstDelayMilliSeconds((P_XLLP_OST_T) g_pOSTRegs, 50);

        if (XllpKeyPadConfigure((XLLP_KEYPAD_REGS *)g_pKeyPadRegs, (XLLP_GPIO_T *)g_pGPIORegs))
        {
            if (XllpSetUpKeyPadInterrupts((XLLP_KEYPAD_REGS *)g_pKeyPadRegs,(XLLP_BOOL_T ) ENABLE))
            {
                bRet = TRUE;
            }
        }
    }

    return(bRet);
}


BOOL KeyPad::IsrThreadProc(void)
{

    m_hevInterrupt = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (m_hevInterrupt != NULL)
    {
        if (InterruptInitialize(SYSINTR_KEYPAD, m_hevInterrupt, NULL, 0))
        {
            KeyPadIstLoop(m_hevInterrupt);
        }
    }

    return(FALSE);
}


DWORD KeyPadIsrThread(KeyPad *pKP)
{
    pKP->IsrThreadProc();

    return(0);
}


BOOL KeyPad::IsrThreadStart(void)
{

   HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyPadIsrThread, this, 0, NULL);
   CloseHandle(hThread);

   return(TRUE);

}


// ----------------------------------------------------------------------------

void WINAPI KeypdPdd_PowerHandler(BOOL bOff)
{

    if(bOff)
    {
        g_pKPC->KeyPadPowerOff();
    }
    else
    {
        g_pKPC->KeyPadPowerOn();
    }

    return;
}


INT WINAPI KeypdPdd_GetEventEx(UINT32 VKeyBuf[16],
                               UINT32 ScanCodeBuf[16],
                               KEY_STATE_FLAGS KeyStateFlagsBuf[16])
{
    UINT8 ui8ScanCode;
    INT cEvents = 0;
    static KEY_STATE_FLAGS KeyStateFlags;

    // Read the keypad scan code.
    g_pKPC->KeyPadDataRead(&ui8ScanCode);

    // If the scan code is '0xff', it indicates that the key is 'up' - indicate to
    // the upper layers by the lack of a key down.  Otherwise, process the key 'down'.
    if(ui8ScanCode == 0xFF)
    {
        KeyStateFlagsBuf[0] = 0;
        cEvents = 1;
    }
    else
    {
        KeyStateFlags = KeyStateDownFlag;
        cEvents = ScanCodeToVKeyEx(ui8ScanCode, KeyStateFlags, VKeyBuf, ScanCodeBuf, KeyStateFlagsBuf);
    }

    return(cEvents);
}


VOID FreeResources()
{
    FreeKeyPadRegs();

    if (g_pKPC) {
        delete g_pKPC;
        g_pKPC = NULL;
    }
}


extern "C"
DWORD Init(DWORD dwContext)
{
    DWORD dwRet = 0;

    // Have we already allocated a keypad controller object?
    if (g_pKPC)
    {
        DEBUGMSG(TRUE, (TEXT("ERROR: bulverde_keypad: Already initialized.\r\n")));
        goto EXIT;
    }

    // Allocate a keypad controller object.
    g_pKPC = new KeyPad;
    if (!g_pKPC)
    {
        DEBUGMSG(TRUE, (TEXT("ERROR: bulverde_keypad: Failed to allocate keypad controller object (Error=0x%x).\r\n"), GetLastError()));
        goto EXIT;
    }

    // Initialize the keypad controller object.
    if (!g_pKPC->Initialize())
    {
        DEBUGMSG(TRUE, (TEXT("ERROR: bulverde_keypad: Failed to initialize keypad control object.\r\n")));
        goto EXIT;
    }

    // Create the keypad interrupt service thread (handles key presses).
    if (!g_pKPC->IsrThreadStart())
    {
        DEBUGMSG(TRUE, (TEXT("ERROR: bulverde_keypad: Failed to initialize keypad interrupt service thread.\r\n")));
        goto EXIT;
    }

    dwRet = (DWORD) g_pKPC;

EXIT:
    if (dwRet == 0) {
        FreeResources();
    }

    return dwRet;
}

extern "C"
BOOL Deinit(DWORD dwContext)
{
    ASSERT(dwContext == (DWORD) g_pKPC);
    FreeResources();
    return TRUE;
}

extern "C"
void PowerDown(DWORD dwContext)
{
    ASSERT(dwContext == (DWORD) g_pKPC);
    KeybdDriverPowerHandler(TRUE);
}

extern "C"
void PowerUp(DWORD dwContext)
{
    ASSERT(dwContext == (DWORD) g_pKPC);
    KeybdDriverPowerHandler(FALSE);
}


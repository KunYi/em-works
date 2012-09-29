//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//
//  File:  KeyPad.cpp
//
//  This module contains the main routines for the KeyPad class.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 6001 6385)
#include <windows.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 4100 4127 4189 6001 6385)
#include <creg.hxx>
#pragma warning(pop)


#include "keypad.h"
#include "hw_lradc.h"





#define HOLDKEY_LIMIT 0x10

static DWORD g_IsHoldkey=0;

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
HANDLE g_hLRADC = NULL;

//-----------------------------------------------------------------------------
// Local Variables

FSL_LRADC_CHANNEL CSP_KEYPAD::m_hwLRADC       = BUILD_KEYPAD_DEFAULT_LRADC();
// total keypad in the system by default
DWORD CSP_KEYPAD::m_dwValidKeys               = KEYPAD_DEFAULT_TOTAL;
DWORD CSP_KEYPAD::m_dwLastKeyIndex            = KEYPAD_INVALID_INDEX;
DWORD CSP_KEYPAD::m_dwCurKeyIndex             = KEYPAD_INVALID_INDEX;

// keypad scan control
DWORD CSP_KEYPAD::m_dwRepeatRate              = KEYPAD_DEFAULT_REPEAT_RATE;
DWORD CSP_KEYPAD::m_dwTicks                   = 0;
DWORD CSP_KEYPAD::m_dwTicksDebounce           = KEYPAD_DEFAULT_TICKS_DEBOUNCE;
DWORD CSP_KEYPAD::m_dwTicksRepeat             = KEYPAD_DEFAULT_TICKS_REPEAT;
DWORD CSP_KEYPAD::m_dwTicksBeforeRepeat       = KEYPAD_DEFAULT_TICKS_BEFORE_REPEAT;

DWORD CSP_KEYPAD::m_dwStateMachine            = KEYPAD_STATE_IDLE;
BOOL CSP_KEYPAD::m_fSendKeyDown               = FALSE;
HWND CSP_KEYPAD::mHWND_Test                   = NULL;

LRADC_CHANNEL  g_LradcCh;

//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//KeyPad class functions

//-----------------------------------------------------------------------------
//
//  Function: Init
//
//  This function initializes the Keypad class members.
//
//  Parameters:
//      dwContext
//          [IN] Pointer to a string containing the registry path
//               to the active key for the stream interface driver.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL BSP_KEYPAD::Init(DWORD dwContext)
{
    TCHAR lpDevName[] = LRADC_FID;
    BOOL bRetVal = FALSE;
    DWORD dwInx = 0, dwAppKeyPadCount = 0;

    DEBUGMSG(1,(TEXT("CSP_KEYPAD::Init ++\r\n")));
    
    m_Context = dwContext;

    g_LradcCh=(LRADC_CHANNEL)KeypadBspGetLradcCh();

    if(KeypadAlloc() == FALSE)
    {
        ERRORMSG(1, (TEXT("KeypadAlloc Failed\r\n")));
        goto KeyInitExit;          /* Error Mapping memory */
    }
    g_hLRADC = LRADCOpenHandle((LPCWSTR)lpDevName);
    if(!g_hLRADC)
    {
        ERRORMSG(1, (TEXT("LRADCOpenHandle INVALID HANDLE\r\n")));
        goto KeyInitExit;
    }
    m_hLRADC=g_hLRADC;
    KeypadBspInit();

    memset(&m_DataIST, 0, sizeof(m_DataIST));
    memset(&m_DataApp, 0, sizeof(m_DataApp));
    
    for(dwInx = 0, dwAppKeyPadCount = 1; dwInx != ARRAYSIZE(m_KeypadAttibute); dwInx++)
    {
        if(((m_KeypadAttibute[dwInx].dwFlag & KEYPAD_ATTR_APP) != 0) &&
           (m_KeypadAttibute[dwInx].szApplicationName != NULL))
        {
            m_DataApp.hEvent[dwAppKeyPadCount] = CreateEvent(NULL, FALSE, FALSE, NULL);
            m_DataApp.lpszApplicationName[dwAppKeyPadCount] = m_KeypadAttibute[dwInx].szApplicationName;
            m_KeypadAttibute[dwInx].hEvent = m_DataApp.hEvent[dwAppKeyPadCount];
            if(m_DataApp.hEvent[dwAppKeyPadCount] == NULL)
            {
                ERRORMSG(1, (TEXT("Create AppkeyEvent Fail\r\n")));
                goto KeyInitExit;
            }
            dwAppKeyPadCount++;
        }
    }
    
    m_DataApp.fTerminate = FALSE;
    if(dwAppKeyPadCount != 1)
    {
        m_DataApp.hEvent[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(m_DataApp.hEvent[0] == NULL)
        {
            ERRORMSG(1, (TEXT("Create hEvent_ApplicationKeyPad Fail\r\n")));
            goto KeyInitExit;
        }
        m_DataApp.dwMaxEvent = dwAppKeyPadCount;
        m_DataApp.hThread = CreateThread(NULL, 0, &CSP_KEYPAD::RelayThreadApp, this, 0, NULL);
        if(m_DataApp.hThread == NULL)
            goto KeyInitExit;
    }

    // Disable the channel interrupt
    LRADCEnableInterrupt(g_hLRADC,(LRADC_CHANNEL)g_LradcCh, FALSE);

    LRADCConfigureChannel(g_hLRADC,(LRADC_CHANNEL)g_LradcCh,                       //Lradc channel
                          TRUE,                                                    //DIVIDE_BY_TWO
                          FALSE,                                                    //ACCUMULATE
                          0);                                                       //NUM_SAMPLES

    m_DataIST.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(m_DataIST.hEvent == NULL)
    {
        ERRORMSG(1, (TEXT("Create m_DataIST.hEvent for IST fail\r\n")));
        goto KeyInitExit;
    }

    m_dwStateMachine = 0;
    m_dwTicks = 0;
    m_dwLastKeyIndex = KEYPAD_INVALID_INDEX;

    m_DataIST.fTerminate = FALSE;
    m_DataIST.hThread       = CreateThread( NULL,
                                            0,
                                            &CSP_KEYPAD::RelayThreadIST,
                                            this,
                                            0,
                                            NULL );
    if(m_DataIST.hThread == NULL)
        goto KeyInitExit;

    bRetVal = TRUE;

KeyInitExit:

    if(bRetVal == FALSE)
    {
        DeInit();
        ERRORMSG(1, (TEXT("ERROR: Init failed..\r\n")));
    }

    return bRetVal;
}

//-----------------------------------------------------------------------------
//
//  Function: GetRegistry
//
//  This function is used to get the data for keypad from registry.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSP_KEYPAD::DeInit(VOID)
{
    DEBUGMSG(1,(TEXT("CSP_KEYPAD::DeInit ++\r\n")));
    DEBUGMSG(1,(TEXT("CSP_KEYPAD::DeInit --\r\n")));
    return;
}


//-----------------------------------------------------------------------------
//
//  Function: KeypadAlloc
//
//  This function is used to map the registers. It is a stub function
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL CSP_KEYPAD::KeypadAlloc(VOID)
{
    BOOL bRetVal = TRUE;

    DEBUGMSG(1,(TEXT("KeypadAlloc ++\r\n")));
    //stub function need to check if required
    mHWND_Test = NULL;
    DEBUGMSG(1,(TEXT("KeypadAlloc --\r\n")));

    return bRetVal;
}

//-----------------------------------------------------------------------------
//
//  Function: KeypadDeAlloc
//
//  This function is used to Unmap the registers. It is a stub function
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
VOID CSP_KEYPAD::KeypadDeAlloc(VOID)
{
    DEBUGMSG(1,(TEXT("KeypadDeAlloc ++\r\n")));
    //stub function need to check if required
    
    DEBUGMSG(1,(TEXT("KeypadDeAlloc --\r\n")));

    return ;
}

//-----------------------------------------------------------------------------
//
//  Function: KeyPadAppThread
//  This function is used to opening the application depending
//      on the key pressed
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
DWORD CSP_KEYPAD::KeyPadAppThread(VOID)
{
    DWORD dwRetVal = 0;
    BOOL bValue = TRUE;
    DWORD dwObjectID;
    PROCESS_INFORMATION ProcInfo;
    STARTUPINFO StartUpInfo;

    memset( &StartUpInfo,0, sizeof(StartUpInfo) );
    StartUpInfo.cb = sizeof(StartUpInfo);

    DEBUGMSG(1,(TEXT("KeyPadAppThread ++\r\n")));

    do
    {
        if(m_DataApp.hEvent)
        {
            dwObjectID = WaitForMultipleObjects(m_DataApp.dwMaxEvent,
                                                m_DataApp.hEvent,
                                                FALSE,
                                                INFINITE);

            if((m_DataApp.fTerminate != FALSE) || (dwObjectID == 0))
                break;

            if(memcmp(m_DataApp.lpszApplicationName[dwObjectID] ,_T("mouse"),5)==0)
                {
                    mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
                    mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
                    continue;
                }

            if(CreateProcess(m_DataApp.lpszApplicationName[dwObjectID],
                             NULL,
                             NULL,
                             NULL,
                             FALSE,
                             CREATE_NEW_CONSOLE, NULL, NULL, &StartUpInfo, &ProcInfo))
            {
                DEBUGMSG(1, (TEXT("Create Process [%s] Successfully!\r\n"),
                              m_DataApp.lpszApplicationName[dwObjectID]));

                // Wait until child process exits.
                WaitForSingleObject( ProcInfo.hProcess, INFINITE );

                // Close process and thread handles.
                CloseHandle( ProcInfo.hProcess );
                CloseHandle( ProcInfo.hThread );
            }
            else
            {
                DEBUGMSG(1, (TEXT("Create Process [%s] fail!\r\n"),
                              m_DataApp.lpszApplicationName[dwObjectID]));
            }
        }
    } while(bValue);

    m_DataApp.fTerminate = TRUE;

    DEBUGMSG(1,(TEXT("KeyPadAppThread --\r\n")));

    return dwRetVal;
}

//-----------------------------------------------------------------------------
//
//  Function: KeyPadInterruptThread
//
//  This function is used to process the keypad interrupt and enable the
//      respective application interrupt
//
//  Parameters:
//      None.
//
//  Returns:
//
//
//-----------------------------------------------------------------------------
DWORD CSP_KEYPAD::KeyPadInterruptThread(VOID)
{
    DWORD dwRetVal = 0, dwSysIntr = 0, dwIrq ;
    UINT16 CurButton = 0;
    BOOL bValue = TRUE;
   
    dwIrq=KeypadBspGetIrq();
    
    DEBUGMSG(1,(TEXT("KeyPadInterruptThread ++ dwIrq:%d\r\n"),dwIrq));
 
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq, sizeof(UINT32), &dwSysIntr, sizeof(DWORD), NULL))
    {
        ERRORMSG(1, (TEXT("KernelIoControl  IOCTL_HAL_REQUEST_SYSINTR Error\r\n")));
        return 0;
    }
    
    // Initialize the Interrupt
    if(InterruptInitialize(dwSysIntr, m_DataIST.hEvent, NULL, NULL) == FALSE)
    {
        m_DataIST.fTerminate = TRUE;
        ERRORMSG(1, (TEXT("ThreadIST Cannot InterruptInitialize at ") \
                     TEXT("SysIntr=%u\r\n"), dwSysIntr));
        goto KPDThreadIST;
    }
    
    // Enable the interrupt
    InterruptDone(dwSysIntr);

    // Setup the keypad channel
    TriggerConversion(m_hwLRADC.dwCh_KeyPAD);

    // 1. clear interrupt status
    // 2. enable interrupt
    do {
        WaitForSingleObject(m_DataIST.hEvent, INFINITE);

        if(m_DataIST.fTerminate != FALSE)
            break;

        DEBUGMSG(1,(TEXT("KeyPadInterruptThread WaitForSingleObject Enter\r\n")));
         
        Sleep(10);
        
        UINT16 u16Vddio = LRADCGetAccumValue(g_hLRADC, VDDIO_VOLTAGE_CH);          // Get new Vddio value
        
        //TODO VDDIO_VOLTAGE_CH  is reuse by other device ,so we suppose the u16Vddio is not change
         if(IsSampleMatch(m_KeypadAttibute[0].dwVoltage, 0xfff) == TRUE)
            u16Vddio=0xfff;
         
         UINT16 u16Vbtn=LRADCGetAccumValue(g_hLRADC, g_LradcCh);

        //Normalize the ladder voltage with Vddio
        if(u16Vddio != 0)
        {
            CurButton = (u16Vbtn << 12)/u16Vddio;
        }
        
        m_dwCurKeyIndex = SearchKey(CurButton);
        
        DEBUGMSG(1,(TEXT("u16Vddio :%x  u16Vbtn:%x m_dwCurKeyIndex:%x  m_dwStateMachine :%x\r\n"),u16Vddio,u16Vbtn,m_dwCurKeyIndex,m_dwStateMachine));

        if(KEYPAD_INVALID_INDEX==m_dwCurKeyIndex)                
            KeypadBspClearIrq();  
        if(g_IsHoldkey>(HOLDKEY_LIMIT/2))
        {
             m_dwStateMachine = KEYPAD_STATE_IDLE; 
        }       
        
        switch(m_dwStateMachine)
        {
            case KEYPAD_STATE_IDLE:
                if(m_dwCurKeyIndex != KEYPAD_INVALID_INDEX)
                {
                    m_dwStateMachine = KEYPAD_STATE_DEBOUNCE;
                    m_dwLastKeyIndex = m_dwCurKeyIndex;
                    m_dwTicks        = m_dwTicksDebounce;
                    m_fSendKeyDown = FALSE;
                    if(mHWND_Test  != NULL)
                        PostMessage(mHWND_Test, WM_DEVICE_KEYPAD, WPARAM_KEYPAD_RELEASE, NULL);
                }

            break;

            case KEYPAD_STATE_DEBOUNCE:
                VerifyKeyAndExecute(KEYPAD_STATE_WAIT_RELEASED, 0);
            break;

            case KEYPAD_STATE_REPEAT_PENDING:
                VerifyKeyAndExecute(KEYPAD_STATE_REPEAT, m_dwTicksRepeat);
            break;

            case KEYPAD_STATE_REPEAT:
                VerifyKeyAndExecute(KEYPAD_STATE_REPEAT, m_dwTicksRepeat);
            break;

            case KEYPAD_STATE_WAIT_RELEASED:
                VerifyKeyAndExecute(KEYPAD_STATE_WAIT_RELEASED, 0);
            break;

            default:
                ERRORMSG(1, (TEXT("Unknown State for KeyScan=%u\r\n"), m_dwStateMachine));
                m_dwStateMachine = KEYPAD_STATE_IDLE;        
            break;
        };
        
        // Clear the interrupt flag of the given lradc channel
        LRADCClearInterruptFlag(g_hLRADC, VDDIO_VOLTAGE_CH);
      
        // Enable the interrupt
        InterruptDone(dwSysIntr);

    } while(bValue);

KPDThreadIST:
    m_DataIST.fTerminate = FALSE;
    DEBUGMSG(1,(TEXT("KeyPadInterruptThread --\r\n")));

    return dwRetVal;
}

//-----------------------------------------------------------------------------
//
//  Function: IOControl
//      This function supports IOControl codes from
//
//  Parameters:
//
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL CSP_KEYPAD::IOControl(DWORD Handle,
                           DWORD dwIoControlCode,
                           PBYTE pInBuf,
                           DWORD nInBufSize,
                           PBYTE pOutBuf,
                           DWORD nOutBufSize,
                           PDWORD pBytesReturned)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pBytesReturned);
    UNREFERENCED_PARAMETER(nOutBufSize);
    UNREFERENCED_PARAMETER(pOutBuf);
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(Handle);

    BOOL bRet = FALSE;

    switch (dwIoControlCode)
    {
    case IOCTL_KEYPAD_START_TEST:

        DEBUGMSG(1, (TEXT("IOControl::IOCTL_KEYPAD_START_TEST\r\n")));

        if(pInBuf != NULL)
        {
            mHWND_Test=(HWND) pInBuf;
            bRet = TRUE;
            DEBUGMSG(1, (TEXT("IOControl::IOCTL_KEYPAD_START_TEST[mHWND_Test=0x%08lx]\r\n"), mHWND_Test));
        }
        break;
    case  IOCTL_KEYPAD_STOP_TEST:

        DEBUGMSG(1, (TEXT("IOControl::IOCTL_KEYPAD_STOP_TEST\r\n")));
        mHWND_Test=NULL;

        bRet = TRUE;
        break;

    default:
        break;
    };
    return bRet;
}


//-----------------------------------------------------------------------------
//
//  Function: VerifyKeyAndExecute
//
//      This function verifies for a valid key press and sends a key event to GWES
//
//  Parameters:
//      dwNextState
//          [IN]    Indicates the next state of the Keypress
//      dwNextTicks
//          [IN]    defines the no. of ticks for debounce
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID CSP_KEYPAD::VerifyKeyAndExecute(DWORD dwNextState, DWORD dwNextTicks)
{
    DEBUGMSG(1,(TEXT("VerifyKeyAndExecute ++\r\n")));
    m_KeypadAttibute[m_dwLastKeyIndex].dwVitualKey;
    if(m_dwLastKeyIndex != m_dwCurKeyIndex)
    {
        DEBUGMSG(1, (TEXT("KeyUp, m_fSendKeyDown=%u, cur=0x%08lx, last=0x%08lx\r\n"),
                      m_fSendKeyDown, m_dwCurKeyIndex, m_dwLastKeyIndex));
        if(m_fSendKeyDown != FALSE)
        {
            if(mHWND_Test == NULL)
                keybd_event((BYTE)m_KeypadAttibute[m_dwLastKeyIndex].dwVitualKey,
                            (BYTE)MapVirtualKey(m_KeypadAttibute[m_dwLastKeyIndex].dwVitualKey, 0),
                            KEYEVENTF_KEYUP,
                            0);
            m_fSendKeyDown=FALSE;
            if(mHWND_Test != NULL)
                PostMessage(mHWND_Test, WM_DEVICE_KEYPAD, WPARAM_KEYPAD_RELEASE, NULL);
        }
        m_dwStateMachine = KEYPAD_STATE_IDLE;
        m_dwLastKeyIndex = KEYPAD_INVALID_INDEX;
    }
    else if((--m_dwTicks == 0) && (m_dwStateMachine != KEYPAD_STATE_WAIT_RELEASED))
    {
        if(m_KeypadAttibute[m_dwLastKeyIndex].dwFlag & KEYPAD_ATTR_APP)
        {
            if((mHWND_Test == NULL) &&  (m_KeypadAttibute[m_dwLastKeyIndex].hEvent != NULL))
                SetEvent(m_KeypadAttibute[m_dwLastKeyIndex].hEvent);
        }
       else 
        {
            if(mHWND_Test == NULL)
                keybd_event((BYTE) m_KeypadAttibute[m_dwLastKeyIndex].dwVitualKey,
                            (BYTE) MapVirtualKey(m_KeypadAttibute[m_dwLastKeyIndex].dwVitualKey, 0),
                            0,
                            0);
            m_fSendKeyDown=TRUE;
        }
        if(mHWND_Test != NULL)
            PostMessage(mHWND_Test, WM_DEVICE_KEYPAD, WPARAM_KEYPAD_PRESS, LPARAM_KEYPAD_PRESS(m_dwLastKeyIndex));
        DEBUGMSG(1, (TEXT("KeyUp, m_fSendKeyDown=%u, cur=0x%08lx, last=0x%08lx\r\n"),
                      m_fSendKeyDown, m_dwCurKeyIndex, m_dwLastKeyIndex));
        DEBUGMSG(1, (TEXT("Press %s\r\n"), m_KeypadAttibute[m_dwLastKeyIndex].szKeyName));
        if(m_dwStateMachine == KEYPAD_STATE_DEBOUNCE)
        {
            if((m_KeypadAttibute[m_dwLastKeyIndex].dwFlag & KEYPAD_ATTR_REPEAT) != 0)
            {
                m_dwStateMachine = KEYPAD_STATE_REPEAT_PENDING;
                m_dwTicks = m_dwTicksBeforeRepeat;
            }
        }
        else
        {
            m_dwStateMachine = dwNextState;
            m_dwTicks = dwNextTicks;
        }
    }
    DEBUGMSG(1,(TEXT("VerifyKeyAndExecute --\r\n")));

    return ;
}
//-----------------------------------------------------------------------------
//
//  Function: TriggerConversion
//
//      This function schedlules a conversion of the given channel
//
//  Parameters:
//      Channel
//          [IN]    Indicates the LRADC Channel no. for conversion
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID CSP_KEYPAD::TriggerConversion(DWORD Channel)
{
    LRADC_CHANNEL keypadChannel = (LRADC_CHANNEL)Channel;
    LRADC_CHANNEL VDDIOChannel  = (LRADC_CHANNEL)LRADC_CH_VDDIO;

    // Setup the trigger loop forever,
    LRADCSetDelayTrigger( g_hLRADC,
                          LRADC_DELAY_TRIGGER0,                                         // Trigger Index
                          (1 << keypadChannel),                 // Lradc channels
                          (1 << LRADC_DELAY_TRIGGER0),  // Restart the triggers
                          0,                                                    // No loop count
                          20);                                              // 0.5*N msec on 2khz

    // Clear the accumulator & NUM_SAMPLES
    LRADCClearAccum(g_hLRADC,keypadChannel);

    //Configure LRADC channel-6 for Vddio measurement
    LRADCConfigureChannel( g_hLRADC,
                           VDDIOChannel,                          //Lradc channel
                           FALSE,          //DIVIDE_BY_TWO (The channel-6 has a HW divide-by-two built in)
                           FALSE,          //ACCUMULATE
                            0);             //NUM_SAMPLES

    // Setup the trigger loop forever,
    LRADCSetDelayTrigger( g_hLRADC,LRADC_DELAY_TRIGGER0,         // Trigger Index
                          (1 << VDDIOChannel),      // Lradc channels
                          (1 << LRADC_DELAY_TRIGGER0), // Restart the triggers
                           0,                         // No loop count
                           20);                       // 0.5*N msec on 2khz

    //Enable LRADC interrupt of LRADC channel-6
    LRADCEnableInterrupt(g_hLRADC,VDDIOChannel, TRUE);

    //Kick off the delay trigger
    LRADCSetDelayTriggerKick(g_hLRADC,LRADC_DELAY_TRIGGER0, TRUE);
}
//-----------------------------------------------------------------------------
//
//  Function: SearchKey
//
//      This function searches for a valid key depending on the VDDIO sample
//
//  Parameters:
//      Sample
//          [IN]    Sample value for the pressed key
//
//  Returns:
//      KeyIndex of the sample depending on the sample value
//      else returns INVALID_KEY_INDEX
//
//-----------------------------------------------------------------------------
DWORD CSP_KEYPAD::SearchKey(DWORD Sample)
{
    DWORD KeyIndex;

     if(IsSampleMatch(m_KeypadAttibute[0].dwVoltage, Sample) != FALSE)
    {
       
        if(g_IsHoldkey<=HOLDKEY_LIMIT)
            g_IsHoldkey++;
        
        return KEYPAD_INVALID_INDEX;
    }

    if(Sample < m_hwLRADC.dwSampleDepressed)
    {
        for(KeyIndex=1; KeyIndex != m_dwValidKeys; KeyIndex++)
        {
            if(IsSampleMatch(m_KeypadAttibute[KeyIndex].dwVoltage, Sample) != FALSE)
            {
                DEBUGMSG(1, (TEXT("m_StateMachine =0x%08lx Sample Match KeyIndex =0x%08lx Sample=0x%08lx\r\n"),
                              m_dwStateMachine,KeyIndex,Sample));
                return KeyIndex;
            }
        }
    }
    
    if(g_IsHoldkey>0) 
        g_IsHoldkey--;
    
    return KEYPAD_INVALID_INDEX;
}

//-----------------------------------------------------------------------------
//
//  Function: IsSampleMatch
//
//      This function checks if the sample value is valid
//
//  Parameters:
//      Bottom
//          [IN]    Bottom value for the pressed key
//      Sample
//          [IN]    Sample value for the pressed key
//
//  Returns:
//      Retuns TRUE if success else returns FALSE
//
//-----------------------------------------------------------------------------
inline BOOL CSP_KEYPAD::IsSampleMatch(DWORD Bottom, DWORD Sample)
{
    DEBUGMSG(1, (TEXT("b=0x%08lx, s=0x%08lx, h=0x%08lx, t=0x%08lx\r\n"),
                  Bottom, Sample, m_hwLRADC.dwHysteresis, Bottom+m_hwLRADC.dwHysteresis));
    return ((Bottom <= Sample) && (((Bottom+m_hwLRADC.dwHysteresis) >= Sample))) ? TRUE : FALSE;
}

//-----------------------------------------------------------------------------
//
//  Function: KeypadBspInit
//
//      This function virtual function for keypad bsp init
//
//  Parameters:
//
//
//  Returns:
//           None
//
//-----------------------------------------------------------------------------
void CSP_KEYPAD::KeypadBspInit()
{

    return ;
}

//-----------------------------------------------------------------------------
//
//  Function: KeypadBspGetIrq
//
//      This function virtual function for get keypad irq 
//
//  Parameters:
//
//
//  Returns:
//           return irq
//
//-----------------------------------------------------------------------------
 DWORD CSP_KEYPAD::KeypadBspGetIrq(VOID)
{
 
    return 0;
}

//-----------------------------------------------------------------------------
//
//  Function: GetRegistry
//
//      This function virtual function for get registry setting
//
//  Parameters:
//
//
//  Returns:
//           Retuns TRUE if success else returns FALSE
//
//----------------------------------------------------------------------------- 
BOOL CSP_KEYPAD::GetRegistry(VOID)
{
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: KeypadBspGetLradcCh
//
//  This function virtual function for get Lradc ch
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
DWORD CSP_KEYPAD::KeypadBspGetLradcCh(VOID)
{
    return LRADC_CH0;
}

//-----------------------------------------------------------------------------
//
//  Function: KeypadBspClearIrq
//
//  This function virtual function for clear IRQ
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void  CSP_KEYPAD::KeypadBspClearIrq(VOID)
{
    return ;
}


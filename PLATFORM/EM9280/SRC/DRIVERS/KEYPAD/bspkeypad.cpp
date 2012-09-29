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

#include "bsp.h"
#include "keypad.h"
#include "hw_lradc.h"

#define SW04_VOLTAGE                    (0x000)                                                                 // vk_menu
#define SW05_VOLTAGE                    (0x156-(KEYPAD_DEFAULT_HYSTERESIS/2))   // vk_right
#define SW06_VOLTAGE                    (0x2AA-(KEYPAD_DEFAULT_HYSTERESIS/2))   // vk_win
#define SW07_VOLTAGE                    (0x3F4-(KEYPAD_DEFAULT_HYSTERESIS/2))   // vk_left
#define SW08_VOLTAGE                    (0x550-(KEYPAD_DEFAULT_HYSTERESIS/2))   // app1(file exp)
#define SW09_VOLTAGE                    (0x6B0-(KEYPAD_DEFAULT_HYSTERESIS/2))   // vk_up
#define SW10_VOLTAGE                    (0x80C-(KEYPAD_DEFAULT_HYSTERESIS/2))   // vk_esc
#define SW11_VOLTAGE                    (0x950-(KEYPAD_DEFAULT_HYSTERESIS/2))   // vk_down
#define SW12_VOLTAGE                    (0xA94-(KEYPAD_DEFAULT_HYSTERESIS/2))   // app2
#define SW13_VOLTAGE                    (0xbFF-(KEYPAD_DEFAULT_HYSTERESIS/2))   // vk_tab
#define SW14_VOLTAGE                    (0xd4A-(KEYPAD_DEFAULT_HYSTERESIS/2))   // vk_ret


#define BUILD_KEYPAD_DEFAULT_ATTRIBUTE_DEV()        { \
        { _T("ESC"),   _T(""),                            _T(""), NULL, SW10_VOLTAGE, KEYPAD_ATTR_REPEAT,  VK_ESCAPE       }, \
        { _T("RET"),   _T(""),                            _T(""), NULL, SW14_VOLTAGE, KEYPAD_ATTR_REPEAT,  VK_RETURN       }, \
        { _T("APP0"),  _T("explorer.exe"),                _T(""), NULL, SW08_VOLTAGE, KEYPAD_ATTR_APP,     0                       }, \
        { _T("RIGHT"), _T(""),                            _T(""), NULL, SW05_VOLTAGE, KEYPAD_ATTR_REPEAT,  VK_RIGHT        }, \
        { _T("APP1"),  _T(""),                            _T(""), NULL, SW12_VOLTAGE, KEYPAD_ATTR_APP,     0                       }, \
        { _T("TAB"),   _T(""),                            _T(""), NULL, SW13_VOLTAGE, KEYPAD_ATTR_REPEAT,  VK_TAB          }, \
        { _T("RWIN"),  _T(""),                            _T(""), NULL, SW06_VOLTAGE, KEYPAD_ATTR_REPEAT,  VK_RWIN         }, \
        { _T("LEFT"),  _T(""),                            _T(""), NULL, SW07_VOLTAGE, KEYPAD_ATTR_REPEAT,  VK_LEFT         }, \
        { _T("MLEFT"), _T("mouse"),                       _T(""), NULL, SW04_VOLTAGE, KEYPAD_ATTR_APP,     MOUSEEVENTF_LEFTUP }, \
        { _T("UP"),    _T(""),                            _T(""), NULL, SW09_VOLTAGE, KEYPAD_ATTR_REPEAT,  VK_UP           }, \
        { _T("DOWN"),  _T(""),                            _T(""), NULL, SW11_VOLTAGE, KEYPAD_ATTR_REPEAT,  VK_DOWN         } }


#define KEY1_VOLTAGE                      (0x0)                                                                 // vk_menu
#define RIGHT_VOLTAGE                     (0x13c-(KEYPAD_DEFAULT_HYSTERESIS/2))   
#define KEY2_VOLTAGE                      (0x2aa-(KEYPAD_DEFAULT_HYSTERESIS/2))  
#define LEFT_VOLTAGE                      (0x3a0-(KEYPAD_DEFAULT_HYSTERESIS/2))   
#define UP_VOLTAGE                        (0x62b-(KEYPAD_DEFAULT_HYSTERESIS/2))   
#define DOWN_VOLTAGE                      (0x890-(KEYPAD_DEFAULT_HYSTERESIS/2))   
#define KEY3_VOLTAGE                      (0x770-(KEYPAD_DEFAULT_HYSTERESIS/2))  
#define SELECT_VOLTAGE                    (0xb00-(KEYPAD_DEFAULT_HYSTERESIS/2))   
#define HOLD_VOLTAGE                      (0xfff-(KEYPAD_DEFAULT_HYSTERESIS/2)) 

#define BUILD_KEYPAD_DEFAULT_ATTRIBUTE_EVK()        { \
        { _T("HOLD"),   _T(""),                            _T(""), NULL, HOLD_VOLTAGE,     KEYPAD_ATTR_REPEAT,    VK_ESCAPE       }, \
        { _T("ESC"),   _T(""),                            _T(""), NULL,  KEY1_VOLTAGE,     KEYPAD_ATTR_REPEAT,    VK_ESCAPE       }, \
        { _T("LEFT"),  _T(""),                            _T(""), NULL,  LEFT_VOLTAGE,     KEYPAD_ATTR_REPEAT,    VK_LEFT         }, \
        { _T("WIN"),  _T(""),                             _T(""), NULL,  KEY2_VOLTAGE,     KEYPAD_ATTR_REPEAT,    VK_RWIN         }, \
        { _T("RIGHT"), _T(""),                            _T(""), NULL,  RIGHT_VOLTAGE,    KEYPAD_ATTR_REPEAT,    VK_RIGHT        }, \
        { _T("UP"),    _T(""),                            _T(""), NULL,  UP_VOLTAGE,       KEYPAD_ATTR_REPEAT,    VK_UP           }, \
        { _T("MENU"),  _T(""),                            _T(""), NULL,  KEY3_VOLTAGE,     KEYPAD_ATTR_REPEAT,    VK_MENU           }, \
        { _T("DOWN"),  _T(""),                            _T(""), NULL,  DOWN_VOLTAGE,     KEYPAD_ATTR_REPEAT,    VK_DOWN         }, \
        { _T("RET"),   _T(""),                            _T(""), NULL,  SELECT_VOLTAGE, KEYPAD_ATTR_REPEAT,  VK_RETURN         } }



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

DWORD g_dwBoardID = BOARDID_EVKBOARD;

//-----------------------------------------------------------------------------
// Local Variables

FSL_KEYPAD_ATTR   BSP_KEYPAD::m_KeypadAttibute_dev[] = BUILD_KEYPAD_DEFAULT_ATTRIBUTE_DEV();
FSL_KEYPAD_ATTR   BSP_KEYPAD::m_KeypadAttibute_evk[] = BUILD_KEYPAD_DEFAULT_ATTRIBUTE_EVK();


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//KeyPad class functions

//-----------------------------------------------------------------------------
//
//  Function: KeypadBspInit
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
void BSP_KEYPAD::KeypadBspInit()
{    
    if (!KernelIoControl(IOCTL_HAL_QUERY_BOARD_ID, NULL, 0, &g_dwBoardID, sizeof(g_dwBoardID), NULL))
    {
        ERRORMSG(1, (_T("KEYPAD Cannot obtain the board ID!\r\n")));
    }

    if(g_dwBoardID != BOARDID_EVKBOARD)
       memcpy(&m_KeypadAttibute, &m_KeypadAttibute_dev, sizeof(m_KeypadAttibute_dev));
    else
        memcpy(&m_KeypadAttibute, &m_KeypadAttibute_evk, sizeof(m_KeypadAttibute_evk));  

    // Enable  the Button Detect 
    LRADCEnableButtonDetect(m_hLRADC,LRADC_BUTTON1, TRUE);
    //  Enable  the Button Detect interrupt irq
    LRADCEnableButtonDetectInterrupt(m_hLRADC,LRADC_BUTTON1, TRUE);
  
    return ;
}

//-----------------------------------------------------------------------------
//
//  Function: KeypadBspGetIrq
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
DWORD BSP_KEYPAD::KeypadBspGetIrq(VOID)
{
    return IRQ_LRADC_BUTTON1;
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
BOOL BSP_KEYPAD::GetRegistry(VOID)
{
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: KeypadBspGetLradcCh
//
//  This function is used to get Lradc ch.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
DWORD BSP_KEYPAD::KeypadBspGetLradcCh(VOID)
{
    return LRADC_CH1;
}


//-----------------------------------------------------------------------------
//
//  Function: KeypadBspClearIrq
//
//  This function is used to clear thhe button irq
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void  BSP_KEYPAD::KeypadBspClearIrq(VOID)
{
    LRADCClearButtonDetect(m_hLRADC,LRADC_BUTTON1);
}
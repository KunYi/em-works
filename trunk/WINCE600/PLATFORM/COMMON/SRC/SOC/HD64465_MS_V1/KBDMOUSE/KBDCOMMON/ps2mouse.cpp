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

  Copyright(c) 1998,1999 Renesas Technology Corp.
  Copyright(c) 1998,1999 3RD Rail Engineering.

	Module Name:

		ps2mouse.cpp

	Revision History:

		26th April 1999		Released
		16th June  1999		Revised
*/

#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <pkfuncs.h>
//#include <oalintr.h>

#include "ps2p465.hpp"
#include "ps2mouse.hpp"
#include "ps2keybd.hpp"
#include "keybddbg.h"

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif

Ps2M465	*v_pp2m;
DWORD dwSysIntr_Mouse;

extern BOOL ReadRegDWORD( LPCWSTR szKeyName, LPCWSTR szValueName, LPDWORD pdwSysIntr );

BOOL
Ps2M465::
Initialize(
	Ps2P465	*pp2p
	)
{

	m_pp2p = pp2p;
	m_hevInterrupt = NULL;
	m_ui8ButtonState = 0;
    m_cReportFormatLength = pp2p -> bIntelliMouseFound() ? 4 : 3;
	return TRUE;
}



BOOL
Ps2M465::
IsrThreadProc(
              void
              )
{
	LPCTSTR szKey = TEXT("HARDWARE\\DEVICEMAP\\MOUSE");
	
    BOOL		bInPacket = FALSE;
    const int	cmsInPacketTimeout = 50;  // was 50 milliseconds
    int			cmsWaitTimeout;
    int			cBytes = 0;
    UINT8		ui8Data;
    UINT8		buf[4];
    long		x,y;
    
    UINT8	ui8Buttons;
    UINT8	ui8XorButtons;
    
    unsigned int	evfMouse;

	DWORD dwEnableWake;
    DWORD dwTransferred;
    
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    
    m_hevInterrupt = CreateEvent(NULL, FALSE, FALSE, NULL);
    if ( m_hevInterrupt == NULL)
    {
        goto leave;
    }
    
    ReadRegDWORD(szKey, _T("SysIntr"), &dwSysIntr_Mouse);
    if(! dwSysIntr_Mouse) {
        goto leave;
    }
    
    if ( !InterruptInitialize(dwSysIntr_Mouse, m_hevInterrupt, NULL, 0) )
    {
        goto leave;
    }

    if (ReadRegDWORD(szKey, _T("EnableWake"), &dwEnableWake)) {
    	if (dwEnableWake != 0) {
    		m_pp2p->SetWake(TRUE);
    	}
    }

    if (m_pp2p->WillWake()) {
    	// Ask the OAL to enable our interrupt to wake the system from suspend.
		DEBUGMSG(ZONE_INIT, (TEXT("Mouse: Enabling wake from suspend\r\n")));
		BOOL fErr = KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &dwSysIntr_Mouse, 
			sizeof(dwSysIntr_Mouse), NULL, 0, &dwTransferred);
    }
    
    v_pp2m -> m_pp2p -> MouseInterruptEnable();
    
    while (TRUE)
    {
        if ( bInPacket )
        {
            cmsWaitTimeout = cmsInPacketTimeout;
        }
        else
        {
            cmsWaitTimeout = INFINITE;
        }
        
        if ( WaitForSingleObject(m_hevInterrupt, cmsWaitTimeout) == WAIT_TIMEOUT )
        {
            
            DEBUGMSG(ZONE_MOUSEDATA, (TEXT("ps2mouse.cpp: Packet timeout cBytes= %d\r\n"), cBytes));
            
            cBytes = 0;
            bInPacket = FALSE;
            m_pp2p -> MouseDataRead( &ui8Data);
            m_pp2p -> MouseDataRead( &ui8Data);
            m_pp2p -> MouseDataRead( &ui8Data);
            m_pp2p -> MouseDataRead( &ui8Data);

            continue;
        }
        
        if ( v_pp2m -> m_pp2p -> MouseDataRead(&ui8Data) )
        {
            if ( cBytes < m_cReportFormatLength  )
            {
                
                if ( cBytes == 0 )
                {
                    // First byte never has bits 6,7 set, 
                    // but always has bit 3 set.
                    if ((ui8Data & 0xC8) != 0x08)
                        goto ByteRefused;
                    
                }

#pragma prefast(suppress:394, "The size check of cBytes avoids a buffer overrun")
                if(cBytes < sizeof(buf)) buf[cBytes++] = ui8Data;
                bInPacket = TRUE;
            }
            
            if ( cBytes == m_cReportFormatLength  )
            {
            	long	w = 0;
                evfMouse = 0;
                ui8XorButtons = (buf[0] ^ m_ui8ButtonState) & 0x03;
                //RETAILMSG(1, (TEXT("Mouse data %x %x %x %x\r\n"), buf[0], buf[1], buf[2],buf[3]));
                if ( ui8XorButtons )
                {
                    ui8Buttons = buf[0];
                    
                    if ( ui8XorButtons & 0x01 )
                    {
                        if ( ui8Buttons & 0x01 )
                        {
                            evfMouse |= MOUSEEVENTF_LEFTDOWN;
                        }
                        else
                        {
                            evfMouse |= MOUSEEVENTF_LEFTUP;
                        }
                    }
                    
                    if ( ui8XorButtons & 0x02 )
                    {
                        if ( ui8Buttons & 0x02 )
                        {
                            evfMouse |= MOUSEEVENTF_RIGHTDOWN;
                        }
                        else
                        {
                            evfMouse |= MOUSEEVENTF_RIGHTUP;
                        }
                    }
                    
                    if ( ui8XorButtons & 0x04 )
                    {
                        if ( ui8Buttons & 0x04 )
                        {
                            evfMouse |= MOUSEEVENTF_MIDDLEDOWN;
                        }
                        else
                        {
                            evfMouse |= MOUSEEVENTF_MIDDLEUP;
                        }
                    }
                    m_ui8ButtonState = buf[0];
                }
                
                x = buf[1];
                y = buf[2];
                
                if (buf[0] & 0x10)
                {
                    x |= 0xFFFFFF00;
                }
                else
                {
                    x &= 0x000000FF;
                }
                
                if (buf[0] & 0x20)
                {
                    y |= 0xFFFFFF00;
                }
                else
                {
                    y &= 0x000000FF;
                }
                
                y = -y;
                
                if ( y || x )
                {
                    evfMouse |= MOUSEEVENTF_MOVE;
                }
                // If IntelliMouse present buf[3] contains the relative displacement of the mouse wheel
                if( m_pp2p -> bIntelliMouseFound() && buf[3]) {
                    evfMouse |= MOUSEEVENTF_WHEEL;
                    // we need to scale by WHEEL_DELTA = 120
                    w = (-(char)buf[3]) * 120;
                }
                
                mouse_event(evfMouse, x, y, w, NULL);
                
                cBytes = 0;
                bInPacket = FALSE;
            }
        }
        else
        {
            ERRORMSG(1,(TEXT("Error reading mouse data\r\n")));
        }
ByteRefused :
        v_pp2m -> m_pp2p -> MouseInterruptEnable();
        InterruptDone(dwSysIntr_Mouse);
        
        }
        
leave:
        return TRUE;
}



DWORD
Ps2M465IsrThread(
	Ps2M465	*pp2m
	)
{

	pp2m -> IsrThreadProc();

	return 0;
}



BOOL
Ps2M465::
IsrThreadStart(
	void
	)
{
	HANDLE	hthrd;

	hthrd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Ps2M465IsrThread, this, 0, NULL);

	//	Since we don't need the handle, close it now.
	CloseHandle(hthrd);
	return TRUE;
}



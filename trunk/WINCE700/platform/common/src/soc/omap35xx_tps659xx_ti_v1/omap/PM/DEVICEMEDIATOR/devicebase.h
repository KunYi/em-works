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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File: DeviceBase.h
//

#if !defined(EA_9A47D44B_8DB2_473f_8DDC_3C7B40E2512D__INCLUDED_)
#define EA_9A47D44B_8DB2_473f_8DDC_3C7B40E2512D__INCLUDED_

#include <linklist.h>

//-----------------------------------------------------------------------------
//
// class: DeviceBase
//
// desc: Base class of all device driver classes.  Defines a set of common
//       abstract methods implemented by all device drivers.
//
//
class DeviceBase : public LIST_ENTRY
{
public:
    //-------------------------------------------------------------------------
    // typedefs
    //-------------------------------------------------------------------------

protected:
    //-------------------------------------------------------------------------
    // member variables
    //-------------------------------------------------------------------------

    HANDLE                                  m_hDevice;
    _TCHAR                                  m_szDeviceName[PNP_MAX_NAMELEN];
    
public:
    //-------------------------------------------------------------------------
    // constructor/destructor
    //-------------------------------------------------------------------------
    DeviceBase() : m_hDevice(NULL)
        {
        m_szDeviceName[0] = NULL;
        }
    
    virtual ~DeviceBase()
        {
        };

public:
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------    
    _TCHAR const* GetDeviceName() const     
        { 
        return m_szDeviceName; 
        }

    HANDLE GetDeviceHandle() const
        {
        return m_hDevice;
        }

public:
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------  
    BOOL operator==(LPCTSTR szDeviceName) const
        {
        if (m_szDeviceName == NULL || szDeviceName == NULL) return FALSE;
        return _tcsicmp(GetDeviceName(), szDeviceName) == 0;
        }
    
public:
    //-------------------------------------------------------------------------
    // virtual methods
    //------------------------------------------------------------------------- 

    virtual BOOL Uninitialize()   
        {
        return TRUE;
        }
    
    virtual BOOL Initialize(_TCHAR const* szDeviceName) 
        {
        if (_tcslen(szDeviceName) >= sizeof(m_szDeviceName) / sizeof(m_szDeviceName[0])) return FALSE;
        _tcscpy_s(m_szDeviceName,_countof(m_szDeviceName), szDeviceName);
        return TRUE;
        }
    
    virtual BOOL SendIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, 
                               DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, 
                               DWORD *lpBytesReturned
                               ) = 0;

};
#endif // !defined(EA_9A47D44B_8DB2_473f_8DDC_3C7                       

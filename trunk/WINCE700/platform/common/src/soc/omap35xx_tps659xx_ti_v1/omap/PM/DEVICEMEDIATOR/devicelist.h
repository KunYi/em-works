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
//  File: devicelist.h
//

#ifndef __DEVICELIST_H__
#define __DEVICELIST_H__

#include <linklist.h>
#include "contextlist.h"


//-----------------------------------------------------------------------------
//
// class: DeviceList
//
// desc: contains an array of asynchronous event handles
//
//
class DeviceList : public ContextList
{
public:
    //-------------------------------------------------------------------------
    // typedefs
    //-------------------------------------------------------------------------

protected:
    //-------------------------------------------------------------------------
    // member variables
    //-------------------------------------------------------------------------

    CRITICAL_SECTION                        m_cs;

public:
    //-------------------------------------------------------------------------
    // constructor/destructor
    //------------------------------------------------------------------------- 

    DeviceList()
        {
        m_idContextList = DEVICEMEDIATOR_DEVICE_LIST;
        }

protected:
    //-------------------------------------------------------------------------
    // protected methods
    //------------------------------------------------------------------------- 

    void Lock()
        {
        ::EnterCriticalSection(&m_cs);
        }

    void Unlock()
        {
        ::LeaveCriticalSection(&m_cs);
        }

public:
    //-------------------------------------------------------------------------
    // virtual methods for DeviceList
    //------------------------------------------------------------------------- 

    virtual BOOL Uninitialize();
    
    virtual BOOL Initialize();

    virtual BOOL InsertDevice(_TCHAR const* szDeviceName,
                              DeviceBase *pDevice,
                              HKEY hKey
                              );
    
    virtual BOOL RemoveDevice(_TCHAR const* szDeviceName,
                              DeviceBase *pDevice
                              );
    
    virtual BOOL SendIoControl(DWORD dwParam, DWORD dwIoControlCode, 
                               LPVOID lpInBuf, DWORD nInBufSize, 
                               LPVOID lpOutBuf, DWORD nOutBufSize, 
                               DWORD *lpBytesReturned
                               );    

};


#endif // !defined(__DEVICELIST_H__)
//-----------------------------------------------------------------------------

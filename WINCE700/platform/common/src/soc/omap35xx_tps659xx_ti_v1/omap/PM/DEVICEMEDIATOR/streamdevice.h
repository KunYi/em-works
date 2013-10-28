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
//  File: StreamDevice.h
//

#if !defined(EA_9836C521_23E9_4d86_BEDD_FA1F9B124189__INCLUDED_)
#define EA_9836C521_23E9_4d86_BEDD_FA1F9B124189__INCLUDED_

#include "DeviceBase.h"

#include <omap_pmext.h>

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

//-----------------------------------------------------------------------------
//
// class: StreamDevice
//
// desc: Wrapper class for stream drivers.
//
//
class StreamDevice : public DeviceBase
{
public:
    //-------------------------------------------------------------------------
    // typedefs
    //-------------------------------------------------------------------------
    enum {iid = STREAMDEVICE_CLASS}; 
    
public:
    //-------------------------------------------------------------------------
    // constructor/destructor
    //-------------------------------------------------------------------------
    StreamDevice() : DeviceBase()
        {
        }
    
public:
    //-------------------------------------------------------------------------
    // virtual methods
    //------------------------------------------------------------------------- 

    //------------------------------------------------------------------------- 
    virtual BOOL Uninitialize()   
        {
        DeviceBase::Uninitialize();
        if (m_hDevice != NULL) 
            {
            CloseHandle(m_hDevice);
            m_hDevice = NULL;
            }
        return TRUE;
        }

    //------------------------------------------------------------------------- 
    virtual BOOL Initialize(_TCHAR const* szDeviceName) 
        {
        DeviceBase::Initialize(szDeviceName);
        m_hDevice = ::CreateFile(szDeviceName, GENERIC_EXECUTE, 
                        DEVACCESS_PMEXT_MODE, NULL, 
                        OPEN_EXISTING, 0, NULL
                        );

        if (m_hDevice == INVALID_HANDLE_VALUE)
            {
            DeviceBase::Uninitialize();
            m_hDevice = NULL;
            }
        
        return m_hDevice != NULL;
        }

    //------------------------------------------------------------------------- 
    virtual BOOL SendIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, 
                                DWORD nInBufSize, LPVOID lpOutBuf, 
                                DWORD nOutBufSize, DWORD *lpBytesReturned
                                )
        {
        BOOL rc = FALSE;
        __try 
            {            
            rc = DeviceIoControl(m_hDevice, dwIoControlCode, lpInBuf, 
                    nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned, NULL
                    );
            }
        __except(EXCEPTION_EXECUTE_HANDLER) 
            {            
            rc = FALSE;
            }
        return rc;
        }

};


#endif // !defined(EA_9836C521_23E9_4d86_BEDD_FA1F9B124189__INCLUDED_)
//-----------------------------------------------------------------------------


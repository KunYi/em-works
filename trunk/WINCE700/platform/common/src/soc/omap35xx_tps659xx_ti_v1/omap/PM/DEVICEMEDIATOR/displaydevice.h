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
//  File: DisplayDevice.h
//

#if !defined(EA_FB261B9C_80E7_42e1_B0AE_D684443582D5__INCLUDED_)
#define EA_FB261B9C_80E7_42e1_B0AE_D684443582D5__INCLUDED_

#include "DeviceBase.h"

//-----------------------------------------------------------------------------
//
// class: DisplayDevice
//
// desc: Wrapper class for display drivers.
//
//
class DisplayDevice : public DeviceBase
{
public:
    //-------------------------------------------------------------------------
    // typedefs
    //-------------------------------------------------------------------------
    
    enum { iid = DISPLAYDEVICE_CLASS };
    
public:
    //-------------------------------------------------------------------------
    // constructor/destructor
    //-------------------------------------------------------------------------
    
    DisplayDevice() : DeviceBase()
        {
        }

public:
    //-------------------------------------------------------------------------
    // virtual methods
    //------------------------------------------------------------------------- 

    virtual BOOL Initialize(_TCHAR const* szDeviceName);
    virtual BOOL Uninitialize();
    virtual BOOL SendIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, 
                                DWORD nInBufSize, LPVOID lpOutBuf, 
                                DWORD nOutBufSize, DWORD *lpBytesReturned
                                );

};
#endif // !defined(EA_FB261B9C_80E7_42e1_B0AE_D684443582D5__INCLUDED_)
//-----------------------------------------------------------------------------
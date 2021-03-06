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
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

#pragma once
#include <BusDriver.hxx>
#include <bus.h>

//------------------------------------------------------------------------------

class DefaultBusContext_t : public BusDriverContext_t
{
    friend class DefaultBus_t;
    
protected:
    BusDriver_t* m_pBus;
    BusDriverChild_t *m_pChild;

protected:
    
    BusDriverChild_t* 
        FindChildByName(
        LPCWSTR childName
        );

protected:
    
    DefaultBusContext_t(
        BusDriver_t *pBus
        );

    virtual
    ~DefaultBusContext_t(
        );

public:
    //----------------------------------------------------------------------
    // BusDriverContext_t interface, see BusDriver.h for comments

    virtual
    BOOL
    Close(
        );

    virtual
    BOOL
    IOControl(
        DWORD code, 
        UCHAR *pInBuffer, 
        DWORD inSize, 
        UCHAR *pOutBuffer,
        DWORD outSize, 
        DWORD *pOutSize
        );
        
};

//------------------------------------------------------------------------------


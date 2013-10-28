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
//
//  File:  policyroot.h
//

#ifndef __POLICYROOT_H__
#define __POLICYROOT_H__

#include "policycontainer.h"

//------------------------------------------------------------------------------

class PolicyAdapter;
class PolicyContext;

//------------------------------------------------------------------------------
//
//  Class:  PolicyRoot
//
class PolicyRoot 
{
public:
    //-------------------------------------------------------------------------
    // typedefs
    //-------------------------------------------------------------------------
    
protected:
    //-------------------------------------------------------------------------
    // member variables
    //-------------------------------------------------------------------------
    
    _TCHAR                          m_szPolicyKey[MAX_PATH];
    PolicyContainer                 m_PolicyContainer;
    PolicyAdapter                  *m_pSystemChangeList;
    PolicyAdapter                  *m_pDeviceChangeList;

public:
    //-------------------------------------------------------------------------
    // constructor/destructor
    //-------------------------------------------------------------------------

    PolicyRoot() : m_PolicyContainer(), m_pSystemChangeList(NULL),
                   m_pDeviceChangeList(NULL)
    {
        *m_szPolicyKey = NULL;
        
    }

public:
    //-------------------------------------------------------------------------
    // inline public routines
    //-------------------------------------------------------------------------

    PolicyAdapter*
    FindPolicy(
        LPCWSTR szId
        )
    {
        return m_PolicyContainer.FindAdapterById(szId);
    }

    BOOL
    PostInitialize()
    {
        return m_PolicyContainer.PostInitialize(this);
    }

    void
    Uninitialize()
    {
        m_PolicyContainer.Uninitialize();
    }

    _TCHAR const*
    GetPolicyPath() const
    {
        return m_szPolicyKey;
    } 
    
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------

    BOOL
    Initialize(
        LPCWSTR context
        );

    void
    AddDeviceChangeObserver(
        PolicyAdapter   *pPolicy
        );

    void
    AddSystemChangeObserver(
        PolicyAdapter   *pPolicy
        );

    DWORD 
    PreSystemStateChange(
        DWORD dwExtContext, 
        LPCTSTR lpNewStateName, 
        DWORD dwFlags
        );

    DWORD 
    PostSystemStateChange(
        DWORD dwExtContext, 
        LPCTSTR lpNewStateName, 
        DWORD dwFlags
        );

    BOOL 
    PreDeviceStateChange(
        UINT dev, 
        UINT oldState, 
        UINT newState
        );
    
    BOOL 
    PostDeviceStateChange(
        UINT dev, 
        UINT oldState, 
        UINT newState
        );

    PolicyAdapter*
    LoadPolicy(
        LPCWSTR szPath
        );
};

//-----------------------------------------------------------------------------
#endif //__POLICYROOT_H__
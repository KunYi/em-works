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
//  File:  constraintroot.h
//

#ifndef __CONSTRAINTROOT_H__
#define __CONSTRAINTROOT_H__

#include "constraintcontainer.h"

//------------------------------------------------------------------------------

class ConstraintAdapter;
class ConstraintContext;

//------------------------------------------------------------------------------
//
//  Class:  ConstraintRoot
//
class ConstraintRoot 
{
public:
    //-------------------------------------------------------------------------
    // typedefs
    //-------------------------------------------------------------------------
    
protected:
    //-------------------------------------------------------------------------
    // member variables
    //-------------------------------------------------------------------------
    
    _TCHAR                          m_szConstraintKey[MAX_PATH];
    ConstraintContainer             m_ConstraintContainer;

public:
    //-------------------------------------------------------------------------
    // constructor/destructor
    //-------------------------------------------------------------------------

    ConstraintRoot() : m_ConstraintContainer()
    {
        *m_szConstraintKey = NULL;
    }

public:
    //-------------------------------------------------------------------------
    // inline public routines
    //-------------------------------------------------------------------------

    ConstraintAdapter*
    FindConstraintAdapterById(
        LPCWSTR szId
        )
    {
        return m_ConstraintContainer.FindAdapterById(szId);
    }

    ConstraintAdapter*
    FindConstraintAdapterByClass(
        DWORD classId
        )
    {
        return m_ConstraintContainer.FindAdapterByClass(classId);
    }

    BOOL
    PostInitialize()
    {
        return m_ConstraintContainer.PostInitialize();
    }

    void
    Uninitialize()
    {
        m_ConstraintContainer.Uninitialize();
    }
    
    _TCHAR const*
    GetConstraintPath() const
    {
        return m_szConstraintKey;
    }    
    
public:
    //-------------------------------------------------------------------------
    // public routines
    //-------------------------------------------------------------------------

    BOOL
    Initialize(
        LPCWSTR context
        );

    ConstraintAdapter*
    LoadConstraintAdapter(
        LPCWSTR szPath
        );
};

//-----------------------------------------------------------------------------
#endif //__CONSTRAINTROOT_H__
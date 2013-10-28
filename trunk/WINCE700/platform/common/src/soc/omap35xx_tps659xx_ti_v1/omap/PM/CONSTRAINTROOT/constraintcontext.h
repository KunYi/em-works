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
//  File:  constraintcontext.h
//

#ifndef __CONSTRAINTCONTEXT_H__
#define __CONSTRAINTCONTEXT_H__

class ConstraintAdapter;

//------------------------------------------------------------------------------
//
//  Class:  ConstraintContext
//
class ConstraintContext
{
public:
    //-------------------------------------------------------------------------
    // member variables
    //-------------------------------------------------------------------------

    DWORD               m_cookie;
    HANDLE              m_hContext;
    ConstraintAdapter  *m_pConstraintAdapter;

public:
    //-------------------------------------------------------------------------
    // constructor/destructor
    //-------------------------------------------------------------------------
    
    ConstraintContext(
        ConstraintAdapter  *pConstraintAdapter
        ) : m_pConstraintAdapter(pConstraintAdapter), m_hContext(NULL)
    {
        m_cookie = (DWORD)this;
    }   

    ~ConstraintContext()
    {
        m_cookie = 0;
    }
};

#endif //__CONSTRAINTCONTEXT_H__
//------------------------------------------------------------------------------
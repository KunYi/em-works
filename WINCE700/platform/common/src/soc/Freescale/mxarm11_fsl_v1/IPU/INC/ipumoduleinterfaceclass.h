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
//----------------------------------------------------------------------------- 
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  IpuModuleInterfaceClass.h
//
//  Class definitions for an IPU module base class
//
//-----------------------------------------------------------------------------

#ifndef __IPUMODULEINTERFACECLASS_H__
#define __IPUMODULEINTERFACECLASS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define P_IPU_REGS IpuModuleInterfaceClass::m_pIPU


//------------------------------------------------------------------------------
// Types
class IpuModuleInterfaceClass
{
    public:
        IpuModuleInterfaceClass();
        ~IpuModuleInterfaceClass();

    protected:
        // These variables are declared static so that 
        // inherited classes can share access to the
        // same variables
        static PCSP_IPU_REGS m_pIPU;
        static HANDLE m_hIpuMutex;
        CRITICAL_SECTION csLockAccess;

    private:
        BOOL IpuAlloc(void);
        void IpuDealloc(void);
};


//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif //__IPUMODULEINTERFACECLASS_H__

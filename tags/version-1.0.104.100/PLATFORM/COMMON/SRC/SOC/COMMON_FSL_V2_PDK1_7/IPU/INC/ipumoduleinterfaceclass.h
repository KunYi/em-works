//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

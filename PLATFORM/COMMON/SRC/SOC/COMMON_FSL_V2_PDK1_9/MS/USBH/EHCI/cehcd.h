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
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//------------------------------------------------------------------------------
// Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------
// 
// Module Name:  
//     cehcd.h
// 
// Abstract:  CUhcd implements the HCDI interface. It mostly
//            just passes requests on to other objects, which
//            do the real work.
//     
// Notes: 
//
//------------------------------------------------------------------------------

#ifndef __CEHCD_H_
#define __CEHCD_H_
class CEhcd;
#include "Chw.h"
#include <CDevice.hpp>
#include "CPipe.h"
#include "usb2lib.h"

// Remove-W4: Warning C4512 workaround
#pragma warning(push)
#pragma warning(disable: 4512)

// this class gets passed into the CUhcd Initialize routine
class CPhysMem;
// this class is our access point to all USB devices
//class CRootHub;

class CEhcd :public CHW, public  USB2lib
{
public:
    // ****************************************************
    // Public Functions for CUhcd
    // ****************************************************
    CEhcd( IN LPVOID pvUhcdPddObject,
                     IN CPhysMem * pCPhysMem,
                     IN LPCWSTR szDriverRegistryKey,
                     IN REGISTER portBase,
                     IN DWORD dwOffset,
                     IN DWORD dwSysIntr);

    ~CEhcd();

    // These functions are called by the HCDI interface
    virtual BOOL DeviceInitialize();
    virtual void DeviceDeInitialize( void );
   
    virtual BOOL    AddedTt( UCHAR uHubAddress,UCHAR uPort) { return USB2lib::AddedTt( uHubAddress,uPort); };
    virtual BOOL    DeleteTt( UCHAR uHubAddress,UCHAR uPort) { return USB2lib::DeleteTt( uHubAddress,uPort); };
    
    //public variables
    CPhysMem * const m_pMem;
private:
    // ****************************************************
    // Private Functions for CUhcd
    // ****************************************************
    DWORD           m_dwSysIntr;
    REGISTER        m_portBase;
};

#pragma warning(pop)

#endif

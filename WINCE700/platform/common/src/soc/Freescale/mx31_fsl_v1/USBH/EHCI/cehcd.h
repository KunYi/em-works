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
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
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



/*---------------------------------------------------------------------------
* Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/


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
#ifdef IRAM_PATCH
    /*
     * Changes
     * 1. the first parameter changed from * to **
     */
    CEhcd( IN LPVOID pvUhcdPddObject,
                     IN CPhysMem** pCPhysMem,
                     IN LPCWSTR szDriverRegistryKey,
                     IN REGISTER portBase,
                     IN DWORD dwSysIntr);
#else
    CEhcd( IN LPVOID pvUhcdPddObject,
                     IN CPhysMem * pCPhysMem,
                     IN LPCWSTR szDriverRegistryKey,
                     IN REGISTER portBase,
                     IN DWORD dwSysIntr);
#endif

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

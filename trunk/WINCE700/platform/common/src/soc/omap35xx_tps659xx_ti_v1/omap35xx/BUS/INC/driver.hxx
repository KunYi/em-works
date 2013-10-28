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
//------------------------------------------------------------------------------
// driver.hxx
//  Defines the base classes for driver and driver contexts
//
#ifndef _DRIVER_HXX_
#define _DRIVER_HXX_

//------------------------------------------------------------------------------
//  Forward class declarations

class RefCount_t;
class Driver_t;
class DriverContext_t;

//------------------------------------------------------------------------------
//
//  Class:  RefCount_t
//
//  This is basic class which allows implement object with reference count.
//  Most object used in driver++ enviroment should be derived from this 
//  class. 
//
class RefCount_t
{

private:
    LONG m_refCount;

protected:
    RefCount_t(
        ) {
        m_refCount = 1;
        };
    
    virtual ~RefCount_t(
        ) { };

public:    
    LONG AddRef(
        ) { 
        return InterlockedIncrement(&m_refCount);
        };

    LONG Release(
        ) {
        LONG count = InterlockedDecrement(&m_refCount);
        if (count <= 0) delete this;
        return count;
        };
};

//------------------------------------------------------------------------------

class Driver_t : public RefCount_t
{

public:

    // CreateDevice function is device manufacturer function. It produces
    // object of class Driver_t representing device. 
    static
    Driver_t* 
    CreateDevice(
        );


    //----------------------------------------------------------------------
    // Following virtual function reflect stream driver API. They are
    // called from C wrapper which implements stream driver interface.
    // Required functions are pure virual (so they must be implemented
    // in derived class), optional functions have empty implementation.

    // Called from XXX_Init
    virtual
    BOOL
    Init(
        LPCWSTR context, 
        LPCVOID pBusContext
        ) = 0;

    // Called from XXX_Deinit
    virtual
    BOOL
    Deinit(
        ) = 0;

    // Called from XXX_PreDeinit
    virtual
    VOID
    PreDeinit(
        ) { };

    // Called from XXX_PowerUp
    virtual
    VOID
    PowerUp(
        ) { };

    // Called from XXX_PowerDown    
    virtual
    VOID
    PowerDown(
        ) { };

    // Called from XXX_Open
    virtual
    DriverContext_t*
    Open(
        DWORD accessCode, 
        DWORD shareMode
        ) = 0;

    
    // Called from XXX_Close through DriverContext_t
    virtual
    BOOL
    Close(
        DriverContext_t* pContext
        ) = 0;

};

//------------------------------------------------------------------------------
//
//  Class:  DriverContext_t
//
class DriverContext_t : public RefCount_t
{
    friend class Driver_t;

public:
    
    //----------------------------------------------------------------------
    // Following virtual function reflect stream driver API. They are
    // called from C wrapper which implements stream driver interface.

    // Called from XXX_Read
    virtual
    DWORD
    Read(
        VOID *pBuffer,
        DWORD count
        ) {
        return -1;
        }

    // Called from XXX_Write
    virtual
    DWORD
    Write(
        VOID *pBuffer,
        DWORD count
        ) {
        return -1;
        }

    // Called from XXX_Seek
    virtual
    DWORD
    Seek(
        LONG amount,
        WORD type
        ) {
        return -1;
        }
    
    // Called from XXX_Close
    virtual
    BOOL
    Close(
        ) = 0;

    // Called from XXX_PreClose
    virtual
    BOOL
    PreClose(
        ) 
        {
        return TRUE;
        };
    
    // Called from XXX_IOControl
    virtual
    BOOL
    IOControl(
        DWORD code, 
        UCHAR *pInBuffer, 
        DWORD inSize, 
        UCHAR *pOutBuffer,
        DWORD outSize, 
        DWORD *pOutSize
        ) {
        return FALSE;
        };

};

//------------------------------------------------------------------------------

class DriverWithSingleContext_t : public RefCount_t
{

public:

    // CreateDevice function is device manufacturer function. It produces
    // object of class Driver_t representing device. 
    static
    Driver_t* 
    CreateDevice(
        );


    //----------------------------------------------------------------------
    // Following virtual function reflect stream driver API. They are
    // called from C wrapper which implements stream driver interface.
    // Required functions are pure virual (so they must be implemented
    // in derived class), optional functions have empty implementation.

    // Called from XXX_Init
    virtual
    BOOL
    Init(
        LPCWSTR context, 
        LPCVOID pBusContext
        ) = 0;

    // Called from XXX_Deinit
    virtual
    BOOL
    Deinit(
        ) = 0;

    // Called from XXX_PreDeinit
    virtual
    VOID
    PreDeinit(
        ) { };

    // Called from XXX_PowerUp
    virtual
    VOID
    PowerUp(
        ) { };

    // Called from XXX_PowerDown    
    virtual
    VOID
    PowerDown(
        ) { };

    // Called from XXX_Open
    virtual
    DriverWithSingleContext_t*
    Open(
        DWORD accessCode, 
        DWORD shareMode
        ) {
        return this;
        }

    // Called from XXX_Read
    virtual
    DWORD
    Read(
        VOID *pBuffer,
        DWORD count
        ) {
        return -1;
        }

    // Called from XXX_Write
    virtual
    DWORD
    Write(
        VOID *pBuffer,
        DWORD count
        ) {
        return -1;
        }

    // Called from XXX_Seek
    virtual
    DWORD
    Seek(
        LONG amount,
        WORD type
        ) {
        return -1;
        }
    
    // Called from XXX_Close
    virtual
    BOOL
    Close(
        ) {
        return TRUE;
        }

    // Called from XXX_PreClose
    virtual
    BOOL
    PreClose(
        ) {
        return TRUE;
        };
    
    // Called from XXX_IOControl
    virtual
    BOOL
    IOControl(
        DWORD code, 
        UCHAR *pInBuffer, 
        DWORD inSize, 
        UCHAR *pOutBuffer,
        DWORD outSize, 
        DWORD *pOutSize
        ) {
        return FALSE;
        };

};

//------------------------------------------------------------------------------

#endif //_DRIVER_HXX_


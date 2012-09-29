/*******************************************************************************
* Global Locate A-GPS chipset Application Programming Interface
*
* Copyright (c) 2001-2007 by Global Locate, Inc. All Rights Reserved.
*
* The information contained herein is confidential property of Global Locate. 
* The use, copying, transfer or disclosure of such information is prohibited 
* except by express written agreement with Global Locate.
*******************************************************************************/
#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#include <windows.h>

// modified ATL code, since the driver guys probably don't have that sitting around

namespace GlobalLocate
{

class CriticalSection
{
public:
    // Since we're not throwing exceptions, we need to have another means to know whether all is good
    // -> separate status function to check whether all is ok, we still do the resource acquisition in constructor
    CriticalSection():
        isCritSecInitialized(false)
    {
        ZeroMemory(&sec, sizeof(CRITICAL_SECTION));
    }

    ~CriticalSection()
    {
        if (isValid())
        {    DeleteCriticalSection(&sec);
        }
    }

    // Queries
    bool isValid() throw()
    {
        if (!isCritSecInitialized)
        {
            InitializeCriticalSection(&sec);
            isCritSecInitialized = true;
        }
        
        return isCritSecInitialized;
    }

    // Commands
    void lock() throw()
    {
        ASSERT(isValid());
        EnterCriticalSection(&sec);
    }
    
    void unlock() throw()
    {
        ASSERT(isValid());
        LeaveCriticalSection(&sec);
    }
    
private:
    CRITICAL_SECTION sec;
    bool isCritSecInitialized;
};

// slightly modified so, it doesn't use templates, but just the CriticalSection above
class CritSecLock
{
public:
    CritSecLock( CriticalSection& cs, bool bInitialLock = true):
        cs(cs),
        locked(false)
    {
        if(bInitialLock)
        {   lock();
        }
    }

    ~CritSecLock() throw()
    {
        if(locked)
        {   unlock();
        }
    }

    void lock() throw()
    {
        cs.lock();
        locked = true;
    }

    void unlock() throw()
    {
        cs.unlock();
        locked = false;
    }

private:
    // Private to avoid accidental use
    CritSecLock(const CritSecLock& ) throw();
    CritSecLock& operator=(const CritSecLock& ) throw();

    CriticalSection& cs;
    bool locked;
};

} // end namespace GlobalLocate

#endif // CRITICAL_SECTION_H

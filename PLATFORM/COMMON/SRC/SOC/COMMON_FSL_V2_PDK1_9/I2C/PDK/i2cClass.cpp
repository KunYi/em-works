//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  i2cclass.c
//
//  This file contains the main I2C protocol engine class.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4127 4201)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <linklist.h>
#include <windev.h>
#include <ceddk.h>
#include <stdlib.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_i2c.h"

#include "i2cbus.h"
#include "i2cclass.h"

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines 
//------------------------------------------------------------------------------
#define  I2C_TIMEOUT_INTERLEAVE     (1)
#define  I2C_TRANSMIT_WAIT          (3840*9)

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables 
//------------------------------------------------------------------------------
static const WORD wI2CClockRateDivider[] = {
    30,  32,  36,  42,  48,  52,  60,  72,   80,   88,  104,  128,  144,  160,  192,  240,
    288, 320, 384, 480, 576, 640, 768, 960, 1152, 1280, 1536, 1920, 2304, 2560, 3072, 3840,
    22,  24,  26,  28,  32,  36,  40,  44,   48,   56,   64,   72,   80,   96,  112,  128,
    160, 192, 224, 256, 320, 384, 448, 512,  640,  768,  896, 1024, 1280, 1536, 1792, 2048
};

#define I2CDIVTABSIZE   (sizeof(wI2CClockRateDivider)/sizeof(wI2CClockRateDivider[0]))
#define I2C_MAXDIVIDER   3840
#define I2C_MINDIVIDER   22

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function: I2CCalculateClkRateDiv
//
// This function will, on obtaining the frequency, determines the nearest clock
// rate divider needed to step the I2C Bus up/down. DO NOT CALL THIS FUNCTION
// directly. The recommended method is to use SDK function I2CSetFrequency().
//
// Parameters:
//      dwFrequency
//          [in] Contains the desired clock frequency of the slave device.
//
// Returns:
//      Returns an index to the array wI2CClockRateDivider. The content in the
//      index holds the nearest clock rate divider available.
//
//-----------------------------------------------------------------------------

WORD I2CClass::CalculateClkRateDiv(DWORD dwFrequency)
{
    INT iFirstRd, iSecondRd;
    WORD wEstDivider;
    DWORD freq;
    BYTE byCRDALen = I2CDIVTABSIZE;

    // Obtain an estimate of the divider required
    //DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER, &freq);
    BSPI2CGetModuleClock(m_iModuleIndex, &freq);
    wEstDivider = (WORD)(freq / dwFrequency);

    // Tolerance control, the look for frequency shall never exceed target frequency +1%
    if ((freq-(dwFrequency*wEstDivider))*100>dwFrequency)
    {
        ++wEstDivider;
    }

    // Search for the nearest divider in the first half of the array
    for (iFirstRd = 0; iFirstRd < (byCRDALen/2-1); iFirstRd++)
    {
        // Once found a divider greater than the estimate, stop
        if (wEstDivider <= wI2CClockRateDivider[iFirstRd])
            break;
    }
    if (wEstDivider == wI2CClockRateDivider[iFirstRd])
    {
        // If the estimated divider matched one of the array entries, no need
        // to search further
        wEstDivider = (WORD)iFirstRd;
    }
    else
    {
        // Going to second round
        for (iSecondRd = (byCRDALen/2); iSecondRd < (byCRDALen-1); iSecondRd++)
        {
            // Again, if a greater entry is found, stop
            if (wEstDivider <= wI2CClockRateDivider[iSecondRd])
                break;
        }
        if (wEstDivider == wI2CClockRateDivider[iSecondRd])
        {
            // If the estimated divider is found in the second round, stop
            wEstDivider = (WORD)iSecondRd;
        }
        else
        {
            // Search for the nearest divider among the 2 portion of the array
            if ((wI2CClockRateDivider[iFirstRd] > wEstDivider) && (wI2CClockRateDivider[iSecondRd] > wEstDivider))
            {
                if ((wI2CClockRateDivider[iFirstRd] - wEstDivider) < (wI2CClockRateDivider[iSecondRd] - wEstDivider))
                    wEstDivider = (WORD)iFirstRd;
                else
                    wEstDivider = (WORD)iSecondRd;
            }
            else
                if (wI2CClockRateDivider[iSecondRd] > wEstDivider)
                {
                    wEstDivider = (WORD)iSecondRd;
                }
                else
                {
                    // Less than setting, use wI2CClockRateDivider[31] as default
                    wEstDivider = (WORD)iFirstRd;
                }
        }
    }
    // Obtain the nearest clock rate divider
    return wEstDivider;
}

//-----------------------------------------------------------------------------
//
// Function: I2CClass::I2CClass
//
// This I2CClass constructor creates all the mutexes, events and heaps required
// for the subsequent use of I2C bus interface. It will also allocate the
// interrupt id from the HAL and binds the interrupt event to the interrupt id.
// This constructor has a built-in mechanism to prevent concurrent execution and a
// multiple interrupt binding procedure.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------

I2CClass::I2CClass(UINT32 index)
{
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass +\r\n")));

    m_iModuleIndex = index;

    // Mapped I2C Register Base Physical Address -> Virtual Address +
    {
        PHYSICAL_ADDRESS phyAddr;

        // Copy I2C physical address
        phyAddr.QuadPart = I2CGetBaseRegAddr(m_iModuleIndex);
        // Map to virtual address
        pI2CReg = (PCSP_I2C_REG) MmMapIoSpace(phyAddr, sizeof(CSP_I2C_REG), FALSE);

        // If mapping fails, fatal error, quit
        if (pI2CReg == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:MmMapIoSpace(): Failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = I2C_ERR_PA_VA_MISSING;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:MmMapIoSpace(): pI2CReg=0x%x \r\n"), pI2CReg));

    // Create Hardware Interrupt Occurrence Event
    {
        hInterrupted = CreateEvent(NULL, FALSE, FALSE, NULL);
        // Able to create or obtain the event?
        if (hInterrupted == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:CreateEvent(): Interrupt Occurrence Event (hardware) Failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = I2C_ERR_EOPS_CREATE;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:CreateEvent(): hInterrupted=0x%x \r\n"), hInterrupted));

    // Create Software Interrupt Event (to obviate priority inversion bugs)
    {
        // Create event in manual reset mode.
        m_hI2CIntrEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        // Able to create or obtain the event?
        if (m_hI2CIntrEvent == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:CreateEvent(): Interrupt Occurrence Event (software) Failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = I2C_ERR_EOPS_CREATE;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:CreateEvent(): m_hI2CIntrEvent=0x%x \r\n"), m_hI2CIntrEvent));

    // Create Software Update Event to notify applications that I2C slave is written.
    {
        // Create event in manual reset mode.
        switch( m_iModuleIndex )
        {
        case 1:
              m_hI2CUpdateSlaveEvent = CreateEvent(NULL, TRUE, FALSE, L"EVENT_I2C1SLAVE");
              break;
        case 2:
              m_hI2CUpdateSlaveEvent = CreateEvent(NULL, TRUE, FALSE, L"EVENT_I2C2SLAVE");
              break;
        case 3:
              m_hI2CUpdateSlaveEvent = CreateEvent(NULL, TRUE, FALSE, L"EVENT_I2C3SLAVE");
              break;
        default:
              m_hI2CUpdateSlaveEvent = NULL;
              break;
        }       
        // Able to create or obtain the event?
        if (m_hI2CUpdateSlaveEvent == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:CreateEvent(): Interrupt Occurrence Event (software) Failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = I2C_ERR_EOPS_CREATE;
            return;
        }
        bSlaveEventNotified = FALSE;
    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:CreateEvent(): m_hI2CIntrEvent=0x%x \r\n"), m_hI2CIntrEvent));

    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:InitializeCriticalSection(): Creating I2C_IOControl Critical Section! \r\n")));

    // Configure IOMUX for I2C pins
    {
        if (!BSPI2CIOMUXConfig(m_iModuleIndex))
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Error configuring IOMUX for I2C.\r\n"), __WFUNCTION__));
            return;
        }
    }

    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:InitializeCriticalSection(): Creating I2C Bus Critical Section! \r\n")));

    // I2C Bus Critical Section
    {
        InitializeCriticalSection(&gcsI2CBusLock);
        InitializeCriticalSection(&gcsI2CSlaveLock);
    }

    // Map IRQ -> System Interrupt ID
    {
        // Copy our I2C IRQ Number
        DWORD dwIrq = I2CGetIRQ(m_iModuleIndex);

        // Get kernel to translate IRQ -> System Interrupt ID
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq, sizeof(DWORD), &dwSysIntr, sizeof(DWORD), NULL))
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:KernelIoControl(): IRQ -> SysIntr Failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = I2C_ERR_IRQ_SYSINTR_MISSING;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:KernelIoControl(): dwSysIntr=0x%x \r\n"), dwSysIntr));

    // Link hInterrupted -> I2C Interrupt Pin
    {
        if (!InterruptInitialize(dwSysIntr, hInterrupted, NULL, 0))
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:Interruptinitialize(): Linking failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = I2C_ERR_INT_INIT;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:Interruptinitialize(): Linking passed! \r\n")));

    // Get BSP interrupt wait timeout value
    {
        iIntrWaitTimeout = BSPGetTimeoutValue();
        iIntrWaitTimeoutRetry = iIntrWaitTimeout/I2C_TIMEOUT_INTERLEAVE;
        iIntrWaitTimeout = I2C_TIMEOUT_INTERLEAVE;
    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:BSPGetTimeoutValue(): Timeout retrieved! \r\n")));


    // Create IST thread to receive hardware interrupts
    {
        // Initialize this to FALSE to allow the IST handler thread to start
        // up and run.
        bTerminateISTThread = FALSE;

        // Start I2C IST thread
        // Initialize thread for I2C interrupt handling
        //      pThreadAttributes = NULL (must be NULL)
        //      dwStackSize = 0 => default stack size determined by linker
        //      lpStartAddress = I2CIST => thread entry point
        //      lpParameter = this => point to thread parameter
        //      dwCreationFlags = 0 => no flags
        //      lpThreadId = NULL => thread ID is not returned
        PREFAST_SUPPRESS(5451, "This warning does not apply to WinCE because it is not a 64-bit OS");
        DWORD tid;
        m_hI2CIST = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)I2CIST, this, 0, &tid);//NULL);

        if (m_hI2CIST == NULL)
        {
            DEBUGMSG(ZONE_INIT,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
            iResult = I2C_ERR_INT_INIT;
            return;
        }
        else
        {
            DEBUGMSG(ZONE_INIT, (TEXT("%s: create I2C IST thread success\r\n"), __WFUNCTION__));
            DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: create I2C IST thread success, TID: %08x\r\n"), __WFUNCTION__, tid));
            CeSetThreadPriority(m_hI2CIST, 100);//THREAD_PRIORITY_TIME_CRITICAL);
        }

    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:BSPGetTimeoutValue(): Timeout retrieved! \r\n")));
    

    // Disable I2C Module Initially
    {
       OUTREG16(&pI2CReg->I2CR, 0);
    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:I2C_WORD_OUT(): I2CR=0x0 \r\n")));

    // Initialize I2C Mode
    byLastMode = I2C_MASTER_MODE;
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass: Default to Master Mode \r\n")));

    // Initialize I2C default address and clock.
    byLastSelfAddr= (BYTE)EXTREG16BF(&pI2CReg->IADR, I2C_IADR_ADR);
    wLastClkRate= EXTREG16BF(&pI2CReg->IFDR, I2C_IFDR_IC);
        
    // Default Mode is interrupt
    m_dxCurrent = D0;
    m_bUsePolling = TRUE;
    m_bDownGradeToPolling = FALSE;
    m_bDelayCleanupPendingEvent = FALSE;

    pSBuf =0;
    bSlaveInUse =0;

    // After this point, all initialization routines completed
    iResult = I2C_NO_ERROR;

    bDispatchEvent = FALSE;

    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass -\r\n")));

}

//-----------------------------------------------------------------------------
//
// Function: I2CClass::~I2CClass
//
// This I2CClass destructor releases all the mutexes, events and heaps created.
// It will also attempt to terminate the worker thread. It has built-in 
// mechanism to determine whether it is safe to unbind the interrupt event from
// the interrupt id. This is to facilitate situations where multiple
// processes have obtained the same file handle to the I2C Bus Interface.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
    
I2CClass::~I2CClass(void)
{
    DEBUGMSG(ZONE_DEINIT, (TEXT("I2CClass::~I2CClass +\r\n")));

    // Release the interrupt resources
    InterruptDisable(dwSysIntr);
    DEBUGMSG(ZONE_DEINIT, (TEXT("I2CClass::~I2CClass: Release System Interrupt \r\n")));

    // Kill interrupt service thread
    if (m_hI2CIST)
    {
        // Set the bTerminateISTThred flag to TRUE and then wake up the IST
        // thread to ensure that it will gracefully terminate (by simply
        // executing all of the "return" calls).
        bTerminateISTThread = TRUE;
        PulseEvent(hInterrupted);

        CloseHandle(m_hI2CIST);
        m_hI2CIST = NULL;
    }

    CloseHandle(hInterrupted);
    // Prevent the later sections from thinking it is still valid
    hInterrupted = NULL;

    CloseHandle(m_hI2CIntrEvent);
    // Prevent the later sections from thinking it is still valid
    m_hI2CIntrEvent = NULL;

    // Release the interrupt
    DEBUGMSG(ZONE_DEINIT, (TEXT("I2CClass::~I2CClass(): Releasing dwSysIntr = 0x%x \r\n"), dwSysIntr));
    if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &dwSysIntr, sizeof(DWORD), NULL, 0, NULL))
        DEBUGMSG(ZONE_DEINIT | ZONE_ERROR, (TEXT("ERROR: Failed to release dwSysIntr.\r\n")));                

    DEBUGMSG(ZONE_DEINIT, (TEXT("I2CClass::~I2CClass:DeleteCriticalSection(): Deleting I2C_IOControl Critical Section! \r\n")));

    // I2C Bus Critical Section
    {
        DeleteCriticalSection(&gcsI2CBusLock);
        DeleteCriticalSection(&gcsI2CSlaveLock);
    }

    // Release Interrupt Occurrence Event -
    {   
        if (hInterrupted != NULL)
            CloseHandle(hInterrupted);

        DEBUGMSG(ZONE_DEINIT, (TEXT("I2CClass::~I2CClass: Release Interrupt Occurence Event \r\n")));
    }

    // Release I2C Register Base Mapped Virtual Memory -
    {
        if (pI2CReg != NULL)
            MmUnmapIoSpace((LPVOID) pI2CReg, sizeof(CSP_I2C_REG));

        DEBUGMSG(ZONE_DEINIT, (TEXT("I2CClass::~I2CClass: Release I2C Register Base Mapped Virtual Memory \r\n")));
    }

    if(pSBuf != NULL) 
    {
        delete pSBuf;
    }
    DEBUGMSG(ZONE_DEINIT, (TEXT("I2CClass::~I2CClass -\r\n")));
}

//-----------------------------------------------------------------------------
//
// Function: I2CClass::Reset
//
// This method performs a software reset on I2C internal register (I2CR). It is
// important to take note that the fields of the I2CClass are not modified in
// any way.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------

VOID I2CClass::Reset(void)
{
    OUTREG16(&pI2CReg->I2CR, 0x0);
    iResult = I2C_NO_ERROR;
}

//-----------------------------------------------------------------------------
//
// Function: I2CClass::GenerateStart
//
// This method performs generates an I2C START signal to a slave device.
//
// Parameters:
//      pI2CPkt
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device
//          
// Returns:  
//      TRUE   Success
//      FALSE  Error or timeout.
//
//-----------------------------------------------------------------------------

BOOL I2CClass::GenerateStart(PI2C_PACKET pI2CPkt)
{
    UINT16 i2sr;
    BYTE bySlaveAddr;
    int intrRetry;

    int count;

    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF), 
                 I2C_I2SR_IIF_LSH) == I2C_I2SR_IIF_PENDING)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("IIF still pend in GenerateStart\r\n")));
    }

    count =0;
    while (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IBB),
                    I2C_I2SR_IBB_LSH) == I2C_I2SR_IBB_BUSY)
    {
        // Intentional polling loop.
        if (!m_bUsePolling)
        {
            Sleep(1);
        }
        ++count;
        if ( count >1000 )
        {
            DEBUGMSG(ZONE_FUNCTION, (_T("Generate Start can't be done\r\n")));
            (*(pI2CPkt->lpiResult)) = I2C_ERR_STATEMENT_CORRUPT;
            return FALSE;
        }
    }

    // Grant Bus Master
    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
             CSP_BITFVAL(I2C_I2CR_MSTA, I2C_I2CR_MSTA_MASTER));

    // Ensure the START operation is done
    count =0;
    while (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IBB),
                    I2C_I2SR_IBB_LSH) != I2C_I2SR_IBB_BUSY)
    {
        // Intentional polling loop.
        ++count;
        if ( count >1000 )
        {
            DEBUGMSG(ZONE_FUNCTION, (_T("Generate Start can't launch\r\n")));
            break;
        }
    }

    // Ensure the START operation is success
    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IAL),
                 I2C_I2SR_IAL_LSH) == I2C_I2SR_IAL_LOST)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("IAL detect in GenerateStart\r\n")));
        (*(pI2CPkt->lpiResult)) = I2C_ERR_ARBITRATION_LOST;
        goto error_cleanup;
    }

    // Ensure the START operation is success
    if (EXTREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
                 I2C_I2CR_MSTA_LSH) != I2C_I2CR_MSTA_MASTER)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("Master mode set failure\r\n")));
        (*(pI2CPkt->lpiResult)) = I2C_ERR_STATEMENT_CORRUPT;
        goto error_cleanup;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("Bus Master Granted!\r\n")));

    { 
        i2sr = INREG16(&pI2CReg->I2SR);
        OUTREG16(&pI2CReg->I2SR, i2sr);
    }

    OUTREG16(&pI2CReg->I2SR, 0);
    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF), 
                 I2C_I2SR_IIF_LSH) == I2C_I2SR_IIF_PENDING)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("IIF can not reset in GenerateStart\r\n")));
        (*(pI2CPkt->lpiResult)) = I2C_ERR_STATEMENT_CORRUPT;
        goto error_cleanup;
    }

    // Transmit the slave address, then change to receive mode after
    // we complete the address cycle.
    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MTX),
             CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT));

    bySlaveAddr = (pI2CPkt->byAddr << 1) |
                  (((pI2CPkt->byRW & I2C_METHOD_MASK) == I2C_RW_READ) ? 1 : 0);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("To I2DR->0x%x (slave addr)\r\n"),
                             bySlaveAddr));

    // Down grade interrupt method to polling when driver enters D4
    if (!m_bDownGradeToPolling && m_bUsePolling)
    {
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
                 CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
        m_bDownGradeToPolling=TRUE;
        DEBUGMSG(1, (_T("Down grade i2c to polling mode.(s)\r\n")));
    }

    if (m_bDownGradeToPolling)
    {
        OUTREG16(&pI2CReg->I2DR, bySlaveAddr);
        goto use_polling;
    }

    // Use interrupt method for transmision. 
    // And down grade interrupt method to polling when driver enters D4
    if (m_bDelayCleanupPendingEvent)
    {
        CleanupPendingInterrupt();
        m_bDelayCleanupPendingEvent = FALSE;
    }

    bDispatchEvent = TRUE;
    OUTREG16(&pI2CReg->I2DR, bySlaveAddr);

    // Wait for interrupt event
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("Waiting for incoming interrupt! \r\n")));

    intrRetry=iIntrWaitTimeoutRetry;
WaitRetry:
    if (m_bUsePolling)
    {
        // And down grade interrupt method to polling mode
        // when driver enters D4 during interrupt transmission.

        bDispatchEvent = FALSE;
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
                 CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
        m_bDownGradeToPolling=TRUE;
        m_bDelayCleanupPendingEvent = TRUE;
        DEBUGMSG(1, (_T("Down grade i2c to polling mode.(s1)\r\n")));
        goto use_polling;
    }

    if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
    {
        if (--intrRetry>0)
        {
            goto WaitRetry;
        }
        i2sr = INREG16(&pI2CReg->I2SR);
        if (CSP_BITFEXT(i2sr, I2C_I2SR_IIF) == I2C_I2SR_IIF_NOT_PENDING)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                                  TEXT("Timed out waiting for interrupt! ")
                                  TEXT("Aborting I2C transfer.\r\n")));

            (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
            goto error_cleanup;
        }
        else 
        {
            if (intrRetry == -10)
            {
                InterruptDone(dwSysIntr);
            }
            else if (intrRetry < -20)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                                      TEXT("Timed out waiting for interrupt! ")
                                      TEXT("Interrupt problem! ")
                                      TEXT("Aborting I2C transfer.\r\n")));

                (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
                goto error_cleanup;
            }
            goto WaitRetry;
        }
    }

    i2sr = INREG16(&pI2CReg->I2SR);

    bDispatchEvent = FALSE;
    OUTREG16(&pI2CReg->I2SR, 0);
    ResetEvent(m_hI2CIntrEvent);
    InterruptDone(dwSysIntr);
    goto IIF_launched;

use_polling:
    intrRetry=I2C_TRANSMIT_WAIT*(iIntrWaitTimeoutRetry);
    if( intrRetry< 1000 )
    {
        intrRetry=1000;
    }

    i2sr = INREG16(&pI2CReg->I2SR);
    while (CSP_BITFEXT(i2sr, I2C_I2SR_IIF) == I2C_I2SR_IIF_NOT_PENDING)
    {
        --intrRetry;
        if (intrRetry <=0)
        {
            (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
            goto error_cleanup;
        }
        i2sr = INREG16(&pI2CReg->I2SR);
    }

    OUTREG16(&pI2CReg->I2SR, 0);

    // IIF launched.
IIF_launched:
    if (CSP_BITFEXT(i2sr, I2C_I2SR_RXAK) == I2C_I2SR_RXAK_NO_ACK_DETECT)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::GenerateStart(): ")
                                 TEXT("No ACK, STOP issued!\r\n")));

        DEBUGMSG(ZONE_FUNCTION, (TEXT("Address cycle No ACK\r\n")));

        (*(pI2CPkt->lpiResult)) = I2C_ERR_NO_ACK_ISSUED;
        goto error_cleanup;
    }   
        
    return TRUE;

error_cleanup:

    bDispatchEvent = FALSE;
    OUTREG16(&pI2CReg->I2SR, 0);

    // Capture Error for debug
    if ( m_bUsePolling && CleanupPendingInterrupt() )
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("Warning: Event remain pending AFTER ADDRESS CYCLE done\r\n")));
    }

    // Send a STOP Signal
    if (EXTREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
                 I2C_I2CR_MSTA_LSH) == I2C_I2CR_MSTA_MASTER)
    {
        GenerateStop();
    }
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: I2CClass::GenerateRepeatedStart
//
// This method performs generates an I2C REPEATED START signal to a slave
// device.
//
// Parameters:
//      pI2CPkt
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device
//      
// Returns:  
//      TRUE   Success.
//      FALSE  Error or timeout.
//
//-----------------------------------------------------------------------------

BOOL I2CClass::GenerateRepeatedStart(PI2C_PACKET pI2CPkt, BOOL bRSTACycleComplete)
{
    BYTE bySlaveAddr;
    UINT16 i2sr;
    int intrRetry;
    UINT16 idx;
    WORD wDivider;
    int count;

    // Set the repeated start bit in the I2C CR.
    if (!bRSTACycleComplete)
    {
        if (m_bDownGradeToPolling)
        {
            idx = EXTREG16BF(&pI2CReg->IFDR, I2C_IFDR_IC);
            wDivider = (idx>=I2CDIVTABSIZE)? I2C_MAXDIVIDER :wI2CClockRateDivider[idx];
            // Delay for 2 SCL clock
            for(count=0; count<1; ++count)
            {
                INREG16(&pI2CReg->I2SR);
            }
        }

        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_RSTA),
                 CSP_BITFVAL(I2C_I2CR_RSTA, I2C_I2CR_RSTA_GENERATE));

        // Switch to Transmit Mode
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MTX), CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT));
    }

    if (m_bDownGradeToPolling)
    {
        idx = EXTREG16BF(&pI2CReg->IFDR, I2C_IFDR_IC);
        wDivider = 1;
        // Delay for 1 SCL clock
        for(count=0; count<wDivider; ++count)
        {
            INREG16(&pI2CReg->I2SR);
        }
    }
    // Temporary fix related to Repeated Start. Delay after repeated start
    // for 1 PAT_REF_CLK period.
    StallExecution(3);

    // Ensure the START operation is success
    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IAL),
                 I2C_I2SR_IAL_LSH) == I2C_I2SR_IAL_LOST)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("IAL detect in GenerateRepeatedStart\r\n")));
       (*(pI2CPkt->lpiResult)) = I2C_ERR_ARBITRATION_LOST;
        goto error_cleanup;
    }

    // Ensure the START operation is success
    if (EXTREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
                 I2C_I2CR_MSTA_LSH) != I2C_I2CR_MSTA_MASTER)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("RepeatedStart: Master mode set failure\r\n")));
       (*(pI2CPkt->lpiResult)) = I2C_ERR_STATEMENT_CORRUPT;
        goto error_cleanup;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("Bus Master Granted!\r\n")));

    { 
        i2sr = INREG16(&pI2CReg->I2SR);
        OUTREG16(&pI2CReg->I2SR, i2sr);
    }

    OUTREG16(&pI2CReg->I2SR, 0);
    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF), 
                 I2C_I2SR_IIF_LSH) == I2C_I2SR_IIF_PENDING)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("IIF can not reset in GenerateStart\r\n")));
       (*(pI2CPkt->lpiResult)) = I2C_ERR_STATEMENT_CORRUPT;
        goto error_cleanup;
    }

    // Append read or write bit to 7 bit slave address.
    bySlaveAddr = (pI2CPkt->byAddr << 1) |
                  (((pI2CPkt->byRW & I2C_METHOD_MASK) == I2C_RW_READ) ? 1 : 0);

    // Down grade interrupt method to polling when driver enters D4
    if (!m_bDownGradeToPolling && m_bUsePolling)
    {
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
                 CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
        m_bDownGradeToPolling=TRUE;
        DEBUGMSG(1, (_T("Down grade i2c to polling mode.(rs)\r\n")));
    }

    if (m_bDownGradeToPolling)
    {
        OUTREG16(&pI2CReg->I2DR, bySlaveAddr);
        goto use_polling;
    }

    // Use interrupt method for transmision. 
    // And down grade interrupt method to polling when driver enters D4
    if (m_bDelayCleanupPendingEvent)
    {
        CleanupPendingInterrupt();
        m_bDelayCleanupPendingEvent = FALSE;
    }

    OUTREG16(&pI2CReg->I2SR, 0);

    bDispatchEvent = TRUE;
    OUTREG16(&pI2CReg->I2DR, bySlaveAddr);

    // Wait for interrupt event
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("Waiting for incoming interrupt! \r\n")));

    intrRetry=iIntrWaitTimeoutRetry;
WaitRetry:
    if (m_bUsePolling)
    {
        // And down grade interrupt method to polling mode
        // when driver enters D4 during interrupt transmission.

        bDispatchEvent = FALSE;
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
                 CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
        m_bDownGradeToPolling=TRUE;
        m_bDelayCleanupPendingEvent = TRUE;
        DEBUGMSG(1, (_T("Down grade i2c to polling mode.(rs1)\r\n")));
        goto use_polling;
    }

    if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
    {
        if (--intrRetry>0)
        {
            goto WaitRetry;
        }
        i2sr = INREG16(&pI2CReg->I2SR);
        if (CSP_BITFEXT(i2sr, I2C_I2SR_IIF) == I2C_I2SR_IIF_NOT_PENDING)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                                  TEXT("Timed out waiting for interrupt! ")
                                  TEXT("Aborting I2C transfer.\r\n")));

            (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
            goto error_cleanup;
        }
        else 
        {
            if (intrRetry == -10)
            {
                InterruptDone(dwSysIntr);
            }
            else if (intrRetry < -20)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                                      TEXT("Timed out waiting for interrupt! ")
                                      TEXT("Interrupt problem! ")
                                      TEXT("Aborting I2C transfer.\r\n")));

                (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
                goto error_cleanup;
            }
            goto WaitRetry;
        }
    }

    i2sr = INREG16(&pI2CReg->I2SR);

    bDispatchEvent = FALSE;
    OUTREG16(&pI2CReg->I2SR, 0);
    ResetEvent(m_hI2CIntrEvent);
    InterruptDone(dwSysIntr);

    goto IIF_launched;

use_polling:
    intrRetry=I2C_TRANSMIT_WAIT*(iIntrWaitTimeoutRetry);
    if( intrRetry< 1000 )
    {
        intrRetry=1000;
    }

    i2sr = INREG16(&pI2CReg->I2SR);
    while (CSP_BITFEXT(i2sr, I2C_I2SR_IIF) == I2C_I2SR_IIF_NOT_PENDING)
    {
        --intrRetry;
        if (intrRetry <=0)
        {
            (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
            goto error_cleanup;
        }
        i2sr = INREG16(&pI2CReg->I2SR);
    }

    OUTREG16(&pI2CReg->I2SR, 0);

    // IIF launched.
IIF_launched:
    if (CSP_BITFEXT(i2sr, I2C_I2SR_RXAK) == I2C_I2SR_RXAK_NO_ACK_DETECT)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::GenerateStart(): ")
                                 TEXT("No ACK, STOP issued!\r\n")));

        (*(pI2CPkt->lpiResult)) = I2C_ERR_NO_ACK_ISSUED;
        goto error_cleanup;
    }   

    return TRUE;

error_cleanup:

    bDispatchEvent = FALSE;
    OUTREG16(&pI2CReg->I2SR, 0);

    // Capture Error for debug
    if ( m_bUsePolling && CleanupPendingInterrupt() )
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("Warning: Event remain pending AFTER ADDRESS CYCLE done\r\n")));
    }

    // Send a STOP Signal
    if (EXTREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
                 I2C_I2CR_MSTA_LSH) == I2C_I2CR_MSTA_MASTER)
    {
        GenerateStop();
    }
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: I2CClass::GenerateStop
//
// This method generates a stop bit on the I2C bus
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID I2CClass::GenerateStop()
{
    int count;
    UINT16 i2cr;
    UINT16 idx;
    WORD wDivider;

    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IBB),
                 I2C_I2SR_IBB_LSH) != I2C_I2SR_IBB_BUSY)
        return;

    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IAL),
                 I2C_I2SR_IAL_LSH) == I2C_I2SR_IAL_LOST)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("IAL detect in GenerateStop\r\n")));
    }

    if (EXTREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
                 I2C_I2CR_MSTA_LSH) == I2C_I2CR_MSTA_SLAVE)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("MSTA not set detect in GenerateStop\r\n")));
    }

    i2cr = INREG16(&pI2CReg->I2CR);
    if ((CSP_BITFEXT(i2cr, I2C_I2CR_MTX)==I2C_I2CR_MTX_RECEIVE)
        &&(CSP_BITFEXT(i2cr, I2C_I2CR_TXAK)==I2C_I2CR_TXAK_ACK_SEND))
    {
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_TXAK),
                 CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND));

        // Delay for 1 SCL clock
        for(count=0; count<(I2C_TRANSMIT_WAIT/9); ++count)
        {
            if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF),
                    I2C_I2SR_IIF_LSH) == I2C_I2SR_IIF_PENDING)
            {
                OUTREG16(&pI2CReg->I2SR, 0);
                break;
            }
        }
    }
    
    if (m_bDownGradeToPolling)
    {
        idx = EXTREG16BF(&pI2CReg->IFDR, I2C_IFDR_IC);
        wDivider = (idx>=I2CDIVTABSIZE)? I2C_MAXDIVIDER :wI2CClockRateDivider[idx];
        // Delay for 2 SCL clock
        for(count=0; count<2; ++count)
        {
            INREG16(&pI2CReg->I2SR);
        }
    }

    // Delay for 1 SCL clock
    wDivider=wI2CClockRateDivider[0];
    for(count=0; count<wDivider; ++count)
    {
        INREG16(&pI2CReg->I2SR);
    }

    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
             CSP_BITFVAL(I2C_I2CR_MSTA, I2C_I2CR_MSTA_SLAVE));

    // Wait for the stop condition to clear...
    count =0;
    while (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IBB),
                    I2C_I2SR_IBB_LSH) == I2C_I2SR_IBB_BUSY)
    {
        // Intentional polling loop.
        ++count;
        if( count >1000 ) {
            DEBUGMSG(ZONE_FUNCTION, (_T("GenerateStop not complete\r\n")));
            DEBUGMSG(ZONE_FUNCTION, (_T("SR=%02x, CR=%02x\r\n"),INREG16(&pI2CReg->I2SR),INREG16(&pI2CReg->I2CR)));
            break;
        }
    }

    if (m_bDownGradeToPolling)
    {
        idx = EXTREG16BF(&pI2CReg->IFDR, I2C_IFDR_IC);
        wDivider = (idx>=I2CDIVTABSIZE)? I2C_MAXDIVIDER :wI2CClockRateDivider[idx];
        // Delay for 1 SCL clock
        for(count=0; count<wDivider; ++count)
        {
            INREG16(&pI2CReg->I2SR);
        }
    }
}


//-----------------------------------------------------------------------------
//
// Function: I2CClass::WritePacket
//
// This method performs a write operation with the data in one I2C_PACKET.
//
// Parameters:
//      pI2CPkt
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device.
//
//      bLast
//          [in] Signifies if this is the last packet to be transmitted.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
BOOL I2CClass::WritePacket(PI2C_PACKET pI2CPkt, BOOL bLast)
{
    int intrRetry;
    UINT16 i2sr;

    PBYTE pWriteBufPtr = pI2CPkt->pbyBuf;

    // Set MTX to switch to transmit mode
    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MTX),
             CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT));

    (*(pI2CPkt->lpiResult)) = I2C_NO_ERROR;

    for (int i = 0; i < pI2CPkt->wLen; i++)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                 TEXT("Transmitting Next Byte: %x! \r\n"),
                                 *pWriteBufPtr));

        // Down grade interrupt method to polling when driver enters D4
        if (!m_bDownGradeToPolling && m_bUsePolling)
        {
            INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
                     CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
            m_bDownGradeToPolling=TRUE;
            DEBUGMSG(1, (_T("Down grade i2c to polling mode.(w)\r\n")));
        }

        if (m_bDownGradeToPolling)
        {
            OUTREG16(&pI2CReg->I2DR, (*pWriteBufPtr));
            pWriteBufPtr++;
            goto use_polling;
        }

        // Use interrupt method for transmision. 
        // And down grade interrupt method to polling when driver enters D4
        if (m_bDelayCleanupPendingEvent)
        {
            CleanupPendingInterrupt();
            m_bDelayCleanupPendingEvent = FALSE;
        }

        bDispatchEvent = TRUE;

        OUTREG16(&pI2CReg->I2DR, (*pWriteBufPtr));
        pWriteBufPtr++;

        // Wait for interrupt event
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                 TEXT("Waiting for incoming interrupt! \r\n")));

        intrRetry=iIntrWaitTimeoutRetry;
WaitRetry:
        if (m_bUsePolling)
        {
            // And down grade interrupt method to polling mode
            // when driver enters D4 during interrupt transmission.

            bDispatchEvent = FALSE;
            INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
                     CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
            m_bDownGradeToPolling=TRUE;
            m_bDelayCleanupPendingEvent = TRUE;
            DEBUGMSG(1, (_T("Down grade i2c to polling mode.(w1)\r\n")));
            goto use_polling;
        }

        if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
        {
            if (--intrRetry>0)
            {
                goto WaitRetry;
            }
            i2sr = INREG16(&pI2CReg->I2SR);
            if (CSP_BITFEXT(i2sr, I2C_I2SR_IIF) == I2C_I2SR_IIF_NOT_PENDING)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                                      TEXT("Timed out waiting for interrupt! ")
                                      TEXT("Aborting I2C transfer.\r\n")));

                (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
                goto error_cleanup;
            }
            else 
            {
                if (intrRetry == -10)
                {
                    InterruptDone(dwSysIntr);
                }
                else if (intrRetry < -20)
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                                          TEXT("Timed out waiting for interrupt! ")
                                          TEXT("Interrupt problem! ")
                                          TEXT("Aborting I2C transfer.\r\n")));

                    (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
                    goto error_cleanup;
                }
                goto WaitRetry;
            }
        }

        // Clear Interrupt Signal
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                 TEXT("Clear Interrupt! \r\n")));

        i2sr = INREG16(&pI2CReg->I2SR);

        bDispatchEvent = FALSE;
        INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF),
                 CSP_BITFVAL(I2C_I2SR_IIF, 0));
        ResetEvent(m_hI2CIntrEvent);
        InterruptDone(dwSysIntr);

        goto IIF_launched;

use_polling:
        intrRetry=I2C_TRANSMIT_WAIT*(iIntrWaitTimeoutRetry);
        if( intrRetry< 1000 )
        {
            intrRetry=1000;
        }

        i2sr = INREG16(&pI2CReg->I2SR);
        while (CSP_BITFEXT(i2sr, I2C_I2SR_IIF) == I2C_I2SR_IIF_NOT_PENDING)
        {
            --intrRetry;
            if (intrRetry <=0)
            {
                (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
                goto error_cleanup;
            }
            i2sr = INREG16(&pI2CReg->I2SR);
        }

        INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF),
                 CSP_BITFVAL(I2C_I2SR_IIF, 0));

        // IIF launched.
IIF_launched:
        if (EXTREG16BF(&pI2CReg->I2SR, I2C_I2SR_IAL))
        {
            // Arbitration lost.  An error has occurred, likely due to a bad
            // slave I2C address.

            // Clear IAL bit (we are already put into Stop)
            INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IAL),
                     CSP_BITFVAL(I2C_I2SR_IAL, I2C_I2SR_IAL_NOT_LOST));

            INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF),
                     CSP_BITFVAL(I2C_I2SR_IIF, 0));

           (*(pI2CPkt->lpiResult)) = I2C_ERR_ARBITRATION_LOST;
            goto error_cleanup;
        }

        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                 TEXT("Got Detect ACK? \r\n")));
        if ( (i<pI2CPkt->wLen -1) &&
            (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_RXAK),
                     I2C_I2SR_RXAK_LSH) == I2C_I2SR_RXAK_NO_ACK_DETECT))
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                     TEXT("No ACK, STOP issued! -\r\n")));

            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                     TEXT("No ACK, STOP issued! -\r\n")));

            (*(pI2CPkt->lpiResult)) = I2C_ERR_NO_ACK_ISSUED;
            goto error_cleanup;
        }   
            
    }
        
    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF), 
                 I2C_I2SR_IIF_LSH) == I2C_I2SR_IIF_PENDING)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("IIF dose not reset in WR.Pkt\r\n")));
    }

    if (bLast)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                 TEXT("Send STOP...this is the last packet ")
                                 TEXT("written. \r\n")));
        // Send STOP signal if this is the last packet to process
        OUTREG16(&pI2CReg->I2SR, 0);
        GenerateStop();
    }

    bDispatchEvent = FALSE;
    OUTREG16(&pI2CReg->I2SR, 0);

    return TRUE;

error_cleanup:

    bDispatchEvent = FALSE;
    OUTREG16(&pI2CReg->I2SR, 0);

    // Capture Error for debug
    if ( m_bUsePolling && CleanupPendingInterrupt() )
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("Warning: Event remain AFTER Write packet done\r\n")));
    }

    // Send a STOP Signal
    if (EXTREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
                 I2C_I2CR_MSTA_LSH) == I2C_I2CR_MSTA_MASTER)
    {
        GenerateStop();
    }
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: I2CClass::ReadPacket
//
// This method performs a read operation with the data in one I2C_PACKET.
//
// Parameters:
//      pI2CPkt
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device.
//
//      bLast
//          [in] Signifies if this is the last packet to be transmitted.
//
//      bAddrCycleComplete
//          [in] Signifies that the slave address cycle was just completed.
//
// Returns:  
//      TRUE   Success.
//      FALSE  Arbitration lost or timeout error.
//
//-----------------------------------------------------------------------------
BOOL I2CClass::ReadPacket(PI2C_PACKET pI2CPkt, BOOL bLast, BOOL bAddrCycleComplete, BOOL *pbRSTACycleComplete)
{
    UINT16 i2sr;
    UINT16 i2cr;
    int count;
    int intrRetry;
    UINT16 idx;
    WORD wDivider;

    PBYTE pReadBufPtr = pI2CPkt->pbyBuf;

    // Switch to Receive Mode
    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MTX), CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_RECEIVE));

    // Clear the TXAK bit to gen an ack when receiving only one byte.
    if (pI2CPkt->wLen == 1)
    {
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_TXAK), CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND));
    }
    else
    {
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_TXAK), CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_ACK_SEND));
    }

    if (bAddrCycleComplete)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Dummy read to trigger I2C Read operation.\r\n")));

        // Down grade interrupt method to polling when driver enters D4
        if (!m_bDownGradeToPolling && m_bUsePolling)
        {
            INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
                     CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
            m_bDownGradeToPolling=TRUE;
            DEBUGMSG(1, (_T("Down grade i2c to polling mode.(r)\r\n")));
        }

        if (m_bDownGradeToPolling)
        {
            bDispatchEvent = FALSE;
            //INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
            //         CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
        }
        else
        {
            // Use interrupt method for transmision. 
            // And down grade interrupt method to polling when driver enters D4
            if (m_bDelayCleanupPendingEvent)
            {
                CleanupPendingInterrupt();
                m_bDelayCleanupPendingEvent = FALSE;
            }

            bDispatchEvent = TRUE;
        }

        // Dummy read to trigger I2C Read operation
        INREG16(&pI2CReg->I2DR);
    }

    (*(pI2CPkt->lpiResult)) = I2C_NO_ERROR;

    for (int i = 0; i < pI2CPkt->wLen; i++)
    {
        // Wait for data transmission to complete.
        // Wait for interrupt event.
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Waiting for incoming interrupt! \r\n")));

        if (m_bDownGradeToPolling)
        {
            if (bDispatchEvent)
            {
                m_bDelayCleanupPendingEvent =TRUE;
                bDispatchEvent = FALSE;
            }
            goto use_polling;
        }

        intrRetry=iIntrWaitTimeoutRetry;
WaitRetry:
        if (m_bUsePolling)
        {
            // And down grade interrupt method to polling mode
            // when driver enters D4 during interrupt transmission.

            bDispatchEvent = FALSE;
            INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
                     CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
            m_bDownGradeToPolling=TRUE;
            m_bDelayCleanupPendingEvent = TRUE;
            DEBUGMSG(1, (_T("Down grade i2c to polling mode.(r1)\r\n")));
            goto use_polling;
        }

        if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
        {
            if (--intrRetry>0)
            {
                goto WaitRetry;
            }
            i2sr = INREG16(&pI2CReg->I2SR);
            if (CSP_BITFEXT(i2sr, I2C_I2SR_IIF) == I2C_I2SR_IIF_NOT_PENDING)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                                      TEXT("Timed out waiting for interrupt! ")
                                      TEXT("Aborting I2C transfer.\r\n")));

                DEBUGMSG(1, (TEXT("I2CClass::ProcessPacket(): ")
                                      TEXT("Timed out waiting for interrupt! ")
                                      TEXT("Aborting I2C transfer.\r\n")));

                (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
                goto error_cleanup;
            }
            else 
            {
                if (intrRetry == -10)
                {
                    DEBUGMSG(1, (TEXT("I2CClass::ProcessPacket(): ")
                                  TEXT("interrupt seems be masked, repaired.\r\n")));
                    InterruptDone(dwSysIntr);
                }
                else if (intrRetry < -20)
                {
                    DEBUGMSG(1, (TEXT("I2CClass::ProcessPacket(): ")
                                  TEXT("Timed out waiting for interrupt! ")
                                  TEXT("Interrupt problem! ")
                                  TEXT("Aborting I2C transfer.\r\n")));

                    (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
                    goto error_cleanup;
                }
                goto WaitRetry;
            }
        }

        i2sr = INREG16(&pI2CReg->I2SR);

        // Clear Interrupt Signal
        bDispatchEvent = FALSE;
        OUTREG16(&pI2CReg->I2SR, 0);
        ResetEvent(m_hI2CIntrEvent);
        InterruptDone(dwSysIntr);
        //DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Clear Interrupt! \r\n")));

        goto IIF_launched;

use_polling:
        intrRetry=I2C_TRANSMIT_WAIT*(iIntrWaitTimeoutRetry);
        if( intrRetry< 1000 )
        {
            intrRetry=1000;
        }

        i2sr = INREG16(&pI2CReg->I2SR);
        while (CSP_BITFEXT(i2sr, I2C_I2SR_IIF) == I2C_I2SR_IIF_NOT_PENDING)
        {
            --intrRetry;
            if (intrRetry <=0)
            {
                (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
                goto error_cleanup;
            }
            i2sr = INREG16(&pI2CReg->I2SR);
        }
        OUTREG16(&pI2CReg->I2SR, 0);

        // IIF launched.
IIF_launched:
        if (CSP_BITFEXT(i2sr, I2C_I2SR_IAL)==I2C_I2SR_IAL_LOST)
        {
            // Arbitration lost.  An error has occurred, likely due to a bad
            // slave I2C address. (we are already put into Stop)

            (*(pI2CPkt->lpiResult)) = I2C_ERR_ARBITRATION_LOST;
            goto error_cleanup;
        }


        // Do not generate an ACK for the last byte 
        if (i == (pI2CPkt->wLen - 2))
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Change to No ACK for last byte. \r\n")));
            INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_TXAK), CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND));
        }
        else if (i == (pI2CPkt->wLen - 1))
        {
            if (bLast)
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Send STOP...this is the last packet read. \r\n")));

                // Send STOP signal if this is the last packet to process
                GenerateStop();
                goto last_read_i2dr;
            }
            else 
            {
                if (m_bDownGradeToPolling)
                {
                    idx = EXTREG16BF(&pI2CReg->IFDR, I2C_IFDR_IC);
                    wDivider = (idx>=I2CDIVTABSIZE)? I2C_MAXDIVIDER :wI2CClockRateDivider[idx];
                    // Delay for 1 SCL clock
                    for(count=0; count<wDivider*2; ++count)
                    {
                        INREG16(&pI2CReg->I2SR);
                    }
                }
                // Send Repeated Start signal to initial next packet
                INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_RSTA),
                         CSP_BITFVAL(I2C_I2CR_RSTA, I2C_I2CR_RSTA_GENERATE));
                if ( pbRSTACycleComplete )
                {
                    *pbRSTACycleComplete = TRUE;
                }
                // Switch to Transmit Mode
                INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MTX), CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT));
                goto last_read_i2dr;
            }
        }

        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Read next byte! \r\n")));

        // Down grade interrupt method to polling when driver enters D4
        if (!m_bDownGradeToPolling && m_bUsePolling)
        {
            INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN),
                     CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
            m_bDownGradeToPolling=TRUE;
        }

        if (!m_bDownGradeToPolling)
        {
            bDispatchEvent = TRUE;
        }

        // Use interrupt method for transmision. 
        // And down grade interrupt method to polling when driver enters D4
        if (m_bDelayCleanupPendingEvent)
        {
            CleanupPendingInterrupt();
            m_bDelayCleanupPendingEvent = FALSE;
        }

last_read_i2dr:
        (*pReadBufPtr) = (BYTE) INREG16(&pI2CReg->I2DR);
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Byte read: %x \r\n"), *pReadBufPtr));
        ++pReadBufPtr;
    }

    OUTREG16(&pI2CReg->I2SR, 0);
    bDispatchEvent = FALSE;

    return TRUE;

error_cleanup:

    bDispatchEvent = FALSE;

    i2cr = INREG16(&pI2CReg->I2CR);
    if ((CSP_BITFEXT(i2cr, I2C_I2CR_MTX)==I2C_I2CR_MTX_RECEIVE)
        &&(CSP_BITFEXT(i2cr, I2C_I2CR_TXAK)==I2C_I2CR_TXAK_ACK_SEND))
    {
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_TXAK),
                 CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND));

        // Delay for 1 SCL clock
        for(count=0; count<(I2C_TRANSMIT_WAIT/9); ++count)
        {
            INREG16(&pI2CReg->I2SR);
        }
    }
    OUTREG16(&pI2CReg->I2SR, 0);

    // Capture Error for debug
    if ( m_bUsePolling && CleanupPendingInterrupt() )
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("Warning: Event remain pending AFTER read packet done\r\n")));
    }

    // Send a STOP Signal
    if (EXTREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
                 I2C_I2CR_MSTA_LSH) == I2C_I2CR_MSTA_MASTER)
    {
        GenerateStop();
    }

    // Release i2c bus 
    INREG16(&pI2CReg->I2DR);
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CClass::CleanupPendingInterrupt
//
// Shall not Interrupt pending before ProcessPackets does it work.
// This function ensure this condition.
//
// Parameters:
//      None. 
//
// Returns:  
//      TRUE:   Clean pending interrupt
//      FALSE:  No interrupt pending
//
//-----------------------------------------------------------------------------
BOOL I2CClass::CleanupPendingInterrupt(void)
{
    BOOL bRet;
    int count;

    bRet = FALSE;
    count=0;
    while (WaitForSingleObject(m_hI2CIntrEvent, 0) == WAIT_OBJECT_0)
    {
        ++count;
        OUTREG16(&pI2CReg->I2SR, 0);
        // Reset the event manually
        ResetEvent(m_hI2CIntrEvent);
        InterruptDone(dwSysIntr);
        bRet = TRUE;
    }
    m_bDelayCleanupPendingEvent=FALSE;
    return bRet;
}
//-----------------------------------------------------------------------------
//
// Function: I2CClass::ProcessPackets
//
// This is the main engine that transmits or receives data from I2C Bus
// Interface. This engine implements the complete I2C Bus Protocol which allows
// the calling process to interact with all I2C-compliant slave device. This
// method has built-in mechanism to prevent concurrent execution.
//
// Parameters:
//      pPacket 
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device.
//
//      dwNumPackets
//          [in] Number of packets to be processed.
//
// Returns:  
//      TRUE   Success.
//      FALSE  Failed due to any one of the following conditions:
//               - failed to enable the I2C clock
//               - GenerateStart() call returned an error
//               - timed out waiting for interrupt notification
//               - arbitration lost
//               - transfer operation timed out
//
//-----------------------------------------------------------------------------
BOOL I2CClass::ProcessPackets(I2C_PACKET packets[], INT32 numPackets)
{
    BOOL retVal = TRUE;
    // Flag to signal if address cycle just completed
    BOOL bAddrCycleComplete;
    BOOL bRSTACycleComplete;

    // Must gain ownership to bus lock mutex
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Waiting for Bus Lock CS \r\n")));
    EnterCriticalSection(&gcsI2CBusLock);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Acquired Bus Lock CS! \r\n")));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket():BSPI2CEnableClock(): Enabling I2C Clock! \r\n")));

    if (!bSlaveInUse)
    {
        // Enabling I2C Clock
        if (!BSPI2CEnableClock(m_iModuleIndex, TRUE))
        {
            DEBUGMSG(ZONE_FUNCTION | ZONE_ERROR, (TEXT("I2CClass::ProcessPacket():BSPI2CEnableClock(): I2C Clock cannot be enabled! \r\n")));
            retVal = FALSE;
            goto __exit;
        }
    }
    else 
    {
        UINT16 i2sr;
        UINT16 retry;
        i2sr = INREG16(&pI2CReg->I2SR);
        retry = 1000;
        while ( /*(CSP_BITFEXT(i2sr, I2C_I2SR_ICF)==I2C_I2SR_ICF_IN_PROGRESS)
                ||*/(CSP_BITFEXT(i2sr, I2C_I2SR_IBB) == I2C_I2SR_IBB_BUSY) )
        {
            // Intentional polling loop.
            if (!m_bUsePolling)
            {
                Sleep(0);
            }
            --retry;
            if (retry==0)
            {
                DEBUGMSG(ZONE_FUNCTION, (_T("I2CClass::ProcessPacket():I2C Bus is not ready for transmitting packets\r\n")));
                retVal = FALSE;
                goto __exit;
            }

            i2sr = INREG16(&pI2CReg->I2SR);
        }
        byInUseSlaveAddr = (BYTE)INREG16(&pI2CReg->IADR);
    }


    // Reset I2CR
    OUTREG16(&pI2CReg->I2CR, 0x0);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Resetting I2CR \r\n")));

    // Try resetting I2DR = 0x0
    //OUTREG16(&pI2CReg->I2DR, 0x0);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Resetting I2DR \r\n")));

    // Configure data sampling rate
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Trying to set IFDR->0x%x! \r\n"), wLastClkRate));
    OUTREG16(&pI2CReg->IFDR, CSP_BITFVAL(I2C_IFDR_IC, wLastClkRate));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Set IFDR->0x%x! \r\n"), INREG16(&pI2CReg->IFDR)));

    // Configure slave address
    OUTREG16(&pI2CReg->IADR, CSP_BITFVAL(I2C_IADR_ADR, byLastSelfAddr));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Configure Self Address->0x%x \r\n"), INREG16(&pI2CReg->IADR)));

    // Enable I2C
    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IEN), CSP_BITFVAL(I2C_I2CR_IEN, I2C_I2CR_IEN_ENABLE));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Enable I2C \r\n")));

    if (!m_bUsePolling)
    {
        m_bDownGradeToPolling=FALSE;

        // Enable I2C Interrupt
        INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN), CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_ENABLE));
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Enable I2C Interrupt \r\n")));
    }
    else
    {
        // No enable I2C Interrupt
        m_bDownGradeToPolling=TRUE;
    }

    // Well, if every thing is ok, and all error exits clean their pending interrupt
    // this shall never need. After all the check adds cost to every i2c xfer call.
    // Actually, only failure xfer need do it.

    // if ( CleanupPendingInterrupt() )
    // {
    //     DEBUGMSG(ZONE_FUNCTION, (_T("Warning: There is Pending interrupt before process packets\r\n")));
    // }

    // For each I2C packet, transfer data as specified
    bRSTACycleComplete = FALSE;
    for (int i = 0; i < numPackets; i++)
    {
        bAddrCycleComplete = FALSE;

        // Initialize Tx/Rx buffer navigator
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Setting up Tx/Rx Buffer Navigator \r\n")));

        if (!m_bDownGradeToPolling &&((packets[i].byRW & I2C_MODE_MASK)==I2C_POLLING_MODE))
        {
            // Disable I2C Interrupt
            INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN), 
                CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_DISABLE));
            m_bDownGradeToPolling = TRUE;
        }

        *(packets[i].lpiResult) = I2C_ERR_STATEMENT_CORRUPT;

        // Send a START signal if this is our first packet
        if (i == 0)
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket():Issuing START command.\r\n")));
            if (!GenerateStart(&packets[i]))
            {
                retVal = FALSE;
                goto __exit;
            }
            bAddrCycleComplete = TRUE;
        }
        // Send a REPEATED START signal if the address
        // changed or the transfer direction changed.
        else 
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket():Issuing REPEATED START command.\r\n")));
            if (!GenerateRepeatedStart(&packets[i], bRSTACycleComplete))
            {
                retVal = FALSE;
                goto __exit;
            }

            bAddrCycleComplete = TRUE;
        }
        // Is I2C in master mode?
        if (byLastMode == I2C_MASTER_MODE)
        {

            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): I2C In Master Mode! \r\n")));
            // I2C Transmitting?
            if ((packets[i].byRW & I2C_METHOD_MASK) == I2C_RW_WRITE)
            {
                if (!WritePacket(&packets[i], (i + 1 == numPackets)))
                {
                    retVal = FALSE;
                    goto __exit;
                }
                bRSTACycleComplete = FALSE;
            }
            // I2C Receiving?
            else
            {
                if (!ReadPacket(&packets[i], 
                                (i + 1 == numPackets), 
                                bAddrCycleComplete,
                                &bRSTACycleComplete))
                {
                    retVal = FALSE;
                    goto __exit;
                }
            }

            *(packets[i].lpiResult) = I2C_NO_ERROR;
        }
    }
    
__exit:

    if (!bSlaveInUse)
    {
        if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF), 
                     I2C_I2SR_IIF_LSH) == I2C_I2SR_IIF_PENDING)
        {
            DEBUGMSG(ZONE_FUNCTION, (_T("IIF dose not reset in ProcessPkt\r\n")));
        }

        // Disable I2C Module
        OUTREG16(&pI2CReg->I2CR, 0);

        // Disable I2C Clock
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Disabling I2C Clock! \r\n")));
        BSPI2CEnableClock(m_iModuleIndex, FALSE);
    }
    else 
    {
        UINT16 i2cr;

        // Disable I2C Module
        OUTREG16(&pI2CReg->I2CR, 0);
        OUTREG16(&pI2CReg->IADR, byInUseSlaveAddr);

        // Init I2CR
        i2cr = CSP_BITFVAL(I2C_I2CR_IEN, I2C_I2CR_IEN_ENABLE);
        OUTREG16(&pI2CReg->I2CR, i2cr);
        i2cr = CSP_BITFVAL(I2C_I2CR_IEN, I2C_I2CR_IEN_ENABLE)|CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_ENABLE);
        OUTREG16(&pI2CReg->I2CR, i2cr);
        OUTREG16(&pI2CReg->I2SR, 0);
    }

    // On completion, release bus lock mutex
    LeaveCriticalSection(&gcsI2CBusLock);

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: I2CIST
//
// This function is the IPU IST thread.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void WINAPI I2CClass::I2CIST(LPVOID lpParameter)
{
    I2CClass *pI2C = (I2CClass *)lpParameter;

    pI2C->I2CInterruptHandler(INFINITE);

    // Should only ever get here with bTerminateISTThread == TRUE. Simply
    // returning here will gracefully terminate the IST handler thread.
    return;
}


//-----------------------------------------------------------------------------
//
// Function: I2CInterruptHandler
//
// This function is the interrupt handler for the I2C.
// It waits for an I2C interrupt, and signals
// the event to the driver.  This additional layer of 
// event signalling is required to prevent priority inversion bugs.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void I2CClass::I2CInterruptHandler(UINT32 timeout)
{
    UINT16 i2sr;
    UINT16 i2cr;
    INT i;
    INT iIdleCount;

    // Loop here until signal to terminate the IST thread is received.

__loop:
    while(!bTerminateISTThread)
    {
        DEBUGMSG (ZONE_FUNCTION, (TEXT("%s: In the loop\r\n"), __WFUNCTION__));

        if (WaitForSingleObject(hInterrupted, timeout) == WAIT_OBJECT_0)
        {

            // Check if we woke up because of a terminate IST thread request.
            if (bTerminateISTThread)
            {
                break;
            }

            DEBUGMSG (ZONE_FUNCTION, (TEXT("%s: Interrupt received\r\n"), __WFUNCTION__));

            i2cr =INREG16(&pI2CReg->I2CR);
            i2sr= INREG16(&pI2CReg->I2SR);

            // if receive unexpected IIF
            //    we discard the data in I2DR
            //    and 
            //    give NO ACK signal to master
            //    and release SCL line
            if ((CSP_BITFEXT(i2cr, I2C_I2CR_MSTA)==I2C_I2CR_MSTA_SLAVE) 
                && (CSP_BITFEXT(i2sr, I2C_I2SR_IAAS)==I2C_I2SR_IAAS_NOT_ADDRESSED)
                && (CSP_BITFEXT(i2sr, I2C_I2SR_IAL)==I2C_I2SR_IAL_NOT_LOST)
                && (CSP_BITFEXT(i2sr, I2C_I2SR_IIF)==I2C_I2SR_IIF_PENDING))
            {
                CSP_BITFCLR(i2cr, I2C_I2CR_MTX);
                i2cr|=CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND);
                OUTREG16(&pI2CReg->I2CR, i2cr);
                INREG16(&pI2CReg->I2DR);
                CSP_BITFCLR(i2sr, I2C_I2SR_IAL);
                CSP_BITFCLR(i2sr, I2C_I2SR_IIF);
                OUTREG16(&pI2CReg->I2SR, i2sr);
                InterruptDone(dwSysIntr);
                continue;
            }


            if ((CSP_BITFEXT(i2cr, I2C_I2CR_MSTA)==I2C_I2CR_MSTA_SLAVE) 
                && (CSP_BITFEXT(i2sr, I2C_I2SR_IAAS)==I2C_I2SR_IAAS_ADDRESSED)
                && (CSP_BITFEXT(i2sr, I2C_I2SR_IIF)==I2C_I2SR_IIF_PENDING))
            {
                
                i2cr= INREG16(&pI2CReg->I2CR);
                CSP_BITFCLR(i2cr, I2C_I2CR_IIEN);
                OUTREG16(&pI2CReg->I2CR, i2cr);

                InterruptDone(dwSysIntr);

iaas_slave:
                // iaas_slave flag:
                // software can't get STOP/RESTART signal from i2c hardware
                // whenever we find IAAS is set, 
                // treat it as the slave is selected again
                while(CSP_BITFEXT(i2sr, I2C_I2SR_IIF)!=I2C_I2SR_IIF_PENDING)
                {
                    i2sr= INREG16(&pI2CReg->I2SR); 
                }

                i2cr= INREG16(&pI2CReg->I2CR);
                CSP_BITFCLR(i2sr, I2C_I2SR_IAL);
                CSP_BITFCLR(i2sr, I2C_I2SR_IIF);
                OUTREG16(&pI2CReg->I2SR, i2sr);

                if (CSP_BITFEXT(i2sr, I2C_I2SR_SRW)==I2C_I2SR_SRW_TRANSMIT)
                {
                    //  i2c is in slave transmit mode
                    if(pSBuf->iBufSize<=1)
                    {
                        i2cr|=CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT);
                        i2cr|=CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND);
                        OUTREG16(&pI2CReg->I2CR, i2cr);
                        OUTREG16(&pI2CReg->I2DR, pSBuf->byBuf[0]);
                        iIdleCount=I2C_TRANSMIT_WAIT;
                        i2sr= INREG16(&pI2CReg->I2SR);
                        while (CSP_BITFEXT(i2sr, I2C_I2SR_IIF)!=I2C_I2SR_IIF_PENDING) 
                        {
                            --iIdleCount;
                            if( iIdleCount<=0 )
                            {
                                goto end_transmit;
                            }
                            i2sr= INREG16(&pI2CReg->I2SR);
                            if (CSP_BITFEXT(i2sr, I2C_I2SR_IAL)==I2C_I2SR_IAL_LOST)
                            {
                                goto end_transmit;
                            }
                            if (CSP_BITFEXT(i2sr, I2C_I2SR_IBB)==I2C_I2SR_IBB_IDLE)
                            {
                                goto end_transmit;
                            }
                            if (CSP_BITFEXT(i2sr, I2C_I2SR_IAAS)==I2C_I2SR_IAAS_ADDRESSED)
                            {
                                goto iaas_slave;
                            }
                            i2cr= INREG16(&pI2CReg->I2CR);
                            if (CSP_BITFEXT(i2cr, I2C_I2CR_IIEN)==I2C_I2CR_IIEN_ENABLE)
                            {
                                // the interface is preempted by master mode
                                goto __loop;
                            }
                        }
                    }
                    else 
                    {
                        i2cr|=CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT);
                        CSP_BITFCLR(i2cr, I2C_I2CR_TXAK);
                        OUTREG16(&pI2CReg->I2CR, i2cr);
                        for (i=0; i<pSBuf->iBufSize; ++i)
                        {
                           if (i>=pSBuf->iBufSize-1) 
                            {
                                i2cr|=CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND);
                                OUTREG16(&pI2CReg->I2CR, i2cr);
                            }
                            OUTREG16(&pI2CReg->I2DR, pSBuf->byBuf[i]);

                            i2sr= INREG16(&pI2CReg->I2SR);
                            iIdleCount=I2C_TRANSMIT_WAIT;
                            while (CSP_BITFEXT(i2sr, I2C_I2SR_IIF)!=I2C_I2SR_IIF_PENDING) 
                            {
                                --iIdleCount;
                                if( iIdleCount<=0 )
                                {
                                    goto end_transmit;
                                }
                                i2sr= INREG16(&pI2CReg->I2SR);
                                if (CSP_BITFEXT(i2sr, I2C_I2SR_IAL)==I2C_I2SR_IAL_LOST)
                                {
                                    goto end_transmit;
                                }
                                if (CSP_BITFEXT(i2sr, I2C_I2SR_IBB)==I2C_I2SR_IBB_IDLE)
                                {
                                    goto end_transmit;
                                }
                                if (CSP_BITFEXT(i2sr, I2C_I2SR_IAAS)==I2C_I2SR_IAAS_ADDRESSED)
                                {
                                    goto iaas_slave;
                                }
                                i2cr= INREG16(&pI2CReg->I2CR);
                                if (CSP_BITFEXT(i2cr, I2C_I2CR_IIEN)==I2C_I2CR_IIEN_ENABLE)
                                {
                                    // the interface is preempted by master mode
                                    goto __loop;
                                }
                            }

                            CSP_BITFCLR(i2sr, I2C_I2SR_IIF);
                            OUTREG16(&pI2CReg->I2SR, i2sr);

                            if (CSP_BITFEXT(i2sr, I2C_I2SR_RXAK)!=I2C_I2SR_RXAK_ACK_DETECT)
                            {
                                goto end_transmit;
                            }
                        }
                    }
end_transmit:
                    i2sr= INREG16(&pI2CReg->I2SR);
                    CSP_BITFCLR(i2sr, I2C_I2SR_IAL);
                    CSP_BITFCLR(i2sr, I2C_I2SR_IIF);
                    OUTREG16(&pI2CReg->I2SR, i2sr);
                    i2cr= INREG16(&pI2CReg->I2CR);
                    CSP_BITFCLR(i2cr, I2C_I2CR_MTX);
                    i2cr |= CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_ENABLE);
                    i2cr |= CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND);
                    OUTREG16(&pI2CReg->I2CR, i2cr);
                    OUTREG16(&pI2CReg->I2SR, i2sr);

                    INREG16(&pI2CReg->I2DR);
                }
                else
                {
                    // I2C slave is in receive mode

                    if(pSBuf->iBufSize<=1)
                    {
                        CSP_BITFCLR(i2cr, I2C_I2CR_MTX);
                        i2cr|=CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND);
                        OUTREG16(&pI2CReg->I2CR, i2cr);
                        INREG16(&pI2CReg->I2DR);
                        iIdleCount=I2C_TRANSMIT_WAIT;
                        i2sr= INREG16(&pI2CReg->I2SR);
                        while (CSP_BITFEXT(i2sr, I2C_I2SR_IIF)!=I2C_I2SR_IIF_PENDING) 
                        {
                            --iIdleCount;
                            if( iIdleCount<=0 )
                            {
                                goto end_receive;
                            }
                            i2sr= INREG16(&pI2CReg->I2SR);
                            if (CSP_BITFEXT(i2sr, I2C_I2SR_IAL)==I2C_I2SR_IAL_LOST)
                            {
                                goto end_receive;
                            }
                            if (CSP_BITFEXT(i2sr, I2C_I2SR_IBB)==I2C_I2SR_IBB_IDLE)
                            {
                                goto end_receive;
                            }
                            if (CSP_BITFEXT(i2sr, I2C_I2SR_IAAS)==I2C_I2SR_IAAS_ADDRESSED)
                            {
                                goto iaas_slave;
                            }
                            i2cr= INREG16(&pI2CReg->I2CR);
                            if (CSP_BITFEXT(i2cr, I2C_I2CR_IIEN)==I2C_I2CR_IIEN_ENABLE)
                            {
                                // the interface is preempted by master mode
                                goto __loop;
                            }
                        }

                        pSBuf->byBuf[0]=(BYTE)INREG16(&pI2CReg->I2DR);
                    }
                    else 
                    {
                        CSP_BITFCLR(i2cr, I2C_I2CR_MTX);
                        CSP_BITFCLR(i2cr, I2C_I2CR_TXAK);
                        OUTREG16(&pI2CReg->I2CR, i2cr);
                        INREG16(&pI2CReg->I2DR);

                        for (i=0; i<pSBuf->iBufSize; ++i)
                        {
                            if (i>=pSBuf->iBufSize-1) 
                            {
                                i2cr|=CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND);
                                OUTREG16(&pI2CReg->I2CR, i2cr);
                            }
                            
                            iIdleCount=I2C_TRANSMIT_WAIT;
                            i2sr= INREG16(&pI2CReg->I2SR);
                            while (CSP_BITFEXT(i2sr, I2C_I2SR_IIF)!=I2C_I2SR_IIF_PENDING) 
                            {
                                --iIdleCount;
                                if( iIdleCount<=0 )
                                {
                                    goto end_receive;
                                }
                                i2sr= INREG16(&pI2CReg->I2SR);
                                if (CSP_BITFEXT(i2sr, I2C_I2SR_IAL)==I2C_I2SR_IAL_LOST)
                                {
                                    goto end_receive;
                                }
                                if (CSP_BITFEXT(i2sr, I2C_I2SR_IBB)==I2C_I2SR_IBB_IDLE)
                                {
                                    goto end_receive;
                                }
                                if (CSP_BITFEXT(i2sr, I2C_I2SR_IAAS)==I2C_I2SR_IAAS_ADDRESSED)
                                {
                                    goto iaas_slave;
                                }
                                i2cr= INREG16(&pI2CReg->I2CR);
                                if (CSP_BITFEXT(i2cr, I2C_I2CR_IIEN)==I2C_I2CR_IIEN_ENABLE)
                                {
                                    // the interface is preempted by master mode
                                    goto __loop;
                                }
                            }

                            pSBuf->byBuf[i]= (BYTE)INREG16(&pI2CReg->I2DR);

                            CSP_BITFCLR(i2sr, I2C_I2SR_IIF);
                            OUTREG16(&pI2CReg->I2SR, i2sr);

                            if (CSP_BITFEXT(i2sr, I2C_I2SR_RXAK)!=I2C_I2SR_RXAK_ACK_DETECT)
                            {
                                goto end_receive;
                            }

                            if (!bSlaveEventNotified)
                            {
                                SetEvent(m_hI2CUpdateSlaveEvent);
                                bSlaveEventNotified = TRUE;
                            }
                        }
                    }
end_receive:
                    i2sr= INREG16(&pI2CReg->I2SR);
                    CSP_BITFCLR(i2sr, I2C_I2SR_IAL);
                    CSP_BITFCLR(i2sr, I2C_I2SR_IIF);
                    OUTREG16(&pI2CReg->I2SR, i2sr);
                    i2cr= INREG16(&pI2CReg->I2CR);
                    i2cr |= CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_ENABLE);
                    i2cr |= CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND);
                    OUTREG16(&pI2CReg->I2CR, i2cr);
                    if (!bSlaveEventNotified)
                    {
                        SetEvent(m_hI2CUpdateSlaveEvent);
                        bSlaveEventNotified = TRUE;
                    }
                }
            }
            else 
            {   
                if (bDispatchEvent)
                {
                    SetEvent(m_hI2CIntrEvent);
                }
                else 
                {
                    if (EXTREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IEN),
                        I2C_I2CR_IEN_LSH)== I2C_I2CR_IEN_ENABLE)
                    {
                        OUTREG16(&pI2CReg->I2CR, 0);
                    }
                    InterruptDone(dwSysIntr);
                    DEBUGMSG (ZONE_FUNCTION, (_T("interrupt event discard.\r\n")));
                }
            }
        }
        else
        {
            DEBUGMSG (ZONE_ERROR, (TEXT("%s: Time out\r\n"), __WFUNCTION__));
        }

    }

    // Should only ever get here with bTerminateISTThread == TRUE.
    return;
}


//-----------------------------------------------------------------------------
//
// Function: RealMode
//
// This method return the Master/Slave settings in register level.
//
// Parameters:
//
// Returns:
//      I2C_MASTER_MODE: i2c circult currently work in MASTER mode
//      I2C_SLAVE_MODE:  i2c circult currently work in SLAVE mode
//
//-----------------------------------------------------------------------------
BYTE I2CClass::RealMode(void)
{
    UINT16 i2cr;
    i2cr = INREG16(&pI2CReg->I2CR);
    if ( CSP_BITFEXT(i2cr, I2C_I2CR_MSTA)==I2C_I2CR_MSTA_SLAVE)
    {
        return I2C_SLAVE_MODE;
    }
    return I2C_MASTER_MODE;
}

//-----------------------------------------------------------------------------
//
// Function: EnableSlave
//
// This method enable i2c slave from clock gating.
//
// Parameters:
//
// Returns:
//      TRUE:  enabled SLAVE mode
//      FALSE: enable fail 
//
//-----------------------------------------------------------------------------
BOOL I2CClass::EnableSlave(void)
{
    UINT16 i2cr;
    BOOL retVal = FALSE;

    if (TryEnterCriticalSection(&gcsI2CBusLock)==FALSE)
    {
        return FALSE;
    }

    // Enabling I2C Clock
    if (!bSlaveInUse)
    {
        if (!BSPI2CEnableClock(m_iModuleIndex, TRUE))
        {
            BSPI2CEnableClock(m_iModuleIndex, FALSE);
            goto __exit;
        }
    }

    i2cr = INREG16(&pI2CReg->I2CR);
    if ( CSP_BITFEXT(i2cr, I2C_I2CR_MSTA)==I2C_I2CR_MSTA_MASTER)
    {
        // Master conversation is in progress
        goto __exit;
    }

    // Reset I2CR
    OUTREG16(&pI2CReg->I2CR, 0);

    // Configure data sampling rate, and slave address
    OUTREG16(&pI2CReg->IFDR, CSP_BITFVAL(I2C_IFDR_IC, wLastClkRate));
    OUTREG16(&pI2CReg->IADR, CSP_BITFVAL(I2C_IADR_ADR, byLastSelfAddr));

    // Init I2CR
    i2cr = CSP_BITFVAL(I2C_I2CR_IEN, I2C_I2CR_IEN_ENABLE);
    OUTREG16(&pI2CReg->I2CR, i2cr);
    i2cr = CSP_BITFVAL(I2C_I2CR_IEN, I2C_I2CR_IEN_ENABLE)|CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_ENABLE);
    OUTREG16(&pI2CReg->I2CR, i2cr);
    OUTREG16(&pI2CReg->I2SR, 0);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::EnableSlave(): Re init I2CR \r\n")));

    // inform the port setting is slave
    bSlaveInUse = TRUE;

    retVal = TRUE;

__exit:
    LeaveCriticalSection(&gcsI2CBusLock);
    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function: DisableSlave
//
// This method disable i2c slave and gate its input clock.
//
// Parameters:
//
// Returns:
//      TRUE:  disabled SLAVE mode
//      FALSE: disable fail 
//
//-----------------------------------------------------------------------------
BOOL I2CClass::DisableSlave(void)
{
    UINT16 i2cr;
    BOOL retVal = FALSE;

    if (TryEnterCriticalSection(&gcsI2CBusLock)==FALSE)
    {
        return FALSE;
    }

    i2cr = INREG16(&pI2CReg->I2CR);
    if ( CSP_BITFEXT(i2cr, I2C_I2CR_MSTA)==I2C_I2CR_MSTA_MASTER)
    {
        // Master conversation is in progress
        goto __exit;
    }

    if (!bSlaveInUse)
    {
        // Slave is already disabled
        retVal = TRUE;
        goto __exit;
    }

    // Reset I2CR
    OUTREG16(&pI2CReg->I2CR, 0);
    OUTREG16(&pI2CReg->I2SR, 0);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::DisableSlave(): I2CR \r\n")));

    // disable i2c module clock
    BSPI2CEnableClock(m_iModuleIndex, FALSE);

    // inform the port setting is no slave 
    bSlaveInUse = FALSE;

    retVal = TRUE;

__exit:
    LeaveCriticalSection(&gcsI2CBusLock);
    return retVal;
}


//-----------------------------------------------------------------------------
//
// Function: SetSlaveText
//
// This method set slave text in slave interface buffer.
//
// Parameters:
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------

BOOL I2CClass::SetSlaveText(PBYTE pBufIn, DWORD dwLen)
{
    if ((INT)dwLen <=0 || (INT)dwLen >pSBuf->iBufSize)
    {
        return FALSE;
    }

    if (TryEnterCriticalSection(&gcsI2CSlaveLock)==FALSE)
    {
        return FALSE;
    }

    memcpy(pSBuf->byBuf, pBufIn, dwLen);
    bSlaveEventNotified = FALSE;
    LeaveCriticalSection(&gcsI2CSlaveLock);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: GetSlaveText
//
// This method get slave text in slave interface buffer.
//
// Parameters:
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------

BOOL I2CClass::GetSlaveText(PBYTE pBufOut, DWORD dwLen, PDWORD pdwActualOut)
{
    if ((INT)dwLen <= 0)
    {
        *pdwActualOut = 0;
        return FALSE;
    }

    if ((INT)dwLen > pSBuf->iBufSize)
    {
        dwLen = pSBuf->iBufSize;
    }

    EnterCriticalSection(&gcsI2CSlaveLock);
    memcpy(pBufOut, pSBuf->byBuf, dwLen);
    *pdwActualOut = dwLen;
    bSlaveEventNotified = FALSE;

    ResetEvent(m_hI2CUpdateSlaveEvent);
    LeaveCriticalSection(&gcsI2CSlaveLock);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SetSlaveSize
//
// This method set buffer size to the slave interface buffer.
//
// Parameters:
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------

BOOL I2CClass::SetSlaveSize(PBYTE pBufIn, DWORD dwLen)
{
    DWORD dwSize;

    if (dwLen != sizeof(DWORD))
    {
        return FALSE;
    }

    memcpy(&dwSize, pBufIn, dwLen);
    if (dwSize<=0 || dwSize>I2CSLAVEBUFSIZE)
    {
        return FALSE;
    }

    if ((INT)dwSize!= pSBuf->iBufSize)
    {
        EnterCriticalSection(&gcsI2CSlaveLock);

        pSBuf->iBufSize = dwSize;
        bSlaveEventNotified = FALSE;
        LeaveCriticalSection(&gcsI2CSlaveLock);
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: GetSlaveSize
//
// This method set buffer size to the slave interface buffer.
//
// Parameters:
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------

BOOL I2CClass::GetSlaveSize(PBYTE pBufOut, DWORD dwLen, PDWORD pdwActualOut)
{
    if (dwLen != sizeof(DWORD))
    {
        return FALSE;
    }
    memcpy(pBufOut, &pSBuf->iBufSize, dwLen);
    *pdwActualOut = dwLen;

    return TRUE;
}

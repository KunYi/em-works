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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
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
#include <stdlib.h>
#pragma warning(pop)

#include "mxarm11.h"
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

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

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
        m_hI2CIST = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)I2CIST, this, 0, NULL);

        if (m_hI2CIST == NULL)
        {
            DEBUGMSG(ZONE_INIT,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
            iResult = I2C_ERR_INT_INIT;
            return;
        }
        else
        {
            DEBUGMSG(ZONE_INIT, (TEXT("%s: create I2C IST thread success\r\n"), __WFUNCTION__));
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

    // After this point, all initialization routines completed
    iResult = I2C_NO_ERROR;

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
    BYTE bySlaveAddr;

    while (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IBB),
                    I2C_I2SR_IBB_LSH) == I2C_I2SR_IBB_BUSY)
        ; // Intentional polling loop.

    // Grant Bus Master
    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
             CSP_BITFVAL(I2C_I2CR_MSTA, I2C_I2CR_MSTA_MASTER));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("Bus Master Granted!\r\n")));

    // Transmit the slave address, then change to receive mode after
    // we complete the address cycle.
    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MTX),
             CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT));

    bySlaveAddr = (pI2CPkt->byAddr << 1) |
                  ((pI2CPkt->byRW == I2C_RW_READ) ? 1 : 0);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("To I2DR->0x%x (slave addr)\r\n"),
                             bySlaveAddr));

    OUTREG16(&pI2CReg->I2DR, bySlaveAddr);

    // Reset the event manually
    ResetEvent(m_hI2CIntrEvent);

    InterruptDone(dwSysIntr);

    // Wait for interrupt event
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("Waiting for incoming interrupt! \r\n")));

    if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                              TEXT("Timed out waiting for interrupt! ")
                              TEXT("Aborting I2C transfer.\r\n")));

        // Send stop signal to abort
        GenerateStop();

        (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
        return FALSE;
    }

    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_RXAK),
                 I2C_I2SR_RXAK_LSH) == I2C_I2SR_RXAK_NO_ACK_DETECT)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::GenerateStart(): ")
                                 TEXT("No ACK, STOP issued!\r\n")));

        // Send a STOP Signal
        GenerateStop();

        (*(pI2CPkt->lpiResult)) = I2C_ERR_NO_ACK_ISSUED;
        return FALSE;
    }   // if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_RXAK),
        //              I2C_I2SR_RXAK_LSH) == I2C_I2SR_RXAK_NO_ACK_DETECT)

    return TRUE;
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

BOOL I2CClass::GenerateRepeatedStart(PI2C_PACKET pI2CPkt)
{
    BYTE bySlaveAddr;

    // Set the repeated start bit in the I2C CR.
    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_RSTA),
             CSP_BITFVAL(I2C_I2CR_RSTA, I2C_I2CR_RSTA_GENERATE));

    // Temporary fix related to Repeated Start. Delay after repeated start
    // for 1 PAT_REF_CLK period.
    StallExecution(3);

    // Append read or write bit to 7 bit slave address.
    bySlaveAddr = (pI2CPkt->byAddr << 1) |
                  ((pI2CPkt->byRW == I2C_RW_READ) ? 1 : 0);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("To I2DR->0x%x (slave addr)\r\n"),
                             bySlaveAddr));

    OUTREG16(&pI2CReg->I2DR, bySlaveAddr);

    // Reset the event manually
    ResetEvent(m_hI2CIntrEvent);

    InterruptDone(dwSysIntr);

    // Wait for interrupt event
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                             TEXT("Waiting for incoming interrupt! \r\n")));

    if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                              TEXT("Timed out waiting for interrupt! ")
                              TEXT("Aborting I2C transfer.\r\n")));

        // Send stop signal to abort
        GenerateStop();

        (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
        return FALSE;
    }

    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_RXAK),
                 I2C_I2SR_RXAK_LSH) == I2C_I2SR_RXAK_NO_ACK_DETECT)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::GenerateStart(): ")
                                 TEXT("No ACK, STOP issued!\r\n")));
        // Send a STOP Signal
        GenerateStop();

        (*(pI2CPkt->lpiResult)) = I2C_ERR_NO_ACK_ISSUED;
        return FALSE;
    }   // if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_RXAK),
        //              I2C_I2SR_RXAK_LSH) == I2C_I2SR_RXAK_NO_ACK_DETECT)

    return TRUE;
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
    if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IBB),
                 I2C_I2SR_IBB_LSH) != I2C_I2SR_IBB_BUSY)
        return;

    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
             CSP_BITFVAL(I2C_I2CR_MSTA, I2C_I2CR_MSTA_SLAVE));

    // Wait for the stop condition to clear...
    while (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IBB),
                    I2C_I2SR_IBB_LSH) == I2C_I2SR_IBB_BUSY)
        ; // Intentional polling loop.
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

        OUTREG16(&pI2CReg->I2DR, (*pWriteBufPtr));
        pWriteBufPtr++;

        // *modified* 20 Feb: To reset the event manually
        ResetEvent(m_hI2CIntrEvent);

        InterruptDone(dwSysIntr);

        // Wait for interrupt event
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                 TEXT("Waiting for incoming interrupt! \r\n")));

        if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): ")
                                  TEXT("Timed out waiting for interrupt! ")
                                  TEXT("Aborting I2C transfer.\r\n")));

            // Send stop signal to abort
            GenerateStop();

            (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
            return FALSE;
        }

        if (EXTREG16BF(&pI2CReg->I2SR, I2C_I2SR_IAL))
        {
            // Arbitration lost.  An error has occurred, likely due to a bad
            // slave I2C address.

            // Clear IAL bit (we are already put into Stop)
            INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IAL),
                     I2C_I2SR_IAL_NOT_LOST);

            (*(pI2CPkt->lpiResult)) = I2C_ERR_ARBITRATION_LOST;
            return FALSE;
        }


        // Clear Interrupt Signal
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                 TEXT("Clear Interrupt! \r\n")));
        INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF),
                 CSP_BITFVAL(I2C_I2SR_IIF, 0));

        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                 TEXT("Got Detect ACK? \r\n")));
        if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_RXAK),
                     I2C_I2SR_RXAK_LSH) == I2C_I2SR_RXAK_NO_ACK_DETECT)
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                     TEXT("No ACK, STOP issued! -\r\n")));

           // Send a STOP Signal
            GenerateStop();

           (*(pI2CPkt->lpiResult)) = I2C_ERR_NO_ACK_ISSUED;
           break;
        }   // if (EXTREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_RXAK),
            //              I2C_I2SR_RXAK_LSH) == I2C_I2SR_RXAK_NO_ACK_DETECT)
    }
    if (bLast)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): ")
                                 TEXT("Send STOP...this is the last packet ")
                                 TEXT("written. \r\n")));
        // Send STOP signal if this is the last packet to process
        GenerateStop();
    }

    return TRUE;
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
BOOL I2CClass::ReadPacket(PI2C_PACKET pI2CPkt, BOOL bLast, BOOL bAddrCycleComplete)
{
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
        // Dummy read to trigger I2C Read operation
        INREG16(&pI2CReg->I2DR);

        // Reset the event manually.
        ResetEvent(m_hI2CIntrEvent);

        // TODO: Needed?  If we remove, will this interfere with interrupt generation for following read op?
        InterruptDone(dwSysIntr);
    }

    (*(pI2CPkt->lpiResult)) = I2C_NO_ERROR;

    for (int i = 0; i < pI2CPkt->wLen; i++)
    {

        // Wait for data transmission to complete.
        // Wait for interrupt event.
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Waiting for incoming interrupt! \r\n")));

        if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): Timed out waiting for interrupt!  Aborting I2C transfer.\r\n")));                // Send STOP signal if this is the last packet to process

            // Send stop signal to abort
            GenerateStop();

            (*(pI2CPkt->lpiResult)) = I2C_ERR_TRANSFER_TIMEOUT;
            return FALSE;
        }

        if (EXTREG16BF(&pI2CReg->I2SR, I2C_I2SR_IAL))
        {
            // Arbitration lost.  An error has occurred, likely due to a bad
            // slave I2C address.

            // Clear IAL bit (we are already put into Stop)
            INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IAL), I2C_I2SR_IAL_NOT_LOST);

            (*(pI2CPkt->lpiResult)) = I2C_ERR_ARBITRATION_LOST;
            return FALSE;
        }

        // Clear Interrupt Signal
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Clear Interrupt! \r\n")));
        INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF), CSP_BITFVAL(I2C_I2SR_IIF, 0));

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
            }
        }

        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Read next byte! \r\n")));
        (*pReadBufPtr) = (BYTE) INREG16(&pI2CReg->I2DR);
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Byte read: %x \r\n"), *pReadBufPtr));
        ++pReadBufPtr;

        // Reset the event manually.
        ResetEvent(m_hI2CIntrEvent);

        InterruptDone(dwSysIntr);
    }

    return TRUE;
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

    // Must gain ownership to bus lock mutex
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Waiting for Bus Lock CS \r\n")));
    EnterCriticalSection(&gcsI2CBusLock);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Acquired Bus Lock CS! \r\n")));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket():BSPI2CEnableClock(): Enabling I2C Clock! \r\n")));

    // Enabling I2C Clock
    if (!BSPI2CEnableClock(m_iModuleIndex, TRUE))
    {
        DEBUGMSG(ZONE_FUNCTION | ZONE_ERROR, (TEXT("I2CClass::ProcessPacket():BSPI2CEnableClock(): I2C Clock cannot be enabled! \r\n")));
        retVal = FALSE;
        goto __exit;
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

    // Enable I2C Interrupt
    INSREG16(&pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IIEN), CSP_BITFVAL(I2C_I2CR_IIEN, I2C_I2CR_IIEN_ENABLE));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Enable I2C Interrupt \r\n")));

    // For each I2C packet, transfer data as specified
    for (int i = 0; i < numPackets; i++)
    {
        bAddrCycleComplete = FALSE;

        // Initialize Tx/Rx buffer navigator
        DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Setting up Tx/Rx Buffer Navigator \r\n")));

        // Send a START signal if this is our first packet
        if (i == 0)
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket():Issuing START command.\r\n")));
            if (!GenerateStart(&packets[i]))
            {
                retVal = FALSE;
                goto __exit;
            }

            // Reset the event manually.
            ResetEvent(m_hI2CIntrEvent);

            InterruptDone(dwSysIntr);

            // Wait for interrupt event
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Waiting for incoming interrupt! \r\n")));

            if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): Timed out waiting for interrupt!  Aborting I2C transfer.\r\n")));

                // Send stop signal to abort
                GenerateStop();

                retVal = FALSE;
                goto __exit;
            }

            if (EXTREG16BF(&pI2CReg->I2SR, I2C_I2SR_IAL))
            {
                // Arbitration lost.  An error has occurred, likely due to a bad
                // slave I2C address.
                *(packets[i].lpiResult) = I2C_ERR_ARBITRATION_LOST;

                // Clear IAL bit (we are already put into Stop)
                INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IAL), I2C_I2SR_IAL_NOT_LOST);
                retVal = FALSE;
                goto __exit;
            }

            // Clear Interrupt Signal
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Clear Interrupt! \r\n")));
            INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF), CSP_BITFVAL(I2C_I2SR_IIF, 0));

            bAddrCycleComplete = TRUE;
        }
        // Send a REPEATED START signal if the address
        // changed or the transfer direction changed.
        else if ((packets[i].byAddr != packets[i - 1].byAddr) ||
                   (packets[i].byRW != packets[i - 1].byRW))
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket():Issuing REPEATED START command.\r\n")));
            GenerateRepeatedStart(&packets[i]);

            // Reset the event manually.
            ResetEvent(m_hI2CIntrEvent);

            InterruptDone(dwSysIntr);

            // Wait for interrupt event
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Waiting for incoming interrupt! \r\n")));

            if (WaitForSingleObject(m_hI2CIntrEvent, iIntrWaitTimeout) == WAIT_TIMEOUT)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("I2CClass::ProcessPacket(): Timed out waiting for interrupt!  Aborting I2C transfer.\r\n")));

                // Send stop signal to abort
                INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IAL), I2C_I2SR_IAL_NOT_LOST);
                retVal = FALSE;
                goto __exit;
            }

            if (EXTREG16BF(&pI2CReg->I2SR, I2C_I2SR_IAL))
            {
                // Arbitration lost.  An error has occurred, likely due to a bad
                // slave I2C address.
                *(packets[i].lpiResult) = I2C_ERR_ARBITRATION_LOST;

                // Clear IAL bit (we are already put into Stop)
                INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IAL), I2C_I2SR_IAL_NOT_LOST);
                retVal = FALSE;
                goto __exit;
            }

            // Clear Interrupt Signal
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Clear Interrupt! \r\n")));
            INSREG16(&pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF), CSP_BITFVAL(I2C_I2SR_IIF, 0));

            bAddrCycleComplete = TRUE;
        }

        // Is I2C in master mode?
        if (byLastMode == I2C_MASTER_MODE)
        {

            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): I2C In Master Mode! \r\n")));
            // I2C Transmitting?
            if (packets[i].byRW == I2C_RW_WRITE)
            {
                if (!WritePacket(&packets[i], (i + 1 == numPackets)))
                {
                    *(packets[i].lpiResult) = I2C_ERR_TRANSFER_TIMEOUT;
                    retVal = FALSE;
                    goto __exit;
                }
            }
            // I2C Receiving?
            else
            {
                if (!ReadPacket(&packets[i], (i + 1 == numPackets), bAddrCycleComplete))
                {
                    *(packets[i].lpiResult) = I2C_ERR_TRANSFER_TIMEOUT;
                    retVal = FALSE;
                    goto __exit;
                }
            }
        }
        else
        {
            // TODO: Is slave mode support needed?
        }
    }
    
__exit:
    // Disable I2C Module
    OUTREG16(&pI2CReg->I2CR, 0);

    // Disable I2C Clock
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Disabling I2C Clock! \r\n")));                

    BSPI2CEnableClock(m_iModuleIndex, FALSE);

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
void I2CClass::I2CIST(LPVOID lpParameter)
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
    // Loop here until signal to terminate the IST thread is received.
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

            SetEvent(m_hI2CIntrEvent);

            // Kernel call to unmask the interrupt so that it can be signalled again
//            InterruptDone(dwSysIntr);
        }
        else
        {
            DEBUGMSG (ZONE_ERROR, (TEXT("%s: Time out\r\n"), __WFUNCTION__));
        }

    }

    // Should only ever get here with bTerminateISTThread == TRUE.
    return;
}


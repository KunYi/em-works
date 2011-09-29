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
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  i2cclass.c
//
//  This file contains the main STMP I2C protocol engine class.
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

PVOID pv_HWregAPBX    = NULL;

//------------------------------------------------------------------------------
// Local Variables 
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function: SetupTimingRegs
//
// This function will, on obtaining the frequency, determines and writes the values for the timing registers.
//
// Parameters:
//      VOID
//
// Returns:
//      VOID
//
//-----------------------------------------------------------------------------

VOID I2CClass::SetupTimingRegs(VOID)
{
    // 100 khz or less: std mode
    if (dwLastClkRate <= 100000)
    {
        HW_I2C_TIMING0_WR(0x00780030); // high time = 120 clocks, read bit at 48 for 95KHz at 24mhz
        HW_I2C_TIMING1_WR(0x00800030); // low time at 128, write bit at 48 for 95 kHz at 24 MHz
    }

    // 400 khz: fast-mode
    else
    {
        HW_I2C_TIMING0_WR(0x000F0007); // high time = 15 clocks, read bit at 7 for 400KHz at 24mhz
        HW_I2C_TIMING1_WR(0x001F000F); // low time at 31, write bit at 15 for 400 kHz at 24 MHz
    }
        
    HW_I2C_TIMING2_WR(0x0015000d); // bus free count of 21 lead in count of 13

    DEBUGMSG(ZONE_FUNCTION, (L"TIMING0: 0x%08x, TIMING1: 0x%08x\r\n", HW_I2C_TIMING0_RD(), HW_I2C_TIMING1_RD()));
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

I2CClass::I2CClass()
{
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass +\r\n")));

    // Mapped I2C Register Base Physical Address -> Virtual Address +
    {
        PHYSICAL_ADDRESS phyAddr;

        // Copy I2C physical address
        phyAddr.QuadPart = CSP_BASE_REG_PA_I2C;
        // Map to virtual address
        pI2CReg = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);

        // If mapping fails, fatal error, quit
        if (pI2CReg == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:MmMapIoSpace(): Failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = I2C_ERR_PA_VA_MISSING;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:MmMapIoSpace(): pI2CReg=0x%x \r\n"), pI2CReg));

    // Mapped APBX DMA Physical Address -> Virtual Address +
    {
        PHYSICAL_ADDRESS phyAddr;

        // Copy I2C physical address
        phyAddr.QuadPart = CSP_BASE_REG_PA_APBX;
        // Map to virtual address
        pv_HWregAPBX = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);

        // If mapping fails, fatal error, quit
        if (pv_HWregAPBX == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:MmMapIoSpace(APBX): Failed! ErrCode=%d \r\n"), GetLastError()));
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

    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:InitializeCriticalSection(): Creating I2C_IOControl Critical Section! \r\n")));

    // Configure IOMUX for I2C pins
    {
        if (!BSPI2CIOMUXConfig())
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
        // map multiple IRQs (IRQ_I2C_DMA and IRQ_I2C_ERROR) to one sysintr
        DWORD dwIrqs[4] = {(DWORD)-1, /*OAL_INTR_TRANSLATE*/ 1 << 3, IRQ_I2C_DMA, IRQ_I2C_ERROR};

        // Get kernel to translate IRQ -> System Interrupt ID
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, dwIrqs, sizeof(dwIrqs), &dwSysIntr, sizeof(DWORD), NULL))
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

    // reset the module and associated DMA channel
    Reset();

    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass:Reset complete! \r\n")));

    // disable interrupts at the beginning
    HW_I2C_CTRL1_CLR(BM_I2C_CTRL1_BCAST_SLAVE_EN |
                                   BM_I2C_CTRL1_BUS_FREE_IRQ_EN |
                                   BM_I2C_CTRL1_OVERSIZE_XFER_TERM_IRQ_EN |
                                   BM_I2C_CTRL1_SLAVE_STOP_IRQ_EN |
                                   BM_I2C_CTRL1_SLAVE_IRQ_EN |
                                   BM_I2C_CTRL1_DATA_ENGINE_CMPLT_IRQ_EN |
                                   BM_I2C_CTRL1_NO_SLAVE_ACK_IRQ_EN |
                                   BM_I2C_CTRL1_EARLY_TERM_IRQ_EN |
                                   BM_I2C_CTRL1_MASTER_LOSS_IRQ_EN
                                   );

    // Clear the CTRL0 register
    HW_I2C_CTRL0_CLR( BM_I2C_CTRL0_RUN |
                                    BM_I2C_CTRL0_PIO_MODE |
                                    BM_I2C_CTRL0_MULTI_MASTER |
                                    BM_I2C_CTRL0_CLOCK_HELD |
                                    BM_I2C_CTRL0_RETAIN_CLOCK |
                                    BM_I2C_CTRL0_PRE_SEND_START |
                                    BM_I2C_CTRL0_POST_SEND_STOP
                                    );

    // initialize DMA adapter
    m_DMAAdapter.Size = sizeof(m_DMAAdapter);
    m_DMAAdapter.BusMaster = TRUE;
    m_DMAAdapter.BusNumber = 0;
    m_DMAAdapter.InterfaceType = ProcessorInternal;

    // Create DMA buffers
    for (DWORD i = 0; i < NUM_BUFFERS_IN_CHAIN; i++)
    {
        m_DMABuffers[i].pBufferedVirtualAddr = OALDMAAllocBuffer(&m_DMAAdapter, MAX_XFER_COUNT + 1 /*64KB*/ , &m_DMABuffers[i].physicalAddress, FALSE );
        if (m_DMABuffers[i].pBufferedVirtualAddr == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:OALDMAAllocBuffer: failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = I2C_ERR_INT_INIT;
            return;
        }
    }    

    // Create descriptor chain
    PHYSICAL_ADDRESS phyAddr;
    m_pDescChain[0] = (I2C_DMA_BUFFER *) OALDMAAllocBuffer(&m_DMAAdapter, sizeof(I2C_DMA_BUFFER) * NUM_BUFFERS_IN_CHAIN, &phyAddr, FALSE );
    m_dwDescChainPhysAddr = phyAddr.LowPart;
    if (m_pDescChain[0] == NULL)
    {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("I2CClass::I2CClass:OALDMAAllocBuffer: failed! ErrCode=%d \r\n"), GetLastError()));
        iResult = I2C_ERR_INT_INIT;
        return;
    }

    
    // Initialize I2C Mode
    byLastMode = I2C_MASTER_MODE;
    DEBUGMSG(ZONE_INIT, (TEXT("I2CClass::I2CClass: Default to Master Mode \r\n")));

    // Initialize I2C default address and clock.
    byLastSelfAddr= (BYTE) (HW_I2C_CTRL1.B.SLAVE_ADDRESS_BYTE);
    dwLastClkRate= DEFAULT_FREQUENCY; // default desired clk to device = 100 Khz
    dwTimeout = BSPI2CGetTimeout();

    // clock gate at the start
    HW_I2C_CTRL0_SET(BM_I2C_CTRL0_CLKGATE);
        
    // Default Mode is interrupt
    m_dxCurrent = D0;

    pSBuf =0;
    bInUse = FALSE;

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

    // free DMA buffers
    for (DWORD i = 0; i < NUM_BUFFERS_IN_CHAIN; i++)
    {   
        if (m_DMABuffers[i].pBufferedVirtualAddr != NULL) {
            OALDMAFreeBuffer(&m_DMAAdapter, MAX_XFER_COUNT + 1, m_DMABuffers[i].physicalAddress,m_DMABuffers[i].pBufferedVirtualAddr,FALSE);
        }
     }    

    // free descriptor chain
    if (m_pDescChain[0])
    {
        PHYSICAL_ADDRESS phyAddr = {m_dwDescChainPhysAddr,0};
        OALDMAFreeBuffer(&m_DMAAdapter, sizeof(I2C_DMA_BUFFER) * NUM_BUFFERS_IN_CHAIN, phyAddr, m_pDescChain[0], FALSE);
    }
    
    // disable interrupt for the I2C DMA channel
    BF_WR(APBX_CTRL1, CH3_CMDCMPLT_IRQ_EN, 0);

    // Release the interrupt resources
    InterruptDisable(dwSysIntr);
    DEBUGMSG(ZONE_DEINIT, (TEXT("I2CClass::~I2CClass: Release System Interrupt \r\n")));

    CloseHandle(hInterrupted);
    // Prevent the later sections from thinking it is still valid
    hInterrupted = NULL;

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
            MmUnmapIoSpace((LPVOID) pI2CReg, 0x1000);

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
// This method performs a software reset on I2C. It is
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
    iResult = I2C_NO_ERROR;
    

    // A soft reset can take multiple clocks to complete, so do NOT gate the
    // clock when setting soft reset. The reset process will gate the clock
    // automatically. Poll until this has happened before subsequently
    // preparing soft-reset and clock gate
    HW_I2C_CTRL0_CLR(BM_I2C_CTRL0_SFTRST);
    HW_I2C_CTRL0_CLR(BM_I2C_CTRL0_CLKGATE);
    
    // asserting soft-reset
    HW_I2C_CTRL0_SET(BM_I2C_CTRL0_SFTRST);
    
    // waiting for confirmation of soft-reset
    while (!HW_I2C_CTRL0.B.CLKGATE);
    // Done.
    
    HW_I2C_CTRL0_CLR(BM_I2C_CTRL0_SFTRST | BM_I2C_CTRL0_CLKGATE);


    // disable the I2C DMA interrupts
    DDKApbxDmaEnableCommandCmpltIrq(HW_APBX_DMA_I2C_CHANNEL, FALSE);
    
    // Put the DMA in RESET State
    DDKApbxDmaResetChan(HW_APBX_DMA_I2C_CHANNEL, TRUE);

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
//      FALSE  Failed
//
//-----------------------------------------------------------------------------
BOOL I2CClass::ProcessPackets(I2C_PACKET packets[], DWORD numPackets)
{
    BOOL retVal = TRUE;
    // Flag to signal if address cycle just completed
    BOOL bRSTACycleComplete;
    DWORD dwCtrl0 = 0, dwDMACmd = 0;

    // Must gain ownership to bus lock mutex
    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Waiting for Bus Lock CS \r\n")));
    EnterCriticalSection(&gcsI2CBusLock);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket(): Acquired Bus Lock CS! \r\n")));

    // cannot process packets in D4 mode because system is shutting down (along with APBX DMA module)
    if (m_dxCurrent == D4)
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("I2CClass::ProcessPacket():I2C Bus is shutting down, unable to process packets\r\n")));
        retVal = FALSE;

        for (DWORD i = 0; i < numPackets; i++)
        {
            *(packets[i].lpiResult) = I2C_ERR_GENERAL;
        }
        
        goto __exit;
     }

    bInUse = TRUE;

    // enable the clock
    HW_I2C_CTRL0_CLR(BM_I2C_CTRL0_CLKGATE);

    // Configure clock frequency and setup/hold times based on dwLastClkRate
    SetupTimingRegs();


    // set ACK_MODE
    HW_I2C_CTRL1_SET(BM_I2C_CTRL1_ACK_MODE);

    // For each I2C packet, transfer data as specified
    bRSTACycleComplete = FALSE;
    for (DWORD i = 0; i < numPackets; i++)
    {
        dwCtrl0 = 0;
        dwDMACmd = 0;

        // link to this buffer from previous buffer
        if (i > 0) {

            m_pDescChain[i] = (I2C_DMA_BUFFER *) ((DWORD) m_pDescChain[0] + i * sizeof(I2C_DMA_BUFFER));         
            m_pDescChain[i - 1]->pNextBuff = m_dwDescChainPhysAddr + i * sizeof(I2C_DMA_BUFFER);

            m_pDescChain[i - 1]->dwDMACmd |= BF_APBX_CHn_CMD_CHAIN(1);          
        }    

        *(packets[i].lpiResult) = I2C_ERR_STATEMENT_CORRUPT;

        // set CMDWORD to 1
        dwDMACmd |= BF_APBX_CHn_CMD_CMDWORDS(1);

        // if this is not the last packet, or it is 1st packet
        if (i != numPackets-1 || i == 0)
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket():Issuing START command: %d\r\n"), i));

            // set the PRE_SEND_START bit in the CTRL0 word
            dwCtrl0 |= BM_I2C_CTRL0_PRE_SEND_START;

            // if there is more than 1 packet in the chain, set the RETAIN_LOCK bit on all buffers except first and last
            if ( numPackets > 1 && i != 0)
                dwCtrl0 |= BF_I2C_CTRL0_RETAIN_CLOCK(BV_I2C_CTRL0_RETAIN_CLOCK__HOLD_LOW);

            // WAIT4ENDCMD
            dwDMACmd |= BF_APBX_CHn_CMD_WAIT4ENDCMD(1);
        }

        // Send a STOP signal if this is our final packet
        if (i == numPackets-1)
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2CClass::ProcessPacket():Issuing STOP command: %d\r\n"), i));

            // set the PRE_SEND_START bit in the CTRL0 word
            dwCtrl0 |= BM_I2C_CTRL0_POST_SEND_STOP;

            // SEND_NAK if this is a read
            if (packets[i].byRW == I2C_RW_READ)
                dwCtrl0 |= BM_I2C_CTRL0_SEND_NAK_ON_LAST;    

            // set IRQONCMPLT if this is last descriptor in the chain
            dwDMACmd |= BF_APBX_CHn_CMD_IRQONCMPLT(1);

            // decrement SEMAPHORE, only on last buffer
            dwDMACmd |= BF_APBX_CHn_CMD_SEMAPHORE(1);

            // no next descriptor
            m_pDescChain[i]->pNextBuff = 0;       

        }        

        
        // set MASTER_MODE bit according to byLastMode
        dwCtrl0 |= (byLastMode == I2C_MASTER_MODE) ? BM_I2C_CTRL0_MASTER_MODE : 0;
        
        // set DIRECTION bit based on packet info
        dwCtrl0 |= (packets[i].byRW == I2C_RW_WRITE) ? BM_I2C_CTRL0_DIRECTION : 0;
        dwDMACmd |= (packets[i].byRW == I2C_RW_WRITE) ? BV_FLD(APBX_CHn_CMD, COMMAND, DMA_READ) : BV_FLD(APBX_CHn_CMD, COMMAND, DMA_WRITE);

        // set XFER_COUNT
        dwCtrl0 |= BF_I2C_CTRL0_XFER_COUNT(packets[i].wLen);
        dwDMACmd |= BF_APBX_CHn_CMD_XFER_COUNT(packets[i].wLen);
        
        // Setup the ith buffer in the chain

        m_pDescChain[i]->dwCtrl0 = dwCtrl0;
        m_pDescChain[i]->pbDataBufferAddress = m_DMABuffers[i].physicalAddress.LowPart; //later, do a memcpy to/from packets[i].pbyBuf;
        m_pDescChain[i]->dwDMACmd = dwDMACmd;

        m_DMABuffers[i].pSrcVirtualAddr = (PVOID) packets[i].pbyBuf;
        m_DMABuffers[i].pSrcSize = packets[i].wLen;

        // copy over the bytes to the DMA buffer before starting the write
        if (packets[i].byRW == I2C_RW_WRITE) {
            CeSafeCopyMemory(m_DMABuffers[i].pBufferedVirtualAddr, m_DMABuffers[i].pSrcVirtualAddr, m_DMABuffers[i].pSrcSize);
        }

        *(packets[i].lpiResult) = I2C_NO_ERROR;

    }

    if(!DDKApbxDmaInitChan(HW_APBX_DMA_I2C_CHANNEL, TRUE))
    {
        ERRORMSG(1, (L"ProcessPackets: Unable to initialize DMA channel\r\n"));
        retVal = FALSE;
        goto __exit;
    }

    // clear any existing interrupt before starting DMA
    DDKApbxDmaClearCommandCmpltIrq(HW_APBX_DMA_I2C_CHANNEL);

    // Start the  DMA channel
    DDKApbxStartDma(HW_APBX_DMA_I2C_CHANNEL,(PVOID) m_dwDescChainPhysAddr, 1);

    // wait for completion
    if (WaitForSingleObject(hInterrupted, dwTimeout) == WAIT_OBJECT_0)
    {

        // error checking
        if (HW_APBX_CTRL2.B.CH3_ERROR_IRQ)
        {
            DEBUGMSG( ZONE_ERROR, (L"I2cClass: Error tranferring data: CTRL0: 0x%08x CTRL1: 0x%08x\r\n", HW_I2C_CTRL0_RD(), HW_I2C_CTRL1_RD()));

            // clear the DMA error interrupt bit
            HW_APBX_CTRL2_CLR(BM_APBX_CTRL2_CH3_ERROR_IRQ);
            
            for (DWORD i = 0; i < numPackets; i++)
            {
                if (HW_I2C_CTRL1.B.NO_SLAVE_ACK_IRQ)
                {
                    *(packets[i].lpiResult) = I2C_ERR_NO_ACK_ISSUED;
                }
                else if (HW_I2C_CTRL1.B.EARLY_TERM_IRQ)
                {
                    *(packets[i].lpiResult) = I2C_ERR_EARLY_TERM;
                }
                else if (HW_I2C_CTRL1.B.MASTER_LOSS_IRQ)
                {
                    *(packets[i].lpiResult) = I2C_ERR_GENERAL;
                }
            }
            
            retVal = FALSE;
            Reset();            // reset module and DMA
        } 

        // success
        else
        {         
            for (DWORD i = 0; i < numPackets; i++)
            {
                // copy over the bytes to the original buffer after read is complete
                if (packets[i].byRW == I2C_RW_READ)
                {
                    CeSafeCopyMemory(m_DMABuffers[i].pSrcVirtualAddr, m_DMABuffers[i].pBufferedVirtualAddr, m_DMABuffers[i].pSrcSize);
                }

            }
        }

    }

    // WAIT_TIMEOUT or WAIT_FAILED
    else
    {       
        DEBUGMSG(ZONE_ERROR, (L"Timeout while waiting for I2C DMA interrupt\r\n"));

        for (DWORD i = 0; i < numPackets; i++)
        {
            *(packets[i].lpiResult) = I2C_ERR_TRANSFER_TIMEOUT;
        }        

        retVal = FALSE;
        Reset();
        
    }

    // Disable the interrupt
    DDKApbxDmaEnableCommandCmpltIrq(HW_APBX_DMA_I2C_CHANNEL, FALSE);

    // clear the I2C DMA interrupt
    DDKApbxDmaClearCommandCmpltIrq(HW_APBX_DMA_I2C_CHANNEL);

    // stop channel (puts it in reset state)
    DDKApbxStopDma(HW_APBX_DMA_I2C_CHANNEL);

    // re-enable I2C and APBX channel interrupts
    InterruptDone(dwSysIntr);

__exit:

    // clock gate
    HW_I2C_CTRL0_SET(BM_I2C_CTRL0_CLKGATE);

    bInUse = FALSE;

    // On completion, release bus lock mutex
    LeaveCriticalSection(&gcsI2CBusLock);

    return retVal;
}


VOID I2CClass::BusyWait(void)
{

    if (bInUse)
    {
        Sleep(0);
    }
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
    if ( HW_I2C_CTRL0.B.MASTER_MODE ==BV_I2C_CTRL0_MASTER_MODE__SLAVE)
    {
        return I2C_SLAVE_MODE;
    }
    return I2C_MASTER_MODE;
}

//-----------------------------------------------------------------------------
//
// Function: EnableSlave
//
// This method enable i2c slave, which is not implemented
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
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: DisableSlave
//
// This method disable i2c slave
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
    byLastMode = I2C_MASTER_MODE;

    return TRUE;

}


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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: cspiClass.cpp
//
// Provides the implementation of the CSPI bus driver to support CSPI
// transactions from multiple client drivers.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <linklist.h>
#include <Devload.h>

#include "csp.h"
#include "cspibus.h"
#include "cspiClass.h"

//------------------------------------------------------------------------------
// External Functions
extern "C" UINT32 BSPCSPICalculateDivRate(UINT32 dwFrequency);
extern "C" BOOL BSPCSPISetIOMux(UINT32 Index);
extern "C" BOOL BSPCSPIReleaseIOMux(UINT32 dwIndex);
extern "C" BOOL BSPCSPIEnableClock(UINT32 dwIndex, BOOL bEnable);

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: cspiClass
//
// The constructor of the class.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
cspiClass::cspiClass()
{
    DEBUGMSG(ZONE_INIT, (TEXT("cspiClass +\r\n")));

    m_pCSPI         = NULL;
    m_cspiOpenCount = 0;
    m_hIntrEvent    = NULL;
    m_hEnQEvent     = NULL;
    m_hThread       = NULL;
    m_hHeap         = NULL;
    m_dwSysIntr     = SYSINTR_UNDEFINED;
    m_bTerminate    = FALSE;
    m_bUsePolling   = FALSE;

    DEBUGMSG(ZONE_INIT, (TEXT("cspiClass -\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ~cspiClass
//
// The destructor of the class.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
cspiClass::~cspiClass()
{
    DEBUGMSG(ZONE_INIT, (TEXT("~cspiClass\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: CspiInitialize
//
// Initializes the CSPI interface and data structures.
//
// Parameters:
//      Index
//          [in] CSPI instance (1 = CSPI1, 2 = CSP2) to initialize
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
BOOL cspiClass::CspiInitialize(DWORD Index)
{
    PHYSICAL_ADDRESS phyAddr;
    DWORD irq;

    m_nIndex = Index;

    // Create global heap for internal queues/buffers
    //      flOptions = 0 => no options
    //      dwInitialSize = 0 => zero bytes initial size
    //      dwMaximumSize = 0 => heap size limited only by available memory
    DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize: m_hHeap=0x%x\r\n"), m_hHeap));
    m_hHeap = HeapCreate(0, 0, 0);

    // Check if HeapCreate failed
    if (m_hHeap == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize: HeapCreate failed!\r\n")));
        goto Error;
    }

    // Create event for CSPI interrupt signaling
    //      pEventAttributes = NULL (must be NULL)
    //      bManualReset = FALSE => resets automatically to nonsignaled
    //                              state after waiting thread released
    //      bInitialState = FALSE => initial state is non-signaled
    //      lpName = NULL => object created without a name
    m_hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Check if CreateEvent failed
    if (m_hIntrEvent == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize: CreateEvent failed!\r\n")));
        goto Error;
    }

    // Create event for process queue thread signaling
    //      pEventAttributes = NULL (must be NULL)
    //      bManualReset = FALSE => resets automatically to nonsignaled
    //                              state after waiting thread released
    //      bInitialState = FALSE => initial state is non-signaled
    //      lpName = NULL => object created without a name
    m_hEnQEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Check if CreateEvent failed
    if (m_hEnQEvent == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize: CreateEvent failed!\r\n")));
        goto Error;
    }

    if (Index == 1) {           // If request is for CSPI1
        phyAddr.QuadPart = CSP_BASE_REG_PA_CSPI1;
        irq = IRQ_CSPI1;
    } else if (Index == 2) {    // If request is for CSPI2
        phyAddr.QuadPart = CSP_BASE_REG_PA_CSPI2;
        irq = IRQ_CSPI2;
    } else if (Index == 3) {    // If request is for CSPI3
        phyAddr.QuadPart = CSP_BASE_REG_PA_CSPI3;
        irq = IRQ_CSPI3;
    } else {                    // Else invalid CSPI instance
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize: Invalid CSPI instance!\r\n")));
        goto Error;
    }

    // Configure IOMUX
    BSPCSPISetIOMux(Index);

    // Map peripheral physical address to virtual address
    m_pCSPI = (PCSP_CSPI_REGS)MmMapIoSpace(phyAddr, sizeof(CSP_CSPI_REGS), FALSE);

    // Check if virtual mapping failed
    if (m_pCSPI == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize: MmMapIoSpace failed!\r\n")));
        goto Error;
    }

    // Call the OAL to translate the IRQ into a SysIntr value.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
        &m_dwSysIntr, sizeof(DWORD), NULL)) {
        RETAILMSG(1, (TEXT("ERROR: Failed to obtain sysintr value for CSPI interrupt.\r\n")));
        goto Error;
    }

    // Register CSPI interrupt
    if (!InterruptInitialize(m_dwSysIntr, m_hIntrEvent, NULL, 0)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize: InterruptInitialize failed!\r\n")));
        goto Error;
    }

    // Create CSPI critical section
    InitializeCriticalSection(&m_cspiCs);

    // Create CSPI Data Exchange critical section
    InitializeCriticalSection(&m_cspiDataXchCs);

    // Initialize CSPI linked list of client queue entries
    InitializeListHead(&m_ListHead);

    DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize: &m_pCSPI=0x%x\r\n"), &m_pCSPI));

    // Disable CSPI to reset internal logic
    INSREG32BF(&m_pCSPI->CONTROLREG, CSPI_CONTROLREG_SPIEN, CSPI_CONTROLREG_SPIEN_DISABLE);

    // Create CSPI queue processing thread if it does not exist
    if (!m_hThread) {
        // Set global termination flag
         m_bTerminate=FALSE;

        // Create processing thread
        //      pThreadAttributes = NULL (must be NULL)
        //      dwStackSize = 0 => default stack size determined by linker
        //      lpStartAddress = CspiProcessQueue => thread entry point
        //      lpParameter = NULL => point to thread parameter
        //      dwCreationFlags = 0 => no flags
        //      lpThreadId = NULL => thread ID is not returned
        m_hThread = ::CreateThread(NULL, 0, CspiProcessQueue, this, 0, NULL);
        DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize: this=0x%x\r\n"),this));

        // Check if CreateThread failed
        if (m_hThread == NULL) {
            DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize: CreateThread failed!\r\n")));
            goto Error;
        }

        // set thread priority
        CeSetThreadPriority (m_hThread, 60);
    }

    m_cspiOpenCount++;

    return TRUE;

Error:
    CspiRelease();

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: CspiRelease
//
// Frees allocated memory, closes reference to handles, and resets the state
// of global member variables.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void cspiClass::CspiRelease()
{
    DEBUGMSG(ZONE_CLOSE, (TEXT("CspiRelease +\r\n")));

    // Kill the exchange packet processing thread
    if (m_hThread) {
        m_bTerminate = TRUE;
        // Try to signal the thread so that it can wake up and terminate
        if (m_hEnQEvent)
            SetEvent(m_hEnQEvent);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    // Release IOMUX pins
    BSPCSPIReleaseIOMux(m_nIndex);

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwSysIntr, sizeof(DWORD), NULL, 0, NULL);
    m_dwSysIntr = SYSINTR_UNDEFINED;

    // Destroy the heap
    if (m_hHeap != NULL) {
        HeapDestroy(m_hHeap);
        m_hHeap = NULL;
    }

    // Close interrupt event handle
    if (m_hIntrEvent) {
        CloseHandle(m_hIntrEvent);
        m_hIntrEvent = NULL;
    }

    // Close enqueue event handle
    if (m_hEnQEvent) {
        CloseHandle(m_hEnQEvent);
        m_hEnQEvent = NULL;
    }

    // Free the virtual space allocated for CSPI memory map
    if (m_pCSPI != NULL) {
        MmUnmapIoSpace(m_pCSPI, sizeof(CSP_CSPI_REGS));
        m_pCSPI = NULL;
    }

    // Deregister the system interrupt
    if (m_dwSysIntr != SYSINTR_UNDEFINED) {
        InterruptDisable(m_dwSysIntr);
        m_dwSysIntr = SYSINTR_UNDEFINED;
    }

    // Delete the critical section
    DeleteCriticalSection(&m_cspiCs);

    // Delete the Data Exchange critical section
    DeleteCriticalSection(&m_cspiDataXchCs);

    DEBUGMSG(ZONE_CLOSE, (TEXT("CspiRelease -\r\n")));
    return;
}

//------------------------------------------------------------------------------
//
// Function: CspiEnqueue
//
// This function is invoked as a result of a CSPI client driver calling
// the CSPI DeviceIoControl with CSPI_IOCTL_EXCHANGE. This function
//
// Parameters:
//      pData
//          [in]
//
// Returns:
//      Returns a handle to the newly created msg queue, or NULL if the
//      msg queue creation failed.
//
//------------------------------------------------------------------------------
BOOL cspiClass::CspiEnqueue(PCSPI_XCH_PKT_T pXchPkt)
{
    PCSPI_XCH_LIST_ENTRY_T pXchListEntry = NULL;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("CspiEnqueue +\r\n")));

    // Allocate space for new list entry
    pXchListEntry = (PCSPI_XCH_LIST_ENTRY_T)HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY,
        sizeof(CSPI_XCH_LIST_ENTRY_T));

    // Check if HeapAlloc failed (fatal error, terminate thread)
    if (pXchListEntry == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiProcessQueue: HeapAlloc failed!\r\n")));
        return FALSE;
    }

    // There are embedded pointers. We already make check in iocontrol that all calls
    // must from kernel. So, there is no need to marshall these embedded pointer.
    pXchListEntry->xchPkt.pBusCnfg = pXchPkt->pBusCnfg;
    pXchListEntry->xchPkt.pTxBuf = pXchPkt->pTxBuf;
    pXchListEntry->xchPkt.pRxBuf = pXchPkt->pRxBuf;
    pXchListEntry->xchPkt.xchCnt = pXchPkt->xchCnt;
    pXchListEntry->xchPkt.xchEvent = pXchPkt->xchEvent;

    // Check whether the completion event is NULL
    // If it's NULL, then send the package immediately
    if (pXchListEntry->xchPkt.xchEvent == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("CspiEnqueue: Sending the package immediately\r\n")));

        // Wrap the exchange in critical section to
        // serialize accesses with CspiProcessQueue thread
        EnterCriticalSection(&m_cspiDataXchCs);

        pXchListEntry->xchPkt.xchCnt = CspiDataExchange(&pXchListEntry->xchPkt);

        LeaveCriticalSection(&m_cspiDataXchCs);

        HeapFree(m_hHeap, 0, pXchListEntry);

        return TRUE;
    }

    // Wrap linked list insert operation in critical section to
    // serialize accesses with CspiProcessQueue thread
    EnterCriticalSection(&m_cspiCs);

    // Insert exchange packet into our list
    InsertTailList(&m_ListHead, &(pXchListEntry->link));

    LeaveCriticalSection(&m_cspiCs);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("CspiEnqueue -\r\n")));

    // Signal queue processing thread that a new packet is ready
    return SetEvent(m_hEnQEvent);
}

//------------------------------------------------------------------------------
//
// Function: CspiProcessQueue
//
// This function is the entry point for a thread that will be created during
// CSPI driver initialization and remain resident to process packet exchange
// requests from client devices.
//
// Parameters:
//      lpParameter
//          [in] Pointer to a single 32-bit parameter value passed to the
//          thread during creation. Currently not used.
//
// Returns:
//      Returns TRUE.
//
//------------------------------------------------------------------------------
DWORD WINAPI cspiClass::CspiProcessQueue(LPVOID lpParameter)
{
    PCSPI_XCH_LIST_ENTRY_T pXchListEntry;
    cspiClass * pCspi = (cspiClass *)lpParameter;
    DEBUGMSG(ZONE_THREAD, (TEXT("CspiProcessQueue: lpParameter=0x%x\r\n"),lpParameter));

    // Until queue processing thread termination requested
    while (!pCspi->m_bTerminate) {
        // While our list of unprocessed queues has not been exhausted
        while (!IsListEmpty(&pCspi->m_ListHead)) {
            // Wrap linked list remove operation in critical section to
            // serialize accesses with CSPI_IOCTL_EXCHANGE
            EnterCriticalSection(&pCspi->m_cspiCs);

            // Get next list entry
            pXchListEntry = (PCSPI_XCH_LIST_ENTRY_T)RemoveHeadList(&pCspi->m_ListHead);

            LeaveCriticalSection(&pCspi->m_cspiCs);

            // Check if mapping failed
            if (pXchListEntry == NULL) {
                DEBUGMSG(ZONE_ERROR, (TEXT("CspiProcessQueue: MapCallerPtr failed!\r\n")));
            }
            // Exchange packet mapped to our space
            else {
                // Wrap the exchange in critical section to
                // serialize accesses with CSPI_IOCTL_EXCHANGE
                EnterCriticalSection(&pCspi->m_cspiDataXchCs);

                // Do the exchange and update the exchange count
                pXchListEntry->xchPkt.xchCnt = pCspi->CspiDataExchange(&pXchListEntry->xchPkt);

                LeaveCriticalSection(&pCspi->m_cspiDataXchCs);

                // Signal that new data available in Rx message queue
                if (!SetEvent(pXchListEntry->xchPkt.xchEvent))
                    DEBUGMSG(ZONE_ERROR, (TEXT("CspiProcessQueue: SetEvent failed\r\n")));
            }

            // Free memory allocated to list entry
            HeapFree(pCspi->m_hHeap, 0, pXchListEntry);
        } // while (!IsListEmpty(&m_ListHead))

        // wait for next list entry to arrive
        WaitForSingleObject(pCspi->m_hEnQEvent, INFINITE);
    } // while (!m_bTerminate)

    pCspi->m_hThread = NULL;

    DEBUGMSG(ZONE_THREAD, (TEXT("CspiProcessQueue -\r\n")));
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: CspiDataExchange
//
// Exchanges CSPI data in Master mode.
//
// Parameters:
//      pXchPkt
//          [in] Points to exchange packet information.
//
// Returns:
//      Returns the number of data exchanges that completed successfully.
//
//------------------------------------------------------------------------------
UINT32 cspiClass::CspiDataExchange(PCSPI_XCH_PKT_T pXchPkt)
{
    PCSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;
    LPVOID pTxBuf = pXchPkt->pTxBuf;
    LPVOID pRxBuf = pXchPkt->pRxBuf;
    UINT32 xchTxCnt = 0;
    UINT32 xchRxCnt = 0;
    volatile UINT32 tmp;
    BOOL bXchDone = FALSE;
    UINT32 (*pfnTxBufRd)(LPVOID);
    void (*pfnRxBufWrt)(LPVOID, UINT32);
    UINT8 bufIncr;

    DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange: &m_pCSPI=0x%x\r\n"), &m_pCSPI));

    // Check all translated pointers
    if ((pBusCnfg == NULL) || (pTxBuf == NULL) || (pRxBuf == NULL))
        return 0;

    // Disable all interrupts
    CLRREG32(&m_pCSPI->INT, 0x0003FE00);


    // Set client CSPI bus configuration based
    //  default EN = disabled
    //  default MODE = master
    //  default XCH = idle
    OUTREG32(&m_pCSPI->CONTROLREG,
        CSP_BITFVAL(CSPI_CONTROLREG_SPIEN, CSPI_CONTROLREG_SPIEN_DISABLE) |
        CSP_BITFVAL(CSPI_CONTROLREG_MODE, CSPI_CONTROLREG_MODE_MASTER) |
        CSP_BITFVAL(CSPI_CONTROLREG_XCH, CSPI_CONTROLREG_XCH_IDLE) |
        CSP_BITFVAL(CSPI_CONTROLREG_CS, pBusCnfg->chipselect) |
        CSP_BITFVAL(CSPI_CONTROLREG_DATARATE, BSPCSPICalculateDivRate(pBusCnfg->freq)) |
        CSP_BITFVAL(CSPI_CONTROLREG_SSPOL, pBusCnfg->sspol) |
        CSP_BITFVAL(CSPI_CONTROLREG_SSCTL, pBusCnfg->ssctl) |
        CSP_BITFVAL(CSPI_CONTROLREG_POL, pBusCnfg->pol) |
        CSP_BITFVAL(CSPI_CONTROLREG_PHA, pBusCnfg->pha) |
        CSP_BITFVAL(CSPI_CONTROLREG_BITCOUNT, pBusCnfg->bitcount - 1) |
        CSP_BITFVAL(CSPI_CONTROLREG_DRCTL, pBusCnfg->drctl));


    // Select access funtions based on exchange bit width
    //
    // Bitcount        Tx/Rx Buffer Access Width
    // --------        -------------------------
    //   1 - 8           UINT8  (unsigned 8-bit)
    //   9 - 16          UINT16 (unsigned 16-bit)
    //  17 - 32          UINT32 (unsigned 32-bit)
    //
    if ((pBusCnfg->bitcount >= 1) && (pBusCnfg->bitcount <= 8)) {
        // 8-bit access width
        pfnTxBufRd = CspiBufRd8;
        pfnRxBufWrt = CspiBufWrt8;
        bufIncr = sizeof(UINT8);
    } else if ((pBusCnfg->bitcount >= 9) && (pBusCnfg->bitcount <= 16)) {
        // 16-bit access width
        pfnTxBufRd = CspiBufRd16;
        pfnRxBufWrt = CspiBufWrt16;
        bufIncr = sizeof(UINT16);
    } else if ((pBusCnfg->bitcount >= 17) && (pBusCnfg->bitcount <= 32)) {
        // 32-bit access width
        pfnTxBufRd = CspiBufRd32;
        pfnRxBufWrt = CspiBufWrt32;
        bufIncr = sizeof(UINT32);
    } else {
        // Unsupported access width
        DEBUGMSG(ZONE_WARN, (TEXT("CspiMasterDataExchange: unsupported bitcount!\r\n")));
        return 0;
    }

    // Enable the clock gating
    BSPCSPIEnableClock(m_nIndex, TRUE);

    // Enable the CSPI
    INSREG32BF(&m_pCSPI->CONTROLREG, CSPI_CONTROLREG_SPIEN, CSPI_CONTROLREG_SPIEN_ENABLE);

    // Until we are done with requested transfers
    while (!bXchDone) {
        // Load Tx FIFO until full, or until we run out of data
        while ((!(INREG32(&m_pCSPI->INT) & CSP_BITFMASK(CSPI_INT_TF)))
            && (xchTxCnt < pXchPkt->xchCnt)) {
            // Put next Tx data into CSPI FIFO
            OUTREG32(&m_pCSPI->TXDATA, pfnTxBufRd(pTxBuf));

            // Increment Tx Buffer to next data point
            pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);

            // Increment Tx exchange counter
            xchTxCnt++;
        }

        // Start exchange
        INSREG32BF(&m_pCSPI->CONTROLREG, CSPI_CONTROLREG_XCH, CSPI_CONTROLREG_XCH_EN);

        if (xchTxCnt == pXchPkt->xchCnt) {
            // If we completed requested transfers, then polling or enable
            // interrupt for transfer completed
            if (m_bUsePolling) {
                // Wait until transaction is complete
                while (!(INREG32(&m_pCSPI->INT) & CSP_BITFMASK(CSPI_INT_TSHFE)));
            } else {
                INSREG32BF(&m_pCSPI->INT, CSPI_INT_TSHFEEN, CSPI_INT_TSHFEEN_ENABLE);
            }

            // Set flag to indicate requested exchange done
            bXchDone = TRUE;
        } else {
            // Otherwise we need to wait until FIFO has more room, so
            // we enable interrupt for Rx FIFO half-full (RHEN) to
            // ensure we can read out data that arrived during exchange
            if (m_bUsePolling) {
                // Wait until RH
                while (!(INREG32(&m_pCSPI->INT) & CSP_BITFMASK(CSPI_INT_RH)));
            } else {
                INSREG32BF(&m_pCSPI->INT, CSPI_INT_RHEN, CSPI_INT_RHEN_ENABLE);
            }
        }

        while (xchRxCnt < xchTxCnt) {
            // Wait for requested transfer interrupt
            WaitForSingleObject(m_hIntrEvent, INFINITE);

            // Disable all interrupts
            CLRREG32(&m_pCSPI->INT, 0x0003FE00);

            // Acknowledge transfer complete (w1c)
            OUTREG32(&m_pCSPI->INT, CSP_BITFMASK(CSPI_INT_TSHFEEN));

            // While there is data in Rx FIFO and we have buffer space
            while ((INREG32(&m_pCSPI->INT) & CSP_BITFMASK(CSPI_INT_RR))
                && (xchRxCnt < pXchPkt->xchCnt)) {
                tmp = INREG32(&m_pCSPI->RXDATA);

                // If receive data is not to be discarded
                if (pRxBuf != NULL) {
                    // Get next Rx data from CSPI FIFO
                    pfnRxBufWrt(pRxBuf, tmp);

                    // Increment Rx Buffer to next data point
                    pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
                }
                // else receive data will be discarded

                // Increment Rx exchange counter
                xchRxCnt++;
            }

            // Signal that interrupt has been handled
            InterruptDone(m_dwSysIntr);

            // If Rx has not caught up to Tx, keep waiting until remaining
            // data arrives
            if (xchRxCnt < xchTxCnt) {
                if (m_bUsePolling) {
                    // Disable all CSPI interrupts
                    CLRREG32(&m_pCSPI->INT, 0x0003FE00);
                } else {
                    // Enable Rx FIFO ready interrupt (RREN)
                    INSREG32BF(&m_pCSPI->INT, CSPI_INT_RREN, CSPI_INT_RREN_ENABLE);
                }
            }
        } // while (xchRxCnt < xchTxCnt)
    } // while(!bXchDone)

#if 0
    // Dump out what we just transmitted and received
    pTxBuf = pXchPkt->pTxBuf;
    pRxBuf = pXchPkt->pRxBuf;
    for (xchTxCnt = 0; xchTxCnt < xchRxCnt; xchTxCnt++) {
        DEBUGMSG(ZONE_THREAD, (TEXT("TXDATA[%d] = 0x%x\r\n"), xchTxCnt, pfnTxBufRd(pTxBuf)));
        pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);
    }
    for (xchTxCnt = 0; xchTxCnt < xchRxCnt; xchTxCnt++) {
        DEBUGMSG(ZONE_THREAD, (TEXT("RXDATA[%d] = 0x%x\r\n"), xchTxCnt, pfnTxBufRd(pRxBuf)));
        pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
    }
#endif

    // Disable the clock gating
    BSPCSPIEnableClock(m_nIndex, FALSE);

    DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange -\r\n")));
    return xchRxCnt;
}

//------------------------------------------------------------------------------
//
// Function: CspiBufRd8
//
// This function is used to access a buffer as an array of 8-bit (UINT8)
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer, promoted to UINT32.
//
//------------------------------------------------------------------------------
UINT32 cspiClass::CspiBufRd8(LPVOID pBuf)
{
    UINT8 *p;

    p = (UINT8 *) pBuf;

    return *p;
}

//------------------------------------------------------------------------------
//
// Function: CspiBufRd16
//
// This function is used to access a buffer as an array of 16-bit (UINT16)
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer, promoted to UINT32.
//
//------------------------------------------------------------------------------
UINT32 cspiClass::CspiBufRd16(LPVOID pBuf)
{
    UINT16 *p;

    p = (UINT16 *) pBuf;

    return *p;
}

//------------------------------------------------------------------------------
//
// Function: CspiBufRd32
//
// This function is used to access a buffer as an array of 32-bit (UINT32)
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer.
//
//------------------------------------------------------------------------------
UINT32 cspiClass::CspiBufRd32(LPVOID pBuf)
{
    UINT32 *p;

    p = (UINT32 *) pBuf;

    return *p;
}

//------------------------------------------------------------------------------
//
// Function: CspiBufWrt8
//
// This function is used to access a buffer as an array of 8-bit (UINT8)
// values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written demoted to UINT8.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void cspiClass::CspiBufWrt8(LPVOID pBuf, UINT32 data)
{
    UINT8 *p;

    p = (UINT8 *) pBuf;

   *p = (UINT8) data;
}

//------------------------------------------------------------------------------
//
// Function: CspiBufWrt16
//
// This function is used to access a buffer as an array of 16-bit (UINT16)
// values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written demoted to UINT16.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void cspiClass::CspiBufWrt16(LPVOID pBuf, UINT32 data)
{
    UINT16 *p;

    p = (UINT16 *) pBuf;

   *p = (UINT16) data;
}

//------------------------------------------------------------------------------
//
// Function: CspiBufWrt32
//
//      This function is used to access a buffer as an array of 32-bit (UINT32)
//      values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void cspiClass::CspiBufWrt32(LPVOID pBuf, UINT32 data)
{
    UINT32 *p;

    p = (UINT32 *) pBuf;

   *p = data;
}


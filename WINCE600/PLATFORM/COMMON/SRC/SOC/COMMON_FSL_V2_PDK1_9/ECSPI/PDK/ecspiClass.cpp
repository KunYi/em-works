//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//------------------------------------------------------------------------------
/*---------------------------------------------------------------------------
* Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//
//  File:  ecspiClass.cpp
//
//  Provides the implementation of the eCSPI bus driver to support eCSPI
//  transactions from multiple client drivers.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <linklist.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ddk.h"
#include "common_ecspi.h"
#include "ecspibus.h"
#include "ecspiClass.h"

#include <Devload.h>

//-----------------------------------------------------------------------------
// External Functions
extern "C" void BSPCSPICalculateDivRate(UINT32 dwFrequency, UINT32 dwTolerance, UINT8 *pPREDIV, UINT8 *pPOSTDIV);
extern "C" BOOL BSPCSPISetIOMux(UINT32 Index);
extern "C" BOOL BSPCSPIEnableClock(UINT32 Index, BOOL bEnable);
extern "C" BOOL BSPCSPIReleaseIOMux(UINT32 dwIndex);
extern "C" DWORD CspCSPIGetBaseRegAddr(UINT32 index);
extern "C" DWORD CspCSPIGetIRQ(UINT32 index);
extern "C" BOOL CspCheckPort(UINT32 Index);


extern "C" BOOL BSPCspiGetChannelPriority(UINT8 (*priority)[2]);
extern "C" DDK_DMA_REQ CspCSPIGetDmaReqTx(UINT32 index);
extern "C" DDK_DMA_REQ CspCSPIGetDmaReqRx(UINT32 index);
extern "C" BOOL BSPCspiIsDMAEnabled(UINT8 Index);
extern "C" BOOL BSPCspiAcquireGprBit(UINT8 Index);
extern "C" BOOL BSPCspiIsAllowPolling(UINT8 Index);
extern "C" BOOL BSPCspiExchange(VOID *lpInBuf, LPVOID pRxBuf, UINT32 nOutBufSize, LPDWORD BytesReturned);
extern "C" void BSPeCSPICSHigh(UINT32 Index, UINT8 CSPOL);


//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
static UINT8 dwSSPOL=0;

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions

inline HRESULT CeOpenCallerBuffer(
    CALLER_STUB_T& CallerStub,
    PVOID pSrcUnmarshalled,
    DWORD cbSrc,
    DWORD ArgumentDescriptor,
    BOOL ForceDuplicate)
{
    CallerStub.m_pCallerUnmarshalled =pSrcUnmarshalled;
    CallerStub.m_cbSize =cbSrc;
    CallerStub.m_ArgumentDescriptor =ArgumentDescriptor; 
    CallerStub.m_pLocalAsync = 0;

    return ::CeOpenCallerBuffer(&CallerStub.m_pLocalSyncMarshalled,
                pSrcUnmarshalled, cbSrc,
                ArgumentDescriptor, ForceDuplicate);
}

inline HRESULT CeCloseCallerBuffer(CALLER_STUB_T& CallerStub)
{
    return ::CeCloseCallerBuffer(
               CallerStub.m_pLocalSyncMarshalled,
               CallerStub.m_pCallerUnmarshalled,
               CallerStub.m_cbSize,
               CallerStub.m_ArgumentDescriptor); 
}

inline HRESULT CeAllocAsynchronousBuffer(CALLER_STUB_T& CallerStub)
{
    return ::CeAllocAsynchronousBuffer(
               &CallerStub.m_pLocalAsync,
               CallerStub.m_pLocalSyncMarshalled,
               CallerStub.m_cbSize,
               CallerStub.m_ArgumentDescriptor); 
}

inline HRESULT CeFreeAsynchronousBuffer(CALLER_STUB_T& CallerStub)
{
    return ::CeFreeAsynchronousBuffer(
               CallerStub.m_pLocalAsync,
               CallerStub.m_pLocalSyncMarshalled,
               CallerStub.m_cbSize,
               CallerStub.m_ArgumentDescriptor); 
}


cspiClass::cspiClass()
{
    DEBUGMSG(ZONE_INIT, (TEXT("cspiClass +\r\n")));
    m_pCSPI = NULL;
    m_cspiOpenCount = 0;
    m_hIntrEvent = NULL;
    m_hEnQEvent = NULL;
    m_hPowerEvent = NULL;
    m_hEnQSemaphore = NULL;
    m_hThread = NULL;
    m_hHeap = NULL;
    m_dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
    m_bTerminate = FALSE;
    m_dxCurrent = D0;
    m_Index = 0;
    m_bUsePolling = FALSE;
    m_bExchanging = FALSE;
    m_bAllowPolling = FALSE;
    DEBUGMSG(ZONE_INIT, (TEXT("cspiClass -\r\n")));
}

cspiClass::~cspiClass()
{
    DEBUGMSG(ZONE_INIT, (TEXT("~cspiClass\r\n")));
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
BOOL cspiClass::CspiInitialize(DWORD Index)
{
    PHYSICAL_ADDRESS phyAddr;
    DWORD irq;

    //Marley eCSPI2 needs specal operation since multi-used
    if(cspiClass::CheckPort()){
        return TRUE;
    }


    m_Index = Index;

    // create global heap for internal queues/buffers
    //      flOptions = 0 => no options
    //      dwInitialSize = 0 => zero bytes initial size
    //      dwMaximumSize = 0 => heap size limited only by available memory
    DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize: m_hHeap=0x%x\r\n"),m_hHeap));
    m_hHeap = HeapCreate(0, 0, 0);

    // check if HeapCreate failed
    if (m_hHeap == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  HeapCreate failed!\r\n")));
        goto Error;
    }

    // create event for CSPI interrupt signaling
    //      pEventAttributes = NULL (must be NULL)
    //      bManualReset = FALSE => resets automatically to nonsignaled
    //                              state after waiting thread released
    //      bInitialState = FALSE => initial state is non-signaled
    //      lpName = NULL => object created without a name
    m_hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // check if CreateEvent failed
    if (m_hIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  CreateEvent failed!\r\n")));
        goto Error;
    }

    // create event for process queue thread signaling
    //      pEventAttributes = NULL (must be NULL)
    //      bManualReset = FALSE => resets automatically to nonsignaled
    //                              state after waiting thread released
    //      bInitialState = FALSE => initial state is non-signaled
    //      lpName = NULL => object created without a name
    m_hEnQEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // check if CreateEvent failed
    if (m_hEnQEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  CreateEvent failed!\r\n")));
        goto Error;
    }

    m_hPowerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // check if CreateEvent failed
    if (m_hPowerEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  CreateEvent failed!\r\n")));
        goto Error;
    }

    // create Semaphore for process queue thread signaling
    //      pEventAttributes = NULL (must be NULL)
    //      lpName = NULL => object created without a name
    m_hEnQSemaphore = CreateSemaphore(NULL, CSPI_MAX_QUEUE_LENGTH, CSPI_MAX_QUEUE_LENGTH, NULL);

    // check if CreateEvent failed
    if (m_hEnQSemaphore == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  CreateSemaphore failed!\r\n")));
        goto Error;
    }

    // Get the Base Address and IRQ according to Index
    phyAddr.QuadPart = CspCSPIGetBaseRegAddr(Index);

    irq = CspCSPIGetIRQ(Index);
    if(!irq || !phyAddr.QuadPart)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  invalid CSPI instance!\r\n")));
        goto Error;
    }
    m_bUseDMA = BSPCspiIsDMAEnabled((UINT8)Index);
    if (m_bUseDMA)
    {
        InitCspiDMA(Index);
        BSPCspiAcquireGprBit((UINT8)Index);
    }

    //m_bAllowPolling = BSPCspiIsAllowPolling((UINT8)Index);
    m_bAllowPolling = FALSE;

    // Configure IOMUX 
    BSPCSPISetIOMux(Index);

    // Map peripheral physical address to virtual address
    m_pCSPI = (PCSP_ECSPI_REG) MmMapIoSpace(phyAddr, sizeof(CSP_ECSPI_REG),
        FALSE);

    // Check if virtual mapping failed
    if (m_pCSPI == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  MmMapIoSpace failed!\r\n")));
        goto Error;
    }

    // Call the OAL to translate the IRQ into a SysIntr value.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
        &m_dwSysIntr, sizeof(DWORD), NULL))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: Failed to obtain sysintr value for CSPI interrupt.\r\n")));
        goto Error;
    }

    // register CSPI interrupt
    if (!InterruptInitialize(m_dwSysIntr, m_hIntrEvent, NULL, 0))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  InterruptInitialize failed!\r\n")));
        goto Error;
    }

    // create CSPI critical section
    InitializeCriticalSection(&m_cspiCs);

    // create CSPI Data Exchange critical section
    InitializeCriticalSection(&m_cspiDataXchCs);

    // initialize CSPI linked list of client queue entries
    InitializeListHead(&m_ListHead);

    DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize: &m_pCSPI=0x%x\r\n"),&m_pCSPI));

    // enable the clock gating
    BSPCSPIEnableClock(m_Index,TRUE);
    
    // disable CSPI to reset internal logic
    INSREG32(&m_pCSPI->CONTROLREG, CSP_BITFMASK(CSPI_CONTROLREG_EN),
        CSP_BITFVAL(CSPI_CONTROLREG_EN, CSPI_CONTROLREG_EN_DISABLE));

    // create CSPI queue processing thread if it does not exist
    if (!m_hThread)
    {
        // set global termination flag
        m_bTerminate=FALSE;

        // create processing thread
        //      pThreadAttributes = NULL (must be NULL)
        //      dwStackSize = 0 => default stack size determined by linker
        //      lpStartAddress = CspiProcessQueue => thread entry point
        //      lpParameter = NULL => point to thread parameter
        //      dwCreationFlags = 0 => no flags
        //      lpThreadId = NULL => thread ID is not returned
        m_hThread = ::CreateThread(NULL, 0, CspiProcessQueue, this, 0, NULL);
        DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize:  this=0x%x\r\n"),this));

        // check if CreateThread failed
        if (m_hThread == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  CreateThread failed!\r\n")));
            goto Error;
        }
    }

    m_cspiOpenCount++;    

    // By default, Loopback is disabled.
    CspiEnableLoopback(FALSE);
    // Disable the clock gating
    BSPCSPIEnableClock(m_Index,FALSE);

    return TRUE;

Error:
    CspiRelease();

    return FALSE;
}


//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void cspiClass::CspiRelease()
{
    DEBUGMSG(ZONE_CLOSE, (TEXT("CspiRelease +\r\n")));

    // kill the exchange packet processing thread
    if (m_hThread)
    {
        m_bTerminate=TRUE;
        // try to signal the thread so that it can wake up and terminate
        if (m_hEnQEvent)
        {
            SetEvent(m_hEnQEvent);
        }
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    // Release IOMUX pins
    BSPCSPIReleaseIOMux(m_Index);
    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwSysIntr, sizeof(DWORD), NULL, 0, NULL);
    m_dwSysIntr = (DWORD)SYSINTR_UNDEFINED;

    // destroy the heap
    if (m_hHeap != NULL)
    {
        HeapDestroy(m_hHeap);
        m_hHeap = NULL;
    }

    // close interrupt event handle
    if (m_hIntrEvent)
    {
        CloseHandle(m_hIntrEvent);
        m_hIntrEvent = NULL;
    }

    // close interrupt event handle
    if (m_hPowerEvent)
    {
        CloseHandle(m_hPowerEvent);
        m_hPowerEvent = NULL;
    }

    // close enqueue event handle
    if (m_hEnQEvent)
    {
        CloseHandle(m_hEnQEvent);
        m_hEnQEvent = NULL;
    }

    // close enqueue Semaphore handle
    if (m_hEnQSemaphore)
    {
        CloseHandle(m_hEnQSemaphore);
        m_hEnQSemaphore = NULL;
    }

    // free the virtual space allocated for CSPI memory map
    if (m_pCSPI != NULL)
    {
        MmUnmapIoSpace(m_pCSPI, sizeof(CSP_ECSPI_REG));
        m_pCSPI = NULL;
    }
    
    // deregister the system interrupt
    if (m_dwSysIntr != SYSINTR_UNDEFINED)
    {
        InterruptDisable(m_dwSysIntr);
        m_dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
    }

    // delete the critical section
    DeleteCriticalSection(&m_cspiCs);

    // delete the Data Exchange critical section
    DeleteCriticalSection(&m_cspiDataXchCs);

    DEBUGMSG(ZONE_CLOSE, (TEXT("CspiRelease -\r\n")));
    
}


//-----------------------------------------------------------------------------
//
// Function: CspiEnqueue
//
// This function is invoked as a result of a CSPI client driver calling
// the CSPI DeviceIoControl with CSPI_IOCTL_EXCHANGE. This function
//
//
// Parameters:
//      pData 
//          [in] 
//
// Returns:
//      Returns a handle to the newly created msg queue, or NULL if the
//      msg queue creation failed.
//
//-----------------------------------------------------------------------------
BOOL cspiClass::CspiEnqueue(PCSPI_XCH_PKT_T pXchPkt)
{
    PCSPI_XCH_LIST_ENTRY_T pXchListEntry;
    DWORD cbSrc;
    HRESULT result;
    struct {
        BOOL bAllocListEntry;
        BOOL bMarshalpBusCnfg;
        BOOL bMarshalTxBuf;
        BOOL bMarshalRxBuf;
        BOOL bMarshalTxBufAsync;
        BOOL bMarshalRxBufAsync;
        BOOL bMarshalpBusCnfgAsync;
    } resource;
    CALLER_STUB_T marshalEventStub;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CspiEnqueue +\r\n")));

    resource.bAllocListEntry = FALSE;
    resource.bMarshalpBusCnfg = FALSE;
    resource.bMarshalTxBuf = FALSE;
    resource.bMarshalRxBuf = FALSE;
    resource.bMarshalTxBufAsync = FALSE;
    resource.bMarshalRxBufAsync = FALSE;
    resource.bMarshalpBusCnfgAsync = FALSE;

    pXchListEntry = NULL;

    // allocate space for new list entry
    pXchListEntry = (PCSPI_XCH_LIST_ENTRY_T) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, 
        sizeof(CSPI_XCH_LIST_ENTRY_T));

    // check if HeapAlloc failed (fatal error, terminate thread)
    if (pXchListEntry == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiProcessQueue:  HeapAlloc failed!\r\n")));
        return FALSE;
    }
    resource.bAllocListEntry = TRUE;

    result = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);

    // Open caller Bus Config buffer.
    result = CeOpenCallerBuffer(
                pXchListEntry->xchPkt.marshalBusCnfgStub,
                pXchPkt->pBusCnfg,
                sizeof(CSPI_BUSCONFIG_T),
                ARG_I_PTR,
                TRUE);

    if (!SUCCEEDED(result)) 
    {
        goto error_cleanup;
    }
    resource.bMarshalpBusCnfg = TRUE;

    // Copy data check config
    pXchListEntry->xchPkt.xchRealPkt.pBusCnfg =(PCSPI_BUSCONFIG_T)pXchListEntry->xchPkt.marshalBusCnfgStub.m_pLocalSyncMarshalled;
    // check translated pointer
    if (pXchListEntry->xchPkt.xchRealPkt.pBusCnfg == NULL)
    {
        goto error_cleanup;
    }

    // Copy data into list entry
    pXchListEntry->xchPkt.xchRealPkt.xchCnt = pXchPkt->xchCnt;

    cbSrc = CspiExchangeSize(&pXchListEntry->xchPkt.xchRealPkt);
    if (cbSrc == 0)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiProcessQueue:  Unsupported exchange size!\r\n")));
        goto error_cleanup;
    }

    if (cbSrc > CSPI_SDMA_BUFFER_SIZE/2)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiProcessQueue:  Exceed exchange size!\r\n")));
        goto error_cleanup;
    }

    // Translate event from event name
    pXchListEntry->xchPkt.xchRealPkt.xchEvent = NULL;
    if (pXchPkt->xchEventLength>0 && pXchPkt->xchEventLength<= CSPI_MAXENQ_EVENT_NAME)
    {
        if (pXchPkt->xchEvent)
        {
            result = CeOpenCallerBuffer(
                marshalEventStub,
                pXchPkt->xchEvent,
                pXchPkt->xchEventLength,
                ARG_I_PTR,
                FALSE);

            if (!SUCCEEDED(result)) 
            {
                goto error_cleanup;
            }

            pXchListEntry->xchPkt.xchRealPkt.xchEvent = 
                CreateEvent(NULL,TRUE, FALSE, pXchPkt->xchEvent);

            CeCloseCallerBuffer(marshalEventStub);

            if (pXchListEntry->xchPkt.xchRealPkt.xchEvent == NULL)
            {
                goto error_cleanup;
            }
        }
    }

    // Open caller Tx buffer.
    result = CeOpenCallerBuffer(
                pXchListEntry->xchPkt.marshalTxStub,
                pXchPkt->pTxBuf,
                cbSrc,
                ARG_I_PTR,
                TRUE);

    if (!SUCCEEDED(result)) 
    {
        goto error_cleanup;
    }
    resource.bMarshalTxBuf = TRUE;
    pXchListEntry->xchPkt.xchRealPkt.pTxBuf = pXchListEntry->xchPkt.marshalTxStub.m_pLocalSyncMarshalled;

    if (pXchPkt->pRxBuf != NULL)
    {
        // Open caller Rx buffer.
        result = CeOpenCallerBuffer(
                    pXchListEntry->xchPkt.marshalRxStub,
                    pXchPkt->pRxBuf,
                    cbSrc,
                    ARG_O_PTR,
                    TRUE);

        if (!SUCCEEDED(result)) 
        {
            goto error_cleanup;
        }
        resource.bMarshalRxBuf = TRUE;
        pXchListEntry->xchPkt.xchRealPkt.pRxBuf = pXchListEntry->xchPkt.marshalRxStub.m_pLocalSyncMarshalled;
    }
    else 
    {
        // Ignore Rx data.
        pXchListEntry->xchPkt.xchRealPkt.pRxBuf = NULL;
    }

    // Check whether the completion event is NULL
    // If it's NULL, then send the package immediately
    if (pXchListEntry->xchPkt.xchRealPkt.xchEvent == NULL)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("CspiEnqueue: Sending the package immediately\r\n")));

        // Wrap the exchange in critical section to 
        // serialize accesses with CspiProcessQueue thread
        EnterCriticalSection(&m_cspiDataXchCs);

        if (m_bUseDMA && pXchListEntry->xchPkt.xchRealPkt.pBusCnfg->usedma && !(pXchListEntry->xchPkt.xchRealPkt.xchCnt>64 && pXchListEntry->xchPkt.xchRealPkt.xchCnt%4))
        {
            pXchListEntry->xchPkt.xchRealPkt.xchCnt = CspiDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
        }
        else
        {
            pXchListEntry->xchPkt.xchRealPkt.xchCnt = CspiNonDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
        }

        LeaveCriticalSection(&m_cspiDataXchCs);
            
        if (resource.bMarshalRxBuf)
        {
            CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalRxStub);
        }
        CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalTxStub);
        CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
        HeapFree(m_hHeap, 0, pXchListEntry);
        return TRUE;
    }

    // wait for system leaveing power down state
    while (m_dxCurrent == D4)
    {
        Sleep(1000);
    }

    // wait for cspi asynchronous queue availiable
    WaitForSingleObject(m_hEnQSemaphore, INFINITE);

    // wrap linked list insert operation in critical section to 
    // serialize accesses with CspiProcessQueue thread
    EnterCriticalSection(&m_cspiCs);
    
    // insert exchange packet into our list
#pragma warning(push)
#pragma warning(disable: 4127)
    InsertTailList(&m_ListHead, &(pXchListEntry->link));
#pragma warning(pop)

    LeaveCriticalSection(&m_cspiCs);
        
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CspiEnqueue -\r\n")));

    // signal queue processing thread that a new packet is ready
    return SetEvent(m_hEnQEvent);

error_cleanup:
    if (resource.bMarshalpBusCnfg)
    {
        CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
    }
    if (resource.bMarshalRxBuf)
    {
        CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalRxStub);
    }
    if (resource.bMarshalTxBuf)
    {
        CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalTxStub);
    }
    if (resource.bAllocListEntry)
    {
        HeapFree(m_hHeap, 0, pXchListEntry);
    }
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: CspiProcessQueue
//
// This function is the entry point for a thread that will be created
// during CSPI driver initialization and remain resident to process
// packet exchange requests from client devices.
//
// Parameters:
//      lpParameter 
//          [in] Pointer to a single 32-bit parameter value passed to the 
//          thread during creation.  Currently not used.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
DWORD WINAPI cspiClass::CspiProcessQueue(LPVOID lpParameter)
{
    PCSPI_XCH_LIST_ENTRY_T pXchListEntry;
    cspiClass * pCspi = (cspiClass *)lpParameter;
    DEBUGMSG(ZONE_THREAD, (TEXT("CspiProcessQueue:  lpParameter=0x%x\r\n"),lpParameter));

    // until queue processing thread termination requested
    while (!pCspi->m_bTerminate)
    {
        // while system is powering down 
        while (pCspi->m_dxCurrent == D4)
        {
            Sleep(1000);
        }

        // while our list of unprocessed queues has not been exhausted
        while (!IsListEmpty(&pCspi->m_ListHead)) 
        {
            // wrap linked list remove operation in critical section to 
            // serialize accesses with CSPI_IOCTL_EXCHANGE
            EnterCriticalSection(&pCspi->m_cspiCs);

            // get next list entry
            pXchListEntry = (PCSPI_XCH_LIST_ENTRY_T) RemoveHeadList(&pCspi->m_ListHead);
        
            LeaveCriticalSection(&pCspi->m_cspiCs);
        
            // check if mapping failed
            if (pXchListEntry == NULL)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("CspiProcessQueue:  MapCallerPtr failed!\r\n")));
            }
            // exchange packet mapped to our space
            else
            {
                // Wrap the exchange in critical section to 
                // serialize accesses with CSPI_IOCTL_EXCHANGE
                EnterCriticalSection(&pCspi->m_cspiDataXchCs);

                // Indicate that exchanging data is working
                pCspi->m_bExchanging = TRUE;

                // do the exchange and update the exchange count
                // Because TAIL function has silicon issue, If the burstlength is more than 64 words and not multiple of 4, DMA transferring will be forbidden
                if (pCspi->m_bUseDMA && pXchListEntry->xchPkt.xchRealPkt.pBusCnfg->usedma && !(pXchListEntry->xchPkt.xchRealPkt.xchCnt>64 && pXchListEntry->xchPkt.xchRealPkt.xchCnt%4))
                {
                    pXchListEntry->xchPkt.xchRealPkt.xchCnt = pCspi->CspiDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
                }
                else
                {
                    pXchListEntry->xchPkt.xchRealPkt.xchCnt = pCspi->CspiNonDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
                }


                // If prepare to go into D4 powerstate, stop to transfer after finishing this data exchanging
                if( pCspi->m_dxCurrent == D4 )
                {
                    // Tell Power dealing function into D4
                    PulseEvent(pCspi->m_hPowerEvent);
                    Sleep(100);
                    WaitForSingleObject(pCspi->m_hPowerEvent, INFINITE);
                }

                // Indicate that exchanging data is over
                pCspi->m_bExchanging = FALSE;

                LeaveCriticalSection(&pCspi->m_cspiDataXchCs);

                CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
                if (pXchListEntry->xchPkt.xchRealPkt.pRxBuf != NULL)
                {
                    CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalRxStub);
                }
                CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalTxStub);

                ReleaseSemaphore(pCspi->m_hEnQSemaphore, 1, NULL);

                // signal that new data available in Rx message queue
                if(!SetEvent(pXchListEntry->xchPkt.xchRealPkt.xchEvent))
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("CspiProcessQueue: SetEvent failed\r\n")));
                } 
                CloseHandle(pXchListEntry->xchPkt.xchRealPkt.xchEvent);

                // free memory allocated to list entry
                HeapFree(pCspi->m_hHeap, 0, pXchListEntry);
            }
        } // while (!IsListEmpty(&m_ListHead))

        // wait for next list entry to arrive
        WaitForSingleObject(pCspi->m_hEnQEvent, INFINITE);

    }  // while (!m_bTerminate)

    pCspi->m_hThread = NULL;

    DEBUGMSG(ZONE_THREAD, (TEXT("CspiProcessQueue -\r\n")));
    
    return TRUE;
}




//-----------------------------------------------------------------------------
//
// Function: CspiNonDMADataExchange
//
// Exchanges CSPI data in Application Processor(CPU) Master mode.
//
// Parameters:
//      pXchPkt
//          [in] Points to exchange packet information.
//
// Returns:
//      Returns the number of data exchanges that completed successfully.
//   
//-----------------------------------------------------------------------------
UINT32 cspiClass::CspiNonDMADataExchange(PCSPI_XCH_PKT0_T pXchPkt)
{
    enum {
        LOAD_TXFIFO, 
        MAX_XCHG, 
        FETCH_RXFIFO
    } xchState;
    PCSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;
    LPVOID pTxBuf = pXchPkt->pTxBuf;
    LPVOID pRxBuf = pXchPkt->pRxBuf;
    UINT32 xchTxCnt = 0;
    UINT32 xchRxCnt = 0;
    volatile UINT32 tmp;
    BOOL bXchDone;
    BOOL bReqPolling;
    UINT32 (*pfnTxBufRd)(LPVOID);
    void (*pfnRxBufWrt)(LPVOID, UINT32);
    UINT8 bufIncr;
    UINT8 byParamChannelSelect;
    UINT8 byParamSSPOL;
    UINT8 byParamPOL;
    UINT8 byParamPHA;
    UINT8 byParamDRCTL;
    UINT8 PreDiv=0;
    UINT8 PostDiv=0;

    DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange: &m_pCSPI=0x%x,bitcount=%d\r\n"),&m_pCSPI,pBusCnfg->BurstLength));
    // check all translated pointers
    if ((pBusCnfg == NULL) || (pTxBuf == NULL))
    {
        return 0;
    }

    // select access funtions based on exchange bit width
    //
    // bitcount        Tx/Rx Buffer Access Width
    // --------        -------------------------
    //   1 - 8           UINT8 (unsigned 8-bit)
    //   9 - 16          UINT16 (unsigned 16-bit)
    //  17 - 32          UINT32 (unsigned 32-bit)
    //
    if ((pBusCnfg->BurstLength >= 1) && (pBusCnfg->BurstLength <= 8))
    {
        // 8-bit access width
        pfnTxBufRd = CspiBufRd8;
        pfnRxBufWrt = CspiBufWrt8;
        bufIncr = sizeof(UINT8);
    }
    else if ((pBusCnfg->BurstLength >= 9) && (pBusCnfg->BurstLength <= 16))
    {
        // 16-bit access width
        pfnTxBufRd = CspiBufRd16;
        pfnRxBufWrt = CspiBufWrt16;
        bufIncr = sizeof(UINT16);
    }
    else if (pBusCnfg->BurstLength >= 17)
    {
        // 32-bit access width
        pfnTxBufRd = CspiBufRd32;
        pfnRxBufWrt = CspiBufWrt32;
        bufIncr = sizeof(UINT32);
    }
    else
    {
        // unsupported access width
        DEBUGMSG(ZONE_WARN, (TEXT("CspiMasterDataExchange:  unsupported BurstLength!\r\n")));
        return 0;
    }

    // Enable the clock gating
    BSPCSPIEnableClock(m_Index,TRUE);

    // disable all interrupts
    OUTREG32(&m_pCSPI->INTREG, 0);

    // set client CSPI bus configuration based
    //  default EN = disabled
    //  default MODE = master
    //  default XCH = idle
    //  default SMC = XCH bit controls master start transfer
    byParamChannelSelect= pBusCnfg->ChannelSelect &((1U<<CSPI_CONTROLREG_CHANNELSELECT_WID) -1);
    byParamSSPOL = pBusCnfg->SSPOL? CSPI_CONFIGREG_SSBPOL_ACTIVEHIGH: CSPI_CONFIGREG_SSBPOL_ACTIVELOW;
    byParamPOL = pBusCnfg->SCLKPOL? CSPI_CONFIGREG_SCLKPOL_ACTIVELOW: CSPI_CONFIGREG_SCLKPOL_ACTIVEHIGH;
    byParamPHA = pBusCnfg->SCLKPHA? CSPI_CONFIGREG_SCLKPHA_PHASE1: CSPI_CONFIGREG_SCLKPHA_PHASE0;
    byParamDRCTL = pBusCnfg->DRCTL&((1U<<CSPI_CONTROLREG_DRCTL_WID) -1);
    bReqPolling = m_bAllowPolling;

    // ENGcm09397
    dwSSPOL &= ~(1<<byParamChannelSelect);

    BSPeCSPICSHigh(m_Index,dwSSPOL);
    
    dwSSPOL |= byParamSSPOL<<byParamChannelSelect;
    // ENGcm09397 end

    BSPCSPICalculateDivRate(pBusCnfg->Freq, pBusCnfg->Freq/10, &PreDiv,  &PostDiv);

    OUTREG32(&m_pCSPI->CONTROLREG, 
        CSP_BITFVAL(CSPI_CONTROLREG_EN, CSPI_CONTROLREG_EN_ENABLE)              |
        CSP_BITFVAL(CSPI_CONTROLREG_HW, CSPI_CONTROLREG_HW_HTMODE_DISABLE)      |
        CSP_BITFVAL(CSPI_CONTROLREG_XCH, CSPI_CONTROLREG_XCH_IDLE)              |
        CSP_BITFVAL(CSPI_CONTROLREG_SMC, CSPI_CONTROLREG_SMC_NORMAL_MODE)       |
        CSP_BITFVAL(CSPI_CONTROLREG_CHANNELMODE, CSPI_CONTROLREG_CHANNELMODE_MASTER<<byParamChannelSelect)|
        CSP_BITFVAL(CSPI_CONTROLREG_PREDIVIDER, PreDiv)                         |
        CSP_BITFVAL(CSPI_CONTROLREG_POSTDIVIDER, PostDiv)                       |
        CSP_BITFVAL(CSPI_CONTROLREG_DRCTL, byParamDRCTL)                        |
        CSP_BITFVAL(CSPI_CONTROLREG_CHANNELSELECT, byParamChannelSelect)        |
        CSP_BITFVAL(CSPI_CONTROLREG_BURSTLENGTH, pBusCnfg->BurstLength-1));

    OUTREG32(&m_pCSPI->CONFIGREG,
        CSP_BITFVAL(CSPI_CONFIGREG_SCLKPHA, byParamPHA<<byParamChannelSelect)   |
        CSP_BITFVAL(CSPI_CONFIGREG_SCLKPOL, byParamPOL<<byParamChannelSelect)   |
        CSP_BITFVAL(CSPI_CONFIGREG_SSBCTRL, CSPI_CONFIGREG_SSBCTRL_SINGLEBURST) |
        CSP_BITFVAL(CSPI_CONFIGREG_SSBPOL, dwSSPOL)                             |//byParamSSPOL<<byParamChannelSelect)  |
        CSP_BITFVAL(CSPI_CONFIGREG_DATACTL, CSPI_CONFIGREG_DATACTL_STAYHIGH)    |
        CSP_BITFVAL(CSPI_CONFIGREG_SCLKCTL, CSPI_CONFIGREG_SCLKCTL_STAYHIGH)    |
        CSP_BITFVAL(CSPI_CONFIGREG_HTLENGTH, 0x0));

    if(m_bUseLoopBack)
    {
        INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_INTERNALLYCONNECTED);
    }
    else
    {
        INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_NOTCONNECTED);
    }

    bXchDone = FALSE;
    xchState = LOAD_TXFIFO;

    // until we are done with requested transfers
    while(!bXchDone)
    {
        switch (xchState)
        {
            case LOAD_TXFIFO: 

                // load Tx FIFO until full, or until we run out of data
                while ((!(INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TF)))
                    && (xchTxCnt < pXchPkt->xchCnt))
                {
                        // put next Tx data into CSPI FIFO
                        OUTREG32(&m_pCSPI->TXDATA, pfnTxBufRd(pTxBuf));

                        // increment Tx Buffer to next data point
                        pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);

                        // increment Tx exchange counter
                        xchTxCnt++;
                }
                
                // start exchange
                INSREG32(&m_pCSPI->CONTROLREG, CSP_BITFMASK(CSPI_CONTROLREG_XCH), 
                    CSP_BITFVAL(CSPI_CONTROLREG_XCH, CSPI_CONTROLREG_XCH_EN));

                xchState = (xchTxCnt<pXchPkt->xchCnt)? MAX_XCHG: FETCH_RXFIFO;

                break;

            case MAX_XCHG:

                if (xchTxCnt >= pXchPkt->xchCnt)
                {
                    xchState = FETCH_RXFIFO;
                    break;
                }
                // we need to wait until FIFO has more room, so we enable 
                // interrupt for Rx FIFO half-full (RHEN) or Rx FIFO ready 
                // to ensure we can
                // read out data that arrived during exchange
                if(m_bUsePolling || bReqPolling)
                {
                    // wait until RR
                    while (!(INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
                        ;
                    tmp = INREG32(&m_pCSPI->RXDATA);

                    // if receive data is not to be discarded
                    if (pRxBuf != NULL)
                    {
                        // get next Rx data from CSPI FIFO
                        pfnRxBufWrt(pRxBuf, tmp);

                        // increment Rx Buffer to next data point
                        pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
                    }

                    // increment Rx exchange counter
                    xchRxCnt++;

                    OUTREG32(&m_pCSPI->TXDATA, pfnTxBufRd(pTxBuf));

                    // increment Tx Buffer to next data point
                    pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);

                    // increment Tx exchange counter
                    xchTxCnt++;
                }
                else
                {
                    // wait until RH, but wait RR is also OK. 
                    INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_RREN), 
                        CSP_BITFVAL(CSPI_INTREG_RREN, CSPI_INTREG_RREN_ENABLE));

                    WaitForSingleObject(m_hIntrEvent, INFINITE);

                    // disable all interrupts
                    OUTREG32(&m_pCSPI->INTREG, 0);

                    while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
                    {
                        tmp = INREG32(&m_pCSPI->RXDATA);

                        // if receive data is not to be discarded
                        if (pRxBuf != NULL)
                        {
                            // get next Rx data from CSPI FIFO
                            pfnRxBufWrt(pRxBuf, tmp);

                            // increment Rx Buffer to next data point
                            pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
                        }

                        // increment Rx exchange counter
                        xchRxCnt++;

                        OUTREG32(&m_pCSPI->TXDATA, pfnTxBufRd(pTxBuf));

                        // increment Tx Buffer to next data point
                        pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);

                        // increment Tx exchange counter
                        xchTxCnt++;

                        if (xchTxCnt >= pXchPkt->xchCnt)
                        {
                            break;
                        }
                    }
                    // signal that interrupt has been handled
                    InterruptDone(m_dwSysIntr);
                }

                if (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC))
                {
                    xchState = FETCH_RXFIFO;
                }
                break;

            case FETCH_RXFIFO:

                // Fetch all rxdata already in RXFIFO
                while ((INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
                {
                    tmp = INREG32(&m_pCSPI->RXDATA);

                    // if receive data is not to be discarded
                    if (pRxBuf != NULL)
                    {
                        // get next Rx data from CSPI FIFO
                        pfnRxBufWrt(pRxBuf, tmp);

                        // increment Rx Buffer to next data point
                        pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
                    }

                    // increment Rx exchange counter
                    xchRxCnt++;
                }

                // Wait all data in the chain exchaged
                if(!(m_bUsePolling || bReqPolling))
                {
                    INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_TCEN), 
                        CSP_BITFVAL(CSPI_INTREG_TCEN, CSPI_INTREG_TCEN_ENABLE));

                    WaitForSingleObject(m_hIntrEvent, INFINITE);

                    // disable all interrupts
                    OUTREG32(&m_pCSPI->INTREG, 0);
                }
                else
                {
                    // wait until transaction is complete
                    while (!(INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC)))
                        ;
                }

                while ((INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
                {
                    tmp = INREG32(&m_pCSPI->RXDATA);

                    // if receive data is not to be discarded
                    if (pRxBuf != NULL)
                    {
                        // get next Rx data from CSPI FIFO
                        pfnRxBufWrt(pRxBuf, tmp);

                        // increment Rx Buffer to next data point
                        pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
                    }

                    // increment Rx exchange counter
                    xchRxCnt++;
                }

                // acknowledge transfer complete (w1c)
                OUTREG32(&m_pCSPI->STATREG, CSP_BITFMASK(CSPI_STATREG_TC));

                if(!(m_bUsePolling|| bReqPolling))
                {
                    // signal that interrupt has been handled
                    InterruptDone(m_dwSysIntr);
                }
                
                if (xchRxCnt >= pXchPkt->xchCnt)
                {
                    // set flag to indicate requested exchange done
                    bXchDone = TRUE;
                }
                else 
                {
                    // exchange stopped, and we have received all data in RXFIFO
                    // then there MUST be some data in the TxBuf, or in the TXFIFO,
                    // or both.
                    // we return the state to restart cspi XCH. 
                    xchState = LOAD_TXFIFO;
                }

                break;

            default:
                bXchDone = TRUE;
                break;
        }
    } // while(!bXchDone)
    
    // disable the CSPI
    INSREG32(&m_pCSPI->CONTROLREG, CSP_BITFMASK(CSPI_CONTROLREG_EN), 
        CSP_BITFVAL(CSPI_CONTROLREG_EN, CSPI_CONTROLREG_EN_DISABLE));

    // Disable the clock gating
    BSPCSPIEnableClock(m_Index,FALSE);
    DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange -\r\n")));

    return xchRxCnt;
}


//-----------------------------------------------------------------------------
//
// Function: CspiDMADataExchange
//
// Exchanges CSPI data with DMA in Master mode.
//
// Parameters:
//      pXchPkt
//          [in] Points to exchange packet information.
//
// Returns:
//      Returns the number of data exchanges that completed successfully.
//   
//-----------------------------------------------------------------------------
UINT32 cspiClass::CspiDMADataExchange(PCSPI_XCH_PKT0_T pXchPkt)
{
    PCSPI_BUSCONFIG_T pBusCnfg;
    LPVOID pTxBuf;
    LPVOID pRxBuf;
    UINT32 xchTxCnt;
    UINT32 xchRxCnt;
    UINT32 dwBurstCount;

    UINT32 dmaTransferCount;
    UINT uiRxStatus;
    UINT uiTxStatus;
    volatile UINT32 tmp;
    BOOL bTxDone;
    BOOL bReqPolling;
    UINT8 byParamChannelSelect;
    UINT8 byParamSSPOL;
    UINT8 byParamPOL;
    UINT8 byParamPHA;
    UINT8 byParamDRCTL;
    UINT8 PreDiv=0;
    UINT8 PostDiv=0;

    DEBUGMSG(ZONE_THREAD, (TEXT("CspiDMADataExchange: &m_pCSPI=0x%x\r\n"),&m_pCSPI));

    pBusCnfg = pXchPkt->pBusCnfg;
    pTxBuf = pXchPkt->pTxBuf;
    pRxBuf = pXchPkt->pRxBuf;

    // check all translated pointers
    if ((pBusCnfg == NULL) || (pTxBuf == NULL))
    {
        return 0;
    }

    if (pXchPkt->xchCnt<=8)
    {
        xchRxCnt = CspiNonDMADataExchange(pXchPkt);
        return xchRxCnt;
    }

    xchTxCnt = 0;
    xchRxCnt = 0;
    uiRxStatus = 0;
    uiTxStatus = 0;
    bTxDone = FALSE;

    // select access funtions based on exchange bit width
    if (pBusCnfg->BurstLength < 17)
    {
        // unsupported access width
        DEBUGMSG(ZONE_WARN, (TEXT("CspiDMADataExchange:  unsupported DMA mode BurstLength!\r\n")));
        return 0;
    }

    // Enable the clock gating
    BSPCSPIEnableClock(m_Index,TRUE);

    // disable all interrupts
    OUTREG32(&m_pCSPI->INTREG, 0);

    // set client CSPI bus configuration based
    //  default EN = disabled
    //  default MODE = master
    //  default XCH = idle
    //  default SMC = XCH bit controls master start transfer
    byParamChannelSelect= pBusCnfg->ChannelSelect &((1U<<CSPI_CONTROLREG_CHANNELSELECT_WID) -1);
    byParamSSPOL = pBusCnfg->SSPOL? CSPI_CONFIGREG_SSBPOL_ACTIVEHIGH: CSPI_CONFIGREG_SSBPOL_ACTIVELOW;
    byParamPOL = pBusCnfg->SCLKPOL? CSPI_CONFIGREG_SCLKPOL_ACTIVELOW: CSPI_CONFIGREG_SCLKPOL_ACTIVEHIGH;
    byParamPHA = pBusCnfg->SCLKPHA? CSPI_CONFIGREG_SCLKPHA_PHASE1: CSPI_CONFIGREG_SCLKPHA_PHASE0;
    byParamDRCTL = pBusCnfg->DRCTL&((1U<<CSPI_CONTROLREG_DRCTL_WID) -1);
    bReqPolling = m_bAllowPolling;

    // ENGcm09397
    dwSSPOL &= ~(1<<byParamChannelSelect);

    BSPeCSPICSHigh(m_Index,dwSSPOL);
    
    dwSSPOL |= byParamSSPOL<<byParamChannelSelect;
    // ENGcm09397 end
    
    // Reset eCSPI register
    OUTREG32(&m_pCSPI->CONTROLREG, CSP_BITFVAL(CSPI_CONTROLREG_EN, CSPI_CONTROLREG_EN_DISABLE)); //reset

    BSPCSPICalculateDivRate(pBusCnfg->Freq, pBusCnfg->Freq/10, &PreDiv,  &PostDiv);

    OUTREG32(&m_pCSPI->CONTROLREG, 
        CSP_BITFVAL(CSPI_CONTROLREG_EN, CSPI_CONTROLREG_EN_ENABLE)         |
        CSP_BITFVAL(CSPI_CONTROLREG_HW, CSPI_CONTROLREG_HW_HTMODE_DISABLE) |
        CSP_BITFVAL(CSPI_CONTROLREG_XCH, CSPI_CONTROLREG_XCH_IDLE)         |
        CSP_BITFVAL(CSPI_CONTROLREG_SMC, CSPI_CONTROLREG_SMC_NORMAL_MODE)  |
        CSP_BITFVAL(CSPI_CONTROLREG_CHANNELMODE, CSPI_CONTROLREG_CHANNELMODE_MASTER<<byParamChannelSelect)|
        CSP_BITFVAL(CSPI_CONTROLREG_PREDIVIDER, PreDiv)                    |
        CSP_BITFVAL(CSPI_CONTROLREG_POSTDIVIDER, PostDiv)                  |
        CSP_BITFVAL(CSPI_CONTROLREG_DRCTL, byParamDRCTL)                   |
        CSP_BITFVAL(CSPI_CONTROLREG_CHANNELSELECT, byParamChannelSelect)   |
        CSP_BITFVAL(CSPI_CONTROLREG_BURSTLENGTH, pBusCnfg->BurstLength-1));

    OUTREG32(&m_pCSPI->CONFIGREG,
        CSP_BITFVAL(CSPI_CONFIGREG_SCLKPHA, byParamPHA<<byParamChannelSelect)  |
        CSP_BITFVAL(CSPI_CONFIGREG_SCLKPOL, byParamPOL<<byParamChannelSelect)  |
        CSP_BITFVAL(CSPI_CONFIGREG_SSBCTRL, CSPI_CONFIGREG_SSBCTRL_SINGLEBURST)|
        CSP_BITFVAL(CSPI_CONFIGREG_SSBPOL, dwSSPOL)                            | //byParamSSPOL<<byParamChannelSelect) |
        CSP_BITFVAL(CSPI_CONFIGREG_DATACTL, CSPI_CONFIGREG_DATACTL_STAYHIGH)   |
        CSP_BITFVAL(CSPI_CONFIGREG_SCLKCTL, CSPI_CONFIGREG_SCLKCTL_STAYHIGH)   |
        CSP_BITFVAL(CSPI_CONFIGREG_HTLENGTH, 0x0));



    if(m_bUseLoopBack)
    {
        INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_INTERNALLYCONNECTED);
    }
    else
    {
        INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_NOTCONNECTED);
    }

    dwBurstCount = pXchPkt->xchCnt / 4;



    if (dwBurstCount)   // pXchPkt->xchCnt is more than 4
    {
        dmaTransferCount = pXchPkt->xchCnt;            //dwBurstCount * 4; ??????????
        m_isDMADone = FALSE;
    }
    else
    {
        goto error_exit;
    }
    // ensure RXFIFO is empty before start DMA
    while ( INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
    {
        INREG32(&m_pCSPI->RXDATA);
    }

    
    OUTREG32(&m_pCSPI->DMAREG, 
        CSP_BITFVAL(CSPI_DMAREG_TXWATER, CSPI_DMA_WATERMARK_TX/4)                   |
        CSP_BITFVAL(CSPI_DMAREG_TXDMAEN, CSPI_DMAREG_TXDEN_ENABLE)                  |
        CSP_BITFVAL(CSPI_DMAREG_RXWATER, CSPI_DMA_WATERMARK_RX/4-1)                 |
        CSP_BITFVAL(CSPI_DMAREG_RXDMAEN, CSPI_DMAREG_RXDEN_ENABLE)                  |
        // If burst length is not multiple of 4, TAIL function will be enabled
        CSP_BITFVAL(CSPI_DMAREG_RXDMALEN, (pXchPkt->xchCnt%4)?pXchPkt->xchCnt:0)    |
        CSP_BITFVAL(CSPI_DMAREG_RXTDEN, (pXchPkt->xchCnt%4)?CSPI_DMAREG_RXTDEN_ENABLE:CSPI_DMAREG_RXTDEN_DISABLE)); 

    // configure RX DMA
    DDKSdmaClearBufDescStatus(m_dmaChanCspiRx,0);
    DDKSdmaSetBufDesc(m_dmaChanCspiRx, 0, 
        DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP,
        PhysDMABufferAddr.LowPart + CSPI_RECV_OFFSET,
        0, DDK_DMA_ACCESS_32BIT, (UINT16)dmaTransferCount * 4); //Count should be in Bytes.
    

    // configure TX DMA
    DDKSdmaClearBufDescStatus(m_dmaChanCspiTx,0);
    DDKSdmaSetBufDesc(m_dmaChanCspiTx, 0, 
        DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP,
        PhysDMABufferAddr.LowPart + CSPI_TXMT_OFFSET,
        0, DDK_DMA_ACCESS_32BIT, (UINT16)pXchPkt->xchCnt * 4); // set the count in bytes.
    MoveDMABuffer(pTxBuf, pXchPkt->xchCnt * 4, FALSE); // set the count in bytes.

    // start Rx DMA channel
    DDKSdmaStartChan(m_dmaChanCspiRx);

    // start Tx DMA channel
    DDKSdmaStartChan(m_dmaChanCspiTx);

    // until we are done with requested transfers
    while(!bTxDone)
    {
        // load Tx FIFO until enough for XCH, or until we run out of data
        while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TE))
            ;
        
        // start exchange
        INSREG32(&m_pCSPI->CONTROLREG, CSP_BITFMASK(CSPI_CONTROLREG_XCH), 
            CSP_BITFVAL(CSPI_CONTROLREG_XCH, CSPI_CONTROLREG_XCH_EN));

        if(m_bUsePolling || bReqPolling)
        {
            // wait until transaction is complete
            while (!(INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC)))
                ;
            uiTxStatus = 0;
            DDKSdmaGetBufDescStatus(m_dmaChanCspiTx, 0, &uiTxStatus);
            if (!(uiTxStatus & DDK_DMA_FLAGS_BUSY) &&  
                (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TE)))
            {
                xchTxCnt = (uiTxStatus & CSP_BITFMASK(SDMA_MODE_COUNT)) / 4;
                bTxDone = TRUE;
            }
        }
        else
        {
            INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_TCEN), 
                CSP_BITFVAL(CSPI_INTREG_TCEN, CSPI_INTREG_TCEN_ENABLE));

            // wait for requested transfer interrupt
            WaitForSingleObject(m_hIntrEvent, INFINITE);


            // disable all interrupts
            OUTREG32(&m_pCSPI->INTREG, 0);

            // acknowledge transfer complete (w1c)
            OUTREG32(&m_pCSPI->STATREG, CSP_BITFMASK(CSPI_STATREG_TC));

            uiTxStatus = 0;
            DDKSdmaGetBufDescStatus(m_dmaChanCspiTx, 0, &uiTxStatus);
            if (!(uiTxStatus & DDK_DMA_FLAGS_BUSY) &&  
                (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TE)))
            {
                xchTxCnt = (uiTxStatus & CSP_BITFMASK(SDMA_MODE_COUNT)) / 4;
                bTxDone = TRUE;
            }

            // signal that interrupt has been handled
            InterruptDone(m_dwSysIntr);
        }
    }

    uiRxStatus = 0;
    DDKSdmaGetBufDescStatus(m_dmaChanCspiRx, 0, &uiRxStatus);
    while ((uiRxStatus & DDK_DMA_FLAGS_BUSY))
    {
        DDKSdmaGetBufDescStatus(m_dmaChanCspiRx, 0, &uiRxStatus);
    }
    m_isDMADone = TRUE;
    xchRxCnt = (uiRxStatus & CSP_BITFMASK(SDMA_MODE_COUNT)) / 4;

    // there are data remain in RXFIFO below STATREG:RH
    if ( pRxBuf!= NULL)
    {
        MoveDMABuffer(pRxBuf, xchRxCnt * 4,TRUE);

        // increment Rx Buffer to next data point
        pRxBuf = (LPVOID) ((UINT) pRxBuf + xchRxCnt * 4);

        // while there is data in Rx FIFO 
        while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
        {
            tmp = INREG32(&m_pCSPI->RXDATA);
            CspiBufWrt32(pRxBuf, tmp);
            // increment Rx Buffer to next data point
            pRxBuf = (LPVOID) ((UINT) pRxBuf + 4);
            ++xchRxCnt;
        }

    }
    else
    {
        // remove data in Rx FIFO
        while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
        {
            INREG32(&m_pCSPI->RXDATA);
            ++xchRxCnt;
        }
    }

    DDKSdmaGetBufDescStatus(m_dmaChanCspiTx, 0, &uiTxStatus);
    DDKSdmaGetBufDescStatus(m_dmaChanCspiRx, 0, &uiRxStatus);

error_exit:
    // disable the CSPI
    INSREG32(&m_pCSPI->CONTROLREG, CSP_BITFMASK(CSPI_CONTROLREG_EN), 
        CSP_BITFVAL(CSPI_CONTROLREG_EN, CSPI_CONTROLREG_EN_DISABLE));

    BSPCSPIEnableClock(m_Index,FALSE);
    DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange -\r\n")));

    return xchRxCnt;
}


//-----------------------------------------------------------------------------
//
// Function: CspiExchangeSize
//
// This function returns data bytes count in every exchange Packet. 
//
// Parameters:
//      None
//
// Returns:
//      Returns data size of exchange packet.
//
//-----------------------------------------------------------------------------
UINT32 cspiClass::CspiExchangeSize(PCSPI_XCH_PKT0_T pXchPkt)
{
    PCSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;

    if ((pBusCnfg->BurstLength >= 1) && (pBusCnfg->BurstLength <= 8))
    {
        // 8-bit access width
        return pXchPkt->xchCnt *sizeof(UINT8);
    }
    else if ((pBusCnfg->BurstLength >= 9) && (pBusCnfg->BurstLength <= 16))
    {
        // 16-bit access width
        return pXchPkt->xchCnt *sizeof(UINT16);
    }
    else if ((pBusCnfg->BurstLength >= 17) && (pBusCnfg->BurstLength <= 32))
    {
        // 32-bit access width
        return pXchPkt->xchCnt *sizeof(UINT32);
    }
    else
    {
        // 32-bit access width
        return pXchPkt->xchCnt *sizeof(UINT32);
    }
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
UINT32 cspiClass::CspiBufRd8(LPVOID pBuf)
{
    UINT8 *p;

    p = (UINT8 *) pBuf;

    return *p;
}


//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
UINT32 cspiClass::CspiBufRd16(LPVOID pBuf)
{
    UINT16 *p;

    p = (UINT16 *) pBuf;

    return *p;
}


//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
UINT32 cspiClass::CspiBufRd32(LPVOID pBuf)
{
    UINT32 *p;

    p = (UINT32 *) pBuf;

    return *p;
}


//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void cspiClass::CspiBufWrt8(LPVOID pBuf, UINT32 data)
{
    UINT8 *p;

    p = (UINT8 *) pBuf;

   *p = (UINT8) data;
}


//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void cspiClass::CspiBufWrt16(LPVOID pBuf, UINT32 data)
{
    UINT16 *p;

    p = (UINT16 *) pBuf;

   *p = (UINT16) data;
}


//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void cspiClass::CspiBufWrt32(LPVOID pBuf, UINT32 data)
{
    UINT32 *p;

    p = (UINT32 *) pBuf;

   *p = data;
}

// ----------------------------------------------------------------------------
// Function: MoveDMABuffer
//     Move data from s/g buffer to DMA buffer or vice versa
//
// Parameters:
//     pBuf 
//       Destination or Source buffer according to bReceive.
//
//     dwLen
//       Length of the buffer to be copied.
//
//     bReceive
//       Used to indicate Receive or Transmit.
//
//     Returns:
//       None.  
//     
// ----------------------------------------------------------------------------
VOID cspiClass::MoveDMABuffer(LPVOID pBuf, DWORD dwLen, BOOL bReceive)
{    
    DEBUGMSG(ZONE_FUNCTION, (_T("Cspi: MoveDMABuffer(%d)+\n"), dwLen));

    if (pBuf == NULL) 
    {
        // security violation
        DEBUGMSG(ZONE_ERROR, (TEXT("Cspi: MoveDMABuffer Failed to map pointer to caller\r\n")));
        return;
    }
    if (bReceive)
        memcpy(pBuf, pVirtDMABufferAddr + CSPI_RECV_OFFSET, dwLen);
    else
        memcpy(pVirtDMABufferAddr + CSPI_TXMT_OFFSET, pBuf, dwLen);

}
//------------------------------------------------------------------------------
//
// Function: MapDMABuffers
//
// Allocate and map DMA buffer 
//
// Parameters:
//        None
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL cspiClass::MapDMABuffers(void)
{
   DMA_ADAPTER_OBJECT Adapter;
   DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::MapDMABuffers+\r\n")));
      
   pVirtDMABufferAddr = NULL;
   
   memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
   Adapter.InterfaceType = Internal;
   Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

   // Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
   pVirtDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, (CSPI_SDMA_BUFFER_SIZE)
                                , &(PhysDMABufferAddr), FALSE);

   if (pVirtDMABufferAddr == NULL)
   {
      DEBUGMSG(ZONE_ERROR, (TEXT("Cspi: MapDMABuffers() - Failed to allocate DMA buffer.\r\n")));
      return(FALSE);
   }

   DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::MapDMABuffers-\r\n")));
   return(TRUE);
}
//------------------------------------------------------------------------------
//
// Function: UnmapDMABuffers
//
//  This function unmaps the DMA buffers previously mapped with the
//  MapDMABuffers function.
//
// Parameters:
//        None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL cspiClass::UnmapDMABuffers(void)
{
    DMA_ADAPTER_OBJECT Adapter;
    DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::UnmapDMABuffers+\r\n")));

    if(pVirtDMABufferAddr)
    {
        memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
        // Logical address parameter is ignored
        PhysDMABufferAddr.QuadPart = 0;
        HalFreeCommonBuffer(&Adapter, 0, PhysDMABufferAddr, (PVOID)pVirtDMABufferAddr, FALSE);
    }

    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: DeinitChannelDMA
//
//  This function deinitializes the DMA channel for output.
//
// Parameters:
//        None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL cspiClass::DeinitChannelDMA(void)
{    
   if (m_dmaChanCspiRx != 0)
   {
       DDKSdmaCloseChan(m_dmaChanCspiRx);
       m_dmaChanCspiRx = 0;
   }
   if (m_dmaChanCspiTx != 0)
   {
       DDKSdmaCloseChan(m_dmaChanCspiTx);
       m_dmaChanCspiTx = 0;
   }
   return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: InitChannelDMA
//
//  This function initializes the DMA channel for output.
//
// Parameters:
//  Index
//      CSPI device Index
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL cspiClass::InitChannelDMA(UINT32 Index)
{
    UINT8 tmp, dmaChannelPriority[2];
    BOOL rc = FALSE;
    Index = Index;

    DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::InitChannelDMA+\r\n")));

    // Check if DMA buffer has been allocated
    if (!PhysDMABufferAddr.LowPart || !pVirtDMABufferAddr)
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA: Invalid DMA buffer physical address.\r\n")));
        goto cleanUp;
    }

    BSPCspiGetChannelPriority(&dmaChannelPriority);
    if (dmaChannelPriority[0]==dmaChannelPriority[1])
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA: Invalid DMA Channel Priority.\r\n")));
        goto cleanUp;
    }

    if (dmaChannelPriority[0]>dmaChannelPriority[1])
    {
        tmp = dmaChannelPriority[0];
        dmaChannelPriority[0] = dmaChannelPriority[1];
        dmaChannelPriority[1] = tmp;

        DEBUGMSG(ZONE_WARN, (_T("WARNING:InitChannelDMA: Reversed DMA Channel Priority.\r\n")));
    }

    // Open virtual DMA channels 
    m_dmaChanCspiRx = DDKSdmaOpenChan(m_dmaReqRx, dmaChannelPriority[1]); /// NULL, CspCSPIGetIRQ(Index));
    DEBUGMSG(ZONE_DMA,(_T("Channel Allocated(Rx) : %d\r\n"),m_dmaChanCspiRx));
    if (!m_dmaChanCspiRx)
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): SdmaOpenChan for input failed.\r\n")));
        goto cleanUp;
    }

    // Allocate DMA chain buffer
    if (!DDKSdmaAllocChain(m_dmaChanCspiRx, 1))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaAllocChain for input failed.\r\n")));
        goto cleanUp;
    }  

    // Initialize the chain and set the watermark level     
    if (!DDKSdmaInitChain(m_dmaChanCspiRx, CSPI_DMA_WATERMARK_RX))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaInitChain failed.\r\n")));
        goto cleanUp;
    }

    // Open virtual DMA channels 
    m_dmaChanCspiTx = DDKSdmaOpenChan(m_dmaReqTx, dmaChannelPriority[0]); /// NULL, CspCSPIGetIRQ(Index));
    DEBUGMSG(ZONE_DMA,(_T("Channel Allocated(Tx) : %d\r\n"),m_dmaChanCspiTx));
    if (!m_dmaChanCspiTx)
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): SdmaOpenChan(Rx) for input failed.\r\n")));
        goto cleanUp;
    }

    // Allocate DMA chain buffer
    if (!DDKSdmaAllocChain(m_dmaChanCspiTx, 1))
    {
         DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaAllocChain for input failed.\r\n")));
         goto cleanUp;
    }  

    // Initialize the chain and set the watermark level 
    if (!DDKSdmaInitChain(m_dmaChanCspiTx, CSPI_DMA_WATERMARK_TX))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaInitChain failed.\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
   if (!rc)
   {
      DeinitChannelDMA();
   }
   DEBUGMSG(ZONE_DMA,(_T("Cspi::InitChannelDMA-\r\n")));
   return rc;
}


//------------------------------------------------------------------------------
//
// Function: InitDMA
//
//  Performs DMA channel intialization
//
// Parameters:
//  Index
//      CSPI device Index
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------

BOOL cspiClass::InitCspiDMA(UINT32 Index) 
{ 
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Cspi: InitDMA+\r\n")));
    Index = Index;
     
    m_dmaReqTx = CspCSPIGetDmaReqTx(Index) ; 
    m_dmaReqRx = CspCSPIGetDmaReqRx(Index) ; 

    // Map the DMA buffers into driver's virtual address space
    if (!MapDMABuffers())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:InitDMA() - Failed to map DMA buffers.\r\n")));
        return FALSE;
    }

    // Initialize the output DMA
    if (!InitChannelDMA(Index))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:InitDMA() - Failed to initialize output DMA.\r\n")));
        return FALSE;
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Cspi::InitDMA-\r\n")));
    return TRUE ; 
}

//------------------------------------------------------------------------------
//
// Function: DeInitDMA
//
//  Performs deintialization of DMA
//
// Parameters:
//         None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL cspiClass::DeInitCspiDMA(void) 
{
    if(!DeinitChannelDMA())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:DeinitChannelDMA() - Failed to deinitialize DMA.\r\n")));
        return FALSE;
    }
    if(!UnmapDMABuffers())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:UnmapDMABuffers() - Failed to Unmap DMA buffers\r\n")));
        return FALSE;
    }
    return TRUE ; 
}

//------------------------------------------------------------------------------
//
// Function: CspiEnableLoopback
//
//  This function controls the LoopBackcontol of the FIFO.
//    Enable loopback also needs to enable Polling also.
//
// Parameters:
//    bEnable
//      Enable or Disable loopback
//
//  Returns:
//      None.
//
//------------------------------------------------------------------------------
void cspiClass::CspiEnableLoopback(BOOL bEnable)
{
    if(bEnable)
    {
        m_bUseLoopBack = TRUE;
    }
    else
    {
        // If this port is also used by kernel, there is to disable loopback
        if( CspCheckPort(m_Index))
        {
            INSREG32BF(&m_pCSPI->CONTROLREG, CSPI_CONTROLREG_EN, CSPI_CONTROLREG_EN_ENABLE);
            INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_NOTCONNECTED);
        }

        m_bUseLoopBack = FALSE;
    }
}


//------------------------------------------------------------------------------
//
// Function: CheckPort
//
//  This function checks if the cspi port is mutliplex port with nic card
//
// Parameters:
//      None.
//
//  Returns:
//      TRUE - The port is mutliplex
//
//      FALSE - The port is not mutliplex
//
//------------------------------------------------------------------------------
BOOL cspiClass::CheckPort(void)
{
    if( CspCheckPort(m_Index)){
        return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: CSPI2DataExchange
//
// The function deals eCSPI2 port transferring through kernal functions because this eCSPI2 
// port is multiplexing
//
// Parameters:
//      None.
//
//  Returns:
//      TRUE - Success
//
//      FALSE - Failure
//
//------------------------------------------------------------------------------
BOOL cspiClass::CSPI2DataExchange(PCSPI_XCH_PKT_T pXchPkt)
{
    INT32 parm[3];
    DWORD BytesReturned=0;
    PCSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;
    LPVOID pRxBuf = pXchPkt->pTxBuf;
    INT32 RxCount = 0;
    UINT8 byParamChannelSelect;
    UINT8 byParamSSPOL;
    UINT8 byParamPOL;
    UINT8 byParamPHA;
    UINT8 byParamDRCTL;
    UINT8 PreDiv=0;
    UINT8 PostDiv=0;
    BOOL rc = FALSE;

    // enable the clock gating
    BSPCSPIEnableClock(m_Index,TRUE);

    if( m_bUseLoopBack )
    {
        INSREG32BF(&m_pCSPI->CONTROLREG, CSPI_CONTROLREG_EN, CSPI_CONTROLREG_EN_ENABLE);
        INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_INTERNALLYCONNECTED);
    }

    byParamChannelSelect= pBusCnfg->ChannelSelect &((1U<<CSPI_CONTROLREG_CHANNELSELECT_WID) -1);
    byParamSSPOL = pBusCnfg->SSPOL? CSPI_CONFIGREG_SSBPOL_ACTIVEHIGH: CSPI_CONFIGREG_SSBPOL_ACTIVELOW;
    byParamPOL = pBusCnfg->SCLKPOL? CSPI_CONFIGREG_SCLKPOL_ACTIVELOW: CSPI_CONFIGREG_SCLKPOL_ACTIVEHIGH;
    byParamPHA = pBusCnfg->SCLKPHA? CSPI_CONFIGREG_SCLKPHA_PHASE1: CSPI_CONFIGREG_SCLKPHA_PHASE0;
    byParamDRCTL = pBusCnfg->DRCTL&((1U<<CSPI_CONTROLREG_DRCTL_WID) -1);

    BSPCSPICalculateDivRate(pBusCnfg->Freq, pBusCnfg->Freq/10, &PreDiv,  &PostDiv);

    parm[0] =   CspCSPIGetBaseRegAddr(m_Index);

    parm[1] =   (CSP_BITFVAL(CSPI_CONTROLREG_BURSTLENGTH, pBusCnfg->BurstLength-1)      |  
                CSP_BITFVAL(CSPI_CONTROLREG_CHANNELSELECT, byParamChannelSelect)        |  
                CSP_BITFVAL(CSPI_CONTROLREG_DRCTL, byParamDRCTL)                        |  
                CSP_BITFVAL(CSPI_CONTROLREG_PREDIVIDER, PreDiv)                         |
                CSP_BITFVAL(CSPI_CONTROLREG_POSTDIVIDER, PostDiv)                       |
                CSP_BITFVAL(CSPI_CONTROLREG_CHANNELMODE, CSPI_CONTROLREG_CHANNELMODE_MASTER<<byParamChannelSelect)|  
                CSP_BITFVAL(CSPI_CONTROLREG_SMC, CSPI_CONTROLREG_SMC_AUTOMATIC_MODE)    |  
                CSP_BITFVAL(CSPI_CONTROLREG_XCH, CSPI_CONTROLREG_XCH_IDLE)              |  
                CSP_BITFVAL(CSPI_CONTROLREG_HW, CSPI_CONTROLREG_HW_HTMODE_DISABLE)      |  
                CSP_BITFVAL(CSPI_CONTROLREG_EN, CSPI_CONTROLREG_EN_ENABLE));

    parm[2] =   (CSP_BITFVAL(CSPI_CONFIGREG_HTLENGTH, 0x0)                              |
                CSP_BITFVAL(CSPI_CONFIGREG_SCLKCTL, CSPI_CONFIGREG_SCLKCTL_STAYLOW)     |
                CSP_BITFVAL(CSPI_CONFIGREG_DATACTL, CSPI_CONFIGREG_DATACTL_STAYLOW)     |
                CSP_BITFVAL(CSPI_CONFIGREG_SSBPOL, byParamSSPOL<<byParamChannelSelect)  |
                CSP_BITFVAL(CSPI_CONFIGREG_SSBCTRL, CSPI_CONFIGREG_SSBCTRL_SINGLEBURST) |
                CSP_BITFVAL(CSPI_CONFIGREG_SCLKPOL, byParamPOL<<byParamChannelSelect)   |
                CSP_BITFVAL(CSPI_CONFIGREG_SCLKPHA, byParamPHA<<byParamChannelSelect));


    if( pXchPkt->xchCnt>0 )
    {
        RxCount = pXchPkt->xchCnt;
    }else{
        RxCount = pBusCnfg->BurstLength/32+1;
    }

    if ( BSPCspiExchange(parm,pRxBuf,RxCount*4,&BytesReturned) )
    {
        if((pXchPkt->xchCnt>0)&&(pXchPkt->pRxBuf!=NULL))
            memcpy(pXchPkt->pRxBuf, pRxBuf, pXchPkt->xchCnt*4);
        rc = TRUE;
    }

    // disable the clock gating
    BSPCSPIEnableClock(m_Index,FALSE);

    return rc;
}



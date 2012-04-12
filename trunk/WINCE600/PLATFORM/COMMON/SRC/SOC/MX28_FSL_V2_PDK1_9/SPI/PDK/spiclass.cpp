//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
//-----------------------------------------------------------------------------
//
//  File:  spiClass.cpp
//
//  Provides the implementation of the SPI bus driver to support SPI
//  transactions from multiple client drivers.
//`
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <linklist.h>
#pragma warning(pop)

#include "spiclass.h"
#include "csp.h"
#include <Devload.h>

#define DBGMSGON TRUE
//-----------------------------------------------------------------------------
// External Functions
extern "C" BOOL BSPSPISetIOMux(UINT32 Index);
extern "C" BOOL BSPSPIReleaseIOMux(UINT32 dwIndex);
extern "C" BOOL BSPSpiIsAllowPolling(UINT8 Index);
extern "C" BOOL BSPSpiIsDMAEnabled(UINT8 Index);
extern "C" BOOL BSPSpiExchange(VOID *lpInBuf, LPVOID pRxBuf, UINT32 nOutBufSize, LPDWORD BytesReturned);
//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------
// Defines

// The frequency of the output signal bit clock SSP_SCK is defined as follows:
//                 SSPCLK
// SSP_SCK  =   -----------------
//               CLOCK_DIVIDE * (1+ CLOCK_RATE)
#define DATA_TIMEOUT          0xFFFF
#define ETH_CLOCK_DIVIDE      0x4     // SSPCLOCK/8
#define SD_CLOCK_DIVIDE       0x2     // SSPCLOCK/60  
#define CLOCK_RATE            0x0

#define SD_SSP_FREQUENCY      48000   // frequency in KHZ
#define ETH_SSP_FREQUENCY     48000
#define SD1_SSP_FREQUENCY     50000   // frequency in KHZ
//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
// Local Functions
//#ifdef POLLING_MODE
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
//#endif

//-----------------------------------------------------------------------------
//
// Function: spiClass
//
// Parameters:
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
spiClass::spiClass()
{
    DEBUGMSG(ZONE_INIT, (TEXT("spiClass +\r\n")));
    m_spiOpenCount = 0;
    m_hEnQEvent = NULL;
    m_bTerminate = FALSE;
    m_dxCurrent = D0;
    m_Index = 0;
    m_bUsePolling = FALSE;
    m_bAllowPolling = FALSE;
    DEBUGMSG(ZONE_INIT, (TEXT("spiClass -\r\n")));
}
//-----------------------------------------------------------------------------
//
// Function: ~spiClass
//
// Parameters:
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
spiClass::~spiClass()
{
    DEBUGMSG(ZONE_INIT, (TEXT("~spiClass\r\n")));
}
//-----------------------------------------------------------------------------
//
// Function: SpiInitialize
//
// Initializes the SPI interface and data structures.
//
// Parameters:
//      Index
//          [in] SPI instance (1 = SPI1, 2 = SPI2) to initialize
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL spiClass::SpiInitialize(DWORD Index)
{
    PHYSICAL_ADDRESS phyAddr;
    m_Index = Index;
//#ifdef POLLING_MODE
    // create global heap for internal queues/buffers
    //      flOptions = 0 => no options
    //      dwInitialSize = 0 => zero bytes initial size
    //      dwMaximumSize = 0 => heap size limited only by available memory
    DEBUGMSG(ZONE_INIT, (TEXT("SpiInitialize: m_hHeap=0x%x\r\n"),m_hHeap));
    m_hHeap = HeapCreate(0, 0, 0);

    // check if HeapCreate failed
    if (m_hHeap == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SpiInitialize:  HeapCreate failed!\r\n")));
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
        DEBUGMSG(ZONE_ERROR, (TEXT("SpiInitialize:  CreateEvent failed!\r\n")));
        goto Error;
    }

    // create Semaphore for process queue thread signaling
    //      pEventAttributes = NULL (must be NULL)
    //      lpName = NULL => object created without a name
    m_hEnQSemaphere = CreateSemaphore(NULL, SPI_MAX_QUEUE_LENGTH, SPI_MAX_QUEUE_LENGTH, NULL);

    // check if CreateEvent failed
    if (m_hEnQSemaphere == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SpiInitialize:  CreateSemaphore failed!\r\n")));
        goto Error;
    }
//#endif
    // Get the Base Address according to Index
    switch (Index)
    {
        case 0:
            phyAddr.QuadPart = CSP_BASE_REG_PA_SSP0;
            break;
        case 1:
            phyAddr.QuadPart = CSP_BASE_REG_PA_SSP1;
            break;
        case 2:
            phyAddr.QuadPart = CSP_BASE_REG_PA_SSP2;
            break;
        case 3:
            phyAddr.QuadPart = CSP_BASE_REG_PA_SSP3;
            break;
        default:
            phyAddr.QuadPart = 0;
            break;
    }

    if(!phyAddr.QuadPart)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SpiInitialize:  invalid SPI instance!\r\n")));
        goto Error;
    }

    // Get the Base Address and IRQ according to Index
    m_bUseDMA       = BSPSpiIsDMAEnabled((UINT8)Index);

    m_bAllowPolling = BSPSpiIsAllowPolling((UINT8)Index);

    // Configure IOMUX 
    BSPSPISetIOMux(Index);

    // Map peripheral physical address to virtual address
    switch(m_Index)
    {
        case 0:
            pv_HWregSSP0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
            break;
        case 1:
            pv_HWregSSP1 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
            break;
        case 2:
            pv_HWregSSP2 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
            break;
        case 3:
            pv_HWregSSP3 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
            break;
        default:
            break;
    }
    // Check if virtual mapping failed
    if((pv_HWregSSP0 == NULL) && (pv_HWregSSP1 == NULL) && (pv_HWregSSP2 == NULL) && (pv_HWregSSP3 == NULL))
    { 
        DEBUGMSG(ZONE_ERROR, (TEXT("SpiInitialize:  MmMapIoSpace failed!\r\n")));
        goto Error;
    }

    // Ungate the block
    HW_SSP_CTRL0_CLR(m_Index,BM_SSP_CTRL0_CLKGATE);
    // Reset the block    
    HW_SSP_CTRL0_SET(m_Index,BM_SSP_CTRL0_SFTRST);

    // Release the Block from Reset and starts the clock
    HW_SSP_CTRL0_CLR(m_Index,BM_SSP_CTRL0_SFTRST | BM_SSP_CTRL0_CLKGATE);

    // create SPI critical section
    InitializeCriticalSection(&m_spiCs);

//#ifdef POLLING_MODE
    // create SPI Data Exchange critical section
    InitializeCriticalSection(&m_spiDataXchCs);

    // initialize SPI linked list of client queue entries
    InitializeListHead(&m_ListHead);

    // create SPI queue processing thread if it does not exist
    if (!m_hThread)
    {
        // set global termination flag
        m_bTerminate=FALSE;

        // create processing thread
        //      pThreadAttributes = NULL (must be NULL)
        //      dwStackSize = 0 => default stack size determined by linker
        //      lpStartAddress = SpiProcessQueue => thread entry point
        //      lpParameter = NULL => point to thread parameter
        //      dwCreationFlags = 0 => no flags
        //      lpThreadId = NULL => thread ID is not returned
        m_hThread = ::CreateThread(NULL, 0, SpiProcessQueue, this, 0, NULL);
        DEBUGMSG(ZONE_INIT, (TEXT("SpiInitialize:  this=0x%x\r\n"),this));

        // check if CreateThread failed
        if (m_hThread == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("SpiInitialize:  CreateThread failed!\r\n")));
            goto Error;
        }
    }
//#endif

    m_spiOpenCount++;    

    return TRUE;

Error:
    SpiRelease();
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: SpiRelease
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
void spiClass::SpiRelease()
{
    DEBUGMSG(ZONE_CLOSE, (TEXT("SpiRelease +\r\n")));

//#ifdef POLLING_MODE
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
//#endif
    // Release IOMUX pins
    BSPSPIReleaseIOMux(m_Index);

//#ifdef POLLING_MODE
    // destroy the heap
    if (m_hHeap != NULL)
    {
        HeapDestroy(m_hHeap);
        m_hHeap = NULL;
    }

    // close enqueue event handle
    if (m_hEnQEvent)
    {
        CloseHandle(m_hEnQEvent);
        m_hEnQEvent = NULL;
    }

    // close enqueue Semaphere handle
    if (m_hEnQSemaphere)
    {
        CloseHandle(m_hEnQSemaphere);
        m_hEnQSemaphere = NULL;
    }
//#endif
    // free the virtual space allocated for SPI memory map
    if (pv_HWregSSP0 != NULL)
    {
        MmUnmapIoSpace(pv_HWregSSP0, 0x1000);
        pv_HWregSSP0 = NULL;
    }
    if (pv_HWregSSP1 != NULL)
    {
        MmUnmapIoSpace(pv_HWregSSP1, 0x1000);
        pv_HWregSSP1 = NULL;
    }
    if (pv_HWregSSP2 != NULL)
    {
        MmUnmapIoSpace(pv_HWregSSP2, 0x1000);
        pv_HWregSSP2 = NULL;
    }
    if (pv_HWregSSP3 != NULL)
    {
        MmUnmapIoSpace(pv_HWregSSP3, 0x1000);
        pv_HWregSSP3 = NULL;
    }

    // delete the critical section
    DeleteCriticalSection(&m_spiCs);

//#ifdef POLLING_MODE
    // delete the Data Exchange critical section
    DeleteCriticalSection(&m_spiDataXchCs);
//#endif

    DEBUGMSG(ZONE_CLOSE, (TEXT("SpiRelease -\r\n")));
}


//-----------------------------------------------------------------------------
//
// Function:  ConfigureSSP
//
// This function Configures the SPI bus required for interaction
//
// Parameters:
//      sInit: pointer to structure HW_SSP_INIT which specifies input parameters to confogure the SPI bus.
//
// Returns:
//      Returns TRUE.

//
//-----------------------------------------------------------------------------
BOOL spiClass::ConfigureSSP(SSP_INIT * sInit)
{
    SSP_CTRL0 sControl0;
    SSP_CTRL1 sControl1;

    DEBUGMSG(DBGMSGON,(TEXT("Configure Channel ++ \r\n")));

    //// Configure SSP Control Register 0   
    sControl0.U = 0;                                                                                                      
                                                                                  
    sControl0.B.LOCK_CS = sInit->bLockCs;
    sControl0.B.IGNORE_CRC = sInit->bIgnoreCrc;
    sControl0.B.BUS_WIDTH = sInit->bBusWidth4;
    sControl0.B.WAIT_FOR_IRQ = sInit->bWaitIrq;
    sControl0.B.LONG_RESP = sInit->bLongResp;
    sControl0.B.CHECK_RESP = sInit->bCheckResp;
    sControl0.B.GET_RESP = sInit->bGetResp;
    sControl0.B.WAIT_FOR_CMD = sInit->bWaitCmd;
    sControl0.B.DATA_XFER = sInit->bDataTransfr;
    //sControl0.B.XFER_COUNT = sInit->u16TransferCount;

     // Configure SSP Control Register 1
    sControl1.U = 0;

    sControl1.B.DMA_ENABLE = sInit->bDmaEnable;
    sControl1.B.CEATA_CCS_ERR_EN = sInit->b_ceata_ccs_err_en;
    sControl1.B.SLAVE_OUT_DISABLE = sInit->bSlaveOutDisable;
    sControl1.B.PHASE = sInit->bPhase;
    sControl1.B.POLARITY = sInit->bPolarity;
    sControl1.B.WORD_LENGTH = sInit->eLength;
    sControl1.B.SLAVE_MODE = sInit->bSlave;
    sControl1.B.SSP_MODE = sInit->eMode;

    // Configure SSP Control Register 0  

    // Write the SSP Control Register 0 and 1 values out to the interface
    HW_SSP_CTRL0_WR(m_Index,sControl0.U);
    HW_SSP_CTRL1_WR(m_Index,sControl1.U);
    HW_SSP_XFER_SIZE_WR(m_Index,sInit->u16TransferCount);
    DEBUGMSG(DBGMSGON,(TEXT("Configure Channel -- \r\n")));

    //DumpRegister();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  SSPResetAfterError
//
// This function Reset and Reconfigure the SSP interface after an Error IRQ.It will be called after an SSP error 
// IRQ event occurs, which causes the SSP interface to freeze up.  This function will perform a soft
// reset of the block. After resetting, the block will be reconfigured using
// the values passed in the sConfigParams.  If the block was in a low power
// state before freezing, it will be returned to that state after
// reconfiguring the interface.
//
//
// Parameters:
//      sConfigParams: pointer to Data structure with field representing the SSP_CTRL0, SSP_CTRL1, SSP_TIMING
//                     registers, and the low power state of the block.
// Returns:
//      Returns TRUE.

//
//-----------------------------------------------------------------------------
BOOL spiClass::SSPResetAfterError(SSP_RESETCONFIG * sConfigparams)
{
    UNREFERENCED_PARAMETER(sConfigparams);
    SSP_Reset();
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  SSPCheckErrors
//
// This function check the SSP Control1 and Status registers for errors.If an error is detected, that indicates that
// Ithe SSP interface is frozen, and needs to be reset by calling the function SSPResetAfterError(). 
// This function will perform a soft reset of the block. The reset will be handled through this function
//! and and a value of TRUE will be returned, indicating an error was 
//! encountered.  The calling function will have to take appropriate action
//! to handle the error condition.
//
//
// Parameters:
//            None.
// Returns:
//      Returns TRUE, if the funtion is success,else return false.

//
//-----------------------------------------------------------------------------
BOOL spiClass::SSPCheckErrors(VOID)
{
    SSP_CTRL1  sControl1;
    SSP_STATUS sStatus;

    // read in the value of the registers
    sControl1.U = HW_SSP_CTRL1_RD(m_Index);

    sStatus.U   = HW_SSP_STATUS_RD(m_Index);

    // Check CTRL1 and STATUS registers for errors
    if(sControl1.B.RESP_TIMEOUT_IRQ || sStatus.B.RESP_TIMEOUT)
    {
        DEBUGMSG(DBGMSGON, (TEXT("SSP: ERROR - Response Timeout\r\n")));
        return FALSE;
    }
    if(sStatus.B.RESP_CRC_ERR)
    {
        DEBUGMSG(DBGMSGON, (TEXT("SSP: Error - Response CRC Error\r\n")));
        return FALSE;
    }
    if(sControl1.B.RESP_ERR_IRQ || sStatus.B.RESP_ERR)
    {
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Response Error\r\n")));
        return FALSE;
    }
    if(sControl1.B.FIFO_OVERRUN_IRQ || sStatus.B.FIFO_OVRFLW)
    {
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Receive Overflow\r\n")));
        return FALSE;
    }
    if(sControl1.B.RECV_TIMEOUT_IRQ)
    {
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Receive Timeout\r\n")));
        return FALSE;
    }
    if(sControl1.B.DATA_CRC_IRQ || sStatus.B.DATA_CRC_ERR)
    {
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Data CRC Error\r\n")));
        return FALSE;
    }
    if(sControl1.B.DATA_TIMEOUT_IRQ)
    {
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Data Timeout\r\n")));
        return FALSE;
    }
    if(sStatus.B.TIMEOUT)
    {
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Timeout\r\n")));
        return FALSE;
    }
    // no error encountered
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  SSPGetIrqStatus
//
// This function Gets the status of the specified SSP interrupt.
//
//
// Parameters:
//            eIrq: An SSP interrupt number.
// Returns:
//      Returns TRUE, if the funtion is success,else return false.

//
//-----------------------------------------------------------------------------
BOOL spiClass::SSPGetIrqStatus(SSP_IRQ eIrq)
{
    SSP_CTRL1  sControl1;
    BOOL Value =0;

    // read in the value of the registers
    sControl1.U = HW_SSP_CTRL1_RD(m_Index);

    // Returns the active SSP interrupt
    switch (eIrq)
    {
        case SSP_IRQ_SDIO: 
            Value = (sControl1.B.SDIO_IRQ & BM_SSP_CTRL1_SDIO_IRQ);
            break;                                        
        case SSP_IRQ_RESP_ERR: 
            Value = (sControl1.B.RESP_ERR_IRQ & BM_SSP_CTRL1_RESP_ERR_IRQ);
            break;                                        
        case SSP_IRQ_RESP_TIMEOUT:                        
            Value = (sControl1.B.RESP_TIMEOUT_IRQ & BM_SSP_CTRL1_RESP_TIMEOUT_IRQ);
            break;                                        
        case SSP_IRQ_DATA_TIMEOUT:                        
            Value = (sControl1.B.DATA_TIMEOUT_IRQ & BM_SSP_CTRL1_DATA_TIMEOUT_IRQ);
            break;                                        
        case SSP_IRQ_DATA_CRC:                            
            Value = (sControl1.B.DATA_CRC_IRQ & BM_SSP_CTRL1_DATA_CRC_IRQ);
            break;                                        
        case SSP_IRQ_FIFO_UNDERRUN:
            Value = (sControl1.B.FIFO_UNDERRUN_IRQ & BM_SSP_CTRL1_FIFO_UNDERRUN_IRQ);
            break;
        case SSP_IRQ_CEATA_CCS_ERR:
            Value = (sControl1.B.CEATA_CCS_ERR_IRQ & BM_SSP_CTRL1_CEATA_CCS_ERR_IRQ);
            break;
        case SSP_IRQ_RECV_TIMEOUT:
            Value = (sControl1.B.RECV_TIMEOUT_IRQ & BM_SSP_CTRL1_RECV_TIMEOUT_IRQ);
            break;
        case SSP_IRQ_FIFO_OVERRUN:
            Value = (sControl1.B.FIFO_OVERRUN_IRQ & BM_SSP_CTRL1_FIFO_OVERRUN_IRQ);
            break;
        default:
            break;
    }
    return Value;
}


//-----------------------------------------------------------------------------
//
// Function:  SSPClearIrq
//
// This function Clears the interrupt flag of a specified SSP interrupt in the
// SSP_CTRL1 register.
//
// Parameters:
//            eIrq: An SSP interrupt number.
// Returns:
//      Returns TRUE, if the funtion is success,else return false.

//
//-----------------------------------------------------------------------------
BOOL spiClass::SSPClearIrq(SSP_IRQ eIrq)
{
    // Returns the active SSP interrupt
    switch (eIrq)
    {
        case SSP_IRQ_SDIO: 
            HW_SSP_CTRL1_CLR(m_Index,BM_SSP_CTRL1_SDIO_IRQ);
            break;                                        
        case SSP_IRQ_RESP_ERR: 
            HW_SSP_CTRL1_CLR(m_Index,BM_SSP_CTRL1_RESP_ERR_IRQ);
            break;                                        
        case SSP_IRQ_RESP_TIMEOUT:           
            HW_SSP_CTRL1_CLR(m_Index,BM_SSP_CTRL1_RESP_TIMEOUT_IRQ);
            break;                                        
        case SSP_IRQ_DATA_TIMEOUT:
            HW_SSP_CTRL1_CLR(m_Index,BM_SSP_CTRL1_DATA_TIMEOUT_IRQ);
            break;                                        
        case SSP_IRQ_DATA_CRC:  
            HW_SSP_CTRL1_CLR(m_Index,BM_SSP_CTRL1_DATA_CRC_IRQ);
            break;                                        
        case SSP_IRQ_FIFO_UNDERRUN:
            HW_SSP_CTRL1_CLR(m_Index,BM_SSP_CTRL1_FIFO_UNDERRUN_IRQ);
            break;
        case SSP_IRQ_CEATA_CCS_ERR:
            HW_SSP_CTRL1_CLR(m_Index,BM_SSP_CTRL1_CEATA_CCS_ERR_IRQ);
            break;
        case SSP_IRQ_RECV_TIMEOUT:
            HW_SSP_CTRL1_CLR(m_Index,BM_SSP_CTRL1_RECV_TIMEOUT_IRQ);
            break;
        case SSP_IRQ_FIFO_OVERRUN:
            HW_SSP_CTRL1_CLR(m_Index,BM_SSP_CTRL1_FIFO_OVERRUN_IRQ);
            break;
        default:
            break;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  SSPConfigTiming
//
// This function configures the timing registrs of a specified SSP Speed in the
// SSP_Timing register.
//
// Parameters:
//            eSpeed: An Speed value to be set.
// Returns:
//      Returns TRUE, if the funtion is success,else return false.

//
//-----------------------------------------------------------------------------
BOOL spiClass::SSPConfigTiming(SSP_SPEED eSpeed)
{
    UINT32 u32Timing,uClkDiv =0;
    SSP_CTRL1  sControl1;
    UINT32 u32SSPFreq = 0, rootfreq = 0, u32Div = 0; 

    sControl1.U = HW_SSP_CTRL1_RD(m_Index);;

    if((eSpeed < SPI_IDENTIFY_SPEED) || (eSpeed >= SPI_UNDEFINED_SPEED))
    {
        DEBUGMSG(DBGMSGON, (TEXT("Error Invalid SSP speed. (%d)\r\n"), eSpeed));
        return FALSE; 
    }

    if (eSpeed == SPI_IDENTIFY_SPEED)
    {
        uClkDiv = SD_CLOCK_DIVIDE; 
        u32SSPFreq = SD_SSP_FREQUENCY; // frequency in KHZ
    }
    else if (eSpeed == SPI_TRANSFER_HS_FAST_SPEED)
    {
        uClkDiv = ETH_CLOCK_DIVIDE;  
        u32SSPFreq = ETH_SSP_FREQUENCY; // frequency in KHZ
    }
    else 
    {
        uClkDiv = SD_CLOCK_DIVIDE;  
        u32SSPFreq = SD1_SSP_FREQUENCY; // frequency in KHZ
    }
    //DDKClockSetSspClk(&u32SSPFreq, TRUE);
    
    if((m_Index == 0) || (m_Index == 1))
    {
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_IO0, &rootfreq);
    }
    if((m_Index == 2) || (m_Index == 3))
    {
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_IO1, &rootfreq);
    }

    u32Div = rootfreq / (u32SSPFreq * 1000) + 1;

    switch(m_Index)
    {
        case 0:
            DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP0, DDK_CLOCK_BAUD_SOURCE_REF_IO0, u32Div );
            DDKClockSetGatingMode(DDK_CLOCK_GATE_SSP0_CLK, FALSE);
            break;
        case 1:
            DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP1, DDK_CLOCK_BAUD_SOURCE_REF_IO0, u32Div );
            DDKClockSetGatingMode(DDK_CLOCK_GATE_SSP1_CLK, FALSE);
            break;
        case 2:
            DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP2, DDK_CLOCK_BAUD_SOURCE_REF_IO1, u32Div );
            DDKClockSetGatingMode(DDK_CLOCK_GATE_SSP2_CLK, FALSE);
            break;
        case 3:
            DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP3, DDK_CLOCK_BAUD_SOURCE_REF_IO1, u32Div );
            DDKClockSetGatingMode(DDK_CLOCK_GATE_SSP3_CLK, FALSE);
            break;
        default:
            break;
    }

    u32Timing = (UINT32)(DATA_TIMEOUT << 16) | uClkDiv << 8  | CLOCK_RATE;         

    //DEBUGMSG(DBGMSGON, (TEXT("DDI_SSP: u32Timing= 0x%X, uClkDiv = %d, uClkRate = %d.\r\n"),u32Timing, uClkDiv, uClkRate));

    HW_SSP_TIMING_WR(m_Index,u32Timing);

    HW_SSP_CTRL1_WR(m_Index,sControl1.U);

    //DumpRegister();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  SSPEnableErrIrq
//
// This function configures the Error Interrupts of the 
// SSP_CTRL1 register.
//
// Parameters:
//          None.
// Returns:
//      Returns TRUE, if the funtion is success,else return false.

//
//-----------------------------------------------------------------------------

BOOL spiClass::SSPEnableErrIrq(BOOL bEnable)
{
    if(bEnable)
        HW_SSP_CTRL1_SET(m_Index,SSP_CTRL1_HANDLED_ERRORS_MASK);
    else
        HW_SSP_CTRL1_CLR(m_Index,SSP_CTRL1_HANDLED_ERRORS_MASK);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  SSPDisableErrIrq
//
// This function Disables the Error Interrupts of the 
// SSP_CTRL1 register.
//
// Parameters:
//          None.
// Returns:
//      Returns TRUE, if the funtion is success,else return false.

//
//-----------------------------------------------------------------------------
BOOL spiClass::SSPDisableErrIrq(VOID)
{
    HW_SSP_CTRL1_CLR(m_Index,SSP_CTRL1_IRQS_MASK);
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: DumpRegister
//
// This function Dumps the SSP registers, used for debugging.
//
// Parameters:
//      None.
//
// Returns:
//      returns TRUE.
//
//------------------------------------------------------------------------------
BOOL spiClass::DumpRegister(VOID)
{
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_CTRL0        = 0x%x\r\n"),  HW_SSP_CTRL0_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_CTRL1        = 0x%x\r\n"),  HW_SSP_CTRL1_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_CMD0         = 0x%x\r\n"),  HW_SSP_CMD0_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_CMD1         = 0x%x\r\n"),  HW_SSP_CMD1_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_COMPREF      = 0x%x\r\n"),  HW_SSP_COMPREF_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_COMPMASK     = 0x%x\r\n"),  HW_SSP_COMPMASK_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_TIMING       = 0x%x\r\n"),  HW_SSP_TIMING_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_DATA         = 0x%x\r\n"),  HW_SSP_DATA_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_SDRESP0      = 0x%x\r\n"),  HW_SSP_SDRESP0_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_SDRESP1      = 0x%x\r\n"),  HW_SSP_SDRESP1_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_SDRESP2      = 0x%x\r\n"),  HW_SSP_SDRESP2_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_SDRESP3      = 0x%x\r\n"),  HW_SSP_SDRESP3_RD(m_Index)));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_STATUS       = 0x%x\r\n"),  HW_SSP_STATUS_RD(m_Index)));

    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: SSP_Reset
//
// This function Resets the SSP H_W .
//
// Parameters:
//      None.
//
// Returns:
//      returns None.
//
//------------------------------------------------------------------------------
VOID spiClass:: SSP_Reset(VOID)
{
    // Reset the block.

    // Prepare for soft-reset by making sure that SFTRST is not currently
    // asserted.
    HW_SSP_CTRL0_CLR(m_Index,BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond for SFTRST to deassert.
    HW_SSP_CTRL0_RD(m_Index);
    HW_SSP_CTRL0_RD(m_Index);
    
    // Also clear CLKGATE so we can wait for its assertion below.
    HW_SSP_CTRL0_CLR(m_Index,BM_SSP_CTRL0_CLKGATE);

    // Now soft-reset the hardware.
    HW_SSP_CTRL0_SET(m_Index,BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond for SFTRST to deassert.
    HW_SSP_CTRL0_RD(m_Index);
    HW_SSP_CTRL0_RD(m_Index);

    // Deassert SFTRST.
    HW_SSP_CTRL0_CLR(m_Index,BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond .
    HW_SSP_CTRL0_RD(m_Index);
    HW_SSP_CTRL0_RD(m_Index);

    // Release the Block from Reset and starts the clock
    HW_SSP_CTRL0_CLR(m_Index,BM_SSP_CTRL0_CLKGATE);

    // Wait at least a microsecond.
    HW_SSP_CTRL0_RD(m_Index);
    HW_SSP_CTRL0_RD(m_Index);

}

//-----------------------------------------------------------------------------
//
// Function: SpiEnqueue
//
// This function is invoked as a result of a SPI client driver calling
// the SPI DeviceIoControl with SPI_IOCTL_EXCHANGE. This function
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
BOOL spiClass::SpiEnqueue(PSPI_XCH_PKT_T pXchPkt)
{
    PSPI_XCH_LIST_ENTRY_T pXchListEntry;
    /*DWORD cbSrc;*/
    HRESULT result;

    struct 
    {
        BOOL bAllocListEntry;
        BOOL bMarshalpBusCnfg;
        BOOL bMarshalBuf;
        BOOL bMarshalBufAsync;
        BOOL bMarshalpBusCnfgAsync;
    } resource;

    CALLER_STUB_T marshalEventStub;
    DEBUGMSG(1, (TEXT("SpiEnqueue +\r\n")));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SpiEnqueue +\r\n")));

    resource.bAllocListEntry = FALSE;
    resource.bMarshalpBusCnfg = FALSE;
    resource.bMarshalBuf = FALSE;
    resource.bMarshalBufAsync = FALSE;
    resource.bMarshalpBusCnfgAsync = FALSE;

    pXchListEntry = NULL;

    // allocate space for new list entry
    pXchListEntry = (PSPI_XCH_LIST_ENTRY_T)HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY,sizeof(SPI_XCH_LIST_ENTRY_T));

    // check if HeapAlloc failed (fatal error, terminate thread)
    if (pXchListEntry == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SpiProcessQueue:  HeapAlloc failed!\r\n")));
        return FALSE;
    }
    resource.bAllocListEntry = TRUE;

    result = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);

    // Open caller Bus Config buffer.
    result = CeOpenCallerBuffer(
                pXchListEntry->xchPkt.marshalBusCnfgStub,
                pXchPkt->pBusCnfg,
                sizeof(SPI_BUSCONFIG_T),
                ARG_I_PTR,
                FALSE);

    if (!SUCCEEDED(result)) 
    {
        goto error_cleanup;
    }
    resource.bMarshalpBusCnfg = TRUE;
    // Copy data check config
    pXchListEntry->xchPkt.xchRealPkt.pBusCnfg =(PSPI_BUSCONFIG_T)pXchListEntry->xchPkt.marshalBusCnfgStub.m_pLocalSyncMarshalled;
    // check translated pointer
    if (pXchListEntry->xchPkt.xchRealPkt.pBusCnfg == NULL)
    {
        goto error_cleanup;
    }
    // Copy data into list entry
    pXchListEntry->xchPkt.xchRealPkt.xchCnt = pXchPkt->xchCnt;

    // Translate event from event name
    pXchListEntry->xchPkt.xchRealPkt.xchEvent = NULL;
    if (pXchPkt->xchEventLength>0 && pXchPkt->xchEventLength<= SPI_MAXENQ_EVENT_NAME)
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

            pXchListEntry->xchPkt.xchRealPkt.xchEvent = CreateEvent(NULL,TRUE, FALSE, pXchPkt->xchEvent);

            CeCloseCallerBuffer(marshalEventStub);

            if (pXchListEntry->xchPkt.xchRealPkt.xchEvent == NULL)
            {
                goto error_cleanup;
            }
        }
    }
    // Open caller buffer.
    result = CeOpenCallerBuffer(
                pXchListEntry->xchPkt.marshalStub,
                pXchPkt->pBuf,
                pXchPkt->xchCnt,
                ARG_I_PTR,
                FALSE);

    if (!SUCCEEDED(result)) 
    {
        goto error_cleanup;
    }
    resource.bMarshalBuf = TRUE;
    pXchListEntry->xchPkt.xchRealPkt.pBuf = pXchListEntry->xchPkt.marshalStub.m_pLocalSyncMarshalled;

    // Check whether the completion event is NULL
    // If it's NULL, then send the package immediately
    if (pXchListEntry->xchPkt.xchRealPkt.xchEvent == NULL)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SpiEnqueue: Sending the package immediately\r\n")));

        // Wrap the exchange in critical section to 
        // serialize accesses with SpiProcessQueue thread
        EnterCriticalSection(&m_spiDataXchCs);

        if (m_bUseDMA && pXchListEntry->xchPkt.xchRealPkt.pBusCnfg->usedma)
        {
            //pXchListEntry->xchPkt.xchRealPkt.xchCnt = SpiDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
        }
        else
        {
            pXchListEntry->xchPkt.xchRealPkt.xchCnt = SpiNonDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
        }

        LeaveCriticalSection(&m_spiDataXchCs);
            
        if (resource.bMarshalBuf)
        {
            CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalStub);
        }
        CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
        HeapFree(m_hHeap, 0, pXchListEntry);
        return TRUE;
    }
    // wait for system leaveing power down state
    while (m_dxCurrent == D4)
    {
        Sleep(1000);
    }
    // wait for spi asynchronous queue availiable
    WaitForSingleObject(m_hEnQSemaphere, INFINITE);

    result = CeAllocAsynchronousBuffer(pXchListEntry->xchPkt.marshalStub);
    if (!SUCCEEDED(result)) 
    {
        goto error_cleanup;
    }
    resource.bMarshalBufAsync = TRUE;
    pXchListEntry->xchPkt.xchRealPkt.pBuf = pXchListEntry->xchPkt.marshalStub.m_pLocalAsync;

    result = CeAllocAsynchronousBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
    if (!SUCCEEDED(result)) 
    {
        goto error_cleanup;
    }
    resource.bMarshalpBusCnfgAsync = TRUE;
    pXchListEntry->xchPkt.xchRealPkt.pBusCnfg = (PSPI_BUSCONFIG_T)pXchListEntry->xchPkt.marshalBusCnfgStub.m_pLocalAsync;

    // wrap linked list insert operation in critical section to 
    // serialize accesses with SpiProcessQueue thread
    EnterCriticalSection(&m_spiCs);
    // insert exchange packet into our list
#pragma warning(push)
#pragma warning(disable: 4127)
    InsertTailList(&m_ListHead, &(pXchListEntry->link));
#pragma warning(pop)

    LeaveCriticalSection(&m_spiCs);
        
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SpiEnqueue -\r\n")));

    // signal queue processing thread that a new packet is ready
    return SetEvent(m_hEnQEvent);

error_cleanup:
    if (resource.bMarshalpBusCnfgAsync)
    {
        CeFreeAsynchronousBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
    }
    if (resource.bMarshalpBusCnfg)
    {
        CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
    }
    if (resource.bMarshalBufAsync)
    {
        CeFreeAsynchronousBuffer(pXchListEntry->xchPkt.marshalStub);
    }
    if (resource.bMarshalBuf)
    {
        CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalStub);
    }
    if (resource.bAllocListEntry)
    {
        HeapFree(m_hHeap, 0, pXchListEntry);
    }
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: SpiProcessQueue
//
// This function is the entry point for a thread that will be created
// during SPI driver initialization and remain resident to process
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
DWORD WINAPI spiClass::SpiProcessQueue(LPVOID lpParameter)
{
    PSPI_XCH_LIST_ENTRY_T pXchListEntry;
    spiClass * pSpi = (spiClass *)lpParameter;
    DEBUGMSG(ZONE_THREAD, (TEXT("SpiProcessQueue:  lpParameter=0x%x\r\n"),lpParameter));

    // until queue processing thread termination requested
    while (!pSpi->m_bTerminate)
    {
        // while system is powering down 
        while (pSpi->m_dxCurrent == D4)
        {
            Sleep(1000);
        }

        // while our list of unprocessed queues has not been exhausted
        while (!IsListEmpty(&pSpi->m_ListHead)) 
        {
            // wrap linked list remove operation in critical section to 
            // serialize accesses with SPI_IOCTL_EXCHANGE
            EnterCriticalSection(&pSpi->m_spiCs);

            // get next list entry
            pXchListEntry = (PSPI_XCH_LIST_ENTRY_T) RemoveHeadList(&pSpi->m_ListHead);
        
            LeaveCriticalSection(&pSpi->m_spiCs);
        
            // check if mapping failed
            if (pXchListEntry == NULL)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("SpiProcessQueue:  MapCallerPtr failed!\r\n")));
            }
            // exchange packet mapped to our space
            else
            {
                // Wrap the exchange in critical section to 
                // serialize accesses with SPI_IOCTL_EXCHANGE
                EnterCriticalSection(&pSpi->m_spiDataXchCs);

                // do the exchange and update the exchange count
                if (pSpi->m_bUseDMA && pXchListEntry->xchPkt.xchRealPkt.pBusCnfg->usedma)
                {
                    //pXchListEntry->xchPkt.xchRealPkt.xchCnt = pSpi->SpiDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
                }
                else
                {
                    pXchListEntry->xchPkt.xchRealPkt.xchCnt = pSpi->SpiNonDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
                }
                
                LeaveCriticalSection(&pSpi->m_spiDataXchCs);

                CeFreeAsynchronousBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
                CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
                if (pXchListEntry->xchPkt.xchRealPkt.pBuf != NULL)
                {
                    CeFreeAsynchronousBuffer(pXchListEntry->xchPkt.marshalStub);
                    CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalStub);
                }

                ReleaseSemaphore(pSpi->m_hEnQSemaphere, 1, NULL);

                // signal that new data available in Rx message queue
                if(!SetEvent(pXchListEntry->xchPkt.xchRealPkt.xchEvent))
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("SpiProcessQueue: SetEvent failed\r\n")));
                } 
                CloseHandle(pXchListEntry->xchPkt.xchRealPkt.xchEvent);

                // free memory allocated to list entry
                HeapFree(pSpi->m_hHeap, 0, pXchListEntry);
            }
        } // while (!IsListEmpty(&m_ListHead))

        // wait for next list entry to arrive
        WaitForSingleObject(pSpi->m_hEnQEvent, INFINITE);

    }  // while (!m_bTerminate)

    pSpi->m_hThread = NULL;

    DEBUGMSG(ZONE_THREAD, (TEXT("SpiProcessQueue -\r\n")));
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SpiNonDMADataExchange
//
// Exchanges SPI data in Application Processor(CPU) Master mode.
//
// Parameters:
//      pXchPkt
//          [in] Points to exchange packet information.
//
// Returns:
//      Returns the number of data exchanges that completed successfully.
//   
//-----------------------------------------------------------------------------
UINT32 spiClass::SpiNonDMADataExchange(PSPI_XCH_PKT0_T pXchPkt)
{
    PSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;
    LPVOID pBuf = pXchPkt->pBuf;
    volatile UINT32 tmp;
    BOOL bXchDone;
    UINT32 xchCnt = 0;
    UINT32 (*pfnBufRd)(LPVOID);
    void (*pfnBufWrt)(LPVOID, UINT32);

    //BOOL bReqPolling;
    UINT8 bufIncr =0;

    // check all translated pointers
    if ((pBusCnfg == NULL) || (pBuf == NULL))
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
    if ((pBusCnfg->bitcount >= 1) && (pBusCnfg->bitcount <= 8))
    {
        // 8-bit access width
        pfnBufRd = SpiBufRd8;
        pfnBufWrt = SpiBufWrt8;
        bufIncr = sizeof(UINT8);
        HW_SSP_XFER_SIZE_WR(m_Index,1);
    }
    else if ((pBusCnfg->bitcount >= 9) && (pBusCnfg->bitcount <= 16))
    {
        // 16-bit access width
        pfnBufRd = SpiBufRd16;
        pfnBufWrt = SpiBufWrt16;
        bufIncr = sizeof(UINT16);
        HW_SSP_XFER_SIZE_WR(m_Index,2);
    }
    else if ((pBusCnfg->bitcount >= 17) && (pBusCnfg->bitcount <= 32))
    {
        // 32-bit access width
        pfnBufRd = SpiBufRd32;
        pfnBufWrt = SpiBufWrt32;
        bufIncr = sizeof(UINT32);
        HW_SSP_XFER_SIZE_WR(m_Index,4);
    }
    else
    {
        // unsupported access width
        DEBUGMSG(ZONE_WARN, (TEXT("SpiMasterDataExchange:  unsupported bitcount!\r\n")));
        return 0;
    }
    
    // set client SPI bus configuration based
    HW_SSP_CTRL0_WR(m_Index,pBusCnfg->SspCtrl0.U);

    if(pBusCnfg->bCmd)
    {
        HW_SSP_CMD0_WR(m_Index,pBusCnfg->SspCmd.U);
        HW_SSP_CMD1_WR(m_Index,pBusCnfg->SspArg.U);
    }

    bXchDone = FALSE;

    // until we are done with requested transfers
    while(!bXchDone)
    {
        if(pBuf != NULL)
        {
            // Process the SDMMC Data Buffer
            if( pBusCnfg->bRead == TRUE)
            {
                while (xchCnt < pXchPkt->xchCnt) 
                {
                    
                    //RETAILMSG(1, (TEXT("start exchange m_Index = %d\r\n"),m_Index));
                    // start exchange
                    HW_SSP_CTRL0_SET(m_Index,BM_SSP_CTRL0_RUN);
                    while((HW_SSP_STATUS_RD(m_Index) & BM_SSP_STATUS_FIFO_EMPTY) != 0);
                    {
                        tmp = HW_SSP_DATA_RD(m_Index);                           
                        // if receive data is not to be discarded
                        if (pBuf != NULL)
                        {
                            // get next Rx data from SPI FIFO
                            pfnBufWrt(pBuf, tmp);
                            // increment Rx Buffer to next data point
                            pBuf = (LPVOID) ((UINT) pBuf + bufIncr);
                        }

                        // increment Rx exchange counter
                        xchCnt++;
                    }
                // set flag to indicate requested exchange done
                bXchDone = TRUE;
                }
            }
            else 
            {
                //RETAILMSG(1, (TEXT("load FIFO m_Index = %d\r\n"),m_Index));
                // load FIFO until full, or until we run out of data
                while (xchCnt < pXchPkt->xchCnt)
                {
                    HW_SSP_CTRL0_SET(m_Index,BM_SSP_CTRL0_RUN);
                    // put next Tx data into SPI FIFO
                    if((HW_SSP_STATUS_RD(m_Index) & BM_SSP_STATUS_FIFO_FULL) == 0)
                    {
                        HW_SSP_DATA_WR(m_Index,pfnBufRd(pBuf));
                    }
                    else
                    {
                        while((HW_SSP_STATUS_RD(m_Index) & BM_SSP_STATUS_FIFO_EMPTY) == 0);
                    }
                    // increment Tx Buffer to next data point
                    pBuf = (LPVOID) ((UINT) pBuf + bufIncr);
                    
                    //Sleep(10);
                    //tmp = INREG32(&m_pSPI->DATA);

                    // increment exchange counter
                    xchCnt++;
                }
                // set flag to indicate requested exchange done
                bXchDone = TRUE;
            }
        }
        else{
              // Process the SDMMC Command
                if(pXchPkt->xchCnt == 0)
                {
                // start exchange
                    HW_SSP_CTRL0_SET(m_Index,BM_SSP_CTRL0_RUN);
                    bXchDone = TRUE;
                }
        }
    } // while(!bXchDone)
    
    DEBUGMSG(ZONE_THREAD, (TEXT("SpiDataExchange -\r\n")));
    return xchCnt;
}

//-----------------------------------------------------------------------------
//
// Function: SpiBufRd8
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
UINT32 spiClass::SpiBufRd8(LPVOID pBuf)
{
    UINT8 *p;
    p = (UINT8 *) pBuf;
    return *p;
}


//-----------------------------------------------------------------------------
//
// Function: SpiBufRd16
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
UINT32 spiClass::SpiBufRd16(LPVOID pBuf)
{
    UINT16 *p;
    p = (UINT16 *) pBuf;
    return *p;
}


//-----------------------------------------------------------------------------
//
// Function: SpiBufRd32
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
UINT32 spiClass::SpiBufRd32(LPVOID pBuf)
{
    UINT32 *p;
    p = (UINT32 *) pBuf;
    return *p;
}


//-----------------------------------------------------------------------------
//
// Function: SpiBufWrt8
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
void spiClass::SpiBufWrt8(LPVOID pBuf, UINT32 data)
{
    UINT8 *p;
    p  = (UINT8 *) pBuf;
    *p = (UINT8) data;
}


//-----------------------------------------------------------------------------
//
// Function: SpiBufWrt16
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
void spiClass::SpiBufWrt16(LPVOID pBuf, UINT32 data)
{
    UINT16 *p;
    p  = (UINT16 *) pBuf;
    *p = (UINT16) data;
}


//-----------------------------------------------------------------------------
//
// Function: SpiBufWrt32
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
void spiClass::SpiBufWrt32(LPVOID pBuf, UINT32 data)
{
    UINT32 *p;
    p  = (UINT32 *) pBuf;
    *p = data;
}


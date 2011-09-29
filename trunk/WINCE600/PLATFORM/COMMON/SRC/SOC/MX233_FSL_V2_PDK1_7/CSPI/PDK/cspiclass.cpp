//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
//-----------------------------------------------------------------------------
//
//  File:  cspiClass.cpp
//
//  Provides the implementation of the CSPI bus driver to support CSPI
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

#include "csp.h"

//#include "mx233_ssp.h"
#include "cspiclass.h"

#include <Devload.h>

#define DBGMSGON TRUE
//-----------------------------------------------------------------------------
// External Functions
extern "C" BOOL BSPCSPISetIOMux(UINT32 Index);
extern "C" BOOL BSPCSPIReleaseIOMux(UINT32 dwIndex);
extern "C" BOOL BSPCspiIsAllowPolling(UINT8 Index);
extern "C" BOOL BSPCspiIsDMAEnabled(UINT8 Index);
extern "C" DWORD CspCSPIGetBaseRegAddr(UINT32 index);
extern "C" BOOL BSPCspiExchange(VOID *lpInBuf, LPVOID pRxBuf, UINT32 nOutBufSize, LPDWORD BytesReturned);

//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------
// Defines

// The frequency of the output signal bit clock SSP_SCK is defined as follows:
//                 SSPCLK
// SSP_SCK  =   -----------------
//               CLOCK_DIVIDE * (1+ CLOCK_RATE)
#define DATA_TIMEOUT  0xFFFF
#define ETH_CLOCK_DIVIDE  0x4     // SSPCLOCK/8
#define SD_CLOCK_DIVIDE   0x2     // SSPCLOCK/60  
#define CLOCK_RATE    0x0

#define SD_SSP_FREQUENCY  48000   // frequency in KHZ
#define ETH_SSP_FREQUENCY 48000
#define SD1_SSP_FREQUENCY 50000   // frequency in KHZ
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


cspiClass::cspiClass()
{
    DEBUGMSG(ZONE_INIT, (TEXT("cspiClass +\r\n")));
    m_pCSPI = NULL;
    m_cspiOpenCount = 0;
    m_hEnQEvent = NULL;
    m_bTerminate = FALSE;
    m_dxCurrent = D0;
    m_Index = 0;
    m_bUsePolling = FALSE;
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
    m_Index = Index;
   
//#ifdef POLLING_MODE
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

        // create Semaphore for process queue thread signaling
    //      pEventAttributes = NULL (must be NULL)
    //      lpName = NULL => object created without a name
    m_hEnQSemaphere = CreateSemaphore(NULL, CSPI_MAX_QUEUE_LENGTH, CSPI_MAX_QUEUE_LENGTH, NULL);

    // check if CreateEvent failed
    if (m_hEnQSemaphere == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  CreateSemaphore failed!\r\n")));
        goto Error;
    }
//#endif
    // Get the Base Address according to Index
    phyAddr.QuadPart = CspCSPIGetBaseRegAddr(Index);

    if(!phyAddr.QuadPart)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  invalid CSPI instance!\r\n")));
       goto Error;
    }

// Get the Base Address and IRQ according to Index
    m_bUseDMA       = BSPCspiIsDMAEnabled((UINT8)Index);

    m_bAllowPolling = BSPCspiIsAllowPolling((UINT8)Index);

    // Configure IOMUX 
    BSPCSPISetIOMux(Index);

    // Map peripheral physical address to virtual address
    m_pCSPI = (PCSP_SSP_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_SSP_REGS), FALSE);

    // Check if virtual mapping failed
    if (m_pCSPI == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  MmMapIoSpace failed!\r\n")));
        goto Error;
    }
   
    OUTREG32 ( &m_pCSPI->CTRL0[CLRREG], BM_SSP_CTRL0_CLKGATE);
    // Reset the Block
    OUTREG32 ( &m_pCSPI->CTRL0[SETREG], BM_SSP_CTRL0_SFTRST);

    // Release the Block from Reset and starts the clock
    OUTREG32 ( &m_pCSPI->CTRL0[CLRREG], BM_SSP_CTRL0_SFTRST | BM_SSP_CTRL0_CLKGATE);

    // create CSPI critical section
    InitializeCriticalSection(&m_cspiCs);

//#ifdef POLLING_MODE
    // create CSPI Data Exchange critical section
    InitializeCriticalSection(&m_cspiDataXchCs);

    // initialize CSPI linked list of client queue entries
    InitializeListHead(&m_ListHead);

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
//#endif
    DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize: &m_pCSPI=0x%x\r\n"),&m_pCSPI));

    m_cspiOpenCount++;    

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
    BSPCSPIReleaseIOMux(m_Index);

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
    // free the virtual space allocated for CSPI memory map
    if (m_pCSPI != NULL)
    {
        MmUnmapIoSpace(m_pCSPI, sizeof(CSP_SSP_REGS));
        m_pCSPI = NULL;
    }

    // delete the critical section
    DeleteCriticalSection(&m_cspiCs);

//#ifdef POLLING_MODE
    // delete the Data Exchange critical section
    DeleteCriticalSection(&m_cspiDataXchCs);
//#endif

    DEBUGMSG(ZONE_CLOSE, (TEXT("CspiRelease -\r\n")));
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
BOOL cspiClass::ConfigureSSP(SSP_INIT * sInit)
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
    sControl0.B.XFER_COUNT = sInit->u16TransferCount;

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
    OUTREG32(&m_pCSPI->CTRL0[BASEREG],sControl0.U);
    OUTREG32(&m_pCSPI->CTRL1[BASEREG],sControl1.U);

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
BOOL cspiClass::SSPResetAfterError(SSP_RESETCONFIG * sConfigparams)
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
BOOL cspiClass::SSPCheckErrors(VOID)
{
    SSP_CTRL1  sControl1;
    SSP_STATUS sStatus;

    // read in the value of the registers
    sControl1.U = INREG32(&m_pCSPI->CTRL1);

    sStatus.U   = INREG32(&m_pCSPI->STATUS);

    // Check CTRL1 and STATUS registers for errors
    if(sControl1.B.RESP_TIMEOUT_IRQ || sStatus.B.RESP_TIMEOUT){
        DEBUGMSG(DBGMSGON, (TEXT("SSP: ERROR - Response Timeout\r\n")));
        return FALSE;
    }
    if(sStatus.B.RESP_CRC_ERR){
        DEBUGMSG(DBGMSGON, (TEXT("SSP: Error - Response CRC Error\r\n")));
        return FALSE;
    }
    if(sControl1.B.RESP_ERR_IRQ || sStatus.B.RESP_ERR){
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Response Error\r\n")));
        return FALSE;
    }
    if(sControl1.B.FIFO_OVERRUN_IRQ || sStatus.B.FIFO_OVRFLW){
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Receive Overflow\r\n")));
        return FALSE;
    }
    if(sControl1.B.RECV_TIMEOUT_IRQ){
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Receive Timeout\r\n")));
        return FALSE;
    }
    if(sControl1.B.DATA_CRC_IRQ || sStatus.B.DATA_CRC_ERR){
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Data CRC Error\r\n")));
        return FALSE;
    }
    if(sControl1.B.DATA_TIMEOUT_IRQ){
        DEBUGMSG(DBGMSGON, (TEXT("SSP:  ERROR - Data Timeout\r\n")));
        return FALSE;
    }
    if(sStatus.B.TIMEOUT){
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
BOOL cspiClass::SSPGetIrqStatus(SSP_IRQ eIrq)
{
    SSP_CTRL1  sControl1;
    BOOL Value =0;

    // read in the value of the registers
    sControl1.U = INREG32(&m_pCSPI->CTRL1);

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
BOOL cspiClass::SSPClearIrq(SSP_IRQ eIrq)
{

    // Returns the active SSP interrupt
    switch (eIrq)
    {
        case SSP_IRQ_SDIO: 
            OUTREG32(&m_pCSPI->CTRL1[CLRREG],BM_SSP_CTRL1_SDIO_IRQ);
            break;                                        
        case SSP_IRQ_RESP_ERR: 
            OUTREG32(&m_pCSPI->CTRL1[CLRREG],BM_SSP_CTRL1_RESP_ERR_IRQ);
            break;                                        
        case SSP_IRQ_RESP_TIMEOUT:                        
            OUTREG32(&m_pCSPI->CTRL1[CLRREG],BM_SSP_CTRL1_RESP_TIMEOUT_IRQ);
            break;                                        
        case SSP_IRQ_DATA_TIMEOUT:                        
            OUTREG32(&m_pCSPI->CTRL1[CLRREG],BM_SSP_CTRL1_DATA_TIMEOUT_IRQ);
            break;                                        
        case SSP_IRQ_DATA_CRC:                            
            OUTREG32(&m_pCSPI->CTRL1[CLRREG],BM_SSP_CTRL1_DATA_CRC_IRQ);
            break;                                        
        case SSP_IRQ_FIFO_UNDERRUN:
            OUTREG32(&m_pCSPI->CTRL1[CLRREG],BM_SSP_CTRL1_FIFO_UNDERRUN_IRQ);
            break;
        case SSP_IRQ_CEATA_CCS_ERR:
            OUTREG32(&m_pCSPI->CTRL1[CLRREG],BM_SSP_CTRL1_CEATA_CCS_ERR_IRQ);
            break;
        case SSP_IRQ_RECV_TIMEOUT:
            OUTREG32(&m_pCSPI->CTRL1[CLRREG],BM_SSP_CTRL1_RECV_TIMEOUT_IRQ);
            break;
        case SSP_IRQ_FIFO_OVERRUN:
            OUTREG32(&m_pCSPI->CTRL1[CLRREG],BM_SSP_CTRL1_FIFO_OVERRUN_IRQ);
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
BOOL cspiClass::SSPConfigTiming(SSP_SPEED eSpeed)
{
    UINT32 u32Timing,uClkDiv =0;
    SSP_CTRL1  sControl1;
    UINT32 u32SSPFreq, rootfreq, u32Div; 

    sControl1.U = INREG32(&m_pCSPI->CTRL1);

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
    else if (eSpeed == SPI_TRANSFER_HS_FAST_SPEED){
        uClkDiv = ETH_CLOCK_DIVIDE;  
        u32SSPFreq = ETH_SSP_FREQUENCY; // frequency in KHZ
    }
    else {
        uClkDiv = SD_CLOCK_DIVIDE;  
        u32SSPFreq = SD1_SSP_FREQUENCY; // frequency in KHZ
    }
    //DDKClockSetSspClk(&u32SSPFreq, TRUE);
    DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_IO, &rootfreq);
    u32Div = rootfreq / (u32SSPFreq*1000) + 1;
    DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP_CLK, DDK_CLOCK_BAUD_SOURCE_REF_IO, u32Div );
    DDKClockSetGatingMode(DDK_CLOCK_GATE_SSP_CLK, FALSE);
    
    u32Timing = (UINT32)(DATA_TIMEOUT << 16) | uClkDiv<<8  | CLOCK_RATE;         

    //DEBUGMSG(DBGMSGON, (TEXT("DDI_SSP: u32Timing= 0x%X, uClkDiv = %d, uClkRate = %d.\r\n"),u32Timing, uClkDiv, uClkRate));

    OUTREG32(&m_pCSPI->TIMING[BASEREG],u32Timing);

    OUTREG32(&m_pCSPI->CTRL1[BASEREG],sControl1.U);

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

BOOL cspiClass::SSPEnableErrIrq(BOOL bEnable)
{
    //UINT32 u32SspCtrl1Reg;

    if(bEnable){
        OUTREG32(&m_pCSPI->CTRL1[SETREG],SSP_CTRL1_HANDLED_ERRORS_MASK);
    }
    else{
        /*u32SspCtrl1Reg = INREG32(&m_pCSPI->CTRL1);*/
        OUTREG32(&m_pCSPI->CTRL1[CLRREG],SSP_CTRL1_HANDLED_ERRORS_MASK);
    }

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

BOOL cspiClass::SSPDisableErrIrq(VOID)
{

    OUTREG32(&m_pCSPI->CTRL1[CLRREG],SSP_CTRL1_IRQS_MASK);
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
BOOL cspiClass::DumpRegister(VOID)
{

    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_CTRL0        = 0x%x\r\n"),  INREG32(&m_pCSPI->CTRL0) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_CTRL1        = 0x%x\r\n"),  INREG32(&m_pCSPI->CTRL1) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_CMD0         = 0x%x\r\n"),  INREG32(&m_pCSPI->CMD0) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_CMD1         = 0x%x\r\n"),  INREG32(&m_pCSPI->CMD1) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_COMPREF      = 0x%x\r\n"),  INREG32(&m_pCSPI->COMPREF) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_COMPMASK     = 0x%x\r\n"),  INREG32(&m_pCSPI->COMPMASK) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_TIMING       = 0x%x\r\n"),  INREG32(&m_pCSPI->TIMING) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_DATA         = 0x%x\r\n"),  INREG32(&m_pCSPI->DATA) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_SDRESP0      = 0x%x\r\n"),  INREG32(&m_pCSPI->SDRESP0) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_SDRESP1      = 0x%x\r\n"),  INREG32(&m_pCSPI->SDRESP1) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_SDRESP2      = 0x%x\r\n"),  INREG32(&m_pCSPI->SDRESP2) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_SDRESP3      = 0x%x\r\n"),  INREG32(&m_pCSPI->SDRESP3) ));
    DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_STATUS       = 0x%x\r\n"),  INREG32(&m_pCSPI->STATUS) ));
    //DEBUGMSG(DBGMSGON, (TEXT("SSP: SSP_DEBUG        = 0x%x\r\n"),  INREG32(&m_pCSPI->DEBUG) ));
   
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
VOID cspiClass:: SSP_Reset(VOID)
{
     // Reset the block.

    // Prepare for soft-reset by making sure that SFTRST is not currently
    // asserted.
    OUTREG32 (&m_pCSPI->CTRL0[CLRREG],BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond for SFTRST to deassert.
    INREG32(&m_pCSPI->CTRL0);
    INREG32(&m_pCSPI->CTRL0);

    // Also clear CLKGATE so we can wait for its assertion below.
    OUTREG32 (&m_pCSPI->CTRL0[CLRREG],BM_SSP_CTRL0_CLKGATE);

    // Now soft-reset the hardware.
    OUTREG32 ( &m_pCSPI->CTRL0[SETREG], BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond for SFTRST to deassert.
    INREG32(&m_pCSPI->CTRL0);
    INREG32(&m_pCSPI->CTRL0);

    // Deassert SFTRST.
    OUTREG32 ( &m_pCSPI->CTRL0[CLRREG], BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond .
    INREG32(&m_pCSPI->CTRL0);
    INREG32(&m_pCSPI->CTRL0);

    // Release the Block from Reset and starts the clock
    OUTREG32 ( &m_pCSPI->CTRL0[CLRREG], BM_SSP_CTRL0_CLKGATE);

    // Wait at least a microsecond.
    INREG32(&m_pCSPI->CTRL0);
    INREG32(&m_pCSPI->CTRL0);
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
    /*DWORD cbSrc;*/
    HRESULT result;

    struct {
        BOOL bAllocListEntry;
        BOOL bMarshalpBusCnfg;
        BOOL bMarshalBuf;
        BOOL bMarshalBufAsync;
        BOOL bMarshalpBusCnfgAsync;
    } resource;

    CALLER_STUB_T marshalEventStub;
    DEBUGMSG(1, (TEXT("CspiEnqueue +\r\n")));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CspiEnqueue +\r\n")));

    resource.bAllocListEntry = FALSE;
    resource.bMarshalpBusCnfg = FALSE;
    resource.bMarshalBuf = FALSE;
    resource.bMarshalBufAsync = FALSE;
    resource.bMarshalpBusCnfgAsync = FALSE;

    pXchListEntry = NULL;

    // allocate space for new list entry
    pXchListEntry = (PCSPI_XCH_LIST_ENTRY_T)HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY,sizeof(CSPI_XCH_LIST_ENTRY_T));

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
                FALSE);

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
        DEBUGMSG(ZONE_FUNCTION, (TEXT("CspiEnqueue: Sending the package immediately\r\n")));

        // Wrap the exchange in critical section to 
        // serialize accesses with CspiProcessQueue thread
        EnterCriticalSection(&m_cspiDataXchCs);

        if (m_bUseDMA && pXchListEntry->xchPkt.xchRealPkt.pBusCnfg->usedma)
        {
            //pXchListEntry->xchPkt.xchRealPkt.xchCnt = CspiDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
        }
        else
        {
            pXchListEntry->xchPkt.xchRealPkt.xchCnt = CspiNonDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
        }

        LeaveCriticalSection(&m_cspiDataXchCs);
            
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
    // wait for cspi asynchronous queue availiable
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
    pXchListEntry->xchPkt.xchRealPkt.pBusCnfg = (PCSPI_BUSCONFIG_T)pXchListEntry->xchPkt.marshalBusCnfgStub.m_pLocalAsync;

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

                // do the exchange and update the exchange count
                if (pCspi->m_bUseDMA && pXchListEntry->xchPkt.xchRealPkt.pBusCnfg->usedma)
                {
                    //pXchListEntry->xchPkt.xchRealPkt.xchCnt = pCspi->CspiDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
                }
                else
                {
                    pXchListEntry->xchPkt.xchRealPkt.xchCnt = pCspi->CspiNonDMADataExchange(&pXchListEntry->xchPkt.xchRealPkt);
                }
                
                LeaveCriticalSection(&pCspi->m_cspiDataXchCs);

                CeFreeAsynchronousBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
                CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalBusCnfgStub);
                if (pXchListEntry->xchPkt.xchRealPkt.pBuf != NULL)
                {
                    CeFreeAsynchronousBuffer(pXchListEntry->xchPkt.marshalStub);
                    CeCloseCallerBuffer(pXchListEntry->xchPkt.marshalStub);
                }

                ReleaseSemaphore(pCspi->m_hEnQSemaphere, 1, NULL);

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
    PCSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;
    LPVOID pBuf = pXchPkt->pBuf;
    volatile UINT32 tmp;
    BOOL bXchDone;
    UINT32 xchCnt = 0;
    UINT32 (*pfnBufRd)(LPVOID);
    void (*pfnBufWrt)(LPVOID, UINT32);

    //BOOL bReqPolling;
    UINT8 bufIncr =0;
    DEBUGMSG(1, (TEXT("CspiDataExchange: &m_pCSPI=0x%x\r\n"),&m_pCSPI));

    DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange: &m_pCSPI=0x%x\r\n"),&m_pCSPI));

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
        pfnBufRd = CspiBufRd8;
        pfnBufWrt = CspiBufWrt8;
        bufIncr = sizeof(UINT8);
    }
    else if ((pBusCnfg->bitcount >= 9) && (pBusCnfg->bitcount <= 16))
    {
        // 16-bit access width
        pfnBufRd = CspiBufRd16;
        pfnBufWrt = CspiBufWrt16;
        bufIncr = sizeof(UINT16);
    }
    else if ((pBusCnfg->bitcount >= 17) && (pBusCnfg->bitcount <= 32))
    {
        // 32-bit access width
        pfnBufRd = CspiBufRd32;
        pfnBufWrt = CspiBufWrt32;
        bufIncr = sizeof(UINT32);
    }
    else
    {
        // unsupported access width
        DEBUGMSG(ZONE_WARN, (TEXT("CspiMasterDataExchange:  unsupported bitcount!\r\n")));
        return 0;
    }
    
    // set client CSPI bus configuration based
    OUTREG32(&m_pCSPI->CTRL0[BASEREG], pBusCnfg->SspCtrl0.U);
    if(pBusCnfg->bCmd){
        OUTREG32(&m_pCSPI->CMD0[BASEREG], pBusCnfg->SspCmd.U);
        OUTREG32(&m_pCSPI->CMD1[BASEREG], pBusCnfg->SspArg.U);
    }

    bXchDone = FALSE;

    // until we are done with requested transfers
    while(!bXchDone)
    {
        if(pBuf != NULL)
        {
            // Process the SDMMC Data Buffer
            if( pBusCnfg->bRead == TRUE){

                while (xchCnt < pXchPkt->xchCnt) {
                    
                    // start exchange
                    OUTREG32(&m_pCSPI->CTRL0[SETREG],BM_SSP_CTRL0_RUN);
                    while((INREG32(&m_pCSPI->STATUS) & BM_SSP_STATUS_FIFO_EMPTY) != 0);
                        tmp = INREG32(&m_pCSPI->DATA);
                    // if receive data is not to be discarded
                    if (pBuf != NULL)
                    {
                        // get next Rx data from CSPI FIFO
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
            else {
                // load FIFO until full, or until we run out of data
                while (xchCnt < pXchPkt->xchCnt)
                {
                    OUTREG32(&m_pCSPI->CTRL0[SETREG],BM_SSP_CTRL0_RUN);
                        // put next Tx data into CSPI FIFO
                    if((INREG32(&m_pCSPI->STATUS) & BM_SSP_STATUS_FIFO_FULL) == 0)
                        OUTREG32(&m_pCSPI->DATA[BASEREG], pfnBufRd(pBuf));
                    else
                        while((INREG32(&m_pCSPI->STATUS) & BM_SSP_STATUS_FIFO_EMPTY) == 0);
                    // increment Tx Buffer to next data point
                    pBuf = (LPVOID) ((UINT) pBuf + bufIncr);
                    
                    //Sleep(10);
                    //tmp = INREG32(&m_pCSPI->DATA);

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
                  OUTREG32(&m_pCSPI->CTRL0[SETREG],BM_SSP_CTRL0_RUN);
                  bXchDone = TRUE;
                }
        }
    } // while(!bXchDone)
    
    DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange -\r\n")));
    return xchCnt;
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


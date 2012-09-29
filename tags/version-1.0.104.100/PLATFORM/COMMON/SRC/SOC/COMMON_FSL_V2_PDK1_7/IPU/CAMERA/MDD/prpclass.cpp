 //
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  PrpClass.cpp
//
//  Implementation of Preprocessor driver class
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>

#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#include "common_ipu.h"
#include "common_macros.h"
#pragma warning(pop)
#include "Ipu.h"
#include "cameradbg.h"
#include "CameraPDDProps.h"
#include "display_vf.h"
#include "IpuModuleInterfaceClass.h"
#include "IpuBufferManager.h"
#include "PrpClass.h"

#pragma warning(disable: 4100)
#pragma warning(disable: 4189)

#pragma warning(disable: 4127)

#pragma warning(disable: 4389)
#pragma warning(disable: 4505)
#pragma warning(disable: 4701)

//------------------------------------------------------------------------------
// External Functions
//For RINGO TVIN +
extern BYTE BSPGetTVinType(void);
extern int BSPGetDefaultCameraFromRegistry();


//------------------------------------------------------------------------------
// External Variables
//For Ringo TVIN +
//extern csiSensorId gSensorInUse;
extern BYTE        gLastTVinType;


//------------------------------------------------------------------------------
// Defines
#define PRP_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define PRP_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#define THREAD_PRIORITY                            250

#define PRP_MAX_NUM_BUFFERS                        100

#define ENC_BUF_IDLE_QUEUE_NAME            L"Enc idle queue"
#define ENC_BUF_READY_QUEUE_NAME           L"Enc ready queue"
#define ENC_BUF_FILLED_QUEUE_NAME          L"Enc filled queue"
#define ENC_ROT_BUF_IDLE_QUEUE_NAME        L"Enc rot idle queue"
#define ENC_ROT_BUF_READY_QUEUE_NAME       L"Enc rot ready queue"
#define ENC_ROT_BUF_FILLED_QUEUE_NAME      L"Enc rot filled queue"
#define VF_BUF_IDLE_QUEUE_NAME             L"Vf idle queue"
#define VF_BUF_READY_QUEUE_NAME            L"Vf ready queue"
#define VF_BUF_FILLED_QUEUE_NAME           L"Vf filled queue"
#define VF_ROT_BUF_IDLE_QUEUE_NAME         L"Vf rot idle queue"
#define VF_ROT_BUF_READY_QUEUE_NAME        L"Vf rot ready queue"
#define VF_ROT_BUF_FILLED_QUEUE_NAME       L"Vf rot filled queue"

#define PRP_MAX_NUM_BUFFERS                        100

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

// Static variables from parent class
// must be defined before they can be used.
PCSP_IPU_REGS IpuModuleInterfaceClass::m_pIPU = NULL;
HANDLE IpuModuleInterfaceClass::m_hIpuMutex = NULL;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// External Functions
extern "C" UINT32 IPUGetTPMEncCSC1Addr();
extern "C" UINT32 IPUGetTPMVFCSC1Addr();



//------------------------------------------------------------------------------
// Local Functions
//static void CsiTestPatternOn(PCSP_IPU_REGS);
static void dumpChannelParams(pPrpIDMACChannelParams);
static void dumpCoeffs(pPrpCSCCoeffs);
static void dumpInterruptRegisters(PCSP_IPU_REGS);
static void dumpIpuRegisters(PCSP_IPU_REGS);
static void ReadVfDMA(PCSP_IPU_REGS);

//-----------------------------------------------------------------------------
//
// Function: PrpClass
//
// Preprocessor class constructor.  Calls PrpInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PrpClass::PrpClass(void)
{
    PrpInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~PrpClass
//
// The destructor for PrpClass.  Calls PrpDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PrpClass::~PrpClass(void)
{
    PrpDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: PrpInit
//
// This function initializes the Image Converter (preprocessor).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpInit(void)
{
    MSGQUEUEOPTIONS queueOptions;

    PRP_FUNCTION_ENTRY();

    // open handle to the IPU_BASE driver in order to enable IC module
    hIPUBase = CreateFile(TEXT("IPU1:"),        // "special" file name
        GENERIC_READ|GENERIC_WRITE,            // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                                                      // security attributes (=NULL)
        OPEN_EXISTING,                                     // creation disposition
        FILE_FLAG_RANDOM_ACCESS,                 // flags and attributes
        NULL);                                                    // template file (ignored)
    if (hIPUBase == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for IPU Interrupt\r\n"), __WFUNCTION__));
    m_hPrpIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_PRP_INTR_EVENT);
    if (m_hPrpIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for IPU Interrupt failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Events to signal pin that frame is ready
    m_hEncEOFEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hEncEOFEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Encoding EOF\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_hVfEOFEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hVfEOFEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Viewfinding EOF\r\n"), __WFUNCTION__));
        return FALSE;
    }


    // Initialize buffer management handles
    m_hEncBufWaitList[prpBuf0RequestEvent] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!m_hEncBufWaitList[prpBuf0RequestEvent])
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request Encoding Buffer0\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_hEncBufWaitList[prpBuf1RequestEvent] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!m_hEncBufWaitList[prpBuf1RequestEvent])
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request Encoding Buffer1\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_hEncRotBufWaitList[prpBuf0RequestEvent] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!m_hEncRotBufWaitList[prpBuf0RequestEvent])
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request Encoding Rotation Buffer0\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_hEncRotBufWaitList[prpBuf1RequestEvent] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!m_hEncRotBufWaitList[prpBuf1RequestEvent])
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request Encoding Rotation Buffer1\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_hVfBufWaitList[prpBuf0RequestEvent] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!m_hVfBufWaitList[prpBuf0RequestEvent])
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request Viewfinding Buffer0\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_hVfBufWaitList[prpBuf1RequestEvent] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!m_hVfBufWaitList[prpBuf1RequestEvent])
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request Viewfinding Buffer1\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_hVfRotBufWaitList[prpBuf0RequestEvent] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!m_hVfRotBufWaitList[prpBuf0RequestEvent])
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request Viewfinding Rotation Buffer0\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_hVfRotBufWaitList[prpBuf1RequestEvent] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!m_hVfRotBufWaitList[prpBuf1RequestEvent])
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request Viewfinding Rotation Buffer1\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Create queues for read messages and write messages
    queueOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    queueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    queueOptions.dwMaxMessages = PRP_MAX_NUM_BUFFERS;
    queueOptions.cbMaxMessage = sizeof(DISPLAY_BUFFER);
    queueOptions.bReadAccess = TRUE; // we need read-access to msgqueue

    // Create read handles to idle and busy queues
    m_hReadVfBufferQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadVfBufferQueue)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error creating Viewfinding Buffer queue.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    queueOptions.bReadAccess = FALSE; // we need write-access to msgqueue

    // Create read handles to idle and busy queues
    m_hWriteVfBufferQueue = OpenMsgQueue(GetCurrentProcess(), m_hReadVfBufferQueue, &queueOptions);
    if (!m_hWriteVfBufferQueue)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error opening viewfinding buffer queue for writing.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    InitializeCriticalSection(&m_csEncStopping);
    InitializeCriticalSection(&m_csVfStopping);

    InitializeCriticalSection(&m_csPrpEnable);

    m_bPrpEnabled = FALSE;

    pEncBufferManager = new IpuBufferManager();
    pVfBufferManager = new IpuBufferManager();
    pEncRotBufferManager = new IpuBufferManager();
    pVfRotBufferManager = new IpuBufferManager();

    m_bEncRotBuffersAllocated = FALSE;
    m_bVfRotBuffersAllocated = FALSE;

    m_iEncNumBuffers = 0;
    m_iVfNumBuffers = 0;
    m_iEncBufSize = 0;
    m_iVfBufSize = 0;

    m_bVfConfigured = FALSE;
    m_bEncConfigured = FALSE;

    m_hDisplay = NULL;

    m_bEncCSC = FALSE;
    m_bVfCSC = FALSE;

    m_bVfDirectDisplay = FALSE;
    m_bVfDisplayActive = FALSE;
    m_bADCDirect = FALSE;

    m_bEncRestartBufferLoop = FALSE;
    m_bVfRestartBufferLoop = FALSE;
    m_bEncRotRestartBufferLoop = FALSE;
    m_bVfRotRestartBufferLoop = FALSE;
    m_bEncRestartISRLoop = FALSE;
    m_bVfRestartISRLoop = FALSE;

    m_bEncRunning = FALSE;
    m_bVfRunning = FALSE;

    m_bEncFlipRot = FALSE;
    m_bVfFlipRot = FALSE;

    m_iEncFrameCount = 0;
    m_iVfFrameCount = 0;

    m_iVfBuf0Ready = FALSE;
    m_iVfBuf1Ready = FALSE;
    m_iEncBuf0Ready = FALSE;
    m_iEncBuf1Ready = FALSE;

    //For Ringo TVIN+
    m_bNTSCtoPAL = FALSE;
    // Get camera-in-use from registry key. Otherwise use default
    m_iCamType = BSPGetDefaultCameraFromRegistry();
    if ( m_iCamType == -1)
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Interrupt initialization failed! (IPU Error Interrupt)\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_hExitPrpISRThread = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;
    if(!m_hExitPrpISRThread)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Exit PrpISRThread\r\n"), __WFUNCTION__));
        return FALSE;
    }
    m_hExitPrpEncThread = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;
    if(!m_hExitPrpEncThread)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Exit PrpEncBufferWorkerThread\r\n"), __WFUNCTION__));
        return FALSE;
    }
    m_hExitPrpEncRotThread = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;
    if(!m_hExitPrpEncRotThread)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Exit PrpEncRotBufferWorkerThread\r\n"), __WFUNCTION__));
        return FALSE;
    }
    m_hExitPrpVfThread = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;
    if(!m_hExitPrpVfThread)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Exit PrpVfBufferWorkerThread\r\n"), __WFUNCTION__));
        return FALSE;
    }
    m_hExitPrpVfRotThread = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;
    if(!m_hExitPrpVfRotThread)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Exit PrpVfRotBufferWorkerThread\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Initialize thread for Preprocessor ISR
    //      pThreadAttributes = NULL (must be NULL)
    //      dwStackSize = 0 => default stack size determined by linker
    //      lpStartAddress = CspiProcessQueue => thread entry point
    //      lpParameter = NULL => point to thread parameter
    //      dwCreationFlags = 0 => no flags
    //      lpThreadId = NULL => thread ID is not returned
    m_hPrpISRThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrpIntrThread, this, 0, NULL);

    if (m_hPrpISRThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create camera ISR thread success\r\n"), __WFUNCTION__));
        CeSetThreadPriority(m_hPrpISRThread, 100);//THREAD_PRIORITY_TIME_CRITICAL);
    }

    // Initialize encoding buffer worker thread
    m_hPrpEncBufThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrpEncBufferWorkerThread, this, 0, NULL);

    if (m_hPrpEncBufThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create encoding buffer worker thread success\r\n"), __WFUNCTION__));
        CeSetThreadPriority(m_hPrpEncBufThread, 100);//THREAD_PRIORITY_TIME_CRITICAL);        
    }

    // Initialize encoding buffer worker thread
    m_hPrpEncRotBufThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrpEncRotBufferWorkerThread, this, 0, NULL);

    if (m_hPrpEncRotBufThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create rotation for encoding buffer worker thread success\r\n"), __WFUNCTION__));
        CeSetThreadPriority(m_hPrpEncRotBufThread, 100);//THREAD_PRIORITY_TIME_CRITICAL);        
    }

    // Initialize viewfinding buffer worker thread
    m_hPrpVfBufThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrpVfBufferWorkerThread, this, 0, NULL);

    if (m_hPrpVfBufThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create viewfinding buffer worker thread success\r\n"), __WFUNCTION__));
        CeSetThreadPriority(m_hPrpVfBufThread, 100);//THREAD_PRIORITY_TIME_CRITICAL);        
    }

    // Initialize viewfinding buffer worker thread
    m_hPrpVfRotBufThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrpVfRotBufferWorkerThread, this, 0, NULL);

    if (m_hPrpVfRotBufThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create rotation for viewfinding buffer worker thread success\r\n"), __WFUNCTION__));
        CeSetThreadPriority(m_hPrpVfRotBufThread, 100);//THREAD_PRIORITY_TIME_CRITICAL);        
    }

#ifdef RINGO_FIX_MEM_ALLOC_ISSUE
    // Allocate buffers
    if (!PrpAllocateVfBuffers(NUM_PIN_BUFFER, MAX_IMAGE_SIZE))
    {
        ERRORMSG(TRUE,(_T("%s: Prp Vf Buffer allocation failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    if (!PrpAllocateEncBuffers(NUM_PIN_BUFFER, MAX_IMAGE_SIZE))
    {
        ERRORMSG(TRUE,(_T("%s: Prp Enc Buffer allocation failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }

#endif

    PRP_FUNCTION_EXIT();

    return TRUE;

Error:
    PrpDeinit();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpDeinit
//
// This function deinitializes the Image Converter (Preprocessor).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpDeinit(void)
{
    PRP_FUNCTION_ENTRY();

    if (m_hPrpIntrEvent != NULL)
    {
        CloseHandle(m_hPrpIntrEvent);
        m_hPrpIntrEvent = NULL;
    }

    CloseHandle(m_hEncEOFEvent);
    m_hEncEOFEvent = NULL;

    CloseHandle(m_hVfEOFEvent);
    m_hVfEOFEvent = NULL;

    CloseHandle(m_hReadVfBufferQueue);
    m_hReadVfBufferQueue = NULL;

    CloseHandle(m_hWriteVfBufferQueue);
    m_hWriteVfBufferQueue = NULL;

    if (m_hDisplay != NULL)
    {
        DeleteDC(m_hDisplay);
        m_hDisplay = NULL;
    }

    if (m_hPrpISRThread)
    {
        // Set PrpIntrThread end
        SetEvent ( m_hExitPrpISRThread ); 
        // Wait for PrpIntrThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hPrpISRThread,INFINITE))
            RETAILMSG(0,(TEXT("Wait for PrpIntrThread end")));

        CloseHandle(m_hPrpISRThread);
        m_hPrpISRThread = NULL;
    }
    CloseHandle(m_hExitPrpISRThread);
    m_hExitPrpISRThread = NULL;

    if (m_hPrpEncBufThread)
    {
        // Set PrpEncBufferWorkerThread end
        SetEvent ( m_hExitPrpEncThread ); 
        // Wait for PrpEncBufferWorkerThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hPrpEncBufThread,INFINITE))
            RETAILMSG(0,(TEXT("Wait for PrpEncBufferWorkerThread end")));

        CloseHandle(m_hPrpEncBufThread);
        m_hPrpEncBufThread = NULL;
    }
    CloseHandle(m_hExitPrpEncThread);
    m_hExitPrpEncThread = NULL;

    if (m_hPrpVfBufThread)
    {
        // Set PrpVfBufferWorkerThread end
        SetEvent ( m_hExitPrpVfThread ); 
        // Wait for PrpVfBufferWorkerThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hPrpVfBufThread,INFINITE))
            RETAILMSG(0,(TEXT("Wait for PrpVfBufferWorkerThread end")));
        
        CloseHandle(m_hPrpVfBufThread);
        m_hPrpVfBufThread = NULL;
    }
    CloseHandle(m_hExitPrpVfThread);
    m_hExitPrpVfThread = NULL;

    if (m_hPrpEncRotBufThread)
    {
        // Set PrpEncRotBufferWorkerThread end
        SetEvent ( m_hExitPrpEncRotThread ); 
        // Wait for PrpEncRotBufferWorkerThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hPrpEncRotBufThread,INFINITE))
            RETAILMSG(0,(TEXT("Wait for PrpEncBufferWorkerThread end")));

        CloseHandle(m_hPrpEncRotBufThread);
        m_hPrpEncRotBufThread = NULL;
    }
    CloseHandle(m_hExitPrpEncRotThread);
    m_hExitPrpEncRotThread = NULL;

    if (m_hPrpVfRotBufThread)
    {
        // Set PrpVfRotBufferWorkerThread end
        SetEvent ( m_hExitPrpVfRotThread ); 
        // Wait for PrpVfRotBufferWorkerThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hPrpVfRotBufThread,INFINITE))
            RETAILMSG(0,(TEXT("Wait for PrpVfBufferWorkerThread end")));
        
        CloseHandle(m_hPrpVfRotBufThread);
        m_hPrpVfRotBufThread = NULL;
    }
    CloseHandle(m_hExitPrpVfRotThread);
    m_hExitPrpVfRotThread = NULL;

#ifdef RINGO_FIX_MEM_ALLOC_ISSUE
    PrpDeleteVfBuffers();

    PrpDeleteEncBuffers();
#endif
    
    PRP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: PrpInitDisplayCharacteristics
//
// This function initializes the display characteristics by
// retrieving them from the display driver.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpInitDisplayCharacteristics(void)
{
    PRP_FUNCTION_ENTRY();

    m_hDisplay = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

    // Get Display Type from Display driver
    if (ExtEscape(m_hDisplay, VF_GET_DISPLAY_INFO, NULL, NULL,
        sizeof(DISPLAY_CHARACTERISTIC), (LPSTR) &m_displayCharacteristics) < 0)
    {
        DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Unable to get display characteristics! \r\n"), __WFUNCTION__));
        return FALSE;
    }

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  PrpAllocateEncBuffers
//
// This function allocates buffers for the encoding channel
// and adds each to the buffer queue.  This function must be called
// in order to allocate physically contiguous buffers for use in
// the IPU's IDMAC.
//
// Parameters:
//      numBuffers
//          [in] Number of buffers to allocate and add
//          to the queues.
//
//      bufSize
//          [in] Size of buffer to create and enqueue.
//
// Returns:
//      Returns virtual pointer to buffer created.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpAllocateEncBuffers(ULONG numBuffers, ULONG bufSize)
{
    PRP_FUNCTION_ENTRY();

    DEBUGMSG(ZONE_ERROR, 
        (TEXT("%s : Buffer size: %x\r\n"), __WFUNCTION__, bufSize));

    pEncBufferManager->PrintBufferInfo();

    m_iEncNumBuffers = numBuffers;
    m_iEncBufSize = bufSize;

    // Initialize MsgQueues when buffers are allocated
    if (!pEncBufferManager->AllocateBuffers(m_iEncNumBuffers, m_iEncBufSize))
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s : Encoding Buffer allocation failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

#ifndef RINGO_FIX_MEM_ALLOC_ISSUE
    if (m_bEncFlipRot)
    {
#endif    
        // Initialize MsgQueues when buffers are allocated
        if (!PrpAllocateEncRotBuffers())
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s : Rotation for encoding Buffer allocation failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }

#ifndef RINGO_FIX_MEM_ALLOC_ISSUE
    }
#endif

    pEncBufferManager->PrintBufferInfo();

    PRP_FUNCTION_EXIT();

    // Return virtual buffer address
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  PrpAllocateEncRotBuffers
//
// This function allocates buffers for the rotation for encoding channel
// and adds each to the buffer queue.  This function must be called
// in order to allocate physically contiguous buffers for use in
// the IPU's IDMAC.
//
// Parameters:
//      numBuffers
//          [in] Number of buffers to allocate and add
//          to the queues.
//
//      bufSize
//          [in] Size of buffer to create and enqueue.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpAllocateEncRotBuffers()
{
    PRP_FUNCTION_ENTRY();

    // Initialize MsgQueues when buffers are allocated
    if (!pEncRotBufferManager->AllocateBuffers(m_iEncNumBuffers, m_iEncBufSize))
    {
        return FALSE;
    }

    m_bEncRotBuffersAllocated = TRUE;

    PRP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  PrpGetEncBufFilled
//
// This function reads the queue of filled encoding channel
// buffers and returns the buffer at the top of the queue.
//
// Parameters:
//      None.
//
// Returns:
//      Pointer to filled buffer, or NULL if failure.
//
//-----------------------------------------------------------------------------
UINT32* PrpClass::PrpGetEncBufFilled()
{
    PRP_FUNCTION_ENTRY();

    if (m_bEncFlipRot)
    {
        PRP_FUNCTION_EXIT();
        // Get filled buffer from the filled buffer queue.
        return pEncRotBufferManager->GetBufferFilled();
    }
    else
    {
        PRP_FUNCTION_EXIT();
        // Get filled buffer from the filled buffer queue.
        return pEncBufferManager->GetBufferFilled();
    }
}


//-----------------------------------------------------------------------------
//
// Function:  PrpDeleteEncBuffers
//
// This function deletes all of the encoding buffers in the Idle 
// and Ready queues.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpDeleteEncBuffers()
{
    PRP_FUNCTION_ENTRY();

    // Delete encoding buffers.
    if (!pEncBufferManager->DeleteBuffers())
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s : Failed to delete buffers!\r\n"), __WFUNCTION__));
        return FALSE;
    }

#ifndef RINGO_FIX_MEM_ALLOC_ISSUE
    // TODO: Make sure that encoding buffers deleted whenever 
    // encoding is turned off
    if (m_bEncFlipRot)
    {
#endif    
        // Delete encoding buffers for rotation.
        if (!pEncRotBufferManager->DeleteBuffers())
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s : Failed to delete buffers!\r\n"), __WFUNCTION__));
            return FALSE;
        }

        m_bEncRotBuffersAllocated = FALSE;

#ifndef RINGO_FIX_MEM_ALLOC_ISSUE        
    }
#endif    

    PRP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  PrpAllocateVfBuffers
//
// This function allocates buffers for the viewfinding channel
// and adds each to the buffer queue.  This function must be called
// in order to allocate physically contiguous buffers for use in
// the IPU's IDMAC.
//
// Parameters:
//      numBuffers
//          [in] Number of buffers to allocate and add
//          to the queues.
//
//      bufSize
//          [in] Size of buffer to create and enqueue.
//
// Returns:
//      Returns virtual pointer to buffer created.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpAllocateVfBuffers(ULONG numBuffers, ULONG bufSize)
{
    PRP_FUNCTION_ENTRY();

    m_iVfNumBuffers = numBuffers;
    m_iVfBufSize = bufSize;

    // Initialize MsgQueues when buffers are allocated
    if (!pVfBufferManager->AllocateBuffers(m_iVfNumBuffers, m_iVfBufSize))
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s : Viewfinding Buffer allocation failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

#ifndef RINGO_FIX_MEM_ALLOC_ISSUE
    if (m_bVfFlipRot)
    {
#endif
        // Initialize MsgQueues when buffers are allocated
        if (!PrpAllocateVfRotBuffers())
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s : Rotation for viewfinding Buffer allocation failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }

#ifndef RINGO_FIX_MEM_ALLOC_ISSUE        
    }
#endif

    PRP_FUNCTION_EXIT();

    // Return virtual buffer address
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  PrpAllocateVfRotBuffers
//
// This function allocates buffers for the rotation for viewfinding channel
// and adds each to the buffer queue.  This function must be called
// in order to allocate physically contiguous buffers for use in
// the IPU's IDMAC.
//
// Parameters:
//      numBuffers
//          [in] Number of buffers to allocate and add
//          to the queues.
//
//      bufSize
//          [in] Size of buffer to create and enqueue.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpAllocateVfRotBuffers()
{
    PRP_FUNCTION_ENTRY();

    // Initialize MsgQueues when buffers are allocated
    if (!pVfRotBufferManager->AllocateBuffers(m_iVfNumBuffers, m_iVfBufSize))
    {
        return FALSE;
    }

    m_bVfRotBuffersAllocated = TRUE;

    PRP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  PrpGetVfBufFilled
//
// This function reads the queue of filled viewfinding channel
// buffers and returns the buffer at the top of the queue.
//
// Parameters:
//      None.
//
// Returns:
//      Pointer to filled buffer, or NULL if failure.
//
//-----------------------------------------------------------------------------
UINT32* PrpClass::PrpGetVfBufFilled()
{
    PRP_FUNCTION_ENTRY();

    if (m_bVfFlipRot)
    {
        PRP_FUNCTION_EXIT();
        // Get filled buffer from the filled buffer queue.
        return pVfRotBufferManager->GetBufferFilled();
    }
    else
    {
        PRP_FUNCTION_EXIT();
        // Get filled buffer from the filled buffer queue.
        return pVfBufferManager->GetBufferFilled();
    }
}

//-----------------------------------------------------------------------------
//
// Function:  PrpDeleteVfBuffers
//
// This function deletes all of the viewfinding buffers in the Idle 
// and Ready queues.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpDeleteVfBuffers()
{
    PRP_FUNCTION_ENTRY();

    // Delete viewfinding buffers.
    if (!pVfBufferManager->DeleteBuffers())
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s : Failed to delete buffers!\r\n"), __WFUNCTION__));
        return FALSE;
    }

#ifndef RINGO_FIX_MEM_ALLOC_ISSUE
    // TODO: Make sure that viewfinding rotation buffers deleted whenever 
    // rotation is turned off
    if (m_bVfFlipRot)
    {
#endif    
        // Delete encoding buffers for rotation.
        if (!pVfRotBufferManager->DeleteBuffers())
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s : Failed to delete buffers!\r\n"), __WFUNCTION__));
            return FALSE;
        }

        m_bVfRotBuffersAllocated = FALSE;

#ifndef RINGO_FIX_MEM_ALLOC_ISSUE        
    }
#endif

    PRP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  PrpGetMaxBuffers
//
// This function returns the maximum number of buffers supported
// by the preprocessor.
// with the IPU hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns the Max buffer number.
//
//-----------------------------------------------------------------------------
UINT32 PrpClass::PrpGetMaxBuffers(void)
{
    return PRP_MAX_NUM_BUFFERS;
}


//-----------------------------------------------------------------------------
//
// Function: PrpConfigureEncoding
//
// This function configures the IC registers and IDMAC
// channels for the preprocessor encoding channel.
//
// Parameters:
//      pPrpEncConfigureData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureEncoding(pPrpEncConfigData configData)
{
    UINT16 iEncOutputWidth = 0, iEncOutputHeight = 0, tempWidth = 0;
    BOOL result = TRUE;
    prpEncOutputFormat  iEncFormat = configData->encFormat;
    prpIDMACChannelParams channelParams;
    prpResizeCoeffs resizeCoeffs;
    prpCSCCoeffs CSCCoeffs;
    UINT32 iYBufLen = 0;
    UINT32 iUBufOffset = 0, iVBufOffset = 0;
    UINT32 strideFactor = 0;
    UINT32 oldVal = 0, newVal = 0, iMask = 0, iBitval = 0;

    PRP_FUNCTION_ENTRY();

    if (iEncFormat != prpEncOutputFormat_Disabled)
    {
        // Set these variables to reduce pointer computations,
        // as these will be referenced several times.
        iEncOutputWidth = configData->encSize.width;
        iEncOutputHeight = configData->encSize.height;

        // Configure rotation BAM parameter
        channelParams.iBAM = (configData->encFlipRot.verticalFlip ? 1 : 0) |
            (configData->encFlipRot.horizontalFlip ? 1 : 0) << 1  |
            (configData->encFlipRot.rotate90 ? 1 : 0) << 2;

        m_bEncFlipRot = (channelParams.iBAM == 0) ? 0 : 1;

        RETAILMSG(0, (TEXT("%s: Encoding BAM:verticalFlip: %x, horizontalFlip: %x, rotate90: %x\r\n"), 
        __WFUNCTION__, configData->encFlipRot.verticalFlip, configData->encFlipRot.horizontalFlip,configData->encFlipRot.rotate90));
        RETAILMSG(0,(TEXT("PrpConfigureEncoding:m_bEncFlipRot = %d"),m_bEncFlipRot));

        //-----------------------------------------------------------------
        // Setup color space conversion
        //-----------------------------------------------------------------

        // Set up CSC for encoding
        if (configData->encCSCEquation != prpCSCNoOp)
        {
            switch (configData->encCSCEquation)
            {
                case prpCSCR2Y_A1:
                case prpCSCR2Y_A0:
                case prpCSCR2Y_B0:
                case prpCSCR2Y_B1:
                    CSCCoeffs.C00 = rgb2yuv_tbl[configData->encCSCEquation][0];
                    CSCCoeffs.C01 = rgb2yuv_tbl[configData->encCSCEquation][1];
                    CSCCoeffs.C02 = rgb2yuv_tbl[configData->encCSCEquation][2];
                    CSCCoeffs.C10 = rgb2yuv_tbl[configData->encCSCEquation][3];
                    CSCCoeffs.C11 = rgb2yuv_tbl[configData->encCSCEquation][4];
                    CSCCoeffs.C12 = rgb2yuv_tbl[configData->encCSCEquation][5];
                    CSCCoeffs.C20 = rgb2yuv_tbl[configData->encCSCEquation][6];
                    CSCCoeffs.C21 = rgb2yuv_tbl[configData->encCSCEquation][7];
                    CSCCoeffs.C22 = rgb2yuv_tbl[configData->encCSCEquation][8];
                    CSCCoeffs.A0 = rgb2yuv_tbl[configData->encCSCEquation][9];
                    CSCCoeffs.A1 = rgb2yuv_tbl[configData->encCSCEquation][10];
                    CSCCoeffs.A2 = rgb2yuv_tbl[configData->encCSCEquation][11];
                    CSCCoeffs.Scale = rgb2yuv_tbl[configData->encCSCEquation][12];
                    break;

                case prpCSCY2R_A1:
                case prpCSCY2R_A0:
                case prpCSCY2R_B0:
                case prpCSCY2R_B1:
                    CSCCoeffs.C00 = yuv2rgb_tbl[configData->encCSCEquation - 4][0];
                    CSCCoeffs.C01 = yuv2rgb_tbl[configData->encCSCEquation - 4][1];
                    CSCCoeffs.C02 = yuv2rgb_tbl[configData->encCSCEquation - 4][2];
                    CSCCoeffs.C10 = yuv2rgb_tbl[configData->encCSCEquation - 4][3];
                    CSCCoeffs.C11 = yuv2rgb_tbl[configData->encCSCEquation - 4][4];
                    CSCCoeffs.C12 = yuv2rgb_tbl[configData->encCSCEquation - 4][5];
                    CSCCoeffs.C20 = yuv2rgb_tbl[configData->encCSCEquation - 4][6];
                    CSCCoeffs.C21 = yuv2rgb_tbl[configData->encCSCEquation - 4][7];
                    CSCCoeffs.C22 = yuv2rgb_tbl[configData->encCSCEquation - 4][8];
                    CSCCoeffs.A0 = yuv2rgb_tbl[configData->encCSCEquation - 4][9];
                    CSCCoeffs.A1 = yuv2rgb_tbl[configData->encCSCEquation - 4][10];
                    CSCCoeffs.A2 = yuv2rgb_tbl[configData->encCSCEquation - 4][11];
                    CSCCoeffs.Scale = yuv2rgb_tbl[configData->encCSCEquation - 4][12];
                    break;

                case prpCSCCustom:
                    CSCCoeffs.C00 = configData->encCSCCoeffs.C00;
                    CSCCoeffs.C01 = configData->encCSCCoeffs.C01;
                    CSCCoeffs.C02 = configData->encCSCCoeffs.C02;
                    CSCCoeffs.C10 = configData->encCSCCoeffs.C10;
                    CSCCoeffs.C11 = configData->encCSCCoeffs.C11;
                    CSCCoeffs.C12 = configData->encCSCCoeffs.C12;
                    CSCCoeffs.C20 = configData->encCSCCoeffs.C20;
                    CSCCoeffs.C21 = configData->encCSCCoeffs.C21;
                    CSCCoeffs.C22 = configData->encCSCCoeffs.C22;
                    CSCCoeffs.A0 = configData->encCSCCoeffs.A0;
                    CSCCoeffs.A1 = configData->encCSCCoeffs.A1;
                    CSCCoeffs.A2 = configData->encCSCCoeffs.A2;
                    CSCCoeffs.Scale = configData->encCSCCoeffs.Scale;
                    break;

                default:
                    DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Invalid encoding CSC equation. \r\n"), __WFUNCTION__));
                    result = FALSE;
                    goto _encDone;
            }

            // Set up the task parameter memory
            PrpTaskParamConfig(prpCSCEncMatrix1, &CSCCoeffs);

            // We will enable bits in IC_CONF when we start channel
            m_bEncCSC = TRUE;
        }
        else
        {
            // We will disable bits in IC_CONF when we start channel
            m_bEncCSC = FALSE;
        }

        //-----------------------------------------------------------------
        // Set up resizing.
        //-----------------------------------------------------------------

        // Encoding path
        if (configData->encFormat != prpEncOutputFormat_Disabled)
        {
            // Vertical resizing
            // Get coefficients and then set registers
            if (!PrpGetResizeCoeffs(configData->inputSize.height,
                configData->encSize.height, &resizeCoeffs))
            {
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Encoding vertical resizing failed. \r\n"), __WFUNCTION__));
                result = FALSE;
                goto _encDone;
            }

            // Set downsizing field
            INSREG32BF(&P_IPU_REGS->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_V,
                resizeCoeffs.downsizeCoeff);

            // Set resizing field
            INSREG32BF(&P_IPU_REGS->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_V,
                resizeCoeffs.resizeCoeff);

            // Horizontal resizing
            // Get coefficients and then set registers
            if (!PrpGetResizeCoeffs(configData->inputSize.width,
                configData->encSize.width, &resizeCoeffs))
            {
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Encoding horizontal resizing failed. \r\n"), __WFUNCTION__));
                result = FALSE;
                goto _encDone;
            }

            // Set downsizing field
            INSREG32BF(&P_IPU_REGS->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_H,
                resizeCoeffs.downsizeCoeff);

            // Set resizing field
            INSREG32BF(&P_IPU_REGS->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_H,
                resizeCoeffs.resizeCoeff);
        }

        //-----------------------------------------------------------------
        // Setup output format
        //-----------------------------------------------------------------

        switch(iEncFormat)
        {
            case prpEncOutputFormat_YUV444:
                channelParams.bInterleaved = FALSE;
                channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV444;
                channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats
                channelParams.iLineStride = iEncOutputWidth;  // Used to compute Y stride
                                          // for YUV non-interleaved
                // TODO:  Make sure this is right: Compute U buffer
                // start address for program channel parameter
                iYBufLen = channelParams.iLineStride * iEncOutputHeight;
                iUBufOffset = iYBufLen;
                iVBufOffset = iYBufLen << 1;

                break;

            case prpEncOutputFormat_YUV422:
                channelParams.bInterleaved = FALSE;
                channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422;
                channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats
                channelParams.iLineStride = iEncOutputWidth;  // Used to compute Y stride
                                          // for YUV non-interleaved
                iYBufLen = channelParams.iLineStride * iEncOutputHeight;
                iUBufOffset = iYBufLen;
                iVBufOffset = iYBufLen + (iYBufLen >> 1);

                break;

            case prpEncOutputFormat_YUV420:
                channelParams.bInterleaved = FALSE;
                channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420;
                channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats
                channelParams.iLineStride = iEncOutputWidth;  // Used to compute Y stride
                                          // for YUV non-interleaved
                iYBufLen = iEncOutputWidth * iEncOutputHeight;
                iUBufOffset = iYBufLen;
                iVBufOffset = iYBufLen + (iYBufLen >> 2);

                break;

            case prpEncOutputFormat_YUV444IL:
                channelParams.pixelFormat.component0_offset = 0;
                channelParams.pixelFormat.component1_offset = 8;
                channelParams.pixelFormat.component2_offset = 16;
                channelParams.pixelFormat.component0_width = 7;
                channelParams.pixelFormat.component1_width = 7;
                channelParams.pixelFormat.component2_width = 7;

                channelParams.iLineStride = iEncOutputWidth * 3;
                channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_24; // Code for 24BPP
                channelParams.bInterleaved = TRUE;
                channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV444;

                break;

            case prpEncOutputFormat_YUYV422:
                channelParams.pixelFormat.component0_offset = 0;
                channelParams.pixelFormat.component1_offset = 8;
                channelParams.pixelFormat.component2_offset = 24;
                channelParams.pixelFormat.component0_width = 7;
                channelParams.pixelFormat.component1_width = 7;
                channelParams.pixelFormat.component2_width = 7;

                channelParams.iLineStride = iEncOutputWidth << 1;
                channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
                channelParams.bInterleaved = TRUE;
                channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;

                break;

            case prpEncOutputFormat_YVYU422:
                channelParams.pixelFormat.component0_offset = 0;
                channelParams.pixelFormat.component1_offset = 24;
                channelParams.pixelFormat.component2_offset = 8;
                channelParams.pixelFormat.component0_width = 7;
                channelParams.pixelFormat.component1_width = 7;
                channelParams.pixelFormat.component2_width = 7;

                channelParams.iLineStride = iEncOutputWidth << 1;
                channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
                channelParams.bInterleaved = TRUE;
                channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;

                break;

            case prpEncOutputFormat_UYVY422:
                channelParams.pixelFormat.component0_offset = 8;
                channelParams.pixelFormat.component1_offset = 0;
                channelParams.pixelFormat.component2_offset = 16;
                channelParams.pixelFormat.component0_width = 7;
                channelParams.pixelFormat.component1_width = 7;
                channelParams.pixelFormat.component2_width = 7;

                channelParams.iLineStride = iEncOutputWidth << 1;
                channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
                channelParams.bInterleaved = TRUE;
                channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;

                break;

            case prpEncOutputFormat_VYUY422:
                channelParams.pixelFormat.component0_offset = 8;
                channelParams.pixelFormat.component1_offset = 16;
                channelParams.pixelFormat.component2_offset = 0;
                channelParams.pixelFormat.component0_width = 7;
                channelParams.pixelFormat.component1_width = 7;
                channelParams.pixelFormat.component2_width = 7;

                channelParams.iLineStride = iEncOutputWidth << 1;
                channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
                channelParams.bInterleaved = TRUE;
                channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;

                break;

            case prpEncOutputFormat_RGB:
                channelParams.pixelFormat.component0_offset = configData->encRGBPixelFormat.component0_offset;
                channelParams.pixelFormat.component1_offset = configData->encRGBPixelFormat.component1_offset;
                channelParams.pixelFormat.component2_offset = configData->encRGBPixelFormat.component2_offset;
                channelParams.pixelFormat.component3_offset = configData->encRGBPixelFormat.component3_offset;
                channelParams.pixelFormat.component0_width = configData->encRGBPixelFormat.component0_width-1;
                channelParams.pixelFormat.component1_width = configData->encRGBPixelFormat.component1_width-1;
                channelParams.pixelFormat.component2_width = configData->encRGBPixelFormat.component2_width-1;
                channelParams.pixelFormat.component3_width = configData->encRGBPixelFormat.component3_width-1;

                switch (configData->encDataWidth)
                {
                    case prpDataWidth_8BPP:
                        channelParams.iLineStride = iEncOutputWidth;
                        channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8;
                        break;

                    case prpDataWidth_16BPP:
                        channelParams.iLineStride = iEncOutputWidth << 1;
                        channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16;
                        break;

                    case prpDataWidth_24BPP:
                        channelParams.iLineStride = iEncOutputWidth * 3;
                        channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_24;
                        break;

                    case prpDataWidth_32BPP:
                        channelParams.iLineStride = iEncOutputWidth << 2;
                        channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_32;
                        break;

                    default:
                        DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Error encoding data width, %d !! \r\n"), __WFUNCTION__, configData->encDataWidth));
                        result =  FALSE;
                        goto _encDone;
                }

                channelParams.bInterleaved = TRUE;
                channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;

                break;

            case prpEncOutputFormat_RGBA:
                // TODO: Add support for RGBA, and find out data path and use cases for RGBA
                channelParams.pixelFormat.component0_offset = 0;
                channelParams.pixelFormat.component1_offset = 8;
                channelParams.pixelFormat.component2_offset = 16;
                channelParams.pixelFormat.component0_width = 7;
                channelParams.pixelFormat.component1_width = 7;
                channelParams.pixelFormat.component2_width = 7;

                // 32 bits per pixel for RGB data
                channelParams.iLineStride = iEncOutputWidth << 2;
                channelParams.bInterleaved = TRUE;
                channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;

                break;

            default:
                DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Invalid encoding output format, %d !! \r\n"), __WFUNCTION__, configData->encFormat));
                result =  FALSE;
                goto _encDone;
        }

        //-----------------------------------------------------------------
        // Image size validity check
        // Setup encoding channel output image size
        //-----------------------------------------------------------------

        if((iEncOutputWidth  < PRP_MIN_OUTPUT_WIDTH) ||
            (iEncOutputHeight < PRP_MIN_OUTPUT_HEIGHT) ||
            (iEncOutputWidth  > PRP_MAX_OUTPUT_WIDTH) ||
            (iEncOutputHeight > PRP_MAX_OUTPUT_HEIGHT) ||
            (configData->inputSize.width > (iEncOutputWidth << 3)) ||
            (configData->inputSize.height > (iEncOutputHeight << 3)))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error encoding channel size: \r\n"), __WFUNCTION__));
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("\t Input Size: width (%d), height (%d)\r\n"),
                    configData->inputSize.width, configData->inputSize.height));
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("\t Encoding Channel Size: width (%d), height (%d)\r\n"),
                    iEncOutputWidth, iEncOutputHeight));
            result = FALSE;
            goto _encDone;
        }

        // Alignment check
        if(iEncFormat != prpEncOutputFormat_RGB)
        {
            if(iEncOutputWidth & 0x01)
            {
                DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Error Channel-1 size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                        iEncOutputWidth, iEncOutputHeight));
                result = FALSE;
                goto _encDone;
            }
        }

        // Set the frame height and width
        channelParams.iHeight = iEncOutputHeight;
        channelParams.iWidth = iEncOutputWidth;

        // Pixel Burst rate always 16 unless we are using a rotation channel
        channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_16;

        channelParams.iBAM = 0;

        // Set encoding channel parameters (IC->Mem for encoding)
        PrpIDMACChannelConfig(0, &channelParams, iUBufOffset, iVBufOffset);

        if (m_bEncFlipRot)
        {
            // Rotation/Flipping datapath

            // First, allocate rotation buffers
            if (!m_bEncRotBuffersAllocated)
            {
                PrpAllocateEncRotBuffers();
            }

            // Burst length is 8 for rotation channels
            channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_8;

            channelParams.iBAM = (configData->encFlipRot.verticalFlip ? 1 : 0) |
                (configData->encFlipRot.horizontalFlip ? 1 : 0) << 1  |
                (configData->encFlipRot.rotate90 ? 1 : 0) << 2;

            // Set encoding channel parameters (Mem->IC for rotation)
            PrpIDMACChannelConfig(10, &channelParams, iUBufOffset, iVBufOffset);

            // If we are rotating, we must Reverse the width, height, and 
            // line stride for the image after rotation is complete.
            if (configData->encFlipRot.rotate90)
            {
                tempWidth = channelParams.iWidth;
                channelParams.iWidth = channelParams.iHeight;
                channelParams.iHeight = tempWidth;
                strideFactor = channelParams.iLineStride / tempWidth;
                channelParams.iLineStride = strideFactor * channelParams.iWidth;
            }
            RETAILMSG(0, (TEXT("%s: Encoding rotate output:Width: %x, Height: %x\r\n"), 
            __WFUNCTION__, channelParams.iWidth, channelParams.iHeight));

            channelParams.iBAM = 0;

            // Set encoding channel parameters (IC->Mem after rotation)
            PrpIDMACChannelConfig(8, &channelParams, iUBufOffset, iVBufOffset);
        }
        else
        {
            // No rotation; data written to memory.

            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW_PRPENC_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW_PRPENC_DEST_SEL, 
                IPU_IPU_FS_PROC_FLOW_PRPENC_DEST_SEL_ARM);

            // Configure frame synchronization
            do
            {
                oldVal = INREG32(&P_IPU_REGS->IPU_FS_PROC_FLOW);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_FS_PROC_FLOW, 
                        oldVal, newVal) != oldVal);
        }

        m_bEncConfigured = TRUE;
    }

_encDone:
    PRP_FUNCTION_EXIT();
    return result;
}

//-----------------------------------------------------------------------------
//
// Function: PrpConfigureViewfinding
//
// This function configures the IC registers and IDMAC
// channels for the preprocessor viewfinding channel.
//
// Parameters:
//      pPrpVfConfigureData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureViewfinding(pPrpVfConfigData configData)
{
    BOOL result = TRUE;
    UINT16 iVfOutputWidth, iVfOutputHeight, tempWidth;
    prpVfOutputFormat iVfFormat = configData->vfFormat;
    prpIDMACChannelParams channelParams;
    prpCSCCoeffs CSCCoeffs;
    prpResizeCoeffs resizeCoeffs;
    DISPLAY_SDC_FG_CONFIG_DATA displayConfig;
    UINT32 strideFactor;
    UINT32 oldVal, newVal, iMask, iBitval;

    PRP_FUNCTION_ENTRY();

    // Now, the viewfinding channel
    if (iVfFormat != prpVfOutputFormat_Disabled)
    {
        // Configure rotation BAM parameter
        channelParams.iBAM = (configData->vfFlipRot.verticalFlip ? 1 : 0) |
            (configData->vfFlipRot.horizontalFlip ? 1 : 0) << 1  |
            (configData->vfFlipRot.rotate90 ? 1 : 0) << 2;

        m_bVfFlipRot = (channelParams.iBAM == 0) ? 0 : 1;

        RETAILMSG(0, (TEXT("%s: Viewfinding BAM:verticalFlip: %x, horizontalFlip: %x, rotate90: %x\r\n"), 
        __WFUNCTION__, configData->vfFlipRot.verticalFlip, configData->vfFlipRot.horizontalFlip,configData->vfFlipRot.rotate90));
        RETAILMSG(0,(TEXT("PrpConfigureViewfinding:m_bVfFlipRot = %d"),m_bVfFlipRot));
        
        // Set these variables to reduce pointer computations,
        // as these will be referenced several times.
        iVfOutputWidth = configData->vfSize.width;
        iVfOutputHeight = configData->vfSize.height;

        // Check that requested width and height do not
        // exceed width and height supported by display
        if (m_displayCharacteristics.width < iVfOutputWidth)
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Requested viewfinding width %x greater than display width %x.  Width will be set to the display width.\r\n"), 
                __WFUNCTION__, iVfOutputWidth, m_displayCharacteristics.width));
        }

        if (m_displayCharacteristics.height < iVfOutputHeight)
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Requested viewfinding height %x greater than display height %x.  Height will be set to the display height. \r\n"), 
                __WFUNCTION__, iVfOutputHeight, m_displayCharacteristics.height));
        }

        //-----------------------------------------------------------------
        // Setup color space conversion
        //-----------------------------------------------------------------

        // Set up CSC for viewfinding
        if (configData->vfCSCEquation != prpCSCNoOp)
        {
            switch (configData->vfCSCEquation)
            {
                case prpCSCR2Y_A1:
                case prpCSCR2Y_A0:
                case prpCSCR2Y_B0:
                case prpCSCR2Y_B1:
                    CSCCoeffs.C00 = rgb2yuv_tbl[configData->vfCSCEquation][0];
                    CSCCoeffs.C01 = rgb2yuv_tbl[configData->vfCSCEquation][1];
                    CSCCoeffs.C02 = rgb2yuv_tbl[configData->vfCSCEquation][2];
                    CSCCoeffs.C10 = rgb2yuv_tbl[configData->vfCSCEquation][3];
                    CSCCoeffs.C11 = rgb2yuv_tbl[configData->vfCSCEquation][4];
                    CSCCoeffs.C12 = rgb2yuv_tbl[configData->vfCSCEquation][5];
                    CSCCoeffs.C20 = rgb2yuv_tbl[configData->vfCSCEquation][6];
                    CSCCoeffs.C21 = rgb2yuv_tbl[configData->vfCSCEquation][7];
                    CSCCoeffs.C22 = rgb2yuv_tbl[configData->vfCSCEquation][8];
                    CSCCoeffs.A0 = rgb2yuv_tbl[configData->vfCSCEquation][9];
                    CSCCoeffs.A1 = rgb2yuv_tbl[configData->vfCSCEquation][10];
                    CSCCoeffs.A2 = rgb2yuv_tbl[configData->vfCSCEquation][11];
                    CSCCoeffs.Scale = rgb2yuv_tbl[configData->vfCSCEquation][12];
                    break;

                case prpCSCY2R_A1:
                case prpCSCY2R_A0:
                case prpCSCY2R_B0:
                case prpCSCY2R_B1:
                    CSCCoeffs.C00 = yuv2rgb_tbl[configData->vfCSCEquation - 4][0];
                    CSCCoeffs.C01 = yuv2rgb_tbl[configData->vfCSCEquation - 4][1];
                    CSCCoeffs.C02 = yuv2rgb_tbl[configData->vfCSCEquation - 4][2];
                    CSCCoeffs.C10 = yuv2rgb_tbl[configData->vfCSCEquation - 4][3];
                    CSCCoeffs.C11 = yuv2rgb_tbl[configData->vfCSCEquation - 4][4];
                    CSCCoeffs.C12 = yuv2rgb_tbl[configData->vfCSCEquation - 4][5];
                    CSCCoeffs.C20 = yuv2rgb_tbl[configData->vfCSCEquation - 4][6];
                    CSCCoeffs.C21 = yuv2rgb_tbl[configData->vfCSCEquation - 4][7];
                    CSCCoeffs.C22 = yuv2rgb_tbl[configData->vfCSCEquation - 4][8];
                    CSCCoeffs.A0 = yuv2rgb_tbl[configData->vfCSCEquation - 4][9];
                    CSCCoeffs.A1 = yuv2rgb_tbl[configData->vfCSCEquation - 4][10];
                    CSCCoeffs.A2 = yuv2rgb_tbl[configData->vfCSCEquation - 4][11];
                    CSCCoeffs.Scale = yuv2rgb_tbl[configData->vfCSCEquation - 4][12];
                    break;

                case prpCSCCustom:
                    CSCCoeffs.C00 = configData->vfCSCCoeffs.C00;
                    CSCCoeffs.C01 = configData->vfCSCCoeffs.C01;
                    CSCCoeffs.C02 = configData->vfCSCCoeffs.C02;
                    CSCCoeffs.C10 = configData->vfCSCCoeffs.C10;
                    CSCCoeffs.C11 = configData->vfCSCCoeffs.C11;
                    CSCCoeffs.C12 = configData->vfCSCCoeffs.C12;
                    CSCCoeffs.C20 = configData->vfCSCCoeffs.C20;
                    CSCCoeffs.C21 = configData->vfCSCCoeffs.C21;
                    CSCCoeffs.C22 = configData->vfCSCCoeffs.C22;
                    CSCCoeffs.A0 = configData->vfCSCCoeffs.A0;
                    CSCCoeffs.A1 = configData->vfCSCCoeffs.A1;
                    CSCCoeffs.A2 = configData->vfCSCCoeffs.A2;
                    CSCCoeffs.Scale = configData->vfCSCCoeffs.Scale;
                    break;

                default:
                    DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Invalid viewfinding CSC equation. \r\n"), __WFUNCTION__));
            }

            // Set up the task parameter memory
            PrpTaskParamConfig(prpCSCVfMatrix1, &CSCCoeffs);

            // We will enable bits in IC_CONF when we start channel
            m_bVfCSC = TRUE;
        }
        else
        {
            // We will disable bits in IC_CONF when we start channel
            m_bVfCSC = FALSE;
        }

        //-----------------------------------------------------------------
        // Set up resizing.
        //-----------------------------------------------------------------

        // Viewfinding path
        if (configData->vfFormat != prpVfOutputFormat_Disabled)
        {
            // Vertical resizing
            // Get coefficients and then set registers
            if (!PrpGetResizeCoeffs(configData->inputSize.height,
                configData->vfSize.height, &resizeCoeffs))
            {
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Viewfinding vertical resizing failed. \r\n"), __WFUNCTION__));
                result = FALSE;
                goto _vfDone;
            }

            // Set downsizing field
            INSREG32BF(&P_IPU_REGS->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_DS_R_V,
                resizeCoeffs.downsizeCoeff);

            // Set resizing field
            INSREG32BF(&P_IPU_REGS->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_RS_R_V,
                resizeCoeffs.resizeCoeff);

            // Horizontal resizing
            // Get coefficients and then set registers
            if (!PrpGetResizeCoeffs(configData->inputSize.width,
                configData->vfSize.width, &resizeCoeffs))
            {
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Viewfinding horizontal resizing failed. \r\n"), __WFUNCTION__));
                result = FALSE;
                goto _vfDone;
            }

            // Set downsizing field
            INSREG32BF(&P_IPU_REGS->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_DS_R_H,
                resizeCoeffs.downsizeCoeff);

            // Set resizing field
            INSREG32BF(&P_IPU_REGS->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_RS_R_H,
                resizeCoeffs.resizeCoeff);
        }

        //-----------------------------------------------------------------
        // Setup output format
        //-----------------------------------------------------------------

        switch(iVfFormat)
        {
            case prpVfOutputFormat_RGB:
                if (m_displayCharacteristics.bpp == 24)
                {
                    channelParams.pixelFormat.component0_offset = 0;
                    channelParams.pixelFormat.component1_offset = 8;
                    channelParams.pixelFormat.component2_offset = 16;
                    channelParams.pixelFormat.component0_width = 7;
                    channelParams.pixelFormat.component1_width = 7;
                    channelParams.pixelFormat.component2_width = 7;

                    // 24 bits per pixel
                    channelParams.iLineStride = iVfOutputWidth * 3;
                    channelParams.bInterleaved = TRUE;
                    channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_24;
                    channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_16;
                }
                else if (m_displayCharacteristics.bpp == 16)
                {
                    channelParams.pixelFormat.component0_offset = 0;
                    channelParams.pixelFormat.component1_offset = 5;
                    channelParams.pixelFormat.component2_offset = 11;
                    channelParams.pixelFormat.component0_width = 4;
                    channelParams.pixelFormat.component1_width = 5;
                    channelParams.pixelFormat.component2_width = 4;

                    // 16 bits per pixel
                    channelParams.iLineStride = iVfOutputWidth * 2;
                    channelParams.bInterleaved = TRUE;
                    channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16;
                    channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_16;
                }

                break;

            case prpVfOutputFormat_RGBA:
                // TODO: Add support for RGBA, and find out data path and use cases for RGBA
                channelParams.pixelFormat.component0_offset = 0;
                channelParams.pixelFormat.component1_offset = 8;
                channelParams.pixelFormat.component2_offset = 16;
                channelParams.pixelFormat.component3_offset = 24;
                channelParams.pixelFormat.component0_width = 7;
                channelParams.pixelFormat.component1_width = 7;
                channelParams.pixelFormat.component2_width = 7;
                channelParams.pixelFormat.component3_width = 7;

                // 32 bits per pixel for RGB data
                channelParams.iLineStride = iVfOutputWidth << 2;
                channelParams.bInterleaved = TRUE;
                channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
                channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_32;
                channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_16;

                break;

            default:
                DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Invalid viewfinding output format, %d !! \r\n"), __WFUNCTION__, configData->vfFormat));
                result =  FALSE;
                goto _vfDone;
        }

        //-----------------------------------------------------------------
        // Image size validity check
        // Setup viewfinding channel output image size
        //-----------------------------------------------------------------

        if((iVfOutputWidth  < PRP_MIN_OUTPUT_WIDTH) ||
            (iVfOutputHeight < PRP_MIN_OUTPUT_HEIGHT) ||
            (iVfOutputWidth  > PRP_MAX_OUTPUT_WIDTH) ||
            (iVfOutputHeight > PRP_MAX_OUTPUT_HEIGHT) ||
            (configData->inputSize.width  > (iVfOutputWidth << 3)) ||
            (configData->inputSize.height > (iVfOutputHeight << 3)))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error viewfinding channel size: \r\n"), __WFUNCTION__));
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("\t Input Size: width (%d), height (%d)\r\n"),
                    configData->inputSize.width, configData->inputSize.height));
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("\t Viewfinding Channel Size: width (%d), height (%d)\r\n"),
                    iVfOutputWidth, iVfOutputHeight));
            result = FALSE;
            goto _vfDone;
        }

        channelParams.iHeight = iVfOutputHeight;
        channelParams.iWidth = iVfOutputWidth;

        // Set member variable for whether we will be 
        // displaying viewfinder data directly to display or not.
        m_bVfDirectDisplay = configData->directDisplay;

        // The following are our scenarios for using
        // the viewfinding channel:
        //      1) Direct Display + SDC: 
        //          - Set up frame sync for task chaining to SDC BG
        //          - Configure display for viewfinding
        //      2) Direct Display + ADC:
        //          - Set up frame sync for task chaining to ADC direct
        //          - Configure display for viewfinding
        //      3) No direct display:
        //          - Set up frame sync to write to memory
        //          - No configuration of display for viewfinding,
        //            as we will not be displaying viewfinding image

        dumpIpuRegisters(P_IPU_REGS);

        if (m_bVfDirectDisplay)
        {
            // Direct display mode enabled

            // Set up display structure to configure
            // display for viewfinding mode.
            displayConfig.verticalFlip = FALSE;
            displayConfig.alpha = 0xFF;
            displayConfig.colorKey = 0;
            displayConfig.plane = IPU_SDC_COM_CONF_GWSEL_FG;
            displayConfig.bpp = m_displayCharacteristics.bpp;
            if (configData->vfFlipRot.rotate90)
            {
                displayConfig.width = iVfOutputHeight;
                displayConfig.height = iVfOutputWidth;
            }
            else
            {
                displayConfig.width = iVfOutputWidth;
                displayConfig.height = iVfOutputHeight;
            }

            displayConfig.stride = displayConfig.width * displayConfig.bpp / 8;

            // Configure display for viewfinding mode
            if (ExtEscape(m_hDisplay, VF_SET_OFFSET, sizeof(POINT),
                    (LPCSTR) &configData->vfOffset, 0, NULL) <= 0)
            {
                DEBUGMSG(ZONE_ERROR,
                      (TEXT("%s: Error initialize VF display mode. \r\n"), __WFUNCTION__));
                return FALSE;
            }

            // Configure display for viewfinding mode
            if (ExtEscape(m_hDisplay, VF_CONFIG, sizeof(DISPLAY_SDC_FG_CONFIG_DATA),
                    (LPCSTR) &displayConfig, 0, NULL) <= 0)
            {
                DEBUGMSG(ZONE_ERROR,
                      (TEXT("%s: Error initialize VF display mode. \r\n"), __WFUNCTION__));
                return FALSE;
            }

            switch (m_displayCharacteristics.eType)
            {
                case eIPU_SDC:
                    m_bADCDirect = FALSE;

                    // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW register
                    iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL);
                    iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL, 
                        IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_ARM);

                    // If display type is a dumb display, we configure
                    // frame synchronization to write data to SDC
                    // Foreground/Background plane.
                    do
                    {
                        oldVal = INREG32(&P_IPU_REGS->IPU_FS_PROC_FLOW);
                        newVal = (oldVal & (~iMask)) | iBitval;
                    } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_FS_PROC_FLOW, 
                                oldVal, newVal) != oldVal);

                    break;
                case eIPU_ADC:

                    m_bADCDirect = TRUE;

                    // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW register
                    iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL);
                    iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL, 
                        IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_ADC_DIRECT);

                    // If display type is a smart display, we configure
                    // frame synchronization to write data to ADC
                    // Direct Viewfinding Channel
                    do
                    {
                        oldVal = INREG32(&P_IPU_REGS->IPU_FS_PROC_FLOW);
                        newVal = (oldVal & (~iMask)) | iBitval;
                    } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_FS_PROC_FLOW, 
                                oldVal, newVal) != oldVal);

                    break;
                default:
                    DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Error.  Display type is neither SDC or ADC!\r\n"),
                        __WFUNCTION__));
                    return FALSE;
            }
        }
        else
        {
            m_bADCDirect = FALSE;

            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL, 
                IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_ARM);

            // Direct display mode is disabled, so the destination
            // should be system memory.
            do
            {
                oldVal = INREG32(&P_IPU_REGS->IPU_FS_PROC_FLOW);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_FS_PROC_FLOW, 
                        oldVal, newVal) != oldVal);
        }

        channelParams.iBAM = 0;

        // Set viewfinding channel parameters
        PrpIDMACChannelConfig(1, &channelParams, 0, 0);

        if (m_bVfFlipRot)
        {
            // Rotation/Flipping datapath

            // First, allocate rotation buffers
            if (!m_bVfRotBuffersAllocated)
            {
                PrpAllocateVfRotBuffers();
            }

            // Burst length is 8 for rotation channels
            channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_8;

            channelParams.iBAM = (configData->vfFlipRot.verticalFlip ? 1 : 0) |
                (configData->vfFlipRot.horizontalFlip ? 1 : 0) << 1  |
                (configData->vfFlipRot.rotate90 ? 1 : 0) << 2;

            // Set viewfinding channel parameters (Mem->IC for rotation)
            PrpIDMACChannelConfig(11, &channelParams, 0, 0);

            // If we are rotating, we must Reverse the width, height, and 
            // line stride for the image after rotation is complete.
            if (configData->vfFlipRot.rotate90)
            {
                tempWidth = channelParams.iWidth;
                channelParams.iWidth = channelParams.iHeight;
                channelParams.iHeight = tempWidth;
                strideFactor = channelParams.iLineStride / tempWidth;
                channelParams.iLineStride = strideFactor * channelParams.iWidth;
            }
            RETAILMSG(0, (TEXT("%s: Viewfinding rotate output:Width: %x, Height: %x\r\n"), 
            __WFUNCTION__, channelParams.iWidth, channelParams.iHeight));

            channelParams.iBAM = 0;

            // Set viewfinding channel parameters (IC->Mem after rotation)
            PrpIDMACChannelConfig(9, &channelParams, 0, 0);
        }

//        dumpIpuRegisters(P_IPU_REGS);

        m_bVfConfigured = TRUE;
    }

_vfDone:
    PRP_FUNCTION_EXIT();
    return result;
}


//-----------------------------------------------------------------------------
//
// Function: PrpStartEncChannel
//
// This function starts the encoding channel preprocessing.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpStartEncChannel(void)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    PRP_FUNCTION_ENTRY();

    pEncBufferManager->PrintBufferInfo();

    // Encoding must have been configured at least once 
    // in order to start the channel
    if (!m_bEncConfigured)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error.  Cannot start encoding channel without first configuring\r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    if (!m_bPrpEnabled)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error.  Cannot start encoding channel without first enabling Prp. \r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    if (m_bEncRunning)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Encoding channel already running.\r\n"), __WFUNCTION__));
        return TRUE;
    }

    m_bEncRestartBufferLoop = TRUE;
    m_bEncRestartISRLoop = TRUE;
    m_bEncRunning = TRUE;
    m_iCurrentEncBuf = 0;
    m_iCurrentEncRotBuf = 0;

    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause Pin to believe a frame was ready.
    ResetEvent(m_hEncEOFEvent);

    if (m_bEncFlipRot)
    {
        m_bEncRotRestartBufferLoop = TRUE;

        // Protect access to IPU_INT_CTRL_1, IPU_CHA_DB_MODE_SEL,
        // IDMAC_CHA_EN, and IC_CONF registers.

        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_8);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_ENABLE);

        // Enable IPU interrupts for channel 8 (IC->Mem after rotation).
        // This is the final channel in the chain, so we want this channel
        // to trigger an interrupt that the camera IST will handle.
        // Also enable IPU interrupts for channel 0 (IC->Mem after pre-proc).
        // This will allow us to inform the waiting thread that
        // we need a new buffer.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_8)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_10);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DUB_BUF)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_DUB_BUF)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_10, IPU_DUB_BUF);

        // Set double-buffering for all channels used
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        // Set current buffer bits, so that we will start on buffer 0.
        SETREG32(&P_IPU_REGS->IPU_CHA_CUR_BUF, 
            CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_IPU_CHA_CUR_BUF_CLEAR)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_IPU_CHA_CUR_BUF_CLEAR)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_10, IPU_IPU_CHA_CUR_BUF_CLEAR));

        // Compute bitmask and shifted bit value for IDMAC enable register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_8)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_10);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_10, IPU_ENABLE);

        // Set the bits to enable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPENC_EN)
            | CSP_BITFMASK(IPU_IC_CONF_PRPENC_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPENC_EN, IPU_IC_CONF_PRPENC_EN_ENABLE)
            | CSP_BITFVAL(IPU_IC_CONF_PRPENC_ROT_EN, IPU_IC_CONF_PRPENC_ROT_EN_ENABLE);

        if (m_bEncCSC)
        {
            iMask |= CSP_BITFMASK(IPU_IC_CONF_PRPENC_CSC1);
            iBitval |= CSP_BITFVAL(IPU_IC_CONF_PRPENC_CSC1, IPU_IC_CONF_PRPENC_CSC1_ENABLE);
        }

        // Enable preprocessing for encoding path in IC and 
        // enable preprocessing for rotation for encoding path in IC.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IC_CONF, 
                    oldVal, newVal) != oldVal);

        // Request buffer for IDMAC Channel 0 (IC->Mem for encoding).
        // This will attempt to set up buffer 0.
        SetEvent(m_hEncBufWaitList[prpBuf0RequestEvent]);

        // Request buffer for IDMAC Channel 0 (IC->Mem for encoding).
        // This will attempt to set up buffer 1.
        SetEvent(m_hEncBufWaitList[prpBuf1RequestEvent]);

        // Request buffer for IDMAC Channel 8 (IC->Mem after 
        // rotation for encoding).
        // This will attempt to set up buffer 0.
        SetEvent(m_hEncRotBufWaitList[prpBuf0RequestEvent]);

        // Request buffer for IDMAC Channel 8 (IC->Mem after 
        // rotation for encoding).
        // This will attempt to set up buffer 1.
        SetEvent(m_hEncRotBufWaitList[prpBuf1RequestEvent]);
    }
    else
    {
        // Protect access to IPU_INT_CTRL_1, IPU_CHA_DB_MODE_SEL,
        // IDMAC_CHA_EN, and IC_CONF registers.

        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_ENABLE);

        // enable IPU interrupts for channel 1 (IC->Mem)
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DUB_BUF);

        // Set double-buffering for all channels used
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        // Set current buffer bits, so that we will start on buffer 0.
        SETREG32(&P_IPU_REGS->IPU_CHA_CUR_BUF, 
            CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_IPU_CHA_CUR_BUF_CLEAR));

        // Compute bitmask and shifted bit value for IDMAC enable register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_ENABLE);

        // Set the bits to enable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPENC_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPENC_EN, IPU_IC_CONF_PRPENC_EN_ENABLE);

        if (m_bEncCSC)
        {
            iMask |= CSP_BITFMASK(IPU_IC_CONF_PRPENC_CSC1);
            iBitval |= CSP_BITFVAL(IPU_IC_CONF_PRPENC_CSC1, IPU_IC_CONF_PRPENC_CSC1_ENABLE);
        }

        // enable preprocessing for viewfinding path in IC
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IC_CONF, 
                    oldVal, newVal) != oldVal);

        // Request buffer for IDMAC Channel 0 (IC->Mem for encoding).
        // This will attempt to set up buffer 0.
        SetEvent(m_hEncBufWaitList[prpBuf0RequestEvent]);

        // Request buffer for IDMAC Channel 1 (IC->Mem for encoding).
        // This will attempt to set up buffer 1.
        SetEvent(m_hEncBufWaitList[prpBuf1RequestEvent]);
    }

    dumpIpuRegisters(P_IPU_REGS);

    PRP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PrpStopEncChannel
//
// This function halts the encoding channel preprocessing.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpStopEncChannel(void)
{
    BOOL bWaitToStop = FALSE;
    UINT32 uTempReg, uTempReg1, uTempReg2, uCount = 0;
    UINT32 oldVal, newVal, iMask, iBitval;
    
    PRP_FUNCTION_ENTRY();

    EnterCriticalSection(&m_csEncStopping);

    // initalize ... for the first time through
    uTempReg = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);
    uTempReg1 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_ENC_TSTAT);
    uTempReg2 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_ENC_ROT_TSTAT);

    // We can't disable tasks until the active channels
    // have completed their current frames.  Make sure
    // that buffers aren't set as ready (indicating that
    // they are yet to start) and that channels are
    // not busy (indicating that channels are still running).
    while ((uTempReg1 == IPU_IPU_TASKS_STAT_ACTIVE) || (uTempReg2 == IPU_IPU_TASKS_STAT_ACTIVE) || (uTempReg & 0x501))
    {
        if (uCount <= 1000) // 100 ms, for a worst-case 10FPS rate
        {
            //..if busy then give up time slice
            Sleep(1);
            uCount++;

            //.. check again after the sleep delay
            uTempReg = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);
            uTempReg1 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_ENC_TSTAT);
            uTempReg2 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_ENC_ROT_TSTAT);
        }
        else
        {
            //.. there is something wrong ....break out
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s(): Error in stopping encoding channel!\r\n"), __WFUNCTION__));
            break;
        }
    }
    m_bEncRunning = FALSE;

    // Protect access to IC_CONF register.
    // Disable IC tasks.  Tasks will stop on completion
    // of the current frame.

    // Compute bitmask and shifted bit value for DB mode sel register
    if (m_bEncFlipRot)
    {
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPENC_EN)
            | CSP_BITFMASK(IPU_IC_CONF_PRPENC_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPENC_EN, IPU_IC_CONF_PRPENC_EN_DISABLE)
            | CSP_BITFVAL(IPU_IC_CONF_PRPENC_ROT_EN, IPU_IC_CONF_PRPENC_ROT_EN_DISABLE);
    }
    else
    {
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPENC_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPENC_EN, IPU_IC_CONF_PRPENC_EN_DISABLE);        
    }

    if (m_bEncCSC)
    {
        iMask |= CSP_BITFMASK(IPU_IC_CONF_PRPENC_CSC1);
        iBitval |= CSP_BITFVAL(IPU_IC_CONF_PRPENC_CSC1, IPU_IC_CONF_PRPENC_CSC1_DISABLE);
    }

    // Use interlocked function to Disable IC tasks.
    do
    {
        oldVal = INREG32(&P_IPU_REGS->IC_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IC_CONF, 
                oldVal, newVal) != oldVal);

 
    LeaveCriticalSection(&m_csEncStopping);

    ResetEvent(m_hEncBufWaitList[prpBuf0RequestEvent]);
    ResetEvent(m_hEncBufWaitList[prpBuf1RequestEvent]);
    ResetEvent(m_hEncRotBufWaitList[prpBuf0RequestEvent]);
    ResetEvent(m_hEncRotBufWaitList[prpBuf1RequestEvent]);
    ResetEvent(m_hEncEOFEvent);

    if (m_bEncFlipRot)
    {
        // Protect access to IDMAC_CHA_EN, IPU_CHA_DB_MODE_SEL
        // and IPU_INT_CTRL_1 registers.

        // Compute bitmask and shifted bit value for DB mode sel register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_8)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_10);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_10, IPU_DISABLE);

        // Use interlocked function to 
        // set the bits to disable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_8)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_10);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_10, IPU_DISABLE);

        // Use interlocked function to turn off double buffering.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for INT_CTRL register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_8);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_DISABLE);

        // Use interlocked function to disable IPU 
        // interrupts for viewfinding channel.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        // Reset the state of the encoding buffers, so that
        // everything will be in place if the channel is restarted.
        pEncBufferManager->ResetBuffers();
        pEncRotBufferManager->ResetBuffers();
    }
    else
    {

        // Protect access to IDMAC_CHA_EN, IPU_CHA_DB_MODE_SEL,
        // and IPU_INT_CTRL_1 registers.

        // Compute bitmask and shifted bit value for DMAIC enable register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DISABLE);

        // Use interlocked function to disable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DISABLE);

        // Use interlocked function to turn off double buffering.
        // This assures that we return to buffer 0 when we next 
        // start up this channel.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for INT CTRL register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DISABLE);

        // Use interlocked function to disable IPU interrupts 
        // for viewfinding channel.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        // Reset the state of the encoding buffers, so that
        // everything will be in place if the channel is restarted.
        pEncBufferManager->ResetBuffers();
    }

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpStartVfChannel
//
// This function starts the viewfinding channel preprocessing.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpStartVfChannel(void)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    PRP_FUNCTION_ENTRY();

    // Viewfinding must have been configured at least once 
    // in order to start the channel
    if (!m_bVfConfigured)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error.  Cannot start viewfinding channel without first configuring\r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    if (!m_bPrpEnabled)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error.  Cannot start encoding channel without first enabling Prp. \r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    if (m_bVfRunning)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Viewfinding channel already running.\r\n"), __WFUNCTION__));
        return TRUE;
    }

    m_bVfRestartBufferLoop = TRUE;
    m_bVfRestartISRLoop = TRUE;
    m_bVfRunning = TRUE;
    m_iCurrentVfBuf = 0;
    m_iCurrentVfRotBuf = 0;

    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause Pin to believe a frame was ready.
    ResetEvent(m_hVfEOFEvent);

    if (m_bVfFlipRot)
    {
        m_bVfRotRestartBufferLoop = TRUE;

        // Protect access to IPU_INT_CTRL_1, IPU_CHA_DB_MODE_SEL,
        // IDMAC_CHA_EN, and IC_CONF registers.

        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_9);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_ENABLE);

        // Enable IPU interrupts for channel 9 (IC->Mem after rotation).
        // This is the final channel in the chain, so we want this channel
        // to trigger an interrupt that the camera IST will handle.
        // Also enable IPU interrupts for channel 1 (IC->Mem).
        // This will allow us to inform the waiting thread that
        // we need a new buffer.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_11)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_9);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DUB_BUF)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_11, IPU_DUB_BUF)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_DUB_BUF);

        // Set double-buffering for all channels used
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        // Set current buffer bits, so that we will start on buffer 0.
        SETREG32(&P_IPU_REGS->IPU_CHA_CUR_BUF, 
            CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_IPU_CHA_CUR_BUF_CLEAR)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_11, IPU_IPU_CHA_CUR_BUF_CLEAR)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_IPU_CHA_CUR_BUF_CLEAR));

        // Compute bitmask and shifted bit value for IDMAC enable register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_11)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_9);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_11, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_ENABLE);

        // Set the bits to enable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPVF_EN)
            | CSP_BITFMASK(IPU_IC_CONF_PRPVF_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPVF_EN, IPU_IC_CONF_PRPVF_EN_ENABLE)
            | CSP_BITFVAL(IPU_IC_CONF_PRPVF_ROT_EN, IPU_IC_CONF_PRPVF_ROT_EN_ENABLE);

        if (m_bVfCSC)
        {
            iMask |= CSP_BITFMASK(IPU_IC_CONF_PRPVF_CSC1);
            iBitval |= CSP_BITFVAL(IPU_IC_CONF_PRPVF_CSC1, IPU_IC_CONF_PRPVF_CSC1_ENABLE);
        }

        // Enable preprocessing for viewfinding path in IC and 
        // enable preprocessing for rotation for viewfinding path in IC.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IC_CONF, 
                    oldVal, newVal) != oldVal);

        Sleep(100);
        
        // If direct display mode is enabled, make display active.
        if (m_bVfDirectDisplay)
        {
            m_bVfDisplayActive = TRUE;
        }

        // Request buffer for IDMAC Channel 1 (IC->Mem for viewfinding)
        // and for Channel 11 (Mem-IC for rotation for viewfinding).
        // This will attempt to set up buffer 0.
        SetEvent(m_hVfBufWaitList[prpBuf0RequestEvent]);

        // Request buffer for IDMAC Channel 1 (IC->Mem for viewfinding)
        // and for Channel 11 (Mem-IC for rotation for viewfinding).
        // This will attempt to set up buffer 1.
        SetEvent(m_hVfBufWaitList[prpBuf1RequestEvent]);

        // Request buffer for IDMAC Channel 9 (IC->Mem after rotation
        // for viewfinding).  This will attempt to set up buffer 0.
        SetEvent(m_hVfRotBufWaitList[prpBuf0RequestEvent]);

        // Request buffer for IDMAC Channel 9 (IC->Mem after rotation
        // for viewfinding).  This will attempt to set up buffer 1.
        SetEvent(m_hVfRotBufWaitList[prpBuf1RequestEvent]);
    }
    else
    {
        // Protect access to IPU_INT_CTRL_1, IPU_CHA_DB_MODE_SEL,
        // IDMAC_CHA_EN, and IC_CONF registers.

        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_ENABLE);

        // enable IPU interrupts for channel 1 (IC->Mem)
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DUB_BUF);

        // Set double-buffering for all channels used
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        // Set current buffer bits, so that we will start on buffer 0.
        SETREG32(&P_IPU_REGS->IPU_CHA_CUR_BUF, 
            CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_IPU_CHA_CUR_BUF_CLEAR));

        // Compute bitmask and shifted bit value for IDMAC enable register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_ENABLE);

        // Set the bits to enable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPVF_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPVF_EN, IPU_IC_CONF_PRPVF_EN_ENABLE);

        if (m_bVfCSC)
        {
            iMask |= CSP_BITFMASK(IPU_IC_CONF_PRPVF_CSC1);
            iBitval |= CSP_BITFVAL(IPU_IC_CONF_PRPVF_CSC1, IPU_IC_CONF_PRPVF_CSC1_ENABLE);
        }

        // enable preprocessing for viewfinding path in IC
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IC_CONF, 
                    oldVal, newVal) != oldVal);

        Sleep(100);
        
        // If direct display mode is enabled, make display active.
        if (m_bVfDirectDisplay)
        {
            m_bVfDisplayActive = TRUE;
        }

        if (!m_bADCDirect)
        {
            // Request buffer for IDMAC Channel 1 (IC->Mem for viewfinding).
            // This will attempt to set up buffer 0.
            SetEvent(m_hVfBufWaitList[prpBuf0RequestEvent]);

            // Request buffer for IDMAC Channel 1 (IC->Mem for viewfinding).
            // This will attempt to set up buffer 1.
            SetEvent(m_hVfBufWaitList[prpBuf1RequestEvent]);
        }
    }

    dumpIpuRegisters(P_IPU_REGS);

    PRP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PrpStopVfChannel
//
// This function halts the viewfinding channel preprocessing.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpStopVfChannel(void)
{
    BOOL bWaitToStop = FALSE;
    DISPLAY_BUFFER dispBufferData;
    DWORD dwFlags, bytesRead;
    UINT32 uTempReg, uTempReg1, uTempReg2, uCount = 0;
    UINT32 oldVal, newVal, iMask, iBitval;

    PRP_FUNCTION_ENTRY();

    EnterCriticalSection(&m_csVfStopping);

    m_bVfRunning = FALSE;

    // initalize ... for the first time through
    uTempReg = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);
    uTempReg1 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_VF_TSTAT);
    uTempReg2 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_VF_ROT_TSTAT);
    // We can't disable tasks until the active channels
    // have completed their current frames.  Make sure
    // that buffers aren't set as ready (indicating that
    // they are yet to start) and that channels are
    // not busy (indicating that channels are still running).
    while ((uTempReg1 == IPU_IPU_TASKS_STAT_ACTIVE) || (uTempReg2 == IPU_IPU_TASKS_STAT_ACTIVE) || (uTempReg & 0xA02))
    {
        if (uCount <= 1000)
        {
            //..give up the remainder of time slice
            Sleep(1);
            uCount++;

            //.. need to check after the sleep delay
            uTempReg = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);
            uTempReg1 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_VF_TSTAT);
            uTempReg2 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_VF_ROT_TSTAT);
        }
        else
        {
            //.. there is something wrong ....break out
            RETAILMSG(ZONE_ERROR,
                (TEXT("%s(): Error in stopping viewfinding channel!\r\n"), __WFUNCTION__));
            //4000,1,0    
            RETAILMSG(0,
                (TEXT("%s():uCount:%x, uTempReg: %x, uTempReg1: %x, uTempReg2: %x\r\n"),__WFUNCTION__,uTempReg,uTempReg1,uTempReg2,uCount));
            
            break;
        }
    }

    // Compute bitmask and shifted bit value for IC CONF register
    if (m_bVfFlipRot)
    {
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPVF_EN)
            | CSP_BITFMASK(IPU_IC_CONF_PRPVF_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPVF_EN, IPU_IC_CONF_PRPVF_EN_DISABLE)
            | CSP_BITFVAL(IPU_IC_CONF_PRPVF_ROT_EN, IPU_IC_CONF_PRPVF_ROT_EN_DISABLE);
    }
    else
    {
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPVF_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPVF_EN, IPU_IC_CONF_PRPVF_EN_DISABLE);        
    }

    if (m_bVfCSC)
    {
        iMask |= CSP_BITFMASK(IPU_IC_CONF_PRPVF_CSC1);
        iBitval |= CSP_BITFVAL(IPU_IC_CONF_PRPVF_CSC1, IPU_IC_CONF_PRPVF_CSC1_DISABLE);
    }

    // Use interlocked function to Disable IC tasks.
    do
    {
        oldVal = INREG32(&P_IPU_REGS->IC_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IC_CONF, 
                oldVal, newVal) != oldVal);


    LeaveCriticalSection(&m_csVfStopping);

    ResetEvent(m_hVfBufWaitList[prpBuf0RequestEvent]);
    ResetEvent(m_hVfBufWaitList[prpBuf1RequestEvent]);
    ResetEvent(m_hVfRotBufWaitList[prpBuf0RequestEvent]);
    ResetEvent(m_hVfRotBufWaitList[prpBuf1RequestEvent]);
    ResetEvent(m_hVfEOFEvent);

    // If direct display mode was enabled, empty VF buffers
    // from queue and disable display.
    if (m_bVfDirectDisplay)
    {
        // Clear the VF Buffer queue by reading out the contents
        while (ReadMsgQueue(m_hReadVfBufferQueue, &dispBufferData, 
            sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
        {
        }

        // Disable Viewfinding in the display driver
        if (ExtEscape(m_hDisplay, VF_DISABLE, 0, NULL, 0, NULL) <= 0)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed disabling display for viewfinding.\r\n"), __WFUNCTION__));
            return FALSE;
        }

        m_bVfDisplayActive = FALSE;
    }

    if (m_bVfFlipRot)
    {
        // Protect access to IDMAC_CHA_EN, IPU_CHA_DB_MODE_SEL
        // and IPU_INT_CTRL_1 registers.

        // Compute bitmask and shifted bit value for DB mode sel register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_11)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_9);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_11, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_DISABLE);

        // Use interlocked function to 
        // set the bits to disable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_11)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_9);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_11, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_DISABLE);

        // Use interlocked function to turn off double buffering.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for INT_CTRL register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_9);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_DISABLE);

        // Use interlocked function to disable IPU 
        // interrupts for viewfinding channel.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        // Reset the state of the viewfinding buffers, so that
        // everything will be in place if the channel is restarted.
        pVfBufferManager->ResetBuffers();
        pVfRotBufferManager->ResetBuffers();
    }
    else
    {
        // Protect access to IDMAC_CHA_EN, IPU_CHA_DB_MODE_SEL,
        // and IPU_INT_CTRL_1 registers.

        // Compute bitmask and shifted bit value for DMAIC enable register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DISABLE);

        // Use interlocked function to disable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DISABLE);

        // Use interlocked function to turn off double buffering.
        // This assures that we return to buffer 0 when we next 
        // start up this channel.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for INT CTRL register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DISABLE);

        // Use interlocked function to disable IPU interrupts 
        // for viewfinding channel.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        // Reset the state of the viewfinding buffers, so that
        // everything will be in place if the channel is restarted.
        pVfBufferManager->ResetBuffers();
    }

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpPauseViewfinding
//
// This function pauses the viewfinding output display, while leaving
// the viewfinding channel running.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpPauseViewfinding(void)
{
    // If direct display mode was enabled, disable display.
    if (m_bVfDirectDisplay && m_bVfDisplayActive)
    {
        // Disable Viewfinding in the display driver
        if (ExtEscape(m_hDisplay, VF_DISABLE, 0, NULL, 0, NULL) <= 0)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed disabling display for viewfinding.\r\n"), __WFUNCTION__));
            return FALSE;
        }

        m_bVfDisplayActive = FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpGetEncFrameCount
//
// This function returns the current encoding channel frame count.
//
// Parameters:
//      None.
//
// Returns:
//      The number of viewfinding frames processed since the
//      last configuration request.
//
//-----------------------------------------------------------------------------
UINT32 PrpClass::PrpGetEncFrameCount()
{
    return m_iEncFrameCount;
}


//-----------------------------------------------------------------------------
//
// Function: PrpGetVfFrameCount
//
// This function returns the current viewfinding channel frame count.
//
// Parameters:
//      None.
//
// Returns:
//      The number of encoding frames processed since the
//      last configuration request.
//
//-----------------------------------------------------------------------------
UINT32 PrpClass::PrpGetVfFrameCount()
{
    return m_iVfFrameCount;
}

//-----------------------------------------------------------------------------
//
// Function: controlledWriteDMAChannelParam
//
// This function uses Interlocked APIs to access IPU_IMA registers
// and then initializes the IDMAC channel parameters.
//
// Parameters:
//      channel
//          [in] String to identify buffer 1 EOF event.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::controlledWriteDMAChannelParam(int channel, int row, int word, unsigned int data)
{
    unsigned addr  =
        0x10000                            // MEM_NU = DMA CPM
        + (((channel * 2) + row)<< 3)    // ROW_NU = (channel*2) << 3
        + word;                            // WORD_NU

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.

    while (1)
    {
        if (INREG32(&P_IPU_REGS->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_IMA_ADDR, 0, addr) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    OUTREG32(&P_IPU_REGS->IPU_IMA_DATA, data);

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);

}

//-----------------------------------------------------------------------------
//
// Function: writeDMAChannelParam
//
// This function initializes the IDMAC channel parameters.
//
// Parameters:
//      channel
//          [in] String to identify buffer 1 EOF event.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::writeDMAChannelParam(int channel, int row, int word, unsigned int data)
{
    unsigned addr  =
        0x10000                            // MEM_NU = DMA CPM
        + (((channel * 2) + row)<< 3)    // ROW_NU = (channel*2) << 3
        + word;                            // WORD_NU

    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, addr);
    OUTREG32(&P_IPU_REGS->IPU_IMA_DATA, data);
}

//-----------------------------------------------------------------------------
//
// Function: PrpIDMACChannelConfig
//
// This function initializes the IDMAC channel parameters.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpIDMACChannelConfig(UINT8 ch,
    pPrpIDMACChannelParams pChannelParams,
    UINT32 iUBufOffset, UINT32 iVBufOffset)
{
    UINT32 newVal;

    PRP_FUNCTION_ENTRY();

    dumpChannelParams(pChannelParams);

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.

    // Set IPU_IMA_ADDR to 1 to gain control of IPU_IMA registers.
    newVal = 1;

    while (1)
    {
        if (INREG32(&P_IPU_REGS->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_IMA_ADDR, 0, newVal) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    if (pChannelParams->bInterleaved)
    {
        writeDMAChannelParam(ch, 0, 0, 0); //Variable data set to 0    31-0
        writeDMAChannelParam(ch, 0, 1, 1<<14); //Variable data set to 0. NSB set  63-32
        writeDMAChannelParam(ch, 0, 2, 0); //Variable data set to 0.   95-64
        writeDMAChannelParam(ch, 0, 3, ((pChannelParams->iWidth-1) << 12) |
                ((pChannelParams->iHeight-1) << 24)); // 127-96
        writeDMAChannelParam(ch, 0, 4, ((pChannelParams->iHeight-1) >> 8)); // 160-128

        // Notice we do not write row 1, words 0 or 1, as these 
        // words control the DMA channel buffers.  This data
        // is written by the Worker Threads for the viewfinding
        // and encoding channels.
        writeDMAChannelParam(ch, 1, 0, 0); //Buffer 0    31-0
        writeDMAChannelParam(ch, 1, 1, 0); //Buffer 1    63- 32
//        writeDMAChannelParam(ch, 1, 0, (unsigned int)pBuf1); //Buffer 0    31-0
//        writeDMAChannelParam(ch, 1, 1, (unsigned int)pBuf2); //Buffer 1    63- 32

        writeDMAChannelParam(ch, 1, 2,
            ((pChannelParams->iBitsPerPixelCode) |
            ((pChannelParams->iLineStride - 1)<<3) |
            (pChannelParams->iFormatCode<<17) | (pChannelParams->iBAM<<20) |
            (pChannelParams->iPixelBurstCode<<25))); // 95-64
        writeDMAChannelParam(ch, 1, 3,
            2 | // SAT code of 0x10 = 32 bit memory access
            (pChannelParams->pixelFormat.component0_offset<<3) |
            (pChannelParams->pixelFormat.component1_offset<<8) |
            (pChannelParams->pixelFormat.component2_offset<<13) |(0<<18) |
            (pChannelParams->pixelFormat.component0_width<<23) |
            (pChannelParams->pixelFormat.component1_width<<26)  |
            (pChannelParams->pixelFormat.component2_width<<29) ); //   127-96
        writeDMAChannelParam(ch, 1, 4, 7);//pChannelParams->pixelFormat.component3_width); //  160-128
    }
    else
    {
        writeDMAChannelParam(ch, 0, 0, 0);  //Variable data set to 0   31-0
        writeDMAChannelParam(ch, 0, 1, ((1<<(46-32)) |
            (((unsigned int)iUBufOffset)<<(53-32))));  //Variable data set to 0. NSB set  63-32
        writeDMAChannelParam(ch, 0, 2, ((((unsigned int)iUBufOffset)>>(64-53)) |
            (((unsigned int)iVBufOffset)<<(79-64))));  //Variable data set to 0.    95-64
        writeDMAChannelParam(ch, 0, 3, ((((unsigned int)iVBufOffset)>>(96-79)) |
            ((pChannelParams->iWidth-1)<<12) | ((pChannelParams->iHeight-1)<<24))); // 127-96
        writeDMAChannelParam(ch, 0, 4, ((pChannelParams->iHeight-1) >> 8));    // 160-128

        // Notice we do not write row 1, words 0 or 1, as these 
        // words control the DMA channel buffers.  This data
        // is written by the Worker Threads for the viewfinding
        // and encoding channels.
//        writeDMAChannelParam(ch, 1, 0, (unsigned int)pBuf1); //Buffer 0   31-0
//        writeDMAChannelParam(ch, 1, 1, (unsigned int)pBuf2); //Buffer 1   63-32
        writeDMAChannelParam(ch, 1, 2,
            (3 | ((pChannelParams->iLineStride - 1)<<3) |
            (pChannelParams->iFormatCode<<17) | (pChannelParams->iBAM<<20) |
            (pChannelParams->iPixelBurstCode<<25))); // 95-64
        writeDMAChannelParam(ch, 1, 3, 0); //   98- 96
    }

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);

    PRP_FUNCTION_EXIT();
    return;
}


//-----------------------------------------------------------------------------
//
// Function: controlledWriteICTaskParam
//
// This function uses Interlocked APIs to access IPU_IMA registers
// and then initializes the IDMAC channel parameters.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::controlledWriteICTaskParam(int row, int word, unsigned int data)
{
    unsigned addr  =
        0x00000                         // MEM_NU = IC TPM
        | ((row)<< 3)                   // ROW_NU = row
        | word;                         // WORD_NU

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.

    while (1)
    {
        if (INREG32(&P_IPU_REGS->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_IMA_ADDR, 0, addr) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    OUTREG32(&P_IPU_REGS->IPU_IMA_DATA, data);

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);
}

//-----------------------------------------------------------------------------
//
// Function: writeICTaskParam
//
// This function initializes the IDMAC channel parameters.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::writeICTaskParam(int row, int word, unsigned int data)
{
    unsigned addr  =
        0x00000                         // MEM_NU = IC TPM
        | ((row)<< 3)                   // ROW_NU = row
        | word;                         // WORD_NU

    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, addr);
    OUTREG32(&P_IPU_REGS->IPU_IMA_DATA, data);
}

//-----------------------------------------------------------------------------
//
// Function: PrpTaskParamConfig
//
// This function sets up the IC task parameter memory
// to configure color space conversion parameters.
//
// Parameters:
//      taskParamMemory
//          [in] The specific task parameter memory to configure
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpTaskParamConfig(prpCSCTaskParam taskParamMemory, pPrpCSCCoeffs pCSCCoeffs)
{
    UINT32 newVal;

    PRP_FUNCTION_ENTRY();

    dumpCoeffs(pCSCCoeffs);

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.
    
    // Set IPU_IMA_ADDR to 1 to gain control of IPU_IMA registers.
    newVal = 1;

    while (1)
    {
        if (INREG32(&P_IPU_REGS->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_IMA_ADDR, 0, newVal) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    // Get the TPM CSC1 addr
    UINT32 TPMEncCSC1Addr = 0;
    UINT32 TPMVFCSC1Addr = 0;
    TPMEncCSC1Addr = IPUGetTPMEncCSC1Addr();
    TPMVFCSC1Addr = IPUGetTPMVFCSC1Addr();
    
    switch (taskParamMemory)
    {
        case prpCSCEncMatrix1:
            // Internal memory address for IPU IMA IC TPM Enc CSC1
            // MX31 - 0x2D1, MX35 - 0x321
            writeICTaskParam(TPMEncCSC1Addr, 0, pCSCCoeffs->C22 | (pCSCCoeffs->C11 << 9) |
                (pCSCCoeffs->C00 <<18) | (pCSCCoeffs->A0 <<27));
            writeICTaskParam(TPMEncCSC1Addr, 1, (pCSCCoeffs->A0 >> 5) | (pCSCCoeffs->Scale<<8) | (0 << 10));    //Scale=2, sat=0

            // MX31 - 0x2D2, MX35 - 0x322
            writeICTaskParam(TPMEncCSC1Addr + 1, 0, pCSCCoeffs->C20 | (pCSCCoeffs->C10 << 9) |
                (pCSCCoeffs->C01 <<18) | (pCSCCoeffs->A1 <<27));
            writeICTaskParam(TPMEncCSC1Addr + 1, 1, (pCSCCoeffs->A1 >> 5));

            // MX31 - 0x2D3, MX35 - 0x323
            writeICTaskParam(TPMEncCSC1Addr + 2, 0, pCSCCoeffs->C21 | (pCSCCoeffs->C12 << 9) |
                (pCSCCoeffs->C02 <<18) | (pCSCCoeffs->A2 <<27));
            writeICTaskParam(TPMEncCSC1Addr + 2, 1, (pCSCCoeffs->A2 >> 5));
            break;

        case prpCSCVfMatrix1:
            // Internal memory address for IPU IMA IC TPM VF CSC1
            // MX31 - 0x5a5, MX35 - 0x645
            writeICTaskParam(TPMVFCSC1Addr, 0, pCSCCoeffs->C22 | (pCSCCoeffs->C11 << 9) |
                (pCSCCoeffs->C00 <<18) | (pCSCCoeffs->A0 <<27));
            writeICTaskParam(TPMVFCSC1Addr, 1, (pCSCCoeffs->A0 >> 5) | (pCSCCoeffs->Scale<<8) | (0 << 10));    //Scale=2, sat=0

            // MX31 - 0x5a6, MX35 - 0x646
            writeICTaskParam(TPMVFCSC1Addr + 1, 0, pCSCCoeffs->C20 | (pCSCCoeffs->C10 << 9) |
                (pCSCCoeffs->C01 <<18) | (pCSCCoeffs->A1 <<27));
            writeICTaskParam(TPMVFCSC1Addr + 1, 1, (pCSCCoeffs->A1 >> 5));

            // MX31 - 0x5a7, MX35 - 0x647
            writeICTaskParam(TPMVFCSC1Addr + 2, 0, pCSCCoeffs->C21 | (pCSCCoeffs->C12 << 9) |
                (pCSCCoeffs->C02 <<18) | (pCSCCoeffs->A2 <<27));
            writeICTaskParam(TPMVFCSC1Addr + 2, 1, (pCSCCoeffs->A2 >> 5));
            break;

        default:
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Invalid task parameter memory selected for CSC configuration. \r\n"), __WFUNCTION__));
    }

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);

    PRP_FUNCTION_EXIT();
    return;
}


//-----------------------------------------------------------------------------
//
// Function: PrpGetResizeCoeffs
//
// This function computes the resizing coefficients from
// the input and output size.
//
// Parameters:
//      inSize
//          [in] Input size (height or width)
//
//      outSize
//          [in] Output size (height of width)
//
//      resizeCoeffs
//          [out] downsizing and resizing coefficients computed.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpGetResizeCoeffs(UINT16 inSize, UINT16 outSize, pPrpResizeCoeffs resizeCoeffs)
{
    UINT16 tempSize;
    UINT16 tempDownsize;

    PRP_FUNCTION_ENTRY();

    // Cannot downsize more than 8:1
    if ((outSize << 3) < inSize)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Maximum downsize ratio is 8:1.  Input Size specified: %d.  Output Size specified: %d. \r\n"),
            __WFUNCTION__, inSize, outSize));
        return FALSE;
    }

    // compute downsizing coefficient
    tempDownsize = 0;
    tempSize = inSize;

    // When input size is SVGA(800*600),output size is QCIF(176*144) or QQVGA(160*120),
    // if using while ((tempSize > outSize) && (tempDownsize < 3)),tempDownsize = 3
    // But Downsizing ratio 1:1(tempDownsize=0), 2:1(1), 4:1(2),Downsizing ratio should be less than 8:1
    //while ((tempSize > outSize) && (tempDownsize < 3))
    while ((tempSize > outSize) && (tempDownsize < 2))//tempDownsize = 0 or 1 or 2
    {
        tempSize >>= 1;
        tempDownsize++;
    }
    resizeCoeffs->downsizeCoeff = tempDownsize;

    // compute resizing coefficient using the following equation:
    //      resizeCoeff = M*(SI -1)/(SO - 1)
    //      where M = 2^13, SI - input size, SO - output size
    // Resizing ratio(resizeCoeff/M) from 2:1 to 1:M,so 1 < resizeCoeff < 8192*2
    resizeCoeffs->resizeCoeff =  8192 * (tempSize - 1) / (outSize - 1);

    RETAILMSG(0,(TEXT("DownsizeCoeff=%x , ResizeCoeff=%x \n"),resizeCoeffs->downsizeCoeff,resizeCoeffs->resizeCoeff));
    PRP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PrpEnable
//
// Enable the Image Converter and the IDMAC channels we will need.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpEnable(void)
{
    DWORD dwBytesTransferred;
    IPU_DRIVER driver = IPU_DRIVER_PRP;
    RETAILMSG (0,
            (TEXT("%s: Enabling PRP!\r\n"), __WFUNCTION__));

    PRP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPrpEnable);

    // If either the ENC or VF channels is already running, then
    // the IC must already be enabled, and we can return.
    if (m_bEncRunning || m_bVfRunning)
    {
        LeaveCriticalSection(&m_csPrpEnable);
        return;
    }

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Enabling IC!\r\n"), __WFUNCTION__));

    if (!DeviceIoControl(hIPUBase,       // file handle to the driver
             IPU_IOCTL_ENABLE_IC,         // I/O control code
             &driver,                                // in buffer
             sizeof(IPU_DRIVER),              // in buffer size
             NULL,                                    // out buffer
             0,                                         // out buffer size
             &dwBytesTransferred,            // number of bytes returned
             NULL))                                  // ignored (=NULL)
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to enable IC!\r\n"), __WFUNCTION__));
    }

    m_bPrpEnabled = TRUE;

    LeaveCriticalSection(&m_csPrpEnable);
//    dumpIpuRegisters(P_IPU_REGS);

    PRP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PrpDisable
//
// Disable the Image Converter and the Preprocessing IDMAC channels.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpDisable(void)
{
    DWORD dwBytesTransferred;
    IPU_DRIVER driver = IPU_DRIVER_PRP;
    RETAILMSG (0,
            (TEXT("%s: Disabling PRP!\r\n"), __WFUNCTION__));

    PRP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPrpEnable);

    // If either the ENC or VF channels is still running, then
    // we should NOT disable the IC.
    if (m_bEncRunning || m_bVfRunning)
    {
        LeaveCriticalSection(&m_csPrpEnable);
        return;
    }

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Disabling IC!\r\n"), __WFUNCTION__));

    if (!DeviceIoControl(hIPUBase,        // file handle to the driver
             IPU_IOCTL_DISABLE_IC,        // I/O control code
             &driver,                                 // in buffer
             sizeof(IPU_DRIVER),               // in buffer size
             NULL,                                     // out buffer
             0,                                          // out buffer size
             &dwBytesTransferred,             // number of bytes returned
             NULL))                                   // ignored (=NULL)
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to disable IC!\r\n"), __WFUNCTION__));
    }

    m_bPrpEnabled = FALSE;

    LeaveCriticalSection(&m_csPrpEnable);

    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PrpClearInterruptStatus
//
// This function is used to clear the CAM interrupt status and signal to the
// kernel that interrupt processing is completed.
//
// Parameters:
//      clearBitmask
//          [in] Mask of bits in status register to clear
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PrpClass::PrpClearInterruptStatus(DWORD clearBitmask)
{
    PRP_FUNCTION_ENTRY();

    // Clear Interrupt Status Bits
    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_1, clearBitmask);

    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PrpIntrThread
//
// This function is the IST thread.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpIntrThread(LPVOID lpParameter)
{
    PrpClass *pPrp = (PrpClass *)lpParameter;

    PRP_FUNCTION_ENTRY();

    pPrp->PrpISRLoop(INFINITE);

    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PrpISRLoop
//
// This function is the interrupt handler for the Preprocessor.
// It waits for the End-Of-Frame (EOF) interrupt, and signals
// the EOF event registered by the user of the preprocessor.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpISRLoop(UINT32 timeout)
{
    DWORD statReg1;
    DISPLAY_BUFFER dispBufferData;
    DWORD dwFlags, bytesRead;
    UINT32 buf0_rdy, buf1_rdy, idmac_busy;
    UINT32 oldVal, newVal, iMask, iBitval;
    BOOL bSetDisplayBuffer = FALSE;
    BYTE byTVinType = 0;

    PRP_FUNCTION_ENTRY();

    // loop here
    while(TRUE)
    {                
        DEBUGMSG (ZONE_DEVICE, (TEXT("%s: In the loop\r\n"), __WFUNCTION__));
        
        if ( WaitForSingleObject ( m_hExitPrpISRThread, 0 ) != WAIT_TIMEOUT )
        {
            RETAILMSG(1,(TEXT("PrpISRLoop for ExitThread")));
            ExitThread(1);
        }

        if (WaitForSingleObject(m_hPrpIntrEvent, timeout) == WAIT_OBJECT_0)
        {
            // Thread is running properly, so we do not have to restart.
            m_bEncRestartISRLoop = FALSE;
            m_bVfRestartISRLoop = FALSE;

            //For Ringo TVIN +: only support PREVIEW channel
            //TVIN only support preview channel
            //Only check NTSC changing to PAL
            //NTSC:csi&prp output/input size is set to 720*480(scane line is 720*525),
            //and when tv type changes to PAL 720*576(scan line is 720*625)
            //It can work but the vedio will flicker,because the csi output size should be bigger then prp input size.
            if (m_iCamType == 4 && gLastTVinType == 0 )//m_iCamType:check csiSensorId_c in bspcsi.h
            {
                byTVinType = BSPGetTVinType();
                if (byTVinType != gLastTVinType)
                {
                    if (gLastTVinType == 0)
                        RETAILMSG(0,(TEXT("%s: TVin Type is changing from NTSC to PAL.\r\n "), __WFUNCTION__));
                    else
                        RETAILMSG(0,(TEXT("%s: TVin Type is changing from PAL to NTSC.\r\n "), __WFUNCTION__));
                    gLastTVinType = byTVinType;
                    
                    m_bNTSCtoPAL = TRUE;
                }
            }

            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Interrupt received\r\n"), __WFUNCTION__));

            statReg1 = INREG32(&P_IPU_REGS->IPU_INT_STAT_1);

            //dumpInterruptRegisters(P_IPU_REGS);
            if (statReg1 & 0x101)
            {
                // Clear interrupt bits
                PrpClearInterruptStatus(0x501);
                buf0_rdy = INREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY);
                buf1_rdy = INREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY);
                idmac_busy = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);

                // EOF for encoding path
                DEBUGMSG (ZONE_DEVICE, (TEXT("%s:*** Encoding End of frame interrupt ***\r\n"), __WFUNCTION__));

                // Rotation Case
                if (m_bEncFlipRot)
                {
                    // Critical section prevents race condition on
                    // reading and writing ready and busy bits
                    EnterCriticalSection(&m_csEncStopping);

                    if (!m_bEncRunning)
                    {
                        // Don't do anything else.  We do not want to re-enable
                        // the buffer ready bits since we are stopping.
                    }
                    else if (m_bEncRestartISRLoop)
                    {
                        // This code ensures that we do not run into problems
                        // if the Enc channel was stopped at an earlier time while
                        // this thread was waiting at the m_csEncStopping critical 
                        // section.  If so, we simply start over and wait for a new
                        // interrupt.

                        LeaveCriticalSection(&m_csEncStopping);
                        continue;
                    }
                    else
                    {
                        // If Channel 0 has completed
                        if (statReg1 & 0x001)
                        {
                            // Protect access to IPU_INT_CTRL_1 register.
                            // Re-enable interrupt, which is disabled in
                            // ipu_base ISR.

                            // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                            iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0);
                            iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_ENABLE);

                            // Also enable IPU interrupts for channel 0 (IC->Mem
                            // after pre-proc).  This will allow us to inform 
                            // the waiting thread that we need a new buffer.
                            do
                            {
                                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                                newVal = (oldVal & (~iMask)) | iBitval;
                            } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                    oldVal, newVal) != oldVal);

                            // We do not trigger EOF to camera 
                            // upper layers, because we have not
                            // necessarily completed rotation yet,
                            // just the preprocessing is complete.

                            if (m_iCurrentEncBuf == 0)
                            {
                                m_iEncBuf0Ready = FALSE;
                                m_iCurrentEncBuf = 1;

                                // Buffer 0 ready for IDMAC Channel 10 (Mem->IC for rotation for encoding)
                                SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_10, IPU_DMA_CHA_READY));

                                // Buffer 0 ready for IDMAC Channel 8 (IC->Mem after rotation for encoding)
                                SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_DMA_CHA_READY));

                                // Check for the case where both buffer 0 and buffer 1
                                // completed before the ISR could service the interrupt.
                                if (m_iEncBuf1Ready && !(buf1_rdy & 0x1) && !(idmac_busy & 0x1))
                                {
                                    // Both buffers have completed
                                    DEBUGMSG(ZONE_ERROR, 
                                      (TEXT("%s : It happened (Buf0 current)!!! $$$.\r\n"), __WFUNCTION__));

                                    // Proceed through step to process EOF for Buffer 1.

                                    // Buffer 1 ready for IDMAC Channel 10 (Mem->IC for rotation for encoding)
                                    SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                                        CSP_BITFVAL(IPU_DMA_CHA_DMAIC_10, IPU_DMA_CHA_READY));

                                    // Buffer 1 ready for IDMAC Channel 8 (IC->Mem after rotation for encoding)
                                    SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                                        CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_DMA_CHA_READY));

                                    // Buffer 1 has also completed, and buffer 0
                                    // is still the next active buffer.
                                    m_iEncBuf1Ready = FALSE;
                                    m_iCurrentEncBuf = 0;
                                }
                            }
                            else if (m_iCurrentEncBuf == 1)
                            {
                                m_iEncBuf1Ready = FALSE;
                                m_iCurrentEncBuf = 0;

                                // Buffer 1 ready for IDMAC Channel 10 (Mem->IC for rotation for encoding)
                                SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_10, IPU_DMA_CHA_READY));

                                // Buffer 1 ready for IDMAC Channel 8 (IC->Mem after rotation for encoding)
                                SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_DMA_CHA_READY));

                                if (m_iEncBuf0Ready && !(buf0_rdy & 0x1) && !(idmac_busy & 0x1))
                                {
                                    // Both buffers have completed
                                    DEBUGMSG(ZONE_ERROR, 
                                        (TEXT("%s : It happened (Buf1 current)!!! $$$.\r\n"), __WFUNCTION__));

                                    // Proceed through step to process EOF for Buffer 0.

                                    // Buffer 0 ready for IDMAC Channel 10 (Mem->IC for rotation for encoding)
                                    SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                                        CSP_BITFVAL(IPU_DMA_CHA_DMAIC_10, IPU_DMA_CHA_READY));

                                    // Buffer 0 ready for IDMAC Channel 8 (IC->Mem after rotation for encoding)
                                    SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                                        CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_DMA_CHA_READY));

                                    // Buffer 0 has also completed, and buffer 1
                                    // is still the next active buffer.
                                    m_iEncBuf0Ready = FALSE;
                                    m_iCurrentEncBuf = 1;
                                }
                            }
                        }

                        // If Channel 8 has completed
                        if (statReg1 & 0x100)
                        {
                            // Protect access to IPU_INT_CTRL_1 register.
                            // Re-enable interrupt, which is disabled in
                            // ipu_base ISR.

                            // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                            iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_8);
                            iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_8, IPU_ENABLE);

                            // Enable IPU interrupts for channel 8 (IC->Mem 
                            // after rotation).  This is the final channel 
                            // in the chain, so we want this channel to trigger
                            // an interrupt that the camera IST will handle.
                            do
                            {
                                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                                newVal = (oldVal & (~iMask)) | iBitval;
                            } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                    oldVal, newVal) != oldVal);

                            // increment frame count
                            m_iEncFrameCount++;

                            // Set non-rotation buffer back to idle
                            pEncBufferManager->SetFilledBuffer();
                            pEncBufferManager->GetBufferFilled();

                            pEncRotBufferManager->SetFilledBuffer();

                            if (m_iCurrentEncRotBuf == 0)
                            {
                                m_iCurrentEncRotBuf = 1;

                                // Request buffer for IDMAC Channel 0 (IC->Mem for encoding).
                                // This will attempt to set up buffer 0.
                                SetEvent(m_hEncBufWaitList[prpBuf0RequestEvent]);

                                // Request buffer for IDMAC Channel 8 (IC->Mem after 
                                // rotation for encoding).
                                // This will attempt to set up buffer 0.
                                SetEvent(m_hEncRotBufWaitList[prpBuf0RequestEvent]);
                            }
                            else if (m_iCurrentEncRotBuf == 1)
                            {
                                m_iCurrentEncRotBuf = 0;

                                // Request buffer for IDMAC Channel 0 (IC->Mem for encoding).
                                // This will attempt to set up buffer 1.
                                SetEvent(m_hEncBufWaitList[prpBuf1RequestEvent]);

                                // Request buffer for IDMAC Channel 8 (IC->Mem after 
                                // rotation for encoding).
                                // This will attempt to set up buffer 1.
                                SetEvent(m_hEncRotBufWaitList[prpBuf1RequestEvent]);
                            }

                            // Trigger Encoder EOF event
                            SetEvent(m_hEncEOFEvent);
                        }
                    }

                    LeaveCriticalSection(&m_csEncStopping);

                }
                // No Rotation Case
                else 
                {
                    // Critical section prevents race condition on
                    // reading and writing ready and busy bits
                    EnterCriticalSection(&m_csEncStopping);

                    if (!m_bEncRunning)
                    {
                        // Don't do anything else.  We do not want to re-enable
                        // the buffer ready bits since we are stopping.
                    }
                    else if (m_bEncRestartISRLoop)
                    {
                        // This code ensures that we do not run into problems
                        // if the Enc channel was stopped at an earlier time while
                        // this thread was waiting at the m_csEncStopping critical 
                        // section.  If so, we simply start over and wait for a new
                        // interrupt.

                        LeaveCriticalSection(&m_csEncStopping);
                        continue;
                    }
                    else
                    {
                        // Protect access to IPU_INT_CTRL_1 register.
                        // Re-enable interrupt, which is disabled in
                        // ipu_base ISR.

                        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_0);
                        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_ENABLE);

                        // Also enable IPU interrupts for channel 0 (IC->Mem
                        // after pre-proc).  This will allow us to inform 
                        // the waiting thread that we need a new buffer.
                        do
                        {
                            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                            newVal = (oldVal & (~iMask)) | iBitval;
                        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                oldVal, newVal) != oldVal);

                        if (m_iCurrentEncBuf == 0)
                        {
                            m_iEncBuf0Ready = FALSE;
                            m_iCurrentEncBuf = 1;

                            // increment frame count
                            m_iEncFrameCount++;

                            pEncBufferManager->SetFilledBuffer();

                            // Request buffer for IDMAC Channel 0 (IC->Mem for encoding).
                            // This will attempt to set up buffer 0.
                            SetEvent(m_hEncBufWaitList[prpBuf0RequestEvent]);

                            // Check for the case where both buffer 0 and buffer 1
                            // completed before the ISR could service the interrupt.
                            if (m_iEncBuf1Ready && !(buf1_rdy & 0x1) && !(idmac_busy & 0x1))
                            {
                                // Both buffers have completed
                                DEBUGMSG(ZONE_ERROR, 
                                  (TEXT("%s : It happened (Buf0 current)!!! $$$.\r\n"), __WFUNCTION__));

                                // Proceed through steps to process EOF for Buffer 0.

                                // increment frame count
                                m_iEncFrameCount++;

                                pEncBufferManager->SetFilledBuffer();

                                // Request buffer for IDMAC Channel 0 (IC->Mem for encoding).
                                // This will attempt to set up buffer 1.
                                SetEvent(m_hEncBufWaitList[prpBuf1RequestEvent]);

                                // Buffer 1 has also completed, and buffer 0
                                // is still the next active buffer.
                                m_iEncBuf1Ready = FALSE;
                                m_iCurrentEncBuf = 0;
                            }
                        }
                        else if (m_iCurrentEncBuf == 1)
                        {
                            m_iEncBuf1Ready = FALSE;
                            m_iCurrentEncBuf = 0;

                            // increment frame count
                            m_iEncFrameCount++;

                            pEncBufferManager->SetFilledBuffer();

                            // Request buffer for IDMAC Channel 0 (IC->Mem for encoding).
                            // This will attempt to set up buffer 1.
                            SetEvent(m_hEncBufWaitList[prpBuf1RequestEvent]);

                            if (m_iEncBuf0Ready && !(buf0_rdy & 0x1) && !(idmac_busy & 0x1))
                            {
                                // Both buffers have completed
                                DEBUGMSG(ZONE_ERROR, 
                                    (TEXT("%s : It happened (Buf1 current)!!! $$$.\r\n"), __WFUNCTION__));

                                // Proceed through steps to process EOF for Buffer 1.

                                // increment frame count
                                m_iEncFrameCount++;

                                pEncBufferManager->SetFilledBuffer();

                                // Request buffer for IDMAC Channel 0 (IC->Mem for encoding).
                                // This will attempt to set up buffer 0.
                                SetEvent(m_hEncBufWaitList[prpBuf0RequestEvent]);

                                // Buffer 0 has also completed, and buffer 1
                                // is still the next active buffer.
                                m_iEncBuf0Ready = FALSE;
                                m_iCurrentEncBuf = 1;
                            }
                        }

                        // Trigger Encoder EOF event
                        SetEvent(m_hEncEOFEvent);
                    }

                    LeaveCriticalSection(&m_csEncStopping);
                }
            }

            if (statReg1 & 0x202)
            {            
                // Clear interrupt status bits
                PrpClearInterruptStatus(0xA02);
                buf0_rdy = INREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY);
                buf1_rdy = INREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY);
                idmac_busy = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);

                // EOF for viewfinding path
                DEBUGMSG (ZONE_DEVICE, (TEXT("%s:*** Viewfinding End of frame interrupt ***\r\n"), __WFUNCTION__));
                RETAILMSG (0, (TEXT("%s:*** Viewfinding End of frame interrupt: statReg1 = %x,buf0_rdy=%x,buf1_rdy=%x,idmac_busy=%x ***\r\n"), __WFUNCTION__, statReg1,buf0_rdy,buf1_rdy,idmac_busy));

                // Viewfinding Rotation Case
                if (m_bVfFlipRot)
                {
                    // Critical section prevents race condition on
                    // reading and writing ready and busy bits
                    EnterCriticalSection(&m_csVfStopping);

                    if (!m_bVfRunning)
                    {
                        // Don't do anything else.  We do not want to re-enable
                        // the buffer ready bits since we are stopping.
                    }
                    else if (m_bVfRestartISRLoop)
                    {
                        // This code ensures that we do not run into problems
                        // if the VF channel was stopped at an earlier time while
                        // this thread was waiting at the m_csVfStopping critical 
                        // section.  If so, we simply start over and wait for a new
                        // interrupt.

                        LeaveCriticalSection(&m_csVfStopping);
                        continue;
                    }
                    else
                    {
                        // If Channel 1 has completed
                        if (statReg1 & 0x002)
                        {
                            // Protect access to IPU_INT_CTRL_1 register.
                            // Re-enable interrupt, which is disabled in
                            // ipu_base ISR.

                            // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                            iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1);
                            iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_ENABLE);

                            // Also enable IPU interrupts for channel 1 (IC->Mem).
                            do
                            {
                                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                                newVal = (oldVal & (~iMask)) | iBitval;
                            } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                    oldVal, newVal) != oldVal);
                            RETAILMSG (0, (TEXT("%s:*** enable IPU interrupts for channel 1: oldVal = %x, newVal = %x ***\r\n"), __WFUNCTION__,oldVal, newVal));

                            // We do not trigger EOF to camera 
                            // upper layers, because we have not
                            // necessarily completed rotation yet,
                            // just the preprocessing is complete.

                            if (m_iCurrentVfBuf == 0)
                            {
                                m_iVfBuf0Ready = FALSE;
                                m_iCurrentVfBuf = 1;

                                // Buffer 0 ready for IDMAC Channel 11 (Mem->IC for rotation for viewfinding)
                                SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_11, IPU_DMA_CHA_READY));

                                // Buffer 0 ready for IDMAC Channel 9 (IC->Mem after rotation for viewfinding)
                                SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_DMA_CHA_READY));

                                // Check for the case where both buffer 0 and buffer 1
                                // completed before the ISR could service the interrupt.
                                if (m_iVfBuf1Ready && !(buf1_rdy & 0x2) && !(idmac_busy & 0x2))
                                {
                                    // Both buffers have completed
                                    DEBUGMSG(ZONE_ERROR, 
                                      (TEXT("%s : It happened (Buf0 current)!!! $$$.\r\n"), __WFUNCTION__));

                                    // Proceed through step to process EOF for Buffer 1.

                                    // Buffer 1 ready for IDMAC Channel 11 (Mem->IC for rotation for viewfinding)
                                    SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                                        CSP_BITFVAL(IPU_DMA_CHA_DMAIC_11, IPU_DMA_CHA_READY));

                                    // Buffer 1 ready for IDMAC Channel 9 (IC->Mem after rotation for viewfinding)
                                    SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                                        CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_DMA_CHA_READY));

                                    // Buffer 1 has also completed, and buffer 0
                                    // is still the next active buffer.
                                    m_iVfBuf1Ready = FALSE;
                                    m_iCurrentVfBuf = 0;
                                }
                            }
                            else if (m_iCurrentVfBuf == 1)
                            {
                                m_iVfBuf1Ready = FALSE;
                                m_iCurrentVfBuf = 0;

                                // Buffer 1 ready for IDMAC Channel 11 (Mem->IC for rotation for viewfinding)
                                SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_11, IPU_DMA_CHA_READY));

                                // Buffer 1 ready for IDMAC Channel 9 (IC->Mem after rotation for viewfinding)
                                SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_DMA_CHA_READY));

                                if (m_iVfBuf0Ready && !(buf0_rdy & 0x2) && !(idmac_busy & 0x2))
                                {
                                    // Both buffers have completed
                                    DEBUGMSG(ZONE_ERROR, 
                                        (TEXT("%s : It happened (Buf1 current)!!! $$$.\r\n"), __WFUNCTION__));

                                    // Proceed through step to process EOF for Buffer 0.

                                    // Buffer 0 ready for IDMAC Channel 11 (Mem->IC for rotation for viewfinding)
                                    SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                                        CSP_BITFVAL(IPU_DMA_CHA_DMAIC_11, IPU_DMA_CHA_READY));

                                    // Buffer 0 ready for IDMAC Channel 9 (IC->Mem after rotation for viewfinding)
                                    SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                                        CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_DMA_CHA_READY));

                                    // Buffer 0 has also completed, and buffer 1
                                    // is still the next active buffer.
                                    m_iVfBuf0Ready = FALSE;
                                    m_iCurrentVfBuf = 1;
                                }
                            }
                        }

                        // If Channels 11 and 9 have completed
                        if (statReg1 & 0x200)
                        {
                            // Protect access to IPU_INT_CTRL_1 register.
                            // Re-enable interrupt, which is disabled in
                            // ipu_base ISR.

                            // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                            iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_9);
                            iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_9, IPU_ENABLE);

                            // Enable IPU interrupts for channel 9 (IC->Mem 
                            // after rotation).  This is the final channel in
                            // the chain, so we want this channel to trigger 
                            // an interrupt that the camera IST will handle.
                            do
                            {
                                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                                newVal = (oldVal & (~iMask)) | iBitval;
                            } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                    oldVal, newVal) != oldVal);

                            // increment frame count
                            m_iVfFrameCount++;

                            // Set non-rotation buffer back to idle
                            pVfBufferManager->SetFilledBuffer();
                            pVfBufferManager->GetBufferFilled();

                            pVfRotBufferManager->SetFilledBuffer();

                            // Trigger Viewfinder EOF event
                            SetEvent(m_hVfEOFEvent);

                            if (m_iCurrentVfRotBuf == 0)
                            {
                                m_iCurrentVfRotBuf = 1;

                                // Request buffer for IDMAC Channel 1 (IC->Mem for viewfinding)
                                // and for Channel 11 (Mem-IC for rotation for viewfinding).
                                // This will attempt to set up buffer 0.
                                SetEvent(m_hVfBufWaitList[prpBuf0RequestEvent]);

                                // Request buffer for IDMAC Channel 9 (IC->Mem after rotation
                                // for viewfinding).  This will attempt to set up buffer 0.
                                SetEvent(m_hVfRotBufWaitList[prpBuf0RequestEvent]);
                            }
                            else if (m_iCurrentVfRotBuf == 1)
                            {
                                m_iCurrentVfRotBuf = 0;

                                // Request buffer for IDMAC Channel 1 (IC->Mem for viewfinding)
                                // and for Channel 11 (Mem-IC for rotation for viewfinding).
                                // This will attempt to set up buffer 1.
                                SetEvent(m_hVfBufWaitList[prpBuf1RequestEvent]);

                                // Request buffer for IDMAC Channel 9 (IC->Mem after rotation
                                // for viewfinding).  This will attept to set up buffer 1.
                                SetEvent(m_hVfRotBufWaitList[prpBuf1RequestEvent]);
                            }

                            if (m_bVfDirectDisplay && m_bVfDisplayActive)
                            {
                                // Read current viewfinding buffer from queue
                                if (!ReadMsgQueue(m_hReadVfBufferQueue, &dispBufferData, sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
                                {
                                    DEBUGMSG(ZONE_ERROR, 
                                        (TEXT("%s : Couldn't read from VF buffer queue.  Skipping SetBuffer\r\n"), __WFUNCTION__));
                                    LeaveCriticalSection(&m_csVfStopping);
                                    continue;
                                }

                                bSetDisplayBuffer = TRUE;
                            }
                        }
                    }

                    LeaveCriticalSection(&m_csVfStopping);
                }
                // No Rotation Viewfinding case
                else
                {
                    // Critical section prevents race condition on
                    // reading and writing ready and busy bits
                    EnterCriticalSection(&m_csVfStopping);

                    if (!m_bVfRunning)
                    {
                        // Don't do anything else.  We do not want to re-enable
                        // the buffer ready bits since we are stopping.
                    }
                    else if (m_bVfRestartISRLoop)
                    {
                        // This code ensures that we do not run into problems
                        // if the VF channel was stopped at an earlier time while
                        // this thread was waiting at the m_csVfStopping critical 
                        // section.  If so, we simply start over and wait for a new
                        // interrupt.

                        LeaveCriticalSection(&m_csVfStopping);
                        continue;
                    }
                    else
                    {
                        // Protect access to IPU_INT_CTRL_1 register.
                        // Re-enable interrupt, which is disabled in
                        // ipu_base ISR.

                        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_1);
                        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_ENABLE);

                        // Also enable IPU interrupts for channel 1 (IC->Mem).
                        do
                        {
                            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                            newVal = (oldVal & (~iMask)) | iBitval;
                        } while (InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                oldVal, newVal) != oldVal);

                        if (m_iCurrentVfBuf == 0)
                        {
                            m_iVfBuf0Ready = FALSE;
                            m_iCurrentVfBuf = 1;

                            // increment frame count
                            m_iVfFrameCount++;

                            pVfBufferManager->SetFilledBuffer();
                        
                            // Request buffer for IDMAC Channel 1 (IC->Mem for viewfinding).
                            // This will attempt to set up buffer 0.
                            SetEvent(m_hVfBufWaitList[prpBuf0RequestEvent]);

                            // Check for the case where both buffer 0 and buffer 1
                            // completed before the ISR could service the interrupt.
                            if (m_iVfBuf1Ready && !(buf1_rdy & 0x2) && !(idmac_busy & 0x2))
                            {
                                // Both buffers have completed
                                DEBUGMSG(ZONE_ERROR, 
                                  (TEXT("%s : It happened (Buf0 current)!!! $$$.\r\n"), __WFUNCTION__));

                                // Proceed through steps to process EOF for Buffer 0.

                                // increment frame count
                                m_iVfFrameCount++;

                                pVfBufferManager->SetFilledBuffer();

                                // Request buffer for IDMAC Channel 9 (IC->Mem after rotation
                                // for viewfinding).  This will attempt to set up buffer 1.
                                SetEvent(m_hVfBufWaitList[prpBuf1RequestEvent]);

                                if (m_bVfDirectDisplay)
                                {
                                    // Read current viewfinding buffer from 
                                    // queue. We do not need to do anything 
                                    // with this data, as we will read again
                                    // for Buffer 1.
                                    if (!ReadMsgQueue(m_hReadVfBufferQueue, &dispBufferData, 
                                        sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
                                    {
                                        DEBUGMSG(ZONE_ERROR, 
                                            (TEXT("%s : Error!  Couldn't read from VF buffer queue.\r\n"), __WFUNCTION__));
                                    }
                                }

                                // Buffer 1 has also completed, and buffer 0
                                // is still the next active buffer.
                                m_iVfBuf1Ready = FALSE;
                                m_iCurrentVfBuf = 0;
                            }
                        }
                        else if (m_iCurrentVfBuf == 1)
                        {
                            m_iVfBuf1Ready = FALSE;
                            m_iCurrentVfBuf = 0;

                            // increment frame count
                            m_iVfFrameCount++;

                            pVfBufferManager->SetFilledBuffer();

                            // Request buffer for IDMAC Channel 1 (IC->Mem for viewfinding).
                            // This will attempt to set up buffer 1.
                            SetEvent(m_hVfBufWaitList[prpBuf1RequestEvent]);

                            if (m_iVfBuf0Ready && !(buf0_rdy & 0x2) && !(idmac_busy & 0x2))
                            {
                                // Both buffers have completed
                                DEBUGMSG(ZONE_ERROR, 
                                    (TEXT("%s : It happened (Buf1 current)!!! $$$.\r\n"), __WFUNCTION__));

                                // Proceed through steps to process EOF for Buffer 1.

                                // increment frame count
                                m_iVfFrameCount++;

                                pVfBufferManager->SetFilledBuffer();

                                // Request buffer for IDMAC Channel 9 (IC->Mem after rotation
                                // for viewfinding).  This will attempt to set up buffer 0.
                                SetEvent(m_hVfBufWaitList[prpBuf0RequestEvent]);

                                if (m_bVfDirectDisplay)
                                {
                                    // Read current viewfinding buffer from 
                                    // queue. We do not need to do anything 
                                    // with this data, as we will read again
                                    // for Buffer 0.
                                    if (!ReadMsgQueue(m_hReadVfBufferQueue, &dispBufferData,
                                        sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
                                    {
                                        DEBUGMSG(ZONE_ERROR, 
                                            (TEXT("%s : Error!  Couldn't read from VF buffer queue.\r\n"), __WFUNCTION__));
                                    }
                                }

                                // Buffer 0 has also completed, and buffer 1
                                // is still the next active buffer.
                                m_iVfBuf0Ready = FALSE;
                                m_iCurrentVfBuf = 1;
                            }
                        }

                        // Trigger Viewfinder EOF event
                        SetEvent(m_hVfEOFEvent);

                        if (m_bVfDirectDisplay && m_bVfDisplayActive)
                        {
                            // Read current viewfinding buffer from queue
                            if (!ReadMsgQueue(m_hReadVfBufferQueue, &dispBufferData, sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
                            {
                                DEBUGMSG(ZONE_ERROR, 
                                    (TEXT("%s : Couldn't read from VF buffer queue.  Skipping SetBuffer\r\n"), __WFUNCTION__));
                                LeaveCriticalSection(&m_csVfStopping);
                                continue;
                            }

                            bSetDisplayBuffer = TRUE;
                        }
                    }

                    LeaveCriticalSection(&m_csVfStopping);
                }
            }

            // We moved this code down here so that we can leave the critical section before
            // calling this code.  Otherwise, we have a deadlock when going into Suspend mode...
            // The ExtEscape function cannot be called while going into suspend.
            if ((bSetDisplayBuffer == TRUE) && m_bVfRunning)
            {
                // Set Buffer in the display driver
                if (ExtEscape(m_hDisplay, VF_BUF_SET, sizeof(PHYSICAL_ADDRESS),
                    (LPCSTR) &dispBufferData.paBuf, 0, NULL) <= 0)
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed setting buffer in display.\r\n"), __WFUNCTION__));
                }

                // Enable Viewfinding in the display driver
                if (ExtEscape(m_hDisplay, VF_ENABLE, 0, NULL, 0, NULL) <= 0)
                {
                   DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed enabling display for viewfinding.\r\n"), __WFUNCTION__));
                }

                bSetDisplayBuffer = FALSE;
            }

            if (INREG32(&P_IPU_REGS->IPU_INT_STAT_5) & 0xFFFF)
            {
                // TODO: Properly Handle Error Cases
                UINT32 uStat5 = INREG32(&P_IPU_REGS->IPU_INT_STAT_5);
                DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Error Interrupt received!\r\n"), __WFUNCTION__));
                if (INREG32(&P_IPU_REGS->IPU_INT_STAT_5) & 0x3800)
                {
                    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Frame Lost.\r\n"), __WFUNCTION__));
                    RETAILMSG (0, (TEXT("%s: Frame Lost.\r\n"), __WFUNCTION__));
                    // Clear frame drop interrupt registers
                    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_5, 0x3800);
                }
                else
                {
                    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Other error.\r\n"), __WFUNCTION__));
                    RETAILMSG (0, (TEXT("%s: Other error.\r\n"), __WFUNCTION__));
                }
            }

        }
        else
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Time out\r\n"), __WFUNCTION__));
        }

    }

    PRP_FUNCTION_EXIT();
    return;
}


//------------------------------------------------------------------------------
//
// Function: PrpEncBufferWorkerThread
//
// This function is the worker thread for the handling buffers.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpEncBufferWorkerThread(LPVOID lpParameter)
{
    PrpClass *pPrp = (PrpClass *)lpParameter;

    PRP_FUNCTION_ENTRY();

    pPrp->PrpEncBufferWorkerRoutine(INFINITE);

    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PrpEncBufferWorkerRoutine
//
// This function is the worker thread routine that assures that
// the buffers are managed correctly in the preprocessor.  When
// a buffer is filled, this function will wait for a new buffer to
// become available and set up the channel to run.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpEncBufferWorkerRoutine(UINT32 timeout)
{
    UINT32* physAddr;
    DWORD dwFrameDropped;
    DWORD result;

    PRP_FUNCTION_ENTRY();

    while (1)
    {
        if ( WaitForSingleObject ( m_hExitPrpEncThread, 0 ) != WAIT_TIMEOUT )
        {
            RETAILMSG(1,(TEXT("PrpEncBufferWorkerRoutine for ExitThread")));
            ExitThread(1);
        }
        
        // Wait until the ISR loop receives a filled buffer, at which
        // time it will signal that it needs a new buffer.
        result = WaitForMultipleObjects(2, m_hEncBufWaitList, FALSE, timeout);

        // Thread running properly, so we can set this to FALSE;
        m_bEncRestartBufferLoop = FALSE;

        DEBUGMSG(ZONE_DEVICE, (_T("%s: Enc buffer requested\r\n"), __WFUNCTION__));

        pEncBufferManager->PrintBufferInfo();

        // A buffer has been requested.  Attempt to read from the ready
        // queue to get an available buffer.  If one is not available,
        // we wait here until it is.
        dwFrameDropped = pEncBufferManager->SetActiveBuffer(&physAddr, INFINITE);
        if (dwFrameDropped == 1)
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Frame Dropped.\r\n"), __WFUNCTION__));
            // m_dwFramesDropped++;
        }
        else if (dwFrameDropped == -1)
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Error, did not receive a buffer.\r\n"), __WFUNCTION__));
            continue;
        }

        // This code ensures that if we do not run into problems
        // if the Enc channel was stopped at an earlier time while
        // this thread was waiting in SetActiveBuffer.  If so,
        // we simply start over at the top of the loop.
        if (m_bEncRestartBufferLoop)
        {
            pEncBufferManager->ResetBuffers();
            continue;
        }

        pEncBufferManager->PrintBufferInfo();

        // Critical section to prevent race condition upon
        // stopping the encoding channel
        EnterCriticalSection(&m_csEncStopping);

        // If we are stopping the encoding channel, return 
        // active buffer to ready queue, and bail out
        // of buffer request routine, as we do not want to
        // continue processing in this channel
        if (!m_bEncRunning)
        {
            pEncBufferManager->ResetBuffers();    
            LeaveCriticalSection(&m_csEncStopping);        
            continue;
        }

        DEBUGMSG(ZONE_DEVICE, (_T("%s: Enc buffer ready\r\n"), __WFUNCTION__));

        DEBUGMSG(ZONE_DEVICE,
            (TEXT("%s: pPhysical address: %x.\r\n"), __WFUNCTION__, physAddr));

        switch(result)
        {
            // We have a buffer now.  Determine which buffer we need 
            // to fill (buf0 or buf1)
            case (WAIT_OBJECT_0 + prpBuf0RequestEvent):
                // Program buffer into task parameter memory
                controlledWriteDMAChannelParam(0, 1, 0, (unsigned int)physAddr);
                controlledWriteDMAChannelParam(0, 1, 0, (unsigned int)physAddr); //Buffer 0    31-0

                // Buffer 0 ready for IDMAC Channel 0 (IC->Mem for encoding)
                SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DMA_CHA_READY));

                if (m_bEncFlipRot)
                {
                    // Program buffer into task parameter memory
                    controlledWriteDMAChannelParam(10, 1, 0, (unsigned int)physAddr); //Buffer 0    31-0
                }

                m_iEncBuf0Ready = TRUE;
                break;

            case (WAIT_OBJECT_0 + prpBuf1RequestEvent):
                // Program buffer into task parameter memory
                controlledWriteDMAChannelParam(0, 1, 1, (unsigned int)physAddr); //Buffer 1    63- 32

                // Buffer 1 ready for IDMAC Channel 0 (IC->Mem for encoding)
                SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_0, IPU_DMA_CHA_READY));

                if (m_bEncFlipRot)
                {
                    // Program buffer into task parameter memory
                    controlledWriteDMAChannelParam(10, 1, 1, (unsigned int)physAddr); //Buffer 1    63- 32
                }

                m_iEncBuf1Ready = TRUE;
                break;

            default:
                // Error
                break;
        }

        dumpIpuRegisters(P_IPU_REGS);

        LeaveCriticalSection(&m_csEncStopping);
    }

    PRP_FUNCTION_EXIT();

    return;
}

//------------------------------------------------------------------------------
//
// Function: PrpEncRotBufferWorkerThread
//
// This function is the worker thread for the handling buffers.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpEncRotBufferWorkerThread(LPVOID lpParameter)
{
    PrpClass *pPrp = (PrpClass *)lpParameter;

    PRP_FUNCTION_ENTRY();

    pPrp->PrpEncRotBufferWorkerRoutine(INFINITE);

    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PrpEncRotBufferWorkerRoutine
//
// This function is the worker thread routine that assures that
// the buffers are managed correctly in the preprocessor.  When
// a buffer is filled, this function will wait for a new buffer to
// become available and set up the channel to run.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpEncRotBufferWorkerRoutine(UINT32 timeout)
{
    UINT32* physAddr;
    DWORD dwFrameDropped;
    DWORD result;

    PRP_FUNCTION_ENTRY();

    while (1)
    {
        if ( WaitForSingleObject ( m_hExitPrpEncRotThread, 0 ) != WAIT_TIMEOUT )
        {
            RETAILMSG(1,(TEXT("PrpEncRotBufferWorkerRoutine for ExitThread")));
            ExitThread(1);
        }
        
        // Wait until the ISR loop receives a filled buffer, at which
        // time it will signal that it needs a new buffer.
        result = WaitForMultipleObjects(2, m_hEncRotBufWaitList, FALSE, timeout);

        // Thread running properly, so we can set this to FALSE;
        m_bEncRotRestartBufferLoop = FALSE;

        DEBUGMSG(ZONE_DEVICE, (_T("%s: Enc buffer requested\r\n"), __WFUNCTION__));

        // A buffer has been requested.  Attempt to read from the ready
        // queue to get an available buffer.  If one is not available,
        // we wait here until it is.
        dwFrameDropped = pEncRotBufferManager->SetActiveBuffer(&physAddr, INFINITE);
        if (dwFrameDropped == 1)
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Frame Dropped.\r\n"), __WFUNCTION__));
            // m_dwFramesDropped++;
        }
        else if (dwFrameDropped == -1)
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Error, did not receive a buffer.\r\n"), __WFUNCTION__));
        }

        // This code ensures that if we do not run into problems
        // if the Enc channel was stopped at an earlier time while
        // this thread was waiting in SetActiveBuffer.  If so,
        // we simply start over at the top of the loop.
        if (m_bEncRotRestartBufferLoop)
        {
            pEncRotBufferManager->ResetBuffers();
            continue;
        }

        DEBUGMSG(ZONE_DEVICE, (_T("%s: Enc rotation buffer ready\r\n"), __WFUNCTION__));

        // Critical section to prevent race condition upon
        // stopping the encoding channel
        EnterCriticalSection(&m_csEncStopping);

        // If we are stopping the encoding channel, return 
        // active buffer to ready queue, and bail out
        // of buffer request routine, as we do not want to
        // continue processing in this channel
        if (!m_bEncRunning)
        {
            pEncRotBufferManager->ResetBuffers(); 
            LeaveCriticalSection(&m_csEncStopping);             
            continue;
        }

        DEBUGMSG(ZONE_DEVICE,
            (TEXT("%s: Physical address: %x.\r\n"), __WFUNCTION__, physAddr));

        switch(result)
        {
            // We have a buffer now.  Determine which buffer we need 
            // to fill (buf0 or buf1)
            case (WAIT_OBJECT_0 + prpBuf0RequestEvent):
                // Program buffer into task parameter memory
                controlledWriteDMAChannelParam(8, 1, 0, (unsigned int)physAddr); //Buffer 0    31-0
                break;
            case (WAIT_OBJECT_0 + prpBuf1RequestEvent):
                // Program buffer into task parameter memory
                controlledWriteDMAChannelParam(8, 1, 1, (unsigned int)physAddr); //Buffer 1    63- 32
                break;
            default:
                // Error
                break;
        }

        LeaveCriticalSection(&m_csEncStopping);
    }

    PRP_FUNCTION_EXIT();

    return;
}


//------------------------------------------------------------------------------
//
// Function: PrpVfBufferWorkerThread
//
// This function is the worker thread for the handling buffers.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpVfBufferWorkerThread(LPVOID lpParameter)
{
    PrpClass *pPrp = (PrpClass *)lpParameter;

    PRP_FUNCTION_ENTRY();

    pPrp->PrpVfBufferWorkerRoutine(INFINITE);

    PRP_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: PrpVfBufferWorkerRoutine
//
// This function is the worker thread routine that assures that
// the buffers are managed correctly in the preprocessor.  When
// a buffer is filled, this function will wait for a new buffer to
// become available and set up the channel to run.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpVfBufferWorkerRoutine(UINT32 timeout)
{
    UINT32* physAddr;
    DISPLAY_BUFFER dispBufferData;
    DWORD dwFrameDropped;
    DWORD result;

    PRP_FUNCTION_ENTRY();

    while (1)
    {
        if ( WaitForSingleObject ( m_hExitPrpVfThread, 0 ) != WAIT_TIMEOUT )
        {
            RETAILMSG(1,(TEXT("PrpVfBufferWorkerRoutine for ExitThread")));
            ExitThread(1);
        }
        
        // Wait until the ISR loop receives a filled buffer, at which
        // time it will signal that it needs a new buffer.
        result = WaitForMultipleObjects(2, m_hVfBufWaitList, FALSE, timeout);

        // Thread running properly, so we can set this to FALSE;
        m_bVfRestartBufferLoop = FALSE;

        DEBUGMSG(ZONE_DEVICE, (_T("%s: Vf buffer requested\r\n"), __WFUNCTION__));

        pVfBufferManager->PrintBufferInfo();

        // A buffer has been requested.  Attempt to read from the ready
        // queue to get an available buffer.  If one is not available,
        // we wait here until it is.
        dwFrameDropped = pVfBufferManager->SetActiveBuffer(&physAddr, INFINITE);
        if (dwFrameDropped == 1)
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Frame Dropped.\r\n"), __WFUNCTION__));
            // m_dwFramesDropped++;
        }
        else if (dwFrameDropped == -1)
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Error, did not receive a buffer.\r\n"), __WFUNCTION__));
        }

        // This code ensures that if we do not run into problems
        // if the VF channel was stopped at an earlier time while
        // this thread was waiting in SetActiveBuffer.  If so,
        // we simply start over at the top of the loop.
        if (m_bVfRestartBufferLoop)
        {
            pVfBufferManager->ResetBuffers();
            continue;
        }

        pVfBufferManager->PrintBufferInfo();

        // Critical section to prevent race condition upon
        // stopping the viewfinding channel
        EnterCriticalSection(&m_csVfStopping);

        // If we are stopping the viewfinding channel, return 
        // active buffer to ready queue, and bail out
        // of buffer request routine, as we do not want to
        // continue processing in this channel
        if (!m_bVfRunning)
        {
            pVfBufferManager->ResetBuffers();
            LeaveCriticalSection(&m_csVfStopping);
            continue;
        }

        switch(result)
        {
            // We have a buffer now.  Determine which buffer we need 
            // to fill (buf0 or buf1)
            case (WAIT_OBJECT_0 + prpBuf0RequestEvent):

                RETAILMSG(0,
                    (TEXT("%s:Buf 0 Physical address: %x.\r\n"), __WFUNCTION__, physAddr));

                // Program buffer into task parameter memory
                controlledWriteDMAChannelParam(1, 1, 0, (unsigned int)physAddr); //Buffer 0    31-0

                // Buffer 0 ready for IDMAC Channel 1 (IC->Mem for viewfinding)
                SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DMA_CHA_READY));

                if (m_bVfFlipRot)
                {
                    // Program buffer into task parameter memory
                    controlledWriteDMAChannelParam(11, 1, 0, (unsigned int)physAddr); //Buffer 0    31-0
                }
                else if (m_bVfDirectDisplay)
                {
                    // If no rotation and direct display, pass the address requested 
                    // to the display for viewfinding.
                    dispBufferData.eSrcBuf = eBUF_0;
                    dispBufferData.paBuf.QuadPart = (LONGLONG) physAddr;

                    // Enqueue newly created buffer
                    if (!WriteMsgQueue(m_hWriteVfBufferQueue, &dispBufferData, sizeof(DISPLAY_BUFFER), 0, 0))
                    {
                        DEBUGMSG(ZONE_ERROR, 
                            (TEXT("%s : Could not write to VF buffer queue.  Error!\r\n"), __WFUNCTION__));
                        LeaveCriticalSection(&m_csVfStopping);
                        continue;
                    }
                }

                m_iVfBuf0Ready = TRUE;

                break;

            case (WAIT_OBJECT_0 + prpBuf1RequestEvent):
                
                RETAILMSG(0,
                    (TEXT("%s:Buf 1 Physical address: %x.\r\n"), __WFUNCTION__, physAddr));

                // Program buffer into task parameter memory
                controlledWriteDMAChannelParam(1, 1, 1, (unsigned int)physAddr); //Buffer 1    63- 32

                // Buffer 1 ready for IDMAC Channel 1 (IC->Mem for viewfinding)
                SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_1, IPU_DMA_CHA_READY));

                if (m_bVfFlipRot)
                {
                    // Program buffer into task parameter memory
                    controlledWriteDMAChannelParam(11, 1, 1, (unsigned int)physAddr); //Buffer 1    63- 32
                }
                else if(m_bVfDirectDisplay)
                {
                    // If no rotation and direct display, pass the address requested 
                    // to the display for viewfinding.
                    dispBufferData.eSrcBuf = eBUF_1;
                    dispBufferData.paBuf.QuadPart = (LONGLONG) physAddr;

                    // Enqueue newly created buffer
                    if (!WriteMsgQueue(m_hWriteVfBufferQueue, &dispBufferData, sizeof(DISPLAY_BUFFER), 0, 0))
                    {
                        DEBUGMSG(ZONE_ERROR, 
                            (TEXT("%s : Could not write to VF buffer queue.  Error!\r\n"), __WFUNCTION__));
                        LeaveCriticalSection(&m_csVfStopping);
                        continue;
                    }
                }

                m_iVfBuf1Ready = TRUE;

                break;
            default:
                // Error
                break;
        }
//        dumpIpuRegisters(P_IPU_REGS);

        LeaveCriticalSection(&m_csVfStopping);
    }

    PRP_FUNCTION_EXIT();

    return;
}

//------------------------------------------------------------------------------
//
// Function: PrpVfRotBufferWorkerThread
//
// This function is the worker thread for the handling buffers.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpVfRotBufferWorkerThread(LPVOID lpParameter)
{
    PrpClass *pPrp = (PrpClass *)lpParameter;

    PRP_FUNCTION_ENTRY();

    pPrp->PrpVfRotBufferWorkerRoutine(INFINITE);

    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PrpVfRotBufferWorkerRoutine
//
// This function is the worker thread routine that assures that
// the buffers are managed correctly in the preprocessor.  When
// a buffer is filled, this function will wait for a new buffer to
// become available and set up the channel to run.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::PrpVfRotBufferWorkerRoutine(UINT32 timeout)
{
    UINT32* physAddr;
    DISPLAY_BUFFER dispBufferData;
    DWORD dwFrameDropped;
    DWORD result;

    PRP_FUNCTION_ENTRY();

    while (1)
    {
        if ( WaitForSingleObject ( m_hExitPrpVfRotThread, 0 ) != WAIT_TIMEOUT )
        {
            RETAILMSG(1,(TEXT("PrpVfRotBufferWorkerRoutine for ExitThread")));
            ExitThread(1);
        }
        
        // Wait until the ISR loop receives a filled buffer, at which
        // time it will signal that it needs a new buffer.
        result = WaitForMultipleObjects(2, m_hVfRotBufWaitList, FALSE, timeout);

        // Thread running properly, so we can set this to FALSE;
        m_bVfRotRestartBufferLoop = FALSE;

        DEBUGMSG(ZONE_DEVICE, (_T("%s: Vf buffer requested\r\n"), __WFUNCTION__));

        // A buffer has been requested.  Attempt to read from the ready
        // queue to get an available buffer.  If one is not available,
        // we wait here until it is.
        dwFrameDropped = pVfRotBufferManager->SetActiveBuffer(&physAddr, INFINITE);
        if (dwFrameDropped == 1)
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Frame Dropped.\r\n"), __WFUNCTION__));
            // m_dwFramesDropped++;
        }
        else if (dwFrameDropped == -1)
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Error, did not receive a buffer.\r\n"), __WFUNCTION__));
        }

        // This code ensures that if we do not run into problems
        // if the VF channel was stopped at an earlier time while
        // this thread was waiting in SetActiveBuffer.  If so,
        // we simply start over at the top of the loop.
        if (m_bVfRotRestartBufferLoop)
        {
            pVfRotBufferManager->ResetBuffers();
            continue;
        }

        pVfRotBufferManager->PrintBufferInfo();

        DEBUGMSG(ZONE_DEVICE, (_T("%s: Vf rotation buffer ready\r\n"), __WFUNCTION__));

        // Critical section to prevent race condition upon
        // stopping the viewfinding channel
        EnterCriticalSection(&m_csVfStopping);

        // If we are stopping the viewfinding channel, return 
        // active buffer to ready queue, and bail out
        // of buffer request routine, as we do not want to
        // continue processing in this channel
        if (!m_bVfRunning)
        {
            pVfRotBufferManager->ResetBuffers();
            LeaveCriticalSection(&m_csVfStopping);
            continue;
        }

        DEBUGMSG(ZONE_DEVICE,
            (TEXT("%s: Physical address: %x.\r\n"), __WFUNCTION__, physAddr));

        switch(result)
        {
            // We have a buffer now.  Determine which buffer we need 
            // to fill (buf0 or buf1)
            case (WAIT_OBJECT_0 + prpBuf0RequestEvent):
                RETAILMSG(0,
                    (TEXT("%s: Buf 0 Physical address: %x.\r\n"), __WFUNCTION__, physAddr));
                
                // Program buffer into task parameter memory
                controlledWriteDMAChannelParam(9, 1, 0, (unsigned int)physAddr); //Buffer 0    31-0

                if (m_bVfDirectDisplay)
                {
                    // If no rotation and direct display, pass the address requested 
                    // to the display for viewfinding.
                    dispBufferData.eSrcBuf = eBUF_0;
                    dispBufferData.paBuf.QuadPart = (LONGLONG) physAddr;

                    // Enqueue newly created buffer
                    if (!WriteMsgQueue(m_hWriteVfBufferQueue, &dispBufferData, sizeof(DISPLAY_BUFFER), 0, 0))
                    {
                        DEBUGMSG(ZONE_ERROR, 
                            (TEXT("%s : Could not write to VF buffer queue.  Error!\r\n"), __WFUNCTION__));
                        LeaveCriticalSection(&m_csVfStopping);
                        continue;
                    }
                }
                break;

            case (WAIT_OBJECT_0 + prpBuf1RequestEvent):
                RETAILMSG(0,
                    (TEXT("%s: Buf 1 Physical address: %x.\r\n"), __WFUNCTION__, physAddr));

                // Program buffer into task parameter memory
                controlledWriteDMAChannelParam(9, 1, 1, (unsigned int)physAddr); //Buffer 1    63- 32

                if (m_bVfDirectDisplay)
                {
                    // If no rotation and direct display, pass the address requested 
                    // to the display for viewfinding.
                    dispBufferData.eSrcBuf = eBUF_1;
                    dispBufferData.paBuf.QuadPart = (LONGLONG) physAddr;

                    // Enqueue newly created buffer
                    if (!WriteMsgQueue(m_hWriteVfBufferQueue, &dispBufferData, sizeof(DISPLAY_BUFFER), 0, 0))
                    {
                        DEBUGMSG(ZONE_ERROR, 
                            (TEXT("%s : Could not write to VF buffer queue.  Error!\r\n"), __WFUNCTION__));
                        LeaveCriticalSection(&m_csVfStopping);
                        continue;
                    }
                }
                break;

            default:
                // Error
                break;
        }

        LeaveCriticalSection(&m_csVfStopping);
    }

    PRP_FUNCTION_EXIT();

    return;
}


//------------------------------------------------------------------------------
//
// Function: CsiTestPatternOn
//
// Turns on CSI test pattern
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PrpClass::CsiTestPatternOn(PCSP_IPU_REGS pIPU)
{
    BYTE red, blue, green;

    // Set up test pattern in CSI
    red = 0x00;
    blue = 0x00;
    green = 0xFF;

    DEBUGMSG(ZONE_ERROR, (TEXT("%s: red blue green = 0x%x\r\n"),
        __WFUNCTION__, (red | (blue << 8) | (green << 16))));

    // Set test pattern in CSI
    OUTREG32(&pIPU->CSI_TST_CTRL, (red | (blue << 8) | (green << 16)));

    // Turn on test pattern generation from the CSI
    INSREG32BF(&P_IPU_REGS->CSI_TST_CTRL, IPU_CSI_TST_CTRL_TEST_GEN_MODE,
        IPU_CSI_TST_CTRL_TEST_GEN_MODE_ACTIVE);

    // CSI->MEM in the IC
//    INSREG32BF(&P_IPU_REGS->IC_CONF, IPU_IC_CONF_CSI_MEM_WR_EN,
//        IPU_IC_CONF_CSI_MEM_WR_EN_ENABLE);
}

void PrpClass::PrpReadInterruptRegisters( )
{
    DWORD dwStat1,dwStat2,dwStat3,dwStat4,dwStat5;
    dwStat1 = INREG32(&P_IPU_REGS->IPU_INT_STAT_1);
    dwStat2 = INREG32(&P_IPU_REGS->IPU_INT_STAT_2);
    dwStat3 = INREG32(&P_IPU_REGS->IPU_INT_STAT_3);
    dwStat4 = INREG32(&P_IPU_REGS->IPU_INT_STAT_4);
    dwStat5 = INREG32(&P_IPU_REGS->IPU_INT_STAT_5);
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_1: %x\r\n"), __WFUNCTION__, dwStat1));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_2: %x\r\n"), __WFUNCTION__, dwStat2));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_3: %x\r\n"), __WFUNCTION__, dwStat3));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_4: %x\r\n"), __WFUNCTION__, dwStat4));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_5: %x\r\n"), __WFUNCTION__, dwStat5)); 
    // Clear Interrupt Status Bits
    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_1, dwStat1);
    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_2, dwStat2);
    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_3, dwStat3);
    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_4, dwStat4);
    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_5, dwStat5);
}

void PrpClass::PrpReadIPURegisters( )
{
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CONF: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_BUF0_RDY: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_BUF1_RDY: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_DB_MODE_SEL: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_CUR_BUF: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_CHA_CUR_BUF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_FS_PROC_FLOW: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_FS_PROC_FLOW)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_FS_DISP_FLOW: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_FS_DISP_FLOW)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_TASKS_STAT: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_TASKS_STAT)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_CTRL_1: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_INT_CTRL_1)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_CTRL_2: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_INT_CTRL_2)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_CTRL_3: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_INT_CTRL_3)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_CTRL_4: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_INT_CTRL_4)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_CTRL_5: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_INT_CTRL_5)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: SDC_COM_CONF: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->SDC_COM_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: SDC_FG_POS: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->SDC_FG_POS)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IC_CONF: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IC_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: CSI_SENS_CONF: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->CSI_SENS_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: CSI_SENS_FRM_SIZE: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->CSI_SENS_FRM_SIZE)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IDMAC_CHA_BUSY: %x\r\n"), __WFUNCTION__, INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY)));    
}

void PrpClass::PrpReadVfDMA( )
{
    ReadVfDMA(P_IPU_REGS);
}

//------------------------------------------------------------------------------
//
// Function: dumpChannelParams
//
//
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
static void dumpChannelParams(pPrpIDMACChannelParams pChannelParams)
{
    DEBUGMSG (ZONE_DEVICE, (TEXT("bInterleaved= %s\r\n"), pChannelParams->bInterleaved ? "TRUE" : "FALSE"));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Width       = %x\r\n"), pChannelParams->iWidth));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Height      = %x\r\n"), pChannelParams->iHeight));
    DEBUGMSG (ZONE_DEVICE, (TEXT("BPP Code    = %x\r\n"), pChannelParams->iBitsPerPixelCode));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Line Stride = %x\r\n"), pChannelParams->iLineStride));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Format Code = %x\r\n"), pChannelParams->iFormatCode));
    DEBUGMSG (ZONE_DEVICE, (TEXT("BAM         = %x\r\n"), pChannelParams->iBAM));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Pixel Burst = %x\r\n"), pChannelParams->iPixelBurstCode));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixWidth0   = %x\r\n"), pChannelParams->pixelFormat.component0_width));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixWidth1   = %x\r\n"), pChannelParams->pixelFormat.component1_width));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixWidth2   = %x\r\n"), pChannelParams->pixelFormat.component2_width));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixOffset0  = %x\r\n"), pChannelParams->pixelFormat.component0_offset));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixOffset1  = %x\r\n"), pChannelParams->pixelFormat.component1_offset));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixOffset2  = %x\r\n"), pChannelParams->pixelFormat.component2_offset));
    return;
}

//------------------------------------------------------------------------------
//
// Function: dumpCoeffs
//
//
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
static void dumpCoeffs(pPrpCSCCoeffs pCSCCoeffs)
{
    DEBUGMSG (ZONE_DEVICE, (TEXT("C00: %x\r\n"), pCSCCoeffs->C00));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C01: %x\r\n"), pCSCCoeffs->C01));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C02: %x\r\n"), pCSCCoeffs->C02));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C10: %x\r\n"), pCSCCoeffs->C10));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C11: %x\r\n"), pCSCCoeffs->C11));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C12: %x\r\n"), pCSCCoeffs->C12));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C20: %x\r\n"), pCSCCoeffs->C20));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C21: %x\r\n"), pCSCCoeffs->C21));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C22: %x\r\n"), pCSCCoeffs->C22));
    DEBUGMSG (ZONE_DEVICE, (TEXT("A0: %x\r\n"), pCSCCoeffs->A0));
    DEBUGMSG (ZONE_DEVICE, (TEXT("A1: %x\r\n"), pCSCCoeffs->A1));
    DEBUGMSG (ZONE_DEVICE, (TEXT("A2: %x\r\n"), pCSCCoeffs->A2));
    return;
}


//------------------------------------------------------------------------------
//
// Function: dumpInterruptRegisters
//
//
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
static void dumpInterruptRegisters(PCSP_IPU_REGS pIPU)
{
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_1: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_1)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_2: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_2)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_3: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_3)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_4: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_4)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_5: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_5)));
    return;
}

//------------------------------------------------------------------------------
//
// Function: dumpIpuRegisters
//
//
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
static void dumpIpuRegisters(PCSP_IPU_REGS pIPU)
{
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_BUF0_RDY: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CHA_BUF0_RDY)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_BUF1_RDY: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CHA_BUF1_RDY)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_DB_MODE_SEL: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CHA_DB_MODE_SEL)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_CUR_BUF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CHA_CUR_BUF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_FS_PROC_FLOW: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_FS_PROC_FLOW)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_FS_DISP_FLOW: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_FS_DISP_FLOW)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_TASKS_STAT: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_TASKS_STAT)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_CTRL_1: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_CTRL_1)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_1: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_1)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_2: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_2)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_3: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_3)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_5: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_5)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: SDC_COM_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->SDC_COM_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: SDC_FG_POS: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->SDC_FG_POS)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IC_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IC_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: CSI_SENS_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->CSI_SENS_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: CSI_SENS_FRM_SIZE: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->CSI_SENS_FRM_SIZE)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IDMAC_CHA_BUSY: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IDMAC_CHA_BUSY)));
}

static void ReadVfDMA(PCSP_IPU_REGS pIPU)
{
    int i;
    UINT32 data;

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.

    while (1)
    {
        if (INREG32(&pIPU->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if (InterlockedTestExchange((LPLONG)&pIPU->IPU_IMA_ADDR, 0, 1) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    OUTREG32(&pIPU->IPU_IMA_ADDR,
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_MEM_NU, IPU_IMA_ADDR_MEM_NU_CPM) |
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_ROW_NU, 2)|
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_WORD_NU, 0));

    for (i = 0; i < 132; i += 32)
    {
        data = INREG32(&pIPU->IPU_IMA_DATA);
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s(): Word0, bits %d - %d: %x\r\n"), __WFUNCTION__, i, i+32, data));
        RETAILMSG(1,
                 (TEXT("%s(): Word0, bits %d - %d: %x\r\n"), __WFUNCTION__, i, i+32, data));
    }

    OUTREG32(&pIPU->IPU_IMA_ADDR,
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_MEM_NU, IPU_IMA_ADDR_MEM_NU_CPM) |
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_ROW_NU, 3)|
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_WORD_NU, 0));

    for (i = 0; i < 132; i += 32)
    {
        data = INREG32(&pIPU->IPU_IMA_DATA);
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s(): Word1, bits %d - %d: %x\r\n"), __WFUNCTION__, i, i+32, data));
        RETAILMSG(1,
                 (TEXT("%s(): Word1, bits %d - %d: %x\r\n"), __WFUNCTION__, i, i+32, data));
    }

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&pIPU->IPU_IMA_ADDR, 0);
}


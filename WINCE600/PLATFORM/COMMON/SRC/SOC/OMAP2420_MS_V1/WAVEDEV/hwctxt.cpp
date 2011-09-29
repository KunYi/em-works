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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//

#include "wavemain.h"
#include "nkintr.h"

//------------------------------------------------------------------------------

HardwareContext *g_pHWContext=NULL;

//------------------------------------------------------------------------------


static void CallInterruptThreadTx(HardwareContext *pHWContext);
static void CallInterruptThreadRx(HardwareContext *pHWContext);
static void CallTimeoutThread(HardwareContext *pHWContext);

//#undef DEBUGMSG
//#define DEBUGMSG(x, y) RETAILMSG(1,y)

//------------------------------------------------------------------------------
//
//  Function: HardwareContext::HardwareContext()
//  
//  HardwareContext constructor. 
//

HardwareContext::HardwareContext()
: m_InputDeviceContext(), m_OutputDeviceContext()
{
    InitializeCriticalSection(&m_Lock);
    m_fTerminating = FALSE;
    m_Initialized=FALSE;

    m_hParent = NULL;
    m_CurPowerState = PwrDeviceUnspecified ;

    m_hAudioInterruptThreadRx = NULL;
    m_hAudioInterruptThreadTx = NULL;
    m_hTimeoutThread = NULL;
    
    m_fRequestedSysIntr = FALSE;
    m_IntrAudioRx = m_IntrAudioTx = SYSINTR_UNDEFINED ;
    m_fRxInterruptIntialized = m_fTxInterruptIntialized = FALSE;

    m_hTimeoutEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    m_hAudioInterruptTx = CreateEvent( NULL, FALSE, FALSE, NULL);
    m_hAudioInterruptRx = CreateEvent( NULL, FALSE, FALSE, NULL);
    
}

//------------------------------------------------------------------------------
//
//  Function: HardwareContext::~HardwareContext()
//  
//  destructor
//

HardwareContext::~HardwareContext()
{
    m_fTerminating = TRUE;

    if (m_hTimeoutEvent!=NULL)
        SetEvent(m_hTimeoutEvent);
    if (m_hAudioInterruptTx!=NULL)
        SetEvent(m_hAudioInterruptTx);
    if (m_hAudioInterruptRx)
        SetEvent(m_hAudioInterruptRx);
    
    Sleep(500); // Sleep 500 ticks to allow all thread terminated.
    TerminateCloseThread(m_hAudioInterruptThreadTx);
    TerminateCloseThread(m_hAudioInterruptThreadRx);
    TerminateCloseThread(m_hTimeoutThread);
    
    if (m_fRxInterruptIntialized)
        InterruptDisable(m_IntrAudioRx);  
    if (m_fTxInterruptIntialized)
        InterruptDisable(m_IntrAudioTx);   
    
    if (m_IntrAudioTx != SYSINTR_UNDEFINED && m_fRequestedSysIntr) {
        KernelIoControl( IOCTL_HAL_RELEASE_SYSINTR,&m_IntrAudioTx, sizeof( m_IntrAudioTx ), NULL,  0,  NULL );
    }
    if (m_IntrAudioRx != SYSINTR_UNDEFINED && m_fRequestedSysIntr) {
        KernelIoControl( IOCTL_HAL_RELEASE_SYSINTR,&m_IntrAudioRx, sizeof( m_IntrAudioRx ), NULL,  0,  NULL );
    }
    if (m_hTimeoutEvent)
        CloseHandle(m_hTimeoutEvent);

    if (m_hParent ) {
        SetDevicePowerState(m_hParent, D4 , NULL);
        CloseBusAccessHandle (m_hParent ) ;
    }
    DeleteCriticalSection(&m_Lock);
}
void HardwareContext::TerminateCloseThread(HANDLE hThread)
{
    if (hThread) {
        if( ::WaitForSingleObject( hThread, 0 ) != WAIT_OBJECT_0 ) // Can't terminate. Force it.
            ::TerminateThread( hThread, ( DWORD ) - 1 ) ;
        ::CloseHandle( hThread );
    }
}

//------------------------------------------------------------------------------
//
//  Function: Init
//  
//  map registers, allocate DMA buffers, set hardware to known state
//

BOOL 
HardwareContext::Init(DWORD Index)
{
    DEBUGMSG(ZONE_AC,(TEXT("WAVEDEV: HardwareContext::Init Index=0x%x\r\n"), Index));

    if (m_Initialized)
    {
        return FALSE;
    }

    m_hParent           = CreateBusAccessHandle((LPCTSTR) Index) ;

    m_DriverIndex       = Index;
    m_InPowerHandler    = FALSE;
    m_InputDMARunning   = FALSE;
    m_OutputDMARunning  = FALSE;
    m_NumForcedSpeaker  = 0;

    m_bDMARunning       = FALSE;
    m_bBtHeadsetSelected= FALSE;
    m_dwBtAudioRouting  = BT_AUDIO_MODEM;
    m_bHeadsetPluggedIn = FALSE;    
    m_bToggleLoadSpeaker= FALSE;   
    m_hRil              = NULL;                 
    
    // Map registers and the DMA buffers into driver's virtual address space
    if (!InitMemories())
    {
        goto Exit;
    }

    // Init codec.
    InitCodec();

    // Configure the DMA controller
    InitDMA();

    // Configure the controller
    InitController();

    if (!InitInterruptThread())
    {
        goto Exit;
    }

    m_Initialized=TRUE;

Exit:
    return m_Initialized;
}

//------------------------------------------------------------------------------
//
//  Function: InitMemories()
//  
//  Map DMA pages into the local address space
//

BOOL 
HardwareContext::InitMemories()
{
    PBYTE pbDMABufIn, pbDMABufOut;

    if (!HWMapControllerRegs ())
    {
        goto ErrExit;
    }

    if (HWMapDMAMemory(AUDIO_DMA_PAGE_SIZE*AUDIO_DMA_PAGES*2))
    {
        pbDMABufIn  = HWDMAMemoryIn(); 
        pbDMABufOut = HWDMAMemoryOut();

        // Save pointers to the virtual addresses so the driver can access them
        for (int i=0; i<AUDIO_DMA_PAGES; i++)
        {
            m_Output_pbDMA_PAGES[i] = pbDMABufOut + AUDIO_DMA_PAGE_SIZE*i;
            m_Input_pbDMA_PAGES[i]  = pbDMABufIn + ((AUDIO_DMA_PAGES+i)*AUDIO_DMA_PAGE_SIZE);
        }

        return TRUE;

    }

ErrExit:

    return FALSE;

}

//------------------------------------------------------------------------------
//
//  Function: InitDMA()
//  
//  set DMA registers to known state
//

void 
HardwareContext::InitDMA()
{
    HWInitInputDMA();
    HWInitOutputDMA();
}

//------------------------------------------------------------------------------
//
//  Function: InitController()
//  
//  
//

void 
HardwareContext::InitController()
{
    HWInitController();
    HWInitNetwork();
}

//------------------------------------------------------------------------------
//
//  Function: InitCodec()
//  
//
//

void 
HardwareContext::InitCodec()
{
    HWInitCodec();
}


//------------------------------------------------------------------------------
//
//  Function: StartOutputDMA()
//  
//  start output DMA on first call and transfer data to DMA buffers
//

void 
HardwareContext::StartOutputDMA()
{
    if (!m_OutputDMARunning)
    {
        // Set output dma to running state in case we get 
        // reentered during TransferOutputBuffer
        m_OutputDMARunning=TRUE;
        
        // Prime the output buffer and turn on DMA if anything got transferred
        ULONG OutputTransferred;
        OutputTransferred = TransferOutputBuffer(0)+TransferOutputBuffer(1);

        // If we didn't transfer any data to the DMA buffers, don't enable DMA
        if (OutputTransferred==0)
        {
            m_OutputDMARunning=FALSE;
            return;
        }

        m_bDMARunning = TRUE;
        
        HWUpdateAudioPRC ();
        HWStartOutputDMA();
    }
}

//------------------------------------------------------------------------------
//
//  Function: StopOutputDMA()
//  
//  stop output DMA and adjust power state
//

void 
HardwareContext::StopOutputDMA()
{
    if (m_OutputDMARunning)
    {
        m_OutputDMARunning=FALSE;
        HWStopOutputDMA();

        if (!m_InputDMARunning && !m_OutputDMARunning)
        {
            // Take care of power management.
            m_bDMARunning = FALSE;
            SetupDelayUpdate ();
        }
    }
}
//------------------------------------------------------------------------------
//
//  Function: StartInputDMA()
//  
//  start wave capture DMA, adjust power state and route audio to modem input
//

void 
HardwareContext::StartInputDMA()
{
    if (!m_InputDMARunning)
    {
#ifdef INPUT_CACHEDMEM
        InputBufferCacheDiscard( m_Input_pbDMA_PAGES[0],AUDIO_DMA_PAGE_SIZE*2);
#endif
        m_InputDMARunning=TRUE;

        m_bDMARunning = TRUE;
        HWUpdateAudioPRC ();
        
        // Turn on the recording path.
        SetRecordMemoPath (TRUE);

        HWStartInputDMA();
    }
}

//------------------------------------------------------------------------------
//
//  Function: StopInputDMA()
//  
//  stop capture DMA and switch off modem input
//

void 
HardwareContext::StopInputDMA()
{
    if (m_InputDMARunning)
    {
        m_InputDMARunning=FALSE;
        
        HWStopInputDMA();

        // Turn off the recording path.
        SetRecordMemoPath (FALSE);

        if (!m_InputDMARunning && !m_OutputDMARunning)
        {
            // Take care of power management.
            m_bDMARunning = FALSE;
            SetupDelayUpdate ();
        }
    }
}

void  HardwareContext::SetupDelayUpdate()
{
    SetEvent(m_hTimeoutEvent);
}
void  HardwareContext::DelayedUpdate()
{
    HWAudioPowerTimeout(TRUE); // Timeout.
}

//------------------------------------------------------------------------------
//
//  Function: GetInterruptThreadPriority()
//  
//  allow registry key to override priority of interrupt threads
//


void  
HardwareContext::GetRegistryValue()
{
    HKEY hDevKey;
    m_dwPriority256 = 210; // Default priority
    m_dwTimeoutTicks = 1000;    // 1 second.
    hDevKey = OpenDeviceKey((LPWSTR)m_DriverIndex);
    if (hDevKey)
    {
        DWORD dwValType;
        DWORD dwValLen;
        dwValLen = sizeof(DWORD);
        RegQueryValueEx(
                       hDevKey,
                       TEXT("Priority256"),
                       NULL,
                       &dwValType,
                       (PUCHAR)&m_dwPriority256,
                       &dwValLen);
        
        dwValLen = sizeof(DWORD);
        RegQueryValueEx(
                       hDevKey,
                       TEXT("SuspendDelayTicks"),
                       NULL,
                       &dwValType,
                       (PUCHAR)&m_dwTimeoutTicks,
                       &dwValLen);
        if (m_dwTimeoutTicks == 0 )
            m_dwTimeoutTicks = INFINITE;
        
        RegCloseKey(hDevKey);
    }

}

//------------------------------------------------------------------------------
//
//  Function: MapIrqToSysIntr()
//  
//  get dynamic mapping of hardware irq to software sysintr
//

DWORD 
HardwareContext::MapIrqToSysIntr(DWORD irq)
{
    DWORD dwSysIntr = SYSINTR_UNDEFINED;
    if (!KernelIoControl(
            IOCTL_HAL_REQUEST_SYSINTR, 
            &irq, 
            sizeof(irq), 
            &dwSysIntr,
            sizeof(dwSysIntr),
            NULL)) 
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("WaveDev: IRQ -> SYSINTR translation failed\r\n")));
        dwSysIntr = SYSINTR_UNDEFINED;
    }

    return dwSysIntr;
}

//------------------------------------------------------------------------------
//
//  Function: InitInterruptThread()
//  
//  start interrupt threads and map hardware interrupts to IST
//

BOOL 
HardwareContext::InitInterruptThread()
{
    DEBUGMSG(ZONE_AC, 
       (TEXT("WaveDev: HardwareContext::InitInterruptThread\r\n")));

    if ( !(m_hTimeoutEvent !=NULL &&  m_hAudioInterruptTx !=NULL &&  m_hAudioInterruptRx !=NULL))
        return FALSE;

    GetRegistryValue();

    m_fRequestedSysIntr    = TRUE;
    m_IntrAudioTx = MapIrqToSysIntr(AUDIO_OUTPUT_DMA_IRQ);
    m_IntrAudioRx = MapIrqToSysIntr(AUDIO_INPUT_DMA_IRQ);
    if (m_IntrAudioTx == SYSINTR_UNDEFINED ||
        m_IntrAudioRx == SYSINTR_UNDEFINED)
    {
        return FALSE;
    }

    m_fRxInterruptIntialized = InterruptInitialize(m_IntrAudioTx, m_hAudioInterruptTx, NULL, 0);
    m_fTxInterruptIntialized  = InterruptInitialize(m_IntrAudioRx, m_hAudioInterruptRx, NULL, 0);
    
    if (!m_fRxInterruptIntialized || !m_fTxInterruptIntialized)
        return FALSE ;


    m_hAudioInterruptThreadTx  = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)CallInterruptThreadTx,
                                            this,
                                            0,
                                            NULL);
    m_hAudioInterruptThreadRx  = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)CallInterruptThreadRx,
                                            this,
                                            0,
                                            NULL);
    m_hTimeoutThread = CreateThread ((LPSECURITY_ATTRIBUTES)NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)CallTimeoutThread,
                                            this,
                                            0,
                                            NULL);
    return (m_hAudioInterruptThreadTx!=NULL && m_hAudioInterruptThreadRx!=NULL && m_hTimeoutThread!=NULL) ;
}

//------------------------------------------------------------------------------
//
//  Function: Deinit()
//  
//  
//

BOOL 
HardwareContext::Deinit()
{
    return HWDeinit();
}

//------------------------------------------------------------------------------
//
//  Function: TransferInputBuffer()
//  
//
//

ULONG 
HardwareContext::TransferInputBuffer(ULONG NumBuf)
{
    ULONG BytesTransferred;
    PBYTE pBufferStart = m_Input_pbDMA_PAGES[NumBuf];
    PBYTE pBufferEnd = pBufferStart + AUDIO_DMA_PAGE_SIZE;
    PBYTE pBufferLast;
    DWORD NumStreams;

#ifdef PROFILE_MIXER
    LARGE_INTEGER liPCStart, liPCStop;
    QueryPerformanceCounter(&liPCStart);
#endif

    pBufferLast = m_InputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd, &NumStreams);
    BytesTransferred = pBufferLast-pBufferStart;

#ifdef INPUT_CACHEDMEM
    InputBufferCacheDiscard( pBufferStart, AUDIO_DMA_PAGE_SIZE);
#endif

#ifdef PROFILE_MIXER 
    QueryPerformanceCounter(&liPCStop);
    m_InputDeviceContext.m_liPCTotal.QuadPart += liPCStop.QuadPart-liPCStart.QuadPart;
#endif

    DEBUGMSG(ZONE_DMA, (TEXT("In(%d) %d bytes, str %d\r\n"),NumBuf, BytesTransferred, NumStreams));
#if 0
	DWORD i;
	PWORD pwData, pwLast;
	RETAILMSG(1, (L"-------------------------------Input----------------------------------\r\n"));
	pwData = (PWORD)pBufferStart;
	pwLast = (PWORD)pBufferLast;
	while (pwData < pwLast)
	{
		RETAILMSG(1, (L"\t"));
		for (i = 0; i < 16 && pwData < pwLast; i++, pwData++)
		{
			RETAILMSG(1, (L"0x%04X, ", *pwData));
		}
		RETAILMSG(1, (L"\r\n"));
	}
	RETAILMSG(1, (L"-------------------------------Input----------------------------------\r\n"));
#endif

    return BytesTransferred;
}

//------------------------------------------------------------------------------
//
//  Function: HandleSaturation()
//  
//  check audio sample buffer for saturation conditions
//
#if USE_HW_SATURATE
void 
HardwareContext::HandleSaturation(PBYTE pBuffer, PBYTE pBufferEnd)
{
    for (;pBuffer<pBufferEnd;pBuffer+=sizeof(HWSAMPLE))
    {
        INT32 CurrSamp;

        CurrSamp = *(HWSAMPLE *)pBuffer;

        // Handle saturation
        if (CurrSamp>AUDIO_SAMPLE_MAX)
        {
            *(HWSAMPLE *)pBuffer = AUDIO_SAMPLE_MAX;
        }
        if (CurrSamp<AUDIO_SAMPLE_MIN)
        {
            *(HWSAMPLE *)pBuffer = AUDIO_SAMPLE_MIN;
        }
    }
}
#endif

//------------------------------------------------------------------------------
//
//  Function: TransferOutputBuffer()
//  
//  copy and reformat wave data for output through DMA
//

ULONG 
HardwareContext::TransferOutputBuffer(ULONG NumBuf)
{
    ULONG BytesTransferred;
    PBYTE pBufferStart = m_Output_pbDMA_PAGES[NumBuf];
    PBYTE pBufferEnd = pBufferStart + AUDIO_DMA_PAGE_SIZE;
    PBYTE pBufferLast;
    DWORD NumStreams;

	DEBUGMSG(ZONE_DMA, (TEXT("TransferOutputBuffer: NumBuf=%d\r\n"), NumBuf));
#if 0
	DWORD i;
	PWORD pwData, pwLast;
	RETAILMSG(1, (L"-------------------------------Output---------------------------------\r\n"));
	pwData = (PWORD)pBufferStart;
	pwLast = (PWORD)(pBufferStart + AUDIO_DMA_PAGE_SIZE);
	while (pwData < pwLast)
	{
		RETAILMSG(1, (L"\t"));
		for (i = 0; i < 16 && pwData < pwLast; i++, pwData++)
		{
			RETAILMSG(1, (L"0x%04X, ", *pwData));
		}
		RETAILMSG(1, (L"\r\n"));
	}
	RETAILMSG(1, (L"-------------------------------Output---------------------------------\r\n"));
#endif

#ifdef PROFILE_MIXER
    LARGE_INTEGER liPCStart, liPCStop;
    QueryPerformanceCounter(&liPCStart);
#endif

    pBufferLast = m_OutputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd, &NumStreams);
#if USE_HW_SATURATE
    // If not using mix saturation, take care saturation here.
    // If more than 1 stream rendered, guard against saturation.
    if (NumStreams>1)
    {
        HandleSaturation(pBufferStart, pBufferLast);
    }
#endif
    // Clear the rest of the buffer
    StreamContext::ClearBuffer(pBufferLast,pBufferEnd);

    BytesTransferred = pBufferLast-pBufferStart;

#ifdef MIXER_CACHEDMEM
    OutputBufferCacheFlush( pBufferStart, AUDIO_DMA_PAGE_SIZE);
#endif

#ifdef PROFILE_MIXER 
    QueryPerformanceCounter(&liPCStop);
    m_OutputDeviceContext.m_liPCTotal.QuadPart += liPCStop.QuadPart-liPCStart.QuadPart;
#endif

    DEBUGMSG(ZONE_DMA, (TEXT("Out(%d) %d bytes str %d\r\n"),NumBuf, BytesTransferred, NumStreams));

    return BytesTransferred;
}

//------------------------------------------------------------------------------
//
//  Function: InterruptThreadTx()
//  
//  play DMA IST
//

void 
HardwareContext::InterruptThreadTx()
{
    DEBUGMSG(ZONE_AC, 
       (TEXT("WaveDev: HardwareContext::InterruptThreadTx\r\n")));

    // make sure that GWES APIs ready before calling: 
    if (WAIT_OBJECT_0 != WaitForAPIReady(SH_GDI, INFINITE))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Wavedev driver: WaitForAPIReady failed.\r\n")));
        return;
    }

    // Bump up the priority since the interrupt must be serviced immediately.
    CeSetThreadPriority(GetCurrentThread(),  m_dwPriority256);

    DEBUGMSG(ZONE_AC, 
       (TEXT("WaveDev: HardwareContext::InterruptThreadTx Enable Interrupt!!!\r\n")));
    while (!m_fTerminating)
    {
        DWORD dwWaitObject;

        InterruptDone(m_IntrAudioTx);

        dwWaitObject = WaitForSingleObject(m_hAudioInterruptTx, INFINITE);

        DEBUGMSG(ZONE_IRQ,(TEXT("WAVE: Tx interrupt\r\n")));

        // Grab the lock
        Lock();

        // Copy data to DMA buffers
        HWTransferOutputBuffers();

        Unlock();
    }
}

//------------------------------------------------------------------------------
//
//  Function: InterruptThreadRx()
//  
//  capture DMA IST
//

void 
HardwareContext::InterruptThreadRx()
{
    DEBUGMSG(ZONE_AC, 
       (TEXT("WaveDev: HardwareContext::InterruptThreadRx\r\n")));

    // make sure that GWES APIs ready before calling: 
    if (WAIT_OBJECT_0 != WaitForAPIReady(SH_GDI, INFINITE))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Wavedev driver: WaitForAPIReady failed.\r\n")));
        return;
    }

    // Bump up the priority since the interrupt must be serviced immediately.
    CeSetThreadPriority(GetCurrentThread(), m_dwPriority256);

    while (!m_fTerminating)
    {
        DWORD dwWaitObject;

        InterruptDone(m_IntrAudioRx);

        dwWaitObject = WaitForSingleObject(m_hAudioInterruptRx, INFINITE);

        DEBUGMSG(ZONE_IRQ,(TEXT("WAVE: Rx interrupt\r\n")));

        // Grab the lock
        Lock();

        // Copy data from DMA buffers
        HWTransferInputBuffers();

        Unlock();
    }
}
void 
HardwareContext::TimeoutThread()
{
    DEBUGMSG(ZONE_AC, 
       (TEXT("WaveDev: HardwareContext::TimeoutThread\r\n")));
    DWORD dwTimeout = m_dwTimeoutTicks;
    while (!m_fTerminating) {
        DWORD dwWaitObject = WaitForSingleObject(m_hTimeoutEvent, dwTimeout);
        if (dwWaitObject == WAIT_TIMEOUT) { // It is timeout. 
			DEBUGMSG(ZONE_AC, 
			   (TEXT("WaveDev: HardwareContext::TimeoutThread Got timeout!\r\n")));
            Lock();
            DelayedUpdate() ;
            Unlock();
            dwTimeout = INFINITE;
        }
        else
            dwTimeout = m_dwTimeoutTicks;
    }
}

//------------------------------------------------------------------------------
//
//  Function:   CallInterruptThreadTx()
//              CallInterruptThreadRx()
//  
//  static helper functions to start IST
//

static void 
CallInterruptThreadTx(HardwareContext *pHWContext)
{
    pHWContext->InterruptThreadTx();
}

static void 
CallInterruptThreadRx(HardwareContext *pHWContext)
{
    pHWContext->InterruptThreadRx();
}
static void 
CallTimeoutThread(HardwareContext *pHWContext)
{
    pHWContext->TimeoutThread();
}

//------------------------------------------------------------------------------
//
//  Function: RecalcSpeakerEnable()
//  
//  RecalcSpeakerEnable decides whether to enable the speaker or not.
//  For now, it only looks at the m_bForceSpeaker variable, but it could
//  also look at whether the headset is plugged in (m_bHeadsetPluggedIn)
//  and/or whether we're in a voice call (m_bInVoiceCall). Some mechanism would
//  need to be implemented to inform the wave driver of changes in the state of
//  these variables however.
//

void 
HardwareContext::RecalcSpeakerEnable()
{
    HWUpdateAudioPRC();
}

//------------------------------------------------------------------------------
//
//  Function: ForceSpeaker()
//  
//  ForceSpeaker is called from the device context to update the state of the
//  m_bForceSpeaker variable by MM_WOM_FORCESPEAKER.  
//

DWORD 
HardwareContext::ForceSpeaker( BOOL bForceSpeaker )
{
    DEBUGMSG( ZONE_AC, (TEXT("WAVE: ForceSpeaker = %d\r\n"), bForceSpeaker));   

    // lock is only taken because of ++/-- operators
    Lock();

    // If m_NumForcedSpeaker is non-zero, audio should be routed to an
    // external speaker (if hw permits).
    if (bForceSpeaker)
    {
        m_NumForcedSpeaker++;
        if (m_NumForcedSpeaker==1)
        {
            RecalcSpeakerEnable();
        }
    }
    else
    {
        if (m_NumForcedSpeaker > 0)
        {
            m_NumForcedSpeaker--;
            if (m_NumForcedSpeaker)
            {
                RecalcSpeakerEnable();
            }
        }
    }   

    Unlock();

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//
//  Function: PowerUp()
//  
//
//

void 
HardwareContext::PowerUp()
{
    HWPowerUp();
}

//------------------------------------------------------------------------------
//
//  Function: PowerDown()
//  
//
//

void 
HardwareContext::PowerDown()
{
    HWPowerDown();
}

//------------------------------------------------------------------------------
//
//  Function: SetExtSpeakerPower()
//  
//  Indicate amplifiers can be powered on/off.
//

BOOL 
HardwareContext::SetExtSpeakerPower( BOOL fOn )
{
    DEBUGMSG( ZONE_ERROR, ( TEXT( "WAVE: SetExtSpeakerPower not implemented!\r\n" )));
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function: PrepareForVoiceCall( BOOL bInVoiceCall )
//  
//  get notice from Ril to get ready for a voice call
//

BOOL 
HardwareContext::PrepareForVoiceCall( BOOL bInVoiceCall )
{
    return HWEnableNetwork( bInVoiceCall );
}

//------------------------------------------------------------------------------
//
//  Function: NotifyHeadsetOn()
//  
//  Route to headset or not.
//
//

VOID 
HardwareContext::NotifyHeadsetOn(BOOL fHeadset)
{
    m_bHeadsetPluggedIn = fHeadset;
    HWUpdateAudioPRC();
}

//------------------------------------------------------------------------------
//
//  Function: ToggleExtSpeaker()
//  
//  Toggle external speaker if available.
//

VOID 
HardwareContext::ToggleExtSpeaker()
{

    m_bToggleLoadSpeaker = TRUE; 
    HWUpdateAudioPRC();
}

//------------------------------------------------------------------------------
//
//  Function: NotifyBtHeadsetOn()
//
//  Route to BT headset or not.
//

VOID 
HardwareContext::NotifyBtHeadsetOn(DWORD dwBtAudioRouting)
{

    m_bBtHeadsetSelected = FALSE;
    m_dwBtAudioRouting = BT_AUDIO_NONE;

    dwBtAudioRouting &= (BT_AUDIO_SYSTEM | BT_AUDIO_MODEM);

    if (dwBtAudioRouting)
    {
        m_bBtHeadsetSelected = TRUE;
        m_dwBtAudioRouting =  dwBtAudioRouting;
    }

    HWUpdateAudioPRC();

}



//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  hwctxt.cpp
//
//  Platform dependent code for the WinCE SPDIF driver. 
//
//   
//-----------------------------------------------------------------------------

#include "wavemain.h"
// Define

//-----------------------------------------------------------------------------
// External Variables

#ifdef DEBUG

extern DBGPARAM dpCurSettings;

#endif // #ifdef DEBUG

extern UINT16 SPDIFDMAPageSize;

//-----------------------------------------------------------------------------
// Defines

const unsigned short HardwareContext::INTR_PRIORITY_REGKEY[12] =
    TEXT("Priority256");


//-----------------------------------------------------------------------------
// Exported Global Variables


//-----------------------------------------------------------------------------
// Local Global Variables

static BOOL g_saveOutputDMARunning = FALSE;
static BOOL g_saveInputDMARunning  = FALSE;
class HardwareContext * HardwareContext::m_pHardwareContext = NULL;


//-----------------------------------------------------------------------------
//
//  Function: CreateHWContext
//
//  This function is invoked by the WAV_Init to create a device instance
//  that will be passed to the WAV_Open and WAV_DeInit functions.  If
//  multiple devices are supported, a unique device instance should be
//  returned for each device.
//
//  Parameters:
//      Index
//          [in] Context parameter passed to the WAV_Init function
//
//  Returns:
//      Returns the instance of HardwareContext.
//
//-----------------------------------------------------------------------------
HardwareContext* HardwareContext::GetHwContext(DWORD Index)
{
    UNREFERENCED_PARAMETER(Index);
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::CreateHWContext\r\n")));

    if (m_pHardwareContext == NULL)
        m_pHardwareContext = new  HardwareContext;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::CreateHWContext\r\n")));

    return m_pHardwareContext;  
}

//-----------------------------------------------------------------------------
//
//  Function: HardwareContext
//
//  This function is the constructor for the hardware context class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
HardwareContext::HardwareContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::HardwareContext\r\n")));

    InitializeCriticalSection(&m_Lock);
    m_bInitialized = FALSE;

    // This flag is only set to TRUE when PowerDown() is called.
    m_bPowerdown = FALSE;

    // The actual interrupt handler thread will be created later by calling
    // CreateThread() in InitInterruptThread().
    m_hInterruptThread = NULL;

    m_hInterrupt = NULL;

    m_pVirtDMABufferAddr = NULL;

    m_pSpdifTxHwContext = NULL;

    m_pSpdifRxHwContext = NULL;

    m_Config = 0;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::HardwareContext\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function: ~HardwareContext
//
//  This function is the destructor for the hardware context class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
HardwareContext::~HardwareContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::~HardwareContext\r\n")));

    Lock();

    Unlock();

    DeleteCriticalSection(&m_Lock);

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::~HardwareContext\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function: Init
//
//  This function initializes the hardware context for the audio device.
//
//  Parameters:
//      Index
//          [in] Context parameter passed to the WAV_Init function
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::Init(DWORD Index)
{
     // 1. Map SPDIF hardware registers and Clock(BSP level)
     // 2. Map DMA buffers
     // 3. Create IST 
     // 4. Call SPDIF RX and TX Init 

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::Init\r\n")));

    // If we have already initialized this device, return error
    if (m_bInitialized)
    {
        return(FALSE);
    }

    m_CodecPwrState = SPDIF_PWR_STATE_OFF;

    // Initialize the driver state/status variables
    m_DriverIndex       = Index;

    BSPSPDIFGetConfig(&m_Config);

    if (TxEnabled())
        m_pSpdifTxHwContext = SpdifTxHwContext::GetHwContext();

    if (RxEnabled())
        m_pSpdifRxHwContext = SpdifRxHwContext::GetHwContext();

    // Initialize BSP-specific configuration.
    if (!BSPInit())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPDIF:HardwareContext::Init() - ")
                              TEXT("BSPInit failed\r\n")));
        goto cleanUp;
    }

    // Map the DMA buffers.
    if (!MapDMABuffers())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPDIF:HardwareContext::Init() - ")
                              TEXT("MapDMABuffers failed\r\n")));
        goto cleanUp;
    }

    // Initialize the  DMA.
    if (!InitDMA(m_PhysDMABufferAddr))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPDIF:HardwareContext::Init() - ")
                              TEXT("Failed to initialize output DMA.\r\n")));
        goto cleanUp;
    }

    if (!BSPRequestIrqs())
     {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPDIF:HardwareContext::Init() - ")
                              TEXT("Failed to request IRQ.\r\n")));
        goto cleanUp;
    }

    // Note that the SPDIF driver interrupt handler will only run if
    // m_Initialized is TRUE so we must initialize the state variable
    // here before trying to create the IST thread.
    m_bInitialized = TRUE;

    // Initialize and create SPDIF processing IST.
    if ((m_hInterruptThread== NULL) && (!InitInterruptThread()))
    {
        ERRMSG("SPDIF.DLL:HardwareContext::Init() - "
               "Failed to initialize interrupt thread.\r\n");
        m_bInitialized = FALSE;
        goto cleanUp;
    }

    //Call RX and TX's Init
    if (m_pSpdifRxHwContext)
        m_pSpdifRxHwContext->Init(Index, this);

    if (m_pSpdifTxHwContext)        
        m_pSpdifTxHwContext->Init(Index, this);

cleanUp:

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::Init\r\n")));

    return m_bInitialized;
}


//-----------------------------------------------------------------------------
//
//  Function:  Deinit
//
//  This function deinitializes the hardware context for the SPDIF device.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::Deinit()
{
    //  1. Stop DMA transfer
    //  2. Unmap SPDIF hardware registers (BSP level)
    //  3. Unmap DMA buffers
    //  4. Call SPDIF RX and TX Deinit 
    
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::Deinit\r\n")));

    Lock();

    if (m_pSpdifRxHwContext)
        m_pSpdifRxHwContext->StopInputDMA();

    if (m_pSpdifTxHwContext)
        m_pSpdifTxHwContext->StopOutputDMA();

    Unlock();

    BSPDeinit();
    // Unmap the SPDIF control registers and DMA buffers.
    UnmapDMABuffers();
    m_bInitialized = FALSE;

    // Wake up the SPDIF driver IST thread so that it can also clean up
    // after itself and then terminate.
    if (m_hInterrupt != NULL)
    {
        SetEvent(m_hInterrupt);
    }

    if (m_pSpdifRxHwContext)
        m_pSpdifRxHwContext->Deinit();

    if (m_pSpdifTxHwContext)
        m_pSpdifTxHwContext->Deinit();

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::Deinit\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  MapDMABuffers
//
//  This function maps the DMA buffers used for SPDIF input/output.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::MapDMABuffers()
{
    PBYTE pVirtDMABufferAddr = NULL;
    BOOL rc = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::MapDMABuffers\r\n")));

    // Allocate a block of virtual memory (physically contiguous) for the
    // DMA buffers. Note that a platform-specific API call is used here to
    // allow for the DMA buffers to be allocated from different memory regions
    // (e.g., internal vs. external memory).
    BSPAllocDMABuffers(&pVirtDMABufferAddr, &m_PhysDMABufferAddr); 
    
    if (pVirtDMABufferAddr == NULL)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SPDIF, HardwareContext::")
                              TEXT("MapDMABuffers() - ")
                              TEXT("Failed to allocate DMA buffer.\r\n")));
        return FALSE;
    }

    m_pVirtDMABufferAddr = pVirtDMABufferAddr;
    // Setup the DMA page pointers.
    //
    // NOTE: Currently, input and output each have two DMA pages: these pages
    //       are used in a round-robin fashion so that the OS can read/write
    //       one buffer while the audio codec chip read/writes the other buffer.
    //
    if (m_pSpdifRxHwContext)
        m_pSpdifRxHwContext->MapDMABuffers(&pVirtDMABufferAddr);
        
    if (m_pSpdifTxHwContext)
        m_pSpdifTxHwContext->MapDMABuffers(&pVirtDMABufferAddr);
    
    //m_Output_pbDMA_PAGES[0] = pVirtDMABufferAddr;
    //m_Output_pbDMA_PAGES[1] = pVirtDMABufferAddr + SPDIFDMAPageSize;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::MapDMABuffers\r\n")));

    return rc;

}

//-----------------------------------------------------------------------------
//
//  Function:  UnmapDMABuffers
//
//  This function unmaps the DMA buffers previously mapped with the
//  MapDMABuffers function.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::UnmapDMABuffers()
{
   BOOL rc = TRUE;
   DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::UnmapDMABuffers\r\n")));

    if (m_pVirtDMABufferAddr)
    {
        BSPDeallocDMABuffers((PVOID)(m_pVirtDMABufferAddr));
        if (m_pSpdifRxHwContext)
            m_pSpdifRxHwContext->UnmapDMABuffers();
        if (m_pSpdifTxHwContext)
            m_pSpdifTxHwContext->UnmapDMABuffers();
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::UnmapDMABuffers\r\n")));

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  InitDMA
//
//  This function initializes the DMA channel for output.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::InitDMA(PHYSICAL_ADDRESS address)
{  
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::InitDMA\r\n")));

    if (m_pSpdifRxHwContext)
    {
        rc = m_pSpdifRxHwContext->InitInputDMA(&address);
    
        if (!rc)
        {
            ERRORMSG(ZONE_ERROR, (_T("SPDIF RX InitInputDMA failed.\r\n")));
            goto cleanUp;
        }
    }

    if (m_pSpdifTxHwContext)
    {
        rc = m_pSpdifTxHwContext->InitOutputDMA(&address);
    
        if (!rc && m_pSpdifRxHwContext)
        {
            m_pSpdifRxHwContext->DeinitInputDMA();
            ERRORMSG(ZONE_ERROR, (_T("SPDIF TX InitOutputDMA failed.\r\n")));
            goto cleanUp;
        }
    }

cleanUp:
   
    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::InitDMA\r\n")));

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  StartOutputDMA
//
//  This function starts outputting the SPDIF data
//  chip via the DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::StartOutputDMA()
{
    BOOL rc = FALSE;
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::StartOutputDMA\r\n")));

    // Must acquire critical section Lock() here so that we don't have a race
    // condition with StopInputDMA() and the interrupt handler.
    Lock();

    BSPSPDIFPowerUp();
    if (m_pSpdifTxHwContext)
        rc = m_pSpdifTxHwContext->StartOutputDMA();
    
    Unlock();

    DEBUGMSG(ZONE_FUNCTION,(TEXT("-HardwareContext::StartOutputDMA\r\n")));

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  StopOutputDMA
//
//  This function stops any DMA activity on the output channel.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopOutputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::StopOutputDMA\r\n")));

    Lock();  
    if (m_pSpdifTxHwContext)
        m_pSpdifTxHwContext->StopOutputDMA();

    if (!m_pSpdifRxHwContext || (m_pSpdifRxHwContext && !m_pSpdifRxHwContext->GetDMARunState()))
        BSPSPDIFPowerDown();
    Unlock();

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::StopOutputDMA\r\n")));
}


//-----------------------------------------------------------------------------
//
//  Function:  StartInputDMA
//
//  This function starts inputting the sound data from the audio codec
//  chip via the DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::StartInputDMA()
{
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::StartInputDMA\r\n")));

    // Must acquire critical section Lock() here so that we don't have a race
    // condition with StopInputDMA() and the interrupt handler.
    Lock();
    BSPSPDIFPowerUp();
    if (m_pSpdifRxHwContext)
        rc = m_pSpdifRxHwContext->StartInputDMA();
    Unlock();

    DEBUGMSG(ZONE_FUNCTION,(TEXT("-HardwareContext::StartInputDMA\r\n")));

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  StopInputDMA
//
//  This function stops any DMA activity on the input channel.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopInputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::StopInputDMA\r\n")));

    Lock();
    if (m_pSpdifRxHwContext)
        m_pSpdifRxHwContext->StopInputDMA();
    if (!m_pSpdifTxHwContext || (m_pSpdifTxHwContext && !m_pSpdifTxHwContext->GetDMARunState()))    
        BSPSPDIFPowerDown();
    Unlock();

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::StopInputDMA\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function:  GetInputDeviceContext
//
//  This function get the InputDeviceContext of SPDIF RX.
//
//  Parameters:
//      Index
//          [in] Context parameter passed from the WAV_Init function
//
//  Returns:
//      Returns pointer to InputDeviceContext class.
//
//-----------------------------------------------------------------------------
DeviceContext *HardwareContext::GetInputDeviceContext(UINT DeviceId)
{
    if (m_pSpdifRxHwContext)
       return m_pSpdifRxHwContext->GetInputDeviceContext(DeviceId);
    else
       return NULL;
}

//-----------------------------------------------------------------------------
//
//  Function:  GetOutputDeviceContext
//
//  This function get the OutputDeviceContext of SPDIF TX.
//
//  Parameters:
//      Index
//          [in] Context parameter passed from the WAV_Init function
//
//  Returns:
//      Returns pointer to OutputDeviceContext class.
//
//-----------------------------------------------------------------------------
DeviceContext *HardwareContext::GetOutputDeviceContext(UINT DeviceId)
{
    if (m_pSpdifTxHwContext)
        return m_pSpdifTxHwContext->GetOutputDeviceContext(DeviceId);
    else
        return NULL;
}

//-----------------------------------------------------------------------------
//
//  Function:  GetInterruptThreadPriority
//
//  This function attempts to retrieve the thread priority for the SPDIF
//  IST from the registry and supplies a default priority if one is not
//  found.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns thread priority for the SPDIF IST.
//
//-----------------------------------------------------------------------------
DWORD HardwareContext::GetInterruptThreadPriority()
{
    HKEY hDevKey;
    DWORD dwValType;
    DWORD dwValLen;
    DWORD dwPrio = INTR_PRIORITY; // Default IST priority

    hDevKey = OpenDeviceKey((LPWSTR)m_DriverIndex);
    if (hDevKey)
    {
        dwValLen = sizeof(DWORD);
        RegQueryValueEx(hDevKey,
                        INTR_PRIORITY_REGKEY,
                        NULL,
                        &dwValType,
                        (PUCHAR)&dwPrio,
                        &dwValLen);
        RegCloseKey(hDevKey);
    }

    return dwPrio;
}

//-----------------------------------------------------------------------------
//
//  Function:  InitInterruptThread
//
//  This function initializes the IST for handling DMA interrupts.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::InitInterruptThread()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::InitInterruptThread\r\n")));

    // Create an automatic reset, unsignaled, and unnamed event for IST
    // signaling.
    m_hInterrupt = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Check if event creation suceeded
    if (!m_hInterrupt)
    {
        ERRMSG("Unable to create SPDIF interrupt event\r\n");
        return(FALSE);
    }

    // Register the SPDIF driver interrupts.
    if (m_dwSysintrSPDIF)
    {
        if (!InterruptInitialize(m_dwSysintrSPDIF, m_hInterrupt, NULL, 0))
        {
            ERRMSG("Unable to initialize interrupt for SPDIF\r\n");
            return FALSE;
        }
    }

    // Create the Interrupt Service Thread (IST).
    m_hInterruptThread =
        CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                     0,
                     (LPTHREAD_START_ROUTINE)CallInterruptThread,
                     this,
                     0,
                     NULL);
    if (!m_hInterruptThread)
    {
        ERRMSG("Unable to create SPDIF interrupt event handling thread\r\n");
        return FALSE;
    }

    // Bump up the priority since the interrupt must be serviced immediately.
    CeSetThreadPriority(m_hInterruptThread, GetInterruptThreadPriority());

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::InitInterruptThread\r\n")));

    return(TRUE);
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerUp
//
//  This function powers up the audio codec chip.  Note that the audio codec
//  chip is ONLY powered up when the user wishes to play or record.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void HardwareContext::PowerUp()
{
    // Restart the SPDIF I/O operations that were suspended when PowerDown()
    // was called.

    if (g_saveOutputDMARunning || g_saveInputDMARunning)
    {
        BSPSPDIFPowerUp();
    }

    if (g_saveOutputDMARunning && m_pSpdifTxHwContext)
    {
        m_pSpdifTxHwContext->StartOutputDMA();
    }

    if (g_saveInputDMARunning && m_pSpdifRxHwContext)
    {
        m_pSpdifRxHwContext->StartInputDMA();
    }

    m_bPowerdown = FALSE;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerDown
//
//  This function powers down the audio codec chip.  If the input/output
//  channels are muted, then this function powers down the appropriate
//  components of the audio codec chip in order to conserve battery power.
//
//  The PowerDown() method will power down the entire audio chip while the
//  PowerDown(const DWORD channel) allows for the independent powerdown of
//  the input and/or output components to allow a finer degree of control.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::PowerDown()
{
    Lock();

    m_bPowerdown = TRUE;

    // Save state for correct powerup handling.
    g_saveOutputDMARunning = (m_pSpdifTxHwContext && m_pSpdifTxHwContext->GetDMARunState());

    g_saveInputDMARunning  = (m_pSpdifRxHwContext && m_pSpdifRxHwContext->GetDMARunState());

    // Request all active SPDIF-related DMA channels to stop.
    if (m_pSpdifTxHwContext)
        m_pSpdifTxHwContext->StopOutputDMA();

    if (m_pSpdifRxHwContext)
        m_pSpdifRxHwContext->StopInputDMA();

    BSPSPDIFPowerDown();

    Unlock();

}

//-----------------------------------------------------------------------------
//
//  Function:  InterruptThread
//
//  This function is the IST for handling SPDIF input and output DMA interrupts.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::InterruptThread()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::InterruptThread\r\n")));

    // Loop repeatedly to handle each SPDIF interrupt event until we
    // deinitialize the SPDIF driver.
    while (m_bInitialized)
    {
        // Wait for the IST to be signaled by the OAL interrupt handler.
        WaitForSingleObject(m_hInterrupt, INFINITE);

        // Immediately terminate this thread if Deinit() has been called to
        // unload the SPDIF driver. Otherwise, handle the SPDIF interrupt
        // event.
        if (!m_bInitialized)
        {
            break;
        }

        Lock();
        
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try
        {
            if (m_pSpdifRxHwContext)
                m_pSpdifRxHwContext->InterruptThread();
            if (m_pSpdifTxHwContext)
                m_pSpdifTxHwContext->InterruptThread();

            InterruptDone(m_dwSysintrSPDIF);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("SPDIF.DLL:InterruptThread() - ")
                                  TEXT("EXCEPTION: %d\r\n"),
                 GetExceptionCode()));
        }

        Unlock();
    }

    CloseHandle(m_hInterrupt);
    m_hInterrupt = NULL;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::InterruptThread\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function:  DumpHwRegs
//
//  This function Dump SPDIF hardware Registers.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::DumpHwRegs()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::DumpHwRegs\r\n")));
    SPDIF_REG regValues;
    memcpy(&regValues, m_pSpdifReg, sizeof(regValues));
    DEBUGMSG(ZONE_TEST,(_T("+DumpHwRegs\r\n")));

    DEBUGMSG(ZONE_TEST,(_T("CTRL REG      = 0x%x\r\n"), HW_SPDIF_CTRL_RD()));
    DEBUGMSG(ZONE_TEST,(_T("STAT REG      = 0x%x\r\n"), HW_SPDIF_STAT_RD()));
    DEBUGMSG(ZONE_TEST,(_T("FRAMECTRL REG = 0x%x\r\n"), HW_SPDIF_FRAMECTRL_RD()));
    DEBUGMSG(ZONE_TEST,(_T("SRR REG       = 0x%x\r\n"), HW_SPDIF_SRR_RD()));
    DEBUGMSG(ZONE_TEST,(_T("DEBUG REG     = 0x%x\r\n"), HW_SPDIF_DEBUG_RD()));
    DEBUGMSG(ZONE_TEST,(_T("VERSION REG   = 0x%x\r\n"), HW_SPDIF_VERSION_RD()));

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::DumpHwRegs\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function:  CallInterruptThread
//
//  This function serves as a wrapper for the IST called within the hardware
//  context.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
DWORD CALLBACK CallInterruptThread(HardwareContext *pHWContext)
{
    if (pHWContext != NULL)
    {
        pHWContext->InterruptThread();
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CallInterruptThread() called with ")
                              TEXT("pHWContext == NULL\r\n")));
    }
    return 1;
}

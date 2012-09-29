//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  txhwctxt.cpp
//
//  ChipSpecific driver for SPDIF device. 
//
//
//-----------------------------------------------------------------------------

#include "wavemain.h"

//-----------------------------------------------------------------------------
// Exported Global Variables


//-----------------------------------------------------------------------------
// Local Global Variables
SpdifTxHwContext *SpdifTxHwContext::m_pSpdifTxHwContext = NULL;

//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
//Externs
extern UINT16 SPDIFDMAPageSize;

//-----------------------------------------------------------------------------
//
//  Function: GetHwContext
//
//  This function  create a unique device instance for SPDIF TX function.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns the pointer to SpdifTxHwContext 
//
//-----------------------------------------------------------------------------
SpdifTxHwContext* SpdifTxHwContext::GetHwContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::GetHwContext\r\n")));

    if (m_pSpdifTxHwContext == NULL)
        m_pSpdifTxHwContext = new SpdifTxHwContext;

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifTxHwContext::GetHwContext\r\n")));

    return m_pSpdifTxHwContext;
}

//-----------------------------------------------------------------------------
//
//  Function: SpdifTxHwContext
//
//  This function is the constructor for the SpdifTxHwContext class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
SpdifTxHwContext::SpdifTxHwContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::SpdifTxHwContext\r\n")));
    m_bInitialized = FALSE;

    m_pOutputDeviceContext = new OutputDeviceContext();

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifTxHwContext::SpdifTxHwContext\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function: ~SpdifTxHwContext
//
//  This function is the destructor for the SpdifTxHwContext class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
SpdifTxHwContext::~SpdifTxHwContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::~SpdifTxHwContext\r\n")));
    
    if (m_pOutputDeviceContext)
    {
        delete m_pOutputDeviceContext;
        m_pOutputDeviceContext = NULL;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::~HardwareContext\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function: Init
//
//  This function initializes the SpdifTxHwContext context for the SPDIF device.
//
//  Parameters:
//      Index
//          [in] not used
//      pReg
//          [in] pointer of SPDIF registers
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::Init(DWORD Index, HardwareContext * pHardwareContext)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::Init\r\n")));
    UNREFERENCED_PARAMETER(Index);

    // If we have already initialized this device, return error
    if (m_bInitialized)
    {
        return(FALSE);
    }

    m_bOutputDMARunning  = FALSE;
    m_bFirstTime = TRUE;
    m_dwOutputDMAStatus = DMA_CLEAR;

    m_pHardwareContext = pHardwareContext;

    m_pSpdifReg = pHardwareContext->GetSPDIFReg();

    DMA_CHANNEL_SPDIF_TX = BSPGetTXDMAChannel();

    memset(&m_intCounter, 0, sizeof(m_intCounter));

    m_pTxFifoPrefill = (PBYTE) new char[m_OutputDMALevel*sizeof(HWSAMPLE)];
    m_iTxFifoPrefill = m_OutputDMALevel;  

    m_bInitialized = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifTxHwContext::Init\r\n")));
    return m_bInitialized;
}

//-----------------------------------------------------------------------------
//
//  Function:  Deinit
//
//  This function deinitializes the SpdifTxHwContext context for the SPDIF device.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::Deinit()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::Deinit\r\n")));

    m_bInitialized = FALSE;
    m_dwOutputDMAStatus = DMA_CLEAR;
    m_pSpdifReg = NULL;

    m_iTxFifoPrefill = 0; 
    if (m_pTxFifoPrefill)
    {
        delete []m_pTxFifoPrefill; 
        m_pTxFifoPrefill = NULL;
    }

    DeinitOutputDMA();

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifTxHwContext::Deinit\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  MapDMABuffers
//
//  This function maps the DMA buffers used for SPDIF TX.
//
//  Parameters:
//      pVirtDMABufferAddr
//          [in/out] Pointer to virtual DMA buffer address, will set the virtual address which 
//                   can be used next call
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::MapDMABuffers(PBYTE* pVirtDMABufferAddr)
{
    m_pOutputDMABuffer[0]  = (*pVirtDMABufferAddr) + (0 * SPDIFDMAPageSize);
    m_pOutputDMABuffer[1]  = (*pVirtDMABufferAddr) + (1 * SPDIFDMAPageSize);

    *pVirtDMABufferAddr =  *pVirtDMABufferAddr + 2 * SPDIFDMAPageSize;

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  UnmapDMABuffers
//
//  This function Unmaps the DMA buffers used for SPDIF TX.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------

BOOL SpdifTxHwContext::UnmapDMABuffers()
{
     m_pOutputDMABuffer[0]  = NULL;
     m_pOutputDMABuffer[1]  = NULL;
     return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  InitOutputDMA
//
//  This function initializes the DMA channel for output.
//
//  Parameters:
//      pAddress
//          [in/out] Pointer to physical DMA buffer address, will set the virtual address which 
//                   can be used next call
//
//  Returns:
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::InitOutputDMA(PHYSICAL_ADDRESS *pAddress)
{
    // Allocate a block of virtual memory (physically contiguos) for the DMA OUTPut Desriptor.
    if(0 == BSPAllocOutputDMADescriptor(&m_DMAOutDescriptorVirtAddr, &m_DMAOutDesPhysAddr))
    {
        ERRMSG("InitOutputDMA: Alloc DMA Descriptor failed");
    } 

    // Setup the DMA Descriptor
    m_DMAOutDescriptorPhysAddr = (PAUDIO_DMA_DESCRIPTOR)m_DMAOutDesPhysAddr.LowPart;
    if(!BSPSetupOutputDMADescriptor(*pAddress))
    {
        ERRMSG("InitOutputDMA: Setup DMA Descriptor failed");
    }
    // disable the SPDIF interrupts
    DDKApbxDmaEnableCommandCmpltIrq(DMA_CHANNEL_SPDIF_TX,FALSE);
    
    // Put the DMA in RESET State
    DDKApbxDmaResetChan(DMA_CHANNEL_SPDIF_TX,TRUE);

    BSPInitOutput();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  DeinitOutputDMA
//
//  This function deinitializes the DMA channel for output.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::DeinitOutputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::DeinitOutputDMA\r\n")));

    // Stop the output devices
    BSPSPDIFStopOutput();

    // Deallocate the virtual memory (physically contiguos) for the DMA OUTPut Desriptor.
    BSPDeallocOutputDMADescriptor(m_DMAOutDescriptorVirtAddr);

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifTxHwContext::DeinitOutputDMA\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  StartOutputDMA
//
//  This function starts outputting the sound data from the SPDIF TX
//  chip via the DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::StartOutputDMA()
{
    BOOL rc = FALSE;
    ULONG nOutputTransferred = 0;

    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::StartOutputDMA\r\n")));
    // Reinitialize SPDIF output if it is not already active.
    if(!m_bOutputDMARunning)
    {
        // Initialize the output DMA state variables.
        m_bOutputDMARunning = TRUE;
        m_dwOutputDMAStatus  = DMA_DONEA | DMA_DONEB;

        PHWSAMPLE txFifoPrefill = (PHWSAMPLE)m_pTxFifoPrefill;

        memset(txFifoPrefill, 0, m_iTxFifoPrefill*sizeof(HWSAMPLE));
        m_pOutputDeviceContext->TransferBuffer((PBYTE)txFifoPrefill,
                                               (PBYTE)(&txFifoPrefill[m_iTxFifoPrefill-1]),
                                               NULL);

        for (int i=0; i<m_iTxFifoPrefill; i++)
        {
            HW_SPDIF_DATA_WR(txFifoPrefill[i]);
        }

        nOutputTransferred = TransferOutputBuffers(m_dwOutputDMAStatus);

        if (nOutputTransferred > 0)
        {
            if(!DDKApbxDmaInitChan(DMA_CHANNEL_SPDIF_TX,TRUE))
            {
                ERRMSG("StartOutputDMA: Unable to initialize output DMA channel\r\n");
                m_bOutputDMARunning = FALSE;
                return FALSE;
            }

            // clear the SPDIF OUT interrupt
            DDKApbxDmaClearCommandCmpltIrq(DMA_CHANNEL_SPDIF_TX);

            // Start the output DMA channel. This must be done before we
            // start Codec because we have to make sure that audio
            // data is already available before that.
            DDKApbxStartDma(DMA_CHANNEL_SPDIF_TX,(PVOID)m_DMAOutDescriptorPhysAddr, 2);

            m_EmptyBufer=0;

            // Enable SPDIF output devices. This must be done after we activate the
            // output DMA channel because the SPDIF hardware will immediately start
            // recording when this call is made.
            BSPSPDIFStartOutput();
        }

    }

    rc = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("-SpdifTxHwContext::StartOutputDMA\r\n")));

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
void SpdifTxHwContext::StopOutputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::StopOutputDMA\r\n")));

    // If the output DMA is running, stop it
    if (m_bOutputDMARunning)
    {
        // Reset output DMA state variables
        m_dwOutputDMAStatus  = DMA_CLEAR;
        m_bOutputDMARunning = FALSE;

        // Disable the interrupt
        DDKApbxDmaEnableCommandCmpltIrq(DMA_CHANNEL_SPDIF_TX, FALSE);

        // clear the SPDIF OUT interrupt
        DDKApbxDmaClearCommandCmpltIrq(DMA_CHANNEL_SPDIF_TX);

        // Disable SPDIF output devices
        BSPSPDIFStopOutput();
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifTxHwContext::StopOutputDMA\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function:  GetDMARunState
//
//  This function return the DMA running state.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for DMA running, FALSE for DMA stopping.
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::GetDMARunState()
{
    return m_bOutputDMARunning;
}

//-----------------------------------------------------------------------------
//
//  Function:  TransferOutputBuffer
//
//  This function retrieves the next "mixed" audio buffer of data to DMA into
//  the output channel.
//
//  Parameters:
//      NumBuf
//          [in] Output DMA page to be filled.
//
//  Returns:
//      Returns number of bytes placed into output buffer.
//
//-----------------------------------------------------------------------------
ULONG SpdifTxHwContext::TransferOutputBuffer(ULONG NumBuf)
{
    ULONG BytesTransferred = 0;

    PBYTE pBufferStart = m_pOutputDMABuffer[NumBuf];
    PBYTE pBufferEnd = pBufferStart + SPDIFDMAPageSize;
    PBYTE pBufferLast;

    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::TransferOutputBuffer\r\n")));
    
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try
    {
        
        pBufferLast = m_pOutputDeviceContext->TransferBuffer(pBufferStart,
                                                           pBufferEnd,
                                                           NULL);

        BytesTransferred = pBufferLast - pBufferStart;

        if(pBufferLast != pBufferEnd)

        // Only mark the DMA buffer as being in use if some data was actually
        // transferred.
            // Enable if you need to clear the rest of the DMA buffer
            // TODO:
            // Probably want to do something better than clearing out remaining
            // buffer samples.  DC output by replicating last sample or
            // some type of fade out would be better.
            StreamContext::ClearBuffer(pBufferLast, pBufferEnd);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPDIF.DLL:TransferOutputBuffer() - ")
                              TEXT("EXCEPTION: %d \r\n"), GetExceptionCode()));
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifTxHwContext::TransferOutputBuffer\r\n")));

    return BytesTransferred;
}

//-----------------------------------------------------------------------------
//
//  Function:  TransferOutputBuffers
//
//  This function determines which output buffer (A or B) needs to be filled
//  with sound data.  The correct buffer is then populated with data and ready
//  to DMA to the output channel.
//
//  Parameters:
//      dwDCSR
//          [in] Current status of the output DMA.
//
//  Returns:
//      Returns number of bytes placed into the output buffers.
//
//-----------------------------------------------------------------------------
ULONG SpdifTxHwContext::TransferOutputBuffers(DWORD dwDCSR)
{
    ULONG BytesTransferred_A = 0;
    ULONG BytesTransferred_B = 0;

    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::TransferOutputBuffers \r\n")));

    if (dwDCSR & DMA_DONEA)
    {
        // DMA buffer A is free, try to refill it.
        BytesTransferred_A = TransferOutputBuffer(OUT_BUFFER_A);

        if (BytesTransferred_A > 0)
        {
            m_dwOutputDMAStatus &= ~DMA_DONEA;

        }
    }

    if (dwDCSR & DMA_DONEB)
    {
        // DMA buffer B is free, try to refill it.
        //
        // Note that we can refill both DMA buffers at once if both DMA
        // buffers are free and there is sufficient data.
        //
        BytesTransferred_B = TransferOutputBuffer(OUT_BUFFER_B);

        if (BytesTransferred_B > 0)
        {
            m_dwOutputDMAStatus &= ~DMA_DONEB;

        }
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifTxHwContext::TransferOutputBuffers\r\n")));

    return BytesTransferred_A + BytesTransferred_B;
}

//-----------------------------------------------------------------------------
//
//  Function:  InterruptThread
//
//  This function Contains the interrupt handling required for SPDIF TX.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SpdifTxHwContext::InterruptThread()
{
    UINT32 nTransferred;
    UINT32 IntrStat =0 , IntrEn = 0;
    if(!DDKApbxDmaGetActiveIrq(DMA_CHANNEL_SPDIF_TX)||!m_bOutputDMARunning)
    {
        return;
    }

    IntrStat = HW_SPDIF_CTRL_RD();
    IntrEn = IntrStat&BM_SPDIF_CTRL_FIFO_ERROR_IRQ_EN;

    if(IntrStat&BM_SPDIF_CTRL_FIFO_OVERFLOW_IRQ)
    {
        m_intCounter.intTxOvFlNum ++;
        DEBUGMSG(ZONE_ERROR, (_T("Tx OverFlow interrupt!\r\n")));
    }

    if(IntrStat&BM_SPDIF_CTRL_FIFO_UNDERFLOW_IRQ)
    {
        m_intCounter.intTxUnFlNum ++;
        DEBUGMSG(ZONE_ERROR, (_T("Tx UnderFlow interrupt!\r\n")));
    }

    // Clear the SPDIF TX interrupt status
    HW_SPDIF_CTRL_SET(BM_SPDIF_CTRL_FIFO_OVERFLOW_IRQ|BM_SPDIF_CTRL_FIFO_UNDERFLOW_IRQ);

    // Set DMA status bits according to retrieved transfer status
    if(DDKApbxGetNextCMDAR(DMA_CHANNEL_SPDIF_TX) == (DWORD)&m_DMAOutDescriptorPhysAddr[OUT_BUFFER_A])
    {
        // The data in DMA buffer A has been transmitted and the
        // buffer may now be reused.
        m_dwOutputDMAStatus |= DMA_DONEA;
    }
    if(DDKApbxGetNextCMDAR(DMA_CHANNEL_SPDIF_TX) == (DWORD)&m_DMAOutDescriptorPhysAddr[OUT_BUFFER_B])
    {
        // The data in DMA buffer B has been transmitted and the
        // buffer may now be reused.
        m_dwOutputDMAStatus |= DMA_DONEB;
    }

    // Try to refill any empty output DMA buffers with new data.
    nTransferred = TransferOutputBuffers(m_dwOutputDMAStatus);

    // Don't restart the DMA unless we actually have some data
    // in at least one of the DMA buffers.
    if (nTransferred == 0)
    {
        m_EmptyBufer++;
    }
    else
    {
        m_EmptyBufer = 0;
    }

    // clear the Audio OUT interrupt
    DDKApbxDmaClearCommandCmpltIrq(DMA_CHANNEL_SPDIF_TX);


    if(m_EmptyBufer >= 5)
    {
        // The upper audio driver layer has no more data, so we can
        // terminate the current DMA operation because all DMA
        // buffers are empty.

        StopOutputDMA();
    }

}

//-----------------------------------------------------------------------------
//
//  Function:  GetNumOutputDevices
//
//  This function returns the number of Output device.
//
//  Parameters:
//      None.
//
//  Returns:
//      The number of output devices.
//
//-----------------------------------------------------------------------------
DWORD SpdifTxHwContext::GetNumOutputDevices(void)
{
    return 1; 
}

//-----------------------------------------------------------------------------
//
//  Function:  SetControlDetails
//
//  This function set the information of SPDIF TX, include CChannel
//
//  Parameters:
//      pDetails
//          [in] CChannel data   
//
//  Returns:
//      TRUE for successful, FALSE for failed.
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::SetControlDetails(PTX_CONTROL_DETAILS pDetails)
{
    if (pDetails->TxStatus.CChannel.H.Data != m_CChanel.H.Data || pDetails->TxStatus.CChannel.L.Data != m_CChanel.L.Data)
    {
        m_CChanel = pDetails->TxStatus.CChannel;

        HW_SPDIF_FRAMECTRL_SET(BF_SPDIF_FRAMECTRL_PRO(m_CChanel.H.Ctrl.ChannelStatus)|
                               BF_SPDIF_FRAMECTRL_AUDIO(m_CChanel.H.Ctrl.AudioFormat)|
                               BF_SPDIF_FRAMECTRL_COPY(m_CChanel.H.Ctrl.Copyright)   |
                               BF_SPDIF_FRAMECTRL_PRE(m_CChanel.H.Ctrl.AddInfo)      |
                               BF_SPDIF_FRAMECTRL_CC(m_CChanel.H.Ctrl.CategoryCode));
    }

    return TRUE;
}


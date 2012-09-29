//--------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//--------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  txhwctxt.cpp
//
//  ChipSpecific driver for SPDIF device. The primary responsibility of this driver is to
//
//
//-----------------------------------------------------------------------------

#include "wavemain.h"
//#include <Mmsystem.h>   // For timer driver interface.

#include "common_macros.h"
#include "common_ddk.h"
#include "common_spdif.h"



//-----------------------------------------------------------------------------
// Exported Global Variables
//SpdifTxHwContext *g_pSpdifTxHwContext = NULL;


//-----------------------------------------------------------------------------
// Local Global Variables
SpdifTxHwContext *SpdifTxHwContext::m_pSpdifTxHwContext = NULL;

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
//Externs
extern UINT16 audioDMAPageSize;

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
    m_dwOutputDMAStatus   = DMA_CLEAR;

    m_pHardwareContext = pHardwareContext;

    m_pSpdifReg = pHardwareContext->GetSPDIFReg();

    memset(&m_intCounter, 0, sizeof(m_intCounter));

    m_pTxFifoPrefill = ( PBYTE) new char[m_OutputDMALevel*sizeof(HWSAMPLE)];
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
    m_dwOutputDMAStatus   = DMA_CLEAR;
    m_pSpdifReg = NULL;

    m_iTxFifoPrefill = 0; 
    if (m_pTxFifoPrefill)
    {
        delete []m_pTxFifoPrefill; 
        m_pTxFifoPrefill = NULL;
    }

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
    m_pOutputDMABuffer[0]  = (*pVirtDMABufferAddr) + (0 * audioDMAPageSize);
    m_pOutputDMABuffer[1]  = (*pVirtDMABufferAddr) + (1 * audioDMAPageSize);

    *pVirtDMABufferAddr =  *pVirtDMABufferAddr + 2 * audioDMAPageSize;

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
    BOOL rc = FALSE;
    DDK_DMA_ACCESS audioDataRXWordSize = DDK_DMA_ACCESS_8BIT;

    UINT8 size = sizeof(HWSAMPLE);
    if (size == sizeof(INT16))
    {
        audioDataRXWordSize = DDK_DMA_ACCESS_16BIT;
    }
    else if (size == sizeof(INT32))
    {
        audioDataRXWordSize = DDK_DMA_ACCESS_32BIT;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifTxHwContext::InitOutputDMA\r\n")));

    // Check if DMA buffer has been allocated
    if (!pAddress->LowPart)
    {
        ERRORMSG(ZONE_ERROR, (_T("Invalid DMA buffer physical address.\r\n")));
        goto cleanUp;
    }

    // Configure the platform-specific output DMA channel
    if (!BSPInitOutput())
    {
        ERRORMSG(ZONE_ERROR, (_T("BSPAudioInitOutput failed.\r\n")));
        goto cleanUp;
    }

    // Allocate a DMA chain for the virtual channel to handle the DMA transfer
    // requests.
    if (!DDKSdmaAllocChain(m_OutputDMAChan, NUM_DMA_BUFFERS))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaAllocChain failed.\r\n")));
        goto cleanUp;
    }

    DEBUGMSG(ZONE_TEST, (_T("SpdifTxHwContext::InitOutputDMA :  pAddress->LowPart = 0x%x\r\n"), pAddress->LowPart));

    // Configure buffer descriptors
    if (!DDKSdmaSetBufDesc(m_OutputDMAChan,
                           0,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT,
                           pAddress->LowPart +
                               (0 * audioDMAPageSize),
                           0,
                           audioDataRXWordSize,
                           audioDMAPageSize))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaSetBufDesc failed.\r\n")));
        goto cleanUp;
    }

    if (!DDKSdmaSetBufDesc(m_OutputDMAChan,
                           1,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT |
                               DDK_DMA_FLAGS_WRAP,
                           pAddress->LowPart +
                               (1 * audioDMAPageSize),
                           0,
                           audioDataRXWordSize,
                           audioDMAPageSize))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaSetBufDesc failed.\r\n")));
        goto cleanUp;
    }

    if(!DDKSdmaInitChain(m_OutputDMAChan, 
                             m_OutputDMALevel * sizeof(HWSAMPLE)))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaInitChain failed.\r\n")));
        goto cleanUp;
    }

    // Indicate the next address can be used
    pAddress->LowPart += 2 * audioDMAPageSize;

    rc = TRUE;

cleanUp:
    if (!rc)
    {
        DeinitOutputDMA();
    }
    
    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifTxHwContext::InitOutputDMA\r\n")));

    return rc;
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

    if (m_OutputDMAChan != 0)
    {
        // Kill the output DMA channel
        DDKSdmaStopChan(m_OutputDMAChan, TRUE);
        m_OutputDMAChan = 0;
    }

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
        
        for (int i=0; i<m_iTxFifoPrefill/2; i++)
        {
            OUTREG32(&m_pSpdifReg->STL, txFifoPrefill[2*i]);
            OUTREG32(&m_pSpdifReg->STR, txFifoPrefill[2*i+1]);
        }

        nOutputTransferred = TransferOutputBuffers(m_dwOutputDMAStatus);

        if (nOutputTransferred > 0)
        {
            // Start the Output DMA.
            DDKSdmaStartChan(m_OutputDMAChan);

            // Enable SPDIF output devices. This must be done after we activate the
            // output DMA channel because the audio hardware will immediately start
            // recording when this call is made.
            BSPSPDIFStartOutput();
        }
    }
    else
    {
        // Try to transfer in case we've got a buffer spare and waiting
        nOutputTransferred = TransferOutputBuffers(m_dwOutputDMAStatus);
  
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

        // Disable audio output devices
        BSPSPDIFStopOutput();

        // Kill the output DMA channel
        DDKSdmaStopChan(m_OutputDMAChan, FALSE);
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
    PBYTE pBufferEnd = pBufferStart + audioDMAPageSize;
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
        BytesTransferred_A =
            TransferOutputBuffer(OUT_BUFFER_A);
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
        BytesTransferred_B =
            TransferOutputBuffer(OUT_BUFFER_B);
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
        static DWORD recCheckFirst = IN_BUFFER_A;
        UINT32 bufDescStatus[NUM_DMA_BUFFERS];
        UINT32 intrStat =0 , intrEn = 0;
        PSPDIF_REG pReg = m_pSpdifReg;
      
        // Handle DMA output if it is active.
        if (m_bOutputDMARunning)
        {
             intrStat = INREG32(&pReg->SIS);
             intrEn = INREG32(&pReg->SIE);

             // Handle SPDIF TX related interrupt
             if (intrStat & intrEn & CSP_BITFMASK(SPDIF_SIS_TXEM))
             {
                 m_intCounter.intTxEmNum ++;
                 DEBUGMSG(ZONE_ERROR, (_T("Tx empty interrupt!\r\n")));              
             }
             
             if (intrStat & intrEn & CSP_BITFMASK(SPDIF_SIS_TXRESYN))
             {
                 m_intCounter.intTxResynNum++;
                 DEBUGMSG(ZONE_ERROR, (_T("Tx FIFO resync interrupt!\r\n")));              
             } 

             if (intrStat & intrEn & CSP_BITFMASK(SPDIF_SIS_TXUNOV))
             {
                 m_intCounter.intTxUnOvNum++;
                 DEBUGMSG(ZONE_ERROR, (_T("Tx FIFO under/overrun interrupt, should not occur!\r\n")));              
             }

            // Clear the SPDIF TX related interrupt status(w1c)
            INSREG32(&m_pSpdifReg->SIS,  
                               CSP_BITFMASK(SPDIF_SIS_TXEM) | 
                               CSP_BITFMASK(SPDIF_SIS_TXRESYN) | 
                               CSP_BITFMASK(SPDIF_SIS_TXUNOV),
                               CSP_BITFMASK(SPDIF_SIS_TXEM) | 
                               CSP_BITFMASK(SPDIF_SIS_TXRESYN) | 
                               CSP_BITFMASK(SPDIF_SIS_TXUNOV));
           
            // Get the transfer status of the DMA output buffers.
            if (!DDKSdmaGetChainStatus(m_OutputDMAChan, bufDescStatus))
            {
                ERRORMSG(ZONE_ERROR,(_T("Could not retrieve output buffer ")
                                     _T("status\r\n")));
            }

            if (bufDescStatus[OUT_BUFFER_A] & DDK_DMA_FLAGS_ERROR)
            {
                ERRORMSG(ZONE_ERROR,(_T("ERROR:  Error flag set in ")
                                     _T("OUT_BUFFER_A descriptor\r\n")));
            }

            if (bufDescStatus[OUT_BUFFER_B] & DDK_DMA_FLAGS_ERROR)
            {
                ERRORMSG(ZONE_ERROR,(_T("ERROR:  Error flag set in ")
                                     _T("OUT_BUFFER_B descriptor\r\n")));
            }

            // Set DMA status bits according to retrieved transfer status.
            if (!(bufDescStatus[OUT_BUFFER_A] & DDK_DMA_FLAGS_BUSY))
            {
                // The data in DMA buffer A has been transmitted and the
                // buffer may now be reused.
                m_dwOutputDMAStatus |= DMA_DONEA;
                DDKSdmaClearBufDescStatus(m_OutputDMAChan, OUT_BUFFER_A);
            }

            if (!(bufDescStatus[OUT_BUFFER_B] & DDK_DMA_FLAGS_BUSY))
            {
                // The data in DMA buffer B has been transmitted and the
                // buffer may now be reused.
                m_dwOutputDMAStatus |= DMA_DONEB;
                DDKSdmaClearBufDescStatus(m_OutputDMAChan, OUT_BUFFER_B);
            }

          
            // Try to refill any empty output DMA buffers with new data.
            TransferOutputBuffers(m_dwOutputDMAStatus);

            // Don't restart the DMA unless we actually have some data
            // in at least one of the DMA buffers.
            if (!(m_dwOutputDMAStatus & DMA_DONEA) ||
                !(m_dwOutputDMAStatus & DMA_DONEB))
            {
                // SDMA will disable itself on buffer underrun.
                // Force channel to be enabled since we now have
                // at least one buffer to be processed.
                DDKSdmaStartChan(m_OutputDMAChan);
            }
            else
            {
                // The upper audio driver layer has no more data, so we can
                // terminate the current DMA operation because all DMA
                // buffers are empty.           
                m_pHardwareContext->StopOutputDMA();      
            }
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
//  This function returns the information of SPDIF RX, include CChannel
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
        INSREG32(&m_pSpdifReg->STCSCH,
                 CSP_BITFMASK(SPDIF_STCSCH_TXCCHANNELCONSH),
                 CSP_BITFVAL(SPDIF_STCSCH_TXCCHANNELCONSH, m_CChanel.H.Data));
        INSREG32(&m_pSpdifReg->STCSCL,  
                 CSP_BITFMASK(SPDIF_STCSCL_TXCCHANNELCONSL),
                 CSP_BITFVAL(SPDIF_STCSCL_TXCCHANNELCONSL, m_CChanel.L.Data));
    }

    return TRUE;
}


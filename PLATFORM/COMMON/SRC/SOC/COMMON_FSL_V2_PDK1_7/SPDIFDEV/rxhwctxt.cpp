 //--------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//--------------------------------------------------------------------------------
//
//  File:  rxhwctxt.cpp
//
//  Driver for SPDIF device. The primary responsibility of this driver is to
//  implement the SPDIF RX function
//
//-----------------------------------------------------------------------------

#include "wavemain.h"

#include "common_macros.h"
#include "common_ddk.h"

//-----------------------------------------------------------------------------
// Exported Global Variables


//-----------------------------------------------------------------------------
// Local Global Variables
SpdifRxHwContext *SpdifRxHwContext::m_pSpdifRxHwContext = NULL;
//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
//Externs
extern UINT16 audioDMAPageSize;

//-----------------------------------------------------------------------------
//
//  Function: GetHwContext
//
//  This function  create a unique device instance for SPDIF RX function.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns the pointer to SpdifRxHwContext 
//
//-----------------------------------------------------------------------------
SpdifRxHwContext* SpdifRxHwContext::GetHwContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::CreateHwContext\r\n")));

    if (m_pSpdifRxHwContext == NULL)
        m_pSpdifRxHwContext = new SpdifRxHwContext;

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::CreateHwContext\r\n")));
    
    return m_pSpdifRxHwContext;


}

//-----------------------------------------------------------------------------
//
//  Function: SpdifRxHwContext
//
//  This function is the constructor for SpdifRxHwContext class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
SpdifRxHwContext::SpdifRxHwContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::SpdifRxHwContext\r\n")));
    m_bInitialized = FALSE;

    m_pInputDeviceContext = new InputDeviceContext();

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::SpdifRxHwContext\r\n")));
}


//-----------------------------------------------------------------------------
//
//  Function: ~SpdifRxHwContext
//
//  This function is the destructor for SpdifRxHwContext class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
SpdifRxHwContext::~SpdifRxHwContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::~SpdifRxHwContext\r\n")));
  
    if (m_pInputDeviceContext)
    {
        delete m_pInputDeviceContext;
        m_pInputDeviceContext = NULL;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::~HardwareContext\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function: Init
//
//  This function initializes the SpdifRxHwContext context for the SPDIF device.
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
BOOL SpdifRxHwContext::Init(DWORD Index, HardwareContext * pHardwareContext)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::Init\r\n")));
    UNREFERENCED_PARAMETER(Index);

    // If we have already initialized this device, return error
    if (m_bInitialized)
    {
        return(FALSE);
    }

    //m_dwInputGain   = 0x0000;
    //m_fInputMute    = TRUE;

    m_bInputDMARunning  = FALSE;
    m_dwInputDMAStatus   = DMA_CLEAR;

    m_pHardwareContext = pHardwareContext;

    m_pSpdifReg = m_pHardwareContext->GetSPDIFReg();

    memset(&m_intCounter, 0, sizeof(m_intCounter));

    m_bInitialized = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::Init\r\n")));

    return m_bInitialized;
}

//-----------------------------------------------------------------------------
//
//  Function:  Deinit
//
//  This function deinitializes the RX context for the SPDIF device.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SpdifRxHwContext::Deinit()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::Deinit\r\n")));

    m_bInitialized = FALSE;
    m_dwInputDMAStatus   = DMA_CLEAR;
    m_pSpdifReg = NULL;

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::Deinit\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  MapDMABuffers
//
//  This function maps the DMA buffers used for SPDIF RX.
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
BOOL SpdifRxHwContext::MapDMABuffers(PBYTE* pVirtDMABufferAddr)
{
    m_pInputDMABuffer[0]  = (*pVirtDMABufferAddr) + (0 * audioDMAPageSize);
    m_pInputDMABuffer[1]  = (*pVirtDMABufferAddr) + (1 * audioDMAPageSize);

    *pVirtDMABufferAddr =  *pVirtDMABufferAddr + 2 * audioDMAPageSize;

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  UnmapDMABuffers
//
//  This function Unmaps the DMA buffers used for SPDIF RX.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------

BOOL SpdifRxHwContext::UnmapDMABuffers()
{
     m_pInputDMABuffer[0]  = NULL;
     m_pInputDMABuffer[1]  = NULL;
     return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  InitInputDMA
//
//  This function initializes the DMA channel for input.
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
BOOL SpdifRxHwContext::InitInputDMA(PHYSICAL_ADDRESS *pAddress)
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

    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::InitInputDMA\r\n")));

    // Check if DMA buffer has been allocated
    if (!pAddress->LowPart)
    {
        ERRORMSG(ZONE_ERROR, (_T("Invalid DMA buffer physical address.\r\n")));
        goto cleanUp;
    }

    // Configure the platform-specific input DMA channel
    if (!BSPInitInput())
    {
        ERRORMSG(ZONE_ERROR, (_T("BSPAudioInitInput failed.\r\n")));
        goto cleanUp;
    }

    // Allocate a DMA chain for the virtual channel to handle the DMA transfer
    // requests.
    if (!DDKSdmaAllocChain(m_InputDMAChan, NUM_DMA_BUFFERS))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaAllocChain failed.\r\n")));
        goto cleanUp;
    }

    DEBUGMSG(ZONE_TEST, (_T("SpdifRxHwContext::InitInputDMA :  pAddress->LowPart = 0x%x\r\n"), pAddress->LowPart));

    // Configure buffer descriptors
    if (!DDKSdmaSetBufDesc(m_InputDMAChan,
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

    if (!DDKSdmaSetBufDesc(m_InputDMAChan,
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

    // Indicate the next address can be used
    pAddress->LowPart += 2 * audioDMAPageSize;

    rc = TRUE;

cleanUp:
    if (!rc)
    {
        DeinitInputDMA();
    }
    

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::InitInputDMA\r\n")));

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DeinitInputDMA
//
//  This function deinitializes the DMA channel for input.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL SpdifRxHwContext::DeinitInputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::DeinitInputDMA\r\n")));

    // Stop the audio input devices
    BSPSPDIFStopInput();

    if (m_InputDMAChan != 0)
    {
        // Kill the input DMA channel
        DDKSdmaStopChan(m_InputDMAChan, TRUE);
        m_InputDMAChan = 0;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::DeinitInputDMA\r\n")));

    return TRUE;
}




//-----------------------------------------------------------------------------
//
//  Function:  StartInputDMA
//
//  This function starts inputting the sound data from the SPDIF RX
//  chip via the DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL SpdifRxHwContext::StartInputDMA()
{

    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::StartInputDMA\r\n")));
    
    // Reinitialize audio input if it is not already active.
    if(!m_bInputDMARunning)
    {
        // Initialize the input DMA state variables.
        m_bInputDMARunning = TRUE;
        m_dwInputDMAStatus  = (DWORD)~(DMA_DONEA | DMA_DONEB);

        // Reset the state of the input DMA chain.
        //
        // Note that the second "watermark" argument to DDKSdmaInitChain()
        // is actually in bytes whereas "m_InputDMALevel" is given in
        // SSI FIFO slots or words. Therefore, we have to multiply by the
        // audio data word size to ensure that we are properly emptying
        // the SSI RX FIFO.
        //
        if(!DDKSdmaInitChain(m_InputDMAChan,
                             m_InputDMALevel * sizeof(HWSAMPLE)))
        {
            DEBUGMSG(ZONE_ERROR, (_T("SpdifRxHwContext::StartInputDMA() - ")
                      _T("Unable to initialize input DMA channel!\r\n")));

            // Must reset the m_InputDMARunning state variable to FALSE
            // if we encounter an error here.
            m_bInputDMARunning = FALSE;

            goto START_ERROR;
        }

        // Start the input DMA.
        DDKSdmaStartChan(m_InputDMAChan);

        // Enable audio input devices. This must be done after we activate the
        // input DMA channel because the audio hardware will immediately start
        // recording when this call is made.
        BSPSPDIFStartInput();
    }

    rc = TRUE;

START_ERROR:
    //Unlock();

    DEBUGMSG(ZONE_FUNCTION,(TEXT("-SpdifRxHwContext::StartInputDMA\r\n")));

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
void SpdifRxHwContext::StopInputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::StopInputDMA\r\n")));

    // If the input DMA is running, stop it
    if (m_bInputDMARunning)
    {
        // Reset input DMA state variables
        m_dwInputDMAStatus  = DMA_CLEAR;
        m_bInputDMARunning = FALSE;

        // Disable SPDIF input devices
        BSPSPDIFStopInput();

        // Kill the output DMA channel
        DDKSdmaStopChan(m_InputDMAChan, FALSE);
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::StopInputDMA\r\n")));
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
BOOL SpdifRxHwContext::GetDMARunState()
{
    return m_bInputDMARunning;
}

//-----------------------------------------------------------------------------
//
//  Function:  TransferInputBuffer
//
//  This function retrieves the chunk of recorded sound data and inputs
//  it into an audio buffer for potential "mixing".
//
//  Parameters:
//      NumBuf
//          [in] Input DMA page to be filled.
//
//  Returns:
//      Returns number of bytes placed into input buffer.
//
//-----------------------------------------------------------------------------
ULONG SpdifRxHwContext::TransferInputBuffer(ULONG NumBuf)
{
    ULONG BytesTransferred = 0;

    PBYTE pBufferStart = m_pInputDMABuffer[NumBuf];
    PBYTE pBufferEnd = pBufferStart + audioDMAPageSize;
    PBYTE pBufferLast;

    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try
    {
        pBufferLast = m_pInputDeviceContext->TransferBuffer(pBufferStart,
                                                          pBufferEnd,NULL);
        BytesTransferred = pBufferLast - pBufferStart;

        // We will mark the input DMA buffer as being in use again regardless
        // of whether or not the data was completely copied out to an
        // application-supplied data buffer. The input DMA buffer must be
        // immediately reused because the audio recording operation is still
        // running.
        if(NumBuf == IN_BUFFER_A)
        {
            m_dwInputDMAStatus &= ~DMA_DONEA;
        }
        else
        {
            m_dwInputDMAStatus &= ~DMA_DONEB;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPDIF.DLL:TransferInputBuffer() - ")
                              TEXT("EXCEPTION: %d \r\n"), GetExceptionCode()));
    }

    return BytesTransferred;
}


//-----------------------------------------------------------------------------
//
//  Function:  TransferInputBuffers
//
//  This function determines which DMA input buffer (A and/or B) needs to be
//  transferred to the audio application-supplied data buffers.  The DMA input
//  buffer can be reused once the data has been copied out.
//
//  If there is insufficient space in the application-supplied data buffers to
//  transfer out all of the data that is currently in the DMA input buffers,
//  then all excess data is simply discarded.
//
//  Parameters:
//      dwDCSR
//          [in] Current status of the input DMA.
//      checkFirst
//          [in] Identifies which of the two available DMA input buffers
//               should be transferred first in order to maintain the
//               correct sequencing of the input audio stream.
//
//  Returns:
//      Returns number of bytes placed into the input buffers.
//
//-----------------------------------------------------------------------------
ULONG SpdifRxHwContext::TransferInputBuffers(DWORD dwDCSR, DWORD checkFirst)
{
    ULONG BytesTransferred=0;

    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::TransferInputBuffers\r\n")));

    if (checkFirst == IN_BUFFER_A)
    {
        if (dwDCSR & DMA_DONEA)
        {
            BytesTransferred = TransferInputBuffer(IN_BUFFER_A);
        }

        if (dwDCSR & DMA_DONEB)
        {
            BytesTransferred += TransferInputBuffer(IN_BUFFER_B);
        }
    }
    else    // (checkFirst == IN_BUFFER_B)
    {
        if (dwDCSR & DMA_DONEB)
        {
            BytesTransferred = TransferInputBuffer(IN_BUFFER_B);
        }

        if (dwDCSR & DMA_DONEA)
        {
            BytesTransferred += TransferInputBuffer(IN_BUFFER_A);
        }
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::TransferInputBuffers\r\n")));

    return BytesTransferred;
}


//-----------------------------------------------------------------------------
//
//  Function:  InterruptThread
//
//  This function Contains the interrupt handling required for SPDIF RX.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SpdifRxHwContext::InterruptThread()
{
        static DWORD recCheckFirst = IN_BUFFER_A;
        UINT32 bufDescStatus[NUM_DMA_BUFFERS];
        UINT32 intrStat =0 , intrEn = 0;
        PSPDIF_REG pReg = m_pSpdifReg;
        static DWORD dwLockLossCount = 0;
        #define MAX_LOCKLOSS 10

        // Handle DMA input (i.e., audio recording) if it is active.
        if (m_bInputDMARunning)
        {
           intrStat = INREG32(&pReg->SIS);
           intrEn = INREG32(&pReg->SIE);

           DEBUGMSG(ZONE_TEST, (_T("Interrupt status = 0x%x! \r\n"), intrStat));

           // Handle SPDIF RX related interrupt
#if 0
           if (intrStat & intrEn & CSP_BITFMASK(SPDIF_SIS_LOCKLOSS))
           {
               //It's really a lock loss...we must stop the dma transfer
               if (dwLockLossCount++ > MAX_LOCKLOSS)                         
           }
#endif
           
           if (intrStat & intrEn & CSP_BITFMASK(SPDIF_SIS_CNEW))
           {
               if (TST_PLL_LOCKED(pReg->SRPC))
               {
                  m_CChanel.H.Data = (INREG32(&pReg->SRCSH) & CSP_BITFMASK(SPDIF_SRCSH_RXCCHANNELH));
                  m_CChanel.L.Data = (INREG32(&pReg->SRCSL) & CSP_BITFMASK(SPDIF_SRCSL_RXCCHANNELL));
               }
               m_intCounter.intRxCNewNum++;
           }

           if (intrStat & intrEn & CSP_BITFMASK(SPDIF_SIS_URXFUL))
           {
               if (TST_PLL_LOCKED(pReg->SRPC))
               {
                  m_UChanel = (INREG32(&pReg->SRU) & CSP_BITFMASK(SPDIF_SRU_RXUCHANNEL));
               }
               m_intCounter.intRxURxFulNum++;
           }

           if (intrStat & intrEn & CSP_BITFMASK(SPDIF_SIS_QRXFUL))
           {
               if (TST_PLL_LOCKED(pReg->SRPC))
               {
                  m_QChanel = (INREG32(&pReg->SRQ) & CSP_BITFMASK(SPDIF_SRQ_RXQCHANNEL));
               }
               m_intCounter.intRxQRxFulNum++;
           }

           if (intrStat & intrEn & CSP_BITFMASK(SPDIF_SIS_RXFIFOFUL))
           {
               m_intCounter.intRxFIFOFulNum++;
               DEBUGMSG(ZONE_ERROR, (_T("Rx FIFO Full interrupt!\r\n")));         
               //DEBUGMSG(ZONE_ERROR, (_T("Rx Left = 0x%x, Rx Right = 0x%x!"),INREG32(&pReg->SRL), INREG32(&pReg->SRR) ));
           }

           if (intrStat & intrEn & CSP_BITFMASK(SPDIF_SIS_RXFIFORESYNC))
           {
               m_intCounter.intRxFIFOResynNum++;
               DEBUGMSG(ZONE_ERROR, (_T("Rx FIFO Resync interrupt!\r\n")));              
           }

           // Clear the SPDIF RX related interrupt status(w1c)
           INSREG32(&m_pSpdifReg->SIS,  
                               CSP_BITFMASK(SPDIF_SIS_CNEW) | 
                               CSP_BITFMASK(SPDIF_SIS_URXFUL) | 
                               CSP_BITFMASK(SPDIF_SIS_QRXFUL) |
                               CSP_BITFMASK(SPDIF_SIS_RXFIFOFUL) |
                               CSP_BITFMASK(SPDIF_SIS_RXFIFORESYNC),
                               CSP_BITFMASK(SPDIF_SIS_CNEW) | 
                               CSP_BITFMASK(SPDIF_SIS_URXFUL) | 
                               CSP_BITFMASK(SPDIF_SIS_QRXFUL) |
                               CSP_BITFMASK(SPDIF_SIS_RXFIFOFUL) |
                               CSP_BITFMASK(SPDIF_SIS_RXFIFORESYNC));
  
           // Get the transfer status of the DMA input buffers.
           if (!DDKSdmaGetChainStatus(m_InputDMAChan, bufDescStatus))
           {
                 ERRORMSG(ZONE_ERROR,(_T("Could not retrieve input buffer ")
                                _T("status\r\n")));
           }

           if (bufDescStatus[IN_BUFFER_A] & DDK_DMA_FLAGS_ERROR)
           {
                 ERRORMSG(ZONE_ERROR,(_T("ERROR:  Error flag set in ")
                               _T("IN_BUFFER_A\r\n")));
           }

          if (bufDescStatus[IN_BUFFER_B] & DDK_DMA_FLAGS_ERROR)
          {
                 ERRORMSG(ZONE_ERROR,(_T("ERROR:  Error flag set in ")
                                _T("IN_BUFFER_B\r\n")));
          }

          // Set DMA status bits according to retrieved transfer status.
          if (!(bufDescStatus[IN_BUFFER_A] & DDK_DMA_FLAGS_BUSY))
          {
          // Mark input buffer A as having new audio data.
                m_dwInputDMAStatus |= DMA_DONEA;
                DDKSdmaClearBufDescStatus(m_InputDMAChan, IN_BUFFER_A);
          }

          if (!(bufDescStatus[IN_BUFFER_B] & DDK_DMA_FLAGS_BUSY))
          {
          // Mark input buffer B as having new audio data.
                m_dwInputDMAStatus |= DMA_DONEB;
                DDKSdmaClearBufDescStatus(m_InputDMAChan, IN_BUFFER_B);
           }

         // Try to empty out the input DMA buffers by copying the data
         // up to the application-supplied data buffers.
         //
         // Note that we require the higher-level audio application
         // to provide enough data buffers to transfer all of the new
         // data from the input DMA buffers. Otherwise, we will be
         // forced to discard the data and just continue recording.
          TransferInputBuffers(m_dwInputDMAStatus,  recCheckFirst);

        // Update the recCheckFirst flag to mark the first input DMA
        // buffer that should be transferred in order to maintain the
        // correct ABABABAB... input DMA buffer sequencing.
        //
        // Note that except for the 2 cases that we check for below, we
        // do not need to change the current value of the recCheckFirst
        // flag. Here are some pseudocode blocks to explain why:
        //
        
           // The same logic can be applied for the cases where the
           // recCheckFirst flag is equal to IN_BUFFER_B.
           if ((recCheckFirst == IN_BUFFER_A) &&
              (m_dwInputDMAStatus & DMA_DONEA) &&
              !(m_dwInputDMAStatus & DMA_DONEB))
           {
           // Input DMA buffer A was full and just transferred but
           // input DMA buffer B was not yet full so we must check
           // it first when handling the next input DMA interrupt.
                 recCheckFirst = IN_BUFFER_B;
           }
           else if ((recCheckFirst == IN_BUFFER_B) &&
                    (m_dwInputDMAStatus & DMA_DONEB) &&
                    !(m_dwInputDMAStatus & DMA_DONEA))
           {
           // Input DMA buffer B was full and just transferred but
           // input DMA buffer A was not yet full so we must check
           // it first when handling the next input DMA interrupt.
                 recCheckFirst = IN_BUFFER_A;
           }

           // Force the input DMA channel to be enabled again since we
           // now have at least one input DMA buffer that's ready to be
           // used.
           DDKSdmaStartChan(m_InputDMAChan);
     }

}



//-----------------------------------------------------------------------------
//
//  Function:  GetNumInputDevices
//
//  This function returns the number of Input device.
//
//  Parameters:
//      None.
//
//  Returns:
//      The number of input devices.
//
//-----------------------------------------------------------------------------

DWORD SpdifRxHwContext::GetNumInputDevices(void)
{
        return 1; 
}

//-----------------------------------------------------------------------------
//
//  Function:  GetControlDetails
//
//  This function returns the information of SPDIF RX, include CChanel, UChannel,
//  QChannel, Frequency.
//
//  Parameters:
//      pDetails
//          [out] Fill the information include CChanel, UChannel, QChannel, Frequency    
//
//  Returns:
//      TRUE for successful, FALSE for failed.
//
//-----------------------------------------------------------------------------
BOOL SpdifRxHwContext::GetControlDetails(PRX_CONTROL_DETAILS pDetails)
{
    pDetails->RxStatus.CChannel = m_CChanel;
    pDetails->RxStatus.UChannel = m_UChanel;
    pDetails->RxStatus.QChannel = m_QChanel;
    pDetails->FreqMesa = BSPGetFreqMesa();
    return TRUE;
}




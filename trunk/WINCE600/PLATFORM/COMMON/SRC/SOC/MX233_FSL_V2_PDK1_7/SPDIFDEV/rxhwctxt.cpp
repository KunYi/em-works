 //-----------------------------------------------------------------------------
 //
 //  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
 //  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
 //  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
 //
 //-----------------------------------------------------------------------------
//
//  File:  rxhwctxt.cpp
//
//  Driver for SPDIF device. The primary responsibility of this driver is to
//  implement the SPDIF RX function
//
//-----------------------------------------------------------------------------

#include "wavemain.h"


//-----------------------------------------------------------------------------
// Exported Global Variables


//-----------------------------------------------------------------------------
// Local Global Variables
SpdifRxHwContext *SpdifRxHwContext::m_pSpdifRxHwContext = NULL;
//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
//Externs
extern UINT16 SPDIFDMAPageSize;

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
    m_pInputDMABuffer[0]  = (*pVirtDMABufferAddr) + (0 * SPDIFDMAPageSize);
    m_pInputDMABuffer[1]  = (*pVirtDMABufferAddr) + (1 * SPDIFDMAPageSize);

    *pVirtDMABufferAddr =  *pVirtDMABufferAddr + 2 * SPDIFDMAPageSize;

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
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::InitInputDMA\r\n")));
    UNREFERENCED_PARAMETER(pAddress);

    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::InitInputDMA\r\n")));

    return TRUE;
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
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::StartInputDMA\r\n")));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("-SpdifRxHwContext::StartInputDMA\r\n")));

    return TRUE;
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
    UNREFERENCED_PARAMETER(NumBuf);

    return 0;
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
    DEBUGMSG(ZONE_FUNCTION,(_T("+SpdifRxHwContext::TransferInputBuffers\r\n")));
    UNREFERENCED_PARAMETER(dwDCSR);
    UNREFERENCED_PARAMETER(checkFirst);
   
    DEBUGMSG(ZONE_FUNCTION,(_T("-SpdifRxHwContext::TransferInputBuffers\r\n")));

    return 0;
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
    return;
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
    return 0; 
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




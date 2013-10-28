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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/ 
//
//  File: dma.cpp
//
//  SDHC controller driver implementation
//  DMA routines
//

#include "SDHC.h"
#include "SDHCRegs.h"

#ifdef SDIO_DMA_ENABLED
DmaConfigInfo_t TxDmaSettings = {

    // element width
    // valid values are:
    //  DMA_CSDP_DATA_TYPE_S8
    //  DMA_CSDP_DATA_TYPE_S16
    //  DMA_CSDP_DATA_TYPE_S32
    //
    DMA_CSDP_DATATYPE_S32,

    // source element index
    // valid values are: 
    //  [-32768, 32768]
    //
    0,

    // source frame index
    // valid values are: 
    //  [-2147483648, 2147483647] : non-packet mode
    //  [-32768, 32768] : packet mode
    //    
    0,

    // source addressing mode
    // valid values are:
    //   DMA_CCR_SRC_AMODE_DOUBLE
    //   DMA_CCR_SRC_AMODE_SINGLE
    //   DMA_CCR_SRC_AMODE_POST_INC   
    //   DMA_CCR_SRC_AMODE_CONST 
    // 
    DMA_CCR_SRC_AMODE_POST_INC,

    // destination element index
    // valid values are: 
    //  [-32768, 32767]
    //
    0,

    // destination frame index
    // valid values are: 
    //  [-2147483648, 2147483647] : non-packet mode
    //  [-32768, 32768] : packet mode
    // 
    0,

    // destination addressing mode
    // valid values are:
    //   DMA_CCR_DST_AMODE_DOUBLE
    //   DMA_CCR_DST_AMODE_SINGLE
    //   DMA_CCR_DST_AMODE_POST_INC   
    //   DMA_CCR_DST_AMODE_CONST 
    //    
    DMA_CCR_DST_AMODE_CONST,

    // dma priority level
    // valid values are
    //   DMA_PRIORITY           : high priority
    //   FALSE                  : low priority
    //
    DMA_PRIORITY,

    // synch mode
    // valid values are
    //   DMA_SYNCH_TRIGGER_NONE : dma is asynchronous
    //   DMA_SYNCH_TRIGGER_DST  : dma to synchronize on destination
    //   DMA_SYNCH_TRIGGER_SRC  : dma to synchronize on source
    //
    DMA_SYNCH_TRIGGER_NONE,

    // synch mode
    // valid values are
    //   DMA_SYNCH_NONE         : no synch mode
    //   DMA_SYNCH_FRAME        : write/read entire frames
    //   DMA_SYNCH_BLOCK        : write/read entire blocks
    //   DMA_SYNCH_PACKET       : write/read entire packets
    //
    DMA_SYNCH_FRAME,

    // dma interrupt mask
    // may be any combination of:
    //   DMA_CICR_PKT_IE
    //   DMA_CICR_BLOCK_IE
    //   DMA_CICR_LAST_IE
    //   DMA_CICR_FRAME_IE
    //   DMA_CICR_HALF_IE
    //   DMA_CICR_DROP_IE
    //   DMA_CICR_MISALIGNED_ERR_IE
    //   DMA_CICR_SUPERVISOR_ERR_IE
    //   DMA_CICR_SECURE_ERR_IE
    //   DMA_CICR_TRANS_ERR_IE
    0 //DMA_CICR_FRAME_IE

};


DmaConfigInfo_t RxDmaSettings = {

    // element width
    // valid values are:
    //  DMA_CSDP_DATA_TYPE_S8
    //  DMA_CSDP_DATA_TYPE_S16
    //  DMA_CSDP_DATA_TYPE_S32
    //
    DMA_CSDP_DATATYPE_S32,

    // source element index
    // valid values are: 
    //  [-32768, 32768]
    //
    0,

    // source frame index
    // valid values are: 
    //  [-2147483648, 2147483647] : non-packet mode
    //  [-32768, 32768] : packet mode
    //    
    0,

    // source addressing mode
    // valid values are:
    //   DMA_CCR_SRC_AMODE_DOUBLE
    //   DMA_CCR_SRC_AMODE_SINGLE
    //   DMA_CCR_SRC_AMODE_POST_INC   
    //   DMA_CCR_SRC_AMODE_CONST 
    // 
    DMA_CCR_SRC_AMODE_CONST,

    // destination element index
    // valid values are: 
    //  [-32768, 32767]
    //
    0,

    // destination frame index
    // valid values are: 
    //  [-2147483648, 2147483647] : non-packet mode
    //  [-32768, 32768] : packet mode
    // 
    0,

    // destination addressing mode
    // valid values are:
    //   DMA_CCR_DST_AMODE_DOUBLE
    //   DMA_CCR_DST_AMODE_SINGLE
    //   DMA_CCR_DST_AMODE_POST_INC   
    //   DMA_CCR_DST_AMODE_CONST 
    //    
    DMA_CCR_DST_AMODE_POST_INC,

    // dma priority level
    // valid values are
    //   DMA_PRIORITY           : high priority
    //   FALSE                  : low priority
    //
    DMA_PRIORITY,

    // synch mode
    // valid values are
    //   DMA_SYNCH_TRIGGER_NONE : dma is asynchronous
    //   DMA_SYNCH_TRIGGER_DST  : dma to synchronize on destination
    //   DMA_SYNCH_TRIGGER_SRC  : dma to synchronize on source
    //
    DMA_SYNCH_TRIGGER_SRC,

    // synch mode
    // valid values are
    //   DMA_SYNCH_NONE         : no synch mode
    //   DMA_SYNCH_FRAME        : write/read entire frames
    //   DMA_SYNCH_BLOCK        : write/read entire blocks
    //   DMA_SYNCH_PACKET       : write/read entire packets
    //
    DMA_SYNCH_FRAME,

    // dma interrupt mask
    // may be any combination of:
    //   DMA_CICR_PKT_IE
    //   DMA_CICR_BLOCK_IE
    //   DMA_CICR_LAST_IE
    //   DMA_CICR_FRAME_IE
    //   DMA_CICR_HALF_IE
    //   DMA_CICR_DROP_IE
    //   DMA_CICR_MISALIGNED_ERR_IE
    //   DMA_CICR_SUPERVISOR_ERR_IE
    //   DMA_CICR_SECURE_ERR_IE
    //   DMA_CICR_TRANS_ERR_IE
    0 //DMA_CICR_FRAME_IE

};


void CSDIOControllerBase::SDIO_InitDMA(void)
{
    m_RxDmaInfo = NULL; // init RX DMA not used
    m_hRxDmaChannel = NULL;
    
    // allocate dma channel
    m_hRxDmaChannel = DmaAllocateChannel(DMA_TYPE_SYSTEM);
    if (m_hRxDmaChannel != NULL)
    {
      DEBUGMSG(ZONE_INIT, (L"OMAP35XX RX DMA enabled\n"));
      
      m_RxDmaInfo = (DmaDataInfo_t *)LocalAlloc(LMEM_ZEROINIT,sizeof(DmaDataInfo_t));
      if(!m_RxDmaInfo)
      {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"ERROR: SDIO_InitDMA: "
                L"Cannot allocate DMA on RX\r\n"
                ));
        goto cleanUp;
      }

      // Disable DMA interrupts for Transmit by passing in NULL for the event parameter
      if (DmaEnableInterrupts(m_hRxDmaChannel, NULL) == FALSE)
      {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"ERROR: SDIO_InitDMA: "
          L"Failed to disable DMA TX interrupt\r\n"
               ));
        goto cleanUp;
      }
    }
    
    m_TxDmaInfo = NULL; // init TX DMA as not used
    
    // allocate dma channel
    m_hTxDmaChannel = DmaAllocateChannel(DMA_TYPE_SYSTEM);
    if (m_hTxDmaChannel != NULL)
    {
      m_TxDmaInfo = (DmaDataInfo_t *)LocalAlloc(LMEM_ZEROINIT,sizeof(DmaDataInfo_t));
      if(!m_TxDmaInfo)
      {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"ERROR: SDIO_InitDMA: "
                L"Cannot allocate DMA on TX\r\n"
                ));
        goto cleanUp;
      }

      m_TxDmaInfo->pSrcBuffer = NULL;
      m_TxDmaInfo->pDstBuffer = NULL;
      m_TxDmaInfo->PhysAddrSrcBuffer = 0;
      m_TxDmaInfo->PhysAddrDstBuffer = 0;
    
      // Disable DMA interrupts for Transmit by passing in NULL for the event parameter
      if (DmaEnableInterrupts(m_hTxDmaChannel, NULL) == FALSE)
      {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"ERROR: SDIO_InitDMA: "
          L"Failed to disable DMA TX interrupt\r\n"
               ));
        goto cleanUp;
      }
    }

cleanUp: ;        
}

void CSDIOControllerBase::SDIO_DeinitDMA(void)
{
    if (m_TxDmaInfo)
    {
      DmaStop(m_TxDmaInfo);
      LocalFree(m_TxDmaInfo);
      m_TxDmaInfo = NULL;
    }
    
    if (m_RxDmaInfo)
    {
      DmaStop(m_RxDmaInfo);
      LocalFree(m_RxDmaInfo);
      m_RxDmaInfo = NULL;
    }

    // Free TX DMA channel
    if (m_hTxDmaChannel != NULL) DmaFreeChannel(m_hTxDmaChannel);

    // Free RX DMA channel
    if (m_hRxDmaChannel != NULL) DmaFreeChannel(m_hRxDmaChannel);
}

//------------------------------------------------------------------------------
//
//  Function: SDIO_InitInputDMA (DWORD dwLen)
//  
//  Initialize the input DMA. 
//

void CSDIOControllerBase::SDIO_InitInputDMA(DWORD dwBlkCnt, DWORD dwBlkSize)
{
    DWORD dwChannel, dwAddr;
    if ( m_dwSlot == MMCSLOT_1 ) 
    {
      dwChannel = SDMA_REQ_MMC1_RX;
      dwAddr = SDIO_INPUT_DMA_SOURCE1;
    }
    else if ( m_dwSlot == MMCSLOT_2 ) 
    {
      dwChannel = SDMA_REQ_MMC2_RX;
      dwAddr = SDIO_INPUT_DMA_SOURCE2;
    }
    else
    {
        ASSERT(0);
        return;
    }
    
    DmaConfigure (m_hRxDmaChannel,
            &RxDmaSettings, dwChannel, m_RxDmaInfo);

    // set up for Rx buffer as 4 frames
    DmaSetElementAndFrameCount (m_RxDmaInfo,
             (UINT16)(dwBlkSize/4), 
             (UINT16)(dwBlkCnt));
    DmaSetDstBuffer (m_RxDmaInfo,
             m_pDmaBuffer,
             m_pDmaBufferPhys.LowPart);
    DmaSetSrcBuffer(m_RxDmaInfo,
             (UINT8 *)&m_pbRegisters->MMCHS_DATA,
             dwAddr);

}

//------------------------------------------------------------------------------
//
//  Function: SDIO_StartInputDMA ()
//  
//  Start the output DMA. 
//

void CSDIOControllerBase::SDIO_StartInputDMA()
{
    PreDmaActivation(DMA_RX);
    DmaStart(m_RxDmaInfo);
}


//------------------------------------------------------------------------------
//
//  Function: SDIO_StopInputDMA ()
//  
//  Stop the input DMA. 
//

void CSDIOControllerBase::SDIO_StopInputDMA()
{
    if (m_RxDmaInfo)
    {
      DmaStop(m_RxDmaInfo);
      PostDmaDeactivation(DMA_RX);
    }
}

//------------------------------------------------------------------------------
//
//  Function: SDIO_InitOutputDMA (DWORD dwLen)
//  
//  Initialize the output DMA. 
//

void CSDIOControllerBase::SDIO_InitOutputDMA(DWORD dwBlkCnt, DWORD dwBlkSize)
{
    DWORD dwChannel, dwAddr;
    if ( m_dwSlot == MMCSLOT_1 ) 
    {
      dwChannel = SDMA_REQ_MMC1_TX;
      dwAddr = SDIO_OUTPUT_DMA_DEST1;
    }
    else if ( m_dwSlot == MMCSLOT_2 ) 
    {
      dwChannel = SDMA_REQ_MMC2_TX;
      dwAddr = SDIO_OUTPUT_DMA_DEST2;
    }
    else return;
    
    DmaConfigure (m_hTxDmaChannel,
            &TxDmaSettings, dwChannel, m_TxDmaInfo);
    
    DmaSetElementAndFrameCount (m_TxDmaInfo,
             (UINT16)(dwBlkSize/4), 
             (UINT16)(dwBlkCnt));
    DmaSetSrcBuffer (m_TxDmaInfo,
              m_pDmaBuffer,
              m_pDmaBufferPhys.LowPart);
    DmaSetDstBuffer(m_TxDmaInfo,
              (UINT8 *)&m_pbRegisters->MMCHS_DATA,
              dwAddr);
}

//------------------------------------------------------------------------------
//
//  Function: SDIO_StartOutputDMA ()
//  
//  Stop the output DMA. 
//

void CSDIOControllerBase::SDIO_StartOutputDMA()
{
    PreDmaActivation(DMA_TX);
    DmaStart(m_TxDmaInfo);
}


//------------------------------------------------------------------------------
//
//  Function: SDIO_StopOutputDMA ()
//  
//  Stop the input DMA. 
//

void CSDIOControllerBase::SDIO_StopOutputDMA()
{
    if (m_TxDmaInfo)
    {
      DmaStop(m_TxDmaInfo);
      PostDmaDeactivation(DMA_TX);
    }
}

VOID CSDIOControllerBase::DumpDMARegs(int inCh)
{
}
#endif

//------------------------------------------------------------------------------
//
//  Function:  CopyDVFSHandles
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
void CSDIOControllerBase::CopyDVFSHandles(UINT processId, HANDLE hAckEvent, HANDLE hActivityEvent)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (
        L"+CopyDVFSHandles(0x%08X, 0x%08X)\r\n", 
        hAckEvent, hActivityEvent
    ));
    
    // make copies of event handles
    DuplicateHandle((HANDLE)processId, hAckEvent, 
        GetCurrentProcess(), &hDVFSAckEvent, 
        0, FALSE, DUPLICATE_SAME_ACCESS
        );

    DuplicateHandle((HANDLE)processId, hActivityEvent, 
        GetCurrentProcess(), &hDVFSActivityEvent, 
        0, FALSE, DUPLICATE_SAME_ACCESS
        );

    DEBUGMSG(SDCARD_ZONE_FUNC, (
        L"-CopyDVFSHandles()\r\n"
    ));
}


//------------------------------------------------------------------------------
//
//  Function:  CheckAndHaltAllDma
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
VOID CSDIOControllerBase::CheckAndHaltAllDma(BOOL bHalt)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (
        L"+CheckAndHaltAllDma(%d)\r\n", 
        bHalt
        ));

    EnterCriticalSection(&csDVFS);
    
    // Need to pause dma's where applicable
    if (nActiveDmaCount == 0 && bHalt == TRUE)
        {
        SetEvent(hDVFSAckEvent);
        }  
    
    LeaveCriticalSection(&csDVFS);

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CheckAndHaltAllDma()\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  PreDmaActivation
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
void CSDIOControllerBase::PreDmaActivation(UINT fTxRx)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (
        L"+PreDmaActivation(%x)\r\n", fTxRx
    ));

    // this operation needs to be atomic to handle a corner case
    EnterCriticalSection(&csDVFS);
    
    // check and wait for DVFS activity to complete
    if (bDVFSActive == TRUE)
        {
        WaitForSingleObject(hDVFSActivityEvent, INFINITE);
        }
    InterlockedIncrement(&nActiveDmaCount);

    if (fTxRx == DMA_TX)
        {
        bTxDmaActive = TRUE;
        }
    else
        {
        bRxDmaActive = TRUE;
        }

    LeaveCriticalSection(&csDVFS);

    DEBUGMSG(SDCARD_ZONE_FUNC, (
        L"-PreDmaActivation()\r\n"
        ));
}

//------------------------------------------------------------------------------
//
//  Function:  PostDmaDeactivation
//
//  Called by dma read/write routines to synchronize dma activity
//  with current DVFS transitions.
//
void CSDIOControllerBase::PostDmaDeactivation(UINT fTxRx)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (
        L"+PostDmaDeactivation(%d)\r\n", fTxRx
    ));
    
    ASSERT(nActiveDmaCount > 0);

    // this operation needs to be atomic to handle a corner case
    EnterCriticalSection(&csDVFS);
    
    // check if all dma's are inactive and signal ack event if so
    InterlockedDecrement(&nActiveDmaCount);
    if (bDVFSActive == TRUE && nActiveDmaCount <= 0)
        {
        SetEvent(hDVFSAckEvent);
        }

    if (fTxRx == DMA_TX)
        {
        bTxDmaActive = FALSE;
        }
    else
        {
        bRxDmaActive = FALSE;
        }

    LeaveCriticalSection(&csDVFS);

    DEBUGMSG(SDCARD_ZONE_FUNC, (
        L"-PostDmaDeactivation()\r\n"
        ));
}


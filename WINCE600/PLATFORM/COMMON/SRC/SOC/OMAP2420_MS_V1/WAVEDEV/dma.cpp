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

#include <string.h>
#include "wavemain.h"
#include "ti_constants.h"
#include "ceddk.h"
#include "omap2420_mcbsp.h"
#include "dma_arb.h"
//#undef DEBUGMSG
//#define DEBUGMSG(x, y) RETAILMSG(1,y)
//#define DEBUG 1

//------------------------------------------------------------------------------
//
//  DMA definitions
//

// looping number in case DMA fails
#define DMA_SAFETY_LOOP_NUM     100

#if defined(DEBUG)
static void DumpDMA_LC(PTSTR szChannel, OMAP2420_DMA_REGS *pCh);
#endif

//------------------------------------------------------------------------------
//
//  Function: OMAP2420DMAContext ()

OMAP2420DMAContext::OMAP2420DMAContext()
{
    m_hCont = NULL;
    m_pInDMAReg = NULL;
    m_pOutDMAReg = NULL;
    m_paAudioDMA.LowPart = 0;
    m_paAudioDMA.HighPart = 0;
    m_pbDMABufOut=NULL;
    m_pbDMABufIn=NULL;
}


//------------------------------------------------------------------------------
//
//  Function: HWMapDMAMemory ()
//  
//  Map DMA related H/W registers and allocate continous DMA pages into the 
//  local address space.
//

BOOL
OMAP2420DMAContext::HWMapDMAMemory(DWORD dwSize)
{
    PBYTE pbDMATemp;
    PHYSICAL_ADDRESS pa;

    /* find the system dma controller */
    uint bytesToAlloc = 0;
    uint err = DMA_ControllerEnum(NULL,&bytesToAlloc);
    if (err!=DMAERR_NOTENOUGHSPACE)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"Could not get size of enumeration of platform dma controllers.\r\n"
        ));
        return FALSE;
    }
    DMACONTROLLER *pContList = (DMACONTROLLER *)malloc(bytesToAlloc);
    if (pContList == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"Could not malloc dma controllers.\r\n"
        ));
        return FALSE;
    }
    err = DMA_ControllerEnum(pContList,&bytesToAlloc);
    if (err)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"Could not enumerate platform dma controllers.\r\n"
        ));
        free(pContList);
        return FALSE;
    }
    /* find the system dma controller */
    uint numEntries = bytesToAlloc/sizeof(DMACONTROLLER);
    uint i;
    for(i=0;i<numEntries;i++)
    {
        if (!_stricmp(pContList[i].mName,"System"))
            break;
    }
    if (i==numEntries)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"Could not find \"System\" dma controller.\r\n"
        ));
        free(pContList);
        return FALSE;
    }
    uint controllerId = pContList[i].mSysId;
    free(pContList);

    /* open the controller now */
    err = DMA_ControllerOpen(controllerId, &m_hCont);
    if (err)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"Could not open \"System\" dma controller.\r\n"
        ));
        return FALSE;
    }

    /* we need to allocate our input and output channels */
    uint chanIO = (1<<AUDIO_INPUT_DMA_CHANNEL) | (1<<AUDIO_OUTPUT_DMA_CHANNEL);
    err = DMA_ControllerAcquireChannels(m_hCont, 2, &chanIO);
    if (err)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"Could not acquire audio i/o channels\r\n"
        ));
        DMA_ControllerClose(m_hCont);
        m_hCont = NULL;
        return FALSE;
    }

    /* if we get here, then we have ownership of the two dma channels we need */

    // map DMA registers into driver address space
    pa.HighPart= 0;
    pa.LowPart = OMAP2420_SDMA_REGS_PA;
    volatile OMAP2420_SDMA_REGS *pDMA_Regs = (OMAP2420_SDMA_REGS *)MmMapIoSpace(pa, sizeof(OMAP2420_SDMA_REGS), FALSE);
    if (!pDMA_Regs)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"ERROR mapping DMA registers.\n"
        ));
        goto ErrExit;
    }

    // enable input DMA IRQ
    err = DMA_ControllerSet(m_hCont, DMACP_L3IntEnb, 1<< AUDIO_INPUT_DMA_CHANNEL);
    if (err)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"ERROR setting IRQ for L3 input dma channel.\r\n"
        ));
        goto ErrExit;
    }
    // enable output DMA IRQ
    err = DMA_ControllerSet(m_hCont, DMACP_L2IntEnb, 1<< AUDIO_OUTPUT_DMA_CHANNEL);
    if (err)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"ERROR setting IRQ for L2 output dma channel.\r\n"
        ));
        goto ErrExit;        
    }

    // get the channel register pointers
    m_pInDMAReg  = &pDMA_Regs->CHNL_CTRL[AUDIO_INPUT_DMA_CHANNEL];
    m_pOutDMAReg = &pDMA_Regs->CHNL_CTRL[AUDIO_OUTPUT_DMA_CHANNEL];

    // allocate the DMA pages
    DMA_ADAPTER_OBJECT AdapterObject; 
    AdapterObject.ObjectSize = sizeof(AdapterObject); 
    AdapterObject.InterfaceType = Internal; 
    AdapterObject.BusNumber = 0; 
    m_pbDMABufOut = NULL;

    pbDMATemp = (PBYTE)HalAllocateCommonBuffer(&AdapterObject, dwSize, &m_paAudioDMA, FALSE);
    if (!pbDMATemp)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"ERROR mapping DMA memory.\n"
        ));
        goto ErrExit;
    }

    m_pbDMABufOut = m_pbDMABufIn = pbDMATemp;

#ifdef MIXER_CACHEDMEM
    // Map DMA buffer also to cached memory for DMA out
    // Mixer will call cache flush after block is completed
    m_pbDMABufOut = (PBYTE)MmMapIoSpace(m_paAudioDMA, dwSize, TRUE);
    if (!m_pbDMABufOut)
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
            L"ERROR mapping DMA buffer to cached memory, using uncached.\n"
        ));
        m_pbDMABufOut = pbDMATemp;
    }
#endif

    return TRUE;

ErrExit:

    m_pInDMAReg  =NULL;
    m_pOutDMAReg =NULL;
    m_pbDMABufOut=NULL;
    m_pbDMABufIn =NULL;

    if (m_hCont)
    {
        DMA_ControllerClose(m_hCont);
        m_hCont = NULL;
    }

    return FALSE;
}


//------------------------------------------------------------------------------
//
//  Function: CheckDMAStatus ()
//  
//  DMA status verification for debugging
//

#if defined(DEBUG)
static void 
CheckDMAStatus(OMAP2420_DMA_REGS  *pDMAReg, BOOL fEnabled)
{
    DWORD dwCount = 0;

    DWORD dwValConfirm = INREG32(&pDMAReg->DMA4_CCR);
    if (fEnabled) 
    {
        while ( !( dwValConfirm & DMA_CCR_ENABLE ) )
        {
            // Put safety checking in case something is wrong
            if (dwCount++>DMA_SAFETY_LOOP_NUM)
                break;

            dwValConfirm = INREG32(&pDMAReg->DMA4_CCR);
            DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
                L"ERROR DMA safety checking = %d %08X\r\n", fEnabled, dwValConfirm
            ));
        }
    }
    else
    {
        while ( dwValConfirm & DMA_CCR_ENABLE )
        {
            // Put safety checking in case something is wrong
            if (dwCount++>DMA_SAFETY_LOOP_NUM)
                break;

            dwValConfirm = INREG32(&pDMAReg->DMA4_CCR);
            DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWMapDMAMemory: "
                L"ERROR DMA safety checking = %d %08X\r\n", fEnabled, dwValConfirm
            ));
        }
    }
}
#endif  // #if defined(DEBUG)


//------------------------------------------------------------------------------
//
//  Function: HWInitInputDMA ()
//  
//  Initialize the input DMA. 
//

void 
OMAP2420DMAContext::HWInitInputDMA()
{
    OMAP2420_DMA_REGS  *pDMAReg;
    DWORD   dwAddr, dwVal;

    DEBUGMSG(ZONE_FUNCTION, (L"+OMAP2420DMAContext::HWInitInputDMA()\r\n"));

    pDMAReg = m_pInDMAReg;

    // disable the channel, clear the configuration
    SETREG32(&pDMAReg->DMA4_CCR, 0);

    // link this channel to itself to get continuous operation
    OUTREG32(&pDMAReg->DMA4_CLNK_CTRL, AUDIO_INPUT_DMA_CHANNEL);

    // 16 bits scalar, no pack, no burst
    OUTREG32(&pDMAReg->DMA4_CSDP, DMA_CSDP_DATATYPE_16BIT);

    // request number, post incremented destination address, high priority
    dwVal = DMA_CCR_SYNC(AUDIO_INPUT_DMA_REQ) | DMA_CCR_DST_AMODE_POST_INC | DMA_CCR_PRIO;
    OUTREG32(&pDMAReg->DMA4_CCR, dwVal);

    // source address
    OUTREG32(&pDMAReg->DMA4_CSSA, AUDIO_INPUT_DMA_SOURCE);

    // destination address
    dwAddr = m_paAudioDMA.LowPart + AUDIO_DMA_PAGES * AUDIO_DMA_PAGE_SIZE;      
    OUTREG32(&pDMAReg->DMA4_CDSA, dwAddr);

    // interrupt conditions
    dwVal = DMA_CICR_FRAME_IE | DMA_CICR_HALF_IE | DMA_CICR_DROP_IE;
    OUTREG32(&pDMAReg->DMA4_CICR, dwVal);

    // number of samples per frame
    OUTREG32(&pDMAReg->DMA4_CEN, AUDIO_DMA_PAGE_SIZE);

    // number of frames per block
    OUTREG32(&pDMAReg->DMA4_CFN, 1);

    // source frame index and element index
    OUTREG32(&pDMAReg->DMA4_CSFI, 0);
    OUTREG32(&pDMAReg->DMA4_CSEI, 0);

    // destination frame index and element index
    OUTREG32(&pDMAReg->DMA4_CDFI, 0);
    OUTREG32(&pDMAReg->DMA4_CDEI, 0);

#if defined(DEBUG)
    DumpDMA_LC(L"Init Input", pDMAReg);
#endif

    DEBUGMSG(ZONE_FUNCTION, (L"-OMAP2420DMAContext::HWInitInputDMA()\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function: HWInitOutputDMA ()
//  
//  Initialize the output DMA. 
//

void 
OMAP2420DMAContext::HWInitOutputDMA()
{
    OMAP2420_DMA_REGS  *pDMAReg;
    DWORD   dwAddr, dwVal;

    DEBUGMSG(ZONE_FUNCTION, (L"+OMAP2420DMAContext::HWInitOutputDMA()\r\n"));

    pDMAReg = m_pOutDMAReg;

    // disable the channel, clear the configuration
    SETREG32(&pDMAReg->DMA4_CCR, 0);

    // link this channel to itself to get continuous operation
    OUTREG32(&pDMAReg->DMA4_CLNK_CTRL, AUDIO_OUTPUT_DMA_CHANNEL);

    // 16 bits scalar, no pack, no burst
    OUTREG32(&pDMAReg->DMA4_CSDP, DMA_CSDP_DATATYPE_16BIT);

    // request number, post incremented source address, high priority
    dwVal = DMA_CCR_SYNC(AUDIO_OUTPUT_DMA_REQ) | DMA_CCR_SRC_AMODE_POST_INC | DMA_CCR_PRIO;
    OUTREG32(&pDMAReg->DMA4_CCR, dwVal);

    // source address
    dwAddr = m_paAudioDMA.LowPart;     
    OUTREG32(&pDMAReg->DMA4_CSSA, dwAddr);

    // destination address
    OUTREG32(&pDMAReg->DMA4_CDSA, AUDIO_OUTPUT_DMA_DEST);

    // interrupt conditions
    dwVal = DMA_CICR_FRAME_IE | DMA_CICR_HALF_IE | DMA_CICR_DROP_IE;
    OUTREG32(&pDMAReg->DMA4_CICR, dwVal);

    // number of samples per frame
    OUTREG32(&pDMAReg->DMA4_CEN, AUDIO_DMA_PAGE_SIZE);

    // number of frames per block
    OUTREG32(&pDMAReg->DMA4_CFN, 1);

    // source frame index and element index
    OUTREG32(&pDMAReg->DMA4_CSFI, 0);
    OUTREG32(&pDMAReg->DMA4_CSEI, 0);

    // destination frame index and element index
    OUTREG32(&pDMAReg->DMA4_CDFI, 0);
    OUTREG32(&pDMAReg->DMA4_CDEI, 0);

#if defined(DEBUG)
    DumpDMA_LC(L"Init Output", pDMAReg);
#endif

    DEBUGMSG(ZONE_FUNCTION, (L"-OMAP2420DMAContext::WInitOutputDMA()\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function: HWStartInputDMA ()
//  
//  Start the output DMA. 
//

void 
OMAP2420DMAContext::HWStartInputDMA()
{
    OMAP2420_DMA_REGS  *pDMAReg;

    DEBUGMSG(ZONE_FUNCTION, (L"OMAP2420DMAContext::HWStartInputDMA()\r\n"));

    pDMAReg = m_pInDMAReg;

#ifdef PROFILE_MIXER
    GetInputDeviceContext(0)->StartMixerProfiler();
#endif

    // enable the channel
    SETREG32(&pDMAReg->DMA4_CLNK_CTRL, DMA_CLNK_CTRL_ENABLE_LINK);
    SETREG32(&pDMAReg->DMA4_CCR, DMA_CCR_ENABLE);

#if defined(DEBUG)
    DumpDMA_LC(L"Start Input", pDMAReg);
    CheckDMAStatus(pDMAReg, TRUE);
#endif

    HWEnableInputChannel(TRUE);

    DEBUGMSG(ZONE_FUNCTION, (L"-OMAP2420DMAContext::HWStartInputDMA()\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function: HWStopInputDMA ()
//  
//  Stop the input DMA. 
//

void 
OMAP2420DMAContext::HWStopInputDMA()
{
    OMAP2420_DMA_REGS  *pDMAReg;
    DWORD i, dwVal;

    DEBUGMSG(ZONE_FUNCTION, (L"+OMAP2420DMAContext::HWStopInputDMA()\r\n"));

    pDMAReg = m_pInDMAReg;

    // disable the DMA channel link and wait for reading to finish
    CLRREG32(&pDMAReg->DMA4_CLNK_CTRL, DMA_CLNK_CTRL_ENABLE_LINK);
    dwVal = INREG32(&pDMAReg->DMA4_CCR);
    for (i = 0; (i < DMA_SAFETY_LOOP_NUM) && (dwVal & DMA_CCR_RD_ACTIVE); i++)
    {
        Sleep(1);
        dwVal = INREG32(&pDMAReg->DMA4_CCR);
    }

    // disable DMA on the channel
    CLRREG32(&pDMAReg->DMA4_CCR, DMA_CCR_ENABLE);

    // clear pending interrupts
    OUTREG32(&pDMAReg->DMA4_CSR, INREG32(&pDMAReg->DMA4_CSR));
    DMA_ControllerSet(m_hCont, DMACP_L3IntAck, 1 << AUDIO_INPUT_DMA_CHANNEL);

    // disable the rest of the channel
    HWEnableInputChannel(FALSE);

#if defined(DEBUG)
    DumpDMA_LC(L"Stop Input", pDMAReg);
    CheckDMAStatus(pDMAReg, FALSE);
#endif

#ifdef PROFILE_MIXER
    LARGE_INTEGER liTotalTime; 
    LARGE_INTEGER liMixerTime;

    GetInputDeviceContext(0)->StopMixerProfiler(&liTotalTime,&liMixerTime);

    DEBUGMSG(ZONE_DMA, (L"OMAP2420DMAContext::HWStopInputDMA: "
        L"Total capture time: %dms, %dms while mixing\r\n", (DWORD) (liTotalTime.QuadPart), (DWORD) (liMixerTime.QuadPart)
    ));
#endif

    DEBUGMSG(ZONE_FUNCTION, (L"-OMAP2420DMAContext::HWStopInputDMA()\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function: HWStartOutputDMA ()
//  
//  Stop the output DMA. 
//

void 
OMAP2420DMAContext::HWStartOutputDMA()
{
    OMAP2420_DMA_REGS  *pDMAReg;
    
    DEBUGMSG(ZONE_FUNCTION, (L"+OMAP2420DMAContext::HWStartOutputDMA()\r\n"));

#ifdef PROFILE_MIXER
    GetOutputDeviceContext(0)->StartMixerProfiler();
#endif

    pDMAReg = m_pOutDMAReg;

    // enable the channel
    SETREG32(&pDMAReg->DMA4_CLNK_CTRL, DMA_CLNK_CTRL_ENABLE_LINK);
    SETREG32(&pDMAReg->DMA4_CCR, DMA_CCR_ENABLE);

#if defined(DEBUG)
    DumpDMA_LC(L"Start Output", pDMAReg);
    CheckDMAStatus(pDMAReg, TRUE);
#endif

    HWEnableOutputChannel(TRUE);

    DEBUGMSG(ZONE_FUNCTION, (L"-OMAP2420DMAContext::HWStartOutputDMA()\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function: HWStopOutputDMA ()
//  
//  Stop the output DMA. 
//

void 
OMAP2420DMAContext::HWStopOutputDMA()
{
    OMAP2420_DMA_REGS  *pDMAReg;
    DWORD i, dwVal;

    DEBUGMSG(ZONE_FUNCTION, (L"+OMAP2420DMAContext::HWStopOutputDMA()\r\n"));

    pDMAReg = m_pOutDMAReg;

    // disable the DMA channel link and wait for writing to finish
    CLRREG32(&pDMAReg->DMA4_CLNK_CTRL, DMA_CLNK_CTRL_ENABLE_LINK);
    dwVal = INREG32(&pDMAReg->DMA4_CCR);
    for (i = 0; (i < DMA_SAFETY_LOOP_NUM) && (dwVal & DMA_CCR_WR_ACTIVE); i++)
    {
        Sleep(1);
        dwVal = INREG32(&pDMAReg->DMA4_CCR);
    }

    // disable DMA on the channel
    CLRREG32(&pDMAReg->DMA4_CCR, DMA_CCR_ENABLE);

    // clear pending interrupts
    OUTREG32(&pDMAReg->DMA4_CSR, INREG32(&pDMAReg->DMA4_CSR));
    DMA_ControllerSet(m_hCont, DMACP_L2IntAck, 1 << AUDIO_OUTPUT_DMA_CHANNEL);

    // disable the rest of the channel
    HWEnableOutputChannel(FALSE);

#if defined(DEBUG)
    DumpDMA_LC(L"Stop Output", pDMAReg);
    CheckDMAStatus(pDMAReg, FALSE);
#endif

#ifdef PROFILE_MIXER
    LARGE_INTEGER liTotalTime; 
    LARGE_INTEGER liMixerTime;

    GetOutputDeviceContext(0)->StopMixerProfiler(&liTotalTime,&liMixerTime);

    DEBUGMSG(ZONE_DMA,(L"OMAP2420DMAContext::HWStopOutputDMA: "
        L"Total play time: %dms, %dms while mixing\r\n", (DWORD) (liTotalTime.QuadPart), (DWORD) (liMixerTime.QuadPart)
    ));
#endif

    DEBUGMSG(ZONE_FUNCTION, (L"-OMAP2420DMAContext::HWStopOutputDMA()\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function: HWTransferInputBuffers ()
//  
//  Transfer capture buffers. 
//

ULONG 
OMAP2420DMAContext::HWTransferInputBuffers()
{
    ULONG BytesTransferred=0;
    OMAP2420_DMA_REGS  *pDMAReg;
    DWORD dwVal, dwOffset;

    pDMAReg = m_pInDMAReg;

    // read the CSR register, clear interrupt bits
    dwVal = INREG32(&pDMAReg->DMA4_CSR);
    OUTREG32(&pDMAReg->DMA4_CSR, dwVal);
    DMA_ControllerSet(m_hCont, DMACP_L3IntAck, 1 << AUDIO_INPUT_DMA_CHANNEL);

    DEBUGMSG(ZONE_FUNCTION, (L"+OMAP2420DMAContext::HWTransferInputBuffers() %08X\n", dwVal));

    // an error occured
    if (dwVal & DMA_CSR_DROP)
    {
        DEBUGMSG(ZONE_ERROR|ZONE_DMA, (L"OMAP2420DMAContext::HWTransferInputBuffers: "
            L"RX DMA CSR error = 0x%x\r\n", dwVal
        ));
    }

    // make sure this is a HALF or FRAME interrupt
    if (!(dwVal & (DMA_CSR_HALF|DMA_CSR_FRAME)))
    {
        DEBUGMSG(ZONE_DMA, (L"OMAP2420DMAContext::HWTransferInputBuffers: "
            L"NOT input interrupt %08X\n", dwVal
        ));
        return BytesTransferred;
    }

    // if both HALF and FRAME are on, then we lost one of them
    if ((dwVal & (DMA_CSR_HALF|DMA_CSR_FRAME)) == (DMA_CSR_HALF|DMA_CSR_FRAME))
    {
        DEBUGMSG(ZONE_ERROR, (L"OMAP2420DMAContext::HWTransferInputBuffers: "
            L"Input buffer interrupt lost\r\n"
        ));

        // pick HALF or FRAME based on which half of the frame the read pointer is in
        dwOffset = INREG32(&pDMAReg->DMA4_CSAC) - INREG32(&pDMAReg->DMA4_CSSA);
        dwVal = (dwOffset >= AUDIO_DMA_PAGE_SIZE) ? DMA_CSR_HALF : DMA_CSR_FRAME;
    }

    // HALF interrupt, reload page 0
    if (dwVal & DMA_CSR_HALF)
    {
        BytesTransferred += TransferInputBuffer(0);
    }

    // FRAME interrupt, reload page 1
    if (dwVal & DMA_CSR_FRAME)
    {
        BytesTransferred += TransferInputBuffer(1);
    }

    // stop DMA when there's no more data
    if (BytesTransferred == 0)
    {
        StopInputDMA();
    }

    return BytesTransferred;
}

//------------------------------------------------------------------------------
//
//  Function: HWTransferOutputBuffers ()
//  
//  Transfer playback buffers. 
//

ULONG 
OMAP2420DMAContext::HWTransferOutputBuffers()
{
    ULONG BytesTransferred=0;
    OMAP2420_DMA_REGS  *pDMAReg;
    DWORD dwVal, dwOffset;

    pDMAReg = m_pOutDMAReg;

    // read the DMA CSR register, clear interrupt bits.
    dwVal = INREG32(&pDMAReg->DMA4_CSR);
    OUTREG32(&pDMAReg->DMA4_CSR, dwVal);
    DMA_ControllerSet(m_hCont, DMACP_L2IntAck, 1 << AUDIO_OUTPUT_DMA_CHANNEL);

    DEBUGMSG(ZONE_FUNCTION, (L"OMAP2420DMAContext::HWTransferOutputBuffers() %08X\n", dwVal));

    // an error occured
    if (dwVal & DMA_CSR_DROP)
    {
        DEBUGMSG(ZONE_ERROR|ZONE_DMA, (L"OMAP2420DMAContext::HWTransferOutputBuffers: "
            L"TX DMA CSR error = 0x%x\r\n", dwVal
        ));
    }

    // make sure this is a HALF or FRAME interrupt
    if (!(dwVal & (DMA_CSR_HALF|DMA_CSR_FRAME)))
    {
        DEBUGMSG(ZONE_DMA, (L"OMAP2420DMAContext::HWTransferOutputBuffers: "
            L"NOT output interrupt %08X\n", dwVal
        ));
        return BytesTransferred;
    }

    // if both HALF and FRAME are on, then we lost an interrupt
    if ((dwVal & (DMA_CSR_HALF|DMA_CSR_FRAME)) == (DMA_CSR_HALF|DMA_CSR_FRAME))
    {   
        DEBUGMSG(ZONE_ERROR|ZONE_DMA, (L"OMAP2420DMAContext::HWTransferOutputBuffers: "
            L"Ooutput buffer interrupt lost\r\n"
        ));

        // pick HALF or FRAME based on which half of the frame the write pointer is in
        dwOffset = INREG32(&pDMAReg->DMA4_CSAC) - INREG32(&pDMAReg->DMA4_CSSA);
        dwVal = (dwOffset >= AUDIO_DMA_PAGE_SIZE) ? DMA_CSR_HALF : DMA_CSR_FRAME;
    }

    // HALF interrupt, reload page 0
    if (dwVal & DMA_CSR_HALF)
    {
        BytesTransferred += TransferOutputBuffer(0);
    }

    // FRAME interrupt, reload page 1
    if (dwVal & DMA_CSR_FRAME)
    {   
        BytesTransferred += TransferOutputBuffer(1);
    }

    // stop DMA when there's no more data
    if (BytesTransferred == 0)
    {
        StopOutputDMA();
    }

    DEBUGMSG(ZONE_FUNCTION, (L"-OMAP2420DMAContext::HWTransferOutputBuffers() %08X %d\r\n", dwVal, BytesTransferred));

    return BytesTransferred;
}

//------------------------------------------------------------------------------
//
//  Function: DumpDMA_LC ()
//  
//  dump DMA logical channel registers
//

#if defined(DEBUG)
static void 
DumpDMA_LC(PTSTR szChannel, OMAP2420_DMA_REGS *pCh)
{
    DEBUGMSG(ZONE_DMA,(L"AC: %s Channel\r\n", szChannel, GetTickCount()));
    DEBUGMSG(ZONE_DMA,(L"AC:    CSDP:      %08X\r\n", INREG32(&pCh->DMA4_CSDP)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CCR:       %08X\r\n", INREG32(&pCh->DMA4_CCR)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CICR:      %08X\r\n", INREG32(&pCh->DMA4_CICR)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CSR:       %08X\r\n", INREG32(&pCh->DMA4_CSR)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CSSA:      %08X\r\n", INREG32(&pCh->DMA4_CSSA)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CDSA:      %08X\r\n", INREG32(&pCh->DMA4_CDSA)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CEN:       %08X\r\n", INREG32(&pCh->DMA4_CEN)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CFN:       %08X\r\n", INREG32(&pCh->DMA4_CFN)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CSFI:      %08X\r\n", INREG32(&pCh->DMA4_CSFI)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CSEI:      %08X\r\n", INREG32(&pCh->DMA4_CSEI)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CSAC:      %08X\r\n", INREG32(&pCh->DMA4_CSAC)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CDAC:      %08X\r\n", INREG32(&pCh->DMA4_CDAC)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CDEI:      %08X\r\n", INREG32(&pCh->DMA4_CDEI)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CDFI:      %08X\r\n", INREG32(&pCh->DMA4_CDFI)));
    DEBUGMSG(ZONE_DMA,(L"AC:    COLOR:     %08X\r\n", INREG32(&pCh->DMA4_COLOR)));
    DEBUGMSG(ZONE_DMA,(L"AC:    CLNK_CTRL: %08X\r\n", INREG32(&pCh->DMA4_CLNK_CTRL)));
}
#endif

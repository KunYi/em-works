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
// OMAP2420 DMA implementation
//
#pragma once

// McBSP address
#define AUDIO_MCBSP_REGS_PA				OMAP2420_MCBSP2_REGS_PA

// McBSP bits in the PRCM clock registers
#define AUDIO_PRCM_FCLKEN_MCBSP			PRCM_FCLKEN1_CORE_EN_MCBSP2
#define AUDIO_PRCM_ICLKEN_MCBSP			PRCM_ICLKEN1_CORE_EN_MCBSP2

// McBSP I/O register addresses
#define	AUDIO_INPUT_DMA_SOURCE			(AUDIO_MCBSP_REGS_PA + MCBSP_DRR1_REG_OFFSET)
#define	AUDIO_OUTPUT_DMA_DEST			(AUDIO_MCBSP_REGS_PA + MCBSP_DXR1_REG_OFFSET)

// DMA request numbers
#define AUDIO_INPUT_DMA_REQ				SDMA_REQ_MCBSP2_RX
#define AUDIO_OUTPUT_DMA_REQ			SDMA_REQ_MCBSP2_TX

// DMA IRQ numbers
#define AUDIO_INPUT_DMA_IRQ				IRQ_SDMA_3
#define AUDIO_OUTPUT_DMA_IRQ			IRQ_SDMA_2

// DMA channel numbers
#define AUDIO_INPUT_DMA_CHANNEL			OMAP2420_DMA_LCH_AUDIO_RX
#define AUDIO_OUTPUT_DMA_CHANNEL		OMAP2420_DMA_LCH_AUDIO_TX

class OMAP2420DMAContext : public HardwareContext
{
public:
    OMAP2420DMAContext();

    BOOL  HWMapDMAMemory( DWORD dwSize );
    void  HWInitInputDMA( void );
    void  HWInitOutputDMA( void );
    void  HWStartOutputDMA( void );
    void  HWStopOutputDMA( void );
    void  HWStartInputDMA( void );
    void  HWStopInputDMA( void );
    ULONG HWTransferInputBuffers( void );
    ULONG HWTransferOutputBuffers( void );
    PBYTE HWDMAMemoryIn( void ) { return m_pbDMABufIn;  }
    PBYTE HWDMAMemoryOut( void ){ return m_pbDMABufOut; }

    // start/stop data transfer of I/O channels
    virtual void HWEnableInputChannel (BOOL fEnable)=0;
    virtual void HWEnableOutputChannel(BOOL fEnable)=0;

protected:
    HANDLE  m_hCont;

    // Pointers to controllers.
    OMAP2420_DMA_REGS	*m_pInDMAReg;	// input channel registers
    OMAP2420_DMA_REGS	*m_pOutDMAReg;	// output channel registers

    // physical and virtual DMA buffer mapping
    PHYSICAL_ADDRESS m_paAudioDMA;
    PBYTE m_pbDMABufIn;
    PBYTE m_pbDMABufOut;

};



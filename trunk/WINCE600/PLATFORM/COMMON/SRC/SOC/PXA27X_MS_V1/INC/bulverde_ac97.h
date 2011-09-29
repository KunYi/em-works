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
//------------------------------------------------------------------------------
//
//  Header: bulverde_ac97.h
//
//  Defines the AC97 audio controller register.
//
#ifndef __BULVERDE_AC97_H
#define __BULVERDE_AC97_H

#if __cplusplus
    extern "C" 
    {
#endif

//
// Audio controller DMA channel masks.
//
#define DMA_CH_RCV          4
#define DMA_CH_MIC          5
#define DMA_CH_OUT          1

#define DMA_CHMAP_AC97_MIC       8          // DRCMR8  - map dma channel for AC97 microphone
#define DMA_CHMAP_AC97_MDMRCV    9          // DRCMR9  - map dma channel for AC97 modem receive
#define DMA_CHMAP_AC97_MDMOUT   10          // DRCMR10 - map dma channel for AC97 modem out
#define DMA_CHMAP_AC97_RCV      11          // DRCMR11 - map dma channel for AC97 pcm receive
#define DMA_CHMAP_AC97_OUT      12          // DRCMR12 - map dma channel for AC97 pcm output

#define DMAC_AC97AUDIOXMIT  (0x1 << DMA_CH_OUT)
#define DMAC_AC97AUDIORCV   (0x1 << DMA_CH_RCV)
#define DMAC_AC97MIC        (0x1 << DMA_CH_MIC)

#define DMA_AUDIO_INTR      (DMAC_AC97AUDIOXMIT | DMAC_AC97AUDIORCV | DMAC_AC97MIC)
#define DMA_BVDCI_INTR      (0x1 << DMA_CH_CAM)


//------------------------------------------------------------------------------
//  Type: BULVERDE_AC97_REG    
//
//  Defines the AC97 register layout.
//

typedef struct
{
    UINT32    pocr;                  // PCM out control register.
    UINT32    picr;                  // PCM in control register.
    UINT32    mccr;                  // Mic in control register.
    UINT32    gcr;                   // Global Control Register.
    UINT32    posr;                  // PCM out status register.
    UINT32    pisr;                  // PCM in status register.
    UINT32    mcsr;                  // Mic in status register.
    UINT32    gsr;                   // global status register.
    UINT32    car;                   // CODEC access register.
    UINT32    rsvd0[7];
    UINT32    pcdr;                  // audio fifo data register.
    UINT32    rsvd1[7];
    UINT32    mcdr;                  // mic in fifo data register.
    UINT32    rsvd2[39];
    UINT32    mocr;                  // Modem out control register.
    UINT32    rsvd3;
    UINT32    micr;                  // Modem in control register.
    UINT32    rsvd4;
    UINT32    mosr;                  // Modem out status register.
    UINT32    rsvd5;
    UINT32    misr;                  // modem in status register.
    UINT32    rsvd6[9];
    UINT32    modr;                  // Modem fifo data register.
    UINT32    rsvd7[47];
    volatile  UINT32  *v_pCodecBaseAddr[64];	 

} BULVERDE_AC97_REG, *PBULVERDE_AC97_REG;

#if __cplusplus
    }
#endif

#endif 

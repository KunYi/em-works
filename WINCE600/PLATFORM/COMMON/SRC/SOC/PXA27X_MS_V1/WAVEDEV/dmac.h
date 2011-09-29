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
/*++

$Workfile: DMAC.h $

$Date: 8/01/02 7:51a $

Abstract:  
      The PDD (Platform Dependent Driver) is responsible for
      communicating with the audio circuit to start and stop playback
      and/or recording and initialize and deinitialize the circuits.

      This driver is specifically written for the UCB 1400 Audio Codec.

	    
Notes: 



--*/
#ifndef __DMAC_H__
#define __DMAC_H__


//#include "bvd1.h"

//todo: combine DMAC.H and dmacbits.h in 1 file and place it in the INC dir
//#include "DMACBITS.H"
#define DMAC_AC97_AUDIO_RCV_FIFO	0x40500040
#define DMAC_AC97_AUDIO_XMIT_FIFO	0x40500040 //(was 0x40500140)
#define DMAC_AC97_AUDIO_MIC_FIFO	0x40500060

#define DMAC_AC97_RCVAB_CMD_MASK	0x6023C000  //0110 00000 01 000 11 11 0 0000000000000
#define DMAC_AC97_XMITAB_CMD_MASK	0x9063C000	//1001 00000 10 000 11 11 0 0000000000000
#define DMAC_AC97_MICAB_CMD_MASK	0x6023C000 	//0110 00000 01 000 11 11 0 0000000000000

/*
// these should be defined in the kernal
#define DMA_RCV_A_DESCRIPTOR_BASE_PHYSICAL		0x0
#define DMA_RCV_B_DESCRIPTOR_BASE_PHYSICAL		0x0
#define DMA_XMIT_A_DESCRIPTOR_BASE_PHYSICAL		0x0
#define DMA_XMIT_B_DESCRIPTOR_BASE_PHYSICAL		0x0
#define DMA_MIC_A_DESCRIPTOR_BASE_PHYSICAL		0x0
#define DMA_MIC_B_DESCRIPTOR_BASE_PHYSICAL		0x0

#define DMA_RCV_A_DESCRIPTOR_BASE_VIRTUAL		0x0
#define DMA_RCV_B_DESCRIPTOR_BASE_VIRTUAL		0x0
#define DMA_XMIT_A_DESCRIPTOR_BASE_VIRTUAL		0x0
#define DMA_XMIT_B_DESCRIPTOR_BASE_VIRTUAL		0x0
#define DMA_MIC_A_DESCRIPTOR_BASE_VIRTUAL		0x0
#define DMA_MIC_B_DESCRIPTOR_BASE_VIRTUAL		0x0

#define DMA_RCV_A_BUFFER_BASE_PHYSICAL			0x0
#define DMA_RCV_B_BUFFER_BASE_PHYSICAL			0x0
#define DMA_XMIT_A_BUFFER_BASE_PHYSICAL			0x0
#define DMA_XMIT_B_BUFFER_BASE_PHYSICAL			0x0
#define DMA_MIC_A_BUFFER_BASE_PHYSICAL			0x0
#define DMA_MIC_B_BUFFER_BASE_PHYSICAL			0x0

#define DMA_RCV_A_BUFFER_BASE_VIRTUAL			0x0
#define DMA_RCV_B_BUFFER_BASE_VIRTUAL			0x0
#define DMA_XMIT_A_BUFFER_BASE_VIRTUAL			0x0
#define DMA_XMIT_B_BUFFER_BASE_VIRTUAL			0x0
#define DMA_MIC_A_BUFFER_BASE_VIRTUAL			0x0
#define DMA_MIC_B_BUFFER_BASE_PHYSICAL			0x0
*/
#define DESC_ADDRESS_MASK			0xFFFFFFF0
//#define DESC_ADDRESS_STOP_MASK		0xFFFFFFF1 //OOPS: TODO: THIS IS WRONG I THINK, CHECK FOR SURE
#define DESC_ADDRESS_STOP_MASK		(0x1U << 0);
#define FORCE_64BIT_ALIGNMENT		0xFFFFFFFC;
#define FORCE_128BIT_ALIGNMENT		0xFFFFFFF8;



//unit       fifo add   w bi burst       src/tar    drmcr
//audio_rcv  0x40500040 4 11  8,  16, 32 Source  0x4000012c
//audio_xmt  0x40500040 4 11  8,  16, 32 Target  0x40000130
//microphone 0x40500060 4 11  8,  16, 32 Source  0x40000120

#define DMA_MAP_VALID_MASK  (0x1U << 7)  // Request is mapped to a valid channel indicated by DRCMRx(3:0)

//now defined in dmacbits.h
//#define DMA_CH_RCV 4
//#define DMA_CH_MIC 5
//#define DMA_CH_OUT 1


#define DCSR_BUSERRINTR     (0x1U << 0)  // Bus error status bit
#define DCSR_STARTINTR      (0x1U << 1)  // Descriptor fetch status 
#define DCSR_ENDINTR        (0x1U << 2)  // finish status
#define DCSR_STOPINTR       (0x1U << 3)  // stopped status
#define DCSR_REQPEND        (0x1U << 8)  // Request Pending (read-only)
#define DCSR_STARTIRQEN     (0x1U << 21) // Enable the start interrupt (when the descriptor is loaded)
#define DCSR_STOPIRQEN       (0x1U << 29) // Enable the stopped interrupt (when the descriptor is done)
#define DCSR_NOFETCH        (0x1U << 30) // Descriptor fetch mode, 0 = fetch
#define DCSR_RUN            (0x1U << 31) // run, 1=start

#define DCMD_LEN_MASK			0xFFF // mask off the length bits
#define DCMD_LEN_ZERO_MASK		0xFFFFFFFFFFFFF000 //clear the length bits
		
typedef enum  
{
	Mic = DMA_CH_MIC,
	Line= DMA_CH_RCV,
}InputSourceType;

struct DMAC_FRAME_DESCRIPTOR 
{
	unsigned int ddadr;		// address of the next frame descriptor (physical address)
	unsigned int dsadr;		// address of the source  data (physical address)
	unsigned int dtadr;     // address of the destination
	unsigned int dcmd;		// dma command
};


// the dmcd struct is for documentation
struct dcmdRegBits 
{
	unsigned len	     :13;
	unsigned rsv0		 :1;   
	unsigned width		 :2; 
	unsigned size		 :2;
	unsigned endian		 :1;
	unsigned flybyt	 	 :1;
	unsigned flybys	 	 :1;
	unsigned endirqen    :1;
	unsigned startirqen  :1;
	unsigned rsv1        :5;
	unsigned flowtrg     :1;
	unsigned flowsrc     :1;
	unsigned inctrgadd   :1;
	unsigned incsrcadd   :1;
};

union DMACmdReg// allow bitfields or masks
{
	volatile struct dcmdRegBits DcmdReg ;
	volatile DWORD DcmdDword;
} ;


typedef enum {
     PLAY_XMIT,
     RECORD_RCV
} AC97_TRANSFER, *PAC97_TRANSFER;

///////////////////////////////////////////////////////////////////////////////


#endif

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
/******************************************************************************
**
**  COPYRIGHT (C) 2001, 2002 Intel Corporation.
**
**  This software as well as the software described in it is furnished under
**  license and may only be used or copied in accordance with the terms of the
**  license. The information in this file is furnished for informational use
**  only, is subject to change without notice, and should not be construed as
**  a commitment by Intel Corporation. Intel Corporation assumes no
**  responsibility or liability for any errors or inaccuracies that may appear
**  in this document or any software that may be provided in association with
**  this document. 
**  Except as permitted by such license, no part of this document may be 
**  reproduced, stored in a retrieval system, or transmitted in any form or by
**  any means without the express written consent of Intel Corporation. 
**
**  FILENAME:       xllp_dmac.h
**
**  PURPOSE: contains all DMA Controller specific macros, typedefs, and prototypes.
**           Declares no storage.
**                  
**
******************************************************************************/

#ifndef __XLLP_DMAC_H__
#define __XLLP_DMAC_H__

#include "xllp_defs.h"

/**
 * DCSR Register 
 **/
#define XLLP_DMAC_DCSR_BUS_ERR_INTR		 (1U<<0)
#define XLLP_DMAC_DCSR_START_INTR		 (1U<<1)
#define XLLP_DMAC_DCSR_END_INTR			 (1U<<2)
#define XLLP_DMAC_DCSR_STOP_INTR		 (1U<<3)
#define XLLP_DMAC_DCSR_REQ_PEND			 (1U<<8)
#define XLLP_DMAC_DCSR_EOR_INTR			 (1U<<9)
#define XLLP_DMAC_DCSR_CMP_ST 			 (1U<<10)
#define XLLP_DMAC_DCSR_CLR_CMP_ST		 (1U<<24)
#define XLLP_DMAC_DCSR_SET_CMP_ST		 (1U<<25)
#define XLLP_DMAC_DCSR_EOR_STOP_EN		 (1U<<26)
#define XLLP_DMAC_DCSR_EOR_JMP_ENT		 (1U<<27)
#define XLLP_DMAC_DCSR_EOR_IRQ_EN		 (1U<<28)
#define XLLP_DMAC_DCSR_STOP_IRQ_EN		 (1U<<29)
#define XLLP_DMAC_DCSR_NO_DESC_FETCH	 (1U<<30)
#define XLLP_DMAC_DCSR_RUN 		  		 (1U<<31)

/**
 * DCMD Register
 **/
#define XLLP_DMAC_DCMD_LEN				 (1U<<0)
#define XLLP_DMAC_DCMD_WIDTH			 (1U<<14)
#define XLLP_DMAC_DCMD_SIZE				 (1U<<16)
#define XLLP_DMAC_DCMD_FLY_BY_T			 (1U<<19)
#define XLLP_DMAC_DCMD_FLY_BY_S			 (1U<<20)
#define XLLP_DMAC_DCMD_END_IRQ_EN		 (1U<<21)
#define XLLP_DMAC_DCMD_START_IRQ_EN		 (1U<<22)
#define XLLP_DMAC_DCMD_ADDR_MODE		 (1U<<23)
#define XLLP_DMAC_DCMD_CMP_EN			 (1U<<25)
#define XLLP_DMAC_DCMD_FLOW_TRG			 (1U<<28)
#define XLLP_DMAC_DCMD_FLOW_SRC			 (1U<<29)
#define XLLP_DMAC_DCMD_INC_TRG_ADDR		 (1U<<30)
#define XLLP_DMAC_DCMD_INC_SRC_ADDR		 (1U<<31)


/**
 * BIT FIELDS
 **/
#define XLLP_BIT_FIELD_0    0
#define XLLP_BIT_FIELD_1    1
#define XLLP_BIT_FIELD_2    2
#define XLLP_BIT_FIELD_3    3
#define XLLP_BIT_FIELD_4    4
#define XLLP_BIT_FIELD_5    5
#define XLLP_BIT_FIELD_6    6
#define XLLP_BIT_FIELD_7    7
#define XLLP_BIT_FIELD_8    8
#define XLLP_BIT_FIELD_9    9
#define XLLP_BIT_FIELD_10   10
#define XLLP_BIT_FIELD_11   11
#define XLLP_BIT_FIELD_12   12
#define XLLP_BIT_FIELD_13   13
#define XLLP_BIT_FIELD_14   14
#define XLLP_BIT_FIELD_15   15
#define XLLP_BIT_FIELD_16   16
#define XLLP_BIT_FIELD_17   17
#define XLLP_BIT_FIELD_18   18
#define XLLP_BIT_FIELD_19   19
#define XLLP_BIT_FIELD_20   20
#define XLLP_BIT_FIELD_21   21
#define XLLP_BIT_FIELD_22   22
#define XLLP_BIT_FIELD_23   23
#define XLLP_BIT_FIELD_24   24
#define XLLP_BIT_FIELD_25   25
#define XLLP_BIT_FIELD_26   26
#define XLLP_BIT_FIELD_27   27
#define XLLP_BIT_FIELD_28   28
#define XLLP_BIT_FIELD_29   29
#define XLLP_BIT_FIELD_30   30
#define XLLP_BIT_FIELD_31   31

/**
 * DMAC DDADRx bit fields 
 **/
#define XLLP_DMAC_DDADR_STOP		(1U<<0)
#define XLLP_DMAC_DDADR_BREN		(1U<<1)

/**
 * Used in DMA handler definition
 **/
#define XLLP_DMAC_CHANNEL_NUM		32
#define XLLP_DMAC_DRCMR1_NUM		64
#define XLLP_DMAC_DRCMR2_NUM		11

/**
 * Some error codes
 **/
#define XLLP_ERR_BAD_CHANNEL                    -1
#define XLLP_ERR_CHANNEL_NOT_ALLOCATED          -2
#define XLLP_ERR_NO_HANDLER                     -3
#define XLLP_ERR_MEM_ALLOC                      -4
#define XLLP_ERR_BAD_MDL                        -5

/**
 * Max length that can be transferred by a single DMA descriptor
 **/
#define XLLP_DCMD_LEN_MASK					0x1FFFU    
#define XLLP_DMAC_ALIGN_MASK                0xFFFFFFF0U

#define XLLP_ALL_INTERRUPTS_STATUS (XLLP_DMAC_DCSR_BUS_ERR_INTR|\
                                    XLLP_DMAC_DCSR_START_INTR  |\
                                    XLLP_DMAC_DCSR_END_INTR    |\
                                    XLLP_DMAC_DCSR_STOP_INTR   |\
                                    XLLP_DMAC_DCSR_EOR_INTR)

#define XLLP_ALL_INTERRUPTS_SOURCE (XLLP_DMAC_DCSR_STOP_IRQ_EN |\
                                    XLLP_DMAC_DCSR_EOR_IRQ_EN)


/**
 * Mask of all writable bits in DCSR; others must be written as 0
 **/
#define XLLP_DCSR_WRITABLES_MSK    (XLLP_DMAC_DCSR_BUS_ERR_INTR |\
                                    XLLP_DMAC_DCSR_START_INTR   |\
                                    XLLP_DMAC_DCSR_END_INTR     |\
                                    XLLP_DMAC_DCSR_STOP_IRQ_EN  |\
                                    XLLP_DMAC_DCSR_NO_DESC_FETCH|\
									XLLP_DMAC_DCSR_EOR_INTR     |\
                                    XLLP_DMAC_DCSR_RUN         )

/**
 * DMA Descriptor
 **/
typedef struct 
{
 	XLLP_VUINT32_T   DDADR;  // descriptor address reg
    XLLP_VUINT32_T   DSADR;  // source address register
    XLLP_VUINT32_T   DTADR;  // target address register
    XLLP_VUINT32_T   DCMD;   // command address register
}XLLP_DMAC_DESCRIPTOR_T, *P_XLLP_DMAC_DESCRIPTOR_T;

/**
 * DMAC Register Definitions
 **/
typedef struct 
{
	XLLP_VUINT32_T DCSR[XLLP_DMAC_CHANNEL_NUM]; /* DMA Control/Status Registers 0-31	*/
    XLLP_VUINT32_T RESERVED0[0x8]; 				/* RESERVED0                        	*/
    XLLP_VUINT32_T DALGN;        				/* DMA Alignment Register 31        	*/
    XLLP_VUINT32_T RESERVED1[0xF]; 				/* RESERVED0                        	*/
    XLLP_VUINT32_T DRQSR0;       				/* DMA DREQ(0) Status Register      	*/
    XLLP_VUINT32_T DRQSR1;       				/* DMA DREQ(1) Status Register      	*/
    XLLP_VUINT32_T RESERVED2[0x2]; 				/* RESERVED1                        	*/
    XLLP_VUINT32_T DINT;         				/* DMA Interrupt Register           	*/
    XLLP_VUINT32_T RESERVED3[0x3]; 				/* RESERVED2                        	*/
    XLLP_VUINT32_T DRCMR1[XLLP_DMAC_DRCMR1_NUM];/* Request to Channel Map for DREQ 0-63	*/
	XLLP_DMAC_DESCRIPTOR_T DDG[XLLP_DMAC_CHANNEL_NUM]; /* DMA Desc Group for channel 0-31 */
    XLLP_VUINT32_T RESERVED4[0x340]; /* RESERVED3                               		*/
    XLLP_VUINT32_T DRCMR2[XLLP_DMAC_DRCMR2_NUM];/* Request to Channel Map 64-67			*/
    XLLP_VUINT32_T RESERVED5[0x3FBBD]; 			/* RESERVED4                            */
} XLLP_DMAC_T, *P_XLLP_DMAC_T;


/**
 *  DMAC peripheral device width
 **/
typedef enum
{
    XLLP_DMAC_WIDTH_0 = 0,
    XLLP_DMAC_WIDTH_8,
    XLLP_DMAC_WIDTH_16,
    XLLP_DMAC_WIDTH_32
}XLLP_DMAC_DEVICE_WIDTH_T;

/**
 * DMAC peripheral burst size
 **/
typedef enum
{
    XLLP_DMAC_RESERVED_SIZE = 0,
    XLLP_DMAC_BURSTSIZE_8,
    XLLP_DMAC_BURSTSIZE_16,
    XLLP_DMAC_BURSTSIZE_32
}XLLP_DMAC_DEVICE_BURSTSIZE_T;

/**
 * DMAC channels
 **/
typedef enum
{
    XLLP_DMAC_CHANNEL_0 = 0,
    XLLP_DMAC_CHANNEL_1,
    XLLP_DMAC_CHANNEL_2,
    XLLP_DMAC_CHANNEL_3,
    XLLP_DMAC_CHANNEL_4,
    XLLP_DMAC_CHANNEL_5,
    XLLP_DMAC_CHANNEL_6,
    XLLP_DMAC_CHANNEL_7,
    XLLP_DMAC_CHANNEL_8,
    XLLP_DMAC_CHANNEL_9,
    XLLP_DMAC_CHANNEL_10,
    XLLP_DMAC_CHANNEL_11,
    XLLP_DMAC_CHANNEL_12,
    XLLP_DMAC_CHANNEL_13,
    XLLP_DMAC_CHANNEL_14,
    XLLP_DMAC_CHANNEL_15,
    XLLP_DMAC_CHANNEL_16,
    XLLP_DMAC_CHANNEL_17,
    XLLP_DMAC_CHANNEL_18,
    XLLP_DMAC_CHANNEL_19,
    XLLP_DMAC_CHANNEL_20,
    XLLP_DMAC_CHANNEL_21,
    XLLP_DMAC_CHANNEL_22,
    XLLP_DMAC_CHANNEL_23,
    XLLP_DMAC_CHANNEL_24,
    XLLP_DMAC_CHANNEL_25,
    XLLP_DMAC_CHANNEL_26,
    XLLP_DMAC_CHANNEL_27,
    XLLP_DMAC_CHANNEL_28,
    XLLP_DMAC_CHANNEL_29,
    XLLP_DMAC_CHANNEL_30,
    XLLP_DMAC_CHANNEL_31
}XLLP_DMAC_CHANNEL_T, *P_XLLP_DMAC_CHANNEL_T;

#define XLLP_INVALID_DMA_CHANNEL    0xFF

/**
 * DMAC DEVICE DRCMR ID's
 **/
typedef enum
{
    XLLP_DMAC_DREQ0 = 0,  	/* DREQ 0, Companion Chip request 0 */
    XLLP_DMAC_DREQ1,  	 	/* DREQ 1, Companion Chip request 1 */
    XLLP_DMAC_I2S_RX,
    XLLP_DMAC_I2S_TX,
    XLLP_DMAC_BTUART_RX,
    XLLP_DMAC_BTUART_TX,
    XLLP_DMAC_FFUART_RX,
    XLLP_DMAC_FFUART_TX,
    XLLP_DMAC_AC97_MIC,
    XLLP_DMAC_AC97_MODEM_RX,
    XLLP_DMAC_AC97_MODEM_TX,
    XLLP_DMAC_AC97_AUDIO_RX,
    XLLP_DMAC_AC97_AUDIO_TX,
    XLLP_DMAC_SSP_1_RX,
    XLLP_DMAC_SSP_1_TX,
    XLLP_DMAC_SSP_2_RX,
    XLLP_DMAC_SSP_2_TX,
    XLLP_DMAC_ICP_RX,
    XLLP_DMAC_ICP_TX,
    XLLP_DMAC_STUART_RX,
    XLLP_DMAC_STUART_TX,
    XLLP_DMAC_MMC_RX,
    XLLP_DMAC_MMC_TX,
    XLLP_DMAC_RESERVED0,	/* RESERVED DRCMR 23*/
    XLLP_DMAC_USB_ENDPOINT_0,
    XLLP_DMAC_USB_ENDPOINT_A,
    XLLP_DMAC_USB_ENDPOINT_B,
    XLLP_DMAC_USB_ENDPOINT_C,
    XLLP_DMAC_USB_ENDPOINT_D,
    XLLP_DMAC_USB_ENDPOINT_E,
    XLLP_DMAC_USB_ENDPOINT_F,
    XLLP_DMAC_USB_ENDPOINT_G,
    XLLP_DMAC_USB_ENDPOINT_H,
    XLLP_DMAC_USB_ENDPOINT_I,
    XLLP_DMAC_USB_ENDPOINT_J,
    XLLP_DMAC_USB_ENDPOINT_K,
    XLLP_DMAC_USB_ENDPOINT_L,
    XLLP_DMAC_USB_ENDPOINT_M,
    XLLP_DMAC_USB_ENDPOINT_N,
    XLLP_DMAC_USB_ENDPOINT_P,
    XLLP_DMAC_USB_ENDPOINT_Q,
    XLLP_DMAC_USB_ENDPOINT_R,
    XLLP_DMAC_USB_ENDPOINT_S,
    XLLP_DMAC_USB_ENDPOINT_T,
    XLLP_DMAC_USB_ENDPOINT_U,
    XLLP_DMAC_USB_ENDPOINT_V,
    XLLP_DMAC_USB_ENDPOINT_W,
    XLLP_DMAC_USB_ENDPOINT_X,
    XLLP_DMAC_BASEBAND_1_RX,
    XLLP_DMAC_BASEBAND_1_TX,
    XLLP_DMAC_BASEBAND_2_RX,
    XLLP_DMAC_BASEBAND_2_TX,
    XLLP_DMAC_BASEBAND_3_RX,
    XLLP_DMAC_BASEBAND_3_TX,
    XLLP_DMAC_BASEBAND_4_RX,
    XLLP_DMAC_BASEBAND_4_TX,
    XLLP_DMAC_BASEBAND_5_RX,
    XLLP_DMAC_BASEBAND_5_TX,
    XLLP_DMAC_BASEBAND_6_RX,
    XLLP_DMAC_BASEBAND_6_TX,
    XLLP_DMAC_BASEBAND_7_RX,
    XLLP_DMAC_BASEBAND_7_TX,
    XLLP_DMAC_USIM_RX,
    XLLP_DMAC_USIM_TX,          // DRCMR63
    XLLP_DMAC_MEMORY_STICK_RX,
    XLLP_DMAC_MEMORY_STICK_TX,
    XLLP_DMAC_SSP_3_RX,
    XLLP_DMAC_SSP_3_TX,	        // DRCMR67
    XLLP_DMAC_QCI_RX1,          // DRCMR68
    XLLP_DMAC_QCI_RX2,
    XLLP_DMAC_QCI_RX3,          // DRCMR70
    XLLP_DMAC_MEM2MEM_MOVE = 99  /* RESERVED for Memory to Memory moves */
}XLLP_DMAC_DRCMR_T, XLLP_DMAC_DEVICE_T;


/**
 * DMAC Interrupts
 **/
typedef enum
{
    XLLP_DMAC_BUS_ERR_INT = 0,
    XLLP_DMAC_START_INT,
    XLLP_DMAC_END_INT,
    XLLP_DMAC_STOP_INT,
    XLLP_DMAC_EOR_INT = 9
}XLLP_DMAC_INTERRUPT_T;

/**
 * DMAC Transfer Type
 **/
typedef enum
{
    XLLP_DMAC_TRANSFER_MEM_TO_MEM = 0,
    XLLP_DMAC_TRANSFER_IO_TO_MEM,
    XLLP_DMAC_TRANSFER_MEM_TO_IO
}XLLP_DMAC_TRANSFER_TYPE_T;

/**
 * DMAC Transfer Mode
 **/
typedef enum
{
    XLLP_DMAC_DESCRIPTOR_MODE = 0,
    XLLP_DMAC_NO_DESCRIPTOR_MODE
}XLLP_DMAC_TRANSFER_MODE_T;

/**
 * DMAC Descriptor Branching Mode
 **/
typedef enum
{
    XLLP_DMAC_DISABLE_DESC_BRANCH = 0,
    XLLP_DMAC_ENABLE_DESC_BRANCH
}XLLP_DMAC_DESC_BRANCH_T;

/**
 * DMAC Descriptor Enable
 **/
typedef enum
{
    XLLP_DMAC_DESC_RUN_CHANNEL = 0,
    XLLP_DMAC_DESC_STOP_CHANNEL
}XLLP_DMAC_DESC_ENABLE_T;

/**
 * DMA Command
 **/
typedef struct 
{
	XLLP_INT16_T                 aLen;    // Length of transfer in bytes. Max 0x1FFF
	XLLP_DMAC_DEVICE_WIDTH_T	 aWidth;  // Width of on-chip peripheral
	XLLP_DMAC_DEVICE_BURSTSIZE_T aSize;   // Max. burst size of each data transferred
	XLLP_BOOL_T					 aEndian; // Device endianness. 0=Little, 1=Big endian
	XLLP_BOOL_T                  aFlyByT; // Fly-By target bit
	XLLP_BOOL_T 				 aFlyByS; // Fly-By source bit
	XLLP_BOOL_T					 aEndIrqEn;   // End Interrupt Enable. When set, 
											   // generate interrupt when aLen=0
	XLLP_BOOL_T                  aStartIrqEn; // Start Int. enable. When set, generate
											   // interrupt after loading descriptor.
											   // Reserved for No Descriptor Mode Xfer
	XLLP_BOOL_T                  aAddrMode;   // Addressing mode for descriptor comparison
	XLLP_BOOL_T					 aCmpEn;      // Descriptor compare enable bit
	XLLP_BOOL_T					 aFlowTrg;    // Flow control of the target.
	XLLP_BOOL_T					 aFlowSrc;    // Flow control of the source
	XLLP_BOOL_T					 aIncTrgAddr; // Target address increment setting
	XLLP_BOOL_T					 aIncSrcAddr; // Source address increment setting
}XLLP_DMAC_COMMAND_T, *P_XLLP_DMAC_COMMAND_T;

/**
 * DMAC Channel Status
 **/
typedef enum
{
 XLLP_DMAC_STATUS_BUSERRINTR = 0, // Bus error causing int.
 XLLP_DMAC_STATUS_STARTINTR,      // Successful Descriptor fetch int.
 XLLP_DMAC_STATUS_ENDINTR,        // Successful completion int.
 XLLP_DMAC_STATUS_STOPINTR,       // Channel state, 0=Running, 1=Stop
 XLLP_DMAC_STATUS_REQPEND = 8,    // Channel Request pending state.
 								  // 1=Channel has pending req, 0=No Req
 XLLP_DMAC_STATUS_EORINT,         // Indicates status of peripheral Rx data
 XLLP_DMAC_STATUS_CMPST           // Descriptor compare status
}XLLP_DMAC_CHANNEL_STATUS_T, *P_XLLP_DMAC_CHANNEL_STATUS_T;


/**
 * DMAC External Request Pins
 **/
typedef enum
{
   XLLP_DMAC_EXTERNAL_PIN_0 = 0,
   XLLP_DMAC_EXTERNAL_PIN_1
}XLLP_DMAC_EXT_PIN_T;

/**
 * DMAC Data Alignment
 **/
typedef enum
{
   XLLP_DMAC_ALIGNMENT_OFF = 0,
   XLLP_DMAC_ALIGNMENT_ON
}XLLP_DMAC_ALIGNMENT_T;


/**
 * DMAC Channel Allocation Priority
 **/
typedef enum
{
   XLLP_DMAC_CHANNEL_PRIORITY_HIGH = 0,
   XLLP_DMAC_CHANNEL_PRIORITY_MEDIUM,
   XLLP_DMAC_CHANNEL_PRIORITY_LOW,
   XLLP_DMAC_CHANNEL_PRIORITY_LOWEST
}XLLP_DMAC_CHANNEL_PRIORITY_T;


#define XLLP_DMAC_DDADR_RESERVED_MASK	 0xFFFFFFF0
#define XLLP_DMAC_DRCMR_ENABLE     		 0x80
#define XLLP_DMAC_DRCMR_DISABLE    		 0x00


/**
 * Represents a contiguous physical block of memory
 **/
typedef struct _PhysBlock
{
    XLLP_UINT32_T   physicalAds;	// Array of physical addresses of each contigious physical block
    XLLP_UINT32_T   blockLength;	// Length of each contigious block

} PhysBlock, *PPhysBlock;

/**
 * A Memory Descriptor List (MDL) structure
 * This stores information about physical mappings of a buffer
 **/
typedef struct _BufferMdl
{
    XLLP_UINT32_T   numEntries;		// Number of entries of physical blocks representing this buffer
    XLLP_UINT8_T*   bufferPtr;		// Virtual Buffer ptr
    PhysBlock       *physBlock;		// Array of Physical blocks for buffer

} BufferMdl, *PBufferMdl;

/**
 * This allows us to allocate descriptors and properly align them (on 16 byte boundaries) as well
 **/
typedef struct _dmaDescInfo
{
    XLLP_UINT8_T*               memPtr;         // Pointer to this block of memory
    XLLP_UINT32_T               numEntries;     // Number of descriptors 
    P_XLLP_DMAC_DESCRIPTOR_T    pDmaDescList;	// Array of properly aligned DMA descriptors

} DmaDescInfo, *PDmaDescInfo;

/**
 * Declaration for secondary handlers invoked by DMA interrupt handler at interrupt time
 **/
typedef void (*DeviceDmaIntHandler) (void* userContext, XLLP_UINT32_T channelDcsr);


/**
 * Information stored on a per DMA channel
 **/
typedef struct _DmaChannelConfigInfo
{   

    XLLP_DMAC_DRCMR_T	drcmrId;

    DeviceDmaIntHandler	pDeviceHandler;		// Pointer to the client's interrupt handler for this channel.
    void*				pUserContext;		// Pass back to registered handler

} DmaChannelConfigInfo,*PDmaChannelConfigInfo;


/**
 * XLLP DMA Primitive Functions
 **/
void XllpDmacFillLinkedDesc(P_XLLP_DMAC_DESCRIPTOR_T pDesc,
							P_XLLP_DMAC_DESCRIPTOR_T pNextDescPhyAddr, 
							XLLP_DMAC_DESC_ENABLE_T aStopContinue,
							XLLP_DMAC_DESC_BRANCH_T  aBranch, 
							XLLP_UINT32_T aSrcAddr,
							XLLP_UINT32_T aTargetAddr, 
							XLLP_DMAC_COMMAND_T *pCmd);
void XllpDmacCfgChannelDescTransfer(P_XLLP_DMAC_DESCRIPTOR_T pDesc,
									XLLP_DMAC_CHANNEL_T aChannel, 
									XLLP_DMAC_DEVICE_T aDevice,
									XLLP_DMAC_ALIGNMENT_T aLignment);
void XllpDmacCfgChannelNoDescTransfer(XLLP_UINT32_T aSourcAddr,  	
									  XLLP_UINT32_T aTargetAddr, 
									  XLLP_DMAC_COMMAND_T *pCmd,
									  XLLP_DMAC_CHANNEL_T aChannel, 
									  XLLP_DMAC_DEVICE_T aDevice,
									  XLLP_DMAC_ALIGNMENT_T aLignment);
void XllpDmacStartTransfer(XLLP_DMAC_CHANNEL_T aChannel);
void XllpDmacStopTransfer(XLLP_DMAC_CHANNEL_T aChannel);
void XllpDmacAttachDesc(P_XLLP_DMAC_DESCRIPTOR_T pDesc,
			    		P_XLLP_DMAC_DESCRIPTOR_T  pPrevDesc, 
						P_XLLP_DMAC_DESCRIPTOR_T pNextDesc,
			    		P_XLLP_DMAC_DESCRIPTOR_T  pStartDesc, 
						XLLP_DMAC_CHANNEL_T aChannel);
void XllpDmacDetachDesc(P_XLLP_DMAC_DESCRIPTOR_T pDesc,
			    		P_XLLP_DMAC_DESCRIPTOR_T pPrevDesc, 
						P_XLLP_DMAC_DESCRIPTOR_T pNextDesc,
						P_XLLP_DMAC_DESCRIPTOR_T  pStartDesc, 
						XLLP_DMAC_CHANNEL_T aChannel);
void XllpDmacGetChannelStatus(XLLP_DMAC_CHANNEL_T aChannel,
							  XLLP_DMAC_CHANNEL_STATUS_T aStatus, 
							  P_XLLP_DMAC_CHANNEL_STATUS_T pStatus);
void XllpDmacClearChannelStatus(XLLP_DMAC_CHANNEL_T aChannel,
								XLLP_DMAC_CHANNEL_STATUS_T aStatus );
void XllpDmacMapDeviceToChannel(XLLP_DMAC_DRCMR_T aDevice,
					 			XLLP_DMAC_CHANNEL_T aChannel);
void XllpDmacUnMapDeviceToChannel(XLLP_DMAC_DRCMR_T aDevice,
								  XLLP_DMAC_CHANNEL_T aChannel);

// This is only needed when running without the XLLP DMA Engine.
XLLP_STATUS_T XllpDmacInitHandle(P_XLLP_DMAC_T address);

void            XllpDmacHwInit(void);
XLLP_STATUS_T	XllpDmacRegisterDeviceHandler(XLLP_DMAC_CHANNEL_T   dmaChannel,
                                              DeviceDmaIntHandler	dmaIntHandler,
                                              void*                 userContext);
XLLP_STATUS_T   XllpDmacUnregisterDeviceHandler(XLLP_DMAC_CHANNEL_T	dmaChannel);

XLLP_STATUS_T   XllpDmacEnableInterrupts (XLLP_DMAC_CHANNEL_T dmaChannel,XLLP_UINT32_T	interruptBitmask);
XLLP_STATUS_T   XllpDmacDisableInterrupts(XLLP_DMAC_CHANNEL_T dmaChannel,XLLP_UINT32_T	interruptBitmask);

void            XllpDmacInterruptHandler(void);

XLLP_INT32_T    XllpDmacGetRemainingXfrLength(XLLP_DMAC_CHANNEL_T	dmaChannel);
XLLP_UINT32_T   XllpDmacGetPhysicalAds(void*	virtualAddress);

XLLP_STATUS_T   XllpDmacCreateDescriptorListFromMdl(    PBufferMdl                  pBufferMdl,
	                                                    XLLP_DMAC_TRANSFER_TYPE_T   transferType,
                                                        PDmaDescInfo                *pDmaDescInfo,		                                        
	                                                    XLLP_UINT32_T               deviceAddress,
	                                                    XLLP_DMAC_COMMAND_T         *pCmd);

XLLP_STATUS_T   XllpDmacCreateDescriptorList(   XLLP_UINT8_T*               pBuffer,
                                                XLLP_UINT32_T               bufferLength,
	                                            XLLP_DMAC_TRANSFER_TYPE_T   transferType,
                                                PDmaDescInfo                *pDmaDescInfo,		                                        
	                                            XLLP_UINT32_T               deviceAddress,
	                                            XLLP_DMAC_COMMAND_T         *pCmd,
                                                XLLP_DMAC_ALIGNMENT_T       *aLign);
PDmaDescInfo    XllpDmacAllocDmaDescriptorList(XLLP_UINT32_T	numEntries);
void            XllpDmacFreeDescriptorList (PDmaDescInfo   pDmaDescInfo);

PBufferMdl      XllpDmacAllocateMdl(XLLP_UINT32_T numEntries);
void            XllpDmacFreeBufferMdl(	XLLP_UINT8_T    *pBuffer,
	                                    XLLP_UINT32_T	bufferLength,
	                                    PBufferMdl      pBufferMdl);

XLLP_STATUS_T   XllpDmacCreateBufferMdl(XLLP_UINT8_T            *pBuffer,
                                        XLLP_UINT32_T           bufferLength,
                                        PBufferMdl              *pBufferMdl,
                                        XLLP_DMAC_ALIGNMENT_T   *aLign);

XLLP_STATUS_T   XllpDmacGetXfrDone( XLLP_DMAC_CHANNEL_T         dmaChannel,
                                    PBufferMdl                  pBufferMdl,
                                    XLLP_DMAC_TRANSFER_TYPE_T   transferType,
                                    XLLP_UINT32_T               *bytesXferred);

XLLP_STATUS_T   XllpDmacSetupTransfer(  XLLP_DMAC_CHANNEL_PRIORITY_T    aChannelPriority,
                                        XLLP_UINT8_T*                   pUserBuffer,
                                        XLLP_UINT32_T                   xferByteCount,
                                        XLLP_DMAC_TRANSFER_TYPE_T       transferType,
                                        XLLP_UINT32_T                   deviceAddress,
                                        XLLP_DMAC_DEVICE_T              aDeviceDrcmr,
                                        DeviceDmaIntHandler             dmaIntHandler,
                                        void*                           pUserContext,
                                        XLLP_UINT32_T                   intEnableBitmask,
                                        XLLP_UINT32_T                   descBasedXfr,
                                        XLLP_DMAC_COMMAND_T*            pCmd,
                                        XLLP_DMAC_CHANNEL_T*            dmaChannel,
                                        PDmaDescInfo*                   pUserDmaDescInfo,
                                        PBufferMdl*                     pUserBufferMdl);

void XllpDmacEnableInterrupt(void);
void XllpDmacDisableInterrupt(void);
void XllpDmacDumpStatus( XLLP_DMAC_CHANNEL_T aChannel );
void XllpDmacSetEor    ( XLLP_DMAC_CHANNEL_T aChannel );

#endif //__DMAC_H__

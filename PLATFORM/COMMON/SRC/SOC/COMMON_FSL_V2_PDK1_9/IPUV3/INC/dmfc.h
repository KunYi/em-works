//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dmfc.h
//
//  Display Multi FIFO Controller definitions
//
//-----------------------------------------------------------------------------

#ifndef __DMFC_H__
#define __DMFC_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

//*****************************
// dmfcConfigData defines
//*****************************

// PixelPerWord values
#define IPU_DMFC_PPW_C_8BPP                           0
#define IPU_DMFC_PPW_C_16BPP                          1
#define IPU_DMFC_PPW_C_24BPP                          2

// FIFOSize values
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_512x128          0
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_256x128          1
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_128x128          2
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_64x128           3
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128           4
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_16x128           5
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_8x128            6
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_4x128            7

// BurstSize values
#define IPU_DMFC_BURST_SIZE_0_32_WORDS                0
#define IPU_DMFC_BURST_SIZE_0_16_WORDS                1
#define IPU_DMFC_BURST_SIZE_0_8_WORDS                 2
#define IPU_DMFC_BURST_SIZE_0_4_WORDS                 3

// StartSeg values
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_0          0
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_1          1
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_2          2
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_3          3
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_4          4
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_5          5
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_6          6
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_7          7

// WaterMarkEnable values
#define IPU_DMFC_WM_EN_ENABLE                         1
#define IPU_DMFC_WM_EN_DISABLE                        0

#define IDMAC_INVALID_CHANNEL                         255
//------------------------------------------------------------------------------
// Types


typedef struct dmfcConfigDataStruct
{
    UINT8 PixelPerWord;
    UINT16  FrameWidth;
    UINT32  FrameHeight;
    UINT8 FIFOSize;
    UINT8 BurstSize;
    UINT8 StartSeg;
    UINT8 WaterMarkEnable;
    UINT8 WaterMarkClear;
    UINT8 WaterMarkSet;
    DWORD DestChannel;
} dmfcConfigData, *pDmfcConfigData;

//------------------------------------------------------------------------------
// Functions
BOOL DMFCRegsInit();
void DMFCRegsCleanup();
void DMFCEnable(void);
void DMFCDisable(void);
BOOL DMFCConfigure(DWORD dwChannel, pDmfcConfigData pConfigData);
BOOL DMFCFIFOIsFull(DWORD dwChannel);
BOOL DMFCFIFOIsEmpty(DWORD dwChannel);

// Debug helper function
void DMFCDumpRegs();

#ifdef __cplusplus
}
#endif

#endif //__DMFC_H__


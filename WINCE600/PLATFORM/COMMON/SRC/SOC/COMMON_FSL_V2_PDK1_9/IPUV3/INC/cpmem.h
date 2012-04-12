//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  cpmem.h
//
//  Private definitions for the Channel Parameter Memory
//
//-----------------------------------------------------------------------------

#ifndef __CPMEM_H__
#define __CPMEM_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define CPMEM_BNDM_DISABLE                         0
#define CPMEM_BNDM_4_LINES                         1
#define CPMEM_BNDM_8_LINES                         2
#define CPMEM_BNDM_16_LINES                        3
#define CPMEM_BNDM_32_LINES                        4
#define CPMEM_BNDM_64_LINES                        5
#define CPMEM_BNDM_128_LINES                       6
#define CPMEM_BNDM_256_LINES                       7

#define CPMEM_BM_DISABLE                           0
#define CPMEM_BM_8X8                               1
#define CPMEM_BM_16X16                             2

#define CPMEM_ROT_DISABLE                          0
#define CPMEM_ROT_ENABLE                           1

#define CPMEM_HF_DISABLE                           0
#define CPMEM_HF_ENABLE                            1

#define CPMEM_VF_DISABLE                           0
#define CPMEM_VF_ENABLE                            1

#define CPMEM_THE_DISABLE                          0
#define CPMEM_THE_ENABLE                           1

#define CPMEM_CAP_COND_SKIP_LOW                    0
#define CPMEM_CAP_COND_SKIP_HIGH                   1

#define CPMEM_CAE_COND_SKIP_DISABLE                0
#define CPMEM_CAE_COND_SKIP_ENABLE                 1

#define CPMEM_PIXEL_BURST_4                        3
#define CPMEM_PIXEL_BURST_8                        7
#define CPMEM_PIXEL_BURST_16                       15
#define CPMEM_PIXEL_BURST_20                       19
#define CPMEM_PIXEL_BURST_32                       31
#define CPMEM_PIXEL_BURST_40                       39
#define CPMEM_PIXEL_BURST_64                       63
#define CPMEM_PIXEL_BURST_128                      127

#define CPMEM_PFS_NON_INTERLEAVED_YUV444           0
#define CPMEM_PFS_NON_INTERLEAVED_YUV422           1
#define CPMEM_PFS_NON_INTERLEAVED_YUV420           2
#define CPMEM_PFS_PARTIAL_INTERLEAVED_YUV422       3
#define CPMEM_PFS_PARTIAL_INTERLEAVED_YUV420       4
#define CPMEM_PFS_CODE                             5
#define CPMEM_PFS_GENERIC_DATA                     6
#define CPMEM_PFS_INTERLEAVED_RGB                  7
#define CPMEM_PFS_INTERLEAVED_YUV422_Y1U1Y2V1      8
#define CPMEM_PFS_INTERLEAVED_YUV422_Y2U1Y1V1      9
#define CPMEM_PFS_INTERLEAVED_YUV422_U1Y1V1Y2      10
#define CPMEM_PFS_INTERLEAVED_YUV422_U1Y2V1Y1      11

#define CPMEM_ALU_ALPHA_SAME_CHANNEL               0
#define CPMEM_ALU_ALPHA_SEPARATE_CHANNEL           1

#define CPMEM_ID_0                                 0
#define CPMEM_ID_1                                 1
#define CPMEM_ID_2                                 2
#define CPMEM_ID_3                                 3

#define CPMEM_BPP_32                               0
#define CPMEM_BPP_24                               1
#define CPMEM_BPP_18                               2
#define CPMEM_BPP_16                               3
#define CPMEM_BPP_12                               4
#define CPMEM_BPP_8                                5
#define CPMEM_BPP_4                                6

#define CPMEM_DEC_SEL_ADDR_0_TO_15                 0
#define CPMEM_DEC_SEL_ADDR_64_TO_79                1
#define CPMEM_DEC_SEL_ADDR_128_TO_143              2
#define CPMEM_DEC_SEL_ADDR_192_TO_207              3

#define CPMEM_DIM_2D                               0
#define CPMEM_DIM_1D                               1

#define CPMEM_SO_PROGRESSIVE                       0
#define CPMEM_SO_INTERLACED                        1

//------------------------------------------------------------------------------
// Types

// Post-Processing RGB or YUV Format Structure
// For RGB, component0 = red, component1 = green, component2 = blue,
// component3 = alpha
// For YUV, component0 = Y, component1 = U, component2 = V, component3 = NA
typedef struct ppPixelFormatStruct
{
    UINT8 component0_width;
    UINT8 component1_width;
    UINT8 component2_width;
    UINT8 component3_width;
    UINT8 component0_offset;
    UINT8 component1_offset;
    UINT8 component2_offset;
    UINT8 component3_offset;
} ICPixelFormatData, *pICPixelFormatData;

// Structure for programming U and V offsets into CPMEM.
typedef struct CPMEMBufOffsetsStruct
{
    BOOL     bInterleaved;  // TRUE if interleaved, FALSE if planar
    UINT32   uOffset;       // Optional starting relative offset of U data
                            // from beginning of buffer.  Ignored if bInterleaved is TRUE.
    UINT32   vOffset;       // Optional starting relative offset of V data
                            // from beginning of buffer.  Ignored if bInterleaved is TRUE.
    UINT32   interlaceOffset;
} CPMEMBufOffsets, *pCPMEMBufOffsets;

typedef struct CPMEMConfigDataStruct
{
    // These parameters only apply to 
    // interleaved mode.  Ignored for planar mode.
    UINT8                 iBitsPerPixel;
    UINT8                 iDecAddrSelect;
    UINT8                 iAccessDimension;
    UINT8                 iScanOrder;

    UINT8                 iBandMode;
    UINT8                 iBlockMode;
    UINT8                 iRotation90;
    UINT8                 iFlipHoriz;
    UINT8                 iFlipVert;
    UINT8                 iThresholdEnable;
    UINT8                 iCondAccessEnable;
    UINT8                 iCondAccessPolarity;
    UINT8                 iPixelBurst;
    UINT8                 iPixelFormat;
    UINT8                 iAlphaUsed;
    UINT8                 iAXI_Id;

    UINT8                 iThreshold;
    UINT16                iHeight;
    UINT16                iWidth;
    UINT32                iLineStride;
    UINT32                iLineStride_Y;
    UINT32                iLineStride_UV;
    ICPixelFormatData     pixelFormatData;
} CPMEMConfigData, *pCPMEMConfigData;

//------------------------------------------------------------------------------
// Functions

// CPMEM Functions
BOOL CPMEMRegsInit();
void CPMEMRegsCleanup();
void CPMEMWrite(DWORD dwChannel, CPMEMConfigData *pCPMEMData, BOOL bInterleaved);
void CPMEMWriteBuffer(DWORD dwChannel, DWORD bufNum, UINT32* pBufAddr);
void CPMEMWriteOffset(DWORD dwChannel, CPMEMBufOffsets *pOffsetData, BOOL bInterleaved);
void CPMEMWriteBandMode(DWORD dwChannel, DWORD dwBandMode);
void CPMEMSwapBuffer(DWORD dwChannel,BOOL sequential);
BOOL CPMEMIsCurrentField1(DWORD dwChannel);
UINT32 CPMEMReadBufferAddr(DWORD dwChannel, DWORD bufNum);
void CPMEMWriteXScroll(DWORD dwChannel, DWORD dwScrollValue);


//for debug
void CPMEMDumpRegs(DWORD dwChannel);

#ifdef __cplusplus
}
#endif

#endif //__CPMEM_H__


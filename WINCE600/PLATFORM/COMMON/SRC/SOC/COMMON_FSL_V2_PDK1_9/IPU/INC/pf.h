//------------------------------------------------------------------------------
//
//  Copyright (C) 2005-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pf.h
//
//  Public definitions for Postfilter Driver
//
//------------------------------------------------------------------------------
#ifndef __PF_H__
#define __PF_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// Postfilter EOF Event 
#define PF_EOF_EVENT_NAME               L"Pf EOF Event"
#define PF_Y_EVENT_NAME                 L"Pf Y Event"
#define PF_CR_EVENT_NAME                L"Pf Cr Event"

// IOCTL to configure PF
#define PF_IOCTL_CONFIGURE              1
// IOCTL to start PF tasks
#define PF_IOCTL_START                  2
// IOCTL to resume a paused H.264 PF task
#define PF_IOCTL_RESUME                 3
// IOCTL to allocate physical memory
#define PF_IOCTL_ALLOC_PHY_MEM          4
// IOCTL to free physical memory
#define PF_IOCTL_FREE_PHY_MEM           5
// IOCTL to Direct Render
#define PF_IOCTL_START2                 6
// IOCTL to VirtualSetAttributesEx
#define PF_IOCTL_SETATTEX                 7

#define PF_SUCCESS                      0
#define PF_ERR_NOT_RUNNING              1
#define PF_ERR_PAUSE_NOT_ENABLED        2
#define PF_ERR_INVALID_PARAMETER        3

//------------------------------------------------------------------------------
// Types
typedef ULONG PhysicalAddress;
typedef PhysicalAddress *PPhysicalAddress;

// Postfiltering operation mode
typedef enum pfModeEnum
{
    pfMode_Disabled,           // No postfiltering
    pfMode_MPEG4Deblock,       // MPEG4 Deblock only
    pfMode_MPEG4Dering,        // MPEG4 Dering only
    pfMode_MPEG4DeblockDering, // MPEG4 Deblock and Dering
    pfMode_H264Deblock,        // H.264 Deblock
} pfMode;

// Postfiltering Frame Size Structure
typedef struct pfFrameSizeStruct {
    UINT16 width;
    UINT16 height;
} pfFrameSize, *pPfFrameSize;

// Structure for Postfilter buffer.
typedef struct pfBufferStruct
{
    int      size;          // Size of buffer allocated
    UINT32  *yBufPtr;       // Y Buffer pointer in memory.
    UINT32   uOffset;       // Optional starting relative offset of U data
                            // from beginning of buffer. Set to 0 to use default 
                            // calculated based on height and stride.
    UINT32   vOffset;       // Optional starting relative offset of V data
                            // from beginning of buffer. Set to 0 to use default 
                            // calculated based on height and stride.
} pfBuffer, *pPfBuffer;

// structure for VirtualSetAttributesEx
typedef struct pfSetAttributeExDataStruct{
  LPVOID lpvAddress;    // starting address of virtual memory
  DWORD cbSize;         // size of virtual memory
}pfSetAttributeExData, *pPfSetAttributeExData;

// Postfiltering Configuration Data Structure
typedef struct pfConfigDataStruct
{
    // Postfiltering mode
    pfMode           mode;

    // Frame data
    pfFrameSize      frameSize;      // frame size
    UINT32           frameStride;    // stride in bytes

    // Quantization Parameter and Boundary Strength data
    UINT32          *qpBuf;          // Buffer containing, sequentially, quantization
                                     // parameter and boundary strength
                                     // data for the Postfilter operation.
    UINT32           qpSize;         // size of QP and BS buffer
}pfConfigData, *pPfConfigData;

// Structure for Postfilter start parameters.
typedef struct PfStartParamsStruct
{
    pPfBuffer        in;             // Pointer to input buffer address and offsets
    pPfBuffer        out;            // Pointer to output buffer address and offsets
    UINT32           h264_pause_row; // Row to pause at for H.264 mode. 0 to disable pause
    // for bit-match issue
    void* qp_buf;   // pointer to current qb buffer
    void* bs_buf;   // pointer to current bs buffer
    int qp_size;    // qb and bs buffer size
}pfStartParams, *pPfStartParams;

// Structure for Postfilter Memory Parameters
typedef struct {
  UINT            physAddr;
  UINT            userVirtAddr;
  UINT            driverVirtAddr;
  UINT            size;
} pfAllocMemoryParams, *pPfAllocMemoryParams;

//------------------------------------------------------------------------------
// Functions

HANDLE PFOpenHandle(void);
BOOL PFCloseHandle(HANDLE);
void PFConfigure(HANDLE hPF, pPfConfigData pConfigData);
void PFStart(HANDLE hPF, pPfStartParams pStartParms);
void PFStart2(HANDLE hPP, pPfStartParams pStartParms,unsigned int VirtualFlag,unsigned int yOffset); //for Direct Render
BOOL PFSetAttributeEx(HANDLE hPF, pPfSetAttributeExData pData);
DWORD PFResume(HANDLE hPF, unsigned int h264_pause_row);
BOOL PFAllocPhysMem(HANDLE hPF, UINT32 cbSize, pPfAllocMemoryParams pBitsStreamBufMemParams); 
BOOL PFFreePhysMem(HANDLE hPF, pfAllocMemoryParams bitsStreamBufMemParams);

#ifdef __cplusplus
}
#endif

#endif   // __PF_H__

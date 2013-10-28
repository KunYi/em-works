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
//------------------------------------------------------------------------------
//
// Copyright (C) 2004,  MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
// File: PrpClass.h
//
// Common definitions for pre-processor module.
//
//------------------------------------------------------------------------------
#ifndef _PRP_CLASS_H_
#define _PRP_CLASS_H_

#ifndef PRP_MEM_MODE_DLL
#include "PrpBufferManager.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines 
#define PRP_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
    
#define PRP_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#ifdef PRP_MEM_MODE_DLL
#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT                         0
#define ZONEID_DEINIT                       1
#define ZONEID_IOCTL                        2
#define ZONEID_DEVICE                       11
#define ZONEID_INFO                         12
#define ZONEID_FUNCTION                     13
#define ZONEID_WARN                         14
#define ZONEID_ERROR                        15

// Debug zone masks
#define ZONEMASK_INIT                       (1 << ZONEID_INIT)
#define ZONEMASK_DEINIT                     (1 << ZONEID_DEINIT)
#define ZONEMASK_IOCTL                      (1 << ZONEID_IOCTL)
#define ZONEMASK_DEVICE                     (1 << ZONEID_DEVICE)
#define ZONEMASK_INFO                       (1 << ZONEID_INFO)
#define ZONEMASK_FUNCTION                   (1 << ZONEID_FUNCTION)
#define ZONEMASK_WARN                       (1 << ZONEID_WARN)
#define ZONEMASK_ERROR                      (1 << ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT                           DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT                         DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL                          DEBUGZONE(ZONEID_IOCTL)
#define ZONE_DEVICE                         DEBUGZONE(ZONEID_DEVICE)
#define ZONE_INFO                           DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION                       DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN                           DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR                          DEBUGZONE(ZONEID_ERROR)
#endif
#endif

#define PRP_IMAGE_MIN_WIDTH                 32
#define PRP_IMAGE_MIN_HEIGHT                32
#define PRP_IMAGE_MAX_WIDTH                 2040
#define PRP_IMAGE_MAX_HEIGHT                2040

// Maximum resize ratio
#define PRP_MAX_RESIZE_RATIO                8    
#define PRP_RESIZE_THRESHOLD                2       // If greater, averaging resize is used,
                                                    // else, bilinear resize is used.

// Check for resize direction
// Pre-processor only supports downscaling
#define VALID_RESIZE_DIRECTION(in, out) \
    ((in.width >= out.width) && \
    (in.height >= out.height))

// Check for valid resize ratio
#define VALID_RESIZE_RATIO(in, out) \
    ((in.width <= PRP_MAX_RESIZE_RATIO * out.width) && \
    (in.height <= PRP_MAX_RESIZE_RATIO * out.height))

#define MAX_RESIZE_RETRIES                  16
#define MAX_RESIZE_COEFFS                   20

#define MAX_COEFF_VALUE                     8
#define RESIZE_COEFF_WIDTH                  3

// Marcos for U V address calculation
#define PRP_YUV420_CB_PTR(buf, size)        ((UINT32)buf + size)
#define PRP_YUV420_CR_PTR(buf, size)        ((UINT32)buf + size + (size >> 2))

// Nullyfying 3/2 factor during U address calculation.
#define YUV420_U_OFFSET(x)                  (x * 2/3)


//------------------------------------------------------------------------------
// Types
//
// Pre-processing Resize Type
typedef enum {
    prpResize_Viewfinding,
    prpResize_Encoding
} prpResizeChannel;

typedef enum {
    prpResize_Horizontal,
    prpResize_Vertical   
} prpResizeDimension;

typedef enum {
    prpResize_Averaging,
    prpResize_Bilinear
} prpResizeAlgorithm;

// Pre-processing buffer selection
typedef enum {
    prpBuf0Request,
    prpBuf1Request,
    prpMaxBufRequests,
} prpBufferRequest;

// Pre-processing channel state
typedef enum {
    prpChannelStarted,
    prpChannelStopped
} prpChannelState;

typedef struct {
    prpResizeAlgorithm algo; 
    UINT16 tbl_len;
    UINT32 output_bits;
    UINT8 resize_tbl[20];
    UINT8 in;   // Only for debug purpose
    UINT8 out;  // Only for debug purpose
} prpResizeParams, *pPrpResizeParams;


//------------------------------------------------------------------------------
// Classes
class PrpClass {
public:
    PrpClass(void);
    ~PrpClass(void);

    BOOL PrpPowerUp(void);
    void PrpPowerDown(void);
    BOOL PrpEnable(void);
    void PrpDisable(void);

    BOOL PrpConfigure(pPrpConfigData pConfigData);
#ifndef PRP_MEM_MODE_DLL
    BOOL PrpAllocateVfBuffers(ULONG numBuffers, ULONG bufSize);
    BOOL PrpAllocateEncBuffers(ULONG numBuffers, ULONG bufSize);
    BOOL PrpDeleteVfBuffers();
    BOOL PrpDeleteEncBuffers();
    UINT32 PrpGetMaxBuffers();
    BOOL PrpGetVfBufFilled(pPrpBufferData);
    BOOL PrpGetEncBufFilled(pPrpBufferData);
    BOOL PrpPutVfBufIdle(pPrpBufferData);
    BOOL PrpPutEncBufIdle(pPrpBufferData);

#else
    BOOL PrpAddBuffers(pPrpBuffers pPrpBufs);
#endif
    UINT32 PrpGetVfFrameCount();
    UINT32 PrpGetEncFrameCount();
    BOOL PrpStartVfChannel();
    BOOL PrpStopVfChannel();
    BOOL PrpStartEncChannel();
    BOOL PrpStopEncChannel();

private:
    BOOL PrpInit(void);
    void PrpDeinit(void);
    BOOL PrpAlloc(void);
    void PrpDealloc(void);
    BOOL PrpEventsInit(void);
    void PrpEventsDeinit(void);
    BOOL PrpConfigureInput(pPrpConfigData pConfigData);
    BOOL PrpConfigureOutput(pPrpConfigData pConfigData);
    BOOL PrpConfigureCSC(pPrpConfigData pConfigData);
    BOOL PrpConfigureResize(pPrpConfigData pConfigData);
    BOOL PrpResize(UINT32 in, UINT32 out, prpResizeChannel channel, 
        prpResizeDimension dimension);
    UINT16 PrpGetGcd(UINT16 x, UINT16 y);
    BOOL PrpGetCoeff(pPrpResizeParams pParam, UINT16 in, UINT16 out);
    void PrpStoreCoeff(UINT8 * resize_tbl, UINT32 * output_bits, 
        UINT8 index, UINT8 base, UINT8 w1, UINT8 output);
    void PrpStoreCoeff(UINT8 * resize_tbl, UINT32 * output_bits, 
        UINT8 startindex, UINT8 len);
    void PrpFillAveragingCoeff(UINT8 * startptr, UINT8 filter_width);
    void PrpPackToRegister(pPrpResizeParams pParams, 
        prpResizeChannel channel, prpResizeDimension dimension);
#ifndef PRP_MEM_MODE_DLL
    BOOL PrpVfGetBufferReady(prpBufferRequest request);
    BOOL PrpEncGetBufferReady(prpBufferRequest request);
#endif
    static void PrpIntrThread(LPVOID lpParameter);
    void PrpIntrRoutine();
    void PrpDumpRegisters(void);

public:
    HANDLE m_hVfEOFEvent;
    HANDLE m_hEncEOFEvent;

private:
    PCSP_PRP_REGS m_pPrpReg;
    prpInputFormat m_iInputFormat;
    prpOutputVfFormat m_iOutputVfFormat;
    prpOutputEncFormat m_iOutputEncFormat;

    pPrpConfigData m_pPrpConfig;

    DWORD m_iPrpSysintr;

    UINT32 m_iEncOutputYSize;   // Used for encoding channel output YUV420
    UINT32 m_iMemInputYSize;    // Used for Memory mode input YUV420

    UINT32 m_iVfFrameCount;
    UINT32 m_iEncFrameCount;

    BOOL m_bEnableVfOutput;
    BOOL m_bEnableEncOutput;
    BOOL m_bConfigured;

    HANDLE m_hPrpIntrEvent;
    HANDLE m_hPrpIntrThread;

    BOOL m_hPowerDown;               // Used for signaling the IST to terminate

#ifndef PRP_MEM_MODE_DLL
    BOOL m_bVfStopReq;
    BOOL m_bEncStopReq;
    HANDLE m_hVfStopEvent;
    HANDLE m_hEncStopEvent;

    PrpBufferManager *pEncBufferManager;
    PrpBufferManager *pVfBufferManager;
#endif

    prpChannelState m_iVfState;
    prpChannelState m_iEncState;
};

#ifdef __cplusplus
}
#endif

#endif // _PRP_CLASS_H_


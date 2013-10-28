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
// File: PpClass.h
//
// Common definitions for post-processor module.
//
//------------------------------------------------------------------------------
#ifndef __PPCLASS_H__
#define __PPCLASS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define PP_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
    
#define PP_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT                         0
#define ZONEID_DEINIT                       1
#define ZONEID_IOCTL                        2
#define ZONEID_THREAD                       3
#define ZONEID_REGS                         11
#define ZONEID_INFO                         12
#define ZONEID_FUNCTION                     13
#define ZONEID_WARN                         14
#define ZONEID_ERROR                        15

// Debug zone masks
#define ZONEMASK_INIT                       (1 << ZONEID_INIT)
#define ZONEMASK_DEINIT                     (1 << ZONEID_DEINIT)
#define ZONEMASK_IOCTL                      (1 << ZONEID_IOCTL)
#define ZONEMASK_THREAD                     (1 << ZONEID_THREAD)
#define ZONEMASK_REGS                       (1 << ZONEID_REGS)
#define ZONEMASK_INFO                       (1 << ZONEID_INFO)
#define ZONEMASK_FUNCTION                   (1 << ZONEID_FUNCTION)
#define ZONEMASK_WARN                       (1 << ZONEID_WARN)
#define ZONEMASK_ERROR                      (1 << ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT                           DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT                         DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL                          DEBUGZONE(ZONEID_IOCTL)
#define ZONE_THREAD                         DEBUGZONE(ZONEID_THREAD)
#define ZONE_REGS                           DEBUGZONE(ZONEID_REGS)
#define ZONE_INFO                           DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION                       DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN                           DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR                          DEBUGZONE(ZONEID_ERROR)
#endif

#define PP_IMAGE_MIN_WIDTH                  32
#define PP_IMAGE_MIN_HEIGHT                 32
#define PP_IMAGE_MAX_WIDTH                  2044
#define PP_IMAGE_MAX_HEIGHT                 2044

#define PP_MB_SIZE                          16

#define PP_MAX_RESIZE_RETRIES               16

#define PP_MAX_RESIZE_COEFFS                32

#define PP_RESIZE_COEFF                     5
#define PP_MAX_COEFF                        (1 << PP_RESIZE_COEFF)

#define PP_MAX_NUM_BUFFERS                  100


//------------------------------------------------------------------------------
// Types
class PpClass {
public:
    PpClass(void);
    ~PpClass(void);
    BOOL PpEnable(void);
    void PpDisable(void);    
    BOOL PpConfigure(pPpConfigData pConfigData);
    BOOL PpEnqueueBuffers(pPpBuffers pPpBufs);
    void PpClearBuffers();
    UINT32 PpGetMaxBuffers(void);
    UINT32 PpGetFrameCount();
    BOOL PpStart();
    BOOL PpStop();

private:
    BOOL PpInit(void);
    void PpDeinit(void);
    BOOL PpAlloc(void);
    void PpDealloc(void);
    BOOL PpEventsInit(void);
    void PpEventsDeinit(void);
    BOOL PpConfigureInput(pPpConfigData pConfigData);
    BOOL PpConfigureOutput(pPpConfigData pConfigData);
    BOOL PpConfigureCSC(pPpConfigData pConfigData);
    BOOL PpConfigureResize(pPpConfigData pConfigData);
    UINT16 PpGetGcd(UINT16 x, UINT16 y);
    INT8 PpResize(UINT16 in, UINT16 out);
    void PpStoreCoeffs(UINT16 w1, UINT16 w2, UINT16 base, 
        UINT16 next, UINT16 index);
    void PpSetupResize(UINT8 iVertStart, UINT8 iVertEnd, 
        UINT8 iHoriStart, UINT8 iHoriEnd);    
    void PpBufferWorkerRoutine();
    void PpIntrRoutine();
    static void PpBufferWorkerThread(LPVOID lpParameter);
    static void PpIntrThread(LPVOID lpParameter);
    void PpDumpRegisters(void);

private:
    PCSP_PP_REGS m_pPpReg;
    
    HANDLE m_hPpIntrEvent;
    HANDLE m_hPpEOFEvent;
    HANDLE m_hPpReadyEvent;
    HANDLE m_hPpIntrThread;
    HANDLE m_hPpBufThread;

    HANDLE m_hReadPpBufferQueue;
    HANDLE m_hWritePpBufferQueue;

    BOOL m_bNeedQP;
    BOOL m_bConfigured;

    DWORD m_iPpSysintr;
    INT8 m_iHoriLen, m_iVertLen;
    UINT32  m_iFrameCount;
    UINT32 m_iResizeCoeffs[PP_MAX_RESIZE_COEFFS];
};

#ifdef __cplusplus
}
#endif

#endif  // __PPCLASS_H__


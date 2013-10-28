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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  PfClass.h
//
//  Common definitions for IPU's Post-Filtering module
//
//------------------------------------------------------------------------------
#include "IpuModuleInterfaceClass.h"
#include "display_vf.h"
#include "IpuBufferManager.h"

#ifndef __PFCLASS_H__
#define __PFCLASS_H__

//------------------------------------------------------------------------------
// Defines

#define PF_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define PF_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT       0
#define ZONEID_DEINIT     1
#define ZONEID_IOCTL      2
#define ZONEID_DEVICE     3

#define ZONEID_INFO       12
#define ZONEID_FUNCTION   13
#define ZONEID_WARN       14
#define ZONEID_ERROR      15

// Debug zone masks
#define ZONEMASK_INIT     (1<<ZONEID_INIT)
#define ZONEMASK_DEINIT   (1<<ZONEID_DEINIT)
#define ZONEMASK_IOCTL    (1<<ZONEID_IOCTL)
#define ZONEMASK_DEVICE   (1<<ZONEID_DEVICE)

#define ZONEMASK_INFO     (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN     (1<<ZONEID_WARN)
#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT       DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL        DEBUGZONE(ZONEID_IOCTL)
#define ZONE_DEVICE       DEBUGZONE(ZONEID_DEVICE)

#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
#endif

#define PF_MIN_WIDTH                16
#define PF_MIN_HEIGHT               16
#define PF_MAX_WIDTH                1024
#define PF_MAX_HEIGHT               1024

#define PF_MAX_PAUSE_ROW            63

// Maximum downscaling resize ratio
#define PF_MAX_DOWNSIZE_RATIO       8    

#define PF_EOF_EVENT                L"PF EOF Interrupt"

//------------------------------------------------------------------------------
// Types

#ifdef __cplusplus
extern "C" {
#endif


// IDMAC Channel configuration structure
typedef struct pfIDMACChannelParamsStruct
{
    UINT8                 iFormatCode;
    UINT32                iLineStride;
    UINT8                 iBitsPerPixelCode;
    UINT8                 iPixelBurstCode;
    UINT16                iHeight;
    UINT16                iWidth;
} pfIDMACChannelParams, *pPfIDMACChannelParams;


//------------------------------------------------------------------------------
// Functions
class PfClass : public IpuModuleInterfaceClass
{
    public:
        PfClass();
        ~PfClass();

        BOOL PfConfigure(pPfConfigData);

        BOOL  PfStartChannel(pPfStartParams);
        BOOL  PfStartChannel2(pPfStartParams,unsigned int,unsigned int); // for Direct Render
        BOOL  PfSetAttributeEx(pPfSetAttributeExData pSetAttributeExDataStruct);        
        DWORD PfResume(UINT32 h264_pause_row);
        
        void *PfAllocPhysMem(UINT32 size, PPhysicalAddress pPhysicalAddress, PUINT pDriverVirtualAddress);
        BOOL  pfFreePhysMem(pPfAllocMemoryParams pBitsStreamBufMemParams);
        
    private:
        BOOL PfInit(void);
        void PfDeinit(void);
        BOOL PfStopChannel(void);
        BOOL PfInitDisplayCharacteristics(void);
        void writeDMAChannelParam(int, int, int, unsigned int);
        void controlledWriteDMAChannelParam(int, int, int, unsigned int);
        void PfIDMACChannelConfig(UINT8, pPfIDMACChannelParams);
        void PfEnable(void);
        void PfDisable(void);
        void PfClearInterruptStatus(DWORD);
        static void PfIntrThread(LPVOID);
        void PfISRLoop(UINT32);

        HANDLE hIPUBase;
        
        BOOL m_bConfigured;

        pfMode m_pfMode;

        HANDLE  m_hPfIntrEvent;

        BOOL m_bPauseEnabled;

        // Event variable
        HANDLE  m_hEOFEvent;
        HANDLE  m_hYEvent;
        HANDLE  m_hCrEvent;
        
        // Thread handles
        HANDLE  m_hPfISRThread;

        // Misc.

        // If TRUE, the PF channel has been 
        // stopped, so we should wait for the current frame to 
        // be completed, and be sure to not re-enable the channel.
        BOOL    m_bRunning;
};

#ifdef __cplusplus
}
#endif

#endif  // __PFCLASS_H__

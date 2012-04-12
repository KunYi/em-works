//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ppClass.h
//
//  Common definitions for IPU's Postprocessor module
//
//------------------------------------------------------------------------------
#include "IpuModuleInterfaceClass.h"
#include "display_vf.h"
#include "IpuBufferManager.h"

#ifndef __PPCLASS_H__
#define __PPCLASS_H__

//------------------------------------------------------------------------------
// Defines

#define PP_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define PP_FUNCTION_EXIT() \
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

#define PP_MIN_INPUT_WIDTH     8
#define PP_MIN_INPUT_HEIGHT    2
#define PP_MAX_INPUT_WIDTH     4096
#define PP_MAX_INPUT_HEIGHT    4096
#define PP_MIN_OUTPUT_WIDTH    8
#define PP_MIN_OUTPUT_HEIGHT   1
// #define PP_MAX_OUTPUT_WIDTH    720 //720 for MX31, 800 for MX35
#define PP_MAX_OUTPUT_HEIGHT   1040

// Maximum downscaling resize ratio
#define PP_MAX_DOWNSIZE_RATIO  8    

#define PP_EOF_EVENT           L"PP EOF Interrupt"

//------------------------------------------------------------------------------
// Types

#ifdef __cplusplus
extern "C" {
#endif


// IDMAC Channel configuration structure
typedef struct ppIDMACChannelParamsStruct
{
    BOOL                  bInterleaved;
    UINT8                 iFormatCode;
    UINT32                iLineStride;
    UINT8                 iBAM;
    UINT8                 iBitsPerPixelCode;
    UINT8                 iPixelBurstCode;
    UINT16                iHeight;
    UINT16                iWidth;
    ppPixelFormat         pixelFormat;
} ppIDMACChannelParams, *pPpIDMACChannelParams;


// This structure is used to return the resizing 
// coefficients from PrpGetResizeCoeffs()
typedef struct ppResizeCoeffsStruct
{
    UINT16 downsizeCoeff;
    UINT16 resizeCoeff;
} ppResizeCoeffs, *pPpResizeCoeffs;

typedef enum
{
    ppInputChannel_Main,
    ppInputChannel_Comb
} ppInputChannel;

//------------------------------------------------------------------------------
// Functions
class PpClass : public IpuModuleInterfaceClass
{
    public:
        PpClass();
        ~PpClass();

        BOOL PpEnqueueBuffers(pPpBuffers);
        void PpClearBuffers(void);

        UINT32* PpGetBufFilled(void);
    
        UINT32 PpGetMaxBuffers(void);

        BOOL PpConfigure(pPpConfigData);

        BOOL  PpStartChannel(void);
        BOOL  PpStopChannel(void);

        BOOL  PpPauseViewfinding(void);

        UINT32 PpGetFrameCount();

    private:
        BOOL PpInit(void);
        void PpDeinit(void);
        BOOL PpInitDisplayCharacteristics(void);
        BOOL PpConfigureInput(pPpConfigData, ppInputChannel);
        BOOL PpConfigureOutput(pPpConfigData);
        void writeDMAChannelParam(int, int, int, unsigned int);
        void controlledWriteDMAChannelParam(int, int, int, unsigned int);
        unsigned int controlledReadDMAChannelParam(int, int, int);
        void PpIDMACChannelConfig(UINT8, pPpIDMACChannelParams);
        void writeICTaskParam(int, int, unsigned int);
        void controlledWriteICTaskParam(int, int, unsigned int);
        void PpTaskParamConfig(pPpCSCCoeffs);
        BOOL PpGetResizeCoeffs(UINT16, UINT16, pPpResizeCoeffs);
        void PpEnable(void);
        void PpDisable(void);
        void PpClearInterruptStatus(DWORD);
        static void PpIntrThread(LPVOID);
        void PpISRLoop(UINT32);
        static void PpBufferWorkerThread(LPVOID);
        void PpBufferWorkerRoutine(UINT32);
        static void PpRotBufferWorkerThread(LPVOID);
        void PpRotBufferWorkerRoutine(UINT32);

        BOOL PpAllocateRotBuffers(UINT32, UINT32);

        HANDLE hIPUBase;

        BOOL m_bConfigured;
        BOOL m_bDisplayInitialized;

        DISPLAY_CHARACTERISTIC m_displayCharacteristics;

        // Frame count variables
        UINT32  m_iFrameCount;

        HANDLE  m_hPpIntrEvent;

        // Event variable
        HANDLE m_hEOFEvent;

        // Thread handles
        HANDLE  m_hPpISRThread,m_hExitPpISRThread;
        HANDLE  m_hPpBufThread,m_hExitPpBufThread;
        HANDLE  m_hPpRotBufThread,m_hExitPpRotBufThread;

        HDC     m_hDisplay;

        HANDLE  m_hReadInputBufferQueue;
        HANDLE  m_hWriteInputBufferQueue;
        HANDLE  m_hReadOutputBufferQueue;
        HANDLE  m_hWriteOutputBufferQueue;
        HANDLE  m_hReadDisplayBufferQueue;
        HANDLE  m_hWriteDisplayBufferQueue;

        // Misc.

        // Determines whether we set up task chaining to
        // transfer viewfinding image to be displayed
        BOOL    m_bVfDirectDisplay;

        // If TRUE, ADC is the display type and direct display
        // enabled, so image goes to ADC directly.  In this case,
        // double-buffering is not needed.
        BOOL    m_bADCDirect;

        // If TRUE, the encoding or viewfinding channel is being 
        // started, so we should correct our buffer worker thread
        // in the case that it was left waiting for a new buffer
        // by a previous run.
        BOOL    m_bRestartBufferLoop;
        BOOL    m_bRotRestartBufferLoop;
        BOOL    m_bRestartISRLoop;

        // If TRUE, viewfinding channel is running AND is being
        // actively displayed.
        BOOL    m_bVfDisplayActive;

        // If TRUE, the PP channel has been 
        // stopped, so we should wait for the current frame to 
        // be completed, and be sure to not re-enable the channel.
        BOOL    m_bRunning;

        CRITICAL_SECTION m_csStopping;

        // If TRUE, flipping/rotation enabled
        BOOL    m_bFlipRot;

        // TRUE if current PP input is a planar YUV surface
        BOOL    m_bInputPlanar;

        // TRUE if current PP combining input is a planar YUV surface
        BOOL    m_bInputCombPlanar;

        // TRUE if current PP output is a planar YUV surface
        BOOL    m_bOutputPlanar;

        // Combining / Color Key variables
        BOOL    m_bCombiningEnabled;
        BOOL    m_bColorKeyEnabled;

        //*****************************
        // Buffer Management Members
        //*****************************

        HANDLE m_hRequestBuffer, m_hRequestRotBuffer;

        IpuBufferManager *pRotBufferManager;

        BOOL m_bRotBuffersAllocated;

        UINT32 m_iLastAllocatedBufferSize;
        
        BOOL m_bRequestSecondBuffer;
        BOOL m_bRotRequestSecondBuffer;

        UINT8 m_iCurrentBuf;
        BOOL m_iBuf0Ready, m_iBuf1Ready;

        // Number indicating which buffer will be the next to fill.
        DWORD  m_iBufferToFill;
        DWORD  m_iRotBufferToFill;
};

#ifdef __cplusplus
}
#endif

#endif  // __PPCLASS_H__

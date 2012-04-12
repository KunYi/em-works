//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
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
#define PP_MAX_OUTPUT_WIDTH    1024
#define PP_MAX_OUTPUT_HEIGHT   1024

#define PP_FUNC_NORESIZE          0
#define PP_FUNC_DOWNSIZE          1
#define PP_FUNC_UPSIZE            1<<1
#define PP_FUNC_ROTATION          1<<2  
#define PP_FUNC_CSC               1<<3
#define PP_FUNC_COMBINE           1<<4

// Maximum downscaling resize ratio
#define PP_MAX_DOWNSIZE_RATIO  8    

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
    UINT8                 iRotation90;
    UINT8                 iFlipHoriz;
    UINT8                 iFlipVert;
    UINT8                 iBandMode;
    UINT8                 iBlockMode;    
    UINT8                 iBitsPerPixelCode;
    UINT8                 iPixelBurstCode;
    UINT16                iHeight;
    UINT16                iWidth;
    icPixelFormat         pixelFormat;
    UINT32                UOffset;     // offset of U buffer from Y buffer start address
                                       // ignored if non-planar image format
    UINT32                VOffset;     // offset of U buffer from Y buffer start address
                                       // ignored if non-planar image format       
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

typedef enum
{
    ppSourceType_ARM,
    ppSourceType_PP,
    ppSourceType_ROT,
} ppSourceType;

//------------------------------------------------------------------------------
// Functions
class PpClass
{
    public:
        PpClass();
        ~PpClass();

        BOOL PpAddInputBuffer(UINT32 *pBuf);
        BOOL PpAddOutputBuffer(UINT32 * pBuf);
        BOOL PpAddInputCombBuffer(UINT32 *pBuf);
        BOOL PpClearBuffers();

        BOOL PpConfigure(pPpConfigData);

        BOOL  PpStartChannel(void);
        BOOL  PpStopChannel(void);

        void PpIntrEnable(UINT8 IntType, BOOL enable);
        void PpWaitForNotBusy(UINT8 IntType);
        BOOL PpChangeMaskPos(RECT *pPos);

        BOOL PpIsBusy();
        BOOL IRTIsBusy();

    private:
        BOOL PpInit(void);
        void PpDeinit(void);
        BOOL PpParamsCheck(pPpConfigData pConfigData);
        BOOL PpInitChannelParams(pIdmaChannel pIDMAChannel);
        BOOL PpConfigureInput(pIcFlipRot pFlipRot, BOOL isCombine);
        BOOL PpConfigureOutput(pIdmaChannel pIDMAChannel);
        BOOL IRTConfigureInput(pIcFlipRot pFlipRot);
        BOOL IRTConfigureOutput(pIcFlipRot pFlipRot);
        
        void PpIDMACChannelConfig(UINT8 ch);
        void PpConfigureMask();
        void PpConfigureCombine(UINT8 alpha, UINT32 colorkey);  
        BOOL PpConfigureResize();
        void PpConfigureCSC(CSCEQUATION CSCEquation, icCSCMatrix CSCMatrix, pIcCSCCoeffs pCustCSCCoeffs);
        BOOL PpGetResizeCoeffs(UINT16, UINT16, pPpResizeCoeffs);

        void PpEnable(void);
        void PpDisable(void);
        void IRTEnable(IPU_DRIVER driver);
        void IRTDisable(IPU_DRIVER driver);

        void ICDumpRegs();
        
        BOOL PpPixelFormatSetting(icFormat iFormat, icPixelFormat PixelFormat,icDataWidth iWidth);

        static void PpIntrThread(LPVOID lpParameter);
        void PpISRLoop(UINT32 timout);

        PCSP_IPU_IC_REGS m_pIC;
        HANDLE hIPUBase;

        BOOL m_bConfigured;
        HANDLE  m_hPpIntrEvent;
        HANDLE  m_hEOFEvent;
        HANDLE  m_hEOMEvent;

        // Event variable

        // Thread handles
        HANDLE  m_hPpISRThread;
        HANDLE  m_hExitPpISRThread;

        // Misc.

        // If TRUE, the PP channel has been 
        // stopped, so we should wait for the current frame to 
        // be completed, and be sure to not re-enable the channel.
        BOOL    m_bRunning;

        CRITICAL_SECTION m_csStopping;
        CRITICAL_SECTION m_csPpEnable;

        // TRUE if current PP input is a planar YUV surface
        BOOL    m_bInputPlanar;

        // TRUE if current PP output is a planar YUV surface
        BOOL    m_bOutputPlanar;

        BOOL    m_bCurrentPlanar;


        //*****************************
        // Buffer Management Members
        //*****************************
        IpuBuffer * m_hRotBuffer;

        ppSourceType m_PpFirstModule;
        ppSourceType m_PpLastModule;
        UINT16 m_PpOutputWidth;
        UINT16 m_PpOutputHeight;
        UINT32 m_PpOutputStride;
        ppIDMACChannelParams m_channelParams;

        BOOL m_bPPEnabled;
        BOOL m_bIRTEnabled;
        UINT8 m_intrType;

        BOOL m_bPPCombEnabled;
        BOOL m_bMaskEnable;
        BOOL m_bPerPixelAlpha;
};

#ifdef __cplusplus
}
#endif

#endif  // __PPCLASS_H__
        

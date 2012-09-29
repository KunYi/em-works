//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  prpClass.h
//
//  Common definitions for IPU's Preprocessor module
//
//------------------------------------------------------------------------------

#ifndef __PRPCLASS_H__
#define __PRPCLASS_H__

//------------------------------------------------------------------------------
// Defines

#define PRP_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define PRP_FUNCTION_EXIT() \
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

#define PRP_FUNC_NORESIZE           0
#define PRP_FUNC_DOWNSIZE           1
#define PRP_FUNC_UPSIZE             1<<1
#define PRP_FUNC_ROTATION           1<<2  
#define PRP_FUNC_CSC                1<<3
#define PRP_FUNC_COMBINE            1<<4

#define PRP_WIDTH_OVERSIZE          1
#define PRP_HEIGHT_OVERSIZE         1<<8

//------------------------------------------------------------------------------
// Types

#ifdef __cplusplus
extern "C" {
#endif


// IDMAC Channel configuration structure
typedef struct prpIDMACChannelParamsStruct
{
    BOOL                  bInterleaved;
    BOOL                  bInterlaced;   // for interlaced tv out
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
    UINT32                UOffset;    // offset of U buffer from Y buffer start address
                                      // ignored if non-planar image format
    UINT32                VOffset;    // offset of U buffer from Y buffer start address
                                      // ignored if non-planar image format        
} prpIDMACChannelParams, *pPrpIDMACChannelParams;


// This structure is used to return the resizing 
// coefficients from PrpGetResizeCoeffs()
typedef struct prpResizeCoeffsStruct
{
    UINT16 downsizeCoeff;
    UINT16 resizeCoeff;
} prpResizeCoeffs, *pPrpResizeCoeffs;

typedef enum
{
    prpOutputChannel_VF,
    prpOutputChannel_ENC
} prpOutputChannel;

typedef enum
{
    prpInputChannel_MAIN,
    prpInputChannel_COMBINE
} prpInputChannel;

typedef enum
{
    prpSourceType_ARM,
    prpSourceType_PRP,
    prpSourceType_ROT,
} prpSourceType;

// Pre-processing task parameter memory selection
typedef enum prpCSCTaskParamEnum
{
    prpCSCEncMatrix1,
    prpCSCVfMatrix1,
} prpCSCTaskParam;


//------------------------------------------------------------------------------
// Functions
class PrpClass 
{
    public:
        PrpClass();
        ~PrpClass();

        BOOL PrpConfigure(pPrpConfigData);

        BOOL  PrpStartChannel(void);
        BOOL  PrpStopChannel(void);
        BOOL  PrpAddInputBuffer(UINT32 *pBuf, BOOL bWait, UINT8 IntType);
        BOOL  PrpAddInputCombBuffer(UINT32 *pBuf);
        BOOL PrpAddVFOutputBuffer(UINT32 *pBuf);
        BOOL PrpClearBuffers(void);

        BOOL PrpIsBusy(void);
        BOOL IRTIsBusy(void);
        BOOL PrpBufferStatus(icBufferStatus * pStatus);

        DISPLAY_CHARACTERISTIC m_displayCharacteristics;


    private:
        BOOL PrpInit(void);
        void PrpDeinit(void);
        
        BOOL PrpInitChannelParams(pIdmaChannel pIDMAChannel, pPrpIDMACChannelParams pIDMAChannelParams);

        BOOL PrpConfigureInput(prpInputChannel inputChannel, prpOutputChannel outputChannel);
        BOOL PrpConfigureOutput(pIdmaChannel pIDMAChannel, prpOutputChannel outputChannel, BOOL bNoCSC);        
        BOOL IRTConfigureInput(pIcFlipRot pFlipRot,prpOutputChannel outputChannel);
        BOOL IRTConfigureOutput(pIcFlipRot pFlipRot,prpOutputChannel outputChannel);
        BOOL DPConfigureInput(BOOL bIsSyncFlow, BOOL bIsInterlaced, prpOutputChannel outputChannel);

        void PrpConfigureCombine(UINT8 alpha, UINT32 colorkey);
        BOOL PrpConfigureResize(prpOutputChannel outputChannel);
        void PrpConfigureCSC(CSCEQUATION CSCEquation, icCSCMatrix CSCMatrix, pIcCSCCoeffs pCustCSCCoeffs,  prpOutputChannel outputChannel);
        BOOL PrpPixelFormatSetting(icFormat iFormat, icPixelFormat PixelFormat,icDataWidth iWidth,pPrpIDMACChannelParams pChannelParams);
        BOOL PrpGetResizeCoeffs(UINT16 inSize, UINT16 outSize, pPrpResizeCoeffs resizeCoeffs);
        void PrpIDMACChannelConfig(UINT8 ch,pPrpIDMACChannelParams pChannelParams);
        BOOL PrpConfigureResize4OversizeParam(UINT16 inSize, UINT16 outSize, prpOutputChannel outputChannel, BOOL bIsWidth);
        
        void PrpEnable(void);
        void PrpDisable(void);

        void IRTEnable(IPU_DRIVER driver);
        void IRTDisable(IPU_DRIVER driver);

        void PrpIntrEnable(UINT8 IntType);
        
        void PrpWaitForNotBusy(UINT8 IntType);
        
        void ICDumpRegs();

        PCSP_IPU_IC_REGS m_pIC;
        
        HANDLE hIPUBase;

        BOOL m_bInputPlanar;
        BOOL m_bCurrentPlanar;
    
        CRITICAL_SECTION m_csPrpEnable;
        BOOL m_bVFPrpEnabled;
        BOOL m_bENCPrpEnabled;
        BOOL m_bVFIRTEnabled;
        BOOL m_bENCIRTEnabled;
        BOOL m_bVFCombEnabled;
        BOOL m_bSYNCDPEnabled;
        BOOL m_bASYNCDPEnabled;
        
        BOOL m_bSYNCDPCSCChanged;
        BOOL m_bASYNCDPCSCChanged;
        
        BOOL m_bDMFCChannelUsing;

        BOOL m_bConfigured;

        HDC     m_hDisplay;

        HANDLE m_hPrpIntrEvent;
        HANDLE m_hDPFGIntrEvent;

        // Misc.
        UINT16  m_iInputWidth, m_iInputHeight;

        // Determines whether we set up task chaining to
        // transfer viewfinding image to be displayed
        BOOL    m_bVfDirectDisplay;
        BOOL    m_bVfDisplayActive;
        BOOL    m_bSYNCisDI1; //to clarify the di channel for synchronous display, vf is always for DI0, enc is always for DI1
        BOOL    m_bSYNCisInterlaced; //to clarify if the dp use interlaced mode or progressive mode, the stop sequence for different mode is different.
        BOOL    m_bInputisInterlaced; //to clarify if the input dat ais interlaced mode, if so the data will be from VDI, the input idma setting should be ignored.
        // If TRUE, the encoding or viewfinding channel has been 
        // stopped, so we should wait for the current frame to 
        // be completed, and be sure to not re-enable the channel.
        BOOL    m_bRunning;

        
        UINT32  m_ENCOversizedFrameFlag;   //The output frame is too large so that we need to split it. 
        UINT32  m_VFOversizedFrameFlag;     //The output frame is too large so that we need to split it. 
        UINT32  m_OversizedInputOffsetStride; 
        UINT32  m_OversizedOutputOffsetStride; 
        UINT32  m_OversizedInputOffsetHeight; 
        UINT32  m_OversizedOutputOffsetHeight; 
        
        
        CRITICAL_SECTION m_csStopping;

        prpSourceType m_PrpFirstModule;
        prpSourceType m_PrpVFLastModule;
        prpSourceType m_PrpENCLastModule;
        UINT16 m_PrpVFOutputWidth;
        UINT16 m_PrpVFOutputHeight;
        UINT32 m_PrpVFOutputStride;
        UINT16 m_PrpENCOutputWidth;
        UINT16 m_PrpENCOutputHeight;
        UINT32 m_PrpENCOutputStride;
        prpIDMACChannelParams m_VFchannelParams;
        prpIDMACChannelParams m_ENCchannelParams;
        
        IpuBuffer * m_hVFRotBuffer;
        IpuBuffer * m_hENCRotBuffer;
        IpuBuffer * m_hVFDpBuffer;
        IpuBuffer * m_hENCDpBuffer;
};

#ifdef __cplusplus
}
#endif

#endif  // __PRPCLASS_H__

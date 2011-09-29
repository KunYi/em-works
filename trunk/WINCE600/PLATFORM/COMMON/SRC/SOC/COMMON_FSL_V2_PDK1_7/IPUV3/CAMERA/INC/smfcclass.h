//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  SMFCClass.h
//
//  Common definitions for SMFC module
//
//------------------------------------------------------------------------------
//

#ifndef __SMFCCLASS_H__
#define __SMFCCLASS_H__

//------------------------------------------------------------------------------
// Defines

#define SMFC_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define SMFC_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))


//------------------------------------------------------------------------------
// Types
#define IPU_SMFC_EOF_EVENT                        L"SMFC EOF Interrupt"

// RGB565
#define RGB565_COMPONENT0_OFFSET 0
#define RGB565_COMPONENT1_OFFSET 5
#define RGB565_COMPONENT2_OFFSET 11
#define RGB565_COMPONENT3_OFFSET 24
#define RGB565_COMPONENT0_WIDTH  5
#define RGB565_COMPONENT1_WIDTH  6
#define RGB565_COMPONENT2_WIDTH  5
#define RGB565_COMPONENT3_WIDTH  0
// RGB888
#define RGB888_COMPONENT0_OFFSET 0
#define RGB888_COMPONENT1_OFFSET 8
#define RGB888_COMPONENT2_OFFSET 16
#define RGB888_COMPONENT3_OFFSET 24
#define RGB888_COMPONENT0_WIDTH  8
#define RGB888_COMPONENT1_WIDTH  8
#define RGB888_COMPONENT2_WIDTH  8
#define RGB888_COMPONENT3_WIDTH  8

// UYVY
#define UYVY_COMPONENT0_OFFSET 0
#define UYVY_COMPONENT1_OFFSET 8
#define UYVY_COMPONENT2_OFFSET 16
#define UYVY_COMPONENT3_OFFSET 24
#define UYVY_COMPONENT0_WIDTH  8
#define UYVY_COMPONENT1_WIDTH  8
#define UYVY_COMPONENT2_WIDTH  8
#define UYVY_COMPONENT3_WIDTH  8
// YUV420
#define YUV420_COMPONENT0_OFFSET 0
#define YUV420_COMPONENT1_OFFSET 8
#define YUV420_COMPONENT2_OFFSET 16
#define YUV420_COMPONENT3_OFFSET 24
#define YUV420_COMPONENT0_WIDTH  8
#define YUV420_COMPONENT1_WIDTH  8
#define YUV420_COMPONENT2_WIDTH  8
#define YUV420_COMPONENT3_WIDTH  8
//
// Color conversion coefficient table
//
// RGB to YUV
static const UINT16 rgb2yuv_tbl[4][13] =
{
    // C00 C01   C02    C10     C11    C12    C20   C21     C22     A0       A1       A2    Scale
    {0x4D, 0x96, 0x1D, 0x1D5, 0x1AB, 0x80, 0x80, 0x195, 0x1EB, 0x00, 0x200, 0x200, 1},  // A1
    {0x42, 0x81, 0x19, 0x1DA, 0x1B6, 0x70, 0x70, 0x1A2, 0x1EE, 0x40, 0x200, 0x200, 1},  // A0
    {0x36, 0xB7, 0x12, 0x1E3, 0x19D, 0x80, 0x80, 0x18C, 0x1F4, 0x00, 0x200, 0x200, 1},  // B1
    {0x2F, 0x9D, 0x10, 0x1E6, 0x1A9, 0x70, 0x70, 0x19A, 0x1F6, 0x40, 0x200, 0x200, 1}   // B0
};

// YUV to RGB
static const UINT16 yuv2rgb_tbl[4][13] =
{
    // C00 C01   C02   C10    C11     C12     C20   C21   C22    A0         A1       A2      Scale
    {0x95, 0x00, 0xCC, 0x95, 0x1CE, 0x198, 0x95, 0xFF, 0x00, 0x1E42, 0x10A, 0x1DD6, 2}, //A1
    {0x4A, 0x66, 0x00, 0x4A, 0x1CC, 0x1E7, 0x4A, 0x00, 0x81, 0x1E42, 0x10F, 0x1DD6, 3}, //A0
    {0x80, 0x00, 0xCA, 0x80, 0x1E8, 0x1C4, 0x80, 0xED, 0x00, 0x1E6D, 0xA8,  0x1E25, 0}, //B1
    {0x4A, 0x73, 0x00, 0x4A, 0x1DE, 0x1F2, 0x4A, 0x00, 0x87, 0x1E10, 0x9A,  0x1DBE, 3}  //B0
};

static const TCHAR SMFCBufEOFEvent[10][50] =
{
    {L"SMFCBUF0_EOF_Interrupt"},
    {L"SMFCBUF1_EOF_Interrupt"},
    {L"SMFCBUF2_EOF_Interrupt"},
    {L"SMFCBUF3_EOF_Interrupt"},
    {L"SMFCBUF4_EOF_Interrupt"},
    {L"SMFCBUF5_EOF_Interrupt"},
    {L"SMFCBUF6_EOF_Interrupt"},
    {L"SMFCBUF7_EOF_Interrupt"},
    {L"SMFCBUF8_EOF_Interrupt"},
    {L"SMFCBUF9_EOF_Interrupt"}
};

// Image Converter RGB or YUV Format Structure
// For RGB, component0 = red, component1 = green, component2 = blue, 
// component3 = alpha
// For YUV, component0 = Y, component1 = U, component2 = V, component3 = NA
typedef struct SMFCPixelFormatStruct {
    UINT8 component0_width;
    UINT8 component1_width;
    UINT8 component2_width;
    UINT8 component3_width;
    UINT8 component0_offset;
    UINT8 component1_offset;
    UINT8 component2_offset;
    UINT8 component3_offset;
} SMFCPixelFormat, *pSMFCPixelFormat;

typedef enum SMFCFormatEnum
{
    SMFCFormat_YUV444 = 1,
    SMFCFormat_YUV422,
    SMFCFormat_YUV420,
    SMFCFormat_YUV422P,  //YUV422 partial interleaved
    SMFCFormat_YUV420P,  //YUV420 partial interleaved
    SMFCFormat_RGB565,
    SMFCFormat_RGB24,
    SMFCFormat_RGBA,
    SMFCFormat_YUV444IL,// YUV444 interleaved
    SMFCFormat_YUYV422, // YUV422 interleaved patterns
    SMFCFormat_YVYU422,
    SMFCFormat_UYVY422,
    SMFCFormat_VYUY422,
    SMFCFormat_Generic,
    SMFCFormat_Disabled,
    SMFCFormat_Undefined
} SMFCFormat;

typedef struct SMFCFrameSizeStruct 
{
    UINT16 width;
    UINT16 height;
} SMFCFrameSize, *pSMFCFrameSize;

typedef enum SMFCDataWidthEnum
{
    SMFCDataWidth_32BPP = 0,
    SMFCDataWidth_24BPP,
    SMFCDataWidth_16BPP,
    SMFCDataWidth_8BPP,
    SMFCDataWidth_4BPP,
    SMFCDataWidth_Undefined
} SMFCDataWidth;

//SMFC Configuration Data Structure
typedef struct SMFCConfigDataStruct
{   
    BOOL directDisplay;

    // For output                       
    SMFCFormat    outputFormat;   
    SMFCFrameSize          outputSize;    
    SMFCDataWidth          outputDataWidth;   // Bits per pixel for RGB format
    SMFCPixelFormat        outputPixelFormat; // Output frame RGB format, set NULL to use standard settings.
}SMFCConfigData, *pSMFCConfigData;


// IDMAC Channel configuration structure
typedef struct SMFCIDMACChannelParamsStruct
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
    SMFCPixelFormat       pixelFormat;
    UINT32                UOffset;    // offset of U buffer from Y buffer start address
                                      // ignored if non-planar image format
    UINT32                VOffset;    // offset of U buffer from Y buffer start address
                                      // ignored if non-planar image format        
} SMFCIDMACChannelParams, *pSMFCIDMACChannelParams;

//------------------------------------------------------------------------------
// Functions
class SMFCClass
{
    public:
        SMFCClass();
        ~SMFCClass();

        void SMFCEnable(void);
        void SMFCDisable(void);
        void DumpSMFCRegs();

        void SMFCSetBufferManager(CamBufferManager *);
        BOOL SMFCAllocateBuffers(CamBufferManager *,ULONG, ULONG);
        BOOL SMFCGetAllocBufPhyAddr(CamBufferManager *,ULONG,UINT32* []);
        BOOL SMFCDeleteBuffers(CamBufferManager *);
        BOOL RegisterBuffer(CamBufferManager *, LPVOID pVirtualAddress, UINT32 pPhysicalAddress);
        BOOL UnregisterBuffer(CamBufferManager *,LPVOID pVirtualAddress, UINT32 pPhysicalAddress);
        BOOL UnregisterAllBuffers();
        BOOL Enqueue(CamBufferManager *);
        BOOL SMFCConfigureEncoding(UINT8 ch,CSI_SELECT csi_sel, CSI_SELECT_FRAME csi_FrameId,pSMFCConfigData configData);
        BOOL SMFCConfigureViewfinding(UINT8 ch,CSI_SELECT csi_sel, CSI_SELECT_FRAME csi_FrameId,pSMFCConfigData configData);
        UINT32* SMFCGetBufFilled();    
        UINT32 SMFCGetMaxBuffers();

        BOOL SMFCConfig(UINT8 ch,CSI_SELECT csi_sel,CSI_SELECT_FRAME csi_FrameId, pSMFCIDMACChannelParams pChannelParams);
        BOOL SMFCInitDisplayCharacteristics(void);
        
        BOOL SMFCStartChannel(void);
        BOOL SMFCStopChannel(void);
        BOOL SMFCPreStopChannel(void);

        UINT32 SMFCGetFrameCount();
        
        BOOL SMFCIsBusy(UINT8);

        void SMFCDumpIntrRegs(UINT8 ch);
        DWORD GetMemoryMode();

        HANDLE  m_hCameraSMFCEOFEvent;
        
        // Frame count variables
        UINT32  m_iSMFCFrameCount;

        CamBufferManager *m_pSMFCBufferManager;

    private:
        BOOL SMFCInit(void);
        void SMFCDeinit(void);

        void SMFCSetChannel(UINT8 ch);
        void SMFCIDMACChannelConfig(UINT8, pSMFCIDMACChannelParams);

        static void SMFCIntrThread(LPVOID);
        void SMFCISRLoop(UINT32);
        
        PCSP_IPU_SMFC_REGS m_pSMFC;
        
        HDC     m_hDisplay;

        DISPLAY_CHARACTERISTIC m_displayCharacteristics;
    
        CRITICAL_SECTION m_csSMFCEnable;
        
        BOOL    m_bSMFCEnabled;
        BOOL m_bSMFCConfigured;              
        BOOL    m_bSMFCRestartISRLoop;       
        BOOL    m_bSMFCRunning;

        HANDLE  m_hSMFCIntrEvent;

        // Thread handles
        HANDLE  m_hSMFCISRThread;        
        HANDLE m_hExitSMFCISRThread;
       
        CRITICAL_SECTION m_csSMFCStopping;
        
        //*****************************
        // Buffer Management Members
        //*****************************
        HANDLE m_hSMFCBufEOFEvent[NUM_PIN_BUFFER_MAX];

        UINT32 m_iSMFCNumBuffers;
        UINT32 m_iSMFCBufSize;
        DWORD  m_dwMemoryMode;

        LARGE_INTEGER m_lpFrequency;
        LARGE_INTEGER m_lpPerformanceCount_start;
        LARGE_INTEGER m_lpPerformanceCount_end;
        UINT32 m_iCurrentHWBufAddr; 
        UINT8  m_iSMFCChannel;

        // For TO2, workaroud for still pin
        int m_iSMFCFirstFrame;
};
#endif  // __SMFCCLASS_H__

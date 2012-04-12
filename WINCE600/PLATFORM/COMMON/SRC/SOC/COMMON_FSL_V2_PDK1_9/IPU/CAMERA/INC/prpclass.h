//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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

#define PRP_MIN_INPUT_WIDTH     8
#define PRP_MIN_INPUT_HEIGHT    2
#define PRP_MAX_INPUT_WIDTH     4096
#define PRP_MAX_INPUT_HEIGHT    4096
#define PRP_MIN_OUTPUT_WIDTH    8
#define PRP_MIN_OUTPUT_HEIGHT   1
#define PRP_MAX_OUTPUT_WIDTH    800
#define PRP_MAX_OUTPUT_HEIGHT   1024

// Maximum downscaling resize ratio
#define PRP_MAX_DOWNSIZE_RATIO    8    

// There is a known issue in CE6 that causes Data Aborts in AllocPhysMem when allocating
// too much pages in a given section, which happens in the driver when operating multiple
// allocation/deallocation in a row. This works around the issue by allocating PRP buffers
// only once with the maximum size required by the largest resolution supported.
#define RINGO_FIX_MEM_ALLOC_ISSUE   1
#define MAX_IMAGE_SIZE              (800*600*2)

//------------------------------------------------------------------------------
// Types

#ifdef __cplusplus
extern "C" {
#endif

//Pre-Processing Input Format
typedef enum prpInputFormatEnum
{
    prpInputFormat_YUV444 = 1,
    prpInputFormat_YUV422, // non-interleaved
    prpInputFormat_YUV420,
    prpInputFormat_RGB,
    prpInputFormat_YUV444IL,// YUV444 interleaved
    prpInputFormat_YUYV422, // YUV422 interleaved patterns
    prpInputFormat_YVYU422, 
    prpInputFormat_UYVY422, 
    prpInputFormat_VYUY422, 
    prpInputFormat_Generic,
} prpInputFormat;

//Pre-Processing Data Width for RGB format
typedef enum prpDataWidthEnum
{
    prpDataWidth_32BPP = 0,
    prpDataWidth_24BPP,
    prpDataWidth_16BPP,
    prpDataWidth_8BPP,
    prpDataWidth_4BPP,
} prpDataWidth;

//Pre-Processing Encoding Channel Output Format
typedef enum prpEncOutputFormatEnum
{
    prpEncOutputFormat_YUV444 = 1,
    prpEncOutputFormat_YUV422,
    prpEncOutputFormat_YUV420,
    prpEncOutputFormat_RGB,
    prpEncOutputFormat_RGBA,
    prpEncOutputFormat_YUV444IL,// YUV444 interleaved
    prpEncOutputFormat_YUYV422, // YUV422 interleaved patterns
    prpEncOutputFormat_YVYU422,
    prpEncOutputFormat_UYVY422,
    prpEncOutputFormat_VYUY422,
    prpEncOutputFormat_Generic,
    prpEncOutputFormat_Disabled,
} prpEncOutputFormat;


// There is no Viewfinding Channel output data width
// parameter because we know that the output width for
// RGB must be 8 bits per color


//Pre-Processing Viewfinding Channel Output Format
typedef enum prpVfOutputFormatEnum
{
    prpVfOutputFormat_RGB,
    prpVfOutputFormat_RGBA,
    prpVfOutputFormat_Disabled,
}prpVfOutputFormat;


//Pre-Processing CSC equation
typedef enum prpCSCEquationEnum
{
    prpCSCR2Y_A1,  //RGB to YUV equation A.1
    prpCSCR2Y_A0,  //RGB to YUV equation A.0
    prpCSCR2Y_B1,  //RGB to YUV equation B.1
    prpCSCR2Y_B0,  //RGB to YUV equation B.0
    prpCSCY2R_A1,  //YUV to RGB equation A.1
    prpCSCY2R_A0,  //YUV to RGB equation A.0
    prpCSCY2R_B1,  //YUV to RGB equation B.1
    prpCSCY2R_B0,  //YUV to RGB equation B.0
    prpCSCNoOp,
    prpCSCCustom
} prpCSCEquation;


// Pre-Processing RGB or YUV Format Structure
// For RGB, component0 = red, component1 = green, component2 = blue, 
// component3 = alpha
// For YUV, component0 = Y, component1 = U, component2 = V, component3 = NA
typedef struct prpPixelFormatStruct {
    UINT8 component0_width;
    UINT8 component1_width;
    UINT8 component2_width;
    UINT8 component3_width;
    UINT8 component0_offset;
    UINT8 component1_offset;
    UINT8 component2_offset;
    UINT8 component3_offset;
} prpPixelFormat, *pPrpPixelFormat;


// Pre-Processing CSC equation coeffprpients
// These should be set when the prpCSCCustom CSC equation is selected
typedef struct prpCSCCoeffsStruct
{
    UINT16 C00;
    UINT16 C01;
    UINT16 C02;
    UINT16 C10;
    UINT16 C11;
    UINT16 C12;
    UINT16 C20;
    UINT16 C21;
    UINT16 C22;
    UINT16 A0;
    UINT16 A1;
    UINT16 A2;
    UINT16 Scale;
} prpCSCCoeffs, *pPrpCSCCoeffs;


// Parameters for flipping and rotating frames
typedef struct prpFlipRotStruct
{
    BOOL verticalFlip;
    BOOL horizontalFlip;
    BOOL rotate90;
} prpFlipRot, *pPrpFlipRot;



//Pre-Processing Frame Size Structure
typedef struct prpFrameSizeStruct {
    UINT16 width;
    UINT16 height;
} prpFrameSize, *pPrpFrameSize;


//Pre-Processing Configuration Data Structure
typedef struct prpEncConfigDataStruct
{
    //---------------------------------------------------------------
    // General controls
    //---------------------------------------------------------------
    BOOL                  directCapture;  // Encoding channel direct capture from CSI to memory
                                          // If direct capture is enabled, data from the CSI is written
                                          // directly to memory without any preprocessing.

    //---------------------------------------------------------------
    // Format controls
    //---------------------------------------------------------------

    // For input
    prpInputFormat        inputFormat;    // YUV or RGB
    prpFrameSize          inputSize;      // input frame size
    prpPixelFormat        inRGBPixelFormat;  // Input frame RGB format, set NULL 
                                             // to use standard settings.
    // For encoding output                       
    prpEncOutputFormat    encFormat;      // Output format for Encoding channel
    prpFrameSize          encSize;        // Channel-1 output size
    prpDataWidth          encDataWidth;   // Bits per pixel for RGB format
    prpPixelFormat        encRGBPixelFormat; // Output frame RGB format, set NULL to use 
                                          // standard settings.
    prpCSCEquation        encCSCEquation; // Selects R2Y or Y2R CSC Equation
    prpCSCCoeffs          encCSCCoeffs;   // Selects R2Y or Y2R CSC Equation
    prpFlipRot            encFlipRot;     // Flip/Rotate controls for Encoding

}prpEncConfigData, *pPrpEncConfigData;


//Pre-Processing Configuration Data Structure
typedef struct prpVfConfigDataStruct
{
    //---------------------------------------------------------------
    // General controls
    //---------------------------------------------------------------
    BOOL                  directDisplay;  // If enabled, viewfinding data will be sent to the display.
                                          // Otherwise, viewfinding data is written into memory.
                                          // In the former case, if the platform uses the ADC
                                          // driver, no buffers are required for viewfinding,
                                          // as the data is sent directly to the display without
                                          // being written to memory.

    //---------------------------------------------------------------
    // Format controls
    //---------------------------------------------------------------

    // For input
    prpInputFormat        inputFormat;    // YUV or RGB
    prpFrameSize          inputSize;      // input frame size
    prpPixelFormat        inRGBPixelFormat;  // Input frame RGB format, set NULL 
                                            // to use standard settings.

    // For viewfinding output
    prpVfOutputFormat     vfFormat;       // Output format for Viewfinding channel
    prpFrameSize          vfSize;         // Channel-2 output size
    POINT                 vfOffset;       // If windowing and direct display enabled, 
                                          // this point specifies an offset for 
                                          // displaying the VF image
    prpCSCEquation        vfCSCEquation;  // Selects R2Y or Y2R CSC Equation
    prpCSCCoeffs          vfCSCCoeffs;    // Selects R2Y or Y2R CSC Equation
    prpFlipRot            vfFlipRot;      // Flip/Rotate controls for VF
}prpVfConfigData, *pPrpVfConfigData;


// IDMAC Channel configuration structure
typedef struct prpIDMACChannelParamsStruct
{
    BOOL                  bInterleaved;
    UINT8                 iFormatCode;
    UINT32                iLineStride;
    UINT8                 iBAM;
    UINT8                 iBitsPerPixelCode;
    UINT8                 iPixelBurstCode;
    UINT16                iHeight;
    UINT16                iWidth;
    prpPixelFormat        pixelFormat;
} prpIDMACChannelParams, *pPrpIDMACChannelParams;


// Pre-processing task parameter memory selection
typedef enum prpCSCTaskParamEnum
{
    prpCSCEncMatrix1,
    prpCSCVfMatrix1,
} prpCSCTaskParam;


// This structure is used to return the resizing 
// coefficients from PrpGetResizeCoeffs()
typedef struct prpResizeCoeffsStruct
{
    UINT16 downsizeCoeff;
    UINT16 resizeCoeff;
} prpResizeCoeffs, *pPrpResizeCoeffs;


// Pre-processing task parameter memory selection
typedef enum prpBufferEventEnum
{
    prpBuf0RequestEvent,
    prpBuf1RequestEvent,
    prpMaxBufRequestEvents,
} prpBufferEvent;


//------------------------------------------------------------------------------
// Functions
class PrpClass : public IpuModuleInterfaceClass
{
    public:
        PrpClass();
        ~PrpClass();

        BOOL PrpInitDisplayCharacteristics(void);
        BOOL PrpAllocateEncBuffers(ULONG, ULONG);
        BOOL PrpAllocateVfBuffers(ULONG, ULONG);
        BOOL PrpDeleteEncBuffers();
        BOOL PrpDeleteVfBuffers();

        UINT32* PrpGetEncBufFilled();
        UINT32* PrpGetVfBufFilled();
    
        UINT32 PrpGetMaxBuffers();

        BOOL PrpConfigureEncoding(pPrpEncConfigData);
        BOOL PrpConfigureViewfinding(pPrpVfConfigData);

        void PrpEnable(void);
        void PrpDisable(void);

        BOOL  PrpStartEncChannel(void);
        BOOL  PrpStopEncChannel(void);
        BOOL  PrpStartVfChannel(void);
        BOOL  PrpStopVfChannel(void);

        BOOL  PrpPauseViewfinding(void);

        UINT32 PrpGetEncFrameCount();
        UINT32 PrpGetVfFrameCount();


        // TODO: Remove - inserted for debug TVIN purposes
        void PrpReadInterruptRegisters( );
        void PrpReadIPURegisters( );
        void PrpReadVfDMA( );


        DISPLAY_CHARACTERISTIC m_displayCharacteristics;

        // Event variables
        HANDLE m_hEncEOFEvent;
        HANDLE m_hVfEOFEvent;

        // For Ringo TVIN+
        BOOL   m_bNTSCtoPAL;
        int    m_iCamType;

    private: 
        BOOL PrpInit(void);
        void PrpDeinit(void);

        void controlledWriteDMAChannelParam(int, int, int, unsigned int);
        void writeDMAChannelParam(int, int, int, unsigned int);
        void PrpIDMACChannelConfig(UINT8, pPrpIDMACChannelParams, 
                                         UINT32, UINT32);
        void controlledWriteICTaskParam(int, int, unsigned int);
        void writeICTaskParam(int, int, unsigned int);
        void PrpTaskParamConfig(prpCSCTaskParam, pPrpCSCCoeffs);
        BOOL PrpGetResizeCoeffs(UINT16, UINT16, pPrpResizeCoeffs);
        void PrpClearInterruptStatus(DWORD);
        static void PrpIntrThread(LPVOID);
        void PrpISRLoop(UINT32);
        static void PrpEncBufferWorkerThread(LPVOID);
        void PrpEncBufferWorkerRoutine(UINT32);
        static void PrpEncRotBufferWorkerThread(LPVOID);
        void PrpEncRotBufferWorkerRoutine(UINT32);
        static void PrpVfBufferWorkerThread(LPVOID);
        void PrpVfBufferWorkerRoutine(UINT32);
        static void PrpVfRotBufferWorkerThread(LPVOID);
        void PrpVfRotBufferWorkerRoutine(UINT32);

        BOOL PrpAllocateEncRotBuffers();
        BOOL PrpAllocateVfRotBuffers();

        // TODO: Remove - inserted for debug purposes
        void CsiTestPatternOn(PCSP_IPU_REGS);

        HANDLE hIPUBase;

        BOOL m_bEncCSC, m_bVfCSC;
  
        CRITICAL_SECTION m_csPrpEnable;
        BOOL m_bPrpEnabled;

        BOOL m_bVfConfigured, m_bEncConfigured;

        // Frame count variables
        UINT32  m_iEncFrameCount;
        UINT32  m_iVfFrameCount;

        HANDLE  m_hPrpIntrEvent;

        // Thread handles
        HANDLE  m_hPrpISRThread;
        HANDLE  m_hPrpEncBufThread, m_hPrpVfBufThread;
        HANDLE  m_hPrpEncRotBufThread, m_hPrpVfRotBufThread;

        HDC     m_hDisplay;

        HANDLE  m_hReadVfBufferQueue;
        HANDLE  m_hWriteVfBufferQueue;

        // Misc.
        UINT16  m_iInputWidth, m_iInputHeight;

        // Determines whether we set up task chaining to
        // transfer viewfinding image to be displayed
        BOOL    m_bVfDirectDisplay;
        BOOL    m_bVfDisplayActive;

        // If TRUE, ADC is the display type and direct display
        // enabled, so image goes to ADC directly.  In this case,
        // double-buffering is not needed.
        BOOL    m_bADCDirect;

        // If TRUE, the encoding or viewfinding channel is being 
        // started, so we should correct our buffer worker thread
        // in the case that it was left waiting for a new buffer
        // by a previous run.
        BOOL    m_bEncRestartBufferLoop, m_bVfRestartBufferLoop;
        BOOL    m_bEncRotRestartBufferLoop, m_bVfRotRestartBufferLoop;
        BOOL    m_bEncRestartISRLoop, m_bVfRestartISRLoop;


        // If TRUE, the encoding or viewfinding channel has been 
        // stopped, so we should wait for the current frame to 
        // be completed, and be sure to not re-enable the channel.
        BOOL    m_bEncRunning, m_bVfRunning;

        CRITICAL_SECTION m_csEncStopping, m_csVfStopping;

        // If TRUE, flipping/rotation enabled for encoding
        // or viewfinding channels
        BOOL    m_bEncFlipRot, m_bVfFlipRot;

        //*****************************

        // Buffer Management Members

        //*****************************
        HANDLE m_hEncBufWaitList[prpMaxBufRequestEvents];
        HANDLE m_hEncRotBufWaitList[prpMaxBufRequestEvents];
        HANDLE m_hVfBufWaitList[prpMaxBufRequestEvents];
        HANDLE m_hVfRotBufWaitList[prpMaxBufRequestEvents];

        IpuBufferManager *pEncBufferManager, *pVfBufferManager;
        IpuBufferManager *pEncRotBufferManager, *pVfRotBufferManager;

        BOOL m_bEncRotBuffersAllocated, m_bVfRotBuffersAllocated;

        UINT32 m_iEncNumBuffers, m_iVfNumBuffers;
        UINT32 m_iEncBufSize, m_iVfBufSize;

        UINT8 m_iCurrentVfBuf, m_iCurrentEncBuf, m_iCurrentVfRotBuf, m_iCurrentEncRotBuf;

        BOOL m_iVfBuf0Ready, m_iVfBuf1Ready;
        BOOL m_iEncBuf0Ready, m_iEncBuf1Ready;

        HANDLE m_hExitPrpISRThread;
        HANDLE m_hExitPrpEncThread,m_hExitPrpEncRotThread;
        HANDLE m_hExitPrpVfThread,m_hExitPrpVfRotThread;

};

#ifdef __cplusplus
}
#endif


#endif  // __PRPCLASS_H__


//-----------------------------------------------------------------------------n
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  PpClass.cpp
//
//  Implementation of postprocessor driver methods
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipu.h"

#include "Ipu.h"
#include "pp.h"
#include "PpClass.h"


//------------------------------------------------------------------------------
// External Functions
extern "C" DWORD PPGetMaxFrameWidth();

extern void BSPSetPPISRPriority();
extern void BSPSetPPBufferThreadPriority();
extern void BSPSetPPRotBufferThreadPriority();


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines

#define THREAD_PRIORITY                            250

#define PP_MAX_NUM_BUFFERS                         100
#define PP_NUM_ROT_BUFFERS                         2

#define PP_IC_TO_MEM_DMA_CHANNEL                   IPU_DMA_CHA_DMAIC_2_LSH
#define PP_MEM_TO_IC_COMBINING_DMA_CHANNEL         IPU_DMA_CHA_DMAIC_4_LSH
#define PP_MEM_TO_IC_DMA_CHANNEL                   IPU_DMA_CHA_DMAIC_5_LSH
#define PP_IC_TO_MEM_ROT_DMA_CHANNEL               IPU_DMA_CHA_DMAIC_12_LSH
#define PP_MEM_TO_IC_ROT_DMA_CHANNEL               IPU_DMA_CHA_DMAIC_13_LSH


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

// Static variables from parent class
// must be defined before they can be used.
PCSP_IPU_REGS IpuModuleInterfaceClass::m_pIPU = NULL;
HANDLE IpuModuleInterfaceClass::m_hIpuMutex = NULL;


//------------------------------------------------------------------------------
// Local Variables


// TODO:  Remove, for debug purposes only
static UINT16 g_iOutputWidth, g_iOutputHeight;
static UINT32 g_iOutputPhysAddr;

//------------------------------------------------------------------------------
// Local Functions
static void dumpChannelParams(pPpIDMACChannelParams);
static void dumpCoeffs(pPpCSCCoeffs);
static void dumpIpuRegisters(PCSP_IPU_REGS);
#if 0 // Remove-W4: Warning C4505 workaround
static void dumpInterruptRegisters(PCSP_IPU_REGS);
static void ReadVfDMA(PCSP_IPU_REGS, UINT32);
static void generateBMP(UINT32 phyImageBuf, LONG width, LONG height);
#endif

//-----------------------------------------------------------------------------
//
// Function: PpClass
//
// Postprocessor class constructor.  Calls PpInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PpClass::PpClass(void)
{
    PpInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~PpClass
//
// The destructor for PpClass.  Calls PpDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PpClass::~PpClass(void)
{
    PpDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: PpInit
//
// This function initializes the Image Converter (postprocessor).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpInit(void)
{
    MSGQUEUEOPTIONS queueOptions;

    PP_FUNCTION_ENTRY();

    // open handle to the IPU_BASE driver in order to enable IC module
    hIPUBase = CreateFile(TEXT("IPU1:"),          // "special" file name
        GENERIC_READ|GENERIC_WRITE,   // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                   // security attributes (=NULL)
        OPEN_EXISTING,            // creation disposition
        FILE_FLAG_RANDOM_ACCESS,  // flags and attributes
        NULL);                  // template file (ignored)
    if (hIPUBase == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for IPU Interrupt\r\n"), __WFUNCTION__));
    m_hPpIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_PP_INTR_EVENT);
    if (m_hPpIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for IPU Interrupt failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for PP EOF event\r\n"), __WFUNCTION__));
    // Events to signal pin that frame is ready
    m_hEOFEvent = CreateEvent(NULL, FALSE, FALSE, PP_EOF_EVENT_NAME);
    if (m_hEOFEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for PP EOF\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Initialize buffer management handles
    m_hRequestBuffer = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hRequestBuffer == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request PP Buffer\r\n"), __WFUNCTION__));
        goto Error;
    }

    m_hRequestRotBuffer = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hRequestRotBuffer == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for Request PP for Rotataion Buffer\r\n"), __WFUNCTION__));
        goto Error;
    }


    DEBUGMSG(ZONE_INIT, (TEXT("%s: Create msgqueues\r\n"), __WFUNCTION__));
    // Create queues for reading and writing messages
    queueOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    queueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    queueOptions.dwMaxMessages = PP_MAX_NUM_BUFFERS;
    queueOptions.cbMaxMessage = sizeof(DISPLAY_BUFFER);
    queueOptions.bReadAccess = TRUE; // we need read-access to msgqueue

    // Create read handles for queues
    m_hReadDisplayBufferQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadDisplayBufferQueue)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error creating Display Buffer queue.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    queueOptions.cbMaxMessage = sizeof(ppBuffers);

    m_hReadInputBufferQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadInputBufferQueue)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error creating Input Buffer queue.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    m_hReadOutputBufferQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadOutputBufferQueue)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error creating Output Buffer queue.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    queueOptions.cbMaxMessage = sizeof(DISPLAY_BUFFER);
    queueOptions.bReadAccess = FALSE; // we need write-access to msgqueue

    // Create write handles for queues
    m_hWriteDisplayBufferQueue = OpenMsgQueue(GetCurrentProcess(), m_hReadDisplayBufferQueue, &queueOptions);
    if (!m_hWriteDisplayBufferQueue)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error opening display buffer queue for writing.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    queueOptions.cbMaxMessage = sizeof(ppBuffers);

    m_hWriteInputBufferQueue = OpenMsgQueue(GetCurrentProcess(), m_hReadInputBufferQueue, &queueOptions);
    if (!m_hWriteInputBufferQueue)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error opening input buffer queue for writing.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    m_hWriteOutputBufferQueue = OpenMsgQueue(GetCurrentProcess(), m_hReadOutputBufferQueue, &queueOptions);
    if (!m_hWriteOutputBufferQueue)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error opening output buffer queue for writing.  Initialization failed! \r\n"), __WFUNCTION__));
    }


    DEBUGMSG(ZONE_INIT, (TEXT("%s: Initializing class variables\r\n"), __WFUNCTION__));
    InitializeCriticalSection(&m_csStopping);

    pRotBufferManager = new IpuBufferManager();

    m_bConfigured = FALSE;
    m_bDisplayInitialized = FALSE;

    m_bRotBuffersAllocated = FALSE;

    m_iLastAllocatedBufferSize = 0;

    m_bInputPlanar = FALSE;
    m_bInputCombPlanar = FALSE;
    m_bOutputPlanar = FALSE;
    
    m_bRequestSecondBuffer = FALSE;
    m_bRotRequestSecondBuffer = FALSE;

    m_hDisplay = NULL;

    m_bVfDirectDisplay = FALSE;
    m_bVfDisplayActive = FALSE;
    m_bADCDirect = FALSE;

    m_bCombiningEnabled = FALSE;
    m_bColorKeyEnabled = FALSE;
    
    m_bRestartBufferLoop = FALSE;
    m_bRotRestartBufferLoop = FALSE;
    m_bRestartISRLoop = FALSE;

    m_bRunning = FALSE;

    m_bFlipRot = FALSE;

    m_iFrameCount = 0;

    m_iBuf0Ready = FALSE;
    m_iBuf1Ready = FALSE;

    // One-time initialization of rotation buffers
    PpAllocateRotBuffers(PP_NUM_ROT_BUFFERS, 720*576*2);  // alloc max memory size for all TV OUT modes

    DEBUGMSG(ZONE_INIT, (TEXT("%s: Spawning threads\r\n"), __WFUNCTION__));

    // Initialize thread for Postprocessor ISR
    //      pThreadAttributes = NULL (must be NULL)
    //      dwStackSize = 0 => default stack size determined by linker
    //      lpStartAddress = CspiProcessQueue => thread entry point
    //      lpParameter = NULL => point to thread parameter
    //      dwCreationFlags = 0 => no flags
    //      lpThreadId = NULL => thread ID is not returned
    PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
    m_hPpISRThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PpIntrThread, this, 0, NULL);

    if (m_hPpISRThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create camera ISR thread success\r\n"), __WFUNCTION__));
    }

    // Initialize PP buffer worker thread
    PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
    m_hPpBufThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PpBufferWorkerThread, this, 0, NULL);

    if (m_hPpBufThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create buffer worker thread success\r\n"), __WFUNCTION__));
    }

    // Initialize PP rotation buffer worker thread
    PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
    m_hPpRotBufThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PpRotBufferWorkerThread, this, 0, NULL);

    if (m_hPpRotBufThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create rotation for buffer worker thread success\r\n"), __WFUNCTION__));
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: Enabling postprocessor\r\n"), __WFUNCTION__));

    PP_FUNCTION_EXIT();

    return TRUE;

Error:
    PpDeinit();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PpDeinit
//
// This function deinitializes the Image Converter (Postprocessor).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpDeinit(void)
{

    PP_FUNCTION_ENTRY();

    // Disable Postprocessor
    PpDisable();

    CloseHandle(m_hPpIntrEvent);
    m_hPpIntrEvent = NULL;

    CloseHandle(m_hEOFEvent);
    m_hEOFEvent = NULL;

    CloseHandle(m_hReadDisplayBufferQueue);
    m_hReadDisplayBufferQueue = NULL;

    CloseHandle(m_hWriteDisplayBufferQueue);
    m_hWriteDisplayBufferQueue = NULL;

    if (m_hDisplay != NULL)
    {
        DeleteDC(m_hDisplay);
        m_hDisplay = NULL;
    }

    if (m_hPpISRThread)
    {
        CloseHandle(m_hPpISRThread);
        m_hPpISRThread = NULL;
    }

    if (m_hPpBufThread)
    {
        CloseHandle(m_hPpBufThread);
        m_hPpBufThread = NULL;
    }
}


//-----------------------------------------------------------------------------
//
// Function:  PpEnqueueBuffers
//
// This function adds buffers for the PP channel.
// Input and output buffers are provided and added to the buffer queue.
// It is assumed that the caller has allocated physically contiguous
// buffers for use in the IPU's IDMAC.
//
// Parameters:
//      pBufs
//          [in] Pointer to structure containing a pointer
//          to an input and output buffer.
//
// Returns:
//      TRUE if successful; FALSE if failed
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpEnqueueBuffers(pPpBuffers pBufs)
{
    // Enqueue input buffer
    if (!WriteMsgQueue(m_hWriteInputBufferQueue, pBufs, sizeof(ppBuffers), 0, 0))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s : Could not write to input buffer queue.  Error!\r\n"),
             __WFUNCTION__));
        return FALSE;
    }

    // Enqueue output buffer
    if (!WriteMsgQueue(m_hWriteOutputBufferQueue, pBufs, sizeof(ppBuffers), 0, 0))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s : Could not write to output buffer queue.  Error!\r\n"),
             __WFUNCTION__));
        return FALSE;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  PpClearBuffers
//
// This function clears out the input and output buffer queues
// for the Post-processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpClearBuffers()
{
    ppBuffers bufs;
    DWORD dwFlags, bytesRead;

    // Read all buffers from Input queue.
    while (ReadMsgQueue(m_hReadInputBufferQueue, &bufs, sizeof(ppBuffers), &bytesRead, 0, &dwFlags)){}

    // Read all buffers from Output queue.
    while (ReadMsgQueue(m_hReadOutputBufferQueue, &bufs, sizeof(ppBuffers), &bytesRead, 0, &dwFlags)){}
}

//-----------------------------------------------------------------------------
//
// Function:  PpGetMaxBuffers
//
// This function returns the maximum number of buffers supported
// by the postprocessor.
// with the IPU hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns the Max buffer number.
//
//-----------------------------------------------------------------------------
UINT32 PpClass::PpGetMaxBuffers(void)
{
    return PP_MAX_NUM_BUFFERS;
}


//-----------------------------------------------------------------------------
//
// Function: PpConfigure
//
// This function configures the IC registers and IDMAC
// channels for the postprocessor channel.
//
// Parameters:
//      pPpConfigureData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigure(pPpConfigData pConfigData)
{
    BOOL result = TRUE;
    ppFormat iFormat = pConfigData->outputFormat;
    ppCSCCoeffs CSCCoeffs;
    ppResizeCoeffs resizeCoeffs;
    UINT16 iResizeOutputHeight, iResizeOutputWidth;

    PP_FUNCTION_ENTRY();

    // Get display characteristics for PpClass.
    // Only perform once.
    if (!m_bDisplayInitialized)
    {
        PpInitDisplayCharacteristics();
        m_bDisplayInitialized = TRUE;
    }

    // If Post-processing enabled, configure
    if (iFormat != ppFormat_Disabled)
    {
        //-----------------------------------------------------------------
        // Setup input source
        //-----------------------------------------------------------------
        PpConfigureInput(pConfigData, ppInputChannel_Main);

        //-----------------------------------------------------------------
        // Setup combining input source (if combining requested)
        //-----------------------------------------------------------------
        if (pConfigData->bCombining)
        {
            m_bCombiningEnabled = TRUE;

            // Enable Global alpha in IC_CONF register
            INSREG32BF(&P_IPU_REGS->IC_CONF, IPU_IC_CONF_IC_GLB_LOC_A,
                IPU_IC_CONF_IC_GLB_LOC_A_GLOBAL);

            // Program Alpha for combining
            INSREG32BF(&P_IPU_REGS->IC_CMBP_1, IPU_IC_CMBP_1_IC_PP_ALPHA_V, pConfigData->alpha);

            // If the color key is all Fs, we disable color key
            if (pConfigData->colorKey == 0xFFFFFFFF)
            {
                m_bColorKeyEnabled = FALSE;
            }
            else
            {
                m_bColorKeyEnabled = TRUE;

                // Program Color Key for combining
                OUTREG32(&P_IPU_REGS->IC_CMBP_2, pConfigData->colorKey);
            }

            PpConfigureInput(pConfigData, ppInputChannel_Comb);
        }
        else
        {
            m_bCombiningEnabled = FALSE;
        }

        //-----------------------------------------------------------------
        // Setup output destination
        //-----------------------------------------------------------------
        PpConfigureOutput(pConfigData);

        //-----------------------------------------------------------------
        // Setup color space conversion
        //-----------------------------------------------------------------

        // Set up CSC for post-processing
        if (pConfigData->CSCEquation != ppCSCNoOp)
        {
            switch (pConfigData->CSCEquation)
            {
                case ppCSCR2Y_A1:
                case ppCSCR2Y_A0:
                case ppCSCR2Y_B0:
                case ppCSCR2Y_B1:
                    CSCCoeffs.C00 = rgb2yuv_tbl[pConfigData->CSCEquation][0];
                    CSCCoeffs.C01 = rgb2yuv_tbl[pConfigData->CSCEquation][1];
                    CSCCoeffs.C02 = rgb2yuv_tbl[pConfigData->CSCEquation][2];
                    CSCCoeffs.C10 = rgb2yuv_tbl[pConfigData->CSCEquation][3];
                    CSCCoeffs.C11 = rgb2yuv_tbl[pConfigData->CSCEquation][4];
                    CSCCoeffs.C12 = rgb2yuv_tbl[pConfigData->CSCEquation][5];
                    CSCCoeffs.C20 = rgb2yuv_tbl[pConfigData->CSCEquation][6];
                    CSCCoeffs.C21 = rgb2yuv_tbl[pConfigData->CSCEquation][7];
                    CSCCoeffs.C22 = rgb2yuv_tbl[pConfigData->CSCEquation][8];
                    CSCCoeffs.A0 = rgb2yuv_tbl[pConfigData->CSCEquation][9];
                    CSCCoeffs.A1 = rgb2yuv_tbl[pConfigData->CSCEquation][10];
                    CSCCoeffs.A2 = rgb2yuv_tbl[pConfigData->CSCEquation][11];
                    CSCCoeffs.Scale = rgb2yuv_tbl[pConfigData->CSCEquation][12];
                    break;

                case ppCSCY2R_A1:
                case ppCSCY2R_A0:
                case ppCSCY2R_B0:
                case ppCSCY2R_B1:
                    CSCCoeffs.C00 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][0];
                    CSCCoeffs.C01 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][1];
                    CSCCoeffs.C02 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][2];
                    CSCCoeffs.C10 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][3];
                    CSCCoeffs.C11 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][4];
                    CSCCoeffs.C12 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][5];
                    CSCCoeffs.C20 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][6];
                    CSCCoeffs.C21 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][7];
                    CSCCoeffs.C22 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][8];
                    CSCCoeffs.A0 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][9];
                    CSCCoeffs.A1 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][10];
                    CSCCoeffs.A2 = yuv2rgb_tbl[pConfigData->CSCEquation - 4][11];
                    CSCCoeffs.Scale = yuv2rgb_tbl[pConfigData->CSCEquation - 4][12];
                    break;

                case ppCSCCustom:
                    CSCCoeffs.C00 = pConfigData->CSCCoeffs.C00;
                    CSCCoeffs.C01 = pConfigData->CSCCoeffs.C01;
                    CSCCoeffs.C02 = pConfigData->CSCCoeffs.C02;
                    CSCCoeffs.C10 = pConfigData->CSCCoeffs.C10;
                    CSCCoeffs.C11 = pConfigData->CSCCoeffs.C11;
                    CSCCoeffs.C12 = pConfigData->CSCCoeffs.C12;
                    CSCCoeffs.C20 = pConfigData->CSCCoeffs.C20;
                    CSCCoeffs.C21 = pConfigData->CSCCoeffs.C21;
                    CSCCoeffs.C22 = pConfigData->CSCCoeffs.C22;
                    CSCCoeffs.A0 = pConfigData->CSCCoeffs.A0;
                    CSCCoeffs.A1 = pConfigData->CSCCoeffs.A1;
                    CSCCoeffs.A2 = pConfigData->CSCCoeffs.A2;
                    CSCCoeffs.Scale = pConfigData->CSCCoeffs.Scale;
                    break;

                default:
                    DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Invalid PP CSC equation. \r\n"), __WFUNCTION__));
            }

            // Set up the task parameter memory
            PpTaskParamConfig(&CSCCoeffs);

            // Now enable CSC in IC configuration register
            INSREG32BF(&P_IPU_REGS->IC_CONF, IPU_IC_CONF_PP_CSC1,
                IPU_IC_CONF_PP_CSC1_ENABLE);
        }
        else
        {
            // Disable CSC in IC configuration register
            INSREG32BF(&P_IPU_REGS->IC_CONF, IPU_IC_CONF_PP_CSC1,
                IPU_IC_CONF_PP_CSC1_DISABLE);
        }

        //-----------------------------------------------------------------
        // Set up resizing.
        //-----------------------------------------------------------------

        // Viewfinding path
        if (pConfigData->outputFormat != ppFormat_Disabled)
        {
            // If we are rotating, we want to resize the input
            // so that it matches the dimensions of the output
            // before rotation. (i.e., width in = height out
            // and height in = width out)
            if (pConfigData->flipRot.rotate90)
            {
                iResizeOutputWidth = pConfigData->outputSize.height;
                iResizeOutputHeight = pConfigData->outputSize.width;
            }
            else
            {
                iResizeOutputWidth = pConfigData->outputSize.width;
                iResizeOutputHeight = pConfigData->outputSize.height;
            }

            // Vertical resizing
            // Get coefficients and then set registers
            if (!PpGetResizeCoeffs(pConfigData->inputSize.height,
                iResizeOutputHeight, &resizeCoeffs))
            {
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Post-processing vertical resizing failed. \r\n"), __WFUNCTION__));
                result = FALSE;
                goto _done;
            }

            // Set downsizing field
            INSREG32BF(&P_IPU_REGS->IC_PP_RSC, IPU_IC_PP_RSC_PP_DS_R_V,
                resizeCoeffs.downsizeCoeff);

            // Set resizing field
            INSREG32BF(&P_IPU_REGS->IC_PP_RSC, IPU_IC_PP_RSC_PP_RS_R_V,
                resizeCoeffs.resizeCoeff);

            // Horizontal resizing
            // Get coefficients and then set registers
            if (!PpGetResizeCoeffs(pConfigData->inputSize.width,
                iResizeOutputWidth, &resizeCoeffs))
            {
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Viewfinding horizontal resizing failed. \r\n"), __WFUNCTION__));
                result = FALSE;
                goto _done;
            }

            // Set downsizing field
            INSREG32BF(&P_IPU_REGS->IC_PP_RSC, IPU_IC_PP_RSC_PP_DS_R_H,
                resizeCoeffs.downsizeCoeff);

            // Set resizing field
            INSREG32BF(&P_IPU_REGS->IC_PP_RSC, IPU_IC_PP_RSC_PP_RS_R_H,
                resizeCoeffs.resizeCoeff);
        }

        m_bConfigured = TRUE;
    }

_done:
    PP_FUNCTION_EXIT();
    return result;
}


//-----------------------------------------------------------------------------
//
// Function: PpInitDisplayCharacteristics
//
// This function initializes the display characteristics by
// retrieving them from the display driver.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpInitDisplayCharacteristics(void)
{
    PP_FUNCTION_ENTRY();

    m_hDisplay = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

    // Get Display Type from Display driver
    if (ExtEscape(m_hDisplay, VF_GET_DISPLAY_INFO, NULL, NULL,
        sizeof(DISPLAY_CHARACTERISTIC), (LPSTR) &m_displayCharacteristics) < 0)
    {
        DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Unable to get display characteristics! \r\n"), __WFUNCTION__));
        return FALSE;
    }

    PP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PpConfigureInput
//
// This function configures the IC registers and IDMAC
// channels for the postprocessor input source.
//
// Parameters:
//      pPpConfigureData_t
//          [in] Pointer to configuration data structure
//
//      inputChannel
//          [in] Input channel, either main PP input, or the combining PP input.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigureInput(pPpConfigData pConfigData, ppInputChannel inputChannel)
{
    BOOL result = TRUE;
    UINT16 iInputWidth, iInputHeight;
    UINT32 iInputStride;
    ppIDMACChannelParams channelParams;
    ppFormat iFormat;
    ppDataWidth iDataWidth;
    ppPixelFormat iRGBFormat;

    PP_FUNCTION_ENTRY();

    // Configure rotation BAM parameter
    channelParams.iBAM = 0;

    // Pixel Burst rate always 16 unless we are using a rotation channel
//    channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_16;
    channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_8;

    // Use proper configuration parameters for the
    // channel being configured
    if (inputChannel == ppInputChannel_Main)
    {
        iFormat = pConfigData->inputFormat;
        iDataWidth = pConfigData->inputDataWidth;
        iRGBFormat.component0_offset = pConfigData->inputRGBPixelFormat.component0_offset;
        iRGBFormat.component1_offset = pConfigData->inputRGBPixelFormat.component1_offset;
        iRGBFormat.component2_offset = pConfigData->inputRGBPixelFormat.component2_offset;
        iRGBFormat.component3_offset = pConfigData->inputRGBPixelFormat.component3_offset;
        iRGBFormat.component0_width = pConfigData->inputRGBPixelFormat.component0_width;
        iRGBFormat.component1_width = pConfigData->inputRGBPixelFormat.component1_width;
        iRGBFormat.component2_width = pConfigData->inputRGBPixelFormat.component2_width;
        iRGBFormat.component3_width = pConfigData->inputRGBPixelFormat.component3_width;

        iInputWidth = pConfigData->inputSize.width;
        iInputHeight = pConfigData->inputSize.height;
        iInputStride = pConfigData->inputStride;

        if ((iFormat == ppFormat_YUV444) || (iFormat == ppFormat_YUV422) ||
            (iFormat == ppFormat_YUV420))
        {
            m_bInputPlanar = TRUE;
        }
        else
        {
            m_bInputPlanar = FALSE;
        }
    }
    else
    {
        iFormat = pConfigData->inputCombFormat;
        iDataWidth = pConfigData->inputCombDataWidth;
        iRGBFormat.component0_offset = pConfigData->inputCombRGBPixelFormat.component0_offset;
        iRGBFormat.component1_offset = pConfigData->inputCombRGBPixelFormat.component1_offset;
        iRGBFormat.component2_offset = pConfigData->inputCombRGBPixelFormat.component2_offset;
        iRGBFormat.component3_offset = pConfigData->inputCombRGBPixelFormat.component3_offset;
        iRGBFormat.component0_width = pConfigData->inputCombRGBPixelFormat.component0_width;
        iRGBFormat.component1_width = pConfigData->inputCombRGBPixelFormat.component1_width;
        iRGBFormat.component2_width = pConfigData->inputCombRGBPixelFormat.component2_width;
        iRGBFormat.component3_width = pConfigData->inputCombRGBPixelFormat.component3_width;

        iInputWidth = pConfigData->inputCombSize.width;
        iInputHeight = pConfigData->inputCombSize.height;
        iInputStride = pConfigData->inputCombStride;

        if ((iFormat == ppFormat_YUV444) || (iFormat == ppFormat_YUV422) ||
            (iFormat == ppFormat_YUV420))
        {
            m_bInputCombPlanar = TRUE;
        }
        else
        {
            m_bInputCombPlanar = FALSE;
        }
    }

    // Set these variables to reduce pointer computations,
    // as these will be referenced several times.
    
    // The stride value used depends on whether we are
    // configuring the main input channel or the combining
    // input channel.

    //-----------------------------------------------------------------
    // Setup input format
    //-----------------------------------------------------------------

    switch(iFormat)
    {
        case ppFormat_YUV444:
            channelParams.bInterleaved = FALSE;
            channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV444;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats

            break;

        case ppFormat_YUV422:
            channelParams.bInterleaved = FALSE;
            channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats

            break;

        case ppFormat_YUV420:
            channelParams.bInterleaved = FALSE;
            channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats

            break;

        case ppFormat_YUV444IL:
            channelParams.pixelFormat.component0_offset = 0;
            channelParams.pixelFormat.component1_offset = 8;
            channelParams.pixelFormat.component2_offset = 16;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_24; // Code for 24BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV444;

            break;

        case ppFormat_YUYV422:
            channelParams.pixelFormat.component0_offset = 0;
            channelParams.pixelFormat.component1_offset = 8;
            channelParams.pixelFormat.component2_offset = 24;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;

            break;

        case ppFormat_YVYU422:
            channelParams.pixelFormat.component0_offset = 0;
            channelParams.pixelFormat.component1_offset = 24;
            channelParams.pixelFormat.component2_offset = 8;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;

            break;

        case ppFormat_UYVY422:
            channelParams.pixelFormat.component0_offset = 8;
            channelParams.pixelFormat.component1_offset = 0;
            channelParams.pixelFormat.component2_offset = 16;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;

            break;

        case ppFormat_VYUY422:
            channelParams.pixelFormat.component0_offset = 8;
            channelParams.pixelFormat.component1_offset = 16;
            channelParams.pixelFormat.component2_offset = 0;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;

            break;

        case ppFormat_RGB:
            channelParams.pixelFormat.component0_offset = iRGBFormat.component0_offset;
            channelParams.pixelFormat.component1_offset = iRGBFormat.component1_offset;
            channelParams.pixelFormat.component2_offset = iRGBFormat.component2_offset;
            channelParams.pixelFormat.component3_offset = iRGBFormat.component3_offset;
//            channelParams.pixelFormat.component3_offset = 0;
            channelParams.pixelFormat.component0_width = iRGBFormat.component0_width-1;
            channelParams.pixelFormat.component1_width = iRGBFormat.component1_width-1;
            channelParams.pixelFormat.component2_width = iRGBFormat.component2_width-1;
            channelParams.pixelFormat.component3_width = iRGBFormat.component3_width-1;
//            channelParams.pixelFormat.component3_width = 0;

            switch (iDataWidth)
            {
                case ppDataWidth_8BPP:
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8;
                    break;

                case ppDataWidth_16BPP:
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16;
                    break;

                case ppDataWidth_24BPP:
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_24;
                    break;

                case ppDataWidth_32BPP:
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_32;
                    break;

                default:
                    DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Error PP data width, %d !! \r\n"), __WFUNCTION__, iDataWidth));
                    result =  FALSE;
                    goto _inputDone;
            }

            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;

            break;

        case ppFormat_RGBA:
            // TODO: Add support for RGBA, and find out data path and use cases for RGBA
            channelParams.pixelFormat.component0_offset = 8;
            channelParams.pixelFormat.component1_offset = 16;
            channelParams.pixelFormat.component2_offset = 24;
            channelParams.pixelFormat.component3_offset = 0;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;
            channelParams.pixelFormat.component3_width = 7;

            // 32 bits per pixel for RGB data
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_32;

            break;

        default:
            DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Invalid PP input format, %d !! \r\n"), __WFUNCTION__, iFormat));
            result =  FALSE;
            goto _inputDone;
    }


    //-----------------------------------------------------------------
    // Image size validity check
    // Setup input image size
    //-----------------------------------------------------------------

    // Boundary check
    if((iInputWidth  > PP_MAX_INPUT_WIDTH) ||
        (iInputHeight > PP_MAX_INPUT_HEIGHT) ||
        (iInputWidth  < PP_MIN_INPUT_WIDTH) ||
        (iInputHeight < PP_MIN_INPUT_HEIGHT))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error input size: \r\n"), __WFUNCTION__));
        DEBUGMSG(ZONE_ERROR,
                (TEXT("\t Input Size: width (%d), height (%d)\r\n"),
                iInputWidth, iInputHeight));
        result = FALSE;
        goto _inputDone;
    }

    // Alignment check
    if(iFormat == ppFormat_YUV420)
    {
        if((iInputWidth & 0x07) ||
            (iInputHeight & 0x01))
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                    iInputWidth, iInputHeight));
            result = FALSE;
            goto _inputDone;
        }
    }
    else
    {
        if(iInputWidth & 0x03)
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                    iInputWidth, iInputHeight));
            result = FALSE;
            goto _inputDone;
        }
    }

    // Set channel parameters for frame height and width
    channelParams.iHeight = iInputHeight;
    channelParams.iWidth = iInputWidth;
    channelParams.iLineStride = iInputStride;

    // Configure the DMA channel, either the Main or Combining channel
    if (inputChannel == ppInputChannel_Main)
    {
        // Configure Channel 5 (Mem->IC for Post-processing)
        PpIDMACChannelConfig(PP_MEM_TO_IC_DMA_CHANNEL, &channelParams);
    }
    else
    {
        // Configure Channel 4 (Mem->IC for Post-processing)
        PpIDMACChannelConfig(PP_MEM_TO_IC_COMBINING_DMA_CHANNEL, &channelParams);
    }

_inputDone:
    PP_FUNCTION_EXIT();
    return result;
}


//-----------------------------------------------------------------------------
//
// Function: PpConfigureOutput
//
// This function configures the IC registers and IDMAC
// channels for the post-processor output.
//
// Parameters:
//      pPpConfigureData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigureOutput(pPpConfigData pConfigData)
{
    BOOL result = TRUE;
    UINT16 iOutputWidth, iOutputHeight, tempWidth;
    UINT32 iOutputBufSize;
    UINT32 iOutputStride;
    ppIDMACChannelParams channelParams;
    DISPLAY_SDC_FG_CONFIG_DATA displayConfig;
    ppFormat iFormat = pConfigData->outputFormat;
    UINT32 oldVal, newVal, iMask, iBitval;
    UINT32 iRotatedStride;
    DWORD MaxFrameWidth;

    PP_FUNCTION_ENTRY();

    // Configure rotation BAM parameter
    channelParams.iBAM = (pConfigData->flipRot.verticalFlip ? 1 : 0) |
        (pConfigData->flipRot.horizontalFlip ? 1 : 0) << 1  |
        (pConfigData->flipRot.rotate90 ? 1 : 0) << 2;

    m_bFlipRot = (channelParams.iBAM == 0) ? 0 : 1;

    // Set these variables to reduce pointer computations,
    // as these will be referenced several times.
    iOutputWidth = pConfigData->outputSize.width;
    iOutputHeight = pConfigData->outputSize.height;
    iOutputStride = pConfigData->outputStride;

    g_iOutputWidth = iOutputWidth;
    g_iOutputHeight = iOutputHeight;

    // This is FALSE as default, and will be set TURE if a 
    // planar format is found
    m_bOutputPlanar = FALSE;

    //-----------------------------------------------------------------
    // Setup output format
    //-----------------------------------------------------------------

    switch(iFormat)
    {
        case ppFormat_YUV444:
            channelParams.bInterleaved = FALSE;
            channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV444;
            iRotatedStride = iOutputHeight;
            iOutputBufSize = iOutputWidth * iOutputHeight * 3;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats
            m_bOutputPlanar = TRUE;

            break;

        case ppFormat_YUV422:
            channelParams.bInterleaved = FALSE;
            channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422;
            iRotatedStride = iOutputHeight;
            iOutputBufSize = (iOutputWidth * iOutputHeight) << 1;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats
            m_bOutputPlanar = TRUE;

            break;

        case ppFormat_YUV420:
            channelParams.bInterleaved = FALSE;
            channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420;
            iRotatedStride = iOutputHeight;
            iOutputBufSize = (iOutputWidth * iOutputHeight * 3) >> 1;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats
            m_bOutputPlanar = TRUE;

            break;

        case ppFormat_YUV444IL:
            channelParams.pixelFormat.component0_offset = 0;
            channelParams.pixelFormat.component1_offset = 8;
            channelParams.pixelFormat.component2_offset = 16;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            iRotatedStride = iOutputHeight * 3;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_24; // Code for 24BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV444;
            iOutputBufSize = iOutputStride * iOutputHeight;

            break;

        case ppFormat_YUYV422:
            channelParams.pixelFormat.component0_offset = 0;
            channelParams.pixelFormat.component1_offset = 8;
            channelParams.pixelFormat.component2_offset = 24;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            iRotatedStride = iOutputHeight << 1;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;
            iOutputBufSize = iOutputStride * iOutputHeight;

            break;

        case ppFormat_YVYU422:
            channelParams.pixelFormat.component0_offset = 0;
            channelParams.pixelFormat.component1_offset = 24;
            channelParams.pixelFormat.component2_offset = 8;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            iRotatedStride = iOutputHeight << 1;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;
            iOutputBufSize = iOutputStride * iOutputHeight;

            break;

        case ppFormat_UYVY422:
            channelParams.pixelFormat.component0_offset = 8;
            channelParams.pixelFormat.component1_offset = 0;
            channelParams.pixelFormat.component2_offset = 16;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            iRotatedStride = iOutputHeight << 1;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;
            iOutputBufSize = iOutputStride * iOutputHeight;

            break;

        case ppFormat_VYUY422:
            channelParams.pixelFormat.component0_offset = 8;
            channelParams.pixelFormat.component1_offset = 16;
            channelParams.pixelFormat.component2_offset = 0;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;

            iRotatedStride = iOutputHeight << 1;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16; // Code for 16BPP
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV422;
            iOutputBufSize = iOutputStride * iOutputHeight;

            break;

        case ppFormat_RGB:
            channelParams.pixelFormat.component0_offset = pConfigData->outputRGBPixelFormat.component0_offset;
            channelParams.pixelFormat.component1_offset = pConfigData->outputRGBPixelFormat.component1_offset;
            channelParams.pixelFormat.component2_offset = pConfigData->outputRGBPixelFormat.component2_offset;
            channelParams.pixelFormat.component3_offset = pConfigData->outputRGBPixelFormat.component3_offset;
//            channelParams.pixelFormat.component3_offset = 0;
            channelParams.pixelFormat.component0_width = pConfigData->outputRGBPixelFormat.component0_width-1;
            channelParams.pixelFormat.component1_width = pConfigData->outputRGBPixelFormat.component1_width-1;
            channelParams.pixelFormat.component2_width = pConfigData->outputRGBPixelFormat.component2_width-1;
            channelParams.pixelFormat.component3_width = pConfigData->outputRGBPixelFormat.component3_width-1;
//            channelParams.pixelFormat.component3_width = 0;

            switch (pConfigData->outputDataWidth)
            {
                case ppDataWidth_8BPP:
                    iRotatedStride = iOutputHeight;
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8;
                    break;

                case ppDataWidth_16BPP:
                    iRotatedStride = iOutputHeight << 1;
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_16;
                    break;

                case ppDataWidth_24BPP:
                    iRotatedStride = iOutputHeight * 3;
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_24;
                    break;

                case ppDataWidth_32BPP:
                    iRotatedStride = iOutputHeight << 2;
                    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_32;
                    break;

                default:
                    DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Error PP data width, %d !! \r\n"), __WFUNCTION__, pConfigData->outputDataWidth));
                    result =  FALSE;
                    goto _outputDone;
            }

            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
            iOutputBufSize = iOutputStride * iOutputHeight;

            break;

        case ppFormat_RGBA:
            // TODO: Add support for RGBA, and find out data path and use cases for RGBA
            channelParams.pixelFormat.component0_offset = 8;
            channelParams.pixelFormat.component1_offset = 16;
            channelParams.pixelFormat.component2_offset = 24;
            channelParams.pixelFormat.component3_offset = 0;
            channelParams.pixelFormat.component0_width = 7;
            channelParams.pixelFormat.component1_width = 7;
            channelParams.pixelFormat.component2_width = 7;
            channelParams.pixelFormat.component3_width = 7;

            // 32 bits per pixel for RGB data
            iRotatedStride = iOutputHeight << 2;
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
            iOutputBufSize = iOutputStride * iOutputHeight;
            channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_32;

            break;

        default:
            DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Invalid PP output format, %d !! \r\n"), __WFUNCTION__, pConfigData->outputFormat));
            result =  FALSE;
            goto _outputDone;
    }


    channelParams.iHeight = iOutputHeight;
    channelParams.iWidth = iOutputWidth;
    channelParams.iLineStride = iOutputStride;


    // If we are rotating, we must Reverse the width, height, and 
    // line stride for the image that is the output from the IC
    // task, and for the image that is the input to the
    // rotation task.  This way, the requested output size is
    // always achieved correctly.  If the input dimensions do
    // not match the reverse of the output dimensions, the
    // image is resized before the rotation task.
    if (pConfigData->flipRot.rotate90)
    {
        tempWidth = channelParams.iWidth;
        channelParams.iWidth = channelParams.iHeight;
        channelParams.iHeight = tempWidth;
        channelParams.iLineStride = iRotatedStride;
    }

    //-----------------------------------------------------------------
    // Image size validity check
    // Setup post-processing channel output image size
    //-----------------------------------------------------------------
    MaxFrameWidth = PPGetMaxFrameWidth();

    if((iOutputWidth  < PP_MIN_OUTPUT_WIDTH) ||
        (iOutputHeight < PP_MIN_OUTPUT_HEIGHT) ||
        (iOutputWidth  > MaxFrameWidth) ||
        (iOutputHeight > PP_MAX_OUTPUT_HEIGHT) ||
        (pConfigData->inputSize.width  > (channelParams.iWidth << 3)) ||
        (pConfigData->inputSize.height > (channelParams.iHeight << 3)))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error post-processing channel size: \r\n"), __WFUNCTION__));
        DEBUGMSG(ZONE_ERROR,
                (TEXT("\t Input Size: width (%d), height (%d)\r\n"),
                pConfigData->inputSize.width, pConfigData->inputSize.height));
        DEBUGMSG(ZONE_ERROR,
                (TEXT("\t Viewfinding Channel Size: width (%d), height (%d)\r\n"),
                channelParams.iWidth, channelParams.iHeight));
        result = FALSE;
        goto _outputDone;
    }

    // Set member variable for whether we will be 
    // displaying viewfinder data directly to display or not.
    m_bVfDirectDisplay = pConfigData->directDisplay;

    // Check validity of output format for direct display
    // (viewfinding) case.
    if (m_bVfDirectDisplay && ((iFormat != ppFormat_RGBA) && (iFormat != ppFormat_RGB)))
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid output format for direct display mode.  Must be RGB.  Disabling viewfinding.\r\n"), __WFUNCTION__));
        m_bVfDirectDisplay = FALSE;
    }

    // The following are our scenarios for using
    // the viewfinding channel:
    //      1) Direct Display + SDC: 
    //          - Set up frame sync for task chaining to SDC BG
    //          - Configure display for viewfinding
    //      2) Direct Display + ADC:
    //          - Set up frame sync for task chaining to ADC direct
    //          - Configure display for viewfinding
    //      3) No direct display:
    //          - Set up frame sync to write to memory
    //          - No configuration of display for viewfinding,
    //            as we will not be displaying viewfinding image

    if (m_bVfDirectDisplay)
    {
        // Direct display mode enabled

        // Check that requested width and height do not
        // exceed width and height supported by display
        if (pConfigData->flipRot.rotate90)
        {
            if (m_displayCharacteristics.width < iOutputHeight)
            {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Requested viewfinding width %x greater than display width %x.  Width will be set to the display width.\r\n"), 
                    __WFUNCTION__, iOutputHeight, m_displayCharacteristics.width));
            }

            if (m_displayCharacteristics.height < iOutputWidth)
            {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Requested viewfinding height %x greater than display height %x.  Height will be set to the display height. \r\n"), 
                    __WFUNCTION__, iOutputWidth, m_displayCharacteristics.height));
            }
        }
        else
        {
            if (m_displayCharacteristics.width < iOutputWidth)
            {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Requested viewfinding width %x greater than display width %x.  Width will be set to the display width.\r\n"), 
                    __WFUNCTION__, iOutputWidth, m_displayCharacteristics.width));
            }

            if (m_displayCharacteristics.height < iOutputHeight)
            {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Requested viewfinding height %x greater than display height %x.  Height will be set to the display height. \r\n"), 
                    __WFUNCTION__, iOutputHeight, m_displayCharacteristics.height));
            }
        }

        switch (m_displayCharacteristics.eType)
        {
            case eIPU_SDC:
                m_bADCDirect = FALSE;

                // Set up display structure to configure
                // display for viewfinding mode.
                displayConfig.verticalFlip = FALSE;
                displayConfig.alpha = 0xFF;
                displayConfig.colorKey = 0;
                displayConfig.plane = IPU_SDC_COM_CONF_GWSEL_FG;
                displayConfig.bpp = m_displayCharacteristics.bpp;
                if (pConfigData->flipRot.rotate90)
                {
                    displayConfig.width = iOutputHeight;
                    displayConfig.height = iOutputWidth;
                }
                else
                {
                    displayConfig.width = iOutputWidth;
                    displayConfig.height = iOutputHeight;
                }

                displayConfig.stride = displayConfig.width * displayConfig.bpp / 8;

                // Configure display for viewfinding mode
                if (ExtEscape(m_hDisplay, VF_SET_OFFSET, sizeof(POINT),
                        (LPCSTR) &pConfigData->offset, 0, NULL) <= 0)
                {
                    DEBUGMSG(ZONE_ERROR,
                          (TEXT("%s: Error initialize VF display mode. \r\n"), __WFUNCTION__));
                    return FALSE;
                }

                // Configure display for viewfinding mode
                if (ExtEscape(m_hDisplay, VF_CONFIG, sizeof(DISPLAY_SDC_FG_CONFIG_DATA),
                    (LPCSTR) &displayConfig, 0, NULL) <= 0)
                {
                    DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Error initializing VF display mode. \r\n"), __WFUNCTION__));
                    return FALSE;
                }

                // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW register
                iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL);
                iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL, 
                    IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_ARM);

                // If display type is a dumb display, we configure
                // frame synchronization to write data to SDC
                // Foreground/Background plane.
                do
                {
                    oldVal = INREG32(&P_IPU_REGS->IPU_FS_PROC_FLOW);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_FS_PROC_FLOW, 
                            oldVal, newVal) != oldVal);

                break;
            case eIPU_ADC:

                m_bADCDirect = TRUE;

                // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW register
                iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL);
                iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL, 
                    IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_ADC_DIRECT);

                // If display type is a smart display, we configure
                // frame synchronization to write data to ADC
                // Direct Viewfinding Channel
                do
                {
                    oldVal = INREG32(&P_IPU_REGS->IPU_FS_PROC_FLOW);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_FS_PROC_FLOW, 
                            oldVal, newVal) != oldVal);

                break;
            default:
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error.  Display type is neither SDC or ADC!\r\n"),
                    __WFUNCTION__));
                return FALSE;
        }
    }
    else
    {
        m_bADCDirect = FALSE;

        // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW register
        iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL);
        iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL, 
            IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_ARM);

        // Direct display mode is disabled, so the destination
        // should be system memory.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_FS_PROC_FLOW);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_FS_PROC_FLOW, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW register
        iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL) |
            CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL) |
            CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL) |
            CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL);
        iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL, 
            IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_ARM) |
            CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL, 
            IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_ARM) |
            CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL, 
            IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL_ARM) |
            CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL, 
            IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL_ARM);

        // Direct display mode is disabled, so the ADC source
        // should be system memory.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_FS_DISP_FLOW);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_FS_DISP_FLOW, 
                    oldVal, newVal) != oldVal);
    }

    channelParams.iBAM = 0;

    // Pixel Burst rate always 16 unless we are using a rotation channel
//    channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_16;
    channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_8;

    // Set viewfinding channel parameters
    PpIDMACChannelConfig(PP_IC_TO_MEM_DMA_CHANNEL, &channelParams);

    if (m_bFlipRot)
    {
        // Rotation/Flipping datapath

        // First, allocate rotation buffers, if they have not been allocated,
        // or if the size needs to be modified.
        /*
        if (!m_bRotBuffersAllocated || (m_iLastAllocatedBufferSize != iOutputBufSize))
        {
            pRotBufferManager->DeleteBuffers();
            PpAllocateRotBuffers(PP_NUM_ROT_BUFFERS, iOutputBufSize);
        }
        */
        pRotBufferManager->PrintBufferInfo();

        // Burst length is 8 for rotation channels
        channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_8;

        channelParams.iBAM = (pConfigData->flipRot.verticalFlip ? 1 : 0) |
            (pConfigData->flipRot.horizontalFlip ? 1 : 0) << 1  |
            (pConfigData->flipRot.rotate90 ? 1 : 0) << 2;

        // Set viewfinding channel parameters (Mem->IC for rotation)
        PpIDMACChannelConfig(PP_MEM_TO_IC_ROT_DMA_CHANNEL, &channelParams);

        // If we are rotating, we must Reverse the width, height, and 
        // line stride again, back to the original output width, height,
        // and stride.  This way, the requested output size is
        // always achieved correctly.
        if (pConfigData->flipRot.rotate90)
        {
            tempWidth = channelParams.iWidth;
            channelParams.iWidth = channelParams.iHeight;
            channelParams.iHeight = tempWidth;
            channelParams.iLineStride = iOutputStride;
        }

        channelParams.iBAM = 0;

        // Set viewfinding channel parameters (IC->Mem after rotation)
        PpIDMACChannelConfig(PP_IC_TO_MEM_ROT_DMA_CHANNEL, &channelParams);
    }
    else
    {
        // Delete PP buffers for rotation.
        // If buffers have not yet been created, this will simply return.
/*
        if (!pRotBufferManager->DeleteBuffers())
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s : Failed to delete buffers!\r\n"), __WFUNCTION__));
            return FALSE;
        }
*/
        m_bRotBuffersAllocated = FALSE;
    }

_outputDone:
    PP_FUNCTION_EXIT();
    return result;
}

//-----------------------------------------------------------------------------
//
// Function: PpStartChannel
//
// This function starts the postprocessing channel.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpStartChannel()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    PP_FUNCTION_ENTRY();

    // PP must have been configured at least once 
    // in order to start the channel
    if (!m_bConfigured)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error.  Cannot start post-processing channel without first configuring\r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    if (m_bRunning)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Viewfinding channel already running.\r\n"), __WFUNCTION__));
        return TRUE;
    }

    // Enable Postprocessor
    PpEnable();

    m_bRestartBufferLoop = TRUE;
    m_bRestartISRLoop = TRUE;
    m_bRunning = TRUE;
    m_iCurrentBuf = 0;

    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause Pin to believe a frame was ready.
    ResetEvent(m_hEOFEvent);

    if (m_bCombiningEnabled)
    {
        if (m_bColorKeyEnabled)
        {
            // Enable Color Key in IC_CONF register
            INSREG32BF(&P_IPU_REGS->IC_CONF, IPU_IC_CONF_IC_KEY_COLOR_EN,
                IPU_IC_CONF_IC_KEY_COLOR_EN_ENABLE);
        }

        // Enable PP Combining in IC_CONF register
        INSREG32BF(&P_IPU_REGS->IC_CONF, IPU_IC_CONF_PP_CMB,
            IPU_IC_CONF_PP_CMB_ENABLE);
    }

    if (m_bFlipRot)
    {
        m_bRotRestartBufferLoop = TRUE;

        // Protect access to IPU_INT_CTRL_1, IPU_CHA_DB_MODE_SEL,
        // IDMAC_CHA_EN, and IC_CONF registers.

        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_12)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_ENABLE);

        // Enable IPU interrupts for channel 12 (IC->Mem after rotation).
        // This is the final channel in the chain, so we want this channel
        // to trigger an interrupt that the camera IST will handle.
        // Also Enable IPU interrupts for channel 2 (IC->Mem after PP).
        // This will allow us to inform the waiting thread that
        // we need a new buffer.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        if (m_bADCDirect)
        {
            // Compute bitmask and shifted bit value for IPU_INT_CTRL_3 register
            iMask = CSP_BITFMASK(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN);
            iBitval = CSP_BITFVAL(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN, IPU_ENABLE);

            // Enable IPU interrupts for channel 12 (IC->Mem after rotation).
            // This is the final channel in the chain, so we want this channel
            // to trigger an interrupt that the camera IST will handle.
            // Also Enable IPU interrupts for channel 2 (IC->Mem after PP).
            // This will allow us to inform the waiting thread that
            // we need a new buffer.
            do
            {
                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_3, 
                        oldVal, newVal) != oldVal);
        }


        // Compute bitmask and shifted bit value for DB mode sel register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_5)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_13)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_12);
        // If combining is requested, add DMA Channel 4
        if (m_bCombiningEnabled)
        {
            iMask |= CSP_BITFMASK(IPU_DMA_CHA_DMAIC_4);
        }

        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_DUB_BUF)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DUB_BUF)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_13, IPU_DUB_BUF)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_DUB_BUF);
        // If combining is requested, add DMA Channel 4
        if (m_bCombiningEnabled)
        {
            iBitval |= CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_DUB_BUF);
        }

        // Set double-buffering for all channels used
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        // Set current buffer bits, so that we will start on buffer 0.
        SETREG32(&P_IPU_REGS->IPU_CHA_CUR_BUF, 
            CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_IPU_CHA_CUR_BUF_CLEAR)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_IPU_CHA_CUR_BUF_CLEAR)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_IPU_CHA_CUR_BUF_CLEAR)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_13, IPU_IPU_CHA_CUR_BUF_CLEAR)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_IPU_CHA_CUR_BUF_CLEAR));

        // Compute bitmask and shifted bit value for IDMAC enable register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_5)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_13)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_12);
        // If combining is requested, add DMA Channel 4
        if (m_bCombiningEnabled)
        {
            iMask |= CSP_BITFMASK(IPU_DMA_CHA_DMAIC_4);
        }

        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_13, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_ENABLE);
        // If combining is requested, add DMA Channel 4
        if (m_bCombiningEnabled)
        {
            iBitval |= CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_ENABLE);
        }

        // Set the bits to enable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PP_EN)
            | CSP_BITFMASK(IPU_IC_CONF_PP_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PP_EN, IPU_IC_CONF_PP_EN_ENABLE)
            | CSP_BITFVAL(IPU_IC_CONF_PP_ROT_EN, IPU_IC_CONF_PP_ROT_EN_ENABLE);

        // Enable postprocessing path in IC and 
        // enable postprocessing for rotation path in IC.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IC_CONF, 
                    oldVal, newVal) != oldVal);

        // If direct display mode is enabled, enable display.
        if (m_bVfDirectDisplay && !m_bADCDirect)
        {
            // Enable Viewfinding in the display driver
            if (ExtEscape(m_hDisplay, VF_ENABLE, 0, NULL, 0, NULL) <= 0)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed enabling display for viewfinding.\r\n"), __WFUNCTION__));
                return FALSE;
            }
            m_bVfDisplayActive = TRUE;
        }

        // Set up and start buffer 0, and
        // set variable m_bVfRequestSecondBuffer to ensure
        // that buffer 1 is requested subsequently.

        // Set next-to-fill buffer to buffer 0
        m_iBufferToFill = 0;

        // Fill buffer 1 after buffer 0 filled for Chan 2
        m_bRequestSecondBuffer = TRUE;

        // Request buffer for IDMAC Channel 1 (IC->Mem for viewfinding)
        // and for Channel 11 (Mem-IC for rotation for viewfinding).
        // This will attempt to set up buffer 0.
        SetEvent(m_hRequestBuffer);

        // Set next-to-fill buffer to buffer 0
        m_iRotBufferToFill = 0;
    }
    else
    {
        // Protect access to IPU_INT_CTRL_1, IPU_CHA_DB_MODE_SEL,
        // IDMAC_CHA_EN, and IC_CONF registers.

        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_ENABLE);

        // enable IPU interrupts for channel 2 (IC->Mem)
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        if (m_bADCDirect)
        {
            // Compute bitmask and shifted bit value for IPU_INT_CTRL_3 register
            iMask = CSP_BITFMASK(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN);
            iBitval = CSP_BITFVAL(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN, IPU_ENABLE);

            // Enable IPU interrupts for channel 12 (IC->Mem after rotation).
            // This is the final channel in the chain, so we want this channel
            // to trigger an interrupt that the camera IST will handle.
            // Also Enable IPU interrupts for channel 2 (IC->Mem after PP).
            // This will allow us to inform the waiting thread that
            // we need a new buffer.
            do
            {
                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_3, 
                        oldVal, newVal) != oldVal);
        }

        // Compute bitmask and shifted bit value for DB mode sel register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_5)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2);
        // If combining is requested, add DMA Channel 4
        if (m_bCombiningEnabled)
        {
            iMask |= CSP_BITFMASK(IPU_DMA_CHA_DMAIC_4);
        }

        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_DUB_BUF)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DUB_BUF);
        // If combining is requested, add DMA Channel 4
        if (m_bCombiningEnabled)
        {
            iBitval |= CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_DUB_BUF);
        }

        // Set double-buffering for all channels used
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);


        // Set current buffer bits, so that we will start on buffer 0.
        SETREG32(&P_IPU_REGS->IPU_CHA_CUR_BUF, 
            CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_IPU_CHA_CUR_BUF_CLEAR)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_IPU_CHA_CUR_BUF_CLEAR));

        // Compute bitmask and shifted bit value for IDMAC enable register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_5)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2);
        // If combining is requested, add DMA Channel 4
        if (m_bCombiningEnabled)
        {
            iMask |= CSP_BITFMASK(IPU_DMA_CHA_DMAIC_4);
        }

        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_ENABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_ENABLE);
        // If combining is requested, add DMA Channel 4
        if (m_bCombiningEnabled)
        {
            iBitval |= CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_ENABLE);
        }

        // Set the bits to enable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PP_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PP_EN, IPU_IC_CONF_PP_EN_ENABLE);

        // enable post-processing path in IC
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IC_CONF, 
                    oldVal, newVal) != oldVal);

        // If direct display mode is enabled, enable display.
        if (m_bVfDirectDisplay && !m_bADCDirect)
        {
            // Enable Viewfinding in the display driver
            if (ExtEscape(m_hDisplay, VF_ENABLE, 0, NULL, 0, NULL) <= 0)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed enabling display for viewfinding.\r\n"), __WFUNCTION__));
                return FALSE;
            }
            m_bVfDisplayActive = TRUE;
        }

        // Set up and start buffer 0, and
        // set variable m_bRequestSecondBuffer to ensure
        // that buffer 1 is requested subsequently.

        // Set next-to-fill buffer to buffer 0
        m_iBufferToFill = 0;

        // Fill buffer 1 after buffer 0 filled
        m_bRequestSecondBuffer = TRUE;

        // Request buffer for IDMAC Channel 2 (IC->Mem after 
        // post-processing).
        // This will attempt to set up buffer 1.
        SetEvent(m_hRequestBuffer);
    }

    dumpIpuRegisters(P_IPU_REGS);

    PP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PpStopChannel
//
// This function halts the post-processing channels.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpStopChannel(void)
{
    DISPLAY_BUFFER dispBufferData;
    DWORD dwFlags, bytesRead;
    UINT32 uTempReg, uTempReg1, uTempReg2, uTempReg3, uCount = 0;
    UINT32 oldVal, newVal, iMask, iBitval;

    PP_FUNCTION_ENTRY();

    EnterCriticalSection(&m_csStopping);

    // If not running, return
    if (m_bRunning == FALSE)
    {
        LeaveCriticalSection(&m_csStopping);    
        return TRUE;
    }

    m_bRunning = FALSE;

    // Disable Postprocessor
    PpDisable();

    // initalize ... for the first time through
    uTempReg = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);
    uTempReg1 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_PP_TSTAT);
    uTempReg2 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_PP_ROT_TSTAT);
    uTempReg3 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_ADC_PPCHAN_LOCK);

    // We can't disable tasks until the active channels
    // have completed their current frames.  Make sure
    // that tasks are not active and that channels are
    // not busy (indicating that channels are still running).
    while ((uTempReg1 == IPU_IPU_TASKS_STAT_ACTIVE) || (uTempReg2 == IPU_IPU_TASKS_STAT_ACTIVE)
            || (uTempReg3 == IPU_IPU_TASKS_ADC_CHAN_LOCKED) || (uTempReg & 0x3024))
    {
        if (uCount <= 1000)
        {
            //..give up the remainder of time slice
            Sleep(1);
            uCount++;

            //.. need to check after the sleep delay
            uTempReg = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);
            uTempReg1 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_PP_TSTAT);
            uTempReg2 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_PP_ROT_TSTAT);
            uTempReg3 = EXTREG32BF(&P_IPU_REGS->IPU_TASKS_STAT, IPU_IPU_TASKS_STAT_ADC_PPCHAN_LOCK);
        }
        else
        {
            //.. there is something wrong ....break out
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s(): Error in stopping viewfinding channel!\r\n"), __WFUNCTION__));
            break;
        }
    }

    LeaveCriticalSection(&m_csStopping);

    // Protect access to IC_CONF register.
    // Disable IC tasks.  Tasks will stop on completion
    // of the current frame.

    // Compute bitmask and shifted bit value for IC CONF register
    if (m_bFlipRot)
    {
        iMask = CSP_BITFMASK(IPU_IC_CONF_PP_EN)
            | CSP_BITFMASK(IPU_IC_CONF_PP_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PP_EN, IPU_IC_CONF_PP_EN_DISABLE)
            | CSP_BITFVAL(IPU_IC_CONF_PP_ROT_EN, IPU_IC_CONF_PP_ROT_EN_DISABLE);
    }
    else
    {
        iMask = CSP_BITFMASK(IPU_IC_CONF_PP_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PP_EN, IPU_IC_CONF_PP_EN_DISABLE);        
    }

    // Use interlocked function to Disable IC tasks.
    do
    {
        oldVal = INREG32(&P_IPU_REGS->IC_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IC_CONF, 
                oldVal, newVal) != oldVal);


    ResetEvent(m_hRequestBuffer);
    ResetEvent(m_hRequestRotBuffer);
    ResetEvent(m_hEOFEvent);

    // If direct display mode was enabled, empty VF buffers
    // from queue and disable display.
    if (m_bVfDirectDisplay && !m_bADCDirect)
    {
        // Clear the VF Buffer queue by reading out the contents
        while (ReadMsgQueue(m_hReadDisplayBufferQueue, &dispBufferData, 
            sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
        {
        }

        // Disable Viewfinding in the display driver
        if (ExtEscape(m_hDisplay, VF_DISABLE, 0, NULL, 0, NULL) <= 0)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed disabling display for PP viewfinding.\r\n"), __WFUNCTION__));
            return FALSE;
        }

        m_bVfDisplayActive = FALSE;
    }

    if (m_bFlipRot)
    {
        // Protect access to IDMAC_CHA_EN, IPU_CHA_DB_MODE_SEL
        // and IPU_INT_CTRL_1 registers.

        // Compute bitmask and shifted bit value for IDMAC_EN register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_5)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_4)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_13)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_12);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_13, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_DISABLE);

        // Use interlocked function to 
        // set the bits to disable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_4)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_13)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_12);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_13, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_DISABLE);

        // Use interlocked function to turn off double buffering.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        if (m_bADCDirect)
        {
            // Compute bitmask and shifted bit value for INT_CTRL_3 register
            iMask = CSP_BITFMASK(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN);
            iBitval = CSP_BITFVAL(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN, IPU_DISABLE);

            // Use interlocked function to turn off double buffering.
            do
            {
                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_3, 
                    oldVal, newVal) != oldVal);
        }

        // Compute bitmask and shifted bit value for INT_CTRL register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_12)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DISABLE);

        // Use interlocked function to disable IPU 
        // interrupts for post-processing channels, including rotation.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);

        // Reset the state of the viewfinding buffers, so that
        // everything will be in place if the channel is restarted.
        pRotBufferManager->ResetBuffers();
    }
    else
    {
        // Protect access to IDMAC_CHA_EN, IPU_CHA_DB_MODE_SEL,
        // and IPU_INT_CTRL_1 registers.

        // Compute bitmask and shifted bit value for DMAIC enable register
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_5)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_4)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DISABLE);

        // Use interlocked function to disable the IDMAC channels.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                    oldVal, newVal) != oldVal);

        // Compute bitmask and shifted bit value for DB mode sel register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_4)
            | CSP_BITFMASK(IPU_DMA_CHA_DMAIC_5);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_DISABLE)
            | CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_DISABLE);

        // Use interlocked function to turn off double buffering.
        // This assures that we return to buffer 0 when we next 
        // start up this channel.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_CHA_DB_MODE_SEL);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_CHA_DB_MODE_SEL, 
                    oldVal, newVal) != oldVal);

        if (m_bADCDirect)
        {
            // Compute bitmask and shifted bit value for INT_CTRL_3 register
            iMask = CSP_BITFMASK(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN);
            iBitval = CSP_BITFVAL(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN, IPU_DISABLE);

            // Use interlocked function to turn off double buffering.
            do
            {
                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_3, 
                    oldVal, newVal) != oldVal);
        }

        // Compute bitmask and shifted bit value for INT CTRL register.
        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2);
        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DISABLE);

        // Use interlocked function to disable IPU interrupts 
        // for viewfinding channel.
        do
        {
            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                    oldVal, newVal) != oldVal);
    }

    // Disable Combining
    if (m_bCombiningEnabled)
    {
        // Enable Color Key in IC_CONF register
        INSREG32BF(&P_IPU_REGS->IC_CONF, IPU_IC_CONF_IC_KEY_COLOR_EN,
            IPU_IC_CONF_IC_KEY_COLOR_EN_DISABLE);

        // Disable PP Combining in IC_CONF register
        INSREG32BF(&P_IPU_REGS->IC_CONF, IPU_IC_CONF_PP_CMB,
            IPU_IC_CONF_PP_CMB_DISABLE);
    }

    PP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpPauseViewfinding
//
// This function pauses the viewfinding output display, while leaving
// the viewfinding channel running.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpPauseViewfinding(void)
{
    // If direct display mode was enabled, disable display.
    if (m_bVfDirectDisplay && m_bVfDisplayActive)
    {
        // Disable Viewfinding in the display driver
        if (ExtEscape(m_hDisplay, VF_DISABLE, 0, NULL, 0, NULL) <= 0)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed disabling display for viewfinding.\r\n"), __WFUNCTION__));
            return FALSE;
        }

        m_bVfDisplayActive = FALSE;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PpGetFrameCount
//
// This function returns the current postprocessing channel frame count.
//
// Parameters:
//      None.
//
// Returns:
//      The number of PP frames processed since the
//      last configuration request.
//
//-----------------------------------------------------------------------------
UINT32 PpClass::PpGetFrameCount()
{
    return m_iFrameCount;
}

//-----------------------------------------------------------------------------
//
// Function:  PpAllocateotBuffers
//
// This function allocates buffers for the rotation for viewfinding channel
// and adds each to the buffer queue.  This function must be called
// in order to allocate physically contiguous buffers for use in
// the IPU's IDMAC.
//
// Parameters:
//      numBuffers
//          [in] Number of buffers to allocate and add
//          to the queues.
//
//      bufSize
//          [in] Size of buffer to create and enqueue.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpAllocateRotBuffers(UINT32 numBuffers, UINT32 bufSize)
{
    PP_FUNCTION_ENTRY();

    // Initialize MsgQueues when buffers are allocated
    if (!pRotBufferManager->AllocateBuffers(numBuffers, bufSize))
    {
        return FALSE;
    }

    m_iLastAllocatedBufferSize = bufSize;

    m_bRotBuffersAllocated = TRUE;

    PP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: controlledWriteDMAChannelParam
//
// This function uses Interlocked APIs to access IPU_IMA registers
// and then initializes the IDMAC channel parameters.
//
// Parameters:
//      channel
//          [in] String to identify buffer 1 EOF event.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::controlledWriteDMAChannelParam(int channel, int row, int word, unsigned int data)
{
    unsigned addr  =
        0x10000                            // MEM_NU = DMA CPM
        + (((channel * 2) + row)<< 3)    // ROW_NU = (channel*2) << 3
        + word;                            // WORD_NU

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.

    for (;;)
    {
        if (INREG32(&P_IPU_REGS->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_IMA_ADDR, 0, addr) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    OUTREG32(&P_IPU_REGS->IPU_IMA_DATA, data);
    
    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);
}


//-----------------------------------------------------------------------------
//
// Function: controlledReadDMAChannelParam
//
// This function uses Interlocked APIs to access IPU_IMA registers
// and then read the IDMAC channel parameters.
//
// Parameters:
//      channel
//          [in] DMA channel number.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
unsigned int PpClass::controlledReadDMAChannelParam(int channel, int row, int word)
{
    unsigned int data;
    unsigned addr  =
        0x10000                            // MEM_NU = DMA CPM
        + (((channel * 2) + row)<< 3)    // ROW_NU = (channel*2) << 3
        + word;                            // WORD_NU

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.

    for (;;)
    {
        if (INREG32(&P_IPU_REGS->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_IMA_ADDR, 0, addr) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    data = INREG32(&P_IPU_REGS->IPU_IMA_DATA);

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);

    return data;
}

//-----------------------------------------------------------------------------
//
// Function: writeDMAChannelParam
//
// This function initializes the IDMAC channel parameters.
//
// Parameters:
//      channel
//          [in] String to identify buffer 1 EOF event.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::writeDMAChannelParam(int channel, int row, int word, unsigned int data)
{
    unsigned addr  =
        0x10000                            // MEM_NU = DMA CPM
        + (((channel * 2) + row)<< 3)    // ROW_NU = (channel*2) << 3
        + word;                            // WORD_NU

    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, addr);
    OUTREG32(&P_IPU_REGS->IPU_IMA_DATA, data);
}

//-----------------------------------------------------------------------------
//
// Function: PpIDMACChannelConfig
//
// This function initializes the IDMAC channel parameters.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpIDMACChannelConfig(UINT8 ch,
    pPpIDMACChannelParams pChannelParams)
{
    UINT32 newVal;

    PP_FUNCTION_ENTRY();

    dumpChannelParams(pChannelParams);

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.

    // Set IPU_IMA_ADDR to 1 to gain control of IPU_IMA registers.
    newVal = 1;

    for (;;)
    {
        if (INREG32(&P_IPU_REGS->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_IMA_ADDR, 0, newVal) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    if (pChannelParams->bInterleaved)
    {
        writeDMAChannelParam(ch, 0, 0, 0); //Variable data set to 0    31-0
        writeDMAChannelParam(ch, 0, 1, 1<<14); //Variable data set to 0. NSB set  63-32
        writeDMAChannelParam(ch, 0, 2, 0); //Variable data set to 0.   95-64
        writeDMAChannelParam(ch, 0, 3, ((pChannelParams->iWidth-1) << 12) |
                ((pChannelParams->iHeight-1) << 24)); // 127-96
        writeDMAChannelParam(ch, 0, 4, ((pChannelParams->iHeight-1) >> 8)); // 160-128

        // Notice we do not write row 1, words 0 or 1, as these 
        // words control the DMA channel buffers.  This data
        // is written when PpStartChannel is called.
        writeDMAChannelParam(ch, 1, 2,
            ((pChannelParams->iBitsPerPixelCode) |
            ((pChannelParams->iLineStride - 1)<<3) |
            (pChannelParams->iFormatCode<<17) | (pChannelParams->iBAM<<20) |
            (pChannelParams->iPixelBurstCode<<25))); // 95-64
        writeDMAChannelParam(ch, 1, 3,
            2 | // SAT code of 0x10 = 32 bit memory access
            (pChannelParams->pixelFormat.component0_offset<<3) |
            (pChannelParams->pixelFormat.component1_offset<<8) |
            (pChannelParams->pixelFormat.component2_offset<<13) |
            (pChannelParams->pixelFormat.component3_offset<<18) |
            (pChannelParams->pixelFormat.component0_width<<23) |
            (pChannelParams->pixelFormat.component1_width<<26)  |
            (pChannelParams->pixelFormat.component2_width<<29) ); //   127-96
        writeDMAChannelParam(ch, 1, 4, pChannelParams->pixelFormat.component3_width); //  160-128
    }
    else
    {
        writeDMAChannelParam(ch, 0, 0, 0);  //Variable data set to 0   31-0
        writeDMAChannelParam(ch, 0, 1, ((1<<(46-32))));  //Variable data set to 0. NSB set  63-32
        // Here, we skip configuration of U and V offsets, as they will be configured
        // in the buffer worker routines.
        writeDMAChannelParam(ch, 0, 3, (((pChannelParams->iWidth-1)<<12) | ((pChannelParams->iHeight-1)<<24))); // 127-96
        writeDMAChannelParam(ch, 0, 4, ((pChannelParams->iHeight-1) >> 8));    // 160-128

        writeDMAChannelParam(ch, 1, 2,
            (3 | ((pChannelParams->iLineStride - 1)<<3) |
            (pChannelParams->iFormatCode<<17) | (pChannelParams->iBAM<<20) |
            (pChannelParams->iPixelBurstCode<<25))); // 95-64
        writeDMAChannelParam(ch, 1, 3, 0); //   98- 96
    }

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);

    PP_FUNCTION_EXIT();
    return;
}


//-----------------------------------------------------------------------------
//
// Function: controlledWriteICTaskParam
//
// This function uses Interlocked APIs to access IPU_IMA registers
// and then initializes the IDMAC channel parameters.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::controlledWriteICTaskParam(int row, int word, unsigned int data)
{
    unsigned addr  =
        0x00000                         // MEM_NU = IC TPM
        | ((row)<< 3)                   // ROW_NU = row
        | word;                         // WORD_NU

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.

    for (;;)
    {
        if (INREG32(&P_IPU_REGS->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_IMA_ADDR, 0, addr) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    OUTREG32(&P_IPU_REGS->IPU_IMA_DATA, data);

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);
}


//-----------------------------------------------------------------------------
//
// Function: writeICTaskParam
//
// This function initializes the IDMAC channel parameters.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::writeICTaskParam(int row, int word, unsigned int data)
{
    unsigned addr  =
        0x00000                         // MEM_NU = IC TPM
        | ((row)<< 3)                   // ROW_NU = row
        | word;                         // WORD_NU

    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, addr);
    OUTREG32(&P_IPU_REGS->IPU_IMA_DATA, data);
}

//-----------------------------------------------------------------------------
//
// Function: PpTaskParamConfig
//
// This function sets up the IC task parameter memory
// to configure color space conversion parameters.
//
// Parameters:
//      pCSCCoeffs
//          [in] Pointer to the CSC coefficients to use in the conversion.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpTaskParamConfig(pPpCSCCoeffs pCSCCoeffs)
{
    UINT32 newVal;
    DWORD MaxFrameWidth;

    PP_FUNCTION_ENTRY();

    dumpCoeffs(pCSCCoeffs);

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.
    
    // Set IPU_IMA_ADDR to 1 to gain control of IPU_IMA registers.
    newVal = 1;

    for (;;)
    {
        if (INREG32(&P_IPU_REGS->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_IMA_ADDR, 0, newVal) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    MaxFrameWidth = PPGetMaxFrameWidth();
    writeICTaskParam(3*MaxFrameWidth+0xc, 0, pCSCCoeffs->C22 | (pCSCCoeffs->C11 << 9) |
        (pCSCCoeffs->C00 <<18) | (pCSCCoeffs->A0 <<27));
    writeICTaskParam(3*MaxFrameWidth+0xc, 1, (pCSCCoeffs->A0 >> 5) | (pCSCCoeffs->Scale<<8) | (0 << 10));    //Scale=2, sat=0

    writeICTaskParam(3*MaxFrameWidth+0xd, 0, pCSCCoeffs->C20 | (pCSCCoeffs->C10 << 9) |
        (pCSCCoeffs->C01 <<18) | (pCSCCoeffs->A1 <<27));
    writeICTaskParam(3*MaxFrameWidth+0xd, 1, (pCSCCoeffs->A1 >> 5));

    writeICTaskParam(3*MaxFrameWidth+0xe, 0, pCSCCoeffs->C21 | (pCSCCoeffs->C12 << 9) |
        (pCSCCoeffs->C02 <<18) | (pCSCCoeffs->A2 <<27));
    writeICTaskParam(3*MaxFrameWidth+0xe, 1, (pCSCCoeffs->A2 >> 5));

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);

    PP_FUNCTION_EXIT();
    return;
}


//-----------------------------------------------------------------------------
//
// Function: PpGetResizeCoeffs
//
// This function computes the resizing coefficients from
// the input and output size.
//
// Parameters:
//      inSize
//          [in] Input size (height or width)
//
//      outSize
//          [in] Output size (height of width)
//
//      resizeCoeffs
//          [out] downsizing and resizing coefficients computed.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpGetResizeCoeffs(UINT16 inSize, UINT16 outSize, pPpResizeCoeffs resizeCoeffs)
{
    UINT16 tempSize;
    UINT16 tempDownsize;
    UINT16 tempOutSize;
    DWORD MaxFrameWidth;

    PP_FUNCTION_ENTRY();

    // Cannot downsize more than 8:1
    if ((outSize << 3) < inSize)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Maximum downsize ratio is 8:1.  Input Size specified: %d.  Output Size specified: %d. \r\n"),
            __WFUNCTION__, inSize, outSize));
        return FALSE;
    }

    // compute downsizing coefficient
    tempDownsize = 0;
    tempSize = inSize;
    tempOutSize = outSize << 1;
    //tempsize must be smaller than outSize x 2, the rest part can be handled by resize module.
    while ((tempSize >= tempOutSize) && (tempDownsize < 2))
    {
        tempSize >>= 1;
        tempDownsize++;
    }
    MaxFrameWidth = PPGetMaxFrameWidth();
    //The main processing section can't accept the resolution larger than 1024
    while((tempSize > MaxFrameWidth)&&(tempDownsize < 2))
    {
        tempSize >>= 1;
        tempDownsize++;
    }
    resizeCoeffs->downsizeCoeff = tempDownsize;

    // compute resizing coefficient using the following equation:
    //      resizeCoeff = M*(SI -1)/(SO - 1)
    //      where M = 2^13, SI - input size, SO - output size
    resizeCoeffs->resizeCoeff =  8192 * (tempSize - 1) / (outSize - 1);

    PP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PpEnable
//
// Enable the Image Converter and the IDMAC channels we will need.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpEnable(void)
{
    DWORD dwBytesTransferred;
    IPU_DRIVER driver = IPU_DRIVER_PP;

    PP_FUNCTION_ENTRY();

    if (!DeviceIoControl(hIPUBase,         // file handle to the driver
             IPU_IOCTL_ENABLE_IC,          // I/O control code
             &driver,                      // in buffer
             sizeof(IPU_DRIVER),           // in buffer size
             NULL,                         // out buffer
             0,                            // out buffer size
             &dwBytesTransferred,           // number of bytes returned
             NULL))                        // ignored (=NULL)
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to enable IC!\r\n"), __WFUNCTION__));
    }

    //dumpIpuRegisters(P_IPU_REGS);

    PP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PpDisable
//
// Disable the Image Converter and the Postprocessing IDMAC channels.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpDisable(void)
{
    DWORD dwBytesTransferred;
    IPU_DRIVER driver = IPU_DRIVER_PP;

    PP_FUNCTION_ENTRY();

    if (!DeviceIoControl(hIPUBase,         // file handle to the driver
             IPU_IOCTL_DISABLE_IC,          // I/O control code
             &driver,                      // in buffer
             sizeof(IPU_DRIVER),           // in buffer size
             NULL,                         // out buffer
             0,                            // out buffer size
             &dwBytesTransferred,           // number of bytes returned
             NULL))                        // ignored (=NULL)
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to disable IC!\r\n"), __WFUNCTION__));
    }

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpClearInterruptStatus
//
// This function is used to clear the CAM interrupt status and signal to the
// kernel that interrupt processing is completed.
//
// Parameters:
//      clearBitmask
//          [in] Mask of bits in status register to clear
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PpClass::PpClearInterruptStatus(DWORD clearBitmask)
{
    PP_FUNCTION_ENTRY();

    // Clear Interrupt Status Bits
    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_1, clearBitmask);

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpIntrThread
//
// This function is the IST thread.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PpClass::PpIntrThread(LPVOID lpParameter)
{
    PpClass *pPp = (PpClass *)lpParameter;

    PP_FUNCTION_ENTRY();

    BSPSetPPISRPriority();

    pPp->PpISRLoop(INFINITE);

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpISRLoop
//
// This function is the interrupt handler for the Postprocessor.
// It waits for the End-Of-Frame (EOF) interrupt, and signals
// the EOF event registered by the user of the postprocessor.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PpClass::PpISRLoop(UINT32 timeout)
{
    DWORD statReg1, statReg3;
    DISPLAY_BUFFER dispBufferData;
    DWORD dwFlags, bytesRead;
    UINT32 buf0_rdy, buf1_rdy, idmac_busy, tasks_stat;
    UINT32 oldVal, newVal, iMask, iBitval;

    PP_FUNCTION_ENTRY();

    // loop here
    for (;;)
    {
        DEBUGMSG (ZONE_DEVICE, (TEXT("%s: In the loop\r\n"), __WFUNCTION__));

        if (WaitForSingleObject(m_hPpIntrEvent, timeout) == WAIT_OBJECT_0)
        {
            // Thread is running properly, so we do not have to restart.
            m_bRestartISRLoop = FALSE;

            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Interrupt received\r\n"), __WFUNCTION__));

            statReg1 = INREG32(&P_IPU_REGS->IPU_INT_STAT_1);
            statReg3 = INREG32(&P_IPU_REGS->IPU_INT_STAT_3);
            buf0_rdy = INREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY);
            buf1_rdy = INREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY);
            idmac_busy = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);
            tasks_stat = INREG32(&P_IPU_REGS->IPU_TASKS_STAT);
            //dumpInterruptRegisters(P_IPU_REGS);
            if (statReg1 & 0x1004)
            {
                // TODO: Remove, for debug purposes only
                //generateBMP(g_iOutputPhysAddr, g_iOutputWidth, g_iOutputHeight);

                // EOF for Post-processor
                DEBUGMSG (ZONE_DEVICE, (TEXT("%s:*** Post-processing End of frame interrupt ***\r\n"), __WFUNCTION__));

                // Rotation Case
                if (m_bFlipRot)
                {
                    // Critical section prevents race condition on
                    // reading and writing ready and busy bits
                    EnterCriticalSection(&m_csStopping);

                    if (!m_bRunning)
                    {
                        // Don't do anything else.  We do not want to re-enable
                        // the buffer ready bits since we are stopping.
                    }
                    else if (m_bRestartISRLoop)
                    {
                        // This code ensures that we do not run into problems
                        // if the PP channel was stopped at an earlier time while
                        // this thread was waiting at the m_csStopping critical 
                        // section.  If so, we simply start over and wait for a new
                        // interrupt.

                        LeaveCriticalSection(&m_csStopping);
                        continue;
                    }
                    else
                    {
                        // If Channel 2 has completed
                        if (statReg1 & 0x004)
                        {
                            // Clear interrupt status bits
                            PpClearInterruptStatus(0x004);

                            // Re-enable control bits so that we are able to receive
                            // interrupts for subsequent tasks in the pipeline

                            // Protect access to IPU_INT_CTRL_1 register.

                            // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                            iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2);
                            iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_ENABLE);

                            // Enable IPU interrupt for channel 2 (IC->Mem after PP).
                            // This will allow us to inform the waiting thread that
                            // we need a new buffer.
                            do
                            {
                                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                                newVal = (oldVal & (~iMask)) | iBitval;
                            } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                    oldVal, newVal) != oldVal);

                            // We do not trigger EOF to camera 
                            // upper layers, because we have not
                            // necessarily completed rotation yet,
                            // just the postprocessing is complete.

                            if (m_iCurrentBuf == 0)
                            {
                                m_iBuf0Ready = FALSE;
                                m_iCurrentBuf = 1;

                                // Check for the case where both buffer 0 and buffer 1
                                // completed before the ISR could service the interrupt.
                                if (m_iBuf1Ready && !(buf1_rdy & 0x4) && !(idmac_busy & 0x4))
                                {
                                    // Both buffers have completed
                                    DEBUGMSG(ZONE_ERROR, 
                                      (TEXT("%s : It happened (Buf0 current)!!! $$$.\r\n"), __WFUNCTION__));

                                    // Proceed through step to process EOF for Buffer 1.

                                    m_bRotRequestSecondBuffer = TRUE;

                                    // Buffer 1 has also completed, and buffer 0
                                    // is still the next active buffer.
                                    m_iBuf1Ready = FALSE;
                                    m_iCurrentBuf = 0;
                                }
                            }
                            else if (m_iCurrentBuf == 1)
                            {
                                m_iBuf1Ready = FALSE;
                                m_iCurrentBuf = 0;

                                if (m_iBuf0Ready && !(buf0_rdy & 0x4) && !(idmac_busy & 0x4))
                                {
                                    // Both buffers have completed
                                    DEBUGMSG(ZONE_ERROR, 
                                        (TEXT("%s : It happened (Buf1 current)!!! $$$.\r\n"), __WFUNCTION__));

                                    m_bRotRequestSecondBuffer = TRUE;

                                    // Buffer 0 has also completed, and buffer 1
                                    // is still the next active buffer.
                                    m_iBuf0Ready = FALSE;
                                    m_iCurrentBuf = 1;
                                }
                            }

                            // Request buffer for IDMAC Channel 12 (IC->Mem after rotation).
                            // This kicks off the rotation channel.
                            SetEvent(m_hRequestRotBuffer);
                        }

                        // If Channels 13 and 12 have completed
                        if (statReg1 & 0x1000)
                        {
                            // increment frame count
                            m_iFrameCount++;

                            // Trigger Viewfinder EOF event
                            SetEvent(m_hEOFEvent);

                            // Clear interrupt status bits
                            PpClearInterruptStatus(0x1000);

                            // Protect access to IPU_INT_CTRL_1 register.

                            // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                            iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_12);
                            iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_ENABLE);

                            // Enable IPU interrupts for channel 12 (IC->Mem after rotation).
                            // This is the final channel in the chain, so we want this channel
                            // to trigger an interrupt that the camera IST will handle.
                            do
                            {
                                oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                                newVal = (oldVal & (~iMask)) | iBitval;
                            } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                    oldVal, newVal) != oldVal);

                            // Set rotation buffer back to idle
                            pRotBufferManager->SetFilledBuffer();
                            pRotBufferManager->GetBufferFilled();

                            SetEvent(m_hRequestBuffer);

                            if (m_bVfDirectDisplay && !m_bADCDirect)
                            {
                                // Read current viewfinding buffer from queue
                                if (!ReadMsgQueue(m_hReadDisplayBufferQueue, &dispBufferData, sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
                                {
                                    DEBUGMSG(ZONE_ERROR, 
                                        (TEXT("%s : Couldn't read from VF buffer queue.  Skipping SetBuffer\r\n"), __WFUNCTION__));
                                    LeaveCriticalSection(&m_csStopping);
                                    continue;
                                }

                                // Set Buffer in the display driver
                                if (ExtEscape(m_hDisplay, VF_BUF_SET, sizeof(PHYSICAL_ADDRESS),
                                    (LPCSTR) &dispBufferData.paBuf, 0, NULL) <= 0)
                                {
                                    DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed setting buffer in display.\r\n"), __WFUNCTION__));
                                }
                            }
                        }
                    }

                    LeaveCriticalSection(&m_csStopping);
                }
                // No Rotation case
                else
                {
                    // Critical section prevents race condition on
                    // reading and writing ready and busy bits
                    EnterCriticalSection(&m_csStopping);

                    if (!m_bRunning)
                    {
                        // Don't do anything else.  We do not want to re-enable
                        // the buffer ready bits since we are stopping.
                    }
                    else if (m_bRestartISRLoop)
                    {
                        // This code ensures that we do not run into problems
                        // if the VF channel was stopped at an earlier time while
                        // this thread was waiting at the m_csVfStopping critical 
                        // section.  If so, we simply start over and wait for a new
                        // interrupt.

                        LeaveCriticalSection(&m_csStopping);
                        continue;
                    }
                    else
                    {
                        // Clear interrupt status bits
                        PpClearInterruptStatus(0x004);

                        // increment frame count
                        m_iFrameCount++;

                        // Trigger PP EOF event
                        SetEvent(m_hEOFEvent);

                        // Protect access to IPU_INT_CTRL_1 register.

                        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAIC_2);
                        iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_ENABLE);

                        // Enable IPU interrupt for channel 2 (IC->Mem after PP).
                        // This will allow us to inform the waiting thread that
                        // we need a new buffer.
                        do
                        {
                            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                            newVal = (oldVal & (~iMask)) | iBitval;
                        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                oldVal, newVal) != oldVal);

                        if (m_iCurrentBuf == 0)
                        {
                            m_iBuf0Ready = FALSE;
                            m_iCurrentBuf = 1;

                            // Check for the case where both buffer 0 and buffer 1
                            // completed before the ISR could service the interrupt.
                            if (m_iBuf1Ready && !(buf1_rdy & 0x4) && !(idmac_busy & 0x4))
                            {
                                // Both buffers have completed
                                DEBUGMSG(ZONE_ERROR, 
                                  (TEXT("%s : It happened (Buf0 current)!!! $$$.\r\n"), __WFUNCTION__));

                                // Proceed through steps to process EOF for Buffer 0.

                                // increment frame count
                                m_iFrameCount++;

                                // Request a new buffer
                                SetEvent(m_hRequestBuffer);

                                if (m_bVfDirectDisplay && !m_bADCDirect)
                                {
                                    // Read current viewfinding buffer from 
                                    // queue. We do not need to do anything 
                                    // with this data, as we will read again
                                    // for Buffer 1.
                                    if (!ReadMsgQueue(m_hReadDisplayBufferQueue, &dispBufferData, 
                                        sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
                                    {
                                        DEBUGMSG(ZONE_ERROR, 
                                            (TEXT("%s : Error!  Couldn't read from VF buffer queue.\r\n"), __WFUNCTION__));
                                    }
                                }

                                // Buffer 1 has also completed, and buffer 0
                                // is still the next active buffer.
                                m_iBuf1Ready = FALSE;
                                m_iCurrentBuf = 0;
                            }
                        }
                        else if (m_iCurrentBuf == 1)
                        {
                            m_iBuf1Ready = FALSE;
                            m_iCurrentBuf = 0;

                            if (m_iBuf0Ready && !(buf0_rdy & 0x4) && !(idmac_busy & 0x4))
                            {
                                // Both buffers have completed
                                DEBUGMSG(ZONE_ERROR, 
                                    (TEXT("%s : It happened (Buf1 current)!!! $$$.\r\n"), __WFUNCTION__));

                                // Proceed through steps to process EOF for Buffer 1.

                                // increment frame count
                                m_iFrameCount++;

                                // Request a new buffer
                                SetEvent(m_hRequestBuffer);

                                if (m_bVfDirectDisplay && !m_bADCDirect)
                                {
                                    // Read current viewfinding buffer from 
                                    // queue. We do not need to do anything 
                                    // with this data, as we will read again
                                    // for Buffer 0.
                                    if (!ReadMsgQueue(m_hReadDisplayBufferQueue, &dispBufferData,
                                        sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
                                    {
                                        DEBUGMSG(ZONE_ERROR, 
                                            (TEXT("%s : Error!  Couldn't read from VF buffer queue.\r\n"), __WFUNCTION__));
                                    }
                                }

                                // Buffer 0 has also completed, and buffer 1
                                // is still the next active buffer.
                                m_iBuf0Ready = FALSE;
                                m_iCurrentBuf = 1;
                            }
                        }

                        // Request a new buffer
                        SetEvent(m_hRequestBuffer);

                        if (m_bVfDirectDisplay && !m_bADCDirect)
                        {
                            // Read current viewfinding buffer from queue
                            if (!ReadMsgQueue(m_hReadDisplayBufferQueue, &dispBufferData, sizeof(DISPLAY_BUFFER), &bytesRead, 0, &dwFlags))
                            {
                                DEBUGMSG(ZONE_ERROR, 
                                    (TEXT("%s : Couldn't read from VF buffer queue.  Skipping SetBuffer\r\n"), __WFUNCTION__));
                                LeaveCriticalSection(&m_csStopping);
                                continue;
                            }

                            // Set Buffer in the display driver
                            if (ExtEscape(m_hDisplay, VF_BUF_SET, sizeof(PHYSICAL_ADDRESS),
                                (LPCSTR) &dispBufferData.paBuf, 0, NULL) <= 0)
                            {
                                DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed setting buffer in display.\r\n"), __WFUNCTION__));
                            }
                        }
                    }

                    LeaveCriticalSection(&m_csStopping);
                }
            }
            else if (statReg3 & 0x100000)
            {
                // ADC direct to PP channel has completed

                // Critical section prevents race condition on
                // reading and writing ready and busy bits
                EnterCriticalSection(&m_csStopping);

                if (!m_bRunning)
                {
                    // Don't do anything else.  We do not want to re-enable
                    // the buffer ready bits since we are stopping.
                }
                else if (m_bRestartISRLoop)
                {
                    // This code ensures that we do not run into problems
                    // if the VF channel was stopped at an earlier time while
                    // this thread was waiting at the m_csVfStopping critical 
                    // section.  If so, we simply start over and wait for a new
                    // interrupt.

                    LeaveCriticalSection(&m_csStopping);
                    continue;
                }
                else
                {
                    // Clear interrupt status bits
                    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_3, 0x100000);

                    // increment frame count
                    m_iFrameCount++;

                    // Trigger PP EOF event
                    SetEvent(m_hEOFEvent);

                    // Protect access to IPU_INT_CTRL_3 register.

                    // Compute bitmask and shifted bit value for IPU_INT_CTRL_3 register
                    iMask = CSP_BITFMASK(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN);
                    iBitval = CSP_BITFVAL(IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN, IPU_ENABLE);

                    // Enable IPU interrupt for ADC Direct From PP.
                    // This will allow us to inform the waiting thread that
                    // we need a new buffer.
                    do
                    {
                        oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_3);
                        newVal = (oldVal & (~iMask)) | iBitval;
                    } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_3, 
                            oldVal, newVal) != oldVal);

                    if (m_iCurrentBuf == 0)
                    {
                        m_iBuf0Ready = FALSE;
                        m_iCurrentBuf = 1;

                        // Check for the case where both buffer 0 and buffer 1
                        // completed before the ISR could service the interrupt.
                        if (m_iBuf1Ready && !(tasks_stat & 0x20000000))
                        {
                            // Both buffers have completed
                            DEBUGMSG(ZONE_ERROR, 
                                (TEXT("%s : It happened (Buf0 current)!!! $$$.\r\n"), __WFUNCTION__));

                            // Proceed through steps to process EOF for Buffer 0.

                            // increment frame count
                            m_iFrameCount++;

                            // Request a new buffer
                            SetEvent(m_hRequestBuffer);

                            // Buffer 1 has also completed, and buffer 0
                            // is still the next active buffer.
                            m_iBuf1Ready = FALSE;
                              m_iCurrentBuf = 0;
                        }
                    }
                    else if (m_iCurrentBuf == 1)
                    {
                        m_iBuf1Ready = FALSE;
                        m_iCurrentBuf = 0;

                        if (m_iBuf0Ready && !(tasks_stat & 0x20000000))
                        {
                            // Both buffers have completed
                            DEBUGMSG(ZONE_ERROR, 
                                (TEXT("%s : It happened (Buf1 current)!!! $$$.\r\n"), __WFUNCTION__));

                            // Proceed through steps to process EOF for Buffer 1.

                            // increment frame count
                            m_iFrameCount++;

                            // Request a new buffer
                            SetEvent(m_hRequestBuffer);

                            // Buffer 0 has also completed, and buffer 1
                            // is still the next active buffer.
                            m_iBuf0Ready = FALSE;
                            m_iCurrentBuf = 1;
                        }
                    }

                    // Request a new buffer
                    SetEvent(m_hRequestBuffer);

                    LeaveCriticalSection(&m_csStopping);
                }
            }

            if (INREG32(&P_IPU_REGS->IPU_INT_STAT_5) & 0xFFFF)
            {
                // TODO: Properly Handle Error Cases
                DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Error Interrupt received!\r\n"), __WFUNCTION__));
                if (INREG32(&P_IPU_REGS->IPU_INT_STAT_5) & 0x3800)
                {
                    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Frame Lost.\r\n"), __WFUNCTION__));
                    // Clear frame drop interrupt registers
                    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_5, 0x3800);
                    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Cleared INT_STAT_5: %x\r\n"),
                            __WFUNCTION__, INREG32(&P_IPU_REGS->IPU_INT_STAT_5)));
                }
                else
                {
                    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Other error.\r\n"), __WFUNCTION__));
                }
            }
        }
        else
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Time out\r\n"), __WFUNCTION__));
        }

    }

#if 0 // Remove-W4: Warning C4702 workaround
    PP_FUNCTION_EXIT();
#endif

    return;
}


//------------------------------------------------------------------------------
//
// Function: PpBufferWorkerThread
//
// This function is the worker thread for the handling buffers.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PpClass::PpBufferWorkerThread(LPVOID lpParameter)
{
    PpClass *pPp = (PpClass *)lpParameter;

    PP_FUNCTION_ENTRY();

    BSPSetPPBufferThreadPriority();

    pPp->PpBufferWorkerRoutine(INFINITE);

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpBufferWorkerRoutine
//
// This function is the worker thread routine that assures that
// the buffers are managed correctly in the postprocessor.  When
// a buffer is filled, this function will wait for a new buffer to
// become available and set up the channel to run.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PpClass::PpBufferWorkerRoutine(UINT32 timeout)
{
    PHYSICAL_ADDRESS inputPhysAddr, inputCombPhysAddr, outputPhysAddr;
    ppBuffers inputBufs, outputBufs;
    UINT32 *rotAddr;
    DISPLAY_BUFFER dispBufferData;
    DWORD dwFlags, bytesRead;
    UINT32 tempVal;
    UINT32 outputUOffset = 0;
    UINT32 outputVOffset = 0;

    PP_FUNCTION_ENTRY();

    outputPhysAddr.QuadPart = 0;

    for (;;)
    {
        // Wait until the ISR loop receives a filled buffer, at which
        // time it will signal that it needs a new buffer.
        WaitForSingleObject(m_hRequestBuffer, timeout);

        // Thread running properly, so we can set this to FALSE;
        m_bRestartBufferLoop = FALSE;

        DEBUGMSG(ZONE_DEVICE, (_T("%s: PP buffer requested\r\n"), __WFUNCTION__));

        // A buffer has been requested.  Attempt to read from the input
        // queue to get a buffer.  If one is not available,
        // we wait here until it is.

        // First, wait for a buffer to be ready in the input queue.
        WaitForSingleObject(m_hReadInputBufferQueue, INFINITE);

        // This code ensures that if we do not run into problems
        // if the PP channel was stopped at an earlier time while
        // this thread was waiting.  If so,
        // we simply start over at the top of the loop.
        if (m_bRestartBufferLoop)
        {
            continue;
        }

        // If rotation enabled, we must also wait for a buffer 
        // to be ready in the rotation queue.  Otherwise,
        // we must wait for a buffer in the output queue.
        if (m_bFlipRot)
        {
            pRotBufferManager->SetActiveBuffer(&rotAddr, INFINITE);
            outputPhysAddr.QuadPart = (LONGLONG) rotAddr;
        }
        else
        {
            WaitForSingleObject(m_hReadOutputBufferQueue, INFINITE);
        }

        // Critical section to prevent race condition upon
        // stopping the post-processing channel
        EnterCriticalSection(&m_csStopping);

        // This code ensures that if we do not run into problems
        // if the PP channel was stopped at an earlier time while
        // this thread was waiting.  If so,
        // we simply start over at the top of the loop.
        // Also, if we are stopping the PP channel, return 
        // active buffer to ready queue, and bail out
        // of buffer request routine, as we do not want to
        // continue processing in this channel
        if (m_bRestartBufferLoop || !m_bRunning)
        {
            if (m_bFlipRot)
            {
                pRotBufferManager->ResetBuffers();
            }
            LeaveCriticalSection(&m_csStopping);
            continue;
        }

        // Now, read input buffer queue to retrieve buffer pointer.
        if (!ReadMsgQueue(m_hReadInputBufferQueue, &inputBufs, sizeof(ppBuffers), &bytesRead, 0, &dwFlags))
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Error: No input buffer to read from input queue\r\n"), __WFUNCTION__));
        }

        inputPhysAddr.QuadPart = (LONGLONG) inputBufs.inputBuf;

        // Read output buffer queue to retrieve buffer pointer,
        // if we aren't rotating.
        if (!m_bFlipRot)
        {
            if (!ReadMsgQueue(m_hReadOutputBufferQueue, &outputBufs, sizeof(ppBuffers), &bytesRead, 0, &dwFlags))
            {
                DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Error: No output buffer to read from input queue\r\n"), __WFUNCTION__));
            }

            outputPhysAddr.QuadPart = (LONGLONG) outputBufs.outputBuf;
            outputUOffset = outputBufs.outputUBufOffset;
            outputVOffset = outputBufs.outputVBufOffset;
        }
        else
        {
            outputUOffset = inputBufs.outputUBufOffset;
            outputVOffset = inputBufs.outputVBufOffset;
        }

        // TODO: Remove, for debug purposes only
//        g_iOutputPhysAddr = outputPhysAddr.LowPart;

        pRotBufferManager->PrintBufferInfo();

        DEBUGMSG(ZONE_DEVICE, (_T("%s: PP buffers ready for Buf %d\r\n"), __WFUNCTION__, m_iBufferToFill));

        DEBUGMSG(ZONE_DEVICE,
            (TEXT("%s: pPhysical address (input): %x.\r\n"), __WFUNCTION__, inputPhysAddr.QuadPart));
        DEBUGMSG(ZONE_DEVICE,
            (TEXT("%s: pPhysical address (output): %x.\r\n"), __WFUNCTION__, outputPhysAddr.QuadPart));

        // Program input buffer into task parameter memory
        controlledWriteDMAChannelParam(5, 1, m_iBufferToFill, (unsigned int)inputPhysAddr.QuadPart); //Buffer 0    31-0

        // If surface is YUV planar, program U and V buffer offsets
        if (m_bInputPlanar)
        {
            controlledWriteDMAChannelParam(5, 0, 1, ((1<<(46-32)) |
                (((unsigned int)inputBufs.inputUBufOffset)<<(53-32))));  //Variable data set to 0. NSB set  63-32
            controlledWriteDMAChannelParam(5, 0, 2, ((((unsigned int)inputBufs.inputUBufOffset)>>(64-53)) |
                (((unsigned int)inputBufs.inputVBufOffset)<<(79-64))));  //Variable data set to 0.    95-64

            // read then write this DMA channel param, since we need to
            // preserve some of the data written to this region
            tempVal = controlledReadDMAChannelParam(5, 0, 3);
            controlledWriteDMAChannelParam(5, 0, 3, tempVal | (((unsigned int)inputBufs.inputVBufOffset)>>(96-79))); // 127-96
        }

        //ReadVfDMA(P_IPU_REGS, PP_MEM_TO_IC_DMA_CHANNEL);

        // If combining is requested, configure DMA Channel 4 for Combining input
        if (m_bCombiningEnabled)
        {
            inputCombPhysAddr.QuadPart = (LONGLONG) inputBufs.inputCombBuf;

            // Program input buffer into task parameter memory
            controlledWriteDMAChannelParam(4, 1, m_iBufferToFill, (unsigned int)inputCombPhysAddr.QuadPart); //Buffer 0    31-0

            //ReadVfDMA(P_IPU_REGS, PP_MEM_TO_IC_COMBINING_DMA_CHANNEL);

            // If surface is YUV planar, program U and V buffer offsets
            if (m_bInputCombPlanar)
            {
                controlledWriteDMAChannelParam(4, 0, 1, ((1<<(46-32)) |
                    (((unsigned int)inputBufs.inputCombUBufOffset)<<(53-32))));  //Variable data set to 0. NSB set  63-32
                controlledWriteDMAChannelParam(4, 0, 2, ((((unsigned int)inputBufs.inputCombUBufOffset)>>(64-53)) |
                    (((unsigned int)inputBufs.inputCombVBufOffset)<<(79-64))));  //Variable data set to 0.    95-64

                // read then write this DMA channel param, since we need to
                // preserve some of the data written to this region
                tempVal = controlledReadDMAChannelParam(4, 0, 3);
                controlledWriteDMAChannelParam(4, 0, 3, tempVal | (((unsigned int)inputBufs.inputCombVBufOffset)>>(96-79))); // 127-96
            }
        }

        // Only set up output buffer for non-ADC Direct case.
        if (!m_bADCDirect)
        {
            // Program output buffer into task parameter memory
            controlledWriteDMAChannelParam(2, 1, m_iBufferToFill, (unsigned int)outputPhysAddr.QuadPart); //Buffer 0    31-0

            // If surface is YUV planar, program U and V buffer offsets
            if (m_bOutputPlanar)
            {
                controlledWriteDMAChannelParam(2, 0, 1, ((1<<(46-32)) |
                    (((unsigned int)outputUOffset)<<(53-32))));  //Variable data set to 0. NSB set  63-32
                controlledWriteDMAChannelParam(2, 0, 2, ((((unsigned int)outputUOffset)>>(64-53)) |
                    (((unsigned int)outputVOffset)<<(79-64))));  //Variable data set to 0.    95-64

                // read then write this DMA channel param, since we need to
                // preserve some of the data written to this region
                tempVal = controlledReadDMAChannelParam(2, 0, 3);
                controlledWriteDMAChannelParam(2, 0, 3, tempVal | (((unsigned int)outputVOffset)>>(96-79))); // 127-96
            }
        }

        //ReadVfDMA(P_IPU_REGS, PP_IC_TO_MEM_DMA_CHANNEL);

        if (m_iBufferToFill == 0)
        {
            // If combining is requested, configure DMA Channel 4 for Combining input
            if (m_bCombiningEnabled)
            {
                // Buffer 0 ready for IDMAC Channel 4 (IC->Mem for PP combining)
                SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY,
                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_DMA_CHA_READY));
            }

            // Buffer 0 ready for IDMAC Channel 5 (IC->Mem for post-processing)
            SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_DMA_CHA_READY));

            // Only set up output buffer for non-ADC Direct case.
            if (!m_bADCDirect)
            {
                // Buffer 0 ready for IDMAC Channel 2 (IC->Mem after PP)
                SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY,
                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DMA_CHA_READY));
            }
        }
        else
        {
            // If combining is requested, configure DMA Channel 4 for Combining input
            if (m_bCombiningEnabled)
            {
                // Buffer 1 ready for IDMAC Channel 4 (IC->Mem for PP combining)
                SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY,
                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_4, IPU_DMA_CHA_READY));
            }

            // Buffer 1 ready for IDMAC Channel 5 (IC->Mem for post-processing)
            SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                CSP_BITFVAL(IPU_DMA_CHA_DMAIC_5, IPU_DMA_CHA_READY));

            // Only set up output buffer for non-ADC Direct case.
            if (!m_bADCDirect)
            {
                // Buffer 1 ready for IDMAC Channel 2 (IC->Mem after PP)
                SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY,
                    CSP_BITFVAL(IPU_DMA_CHA_DMAIC_2, IPU_DMA_CHA_READY));
            }
        }

        if (m_bFlipRot)
        {
            // In rotation case, we program rotation buffer into
            // Channel 13 (Mem->IC for Rotation after PP).
            controlledWriteDMAChannelParam(13, 1, m_iBufferToFill, (unsigned int)outputPhysAddr.QuadPart); //Buffer 0    31-0
            
            // If the output surface is YUV planar, program U and V buffer offsets
            // Use input offsets, since they reflect the unrotated offsets.
            if (m_bOutputPlanar)
            {
                controlledWriteDMAChannelParam(13, 0, 1, ((1<<(46-32)) |
                    (((unsigned int)outputUOffset)<<(53-32))));  //Variable data set to 0. NSB set  63-32
                controlledWriteDMAChannelParam(13, 0, 2, ((((unsigned int)outputUOffset)>>(64-53)) |
                    (((unsigned int)outputVOffset)<<(79-64))));  //Variable data set to 0.    95-64

                // read then write this DMA channel param, since we need to
                // preserve some of the data written to this region
                tempVal = controlledReadDMAChannelParam(13, 0, 3);
                controlledWriteDMAChannelParam(13, 0, 3, tempVal | (((unsigned int)outputVOffset)>>(96-79))); // 127-96
            }
        }
        else if (m_bVfDirectDisplay && !m_bADCDirect)
        {
            // If no rotation and direct display, pass the address requested 
            // to the display for viewfinding.
            dispBufferData.eSrcBuf = m_iBufferToFill ? eBUF_1 : eBUF_0;
            dispBufferData.paBuf.QuadPart = (LONGLONG) outputPhysAddr.QuadPart;

            // Enqueue newly created buffer
            if (!WriteMsgQueue(m_hWriteDisplayBufferQueue, &dispBufferData, sizeof(DISPLAY_BUFFER), 0, 0))
            {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s : Could not write to buffer queue.  Error!\r\n"), __WFUNCTION__));
                LeaveCriticalSection(&m_csStopping);
                continue;
            }
        }

        if (m_iBufferToFill == 0)
        {
            m_iBuf0Ready = TRUE;
        }
        else
        {
            m_iBuf1Ready = TRUE;
        }

        // Flip which buffer is next to fill
        m_iBufferToFill = m_iBufferToFill ? 0: 1;

        if (m_bRequestSecondBuffer)
        {
            // Request that another buffer be filled.
            SetEvent(m_hRequestBuffer);

            m_bRequestSecondBuffer = FALSE;
        }

        dumpIpuRegisters(P_IPU_REGS);

        LeaveCriticalSection(&m_csStopping);
    }

#if 0 // Remove-W4: Warning C4702 workaround
    PP_FUNCTION_EXIT();
#endif

    return;
}

//------------------------------------------------------------------------------
//
// Function: PpRotBufferWorkerThread
//
// This function is the worker thread for the handling buffers.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PpClass::PpRotBufferWorkerThread(LPVOID lpParameter)
{
    PpClass *pPp = (PpClass *)lpParameter;

    PP_FUNCTION_ENTRY();

    BSPSetPPRotBufferThreadPriority();

    pPp->PpRotBufferWorkerRoutine(INFINITE);

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpRotBufferWorkerRoutine
//
// This function is the worker thread routine that assures that
// the buffers are managed correctly in the postprocessor.  When
// a buffer is filled, this function will wait for a new buffer to
// become available and set up the channel to run.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PpClass::PpRotBufferWorkerRoutine(UINT32 timeout)
{
    PHYSICAL_ADDRESS outputPhysAddr;
    DWORD dwFlags, bytesRead;
    ppBuffers outputBufs;
    UINT32 tempVal;

    PP_FUNCTION_ENTRY();

    for (;;)
    {
        // Wait until the ISR loop receives a filled buffer, at which
        // time it will signal that it needs a new buffer.
        WaitForSingleObject(m_hRequestRotBuffer, timeout);

        // Thread running properly, so we can set this to FALSE;
        m_bRotRestartBufferLoop = FALSE;

        DEBUGMSG(ZONE_DEVICE, (_T("%s: Rot buffer requested\r\n"), __WFUNCTION__));

        // A buffer has been requested.  Attempt to read from the output
        // queue to get a buffer.  If one is not available,
        // we wait here until it is.

        // First, wait for a buffer to be ready in the output queue.
        WaitForSingleObject(m_hReadOutputBufferQueue, INFINITE);

        // Critical section to prevent race condition upon
        // stopping the PP channel
        EnterCriticalSection(&m_csStopping);

        // This code ensures that if we do not run into problems
        // if the PP channel was stopped at an earlier time while
        // this thread was waiting in WaitForSingleObjectr.  If so,
        // we simply start over at the top of the loop.
        // Also, if we are stopping the PP channel, bail out
        // of buffer request routine, as we do not want to
        // continue processing in this channel
        if (m_bRotRestartBufferLoop || !m_bRunning)
        {
            LeaveCriticalSection(&m_csStopping);
            continue;
        }

        DEBUGMSG(ZONE_DEVICE, (_T("%s: PP rotation buffer ready for Buf %d\r\n"), __WFUNCTION__, m_iRotBufferToFill));

        // Read output buffer queue to retrieve buffer pointer.
        if (!ReadMsgQueue(m_hReadOutputBufferQueue, &outputBufs, sizeof(ppBuffers), &bytesRead, 0, &dwFlags))
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Error: No input buffer to read from input queue\r\n"), __WFUNCTION__));
        }

        outputPhysAddr.QuadPart = (LONGLONG) outputBufs.outputBuf;

        DEBUGMSG(ZONE_DEVICE,
            (TEXT("%s: Physical address: %x.\r\n"), __WFUNCTION__, outputPhysAddr.QuadPart));

        // Program buffer into task parameter memory
        controlledWriteDMAChannelParam(12, 1, m_iRotBufferToFill, (unsigned int)outputPhysAddr.QuadPart); //Buffer 0    31-0

        // If the output surface is YUV planar, program U and V buffer offsets
        if (m_bOutputPlanar)
        {
            controlledWriteDMAChannelParam(12, 0, 1, ((1<<(46-32)) |
                (((unsigned int)outputBufs.outputUBufOffset)<<(53-32))));  //Variable data set to 0. NSB set  63-32
            controlledWriteDMAChannelParam(12, 0, 2, ((((unsigned int)outputBufs.outputUBufOffset)>>(64-53)) |
                (((unsigned int)outputBufs.outputVBufOffset)<<(79-64))));  //Variable data set to 0.    95-64

            // read then write this DMA channel param, since we need to
            // preserve some of the data written to this region
            tempVal = controlledReadDMAChannelParam(12, 0, 3);
            controlledWriteDMAChannelParam(12, 0, 3, tempVal | (((unsigned int)outputBufs.outputVBufOffset)>>(96-79))); // 127-96
        }

        if (m_iRotBufferToFill == 0)
        {
            // Proceed through step to process EOF for Buffer 0.

            // Buffer 0 ready for IDMAC Channel 13 (Mem->IC for rotation)
            SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                CSP_BITFVAL(IPU_DMA_CHA_DMAIC_13, IPU_DMA_CHA_READY));

            // Buffer 0 ready for IDMAC Channel 12 (IC->Mem after rotation)
            SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
                CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_DMA_CHA_READY));
        }
        else
        {
            // Buffer 1 ready for IDMAC Channel 13 (Mem->IC for rotation)
            SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                CSP_BITFVAL(IPU_DMA_CHA_DMAIC_13, IPU_DMA_CHA_READY));

            // Buffer 1 ready for IDMAC Channel 12 (IC->Mem after rotation)
            SETREG32(&P_IPU_REGS->IPU_CHA_BUF1_RDY, 
                CSP_BITFVAL(IPU_DMA_CHA_DMAIC_12, IPU_DMA_CHA_READY));
        }

        m_iRotBufferToFill = m_iRotBufferToFill ? 0 : 1;

        if (m_bRotRequestSecondBuffer)
        {
            // Request that another buffer be filled.
            SetEvent(m_hRequestRotBuffer);

            m_bRotRequestSecondBuffer = FALSE;
        }

        dumpIpuRegisters(P_IPU_REGS);

        LeaveCriticalSection(&m_csStopping);
    }

#if 0 // Remove-W4: Warning C4702 workaround
    PP_FUNCTION_EXIT();
#endif

    return;
}


//------------------------------------------------------------------------------
//
// Function: dumpChannelParams
//
//
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
static void dumpChannelParams(pPpIDMACChannelParams pChannelParams)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pChannelParams);

    DEBUGMSG (ZONE_DEVICE, (TEXT("bInterleaved= %s\r\n"), pChannelParams->bInterleaved ? "TRUE" : "FALSE"));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Width       = %x\r\n"), pChannelParams->iWidth));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Height      = %x\r\n"), pChannelParams->iHeight));
    DEBUGMSG (ZONE_DEVICE, (TEXT("BPP Code    = %x\r\n"), pChannelParams->iBitsPerPixelCode));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Line Stride = %x\r\n"), pChannelParams->iLineStride));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Format Code = %x\r\n"), pChannelParams->iFormatCode));
    DEBUGMSG (ZONE_DEVICE, (TEXT("BAM         = %x\r\n"), pChannelParams->iBAM));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Pixel Burst = %x\r\n"), pChannelParams->iPixelBurstCode));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixWidth0   = %x\r\n"), pChannelParams->pixelFormat.component0_width));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixWidth1   = %x\r\n"), pChannelParams->pixelFormat.component1_width));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixWidth2   = %x\r\n"), pChannelParams->pixelFormat.component2_width));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixWidth3   = %x\r\n"), pChannelParams->pixelFormat.component3_width));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixOffset0  = %x\r\n"), pChannelParams->pixelFormat.component0_offset));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixOffset1  = %x\r\n"), pChannelParams->pixelFormat.component1_offset));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixOffset2  = %x\r\n"), pChannelParams->pixelFormat.component2_offset));
    DEBUGMSG (ZONE_DEVICE, (TEXT("PixOffset3  = %x\r\n"), pChannelParams->pixelFormat.component3_offset));
    return;
}

//------------------------------------------------------------------------------
//
// Function: dumpCoeffs
//
//
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
static void dumpCoeffs(pPpCSCCoeffs pCSCCoeffs)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pCSCCoeffs);

    DEBUGMSG (ZONE_DEVICE, (TEXT("C00: %x\r\n"), pCSCCoeffs->C00));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C01: %x\r\n"), pCSCCoeffs->C01));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C02: %x\r\n"), pCSCCoeffs->C02));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C10: %x\r\n"), pCSCCoeffs->C10));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C11: %x\r\n"), pCSCCoeffs->C11));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C12: %x\r\n"), pCSCCoeffs->C12));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C20: %x\r\n"), pCSCCoeffs->C20));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C21: %x\r\n"), pCSCCoeffs->C21));
    DEBUGMSG (ZONE_DEVICE, (TEXT("C22: %x\r\n"), pCSCCoeffs->C22));
    DEBUGMSG (ZONE_DEVICE, (TEXT("A0: %x\r\n"), pCSCCoeffs->A0));
    DEBUGMSG (ZONE_DEVICE, (TEXT("A1: %x\r\n"), pCSCCoeffs->A1));
    DEBUGMSG (ZONE_DEVICE, (TEXT("A2: %x\r\n"), pCSCCoeffs->A2));
    return;
}

#if 0 // Remove-W4: Warning C4505 workaround
//------------------------------------------------------------------------------
//
// Function: dumpInterruptRegisters
//
//
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
static void dumpInterruptRegisters(PCSP_IPU_REGS pIPU)
{
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_1: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_1)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_2: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_2)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_3: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_3)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_4: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_4)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_5: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_5)));
    return;
}
#endif

//------------------------------------------------------------------------------
//
// Function: dumpIpuRegisters
//
//
//
// Parameters:
//      None
//
// Returns:
//      None
//
//---------------------------------------------------------------------
static void dumpIpuRegisters(PCSP_IPU_REGS pIPU)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pIPU);

    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_BUF0_RDY: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CHA_BUF0_RDY)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_BUF1_RDY: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CHA_BUF1_RDY)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_DB_MODE_SEL: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CHA_DB_MODE_SEL)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_CHA_CUR_BUF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_CHA_CUR_BUF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_FS_PROC_FLOW: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_FS_PROC_FLOW)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_FS_DISP_FLOW: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_FS_DISP_FLOW)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_TASKS_STAT: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_TASKS_STAT)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_CTRL_1: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_CTRL_1)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_CTRL_3: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_CTRL_3)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_1: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_1)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_2: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_2)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_3: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_3)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_5: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_5)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: SDC_COM_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->SDC_COM_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: SDC_FG_POS: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->SDC_FG_POS)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IC_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IC_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IC_CMBP_1: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IC_CMBP_1)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IC_CMBP_2: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IC_CMBP_2)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: CSI_SENS_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->CSI_SENS_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: CSI_SENS_FRM_SIZE: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->CSI_SENS_FRM_SIZE)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IDMAC_CHA_EN: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IDMAC_CHA_EN)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IDMAC_CHA_BUSY: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IDMAC_CHA_BUSY)));
}


#if 0 // Remove-W4: Warning C4505 workaround
static void ReadVfDMA(PCSP_IPU_REGS pIPU, UINT32 ch)
{
    int i;
    UINT32 data;

    // Software-controlled access to IMA registers
    // IMA registers may only be accessed if IMA_ADDR is
    // set to 0.

    for (;;)
    {
        if (INREG32(&pIPU->IPU_IMA_ADDR) == 0)
        {
            // Try to set IPU_IMA registers.
            if ((UINT32) InterlockedTestExchange((LPLONG)&pIPU->IPU_IMA_ADDR, 0, 1) == 0)
            {
                // Successfully set IMA_ADDR.
                break;
            }
        }
        // IPU_IMA controlled by another process.
        // Surrender CPU and then try again.
        Sleep(1);
    }

    OUTREG32(&pIPU->IPU_IMA_ADDR,
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_MEM_NU, IPU_IMA_ADDR_MEM_NU_CPM) |
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_ROW_NU, ch * 2)|
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_WORD_NU, 0));

    for (i = 0; i < 132; i += 32)
    {
        data = INREG32(&pIPU->IPU_IMA_DATA);
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s(): Word0, bits %d - %d: %x\r\n"), __WFUNCTION__, i, i+32, data));
    }

    OUTREG32(&pIPU->IPU_IMA_ADDR,
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_MEM_NU, IPU_IMA_ADDR_MEM_NU_CPM) |
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_ROW_NU, ch * 2 + 1)|
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_WORD_NU, 0));

    for (i = 0; i < 132; i += 32)
    {
        data = INREG32(&pIPU->IPU_IMA_DATA);
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s(): Word1, bits %d - %d: %x\r\n"), __WFUNCTION__, i, i+32, data));
    }

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&pIPU->IPU_IMA_ADDR, 0);
}
#endif

#if 0 // Remove-W4: Warning C4505 workaround
//------------------------------------------------------------------------------
//
// Function: generateBMP
//
// This function creates a BMP file from the PP output.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
static void generateBMP(UINT32 phyImageBuf, LONG width, LONG height)
{
    HANDLE hPPOutputFile;
    static wchar_t *fileName = TEXT("\\release\\PPOutput_driver.bmp");
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    DWORD bytesWritten;
    UINT32 iImageBytes;

    PHYSICAL_ADDRESS physAddr;
    physAddr.LowPart = phyImageBuf;

    UINT32 *virtAddr = (UINT32 *) MmMapIoSpace(physAddr, width * height * 3/* + 20*/, FALSE);

    // Create structures for a bitmap of RGB888

    iImageBytes = height * width * 3;// + 20;

    // Fill bitmap structures
    bmfh.bfType = 19778; // standard value
    bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + iImageBytes; // size of the file in bytes
    bmfh.bfReserved1 = 0; // reserved
    bmfh.bfReserved2 = 0; // reserved
    bmfh.bfOffBits = 54; // offset from beginning of file to bitmap data

    bmih.biSize = 40; // size of BITMAPINFOHEADER structure in bytes
    bmih.biWidth = width; // bitmap width
    bmih.biHeight = height; // bitmap height
    bmih.biPlanes = 1; // bitmap planes
    bmih.biBitCount = 24; // bits per pixel
    bmih.biCompression = 0; // type of compression
    bmih.biSizeImage = iImageBytes; //0; // size of image data (0 if no compression)
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant= 0;

    DEBUGMSG(ZONE_ERROR,
             (TEXT("%s: Output file name: %s\r\n"), __WFUNCTION__, fileName));

    //-----------------------------
    // Open file to write BMP to
    //-----------------------------
    
    hPPOutputFile = CreateFile(fileName,          // "special" file name
        GENERIC_WRITE | GENERIC_READ,  // desired access
        0,                      // sharing mode
        NULL,                   // security attributes (=NULL)
        CREATE_ALWAYS,            // creation disposition
        FILE_ATTRIBUTE_NORMAL,  // flags and attributes
        NULL);                  // template file (ignored)
    if (hPPOutputFile == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s: Could not open file to write YUV data.  Aborting.\r\n"), __WFUNCTION__));
        return;
    }

    // Write bitmapfileheader
    if (!WriteFile(hPPOutputFile, (LPVOID) &bmfh, sizeof(BITMAPFILEHEADER), (LPDWORD) &bytesWritten, NULL))
    {
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s: Could not write bit map file header to BMP.\r\n"), __WFUNCTION__));
    }

    // Write bitmapinfoheader
    if (!WriteFile(hPPOutputFile, (LPVOID) &bmih, sizeof(BITMAPINFOHEADER), (LPDWORD) &bytesWritten, NULL))
    {
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s: Could not write bit map info header to BMP.\r\n"), __WFUNCTION__));
    }

    // Write bitmap info
    if (!WriteFile(hPPOutputFile, (LPVOID) virtAddr, iImageBytes, (LPDWORD) &bytesWritten, NULL))
    {
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s: Could not write image data to BMP.\r\n"), __WFUNCTION__));
    }

    CloseHandle(fileName);
}
#endif

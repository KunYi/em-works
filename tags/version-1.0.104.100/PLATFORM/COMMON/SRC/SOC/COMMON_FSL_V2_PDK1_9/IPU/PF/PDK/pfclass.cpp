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
//  Copyright (C) 2005-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  PfClass.cpp
//
//  Implementation of postfilter driver methods
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#define WINCEMACRO
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipu.h"

#include "Ipu.h"
#include "pf.h"
#include "PfClass.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines

#define THREAD_PRIORITY                            250

#define PF_MAX_NUM_BUFFERS                         100
#define PF_NUM_ROT_BUFFERS                         2

#define PF_QP_DMA_CHANNEL                          IPU_DMA_CHA_DMAPF_0_LSH
#define PF_BS_DMA_CHANNEL                          IPU_DMA_CHA_DMAPF_1_LSH
#define PF_Y_INPUT_DMA_CHANNEL                     IPU_DMA_CHA_DMAPF_2_LSH
#define PF_U_INPUT_DMA_CHANNEL                     IPU_DMA_CHA_DMAPF_3_LSH
#define PF_V_INPUT_DMA_CHANNEL                     IPU_DMA_CHA_DMAPF_4_LSH
#define PF_Y_OUTPUT_DMA_CHANNEL                    IPU_DMA_CHA_DMAPF_5_LSH
#define PF_U_OUTPUT_DMA_CHANNEL                    IPU_DMA_CHA_DMAPF_6_LSH
#define PF_V_OUTPUT_DMA_CHANNEL                    IPU_DMA_CHA_DMAPF_7_LSH


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


//------------------------------------------------------------------------------
// Local Functions
static void dumpChannelParams(pPfIDMACChannelParams);
static void dumpIpuRegisters(PCSP_IPU_REGS);
static void generateBMP(UINT32 phyImageBuf, LONG width, LONG height);
#if 0 // Remove-W4: Warning C4505 workaround
static void dumpInterruptRegisters(PCSP_IPU_REGS);
static void ReadVfDMA(PCSP_IPU_REGS);
#endif

//-----------------------------------------------------------------------------
//
// Function: PfClass
//
// Postprocessor class constructor.  Calls PfInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PfClass::PfClass(void)
{
    PfInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~PfClass
//
// The destructor for PfClass.  Calls PfDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PfClass::~PfClass(void)
{
    PfDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: PfInit
//
// This function initializes the Post-filter driver.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL PfClass::PfInit(void)
{
    PF_FUNCTION_ENTRY();

    // open handle to the IPU_BASE driver in order to enable PF module
    hIPUBase = CreateFile(TEXT("IPU1:"),    // "special" file name
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (=NULL)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags and attributes
        NULL);                              // template file (ignored)
    if (hIPUBase == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    
    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for IPU Interrupt\r\n"), __WFUNCTION__));
    m_hPfIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_PF_INTR_EVENT);
    if (m_hPfIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for IPU Interrupt failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for PF EOF event\r\n"), __WFUNCTION__));
    // Events to signal pin that frame is ready
    m_hEOFEvent = CreateEvent(NULL, FALSE, FALSE, PF_EOF_EVENT_NAME);
    if (m_hEOFEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for PF EOF\r\n"), __WFUNCTION__));
        goto Error;
    }
    
    m_hYEvent = CreateEvent(NULL, FALSE, FALSE, PF_Y_EVENT_NAME);
    m_hCrEvent = CreateEvent(NULL, FALSE, FALSE, PF_CR_EVENT_NAME);
    if ((m_hYEvent == NULL)||(m_hCrEvent == NULL))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent(Y,Cr) failed for PF EOF\r\n"), __WFUNCTION__));
        goto Error;
    }   
    
    DEBUGMSG(ZONE_INIT, (TEXT("%s: Initializing class variables\r\n"), __WFUNCTION__));

    m_bConfigured = FALSE;
    m_pfMode = pfMode_Disabled;
    m_bRunning = FALSE;
    m_bPauseEnabled = FALSE;

    DEBUGMSG(ZONE_INIT, (TEXT("%s: Spawning threads\r\n"), __WFUNCTION__));

    // Initialize thread for Post-filter ISR
    //      pThreadAttributes = NULL (must be NULL)
    //      dwStackSize = 0 => default stack size determined by linker
    //      lpStartAddress = CspiProcessQueue => thread entry point
    //      lpParameter = NULL => point to thread parameter
    //      dwCreationFlags = 0 => no flags
    //      lpThreadId = NULL => thread ID is not returned
    PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
    m_hPfISRThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PfIntrThread, this, 0, NULL);

    if (m_hPfISRThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create camera ISR thread success\r\n"), __WFUNCTION__));
        CeSetThreadPriority(m_hPfISRThread, 100);//THREAD_PRIORITY_TIME_CRITICAL);
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: Enabling postfilter\r\n"), __WFUNCTION__));

    PF_FUNCTION_EXIT();

    return TRUE;

Error:
    PfDeinit();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PfDeinit
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
void PfClass::PfDeinit(void)
{
    PF_FUNCTION_ENTRY();

    // Disable Postprocessor
    PfDisable();

    CloseHandle(m_hPfIntrEvent);
    m_hPfIntrEvent = NULL;

    CloseHandle(m_hEOFEvent);
    m_hEOFEvent = NULL;

    CloseHandle(m_hYEvent);
    m_hYEvent = NULL;
    
    CloseHandle(m_hCrEvent);
    m_hCrEvent = NULL;
    
    if (m_hPfISRThread)
    {
        CloseHandle(m_hPfISRThread);
        m_hPfISRThread = NULL;
    }

    PF_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: PfConfigure
//
// This function configures the IC registers and IDMAC
// channels for the post-filtering channel.
//
// Parameters:
//      pPfConfigureData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PfClass::PfConfigure(pPfConfigData pConfigData)
{
    BOOL result = TRUE;
    UINT16 iWidth, iHeight;
    UINT32 iStride;
    UINT32 iBSBufOffset;
    pfIDMACChannelParams channelParams;

    PF_FUNCTION_ENTRY();

    // Set these variables to reduce pointer computations,
    // as these will be referenced several times.
    iWidth = pConfigData->frameSize.width;
    iHeight = pConfigData->frameSize.height;
    iStride = pConfigData->frameStride;

    //-----------------------------------------------------------------
    // Image size validity check
    // Setup input image size
    //-----------------------------------------------------------------

    // Boundary check
    if((iWidth  > PF_MAX_WIDTH) ||
        (iHeight > PF_MAX_HEIGHT) ||
        (iWidth  < PF_MIN_WIDTH) ||
        (iHeight < PF_MIN_HEIGHT))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error frame size: \r\n"), __WFUNCTION__));
        DEBUGMSG(ZONE_ERROR,
                (TEXT("\t Frame Size: width (%d), height (%d)\r\n"),
                iWidth, iHeight));
        result = FALSE;
        goto _done;
    }

    // Alignment check
    if((iWidth & 0x07) ||
        (iHeight & 0x01))
    {
        DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Error frame size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                iWidth, iHeight));
        result = FALSE;
        goto _done;
    }

    //-----------------------------------------------------------------
    // Setup channel parameters for Y, U, and V input and output channels
    //-----------------------------------------------------------------

    // The format, bits per pixel, and pixel burst rate are all the
    // same for the Y, U, and V DMA channels for input and output
    channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_GENERIC;
    channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats
    channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_32; // Burst rate of 32 pixels

    // Set Y channel parameters for height, width, and stride
    channelParams.iHeight = iHeight;
    channelParams.iWidth = iWidth;
    channelParams.iLineStride = iStride;

    // Configure Channel 26 (Y Input for Postfiltering)
    PfIDMACChannelConfig(PF_Y_INPUT_DMA_CHANNEL, &channelParams);

    // Configure Channel 29 (Y Output for Postfiltering)
    PfIDMACChannelConfig(PF_Y_OUTPUT_DMA_CHANNEL, &channelParams);

    // Set U and V channel parameters for height, width, and stride
    channelParams.iHeight = iHeight / 2;
    channelParams.iWidth = iWidth / 2;
    channelParams.iLineStride = iStride / 2;

    // Configure Channel 27 (U Input for Postfiltering)
    PfIDMACChannelConfig(PF_U_INPUT_DMA_CHANNEL, &channelParams);

    // Configure Channel 30 (U Output for Postfiltering)
    PfIDMACChannelConfig(PF_U_OUTPUT_DMA_CHANNEL, &channelParams);

    // Configure Channel 28 (V Input for Postfiltering)
    PfIDMACChannelConfig(PF_V_INPUT_DMA_CHANNEL, &channelParams);

    // Configure Channel 31 (V Input for Postfiltering)
    PfIDMACChannelConfig(PF_V_OUTPUT_DMA_CHANNEL, &channelParams);


    //-----------------------------------------------------------------
    // Setup channel parameters for QP and BS channels
    //-----------------------------------------------------------------

    m_pfMode = pConfigData->mode;

    // Set PF_CONF register with Postfilter mode
    INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_PF_TYPE, m_pfMode);

    // Set these variables to reduce pointer computations,
    // as these will be referenced several times.
    iWidth = pConfigData->frameSize.width;
    iHeight = pConfigData->frameSize.height;

    if (m_pfMode == pfMode_H264Deblock)
    {
        // Configure parameters for quantization parameter channel
        channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_GENERIC;
        channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_32; // required for planar formats
        channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_4; // Burst rate of 4 pixels

        channelParams.iWidth = (iWidth + 15) / 16;
        channelParams.iHeight = (iHeight + 15) / 16;
        channelParams.iLineStride = channelParams.iWidth * 4;

        // Configure Channel 24 (QP for Postfiltering)
        PfIDMACChannelConfig(PF_QP_DMA_CHANNEL, &channelParams);

        // Program Quantization Parameter buffer into channel parameter memory
        controlledWriteDMAChannelParam(PF_QP_DMA_CHANNEL, 1, 0, (unsigned int) pConfigData->qpBuf); //Buffer 0    31-0

        // Configure parameters for boundary strength channel
        channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_GENERIC;
        channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats
        channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_16; // Burst rate of 16 pixels

        iBSBufOffset = channelParams.iWidth * channelParams.iHeight * 4;
        channelParams.iWidth *= 4;
        channelParams.iHeight *= 4;
        channelParams.iLineStride = channelParams.iWidth;
        
        // Configure Channel 25 (BS for Postfiltering)
        PfIDMACChannelConfig(PF_BS_DMA_CHANNEL, &channelParams);

        // Program Boundary Strength Parameter buffer into channel parameter memory
        controlledWriteDMAChannelParam(PF_BS_DMA_CHANNEL, 1, 0, (UINT32)pConfigData->qpBuf + iBSBufOffset); //Buffer 0    31-0
    }
    else if (m_pfMode != pfMode_Disabled) // MPEG4 mode
    {
        // Configure parameters for quantization parameter channel
        channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_GENERIC;
        channelParams.iBitsPerPixelCode = IDMAC_BPP_CODE_8; // required for planar formats
        channelParams.iPixelBurstCode = IDMAC_PIXEL_BURST_CODE_4; // Burst rate of 4 pixels

        channelParams.iWidth = (iWidth + 15) / 16;
        channelParams.iHeight = (iHeight + 15) / 16;
        channelParams.iLineStride = (channelParams.iWidth + 3) & ~0x3UL;

        // Configure Channel 24 (QP for Postfiltering)
        PfIDMACChannelConfig(PF_QP_DMA_CHANNEL, &channelParams);
        
        // Program Quantization Parameter buffer into channel parameter memory
        controlledWriteDMAChannelParam(PF_QP_DMA_CHANNEL, 1, 0, (unsigned int) pConfigData->qpBuf); //Buffer 0    31-0
    }

    m_bConfigured = TRUE;

_done:
    PF_FUNCTION_EXIT();
    return result;
}

//-----------------------------------------------------------------------------
//
// Function: PfStartChannel
//
// This function starts the post-filtering channel.
//
// Parameters:
// Parameters:
//      pStartParms
//          [in] Pointer to start data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PfClass::PfStartChannel(pPfStartParams pStartParms)
{
    UINT32 oldVal, newVal, iMask, iBitval;
    UINT32 iInputYBufPtr, iInputUBufPtr, iInputVBufPtr;
    UINT32 iOutputYBufPtr, iOutputUBufPtr, iOutputVBufPtr;

    PF_FUNCTION_ENTRY();

    // PF must have been configured at least once 
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

    m_bRunning = TRUE;

    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause Pin to believe a frame was ready.
    ResetEvent(m_hEOFEvent);
    ResetEvent(m_hYEvent);
    ResetEvent(m_hCrEvent);
    
    // Enable PF submodule
    PfEnable();
    
    if (m_pfMode == pfMode_H264Deblock) // H.264 requires output buffer equal to input
    {
        pStartParms->out = pStartParms->in;

        if (pStartParms->h264_pause_row != 0)
        {
            // Enable and set up pause row
            INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_EN, 
                IPU_PF_CONF_H264_Y_PAUSE_EN_ENABLE);
            INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_ROW, 
                pStartParms->h264_pause_row);

            m_bPauseEnabled = TRUE;
        }
        else
        {
            // Disable pause row
            INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_EN, 
                IPU_PF_CONF_H264_Y_PAUSE_EN_DISABLE);

            m_bPauseEnabled = FALSE;
        }
    }
    else
    {
        // Disable pause row
        INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_EN, 
            IPU_PF_CONF_H264_Y_PAUSE_EN_DISABLE);

        m_bPauseEnabled = FALSE;
    }


    // Protect access to IPU_INT_CTRL_1 and IDMAC_CHA_EN registers.

    // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
    iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAPF_5)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_6)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_7);
    iBitval = (UINT32) (CSP_BITFVAL(IPU_DMA_CHA_DMAPF_5, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_6, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_7, IPU_ENABLE));

    // enable IPU interrupts for channels 29, 30, 31 (PF Y, U, and V Output)
    do
    {
        oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                oldVal, newVal) != oldVal);

    // Compute bitmask and shifted bit value for IDMAC enable register.
    iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAPF_0)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_2)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_3)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_4)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_5)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_6)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_7);
    iBitval = (UINT32) (CSP_BITFVAL(IPU_DMA_CHA_DMAPF_0, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_2, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_3, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_4, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_5, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_6, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_7, IPU_ENABLE));
    if (m_pfMode == pfMode_H264Deblock)
    {
        iMask |= CSP_BITFMASK(IPU_DMA_CHA_DMAPF_1);
        iBitval |= CSP_BITFVAL(IPU_DMA_CHA_DMAPF_1, IPU_ENABLE);
    }

    // Set the bits to enable the IDMAC channels.
    do
    {
        oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                oldVal, newVal) != oldVal);

    iInputYBufPtr = (UINT32)pStartParms->in->yBufPtr;
    iInputUBufPtr = iInputYBufPtr + pStartParms->in->uOffset;
    iInputVBufPtr = iInputYBufPtr + pStartParms->in->vOffset;

    // for bit-match issue
    // Program Quantization Parameter buffer into channel parameter memory
    controlledWriteDMAChannelParam(PF_QP_DMA_CHANNEL, 1, 0, (unsigned int)pStartParms->qp_buf); //Buffer 0    31-0
    // Program Boundary Strength Parameter buffer into channel parameter memory
    controlledWriteDMAChannelParam(PF_BS_DMA_CHANNEL, 1, 0, (unsigned int)pStartParms->bs_buf); //Buffer 0    31-0

    // Program input Y, U, and V buffers into channel parameter memory
    controlledWriteDMAChannelParam(PF_Y_INPUT_DMA_CHANNEL, 1, 0, iInputYBufPtr); //Buffer 0    31-0
    controlledWriteDMAChannelParam(PF_U_INPUT_DMA_CHANNEL, 1, 0, (unsigned int)iInputUBufPtr); //Buffer 0    31-0
    controlledWriteDMAChannelParam(PF_V_INPUT_DMA_CHANNEL, 1, 0, (unsigned int)iInputVBufPtr); //Buffer 0    31-0

    iOutputYBufPtr = (UINT32)pStartParms->out->yBufPtr;
    iOutputUBufPtr = iOutputYBufPtr + pStartParms->out->uOffset;
    iOutputVBufPtr = iOutputYBufPtr + pStartParms->out->vOffset;
    
    // Program output Y, U, and V buffers into channel parameter memory
    controlledWriteDMAChannelParam(PF_Y_OUTPUT_DMA_CHANNEL, 1, 0, (unsigned int)iOutputYBufPtr); //Buffer 0    31-0
    controlledWriteDMAChannelParam(PF_U_OUTPUT_DMA_CHANNEL, 1, 0, (unsigned int)iOutputUBufPtr); //Buffer 0    31-0
    controlledWriteDMAChannelParam(PF_V_OUTPUT_DMA_CHANNEL, 1, 0, (unsigned int)iOutputVBufPtr); //Buffer 0    31-0

    dumpIpuRegisters(P_IPU_REGS);

    // Buffers ready for all PF DMA Channels
    SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_0, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_2, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_3, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_4, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_5, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_6, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_7, IPU_DMA_CHA_READY));
    if (m_pfMode == pfMode_H264Deblock)
    {
        SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
             CSP_BITFVAL(IPU_DMA_CHA_DMAPF_1, IPU_DMA_CHA_READY));
    }

    PF_FUNCTION_EXIT();

    return TRUE;
}

// for Direct Render
//-----------------------------------------------------------------------------
//
// Function: PfStartChannel2
//
// This function starts the post-filtering channel.
//
// Parameters:
// Parameters:
//      pStartParms
//          [in] Pointer to start data structure
//
//      VirtualFlag
//          [in] Flag to indicate if virtual memory pointer
//
//      yOffset
//          [in] offset to the pointer, in byte
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PfClass::PfStartChannel2(pPfStartParams pStartParms,unsigned int VirtualFlag,unsigned int yOffset)
{
    UINT32 oldVal, newVal, iMask, iBitval;
    UINT32 iInputYBufPtr, iInputUBufPtr, iInputVBufPtr;
    UINT32 iOutputYBufPtr, iOutputUBufPtr, iOutputVBufPtr;
    PVOID pDestMarshalled;      // For WinCE 6.00 pointer marshalling.    

    PF_FUNCTION_ENTRY();

    // PF must have been configured at least once 
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

    m_bRunning = TRUE;

    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause Pin to believe a frame was ready.
    ResetEvent(m_hEOFEvent);
    ResetEvent(m_hYEvent);
    ResetEvent(m_hCrEvent);
    
    // Enable PF submodule
    PfEnable();
    
    if (m_pfMode == pfMode_H264Deblock) // H.264 requires output buffer equal to input
    {
        pStartParms->out = pStartParms->in;

        if (pStartParms->h264_pause_row != 0)
        {
            // Enable and set up pause row
            INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_EN, 
                IPU_PF_CONF_H264_Y_PAUSE_EN_ENABLE);
            INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_ROW, 
                pStartParms->h264_pause_row);

            m_bPauseEnabled = TRUE;
        }
        else
        {
            // Disable pause row
            INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_EN, 
                IPU_PF_CONF_H264_Y_PAUSE_EN_DISABLE);

            m_bPauseEnabled = FALSE;
        }
    }
    else
    {
        // Disable pause row
        INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_EN, 
            IPU_PF_CONF_H264_Y_PAUSE_EN_DISABLE);

        m_bPauseEnabled = FALSE;
    }


    // Protect access to IPU_INT_CTRL_1 and IDMAC_CHA_EN registers.

    // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
    iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAPF_5)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_6)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_7);
    iBitval = (UINT32) (CSP_BITFVAL(IPU_DMA_CHA_DMAPF_5, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_6, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_7, IPU_ENABLE));

    // enable IPU interrupts for channels 29, 30, 31 (PF Y, U, and V Output)
    do
    {
        oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                oldVal, newVal) != oldVal);

    // Compute bitmask and shifted bit value for IDMAC enable register.
    iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAPF_0)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_2)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_3)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_4)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_5)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_6)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_7);
    iBitval = (UINT32) (CSP_BITFVAL(IPU_DMA_CHA_DMAPF_0, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_2, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_3, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_4, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_5, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_6, IPU_ENABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_7, IPU_ENABLE));
    if (m_pfMode == pfMode_H264Deblock)
    {
        iMask |= CSP_BITFMASK(IPU_DMA_CHA_DMAPF_1);
        iBitval |= CSP_BITFVAL(IPU_DMA_CHA_DMAPF_1, IPU_ENABLE);
    }

    // Set the bits to enable the IDMAC channels.
    do
    {
        oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                oldVal, newVal) != oldVal);

    if(VirtualFlag)
    {   
        DWORD addr;     
        LPVOID mapaddr;
        BOOL ret;

        if (FAILED(CeOpenCallerBuffer(&pDestMarshalled,
                                          ((char*)pStartParms->in->yBufPtr)-yOffset,
                                          4,
                                          ARG_I_PTR,
                                          FALSE)))
        {
                ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() for pStartParms->in->yBufPtr-yOffset ")
                                TEXT("failed\r\n")));
                return FALSE;
        }
        mapaddr = pDestMarshalled;
        ret=LockPages(mapaddr, 4, &addr, LOCKFLAG_QUERY_ONLY);
        iInputYBufPtr=(UINT32)addr+yOffset;
        iInputYBufPtr=iInputYBufPtr<<(UserKInfo[KINX_PFN_SHIFT]);
        // For WinCE 6.00 we must release the marshalled buffers here.
        if (FAILED(CeCloseCallerBuffer(pDestMarshalled,
                                          ((char*)pStartParms->in->yBufPtr)-yOffset,
                                           4,
                                           ARG_O_PTR)))
        {
                ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for pStartParms->in->yBufPtr-yOffset ")
                                TEXT("failed\r\n")));
                return FALSE;
        }        
    }
    else
    {
        iInputYBufPtr = (UINT32)pStartParms->in->yBufPtr;
    }
    iInputUBufPtr = iInputYBufPtr + pStartParms->in->uOffset;
    iInputVBufPtr = iInputYBufPtr + pStartParms->in->vOffset;

    // for bit-match issue
    // Program Quantization Parameter buffer into channel parameter memory
    controlledWriteDMAChannelParam(PF_QP_DMA_CHANNEL, 1, 0, (unsigned int)pStartParms->qp_buf); //Buffer 0    31-0
    // Program Boundary Strength Parameter buffer into channel parameter memory
    controlledWriteDMAChannelParam(PF_BS_DMA_CHANNEL, 1, 0, (unsigned int)pStartParms->bs_buf); //Buffer 0    31-0

    // Program input Y, U, and V buffers into channel parameter memory
    controlledWriteDMAChannelParam(PF_Y_INPUT_DMA_CHANNEL, 1, 0, iInputYBufPtr); //Buffer 0    31-0
    controlledWriteDMAChannelParam(PF_U_INPUT_DMA_CHANNEL, 1, 0, (unsigned int)iInputUBufPtr); //Buffer 0    31-0
    controlledWriteDMAChannelParam(PF_V_INPUT_DMA_CHANNEL, 1, 0, (unsigned int)iInputVBufPtr); //Buffer 0    31-0

    if(VirtualFlag)
    {
        DWORD addr;
        LPVOID mapaddr;
        BOOL ret;

        if (FAILED(CeOpenCallerBuffer(&pDestMarshalled,
                                          ((char*)pStartParms->out->yBufPtr)-yOffset,
                                          4,
                                          ARG_I_PTR,
                                          FALSE)))
        {
                ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() for pStartParms->out->yBufPtr-yOffset ")
                                TEXT("failed\r\n")));
                return FALSE;
        }
        mapaddr = pDestMarshalled;
        ret=LockPages(mapaddr, 4, &addr, LOCKFLAG_QUERY_ONLY);      
        iOutputYBufPtr=(UINT32)addr+yOffset;
        iOutputYBufPtr=iOutputYBufPtr<<(UserKInfo[KINX_PFN_SHIFT]);
        // For WinCE 6.00 we must release the marshalled buffers here.
        if (FAILED(CeCloseCallerBuffer(pDestMarshalled,
                                           ((char*)pStartParms->out->yBufPtr)-yOffset,
                                           4,
                                           ARG_O_PTR)))
        {
                ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for pStartParms->out->yBufPtr-yOffset ")
                                TEXT("failed\r\n")));
                return FALSE;
        }        
    }
    else
    {
        iOutputYBufPtr = (UINT32)pStartParms->out->yBufPtr;
    }
    iOutputUBufPtr = iOutputYBufPtr + pStartParms->out->uOffset;
    iOutputVBufPtr = iOutputYBufPtr + pStartParms->out->vOffset;
    
    // Program output Y, U, and V buffers into channel parameter memory
    controlledWriteDMAChannelParam(PF_Y_OUTPUT_DMA_CHANNEL, 1, 0, (unsigned int)iOutputYBufPtr); //Buffer 0    31-0
    controlledWriteDMAChannelParam(PF_U_OUTPUT_DMA_CHANNEL, 1, 0, (unsigned int)iOutputUBufPtr); //Buffer 0    31-0
    controlledWriteDMAChannelParam(PF_V_OUTPUT_DMA_CHANNEL, 1, 0, (unsigned int)iOutputVBufPtr); //Buffer 0    31-0

    dumpIpuRegisters(P_IPU_REGS);

    // Buffers ready for all PF DMA Channels
    SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_0, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_2, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_3, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_4, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_5, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_6, IPU_DMA_CHA_READY) |
         CSP_BITFVAL(IPU_DMA_CHA_DMAPF_7, IPU_DMA_CHA_READY));
    if (m_pfMode == pfMode_H264Deblock)
    {
        SETREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY, 
             CSP_BITFVAL(IPU_DMA_CHA_DMAPF_1, IPU_DMA_CHA_READY));
    }

    PF_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PfSetAttributeEx
//
// This function call VirtualSetAttributesEx() to set the virtual memory attributes.
//
// Parameters:
// Parameters:
//      pData
//          [in] Pointer to VirtualSetAttributesEx data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PfClass::PfSetAttributeEx(pPfSetAttributeExData pData)
{
    DWORD oldFlag;

    // change to write-through caching, normal space(strongly ordered by default)
    if(VirtualSetAttributesEx((void*)GetCallerVMProcessId(), pData->lpvAddress, pData->cbSize, 0x8, 0xC, &oldFlag))
    {
//        RETAILMSG(1, (TEXT("##############PfSetAttributeEx: ADD=0x%x, SIZE=0x%x, OLDATTR=0x%x\r\n"), pData->lpvAddress,pData->cbSize, oldFlag )); 
        return TRUE;
    }else
    {
        ERRORMSG(1, (TEXT("PfSetAttributeEx() failed! \r\n")));
//        RETAILMSG(1, (TEXT("##############PfSetAttributeEx: ADD=0x%x, SIZE=0x%x, OLDATTR=0x%x\r\n"), pData->lpvAddress,pData->cbSize, oldFlag )); 
        return FALSE;
    }

}

//-----------------------------------------------------------------------------
//
// Function: PfResume
//
// This function resumes the Postfilter operation after pausingstarts the post-filtering channel.
//
// Parameters:
//      h264_pause_row
//      [in] Specifies a new pause row for the h264 postfilter operation.  This pause row
//      must be greater than the original pause row in order to pause the operation.  Set
//      this parameter to 0 to disable pausing.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
DWORD PfClass::PfResume(UINT32 h264_pause_row) // Row to pause at for H.264 mode. 0 to disable pause)
{
    // Only resume if we are running and were paused 
    if (!m_bRunning)
    {
        // return error since Postfilter is not running
        return PF_ERR_NOT_RUNNING;
    }
    else if (!m_bPauseEnabled)
    {
        // return error since pause is not enabled
        return PF_ERR_PAUSE_NOT_ENABLED;
    }
    else
    {
        if (h264_pause_row != 0)
        {
            if ((h264_pause_row > PF_MAX_PAUSE_ROW) || (h264_pause_row < 0))
            {
                // return error since pause row value is not in a valid range
                return PF_ERR_INVALID_PARAMETER;
            }

            // Enable and set up pause row
            INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_ROW, 
                h264_pause_row);

            m_bPauseEnabled = TRUE;
        }
        else
        {
            // Disable pause row
            INSREG32BF(&P_IPU_REGS->PF_CONF, IPU_PF_CONF_H264_Y_PAUSE_EN, 
                IPU_PF_CONF_H264_Y_PAUSE_EN_DISABLE);
            
            m_bPauseEnabled = FALSE;
        }
    }

    return PF_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PfStopChannel
//
// This function halts the post-filtering channels.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PfClass::PfStopChannel(void)
{
    UINT32 uTempReg, uTempReg1, uCount = 0;
    UINT32 oldVal, newVal, iMask, iBitval;

    PF_FUNCTION_ENTRY();

    // If not running, return
    if (m_bRunning == FALSE)
    {
        return TRUE;
    }

    m_bRunning = FALSE;

    // initalize ... for the first time through
    uTempReg = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);
    uTempReg1 = INREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY);

    // We can't disable tasks until the active channels
    // have completed their current frames.  Make sure
    // that buffers aren't set as ready (indicating that
    // they are yet to start) and that channels are
    // not busy (indicating that channels are still running).
    while ((uTempReg1 & 0xFE000000) || (uTempReg & 0xFE000000))
    {
        if (uCount <= 1000)
        {
            //..give up the remainder of time slice
            Sleep(1);
            uCount++;

            //.. need to check after the sleep delay
            uTempReg = INREG32(&P_IPU_REGS->IDMAC_CHA_BUSY);
            uTempReg1 = INREG32(&P_IPU_REGS->IPU_CHA_BUF0_RDY);
        }
        else
        {
            //.. there is something wrong ....break out
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s(): Error in stopping PF channel!\r\n"), __WFUNCTION__));
            break;
        }
    }

    ResetEvent(m_hEOFEvent);

    // Protect access to IDMAC_CHA_EN and IPU_INT_CTRL_1 registers.

    // Compute bitmask and shifted bit value for IDMAC enable register.
    iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAPF_0)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_1)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_2)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_3)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_4)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_5)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_6)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_7);
    iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAPF_0, IPU_DISABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_1, IPU_DISABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_2, IPU_DISABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_3, IPU_DISABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_4, IPU_DISABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_5, IPU_DISABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_6, IPU_DISABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_7, IPU_DISABLE);

    // Use interlocked function to disable the IDMAC channels.
    do
    {
        oldVal = INREG32(&P_IPU_REGS->IDMAC_CHA_EN);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IDMAC_CHA_EN, 
                oldVal, newVal) != oldVal);

    // Compute bitmask and shifted bit value for INT CTRL register.
    iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAPF_5)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_6)
        | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_7);
    iBitval = CSP_BITFVAL(IPU_DMA_CHA_DMAPF_5, IPU_DISABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_6, IPU_DISABLE)
        | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_7, IPU_DISABLE);

    // Use interlocked function to disable IPU interrupts 
    // for viewfinding channel.
    do
    {
        oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                oldVal, newVal) != oldVal);

    // Disable PF submodule
    PfDisable();
    
    dumpIpuRegisters(P_IPU_REGS);

    PF_FUNCTION_EXIT();

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
void PfClass::controlledWriteDMAChannelParam(int channel, int row, int word, unsigned int data)
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
void PfClass::writeDMAChannelParam(int channel, int row, int word, unsigned int data)
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
// Function: PfIDMACChannelConfig
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
void PfClass::PfIDMACChannelConfig(UINT8 ch, pPfIDMACChannelParams pChannelParams)
{
    UINT32 newVal;

    PF_FUNCTION_ENTRY();

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

    writeDMAChannelParam(ch, 0, 0, 0); //Variable data set to 0    31-0
    writeDMAChannelParam(ch, 0, 1, 1<<14); //Variable data set to 0. NSB set  63-32
    writeDMAChannelParam(ch, 0, 2, 0); //Variable data set to 0.   95-64
    writeDMAChannelParam(ch, 0, 3, ((pChannelParams->iWidth-1) << 12) |
            ((pChannelParams->iHeight-1) << 24)); // 127-96
    writeDMAChannelParam(ch, 0, 4, ((pChannelParams->iHeight-1) >> 8)); // 160-128

    // Notice we do not write row 1, words 0 or 1, as these 
    // words control the DMA channel buffers.  This data
    // is written when PfStartChannel is called.
    writeDMAChannelParam(ch, 1, 2,
        ((pChannelParams->iBitsPerPixelCode) |
        ((pChannelParams->iLineStride - 1)<<3) |
        (pChannelParams->iFormatCode<<17) |
        (pChannelParams->iPixelBurstCode<<25))); // 95-64
    writeDMAChannelParam(ch, 1, 3, 2); // SAT code of 0x10 = 32 bit memory access

    // Cede control of IPU_IMA registers by writing 0 to IPU_IMA_ADDR
    OUTREG32(&P_IPU_REGS->IPU_IMA_ADDR, 0);

    PF_FUNCTION_EXIT();
    return;
}


//-----------------------------------------------------------------------------
//
// Function: PfEnable
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
void PfClass::PfEnable(void)
{
    DWORD dwBytesTransferred;

    PF_FUNCTION_ENTRY();

    if (!DeviceIoControl(hIPUBase,         // file handle to the driver
             IPU_IOCTL_ENABLE_PF,          // I/O control code
             NULL,                         // in buffer
             0,                            // in buffer size
             NULL,                         // out buffer
             0,                            // out buffer size
             &dwBytesTransferred,          // number of bytes returned
             NULL))                        // ignored (=NULL)
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to enable PF!\r\n"), __WFUNCTION__));
    }

    dumpIpuRegisters(P_IPU_REGS);

    PF_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PfDisable
//
// Disable the Post-filtering unit.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PfClass::PfDisable(void)
{
    DWORD dwBytesTransferred;

    PF_FUNCTION_ENTRY();

    if (!DeviceIoControl(hIPUBase,         // file handle to the driver
             IPU_IOCTL_DISABLE_PF,         // I/O control code
             NULL,                         // in buffer
             0,                            // in buffer size
             NULL,                         // out buffer
             0,                            // out buffer size
             &dwBytesTransferred,          // number of bytes returned
             NULL))                        // ignored (=NULL)
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to disable PF!\r\n"), __WFUNCTION__));
    }

    PF_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PfClearInterruptStatus
//
// This function is used to clear the PF interrupt status and signal to the
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
void PfClass::PfClearInterruptStatus(DWORD clearBitmask)
{
    PF_FUNCTION_ENTRY();

    // Clear Interrupt Status Bits
    OUTREG32(&P_IPU_REGS->IPU_INT_STAT_1, clearBitmask);

    PF_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PfIntrThread
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
void PfClass::PfIntrThread(LPVOID lpParameter)
{
    PfClass *pPf = (PfClass *)lpParameter;

    PF_FUNCTION_ENTRY();

    pPf->PfISRLoop(INFINITE);

    PF_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PfISRLoop
//
// This function is the interrupt handler for the Post-filterer.
// It waits for the End-Of-Frame (EOF) interrupt, and signals
// the EOF event registered by the user of the post-filterer.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//---------------------------------------------------------------------
void PfClass::PfISRLoop(UINT32 timeout)
{
    DWORD statReg1, chanMask;
    DWORD dwIntrCnt = 0;
    UINT32 oldVal, newVal, iMask, iBitval;

    PF_FUNCTION_ENTRY();

    // loop here
    for (;;)
    {
        DEBUGMSG (ZONE_DEVICE, (TEXT("%s: In the loop\r\n"), __WFUNCTION__));

        if (WaitForSingleObject(m_hPfIntrEvent, timeout) == WAIT_OBJECT_0)
        {
            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Interrupt received\r\n"), __WFUNCTION__));

            statReg1 = INREG32(&P_IPU_REGS->IPU_INT_STAT_1);
            //dumpInterruptRegisters(P_IPU_REGS);
            if (statReg1 & 0xE0000000) // check PF channel status bits only
            {
                // EOF for Post-processor
                DEBUGMSG (ZONE_DEVICE, (TEXT("%s:*** Post-filtering End of frame interrupt ***\r\n"), __WFUNCTION__));

                if (!m_bRunning)
                {
                    // Don't do anything else.  We do not want to re-enable
                    // the buffer ready bits since we are stopping.
                    dwIntrCnt = 0;
                }
                else
                {
                    // Determine which channels have completed

                    // Check Y Output Channel
                    chanMask = 0x20000000;
                    if ((statReg1 & chanMask) & chanMask)
                    {
                        // Y output channel completed
                        dwIntrCnt++;
                        PfClearInterruptStatus(chanMask);
                        SetEvent(m_hYEvent);
                        CacheSync(CACHE_SYNC_DISCARD);
                    }

                    // Check U Output Channel
                    chanMask = 0x40000000;
                    if ((statReg1 & chanMask) & chanMask)
                    {
                        // U output channel completed
                        dwIntrCnt++;
                        PfClearInterruptStatus(chanMask);
                    }

                    // Check V Output Channel
                    chanMask = 0x80000000;
                    if ((statReg1 & chanMask) & chanMask)
                    {
                        // V output channel completed
                        dwIntrCnt++;
                        PfClearInterruptStatus(chanMask);
                        SetEvent(m_hCrEvent);
                        CacheSync(CACHE_SYNC_DISCARD);
                    }

                    // PF operation is only done if we have received
                    // 3 interrupts (Y, U, and V output channels)
                    if (dwIntrCnt > 2)
                    {
                        // Clear all PF-related Status bits
                        chanMask = 0xE0000000;
                        PfClearInterruptStatus(chanMask);

                        if (!PfStopChannel())
                        {
                            DEBUGMSG (ZONE_DEVICE, (TEXT("%s: Could not properly stop PF channel!\r\n"), __WFUNCTION__));
                        }

                        // Trigger PF EOF event
                        SetEvent(m_hEOFEvent);

                        dwIntrCnt = 0;
                    }
                    else
                    {
                        // Re-enable interrupt control bits so
                        // that we can continue to receive PF interrupts.

                        // Protect access to IPU_INT_CTRL_1 register.

                        // Compute bitmask and shifted bit value for IPU_INT_CTRL_1 register
                        iMask = CSP_BITFMASK(IPU_DMA_CHA_DMAPF_5)
                            | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_6)
                            | CSP_BITFMASK(IPU_DMA_CHA_DMAPF_7);
                        iBitval = (UINT32) (CSP_BITFVAL(IPU_DMA_CHA_DMAPF_5, IPU_ENABLE)
                            | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_6, IPU_ENABLE)
                            | CSP_BITFVAL(IPU_DMA_CHA_DMAPF_7, IPU_ENABLE));

                        // enable IPU interrupts for channels 29, 30, 31 (PF Y, U, and V Output)
                        do
                        {
                            oldVal = INREG32(&P_IPU_REGS->IPU_INT_CTRL_1);
                            newVal = (oldVal & (~iMask)) | iBitval;
                        } while ((UINT32) InterlockedTestExchange((LPLONG)&P_IPU_REGS->IPU_INT_CTRL_1, 
                                oldVal, newVal) != oldVal);
                    }
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
    PF_FUNCTION_EXIT();
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
static void dumpChannelParams(pPfIDMACChannelParams pChannelParams)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pChannelParams);

    DEBUGMSG (ZONE_DEVICE, (TEXT("Width       = %x\r\n"), pChannelParams->iWidth));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Height      = %x\r\n"), pChannelParams->iHeight));
    DEBUGMSG (ZONE_DEVICE, (TEXT("BPP Code    = %x\r\n"), pChannelParams->iBitsPerPixelCode));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Line Stride = %x\r\n"), pChannelParams->iLineStride));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Format Code = %x\r\n"), pChannelParams->iFormatCode));
    DEBUGMSG (ZONE_DEVICE, (TEXT("Pixel Burst = %x\r\n"), pChannelParams->iPixelBurstCode));
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
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_1: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_1)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_2: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_2)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_3: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_3)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IPU_INT_STAT_5: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IPU_INT_STAT_5)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: PF_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->PF_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: SDC_COM_CONF: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->SDC_COM_CONF)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: SDC_FG_POS: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->SDC_FG_POS)));
    DEBUGMSG (ZONE_DEVICE, (TEXT("%s: IDMAC_CHA_BUSY: %x\r\n"), __WFUNCTION__, INREG32(&pIPU->IDMAC_CHA_BUSY)));
}

#if 0 // Remove-W4: Warning C4505 workaround
static void ReadVfDMA(PCSP_IPU_REGS pIPU)
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
        Sleep(0);
    }

    OUTREG32(&pIPU->IPU_IMA_ADDR,
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_MEM_NU, IPU_IMA_ADDR_MEM_NU_CPM) |
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_ROW_NU, 2)|
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_WORD_NU, 0));

    for (i = 0; i < 132; i += 32)
    {
        data = INREG32(&pIPU->IPU_IMA_DATA);
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s(): Word0, bits %d - %d: %x\r\n"), __WFUNCTION__, i, i+32, data));
    }

    OUTREG32(&pIPU->IPU_IMA_ADDR,
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_MEM_NU, IPU_IMA_ADDR_MEM_NU_CPM) |
            CSP_BITFVAL( IPU_IPU_IMA_ADDR_ROW_NU, 3)|
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

//------------------------------------------------------------------------------
//
//  Function:  PfAllocPhysMem
//
//  This function allocates physically contiguous memory
//  
//  Parameters: 
//      cbSize   
//          [in] Number of bytes to allocate
//
//      pPhysicalAddress
//          [out] Pointer to a physical address
//
//      driverVirtualAddress
//          [out] Pointer to a virtual Address 
//
//  Returns: If the allocation is failed, return FALSE, otherwise, return user 
//           virtual address. 
// 
//------------------------------------------------------------------------------
void *PfClass::PfAllocPhysMem(UINT32 cbSize, PPhysicalAddress pPhysicalAddress, PUINT pDriverVirtualAddress)
{
    LPVOID lpUserAddr = NULL; 
    ULONG SourceSize; 
    ULONGLONG SourcePhys; 
    void* pvProcess;
    
    PF_FUNCTION_ENTRY();
    
    pvProcess=(void*)GetCallerVMProcessId(); 
    *pDriverVirtualAddress = (ULONG)AllocPhysMem(cbSize, PAGE_READWRITE, 0, 0, pPhysicalAddress);

    if (*pDriverVirtualAddress == 0)
    {
        PF_FUNCTION_EXIT();
        return NULL;
    } 

    SourcePhys = *pPhysicalAddress & ~(PAGE_SIZE - 1); 
    SourceSize = cbSize + (*pPhysicalAddress & (PAGE_SIZE - 1)); 

    lpUserAddr = VirtualAllocEx(pvProcess, 0, SourceSize, MEM_RESERVE, PAGE_NOACCESS); 
    if (lpUserAddr == NULL)
    {
        FreePhysMem((LPVOID )*pDriverVirtualAddress);
        PF_FUNCTION_EXIT();
        return NULL; 
    }
    
#pragma warning(push)
#pragma warning(disable: 4305) 
   
    if (!VirtualCopyEx(pvProcess, lpUserAddr, GetCurrentProcess(),(PVOID)(SourcePhys >> 8), 
                                  SourceSize, PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
#pragma warning(pop)                                  
    {
        FreePhysMem((LPVOID )*pDriverVirtualAddress);
        VirtualFreeEx(pvProcess, lpUserAddr, 0, MEM_RELEASE);
        PF_FUNCTION_EXIT();
        return NULL;
    }

    PF_FUNCTION_EXIT();
    
    {
        BOOL ret;
        DWORD oldflag;
        // change to write-through caching, normal space(strongly ordered by default)
        ret=VirtualSetAttributesEx(pvProcess,lpUserAddr,SourceSize,0x8,0xC,&oldflag);
        if(ret!=TRUE)
        {
            ERRORMSG(1, (TEXT("VirtualSetAttributesEx() failed \r\n")));
        }
        else
        {
/*
            // query if attributes is set
            ret=VirtualSetAttributesEx(pvProcess,lpUserAddr,SourceSize,0,0,&oldflag);
            if(ret==TRUE)
                RETAILMSG(1, (TEXT("ex:lpUserAddr : 0x%x, Old flag: 0x%x \r\n"), lpUserAddr,oldflag));  
*/
        }
    }

    return lpUserAddr;
    
}


//------------------------------------------------------------------------------
//
//  Function:  pfFreePhysMem
//
//  This function releases physical memory back to the system
//  
//  Parameters:
//      pBitsStreamBufMemParams 
//          [in] Pointer to the memory address parameters returned from PFAllocPhysMem
//
//  Return Values:
//      If the free is sucessful,then return TRUE; 
//      otherwise, return FALSE.
//
//------------------------------------------------------------------------------

BOOL PfClass::pfFreePhysMem(pPfAllocMemoryParams pBitsStreamBufMemParams) 
{
    BOOL bResult = FALSE;
       
    bResult = VirtualFreeEx((void*)GetCallerVMProcessId(), (PVOID)pBitsStreamBufMemParams->userVirtAddr, MEM_DECOMMIT, pBitsStreamBufMemParams->size); 
   
    return bResult;
}

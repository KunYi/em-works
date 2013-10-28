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
// File: PpClass.cpp
//
// This file contains the methods of post-processing class.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include <NKIntr.h>
#include "csp.h"
#include "emma.h"
#include "pp.h"
#include "PpClass.h"

//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPPpAlloc();
extern void BSPPpDealloc();
extern BOOL BSPPpSetClockGatingMode(BOOL bEnable);
extern void BSPSetPpBufferThreadPriority();
extern void BSPSetPpIntrThreadPriority();

//------------------------------------------------------------------------------
// External Variables 
 
//------------------------------------------------------------------------------
// Defines 

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables 

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: PpClass
//
// Post-processor class constructor.  Calls PpInit to initialize module.
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
// This function initializes the post-processor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpInit(void)
{
    PP_FUNCTION_ENTRY();

    if (!PpAlloc())
        goto _error;

    if (!PpEventsInit())
        goto _error;

    // Initial settings
    m_bConfigured = FALSE;

    PP_FUNCTION_EXIT();

    return TRUE;
    
_error:
    
    PpDeinit();

    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PpDeinit
//
// This function deinitializes the post-processor.
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

    PpEventsDeinit();
    
    PpDealloc();

    PP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PpAlloc
//
// This function allocates data structure for post-processor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpAlloc(void)
{
    MSGQUEUEOPTIONS queueOptions;
    
    PP_FUNCTION_ENTRY();
    
    // Map post-processor registers
    PHYSICAL_ADDRESS phyAddr;
    phyAddr.QuadPart = CSP_BASE_REG_PA_EMMA_PP;
    m_pPpReg = (PCSP_PP_REGS)MmMapIoSpace(phyAddr, sizeof(CSP_PP_REGS), FALSE);
    if (m_pPpReg == NULL) {
        DEBUGMSG(ZONE_ERROR, 
          (TEXT("%s: Map PP registers failed\r\n"), __WFUNCTION__));
        return FALSE;
    }    

    //
    // Create queues for reading and writing messages
    //
    queueOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    queueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    queueOptions.dwMaxMessages = PP_MAX_NUM_BUFFERS;
    queueOptions.cbMaxMessage = sizeof(ppBuffers);
    queueOptions.bReadAccess = TRUE;    // we need read-access to msgqueue

    // Create read handles for queues    
    m_hReadPpBufferQueue = CreateMsgQueue(NULL, &queueOptions);
    if (m_hReadPpBufferQueue == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Creating input buffer queue failed.\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    queueOptions.bReadAccess = FALSE;   // we need write-access to msgqueue

    // Create write handles for queues
    m_hWritePpBufferQueue = OpenMsgQueue(GetCurrentProcess(), 
        m_hReadPpBufferQueue, &queueOptions);
    if (m_hWritePpBufferQueue == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Opening input buffer queue for writing failed.\r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    // Call BSP-Specific allocation
    if (!BSPPpAlloc()) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Calling BSPPpAlloc() failed.\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    PP_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpDealloc
//
// This function deallocates data structure for post-processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpDealloc(void)
{
    PP_FUNCTION_ENTRY();
    
    // Unmap post-processor registers
    if (m_pPpReg != NULL) {
        MmUnmapIoSpace(m_pPpReg, sizeof(CSP_PP_REGS));
        m_pPpReg = NULL;
    }

    //
    // Close messages queues handlers
    //
    if (m_hReadPpBufferQueue != NULL) {    
        CloseHandle(m_hReadPpBufferQueue);
        m_hReadPpBufferQueue = NULL;
    }

    if (m_hWritePpBufferQueue != NULL) {
        CloseHandle(m_hWritePpBufferQueue);
        m_hWritePpBufferQueue = NULL;
    }

    // Call BSP-Specific deallocation
    BSPPpDealloc();

    PP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PpEventsInit
//
// This function initializes events related for post-processor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpEventsInit(void)
{
    PP_FUNCTION_ENTRY();
    
    //
    // Initialization for post-processor interrupt
    //
    // Create event for post-processor interrupt
    m_hPpIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hPpIntrEvent == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for PP interrupt failed\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    // Request sysintr for post-processor interrupt
    DWORD irq = IRQ_EMMAPP;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
        &m_iPpSysintr, sizeof(DWORD), NULL)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Request SYSINTR for PP interrupt failed\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    // Initialize post-processor interrupt
    if (!InterruptInitialize(m_iPpSysintr, m_hPpIntrEvent, NULL, NULL)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Initialize PP interrupt failed\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Create the thread for post-processor interrupt
    m_hPpIntrThread = ::CreateThread(NULL, 0, 
        (LPTHREAD_START_ROUTINE)PpIntrThread, this, 0, NULL);
    if (m_hPpIntrThread == NULL) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Create thread for PP interrupt failed\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    //
    // Initialize post-processing buffer worker thread
    //
    m_hPpBufThread = ::CreateThread(NULL, 0, 
        (LPTHREAD_START_ROUTINE)PpBufferWorkerThread, this, 0, NULL);
    if (m_hPpBufThread == NULL) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Create thread for post-processing buffer worker failed\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    //
    // Create other required events
    //
    // Event for signaling pin that frame is ready
    m_hPpEOFEvent = CreateEvent(NULL, FALSE, FALSE, PP_EOF_EVENT_NAME);
    if (m_hPpEOFEvent == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for indicating PP EOF\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    // Event for signaling buffer worker thread that 
    // post-processor is ready for the new processing.
    m_hPpReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hPpReadyEvent == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for indicating PP ready\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    PP_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpEventsDeinit
//
// This function deinitializes events related for post-processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpEventsDeinit(void)
{
    PP_FUNCTION_ENTRY();
    
    // Release sysintr for post-processor interrupt
    if (m_iPpSysintr != SYSINTR_UNDEFINED) {
        // Disable post-processor interrupt first
        InterruptDisable(m_iPpSysintr);
        if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_iPpSysintr, 
            sizeof(DWORD), NULL, 0, NULL)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Release sysintr for PP interrupt failed\r\n"), 
                __WFUNCTION__));
        }
    }

    // Close events
    if (m_hPpEOFEvent != NULL) {
        CloseHandle(m_hPpEOFEvent);
        m_hPpEOFEvent = NULL;
    }

    if (m_hPpReadyEvent != NULL) {
        CloseHandle(m_hPpReadyEvent);
        m_hPpReadyEvent = NULL;
    }

    if (m_hPpIntrEvent != NULL) {
        CloseHandle(m_hPpIntrEvent);
        m_hPpIntrEvent = NULL;
    }

    // Terminate interrupt threads
    if (m_hPpIntrThread != NULL) {
        TerminateThread(m_hPpIntrThread, 0);
        CloseHandle(m_hPpIntrThread);
        m_hPpIntrThread = NULL;
    }

    if (m_hPpBufThread != NULL) {
        TerminateThread(m_hPpBufThread, 0);
        CloseHandle(m_hPpBufThread);
        m_hPpBufThread = NULL;
    }

    PP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PpEnable
//
// This function enables post-processor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpEnable(void)
{
    PP_FUNCTION_ENTRY();

    // Enable eMMA clock
    if (!BSPPpSetClockGatingMode(TRUE)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Enable PP failed. It probably is being used by someone. \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }
    
    // Software reset post-processor
    INSREG32BF(&m_pPpReg->PP_CNTL, PP_CNTL_SWRST, PP_CNTL_SWRST_RESET);
    // Suppose it will clear itself after software reset
    while (EXTREG32BF(&m_pPpReg->PP_CNTL, PP_CNTL_SWRST)) {
        Sleep(0); // Relinquish current time slot for other thread
    }

    // Enable interrupts for frame completing and error occuring
    // Disable the MB completing interrupt, since we don't support it
    OUTREG32(&m_pPpReg->PP_INTRCNTL, 
        CSP_BITFVAL(PP_INTRCNTL_FRAME_COMP_INTR_EN, 
            PP_INTRCNTL_FRAME_COMP_INTR_EN_ENABLE) |
        CSP_BITFVAL(PP_INTRCNTL_ERR_INTR_EN, 
            PP_INTRCNTL_ERR_INTR_EN_ENABLE));

    // Empty buffer queue
    PpClearBuffers();
    
    PP_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpDisable
//
// This function disables post-processor.
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
    PP_FUNCTION_ENTRY();
    
    // Disable post-processor first
    OUTREG32(&m_pPpReg->PP_CNTL, 0);

    // In case there is still one last pending interrupt
    // Clear interrupt status (w1c)
    OUTREG32(&m_pPpReg->PP_INTRSTATUS, 
        CSP_BITFVAL(PP_INTRSTATUS_ERR_INTR, 1) | 
        CSP_BITFVAL(PP_INTRSTATUS_FRAME_COMP_INTR, 1));

    // Disable interrupt
    OUTREG32(&m_pPpReg->PP_INTRCNTL, 0);

    // Disable eMMA clock
    BSPPpSetClockGatingMode(FALSE);

    // Revert flag
    m_bConfigured = FALSE;

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpConfigure
//
// This function is used to configure post-processor registers.
//
// Parameters:      
//      pConfigData
//          [in] Pointer to configuaration data structure.
//
// Returns:     
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PpClass::PpConfigure(pPpConfigData pConfigData)
{
    PP_FUNCTION_ENTRY();

    if (pConfigData == NULL) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid configuration data!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    
    // Enable required operations
    if (pConfigData->bDeblock)
        INSREG32BF(&m_pPpReg->PP_CNTL, 
            PP_CNTL_DEBLOCKEN, PP_CNTL_DEBLOCKEN_ENABLE);
    else
        INSREG32BF(&m_pPpReg->PP_CNTL, 
            PP_CNTL_DEBLOCKEN, PP_CNTL_DEBLOCKEN_DISABLE);
    
    if (pConfigData->bDering)
        INSREG32BF(&m_pPpReg->PP_CNTL, 
            PP_CNTL_DERINGEN, PP_CNTL_DERINGEN_ENABLE);
    else
        INSREG32BF(&m_pPpReg->PP_CNTL, 
            PP_CNTL_DERINGEN, PP_CNTL_DERINGEN_DISABLE);

    // Indicates if we need quantizer parameter from user
    m_bNeedQP = (pConfigData->bDeblock || pConfigData->bDering);

    if (!PpConfigureInput(pConfigData))
        goto _error;

    if (!PpConfigureOutput(pConfigData))
        goto _error;

    if (!PpConfigureCSC(pConfigData))
        goto _error;

    if (!PpConfigureResize(pConfigData))
        goto _error;

    // Indicates the configuration is done
    m_bConfigured = TRUE;

    PP_FUNCTION_EXIT();

    return TRUE;

_error:
    // Make a software reset
    INSREG32BF(&m_pPpReg->PP_CNTL, PP_CNTL_SWRST, PP_CNTL_SWRST_RESET);
    // Suppose it will clear itself after software reset
    while (EXTREG32BF(&m_pPpReg->PP_CNTL, PP_CNTL_SWRST)) {
        Sleep(0); // Relinquish current time slot for other thread
    }
    
    DEBUGMSG(ZONE_ERROR, 
        (TEXT("%s: Post-processor configuration failed\r\n"), 
        __WFUNCTION__));

    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PpConfigureInput
//
// This function configures the Post-processor input source.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigureInput(pPpConfigData pConfigData)
{   
    UINT16 iQFWidth;
    UINT16 iInputStride;

    PP_FUNCTION_ENTRY();

    // Alignment check
    if ((pConfigData->inputSize.width & 0x07) || 
        (pConfigData->inputSize.height & 0x07) ||
        (pConfigData->inputStride & 0x07)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Input image size is not aligned\r\n"), __WFUNCTION__));  
        return FALSE;
    }

    // Boundary check
    if ((pConfigData->inputSize.width < PP_IMAGE_MIN_WIDTH) || 
        (pConfigData->inputSize.height < PP_IMAGE_MIN_HEIGHT) || 
        (pConfigData->inputSize.width > PP_IMAGE_MAX_WIDTH) || 
        (pConfigData->inputSize.height > PP_IMAGE_MAX_HEIGHT)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Input image size is invalid\r\n"), __WFUNCTION__)); 
        return FALSE;
    }

    // Set input frame parameter register
    INSREG32BF(&m_pPpReg->PP_PROCESS_FRAME_PARA, 
        PP_PROCESS_FRAME_PARA_PROCESS_FRAME_HEIGHT, 
        pConfigData->inputSize.height);
    INSREG32BF(&m_pPpReg->PP_PROCESS_FRAME_PARA, 
        PP_PROCESS_FRAME_PARA_PROCESS_FRAME_WIDTH, 
        pConfigData->inputSize.width);   

    // Set quantizer frame width if need
    if (m_bNeedQP) {
        // Must be multiple of 4 bytes
        iQFWidth = pConfigData->inputSize.width / PP_MB_SIZE;
        iQFWidth = ((iQFWidth + 3) >> 2) << 2;
        INSREG32BF(&m_pPpReg->PP_SOURCE_FRAME_WIDTH, 
            PP_SOURCE_FRAME_WIDTH_QUANTIZER_FRAME_WIDTH, iQFWidth);
    }       

    // Set input line stride
    if (pConfigData->inputStride == 0) {
        // The case it's zero
        iInputStride = pConfigData->inputSize.width;
    } else {
        if (pConfigData->inputStride < pConfigData->inputSize.width) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Input stride (%d) is less than the least valid value (%d)\r\n"), 
                __WFUNCTION__, 
                pConfigData->inputStride, pConfigData->inputSize.width));
            return FALSE;
        } else {
            iInputStride = pConfigData->inputStride;
        }
    }
    
    INSREG32BF(&m_pPpReg->PP_SOURCE_FRAME_WIDTH, 
        PP_SOURCE_FRAME_WIDTH_Y_INPUT_LINE_STRIDE, iInputStride);

    PP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpConfigureOutput
//
// This function configures the Post-processor output.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigureOutput(pPpConfigData pConfigData)
{
    UINT8 iBpp;
    UINT16 iOutputStride;

    PP_FUNCTION_ENTRY();

    // Alignment check
    if ((pConfigData->outputSize.width & 0x01) ||
        (pConfigData->outputSize.height & 0x01) ||
        (pConfigData->outputStride & 0x03)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Output image size is not aligned\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Boundary check
    if ((pConfigData->outputSize.width < PP_IMAGE_MIN_WIDTH) || 
        (pConfigData->outputSize.height < PP_IMAGE_MIN_HEIGHT) || 
        (pConfigData->outputSize.width > PP_IMAGE_MAX_WIDTH) || 
        (pConfigData->outputSize.height > PP_IMAGE_MAX_HEIGHT)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Output image size is invalid\r\n"), __WFUNCTION__)); 
        return FALSE;
    }

    // Set output image parameter register
    INSREG32BF(&m_pPpReg->PP_DEST_IMAGE_SIZE, 
        PP_DEST_IMAGE_SIZE_OUT_IMAGE_HEIGHT, pConfigData->outputSize.height);
    INSREG32BF(&m_pPpReg->PP_DEST_IMAGE_SIZE, 
        PP_DEST_IMAGE_SIZE_OUT_IMAGE_WIDTH, pConfigData->outputSize.width);

    // Parameters validity check
    switch (pConfigData->outputFormat) {
        case ppCSCOutputFormat_RGB16:
        case ppCSCOutputFormat_YUV422:
            iBpp = 2;
            break;

        case ppCSCOutputFormat_RGB32:
            iBpp = 4;
            break;
    }
    if ((pConfigData->outputStride != 0) && 
        (pConfigData->outputStride < (pConfigData->outputSize.width * iBpp))) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Output stride %d is less than least valid value %d!\r\n"), 
            __WFUNCTION__, 
            pConfigData->outputStride, pConfigData->outputSize.width * iBpp));
        return FALSE;
    }

    // Set output line stride
    iOutputStride = pConfigData->outputStride;
    if (iOutputStride == 0) {
        // The case it's zero
        iOutputStride = pConfigData->outputSize.width * iBpp;
        // Output stride should always be rounded up as a multiple of 4 bytes
        iOutputStride = ((iOutputStride + 3) >> 2) << 2;
    }

    INSREG32BF(&m_pPpReg->PP_DEST_DISPLAY_WIDTH, 
        PP_DEST_DISPLAY_WIDTH_OUPUT_LINE_STRIDE, iOutputStride);

    // Setup DEST_FRAME_FORMAT_CNTL regsiter for RGB/YUV422
    OUTREG32(&m_pPpReg->PP_DEST_FRAME_FORMAT_CNTL, 
        CSP_BITFVAL(PP_DEST_FRAME_FORMAT_CNTL_BLUE_WIDTH, 
            pConfigData->outputPixelFormat.component2_width) |
        CSP_BITFVAL(PP_DEST_FRAME_FORMAT_CNTL_GREEN_WIDTH, 
            pConfigData->outputPixelFormat.component1_width) |
        CSP_BITFVAL(PP_DEST_FRAME_FORMAT_CNTL_RED_WIDTH, 
            pConfigData->outputPixelFormat.component0_width) |
        CSP_BITFVAL(PP_DEST_FRAME_FORMAT_CNTL_BLUE_OFFSET, 
            pConfigData->outputPixelFormat.component2_offset) |
        CSP_BITFVAL(PP_DEST_FRAME_FORMAT_CNTL_GREEN_OFFSET, 
            pConfigData->outputPixelFormat.component1_offset) |
        CSP_BITFVAL(PP_DEST_FRAME_FORMAT_CNTL_RED_OFFSET, 
            pConfigData->outputPixelFormat.component0_offset));

    PP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpConfigureCSC
//
// This function configures the Post-processor CSC.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigureCSC(pPpConfigData pConfigData)
{
    PP_FUNCTION_ENTRY();

    if (pConfigData->outputFormat == ppCSCOutputFormat_YUV422) {
        // The output format is YUV422
        DEBUGMSG(ZONE_INFO, 
            (TEXT("%s: Output format is YUV422\r\n"), __WFUNCTION__));
        INSREG32BF(&m_pPpReg->PP_CNTL, PP_CNTL_CSCEN, PP_CNTL_CSCEN_YUV);        
    } else {
        // The output format is RGB
        switch (pConfigData->outputFormat) {
            case ppCSCOutputFormat_RGB16:
                DEBUGMSG(ZONE_INFO, 
                    (TEXT("%s: Output format is RGB16\r\n"), __WFUNCTION__));
                break;
                
            case ppCSCOutputFormat_RGB32:
                DEBUGMSG(ZONE_INFO, 
                    (TEXT("%s: Output format is RGB32\r\n"), __WFUNCTION__));
                break;

            default:
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Invalid CSC output format (%d)\r\n"), 
                    __WFUNCTION__, pConfigData->outputFormat));                
                return FALSE;
        }

        // Set CSC equation
        if ((pConfigData->CSCEquation != ppCSCEquationA_1) &&
            (pConfigData->CSCEquation != ppCSCEquationA_0) &&
            (pConfigData->CSCEquation != ppCSCEquationB_1) &&
            (pConfigData->CSCEquation != ppCSCEquationB_0)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Invalid CSC equation (%d)\r\n"), 
                __WFUNCTION__, pConfigData->CSCEquation));
            return FALSE;
        } else {
            OUTREG32(&m_pPpReg->PP_CSC_COEF_0123,
                CSP_BITFVAL(PP_CSC_COEF_0123_C0, 
                    yuv2rgb_tbl[pConfigData->CSCEquation][0]) |
                CSP_BITFVAL(PP_CSC_COEF_0123_C1, 
                    yuv2rgb_tbl[pConfigData->CSCEquation][1]) |
                CSP_BITFVAL(PP_CSC_COEF_0123_C2, 
                    yuv2rgb_tbl[pConfigData->CSCEquation][2]) |
                CSP_BITFVAL(PP_CSC_COEF_0123_C3, 
                    yuv2rgb_tbl[pConfigData->CSCEquation][3]));
            
            OUTREG32(&m_pPpReg->PP_CSC_COEF_4,
                CSP_BITFVAL(PP_CSC_COEF_4_C4, 
                    yuv2rgb_tbl[pConfigData->CSCEquation][4]) |
                CSP_BITFVAL(PP_CSC_COEF_4_X0, 
                    yuv2rgb_tbl[pConfigData->CSCEquation][5]));

        }

        // Set control register for CSC
        INSREG32BF(&m_pPpReg->PP_CNTL, 
            PP_CNTL_CSCEN, PP_CNTL_CSCEN_RGB);
        INSREG32BF(&m_pPpReg->PP_CNTL, 
            PP_CNTL_CSC_OUT, pConfigData->outputFormat);
        INSREG32BF(&m_pPpReg->PP_CNTL, 
            PP_CNTL_CSC_TABLE_SEL, pConfigData->CSCEquation);
    }
    
    PP_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpConfigureResize
//
// This function configures the Post-processor resizing.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigureResize(pPpConfigData pConfigData)
{
    UINT8 iRetry = 0;
    UINT8 iHoriStart, iHoriEnd;
    UINT8 iVertStart, iVertEnd;

    FLOAT fHoriRatio, fVertRatio;
    
    UINT16 iInWidth = pConfigData->inputSize.width;
    UINT16 iInHeight = pConfigData->inputSize.height;
    UINT16 iOutWidth = pConfigData->outputSize.width;
    UINT16 iOutHeight = pConfigData->outputSize.height;
    UINT16 iVarInWidth, iVarOutWidth;
    UINT16 iVarInHeight, iVarOutHeight;

    PP_FUNCTION_ENTRY();
    
    // Only support 2:1 to 1:4 and a fixed 4:1
    if (((4 * iInWidth < iOutWidth) || (iInWidth > 2 * iOutWidth)) && 
        (iInWidth != 4 * iOutWidth)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid width resizing ratio\r\n"), __WFUNCTION__));
        return FALSE;
    }

    if (((4 * iInHeight < iOutHeight) || (iInHeight > 2 * iOutHeight)) &&
        (iInHeight != 4 * iOutHeight)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid height resizing ratio\r\n"), __WFUNCTION__));
        return FALSE;
    }

    //
    // Resizing ratio calculation
    //
    m_iHoriLen = 0;
    m_iVertLen = 0;

    // Width
    if (iInWidth != iOutWidth) {
        iVarInWidth = iInWidth;
        iVarOutWidth = iOutWidth;
        while (iRetry < PP_MAX_RESIZE_RETRIES) {
            UINT16 gcd = PpGetGcd(iVarInWidth, iVarOutWidth);
            m_iHoriLen = PpResize(iVarInWidth/gcd, iVarOutWidth/gcd);
            if (m_iHoriLen < 0) {
                iRetry++;
                iVarOutWidth++;
                m_iHoriLen = 0;
            }
            else break;
        }
    } else {
        // Calculate it even for 1:1
        m_iHoriLen = PpResize(1, 1);
        if (m_iHoriLen < 0) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Width 1:1 resize calculation failure\r\n"), 
                __WFUNCTION__));
            return FALSE;
        }
    }
    if (iRetry == PP_MAX_RESIZE_RETRIES) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Width resize failure\r\n"), __WFUNCTION__));
        return FALSE;
    }
    iHoriStart = 0;
    iHoriEnd = m_iHoriLen - 1;

    // Height
    iRetry = 0;
    fHoriRatio = (iInWidth > iOutWidth) ?
        ((float)iInWidth / (float)iOutWidth) : 
        ((float)iOutWidth / (float)iInWidth);
    fVertRatio = (iInHeight > iOutHeight) ? 
        ((float)iInHeight / (float)iOutHeight) : 
        ((float)iOutHeight / (float)iInHeight);
    if (fHoriRatio != fVertRatio) {
        if (iInHeight != iOutHeight) {
            // UINT16 inHeight = iInHeight;
            iVarInHeight = iInHeight;
            iVarOutHeight = iOutHeight;
            while (iRetry < PP_MAX_RESIZE_RETRIES) {
                UINT16 gcd = PpGetGcd(iVarInHeight, iVarOutHeight);
                m_iVertLen = PpResize(iVarInHeight/gcd, iVarOutHeight/gcd);
                if (m_iVertLen < 0) {
                    iRetry++;
                    iVarOutHeight++;
                    m_iVertLen = 0;
                } else break;
            }
        } else {
            // Calculate it even for 1:1
            m_iVertLen = PpResize(1, 1);
            if (m_iVertLen < 0) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Height 1:1 resize calculation failure\r\n"), 
                    __WFUNCTION__));
                return FALSE;
            }
        }
        if (iRetry == PP_MAX_RESIZE_RETRIES) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Width resize failure\r\n"), __WFUNCTION__));
            return FALSE;
        }
        iVertStart = m_iHoriLen;
        iVertEnd = m_iHoriLen + m_iVertLen - 1;
    } else {
        iVertStart = iHoriStart;
        iVertEnd = iHoriEnd;
    }
    
    PpSetupResize(iVertStart, iVertEnd, iHoriStart, iHoriEnd);

    PP_FUNCTION_EXIT();
    
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PpGetGcd
//
// This function is used to calculate the greatest common divider of the 
// two passed in parameters.  This code is reused from Symbian.
//
// Parameters:  
//      x
//          [in] One number.
//
//      y
//          [in] The other one.
//
// Returns:
//      The greatest common divider of x and y.
//
//------------------------------------------------------------------------------
UINT16 PpClass::PpGetGcd(UINT16 x, UINT16 y)
{
    UINT16 temp;
    
    if (x < y) {
        temp = y;
        y = x;
        x = temp;
    }
    
    if ((x % y) == 0) return y;

    while ((temp = (x % y)) != 0) {
        x = y;
        y = temp;
    }
    
    return y; 
}

//------------------------------------------------------------------------------
//
// Function: PpResize
//
// This function is used to build resize table.  Bilinear interpolation is 
// implemented.  There is a special case for 4:1 scaling. 
//
// This code is reused from Symbian team.
//
// Parameters:
//      in
//          [in] Width/height of input frame.
//
//      out
//          [in] Width/height of output frame.
//
// Returns:     
//      Resized width/height if positive otherwise indicates failure.
//
//------------------------------------------------------------------------------
INT8 PpClass::PpResize(UINT16 in, UINT16 out)
{
    UINT8 iResizeLen = 0;
    
    // Up sampling
    if (in <= out) {
        UINT16 pos = 0;
        for (UINT16 i = 0; i < out; i++) {
            UINT16 w1 = out - pos;
            UINT16 w2 = pos;
            UINT16 next = 0;
            pos += in;
            if (pos >= out) {
                pos -= out;
                next++;
            }
            if ((iResizeLen + m_iHoriLen) >= PP_MAX_RESIZE_COEFFS) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Up sampling exceeds maximum coeffs\r\n"), 
                    __WFUNCTION__));
                return (-1);
            }
            
            PpStoreCoeffs(w1, w2, out, next, iResizeLen);
            iResizeLen++;
            
            if (pos == 0) break;
        }
    } else if (in <= 2 * out) {
        // Handle the cases for 2:1 and lower for down sampling
        UINT16 inPos1 = out;
        UINT16 inPosInc = 2 * out;
        UINT16 inPos2   = inPos1 + inPosInc;
        UINT16 outPos = in;
        UINT16 outPosInc = 2 * in;
        UINT16 init_carry = in - out;
        UINT16 carry = init_carry;
        do {
            int w1 = inPos2 - outPos;
            int w2 = outPos - inPos1;
            int n = 0;
            outPos += outPosInc;
            carry += outPosInc;
            while (inPos2 < outPos) {
                inPos1 = inPos2;
                inPos2 += inPosInc;
                n++;
                carry -= inPosInc;
            }
            
            if ((iResizeLen + m_iHoriLen) >= PP_MAX_RESIZE_COEFFS) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Down sampling exceeds maximum coeffs\r\n"), 
                    __WFUNCTION__));

                return (-1);
            }
            
            PpStoreCoeffs(w1, w2, 2 * out, n, iResizeLen);
            iResizeLen++;
        } while (carry != init_carry);
    } else if (in == 4 * out) { 
        // Down sampling by 4:1        
        m_iResizeCoeffs[m_iHoriLen] = 
            CSP_BITFVAL(PP_RESIZE_COEF_TBL_W, ((1 << PP_RESIZE_COEFF) / 2)) |
            CSP_BITFVAL(PP_RESIZE_COEF_TBL_N, 1) |
            CSP_BITFVAL(PP_RESIZE_COEF_TBL_OP, PP_RESIZE_COEF_TBL_OP_PIXELS);

        m_iResizeCoeffs[m_iHoriLen + 1] = CSP_BITFVAL(PP_RESIZE_COEF_TBL_N, 1) | 
            CSP_BITFVAL(PP_RESIZE_COEF_TBL_OP, PP_RESIZE_COEF_TBL_OP_NOPIXELS);

        m_iResizeCoeffs[m_iHoriLen + 2] = CSP_BITFVAL(PP_RESIZE_COEF_TBL_N, 1) | 
            CSP_BITFVAL(PP_RESIZE_COEF_TBL_OP, PP_RESIZE_COEF_TBL_OP_NOPIXELS);

        m_iResizeCoeffs[m_iHoriLen + 3] = CSP_BITFVAL(PP_RESIZE_COEF_TBL_N, 1) | 
            CSP_BITFVAL(PP_RESIZE_COEF_TBL_OP, PP_RESIZE_COEF_TBL_OP_NOPIXELS);

        iResizeLen = 4;
    } else {
        // The resize ratio is not supported
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Unsupported resizing ratio\r\n"), 
            __WFUNCTION__));
        return (-1);
    }
    
    return iResizeLen;
}

//------------------------------------------------------------------------------
//
// Function: PpStoreCoeffs
//
// This function is used to store the resizing instructions in the array
// m_iResizeCoeffs[].  Each entry in the array is in the form (w1, w2, n). 
// An output pixel will be generated with the value (w1*in1 + w2*in2) and 
// the resize engine will then read in a n new input pixels (where n != 0), 
// where in1 and in2 are two adjacent pixels.
//
// This code is reused from Symbian team.
//
// Parameters:
//      ...
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PpClass::PpStoreCoeffs(UINT16 w1, UINT16 w2, UINT16 base, 
    UINT16 next, UINT16 index)
{
    UINT16 iCoeff;
    
    if (m_iHoriLen != 0)
        index += m_iHoriLen;

    iCoeff = ((w1 << PP_RESIZE_COEFF) + (base >> 1)) / base;
    
    // 31 means 32, so 30 will handle both 30 & 31
    if (iCoeff == (PP_MAX_COEFF - 1))
        iCoeff--;
    else if (iCoeff == 1) 
        iCoeff++;
    else if (iCoeff == PP_MAX_COEFF)
        // 32 should be set to 31, as the HW treats 31 binary as 32 decimal
        iCoeff = PP_MAX_COEFF - 1;

    m_iResizeCoeffs[index] = CSP_BITFVAL(PP_RESIZE_COEF_TBL_W, iCoeff) |
        CSP_BITFVAL(PP_RESIZE_COEF_TBL_N, next) |
        CSP_BITFVAL(PP_RESIZE_COEF_TBL_OP, PP_RESIZE_COEF_TBL_OP_PIXELS);
}

//------------------------------------------------------------------------------
//
// Function: PpSetupResize
//
// This function is used to setup the resize registers.
//
// Parameters:
//      iVertStart
//          [in] Start index of vertial resize table.
//
//      iVertEnd
//          [in] End index of vertial resize table.
//
//      iHoriStart
//          [in] Start index of horizontal resize table.
//
//      iHoriEnd
//          [in] End index of horizontal resize table.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PpClass::PpSetupResize(UINT8 iVertStart, UINT8 iVertEnd, 
    UINT8 iHoriStart, UINT8 iHoriEnd)
{
    UINT16 len;
    UINT16 i;

    // Setup RESIZE_TBL_INDEX regiser
    OUTREG32(&m_pPpReg->PP_RESIZE_TABLE_INDEX, 
        CSP_BITFVAL(PP_RESIZE_TABLE_INDEX_VERT_TBL_END_INDEX, iVertEnd) |
        CSP_BITFVAL(PP_RESIZE_TABLE_INDEX_VERT_TBL_START_INDEX, iVertStart) |
        CSP_BITFVAL(PP_RESIZE_TABLE_INDEX_HORI_TBL_END_INDEX, iHoriEnd) |
        CSP_BITFVAL(PP_RESIZE_TABLE_INDEX_HORI_TBL_START_INDEX, iHoriStart));

    // Setup RESIZE_TABLE register
    if ((iVertEnd != iHoriEnd) && (iVertStart != iHoriStart)) {
        len = (iVertEnd - iVertStart) + (iHoriEnd - iHoriStart) + 2;
    } else {
        // If the horizontal and vertical resize ratios are the same,
        // then the iHoriStart can be the same as iVertStart and iHoriEnd 
        // can be the same as iVertEnd. 
        len = iVertEnd - iVertStart + 1;
    }
    for (i = 0; i < len; i++)
        OUTREG32(&m_pPpReg->PP_RESIZE_COEF_TBL[i], m_iResizeCoeffs[i]);
}

//-----------------------------------------------------------------------------
//
// Function: PpEnqueueBuffers
//
// This function adds buffers for the PP channel.  Input and output buffers
// are provided and added to the buffer queue.  It is assumed that the caller 
// has allocated physically contiguous buffers.
//
// Parameters:
//      pPpBufs
//          [in] Pointer to structure containing a pointer to an input and 
//          output buffer.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpEnqueueBuffers(pPpBuffers pPpBufs)
{
    PP_FUNCTION_ENTRY();

    //
    // Buffer boundary check
    // QP is needed only when deblock and/or dering enabled
    // Input, output and quantizer buffer should be word aligned
    //
    if (m_bNeedQP) {
        if ((pPpBufs->InQPBuf == NULL) || 
            (((UINT32)(pPpBufs->InQPBuf) & 0x03) != 0)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Invalid quantizer buffer (0x%08lX).\r\n"), 
                __WFUNCTION__, pPpBufs->InQPBuf));
            return FALSE;
        }
    }

    if ((pPpBufs->InYBuf == NULL) || 
        ((UINT32)(pPpBufs->InYBuf) & 0x03) != 0) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid input Y buffer (0x%08lX).\r\n"), 
            __WFUNCTION__, pPpBufs->InYBuf));
        return FALSE;     
    }

    if ((pPpBufs->InUBuf == NULL) || 
        ((UINT32)(pPpBufs->InUBuf) & 0x03) != 0) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid input U buffer (0x%08lX).\r\n"), 
            __WFUNCTION__, pPpBufs->InUBuf));
        return FALSE;     
    }

    if ((pPpBufs->InVBuf == NULL) || 
        ((UINT32)(pPpBufs->InVBuf) & 0x03) != 0) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid input V buffer (0x%08lX).\r\n"), 
            __WFUNCTION__, pPpBufs->InVBuf));
        return FALSE;     
    }

    if ((pPpBufs->OutBuf == NULL) || 
        ((UINT32)(pPpBufs->OutBuf) & 0x03) != 0) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid output buffer (0x%08lX).\r\n"), 
            __WFUNCTION__, pPpBufs->OutBuf));
        return FALSE;     
    }

    //
    // Enqueue the Post-processing buffer
    //
    if (!WriteMsgQueue(m_hWritePpBufferQueue, pPpBufs, 
        sizeof(ppBuffers), 0, 0)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Could not write to Post-processing buffer queue.\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    PP_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpClearBuffers
//
// This function clears out the work buffer queues for the Post-processor.
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
    DWORD dwFlags, dwBytesRead;

    // Read all buffers from buffer queue.
    while (ReadMsgQueue(m_hReadPpBufferQueue, &bufs, sizeof(ppBuffers), 
        &dwBytesRead, 0, &dwFlags));
}

//-----------------------------------------------------------------------------
//
// Function:  PpGetMaxBuffers
//
// This function returns the maximum number of buffers supported
// by the Post-processor.
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

//------------------------------------------------------------------------------
//
// Function: PpStart
//
// This function starts the postprocessing.
//
// Parameters:      
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//------------------------------------------------------------------------------
BOOL PpClass::PpStart()
{
    PP_FUNCTION_ENTRY();

    // Must have been configured
    if (!m_bConfigured) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Cannot start post-processing without configuring\r\n"),
            __WFUNCTION__));
        return FALSE;
    }
    
    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause PIN to believe a frame was ready.
    ResetEvent(m_hPpEOFEvent);
    
    // Initially set post-processor ready event
    SetEvent(m_hPpReadyEvent);

    // Frame counter
    m_iFrameCount = 0;

    PP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PpStop
//
// This function stops the postprocessing.
//
// Parameters:      
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//------------------------------------------------------------------------------
BOOL PpClass::PpStop()
{
    PP_FUNCTION_ENTRY();

    // Hang up buffer worker thread
    ResetEvent(m_hPpReadyEvent);

    PP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpGetFrameCount
//
// This function returns the current processed frame count since starting 
// post-processing.
//
// Parameters:
//      None.
//
// Returns:
//      The number of PP frames processed.
//
//-----------------------------------------------------------------------------
UINT32 PpClass::PpGetFrameCount()
{
    return m_iFrameCount;
}

//------------------------------------------------------------------------------
//
// Function: PpBufferWorkerThread
//
// This function is the worker thread for the handling buffers.
//
// Parameters:
//      lpParameter
//          [in] The parameter from CreateThread().
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PpClass::PpBufferWorkerThread(LPVOID lpParameter)
{
    PpClass *pPp = (PpClass *)lpParameter;

    PP_FUNCTION_ENTRY();

    BSPSetPpBufferThreadPriority();

    pPp->PpBufferWorkerRoutine();

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpBufferWorkerRoutine
//
// This function is the worker thread routine that assures the buffers are 
// managed correctly in the postprocessor.  When a buffer is filled, this 
// function will wait for a new buffer to become available and set up the 
// postprocessor to process.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PpClass::PpBufferWorkerRoutine()
{
    ppBuffers ppBufs;
    DWORD dwFlags, dwBytesRead;
    
    PP_FUNCTION_ENTRY();

    while (1) {
        // A buffer has been requested.  Attempt to read from the 
        // post-processing queue to get a buffer.  If one is not available,
        // we wait here until it is.
        WaitForSingleObject(m_hReadPpBufferQueue, INFINITE);

        // Now, read post-processing buffer queue to retrieve buffer pointer.
        if (!ReadMsgQueue(m_hReadPpBufferQueue, &ppBufs, 
            sizeof(ppBuffers), &dwBytesRead, 0, &dwFlags)) {
            DEBUGMSG (ZONE_THREAD | ZONE_INFO, 
                (TEXT("%s: No buffer reading from post-processing queue\r\n"), 
                __WFUNCTION__));
        }

        // Waiting until post-processor is ready for next frame processing
        WaitForSingleObject(m_hPpReadyEvent, INFINITE);

        // Fill post-processor buffer registers
        OUTREG32(&m_pPpReg->PP_SOURCE_Y_PTR, (UINT32)ppBufs.InYBuf);
        OUTREG32(&m_pPpReg->PP_SOURCE_CB_PTR, (UINT32)ppBufs.InUBuf);
        OUTREG32(&m_pPpReg->PP_SOURCE_CR_PTR, (UINT32)ppBufs.InVBuf);
        OUTREG32(&m_pPpReg->PP_QUANTIZER_PTR, (UINT32)ppBufs.InQPBuf);
        OUTREG32(&m_pPpReg->PP_DEST_RGB_PTR, (UINT32)ppBufs.OutBuf);
        
        // Start post-processing
        // This bit is self cleaned when one frame is finished.
        INSREG32BF(&m_pPpReg->PP_CNTL, PP_CNTL_PP_EN, PP_CNTL_PP_EN_ENABLE);
    }

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpIntrThread
//
// This function is the IST thread of Post-Processor.
//
// Parameters:
//      lpParameter
//          [in] The parameter from CreateThread().
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PpClass::PpIntrThread(LPVOID lpParameter)
{
    PpClass *pPp = (PpClass *)lpParameter;

    PP_FUNCTION_ENTRY();

    BSPSetPpIntrThreadPriority();

    pPp->PpIntrRoutine();

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpIntrRoutine
//
// This function is the interrupt handler for the Postprocessor.  It waits for
// the End-Of-Frame (EOF) interrupt, and signals the EOF event registered by 
// the user of the postprocessor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PpClass::PpIntrRoutine()
{    
    PP_FUNCTION_ENTRY();

    while (1) {
        // Wait post-processor interrupt event
        WaitForSingleObject(m_hPpIntrEvent, INFINITE);

        // Get pending interrupt type
        // MB intterupt disabled so only check error and frame interrupts.
        if (EXTREG32BF(&m_pPpReg->PP_INTRSTATUS, 
            PP_INTRSTATUS_ERR_INTR)) {
            // Suppose user will be aware of post-processing error 
            // by waiting EOF event timeout.
            DEBUGMSG(ZONE_THREAD, 
                (TEXT("%s: Post-processor processing error!\r\n"), 
                __WFUNCTION__));
        }
        
        if (EXTREG32BF(&m_pPpReg->PP_INTRSTATUS, 
            PP_INTRSTATUS_FRAME_COMP_INTR)) {
            DEBUGMSG(ZONE_THREAD, 
                (TEXT("%s: Post-processor completed one frame.\r\n"), 
                __WFUNCTION__));

            // Trigger EOF event to notify user 
            SetEvent(m_hPpEOFEvent);

            m_iFrameCount++;
        }

        // Clear interrupt status (w1c)
        OUTREG32(&m_pPpReg->PP_INTRSTATUS, 
            CSP_BITFVAL(PP_INTRSTATUS_ERR_INTR, 1) | 
            CSP_BITFVAL(PP_INTRSTATUS_FRAME_COMP_INTR, 1));

        InterruptDone(m_iPpSysintr);

        // Set event signaling buffer worker thread that 
        // post-processor is ready for next frame processing.
        SetEvent(m_hPpReadyEvent);
    }

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpDumpRegisters
//
// This function dumps post-processor registers.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PpClass::PpDumpRegisters(void)
{
    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: CNTL = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_CNTL));

    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: INTRCNTL = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_INTRCNTL));

    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: INTRSTATUS = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_INTRSTATUS));

    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: SOURCE_Y_PTR = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_SOURCE_Y_PTR));
    
    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: SOURCE_CB_PTR = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_SOURCE_CB_PTR));
    
    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: SOURCE_CR_PTR = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_SOURCE_CR_PTR));
    
    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: QUANTIZER_PTR = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_QUANTIZER_PTR));
    
    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: PROCESS_FRAME_PARA = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_PROCESS_FRAME_PARA));
    
    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: SOURCE_FRAME_WIDTH = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_SOURCE_FRAME_WIDTH));
    
    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: DEST_DISPLAY_WIDTH = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_DEST_DISPLAY_WIDTH));
    
    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: DEST_IMAGE_SIZE = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_DEST_IMAGE_SIZE));

    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: DEST_FRAME_FORMAT_CNTL = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_DEST_FRAME_FORMAT_CNTL));

    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: RESIZE_TABLE_INDEX = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_RESIZE_TABLE_INDEX));

    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: CSC_COEF_0123 = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_CSC_COEF_0123));

    DEBUGMSG(ZONE_REGS, 
        (TEXT("%s: CSC_COEF_4 = 0x%08lX\r\n"), 
        __WFUNCTION__, m_pPpReg->PP_CSC_COEF_4));
}


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
// File: PrpClass.cpp
//
// This file contains the methods of pre-processing class.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include <NKIntr.h>
#include "csp.h"
#include "emma.h"
#include "prp.h"
#include "PrpClass.h"
#ifndef PRP_MEM_MODE_DLL
#include "emmadbg.h"
#endif


//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPPrpAlloc();
extern void BSPPrpDealloc();
extern BOOL BSPPrpSetClockGatingMode(BOOL bEnable);
extern void BSPSetPrpIntrThreadPriority();

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
// Function: PrpClass
//
// Pre-processor class constructor.  Calls PrpInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PrpClass::PrpClass(void)
{
    PrpInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~PrpClass
//
// The destructor for PrpClass.  Calls PrpDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PrpClass::~PrpClass(void)
{
    PrpDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: PrpInit
//
// This function initializes the pre-processor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpInit(void)
{
    PRP_FUNCTION_ENTRY();

    m_bConfigured = FALSE;
    m_bEnableVfOutput = FALSE;
    m_bEnableEncOutput = FALSE;

    m_iVfFrameCount = 0;
    m_iEncFrameCount = 0;

#ifndef PRP_MEM_MODE_DLL
    m_bVfStopReq = FALSE;
    m_bEncStopReq = FALSE;

    m_iVfState = prpChannelStopped;
    m_iEncState = prpChannelStopped;
#endif

    m_pPrpConfig = NULL;

    if (!PrpAlloc())
        goto _error;

    PRP_FUNCTION_EXIT();

    return TRUE;
    
_error:
    
    PrpDeinit();

    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpDeinit
//
// This function deinitializes the pre-processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpDeinit(void)
{
    PRP_FUNCTION_ENTRY();
    
    PrpDealloc();

    PRP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PrpAlloc
//
// This function allocates data structure for pre-processor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpAlloc(void)
{    
    PRP_FUNCTION_ENTRY();
    
    // Map pre-processor registers
    PHYSICAL_ADDRESS phyAddr;
    phyAddr.QuadPart = CSP_BASE_REG_PA_EMMA_PRP;
    m_pPrpReg = (PCSP_PRP_REGS )MmMapIoSpace(phyAddr, 
        sizeof(CSP_PRP_REGS), FALSE);
    if (m_pPrpReg == NULL) {
        DEBUGMSG(ZONE_ERROR, 
          (TEXT("%s: Map PrP registers failed\r\n"), __WFUNCTION__));
        return FALSE;
    }    

    // Call BSP-Specific allocation
    if (!BSPPrpAlloc()) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Calling BSPPrpAlloc() failed.\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

#ifndef PRP_MEM_MODE_DLL
    // Buffer manager
    pEncBufferManager = new PrpBufferManager();
    pVfBufferManager = new PrpBufferManager();
#endif

    PRP_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpDealloc
//
// This function deallocates data structure for pre-processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpDealloc(void)
{
    PRP_FUNCTION_ENTRY();
    
    // Unmap PrP registers
    if (m_pPrpReg != NULL) {
        MmUnmapIoSpace(m_pPrpReg, sizeof(CSP_PRP_REGS));
        m_pPrpReg = NULL;
    }

    // Call BSP-Specific deallocation
    BSPPrpDealloc();

#ifndef PRP_MEM_MODE_DLL
    // Buffer manager
    if (pEncBufferManager != NULL)
        delete pEncBufferManager;
    if (pVfBufferManager != NULL)
        delete pVfBufferManager;
#endif

    if (NULL != m_pPrpConfig)
    {
       free(m_pPrpConfig);
       m_pPrpConfig = NULL;
    }

    PRP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PrpEventsInit
//
// This function initializes events related for pre-processor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpEventsInit(void)
{
    PRP_FUNCTION_ENTRY();
    
    //
    // Initialization for pre-processor interrupt
    //
    // Create event for pre-processor interrupt
    m_hPrpIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hPrpIntrEvent == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for PrP interrupt failed\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    // Request sysintr for pre-processor interrupt
    DWORD irq = IRQ_EMMAPRP;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
        &m_iPrpSysintr, sizeof(DWORD), NULL)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Request SYSINTR for PrP interrupt failed\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    // Initialize pre-processor interrupt
    if (!InterruptInitialize(m_iPrpSysintr, m_hPrpIntrEvent, NULL, NULL)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Initialize PrP interrupt failed\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Create the thread for pre-processor interrupt
    m_hPowerDown = FALSE;
    m_hPrpIntrThread = ::CreateThread(NULL, 0, 
        (LPTHREAD_START_ROUTINE)PrpIntrThread, this, 0, NULL);
    if (m_hPrpIntrThread == NULL) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Create thread for PrP interrupt failed\r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    //
    // Create other required events
    //
    // Event for signaling pin that viewfinding frame is ready
    m_hVfEOFEvent = CreateEvent(NULL, FALSE, FALSE, PRP_VF_EOF_EVENT_NAME);
    if (m_hVfEOFEvent == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for PRP viewfinding channel EOF failed! \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    // Event for signaling pin that encoding frame is ready
    m_hEncEOFEvent = CreateEvent(NULL, FALSE, FALSE, PRP_ENC_EOF_EVENT_NAME);
    if (m_hEncEOFEvent == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for PRP encoding channel EOF failed! \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

#ifndef PRP_MEM_MODE_DLL
    // Event for channel stop request
    m_hVfStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hVfStopEvent == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for viewfinding channel stop request failed! \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    m_hEncStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hEncStopEvent == NULL) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for encoding channel stop request failed! \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }
#endif

    PRP_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpEventsDeinit
//
// This function deinitializes events related for pre-processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpEventsDeinit(void)
{    
    PRP_FUNCTION_ENTRY();

    // Close events
    if (m_hVfEOFEvent != NULL) {
        CloseHandle(m_hVfEOFEvent);
        m_hVfEOFEvent = NULL;
    }

    if (m_hEncEOFEvent != NULL) {
        CloseHandle(m_hEncEOFEvent);
        m_hEncEOFEvent = NULL;
    }
    
    if (m_hPrpIntrEvent != NULL) {
        CloseHandle(m_hPrpIntrEvent);
        m_hPrpIntrEvent = NULL;
    }

#ifndef PRP_MEM_MODE_DLL
    if (m_hVfStopEvent != NULL) {
        CloseHandle(m_hVfStopEvent);
        m_hVfStopEvent = NULL;
    }

    if (m_hEncStopEvent != NULL) {
        CloseHandle(m_hEncStopEvent);
        m_hEncStopEvent = NULL;
    }
#endif

    m_hPowerDown = TRUE;
    if (m_hPrpIntrThread != NULL) 
    {
        SetInterruptEvent(m_iPrpSysintr);    // Simulate a interrupt event to let IST pass waiting and check the power down request
        WaitForSingleObject(m_hPrpIntrThread, 10000);    // Wait the IST to terminate
        CloseHandle(m_hPrpIntrThread);
        m_hPrpIntrThread = NULL;
    }

    // Release sysintr for pre-processor interrupt
    if ((m_iPrpSysintr != SYSINTR_UNDEFINED) &&
        (m_iPrpSysintr > SYSINTR_FIRMWARE) &&
        (m_iPrpSysintr <= SYSINTR_MAXIMUM) )
    {
        // Disable pre-processor interrupt first
        InterruptDisable(m_iPrpSysintr);
        if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_iPrpSysintr, 
            sizeof(DWORD), NULL, 0, NULL)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Release sysintr for PrP interrupt failed\r\n"), 
                __WFUNCTION__));
        }
    }

    PRP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: PrpPowerUp
//
// This function initialize the interrupt and event.
// Doing this before enable Prp module.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE : for success.
//      FALSE : for failure.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpPowerUp()
{
    PRP_FUNCTION_ENTRY();

    // Enable interrupt and event
    if (!PrpEventsInit()) {
        DEBUGMSG(ZONE_ERROR,  (TEXT("%s: PrpEventsInit failed! \r\n"), __WFUNCTION__));
        PrpEventsDeinit();
        return FALSE;
    }
    
    PRP_FUNCTION_EXIT();
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpPowerDown
//
// This function deinitialize the interrupt and event.
// Doing this after disable Prp Module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpPowerDown()
{
    PRP_FUNCTION_ENTRY();

    // Disable interrupt and event
    PrpEventsDeinit();

    PRP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PrpEnable
//
// This function enables pre-processor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE : for success.
//      FALSE : for failure.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpEnable(void)
{
    PRP_FUNCTION_ENTRY();

    // Enable eMMA clock
    if (!BSPPrpSetClockGatingMode(TRUE)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Enable PrP failed. It probably is being used by someone. \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    // Enable pre-processor clock gating.
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CLKEN, PRP_CNTL_CLKEN_ON);
        
    // Software reset pre-processor
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_SWRST, PRP_CNTL_SWRST_RESET);
    // Suppose it will clear itself after software reset
    while (EXTREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_SWRST)) {
        Sleep(1); // Relinquish current time slot for other thread
    }

    // Enable all pre-processor interrupts
    // CH2OVF is only available when CH2 flow control disabled
    OUTREG32(&m_pPrpReg->PRP_INTRCNTL, 
        CSP_BITFMASK(PRP_INTRCNTL_RDERRIE) |
        CSP_BITFMASK(PRP_INTRCNTL_CH1WERRIE) |
        CSP_BITFMASK(PRP_INTRCNTL_CH2WERRIE) |
        CSP_BITFMASK(PRP_INTRCNTL_CH2FCIE) |
        CSP_BITFMASK(PRP_INTRCNTL_CH1FCIE) |
        CSP_BITFMASK(PRP_INTRCNTL_CH2OVFIE) |
        CSP_BITFMASK(PRP_INTRCNTL_LBOVFIE));
    
    // When power up PrP Module the first time. Needn't configure PrP here.
    // When Enable the module by power management, restore the current setting.
    if (TRUE == m_bConfigured) 
    {
       PrpConfigure(m_pPrpConfig);
    }
        
    PRP_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpDisable
//
// This function disables pre-processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpDisable(void)
{
    PRP_FUNCTION_ENTRY();
    
    // Disable pre-processor first
    OUTREG32(&m_pPrpReg->PRP_CNTL, 0);

    // In case there is still one last pending interrupt
    // Clear interrupt status (w1c)
    OUTREG32(&m_pPrpReg->PRP_INTRSTATUS, 
        CSP_BITFMASK(PRP_INTRSTATUS_READERR) |
        CSP_BITFMASK(PRP_INTRSTATUS_CH1WRERR) |
        CSP_BITFMASK(PRP_INTRSTATUS_CH2WRERR) |
        CSP_BITFMASK(PRP_INTRSTATUS_CH1B1CI) |
        CSP_BITFMASK(PRP_INTRSTATUS_CH1B2CI) |
        CSP_BITFMASK(PRP_INTRSTATUS_CH2B1CI) |
        CSP_BITFMASK(PRP_INTRSTATUS_CH2B2CI) |
        CSP_BITFMASK(PRP_INTRSTATUS_CH2OVF) |
        CSP_BITFMASK(PRP_INTRSTATUS_LBOVF));

    // Disable interrupt
    OUTREG32(&m_pPrpReg->PRP_INTRCNTL, 0);

    // Disable pre-processor clock gating.
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CLKEN, PRP_CNTL_CLKEN_OFF);

    // Disable eMMA clock
    BSPPrpSetClockGatingMode(FALSE);

    PRP_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: PrpConfigure
//
// This function is used to configure pre-processor Registers.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuaration data structure.
//
// Returns:     
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpConfigure(pPrpConfigData pConfigData)
{
    PRP_FUNCTION_ENTRY();

    if (pConfigData == NULL)
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid configuration data!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // If it is the first time we configure the Prp module, allocate a memory
    // to store the current Prp module configuration.
    if (NULL == m_pPrpConfig)
    {
      m_pPrpConfig = (pPrpConfigData) malloc (sizeof(prpConfigData));
      if (NULL == m_pPrpConfig)    
      {
        DEBUGMSG(ZONE_ERROR, 
           (TEXT("%s: Get memory for store Prp Configure data failed!\r\n"), __WFUNCTION__));
        goto _error;
      }
    }
    
    if (m_pPrpConfig != pConfigData)
    {
       // Only if new setting comes, saving current setting.
       memcpy(m_pPrpConfig,pConfigData,sizeof(prpConfigData));
    }

    if (!PrpConfigureInput(pConfigData))
        goto _error;

    if (!PrpConfigureOutput(pConfigData))
        goto _error;

    if (!PrpConfigureCSC(pConfigData))
        goto _error;

    if (!PrpConfigureResize(pConfigData))
        goto _error;

    // Indicates the configuration is done
    m_bConfigured = TRUE;

    PRP_FUNCTION_EXIT();

    return TRUE;

_error:
    // Make a software reset
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_SWRST, PRP_CNTL_SWRST_RESET);
    // Suppose it will clear itself after software reset
    while (EXTREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_SWRST)) {
        Sleep(1); // Relinquish current time slot for other thread
    }
    
    DEBUGMSG(ZONE_ERROR, 
        (TEXT("%s: Pre-processor configuration failed\r\n"), 
        __WFUNCTION__));

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PrpConfigureInput
//
// This function configures the pre-processor input source.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureInput(pPrpConfigData pConfigData)
{
    UINT16 nInputBytesPerLine;
    
    PRP_FUNCTION_ENTRY();

    // Get input format
    m_iInputFormat = pConfigData->inputFormat;

    // Alignment check
    if (m_iInputFormat == prpInputFormat_YUV420) {
        if ((pConfigData->inputSize.width & 0x07) || 
            (pConfigData->inputSize.height & 0x01)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Input YUV420 image size is not aligned. \r\n"), 
                __WFUNCTION__));
            return FALSE;
        }
    } else { // RGB, YUV422, YUV444
        if (pConfigData->inputSize.width & 0x03) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Input image size is not aligned. \r\n"), 
                __WFUNCTION__));
            return FALSE;
        }
    }

    // Boundary check
    if ((pConfigData->inputSize.width < PRP_IMAGE_MIN_WIDTH) || 
        (pConfigData->inputSize.height < PRP_IMAGE_MIN_HEIGHT) || 
        (pConfigData->inputSize.width > PRP_IMAGE_MAX_WIDTH) || 
        (pConfigData->inputSize.height > PRP_IMAGE_MAX_HEIGHT)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Input image size is invalid. \r\n"), __WFUNCTION__)); 
        return FALSE;
    }

    // Setup input image size parameters
    INSREG32BF(&m_pPrpReg->PRP_SOURCE_FRAME_SIZE, 
        PRP_SOURCE_FRAME_SIZE_PICTURE_X_SIZE, pConfigData->inputSize.width);
    INSREG32BF(&m_pPrpReg->PRP_SOURCE_FRAME_SIZE, 
        PRP_SOURCE_FRAME_SIZE_PICTURE_Y_SIZE, pConfigData->inputSize.height);

    // Setup input format parameters
    switch (m_iInputFormat) {
        case prpInputFormat_YUV420:
            // YUV420 input only be available in Memory mode.
#ifndef PRP_MEM_MODE_DLL
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: YUV420 is an invalid input format for CSI mode. \r\n"), 
                __WFUNCTION__));
            return FALSE;
#else
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_DATA_IN_MODE, PRP_CNTL_DATA_IN_MODE_YUV420);
            nInputBytesPerLine = pConfigData->inputSize.width * 3 / 2;
            break;
#endif

        case prpInputFormat_YUYV422:
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_DATA_IN_MODE, PRP_CNTL_DATA_IN_MODE_YUV422);
            OUTREG32(&m_pPrpReg->PRP_SOURCE_PIXEL_FORMAT_CNTL, 0x22000888);
            nInputBytesPerLine = pConfigData->inputSize.width * 2;
            break;

        case prpInputFormat_YVYU422:
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_DATA_IN_MODE, PRP_CNTL_DATA_IN_MODE_YUV422);
            OUTREG32(&m_pPrpReg->PRP_SOURCE_PIXEL_FORMAT_CNTL, 0x20100888);
            nInputBytesPerLine = pConfigData->inputSize.width * 2;
            break;

        case prpInputFormat_UYVY422:
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_DATA_IN_MODE, PRP_CNTL_DATA_IN_MODE_YUV422);
            OUTREG32(&m_pPrpReg->PRP_SOURCE_PIXEL_FORMAT_CNTL, 0x03080888);
            nInputBytesPerLine = pConfigData->inputSize.width * 2;
            break;

        case prpInputFormat_VYUY422:
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_DATA_IN_MODE, PRP_CNTL_DATA_IN_MODE_YUV422);
            OUTREG32(&m_pPrpReg->PRP_SOURCE_PIXEL_FORMAT_CNTL, 0x01180888);
            nInputBytesPerLine = pConfigData->inputSize.width * 2;
            break;

        case prpInputFormat_YUV444:
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_DATA_IN_MODE, PRP_CNTL_DATA_IN_MODE_YUV444);
            OUTREG32(&m_pPrpReg->PRP_SOURCE_PIXEL_FORMAT_CNTL, 0x62080888);
            nInputBytesPerLine = pConfigData->inputSize.width * 4;
            break;

        case prpInputFormat_RGB16:
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_DATA_IN_MODE, PRP_CNTL_DATA_IN_MODE_RGB16);
            OUTREG32(&m_pPrpReg->PRP_SOURCE_PIXEL_FORMAT_CNTL, 0x2CA00565);
            nInputBytesPerLine = pConfigData->inputSize.width * 2;
            break;

        case prpInputFormat_RGB32:
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_DATA_IN_MODE, PRP_CNTL_DATA_IN_MODE_RGB32);
            OUTREG32(&m_pPrpReg->PRP_SOURCE_PIXEL_FORMAT_CNTL, 0x41000888);
            nInputBytesPerLine = pConfigData->inputSize.width * 4;
            break;

        default:
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Invalid input format. \r\n"), __WFUNCTION__));
            return FALSE;
    }

    // Setup FIFO level
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_INPUT_FIFO_LEVEL, PRP_CNTL_INPUT_FIFO_LEVEL_128W);
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_RZ_FIFO_LEVEL, PRP_CNTL_RZ_FIFO_LEVEL_64W);

#ifndef PRP_MEM_MODE_DLL
    // Setup CSI windowing
    if (pConfigData->bWindowing) {
        // The number of pixels to skip from start of a line 
        // should be multiple of 2
        if (pConfigData->inputStride & 0x01) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: The input stride for CSI windowing is not aligned. \r\n"),
                __WFUNCTION__));
            return FALSE;
        }
        
        INSREG32BF(&m_pPrpReg->PRP_SOURCE_LINE_STRIDE, 
            PRP_SOURCE_LINE_STRIDE_SOURCE_LINE_STRIDE, pConfigData->inputStride);
        INSREG32BF(&m_pPrpReg->PRP_SOURCE_LINE_STRIDE, 
            PRP_SOURCE_LINE_STRIDE_CSI_LINE_SKIP, pConfigData->CSILineSkip);
        
        INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_WEN, PRP_CNTL_WEN_ENABLE);
    }

    // Setup frame skip control
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_IN_TSKIP, pConfigData->CSIInputSkip);
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_CH1_TSKIP, pConfigData->VfOutputSkip);
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_CH2_TSKIP, pConfigData->EncOutputSkip);
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_SKIP_FRAME, PRP_CNTL_SKIP_FRAME_STOP);

    // Setup LOOP mode
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_CH1_LEN, PRP_CNTL_CH1_LEN_ENABLE);
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_CH2_LEN, PRP_CNTL_CH2_LEN_ENABLE);

    // CSI mode
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_CSIEN, PRP_CNTL_CSIEN_CSI);
#else // PRP_MEM_MODE_DLL
    if (pConfigData->bWindowing) {
        if (m_iInputFormat == prpInputFormat_YUV420) {
            if (pConfigData->inputStride & 0x07) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: The input stride in bytes must be 8 aligned for Memory mode YUV420. \r\n"),
                    __WFUNCTION__));
                return FALSE;
            }
        } else if (pConfigData->inputStride & 0x03) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: The input stride in bytes must be 4 aligned for Memory mode. \r\n"),
                __WFUNCTION__));
            return FALSE;
        }

        if (pConfigData->inputStride < nInputBytesPerLine) {
            DEBUGMSG(ZONE_WARN, 
                (TEXT("%s: The input stride in bytes is less than the minimum valid one. \r\n"),
                __WFUNCTION__));
            return FALSE;
        }

        INSREG32BF(&m_pPrpReg->PRP_SOURCE_LINE_STRIDE, 
            PRP_SOURCE_LINE_STRIDE_SOURCE_LINE_STRIDE, pConfigData->inputStride);

        // Enable windowing
        INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_WEN, PRP_CNTL_WEN_ENABLE);

        // Only used for YUV420
        m_iMemInputYSize = YUV420_U_OFFSET(pConfigData->inputStride) * pConfigData->inputSize.height;
    } else {
        INSREG32BF(&m_pPrpReg->PRP_SOURCE_LINE_STRIDE, 
            PRP_SOURCE_LINE_STRIDE_SOURCE_LINE_STRIDE, nInputBytesPerLine);

        // Only used for YUV420
        m_iMemInputYSize = YUV420_U_OFFSET(nInputBytesPerLine) * pConfigData->inputSize.height;
    }
    
    // Memory mode
    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
        PRP_CNTL_CSIEN, PRP_CNTL_CSIEN_MEM);
#endif // PRP_MEM_MODE_DLL

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrpConfigureOutput
//
// This function configures the pre-processor output.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureOutput(pPrpConfigData pConfigData)
{
    UINT16 nOutputVfBPP;

    PRP_FUNCTION_ENTRY();

    // Get output format for both viewfinding and encoding channels
    m_iOutputVfFormat = pConfigData->outputVfFormat;
    if (m_iOutputVfFormat == prpVfOutputFormat_Disabled)
        m_bEnableVfOutput = FALSE;
    else
        m_bEnableVfOutput = TRUE;

    m_iOutputEncFormat = pConfigData->outputEncFormat;
    if (m_iOutputEncFormat == prpEncOutputFormat_Disabled)
        m_bEnableEncOutput = FALSE;
    else
        m_bEnableEncOutput = TRUE;

    // Sainty check
    if ((m_bEnableVfOutput == FALSE) && (m_bEnableEncOutput == FALSE)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Both channels are disabled so we can do nothing! \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    //
    // Viewfinding channel configuration
    //
    if (m_bEnableVfOutput) {
        // Alignment check
        if (m_iOutputVfFormat == prpVfOutputFormat_RGB8) {
            if (pConfigData->outputVfSize.width & 0x03) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Width of viewfinding output image in RGB8 is not aligned. \r\n"), 
                    __WFUNCTION__));
                return FALSE;
            }
        } else if (m_iOutputVfFormat != prpVfOutputFormat_RGB32) { // RGB16, YUV422
            if (pConfigData->outputVfSize.width & 0x01) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Width of viewfinding output image in RGB16/YUV422 is not aligned. \r\n"), 
                    __WFUNCTION__));
                return FALSE;
            }
        }

        // Boundary check
        if ((pConfigData->outputVfSize.width < PRP_IMAGE_MIN_WIDTH) || 
            (pConfigData->outputVfSize.height < PRP_IMAGE_MIN_HEIGHT) || 
            (pConfigData->outputVfSize.width > PRP_IMAGE_MAX_WIDTH) || 
            (pConfigData->outputVfSize.height > PRP_IMAGE_MAX_HEIGHT)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Size of viewfinding output image is invalid. \r\n"), 
                __WFUNCTION__));
            return FALSE;
        }

        // Setup output image size parameters
        INSREG32BF(&m_pPrpReg->PRP_CH1_OUT_IMAGE_SIZE, 
            PRP_CH1_OUT_IMAGE_SIZE_CH1_OUT_IMAGE_WIDTH, 
            pConfigData->outputVfSize.width);
        INSREG32BF(&m_pPrpReg->PRP_CH1_OUT_IMAGE_SIZE, 
            PRP_CH1_OUT_IMAGE_SIZE_CH1_OUT_IMAGE_HEIGHT, 
            pConfigData->outputVfSize.height);

        // Setup output image format for viewfinding channel
        switch (m_iOutputVfFormat) {
            case prpVfOutputFormat_Disabled:
                m_bEnableVfOutput = FALSE;
                break;

            case prpVfOutputFormat_RGB8:
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH1_OUT_MODE, PRP_CNTL_CH1_OUT_MODE_RGB8);
                OUTREG32(&m_pPrpReg->PRP_CH1_PIXEL_FORMAT_CNTL, 0x14400332);
                nOutputVfBPP = 1;
                break;

            case prpVfOutputFormat_RGB16:
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH1_OUT_MODE, PRP_CNTL_CH1_OUT_MODE_RGB16);
                OUTREG32(&m_pPrpReg->PRP_CH1_PIXEL_FORMAT_CNTL, 0x2CA00565);
                nOutputVfBPP = 2;
                break;

            case prpVfOutputFormat_RGB32:
                INSREG32BF(&m_pPrpReg->PRP_CNTL,
                    PRP_CNTL_CH1_OUT_MODE, PRP_CNTL_CH1_OUT_MODE_RGB32);
                OUTREG32(&m_pPrpReg->PRP_CH1_PIXEL_FORMAT_CNTL, 0x41000888);
                nOutputVfBPP = 4;
                break;

            case prpVfOutputFormat_YUYV422:
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH1_OUT_MODE, PRP_CNTL_CH1_OUT_MODE_YUV422);
                OUTREG32(&m_pPrpReg->PRP_CH1_PIXEL_FORMAT_CNTL, 0x62000888);
                nOutputVfBPP = 2;
                break;

            case prpVfOutputFormat_YVYU422:
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH1_OUT_MODE, PRP_CNTL_CH1_OUT_MODE_YUV422);
                OUTREG32(&m_pPrpReg->PRP_CH1_PIXEL_FORMAT_CNTL, 0x60100888);
                nOutputVfBPP = 2;
                break;
                
            case prpVfOutputFormat_UYVY422:
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH1_OUT_MODE, PRP_CNTL_CH1_OUT_MODE_YUV422);
                OUTREG32(&m_pPrpReg->PRP_CH1_PIXEL_FORMAT_CNTL, 0x43080888);
                nOutputVfBPP = 2;
                break;

            case prpVfOutputFormat_VYUY422:
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH1_OUT_MODE, PRP_CNTL_CH1_OUT_MODE_YUV422);
                OUTREG32(&m_pPrpReg->PRP_CH1_PIXEL_FORMAT_CNTL, 0x41180888);
                nOutputVfBPP = 2;
                break;

            default:
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Invalid output format for viewfinding channel. \r\n"), 
                    __WFUNCTION__));
                return FALSE;
        }

        // Setup viewfinding channel output stride
        if (pConfigData->outputVfStride & 0x03) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Viewfinding channel output stride is not aligned. \r\n"), 
                __WFUNCTION__));
            return FALSE;
        }
            
        if (pConfigData->outputVfStride < 
            pConfigData->outputVfSize.width * nOutputVfBPP) {
            DEBUGMSG(ZONE_WARN, 
                (TEXT("%s: Viewfinding channel output stride needs fixup. \r\n"), 
                __WFUNCTION__));
            INSREG32BF(&m_pPrpReg->PRP_CH1_LINE_STRIDE, 
                PRP_CH1_LINE_STRIDE_CH1_OUT_LINE_STRIDE, 
                pConfigData->outputVfSize.width * nOutputVfBPP);
        } else {
            INSREG32BF(&m_pPrpReg->PRP_CH1_LINE_STRIDE, 
                PRP_CH1_LINE_STRIDE_CH1_OUT_LINE_STRIDE, 
                pConfigData->outputVfStride);
        }
    }

    //
    // Encoding channel configuration
    //
    if (m_bEnableEncOutput) {
        // Alignment check
        if (m_iOutputEncFormat == prpEncOutputFormat_YUV420) {
            if ((pConfigData->outputEncSize.width & 0x07) || 
                (pConfigData->outputEncSize.height & 0x01)) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Size of encoding output image in YUV420 is not aligned. \r\n"), 
                    __WFUNCTION__));
                return FALSE;
            }
        } else { // YUV422, YUV444
            if (pConfigData->outputEncSize.width & 0x01) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Width of encoding output image in YUV422/YUV444 is not aligned. \r\n"),
                    __WFUNCTION__));
                return FALSE;
            }
        }

        // Boundary check
        if ((pConfigData->outputEncSize.width < PRP_IMAGE_MIN_WIDTH) || 
            (pConfigData->outputEncSize.height < PRP_IMAGE_MIN_HEIGHT) || 
            (pConfigData->outputEncSize.width > PRP_IMAGE_MAX_WIDTH) || 
            (pConfigData->outputEncSize.height > PRP_IMAGE_MAX_HEIGHT)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Size of encoding output image is invalid. \r\n"), 
                __WFUNCTION__));
            return FALSE;
        }

        // Setup output image size parameters
        INSREG32BF(&m_pPrpReg->PRP_CH2_OUT_IMAGE_SIZE, 
            PRP_CH2_OUT_IMAGE_SIZE_CH2_OUT_IMAGE_WIDTH, 
            pConfigData->outputEncSize.width);
        INSREG32BF(&m_pPrpReg->PRP_CH2_OUT_IMAGE_SIZE, 
            PRP_CH2_OUT_IMAGE_SIZE_CH2_OUT_IMAGE_HEIGHT, 
            pConfigData->outputEncSize.height);

        // Save image Y size for 4:2:0 U V buffer address calculation 
        // later in function PrpBufferSetup()
        m_iEncOutputYSize = pConfigData->outputEncSize.width * 
            pConfigData->outputEncSize.height;

        // Setup output image format for encoding channel
        switch (m_iOutputEncFormat) {
            case prpEncOutputFormat_Disabled:
                m_bEnableEncOutput = FALSE;
                break;

            case prpEncOutputFormat_YUV420:
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH2_OUT_MODE, PRP_CNTL_CH2_OUT_MODE_YUV420);
                break;

            case prpEncOutputFormat_YUV422:
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH2_OUT_MODE, PRP_CNTL_CH2_OUT_MODE_YUV422);
                break;

            case prpEncOutputFormat_YUV444:
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH2_OUT_MODE, PRP_CNTL_CH2_OUT_MODE_YUV444);
                break;

            default:
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Invalid output format for encoding channel. \r\n"), 
                    __WFUNCTION__));
                return FALSE;
        }

#ifndef PRP_MEM_MODE_DLL
        // Enable encoding channel flow control
        INSREG32BF(&m_pPrpReg->PRP_CNTL, 
            PRP_CNTL_CH2FEN, PRP_CNTL_CH2FEN_ENABLE);
        INSREG32BF(&m_pPrpReg->PRP_CNTL, 
            PRP_CNTL_CH2B1EN, PRP_CNTL_CH2B1EN_ENABLE);
        INSREG32BF(&m_pPrpReg->PRP_CNTL, 
            PRP_CNTL_CH2B2EN, PRP_CNTL_CH2B2EN_ENABLE);
#endif
    }

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpConfigureCSC
//
// This function configures the pre-processor CSC.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureCSC(pPrpConfigData pConfigData)
{
    BOOL bInputRGB, bVfOutputRGB;
    
    PRP_FUNCTION_ENTRY();

    // Check if CSC is needed. 
    // Encoding channel always output YUV format, therefore we only need 
    // check input and viewfinding channel output.
    bInputRGB = (pConfigData->inputFormat == prpInputFormat_RGB16) || 
        (pConfigData->inputFormat == prpInputFormat_RGB32);
    
    bVfOutputRGB = (pConfigData->outputVfFormat == prpVfOutputFormat_RGB8) || 
        (pConfigData->outputVfFormat == prpVfOutputFormat_RGB16) || 
        (pConfigData->outputVfFormat == prpVfOutputFormat_RGB32);

    if (!bInputRGB && !bVfOutputRGB) {
        DEBUGMSG(ZONE_FUNCTION, 
            (TEXT("%s: CSC is not needed. \r\n"), __WFUNCTION__));
        return TRUE;
    }

    // Setup CSC equation
    switch (pConfigData->CSCEquation) {
        // RGB to YUV
        case prpCSCR2Y_A1:
        case prpCSCR2Y_A0:
        case prpCSCR2Y_B1:
        case prpCSCR2Y_B0:
            if (!bInputRGB) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: CSC setting is not consistent with input image format! \r\n"), 
                    __WFUNCTION__));
               return FALSE;
            }
            
            OUTREG32(&m_pPrpReg->PRP_CSC_COEF_012,
                CSP_BITFVAL(PRP_CSC_COEF_012_C0, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][0]) |
                CSP_BITFVAL(PRP_CSC_COEF_012_C1, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][1]) |
                CSP_BITFVAL(PRP_CSC_COEF_012_C2, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][2]));

            OUTREG32(&m_pPrpReg->PRP_CSC_COEF_345,
                CSP_BITFVAL(PRP_CSC_COEF_345_C3, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][3]) |
                CSP_BITFVAL(PRP_CSC_COEF_345_C4, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][4]) |
                CSP_BITFVAL(PRP_CSC_COEF_345_C5, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][5]));

            OUTREG32(&m_pPrpReg->PRP_CSC_COEF_678,
                CSP_BITFVAL(PRP_CSC_COEF_678_C6, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][6]) |
                CSP_BITFVAL(PRP_CSC_COEF_678_C7, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][7]) |
                CSP_BITFVAL(PRP_CSC_COEF_678_C8, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][8]) |
                CSP_BITFVAL(PRP_CSC_COEF_678_X0, 
                    rgb2yuv_tbl[pConfigData->CSCEquation][9]));

            break;

        // YUV to RGB
        case prpCSCY2R_A1:
        case prpCSCY2R_A0:          
        case prpCSCY2R_B1:
        case prpCSCY2R_B0:          
            if (bInputRGB) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: CSC setting is not consistent with input image format! \r\n"), 
                    __WFUNCTION__));
               return FALSE;
            }

            OUTREG32(&m_pPrpReg->PRP_CSC_COEF_012,
                CSP_BITFVAL(PRP_CSC_COEF_012_C0, 
                    yuv2rgb_tbl[pConfigData->CSCEquation - 4][0]) |
                CSP_BITFVAL(PRP_CSC_COEF_012_C1, 
                    yuv2rgb_tbl[pConfigData->CSCEquation - 4][1]) |
                CSP_BITFVAL(PRP_CSC_COEF_012_C2, 
                    yuv2rgb_tbl[pConfigData->CSCEquation - 4][2]));

            OUTREG32(&m_pPrpReg->PRP_CSC_COEF_345,
                CSP_BITFVAL(PRP_CSC_COEF_345_C3, 
                    yuv2rgb_tbl[pConfigData->CSCEquation - 4][3]) |
                CSP_BITFVAL(PRP_CSC_COEF_345_C4, 
                    yuv2rgb_tbl[pConfigData->CSCEquation - 4][4]) |
                CSP_BITFVAL(PRP_CSC_COEF_345_C5, 0));

            OUTREG32(&m_pPrpReg->PRP_CSC_COEF_678,
                CSP_BITFVAL(PRP_CSC_COEF_678_C6, 0) |
                CSP_BITFVAL(PRP_CSC_COEF_678_C7, 0) |
                CSP_BITFVAL(PRP_CSC_COEF_678_C8, 0) |
                CSP_BITFVAL(PRP_CSC_COEF_678_X0, 
                    yuv2rgb_tbl[pConfigData->CSCEquation - 4][5]));

            break;
        
        default:
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Invalid CSC equation. \r\n"), 
                __WFUNCTION__));
            return FALSE;
    }
    
    PRP_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpConfigureResize
//
// This function configures the pre-processor resizing.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureResize(pPrpConfigData pConfigData)
{
    PRP_FUNCTION_ENTRY();

    // Pre-processor resizer only support downsaling
    if (m_bEnableVfOutput) {
        if (!VALID_RESIZE_DIRECTION(pConfigData->inputSize, 
            pConfigData->outputVfSize)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Pre-processor can not support upscaling on viewfinding channel! \r\n"), 
                __WFUNCTION__));
            return FALSE;
        }
    }

    if (m_bEnableEncOutput) {
        if (!VALID_RESIZE_DIRECTION(pConfigData->inputSize, 
            pConfigData->outputEncSize)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Pre-processor can not support upscaling on encoding channel! \r\n"), 
                __WFUNCTION__));
            return FALSE;
        }
    }
    
    // Calculate resize    
    if (m_bEnableVfOutput) {
        //
        // Viewfinding channel is enabled
        //
        if (VALID_RESIZE_RATIO(pConfigData->inputSize, 
            pConfigData->outputVfSize)) {
            //
            // Cascade resize is not needed
            //
            if (m_bEnableEncOutput) {
                //
                // And encoding channel is enabled
                //
                if (VALID_RESIZE_RATIO(pConfigData->inputSize, 
                    pConfigData->outputEncSize)) {
                    //
                    // Encoding channel horizontal resize
                    //
                    if (PrpResize(pConfigData->inputSize.width, 
                        pConfigData->outputEncSize.width, 
                        prpResize_Encoding, prpResize_Horizontal) == FALSE) {
                        DEBUGMSG(ZONE_ERROR, 
                            (TEXT("%s: Encoding channel horizontal resize failed! \r\n"), 
                            __WFUNCTION__));
                        return FALSE;
                    }
                    //
                    // Encoding channel vertical resize
                    //
                    if (PrpResize(pConfigData->inputSize.height, 
                        pConfigData->outputEncSize.height, 
                        prpResize_Encoding, prpResize_Vertical) == FALSE) {
                        DEBUGMSG(ZONE_ERROR, 
                            (TEXT("%s: Encoding channel vertical resize failed! \r\n"), 
                            __WFUNCTION__));
                        return FALSE;
                    }
                    //
                    // Viewfinding channel horizontal resize
                    //
                    if (PrpResize(pConfigData->inputSize.width, 
                        pConfigData->outputVfSize.width, 
                        prpResize_Viewfinding, prpResize_Horizontal) == FALSE) {
                        DEBUGMSG(ZONE_ERROR, 
                            (TEXT("%s: Viewfinding channel horizontal resize failed! \r\n"), 
                            __WFUNCTION__));
                        return FALSE;
                    }
                    //
                    // Viewfinding channel vertical resize
                    //
                    if (PrpResize(pConfigData->inputSize.height, 
                        pConfigData->outputVfSize.height, 
                        prpResize_Viewfinding, prpResize_Vertical) == FALSE) {
                        DEBUGMSG(ZONE_ERROR, 
                            (TEXT("%s: Viewfinding channel vertical resize failed! \r\n"), 
                            __WFUNCTION__));
                        return FALSE;
                    }

                    // Disable cascade resize
                    INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                        PRP_CNTL_CH1BYP, PRP_CNTL_CH1BYP_DISCASCADE);
                } else {
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Encoding channel resize ratio is invalid! \r\n"), 
                        __WFUNCTION__));
                    return FALSE;
                }
            } else { // Only viewfinding channel is enabled
                //
                // Viewfinding channel horizontal resize
                //
                if (PrpResize(pConfigData->inputSize.width, 
                    pConfigData->outputVfSize.width, 
                    prpResize_Viewfinding, prpResize_Horizontal) == FALSE) {
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Viewfinding channel (only) horizontal resize failed! \r\n"), 
                        __WFUNCTION__));
                    return FALSE;
                }
                //
                // Viewfinding channel vertical resize
                //
                if (PrpResize(pConfigData->inputSize.height, 
                    pConfigData->outputVfSize.height, 
                    prpResize_Viewfinding, prpResize_Vertical) == FALSE) {
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Viewfinding channel (only) vertical resize failed! \r\n")));
                    return FALSE;
                }

                // Disable cascade resize
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH1BYP, PRP_CNTL_CH1BYP_DISCASCADE);
            }
        } else {
            //
            // Cascade resize is needed
            //
            if (m_bEnableEncOutput) { // Encoding channel is enabled
                // Viewfinding size must be less than encoding
                if ((pConfigData->outputVfSize.width >= pConfigData->outputEncSize.width) || 
                    (pConfigData->outputVfSize.height >= pConfigData->outputEncSize.height)) {
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Viewfinding size is bigger than encoding! \r\n"), 
                        __WFUNCTION__));
                    return FALSE;
                }

                if (VALID_RESIZE_RATIO(pConfigData->inputSize, 
                    pConfigData->outputEncSize)) {
                    if (VALID_RESIZE_RATIO(pConfigData->outputEncSize, 
                        pConfigData->outputVfSize)) {
                        //
                        // Encoding channel horizontal resize
                        //
                        if (PrpResize(pConfigData->inputSize.width, 
                            pConfigData->outputEncSize.width, 
                            prpResize_Encoding, prpResize_Horizontal) == FALSE) {
                            DEBUGMSG(ZONE_ERROR, 
                                (TEXT("%s: Encoding channel (cascade) horizontal resize failed! \r\n"), 
                                __WFUNCTION__));
                            return FALSE;
                        }
                        //
                        // Encoding channel vertical resize
                        //
                        if (PrpResize(pConfigData->inputSize.height, 
                            pConfigData->outputEncSize.height, 
                            prpResize_Encoding, prpResize_Vertical) == FALSE) {
                            DEBUGMSG(ZONE_ERROR, 
                                (TEXT("%s: Encoding channel (cascade) vertical resize failed! \r\n"), 
                                __WFUNCTION__));
                            return FALSE;
                        }
                        //
                        // Viewfinding channel horizontal resize
                        //
                        if (PrpResize(pConfigData->outputEncSize.width, 
                            pConfigData->outputVfSize.width, 
                            prpResize_Viewfinding, prpResize_Horizontal) == FALSE) {
                            DEBUGMSG(ZONE_ERROR, 
                                (TEXT("%s: Viewfinding channel (cascade) horizontal resize failed! \r\n"), 
                                __WFUNCTION__));
                            return FALSE;
                        }
                        //
                        // Viewfinding channel vertical resize
                        //
                        if (PrpResize(pConfigData->outputEncSize.height, 
                            pConfigData->outputVfSize.height, 
                            prpResize_Viewfinding, prpResize_Vertical) == FALSE) {
                            DEBUGMSG(ZONE_ERROR, 
                                (TEXT("%s: Viewfinding channel (cascade) vertical resize failed! \r\n"), 
                                __WFUNCTION__));
                            return FALSE;
                        }

                        // Enable cascade resize
                        INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                            PRP_CNTL_CH1BYP, PRP_CNTL_CH1BYP_CASCADE);
                    } else {
                        DEBUGMSG(ZONE_ERROR, 
                            (TEXT("%s: Encoding channel to viewfinding channel resize ratio is invalid! \r\n"), 
                            __WFUNCTION__));
                        return FALSE;
                    }
                } else {
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Encoding channel (cascode) resize ratio is invalid! \r\n"), 
                        __WFUNCTION__));
                    return FALSE;
                }
            } else {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Encoding channel is not enabled for the cascode resize! \r\n"), 
                    __WFUNCTION__));
                return FALSE;
            }
        }
    } else { // Viewfinding channel is not enabled
        if (m_bEnableEncOutput) {
            //
            // Only encoding channel enabled
            //
            if (VALID_RESIZE_RATIO(pConfigData->inputSize, 
                pConfigData->outputEncSize)) {
                //
                // Encoding channel horizontal resize
                //
                if (PrpResize(pConfigData->inputSize.width, 
                    pConfigData->outputEncSize.width, 
                    prpResize_Encoding, prpResize_Horizontal) == FALSE) {
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Encoding channel (only) horizontal resize failed! \r\n"), 
                        __WFUNCTION__));
                    return FALSE;
                }
                //
                // Encoding channel vertical resize
                //
                if (PrpResize(pConfigData->inputSize.height, 
                    pConfigData->outputEncSize.height, 
                    prpResize_Encoding, prpResize_Vertical) == FALSE) {
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Encoding channel (only) vertical resize failed! \r\n"), 
                        __WFUNCTION__));
                    return FALSE;
                }

                // Just make sure cascade resize is not being used
                INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                    PRP_CNTL_CH1BYP, PRP_CNTL_CH1BYP_DISCASCADE);

            } else {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Encoding channel (only) resize ratio is invalid! \r\n"), 
                    __WFUNCTION__));
                return FALSE;
            }
        } else {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Both viewfinding and encoding channels are disabled! \r\n"), 
                __WFUNCTION__));
            return FALSE;
        }
    }

    PRP_FUNCTION_EXIT();
    
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrpResize
//
// This function is used to build resizing table.  Resize registers are setup
// for the given group.
//
// Parameters:  
//      in
//          [in] Width/height of resizing input.
//
//      out
//          [in] Width/height of resizing output.
//
//      channel
//          [in] Viewfinding or encoding channel.
//
//      dimension
//          [in] Horizontal or vertical.
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpResize(UINT32 in, UINT32 out, prpResizeChannel channel, 
    prpResizeDimension dimension)
{
    pPrpResizeParams pParam = NULL;
    UINT8 retry = 0;
    BOOL bRet = TRUE;

    pParam = (pPrpResizeParams) malloc(sizeof(prpResizeParams));
    if (pParam == NULL) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Resize parameter object allocation failed! \r\n"), 
            __WFUNCTION__));
        bRet = FALSE;
        goto _exit;
    }

    memset(pParam, 0, sizeof(prpResizeParams));

    if (in == out) {
        bRet = PrpGetCoeff(pParam, 1, 1);
        if (bRet == FALSE) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Resizing failed due to same input and output size. \r\n"),
                __WFUNCTION__));
            goto _exit;
        }
    } else {
        UINT tmpOut = out;
        while (retry <= MAX_RESIZE_RETRIES) {
            UINT16 gcd = PrpGetGcd(in, tmpOut);
            bRet = PrpGetCoeff(pParam, in/gcd, tmpOut/gcd);
            if (bRet == TRUE) {
                // Get it!! Quit loop
                pParam->in = in / gcd;
                pParam->out = tmpOut / gcd;

                DEBUGMSG(ZONE_FUNCTION, 
                    (TEXT("%s: Resize actual size, in (%d), out (%d) \r\n"), 
                    __WFUNCTION__, pParam->in, pParam->out));
                break;
            } else {
                retry++;
                tmpOut++;
            }
        }
    }

    if (retry > MAX_RESIZE_RETRIES) {
        // In this case, PrpGetCoeff() failed.
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Pre-processor gets coefficient failed! \r\n"), 
            __WFUNCTION__));
        goto _exit;  
    }

    PrpPackToRegister(pParam, channel, dimension);

_exit:
    if (pParam) {
        free(pParam);
        pParam = NULL;
    }

    return bRet;
}

//------------------------------------------------------------------------------
//
// Function: PrpGetGcd
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
UINT16 PrpClass::PrpGetGcd(UINT16 x, UINT16 y)
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
// Function: PrpGetCoeff
//
// This function is used to calculate the resize coefficients.
//
// Parameters:
//      pParam
//          [in] Pre-processing resize parameters.
//
//      in
//          [in] input.
//
//      out
//          [in] output.
//
// Returns:     
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpGetCoeff(pPrpResizeParams pParam, UINT16 in, UINT16 out)
{
    if (in > MAX_RESIZE_COEFFS) {
        // We support only 20 coefficients space
        DEBUGMSG(ZONE_FUNCTION, 
            (TEXT("%s: No enough coefficient space! \r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    if (in == out) {
        //
        // Averaging interpolation
        //
        pParam->tbl_len = 1;
        pParam->algo = prpResize_Averaging;
        pParam->resize_tbl[0] = MAX_COEFF_VALUE - 1;
        pParam->output_bits = 1;
    } else if (in < PRP_RESIZE_THRESHOLD * out) {
        //
        // Bilinear interpolation
        //
        pParam->tbl_len = in;
        pParam->algo = prpResize_Bilinear;

        // For downscaling from 1:1 to 2:1
        UINT16 inPos1 = out;
        UINT16 inPosInc = out << 1;
        UINT16 inPos2 = inPos1 + inPosInc;
        UINT16 outPos = in;
        UINT16 outPosInc = in << 1;
        UINT16 init_carry = in - out;
        UINT16 carry = init_carry;
        UINT8 index = 0;

        do {
            int w1 = inPos2 - outPos;
            int n = 0;

            outPos += outPosInc;
            carry += outPosInc;

            PrpStoreCoeff(pParam->resize_tbl, &pParam->output_bits, 
                index, (out<<1), w1, 1); // (w1,1)
            
            index++;

            while (inPos2 < outPos) {
                inPos1 = inPos2;    
                inPos2 += inPosInc;
                n++;
                carry -= inPosInc;

                if (n > 1) {
                    PrpStoreCoeff(pParam->resize_tbl, &pParam->output_bits, 
                        index, (out<<1), 0, 0); //(X,0)
                    index++;
                }
            }
        } while (carry != init_carry);
    } else {
        //
        // Average algo for all the other downscaling > 2:1
        //
        pParam->tbl_len = in;
        pParam->algo = prpResize_Averaging;

        // Start algo
        UINT8 piece_len = in/out;
        UINT8 num_pieces_plus_1 = in % out;
        UINT8 num_pieces_plus_0 = out - num_pieces_plus_1;
        UINT8 cur_index = 0;

        for (; num_pieces_plus_1 > 0; num_pieces_plus_1--) {
            PrpStoreCoeff(pParam->resize_tbl, &pParam->output_bits, 
                cur_index, piece_len+1);
            cur_index += (piece_len+1);
        }

        for (; num_pieces_plus_0 > 0; num_pieces_plus_0--) {
            PrpStoreCoeff(pParam->resize_tbl, &pParam->output_bits, 
                cur_index, piece_len);
            cur_index += (piece_len);
        }
    } 

    return TRUE;    
}

//------------------------------------------------------------------------------
//
// Function: PrpStoreCoeff
//
// This function is to store the coefficients for bilinear resizing.
//
// Parameters:
//      resize_tbl
//          [out] Resizing table.
//
//      output_bits
//          [out] Number of bits to output.
//
//      index
//          [in] Index in the table.
//
//      base
//          [in] UINT8.
//
//      w1
//          [in] UINT8.
//
//      output
//          [in] UINT8.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PrpClass::PrpStoreCoeff(UINT8 *resize_tbl, UINT32 *output_bits, 
    UINT8 index, UINT8 base, UINT8 w1, UINT8 output)
{
    // w1  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
    // reg | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 6 | 7 |
    
    w1 = ((w1 << RESIZE_COEFF_WIDTH) + (base >> 1)) / base;

    if (w1 >= MAX_COEFF_VALUE - 1) {
        // Coeff 7 is regarded as 8.
        w1--;
    } else if (w1 == 1) {
        // Because 7 (8-1) is regarded as 8 (8-0),
        // coeff 1 is regarded as 0.
        w1--;
    }

    resize_tbl[index] = w1;
    
    if (output == 1) 
        *output_bits |= (1UL << index);
    else
        *output_bits &= ~(1UL << index);
}

//------------------------------------------------------------------------------
//
// Function: PrpStoreCoeff
//
// This function is to store the coefficients for averaging resize.
//
// Parameters:
//      resize_tbl
//          [out] Resizing table.
//
//      output_bits
//          [out] Number of bits to output.
//
//      startindex
//          [in] The start index in resizing table.
//
//      len
//          [in] Length.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PrpClass::PrpStoreCoeff(UINT8 *resize_tbl, UINT32 *output_bits, 
    UINT8 startindex, UINT8 len)
{
    PrpFillAveragingCoeff(&resize_tbl[startindex], len);

    // Don't output for 1st len-1 bits
    *output_bits &= ~( (0xFFFFFFFFUL >> (32-len)) << startindex ); 
    // Output for len bit
    *output_bits |= 1UL << (startindex+len-1);
}

//------------------------------------------------------------------------------
//
// Function: PrpFillAveragingCoeff
//
// This function is to use Gaussian filter coefficients for averaging resize.
//
// Parameters:
//      startptr
//          [in] The start pointer.
//
//      filter_width
//          [in] The filter width.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PrpClass::PrpFillAveragingCoeff(UINT8 *startptr, UINT8 filter_width)
{
    // This routine contains hardcoded convolution filter coeffs based on
    // PrP's constraint that sum of each filter can only be max 8.
    static const UINT8 filter_coeff_tbl[][9] = {
        {8},
        {4,4},
        {2,4,2},
        {2,2,2,2},
        {1,2,2,2,1},
        {1,1,2,2,1,1},
        {1,1,1,2,1,1,1},
        {1,1,1,1,1,1,1,1},  // similar to 6
        {1,1,1,1,1,1,1,1,0} // similar to 7
    };

    if (filter_width >= 10) {
        // Use equal distribution if filter width is 10 or more
        int zeros = (filter_width - 8) / 2;
        int i = zeros + (filter_width & 1); // if odd then add one

        for(; i--; *(startptr++) = 0);
        for(i=8; i--; *(startptr++) = 1);
        for(i=zeros; i--; *(startptr++) = 0);
    } else {
        // Copy coeff from table
        memcpy(startptr, filter_coeff_tbl[filter_width-1], filter_width);
    }
}

//------------------------------------------------------------------------------
//
// Function: PrpPackToRegister
//
// This function is to load the calculated resize coeffients into resize 
// registers.  It is tightly dependent on hardware and hence not portable.
//
// Parameters:
//      pParams
//          [in] Resize parameters.
//
//      channel
//          [in] Channel selection.
//
//      dimension
//          [in] Dimension selection.
//
// Returns:     
//
//------------------------------------------------------------------------------
void PrpClass::PrpPackToRegister(pPrpResizeParams pParams, 
    prpResizeChannel channel, prpResizeDimension dimension)
{
    UINT16 len = pParams->tbl_len;
    UINT8 *tblptr = pParams->resize_tbl + len - 1; // Set pointer to last valid elem
    UINT32 rz_coef2 = 0;
    UINT32 rz_coef1 = 0;

    // Pack resize coef
    switch (len) {
        // rz_coef2
        case 20: rz_coef2 |= *(tblptr--) & 0x7; rz_coef2 <<= 3;
        case 19: rz_coef2 |= *(tblptr--) & 0x7; rz_coef2 <<= 3;
        case 18: rz_coef2 |= *(tblptr--) & 0x7; rz_coef2 <<= 3;
        case 17: rz_coef2 |= *(tblptr--) & 0x7; rz_coef2 <<= 3;
        case 16: rz_coef2 |= *(tblptr--) & 0x7; rz_coef2 <<= 4;

        case 15: rz_coef2 |= *(tblptr--) & 0x7; rz_coef2 <<= 3;
        case 14: rz_coef2 |= *(tblptr--) & 0x7; rz_coef2 <<= 3;
        case 13: rz_coef2 |= *(tblptr--) & 0x7; rz_coef2 <<= 3;
        case 12: rz_coef2 |= *(tblptr--) & 0x7; rz_coef2 <<= 3;
        case 11: rz_coef2 |= *(tblptr--) & 0x7;

        // rz_coef1
        case 10: rz_coef1 |= *(tblptr--) & 0x7; rz_coef1 <<= 3;
        case  9: rz_coef1 |= *(tblptr--) & 0x7; rz_coef1 <<= 3;
        case  8: rz_coef1 |= *(tblptr--) & 0x7; rz_coef1 <<= 3;
        case  7: rz_coef1 |= *(tblptr--) & 0x7; rz_coef1 <<= 3;
        case  6: rz_coef1 |= *(tblptr--) & 0x7; rz_coef1 <<= 4;

        case  5: rz_coef1 |= *(tblptr--) & 0x7; rz_coef1 <<= 3;
        case  4: rz_coef1 |= *(tblptr--) & 0x7; rz_coef1 <<= 3;
        case  3: rz_coef1 |= *(tblptr--) & 0x7; rz_coef1 <<= 3;
        case  2: rz_coef1 |= *(tblptr--) & 0x7; rz_coef1 <<= 3;
        case  1: rz_coef1 |= *(tblptr--) & 0x7; 
            break;

        default: 
            break;
    }

    // Setup resize registers
    OUTREG32(&m_pPrpReg->PRP_RESIZE[channel*2 + dimension].PRP_RZ_COEF1, rz_coef1);
    OUTREG32(&m_pPrpReg->PRP_RESIZE[channel*2 + dimension].PRP_RZ_COEF2, rz_coef2);
    OUTREG32(&m_pPrpReg->PRP_RESIZE[channel*2 + dimension].PRP_RZ_VALID, 
        CSP_BITFVAL(PRP_RZ_VALID_OV, pParams->output_bits) |
        CSP_BITFVAL(PRP_RZ_VALID_TBL_LEN, len) |
        CSP_BITFVAL(PRP_RZ_VALID_AVG_BIL, pParams->algo));
}

#ifndef PRP_MEM_MODE_DLL
//------------------------------------------------------------------------------
//
// Function: PrpAllocateVfBuffers
//
// This function allocates physically contiguous buffers for the 
// pre-processor viewfinding channel.
//
// Parameters:
//      numBuffers
//          [in] Number of buffers to allocate.
//
//      bufSize
//          [in] Size of buffer to create.
//
// Returns:
//      Returns virtual pointer to buffer created.  
//      Returns NULL if unsuccessful.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpAllocateVfBuffers(ULONG numBuffers, ULONG bufSize)
{
    PRP_FUNCTION_ENTRY();

    if (numBuffers < PRP_MIN_NUM_BUFFERS) {
        // Ignore the number from upper layer
        numBuffers = PRP_MIN_NUM_BUFFERS;
    }

    if (!pVfBufferManager->AllocateBuffers(numBuffers, bufSize)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Viewfinding buffer allocation failed! \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }
    
    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrpGetVfBufFilled
//
// This function get the virtual/physics address of the filled viewfinding 
// channel buffer.
//
// Parameters:
//       pPrpBufferData
//           [out]  The virtual/physics address of the filled viewfinding channel buffer.
//
// Returns:
//      TRUE for success, FALSE for failure.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpGetVfBufFilled(pPrpBufferData pBufData)
{
    return pVfBufferManager->GetBufferFilled(pBufData);
}

//------------------------------------------------------------------------------
//
// Function: PrpPutVfBufIdle
//
// This function put the used buffer to Idle Queue.
//
// Parameters:
//       pPrpBufferData
//           [in]  The virtual/physics address of the filled viewfinding channel buffer.
//
// Returns:
//      TRUE for success,FALSE for failure.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpPutVfBufIdle(pPrpBufferData pBufData)
{
    return  pVfBufferManager->PutBufferIdle(pBufData);
}

//------------------------------------------------------------------------------
//
// Function:  PrpDeleteVfBuffers
//
// This function deletes the two viewfinding buffers.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpDeleteVfBuffers()
{
    PRP_FUNCTION_ENTRY();

    if (!pVfBufferManager->DeleteBuffers()) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Failed to delete viewfinding buffers! \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrpAllocateEncBuffers
//
// This function allocates two physically contiguous buffers for the 
// pre-processor encoding channel.
//
// Parameters:
//      numBuffers
//          [in] Number of buffers to allocate.
//
//      bufSize
//          [in] Size of buffer to create.
//
// Returns:
//      Returns virtual pointer to buffer created.  
//      Returns NULL if unsuccessful.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpAllocateEncBuffers(ULONG numBuffers, ULONG bufSize)
{
    PRP_FUNCTION_ENTRY();

    if (numBuffers < PRP_MIN_NUM_BUFFERS) {
        // Ignore the number from upper layer
        numBuffers = PRP_MIN_NUM_BUFFERS;
    }

    if (!pEncBufferManager->AllocateBuffers(numBuffers, bufSize)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Encoding buffer allocation failed! \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function:  PrpGetEncBufFilled
//
// This function get the virtual/physics address of the filled capture 
// channel buffers.
//
// Parameters:
//       pPrpBufferData
//           [out]  The virtual/physics address of the filled encoder channel buffer.
//
// Returns:
//      TRUE for success, FALSE for failure.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpGetEncBufFilled(pPrpBufferData pBufData)
{    
    return pEncBufferManager->GetBufferFilled(pBufData);
}

//------------------------------------------------------------------------------
//
// Function: PrpPutEncBufIdle
//
// This function write the used buffer to Idle queue.
//
// Parameters:
//       pPrpBufferData
//           [in]  The virtual/physics address of the filled encoder channel buffer.
//
// Returns:
//      TRUE for success,FALSE for failure.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpPutEncBufIdle(pPrpBufferData pBufData)
{
    return  pEncBufferManager->PutBufferIdle(pBufData);
}

//------------------------------------------------------------------------------
//
// Function:  PrpDeleteEncBuffers
//
// This function deletes the two viewfinding buffers.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpDeleteEncBuffers()
{
    PRP_FUNCTION_ENTRY();

    if (!pEncBufferManager->DeleteBuffers()) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Failed to delete encoding buffers! \r\n"), 
            __WFUNCTION__));
        return FALSE;
    }

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpGetMaxBuffers
//
// This function returns the maximum number of buffers supported
// by the preprocessor.
//
// Parameters:
//      None.
//
// Returns:
//      Returns the Max buffer number.
//
//-----------------------------------------------------------------------------
UINT32 PrpClass::PrpGetMaxBuffers(void)
{
    return PRP_MAX_NUM_BUFFERS;
}

//------------------------------------------------------------------------------
//
// Function: PrpVfGetBufferReady
//
// This function assures that the buffers are managed correctly 
// in the preprocessor.  When a buffer is filled, this function will 
// wait for a new buffer to become available and set up the channel to run.
//
// Parameters:
//      request
//          [in] Indicates the buffer to make ready.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpVfGetBufferReady(prpBufferRequest request)
{
    LPVOID physAddr;
    DWORD dwFrameDropped;

    PRP_FUNCTION_ENTRY();

    // A buffer has been requested.  Attempt to read from the idle
    // queue to get an available buffer.  If one is not available,
    // we read from filled queue and report one frame dropped.
    dwFrameDropped = pVfBufferManager->SetActiveBuffer(&physAddr, INFINITE);
    if (dwFrameDropped == 1) {
        DEBUGMSG(ZONE_WARN,  
            (TEXT("%s: Viewfinding frame dropped. \r\n"), __WFUNCTION__));
    } else if (dwFrameDropped == -1) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Error, viewfinding did not receive a buffer. \r\n"), 
        __WFUNCTION__));
        return FALSE;
    }

    // We have a buffer now.  Determine which buffer we need 
    // to fill (Buf0 or Buf1)
    switch (request) {
        case prpBuf0Request:
            // Setup Buf0 pointer
            OUTREG32(&m_pPrpReg->PRP_DEST_RGB1_PTR, (UINT32)physAddr);
            break;

        case prpBuf1Request:
            // Setup Buf1 pointer (LOOP mode always enabled)
            OUTREG32(&m_pPrpReg->PRP_DEST_RGB2_PTR, (UINT32)physAddr);
            break;
                
        default:
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Invalid viewfinding buffer ready request! \r\n"), 
                __WFUNCTION__));
            return FALSE;
    }

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrpEncGetBufferReady
//
// This function assures that the buffers are managed correctly 
// in the preprocessor.  When a buffer is filled, this function will 
// wait for a new buffer to become available and set up the channel to run.
//
// Parameters:
//      request
//          [in] Indicates the buffer to make ready.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpEncGetBufferReady(prpBufferRequest request)
{
    LPVOID physAddr;
    DWORD dwFrameDropped;

    PRP_FUNCTION_ENTRY();

    // A buffer has been requested.  Attempt to read from the idle
    // queue to get an available buffer.  If one is not available,
    // we read from filled queue and report one frame dropped.
    dwFrameDropped = pEncBufferManager->SetActiveBuffer(&physAddr, INFINITE);
    if (dwFrameDropped == 1) {
        DEBUGMSG(ZONE_DEVICE, 
            (TEXT("%s: Encoding frame dropped. \r\n"), __WFUNCTION__));
    } else if (dwFrameDropped == -1) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Error, viewfinding did not receive a buffer. \r\n"), 
        __WFUNCTION__));
        return FALSE;
    }

    // We have a buffer now.  Determine which buffer we need 
    // to fill (Buf0 or Buf1)
    switch (request) {
        case prpBuf0Request:
            // Setup Buf0 pointers
            OUTREG32(&m_pPrpReg->PRP_DEST_Y_PTR, (UINT32)physAddr);
            // Setup Y V buffers for YUV420
            if (m_iOutputEncFormat == prpEncOutputFormat_YUV420) {
                OUTREG32(&m_pPrpReg->PRP_DEST_CB_PTR, 
                    PRP_YUV420_CB_PTR(physAddr, m_iEncOutputYSize));
                OUTREG32(&m_pPrpReg->PRP_DEST_CR_PTR, 
                    PRP_YUV420_CR_PTR(physAddr, m_iEncOutputYSize));
            }
            break;

        case prpBuf1Request:
            // Setup Buf1 pointers (LOOP mode always enabled)
            OUTREG32(&m_pPrpReg->PRP_SOURCE_Y_PTR, (UINT32)physAddr);
            // Setup Y V buffers for YUV420
            if (m_iOutputEncFormat == prpEncOutputFormat_YUV420) {
                OUTREG32(&m_pPrpReg->PRP_SOURCE_CB_PTR, 
                    PRP_YUV420_CB_PTR(physAddr, m_iEncOutputYSize));
                OUTREG32(&m_pPrpReg->PRP_SOURCE_CR_PTR, 
                    PRP_YUV420_CR_PTR(physAddr, m_iEncOutputYSize));
            }
            break;

        default:
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Invalid encoding buffer ready request! \r\n"), 
                __WFUNCTION__));
            return FALSE;
    }

    PRP_FUNCTION_EXIT();

    return TRUE;
}

#else // PRP_MEM_MODE_DLL

//-----------------------------------------------------------------------------
//
// Function: PrpAddBuffers
//
// This function adds buffers for the PrP channel.  Input and output buffers
// are provided.  It is assumed that the caller has allocated physically
// contiguous buffers.
//
// Parameters:
//      pPrpBufs
//          [in] Pointer to structure containing a pointer to an input and 
//          output buffer.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpAddBuffers(pPrpBuffers pPrpBufs)
{
    PRP_FUNCTION_ENTRY();

    //
    // Buffer check
    //
    if ((pPrpBufs->InBuf == NULL) || 
        (((UINT32)(pPrpBufs->InBuf) & 0x03) != 0)) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Invalid input buffer (0x%08lX).\r\n"), 
            __WFUNCTION__, pPrpBufs->InBuf));
        return FALSE;
    }

    if (m_bEnableVfOutput) {
        if ((pPrpBufs->OutVfBuf == NULL) || 
            ((UINT32)(pPrpBufs->OutVfBuf) & 0x03) != 0) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Invalid viewfinding output buffer (0x%08lX).\r\n"), 
                __WFUNCTION__, pPrpBufs->OutVfBuf));
            return FALSE;     
        }
    }
    
    if (m_bEnableEncOutput) {
        if ((pPrpBufs->OutEncBuf == NULL) || 
            ((UINT32)(pPrpBufs->OutEncBuf) & 0x03) != 0) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Invalid encoding output buffer (0x%08lX).\r\n"), 
                __WFUNCTION__, pPrpBufs->OutEncBuf));
            return FALSE;     
        }
    }
    
    //
    // Add the Pre-processing buffer
    //
    // Input buffer
    OUTREG32(&m_pPrpReg->PRP_SOURCE_Y_PTR, (UINT32)pPrpBufs->InBuf);
    if (m_iInputFormat == prpInputFormat_YUV420) {
        OUTREG32(&m_pPrpReg->PRP_SOURCE_CB_PTR, 
            PRP_YUV420_CB_PTR(pPrpBufs->InBuf, m_iMemInputYSize));
        OUTREG32(&m_pPrpReg->PRP_SOURCE_CR_PTR, 
            PRP_YUV420_CR_PTR(pPrpBufs->InBuf, m_iMemInputYSize));
    }

    // Viewfinding output buffer
    if (m_bEnableVfOutput)
        OUTREG32(&m_pPrpReg->PRP_DEST_RGB1_PTR, (UINT32)pPrpBufs->OutVfBuf);

    // Encoding output buffer
    if (m_bEnableEncOutput) {
        OUTREG32(&m_pPrpReg->PRP_DEST_Y_PTR, (UINT32)pPrpBufs->OutEncBuf);
        if (m_iOutputEncFormat == prpEncOutputFormat_YUV420) {
            OUTREG32(&m_pPrpReg->PRP_DEST_CB_PTR, 
               PRP_YUV420_CB_PTR(pPrpBufs->OutEncBuf, m_iEncOutputYSize));
            OUTREG32(&m_pPrpReg->PRP_DEST_CR_PTR, 
               PRP_YUV420_CR_PTR(pPrpBufs->OutEncBuf, m_iEncOutputYSize));
        }
    }

    // Start viewfinding
    if (m_bEnableVfOutput)
        INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CH1EN, PRP_CNTL_CH1EN_ENABLE);

    // Start encoding
    if (m_bEnableEncOutput)
        INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CH2EN, PRP_CNTL_CH2EN_ENABLE);

    PRP_FUNCTION_EXIT();
    
    return TRUE;
}
#endif // PRP_MEM_MODE_DLL

//------------------------------------------------------------------------------
//
// Function: PrpGetVfFrameCount
//
// This function returns the current viewfinding channel frame count.
//
// Parameters:
//      None.
//
// Returns:
//      The number of viewfinding frames processed since the
//      last configuration request.
//
//------------------------------------------------------------------------------
UINT32 PrpClass::PrpGetVfFrameCount()
{
    return m_iVfFrameCount;
}

//------------------------------------------------------------------------------
//
// Function: PrpGetEncFrameCount
//
// This function returns the current encoding channel frame count.
//
// Parameters:
//      None.
//
// Returns:
//      The number of encoding frames processed since the
//      last configuration request.
//
//------------------------------------------------------------------------------
UINT32 PrpClass::PrpGetEncFrameCount()
{
    return m_iEncFrameCount;
}

//------------------------------------------------------------------------------
//
// Function: PrpStartVfChannel
//
// This function starts the pre-processor viewfinding channel.
//
// Parameters:      
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpStartVfChannel()
{
    PRP_FUNCTION_ENTRY();

    // Configuration must have been done
    if (!m_bConfigured) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Can not start viewfinding channel without first configuring! \r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    // If viewfinding channel is enabled
    if (!m_bEnableVfOutput) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Can not start the disabled viewfinding channel! \r\n"),
            __WFUNCTION__));
        return FALSE;
    }

#ifndef PRP_MEM_MODE_DLL
    // State check
    if (m_iVfState == prpChannelStarted) {
        DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s: Viewfinding channel has already been started. \r\n"), 
            __WFUNCTION__));
        return TRUE;
    }

    // Make both Buf0 and Buf1 ready for viewfinding 
    // channel start working
    PrpVfGetBufferReady(prpBuf0Request);
    PrpVfGetBufferReady(prpBuf1Request);
    
    // Reset stuff for viewfinding channel stop request
    m_bVfStopReq = FALSE;
    ResetEvent(m_hVfStopEvent);

    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause Pin to believe a frame was ready.
    ResetEvent(m_hVfEOFEvent);

    m_iVfFrameCount = 0;

    // Start viewfinding channel
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CH1EN, PRP_CNTL_CH1EN_ENABLE);

    // Set viewfinding channel state as started
    m_iVfState = prpChannelStarted;
#else
    ResetEvent(m_hVfEOFEvent);
    m_iVfFrameCount = 0;
#endif

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrpStopVfChannel
//
// This function stops the pre-processor viewfinding channel.
//
// Parameters:      
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpStopVfChannel()
{    
    PRP_FUNCTION_ENTRY();

#ifndef PRP_MEM_MODE_DLL
    // State check
    if (m_iVfState == prpChannelStopped) {
        DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s: Viewfinding channel has already been stopped. \r\n"), 
            __WFUNCTION__));
        return TRUE;
    }

    // Request viewfinding channel stop
    m_bVfStopReq = TRUE;

    // Wait here 1s for PrpIntrThread() triggering
    if (WaitForSingleObject(m_hVfStopEvent, 1000) == WAIT_TIMEOUT) {
        // There must be something wrong
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error in stopping viewfinding channel! \r\n"), 
            __WFUNCTION__));
    }
    
    // Stop pre-processor viewfinding channel
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CH1EN, PRP_CNTL_CH1EN_DISABLE);

    // Reset the state of the viewfinding buffers, so that
    // everything will be in place if the channel is restarted.
    pVfBufferManager->ResetBuffers();

    // Set viewfinding channel status as stopped
    m_iVfState = prpChannelStopped;
#else
    // The channel should be already stopped after last frame finished
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CH1EN, PRP_CNTL_CH1EN_DISABLE);
#endif

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrpStartEncChannel
//
// This function starts the pre-processor encoding channel.
//
// Parameters:      
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpStartEncChannel()
{
    PRP_FUNCTION_ENTRY();
    
    // Configuration must have been done
    if (!m_bConfigured) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Can not start encoding channel without first configuring! \r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    // If encoding channel is enabled
    if (!m_bEnableEncOutput) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Can not start the disabled encoding channel! \r\n"),
            __WFUNCTION__));
        return FALSE;
    }

#ifndef PRP_MEM_MODE_DLL
    // State check
    if (m_iEncState == prpChannelStarted) {
        DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s: Encoding channel has already been started. \r\n"), 
            __WFUNCTION__));
        return TRUE;
    }

    // Make both Buf0 and Buf1 ready for encoding 
    // channel start working
    PrpEncGetBufferReady(prpBuf0Request);
    PrpEncGetBufferReady(prpBuf1Request);

    // Reset stuff for encoding channel stop request
    m_bEncStopReq = FALSE;
    ResetEvent(m_hEncStopEvent);

    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause Pin to believe a frame was ready.
    ResetEvent(m_hEncEOFEvent);

    m_iEncFrameCount = 0;
    
    // Start encoding channel
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CH2EN, PRP_CNTL_CH2EN_ENABLE);

    // Set encoding channel state as started
    m_iEncState = prpChannelStarted;
#else
    ResetEvent(m_hEncEOFEvent);
    m_iEncFrameCount = 0;
#endif

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrpStopEncChannel
//
// This function stops the pre-processor encoding channel.
//
// Parameters:      
//      None.
//
// Returns:
//      TRUE if successful; FALSE if failed.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpStopEncChannel()
{
    PRP_FUNCTION_ENTRY();

#ifndef PRP_MEM_MODE_DLL
    // Status check
    if (m_iEncState == prpChannelStopped) {
        DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s: Encoding channel has already been stopped. \r\n"), 
            __WFUNCTION__));
        return TRUE;
    }

    // Request encoding channel stop
    m_bEncStopReq = TRUE;

    // Wait here 1s for PrpIntrThread triggering
    if (WaitForSingleObject(m_hEncStopEvent, 1000) == WAIT_TIMEOUT) {
        // There must be something wrong
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error in stopping encoding channel! \r\n"), 
            __WFUNCTION__));
    }

    // Stop pre-processor encoding channel
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CH2EN, PRP_CNTL_CH2EN_DISABLE);

    // Reset the state of the viewfinding buffers, so that
    // everything will be in place if the channel is restarted.
    pEncBufferManager->ResetBuffers();

    // Set encoding channel state as stopped
    m_iEncState = prpChannelStopped;
#else
    // The channel should be already stopped after last frame finished
    INSREG32BF(&m_pPrpReg->PRP_CNTL, PRP_CNTL_CH2EN, PRP_CNTL_CH2EN_DISABLE);
#endif

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrpIntrThread
//
// This function is the IST thread of pre-processor.
//
// Parameters:
//      lpParameter
//          [in] The parameter from CreateThread().
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PrpClass::PrpIntrThread(LPVOID lpParameter)
{
    PrpClass *pPrp = (PrpClass *)lpParameter;

    PRP_FUNCTION_ENTRY();

    BSPSetPrpIntrThreadPriority();

    pPrp->PrpIntrRoutine();

    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PrpIntrRoutine
//
// This function is the interrupt handler for the pre-processor.  It waits for
// the End-Of-Frame (EOF) interrupt, and signals the EOF event waited by 
// the user of the pre-processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PrpClass::PrpIntrRoutine()
{    
    UINT32 iIntrStatus;

    PRP_FUNCTION_ENTRY();

    while (1) {
        // Wait pre-processor interrupt event
        WaitForSingleObject(m_hPrpIntrEvent, INFINITE);

        if ( TRUE == m_hPowerDown ) break;

        iIntrStatus = INREG32(&m_pPrpReg->PRP_INTRSTATUS);

        // Check pending interrupt type
        if (iIntrStatus & CSP_BITFMASK(PRP_INTRSTATUS_CH1B1CI)) {
            DEBUGMSG(ZONE_DEVICE, 
                (TEXT("%s: Pre-processor viewfinding channel Buf-0 completed! \r\n"), 
                __WFUNCTION__));

#ifndef PRP_MEM_MODE_DLL
            // Trigger channel stop event for PrpStopVfChannel()
            if (m_bVfStopReq)
                SetEvent(m_hVfStopEvent);

            // Trigger EOF for upper layer
            pVfBufferManager->SetFilledBuffer();
            SetEvent(m_hVfEOFEvent);

            m_iVfFrameCount++;

            // Request Buf0 ready
            PrpVfGetBufferReady(prpBuf0Request);
#else
            // Trigger EOF event to notify user
            SetEvent(m_hVfEOFEvent);
            m_iVfFrameCount++;
#endif
        }

#ifndef PRP_MEM_MODE_DLL
        if (iIntrStatus & CSP_BITFMASK(PRP_INTRSTATUS_CH1B2CI)) {
            DEBUGMSG(ZONE_DEVICE, 
                (TEXT("%s: Pre-processor viewfinding channel Buf-1 completed! \r\n"), 
                __WFUNCTION__));

            // Trigger channel stop event for PrpStopVfChannel()
            if (m_bVfStopReq)
                SetEvent(m_hVfStopEvent);

            // Trigger EOF for upper layer
            pVfBufferManager->SetFilledBuffer();
            SetEvent(m_hVfEOFEvent);

            m_iVfFrameCount++;

            // Request Buf1 ready
            PrpVfGetBufferReady(prpBuf1Request);
        }
#endif

        if (iIntrStatus & CSP_BITFMASK(PRP_INTRSTATUS_CH2B1CI)) {
            DEBUGMSG(ZONE_DEVICE, 
                (TEXT("%s: Pre-processor encoding channel Buf-0 completed! \r\n"), 
                __WFUNCTION__));

#ifndef PRP_MEM_MODE_DLL
            // Trigger channel stop event for PrpStopEncChannel()
            if (m_bEncStopReq)
                SetEvent(m_hEncStopEvent);

            // Trigger EOF for upper layer
            pEncBufferManager->SetFilledBuffer();
            SetEvent(m_hEncEOFEvent);

            m_iEncFrameCount++;

            // Request Buf0 ready and then resume it
            PrpEncGetBufferReady(prpBuf0Request);
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_CH2B1EN, PRP_CNTL_CH2B1EN_ENABLE);
#else
            // Trigger EOF event to notify user
            SetEvent(m_hEncEOFEvent);
            m_iEncFrameCount++;
#endif
        }

#ifndef PRP_MEM_MODE_DLL
        if (iIntrStatus & CSP_BITFMASK(PRP_INTRSTATUS_CH2B2CI)) {
            DEBUGMSG(ZONE_DEVICE, 
                (TEXT("%s: Pre-processor encoding channel Buf-1 completed! \r\n"), 
                __WFUNCTION__));

            // Trigger channel stop event for PrpStopEncChannel()
            if (m_bEncStopReq)
                SetEvent(m_hEncStopEvent);

            // Trigger EOF for upper layer
            pEncBufferManager->SetFilledBuffer();
            SetEvent(m_hEncEOFEvent);

            m_iEncFrameCount++;

            // Request Buf1 ready and then resume it
            PrpEncGetBufferReady(prpBuf1Request);
            INSREG32BF(&m_pPrpReg->PRP_CNTL, 
                PRP_CNTL_CH2B2EN, PRP_CNTL_CH2B2EN_ENABLE);
        }
#endif

        if (iIntrStatus & CSP_BITFMASK(PRP_INTRSTATUS_LBOVF))
            DEBUGMSG(ZONE_DEVICE, 
                (TEXT("%s: Pre-processor line buffer overflow! \r\n"), 
                __WFUNCTION__));

        if (iIntrStatus & CSP_BITFMASK(PRP_INTRSTATUS_CH2OVF))
            DEBUGMSG(ZONE_DEVICE, 
                (TEXT("%s: Pre-processor encoding channel frame dropped! \r\n"), 
                __WFUNCTION__));

        if (iIntrStatus & CSP_BITFMASK(PRP_INTRSTATUS_READERR))
            DEBUGMSG(ZONE_DEVICE, 
                (TEXT("%s: Pre-processor read error! \r\n"), 
                __WFUNCTION__));

        if (iIntrStatus & CSP_BITFMASK(PRP_INTRSTATUS_CH1WRERR))
            DEBUGMSG(ZONE_DEVICE, 
                (TEXT("%s: Pre-processor viewfinding channel write error! \r\n"), 
                __WFUNCTION__));

        if (iIntrStatus & CSP_BITFMASK(PRP_INTRSTATUS_CH2WRERR))
            DEBUGMSG(ZONE_DEVICE, 
                (TEXT("%s: Pre-processor encoding channel write error! \r\n"), 
                __WFUNCTION__));

        // Clear interrupt status (w1c)
        OUTREG32(&m_pPrpReg->PRP_INTRSTATUS, iIntrStatus);

        InterruptDone(m_iPrpSysintr);
    }

    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PrpDumpRegisters
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
void PrpClass::PrpDumpRegisters(void)
{
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: CNTL = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_CNTL));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: INTRCNTL = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_INTRCNTL));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: INTRSTATUS = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_INTRSTATUS));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: SOURCE_Y_PTR = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_SOURCE_Y_PTR));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: SOURCE_CB_PTR = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_SOURCE_CB_PTR));    
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: SOURCE_CR_PTR = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_SOURCE_CR_PTR));    
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: DEST_RGB1_PTR = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_DEST_RGB1_PTR));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: DEST_RGB2_PTR = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_DEST_RGB2_PTR));    
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: DEST_CR_PTR = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_DEST_Y_PTR));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: DEST_CB_PTR = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_DEST_CB_PTR));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: DEST_CR_PTR = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_DEST_CR_PTR));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: SOURCE_FRAME_SIZE = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_SOURCE_FRAME_SIZE));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: CH1_LINE_STRIDE = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_CH1_LINE_STRIDE));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: CH1_OUT_IMAGE_SIZE = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_CH1_OUT_IMAGE_SIZE));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: CH2_OUT_IMAGE_SIZE = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_CH2_OUT_IMAGE_SIZE));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: SOURCE_LINE_STRIDE = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_SOURCE_LINE_STRIDE));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: SOURCE_PIXEL_FORMAT_CNTL = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_SOURCE_PIXEL_FORMAT_CNTL));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: CH1_PIXEL_FORMAT_CNTL = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_CH1_PIXEL_FORMAT_CNTL));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: CSC_COEF_012 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_CSC_COEF_012));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: CSC_COEF_345 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_CSC_COEF_345));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: CSC_COEF_678 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_CSC_COEF_678));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH1_HCOEF1 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[0].PRP_RZ_COEF1));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH1_HCOEF2 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[0].PRP_RZ_COEF2));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH1_HVALID = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[0].PRP_RZ_VALID));   
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH1_VCOEF1 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[1].PRP_RZ_COEF1));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH1_VCOEF2 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[1].PRP_RZ_COEF2));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH1_VVALID = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[1].PRP_RZ_VALID));   
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH2_HCOEF1 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[2].PRP_RZ_COEF1));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH2_HCOEF2 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[2].PRP_RZ_COEF2));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH2_HVALID = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[2].PRP_RZ_VALID));   
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH2_VCOEF1 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[3].PRP_RZ_COEF1));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH2_VCOEF2 = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[3].PRP_RZ_COEF2));
    DEBUGMSG(ZONE_DEVICE, 
        (TEXT("%s: RESIZE_CH2_VVALID = 0x%08lX \r\n"), 
        __WFUNCTION__, m_pPrpReg->PRP_RESIZE[3].PRP_RZ_VALID));   
}


//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CameraPPClass.cpp
//
//  Implementation of CameraPP driver class
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4100 4115 4127 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"
#include "dp.h"
#include "idmac.h"
#include "cpmem.h"
#include "cm.h"
#include "cambuffermanager.h"
#include "tpm.h"
#include "pp.h"
#include "SMFC.h"
#include "SMFCClass.h"
#include "CameraPPClass.h"
#include "cameradbg.h"
#pragma warning(disable: 4100 4127 4189)

//------------------------------------------------------------------------------
// External Functions


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
// Function: CameraPPClass
//
// CameraPP class constructor.  Calls CameraPPInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
CameraPPClass::CameraPPClass(void)
{
    CameraPPInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~CameraPPClass
//
// The destructor for CameraPPClass.  Calls CameraPPDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
CameraPPClass::~CameraPPClass(void)
{
    CameraPPDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: CameraPPInit
//
// This function initializes the class of cameraPP.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::CameraPPInit()
{
    CAMERAPP_FUNCTION_ENTRY();
    
    InitializeCriticalSection(&m_csCameraPPStopping);

    m_bCameraPPRunning = FALSE;
    m_iCameraPPFrameCount = 0;

    QueryPerformanceFrequency(&m_lpFrequency);
    m_lpPerformanceCount_start.QuadPart = 0;
    m_lpPerformanceCount_end.QuadPart = 0; 

    m_pCameraPPBufferManager = NULL;
    m_hPP = NULL;

    for (UINT32 i = 0; i < NUM_PIN_BUFFER_MAX; i++)
    {
        m_pCameraPPInputBufPhyAddr[i] = NULL;
    }

    // Events to signal pin that frame is ready
    m_hCameraPPEOFEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hCameraPPEOFEvent == NULL)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for Frame EOF\r\n"), __WFUNCTION__));
        CAMERAPP_FUNCTION_EXIT();
        return FALSE;
    }

    // Events to signal that CameraPP input buffer ready
    for (UINT32 i = 0; i<NUM_PIN_BUFFER_MAX; i++)
    {
        m_hCameraPPInputBufEvent[i] = CreateEvent(NULL, FALSE, FALSE, SMFCBufEOFEvent[i]);
        if (m_hCameraPPInputBufEvent[i] == NULL)
        {
            ERRORMSG(TRUE,
                (TEXT("%s: CreateEvent for CamerPP input buffer %d failed\r\n"), __WFUNCTION__, i));
            CAMERAPP_FUNCTION_EXIT();
            return FALSE;
        }
    }
    
    // Events to Exit CameraPPISRThread
    m_hExitCameraPPWorkerThread = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;
    if(NULL == m_hExitCameraPPWorkerThread)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for Exit CameraPPISRThread\r\n"), __WFUNCTION__));
        CAMERAPP_FUNCTION_EXIT();
        return FALSE;
    }

    //      Initialize thread for Preprocessor ISR
    //      pThreadAttributes = NULL (must be NULL)
    //      dwStackSize = 0 => default stack size determined by linker
    //      lpStartAddress = CspiProcessQueue => thread entry point
    //      lpParameter = NULL => point to thread parameter
    //      dwCreationFlags = 0 => no flags
    //      lpThreadId = NULL => thread ID is not returned
    m_hCameraPPWorkerThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CameraPPWorkerThread, this, 0, NULL);
    if (m_hCameraPPWorkerThread == NULL)
    {
        ERRORMSG(TRUE,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        CAMERAPP_FUNCTION_EXIT();
        return FALSE;
    }
    else
    {
        CeSetThreadPriority(m_hCameraPPWorkerThread, 251);
        RETAILMSG(0, (TEXT("%s: create m_hCameraPP ISR thread success\r\n"), __WFUNCTION__));
    }

    CAMERAPP_FUNCTION_EXIT();
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CameraPPDeinit
//
// This function deinitializes the class of CameraPP.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CameraPPClass::CameraPPDeinit(void)
{
    CAMERAPP_FUNCTION_ENTRY();

    if (m_hCameraPPEOFEvent != NULL)
    {
        CloseHandle(m_hCameraPPEOFEvent);
        m_hCameraPPEOFEvent = NULL;
    }

    for (UINT32 i = 0; i<NUM_PIN_BUFFER_MAX; i++)
    {
        CloseHandle(m_hCameraPPInputBufEvent[i]);
        m_hCameraPPInputBufEvent[i] = NULL;
    }
    
    if (m_hCameraPPWorkerThread)
    {    
        // Set CameraPPISRThread end
        SetEvent ( m_hExitCameraPPWorkerThread ); 
        // Wait for m_hCameraPPWorkerThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hCameraPPWorkerThread,INFINITE))
            RETAILMSG(0,(TEXT("Wait for SMFCIntrThread end")));

        CloseHandle(m_hCameraPPWorkerThread);
        m_hCameraPPWorkerThread = NULL;
    }
    if (m_hExitCameraPPWorkerThread != NULL)
    {
        CloseHandle(m_hExitCameraPPWorkerThread);
        m_hExitCameraPPWorkerThread = NULL;
    }
    
    DeleteCriticalSection(&m_csCameraPPStopping); 
        
    CAMERAPP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: CameraPPOpenHandle
//
// This method creates a handle to the PP stream driver.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL CameraPPClass::CameraPPOpenHandle(void)
{
    CAMERAPP_FUNCTION_ENTRY();
    
    if (m_hPP == NULL)
    {
        m_hPP = PPOpenHandle();
        // Fail if no PP handle
        if (m_hPP == INVALID_HANDLE_VALUE)
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Opening PP handle failed!\r\n"), __WFUNCTION__));
            CAMERAPP_FUNCTION_EXIT();
            return FALSE;
        } 
    }

    CAMERAPP_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: CameraPPCloseHandle
//
// This method closes a handle to the PP stream driver.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL CameraPPClass::CameraPPCloseHandle(void)
{
    CAMERAPP_FUNCTION_ENTRY();

    if (NULL != m_hPP)
    {
        if(!PPCloseHandle(m_hPP))
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Closing PP handle failed!\r\n"), __WFUNCTION__));
            CAMERAPP_FUNCTION_EXIT();
            return FALSE;
        }
        m_hPP = NULL;
    }

    CAMERAPP_FUNCTION_EXIT();
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CameraPPSetBufferManager
//
// This function sets the current CameraPP buffer manager object pointer.
//
// Parameters:
//      pCameraPPBufferManager.
//          [in] The curremt Buffer Manager object pointer.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CameraPPClass::CameraPPSetBufferManager(CamBufferManager * pCameraPPBufferManager)
{                                                                 
    m_pCameraPPBufferManager = pCameraPPBufferManager;
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: m_pCameraPPBufferManager is %x\r\n"),
                                    __WFUNCTION__,m_pCameraPPBufferManager));
}


//-----------------------------------------------------------------------------
//
// Function: CameraPPGetFrameCount
//
// This function returns the current CameraPP channel frame count.
//
// Parameters:
//      None.
//
// Returns:
//      The number of the frames processed since the
//      last configuration request.
//
//-----------------------------------------------------------------------------
UINT32 CameraPPClass::CameraPPGetFrameCount()
{

    return m_iCameraPPFrameCount;

}


//-----------------------------------------------------------------------------
//
// Function:  CameraPPAllocateBuffers
//
// This function allocates buffers for the PP
// and adds each to the buffer queue.  This function must be called
// in order to allocate physically contiguous buffers for use in
// the IPU's IDMAC.
//
// Parameters:
//      pCameraPPBufferManager
//          [in] the current buffer manager pointer.
//
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
// for the issue: new buffer manager
BOOL CameraPPClass::CameraPPAllocateBuffers(CamBufferManager * pCameraPPBufferManager,ULONG numBuffers, ULONG bufSize)
{
    CAMERAPP_FUNCTION_ENTRY();
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: PP AllocateBuffers Begin, BufSize is %x\r\n"),
                                    __WFUNCTION__,bufSize));

    m_iCameraPPFrameCount = 0;

    // Initialize MsgQueues when buffers are allocated
    if (!pCameraPPBufferManager->AllocateBuffers(numBuffers, bufSize))
    {
        ERRORMSG(TRUE, 
            (TEXT("%s :PP Buffer allocation failed!\r\n"), __WFUNCTION__));
        CAMERAPP_FUNCTION_EXIT();
        return FALSE;
    }
    
    DEBUGMSG(ZONE_FUNCTION, 
            (TEXT("%s :PP Buffer allocation!\r\n"), __WFUNCTION__));
        
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: PP AllocateBuffers finished\r\n"),__WFUNCTION__));

    CAMERAPP_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  CameraPPGetBufFilled
//
// This function reads the queue of filled CameraPP channel
// buffers and returns the buffer at the top of the queue.
//
// Parameters:
//      None.
//
// Returns:
//      Pointer to filled buffer, or NULL if failure.
//
//-----------------------------------------------------------------------------
UINT32* CameraPPClass::CameraPPGetBufFilled()
{
    // Get filled buffer from the filled CameraPP buffer queue.
    return m_pCameraPPBufferManager->GetBufferFilled();

}

//------------------------------------------------------------------------------
//
// Function: CameraPPSetInputBufPhyAddr
//
// This method allows the caller to set input buffers to
// the PP buffer queues.
//
// Parameters:
//      numBuffers
//          [in] Number of Buffer Array.
//      pPysAddr[]
//          [in] Pointer to input Buffer physical address.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//------------------------------------------------------------------------------
BOOL CameraPPClass::CameraPPSetInputBufPhyAddr(ULONG numBuffers,UINT32* pPhyAddr[])
{
    for (ULONG i = 0; i < numBuffers; i++)
    {
        m_pCameraPPInputBufPhyAddr[i] = pPhyAddr[i]; 

        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:m_pCameraPPInputBufPhyAddr[%d] = %x \r\n"),
                                __WFUNCTION__,i,m_pCameraPPInputBufPhyAddr[i]));

        if (NULL == m_pCameraPPInputBufPhyAddr[i])
            return FALSE;
    }

    return TRUE;    
}

//-----------------------------------------------------------------------------
//
// Function:  CameraPPGetInputBufPhyAddr
//
// This function gets the input buffers physical address. 
//
// Parameters:
//      num.
//          [in] Number of Buffer Array.
//
// Returns:
//      Pointer to input buffer address, or NULL if failure.
//
//-----------------------------------------------------------------------------
UINT32* CameraPPClass::CameraPPGetInputBufPhyAddr(ULONG num)
{

    return m_pCameraPPInputBufPhyAddr[num];

}

//-----------------------------------------------------------------------------
//
// Function:  CameraPPDeleteBuffers
//
// This function deletes all of the CameraPP buffers in the Idle and Ready queues.
//
// Parameters:
//      pCameraPPBufferManager
//          [in] The curremt Buffer Manager object pointer.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
// for the issue: new buffer manager
BOOL CameraPPClass::CameraPPDeleteBuffers(CamBufferManager * pCameraPPBufferManager)
{
    CAMERAPP_FUNCTION_ENTRY();
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: PP DeleteBuffers Begin\r\n"),__WFUNCTION__));
    
    // Delete PP buffers.
    if (!pCameraPPBufferManager->DeleteBuffers())
    {
        ERRORMSG(TRUE, 
            (TEXT("%s : Failed to delete PP buffers!\r\n"), __WFUNCTION__));
        CAMERAPP_FUNCTION_EXIT();
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: PP DeleteBuffers finished\r\n"),__WFUNCTION__));
    CAMERAPP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CameraPPStartChannel
//
// This function starts IDMA channel that SMFC to PP used.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::CameraPPStartChannel(void)
{
    CAMERAPP_FUNCTION_ENTRY();
    DEBUGMSG(ZONE_FUNCTION,
                (TEXT("%s(): Starting CameraPP channel begin!\r\n"),__WFUNCTION__));
 
    // For PP in camera
    if (m_bCameraPPRunning)
    {
        RETAILMSG(0,
            (TEXT("%s: SMFC to PP channel already running.\r\n"), __WFUNCTION__));
        CAMERAPP_FUNCTION_EXIT();
        return TRUE;
    }

    m_bCameraPPRunning = TRUE;

    ResetEvent(m_hCameraPPEOFEvent);
    for (UINT32 i = 0; i<NUM_PIN_BUFFER_MAX; i++)
        ResetEvent(m_hCameraPPInputBufEvent[i]);
    
    //-----------------------------
    // Start PP
    //-----------------------------
    if (NULL != m_hPP)
    {
        if (!PPStart(m_hPP))
        {
            ERRORMSG(TRUE,(TEXT("%s: Start PP fail!\r\n"),__WFUNCTION__));
            CAMERAPP_FUNCTION_EXIT();
            return FALSE;  
        }  
    }
    else
    {
        RETAILMSG(1,(TEXT("%s: Can not get m_hPP handle\r\n"),__WFUNCTION__));
    }

    DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s(): Starting CameraPP channel End!\r\n"),__WFUNCTION__));
    CAMERAPP_FUNCTION_EXIT();
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CameraPPStopChannel
//
// This function stops IDMA channel that SMFC to PP used.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::CameraPPStopChannel(void)
{
    EnterCriticalSection(&m_csCameraPPStopping); 

    DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s(): Stopping CameraPP channel begin!\r\n"),__WFUNCTION__));

    if (NULL != m_hPP)
    {             
        if(!m_bCameraPPRunning)
        {
            DEBUGMSG(ZONE_FUNCTION,
                    (TEXT("%s(): Stopping CameraPP channel end - Do nothing!\r\n"),__WFUNCTION__));
            LeaveCriticalSection(&m_csCameraPPStopping);
            return TRUE;
        }       
        m_bCameraPPRunning = FALSE;    
    
        if(!PPStop(m_hPP))
        {
            ERRORMSG(TRUE, (TEXT("%s: Stop PP fail!\r\n"),__WFUNCTION__));
            LeaveCriticalSection(&m_csCameraPPStopping);
            return FALSE;  
        }  

        if (!PPClearBuffers(m_hPP))
        {
            ERRORMSG(TRUE, (TEXT("%s: PPClearBuffers fail!\r\n"),__WFUNCTION__));
            LeaveCriticalSection(&m_csCameraPPStopping);
            return FALSE;  
        } 
                
        ResetEvent(m_hCameraPPEOFEvent);
        for (UINT32 i = 0; i<NUM_PIN_BUFFER_MAX; i++)
            ResetEvent(m_hCameraPPInputBufEvent[i]);
        
        // Reset the state of the RotFlip buffers, so that
        // everything will be in place if the channel is restarted.
        m_pCameraPPBufferManager->ResetBuffers();
    }    
    else
    {
        RETAILMSG(1,(TEXT("%s: m_hPP handle is NULL\r\n"),__WFUNCTION__));
    }

    LeaveCriticalSection(&m_csCameraPPStopping);

    DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s(): Stopping CameraPP channel End!\r\n"),__WFUNCTION__));
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CameraPPConfigure
//
// This function configures the SMFC,IC registers and IDMAC
// channels for the post processor channel.
//
// Parameters:
//      pSMFCData
//          [in] Pointer to SMFC configuration data structure
//      pPPData
//          [in] Pointer to PP configuration data struture
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::CameraPPConfigure(pSMFCConfigData pSMFCData,pPpConfigData pPPData)
{    
    //-----------------------------
    // Configure PP
    //-----------------------------
    
    // Set up input format and data width
    if(!CameraPPInputConfigure(pSMFCData,&(pPPData->inputIDMAChannel)))
    {
        ERRORMSG(TRUE,(TEXT("%s:Config PP input data Fail!\r\n"),__WFUNCTION__));
        return FALSE;
    }
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CameraPPInputConfigure finish!\r\n"),__WFUNCTION__));

    // Set up output format and data width
    if(!CameraPPOutputConfigure(pPPData))
    {
        ERRORMSG(TRUE,(TEXT("%s:Config PP Output Data Fail!\r\n"),__WFUNCTION__));
        return FALSE;        
    }
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CameraPPOutputConfigure finish!\r\n"),__WFUNCTION__));

    // Set PP
    if (NULL != m_hPP)
    {
        if (!PPConfigure(m_hPP, pPPData))
        {
            ERRORMSG(TRUE,(TEXT("%s:Config PP Fail!\r\n"),__WFUNCTION__));
            return FALSE;   
        }
        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PPConfigure finish!\r\n"),__WFUNCTION__));
    }
    else
    {
        RETAILMSG(1,(TEXT("%s: Cannot config PP,m_hPP handle is NULL\r\n"),__WFUNCTION__));
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CameraPPInputConfigure
//
// This function configures the IDMAC -> IC
// channels for the post processor channel.
//
// Parameters:
//      pSMFCData
//          [in] Pointer to SMFC configuration data structure
//      PPPinput
//          [in] Pointer to PP input Idmachannel data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::CameraPPInputConfigure(pSMFCConfigData pSMFCData,pIdmaChannel pPPinput)
{   
    int iInputHeight,iInputWidth;
    
    // Set input size
    pPPinput->FrameSize.width = pSMFCData->outputSize.width; 
    pPPinput->FrameSize.height = pSMFCData->outputSize.height;

    iInputWidth = pPPinput->FrameSize.width;
    iInputHeight = pPPinput->FrameSize.height;

    // Set input pixel format
    pPPinput->PixelFormat.component0_offset = pSMFCData->outputPixelFormat.component0_offset;
    pPPinput->PixelFormat.component1_offset = pSMFCData->outputPixelFormat.component1_offset;
    pPPinput->PixelFormat.component2_offset = pSMFCData->outputPixelFormat.component2_offset;;    
    pPPinput->PixelFormat.component3_offset = pSMFCData->outputPixelFormat.component3_offset;;
    pPPinput->PixelFormat.component0_width = pSMFCData->outputPixelFormat.component0_width;
    pPPinput->PixelFormat.component1_width = pSMFCData->outputPixelFormat.component1_width;
    pPPinput->PixelFormat.component2_width = pSMFCData->outputPixelFormat.component2_width;
    pPPinput->PixelFormat.component3_width = pSMFCData->outputPixelFormat.component3_width;
                
    // Set up input format and data width
    switch(pSMFCData->outputFormat)
    {
        case SMFCFormat_RGB24:
            pPPinput->FrameFormat = icFormat_RGB;
            pPPinput->DataWidth = icDataWidth_24BPP;
            pPPinput->LineStride = pPPinput->FrameSize.width * 3;
            break;
                    
        case SMFCFormat_RGB565:               
            pPPinput->FrameFormat = icFormat_RGB;
            pPPinput->DataWidth = icDataWidth_16BPP;
            pPPinput->LineStride = pPPinput->FrameSize.width * 2;
                break;

        case SMFCFormat_UYVY422:
            pPPinput->FrameFormat = icFormat_UYVY422;
            pPPinput->DataWidth = icDataWidth_16BPP;
            pPPinput->LineStride = pPPinput->FrameSize.width * 2;
        break;

        case SMFCFormat_YUV420:
            pPPinput->FrameFormat = icFormat_YUV420;
            pPPinput->DataWidth = icDataWidth_8BPP;
            pPPinput->LineStride = pPPinput->FrameSize.width;

            pPPinput->UBufOffset = iInputHeight * iInputWidth + (iInputHeight * iInputWidth)/4;
            pPPinput->VBufOffset = iInputHeight * iInputWidth;
        break;

        case SMFCFormat_YUV420P:
            pPPinput->FrameFormat = icFormat_YUV420P;
            pPPinput->DataWidth = icDataWidth_8BPP;
            pPPinput->LineStride = pPPinput->FrameSize.width;

            pPPinput->VBufOffset = iInputHeight * iInputWidth;
            pPPinput->UBufOffset = pPPinput->VBufOffset;         
        break;
        
        default:
            ERRORMSG(TRUE,(TEXT("%s:PP Input Format Error!\r\n"),__WFUNCTION__));
            return FALSE;
        break;
    }
        
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CameraPPOutputConfigure
//
// This function configures the IC -> IDMAC
// channels for the post processor channel.
//
// Parameters:
//      pPPData
//          [in] Pointer to PP configuration data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::CameraPPOutputConfigure(pPpConfigData pPPData)
{
    int iOutputWidth,iOutputHeight;
    
    iOutputWidth = pPPData->outputIDMAChannel.FrameSize.width;
    iOutputHeight = pPPData->outputIDMAChannel.FrameSize.height;
    
    // Set up output format and data width    
    switch(pPPData->outputIDMAChannel.FrameFormat)
    {
        case icFormat_RGB:
            switch(pPPData->outputIDMAChannel.DataWidth)
            {
                case icDataWidth_16BPP:
                    pPPData->outputIDMAChannel.PixelFormat.component0_offset = RGB565_COMPONENT0_OFFSET;
                    pPPData->outputIDMAChannel.PixelFormat.component1_offset = RGB565_COMPONENT1_OFFSET;
                    pPPData->outputIDMAChannel.PixelFormat.component2_offset = RGB565_COMPONENT2_OFFSET;    
                    pPPData->outputIDMAChannel.PixelFormat.component3_offset = RGB565_COMPONENT3_OFFSET;
                    pPPData->outputIDMAChannel.PixelFormat.component0_width = RGB565_COMPONENT0_WIDTH -1;
                    pPPData->outputIDMAChannel.PixelFormat.component1_width = RGB565_COMPONENT1_WIDTH -1;
                    pPPData->outputIDMAChannel.PixelFormat.component2_width = RGB565_COMPONENT2_WIDTH -1;
                    pPPData->outputIDMAChannel.PixelFormat.component3_width = RGB565_COMPONENT3_WIDTH; 
                    
                    pPPData->outputIDMAChannel.LineStride = pPPData->outputIDMAChannel.FrameSize.width * 2;
                    break;

                 case icDataWidth_24BPP:
                    pPPData->outputIDMAChannel.PixelFormat.component0_offset = RGB888_COMPONENT0_OFFSET;
                    pPPData->outputIDMAChannel.PixelFormat.component1_offset = RGB888_COMPONENT1_OFFSET;
                    pPPData->outputIDMAChannel.PixelFormat.component2_offset = RGB888_COMPONENT2_OFFSET;    
                    pPPData->outputIDMAChannel.PixelFormat.component3_offset = RGB888_COMPONENT3_OFFSET;
                    pPPData->outputIDMAChannel.PixelFormat.component0_width = RGB888_COMPONENT0_WIDTH -1;
                    pPPData->outputIDMAChannel.PixelFormat.component1_width = RGB888_COMPONENT1_WIDTH -1;
                    pPPData->outputIDMAChannel.PixelFormat.component2_width = RGB888_COMPONENT2_WIDTH -1;
                    pPPData->outputIDMAChannel.PixelFormat.component3_width = RGB888_COMPONENT3_WIDTH; 
                    
                    pPPData->outputIDMAChannel.LineStride = pPPData->outputIDMAChannel.FrameSize.width * 3;
                    break;                    
            }
        break;

        case icFormat_UYVY422:
            pPPData->outputIDMAChannel.PixelFormat.component0_offset = UYVY_COMPONENT0_OFFSET;
            pPPData->outputIDMAChannel.PixelFormat.component1_offset = UYVY_COMPONENT1_OFFSET;
            pPPData->outputIDMAChannel.PixelFormat.component2_offset = UYVY_COMPONENT2_OFFSET;    
            pPPData->outputIDMAChannel.PixelFormat.component3_offset = UYVY_COMPONENT3_OFFSET;
            pPPData->outputIDMAChannel.PixelFormat.component0_width = UYVY_COMPONENT0_WIDTH -1;
            pPPData->outputIDMAChannel.PixelFormat.component1_width = UYVY_COMPONENT1_WIDTH -1;
            pPPData->outputIDMAChannel.PixelFormat.component2_width = UYVY_COMPONENT2_WIDTH -1;
            pPPData->outputIDMAChannel.PixelFormat.component3_width = UYVY_COMPONENT3_WIDTH -1; 

            pPPData->outputIDMAChannel.DataWidth = icDataWidth_16BPP;
            pPPData->outputIDMAChannel.LineStride = pPPData->outputIDMAChannel.FrameSize.width * 2;
        break;

        case icFormat_YUV420:
            pPPData->outputIDMAChannel.PixelFormat.component0_offset = YUV420_COMPONENT0_OFFSET;
            pPPData->outputIDMAChannel.PixelFormat.component1_offset = YUV420_COMPONENT1_OFFSET;
            pPPData->outputIDMAChannel.PixelFormat.component2_offset = YUV420_COMPONENT2_OFFSET;    
            pPPData->outputIDMAChannel.PixelFormat.component3_offset = YUV420_COMPONENT3_OFFSET;
            pPPData->outputIDMAChannel.PixelFormat.component0_width = YUV420_COMPONENT0_WIDTH -1;
            pPPData->outputIDMAChannel.PixelFormat.component1_width = YUV420_COMPONENT1_WIDTH -1;
            pPPData->outputIDMAChannel.PixelFormat.component2_width = YUV420_COMPONENT2_WIDTH -1;
            pPPData->outputIDMAChannel.PixelFormat.component3_width = YUV420_COMPONENT3_WIDTH -1; 
            
            pPPData->outputIDMAChannel.DataWidth = icDataWidth_8BPP;
            pPPData->outputIDMAChannel.LineStride = pPPData->outputIDMAChannel.FrameSize.width;

            pPPData->outputIDMAChannel.UBufOffset = iOutputHeight * iOutputWidth + (iOutputHeight * iOutputWidth)/4;
            pPPData->outputIDMAChannel.VBufOffset = iOutputHeight * iOutputWidth;
        break;

        case icFormat_YUV420P:
            pPPData->outputIDMAChannel.PixelFormat.component0_offset = YUV420_COMPONENT0_OFFSET;
            pPPData->outputIDMAChannel.PixelFormat.component1_offset = YUV420_COMPONENT1_OFFSET;
            pPPData->outputIDMAChannel.PixelFormat.component2_offset = YUV420_COMPONENT2_OFFSET;    
            pPPData->outputIDMAChannel.PixelFormat.component3_offset = YUV420_COMPONENT3_OFFSET;
            pPPData->outputIDMAChannel.PixelFormat.component0_width = YUV420_COMPONENT0_WIDTH -1;
            pPPData->outputIDMAChannel.PixelFormat.component1_width = YUV420_COMPONENT1_WIDTH -1;
            pPPData->outputIDMAChannel.PixelFormat.component2_width = YUV420_COMPONENT2_WIDTH -1;
            pPPData->outputIDMAChannel.PixelFormat.component3_width = YUV420_COMPONENT3_WIDTH -1; 
            
            pPPData->outputIDMAChannel.DataWidth = icDataWidth_8BPP;
            pPPData->outputIDMAChannel.LineStride = pPPData->outputIDMAChannel.FrameSize.width;
            
            pPPData->outputIDMAChannel.VBufOffset = iOutputHeight * iOutputWidth;
            pPPData->outputIDMAChannel.UBufOffset = pPPData->outputIDMAChannel.VBufOffset;            
        break;

        default:
            ERRORMSG(TRUE,(TEXT("%s:PP Onput Format Error!\r\n"),__WFUNCTION__));
            return FALSE;         
    }    

    //-----------------------------------------------------------------
    // Setup color space conversion
    //-----------------------------------------------------------------
    
    // Set up PP channel CSC parameters based on input. 
    switch(pPPData->outputIDMAChannel.FrameFormat)
    {
        case icFormat_RGB:
        case icFormat_RGBA:
            if ((pPPData->inputIDMAChannel.FrameFormat == icFormat_RGB) || 
                 (pPPData->inputIDMAChannel.FrameFormat == icFormat_RGBA))
                pPPData->CSCEquation = CSCNoOp;
            else
                pPPData->CSCEquation = CSCY2R_A1;

            break;

        case icFormat_UYVY422:
        case icFormat_VYUY422:
        case icFormat_YUV420:
        case icFormat_YUV420P:
        case icFormat_YUV422:
        case icFormat_YUV422P:
        case icFormat_YUV444:
        case icFormat_YUV444IL:
        case icFormat_YUYV422:
        case icFormat_YVYU422: 
            if ((pPPData->inputIDMAChannel.FrameFormat == icFormat_RGB) || 
                 (pPPData->inputIDMAChannel.FrameFormat == icFormat_RGBA))
                pPPData->CSCEquation = CSCR2Y_A1;
            else
                pPPData->CSCEquation = CSCNoOp;

            break;   

        default:
            ERRORMSG(TRUE,(TEXT("%s:PP Onput CSC Error!\r\n"),__WFUNCTION__));
            return FALSE;
    }
    
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: CameraPPWorkerThread
//
// This function is the CameraPP worker thread.
//
// Parameters:
//      lpParameter
//          [in] CameraPP handler
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void CameraPPClass::CameraPPWorkerThread(LPVOID lpParameter)
{
    CameraPPClass *pCameraPP = (CameraPPClass *)lpParameter;

    CAMERAPP_FUNCTION_ENTRY();

    pCameraPP->CameraPPWorkerLoop(INFINITE);

    CAMERAPP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: CameraPPWorkerLoop
//
// This function is the cameraPP worker loop.
// It waits for the SMFC EOF event, and send the buffer phy addr  
// to PP for handling.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for SMFC EOF event.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void CameraPPClass::CameraPPWorkerLoop(UINT32 timeout)
{
    CAMERAPP_FUNCTION_ENTRY();
    UINT32* physInputAddr = NULL;
    UINT32* physOutputAddr = NULL;
    DWORD result = 0;
    UINT32 uNumBuf;
    UINT32 i = 0;

    // loop here
    while(TRUE)
    {
        if ( WaitForSingleObject ( m_hExitCameraPPWorkerThread, 0 ) != WAIT_TIMEOUT )
        {
            ExitThread(1);
        }

        result = WaitForMultipleObjects(NUM_PIN_BUFFER_MAX, m_hCameraPPInputBufEvent, FALSE, timeout);
        for (i = 0; i<NUM_PIN_BUFFER_MAX; i++)
        {
            if( result == (WAIT_OBJECT_0 + i))
            {
                physInputAddr = CameraPPGetInputBufPhyAddr(i);
                break;
            }
        }

        if( i >= NUM_PIN_BUFFER_MAX)
        {
            ERRORMSG(TRUE,(TEXT("%s:PP Get Input buffer address event fail!\r\n"),__WFUNCTION__));
            continue;
        }

        if (NULL == physInputAddr)
        {
            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PP Get Input buffer address is NULL!\r\n"),__WFUNCTION__));
            continue;
        }
       
        // Critical section prevents race condition on
        // reading and writing ready and busy bits
        EnterCriticalSection(&m_csCameraPPStopping);
                        
        if (!m_bCameraPPRunning)
        {
            // Don't do anything else.  We do not want to re-enable
            // the buffer ready bits since we are stopping.
            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Stop CameraPP loop for m_bCameraPPRunning FALSE\r\n"),
                                         __WFUNCTION__));
        }
        else
        {
            // Add input and output buffers to PP queues.
            if (NULL != m_hPP)
            {
                if (!PPInterruptEnable(m_hPP, FRAME_INTERRUPT))
                {
                    ERRORMSG(TRUE,(TEXT("%s: PPInterruptEnable fail!\r\n"),__WFUNCTION__));
                    LeaveCriticalSection(&m_csCameraPPStopping);
                    continue;
                }  
                        
                //-----------------------------
                // Queue Input/Output Buffers
                //-----------------------------
                // Add input buffer
                if (!PPAddInputBuffer(m_hPP, (UINT32)physInputAddr))
                {
                    ERRORMSG(TRUE,(TEXT("%s:PP Add Input Buffer fail\r\n"),__WFUNCTION__));
                    LeaveCriticalSection(&m_csCameraPPStopping);
                    continue;
                }    
        
                // Add output buffer
                // A buffer has been requested.  Attempt to read from the ready
                // queue to get an available buffer.  If one is not available,
                // we wait here until it is.
                LeaveCriticalSection(&m_csCameraPPStopping);
                m_pCameraPPBufferManager->SetActiveBuffer(&physOutputAddr, INFINITE);
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PP Get Output Buffer %x!\r\n"),
                                        __WFUNCTION__,physOutputAddr));
                if (NULL == physOutputAddr)
                {
                    ERRORMSG(FALSE,(TEXT("%s:PP Get Output Buffer fail!\r\n"),__WFUNCTION__));
                    continue;
                }
                    
                EnterCriticalSection(&m_csCameraPPStopping);
                if (!PPAddOutputBuffer(m_hPP, (UINT32)physOutputAddr))
                {
                    ERRORMSG(TRUE,(TEXT("%s:PP Add Output Buffer fail!\r\n"),__WFUNCTION__));
                    LeaveCriticalSection(&m_csCameraPPStopping);
                    continue;
                }    

                if (!PPWaitForNotBusy(m_hPP, FRAME_INTERRUPT))
                {
                    ERRORMSG(TRUE,(TEXT("%s:PPWaitForNotBusy fail\r\n"),__WFUNCTION__));
                    LeaveCriticalSection(&m_csCameraPPStopping);
                    continue;
                }
                else
                {
                    uNumBuf = m_pCameraPPBufferManager->SetFilledBuffer();

                    if(uNumBuf != (DWORD)-1)
                    {                    
                        // increment frame count
                        m_iCameraPPFrameCount++;

                        // Trigger Frame EOF event
                        QueryPerformanceCounter(&m_lpPerformanceCount_end);
                        LARGE_INTEGER timeCount;
                        timeCount.QuadPart = (((m_lpPerformanceCount_end.QuadPart - m_lpPerformanceCount_start.QuadPart)*1000000)/(m_lpFrequency.QuadPart));
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("PP output interval is %I64d\r\n"),
                                                timeCount.QuadPart));
                        m_lpPerformanceCount_start.QuadPart  = m_lpPerformanceCount_end.QuadPart;
                        SetEvent(m_hCameraPPEOFEvent);
                    }
                    else
                    {
                        RETAILMSG(1,(TEXT("CameraPPClass::CameraPPWorkerLoop get output failed, m_pCameraPPBufferManager is %x\r\n"),m_pCameraPPBufferManager));
                    }
                }
            }
            else
            {
                RETAILMSG(1,(TEXT("CameraPPClass::CameraPPWorkerLoop not get m_pp handle\r\n")));
            }
        }
        
        LeaveCriticalSection(&m_csCameraPPStopping);
    }

    CAMERAPP_FUNCTION_EXIT();
    return;

}

//-----------------------------------------------------------------------------
//
// Function:  Enqueue
//
// This function enqueue buffer from Locked queue to Idle queue.
//
// Parameters:
//      pCameraPPBufferManager
//          [in] The curremt Buffer Manager object pointer.
//
// Returns:
//      Returns TRUE if successfule.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::Enqueue(CamBufferManager * pCameraPPBufferManager)
{
    CAMERAPP_FUNCTION_ENTRY();
    DEBUGMSG(ZONE_FUNCTION,(TEXT("CameraPPClass::Enqueue\r\n")));

    if (!pCameraPPBufferManager->EnQueue())
    {
        ERRORMSG(TRUE, 
            (TEXT("%s :Enqueue Buffer failed!\r\n"), __WFUNCTION__));
        CAMERAPP_FUNCTION_EXIT();
        return FALSE;
    }   

    CAMERAPP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: RegisterBuffer
//
// This function register buffer for the CameraPP
// and adds each to the buffer queue.
//
// Parameters:
//      pCameraPPBufferManager
//          [in] The curremt Buffer Manager object pointer.
//
//      pVirtualAddress
//          [in] the virtual address of buffer want to be registered.
//
//      pPhysicalAddress
//          [in] the physical address of buffer want to be registered.
//
// Returns:
//      Returns TRUE if successfule.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::RegisterBuffer(CamBufferManager * pCameraPPBufferManager, LPVOID pVirtualAddress, UINT32 pPhysicalAddress)
{
    CAMERAPP_FUNCTION_ENTRY();

    m_iCameraPPNumBuffers++;

    if (!pCameraPPBufferManager->RegisterBuffer(pVirtualAddress, pPhysicalAddress))
    {
        ERRORMSG(TRUE, 
            (TEXT("%s :Register Buffer failed!\r\n"), __WFUNCTION__));
        CAMERAPP_FUNCTION_EXIT();
        return FALSE;  
    }

    CAMERAPP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  UnregisterBuffer
//
// This function unregister buffers.
//
// Parameters:
//      pCameraPPBufferManager
//          [in] The curremt Buffer Manager object pointer.
//
//      pVirtualAddress
//          [in] the virtual address of buffer want to be registered.
//
//      pPhysicalAddress
//          [in] the physical address of buffer want to be registered.
//
// Returns:
//      Returns TRUE if successfule.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::UnregisterBuffer(CamBufferManager * pCameraPPBufferManager, LPVOID pVirtualAddress, UINT32 pPhysicalAddress)
{
    BOOL blRet;

    blRet = pCameraPPBufferManager->UnregisterBuffer(pVirtualAddress, pPhysicalAddress);

    if(blRet)
    {
        m_iCameraPPNumBuffers--;
    }

    return blRet;
}

//-----------------------------------------------------------------------------
//
// Function:  UnregisterAllBuffers
//
// This function deregister all of the buffers in the Idle, 
// Busy, and Filled queues.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL CameraPPClass::UnregisterAllBuffers()
{
    return m_pCameraPPBufferManager->UnregisterAllBuffers();
}


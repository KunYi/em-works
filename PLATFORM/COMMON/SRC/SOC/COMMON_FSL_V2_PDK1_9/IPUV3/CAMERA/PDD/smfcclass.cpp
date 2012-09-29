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
//  Copyright (C) 2008-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  SMFCClass.cpp
//
//  Implementation of Sensor Multi FIFO Controller driver class
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
#include "smfc.h"
#include "SMFCClass.h"
#include "cameradbg.h"
#pragma warning(disable: 4100 4127 4189)


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define SMFC_MAX_NUM_BUFFERS                        100

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
// Function: SMFCClass
//
// SMFC class constructor.  Calls SMFCInit to initialize module.
//
// Parameters:
//      SMFCInstance
//          [in]SMFC instance number
//      csi_sel
//          [in]Csi interface
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
SMFCClass::SMFCClass(UINT32 SMFCInstance, CSI_SELECT csi_sel)
{
    SMFCInit(SMFCInstance,csi_sel);
}

//-----------------------------------------------------------------------------
//
// Function: ~SMFCClass
//
// The destructor for SMFCClass.  Calls SMFCDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
SMFCClass::~SMFCClass(void)
{
    SMFCDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: SMFCInit
//
// This function initializes the Image Signal Process (SMFC).
//
// Parameters:
//      SMFCInstance
//          [in]SMFC instance number
//      csi_sel
//          [in]Csi interface
//
// Returns:
//      TRUE for successfull,otherwise for failure.
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCInit(UINT32 SMFCInstance, CSI_SELECT csi_sel)
{
    SMFC_FUNCTION_ENTRY();

    QueryPerformanceFrequency(&m_lpFrequency);
    m_lpPerformanceCount_start.QuadPart = 0;
    m_lpPerformanceCount_end.QuadPart = 0; 

    if (!SMFCRegsInit())
    {
        ERRORMSG (TRUE, (TEXT("%s: Failed to enable SMFC Regs!\r\n"), __WFUNCTION__));
        goto Error;
    }
    
    if (!CMRegsInit())
    {
        ERRORMSG (TRUE, (TEXT("%s: Failed to enable CM Regs!\r\n"), __WFUNCTION__));
        goto Error;
    }
    if (!IDMACRegsInit())
    {
        ERRORMSG (TRUE, (TEXT("%s: Failed to enable IDMAC Regs!\r\n"), __WFUNCTION__));
        goto Error;
    }    
    if (!CPMEMRegsInit())
    {
        ERRORMSG (TRUE, (TEXT("%s: Failed to enable CPMEM Regs!\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Current SMFCClass instance(Channel) num (1 ~ 4)
    m_uSMFCInstance = SMFCInstance;

    // CSI0 map to SMFC ch0
    // CSI1 map to SMFC ch2
    // Other CSI interface and SMFC ch doesn't used
    if( csi_sel == CSI_SELECT_CSI0 )
    {
        m_hSMFCIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_SMFC_INTR_EVENT0);
        if (m_hSMFCIntrEvent == NULL)
        {
            ERRORMSG(TRUE,
                (TEXT("%s: CreateEvent for SMFC Interrupt failed\r\n"), __WFUNCTION__));
            goto Error;
        }

        m_iSMFCChannel = IDMAC_CH_SMFC_CH0;
    }
    else if( csi_sel == CSI_SELECT_CSI1 )
    {
        m_hSMFCIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_SMFC_INTR_EVENT2);
        if (m_hSMFCIntrEvent == NULL)
        {
            ERRORMSG(TRUE,
                (TEXT("%s: CreateEvent for SMFC Interrupt failed\r\n"), __WFUNCTION__));
            goto Error;
        }

        m_iSMFCChannel = IDMAC_CH_SMFC_CH2;
    }
    else
    {
        ERRORMSG(TRUE,
                (TEXT("%s: Can not support other CSI interface\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Events to signal pin that frame is ready
    m_hCameraSMFCEOFEvent = CreateEvent(NULL, FALSE, FALSE, IPU_SMFC_EOF_Event[m_uSMFCInstance - 1]);
    if (m_hCameraSMFCEOFEvent == NULL)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent for SMFC EOF Interrupt %d failed\r\n"), __WFUNCTION__,m_uSMFCInstance));
        goto Error;
    }

    for (UINT i = 0; i < IPU_SMFC_CH_MAX; i++)
    {
        m_hSMFCEnableEvent[i] = CreateEvent(NULL, TRUE, FALSE, IPU_SMFC_Enable_Event[i]);
        if(NULL == m_hSMFCEnableEvent[i])
        {
            ERRORMSG(TRUE,
                (TEXT("%s: CreateEvent failed for SMFC Channel %d Enable\r\n"), __WFUNCTION__,i));
            goto Error;
        }
    }

    // Events to exit SMFCISRThread
    m_hExitSMFCISRThread = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;
    if(NULL == m_hExitSMFCISRThread)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for Exit SMFCISRThread\r\n"), __WFUNCTION__));
        goto Error;
    }

    //Only SMFC Instance No.1 can report buffer to PP
    if( m_uSMFCInstance == 1)
    {
        // Initialize buffer EOF handles
        for (UINT i = 0; i < NUM_PIN_BUFFER_MAX; i++)
        {
            m_hSMFCBufEOFEvent[i] = CreateEvent(NULL, FALSE, FALSE, SMFCBufEOFEvent[i]);
            if(NULL == m_hSMFCBufEOFEvent[i])
            {
                ERRORMSG(TRUE,
                    (TEXT("%s: CreateEvent failed for Request SMFCEOF Buffer %d\r\n"), __WFUNCTION__,i));
                goto Error;
            }
        }
    }

    InitializeCriticalSection(&m_csSMFCStopping);
    InitializeCriticalSection(&m_csSMFCEnable);

    m_pSMFCBufferManager = NULL;

    m_bSMFCConfigured = FALSE;
    m_bSMFCEnabled = FALSE;
    m_bSMFCRunning = FALSE;
   
    m_bSMFCRestartISRLoop = FALSE;

    m_iSMFCFrameCount = 0;
    m_iSMFCFirstFrame = 0;

    m_dwMemoryMode = BUFFER_MODE_DRIVER;

    // Initialize thread for Preprocessor ISR
    //      pThreadAttributes = NULL (must be NULL)
    //      dwStackSize = 0 => default stack size determined by linker
    //      lpStartAddress = CspiProcessQueue => thread entry point
    //      lpParameter = NULL => point to thread parameter
    //      dwCreationFlags = 0 => no flags
    //      lpThreadId = NULL => thread ID is not returned
    m_hSMFCISRThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SMFCIntrThread, this, 0, NULL);
    if (m_hSMFCISRThread == NULL)
    {
        ERRORMSG(TRUE,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    else
    {
        RETAILMSG(0, (TEXT("%s: create SMFC ISR thread success\r\n"), __WFUNCTION__));
        CeSetThreadPriority(m_hSMFCISRThread, 100);
    }

    SMFC_FUNCTION_EXIT();

    return TRUE;

Error:
    SMFCDeinit();
    SMFC_FUNCTION_EXIT();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: SMFCDeinit
//
// This function deinitializes the Image Signal Process (SMFC).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SMFCClass::SMFCDeinit(void)
{
    INT32 i;

    SMFC_FUNCTION_ENTRY();

    DeleteCriticalSection(&m_csSMFCStopping); 
    DeleteCriticalSection(&m_csSMFCEnable); 

    if (m_hSMFCIntrEvent != NULL)
    {
        CloseHandle(m_hSMFCIntrEvent);
        m_hSMFCIntrEvent = NULL;
    }

    if (m_hCameraSMFCEOFEvent != NULL)
    {
        CloseHandle(m_hCameraSMFCEOFEvent);
        m_hCameraSMFCEOFEvent = NULL;
    }

    for (i = 0; i < NUM_PIN_BUFFER_MAX; i++)
    {
        CloseHandle(m_hSMFCBufEOFEvent[i]);
        m_hSMFCBufEOFEvent[i] = NULL;
    }

    for (i = 0; i < IPU_SMFC_CH_MAX; i++)
    {
        CloseHandle(m_hSMFCEnableEvent[i]);
        m_hSMFCEnableEvent[i] = NULL;
    }

    if (m_hSMFCISRThread)
    {    
        // Set SMFCIntrThread end
        SetEvent ( m_hExitSMFCISRThread ); 
        // Wait for SMFCIntrThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hSMFCISRThread,INFINITE))
            RETAILMSG(0,(TEXT("Wait for SMFCIntrThread end")));

        CloseHandle(m_hSMFCISRThread);
        m_hSMFCISRThread = NULL;
    }
    if (m_hExitSMFCISRThread != NULL)
    {
        CloseHandle(m_hExitSMFCISRThread);
        m_hExitSMFCISRThread = NULL;
    }
    
    SMFCRegsCleanup();
    
    SMFC_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: SMFCEnable
//
// Enable the SMFC.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SMFCClass::SMFCEnable(void)
{
    SMFC_FUNCTION_ENTRY();

    if (m_bSMFCEnabled)
    {
        SMFC_FUNCTION_EXIT();
        return;
    }

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csSMFCEnable);

    // IF any SMFC channel is enable, SMFC register is enable
    // IF all SMFC channel is not enable, We will enable SMFC register there
    if( WaitForMultipleObjects (IPU_SMFC_CH_MAX, m_hSMFCEnableEvent, FALSE, 0 ) == WAIT_TIMEOUT )
    {
        // Enable SMFC
        SMFCRegsEnable();
        DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFCEnable: SMFCRegsEnable!\r\n")));
    }

    m_bSMFCEnabled = TRUE;

    LeaveCriticalSection(&m_csSMFCEnable);

    SMFC_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: SMFCDisable
//
// Disable the SMFC.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SMFCClass::SMFCDisable(void)
{
    SMFC_FUNCTION_ENTRY();

    if (!m_bSMFCEnabled)
    {
        SMFC_FUNCTION_EXIT();
        return;
    }

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csSMFCEnable);

    // IF any SMFC channel is enable, SMFC register is enable
    // Only all SMFC channel is disable, We can disable SMFC register there
    if( WaitForMultipleObjects (IPU_SMFC_CH_MAX, m_hSMFCEnableEvent, FALSE, 0 ) == WAIT_TIMEOUT )
    {
        // Disable SMFC
        SMFCRegsDisable();
        DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFCDisable: SMFCRegsDisable!\r\n")));
    }

    m_bSMFCEnabled = FALSE;

    LeaveCriticalSection(&m_csSMFCEnable);

    SMFC_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: SMFCIsBusy
//
// This function checks smfc IDMAC channel status
//
// Parameters:
//      ch
//          [in]The SMFC IDMAC channel number:0~3.
//
// Returns:
//      TRUE if busy.
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCIsBusy(UINT32 ch)
{
    BOOL ret = FALSE;
    
    SMFC_FUNCTION_ENTRY();
    switch (ch)
    {
        case IDMAC_CH_SMFC_CH0:
            if(CMGetProcTaskStatus(IPU_PROC_TASK_CSI2MEM_SMFC0) == IPU_PROC_TASK_STAT_ACTIVE)

                ret = TRUE;
            break;
            
        case IDMAC_CH_SMFC_CH1:
            if(CMGetProcTaskStatus(IPU_PROC_TASK_CSI2MEM_SMFC1) == IPU_PROC_TASK_STAT_ACTIVE)

                ret = TRUE;
            break;
            
        case IDMAC_CH_SMFC_CH2:
            if(CMGetProcTaskStatus(IPU_PROC_TASK_CSI2MEM_SMFC2) == IPU_PROC_TASK_STAT_ACTIVE)

                ret = TRUE;
            break;
            
        case IDMAC_CH_SMFC_CH3:
            if(CMGetProcTaskStatus(IPU_PROC_TASK_CSI2MEM_SMFC3) == IPU_PROC_TASK_STAT_ACTIVE)

                ret = TRUE;
            break;            
    }

    SMFC_FUNCTION_EXIT();
    return ret;
}


//-----------------------------------------------------------------------------
//
// Function: SMFCSetChannel
//
// This function Set the SMFC using IDMAC channel.
//
// Parameters:
//      ch
//          [in]The SMFC IDMAC channel number:0~3.
//
// Returns:
//      Null.
//
//-----------------------------------------------------------------------------
void SMFCClass::SMFCSetChannel(UINT32 ch)
{
    m_iSMFCChannel = ch;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:The using SMFC IDMAC channel m_iSMFCChannel is %x\r\n"),
                                    __WFUNCTION__,m_iSMFCChannel));
}


//-----------------------------------------------------------------------------
//
// Function:  Enqueue
//
// This function enqueue buffer from Locked queue to Idle queue.
//
// Parameters:
//      pSMFCBufferManager
//          [in] The current Buffer Manager object pointer.
//
// Returns:
//      Returns TRUE if successfule.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::Enqueue(CamBufferManager * pSMFCBufferManager)
{
    SMFC_FUNCTION_ENTRY();

    if (!pSMFCBufferManager->EnQueue())
    {
        ERRORMSG(TRUE, 
            (TEXT("%s :Enqueue Buffer failed!\r\n"), __WFUNCTION__));
        SMFC_FUNCTION_EXIT();
        return FALSE;
    }   

    SMFC_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  RegisterBuffer
//
// This function register buffer for the SMFC
// and adds each to the buffer queue.
//
// Parameters:
//      pSMFCBufferManager
//          [in] The current Buffer Manager object pointer.
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
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::RegisterBuffer(CamBufferManager * pSMFCBufferManager, LPVOID pVirtualAddress, UINT32 pPhysicalAddress)
{
    SMFC_FUNCTION_ENTRY();

    m_iSMFCNumBuffers++;
    m_dwMemoryMode = BUFFER_MODE_CLIENT;

    if (!pSMFCBufferManager->RegisterBuffer(pVirtualAddress, pPhysicalAddress))
    {
        ERRORMSG(TRUE, 
            (TEXT("%s :Register Buffer failed!\r\n"), __WFUNCTION__));
        SMFC_FUNCTION_EXIT();
        return FALSE;  
    }

    SMFC_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  UnregisterBuffer
//
// This function unregister buffers.
//
// Parameters:
//      pSMFCBufferManager
//          [in] The current Buffer Manager object pointer.
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
BOOL SMFCClass::UnregisterBuffer(CamBufferManager * pSMFCBufferManager, LPVOID pVirtualAddress, UINT32 pPhysicalAddress)
{
    BOOL blRet;

    blRet = pSMFCBufferManager->UnregisterBuffer(pVirtualAddress, pPhysicalAddress);

    if(blRet)
    {
        m_iSMFCNumBuffers--;
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
BOOL SMFCClass::UnregisterAllBuffers()
{
    return m_pSMFCBufferManager->UnregisterAllBuffers();
}

//-----------------------------------------------------------------------------
//
// Function:  GetMemoryMode
//
// This function get memory mode of smfc class
//
// Parameters:
//      None.
//
// Returns:
//      BUFFER_MODE_DRIVER or BUFFER_MODE_CLIENT.
//
//-----------------------------------------------------------------------------
DWORD SMFCClass::GetMemoryMode()
{
    return m_dwMemoryMode;
}

//-----------------------------------------------------------------------------
//
// Function: SMFCSetBufferManager
//
// This function sets the current SMFC buffer manager object pointer.
//
// Parameters:
//      pSMFCBufferManager.
//      [in] The current Buffer Manager object pointer.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SMFCClass::SMFCSetBufferManager(CamBufferManager * pSMFCBufferManager)
{                                                                 
    m_pSMFCBufferManager = pSMFCBufferManager;
    
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: m_pSMFCBufferManager is %x\r\n"),
                                    __WFUNCTION__,m_pSMFCBufferManager));
}

//-----------------------------------------------------------------------------
//
// Function:  SMFCAllocateBuffers
//
// This function allocates buffers for the SMFC
// and adds each to the buffer queue.  This function must be called
// in order to allocate physically contiguous buffers for use in
// the IPU's IDMAC.
//
// Parameters:
//      pSMFCBufferManager
//          [in] The current Buffer Manager object pointer.
//
//      numBuffers
//          [in] Number of buffers to allocate and add
//          to the queues.
//
//      bufSize
//          [in] Size of buffer to create and enqueue.
//
// Returns:
//      Returns virtual pointer to buffer created.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCAllocateBuffers(CamBufferManager * pSMFCBufferManager,ULONG numBuffers, ULONG bufSize)
{
    SMFC_FUNCTION_ENTRY();
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFC AllocateBuffers Begin, bufsize is %x\r\n"),__WFUNCTION__,bufSize));

    m_iSMFCFrameCount = 0;

    m_dwMemoryMode = BUFFER_MODE_DRIVER;
    // Initialize MsgQueues when buffers are allocated
    if (!pSMFCBufferManager->AllocateBuffers(numBuffers, bufSize))
    {
        ERRORMSG(TRUE, 
            (TEXT("%s :Buffer allocation failed!\r\n"), __WFUNCTION__));
        SMFC_FUNCTION_EXIT();
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFC AllocateBuffers finished\r\n"),__WFUNCTION__));

    SMFC_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  SMFCGetAllocBufPhyAddr
//
// This function allocates buffers for the SMFC
// and adds each to the buffer queue.  This function must be called
// in order to allocate physically contiguous buffers for use in
// the IPU's IDMAC.
//
// Parameters:
//      pSMFCBufferManager
//          [in] The current Buffer Manager object pointer.
//
//      numBuffers
//          [in] Number of buffers to allocate and add
//          to the queues.
//
//      bufSize
//          [in] Size of buffer to create and enqueue.
//
// Returns:
//      TRUE for Successful,otherwise for failure.
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCGetAllocBufPhyAddr(CamBufferManager * pSMFCBufferManager, ULONG numBuffers,UINT32* pPhyAddr[])
{
    DWORD dBufNum = 0;
    
    SMFC_FUNCTION_ENTRY();

    dBufNum = pSMFCBufferManager->GetAllocBufferPhyAddr(pPhyAddr);

    if(dBufNum != numBuffers)
    {
        ERRORMSG(TRUE, 
            (TEXT("%s :Get Buffer allocation physical address failed!\r\n"), __WFUNCTION__));
        SMFC_FUNCTION_EXIT();
        return FALSE;
    }

    SMFC_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  SMFCGetBufFilled
//
// This function reads the queue of filled SMFC channel
// buffers and returns the buffer at the top of the queue.
//
// Parameters:
//      None.
//
// Returns:
//      Pointer to filled buffer, or NULL if failure.
//
//-----------------------------------------------------------------------------
UINT32* SMFCClass::SMFCGetBufFilled()
{
    // Get filled buffer from the filled buffer queue.
    return m_pSMFCBufferManager->GetBufferFilled();
}

//-----------------------------------------------------------------------------
//
// Function:  SMFCDeleteBuffers
//
// This function deletes all of the SMFC in the Idle and Ready queues.
//
// Parameters:
//      pSMFCBufferManager
//          [in] The current Buffer Manager object pointer.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCDeleteBuffers(CamBufferManager * pSMFCBufferManager)
{
    SMFC_FUNCTION_ENTRY();

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFC DeleteBuffers Begin\r\n"),__WFUNCTION__));

    // Delete buffers.
    // for the issue: new buffer manager
    //if (!m_pSMFCBufferManager->DeleteBuffers())
    if (!pSMFCBufferManager->DeleteBuffers())
    {
        ERRORMSG(TRUE, 
            (TEXT("%s : Failed to delete buffers!\r\n"), __WFUNCTION__));
        SMFC_FUNCTION_EXIT();
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFC DeleteBuffers finished\r\n"),__WFUNCTION__));
    SMFC_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  SMFCGetMaxBuffers
//
// This function returns the maximum number of buffers supported
// by the SMFC.
//
// Parameters:
//      None.
//
// Returns:
//      Returns the Max buffer number.
//
//-----------------------------------------------------------------------------
UINT32 SMFCClass::SMFCGetMaxBuffers(void)
{
    return SMFC_MAX_NUM_BUFFERS;
}

//-----------------------------------------------------------------------------
//
// Function: SMFCConfigureViewfinding
//
// This function configures the SMFC registers and IDMAC
// channels parameters (SMFC -> IDMACn).
// Sensor->CSI->SMFC->IDMAC->PP->IDMAC for viewfinding channel
//
// Parameters:
// Parameters:
//      ch
//        [in] channel number
//      csi_sel
//        [in] The input data from CSI module:CSI0,CSI1.
//      csi_FrameId
//        [in] The input CSI frame ID:0~3.CSI moduel(Two interface: CSI0,CSI1) can 
//             handle up four stream of data(four input sensor at most).
//      configureData
//        [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCConfigureViewfinding(UINT32 ch,CSI_SELECT csi_sel, CSI_SELECT_FRAME csi_FrameId,pSMFCConfigData configData)
{
    // If only run capture pin,it will only config SMFC
    // But if only run preview pin,it need config SMFC and PP
    if(!SMFCConfigureEncoding(ch,csi_sel,csi_FrameId,configData))
        return FALSE;

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SMFCConfigureEncoding
//
// This function configures the SMFC registers and IDMAC
// channels for the SMFC output channel .
// Sensor->CSI->SMFC->IDMAC for encoding channel 
//
// Parameters:
//      ch
//        [in] channel number
//      csi_sel
//        [in] The input data from CSI module:CSI0,CSI1.
//      csi_FrameId
//        [in] The input CSI frame ID:0~3.CSI moduel(Two interface: CSI0,CSI1) can 
//             handle up four stream of data(four input sensor at most).
//      configData
//        [in] Pointer to configuration data structure.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCConfigureEncoding(UINT32 ch,CSI_SELECT csi_sel,CSI_SELECT_FRAME csi_FrameId,pSMFCConfigData configData)
{
    UINT16 iVfOutputWidth, iVfOutputHeight;
    BOOL result = TRUE;
    SMFCFormat  iVfFormat = configData->outputFormat;
    BOOL bCurrentPlanar = FALSE;
    SMFCIDMACChannelParams channelParams;
    CPMEMBufOffsets OffsetData;

    memset (&channelParams, 0, sizeof(SMFCIDMACChannelParams));
    memset(&OffsetData, 0, sizeof(CPMEMBufOffsets));

    //////////////////////////////////////////////////////////
    // Begin to prepare data for IDMA     
    //////////////////////////////////////////////////////////
    
    iVfOutputWidth = configData->outputSize.width;
    iVfOutputHeight = configData->outputSize.height;
    
    // Set output pixel format
    channelParams.pixelFormat.component0_offset = configData->outputPixelFormat.component0_offset;
    channelParams.pixelFormat.component1_offset = configData->outputPixelFormat.component1_offset;
    channelParams.pixelFormat.component2_offset = configData->outputPixelFormat.component2_offset;;    
    channelParams.pixelFormat.component3_offset = configData->outputPixelFormat.component3_offset;;
    channelParams.pixelFormat.component0_width = configData->outputPixelFormat.component0_width;
    channelParams.pixelFormat.component1_width = configData->outputPixelFormat.component1_width;
    channelParams.pixelFormat.component2_width = configData->outputPixelFormat.component2_width;
    channelParams.pixelFormat.component3_width = configData->outputPixelFormat.component3_width;  

    // CPM paremeters
    switch(iVfFormat)
    {
        case SMFCFormat_RGB24:
            
            // 24 bits per pixel
            channelParams.iLineStride = iVfOutputWidth * 3;
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
            channelParams.iBitsPerPixelCode = CPMEM_BPP_24;
            channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_16;
            break;
                    
        case SMFCFormat_RGB565:
            
            // 16 bits per pixel
            channelParams.iLineStride = iVfOutputWidth * 2;
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
            channelParams.iBitsPerPixelCode = CPMEM_BPP_16;
            channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_16;  
            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Set SMFCFormat_RGB. \r\n"),__WFUNCTION__));
            break;

        case SMFCFormat_UYVY422:
            
            channelParams.iLineStride = iVfOutputWidth * 2;
            channelParams.bInterleaved = TRUE;
            channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_UYVY2;
            channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_16;
            channelParams.iBitsPerPixelCode = CPMEM_BPP_16;
            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Set SMFCFormat_UYVY422. \r\n"),__WFUNCTION__));
            break;

        case SMFCFormat_YUV420:
            
            channelParams.iLineStride = iVfOutputWidth;//YV12 is 12 bits per pixel
            channelParams.bInterleaved = FALSE;
            channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420;
            channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_16;
            channelParams.iBitsPerPixelCode = CPMEM_BPP_8;
            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Set SMFCFormat_YUV420. \r\n"),__WFUNCTION__));
            break;

        case SMFCFormat_YUV420P:
            
            channelParams.iLineStride = iVfOutputWidth;//NV12 is 12 bits per pixel
            channelParams.bInterleaved = FALSE;
            channelParams.iFormatCode = IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420;
            channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_16;
            channelParams.iBitsPerPixelCode = CPMEM_BPP_8;
            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Set SMFCFormat_YUV420P. \r\n"),__WFUNCTION__));
            break;

        default:
            ERRORMSG(TRUE,  (TEXT("%s: Invalid viewfinding output format, %d !! \r\n"), __WFUNCTION__, configData->outputFormat));
            result =  FALSE;
    }

    // Set output data size
    channelParams.iHeight = iVfOutputHeight;
    channelParams.iWidth = iVfOutputWidth;
   
    // Configure rotation BAM parameter 
    // SMFC not do Rotation and Flip
    channelParams.iRotation90 = 0;
    channelParams.iFlipHoriz = 0;
    channelParams.iFlipVert = 0;
    channelParams.iBlockMode = 0;
    channelParams.iBandMode = CPMEM_BNDM_DISABLE;

    // CPM offset parameters
    if ((iVfFormat == SMFCFormat_YUV444) || (iVfFormat == SMFCFormat_YUV422) ||
        (iVfFormat == SMFCFormat_YUV420) || (iVfFormat == SMFCFormat_YUV422P) ||
        (iVfFormat == SMFCFormat_YUV420P))
    {
        bCurrentPlanar = TRUE;
    }
    else
    {
        bCurrentPlanar = FALSE;
    }

    if (bCurrentPlanar)
    {        
        OffsetData.bInterleaved = FALSE;
        
        OffsetData.vOffset = channelParams.iWidth* channelParams.iHeight;
        if (IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420 == channelParams.iFormatCode)
            OffsetData.uOffset = OffsetData.vOffset + OffsetData.vOffset/4;            
        else if (IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420  == channelParams.iFormatCode)
            OffsetData.uOffset = OffsetData.vOffset;  

        OffsetData.interlaceOffset = 0;
    }

    // Set CPM offset
    if (bCurrentPlanar)                                              
        CPMEMWriteOffset(ch, &OffsetData, OffsetData.bInterleaved);

    // Set SMFC using IDMAC channel
    SMFCSetChannel(ch);
    
    // Config IDMAC channel parameters (SMFC->IDMAC->Mem)
    SMFCIDMACChannelConfig(ch, &channelParams);

    // Config SMFC registers
    SMFCConfig(ch,csi_sel,csi_FrameId,&channelParams);

    m_bSMFCConfigured = TRUE;

    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: SMFCConfig
//
// This function configs the SMFC registers.
//
// Parameters:
//      ch
//          [in]channel 0 ~ channel 3.
//      csi_sel
//          [in] The input data from CSI module:CSI0,CSI1.
//      csi_FrameId
//          [in] The input CSI frame ID:0~3.CSI moduel(Two interface: CSI0,CSI1) can 
//               handle up four stream of data(four input sensor at most).
//      pChannelParams
//          [in] The pointer to SMFC IDMAC Channel parameters structure.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCConfig(UINT32 ch,CSI_SELECT csi_sel,CSI_SELECT_FRAME csi_FrameId, pSMFCIDMACChannelParams pChannelParams)
{
    //-----------------------------------
    //initalize the processing flow
    //-----------------------------------
    switch (ch)
    {
        case IDMAC_CH_SMFC_CH0:
            // the source should be mcu
            CMSetProcFlow(IPU_PROC_FLOW_SMFC0_DEST,
                                        IPU_IPU_FS_PROC_FLOW3_DEST_SEL_MCU);           
            break;
            
         case IDMAC_CH_SMFC_CH1:
             CMSetProcFlow(IPU_PROC_FLOW_SMFC1_DEST,
                                         IPU_IPU_FS_PROC_FLOW3_DEST_SEL_MCU); 
             break;

         case IDMAC_CH_SMFC_CH2:
            
            CMSetProcFlow(IPU_PROC_FLOW_SMFC2_DEST,
                                        IPU_IPU_FS_PROC_FLOW3_DEST_SEL_MCU); 
            
            break;            

        case IDMAC_CH_SMFC_CH3:
           CMSetProcFlow(IPU_PROC_FLOW_SMFC3_DEST,
                                       IPU_IPU_FS_PROC_FLOW3_DEST_SEL_MCU); 
            
           break;
     }

    // Set up SMFC_MAP
    SMFCSetCSIMap(ch,csi_sel,csi_FrameId);

    // Set up SMFC_BS
    SMFCSetBurstSize(ch,pChannelParams->iBitsPerPixelCode,pChannelParams->iFormatCode,pChannelParams->iPixelBurstCode);
 
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: SMFCInitDisplayCharacteristics
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
BOOL SMFCClass::SMFCInitDisplayCharacteristics(void)
{
    m_hDisplay = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

    // Get Display Type from Display driver
    if (ExtEscape(m_hDisplay, VF_GET_DISPLAY_INFO, NULL, NULL,
        sizeof(DISPLAY_CHARACTERISTIC), (LPSTR) &m_displayCharacteristics) < 0)
    {
        DEBUGMSG(ZONE_FUNCTION,  (TEXT("%s: Unable to get display characteristics! \r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SMFCStartDMAChannel
//
// This function starts IDMA channel that SMFC used.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCStartChannel(void)
{
    DWORD dwFrameDropped;
    UINT32* physAddr;
    SMFC_FUNCTION_ENTRY();
    DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s(): Starting SMFC channel %d begin!\r\n"),__WFUNCTION__,m_iSMFCChannel));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFC start begin++\r\n")));
    
    // SMFC must have been configured at least once 
    // in order to start the channel
    if (!m_bSMFCConfigured)
    {
        SMFC_FUNCTION_EXIT();
        ERRORMSG(TRUE,
            (TEXT("%s: Error.  Cannot start SMFC channel without first configuring\r\n"),
            __WFUNCTION__));
        return FALSE;
    }
    
    if (m_bSMFCRunning)
    {
        SMFC_FUNCTION_EXIT();
        DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s: SMFC channel already running.\r\n"), __WFUNCTION__));
        return TRUE;
    }

    SMFCEnable();

    // Protect access to IPU_INT_CTRL_1, IPU_CHA_DB_MODE_SEL,
    // IDMAC_CHA_EN registers.
    
    // Not using EOF interrupt:IPU_INT_CTRL_1
    // Enable NFACK interrupt:IPU_INT_CTRL_1
    IDMACChannelClearIntStatus(m_iSMFCChannel, IPU_INTR_TYPE_NFACK);
    IDMACChannelIntCntrl(m_iSMFCChannel, IPU_INTR_TYPE_NFACK, TRUE);  //ch 0
    
    // Set DB mode:IPU_CH_DB_MODE_SEL0
    IDMACChannelDBMODE(m_iSMFCChannel,TRUE);
    
    // Set current buffer bits, so that we will start on buffer 0:IPU_CH_DB_MODE_SEL0
    IDMACChannelCurrentBufIsBuf0(m_iSMFCChannel);
    
    // Set current buffer bits, so that we will start on buffer 0:IPU_CH_DB_MODE_SEL0
    IDMACChannelClrCurrentBuf(m_iSMFCChannel);

    // Enable channel:IDMAC_CH_EN_1
    IDMACChannelEnable(m_iSMFCChannel);

    m_bSMFCRunning = TRUE;
    m_bSMFCRestartISRLoop = TRUE;

    // Signal a Event: One SMFC channel is enable
    SetEvent(m_hSMFCEnableEvent[m_iSMFCChannel]);

    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause Pin to believe a frame was ready.
    ResetEvent(m_hCameraSMFCEOFEvent);

    //Only SMFC Instance No.1 can report buffer to PP
    if( m_uSMFCInstance == 1)
    {
        for (UINT32 i = 0; i<NUM_PIN_BUFFER_MAX; i++)
            ResetEvent(m_hSMFCBufEOFEvent[i]);
    }

    dwFrameDropped = m_pSMFCBufferManager->SetActiveBuffer(&physAddr, INFINITE);
    if(dwFrameDropped == -1)
    {
        ERRORMSG(TRUE, (TEXT("%s: SetActiveBuffer error\r\n"), __WFUNCTION__));
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SetActiveBuffer 0x%x\r\n"), __WFUNCTION__,physAddr));

    CPMEMWriteBuffer(m_iSMFCChannel,0, physAddr);
    IDMACChannelBUF0SetReady(m_iSMFCChannel);

    m_iCurrentHWBufAddr = 0;
    m_iSMFCFirstFrame = 0;

    DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s(): Starting SMFC channel %d End!\r\n"),__WFUNCTION__,m_iSMFCChannel));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFC start end++\r\n")));

    SMFC_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SMFCPreStopDMAChannel
//
// This function wait for the SMFC not busy.
// Reason of adding this funciton:
//      When stopping the SMFC channel, must wait for SMFC not busy , 
//      Otherwise system can't enter into the suspend mode.
//
// Please pay attention to this thing:
//      When stopping the stream flow, must guarantee that
//      CSI is disabled before SMFC.
//      Otherwise, after start and stop the stream flow some times,
//      SMFC will not work correctly.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCPreStopChannel(void)
{
    UINT32 uCount = 0;

    EnterCriticalSection(&m_csSMFCStopping);

    while(IDMACChannelIsBusy(m_iSMFCChannel)
               ||SMFCIsBusy(m_iSMFCChannel))
    {
        if (uCount <= 1000)
        {
            //..give up the remainder of time slice
            Sleep(1);
            uCount++;
        }
        else
        {
            //.. there is something wrong ....break out
            RETAILMSG(TRUE,(TEXT("%s(): Error in stopping SMFC channel!\r\n"),__WFUNCTION__));
            break;
        }    
    }

    LeaveCriticalSection(&m_csSMFCStopping);

    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: SMFCStopDMAChannel
//
// This function stops IDMA channel that SMFC used.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE for successful,otherwise for failure.
//
//-----------------------------------------------------------------------------
BOOL SMFCClass::SMFCStopChannel(void)
{    
    SMFC_FUNCTION_ENTRY();
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFC stop begin++\r\n")));

    DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s(): Stopping SMFC channel %d begin!\r\n"),__WFUNCTION__,m_uSMFCInstance));
    
    // If not running, return
    if (!m_bSMFCRunning)
    {
        DEBUGMSG(ZONE_FUNCTION,
                (TEXT("%s(): Stopping SMFC channel end - Do nothing!\r\n"),__WFUNCTION__));
        SMFC_FUNCTION_EXIT();
        return TRUE;
    } 

    EnterCriticalSection(&m_csSMFCStopping);
        
    IDMACChannelDisable(m_iSMFCChannel);
        
    IDMACChannelDBMODE(m_iSMFCChannel,FALSE);

    IDMACChannelBUF0ClrReady(m_iSMFCChannel);

    IDMACChannelBUF1ClrReady(m_iSMFCChannel);

    IDMACChannelClrCurrentBuf(m_iSMFCChannel);

    // Signal a Event: One SMFC channel is disable
    ResetEvent(m_hSMFCEnableEvent[m_iSMFCChannel]);

    SMFCDisable();

    m_bSMFCRunning = FALSE;  

    ResetEvent(m_hCameraSMFCEOFEvent);

    //Only SMFC Instance No.1 can report buffer to PP
    if( m_uSMFCInstance == 1)
    {
        for (UINT32 i = 0; i<NUM_PIN_BUFFER_MAX; i++)
        {
            ResetEvent(m_hSMFCBufEOFEvent[i]);
        }
    }

    LeaveCriticalSection(&m_csSMFCStopping);

    m_pSMFCBufferManager->ResetBuffers();

    m_iSMFCFirstFrame = 0;

    DEBUGMSG(ZONE_FUNCTION,
                (TEXT("%s(): Stopping SMFC channel End!\r\n"),__WFUNCTION__));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFC stop end++\r\n")));

    SMFC_FUNCTION_EXIT();

    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: SMFCGetFrameCount
//
// This function returns the current SMFC channel frame count.
//
// Parameters:
//      None.
//
// Returns:
//      The number of the frames processed since the
//      last configuration request.
//
//-----------------------------------------------------------------------------
UINT32 SMFCClass::SMFCGetFrameCount()
{
    return m_iSMFCFrameCount;
}


//-----------------------------------------------------------------------------
//
// Function: SMFCIDMACChannelConfig
//
// This function configures the (SMFC ->) IDMAC parameters.
//
// Parameters:
//      ch
//          [in] The IDMAC channel being queried.
//
//      pChannelParams
//          [in] The pointer to SMFC IDMAC Channel parameters structure.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SMFCClass::SMFCIDMACChannelConfig(UINT32 ch,
    pSMFCIDMACChannelParams pChannelParams)
{ 
    CPMEMConfigData CPMEMData;
    memset(&CPMEMData,0,sizeof(CPMEMConfigData));

    CPMEMData.iBitsPerPixel = pChannelParams->iBitsPerPixelCode;
    CPMEMData.iDecAddrSelect = CPMEM_DEC_SEL_ADDR_0_TO_15; // Don't care, only for 4BPP mode   //not support yet
    CPMEMData.iAccessDimension = CPMEM_DIM_2D; // 1D scan is only used for csi generic data   
    CPMEMData.iScanOrder = CPMEM_SO_PROGRESSIVE; //always progressive, not support interlaced yet
    CPMEMData.iThresholdEnable = CPMEM_THE_DISABLE;
    CPMEMData.iThreshold = 0;
    CPMEMData.iCondAccessEnable = CPMEM_CAE_COND_SKIP_DISABLE;    // for future use
    CPMEMData.iCondAccessPolarity = CPMEM_CAP_COND_SKIP_LOW;  // for future use
    CPMEMData.iAlphaUsed = CPMEM_ALU_ALPHA_SAME_CHANNEL;  // seperate alpha not support yet
    CPMEMData.iAXI_Id = CPMEM_ID_0; //not used yet     

    CPMEMData.iBandMode = pChannelParams->iBandMode;
    if(CPMEMData.iBandMode != CPMEM_BNDM_DISABLE)
    {
        IDMACChannelSetBandMode(ch,IDMAC_CH_BAND_MODE_ENABLE);
    }
    else
    {
        IDMACChannelSetBandMode(ch,IDMAC_CH_BAND_MODE_DISABLE);
    }  
    CPMEMData.iBlockMode = pChannelParams->iBlockMode;
    CPMEMData.iRotation90 = pChannelParams->iRotation90;
    CPMEMData.iFlipHoriz = pChannelParams->iFlipHoriz;
    CPMEMData.iFlipVert = pChannelParams->iFlipVert;    
    CPMEMData.iPixelBurst = pChannelParams->iPixelBurstCode;
    
    CPMEMData.iPixelFormat = pChannelParams->iFormatCode;  
    CPMEMData.iWidth = pChannelParams->iWidth - 1;
    CPMEMData.iHeight = pChannelParams->iHeight - 1;
    CPMEMData.iLineStride =  pChannelParams->iLineStride - 1;
    CPMEMData.iLineStride_Y = pChannelParams->iLineStride - 1;
    if((pChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422)
    || (pChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420))
    {
        CPMEMData.iLineStride_UV = ((CPMEMData.iLineStride_Y+1) >>1) - 1;
    }
    else
    {
        CPMEMData.iLineStride_UV = CPMEMData.iLineStride_Y;        
    }

    memcpy(&CPMEMData.pixelFormatData, &pChannelParams->pixelFormat, sizeof(SMFCPixelFormat));
    CPMEMWrite(ch, &CPMEMData, pChannelParams->bInterleaved);
    Sleep(100);  
 
}

//------------------------------------------------------------------------------
//
// Function: SMFCIntrThread
//
// This function is the IST thread.
//
// Parameters:
//      lpParameter
//          [in] SMFC handler
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void SMFCClass::SMFCIntrThread(LPVOID lpParameter)
{
    SMFCClass *pSMFC = (SMFCClass *)lpParameter;

    SMFC_FUNCTION_ENTRY();

    pSMFC->SMFCISRLoop(INFINITE);

    SMFC_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: SMFCISRLoop
//
// This function is the interrupt handler for the SMFC.
// It waits for the SMFC interrupt, and signals
// the EOF(End of frame) or EOM(End of first module) event registered 
// by the user of the SMFC.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for NFACK interrupt.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void SMFCClass::SMFCISRLoop(UINT32 timeout)
{
    SMFC_FUNCTION_ENTRY();
    UINT32 uNumBuf;
    UINT32 uCurrentBufAddr;
    UINT32 uCurrentBufIndex;
    UINT32* physAddr;
    DWORD dwFrameDropped;
    int temp = 0 ;

    HANDLE hLoopEvent[2];
    DWORD result;

    hLoopEvent[0] = m_hExitSMFCISRThread;
    hLoopEvent[1] = m_hSMFCIntrEvent;

    // loop here
    while((result = WaitForMultipleObjects (2, hLoopEvent, FALSE, timeout )) != WAIT_TIMEOUT)
    {
        RETAILMSG (0, (TEXT("%s: In the loop ch:%d Ins %d\r\n"), __WFUNCTION__,m_iSMFCChannel,m_uSMFCInstance));

        if ( result == WAIT_OBJECT_0 + 0 )
        {
            break;
        }

        if ( result == WAIT_OBJECT_0 + 1 )
        {
            // Thread is running properly, so we do not have to restart.
            m_bSMFCRestartISRLoop = FALSE;

            // Critical section prevents race condition on
            // reading and writing ready and busy bits
            EnterCriticalSection(&m_csSMFCStopping);
                        
            if (!m_bSMFCRunning)
            {
                // Don't do anything else.  We do not want to re-enable
                // the buffer ready bits since we are stopping.
                RETAILMSG(0,(TEXT("%s: Stop SMFC loop for m_bSMFCRunning FALSE\r\n"),__WFUNCTION__));
            }
            else if (m_bSMFCRestartISRLoop)
            {
                // This code ensures that we do not run into problems
                // if the SMFCl was stopped at an earlier time while
                // this thread was waiting at the m_csSMFCStopping critical 
                // section.  If so, we simply start over and wait for a new
                // interrupt.         
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:LeaveCriticalSection for m_bSMFCRestartISRLoop\r\n"),__WFUNCTION__));
                LeaveCriticalSection(&m_csSMFCStopping);
                continue;
            }
            else
            {
                // NFACK for SMFC
                if( IDMACChannelGetIntStatus(m_iSMFCChannel,IPU_INTR_TYPE_NFACK))//IPU_INT_STAT_1:ch0
                {
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFC NFACK begin++\r\n")));
                    // Clear NFACK status bits
                    IDMACChannelClearIntStatus(m_iSMFCChannel,IPU_INTR_TYPE_NFACK);
                    
                    if (IDMACChannelCurrentBufIsBuf1(m_iSMFCChannel)) 
                    {
                        while(IDMACChannelBUF1IsReady(m_iSMFCChannel))
                        {
                            ERRORMSG(TRUE,
                                          (TEXT("%s: Current buffer is buffer1 but buffer 1 is ready\r\n"), __WFUNCTION__));
                            Sleep(1);
                        }
                        uCurrentBufIndex = 1;
                        uCurrentBufAddr = CPMEMReadBufferAddr(m_iSMFCChannel, 1);
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("current buffer is buffer 1, address = %x \r\n"),uCurrentBufAddr));
                    }
                    else
                    {
                        while(IDMACChannelBUF0IsReady(m_iSMFCChannel))
                        {
                            ERRORMSG(TRUE,
                                          (TEXT("%s: Current buffer is buffer0 but buffer 0 is ready\r\n"), __WFUNCTION__));
                            Sleep(1);
                        }
                        uCurrentBufIndex = 0; 
                        uCurrentBufAddr = CPMEMReadBufferAddr(m_iSMFCChannel, 0);
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("current buffer is buffer 0, address = %x \r\n"),uCurrentBufAddr));
                    }  

                    // When it is the first time received NFACK, we should not get buffer out, and we need set a new buffer only
                    if(m_iCurrentHWBufAddr == 0)
                    {
                        m_iCurrentHWBufAddr = uCurrentBufAddr;

                        LeaveCriticalSection(&m_csSMFCStopping);
                        dwFrameDropped = m_pSMFCBufferManager->SetActiveBuffer(&physAddr, INFINITE);
                        EnterCriticalSection(&m_csSMFCStopping);

                        DEBUGMSG(ZONE_FUNCTION, (_T("%s: SMFC SetActiveBuffer return - pPhysical address: %x\r\n"), 
                                         __WFUNCTION__, physAddr));
                        if (dwFrameDropped == 1)
                        {
                            DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Frame Dropped.\r\n"), __WFUNCTION__));
                        }
                        else if (dwFrameDropped == -1)
                        {
                            DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Error, did not receive a buffer.\r\n"), __WFUNCTION__));
                            IDMACChannelIntCntrl(m_iSMFCChannel,IPU_INTR_TYPE_NFACK,TRUE);
                            LeaveCriticalSection(&m_csSMFCStopping);
                            continue;
                        }

                        if (uCurrentBufIndex) 
                        { 
                            if (IDMACChannelCurrentBufIsBuf0(m_iSMFCChannel)) 
                            {
                                RETAILMSG(1,(TEXT("ERROR During process , current buffer changed!!!!!! addr = 0x%x\r\n"),physAddr));
                            }
                            else
                            {
                                CPMEMWriteBuffer(m_iSMFCChannel,0, physAddr);
                                IDMACChannelBUF0SetReady(m_iSMFCChannel);
                            }
                        }
                        else
                        {
    
                            if (IDMACChannelCurrentBufIsBuf1(m_iSMFCChannel)) 
                            {
                                RETAILMSG(1,(TEXT("ERROR During process , current buffer changed!!!!!! addr = 0x%x\r\n"),physAddr));
                            }
                            else
                            {
                                CPMEMWriteBuffer(m_iSMFCChannel, 1, physAddr);
                                IDMACChannelBUF1SetReady(m_iSMFCChannel);
                            }
                        } 
                        
                        IDMACChannelIntCntrl(m_iSMFCChannel,IPU_INTR_TYPE_NFACK,TRUE);
                        LeaveCriticalSection(&m_csSMFCStopping);
                        continue;
                    }

                    if(uCurrentBufAddr == m_iCurrentHWBufAddr)
                    {
                        RETAILMSG(0,(TEXT("The HW address don't change %x, so we can't set new buffer\r\n"),m_iCurrentHWBufAddr));
                        #if 0
                        // if occurs the exception, maintains the address of the last time and reset bufrdy
                        if(m_dwMemoryMode == BUFFER_MODE_DRIVER)
                        {
                            if (uCurrentBufIndex) 
                            { 
                                if (IDMACChannelCurrentBufIsBuf0(m_iSMFCChannel)) 
                                {
                                    RETAILMSG(1,(TEXT("ERROR During process , current buffer changed!!!!!!\r\n")));
                                }

                                IDMACChannelBUF0SetReady(m_iSMFCChannel);
                            }
                            else
                            {
                                if (IDMACChannelCurrentBufIsBuf1(m_iSMFCChannel)) 
                                {
                                    RETAILMSG(1,(TEXT("ERROR During process , current buffer changed!!!!!!\r\n")));
                                }

                                IDMACChannelBUF1SetReady(m_iSMFCChannel);
                            } 

                            IDMACChannelIntCntrl(m_iSMFCChannel,IPU_INTR_TYPE_NFACK,TRUE);
                            LeaveCriticalSection(&m_csSMFCStopping);
                            continue;
                        }
                        #endif
                    }
                    
                    m_iCurrentHWBufAddr = uCurrentBufAddr;
                    
                    LeaveCriticalSection(&m_csSMFCStopping);
                    dwFrameDropped = m_pSMFCBufferManager->SetActiveBuffer(&physAddr, INFINITE);
                    EnterCriticalSection(&m_csSMFCStopping);

                    DEBUGMSG(ZONE_FUNCTION, (_T("%s: SMFC SetActiveBuffer return - pPhysical address: %x\r\n"), 
                                     __WFUNCTION__, physAddr));
                    if (dwFrameDropped == 1)
                    {
                        DEBUGMSG (ZONE_FUNCTION, (TEXT("%s: Frame Dropped.\r\n"), __WFUNCTION__));
                    }
                    else if (dwFrameDropped == -1)
                    {
                        DEBUGMSG (ZONE_FUNCTION, (TEXT("%s: Error, did not receive a buffer.\r\n"), __WFUNCTION__));
                        IDMACChannelIntCntrl(m_iSMFCChannel,IPU_INTR_TYPE_NFACK,TRUE);
                        LeaveCriticalSection(&m_csSMFCStopping);
                        continue;
                    }
                
                    // Critical section to prevent race condition upon
                    // stopping the encoding channel
                    
                    // If we are stopping the encoding channel, return 
                    // active buffer to ready queue, and bail out
                    // of buffer request routine, as we do not want to
                    // continue processing in this channel
                    if (!m_bSMFCRunning)
                    {
                        LeaveCriticalSection(&m_csSMFCStopping);
                        m_pSMFCBufferManager->ResetBuffers();
                        EnterCriticalSection(&m_csSMFCStopping);

                        IDMACChannelIntCntrl(m_iSMFCChannel,IPU_INTR_TYPE_NFACK,TRUE);
                        LeaveCriticalSection(&m_csSMFCStopping);        
                        continue;
                    }
        
                    DEBUGMSG(ZONE_FUNCTION, (_T("%s: SMFC buffer ready - pPhysical address: %x\r\n"), 
                                    __WFUNCTION__, physAddr));
        
                    if (uCurrentBufIndex) 
                    { 
                        if (IDMACChannelCurrentBufIsBuf0(m_iSMFCChannel)) 
                        {
                            RETAILMSG(1,(TEXT("ERROR During process , current buffer changed!!!!!! addr = 0x%x\r\n"),physAddr));
                        }
                        else
                        {
                            CPMEMWriteBuffer(m_iSMFCChannel,0, physAddr);
                            IDMACChannelBUF0SetReady(m_iSMFCChannel);
                        }
                    }
                    else
                    {

                        if (IDMACChannelCurrentBufIsBuf1(m_iSMFCChannel)) 
                        {
                            RETAILMSG(1,(TEXT("ERROR During process , current buffer changed!!!!!! addr = 0x%x\r\n"),physAddr));
                        }
                        else
                        {
                            CPMEMWriteBuffer(m_iSMFCChannel, 1, physAddr);
                            IDMACChannelBUF1SetReady(m_iSMFCChannel);
                        }
                    }  

                    LeaveCriticalSection(&m_csSMFCStopping);
                    uNumBuf = m_pSMFCBufferManager->SetFilledBuffer();
                    EnterCriticalSection(&m_csSMFCStopping);

                    if(uNumBuf != (DWORD)-1)
                    {
                        DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: SMFC Set event %d to PP\r\n"), __WFUNCTION__,uNumBuf));
                        m_iSMFCFrameCount++;
                                                
                        // For TO2, the first frame always tearing, so drop the first frame
                        // workaroud for the issue
                        if(m_dwMemoryMode == BUFFER_MODE_DRIVER)
                        {
                            if (m_iSMFCFirstFrame == 0)
                            {
                                m_iSMFCFirstFrame++;
                                // Set the buffer from filled queue to idle queue
                                LeaveCriticalSection(&m_csSMFCStopping);
                                m_pSMFCBufferManager->GetBufferFilled();
                                EnterCriticalSection(&m_csSMFCStopping);
                                
                                // Enable interrupt bits
                                IDMACChannelIntCntrl(m_iSMFCChannel,IPU_INTR_TYPE_NFACK,TRUE);
                                LeaveCriticalSection(&m_csSMFCStopping);
                                continue;
                            }
                        }

                        //Only SMFC Instance No.1 can report buffer to PP
                        if( m_uSMFCInstance == 1)
                        {
                            // Trigger SMFCBuf EOF event to PP for Preview pin using
                            SetEvent(m_hSMFCBufEOFEvent[uNumBuf]);
                        }

                        QueryPerformanceCounter(&m_lpPerformanceCount_end);
                        LARGE_INTEGER timeCount;
                        timeCount.QuadPart = (((m_lpPerformanceCount_end.QuadPart - m_lpPerformanceCount_start.QuadPart)*1000000)/(m_lpFrequency.QuadPart));
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFC output interval is %I64d\r\n"),timeCount.QuadPart));
                        m_lpPerformanceCount_start.QuadPart  = m_lpPerformanceCount_end.QuadPart;
                        // Trigger SMFC EOF event for Still/Capture pin using
                        SetEvent(m_hCameraSMFCEOFEvent);
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFC EOF end++\r\n")));
                    }
                    else
                    {
                        if(m_iSMFCFrameCount % 100 == 1) 
                            DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: SMFC Set filled buffer failed\r\n"), __WFUNCTION__));
                    }
                    
                    // Enable interrupt bits
                    IDMACChannelIntCntrl(m_iSMFCChannel,IPU_INTR_TYPE_NFACK,TRUE);
     
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("SMFC NFACK end++\r\n")));
                }
                else
                {
                    // Clear and Enable EOF interrupt bits if using EOF  
                    
                    // Clear and Enable NFACK interrupt bits
                    IDMACChannelClearIntStatus(m_iSMFCChannel,IPU_INTR_TYPE_NFACK);
                    IDMACChannelIntCntrl(m_iSMFCChannel,IPU_INTR_TYPE_NFACK,TRUE);
                }   
            }
            
            LeaveCriticalSection(&m_csSMFCStopping);
        }

    }

    SMFC_FUNCTION_EXIT();
    return;
}

void SMFCClass::SMFCDumpIntrRegs(UINT32 ch)
{
    RETAILMSG (1, (TEXT("\n\nSMFC Interrupts Registers\r\n")));
    
    if(IDMACChannelQueryIntEnable(ch,IPU_INTR_TYPE_EOF))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_EOF enabled\r\n"),__WFUNCTION__));
    if(IDMACChannelQueryIntEnable(ch,IPU_INTR_TYPE_NFACK))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_NFACK enabled\r\n"),__WFUNCTION__));
    if(IDMACChannelQueryIntEnable(ch,IPU_INTR_TYPE_NFB4EOF))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_NFB4EOF enabled\r\n"),__WFUNCTION__));
    if(IDMACChannelQueryIntEnable(ch,IPU_INTR_TYPE_EOS))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_EOS enabled\r\n"),__WFUNCTION__));
    if(IDMACChannelQueryIntEnable(ch,IPU_INTR_TYPE_EOBND))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_EOBND enabled\r\n"),__WFUNCTION__));
    if(IDMACChannelQueryIntEnable(ch,IPU_INTR_TYPE_TH))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_TH enabled\r\n"),__WFUNCTION__));
    if(IDMACChannelQueryIntEnable(ch,IPU_INTR_TYPE_GENERAL))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_GENERAL enabled\r\n"),__WFUNCTION__));

    if(IDMACChannelGetIntStatus(ch,IPU_INTR_TYPE_EOF))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_EOF received\r\n"),__WFUNCTION__));
    if(IDMACChannelGetIntStatus(ch,IPU_INTR_TYPE_NFACK))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_NFACK received\r\n"),__WFUNCTION__));
    if(IDMACChannelGetIntStatus(ch,IPU_INTR_TYPE_NFB4EOF))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_NFB4EOF received\r\n"),__WFUNCTION__));
    if(IDMACChannelGetIntStatus(ch,IPU_INTR_TYPE_EOS))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_EOS received\r\n"),__WFUNCTION__));
    if(IDMACChannelGetIntStatus(ch,IPU_INTR_TYPE_EOBND))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_EOBND received\r\n"),__WFUNCTION__));
    if(IDMACChannelGetIntStatus(ch,IPU_INTR_TYPE_TH))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_TH received\r\n"),__WFUNCTION__));
    if(IDMACChannelGetIntStatus(ch,IPU_INTR_TYPE_GENERAL))
        RETAILMSG(1, (TEXT("IPU_INTR_TYPE_GENERAL received\r\n"),__WFUNCTION__));
}


void SMFCClass::DumpSMFCRegs(void)
{
    CMDumpRegs();
    SMFCDumpRegs();
    IDMACDumpRegs();
    CPMEMDumpRegs(0);
}


//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CsiClass.cpp
//
//  Implementation of CMOS Sensor Interface Product Device Driver
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#include <NKIntr.h>
#include <winddi.h>
#include <ddgpe.h>
#pragma warning(pop)

#include "CameraPDDProps.h"
#include "CamBufferManager.h"
#include "CsiClass.h"
#include "cameradbg.h"
#pragma warning(disable: 4100 4701)

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines

#define CSI_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define CSI_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

CsiClass::CsiClass()
{
    m_pCSI = NULL;
    m_pBufferManager = NULL;
    m_bIsCsiEnabled = FALSE;
    m_dwCsiSysIntr = (DWORD)SYSINTR_UNDEFINED;
    m_hCSIEOFEvent = NULL;
    m_hExitCsiISRThread = NULL;
    m_hCsiISRThread = NULL;
    m_hCsiIntrEvent = NULL;
    m_dwCurrentDMA = 1;
    m_dwFramerate = 0;

    CsiInit();
}
 
CsiClass::~CsiClass()
{
    CsiDeinit();
}

void CsiClass::CsiEnable(void)
{
    if (TRUE == m_bIsCsiEnabled)
        return;

    CSI_FUNCTION_ENTRY();

    // Turn Modfule Clocks On
    BSPSensorSetClockGatingMode(TRUE);

    // Enable MCLK
    m_pCSI->CSICR1 |= CSP_BITFVAL(CSI_CSICR1_MCLKEN,CSI_CSICR1_MCLKEN_ENABLE);

    m_bIsCsiEnabled = TRUE;

    CSI_FUNCTION_EXIT();
}

void CsiClass::CsiDisable(void)
{
    if (FALSE == m_bIsCsiEnabled)
        return;

    CSI_FUNCTION_ENTRY();

    // Disable MCLK
    m_pCSI->CSICR1 &= ~CSP_BITFVAL(CSI_CSICR1_MCLKEN,CSI_CSICR1_MCLKEN_ENABLE);
 
    // Turn Modfule Clocks Off
    BSPSensorSetClockGatingMode(FALSE);
    
    m_bIsCsiEnabled = FALSE;

    CSI_FUNCTION_EXIT();
}
    
BOOL CsiClass::CsiConfigureSensor(DWORD dwFramerate)
{
    CSI_FUNCTION_ENTRY();

    // Enable Sensor
    BSPEnableCamera();

    //Reset Sensor
    BSPResetCamera();

    // General control register
    m_pCSI->CSICR1 |=   CSP_BITFVAL(CSI_CSICR1_SWAP16_EN,CSI_CSICR1_SWAP16_EN_ENABLE)       |
                        CSP_BITFVAL(CSI_CSICR1_EXT_VSYNC,CSI_CSICR1_EXT_VSYNC_INTERNAL)     |
                        CSP_BITFVAL(CSI_CSICR1_EOF_INT_EN,CSI_CSICR1_EOF_INT_EN_DISABLE)    |
                        CSP_BITFVAL(CSI_CSICR1_PRP_IF_EN,CSI_CSICR1_PRP_IF_EN_DISABLE)      |
                        CSP_BITFVAL(CSI_CSICR1_CCIR_MODE,CSI_CSICR1_CCIR_MODE_PROGRESSIVE)  |
                        CSP_BITFVAL(CSI_CSICR1_COF_INT_EN,CSI_CSICR1_COF_INT_EN_DISABLE)    |
                        CSP_BITFVAL(CSI_CSICR1_SF_OR_INTEN,CSI_CSICR1_SF_OR_INTEN_DISABLE)  |
                        CSP_BITFVAL(CSI_CSICR1_RF_OR_INTEN,CSI_CSICR1_RF_OR_INTEN_DISABLE)  |
                        CSP_BITFVAL(CSI_CSICR1_SFF_DMA_DONE_INTEN,CSI_CSICR1_SFF_DMA_DONE_INTEN_DISABLE)  |
                        CSP_BITFVAL(CSI_CSICR1_STATFF_INTEN,CSI_CSICR1_STATFF_INTEN_DISABLE)|
                        CSP_BITFVAL(CSI_CSICR1_FB2_DMA_DONE_INTEN,CSI_CSICR1_FB2_DMA_DONE_INTEN_DISABLE)  |
                        CSP_BITFVAL(CSI_CSICR1_FB1_DMA_DONE_INTEN,CSI_CSICR1_FB1_DMA_DONE_INTEN_DISABLE)  |
                        CSP_BITFVAL(CSI_CSICR1_RXFF_INTEN,CSI_CSICR1_RXFF_INTEN_DISABLE)    |
                        CSP_BITFVAL(CSI_CSICR1_SOF_POL,CSI_CSICR1_SOF_POL_RISING)           |
                        CSP_BITFVAL(CSI_CSICR1_SOF_INTEN,CSI_CSICR1_SOF_INTEN_DISABLE)      |
                        CSP_BITFVAL(CSI_CSICR1_MCLKDIV,CSI_CSICR1_MCLKDIV_VALUE(0))         |
                        CSP_BITFVAL(CSI_CSICR1_HSYNC_POL,CSI_CSICR1_HSYNC_POL_HIGH)         |
                        CSP_BITFVAL(CSI_CSICR1_CCIR_EN,CSI_CSICR1_CCIR_EN_TRADITIONAL)      |
                        CSP_BITFVAL(CSI_CSICR1_MCLKEN,CSI_CSICR1_MCLKEN_ENABLE)             |
                        CSP_BITFVAL(CSI_CSICR1_FCC, CSI_CSICR1_FCC_ASYNC)                   |
                        CSP_BITFVAL(CSI_CSICR1_PACK_DIR, CSI_CSICR1_PACK_DIR_MSB)           | 
                        CSP_BITFVAL(CSI_CSICR1_GCLK_MODE,CSI_CSICR1_GCLK_MODE_GATED)        |
                        CSP_BITFVAL(CSI_CSICR1_INV_DATA,CSI_CSICR1_INV_DATA_DIRECT)         |
                        CSP_BITFVAL(CSI_CSICR1_INV_PCLK,CSI_CSICR1_INV_PCLK_DIRECT)         |
                        CSP_BITFVAL(CSI_CSICR1_REDGE,CSI_CSICR1_REDGE_RISING)               |
                        CSP_BITFVAL(CSI_CSICR1_PIXEL_BIT,CSI_CSICR1_PIXEL_BIT_8BIT)  ;

    //m_pCSI->CSICR2 |=   CSP_BITFVAL(CSI_CSICR2_DMA_BURST_TYPE_RFF,CSI_CSICR2_DMA_BURST_TYPE_RFF_INCR8);
    
    // Configure Data Input 
    switch(BSPCSIGetInputDataConfig())
    {
        case One8BitSensor:
            m_pCSI->CSICR3 |= CSP_BITFVAL(CSI_CSICR3_TWO_8BIT_SENSOR,CSI_CSICR3_TWO_8BIT_SENSOR_ONE);
            m_pCSI->CSICR1 |= CSP_BITFVAL(CSI_CSICR1_PIXEL_BIT,CSI_CSICR1_PIXEL_BIT_8BIT);
            break;
        case One10BitSensor:
            m_pCSI->CSICR3 |= CSP_BITFVAL(CSI_CSICR3_TWO_8BIT_SENSOR,CSI_CSICR3_TWO_8BIT_SENSOR_ONE);
            m_pCSI->CSICR1 |= CSP_BITFVAL(CSI_CSICR1_PIXEL_BIT,CSI_CSICR1_PIXEL_BIT_10BIT);
            break;
        case Two8BitSensors:
        case One16BitSensor:
            m_pCSI->CSICR3 |= CSP_BITFVAL(CSI_CSICR3_TWO_8BIT_SENSOR,CSI_CSICR3_TWO_8BIT_SENSOR_TWO);
            m_pCSI->CSICR1 |= CSP_BITFVAL(CSI_CSICR1_PIXEL_BIT,CSI_CSICR1_PIXEL_BIT_8BIT);              
            break;
        default:
            ERRORMSG(TRUE, (_T("%s: Invalid camera sensor input data config.\r\n"), __WFUNCTION__));
            return FALSE;
            break;
    }

    // DMA configuration register
    m_pCSI->CSICR3 |=   CSP_BITFVAL(CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_16WORDS)   |
                        CSP_BITFVAL(CSI_CSICR3_DMA_REFLASH_RFF, CSI_CSICR3_DMA_REFLASH_RFF_FLASH) |
                        CSP_BITFVAL(CSI_CSICR3_DMA_REFLASH_SFF, CSI_CSICR3_DMA_REFLASH_SFF_FLASH) ;

    // Initialize sensor setting: set sensor output framerate as 30fps
    m_dwFramerate = dwFramerate;
    BSPSetupCamera(m_dwFramerate);     

    CSI_FUNCTION_EXIT();

    return TRUE;
}

BOOL CsiClass::CsiConfigure(csiSensorOutputFormat outFormat, csiSensorOutputResolution outResolution)
{
    int BitScale = 16;

    CSI_FUNCTION_ENTRY();
 
    // Set the output format for the BSP-specific camera module.
    if (!BSPCameraSetOutputFormat(outFormat)) 
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: Invalid camera sensor output format.\r\n"), __WFUNCTION__));
        return FALSE;
    }
 
    // Set IC configuration image parameteres based on the output format
    switch(outFormat)
    {
        case csiSensorOutputFormat_RGB565:
        case csiSensorOutputFormat_RGB555:
        case csiSensorOutputFormat_YUV422_UYVY:
        case csiSensorOutputFormat_YUV422_YUY2:
             DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set BitScale = 16\r\n")));
             BitScale = 16;
             
             INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_CCIR_EN,CSI_CSICR1_CCIR_EN_TRADITIONAL);
             INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_GCLK_MODE,CSI_CSICR1_GCLK_MODE_GATED);
             // This bit only works in gated-clock¡ªthat is, GCLK_MODE = 1 and CCIR_EN = 0.
             INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_HSYNC_POL,CSI_CSICR1_HSYNC_POL_HIGH);
             INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_REDGE,CSI_CSICR1_REDGE_RISING);
             break;
            
        case csiSensorOutputFormat_YUV420:
             DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set BitScale = 12\r\n")));
             BitScale = 12;
             
             INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_CCIR_EN,CSI_CSICR1_CCIR_EN_TRADITIONAL);
             INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_GCLK_MODE,CSI_CSICR1_GCLK_MODE_GATED);
             // This bit only works in gated-clock¡ªthat is, GCLK_MODE = 1 and CCIR_EN = 0.
             INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_HSYNC_POL,CSI_CSICR1_HSYNC_POL_HIGH);
             INSREG32BF(&m_pCSI->CSICR1, CSI_CSICR1_REDGE,CSI_CSICR1_REDGE_RISING);
             break;     
          
        default:
             break;
    }
    
    // Set the output resolution for the BSP-specific camera module.
    if (!BSPCameraSetOutputResolution(m_dwFramerate,outResolution)) 
    {
        ERRORMSG(TRUE, (_T("%s: Invalid camera sensor output resolution.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Set IC configuration image parameteres based on the output resolution 
    switch (outResolution)
    {
        case csiSensorOutputResolution_1280_960:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set csiSensorOutputResolution_1280_960\r\n")));
            INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_8WORDS);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_WIDTH,   1280 * BitScale / 8);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_HEIGHT,  960);
            break;

        case csiSensorOutputResolution_1024_800:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set csiSensorOutputResolution_1024_800\r\n")));
            INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_16WORDS);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_WIDTH,   1024 * BitScale / 8);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_HEIGHT,  800);
            break;
            
        case csiSensorOutputResolution_SVGA:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set csiSensorOutputResolution_SVGA\r\n")));
            INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_16WORDS);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_WIDTH,   800 * BitScale / 8);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_HEIGHT,  600);
            break;

        case csiSensorOutputResolution_VGA:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set csiSensorOutputResolution_VGA\r\n")));
            INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_16WORDS);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_WIDTH,   640 * BitScale / 8);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_HEIGHT,  480);
            break;

        case csiSensorOutputResolution_QVGA:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set csiSensorOutputResolution_QVGA\r\n")));
            INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_16WORDS);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_WIDTH,   320 * BitScale / 8);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_HEIGHT,  240);
            break;

        case csiSensorOutputResolution_CIF:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set csiSensorOutputResolution_CIF\r\n")));
            INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_16WORDS);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_WIDTH,   352 * BitScale / 8);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_HEIGHT,  288);
            break;

        case csiSensorOutputResolution_QCIF:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set csiSensorOutputResolution_QCIF\r\n")));
            INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_8WORDS);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_WIDTH,   176 * BitScale / 8);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_HEIGHT,  144);
            break;

        case csiSensorOutputResolution_QQVGA:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set csiSensorOutputResolution_QQVGA\r\n")));
            INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_16WORDS);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_WIDTH,   160 * BitScale / 8);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_HEIGHT,  120);
            break;

        case csiSensorOutputResolution_QQCIF:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiConfigure: Set csiSensorOutputResolution_QQCIF\r\n")));
            INSREG32BF(&m_pCSI->CSICR3, CSI_CSICR3_RXFF_LEVEL, CSI_CSICR3_RXFF_LEVEL_8WORDS);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_WIDTH,   88 * BitScale / 8);
            INSREG32BF(&m_pCSI->CSIIMAGPARA, CSI_CSIIMAGPARA_IMAGE_HEIGHT,  72);
            break;
            
        default:
            break;
    }

    CSI_FUNCTION_EXIT();

    return TRUE;
}
        
void CsiClass::CsiZoom(DWORD zoomVal)
{
    CSI_FUNCTION_ENTRY();

    BSPSetDigitalZoom(zoomVal);

    CSI_FUNCTION_EXIT();
}

void CsiClass::CsiChangeFrameRate(DWORD rate)
{
    CSI_FUNCTION_ENTRY();

    if (rate == m_dwFramerate)
        return;
    
    // MCLK is 24M, and the output framerate based on sensor config
    // Sensor output SVGA for 30fps
    // Sensor output UXGA for 15fps
    m_dwFramerate = rate;
    BSPSetupCamera(m_dwFramerate);

    CSI_FUNCTION_EXIT();
}

DWORD CsiClass::CsiGetFrameCount(void)
{
    return CSP_BITFEXT(INREG32(&m_pCSI->CSICR3), CSI_CSICR3_FRMCNT);
}
   
static DWORD WINAPI CsiIntrThread(LPVOID lpParameter)
{
    CsiClass *pCSI = (CsiClass *)lpParameter;

    return pCSI->CsiISRLoop(INFINITE);
}

void CsiClass::CsiInit(void)
{
    CSI_FUNCTION_ENTRY();

    PHYSICAL_ADDRESS phyAddr;
    DWORD irq;

    phyAddr.QuadPart = CSP_BASE_REG_PA_CSI;

    // Map peripheral physical address to virtual address
    m_pCSI = (PCSP_CSI_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_CSI_REGS), FALSE); 
        
    // Check if virtual mapping failed
    if (m_pCSI == NULL)
    {
        ERRORMSG(TRUE, (_T("%s:  MmMapIoSpace failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Configure CSI controller pins
    if (!BSPCSIIOMUXConfig())
    {
        ERRORMSG(TRUE, (TEXT("%s: Error configuring GPIO for CSI.\r\n"), __WFUNCTION__));
        return;
    }

    // Get camera-in-use from registry key. Otherwise use default
    if (!BSPGetDefaultCameraFromRegistry())
    {
        ERRORMSG(TRUE, (TEXT("%s: Get camera from registry failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Initialize the Buffer Manager
    m_pBufferManager = new CamBufferManager();
    if (m_pBufferManager == NULL)
    {
        ERRORMSG(TRUE, (_T("%s: Error while instantiating the CamBufferManager\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Events to signal pin that frame is ready
    m_hCSIEOFEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hCSIEOFEvent == NULL)
    {
        ERRORMSG(TRUE, (_T("%s: CreateEvent failed for Frame EOF\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Event to trigger ISR
    m_hCsiIntrEvent = CreateEvent ( NULL, FALSE, FALSE, L"INTR_EVENT_CSI") ;
    if(NULL == m_hCsiIntrEvent)
    {
        ERRORMSG(TRUE, (_T("%s: CreateEvent failed for CSI Interrupt event\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Event to Exit CsiISRThread
    m_hExitCsiISRThread = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;
    if(m_hExitCsiISRThread == NULL)
    {
        ERRORMSG(TRUE, (_T("%s: CreateEvent failed for exiting CsiISRThread\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Translate IRQ into SYSINTR
    irq = IRQ_CSI;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(irq), &m_dwCsiSysIntr, sizeof(m_dwCsiSysIntr), NULL))
    {
        ERRORMSG(TRUE, (_T("%s: CSI Sysintr request failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Register interrupt
    if (!InterruptInitialize(m_dwCsiSysIntr, m_hCsiIntrEvent, NULL, 0))
    {
        ERRORMSG(TRUE, (_T("InterruptInitialize failed for CSI SYSINTR! - Error=%d\r\n"), GetLastError()));
        goto Error;
    }

    // Create ISR Handler thread
    m_hCsiISRThread = CreateThread(NULL, 0, CsiIntrThread, this, 0, NULL);
    if (m_hCsiISRThread == NULL)
    {
        ERRORMSG(TRUE, (_T("%s: CreateThread for CSI ISR failed\r\n"), __WFUNCTION__));
        goto Error;
    }

#ifdef SENNA_FIX_MEM_ALLOC_ISSUE
    // Allocate buffers
    CsiAllocateBuffers(NUM_PIN_BUFFER, MAX_IMAGE_SIZE);
#endif

    CSI_FUNCTION_EXIT();

    return;
 
Error:
    CsiDeinit();

    CSI_FUNCTION_EXIT();

    return;
}

void CsiClass::CsiDeinit(void)
{
    CSI_FUNCTION_ENTRY();

    CsiDisable();

#ifdef SENNA_FIX_MEM_ALLOC_ISSUE
    CsiDeleteBuffers();
#endif

    if (m_hCSIEOFEvent)
    {
        CloseHandle(m_hCSIEOFEvent);
        m_hCSIEOFEvent = NULL;
    }

    if (m_hCsiISRThread)
    {
        // Set CsiISRThread end
        SetEvent( m_hExitCsiISRThread ); 
        // Wait for m_hExitCsiISRThread end
        WaitForSingleObject(m_hExitCsiISRThread,INFINITE);

        CloseHandle(m_hCsiISRThread);
        m_hCsiISRThread = NULL;
    }

    if (m_hExitCsiISRThread)
    {   
        CloseHandle(m_hExitCsiISRThread);
        m_hExitCsiISRThread = NULL;
    }

    if (m_dwCsiSysIntr != (DWORD)SYSINTR_UNDEFINED)
    {
        InterruptDisable(m_dwCsiSysIntr);
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwCsiSysIntr, sizeof(DWORD), NULL, 0, NULL);
        m_dwCsiSysIntr = (DWORD)SYSINTR_UNDEFINED;
    }

    if (m_hCsiIntrEvent)
    {
        CloseHandle(m_hCsiIntrEvent);
        m_hCsiIntrEvent = NULL;
    }

    if (m_pCSI)
    {
        MmUnmapIoSpace(m_pCSI, sizeof(CSP_CSI_REGS));
        m_pCSI = NULL;
    }

    if (m_pBufferManager)
    {
        delete m_pBufferManager;
        m_pBufferManager = NULL;
    }

    BSPDeleteCamera();

    CSI_FUNCTION_EXIT();
}

BOOL CsiClass::CsiAllocateBuffers(ULONG numBuffers, ULONG bufSize)
{
    CSI_FUNCTION_ENTRY();

    // Initialize MsgQueues when buffers are allocated
    if (!m_pBufferManager->AllocateBuffers(numBuffers, bufSize))
    {
        ERRORMSG(TRUE, (TEXT("%s :CSI Buffer allocationfailed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CSI_FUNCTION_EXIT();

    return TRUE;
}

BOOL CsiClass::CsiDeleteBuffers()
{
    CSI_FUNCTION_ENTRY();

    // Make sure all buffers are idle before deleting them
    m_pBufferManager->ResetBuffers();

    // Delete CSI buffers.
    if (!m_pBufferManager->DeleteBuffers())
    {
        ERRORMSG(TRUE, (TEXT("%s : Failed to delete CSI buffers!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CSI_FUNCTION_EXIT();

    return TRUE;
}

BOOL CsiClass::RegisterBuffer(ULONG bufSize, LPVOID pVirtualAddress, UINT32 pPhysicalAddress)
{
    CSI_FUNCTION_ENTRY();

    // Register a CSI buffer
    if (!m_pBufferManager->RegisterBuffer(pVirtualAddress, pPhysicalAddress))
    {
        ERRORMSG(TRUE, (TEXT("%s :Register Buffer failed!\r\n"), __WFUNCTION__));
        return FALSE;  
    }

    CSI_FUNCTION_EXIT();

    return TRUE;
}

BOOL CsiClass::UnregisterBuffer(LPVOID pVirtualAddress, UINT32 pPhysicalAddress)
{
    return m_pBufferManager->UnregisterBuffer(pVirtualAddress, pPhysicalAddress);
}

BOOL CsiClass::Enqueue(void)
{
    // Enqueue a CSI buffer
    return m_pBufferManager->EnQueue();
}

UINT32* CsiClass::GetBufFilled(void)
{
    // Return a pointer to a filled buffer
    return m_pBufferManager->GetBufferFilled();
}

BOOL CsiClass::CsiStartChannel(void)
{
    CSI_FUNCTION_ENTRY();
    
    UINT32* pPhysBufAddr = NULL; 

    // Make sure all the buffers are idle
    m_pBufferManager->ResetBuffers();

    // Reset events
    ResetEvent(m_hCSIEOFEvent);

    // Reset the Frame Counter
    m_pCSI->CSICR3 |=   CSP_BITFVAL(CSI_CSICR3_FRMCNT_RST,CSI_CSICR3_FRMCNT_RESET_ENABLE);

    // Retrieve an idle buffer for DMA1
    if (m_pBufferManager->SetActiveBuffer(&pPhysBufAddr, INFINITE) < 0)
    {
        return FALSE;
    }

    // Pass its address to the CSI DMA controller
    m_pCSI->CSIDMASAFB1 = (UINT32)pPhysBufAddr;

    // Retrieve an idle buffer for DMA2
    if (m_pBufferManager->SetActiveBuffer(&pPhysBufAddr, INFINITE) < 0)
    {
        return FALSE;
    }

    // Pass its address to the CSI DMA controller
    m_pCSI->CSIDMASAFB2 = (UINT32)pPhysBufAddr;
    
    // Set current DMA to DMA1
    m_dwCurrentDMA = 1;

    // Clear interrupt status               
    OUTREG32(&m_pCSI->CSISR, INREG32(&m_pCSI->CSISR));

    // Start the DMA channel
    m_pCSI->CSICR3 |=   CSP_BITFVAL(CSI_CSICR3_DMA_REQ_EN_RFF, CSI_CSICR3_DMA_REQ_EN_RFF_ENABLE);

    // Wait for the first Start-Of-Frame
    {
        volatile DWORD SOFTimeout = 1000000;
        while((CSP_BITFEXT(INREG32(&m_pCSI->CSISR), CSI_CSISR_SOF_INT) != CSI_CSISR_SOF_INT_W1L) &&
              (SOFTimeout-- > 0));
    }

    // Reset the Rx FIFO
    m_pCSI->CSICR1 |=   CSP_BITFVAL(CSI_CSICR1_CLR_RXFIFO,CSI_CSICR1_CLR_RXFIFO_CLEAR);

    // Reset the DMA controller
    m_pCSI->CSICR3 |=   CSP_BITFVAL(CSI_CSICR3_DMA_REFLASH_RFF, CSI_CSICR3_DMA_REFLASH_RFF_FLASH) ;

    // Enable interrupts
    m_pCSI->CSICR1 |=   CSP_BITFVAL(CSI_CSICR1_FB1_DMA_DONE_INTEN,CSI_CSICR1_FB1_DMA_DONE_INTEN_ENABLE)     |
                        CSP_BITFVAL(CSI_CSICR1_FB2_DMA_DONE_INTEN,CSI_CSICR1_FB2_DMA_DONE_INTEN_ENABLE)     |
                        CSP_BITFVAL(CSI_CSICR1_RF_OR_INTEN,CSI_CSICR1_RF_OR_INTEN_ENABLE)                   ;

    CSI_FUNCTION_EXIT();

    return TRUE;
}

BOOL CsiClass::CsiStopChannel(void)
{
    CSI_FUNCTION_ENTRY();

    // Stop the DMA channel
    m_pCSI->CSICR3 &=   ~CSP_BITFVAL(CSI_CSICR3_DMA_REQ_EN_RFF, CSI_CSICR3_DMA_REQ_EN_RFF_ENABLE);  

    // Disable interrupts
    m_pCSI->CSICR1 &= ~(CSP_BITFVAL(CSI_CSICR1_FB1_DMA_DONE_INTEN,CSI_CSICR1_FB1_DMA_DONE_INTEN_ENABLE) |
                        CSP_BITFVAL(CSI_CSICR1_FB2_DMA_DONE_INTEN,CSI_CSICR1_FB2_DMA_DONE_INTEN_ENABLE) |
                        CSP_BITFVAL(CSI_CSICR1_RF_OR_INTEN,CSI_CSICR1_RF_OR_INTEN_ENABLE)               );

    // Clear interrupt status               
    OUTREG32(&m_pCSI->CSISR, INREG32(&m_pCSI->CSISR));

    m_pCSI->CSIDMASAFB1 = 0xFFFFFFFF;
    m_pCSI->CSIDMASAFB2 = 0xFFFFFFFF;

    CSI_FUNCTION_EXIT();

    return TRUE;
}

void CsiClass::PrintBufferInfo(void)
{
    m_pBufferManager->PrintBufferInfo();
}

BOOL CsiClass::CsiISRLoop(UINT32 timeout)
{
    BOOL bRet = TRUE;
    DWORD dwStatus, index;
    UINT32* pPhysBufAddr = NULL;
    REG32 *pDMAReg;
    DWORD dwMask;
    DWORD dwResult = 0;

    for (;;)
    {
        if ( WaitForSingleObject ( m_hExitCsiISRThread, 0 ) != WAIT_TIMEOUT )
        {
            ExitThread(1);
        }

        if ( WaitForSingleObject(m_hCsiIntrEvent, INFINITE) == WAIT_OBJECT_0 )
        {
            // Retrieve interrupt status
            dwStatus = INREG32(&m_pCSI->CSISR);

            index = m_dwCurrentDMA;

            while (dwStatus & (CSP_BITFMASK(CSI_CSISR_DMA_TSF_DONE_FB1) | CSP_BITFMASK(CSI_CSISR_DMA_TSF_DONE_FB2)))
            {
                if (index == 1)
                {
                    dwMask = CSP_BITFMASK(CSI_CSISR_DMA_TSF_DONE_FB1);
                    pDMAReg = &(m_pCSI->CSIDMASAFB1);
                }
                else
                {
                    dwMask = CSP_BITFMASK(CSI_CSISR_DMA_TSF_DONE_FB2);
                    pDMAReg = &(m_pCSI->CSIDMASAFB2);
                }

                if (dwStatus & dwMask)
                {
                    // Retrieve an idle buffer
                    dwResult = m_pBufferManager->SetActiveBuffer(&pPhysBufAddr, INFINITE);

                    if(dwResult == -1)
                    {
                        RETAILMSG(1,(TEXT("CsiClass::CsiISRLoop SetActiveBuffer failed! There are some thing wrong...")));
                    }
                    else
                    {
                        // Pass its address to the CSI DMA controller
                        *pDMAReg = (UINT32)pPhysBufAddr;
                    }

                    // Clear interrupt status
                    OUTREG32(&m_pCSI->CSISR, dwMask);

                    // Move the buffer from BUSY to FILL
                    m_pBufferManager->SetFilledBuffer();

                }
                index = (index + 1) % 2;

                dwStatus = INREG32(&m_pCSI->CSISR);
            }
            m_dwCurrentDMA = index;

            // Notify the upper layer of the received frame
            SetEvent(m_hCSIEOFEvent); 

            if (CSP_BITFEXT(dwStatus, CSI_CSISR_RFF_OR_INT) == CSI_CSISR_RF_OR_INT_W1L)
            {
                DEBUGMSG(ZONE_FUNCTION,(TEXT("CsiISRLoop: CSI DMA receive overrun occured, resetting DMA channel\r\n")));

                // Reset the Rx FIFO
                m_pCSI->CSICR1 |=   CSP_BITFVAL(CSI_CSICR1_CLR_RXFIFO,CSI_CSICR1_CLR_RXFIFO_CLEAR);
            }

            // Clear interrupt status               
            OUTREG32(&m_pCSI->CSISR, dwStatus);

            InterruptDone(m_dwCsiSysIntr);
        }
        else 
        {
            // Abnormal signal
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}

void CsiClass::dumpCsiRegisters(void)
{
    RETAILMSG (1, (TEXT("DumpCsiRegisters!\r\n")));
    RETAILMSG (1, (TEXT("dumpCsiRegister: CSICR1: %x\r\n"),  INREG32(&m_pCSI->CSICR1)));
    RETAILMSG (1, (TEXT("dumpCsiRegister: CSICR2: %x\r\n"),  INREG32(&m_pCSI->CSICR2)));
    RETAILMSG (1, (TEXT("dumpCsiRegister: CSICR3: %x\r\n"),  INREG32(&m_pCSI->CSICR3)));
    RETAILMSG (1, (TEXT("dumpCsiRegister: CSIFBUFPARA: %x\r\n"), INREG32(&m_pCSI->CSIFBUFPARA)));
    RETAILMSG (1, (TEXT("dumpCsiRegister: CSIIMAGPARA: %x\r\n"), INREG32(&m_pCSI->CSIIMAGPARA)));
    return;
}


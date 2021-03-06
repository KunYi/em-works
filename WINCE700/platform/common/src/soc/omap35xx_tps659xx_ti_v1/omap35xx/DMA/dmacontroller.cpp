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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: dmacontroller.cpp
//

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <pm.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include "_debug.h"
#include "dmacontroller.h" 
#include "dmamediator.h"

// dma mapping
// L0 --> SystemDmaController::DmaMediator
// L2 --> CameraDmaController::DmaMediator

//------------------------------------------------------------------------------
BOOL 
DmaControllerBase::Initialize(
    PHYSICAL_ADDRESS const& pa, 
    DWORD size
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"+DmaControllerBase::Initialize\r\n"));

    BOOL rc = FALSE;

    ASSERT(m_pDmaMediator == NULL);
    m_pDmaMediator = new DmaMediator();
    if (m_pDmaMediator == NULL)
        {
        RETAILMSG(ZONE_ERROR, (L"ERROR: DmaControllerBase::Initialize: "
            L"Failed to allocate DmaMediator\r\n"
            ));
        goto cleanUp;
        }

    // initialize observers
    if (m_pDmaMediator->Initialize(m_maxChannels, m_rgChannels) == FALSE)
        {
        goto cleanUp;
        }

    m_rgChannelPhysAddr = (DWORD*)LocalAlloc(LPTR, sizeof(DWORD) * m_maxChannels);
    if (m_rgChannelPhysAddr == NULL)
        {
        RETAILMSG(ZONE_ERROR, (L"ERROR: DmaControllerBase::Initialize: "
            L"Failed to allocate array\r\n"
            ));
        goto cleanUp;
        }

    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-DmaControllerBase::Initialize"
        L"(rc=%d)\r\n", rc
        ));

    if (rc == FALSE) Uninitialize();    
    return rc;
}


//------------------------------------------------------------------------------
void
DmaControllerBase::Uninitialize()
{
    DEBUGMSG(ZONE_FUNCTION, (L"+DmaControllerBase::Uninitialize\r\n"));

    if (m_pDmaMediator != NULL)
        {
        m_pDmaMediator->Uninitialize();
        delete m_pDmaMediator;
        m_pDmaMediator = NULL;
        }
    
    if (m_rgChannelPhysAddr != NULL)
        {
        LocalFree(m_rgChannelPhysAddr);
        m_rgChannelPhysAddr = NULL;
        }

    DEBUGMSG(ZONE_FUNCTION, (L"-DmaControllerBase::Uninitialize\r\n"));
}


//------------------------------------------------------------------------------
BOOL 
DmaControllerBase::ReserveChannel(DmaChannelContext_t *pContext)
{
    DEBUGMSG(ZONE_FUNCTION, (L"+DmaControllerBase::ReserveChannel\r\n"));

    DWORD rc = 0;

    // check map for an unused channel
    int i = FindUnusedChannel();
    if (i == -1)
        {
        DEBUGMSG(ZONE_WARN, (L"WARN: DmaControllerBase::ReserveChannel: "
            L"All DMA channels in use!!!!!!\r\n"
            ));
        goto cleanUp;
        }

    // return physical address of channel    
    ASSERT(i < m_maxChannels);
    pContext->index = i;
    pContext->channelAddress = m_rgChannelPhysAddr[i];
    pContext->registerSize = sizeof(OMAP_DMA_LC_REGS);

    // reset interrupts for logical channel
    EnableInterrupt(i, FALSE);
     
    rc = TRUE;    
cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-DmaControllerBase::ReserveChannel"
        L"(0x%08X)\r\n", rc
        ));
    return rc;
}


//------------------------------------------------------------------------------
void 
DmaControllerBase::ReleaseChannel(DmaChannelContext_t const *pContext)
{
    DEBUGMSG(ZONE_FUNCTION, (L"+DmaControllerBase::ReleaseChannel"
        L"(0x%08X)\r\n",pContext
        ));

    if (pContext->index >= m_maxChannels)
        {
        RETAILMSG(ZONE_ERROR, (L"ERROR! DmaControllerBase::ReleaseChannel: "
            L"invalid index(%d) is used to release dma channel",
            pContext->index
            ));
        goto cleanUp;
        }

    // stop dma channel
    ASSERT(m_rgChannels != NULL);  
    EnableInterrupt(pContext->index, FALSE);
    m_pDmaMediator->ResetChannel(pContext->index);
    ResetUsedChannel(pContext->index);
    
cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-DmaControllerBase::ReleaseChannel\r\n"));
}


//------------------------------------------------------------------------------
BOOL 
DmaControllerBase::StartInterruptThread(
    DWORD irq, 
    DWORD priority)
{
    DEBUGMSG(ZONE_FUNCTION, (L"+DmaControllerBase::StartInterruptThread: "
        L"(irq=%d, priority=%d)\r\n", irq, priority
        ));

    BOOL rc = FALSE;
    rc = m_pDmaMediator->StartInterruptHandler(irq, priority);
    
    DEBUGMSG(ZONE_FUNCTION, (L"-DmaControllerBase::StartInterruptThread: "
        L"rc=%d\r\n", rc
        ));
    return rc;
}


//------------------------------------------------------------------------------
BOOL 
DmaControllerBase::SetSecondaryInterruptHandler(
    DmaChannelContext_t const *pContext, 
    HANDLE hEvent,
    DWORD processId
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"+DmaControllerBase::SetSecondaryInterruptHandler: "
        L"context=0x%08X, hEvent=0x%08X, processId=0x%08X\r\n", 
        pContext, hEvent, processId
        ));

    BOOL rc = FALSE;

    // verify the context is valid
    int index = pContext->index;
    if (index >= m_maxChannels)
        {
        RETAILMSG(ZONE_ERROR, (L"DmaControllerBase::SetSecondaryInterruptHandler: "
            L"invalid index (0x%08X)", index
            ));
        goto cleanUp;
        }
    
    // enable/disable repreat mode   
    rc = m_pDmaMediator->SetEventHandler(index, hEvent, processId);

cleanUp:    
    DEBUGMSG(ZONE_FUNCTION, (L"-DmaControllerBase::SetSecondaryInterruptHandler\r\n"));
    return rc;
}


//------------------------------------------------------------------------------
BOOL 
DmaControllerBase::InterruptDone(
    DmaChannelContext_t const *pContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"+DmaControllerBase::InterruptDone: "
        L"context=0x%08X\r\n", pContext
        ));

    BOOL rc = FALSE;

    // verify the context is valid
    int index = pContext->index;
    if (index >= m_maxChannels)
        {
        RETAILMSG(ZONE_ERROR, (L"DmaControllerBase::GetLastInterruptStatus: "
            L"invalid index (0x%08X)", index
            ));
        goto cleanUp;
        }
    
    // enable interrupt
    rc = m_pDmaMediator->InterruptDone(index); 

cleanUp:    
    DEBUGMSG(ZONE_FUNCTION, (L"-DmaControllerBase::InterruptDone\r\n"));
    return rc;
}

//------------------------------------------------------------------------------
BOOL 
SystemDmaController::Initialize(
    PHYSICAL_ADDRESS const& pa, 
    DWORD size
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"+SystemDmaController::Initialize\r\n"));

    BOOL rc = FALSE;

    InitializeCriticalSection(&m_cs);
    
    // map virtual address to hardware register
    m_pDmaController = (OMAP_SDMA_REGS*)MmMapIoSpace(pa, size, FALSE
                                                );
    if (m_pDmaController == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SystemDmaController::Initialize: "
            L"Failed map System DMA controller registers\r\n"
            ));
        goto cleanUp;
        }
    m_registerSize = size;

    // set pointer to dma logical channels
    m_rgChannels = m_pDmaController->CHNL_CTRL;

    if (DmaControllerBase::Initialize(pa, size) == FALSE)
        {
        goto cleanUp;
        }    

    // set dma controller
    m_pDmaMediator->SetDmaController((HANDLE)1, this);

    // init disable standby count
    m_dwDisableStandbyCount=0;

    // populate physical address of logical dma channel into array
    DWORD ChannelPhysAddr = pa.LowPart + offset(OMAP_SDMA_REGS, CHNL_CTRL);
    for (int i = 0; i < m_maxChannels; ++i)
        {
        m_rgChannelPhysAddr[i] = ChannelPhysAddr;
        ChannelPhysAddr += sizeof(OMAP_DMA_LC_REGS);
        }

    // reset dma interrupts and status
    OUTREG32(&m_pDmaController->DMA4_IRQENABLE_L0, 0x00000000);
    OUTREG32(&m_pDmaController->DMA4_IRQSTATUS_L0, 0xFFFFFFFF);

    // AUTO IDLE mode
    OUTREG32(&m_pDmaController->DMA4_OCP_SYSCONFIG, 
        SYSCONFIG_AUTOIDLE | SYSCONFIG_SMARTSTANDBY | SYSCONFIG_SMARTIDLE
        );
    
    OUTREG32(&m_pDmaController->DMA4_GCR, 
        DMA_GCR_ARBITRATION_RATE(0xFF) | DMA_GCR_HITHREAD_RSVP(0) |
        DMA_GCR_FIFODEPTH(64)
        );

    // indicate DMA registers need to be saved for OFF mode
    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_DMA);
    
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-SystemDmaController::Initialize"
        L"(rc=%d)\r\n", rc
        ));
    
    if (rc == FALSE) Uninitialize();
    return rc;
}


//------------------------------------------------------------------------------
void 
SystemDmaController::Uninitialize()
{
    DEBUGMSG(ZONE_FUNCTION, (L"+SystemDmaController::Uninitialize\r\n"));
    
    DmaControllerBase::Uninitialize();
    if (m_pDmaController != NULL)
        {
        MmUnmapIoSpace((void*)m_pDmaController, m_registerSize);
        m_rgChannels = NULL;
        m_pDmaController = NULL;
        m_registerSize = 0;
        }

    DeleteCriticalSection(&m_cs);

    DEBUGMSG(ZONE_FUNCTION, (L"-SystemDmaController::Uninitialize\r\n"));
}
 

//------------------------------------------------------------------------------
void 
SystemDmaController::EnableInterrupt(
    int index, 
    BOOL bEnable
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"+SystemDmaController::EnableInterrupt: "
        L"i=0x%08X, bEnable=0x%08X\r\n", index, bEnable
        ));
    
    // enable/disable DMA interrupts
    DWORD mask = (1 << index);

    EnterCriticalSection(&m_cs);
    SETREG32(&m_pDmaController->DMA4_IRQSTATUS_L0, mask);
    if (bEnable == TRUE)
        {
        SETREG32(&m_pDmaController->DMA4_IRQENABLE_L0, mask);        
        }
    else
        {
        CLRREG32(&m_pDmaController->DMA4_IRQENABLE_L0, mask);        
        }
    LeaveCriticalSection(&m_cs);

    // indicate DMA registers need to be saved for OFF mode
    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_DMA);

    DEBUGMSG(ZONE_FUNCTION, (L"-SystemDmaController::EnableInterrupt\r\n"));
}

//------------------------------------------------------------------------------
BOOL
SystemDmaController::GetInterruptMask(
    HANDLE context, 
    DWORD *pdwMask
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"+SystemDmaController::GetInterruptMask: "
        L"context=0x%08X, pdwMask=0x%08X\r\n", context, pdwMask
        ));
    
    // get enabled, pending DMA interrupts
    EnterCriticalSection(&m_cs);
    *pdwMask = INREG32(&m_pDmaController->DMA4_IRQSTATUS_L0);
    *pdwMask &= INREG32(&m_pDmaController->DMA4_IRQENABLE_L0);
    LeaveCriticalSection(&m_cs);

    DEBUGMSG(ZONE_FUNCTION, (L"-SystemDmaController::GetInterruptMaskS\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
SystemDmaController::ClearInterruptMask(
    HANDLE context, 
    DWORD dwMask
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"+SystemDmaController::ClearInterruptMask: "
        L"context=0x%08X, dwMask=0x%08X\r\n", context, dwMask
        ));
        
    // disable interrupts
    EnterCriticalSection(&m_cs);
    CLRREG32(&m_pDmaController->DMA4_IRQENABLE_L0, dwMask);
    
    // reset status bits
    OUTREG32(&m_pDmaController->DMA4_IRQSTATUS_L0, dwMask);
    LeaveCriticalSection(&m_cs);

    // indicate DMA registers need to be saved for OFF mode
    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_DMA);

    DEBUGMSG(ZONE_FUNCTION, (L"-SystemDmaController::ClearInterruptMask\r\n"));
    return TRUE;
}


//------------------------------------------------------------------------------
BOOL
SystemDmaController::DisableStandby(
    HANDLE context, 
    BOOL noStandby
    )
{
    ULONG ocpSysconfig;

    DEBUGMSG(ZONE_FUNCTION, (L"+SystemDmaController::DisableStandby: "
        L"context=0x%08X, noStandby=0x%08X\r\n", context, noStandby
        ));

    EnterCriticalSection(&m_cs);
    if (noStandby)
        {
        if (m_dwDisableStandbyCount++ == 0)
            {
            ocpSysconfig = INREG32(&m_pDmaController->DMA4_OCP_SYSCONFIG);
            ocpSysconfig &= ~(SYSCONFIG_IDLE_MASK | SYSCONFIG_STANDBY_MASK);
            ocpSysconfig |= (SYSCONFIG_NOIDLE | SYSCONFIG_NOSTANDBY);
            OUTREG32(&m_pDmaController->DMA4_OCP_SYSCONFIG, ocpSysconfig);
            }
        }
    else
        {
        // assumes Standby is only manipulated in this class
        // in the Initialize function and in this function
        if (--m_dwDisableStandbyCount == 0)
            {
            ocpSysconfig = INREG32(&m_pDmaController->DMA4_OCP_SYSCONFIG);
            ocpSysconfig &= ~(SYSCONFIG_IDLE_MASK | SYSCONFIG_STANDBY_MASK);
            ocpSysconfig |= (SYSCONFIG_SMARTIDLE | SYSCONFIG_SMARTSTANDBY);
            OUTREG32(&m_pDmaController->DMA4_OCP_SYSCONFIG, ocpSysconfig);
            }
        }
    LeaveCriticalSection(&m_cs);

    DEBUGMSG(ZONE_FUNCTION, (L"-SystemDmaController::DisableStandby\r\n"));
    return TRUE;
}


//------------------------------------------------------------------------------


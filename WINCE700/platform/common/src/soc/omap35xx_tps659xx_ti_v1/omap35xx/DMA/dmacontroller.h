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
//  File: dmacontroller.h
//

#ifndef __DMACONTROLLER_H
#define __DMACONTROLLER_H
  

class DmaMediator;

//------------------------------------------------------------------------------
//  class DmaControllerBase
//
//  base class for all dma controller's
//
class DmaControllerBase
{ 
friend DmaMediator;

    //--------------------------------------------------------------------------
    // member variables
    //
protected:
    DWORD                       m_ffDmaChannels;
    int                         m_maxChannels;
    DmaMediator                *m_pDmaMediator;
    DWORD                      *m_rgChannelPhysAddr;
    OMAP_DMA_LC_REGS           *m_rgChannels;
    DWORD                       m_dwDisableStandbyCount;


    //--------------------------------------------------------------------------
    // constructor
    //
public:
    DmaControllerBase(int count) : m_maxChannels(count), m_ffDmaChannels(0), 
                                   m_rgChannelPhysAddr(NULL), 
                                   m_rgChannels(NULL), 
                                   m_pDmaMediator(NULL)
    {
    }


    //--------------------------------------------------------------------------
    // internal methods
    //
protected:
    int FindUnusedChannel()
    {
        // if mask is zero then channel is available
        int i = 0;
        int ff = 1;
        while ((ff & m_ffDmaChannels) && (i < m_maxChannels))
            {
            ff <<= 1;
            ++i;
            }
        
        if (i >= m_maxChannels)
            {
            ff = 0;
            i = -1;
            }
        
        m_ffDmaChannels |= ff;        
        return i;
    }

    void ResetUsedChannel(int channel)
    {
        // debug check to see if channel was actually reserved
        ASSERT((1 << channel) & m_ffDmaChannels);
        m_ffDmaChannels &= ~(1 << channel); 
    }

    //--------------------------------------------------------------------------
    // internal methods
    //
protected:

    //--------------------------------------------------------------------------
    // EnableInterrupt
    //  subclasses should enable/disable interrupts for the logical channel
    //
    virtual void EnableInterrupt(int index, BOOL bEnable) = 0;

    //--------------------------------------------------------------------------
    // GetInterruptMask
    //  retrieves a mask indicating which logical channels caused the
    //  interrupt
    //
    virtual BOOL GetInterruptMask(HANDLE context, DWORD *pdwMask) = 0;

    //--------------------------------------------------------------------------
    // ClearInterruptMask
    //  clears the interrupt mask
    //
    virtual BOOL ClearInterruptMask(HANDLE context, DWORD dwMask) = 0;

    //--------------------------------------------------------------------------
    // public methods
    //
public:

    //--------------------------------------------------------------------------
    // SetSecondaryInterruptHandler
    //  enables the dma interrupts for a dma logical channel.  Interrupt for
    //  a dma channel is disabled by passing in a NULL event handle
    //
    BOOL SetSecondaryInterruptHandler(DmaChannelContext_t const * pContext, 
            HANDLE hEvent, DWORD processId);

    //--------------------------------------------------------------------------
    // InterruptDone
    //  resets the interrupt
    //
    BOOL InterruptDone(DmaChannelContext_t const *pContext);

    //--------------------------------------------------------------------------
    // StartInterruptThread
    //  Initializes the class
    //    
    virtual BOOL StartInterruptThread(DWORD irq, DWORD priority);

    //--------------------------------------------------------------------------
    // ReserveChannel
    //  returns -1 if failed no dma channel is available else returns the
    //  physical address of the dma channel
    //
    virtual BOOL ReserveChannel(DmaChannelContext_t *pContext);

    //--------------------------------------------------------------------------
    // ReleaseChannel
    //  marks the dma channel available for future reserve requests.  The
    //  <channel> parameter should be the physical address of the dma channel 
    //  being released
    //
    virtual void ReleaseChannel(DmaChannelContext_t const *pContext);    
    
    //--------------------------------------------------------------------------
    // Uninitialize
    //  Prepares object for shutdown
    // 
    virtual void Uninitialize();
    
    //--------------------------------------------------------------------------
    // Initialize
    //  Initializes the class
    //    
    virtual BOOL Initialize(PHYSICAL_ADDRESS const& pa, DWORD size);    

    //--------------------------------------------------------------------------
    // DisableStandby
    //  disable standby in OCP_SYSCONFIG
    //
    virtual BOOL DisableStandby(HANDLE context, BOOL noStandby) = 0;   

};


//------------------------------------------------------------------------------
//  class SystemDmaController
//
//  System DMA controller
//
class SystemDmaController : public DmaControllerBase
{
    //--------------------------------------------------------------------------
    // member variables
    //
protected:

    DWORD                       m_registerSize;
    OMAP_SDMA_REGS*             m_pDmaController;
    CRITICAL_SECTION            m_cs;    

    //--------------------------------------------------------------------------
    // constructor/destructor
    //
public:

    SystemDmaController() : DmaControllerBase(OMAP_DMA_LOGICAL_CHANNEL_COUNT),
                            m_pDmaController(NULL)
    {
    }
    
    ~SystemDmaController()
    {
        Uninitialize();
    }

    //--------------------------------------------------------------------------
    // internal methods
    //
protected:

    //--------------------------------------------------------------------------
    // EnableInterrupt
    //  subclasses should enable/disable interrupts for the logical channel
    //
    virtual void EnableInterrupt(int index, BOOL bEnable);
    
    //--------------------------------------------------------------------------
    // external methods
    //
public:

    virtual void Uninitialize();
    virtual BOOL Initialize(PHYSICAL_ADDRESS const& pa, DWORD size);
    virtual BOOL GetInterruptMask(HANDLE context, DWORD *pdwMask);
    virtual BOOL ClearInterruptMask(HANDLE context, DWORD dwMask);
    virtual BOOL DisableStandby(HANDLE context, BOOL noStandby);
};



#endif // __DMACONTROLLER_H

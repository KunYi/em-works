//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_apbxdma.c
//
//  This file contains a DDK interface for the AHB to APBX DMA module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Local Variables
PVOID pv_HWregAPBX = NULL;
static HANDLE g_hApbxdmaMutex;

//-----------------------------------------------------------------------------
// Local Functions
BOOL ApbxDmaInit(void);
BOOL ApbxDmaDeInit(void);
BOOL ApbxDmaAlloc(void);
BOOL ApbxDmaDealloc(void);
static BOOL ApbxDmaValidChan(UINT8 Channel);
static void ApbxLockChan(void);
static void ApbxUnLockChan(void);

void ApbxDumpCtrlRegs(void);
void ApbxDumpChanRegs(UINT8 Channel);
void ApbxZeroSemaphore(UINT8 Channel);

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxStartDma
//
//  This function loads the NEXTCOMMAND address and increments the semaphore to
//  start the DMA operation for first command.
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//      memAddrPA
//          [in] - physical addr for the DMA 
//
//      semaphore
//          [in] - dma semaphore
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxStartDma(UINT8 Channel,PVOID memAddrPA, UINT8 semaphore)
{
    BOOL rc = FALSE;
    UINT32 memAddressPA = (UINT32)memAddrPA;

    // Lock
    ApbxLockChan();

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }
    // validate the memAddrPA
    if(0 == memAddrPA)
    {
        ERRORMSG(1, (_T("DDKApbxStartDma failed!\r\n")));
        goto cleanUp;
    }
#ifdef DUMP_APBX_REGS
    DEBUGMSG(1, (_T("DDKApbxStartDma on channel %d PA=%08x\r\n"),Channel, memAddressPA));
    ApbxDumpCtrlRegs();
    ApbxDumpChanRegs(Channel);
#endif
    //// Load command chain pointer
	//HW_APBX_CHn_NXTCMDAR_WR(Channel, memAddressPA);
    //// Increment dma semaphore
    //BW_APBX_CHn_SEMA_INCREMENT_SEMA(Channel, semaphore);
	//
	// CS&ZHL AUG-23-2012: set next_cmd address only if semaphore != 0
	//
	if(semaphore)
	{
		HW_APBX_CHn_NXTCMDAR_WR(Channel, memAddressPA);
		// Increment dma semaphore
		BW_APBX_CHn_SEMA_INCREMENT_SEMA(Channel, semaphore);
	}
	else
	{
		// Increment dma semaphore by net one
		BW_APBX_CHn_SEMA_INCREMENT_SEMA(Channel, 1);
	}

    rc = TRUE;

cleanUp:

    // Unlock
    ApbxUnLockChan();

    return rc;
}

UINT32 DDKApbxGetNextCMDAR(UINT8 Channel)
{
    UINT32 RegNextCMDAR;
    RegNextCMDAR = HW_APBX_CHn_NXTCMDAR_RD(Channel);

    return (RegNextCMDAR);
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxStopDma
//
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxStopDma(UINT8 Channel)
{
    BOOL rc = FALSE;

    // Put the channel in RESET state
    if(!DDKApbxDmaResetChan(Channel, TRUE))
    {
        ERRORMSG(1, (_T("DDKApbxStopDma Invalid Channel!\r\n")));
        goto cleanUp;
    }
    rc = TRUE;

#ifdef DUMP_APBX_REGS
    DEBUGMSG(1, (_T("DDKApbxStopDma on channel %d\r\n"),Channel));
    ApbxDumpCtrlRegs();
    ApbxDumpChanRegs(Channel);
#endif
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaInitChan
//
//  This function inits the requested DMA channel
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxDmaInitChan(UINT8 Channel,BOOL bEnableIrq)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Lock
    ApbxLockChan();

    // unFreeze channel
    if(!DDKApbxDmaFreezeChan(Channel,FALSE))
    {
        ERRORMSG(1, (_T("DDKApbxDmaInitChan.. failed to UnFreeze channel!\r\n")));
        goto cleanUp;
    }

    // come out from reset state
    if( !DDKApbxDmaResetChan(Channel,FALSE))
    {
        ERRORMSG(1, (_T("DDKApbxDmaInitChan.. Failed to reset channel!\r\n")));
        goto cleanUp;
    }

    // Enable/Disable Irq based on the request
    if( !DDKApbxDmaEnableCommandCmpltIrq(Channel,bEnableIrq))
    {
        ERRORMSG(1, (_T("DDKApbxDmaInitChan.. Failed to Enable/Disable IRQ!\r\n")));
        goto cleanUp;
    }

#ifdef DUMP_APBX_REGS
    DEBUGMSG(1, (_T("DDKApbxInitDma on channel %d (enable irq=%d)\r\n"),Channel,bEnableIrq));
    ApbxDumpCtrlRegs();
    ApbxDumpChanRegs(Channel);
#endif
    rc = TRUE;

cleanUp:

    // Unlock
    ApbxUnLockChan();

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaGetActiveIrq
//
//  This function gets the active Interrupt for respective channel
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxDmaGetActiveIrq(UINT8 Channel)
{
    BOOL rc = FALSE;
    UINT32 RegCTRL1;

    // Lock
    ApbxLockChan();

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    RegCTRL1 =  HW_APBX_CTRL1_RD();
    RegCTRL1 &= 0xFFFF;

    if(RegCTRL1 & (1 << Channel))
    {
        rc = TRUE;
    }

cleanUp:

    // Unlock
    ApbxUnLockChan();

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaClearCommandCmpltIrq
//
//  This function Clears the Interrupt for respective channel
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxDmaClearCommandCmpltIrq(UINT8 Channel)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Clear the channel Interrupt
    HW_APBX_CTRL1_CLR(1 << Channel);
    rc = TRUE;

cleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaClearErrorIrq
//
//  This function Clears the Interrupt for respective channel
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxDmaClearErrorIrq(UINT8 Channel)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Clear the channel Error irq Interrupt
    HW_APBX_CTRL2_CLR(1 << Channel);
    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaEnableCommandCmpltIrq
//
//  This function Enable the Interrupt for respective channel
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//      bEnable
//          [in] - TRUE to enable, FALSE to disable
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxDmaEnableCommandCmpltIrq(UINT8 Channel,BOOL bEnable)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    if( bEnable)
    {
        HW_APBX_CTRL1_SET(1 << (16 + Channel));          //Enable interrupt
    }
    else
    {
        HW_APBX_CTRL1_CLR(1 << (16 + Channel));          //Disable interrupt
    }
    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaGetErrorStatus
//
//  This function returns the error status value for the channel.
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//  Returns:
//      DDK_DMA_ERROR_NONE - dma completed with no error
//      DDK_DMA_ERROR_INCOMPLETE - completion irq=0 & err irq=0
//      DDK_DMA_ERROR_EARLYTERM - early termination occurred
//      DDK_DMA_ERROR_BUS - bus error occurred
//      DDK_DMA_ERROR_UNKNOWN - any other combination
//
//
//  err     err     compl
//  status  irq     irq
//
//  x       0       1       complete & !err = no error
//  1       1       x       bus err
//  0       1       x       early term
//  x       0       0       incomplete
//
//
//-----------------------------------------------------------------------------
UINT32 DDKApbxDmaGetErrorStatus(UINT8 Channel)
{
    UINT32 RegCTRL1,RegCTRL2;
    UINT32 err,stat,cmpl;
    UINT32 result = DDK_DMA_ERROR_UNKNOWN;

    // Lock
    ApbxLockChan();

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    RegCTRL1 =  HW_APBX_CTRL1_RD();
    RegCTRL2 =  HW_APBX_CTRL2_RD();

    cmpl  = (RegCTRL1 & (1       << Channel)) ? 1 : 0;
    err   = (RegCTRL2 & (1       << Channel)) ? 1 : 0;
    stat  = (RegCTRL2 & (0x10000 << Channel)) ? 1 : 0;

    if(cmpl & !err)
    {
        result = DDK_DMA_ERROR_NONE;
    }
    else if (err & !stat)
    {
        result = DDK_DMA_ERROR_EARLYTERM;
    }
    else if (err &  stat)
    {
        result = DDK_DMA_ERROR_BUS;
    }
    else if (!cmpl & !err)
    {
        result = DDK_DMA_ERROR_INCOMPLETE;
    }
cleanUp:

    // Unlock
    ApbxUnLockChan();

    return result;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaResetChan
//
//  This function Resets the AHB to APBX bridge channel based on the argument
//  Channel
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns the virtual channel index if successful,
//      otherwise returns NULL.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxDmaResetChan(UINT8 Channel, BOOL bReset)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // RESET the channel
    if(bReset)
    {
        HW_APBX_CHANNEL_CTRL_SET( 1 << (16 + Channel));
    }
    else
    {
        HW_APBX_CHANNEL_CTRL_CLR( 1 << (16 + Channel));

        // Wait till RESET happen
        for(; (HW_APBX_CHANNEL_CTRL_RD() & (1 << (16 + Channel))) != 0;) ;

        ApbxZeroSemaphore(Channel);
    }

    rc = TRUE;

cleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaFreezeChan
//
//  This function Freeze the AHB to APBX bridge channel based on the argument
//  Channel
//
//  Parameters:
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns the virtual channel index if successful,
//      otherwise returns NULL.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxDmaFreezeChan(UINT8 Channel, BOOL bFreeze)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    if( bFreeze )               // Freeze the channel
    {
        HW_APBX_CHANNEL_CTRL_SET(1 << Channel);
    }
    else                                // unFreeze the channel
    {
        HW_APBX_CHANNEL_CTRL_CLR(1 << Channel);

        // Wait till unFreeze happen
        for(; (HW_APBX_CHANNEL_CTRL_RD() & (1 << (Channel))) != 0;) ;
    }
    rc = TRUE;
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  ApbxDmaInit
//
//  This function initializes the AHB To APBX hardware bridge.  Called by the
//  Device Manager to initialize a device.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL ApbxDmaInit(void)
{
    BOOL rc = FALSE;

    // Enable Normal APBX DMA operation

    HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_SFTRST);
    HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_CLKGATE);

    // Asserting soft reset
    HW_APBX_CTRL0_SET(BM_APBX_CTRL0_SFTRST);

    // Waiting for confirmation of soft reset
    while(!HW_APBX_CTRL0.B.CLKGATE)
    {
        // busy wait
    }

    // Done with Softreset
    HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_SFTRST);
    HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_CLKGATE);

    rc = TRUE;

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  ApbxDmaDeInit
//
// This function deinitializes the AHB To APBX hardware bridge.  Called by the
// Device Manager to de-initialize a device.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL ApbxDmaDeInit(void)
{
    BOOL rc = TRUE;

    // Asserting soft reset (This will disable clocking with APBX DMA and hold it
    // in it's reset(Lowest power) state.

    HW_APBX_CTRL0_SET(BM_APBX_CTRL0_SFTRST);

    rc = TRUE;
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  ApbxLockChan
//
//  This function locks the Apbx channel to provide
//  safe access to the control channel shared resources.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void ApbxLockChan(void)
{
    WaitForSingleObject(g_hApbxdmaMutex, INFINITE);
}
//-----------------------------------------------------------------------------
//
//  Function:  ApbxUnLockChan
//
//  This function unlocks the APBX channel.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void ApbxUnLockChan(void)
{
    ReleaseMutex(g_hApbxdmaMutex);
}
//-----------------------------------------------------------------------------
//
// Function:  ApbxDmaAlloc
//
// This function allocates the data structures required for interaction
// with the AHB To APBX hardware bridge.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL ApbxDmaAlloc(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    if (pv_HWregAPBX == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_APBX;

        // Map peripheral physical address to virtual address
        pv_HWregAPBX = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregAPBX == NULL)
        {
            ERRORMSG(1, (_T("ApbxDmaAlloc::MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }
    if (g_hApbxdmaMutex == NULL)
    {
        g_hApbxdmaMutex = CreateMutex(NULL, FALSE, L"MUTEX_SDMA");

        if (g_hApbxdmaMutex == NULL)
        {
            ERRORMSG(1, (_T("CreateMutex failed!\r\n")));
            goto cleanUp;
        }
    }


    rc = TRUE;

cleanUp:
    if (!rc) ApbxDmaDealloc();
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  ApbxDmaDealloc
//
// This function deallocates the data structures required for interaction
// with the AHB To APBX hardware bridge.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL ApbxDmaDealloc(void)
{
    // Unmap peripheral registers
    if (pv_HWregAPBX)
    {
        MmUnmapIoSpace(pv_HWregAPBX, 0x1000);
        pv_HWregAPBX = NULL;
    }
    if (g_hApbxdmaMutex)
    {
        CloseHandle(g_hApbxdmaMutex);
        g_hApbxdmaMutex = NULL;
    }

    return TRUE;
}
//-----------------------------------------------------------------------------
//
//  Function:  ApbxDmaValidChan
//
//  This function determines if a channel is valid and configured properly
//  for allocation and configuration of buffer descriptors.
//
//  Parameters:
//      chan
//          [in] - channel number to validate .
//
//  Returns:
//      Returns TRUE if the specified channel is valid, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL ApbxDmaValidChan(UINT8 Channel)
{
    BOOL rc = FALSE;

    // Validate channel
    if (Channel >= DDK_MAX_APBX_CHANNEL)
    {
        ERRORMSG(1, (_T("ApbxDmaValidChan..%d is Invalid channel!\r\n"),Channel));
        goto cleanUp;
    }
    rc = TRUE;

cleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaGetErrorIrq
//
//  This function returns the error irq state for respective channel
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxDmaGetErrorIrq(UINT8 Channel)
{
    BOOL rc = FALSE;
    UINT32 RegCTRL2;

    // Lock
    ApbxLockChan();

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    RegCTRL2 =  HW_APBX_CTRL2_RD();
    RegCTRL2 &= 0xFFFF;

    if(RegCTRL2 & (1 << Channel))
    {
        rc = TRUE;
    }

cleanUp:

    // Unlock
    ApbxUnLockChan();

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbxDmaClearInterrupts
//
//  This function clears both the completion and the error irq bits
//  for the requested channel, as well as the completion irq enable.
//  Within an IST context, this must be called prior to calling
//  InterruptDone() otherwise the IST will continue to loop.
//
//  Parameters:
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns TRUE if successful,
//      otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKApbxDmaClearInterrupts(UINT8 Channel)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    HW_APBX_CTRL1_CLR(0x10001 << Channel);
    HW_APBX_CTRL2_CLR(0x10001 << Channel);

    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  ApbxDumpCtrlRegs
//
//  Dump the apbx ctrl registers.
//
//  Parameters:
//      Channel
//          [in] - channel number.
//
//  Returns:
//      none
//-----------------------------------------------------------------------------
void ApbxDumpCtrlRegs(void)
{
#ifdef DUMP_APBX_REGS
    PHYSICAL_ADDRESS phyAddr;
    PCSP_APBX_REG p;
    phyAddr.QuadPart = CSP_BASE_REG_PA_APBX;
    p = (PCSP_APBX_REG) MmMapIoSpace(phyAddr, sizeof(CSP_APBX_REG), FALSE);

    RETAILMSG(1, (L"HW_APBX_CTRL0         %08X\r\n",p->ctrl0.U));
    RETAILMSG(1, (L"HW_APBX_CTRL1         %08X\r\n",p->ctrl1.U));
    RETAILMSG(1, (L"HW_APBX_CTRL2         %08X\r\n",p->ctrl2.U));
    RETAILMSG(1, (L"HW_APBX_CHANNEL_CTRL  %08X\r\n",p->channel_ctrl.U));
    RETAILMSG(1, (L"HW_APBX_DEVSEL        %08X\r\n",p->devsel.U));
    MmUnmapIoSpace((LPVOID) p, sizeof(PCSP_APBX_CHAN_REG));
#endif
}

//-----------------------------------------------------------------------------
//
//  Function:  ApbxDumpChanRegs
//
//  Dump the channel's registers.
//
//  Parameters:
//      Channel
//          [in] - channel number.
//
//  Returns:
//      none
//-----------------------------------------------------------------------------
void ApbxDumpChanRegs(UINT8 Channel)
{
#ifdef DUMP_APBX_REGS
    PHYSICAL_ADDRESS phyAddr;
    PCSP_APBX_CHAN_REG p;
    phyAddr.QuadPart = CSP_BASE_REG_PA_APBX + 0x00000100 + Channel*0x70;
    p = (PCSP_APBX_CHAN_REG) MmMapIoSpace(phyAddr, sizeof(CSP_APBX_CHAN_REG), FALSE);
    RETAILMSG(1, (L"HW_APBX_CH%d_CURCMDAR  %08X\r\n",Channel,p->curcmdar.U));
    RETAILMSG(1, (L"HW_APBX_CH%d_NXTCMDAR  %08X\r\n",Channel,p->nxtcmdar.U));
    RETAILMSG(1, (L"HW_APBX_CH%d_CMD       %08X\r\n",Channel,p->cmd.U));
    RETAILMSG(1, (L"HW_APBX_CH%d_BAR       %08X\r\n",Channel,p->bar.U));
    RETAILMSG(1, (L"HW_APBX_CH%d_SEMA      %08X\r\n",Channel,p->sema.U));
    RETAILMSG(1, (L"HW_APBX_CH%d_DEBUG1    %08X\r\n",Channel,p->debug1.U));
    RETAILMSG(1, (L"HW_APBX_CH%d_DEBUG2    %08X\r\n",Channel,p->debug2.U));
    MmUnmapIoSpace((LPVOID) p, sizeof(PCSP_APBX_CHAN_REG));
#else
    UNREFERENCED_PARAMETER(Channel);
#endif
}

//-----------------------------------------------------------------------------
//
//  Function:  ApbxZeroSemaphore
//
//  Zero the channel's semaphore count by adding its 2s complement.
//
//  Parameters:
//      Channel
//          [in] - channel number.
//
//  Returns:
//      none
//-----------------------------------------------------------------------------
void ApbxZeroSemaphore(UINT8 Channel)
{
    DWORD phore;
    
    // Validate channel
    if(!ApbxDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        return;
    }
    
    phore = (HW_APBX_CHn_SEMA_RD(Channel) & BM_APBX_CHn_SEMA_PHORE) >> BP_APBX_CHn_SEMA_PHORE;
    phore = ~phore + 1;
    BW_APBX_CHn_SEMA_INCREMENT_SEMA(Channel, phore);
}

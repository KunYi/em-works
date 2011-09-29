//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_apbhdma.c
//
//  This file contains a DDK interface for the AHB to APBH DMA module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#ifdef BOOTLOADER
#include <oal.h>
#endif
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
PVOID pv_HWregAPBH = NULL;

//-----------------------------------------------------------------------------
// Local Functions
BOOL ApbhDmaInit(void);
BOOL ApbhDmaDeInit(void);
BOOL ApbhDmaAlloc(void);
BOOL ApbhDmaDealloc(void);
static BOOL ApbhDmaValidChan(UINT8 Channel);

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhStartDma
//
//  This function loads the NEXTCOMMAND address and increments the semaphore to
//  start the DMA operation for first command.
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
BOOL DDKApbhStartDma(UINT8 Channel,PVOID memAddrPA, UINT8 semaphore)
{
    BOOL rc = FALSE;
    UINT32 memAddressPA = (UINT32)memAddrPA;

    // Validate channel
    if(!ApbhDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Turn on the channel clockgate
    if(!DDKApbhDmaChanCLKGATE(Channel,FALSE))
    {
        ERRORMSG(1, (_T("DDKApbhDmaInitChan.. failed to set the channel CLKGATE!\r\n")));
        goto cleanUp;
    }

    // validate the memAddrPA
    if(0 == memAddrPA)
    {
        ERRORMSG(1, (_T("DDKApbhStartDma failed!\r\n")));
        goto cleanUp;
    }

    // Load command chain pointer
    HW_APBH_CHn_NXTCMDAR_WR(Channel, memAddressPA);

    // Increment dma semaphore
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(Channel, semaphore);

    rc = TRUE;

cleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhStopDma
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
BOOL DDKApbhStopDma(UINT8 Channel)
{
    BOOL rc = FALSE;

    // Turn off the channel clockgate
    if(!DDKApbhDmaChanCLKGATE(Channel,TRUE))
    {
        ERRORMSG(1, (_T("DDKApbhDmaInitChan.. failed to set the channel CLKGATE!\r\n")));
        goto cleanUp;
    }
    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhDmaInitChan
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
BOOL DDKApbhDmaInitChan(UINT8 Channel,BOOL bEnableIrq)
{
    BOOL rc = FALSE;

    // Validate channel
    if (Channel > DDK_MAX_APBH_CHANNEL )
    {
        ERRORMSG(1, (_T("DDKApbhDmaInitChan..Invalid channel!\r\n")));
        goto cleanUp;
    }

    // Enable clock for respective channel
    if(!DDKApbhDmaChanCLKGATE(Channel,FALSE))
    {
        ERRORMSG(1, (_T("DDKApbhDmaInitChan.. failed to set the channel CLKGATE!\r\n")));
        goto cleanUp;
    }

    // unFreeze channel
    if(!DDKApbhDmaFreezeChan(Channel,FALSE))
    {
        ERRORMSG(1, (_T("DDKApbhDmaInitChan.. failed to UnFreeze channel!\r\n")));
        goto cleanUp;
    }

    // Reset channel
    if( !DDKApbhDmaResetChan(Channel,FALSE))
    {
        ERRORMSG(1, (_T("DDKApbhDmaInitChan.. Failed to reset channel!\r\n")));
        goto cleanUp;
    }

    // Enable/Disable Irq based on the request
    if( !DDKApbhDmaEnableCommandCmpltIrq(Channel,bEnableIrq))
    {
        ERRORMSG(1, (_T("DDKApbhDmaInitChan.. Failed to Enable/Disable IRQ!\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhDmaChanCLKGATE
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
BOOL DDKApbhDmaChanCLKGATE(UINT8 Channel,BOOL bClockGate)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbhDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    if( bClockGate)
        HW_APBH_CTRL0_SET(1 << (8 + Channel));           //Reset state
    else
        HW_APBH_CTRL0_CLR(1 << (8 + Channel) );         //Normal operation

    rc =TRUE;
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhDmaClearCommandCmpltIrq
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
BOOL DDKApbhDmaClearCommandCmpltIrq(UINT8 Channel)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbhDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Clear the channel Error irq Interrupt
    HW_APBH_CTRL1_CLR(1 << Channel);
    rc = TRUE;

cleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhDmaEnableCommandCmpltIrq
//
//  This function Enable the Interrupt for respective channel
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
BOOL DDKApbhDmaEnableCommandCmpltIrq(UINT8 Channel,BOOL bEnable)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbhDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    if( bEnable)
        HW_APBH_CTRL1_SET(1 << (16 + Channel));          //Enable interrupt
    else
        HW_APBH_CTRL1_CLR(1 << (16 + Channel) );                 //Disable interrupt

    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhDmaResetChan
//
//  This function Resets the AHB to APBH bridge channel based on the argument
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
BOOL DDKApbhDmaResetChan(UINT8 Channel,BOOL bReset)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbhDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // RESET the channel
    if(bReset)
        HW_APBH_CTRL0_SET( 1 << ( 16 + Channel));
    else
        HW_APBH_CTRL0_CLR( 1 << ( 16 + Channel));

    rc = TRUE;
cleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhDmaFreezeChan
//
//  This function Freeze the AHB to APBH bridge channel based on the argument
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
BOOL DDKApbhDmaFreezeChan(UINT8 Channel,BOOL bFreeze)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbhDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    if( bFreeze )               // Freeze the channel
        HW_APBH_CTRL0_SET(1 << Channel);
    else                                // unFreeze the channel
        HW_APBH_CTRL0_CLR(1 << Channel);

    rc = TRUE;
cleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhDmaGetPhore
//
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
UINT32 DDKApbhDmaGetPhore(UINT32 Channel)
{
    DWORD dwPHORE;

    dwPHORE = HW_APBH_CHn_SEMA(Channel).B.PHORE;

    return (dwPHORE);
}

//-----------------------------------------------------------------------------
//
//  Function:  ApbhDmaInit
//
//  This function initializes the AHB To APBH hardware bridge.  Called by the
//  Device Manager to initialize a device.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL ApbhDmaInit(void)
{
    BOOL rc = FALSE;

    // Enable Normal APBH DMA operation

    HW_APBH_CTRL0_CLR(BM_APBH_CTRL0_SFTRST);
    HW_APBH_CTRL0_CLR(BM_APBH_CTRL0_CLKGATE);

    // Asserting soft reset
    HW_APBH_CTRL0_SET(BM_APBH_CTRL0_SFTRST);

    // Waiting for confirmation of soft reset
    while(!HW_APBH_CTRL0.B.CLKGATE)
    {
        // busy wait
    }

    // Done with Softreset
    HW_APBH_CTRL0_CLR(BM_APBH_CTRL0_SFTRST);
    HW_APBH_CTRL0_CLR(BM_APBH_CTRL0_CLKGATE);

    rc = TRUE;

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  ApbhDmaDeInit
//
// This function deinitializes the AHB To APBH hardware bridge.  Called by the
// Device Manager to de-initialize a device.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL ApbhDmaDeInit(void)
{
    BOOL rc = TRUE;

    // Asserting soft reset (This will disable clocking with APBH DMA and hold it
    // in it's reset(Lowest power) state.

    HW_APBH_CTRL0_SET(BM_APBH_CTRL0_SFTRST);

    // Deallocate APBH DMA driver data structures
    ApbhDmaDealloc();

    rc = TRUE;
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  ApbhDmaAlloc
//
// This function allocates the data structures required for interaction
// with the AHB To APBH hardware bridge.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL ApbhDmaAlloc(void)
{
    BOOL rc = FALSE;

#ifndef BOOTLOADER
    PHYSICAL_ADDRESS phyAddr;

#endif
    if (pv_HWregAPBH == NULL)
    {
#ifdef BOOTLOADER
        // Map peripheral physical address to virtual address
        pv_HWregAPBH = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_APBH);
#else
        phyAddr.QuadPart = CSP_BASE_REG_PA_APBH;

        // Map peripheral physical address to virtual address
        pv_HWregAPBH = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);
#endif

        // Check if virtual mapping failed
        if (pv_HWregAPBH == NULL)
        {
            ERRORMSG(1, (_T("ApbhDmaAlloc::MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    if (!rc) ApbhDmaDealloc();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  ApbhDmaDealloc
//
// This function deallocates the data structures required for interaction
// with the AHB To APBH hardware bridge.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL ApbhDmaDealloc(void)
{
    // Unmap peripheral registers
    if (pv_HWregAPBH)
    {

#ifndef BOOTLOADER
        MmUnmapIoSpace(pv_HWregAPBH, 0x1000);
#endif

        pv_HWregAPBH = NULL;
    }

    return TRUE;
}
//-----------------------------------------------------------------------------
//
//  Function:  ApbhDmaValidChan
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
static BOOL ApbhDmaValidChan(UINT8 Channel)
{
    BOOL rc = FALSE;

    // Validate channel
    if (Channel > DDK_MAX_APBH_CHANNEL)
    {
        ERRORMSG(1, (_T("DDKApbhDmaInitChan..Invalid channel!\r\n")));
        goto cleanUp;
    }
    rc = TRUE;

cleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKApbhDmaClearErrorIrq
//
//  This function Clears the sticky error flag for respective channel
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
BOOL DDKApbhDmaClearErrorIrq(UINT8 Channel)
{
    BOOL rc = FALSE;

    // Validate channel
    if(!ApbhDmaValidChan(Channel))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Clear the channel Error irq Interrupt
    HW_APBH_CTRL2_CLR(1 << Channel);
    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

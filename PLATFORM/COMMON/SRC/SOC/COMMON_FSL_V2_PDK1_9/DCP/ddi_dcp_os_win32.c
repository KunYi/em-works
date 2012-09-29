//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ddi_dcp_os_win32.c
//  Brief data co-processor interface depending on OS
//
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)

#include "ddi_dcp_os_private.h"

void DCPInitCSCSysVar();

extern BOOL g_DCPInitialized;

PHYSICAL_ADDRESS dcp_phys_address;

// This semaphore is initially set to DCP_MAX_CHANNELS and decremented when
// a DCP channel is acquired and incremented when a channel is released.
// The dcp_AcquireChannel function suspends on this semaphore when all
// channels are in use.
static HANDLE g_DCPSemaphore;

// This mutex is used in the dcp_AcquireChannel function to make sure only
// one thread can be in the function at a time.  This prevents 2 threads
// from accidentally getting the same channel.
static HANDLE g_DCPMutex;

static LONG dcp_count;

//-----------------------------------------------------------------------------
//
// Function : dcp_os_Init
//
//  Initialize OS variables for DCP 
//
// Parameters:
//      Channels
//          [in] Pointer to channel structure array
//             
//      ChannelCount   
//          [in] The number of channel
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_os_Init(DCPChannel_t* Channels, int ChannelCount)
{
    int i;

    for (i = 0; i < ChannelCount; i++)
    Channels[i].swSemaphore = CreateSemaphore(NULL, 1, 1, L"DCP Channel");

    // Create our semaphore and mutexs.  Set the semaphore to
    // DCP_MAX_CHANNELS showing all for channels are available.

    g_DCPSemaphore = CreateSemaphore(NULL,
                    DCP_MAX_CHANNELS,
                    DCP_MAX_CHANNELS,
                    L"g_DCPSemaphore");

    DCPInitCSCSysVar();

    dcp_count = DCP_MAX_CHANNELS;

    g_DCPMutex = CreateMutex(NULL, FALSE, L"g_DCPMutex");

    return ERROR_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function : dcp_GetChannelSemaphore
//
//  Gets the individual channel semaphores
//
//  Each channel has its own semaphore used to indicate when it's running.
//  It gets set in the dcp_ExecutePackets function and cleared when it
//  completes and calls ddi_dcp_isr.
//
// Parameters:
//      Channel
//          [in] Channel number
//             
//      Timeout
//          [in] Timeout in milliseconds or set to TX_WAIT_FOREVER
//              to wait forever.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_GetChannelSemaphore(DCPChannel_t* Channel, UINT32 TimeOut)
{
    RtResult Status = ERROR_SUCCESS;

    if (WaitForSingleObject(Channel->swSemaphore, TimeOut) != WAIT_OBJECT_0)
        Status = WAIT_TIMEOUT;

    return Status;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_PutChannelSemaphore
//
//  Puts the individual channel's semaphore.
//
//  Each channel has its own semaphore used to indicate when it's running.
//  It gets set in the dcp_ExecutePackets function and cleared when it
//  completes and calls ddi_dcp_isr.
//
// Parameters:
//      Channel
//          [in] Channel number
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_PutChannelSemaphore(DCPChannel_t* Channel)
{
    ReleaseSemaphore(Channel->swSemaphore, 1, NULL);

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_PutSemaphore
//
//  Increments the DCP semaphore.
//
//  This semaphore is used to keep track of the free channels and allows
//  threads to suspend when no channels are available.
//
// Parameters:
//      None
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_PutSemaphore()
{
    ReleaseSemaphore(g_DCPSemaphore, 1, &dcp_count);

    // The semaphore count can never be
    // more than the available channels
    if(++dcp_count > DCP_MAX_CHANNELS)
    {
        ERRORMSG (1, (L"The semaphore count can never be more than the available channels for DCP\r\n"));
        ASSERT(0);
        return ERROR_NO_MORE_ITEMS;
    }

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_PutSemaphore
//
// Get the DCP's semaphore. 
//
// If no channels are free the function will suspend until one comes availble.
//
// Parameters:
//      None
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_GetSemaphore()
{
    // If we can't get a handle to a channel in 3 seconds
    // something is wrong.
    if (WaitForSingleObject(g_DCPSemaphore, 3000) != WAIT_OBJECT_0)
    {
        return WAIT_TIMEOUT;
    }

    dcp_count--;

    return S_OK;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_GetSemaphoreCount
//
// Get the DCP's semaphore count. 
//
// Parameters:
//      None
//
// Returns:
//      DCP's semaphore count
//
//-----------------------------------------------------------------------------
int dcp_GetSemaphoreCount(void)
{
    return (int) dcp_count;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_AcquireMutex
//
// Acquire the DCP's mutex. 
//
// Parameters:
//      None
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_AcquireMutex(void)
{
    // Get the mutex so only one thread can be in this function at a time.
    if (WaitForSingleObject(g_DCPMutex, 3000) != WAIT_OBJECT_0)
        return -1;

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_ReleaseMutex
//
// Release the DCP's mutex. 
//
// Parameters:
//      None
//
// Returns:
//      ERROR_SUCCESS
//
//-----------------------------------------------------------------------------
RtResult dcp_ReleaseMutex(void)
{
    // Release the MUTEX so other threads can call this function
    ReleaseMutex(g_DCPMutex);

    return ERROR_SUCCESS;
}

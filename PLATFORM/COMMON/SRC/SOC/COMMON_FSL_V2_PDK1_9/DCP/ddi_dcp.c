//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ddi_dcp.c
//  Brief data co-processor interface
//
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes and external references
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)

#include "ddi_dcp_os_private.h"

//------------------------------------------------------------------------------
// Externs
//
// g_DCPChannel is a structure that keeps track of information about the
// channels.  The structure is defined in common_dcp.h and has the following elements
//
// NOTE: ONLY THIS FILE AND ddi_dcp_init.c SHOULD ACCESS THIS VARIABLE
//
// Available        - Bit value set to 1 when the channel is free to be used.
// Locked           - Bit value set to 1 when the channel has been locked for
//                    long term use
// Callback         - Bit value set to 1 when a call back function is requested
//                    when the channel completes
// CallbackFunction - Pointer to the function that will be called when the
//                    channel completes it's requested operation.  This
//                    parameter is only used when Callback is set to 1
// CallbackData     - Pointer that will be passed into the callback function.
// swSemaphore      - A semaphore used to wait for the channel to complete it's
//                    operation. The function dcp_ExecutePackets gets the
//                    semaphore and the ISR routine puts it back.
// WorkPacket       - A static work packet for each channel.  Other work packets
//                    can also be used.

extern DCPChannel_t* g_DCPChannel;

//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------
RtResult dcp_WaitOnChannel(UINT32 Channel, UINT32 TimeOut);
RtResult dcp_IncrementHandle(UINT32 Channel);
INT32 dcp_NumLockedChannels(void);
RtResult dcp_GetNextAvailableChannel(UINT32 *Channel);
RtResult dcp_MakeChannelAvailable(UINT32 Channel);

BOOL DCPNoCSCOrCSCAvailable();

//------------------------------------------------------------------------------
//
// Function : dcp_ExecutePackets
//
// Runs a DCP channel using the specified work packets.
//
// Parameters:
//      Handle            
//          [in] Handle pointing to the channel to run
//
//      Packets           
//          [in] Pointer to the first work packet to execute.
//              Additional packets can fallow.  Packets do not
//              have to be contiguous in memory because each
//              packet contains a pointer to the next packet. 
//
//      NumberOfPackets
//          [in] Total number of packets to execute
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_ExecutePackets(DCPHandle_t Handle, DCPWorkPacket_t *Packets, UINT32 NumberOfPackets)
{
    UINT32    Channel; // Channel number to execute
    RtResult  Status;  // Status to be returned
    
    Status = dcp_GetChannelNumber(Handle, &Channel);

    if (Status == ERROR_SUCCESS)
    {
        // Grab the channel's semaphore to show that it's running.  
        // This semaphore get's cleared in the ISR routine.
        if (dcp_GetChannelSemaphore(&g_DCPChannel[Channel], 20) != ERROR_SUCCESS)
        {
            // This semaphore shold be free if we make it to here so if we don't get
            // the semaphore right away there is a problem.
            Status = WAIT_TIMEOUT;
            ERRORMSG (1, (L"DCP get channel semaphore failed!\r\n"));
            ASSERT(0);
            return Status;
        }

        Status = dcp_hw_RunChannel(Channel,
                    virt2phys(Packets),
                    NumberOfPackets);
    }

    return Status;
}

//------------------------------------------------------------------------------
//
// Function : dcp_AcquireChannel
//
// Get a handle to an available DCP channel.
//
//  This function is used to allocate DCP channels to different threads.  
//  Unless LockIt is set to TRUE the handle will be released when it
//  completes executing the work packets.
//  
//  Channels are acquired in reverse order from DCP_MAX_CHANNELS (currently
//  set to 3) to 0.  This keeps channel 0 free most of the time so the VMI
//  will have access to it.  Also, the context buffer is laid out so the
//  highest channel numbers will have the lowest address in memory.
//
// Parameters:
//      Handle            
//          [out] A pointer to the place where the handle will be
//                   returned.  This will be set to 0 if an error occures.
//                   0 is always an invalid handle.
//
//      DesiredChannel           
//          [in] Lets the thread specify what channel number it
//              would like to have.  This is only needed for the VMI
//              because it needs to be channel 0.  Channels 1 through
//              DCP_MAX_CHANNELS are all the same.  If this value is set
//              to DCP_GET_ANY_CHANNEL or any value greater than
//              DCP_MAX_CHANNELS the function we get the next available
//              channel.
//
//      LockIt
//          [in] Boolean value used to lock a channel.  Channels
//              acquired with a lock are not released until the 
//              dcp_ReleaseLockedChannel is called.  Non-locked channels
//              will get released in the ISR.  Locking a channel
//              guarantees that you can have a DCP channel when you need
//              one but also reduces the number of DCP channels left for
//              the rest of the system.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_AcquireChannel(DCPHandle_t *Handle, UINT32 DesiredChannel, BOOL LockIt)
{
    UINT32    Channel;

    if (dcp_AcquireMutex() != ERROR_SUCCESS)
    {
        ERRORMSG (1, (L"DCP acquire mutex failed!\r\n"));
        ASSERT(0);
        return WAIT_TIMEOUT;
    }

    // Make sure there is an available channel.
    if (dcp_GetSemaphore() != ERROR_SUCCESS)
    {
        ERRORMSG (1, (L"DCP get semaphore failed!\r\n"));
        dcp_ReleaseMutex();
        ASSERT(0);
        return WAIT_TIMEOUT;
    }

    // See if the DCP is running and if not turn it
    // on.  To save power, the DCP will get turned off
    // if no channels are acquired and the CSC is not
    // being used.
    if (dcp_hw_IsEnabled() == FALSE)
    {
        dcp_hw_Enable(TRUE);
    }

    if (LockIt)
    {
        // We will not allow all channels to get locked to 
        // make sure at least one is alway available.
        if (dcp_NumLockedChannels() >= DCP_MAX_LOCKED_CHANNELS)
        {
            ERRORMSG(1, (L"At least one channel should be available for DCP.\r\n"));
            // Set the handle to an invalid number
            *Handle = 0;  

            // Increment the semaphore because the channel we
            // were going to grab is now available.
            dcp_PutSemaphore(); 
            dcp_ReleaseMutex();
            return ERROR_NO_MORE_ITEMS;
        }
    }

    // If desired channel is valid see if that channel
    // is available.  If not fail the call.
    if (DesiredChannel < DCP_MAX_CHANNELS)
    {
        if (g_DCPChannel[DesiredChannel].Available == FALSE)
        {
            ERRORMSG(1, (L"Desired channel is not available for DCP.\r\n"));
            // Set the handle to an invalid number
            *Handle = 0;

            // Increment the semaphore because the channel we
            // were going to grab is now available.
            dcp_PutSemaphore();
            dcp_ReleaseMutex();
            return ERROR_INVALID_ACCESS;
        }

        // If we get to here then the desired channel is
        // available
        Channel = DesiredChannel;
    }
    else
    {
        if (dcp_GetNextAvailableChannel(&Channel) != ERROR_SUCCESS)
        {
            // This call should never fail.  If it does then
            // it means that our semaphore is out of sync
            // with the number of available channels.
            ERRORMSG (1, (L"No more available channel for DCP!\r\n"));
            dcp_PutSemaphore(); 
            dcp_ReleaseMutex();
            ASSERT(0);
            return ERROR_NO_MORE_ITEMS;
        }
    }

    // If we get to here then the variable 'Channel' is
    // set to a valid available channel.

    g_DCPChannel[Channel].Available = 0;
    *Handle = g_DCPChannel[Channel].Handle;

    if (LockIt == TRUE)
    {
        g_DCPChannel[Channel].Locked = 1;
    }

    // Clear any old callback information
    if (dcp_SetCallbackInfo(g_DCPChannel[Channel].Handle, NULL, NULL) != ERROR_SUCCESS)
    {
        ERRORMSG (1, (L"Invalid channel handle for DCP!\r\n"));
        dcp_MakeChannelAvailable(Channel);
        dcp_ReleaseMutex();
        ASSERT(0);
        return ERROR_INVALID_HANDLE;
    }

    if (dcp_hw_ChannelInterruptEnable(Channel, TRUE) != ERROR_SUCCESS)
    {
        dcp_MakeChannelAvailable(Channel);
        dcp_ReleaseMutex();
        return ERROR_INVALID_DATA;
    }

    if (dcp_hw_ChannelEnable(Channel, TRUE) != ERROR_SUCCESS)
    {
        dcp_MakeChannelAvailable(Channel);
        dcp_ReleaseMutex();
        return ERROR_INVALID_DATA;
    }

    dcp_ReleaseMutex();

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function : dcp_MakeChannelAvailable
//
//  Sets the variables in the g_DCPChannel structure to show the channel
//  is now availble
//
// Parameters:
//      Channel
//          [in] Valid channel number between 0 and DCP_MAX_CHANNELS
//             
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_MakeChannelAvailable(UINT32 Channel)
{
    // Make sure we are passing in valid channel values
    if(Channel >= DCP_MAX_CHANNELS)
    {
        ERRORMSG (1, (L"Invalid channel for DCP!\r\n"));
        ASSERT(0);
        return ERROR_INVALID_ACCESS;
    }

    // We can only call this for channles that are
    // being used.  Otherwise our semaphore count
    // would not match the actual available
    // channels
    if(g_DCPChannel[Channel].Available != FALSE)
    {
        RETAILMSG (1, (L"DCP channel has already available!\r\n"));
        return ERROR_INVALID_ACCESS;
    }

    g_DCPChannel[Channel].Locked = FALSE;
    g_DCPChannel[Channel].Available = TRUE;

    // Incrementing the handle everytime it's
    // released invalidates all previous handles.
    dcp_IncrementHandle(Channel);

    if (dcp_hw_ChannelEnable(Channel, FALSE) != ERROR_SUCCESS)
    {
        return ERROR_INVALID_ACCESS;
    }

    // Disable the interrupt for this channel
    if (dcp_hw_ChannelInterruptEnable(Channel, FALSE) != ERROR_SUCCESS)
    {
        return ERROR_INVALID_ACCESS;
    }

    // Increment the semaphore count so it
    // matches the number of free channels.
    dcp_PutSemaphore();

    // If all of the channels are available and the CSC is not running power down
    // the DCP to save power.
    if ((dcp_GetSemaphoreCount() == DCP_MAX_CHANNELS) &&
        (DCPNoCSCOrCSCAvailable() == TRUE))
    {
        dcp_hw_Enable(FALSE);
    }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function : dcp_GetChannelWorkPacket
//
//  Get a pointer to a channel's workpacket
//
// Parameters:
//      Handle
//          [in] Handle of the channel.
//
//      Packet
//          [out] Location where the pointer is stored.
//             
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_GetChannelWorkPacket(DCPHandle_t Handle, DCPWorkPacket_t **Packet)
{
    UINT32 Channel;
    RtResult  Status;

    *Packet = NULL;

    Status = dcp_GetChannelNumber(Handle, &Channel);
    if (Status != ERROR_SUCCESS) return Status;

    // Make sure the channel has finished processing
    // before we change it's workpacket
    Status = dcp_WaitOnChannel(Channel, 3000);

    if (Status == ERROR_SUCCESS)
    *Packet = &(g_DCPChannel[Channel].WorkPacket);

    return Status;
}


//------------------------------------------------------------------------------
//
// Function : dcp_SetCallbackInfo
//
//  Sets the callback information that will get called when the channel
//  finishes it's operation.
//
// Parameters:
//      Handle
//          [in] Handle to the channel
//             
//      Function
//          [in] Pointer to the function to be called
//
//      PrivateData
//          [in] Pointer that will be passed when the function
//              is called
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_SetCallbackInfo(DCPHandle_t Handle, DCPCallback_t Function, void *PrivateData)
{
    UINT32    Channel;
    RtResult  Status;

    Status = dcp_GetChannelNumber(Handle, &Channel);

    if (Status == ERROR_SUCCESS)
    {
        g_DCPChannel[Channel].CallbackFunction = Function;
        g_DCPChannel[Channel].CallbackData = PrivateData;
    }

    return Status;
}

//------------------------------------------------------------------------------
//
// Function : dcp_WaitOnChannel
//
//  Waits for a channel to complete it's operation.
//
//  Parameters:
//      Channel
//          [in] Channel number between 0 and DCP_MAX_CHANNELS
//
//      Timeout
//          [in] Time out in milliseconds.  Set to TX_WAIT_FOREVER
//              to wait forever.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_WaitOnChannel(UINT32 Channel, UINT32 TimeOut)
{
    RtResult Status;

    if(Channel >= DCP_MAX_CHANNELS)
    {
        ERRORMSG (1, (L"Invalid channel for DCP!\r\n"));
        ASSERT(0);
        return ERROR_INVALID_ACCESS;
    }

    // Suspend the thread until the channel has stopped
    // executing work packets or we time out.
    Status = dcp_GetChannelSemaphore(&g_DCPChannel[Channel], TimeOut);
    if (Status == ERROR_SUCCESS)
    {
        // Put the semaphore back so the channel can be used.
        dcp_PutChannelSemaphore(&g_DCPChannel[Channel]);
    }

    return Status;
}


//------------------------------------------------------------------------------
//
// Function : dcp_GetChannelNumber
//
//  Get the channel number associated with a handle
//
//  This function gets the channel number of a valid handle.  If the handle is 
//  not valid the function returns an error.  If you get an error it's probably 
//  because the channel completing it's operation for that handle and has been
//  released.
//
//  Parameters:
//      Handle
//          [in] Handle to the channel
//
//      Channel
//          [out] Value of the channel
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_GetChannelNumber(DCPHandle_t Handle, UINT32 *Channel)
{
    UINT32 Ch;

    // Mask off the handle part and get the channel
    // number.
    Ch = CHANNEL_FROM_HANDLE(Handle);

    // Make sure that the handle matches the channel's
    // current value. 
    //
    // If it does not it's probably because the channel
    // completing it's operation for that handle and has been
    // released
    if (g_DCPChannel[Ch].Handle != Handle)
    {
        *Channel = 0xffffffff;
        return ERROR_INVALID_HANDLE;
    }

    *Channel = Ch;

    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function : dcp_GetNextAvailableChannel
//
//  Get the next available channel.
//
//  Because the VMI needs channel 0 and also the layout of the context buffer, 
//  the channels are acquired in reverse order starting at channel number 
//  DCP_MAX_CHANNELS 
//
//  Parameters:
//      Channel
//          [out] Holds the channel if successfull
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_GetNextAvailableChannel(UINT32 *Channel)
{
    INT32 i;

    for (i = (DCP_MAX_CHANNELS-1); i >= 0; i--)
    {
        if (g_DCPChannel[i].Available == TRUE)
        {
            *Channel = i;
            return ERROR_SUCCESS;
        }
    }

    return ERROR_NO_MORE_ITEMS;
}

//------------------------------------------------------------------------------
//
// Function : dcp_NumLockedChannels
//
//  Retuns the number of locked channels
//
//  Parameters:
//      None
//
// Returns:
//      Number of locked channels.
//
//------------------------------------------------------------------------------
INT32 dcp_NumLockedChannels()
{
    INT32 Count;
    INT32 i;
    
    Count = 0;
    for (i = 0; i < DCP_MAX_CHANNELS; i++)
    {
        if (g_DCPChannel[i].Locked == TRUE) Count++;
    }

    return Count;
}

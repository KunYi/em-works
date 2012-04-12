//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ddi_dcp_main.c
//  Brief data co-processor interface
//
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes and external references
//-----------------------------------------------------------------------------
#include <string.h>
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)

#include "ddi_dcp_os_private.h"

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
RtResult dcp_WaitOnChannel(UINT32 Channel, UINT32 TimeOut);
RtResult dcp_MakeChannelAvailable(UINT32 Channel);

//-----------------------------------------------------------------------------
//
// Function : dcp_memcopyAsync
//
// Performs an asynchronous memory copy.
//
// This function returns immediately so the calling thread will need to use a
// callback or the returned handle to find out when the operation is complete.
//
// Parameters:
//      Source
//          [in] Pointer to the source data
//
//      Destination
//          [in] Physical address where the data will be copied to
//
//      Length
//          [in] Size, in bytes, to copy
//
//      Callback
//          [in] Function pointer to a function to call when the
//              operation completes.  This value can be NULL if
//              no callback is required.
//
//      PrivateData
//          [in] This pointer will be the argument when the callback
//              operation completes.  This value can be NULL if
//              no callback is required.
//
//      RtHandle
//          [out] This value will contain the handle of the channel 
//                  being used in this copy routine.  It can be NULL 
//                  if the handle is not desired.  dcp_WaitForComplete
//                  can be called with this handle to cause the thread
//                  to wait for the operation to complete.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
// 
//-----------------------------------------------------------------------------
RtResult dcp_memcopyAsync(void *Source, void *Destination, UINT32 Length, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle)
{
    DCPHandle_t     Handle;
    DCPWorkPacket_t *Packet;
    RtResult      Status;

    Status = dcp_AcquireChannel(&Handle, DCP_GET_ANY_CHANNEL, FALSE);
    if (Status == ERROR_SUCCESS)
    {
        Status = dcp_SetCallbackInfo(Handle, Callback, PrivateData);
        if(Status != ERROR_SUCCESS)
        {
            Status = ERROR_INVALID_ACCESS;
            ERRORMSG (1, (L"DCP Invalid Handle!\r\n"));
            ASSERT(0);
            return Status;
        }

        if (RtHandle != NULL)
        {
            *RtHandle = Handle;
        }

        Status = dcp_GetChannelWorkPacket(Handle, &Packet);
        if(Status != ERROR_SUCCESS)
        {
            Status = ERROR_INVALID_ACCESS;
            ERRORMSG (1, (L"DCP get channel work packet failed!\r\n"));
            ASSERT(0);
            return Status;
        }

        Packet->NextPacket = NULL;

        Packet->Ctrl0.U = 0;
        Packet->Ctrl0.B.INTERRUPT = 1;
        Packet->Ctrl0.B.DECR_SEMAPHORE = 1;
        Packet->Ctrl0.B.ENABLE_MEMCOPY = 1;

        Packet->Ctrl1.U = 0;

        Packet->Source = Source;
        Packet->Destination = Destination;

        Packet->Size = Length;

        Packet->Payload = NULL;
        Packet->Status.U = 0;

        CacheRangeFlush(NULL, 0, CACHE_SYNC_DISCARD);
        
        Status = dcp_ExecutePackets(Handle, Packet, 1);

    }

    return Status;

}

//-----------------------------------------------------------------------------
//
// Function : dcp_memcopy
//
//  Performs a synchronous memory copy
//  Brief Performs a synchronous memory copy
//
//  This function performs a memory copy and suspends the calling thread until
//  the memory copy is complete.
//
// Parameters:
//      Source
//          [in] Pointer to the source data
//             
//      Destination   
//          [in] Physical address where the data will be copied to
//
//      Length
//          [in] Size, in bytes, to copy
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_memcopy(void *Source, void *Destination, UINT32 Length)
{
    RtResult      Status;
    DCPHandle_t     Handle;

    // Call the Async memory copy and then wait for it to complete.
    Status = dcp_memcopyAsync(Source, Destination, Length, NULL, NULL, &Handle);
    if (Status == ERROR_SUCCESS)
    {
        Status = dcp_WaitForComplete(Handle, INFINITE);
    }

    return Status;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_memfillAsync
//
// Performs an synchronous memory fill.
//
//  This function returns immediately so the calling thread will need to use a
//  callback or the returned handle to find out when the operation is complete.
//
// Parameters:
//      Source
//          [in] byte value to fill in memory
//             
//      DestinationPhy
//          [in] Physical address of  location to fill
//
//      DestinationVrt   
//          [in] Virtual address of  location to fill
//
//      Length
//          [in] Size, in bytes, to fill
//
//      Callback       
//          [in] Function pointer to a function to call when the
//              operation completes.  This value can be NULL if
//              no callback is required.
//
//      PrivateData    
//          [in] This pointer will be the argument when the callback
//              function is called.  This argument should be NULL 
//              if Callback is NULL and can be NULL is no privated 
//              data is needed.
//
//      RtHandle
//          [out] This value will contain the handle of the channel 
//              being used in this copy routine.  It can be NULL 
//              if the handle is not desired.  dcp_WaitForComplete
//              can be called with this handle to cause the thread
//              to wait for the operation to complete.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_memfillAsync(UINT8 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Length, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle)
{
    DCPHandle_t     Handle;
    DCPWorkPacket_t *Packet;
    RtResult      Status;
    UINT32      SizeOnWordBoundary;

    Status = dcp_AcquireChannel(&Handle, DCP_GET_ANY_CHANNEL, FALSE);
    if (Status == ERROR_SUCCESS)
    {
        Status = dcp_SetCallbackInfo(Handle, Callback, PrivateData);
        if(Status != ERROR_SUCCESS)
        {
            Status = ERROR_INVALID_ACCESS;
            ERRORMSG (1, (L"DCP Invalid Handle!\r\n"));
            ASSERT(0);
            return Status;
        }

        if (RtHandle != NULL)
        {
            *RtHandle = Handle;
        }

        Status = dcp_GetChannelWorkPacket(Handle, &Packet);
        if(Status != ERROR_SUCCESS)
        {
            Status = ERROR_INVALID_ACCESS;
            ERRORMSG (1, (L"DCP get channel work packet failed!\r\n"));
            ASSERT(0);
            return Status;
        }

        Packet->NextPacket = NULL;

        Packet->Ctrl0.U = 0;
        Packet->Ctrl0.B.INTERRUPT = 1;
        Packet->Ctrl0.B.DECR_SEMAPHORE = 1;
        Packet->Ctrl0.B.ENABLE_MEMCOPY = 1;
        Packet->Ctrl0.B.CONSTANT_FILL = 1;
        Packet->Ctrl1.U = 0;

        // Fill the 32-bit register with 4 copies of Source
        Packet->Source = (void*)(Source + Source*0x100 + Source*0x10000 + Source*0x1000000);

        Packet->Destination = DestinationPhy;

        // 6/20/07
        // The DCP hardware has a problem and can only fill 32bit words.
        // If Length % 4 != 0 then the last 1, 2 or 3 bytes will not
        // get valid values.  To compensate for this I use the DCP to
        // fill all words and then use memset to fill the 3 or less
        // bytes.
        SizeOnWordBoundary = Length & 0xfffffffc;
        Packet->Size = SizeOnWordBoundary;

        Packet->Payload = NULL;
        Packet->Status.U = 0;

        CacheRangeFlush(DestinationVrt, Length, CACHE_SYNC_DISCARD);

        Status = dcp_ExecutePackets(Handle, Packet, 1);

        // If there are bytes left to fill use memset to fill them.
        // see reason above.
        Length = Length -SizeOnWordBoundary;
        if (Length > 0)
        {
            memset((void*)((UINT32) DestinationVrt + SizeOnWordBoundary),
                Source, Length);
        }

    }

    return Status;

}

//-----------------------------------------------------------------------------
//
// Function : dcp_memfill
//
// Performs a synchronous memory fill
//
// This function performs a memory fill and suspends the calling thread until
// the memory fill is complete.
//
// Parameters:
//      Source        
//          [in] unsigned char value used to fill the memory
//             
//      DestinationPhy   
//          [in] physical address where the data that will be filled with Source
//
//      DestinationVrt   
//          [in] Virtual address where the data that will be filled with Source
//
//      Length        
//          [in] Size, in bytes, to copy
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_memfill(UINT8 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Length)
{
    RtResult      Status;
    DCPHandle_t     Handle;

    // Call the Async memory copy and then wait for it to complete.
    Status = dcp_memfillAsync(Source, DestinationPhy, DestinationVrt, Length, NULL, NULL, &Handle);
    if (Status == ERROR_SUCCESS)
    {
        Status = dcp_WaitForComplete(Handle, INFINITE);
    }

    return Status;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_WaitForComplete
//
//  Wait for a channel to complete it's operation.
//
//  This function will suspend the calling thread until the channel has
//  completed it operation.
//
// Parameters:
//      Handle
//          [in] Valid handle to the channel
//             
//      Timeout 
//          [in] Timeout in milliseconds to wait for the channel to
//              complete.  Can be set to TX_WAIT_FOREVER to wait
//              forever.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_WaitForComplete(DCPHandle_t Handle, UINT32 TimeOut)
{
    UINT32    Channel;
    RtResult  Status;

    Status = dcp_GetChannelNumber(Handle, &Channel);

    if (Status != ERROR_SUCCESS) return ERROR_SUCCESS;

    if (dcp_WaitOnChannel(Channel, TimeOut) != ERROR_SUCCESS)
    return ERROR_INVALID_ACCESS;

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_LockChannel
//
//  Acquires a dedicated DCP channel to be used until it's freed.
//
//  This enables processes to grab a DCP channel and use it over and over 
//  without having it returned to the system.  It guarantees that the process
//  will always have a DCP channel.  Locking a channel guarantees that you
//  can have a DCP channel when you need one but also reduces the number of 
//  DCP channels left for the rest of the system.
//
// Parameters:
//      Handle
//          [out] pointer to where the handle will be stored.
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
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_LockChannel(DCPHandle_t *Handle, UINT32 DesiredChannel)
{
    return dcp_AcquireChannel(Handle, DesiredChannel, TRUE);
}

//-----------------------------------------------------------------------------
//
// Function : dcp_ReleaseLockedChannel
//
//  Releases a DCP channel acquired from dcp_AcquireChannel where LockIt
//  was set to TRUE.
//
// Parameters:
//      Handle
//          [in] The handle returned from dcp_AcquireChannel
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_ReleaseLockedChannel(DCPHandle_t Handle)
{
    UINT32    Channel;
    RtResult  Status;

    Status = dcp_GetChannelNumber(Handle, &Channel);

    if (Status == ERROR_SUCCESS)
    {
        // We can't release the channel until it has completed
        // it operations.
        if (dcp_WaitOnChannel(Channel, 3000) != ERROR_SUCCESS)
        {
            ERRORMSG (1, (L"DCP wait on channel failed!\r\n"));
            ASSERT(0);
            return WAIT_TIMEOUT;
        }

        dcp_MakeChannelAvailable(Channel);

    }
    else
    {
        // If we hit this SystemHalt if probably means
        // that the Handle was not acquired with a lock
        // and the channel has already been released.
        ERRORMSG (1, (L"DCP Invalid Handle!\r\n"));
        ASSERT(0);
    }

    return Status;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_SetChannelPriority
//
//  Set the priority of a channel
//
// Parameters:
//      Channel
//          [in] Channel number
//             
//      Priority
//          [in] Priority value to set the channel to.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_SetChannelPriority(UINT32 Channel, DCPPriority_t Priority)
{
    RtResult Status;

    if(Channel >= DCP_MAX_CHANNELS)
    {
        ERRORMSG (1, (L"DCP Invalid Channel!\r\n"));
        ASSERT(0);
        return ERROR_INVALID_ACCESS;
    }

    switch(Priority)
    {
        case DCP_HIGH_PRIORITY:
        Status = dcp_hw_SetChannelHighPriority(Channel, TRUE);
        break;

        case DCP_LOW_PRIORITY:
        Status = dcp_hw_SetChannelHighPriority(Channel, TRUE);
        break;

        default:
        Status = ERROR_INVALID_DATA;
    }

    return Status;
}

//-----------------------------------------------------------------------------
//
// Function : dcp_blt
//
//  Perform a graphic BLT operation
//
//  The DCP also has the ability to do basic "blit" operations that are typical 
//  in graphics operations. Blit source buffers must be contiguous. The output 
//  destination buffer for a blit operation is defined as a "M runs of N bytes" 
//  that define a rectangular region in a frame buffer. For blit operations, 
//  each line of the blit may consist of any number of bytes.  
//  
//  This function returns immediately and does not wait for the BLT to complete
//
// Parameters:
//      Source
//          [in] Pointer to the source images.  MUST BE CONTIGUOUS
//             
//      Destination   
//          [in] Physical address Where the image is copied to.
//
//      Width         
//          [in] Source width, in bytes
//
//      Height        
//          [in] Source height
//
//      Stride        
//          [in] Destination width, in bytes
//
//      Callback      
//          [in] Function pointer to a function to call when the
//              operation completes.  This value can be NULL if
//              no callback is required.
//
//      PrivateData   
//          [in] This pointer will be the argument when the callback
//              function is called.  This argument should be NULL 
//              if Callback is NULL and can be NULL is no privated 
//              data is needed.
//
//      RtHandle     
//          [out] This value will contain the handle of the channel 
//              being used in this copy routine.  It can be NULL 
//              if the handle is not desired.  dcp_WaitForComplete
//              can be called with this handle to cause the thread
//              to wait for the operation to complete.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_blt(void *Source, void *Destination, UINT32 Width, UINT32 Height, UINT32 Stride, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle)
{
    DCPHandle_t     Handle;
    DCPWorkPacket_t *Packet;
    RtResult      Status;

    Status = dcp_AcquireChannel(&Handle, DCP_GET_ANY_CHANNEL, FALSE);
    if (Status != ERROR_SUCCESS) return Status;

    Status = dcp_SetCallbackInfo(Handle, Callback, PrivateData);
    if(Status != ERROR_SUCCESS)
    {
        Status = ERROR_INVALID_ACCESS;
        ERRORMSG (1, (L"DCP Invalid Handle!\r\n"));
        ASSERT(0);
        return Status;
    }

    if (RtHandle != NULL)
    *RtHandle = Handle;

    Status = dcp_GetChannelWorkPacket(Handle, &Packet);
    if(Status != ERROR_SUCCESS)
    {
        Status = ERROR_INVALID_ACCESS;
        ERRORMSG (1, (L"DCP get channel work packet failed!\r\n"));
        ASSERT(0);
        return Status;
    }

    Packet->NextPacket = NULL;

    Packet->Ctrl0.U = 0;
    Packet->Ctrl0.B.INTERRUPT = 1;
    Packet->Ctrl0.B.DECR_SEMAPHORE = 1;
    Packet->Ctrl0.B.ENABLE_BLIT = 1;

    Packet->Ctrl1.U = Stride;

    Packet->Source = Source;
    Packet->Destination = Destination;

    Packet->Size = (Height << 16) | Width;

    Packet->Payload = NULL;
    Packet->Status.U = 0;
    
    CacheRangeFlush(NULL, 0, CACHE_SYNC_DISCARD);

    return dcp_ExecutePackets(Handle, Packet, 1);
}

//-----------------------------------------------------------------------------
//
// Function : dcp_bltfill
//
//  Perform a graphic BLT fill operation
//
//  Same as the dcp_blt function execpt instead of BLTting an image this
//  function BLTs a constant value specified by Source
//
//  This function returns immediately and does not wait for the BLT fill to complete
//
// Parameters:
//      Source
//          [in] unsigned char value the reqion willget filled with
//             
//      DestinationPhy
//          [in] Physical address of location where the fill will occur.
//
//      DestinationVrt   
//          [in] Virtual address of location where the fill will occur.
//
//      Width
//          [in] Width of region to fill
//
//      Height        
//          [in] Height of region to fill
//
//      Stride        
//          [in] Destination width, in bytes
//
//      Callback      
//          [in] Function pointer to a function to call when the
//              operation completes.  This value can be NULL if
//              no callback is required.
//
//      PrivateData   
//          [in] This pointer will be the argument when the callback
//              unction is called.  This argument should be NULL 
//              if Callback is NULL and can be NULL is no privated 
//              data is needed.
//  
//      RtHandle     
//          [out] This value will contain the handle of the channel 
//              being used in this copy routine.  It can be NULL 
//              f the handle is not desired.  dcp_WaitForComplete
//              can be called with this handle to cause the thread
//              to wait for the operation to complete.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//-----------------------------------------------------------------------------
RtResult dcp_bltfill(UINT32 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Width, UINT32 Height, UINT32 Stride, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle)
{
    DCPHandle_t     Handle;
    DCPWorkPacket_t *Packet;
    RtResult      Status;

    Status = dcp_AcquireChannel(&Handle, DCP_GET_ANY_CHANNEL, FALSE);
    if (Status != ERROR_SUCCESS) return Status;

    Status = dcp_SetCallbackInfo(Handle, Callback, PrivateData);
    if(Status != ERROR_SUCCESS)
    {
        Status = ERROR_INVALID_ACCESS;
        ERRORMSG (1, (L"DCP Invalid Handle!\r\n"));
        ASSERT(0);
        return Status;
    }

    if (RtHandle != NULL) *RtHandle = Handle;

    Status = dcp_GetChannelWorkPacket(Handle, &Packet);
    if(Status != ERROR_SUCCESS)
    {
        Status = ERROR_INVALID_ACCESS;
        ERRORMSG (1, (L"DCP get channel work packet failed!\r\n"));
        ASSERT(0);
        return Status;
    }

    Packet->NextPacket = NULL;

    Packet->Ctrl0.U = 0;
    Packet->Ctrl0.B.INTERRUPT = 1;
    Packet->Ctrl0.B.DECR_SEMAPHORE = 1;
    Packet->Ctrl0.B.ENABLE_BLIT = 1;
    Packet->Ctrl0.B.CONSTANT_FILL = 1;

    Packet->Ctrl1.U = Stride;

    Packet->Source = (void*)Source;
    Packet->Destination = DestinationPhy;

    Packet->Size = (Height << 16) | Width;

    Packet->Payload = NULL;
    Packet->Status.U = 0;

    CacheRangeFlush(DestinationVrt, Stride * Height, CACHE_SYNC_DISCARD);

    return dcp_ExecutePackets(Handle, Packet, 1);
}




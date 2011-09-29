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
/////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes and external references
////////////////////////////////////////////////////////////////////////////////
#include <string.h>
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)

#include "ddi_dcp_os_private.h"

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_WaitOnChannel(UINT32 Channel, UINT32 TimeOut);
HRESULT dcp_MakeChannelAvailable(UINT32 Channel);

////////////////////////////////////////////////////////////////////////////////
//! \brief Performs an asynchronous memory copy.
//!
//! This function returns immediately so the calling thread will need to use a
//! callback or the returned handle to find out when the operation is complete.
//!
//! \fntype Function
//!
//! \param[in]  Source        Pointer to the source data
//!             
//! \param[in]  Destination   Physical address where the data will be copied to
//!
//! \param[in]  Length        Size, in bytes, to copy
//!
//! \param[in] Callback       Function pointer to a function to call when the
//!                           operation completes.  This value can be NULL if
//!                           no callback is required.
//!
//! \param[in] PrivateData    This pointer will be the argument when the callback
//!                           function is called.  This argument should be NULL 
//!                           if Callback is NULL and can be NULL is no privated 
//!                           data is needed.
//!
//! \param[out] RtHandle      This value will contain the handle of the channel 
//!                           being used in this copy routine.  It can be NULL 
//!                           if the handle is not desired.  dcp_WaitForComplete
//!                           can be called with this handle to cause the thread
//!                           to wait for the operation to complete.
//!
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_memcopyAsync(void *Source, void *Destination, UINT32 Length, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle)
{
    DCPHandle_t     Handle;
    DCPWorkPacket_t *Packet;
    HRESULT      Status;

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

////////////////////////////////////////////////////////////////////////////////
//! \brief Performs a synchronous memory copy
//!
//! This function performs a memory copy and suspends the calling thread until
//! the memory copy is complete.
//!
//! \fntype Function
//!
//! \param[in]  Source        Pointer to the source data
//!             
//! \param[in]  Destination   Physical address where the data will be copied to
//!
//! \param[in]  Length        Size, in bytes, to copy
//!
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_memcopy(void *Source, void *Destination, UINT32 Length)
{
    HRESULT      Status;
    DCPHandle_t     Handle;

    // Call the Async memory copy and then wait for it to complete.
    Status = dcp_memcopyAsync(Source, Destination, Length, NULL, NULL, &Handle);
    if (Status == ERROR_SUCCESS)
    {
        Status = dcp_WaitForComplete(Handle, INFINITE);
    }

    return Status;
}

////////////////////////////////////////////////////////////////////////////////
//! \brief Performs an synchronous memory fill.
//!
//! This function returns immediately so the calling thread will need to use a
//! callback or the returned handle to find out when the operation is complete.
//!
//! \fntype Function
//!
//! \param[in]  Source        byte value to fill in memory
//!             
//! \param[in]  DestinationPhy   Physical address of  location to fill
//!
//! \param[in]  DestinationVrt   Virtual address of  location to fill
//!
//! \param[in]  Length        Size, in bytes, to fill
//!
//! \param[in] Callback       Function pointer to a function to call when the
//!                           operation completes.  This value can be NULL if
//!                           no callback is required.
//!
//! \param[in] PrivateData    This pointer will be the argument when the callback
//!                           function is called.  This argument should be NULL 
//!                           if Callback is NULL and can be NULL is no privated 
//!                           data is needed.
//!
//! \param[out] RtHandle      This value will contain the handle of the channel 
//!                           being used in this copy routine.  It can be NULL 
//!                           if the handle is not desired.  dcp_WaitForComplete
//!                           can be called with this handle to cause the thread
//!                           to wait for the operation to complete.
//!
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_memfillAsync(UINT8 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Length, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle)
{
    DCPHandle_t     Handle;
    DCPWorkPacket_t *Packet;
    HRESULT      Status;
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

////////////////////////////////////////////////////////////////////////////////
//! \brief Performs a synchronous memory fill
//!
//! This function performs a memory fill and suspends the calling thread until
//! the memory fill is complete.
//!
//! \fntype Function
//!
//! \param[in]  Source        unsigned char value used to fill the memory
//!             
//! \param[in]  DestinationPhy   physical address where the data that will be filled with Source
//!
//! \param[in]  DestinationVrt   Virtual address where the data that will be filled with Source
//!
//! \param[in]  Length        Size, in bytes, to copy
//!
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_memfill(UINT8 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Length)
{
    HRESULT      Status;
    DCPHandle_t     Handle;

    // Call the Async memory copy and then wait for it to complete.
    Status = dcp_memfillAsync(Source, DestinationPhy, DestinationVrt, Length, NULL, NULL, &Handle);
    if (Status == ERROR_SUCCESS)
    {
        Status = dcp_WaitForComplete(Handle, INFINITE);
    }

    return Status;
}

////////////////////////////////////////////////////////////////////////////////
//! \brief Wait for a channel to complete it's operation.
//!
//! This function will suspend the calling thread until the channel has
//! completed it operation.
//!
//! \fntype Function
//!
//! \param[in]  Handle - Valid handle to the channel
//!             
//! \param[in]  Timeout - Timeout in milliseconds to wait for the channel to
//!                       complete.  Can be set to TX_WAIT_FOREVER to wait
//!                       forever.
//!
//! \retval SUCCESS            - No error
//! \retval ERROR_DDI_DCP_FAIL - Timed out waiting for the channel to complete
//! 
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_WaitForComplete(DCPHandle_t Handle, UINT32 TimeOut)
{
    UINT32    Channel;
    HRESULT  Status;

    Status = dcp_GetChannelNumber(Handle, &Channel);

    if (Status != ERROR_SUCCESS) return ERROR_SUCCESS;

    if (dcp_WaitOnChannel(Channel, TimeOut) != ERROR_SUCCESS)
    return ERROR_INVALID_ACCESS;

    return ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! \brief Acquires a dedicated DCP channel to be used until it's freed.
//!
//! This enables processes to grab a DCP channel and use it over and over 
//! without having it returned to the system.  It guarantees that the process
//! will always have a DCP channel.  Locking a channel guarantees that you
//! can have a DCP channel when you need one but also reduces the number of 
//! DCP channels left for the rest of the system.
//! 
//! \fntype Function
//!
//! \param[out]  Handle - pointer to where the handle will be stored.
//!             
//! \param[in]  DesiredChannel - Lets the thread specify what channel number it
//!                   would like to have.  This is only needed for the VMI
//!                   because it needs to be channel 0.  Channels 1 through
//!                   DCP_MAX_CHANNELS are all the same.  If this value is set
//!                   to DCP_GET_ANY_CHANNEL or any value greater than
//!                   DCP_MAX_CHANNELS the function we get the next available
//!                   channel.
//!
//! \retval SUCCESS No error
//! \retval ERROR_DDI_DCP_MAX_LOCKED - 
//! \retval ERROR_DDI_DCP_CHANNEL_NOT_AVAILABLE
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_LockChannel(DCPHandle_t *Handle, UINT32 DesiredChannel)
{
    return dcp_AcquireChannel(Handle, DesiredChannel, TRUE);
}

////////////////////////////////////////////////////////////////////////////////
//! \brief Releases a DCP channel acquired from dcp_AcquireChannel where LockIt
//!        was set to TRUE.
//!
//! \fntype Function
//!
//! \param[in] Handle - The handle returned from dcp_AcquireChannel
//!             
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_ReleaseLockedChannel(DCPHandle_t Handle)
{
    UINT32    Channel;
    HRESULT  Status;

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

////////////////////////////////////////////////////////////////////////////////
//! \brief Set the priority of a channel
//!
//! \fntype Function
//!
//! \param[in]  Channel - Channel number
//!             
//! \param[in]  Priority - Priority value to set the channel to.
//!
//! \retval SUCCESS             No error
//! \retval ERROR_DDI_DCP_FAIL  Failed
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_SetChannelPriority(UINT32 Channel, DCPPriority_t Priority)
{
    HRESULT Status;

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

////////////////////////////////////////////////////////////////////////////////
//! \brief Perform a graphic BLT operation
//!
//! The DCP also has the ability to do basic "blit" operations that are typical 
//! in graphics operations. Blit source buffers must be contiguous. The output 
//! destination buffer for a blit operation is defined as a "M runs of N bytes" 
//! that define a rectangular region in a frame buffer. For blit operations, 
//! each line of the blit may consist of any number of bytes.  
//!
//! This function returns immediately and does not wait for the BLT to complete
//!
//! \fntype Function
//!
//! \param[in]  Source        Pointer to the source images.  MUST BE CONTIGUOUS
//!             
//! \param[in]  Destination   Physical address Where the image is copied to.
//!
//! \param[in]  Width         Source width, in bytes
//!
//! \param[in]  Height        Source height
//!
//! \param[in]  Stride        Destination width, in bytes
//!
//! \param[in]  Callback      Function pointer to a function to call when the
//!                           operation completes.  This value can be NULL if
//!                           no callback is required.
//!
//! \param[in]  PrivateData   This pointer will be the argument when the callback
//!                           function is called.  This argument should be NULL 
//!                           if Callback is NULL and can be NULL is no privated 
//!                           data is needed.
//!
//! \param[out]  RtHandle     This value will contain the handle of the channel 
//!                           being used in this copy routine.  It can be NULL 
//!                           if the handle is not desired.  dcp_WaitForComplete
//!                           can be called with this handle to cause the thread
//!                           to wait for the operation to complete.
//!
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_blt(void *Source, void *Destination, UINT32 Width, UINT32 Height, UINT32 Stride, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle)
{
    DCPHandle_t     Handle;
    DCPWorkPacket_t *Packet;
    HRESULT      Status;

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

////////////////////////////////////////////////////////////////////////////////
//! \brief Perform a graphic BLT fill operation
//!
//! Same as the dcp_blt function execpt instead of BLTting an image this
//! function BLTs a constant value specified by Source
//!
//! This function returns immediately and does not wait for the BLT fill to complete
//!
//! \fntype Function
//!
//! \param[in]  Source        unsigned char value the reqion willget filled with
//!             
//! \param[in]  DestinationPhy   Physical address of location where the fill will occur.
//!
//! \param[in]  DestinationVrt   Virtual address of location where the fill will occur.
//!
//! \param[in]  Width         Width of region to fill
//!
//! \param[in]  Height        Height of region to fill
//!
//! \param[in]  Stride        Destination width, in bytes
//!
//! \param[in]  Callback      Function pointer to a function to call when the
//!                           operation completes.  This value can be NULL if
//!                           no callback is required.
//!
//! \param[in]  PrivateData   This pointer will be the argument when the callback
//!                           function is called.  This argument should be NULL 
//!                           if Callback is NULL and can be NULL is no privated 
//!                           data is needed.
//!
//! \param[out]  RtHandle     This value will contain the handle of the channel 
//!                           being used in this copy routine.  It can be NULL 
//!                           if the handle is not desired.  dcp_WaitForComplete
//!                           can be called with this handle to cause the thread
//!                           to wait for the operation to complete.
//!
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_bltfill(UINT32 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Width, UINT32 Height, UINT32 Stride, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle)
{
    DCPHandle_t     Handle;
    DCPWorkPacket_t *Packet;
    HRESULT      Status;

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

////////////////////////////////////////////////////////////////////////////////
//! \brief Performs color space conversion on a frame of data.
//!
//! The DCP module supports color-space conversion to assist software in video
//! processing.  The function of the color-space converter is to reformat the 
//! YUV-formatted video buffer created by the video decode algorithm into the 
//! RGB color format required for display on an LCD.
//!
//!    * The color-space converter will accept YUV or YCbCr data in planar
//!      4:2:0 or 4:2:2 format. The output of the CSC is either a 16-bit 
//!      RGB frame buffer (565 format), a 24-bit unpacked frame buffer 
//!      (24-bit pixels stored as a 32-bit word), or an interleaved YCbCr
//!      4:2:2 format to support LCD displays that accept YCbCr data.
//!
//!    * In addition, the logic supports 8-bit delta pixel displays by
//!      simple reordering of RGB components on odd scan lines
//!      (selectable via mode bits).
//!
//! \fntype Function
//!
//! Input and output parameters are set using the variable type CSCBuffer_t.
//! This structure type has the following elements:
//!
//!     void        *pRGBBuffer Destination data pointer.  Not used when the 
//!                             variable is used for the pInput parameter.
//!     void        *pYBuffer   Pointer to the Y component video input or the 
//!                             interlaced data if the input is YCbCr.  Not 
//!                             used when the variable is used for pOutput 
//!                             parameter.
//!     void        *pUBuffer   Pointer to the U component video input.  Not 
//!                             used if the input format is YCbCr or if the 
//!                             variable is used for pOutput parameter.
//!     void        *pVBuffer   Pointer to the V component video input.  Not
//!                             used if the input format is YCbCr or if the 
//!                             variable is used for pOutput parameter.
//!     UINT32    Width       Width of the input when used for pInput and 
//!                             output when used for pOutput.
//!     UINT32    Height      Height of the input when used for pInput and
//!                             output when used for pOutput.
//!     UINT32    Stride      Width of the output buffer and is only used
//!                             when the variable is for pInput.  This 
//!                             variable allows only a portion of the video
//!                             to be converted and not the full frame.
//!     CSCFormat_t Format      The format of the video.
//!
//!###########################################################################
//!
//! \param[in]  pInput        Pointer to a CSCBuffer type structure containing 
//!                           information about the source data
//!             
//! \param[in]  pOutput       Pointer to a CSCBuffer type structure containing
//!                           information about the destination data
//!             
//! \param[in]  bRotate       Set to TRUE to rotate the final image.
//!             
//! \param[in]  Callback      Function pointer to a function to call when the
//!                           operation completes.  If this parameter is NULL
//!                           the function will not return until the conversion
//!                           is complete.
//!
//! \param[in]  PrivateData   This pointer will be the argument when the callback
//!                           function is called.  This argument should be NULL 
//!                           if Callback is NULL and can be NULL is no privated 
//!                           data is needed.
//!
//! \param[in]  pCoefficients Pointer to a set of coefficients to use for the
//!                           color space conversion.  Set to NULL to use the
//!                           default values.
//!
//!###########################################################################
//!
//! EXAMPLE:
//! 
//!   This is how you would setup the structures to take a video input of 320x240 
//!   and only grab the first 160 pixels of the input video, color space convert 
//!   them and then scale that to 200 x 300.    
//!
//!      CSCBuffer_t In;
//!      CSCBuffer_t Out;
//!      
//!      In.pYBuffer = Point to Y data;
//!      In.pUBuffer = Point to U data;
//!      In.pVBuffer = Point to V data;
//!      In.Stride = 320;        // Set to the width of the incomming video
//!      In.Width = 160;         // Number of pixles to grab
//!      In.Height = 240;        // Number of lines to grab
//!      In.Format = DCP_YUV420; // Format of in comming video.
//!      
//!      Out.pRGBBuffer = Where the converted image gets stored;
//!      Out.Width = 200;        // Output Size 
//!      Out.Height = 300;       // 
//!      Out.Format = DCP_RGB16_565;
//!      
//!      dcp_csc_DoConversion(&In, &Out, FALSE, NULL, NULL, NULL);
//!      
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_csc_DoConversion(CSCBuffer_t *pInput, CSCBuffer_t *pOutput, BOOL bRotate, DCPCallback_t pCallbackFunc, void *PrivateData, CSCCoefficients_t *pCoefficients)
{
    HRESULT  Status;

    // Wait your turn for the CSC
    Status = dcp_csc_GetSemaphore(INFINITE);
    if (Status == ERROR_SUCCESS)
    {
        // flush the entire cache
        CacheRangeFlush(NULL, 0, CACHE_SYNC_DISCARD);

        dcp_csc_SetCallbackInfo(pCallbackFunc, PrivateData);

        // Set the coefficients.  If this parameter is NULL
        // default coefficients will be used.
        dcp_csc_SetCoefficients(pCoefficients);

        dcp_hw_SetOutputBuffer(pOutput->pRGBBuffer);
        dcp_hw_SetInputBuffer(pInput->pYBuffer,
            pInput->pUBuffer,
            pInput->pVBuffer);

        Status = dcp_hw_SetOutputFormat(pOutput->Format);
        if (Status != ERROR_SUCCESS) 
        {
            // If there is an error put the semaphore back
            // and return
            dcp_csc_PutSemaphore();
            return Status;
        }

        Status = dcp_hw_SetInputFormat(pInput->Format);
        if (Status != ERROR_SUCCESS) 
        {
            // If there is an error put the semaphore back
            // and return
            dcp_csc_PutSemaphore();
            return Status;
        }

        // If we are using a stride then we are only grabing
        // a portion of the input video.
        if (pInput->Stride > 0)
        {
            // The stride has to be greater than the number
            // of pixels we are grabbing.
            if(pInput->Stride < pInput->Width)
            {
                Status = ERROR_INVALID_DATA;
                ERRORMSG (1, (L"Invalid data for DCP CSC!\r\n"));
                ASSERT(0);
                return Status;
            }
            dcp_hw_SetInputSize(pInput->Stride, pInput->Height);
        }
        else
        {
            dcp_hw_SetInputSize(pInput->Width, pInput->Height);
        }

        // It may seem strange to be using input values on the output
        // register but this is pre-CSC.
        dcp_hw_SetOutputSize(pInput->Width, pInput->Height);

        dcp_hw_SetScale(pInput->Width, pInput->Height,
            pOutput->Width, pOutput->Height);

        dcp_hw_Rotate(bRotate);

        dcp_hw_RunCSC();

        // If there is no callback then wait for the conversion
        // to complete before we return.
        if (pCallbackFunc == NULL) 
        {
            dcp_csc_WaitForComplete(INFINITE);
        }
    }

    return ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! \brief Set the priority of the color space converter to the other DCP
//!        Channels.
//!
//! \fntype Function
//!
//! \param[in] Priority 
//!             
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_csc_SetPriority(DCPPriority_t Priority)
{
    HRESULT Status;

    if((Priority < DCP_BACKGROUND)||(Priority > DCP_HIGH_PRIORITY))
    {
        Status = ERROR_INVALID_DATA;
        ERRORMSG (1, (L"Invalid priority setting for DCP CSC!\r\n"));
        ASSERT(0);
        return Status;
    }

    Status = dcp_hw_CSCSetPriority((UINT32)Priority);

    return Status;
}



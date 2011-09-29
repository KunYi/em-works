//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ddi_dcp_common.c
//  Brief data co-processor interface
//
//
/////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes and external references
////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)
#include "ddi_dcp.h"

DCPChannel_t* g_DCPChannel;

////////////////////////////////////////////////////////////////////////////////
//! \brief Increments a channles handle.
//!
//! This function is called when  a handle has been released.  By incrementing
//! the handle we invalidate all previous handles.
//!
//! The current handle format is 0xCnnnnnnn where C is the actual channel number
//! and nnnnnnn is a counter starting at 1 when the DCP is initialized.
//!
//! \fntype Function
//!
//! \param[in]  Channel - Channel number
//!             
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_IncrementHandle(UINT32 Channel)
{
    UINT32 i;

    if(Channel >= DCP_MAX_CHANNELS)
    {
        ERRORMSG (1, (L"Invalid channel number for DCP!\r\n"));
        ASSERT(0);
        return ERROR_INVALID_ACCESS;
    }

    // Get the current value.
    i = g_DCPChannel[Channel].Handle;

    // mask off the channel information
    i = i & DCP_HANDLE_MASK;

    // Increment the value and put the
    // Channel information back
    i++;
    i |= (Channel << DCP_HANDLE_SHIFT);

    g_DCPChannel[Channel].Handle = i;

    return ERROR_SUCCESS;
}





//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ddi_dcp_init.c
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

#pragma warning(push)
#pragma warning(disable: 4214)
#include <ceddk.h>
#pragma warning(pop)

#include "ddi_dcp_os_private.h"

////////////////////////////////////////////////////////////////////////////////
// Externs
////////////////////////////////////////////////////////////////////////////////
extern CSCCoefficients_t    g_DefaultCoefficients;
extern PVOID    pv_HWregDCP;    

extern DCPChannel_t* g_DCPChannel;

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////
BOOL g_DCPInitialized = FALSE;

// Prototype for increment handle.  I don't want
// to put this in ddi_dcp.h because I don't want
// code outsize of this DDI driver to call it.
HRESULT dcp_IncrementHandle(UINT32 Channel);

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
void ddi_dcp_isr(void *pData);
void ddi_dcp_vmi_isr(void *pData);

////////////////////////////////////////////////////////////////////////////////
//! \brief
//!
//! \fntype Function
//!
//! \param[in]
//!             
//! \retval SUCCESS No error
//!
////////////////////////////////////////////////////////////////////////////////
HRESULT dcp_Initialize()
{
    INT32 i;
    DMA_ADAPTER_OBJECT dcp_adapter;

    // If g_DCPInitialized is not FALSE then
    // we have already initialized the
    // DCP.
    if (g_DCPInitialized != FALSE) return ERROR_SUCCESS;

    // Initialize the hardware
    dcp_hw_Initialize();

    // Keep the VMI channel's IRQ the same as the
    // other channels until the VMI channel is
    // acquired.
    dcp_hw_VMIMergeIRQ(TRUE);

    // Until a channel is acquired or the color
    // space converter is used we will keep the 
    // DCP off to save power.
    dcp_hw_Enable(FALSE);

    g_DCPChannel = HalAllocateCommonBuffer(&dcp_adapter,
        sizeof(DCPChannel_t) * DCP_MAX_CHANNELS,
        &dcp_phys_address,
        FALSE);

    for (i = 0; i < DCP_MAX_CHANNELS; i++)
    {
        g_DCPChannel[i].Available = TRUE;
        g_DCPChannel[i].Locked = FALSE;

        // Clear the handle and then increment it
        // to 1.  I don't wan't to just assign the 
        // handle 1 in case we change the
        // behaviour of dcp_IncrementHandle
        g_DCPChannel[i].Handle = 0;
        dcp_IncrementHandle(i);
    }

    dcp_os_Init(g_DCPChannel, DCP_MAX_CHANNELS);

    dcp_hw_ContextSwitching(FALSE, NULL);

    dcp_isr_Init();

    // Load the default coefficients for the
    // color space converter
    g_DefaultCoefficients.Register0.U = HW_DCP_CSCCOEFF0_RD();
    g_DefaultCoefficients.Register1.U = HW_DCP_CSCCOEFF1_RD();
    g_DefaultCoefficients.Register2.U = HW_DCP_CSCCOEFF2_RD();

    g_DCPInitialized = TRUE;

    return ERROR_SUCCESS;
}


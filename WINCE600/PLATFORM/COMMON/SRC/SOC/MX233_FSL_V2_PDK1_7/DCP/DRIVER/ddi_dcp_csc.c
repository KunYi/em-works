//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//  File: ddi_dcp_csc.c
//! Brief: data co-processor interface
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

#include "ddi_dcp_os_private.h"

////////////////////////////////////////////////////////////////////////////////
// Externs
////////////////////////////////////////////////////////////////////////////////
extern BOOL g_DCPInitialized;
extern PVOID pv_HWregDCP;
////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////
BOOL                g_cscCallbackEnable = FALSE;
DCPCallback_t       g_cscCallback = NULL;
void                *g_cscCallbackData = NULL;
CSCCoefficients_t   g_DefaultCoefficients;

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
HRESULT dcp_csc_SetCallbackInfo(DCPCallback_t CallbackFunction, void *PrivateData)
{
    if (CallbackFunction != NULL)
    {
        g_cscCallback = CallbackFunction;
        g_cscCallbackData = PrivateData;
        g_cscCallbackEnable = TRUE;
    }
    else
    {
        g_cscCallback = NULL;
        g_cscCallbackData = NULL;
        g_cscCallbackEnable = FALSE;
    }

    return ERROR_SUCCESS;
}

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
HRESULT dcp_csc_WaitForComplete(UINT32 TimeOut)
{
    HRESULT Status;

    Status = dcp_csc_GetSemaphore(TimeOut);
    if (Status == ERROR_SUCCESS)
    {
        dcp_csc_PutSemaphore();
    }

    return Status;
}

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
HRESULT dcp_csc_SetCoefficients(CSCCoefficients_t *Coefficients)
{
    if (Coefficients == NULL)
    {
        // Use the default.  We need to set the coefficients every
        // time because multiple threads can be using the CSC and
        // we don't know if a previous thread changed them.
        HW_DCP_CSCCOEFF0_WR(g_DefaultCoefficients.Register0.U);
        HW_DCP_CSCCOEFF1_WR(g_DefaultCoefficients.Register1.U);
        HW_DCP_CSCCOEFF2_WR(g_DefaultCoefficients.Register2.U);
    }
    else
    {
        HW_DCP_CSCCOEFF0_WR(Coefficients->Register0.U);
        HW_DCP_CSCCOEFF1_WR(Coefficients->Register1.U);
        HW_DCP_CSCCOEFF2_WR(Coefficients->Register2.U);
    }

    return ERROR_SUCCESS;
}


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
BOOL dcp_csc_Available()
{
    return (dcp_csc_GetSemaphoreCount() != 0);
}


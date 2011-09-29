//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  cspipu_camera.c
//
//  Provides Internal Memory configuration routines for
//  the camera driver.
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
#define CSP_IPU_IMA_TPM_ENC_CSC1 (0x321) //MX35-0x321
#define CSP_IPU_IMA_TPM_VF_CSC1 (0x645) //MX35-0x645
#define CSP_IPU_IMA_TPM_VF_CSC2 (0x648) //MX35-0x648



//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function:  IPUGetTPMEncCSC1Addr
//
// This function returns the internal memory address for the
// IPU IMA IC Task Parameter(Encoding CSC1) memory.
//
// Parameters:
//      None.
//
// Returns:
//      Internal memory address for IPU IMA IC TPM Enc CSC1.
//
//-----------------------------------------------------------------------------
UINT32 IPUGetTPMEncCSC1Addr(void)
{
    return CSP_IPU_IMA_TPM_ENC_CSC1;
}

//
// Function:  IPUGetTPMVFCSC1Addr
//
// This function returns the internal memory address for the
// IPU IMA IC Task Parameter(Viewfinder CSC1) memory.
//
// Parameters:
//      None.
//
// Returns:
//      Internal memory address for IPU IMA IC TPM VF CSC1.
//
//-----------------------------------------------------------------------------
UINT32 IPUGetTPMVFCSC1Addr(void)
{
    return CSP_IPU_IMA_TPM_VF_CSC1;
}

//
// Function:  IPUGetTPMVFCSC2Addr
//
// This function returns the internal memory address for the
// IPU IMA IC Task Parameter(Viewfinder CSC2) memory.
//
// Parameters:
//      None.
//
// Returns:
//      Internal memory address for IPU IMA IC TPM VF CSC2.
//
//-----------------------------------------------------------------------------
UINT32 IPUGetTPMVFCSC2Addr(void)
{
    return CSP_IPU_IMA_TPM_VF_CSC2;
}



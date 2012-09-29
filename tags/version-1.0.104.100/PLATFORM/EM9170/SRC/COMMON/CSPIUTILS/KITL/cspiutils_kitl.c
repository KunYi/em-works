//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  cspiutils_kitl.c
//
//  CSPI support for shared access between OAL, KITL, and drivers.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"

//-----------------------------------------------------------------------------
//
//  Function:  CSPIRequest
//
//  Acquires exclusive access to shared CSPI port.
//
//  Parameters:
//      pCSPI
//          [in] Pointer to CSPI registers.
//
//      controlReg
//          [in] CSPI CONTROLREG value to be programmed.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID CSPIRequest(PCSP_CSPI_REG pCSPI, UINT32 controlReg)
{
    OUTREG32(&pCSPI->CONREG, controlReg);
}


//-----------------------------------------------------------------------------
//
//  Function:  CSPIRelease
//
//  Releases exclusive access to shared CSPI port.
//
//  Parameters:
//      pCSPI
//          [in] Pointer to CSPI registers.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID CSPIRelease(PCSP_CSPI_REG pCSPI)
{
    // We give up exclusive access by clearing the EN bit
    INSREG32BF(&pCSPI->CONREG, CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE);
}


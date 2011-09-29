//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  cspiutils_boot.c
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
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pCSPI);

    OUTREG32(&pCSPI->CONREG, controlReg);
    OUTREG32(&pCSPI->PERIODREG, 2);
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
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pCSPI);

    // Stub for bootloader.  Bootloader is runs as single-threaded
    // with interrupts disabled, so we don't need special support
    // here
}


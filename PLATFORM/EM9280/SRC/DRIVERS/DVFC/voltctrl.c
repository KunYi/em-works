//-----------------------------------------------------------------------------
//
// Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  voltctrl.cpp
//
//  Provides PMIC voltage control support for the DVFC driver.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
#include "bsp.h"
#include "pmu.h"

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines



//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
//
//  Function:  DvfcUpdateSupplyVoltage
//
// This function sets the VDDD value and VDDD brownout level specified by the
// input parameters. If the new brownout level is equal to the current setting
// it'll only update the VDDD setting. If the new brownout level is less than
// the current setting, it will update the VDDD brownout first and then the VDDD.
// Otherwise, it will update the VDDD first and then the brownout. This
// arrangement is intended to prevent from false VDDD brownout. This function
// will not return until the output VDDD stable.

//
//  Parameters:
//      u16Vddd_mV
//          [in] Vddd voltage in millivolts.
//
//      u16VdddBrownout_mV
//          [in]  Vddd brownout in millivolts
//
//      domain
//          [in] Specifies DVFC domain.
//
//  Returns:
//
//-----------------------------------------------------------------------------
BOOL  DvfcUpdateSupplyVoltage(UINT32  NewTarget, UINT32  NewBrownout, DDK_DVFC_DOMAIN domain)
{
    BOOL    rtn = TRUE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(domain);

    PmuSetVddd(NewTarget,NewBrownout );

    return rtn;
}


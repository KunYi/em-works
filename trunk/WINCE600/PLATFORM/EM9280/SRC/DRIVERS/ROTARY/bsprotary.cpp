//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//
//  File:  BspRotary.cpp
//
//  This module contains the main routines for the Rotary driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 6001 6385)
#include <windows.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 4100 4127 4189 6001 6385)
#include <creg.hxx>
#pragma warning(pop)

#include "bsp.h"


extern "C" VOID BspRotarySetIOmux();
//-----------------------------------------------------------------------------
//
//  Function: BspRotarySetIOmux
//
//  Configure the debounce control of the rotary decoder.
//
//  Parameters:
//        None
//  Returns:
//        None
//-----------------------------------------------------------------------------
VOID BspRotarySetIOmux()
{
    //mux selection for rotarya rotaryb
    DDKIomuxSetPinMux(DDK_IOMUX_TIMROT_ROTARYA_0, DDK_IOMUX_MODE_02); 
    DDKIomuxSetPinMux(DDK_IOMUX_TIMROT_ROTARYB_0,  DDK_IOMUX_MODE_02);
}



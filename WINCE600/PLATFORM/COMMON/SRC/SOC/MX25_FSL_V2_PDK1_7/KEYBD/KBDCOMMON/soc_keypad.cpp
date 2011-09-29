//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  Keypad.cpp
//
//  Implementation of SOC-specific keypad support.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"

UINT32 CSPKppGetIrq(VOID)
{
    return IRQ_KPP;
}

UINT32 CSPKppGetBaseRegAddr(VOID)
{
    return CSP_BASE_REG_PA_KPP;
}

//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  nand_gpmi.h
//
//
//
//-----------------------------------------------------------------------------
#ifndef NAND_GPMI_H
#define NAND_GPMI_H

#include "nand_hal.h"
#include <regsgpmi.h>
#include <regsclkctrl.h>

BOOL GPMI_Init();
VOID GPMI_SetTiming(NANDTiming * pNewNANDTiming, UINT32 u32GpmiPeriod_ns);

#endif // NAND_GPMI_H

//------------------------------------------------------------------------------
//
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 
// File:  
//     i_pdd.c
// 
// Description: 
//     This file includes pdd.c
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <celog.h>
#include <oal.h>
#pragma warning(pop)

#pragma warning(disable: 4100)
#define CACHE_LINE_SIZE 32
#include "mx28_usb.h"

extern void usb_reg_write(UINT32,UINT32,UINT32);
#undef OUTREG32
#define OUTREG32(x,y) usb_reg_write((UINT32)(x),(UINT32)(y),(UINT32)1)

#include "pdd.c"

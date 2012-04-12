//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  lcdc_mode.h
//
//  Provides definitions for LCDC mode.
//
//------------------------------------------------------------------------------
#ifndef __LCDC_MODE_H
#define __LCDC_MODE_H

#include "common_lcdc.h"

// LCDC Content Struct
typedef struct {
  DWORD    dwPanelType;  
  GPEMode  GPEModeInfo;
  UINT32   Hwidth;
  UINT32   Hwait1;
  UINT32   Hwait2;
  UINT32   Vwidth;
  UINT32   Vwait1;
  UINT32   Vwait2;
  union
  {
    PCR_CFG  PCRCfg;
    UINT32   uPCRCfg;
  } PCR_CFG;
} LCDC_MODE_DESCRIPTOR, *PLCDC_MODE_DESCRIPTOR;

#endif //__LCDC_MODE_H

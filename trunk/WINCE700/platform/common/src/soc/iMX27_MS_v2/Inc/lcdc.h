//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: inc/lcdc.h
//
//  Header file for lcdc driver.
//
//------------------------------------------------------------------------------
#ifndef _INC_LCDC_H_
#define _INC_LCDC_H_

//------------------------------------------------------------------------------
// MACRO DEFINITIONS 
//------------------------------------------------------------------------------
#define LCDC_MODE_NUM                1
#define ClearGraphicWindowRegs(Reg) { \
  Reg->GWSAR  =  0;       \
  Reg->GWSR   =  0;       \
  Reg->GWVPWR =  0;       \
  Reg->GWPR   =  0;       \
  Reg->GWCR   =  0;       \
}

//--------------------------------------------------------
// DrvEscape return codes
//--------------------------------------------------------
#define ESC_FAILED          (-1)
#define ESC_NOT_SUPPORTED   (0)
#define ESC_SUCCESS         (1)


//------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
extern void   BSPSetDevCaps(GDIINFO * pDevCaps);
extern ULONG* BSPGetLCDCBitMasks(void);
extern ULONG  BSPGetModeNum(void);
extern SCODE  BSPGetModeInfo(GPEMode * pMode, int modeNo);
extern SCODE  BSPSetMode(int modeId, HPALETTE *pPalette);
extern BOOL   BSPInitLCDC(PLCDCCTX pContext);
extern void   BSPDeinitLCDC(PLCDCCTX pContext);
extern void   BSPTurnOnLCD(void);
extern void   BSPTurnOffLCD(void);
extern UINT16 BSPGetPixelSizeInByte(void);

extern BOOL   BSPInitializeTVOut(BOOL tvout_ntsc);
extern BOOL   BSPDeinitializeTVOut();
extern void   BSPPowerOnTVOut();
extern void   BSPPowerOffTVOut();
extern struct DisplayPanel*  BSPConfigPanel(BOOL bTVModeActive, BOOL bTVNTSCOut, DWORD dwPanelType);
/*********************************************************************
 END OF FILE
*********************************************************************/
#endif   /* _INC_LCDC_H_ */

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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

#ifndef __LCD_H__
#define __LCD_H__

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
//  Defines/Enums

//
//  TV Out display controller settings
//

#define NTSC_WIDTH      720
#define NTSC_HEIGHT     480

#define PAL_WIDTH       720
#define PAL_HEIGHT      576

#define TVOUT_COMPOSITE (0x0A)
#define TVOUT_SVIDEO    (0x0D)

//------------------------------------------------------------------------------
//
//  LCD PDD Functions
//
//  LCD Physical Layer Interace
//


BOOL
LcdPdd_LCD_Initialize(
    OMAP_DSS_REGS         *pDSSRegs,
    OMAP_DISPC_REGS       *pDispRegs,
    OMAP_RFBI_REGS        *pRfbiRegs,
    OMAP_VENC_REGS        *pVencRegs,
    OMAP_PRCM_DSS_CM_REGS *pPrcmDssCM
    );
        
BOOL
LcdPdd_TV_Initialize(
    OMAP_DSS_REGS       *pDSSRegs,
    OMAP_DISPC_REGS     *pDispRegs,
    OMAP_RFBI_REGS      *pRfbiRegs,
    OMAP_VENC_REGS      *pVencRegs
    );


BOOL
LcdPdd_GetMemory(
    DWORD   *pVideoMemLen,
    DWORD   *pVideoMemAddr
    );

BOOL
LcdPdd_LCD_GetMode(
    DWORD   *pPixelFormat,
    DWORD   *pWidth,
    DWORD   *pHeight,
    DWORD   *pPixelClock
    );

BOOL
LcdPdd_TV_GetMode(
    DWORD   *pWidth,
    DWORD   *pHeight,
    DWORD   *pMode
    );


BOOL
LcdPdd_SetPowerLevel(
    DWORD   dwPowerLevel
    );

BOOL
LcdPdd_DVI_Select(
    BOOL bEnable
    );

BOOL
LcdPdd_DVI_Enabled(void);

DWORD
LcdPdd_Get_PixClkDiv(void);

#ifdef __cplusplus
}
#endif

#endif __LCD_H__


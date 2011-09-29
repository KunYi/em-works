//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_display.h
//
//  Header file for Display driver.
//
//------------------------------------------------------------------------------


#ifndef __SOC_DISPLAY_H
#define __SOC_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define SCREEN_PIX_WIDTH_VGA        640
#define SCREEN_PIX_HEIGHT_VGA       480

#define SCREEN_PIX_WIDTH_QVGA        320
#define SCREEN_PIX_HEIGHT_QVGA       240

#define SCREEN_PIX_WIDTH_WQVGA        400
#define SCREEN_PIX_HEIGHT_WQVGA       272

#define SCREEN_PIX_WIDTH_D1         720
#define SCREEN_PIX_HEIGHT_D1_NTSC   480
#define SCREEN_PIX_HEIGHT_D1_PAL    576
#define SCREEN_PIX_WIDTH_720P       1280
#define SCREEN_PIX_HEIGHT_720P      720

#define DISP_MIN_WIDTH                             8
#define DISP_MIN_HEIGHT                            8
#define DISP_BPP                                   16
#define DISP_BYTES_PP                              (DISP_BPP / 8)

#define CUSTOMESCAPECODEBASE        100100
#define VF_GET_DISPLAY_INFO         (CUSTOMESCAPECODEBASE + 0)

enum
{
     DISPLAY_MODE_DEVICE=0
    ,DISPLAY_MODE_NTSC
    ,DISPLAY_MODE_NTSC_J
    ,DISPLAY_MODE_PAL
    // NOTE: the following modes are supported by the
    // hardware but not implemented in software.
    // NTSC-M Mode
    // PAL-B Mode
    // PAL-M Mode
    // PAL-N Mode
    // PAL-CN Mode
    // NTSC with 700:300 scaling on "G"
    // PAL-60 Mode
    // NTSC Progressive
    ,DISPLAY_MODE_NONE=0x7f
};
// TV out driver escape codes and defines
#ifndef DISPLAY_GET_OUTPUT_MODE
#define DISPLAY_GET_OUTPUT_MODE (CUSTOMESCAPECODEBASE + 7)
#endif
#ifndef DISPLAY_SET_OUTPUT_MODE
#define DISPLAY_SET_OUTPUT_MODE (CUSTOMESCAPECODEBASE + 8)
#endif

// display driver escape code for set video memory attribution
#ifndef VM_SETATTEX
#define VM_SETATTEX                 (CUSTOMESCAPECODEBASE + 11)
#endif

#ifndef DISPLAY_SET_BRIGHTNESS
#define DISPLAY_SET_BRIGHTNESS (CUSTOMESCAPECODEBASE + 12) // Brightness is a DWORD %
#endif
#ifndef DISPLAY_GET_BRIGHTNESS
#define DISPLAY_GET_BRIGHTNESS (CUSTOMESCAPECODEBASE + 13) // Brightness is a DWORD %
#endif

#define DISPLAY_DLS_GET_CSC     (CUSTOMESCAPECODEBASE + 14)
#define DISPLAY_DLS_SET_CSC     (CUSTOMESCAPECODEBASE + 15)

#ifndef DISPLAY_GET_GAMMA_VALUE
#define DISPLAY_GET_GAMMA_VALUE (CUSTOMESCAPECODEBASE + 16) // Gamma is a float %
#endif
#ifndef DISPLAY_SET_GAMMA_VALUE
#define DISPLAY_SET_GAMMA_VALUE (CUSTOMESCAPECODEBASE + 17) // Gamma is a float %
#endif


#ifndef CGMSA_GET_OUTPUT_MODE
#define CGMSA_GET_OUTPUT_MODE     (CUSTOMESCAPECODEBASE + 28)
#endif
#ifndef CGMSA_SET_OUTPUT_MODE
#define CGMSA_SET_OUTPUT_MODE     (CUSTOMESCAPECODEBASE + 29)
#endif


#ifndef CGMSA_MODE_NTSC
#define CGMSA_MODE_NTSC     0
#endif
#ifndef CGMSA_MODE_PAL
#define CGMSA_MODE_PAL      1
#endif

#if __cplusplus
}
#endif

#endif //__SOC_DISPLAY_H

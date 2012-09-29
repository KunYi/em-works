//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// FILE:    bsplcdc.h
//
//  This file contains structure and definitions required by BSP part of the LCDC driver
//
//------------------------------------------------------------------------------


#ifndef _BSP_DRIVERS_DISPLAY_LCDC_H
#define _BSP_DRIVERS_DISPLAY_LCDC_H

//------------------------------------------------------------------------------
// MACRO and TYPE DEFINITIONS 
//------------------------------------------------------------------------------

typedef enum PanelType_c
{
    DISPLAY_CHUNGHWA_CLAA057VA01CT = 0, // VGA display

    // Add your display panel here
    // ...
    DISPLAY_SHARP_LQ057 = 1,						// 5.7" TFT LCD 320*240
    DISPLAY_CHIMEI_LR430LC9001 = 2,		// 4.3" TFT LCD 480*272
    DISPLAY_INNOLUX_AT056TN52 = 3,			// 5.6" TFT LCD 640*480
    DISPLAY_INNOLUX_AT070TN83V1 = 4,		// 7.0" TFT LCD 800*480
    DISPLAY_AUO_G084SN03 = 5,					// 8.4" TFT LCD 800*600

    numPanels           // Define the numPanel so value automaticaly the number of panel
}PanelType;

#define PANEL_TYPE_DEFAULT          DISPLAY_CHUNGHWA_CLAA057VA01CT

//------------------------------------------------------------------------------
// DDLCDC registry entries
//------------------------------------------------------------------------------
#define REG_LCDC_PATH                   TEXT("Drivers\\Display\\LCDC")
#define REG_PANEL_TYPE                  TEXT("PanelType")


#endif  /* _BSP_DRIVERS_DISPLAY_LCDC_H */

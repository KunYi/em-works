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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//------------------------------------------------------------------------------
//
//  Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------


#ifndef _SDBUS_EXT_
#define _SDBUS_EXT_

#include "sdhcd.h"

typedef enum _SD_SLOT_OPTION_CODE_EXT{
    SDHCDEnableDDRMode = 0xF0,
    SDHCDDisableDDRMode
} SD_SLOT_OPTION_CODE_EXT, *PSD_SLOT_OPTION_CODE_EXT;

#define MMC_EXT_CSD_BOOTCONF        179
#define MMC_EXT_CSD_BUSWIDTH        183
#define MMC_EXT_CSD_HSTIMING        185
#define MMC_EXT_CSD_CARDTYPE        196


// Host MCC 8bit capable.
#define SD_SLOT_MMC_8BIT_CAPABLE                 0x20010000

#define SD_CMD_MMC_SEND_EXT_CSD     8       // CMD8 

#define SD_BUS_INTERFACE_VERSION_MAJOR  2
#define SD_BUS_INTERFACE_VERSION_MINOR  0

#define SD_CSD_SPECVER_BIT_SLICE            122
#define SD_CSD_SPECVER_SLICE_SIZE           4

#define SDIO_CCCR_SPEC_REV_1_2      0x02

#define SD_HIGH_SPEED_RATE              50000000    // 50 Mhz
#define MMC_HIGH_SPEED_RATE           52000000    // 52 Mhz

// MMC bus width test commands
#define SD_CMD_MMC_BUSTEST_W    19
#define SD_CMD_MMC_BUSTEST_R    14

#endif
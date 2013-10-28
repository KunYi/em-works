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
//
// Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: cspibus.h
//
// Header file for cspi bus driver.
//
//------------------------------------------------------------------------------
#ifndef __CSPIBUS_H__
#define __CSPIBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define CSPI_IOCTL_EXCHANGE     0x10

//------------------------------------------------------------------------------
// Types
//
// CSPI bus configuration
typedef struct
{
    UINT8   chipselect;
    UINT32  freq;
    UINT8   bitcount;
    BOOL    sspol;
    BOOL    ssctl;
    BOOL    pol;
    BOOL    pha;
    UINT8   drctl;
} CSPI_BUSCONFIG_T, *PCSPI_BUSCONFIG_T;

// CSPI exchange packet
typedef struct
{
    PCSPI_BUSCONFIG_T pBusCnfg;
    LPVOID pTxBuf;
    LPVOID pRxBuf;
    UINT32 xchCnt;
    HANDLE xchEvent;
} CSPI_XCH_PKT_T, *PCSPI_XCH_PKT_T;

//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif // __CSPIBUS_H__


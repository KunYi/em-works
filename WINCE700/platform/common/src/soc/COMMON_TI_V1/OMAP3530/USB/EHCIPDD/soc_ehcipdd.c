// All rights reserved ADENEO EMBEDDED 2010
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
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:
    system.c

Abstract:
    SOC dependant part of the USB Universal Host Controller Driver (EHCD).

Notes:
--*/

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
//
//  File:  soc_ehcipdd.c
//
#pragma warning (push)
#pragma warning (disable: 4115 4124 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ddkreg.h>
#include <devload.h>
#include <giisr.h>
#include <hcdddsi.h>
#include <cebuscfg.h>
#include <ceddkex.h>
#include <initguid.h>
#pragma warning (pop)

#include "omap.h"
#include "omap3530_prcm.h"
#include "soc_cfg.h"
#include "ehcipdd.h"

void SOCEhciConfigure(SEHCDPdd * pPddObject)
{
	OMAP_PRCM_USBHOST_PRM_REGS* pOMAP_PRCM_USBHOST_PRM_REGS = NULL;
	OMAP_PRCM_CORE_PRM_REGS* pOMAP_PRCM_CORE_PRM_REGS		= NULL;
    PHYSICAL_ADDRESS pa;

	// Map registers
	pa.QuadPart = pPddObject->USBHInfo.PRCM_USBHOST_REGS_PA;
    pOMAP_PRCM_USBHOST_PRM_REGS = (OMAP_PRCM_USBHOST_PRM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_USBHOST_PRM_REGS), FALSE);

    pa.QuadPart = pPddObject->USBHInfo.PRCM_CORE_REGS_PA;
    pOMAP_PRCM_CORE_PRM_REGS = (OMAP_PRCM_CORE_PRM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_CORE_PRM_REGS), FALSE);

    // enable USB host save and restore mechanism
    pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTCTRL_USBHOST |= (0x01 << 4); //SAVEANDRESTORE

    // enable USBTLL save and restore mechanism
    pOMAP_PRCM_CORE_PRM_REGS->PM_PWSTCTRL_CORE |= (0x01 << 4); //SAVEANDRESTORE

	MmUnmapIoSpace(pOMAP_PRCM_USBHOST_PRM_REGS, sizeof(OMAP_PRCM_USBHOST_PRM_REGS));
	MmUnmapIoSpace(pOMAP_PRCM_CORE_PRM_REGS, sizeof(OMAP_PRCM_CORE_PRM_REGS));
}
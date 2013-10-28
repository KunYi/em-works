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
// Copyright (C) 2005, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

/*
 *  File:       XVC.h
 *  Purpose:    XVC module definitions
*/

#ifndef _INC_XVC_H
#define _INC_XVC_H

/*********************************************************************
 LOADER DEFINITIONS
*********************************************************************/

typedef struct {
    CSP_USB_REGS *pUSBRegs;
    DWORD   dwSysIntr;
    DWORD   dwIrq;
    HANDLE  hIntrEvent;
    DWORD   memBase;
    DWORD   memLen;
    DWORD   devState; /* 0 - nothing connect, 1 - something connect */
    BOOL    bIsMX31TO2; // TRUE- If hardware is TO2 else FALSE
    DWORD   IsOTGSupport;
    BOOL    bInXVC;
    TCHAR   szOTGGroup[15];
    BOOL    bResume;
    CEDEVICE_POWER_STATE CurPMPowerState;
#ifdef CABLE_SOLUTION
    DWORD dwUSBIntrValue;  // USBINTR value to be saved before power down
#endif

    BOOL    bUSBCoreClk;
    BOOL    bPanicMode;
    CRITICAL_SECTION csPhyLowMode;
    BSP_USB_CALLBACK_FNS fnUsbXvr;
} USBXVC, *PUSBXVC;

extern BOOL InitializeOTGTransceiver(PCSP_USB_REGS *pUSBRegs, BOOL IsHost);
extern DWORD GetSysIntr(void);
#endif /* _INC_XVC_H */

/*********************************************************************
 END OF FILE
*********************************************************************/

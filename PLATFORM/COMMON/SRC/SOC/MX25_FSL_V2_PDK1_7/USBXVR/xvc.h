//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
// File:
//     XVC.h
// Purpose:
//     XVC module definitions
//------------------------------------------------------------------------------

#ifndef _INC_XVC_H
#define _INC_XVC_H

typedef struct {
    CSP_USB_REGS *pUSBRegs;
    DWORD   dwSysIntr;
    DWORD   dwIrq;
    HANDLE  hIntrEvent;
    DWORD   memBase;
    DWORD   memLen;
    DWORD   devState;     // 0 - nothing connect, 1 - something connect
    DWORD   IsOTGSupport;
    BOOL    bInXVC;
    TCHAR   szOTGGroup[15];
    BOOL    bResume;
    CEDEVICE_POWER_STATE CurPMPowerState;
    DWORD dwUSBIntrValue;  // USBINTR value to be saved before power down
    BOOL    bUSBCoreClk;
    CRITICAL_SECTION csPhyLowMode;
    BSP_USB_CALLBACK_FNS fnUsbXvr;
    BOOL    bExitThread;
    HANDLE  hInterruptServiceThread;
} USBXVC, *PUSBXVC;

extern BOOL InitializeOTGTransceiver(PCSP_USB_REGS *pUSBRegs, BOOL IsHost);
extern DWORD GetSysIntr(DWORD irq);
#endif // _INC_XVC_H

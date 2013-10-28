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
#ifndef _X86BOOTINFO_H_
#define _X86BOOTINFO_H_

#include <kitlprot.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


typedef struct _x86BootInfo {
    DWORD   dwKitlBaseAddr;     // Base I/O address for debug Ether adapter
    DWORD   dwKitlIP;           // IP address
    DWORD   dwKitlDebugZone;    // Allow KITL debug zones to be turned on from loadcepc
    WORD    wMac[3];            // MAC address
    WORD    KitlTransport;      // Transport for Kitl communication    
    BOOL    fKitlVMINI;         // VMINI enabled/disabled
    UCHAR   ucKitlAdapterType;  // Type of KITL adapter
    UCHAR   ucKitlIrq;          // IRQ line to use for debug Ether adapter
    #define OAL_KITL_IRQ_INVALID    0xFF
    UCHAR   ucComPort;
    UCHAR   ucBaudDivisor;
    UCHAR   szDeviceName[KITL_MAX_DEV_NAMELEN];  // KITL device name.

    DWORD   dwRebootAddr;       // Reboot entry point set by eboot and used during warm reset
    WORD    cxDisplayScreen;    // displayable X size
    WORD    cyDisplayScreen;    // displayable Y size
    WORD    bppScreen;          // color depth
    BYTE    NANDBootFlags;      // Boot flags related to NAND support.
    BYTE    NANDBusNumber;      // NAND controller PCI bus number.
    DWORD   NANDSlotNumber;     // NAND controller PCI slot number.

    UCHAR   ucPCIConfigType;    // PCI config type
    UCHAR   fStaticIP;          // use static IP or not

    BOOL    fFormatUserStore;
    ULARGE_INTEGER RamTop;      // The top of usable RAM
} X86BootInfo, *PX86BootInfo;


extern void BSPInitDfltBootInfo (PX86BootInfo pBootInfo);

// A Global used in the OAL, with contains information passed from eboot for booting
extern PX86BootInfo g_pX86Info;


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _X86BOOTINFO_H_

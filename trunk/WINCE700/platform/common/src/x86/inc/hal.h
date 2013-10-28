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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------

VOID    PCIInitBusInfo();

VOID    ISAInitBusInfo();

ULONG
ISAGetBusDataByOffset(
                     IN ULONG BusNumber,
                     IN ULONG SlotNumber,
                     OUT PVOID Buffer,
                     IN ULONG Offset,
                     IN ULONG Length
                     );

ULONG
ISASetBusDataByOffset(
                     IN ULONG BusNumber,
                     IN ULONG SlotNumber,
                     IN PVOID Buffer,
                     IN ULONG Offset,
                     IN ULONG Length
                     );

ULONG
PCIGetBusDataByOffset(
                     IN ULONG BusNumber,
                     IN ULONG SlotNumber,
                     OUT PVOID Buffer,
                     IN ULONG Offset,
                     IN ULONG Length
                     );

ULONG
PCISetBusDataByOffset(
                     IN ULONG BusNumber,
                     IN ULONG SlotNumber,
                     IN PVOID Buffer,
                     IN ULONG Offset,
                     IN ULONG Length
                     );

ULONG
PCIReadBusData(
              IN ULONG BusNumber,
              IN ULONG DeviceNumber,
              IN ULONG FunctionNumber,
              OUT PVOID Buffer,
              IN ULONG Offset,
              IN ULONG Length
              );

ULONG
PCIWriteBusData(
              IN ULONG BusNumber,
              IN ULONG DeviceNumber,
              IN ULONG FunctionNumber,
              IN PVOID Buffer,
              IN ULONG Offset,
              IN ULONG Length
              );




// We support the IOCTL_SET_KERNEL_DEV_PORT ioctl, so lets define the various
// ports which we might want to let the user configure for debugging.
#define KERNEL_PORT_NONE        0
#define KERNEL_PORT_COM1        1
#define KERNEL_PORT_COM2        2
#define KERNEL_PORT_COM3        3
#define KERNEL_PORT_COM4        4
#define KERNEL_PORT_LPT1        5
#define KERNEL_PORT_LPT2        6
#define KERNEL_PORT_ETH1        7

#define KERNEL_PORT_MAX         KERNEL_PORT_ETH1

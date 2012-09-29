//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------//
// FILE:    serialutils.h
//
//  This file contains structure definitions required by serial bootloader/KITL.
//
//------------------------------------------------------------------------------

#ifndef __SERIALUTILS_H__
#define __SERIALUTILS_H__

// structure for serial information
typedef struct _SERIAL_INFO{
    DWORD   uartBaseAddr;   // Serial Physical Base Address
    DWORD   baudRate;       // Baud rate
    DWORD   dataBits;       // Data Bits 7 or 8 bits
    DWORD   parity;         // Even or Odd Parity
    DWORD   stopBits;       // 1 or 2 stopbits
    DWORD   flowControl;    // Hardware or no flowcontrol
    BOOL    bParityEnable;  // Parity Enable or Disable
} SERIAL_INFO, *PSERIAL_INFO;


extern BOOL OALConfigSerialUART(PSERIAL_INFO pSerInfo);
extern BOOL OALConfigSerialIOMUX(DWORD uartBaseAddr,
                                 PCSP_IOMUX_REGS pIOMUX);

#endif

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
//  Header: s3c6410_spi.h
//
//  Defines the Serial Peripheral Interface (SPI) controller CPU register layout and
//  definitions.
//
#ifndef __S3C6410_SPI_H
#define __S3C6410_SPI_H

#if __cplusplus
    extern "C"
    {
#endif

#include <winioctl.h>
#include <ceddk.h>

typedef struct  
{
    UINT32      CH_CFG;        // 00
    UINT32      CLK_CFG;       // 04
    UINT32      MODE_CFG;      // 08
    UINT32      SLAVE_SEL;     // 0C
    UINT32      SPI_INT_EN;    // 10
    UINT32      SPI_STATUS;    // 14
    UINT32      SPI_TX_DATA;   // 18
    UINT32      SPI_RX_DATA;   // 1C
    UINT32      PACKET_COUNT;  // 20
    UINT32      PENDING_CLEAR; // 24
    UINT32      SWAP_CFG;      // 28
    UINT32      FB_CLK_SEL;    // 2C
} S3C6410_SPI_REG, *PS3C6410_SPI_REG; 


// IOCTL Commands
#define    SPI_IOCTL_SET_CONFIG            CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    SPI_IOCTL_START                CTL_CODE(FILE_DEVICE_SERIAL_PORT, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define    SPI_MASTER_MODE    1
#define    SPI_SLAVE_MODE        0


typedef struct {
    PVOID                VirtualAddress;
    PVOID            PhysicalAddress;
} DMA_BUFFER, *PDMA_BUFFER;

typedef struct {
    DWORD                    dwMode;

    BOOL                    bUseFullDuflex;

    DWORD                    dwRxBurstDataLen;
    BOOL                    bUseRxDMA;
    BOOL                    bUseRxIntr;

    DWORD                    dwTxBurstDataLen;
    BOOL                    bUseTxDMA;
    BOOL                    bUseTxIntr;

    DWORD                    dwPrescaler;
    DWORD                    dwTimeOutVal;
} SET_CONFIG, *PSET_CONFIG;



//------------------------------------------------------------------------------

#if __cplusplus
       }
#endif

#endif    // __S3C6410_SPI_H

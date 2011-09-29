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
//------------------------------------------------------------------------------
//
//  File:  nand.h
//
#ifndef __NAND_H
#define __NAND_H

//------------------------------------------------------------------------------
//  Boot Partition buffer
//
//  Buffer size is calculated as blockSize + sectorSize + (sectors/block) * 8.
//  Value used must be sufficient for all supported NANDs.

#define OMAP2420_NAND_BPART_BUFFER_SIZE     0x4300

//------------------------------------------------------------------------------
//  NAND Flash Commands

#define OMAP2420_NAND_CMD_READ          0x00        //  Read
#define OMAP2420_NAND_CMD_READ1         0x01        //  Read1
#define OMAP2420_NAND_CMD_READ2         0x50        //  Read2
#define OMAP2420_NAND_CMD_READID        0x90        //  ReadID
#define OMAP2420_NAND_CMD_WRITE         0x80        //  Write phase 1
#define OMAP2420_NAND_CMD_WRITE2        0x10        //  Write phase 2
#define OMAP2420_NAND_CMD_ERASE         0x60        //  Erase phase 1
#define OMAP2420_NAND_CMD_ERASE2        0xd0        //  Erase phase 2
#define OMAP2420_NAND_CMD_STATUS        0x70        //  Status read
#define OMAP2420_NAND_CMD_RESET         0xff        //  Reset

//------------------------------------------------------------------------------
//  Status bit pattern

#define OMAP2420_NAND_STATUS_READY      0x40        //  Ready
#define OMAP2420_NAND_STATUS_ERROR      0x01        //  Error

//------------------------------------------------------------------------------
//
//  Functions:  ECC_CorrectData/ECC_Compute
//
//  Functions used to calculate/correct data. OMAP2420 uses 3 bytes ECC per
//  512 byte sector. It is possible for OEM replace ECC algorithm and spare
//  area layour with exception of XLDR boot partition which has to use 
//  algorithm used by internal OMAP2420 ROM code.
//
BOOL ECC_CorrectData(UCHAR *pData, UCHAR existingECC[3], UCHAR newECC[3]);
VOID ECC_Compute(UCHAR data[512], UCHAR ecc[3]);

//------------------------------------------------------------------------------
//
//  Type:  NAND_SPARE_AREA
//
//  Spare area layout on NAND used for OMAP2420. Note this is slighly different
//  from "standard" Samsung layout. See above.
//
#pragma pack( push, 1 )
typedef struct {
    WORD  badBlock1;            // Indicates if block is BAD
    BYTE  ecc[3];               // ECC values
    BYTE  badBlock;             // Indicates if block is BAD
    BYTE  unused;               // Unused byte
    BYTE  oemReserved;          // For use by OEM
    WORD  reserved2;            // Reserved - used by FAL
    WORD  badBlock2;            // Indicates if block is BAD
    DWORD reserved1;            // Reserved - used by FAL
} OMAP2420_NAND_SPARE_AREA;
#pragma pack( pop )

//------------------------------------------------------------------------------
//
//  Type:  NAND_REGS
//
//  Hardware NAND registers structure as used on OMAP2420
//
typedef volatile struct {
    UINT16 DATA;
    UINT16 CMD;
    UINT16 ADDRESS;
} OMAP2420_NAND_REGS;

//------------------------------------------------------------------------------
//
//  Type:  NAND_INFO
//
//  NAND flash memory geometry info structure
//
typedef struct {
    UINT8  manufacturerId;
    UINT8  deviceId;
    UINT32 blocks;
    UINT32 sectorsPerBlock;
    UINT32 sectorSize;
    BOOL   extendedAddress;
} OMAP2420_NAND_INFO;

//------------------------------------------------------------------------------

extern OMAP2420_NAND_INFO g_nandInfo[];

//------------------------------------------------------------------------------

#endif // __NAND_H

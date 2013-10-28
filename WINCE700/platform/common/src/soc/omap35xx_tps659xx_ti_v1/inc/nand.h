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
//  File:  nand.h
//
//  This file contains information about NAND flash supported by FMD driver.
//
#ifndef __NAND_H
#define __NAND_H


//------------------------------------------------------------------------------
//  Boot Partition buffer
//
//  Buffer size is calculated as blockSize + sectorSize + (sectors/block) * 8.
//  Value used must be sufficient for all supported NANDs.

#define NAND_BPART_BUFFER_SIZE      0x20A00

//------------------------------------------------------------------------------
//  NAND Flash Commands

#define NAND_CMD_READ1              0x00        //  Read
#define NAND_CMD_READ2              0x30        //  Read1
#define NAND_CMD_READ_CPB           0x00        // Read for Copy back
#define NAND_CMD_READ_CPB1          0x35        // Read for Copy back 1
#define NAND_CMD_READID             0x90        //  ReadID
#define NAND_CMD_WRITE1             0x80        //  Write phase 1
#define NAND_CMD_WRITE2             0x10        //  Write phase 2
#define NAND_CMD_ERASE_SETUP        0x60        //  Erase phase 1
#define NAND_CMD_ERASE_CONFIRM      0xd0        //  Erase phase 2
#define NAND_CMD_STATUS             0x70        //  Status read

#define NAND_CMD_UNLOCK_LOW         0x23        //  
#define NAND_CMD_UNLOCK_HIGH        0x24        //  

#define NAND_CMD_RESET              0xff        //  Reset


//------------------------------------------------------------------------------
//  Status bit pattern

#define NAND_STATUS_READY           0x40        //  Ready
#define NAND_STATUS_ERROR           0x01        //  Error

//------------------------------------------------------------------------------
//  Status bit pattern
#define NAND_DATA_READ              0x1         // flag for data read op
#define NAND_DATA_WRITE             0x2         // flag for data write op 

//------------------------------------------------------------------------------
//  Geometry info structure

typedef struct {
    UINT8  manufacturerId;
    UINT8  deviceId;
    UINT32 blocks;
    UINT32 sectorsPerBlock;
    UINT32 sectorSize;
    BOOL   wordData;
} NAND_INFO;

//-----------------------------------------------------------------------------


#endif // __NAND_H

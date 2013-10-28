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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: mx31_ata.h
//
//  Provides definitions for ATA module based on Freescale MX31 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX31_ATA_H
#define __MX31_ATA_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
#if 0
typedef struct{
    UCHAR    ATATimeOff;        // 00: ATA Tx Control Register
    UCHAR    ATATimeOn;      // 04: ATA Tx Count Register
    UCHAR    ATATime1;        // 08: ATA Rx Control Register
    UCHAR    ATATime2W;     // 0C: ATA Tx Status Register
    UCHAR    ATATime2R;     // 10: ATA Rx Status Register
    UCHAR    ATATimeAX;       // 14: ATA Tx FIFO Register
    UCHAR    ATATimePIORdx;       // 18: ATA Rx FIFO Register
    UCHAR    ATATime4;          // 1C: ATA Control Register
    UCHAR    ATATime9;          // 1C: ATA Control Register
    UCHAR    OtherParas1[0x24-0x09];
    UCHAR    ATAControl;
    UCHAR    _FillBytes1[3];
    UCHAR    InterruptPending;
    UCHAR    _FillBytes2[3];
    UCHAR    InterruptEnable;
    UCHAR    _FillBytes20[3];
    UCHAR    InterruptClear;
    UCHAR    _FillBytes21[3];
    UCHAR    OtherParas2[0xA0-0x34];
    USHORT    DriveData;
    UCHAR    _FillByte3[2];
    UCHAR    DriveFeatures;
    UCHAR    _FillByte4[3];
    UCHAR    DriveSectorCount;
    UCHAR    _FillByte5[3];
    UCHAR    DriveSectorNum;
    UCHAR    _FillByte6[3];
    UCHAR    DriveCylLow;
    UCHAR    _FillByte7[3];
    UCHAR    DriveCylHigh;
    UCHAR    _FillByte8[3];
    UCHAR    DriveDevHead;
    UCHAR    _FillByte9[3];
    UCHAR    DriveCommandStatus;
    UCHAR    _FillByte10[3];
    UINT32   Reserved[6];
    UCHAR    DriveAltStatus;
    UCHAR    _FillByte11[3];
} CSP_ATA_REG, *PCSP_ATA_REG;

#endif

typedef struct{
    UINT32   ATA_TIME_CONFIG0;
    UINT32   ATA_TIME_CONFIG1;
    UINT32   ATA_TIME_CONFIG2;
    UINT32   ATA_TIME_CONFIG3;
    UINT32   ATA_TIME_CONFIG4;
    UINT32   ATA_TIME_CONFIG5;
    UINT32   FIFO_DATA_32[2];
    UINT32   FIFO_FILL;
    UCHAR    ATAControl;
    UCHAR    _FillBytes1[3];
    UCHAR    InterruptPending;
    UCHAR    _FillBytes2[3];
    UCHAR    InterruptEnable;
    UCHAR    _FillBytes3[3];
    UCHAR    InterruptClear;
    UCHAR    _FillBytes4[3];
    UCHAR    FIFOAlarm;
    UCHAR    _FillBytes5[3];
    UCHAR    _FillBytes6[0x68];
    USHORT    DriveData;
    UCHAR    _FillBytes7[2];
    UCHAR    DriveFeatures;
    UCHAR    _FillBytes8[3];
    UCHAR    DriveSectorCount;
    UCHAR    _FillBytes9[3];
    UCHAR    DriveSectorNum;
    UCHAR    _FillBytes10[3];
    UCHAR    DriveCylLow;
    UCHAR    _FillBytes11[3];
    UCHAR    DriveCylHigh;
    UCHAR    _FillBytes12[3];
    UCHAR    DriveDevHead;
    UCHAR    _FillBytes13[3];
    UCHAR    DriveCommandStatus;
    UCHAR    _FillBytes14[3];
    UINT32   Reserved[6];
    UCHAR    DriveAltStatus;
    UCHAR    _FillBytes15[3];
} CSP_ATA_REG, *PCSP_ATA_REG;

#define ATA_DRIVE_REG_OFFSET 0xA0
#define ATA_DRIVE_ALT_OFFSET 0xD8

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------

#define ATA_TIME_CONFIG0_OFFSET  0x0000 
#define ATA_TIME_CONFIG1_OFFSET  0x0004 
#define ATA_TIME_CONFIG2_OFFSET  0x0008 
#define ATA_TIME_CONFIG3_OFFSET  0x000C 
#define ATA_TIME_CONFIG4_OFFSET  0x0010 
#define ATA_TIME_CONFIG5_OFFSET  0x0014 
#define ATA_FIFO_DATA_32_OFFSET  0x0018 
#define ATA_FIFO_DATA_16_OFFSET  0x001C 
#define ATA_FIFO_FILL_OFFSET     0x0020 
#define ATA_CONTROL_OFFSET       0x0024 
#define ATA_INT_PENDING_OFFSET   0x0028 
#define ATA_INT_ENABLE_OFFSET    0x002C 
#define ATA_INT_CLEAR_OFFSET     0x0030 
#define ATA_FIFO_ALARM_OFFSET    0x0034 
#define ATA_DCTR_OFFSET          0x00D8 // drive control register (w, alt. stat reg (r
#define ATA_DDTR_OFFSET          0x00A0 // drive data register (rw
#define ATA_DFTR_OFFSET          0x00A4 // drive features regi (w, error reg (r
#define ATA_DSCR_OFFSET          0x00A8 //  drive sector count reg
#define ATA_DSNR_OFFSET          0x00AC //  drive sector number reg
#define ATA_DCLR_OFFSET          0x00B0 //  drive cylinder low reg
#define ATA_DCHR_OFFSET          0x00B4 //  drive cylinder high reg 
#define ATA_DDHR_OFFSET          0x00B8 //  drive device head reg
#define ATA_DCDR_OFFSET          0x00BC //  drive command reg (w, status reg (r
#define ATA_DALT_OFFSET          0x00D8 //  drive control reg (w, alternate status reg (r

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define ATA_CONTROL_IORDY_EN_LSH             0
#define ATA_CONTROL_DMA_WRITE_LSH            1
#define ATA_CONTROL_DMA_ULTRA_SELECTED_LSH   2
#define ATA_CONTROL_DMA_PENDING_LSH          3
#define ATA_CONTROL_FIFO_RCV_EN_LSH          4
#define ATA_CONTROL_FIFO_TX_EN_LSH           5
#define ATA_CONTROL_ATARESET_LSH             6
#define ATA_CONTROL_FIFORESET_LSH            7 


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define ATA_CONTROL_IORDY_EN_WID             0
#define ATA_CONTROL_DMA_WRITE_WID            1
#define ATA_CONTROL_DMA_ULTRA_SELECTED_WID   2
#define ATA_CONTROL_DMA_PENDING_WID          3
#define ATA_CONTROL_FIFO_RCV_EN_WID          4
#define ATA_CONTROL_FIFO_TX_EN_WID           5
#define ATA_CONTROL_ATARESET_WID             6
#define ATA_CONTROL_FIFORESET_WID            7 

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define ATA_CONTROL_IORDY_EN_DISABLE            0
#define ATA_CONTROL_IORDY_EN_ENABLE             1

#define ATA_CONTROL_DMA_WRITE_DISABLE           0
#define ATA_CONTROL_DMA_WRITE_ENABLE            1

#define ATA_CONTROL_DMA_ULTRA_SELECTED_DISABLE  0
#define ATA_CONTROL_DMA_ULTRA_SELECTED_ENABLE   1

#define ATA_CONTROL_DMA_PENDING_DISABLE         0
#define ATA_CONTROL_DMA_PENDING_ENABLE          1

#define ATA_CONTROL_FIFO_RCV_EN_DIABLE          0
#define ATA_CONTROL_FIFO_RCV_EN_ENABLE          1

#define ATA_CONTROL_FIFO_TX_EN_DISABLE          0
#define ATA_CONTROL_FIFO_TX_EN_ENABLE           1

#define ATA_CONTROL_ATARESET_RESET              0
#define ATA_CONTROL_ATARESET_NORMAL             1

#define ATA_CONTROL_FIFORESET_RESET             0 
#define ATA_CONTROL_FIFORESET_NORMAL            1 


#ifdef __cplusplus
}
#endif

#endif // __MX31_ATA_H

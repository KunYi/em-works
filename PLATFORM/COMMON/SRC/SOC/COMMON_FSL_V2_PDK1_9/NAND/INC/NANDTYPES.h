//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  NANDtypes.h
//
//  Contains definitions for FMD impletation of NAND flash memory device.
//
//------------------------------------------------------------------------------
#ifndef __NAND_TYPES_H__
#define __NAND_TYPES_H__

/* CS&ZHL NOV-6-2011: for memory
typedef struct _FlashInfo
{
    FLASH_TYPE  flashType;
    DWORD       dwNumBlocks;
    DWORD       dwBytesPerBlock;
    WORD        wSectorsPerBlock;
    WORD        wDataBytesPerSector;

}FlashInfo, *PFlashInfo;

typedef struct _NANDTiming
{
    BYTE DataSetup;				// -> tRP or tWP
    BYTE DataHold;				// -> tRH or tWH
    BYTE AddressSetup;			// -> tCEA
    BYTE DataSample;			// -> tREA
}NANDTiming, *PNANDTiming;

*/

NandChipInfo ChipInfo[] = 
{
	//---------------------------------------------------------------------------- 
    // CS&ZHL MAR-5-2012: add K9F1G08U0A with follwing timing parameters:
	// tREA => DataSample = 20ns, tRP => DataSetup = 30ns > (tREA + 4), no delay required!
	//---------------------------------------------------------------------------- 
    {
        {NAND, 1024, 2048 * 64, 64, 2048},      //FlashInfo   fi; 
        {0xEC, 0xF1, 0x00, 0x95},				//BYTE        NANDCode[NANDID_LENGTH] -> {MakerID, DevID, 3rdID, 4thID}
        3,										//BYTE        NumBlockCycles
        5,										//BYTE        ChipAddrCycleNum
        8,										//BYTE        DataWidth
        1,										//BYTE        BBMarkNum
        {0},									//BYTE        BBMarkPage
        6,										//BYTE        StatusBusyBit
        0,										//BYTE        StatusErrorBit
        64,										//WORD		  SpareDataLength
        0x70,									//BYTE        CmdReadStatus
        0x00,									//BYTE        CmdRead1
        0x30,									//BYTE        CmdRead2
        0x90,									//BYTE        CmdReadId
        0xff,									//BYTE        CmdReset
        0x80,									//BYTE        CmdWrite1
        0x10,									//BYTE        CmdWrite2
        0x60,									//BYTE        CmdErase1
        0xD0,									//BYTE        CmdErase2
        {40, 30, 30, 20}						//NANDTiming  timings => {DataSetup, DataHold, AddressSetup, DataSample}
    },
    //CS&ZHL NOV-6-2011: add K9F2G08U0A
    {
        {NAND, 2048, 2048 * 64, 64, 2048},      //FlashInfo   fi; 
        {0xEC, 0xDA, 0x10, 0x95},				//BYTE        NANDCode[NANDID_LENGTH] -> {MakerID, DevID, 3rdID, 4thID}
        3,										//BYTE        NumBlockCycles
        5,										//BYTE        ChipAddrCycleNum
        8,										//BYTE        DataWidth
        1,										//BYTE        BBMarkNum
        {0},									//BYTE        BBMarkPage
        6,										//BYTE        StatusBusyBit
        0,										//BYTE        StatusErrorBit
        64,										//WORD		  SpareDataLength
        0x70,									//BYTE        CmdReadStatus
        0x00,									//BYTE        CmdRead1
        0x30,									//BYTE        CmdRead2
        0x90,									//BYTE        CmdReadId
        0xff,									//BYTE        CmdReset
        0x80,									//BYTE        CmdWrite1
        0x10,									//BYTE        CmdWrite2
        0x60,									//BYTE        CmdErase1
        0xD0,									//BYTE        CmdErase2
        {40, 30, 30, 20}						//NANDTiming  timings => {DataSetup, DataHold, AddressSetup, DataSample}
    },
    //MT29F16G08
    {
        {NAND, 4096, 4096 * 128, 128, 4096},        //FlashInfo   fi; 
        {0x2C, 0xD5, 0x94, 0x3e},                   //BYTE        NANDCode[NANDID_LENGTH]
        3,                                          //BYTE        NumBlockCycles
        5,                                          //BYTE        ChipAddrCycleNum
        8,                                          //BYTE        DataWidth
        1,                                          //BYTE        BBMarkNum
        {0},                                        //BYTE        BBMarkPage
        6,                                          //BYTE        StatusBusyBit
        0,                                          //BYTE        StatusErrorBit
        218,                                        //WORD        SpareDataLength
        0x70,                                       //BYTE        CmdReadStatus
        0x00,                                       //BYTE        CmdRead1
        0x30,                                       //BYTE        CmdRead2
        0x90,                                       //BYTE        CmdReadId
        0xff,                                       //BYTE        CmdReset
        0x80,                                       //BYTE        CmdWrite1
        0x10,                                       //BYTE        CmdWrite2
        0x60,                                       //BYTE        CmdErase1
        0xD0,                                       //BYTE        CmdErase2
        {25, 16, 25, 20}                            //NANDTiming  timings
    },
    //MT29F8G08
    {
        {NAND, 4096, 2048 * 128, 128, 2048},        //FlashInfo   fi;     
        {0x2C, 0xD3, 0x94, 0x2d},                   //BYTE        NANDCode[NANDID_LENGTH]
        3,                                          //BYTE        NumBlockCycles
        5,                                          //BYTE        ChipAddrCycleNum
        8,                                          //BYTE        DataWidth
        1,                                          //BYTE        BBMarkNum
        {0},                                        //BYTE        BBMarkPage
        6,                                          //BYTE        StatusBusyBit
        0,                                          //BYTE        StatusErrorBit
        64,                                        //WORD        SpareDataLength
        0x70,                                       //BYTE        CmdReadStatus
        0x00,                                       //BYTE        CmdRead1
        0x30,                                       //BYTE        CmdRead2
        0x90,                                       //BYTE        CmdReadId
        0xff,                                       //BYTE        CmdReset
        0x80,                                       //BYTE        CmdWrite1
        0x10,                                       //BYTE        CmdWrite2
        0x60,                                       //BYTE        CmdErase1
        0xD0,                                       //BYTE        CmdErase2
        {25, 16, 25, 20}                             //NANDTiming  timings
    },
    //MT29F16G08DAA
    {
        {NAND, 4096, 4096 * 64, 64, 4096},        //FlashInfo   fi;     
        {0x2C, 0xD3, 0x90, 0x2e},                   //BYTE        NANDCode[NANDID_LENGTH]
        3,                                          //BYTE        NumBlockCycles
        5,                                          //BYTE        ChipAddrCycleNum
        8,                                          //BYTE        DataWidth
        1,                                          //BYTE        BBMarkNum
        {0},                                        //BYTE        BBMarkPage
        6,                                          //BYTE        StatusBusyBit
        0,                                          //BYTE        StatusErrorBit
        218,                                        //WORD        SpareDataLength
        0x70,                                       //BYTE        CmdReadStatus
        0x00,                                       //BYTE        CmdRead1
        0x30,                                       //BYTE        CmdRead2
        0x90,                                       //BYTE        CmdReadId
        0xff,                                       //BYTE        CmdReset
        0x80,                                       //BYTE        CmdWrite1
        0x10,                                       //BYTE        CmdWrite2
        0x60,                                       //BYTE        CmdErase1
        0xD0,                                       //BYTE        CmdErase2
        {11, 8, 15, 20}                             //NANDTiming  timings
    },
    //K9LBG08U0D
    {
        {NAND, 8192, 4096 * 128, 128, 4096},        //FlashInfo   fi;     
        {0xEC, 0xD7, 0xD5, 0x29},                   //BYTE        NANDCode[NANDID_LENGTH]
        3,                                          //BYTE        NumBlockCycles
        5,                                          //BYTE        ChipAddrCycleNum
        8,                                          //BYTE        DataWidth
        1,                                          //BYTE        BBMarkNum
        {127},                                      //BYTE        BBMarkPage
        6,                                          //BYTE        StatusBusyBit
        0,                                          //BYTE        StatusErrorBit
        218,                                        //WORD        SpareDataLength
        0x70,                                       //BYTE        CmdReadStatus
        0x00,                                       //BYTE        CmdRead1
        0x30,                                       //BYTE        CmdRead2
        0x90,                                       //BYTE        CmdReadId
        0xff,                                       //BYTE        CmdReset
        0x80,                                       //BYTE        CmdWrite1
        0x10,                                       //BYTE        CmdWrite2
        0x60,                                       //BYTE        CmdErase1
        0xD0,                                       //BYTE        CmdErase2
        {20, 8, 15, 20}                             //NANDTiming  timings
    },
};

#endif    // __NAND_TYPES_H__


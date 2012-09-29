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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include <fmd.h>
#include "common_nandfmd.h"
#include "nandbsp.h"

#define BBT_BLOCK_NUM						4       // Block numbers : store BBT
#define NAND_NUM_OF_CLUSTER			1
#define NAND_NUM_OF_DEVICES			1
#define INTERLEAVE_MODE					FALSE

BYTE g_BBTFirstBuf[(((NAND_BLOCK_CNT * NAND_NUM_OF_DEVICES * NAND_NUM_OF_CS + 7) / 8 + NAND_PAGE_SIZE - 1) / NAND_PAGE_SIZE) * NAND_PAGE_SIZE];
BYTE g_BBTSecondBuf[(((NAND_BLOCK_CNT * NAND_NUM_OF_DEVICES * NAND_NUM_OF_CS + 7) / 8 + NAND_PAGE_SIZE - 1) / NAND_PAGE_SIZE) * NAND_PAGE_SIZE];
BYTE g_SectorBuf[NAND_PAGE_SIZE];

VOID BSPNAND_GetFlashInfo(PFlashInfoExt pFlashInfo)
{
    pFlashInfo->fi.dwNumBlocks = NAND_BLOCK_CNT;
    pFlashInfo->fi.wDataBytesPerSector = NAND_PAGE_SIZE;
    pFlashInfo->fi.wSectorsPerBlock = NAND_PAGE_CNT;
    pFlashInfo->fi.dwBytesPerBlock = pFlashInfo->fi.wDataBytesPerSector * pFlashInfo->fi.wSectorsPerBlock;
    
    pFlashInfo->NANDIDCode = NAND_ID_CODE;
    pFlashInfo->BBMarkNum = BBI_NUM;					// BBI_NUM = 1
    pFlashInfo->BBMarkPage = BBMarkPage;				// -> BYTE    BBMarkPage[1] = {127};
    pFlashInfo->DataWidth = NAND_BUS_WIDTH;
    pFlashInfo->NumBlockCycles = 3;							// for large sector only!
    pFlashInfo->ChipAddrCycleNum = 5;						// for large sector only!
    pFlashInfo->NumberOfChip = NAND_NUM_OF_DEVICES * NAND_NUM_OF_CS;
    pFlashInfo->ILSupport = INTERLEAVE_MODE;
    pFlashInfo->SpareDataLength = NAND_SPARE_SIZE;
    pFlashInfo->ClusterCount = NAND_NUM_OF_CLUSTER;
    pFlashInfo->BBIMainAddr = BBI_MAIN_ADDR;		// 2-byte alignment seems required
    pFlashInfo->StatusErrorBit = NAND_STATUS_ERROR_BIT;
    pFlashInfo->StatusBusyBit = NAND_STATUS_BUSY_BIT;
    
    pFlashInfo->CmdRead1 = CMD_READ;
    pFlashInfo->CmdRead2 = CMD_READ2;
    pFlashInfo->CmdReadId = CMD_READID;
    pFlashInfo->CmdReset = CMD_RESET;
    pFlashInfo->CmdWrite1 = CMD_WRITE;
    pFlashInfo->CmdWrite2 = CMD_WRITE2;
    pFlashInfo->CmdErase1 = CMD_ERASE;
    pFlashInfo->CmdErase2 = CMD_ERASE2;
    pFlashInfo->CmdReadStatus = CMD_STATUS;
}


VOID BSPNAND_GetBufferPointer(PBBTBuffer pBBT)
{   
    pBBT->pSectorBuf = g_SectorBuf;
    pBBT->pBBTFirstBuf = g_BBTFirstBuf;
    pBBT->pBBTSecondBuf = g_BBTSecondBuf;
    pBBT->dwBBTFirstSize = sizeof(g_BBTFirstBuf);
    pBBT->dwBBTSecondSize = sizeof(g_BBTFirstBuf);
    pBBT->dwBBTStartBlock = NAND_BLOCK_CNT - BBT_BLOCK_NUM;
    pBBT->dwBBTEndBlock = NAND_BLOCK_CNT;
    pBBT->dwCs = NAND_NUM_OF_DEVICES * NAND_NUM_OF_CS - 1;
}



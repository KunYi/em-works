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
//
//  File:  cspnand.h
//
//  Contains definitions for FMD impletation of the SoC NAND flash controller 
//  and NAND memory device.
//
//------------------------------------------------------------------------------
#ifndef __CSPNAND_H__
#define __CSPNAND_H__

#if    __cplusplus
extern "C" {
#endif

#define NF_CMD(cmd)             { OUTREG16(&g_pNFC->NAND_FLASH_CMD, (cmd)); \
                                  OUTREG16(&g_pNFC->NAND_FLASH_CONFIG2, CSP_BITFMASK(NANDFC_NAND_FLASH_CONFIG2_FCMD)); \
                                  NFCWait(); }

#define NF_ADDR(addr)           { OUTREG16(&g_pNFC->NAND_FLASH_ADDR, (addr)); \
                                  OUTREG16(&g_pNFC->NAND_FLASH_CONFIG2, CSP_BITFMASK(NANDFC_NAND_FLASH_CONFIG2_FADD)); \
                                  NFCWait(); }

#define NF_RD_PAGE()            { OUTREG16(&g_pNFC->NAND_FLASH_CONFIG2, CSP_BITFMASK(NANDFC_NAND_FLASH_CONFIG2_FDO_PAGE)); \
                                  NFCWait(); }

#define NF_WR_PAGE()            { OUTREG16(&g_pNFC->NAND_FLASH_CONFIG2, CSP_BITFMASK(NANDFC_NAND_FLASH_CONFIG2_FDI)); \
                                  NFCWait(); }

#define NF_RD_ID()              { OUTREG16(&g_pNFC->NAND_FLASH_CONFIG2, CSP_BITFMASK(NANDFC_NAND_FLASH_CONFIG2_FDO_ID)); \
                                  NFCWait(); }

#define NF_RD_STATUS()          { OUTREG16(&g_pNFC->NAND_FLASH_CONFIG2, CSP_BITFMASK(NANDFC_NAND_FLASH_CONFIG2_FDO_STATUS)); \
                                  NFCWait(); }

#define NF_BUF_ADDR(num)        { INSREG16BF(&g_pNFC->RAM_BUFFER_ADDRESS, NANDFC_RAM_BUFFER_ADDRESS_RBA, (num)); }


#define NF_ADDR_COL(ColumnAddr) { NF_ADDR((ColumnAddr) & 0xFF);  \
                                  NF_ADDR(((ColumnAddr) >> 8) & 0xFF); }

#define NF_ADDR_PAGE(PageAddr)  { NF_ADDR((PageAddr) & 0xFF);    \
                                  NF_ADDR(((PageAddr) >> 8) & 0xFF);  \
                                  NF_ADDR(((PageAddr) >> 16) & 0xFF); }
                                        
#ifdef __cplusplus
}
#endif                                        

#endif    // __CSPNAND_H__

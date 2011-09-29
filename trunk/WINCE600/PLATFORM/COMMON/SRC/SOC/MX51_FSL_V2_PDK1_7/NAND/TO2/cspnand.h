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
#include "mx51_nandfc_to2.h"

// NFC operate macro

#define NF_CMD(cmd)             { OUTREG32(&g_pNfcAxi->NAND_CMD, (cmd));}

#define NF_CMD_ATOMIC(cmd)      { OUTREG32(&g_pNfcAxi->NAND_CMD, (cmd)); \
                                  OUTREG32(&g_pNfcAxi->LAUNCH_NFC, CSP_BITFMASK(NFC_LAUNCH_NFC_FCMD)); \
                                  NFCWait(); }

#define NF_ADDR(LowAddr, HighAddr) {OUTREG32(&g_pNfcAxi->NAND_ADD_LOW[0], ((LowAddr) | ((HighAddr & 0xffff)<<16))); \
                                  INSREG32BF(&g_pNfcAxi->NAND_ADD_HIGH[0],NFC_NAND_ADD_HIGH_EVEN_GROUP,((HighAddr & 0xffff0000)>>16));}

#define NF_ADDR_ATOMIC(addr)    { OUTREG32(&g_pNfcAxi->NAND_ADD_LOW[0], (addr)); \
                                  OUTREG32(&g_pNfcAxi->LAUNCH_NFC, CSP_BITFMASK(NFC_LAUNCH_NFC_FADD)); \
                                  NFCWait(); }

#define NF_AUTO_LAUNCH(Operation) {OUTREG32(&g_pNfcAxi->LAUNCH_NFC, CSP_BITFMASK(Operation));}

#define NF_WAIT_READ_STATUS_READY() {NFCWait();}

#define NF_GET_RB_STATUS()      {while (!(INREG32(&g_pNfcIp->NFC_IPC) & CSP_BITFMASK(NFC_NFC_IPC_RB_B)));}

#define NF_RD_PAGE()            { OUTREG32(&g_pNfcAxi->LAUNCH_NFC, CSP_BITFMASK(NFC_LAUNCH_NFC_FDO_PAGE)); \
                                  NFCWait(); }

#define NF_WR_PAGE()            { OUTREG32(&g_pNfcAxi->LAUNCH_NFC, CSP_BITFMASK(NFC_LAUNCH_NFC_FDI)); \
                                  NFCWait(); }

#define NF_RD_ID()              { OUTREG32(&g_pNfcAxi->LAUNCH_NFC, CSP_BITFMASK(NFC_LAUNCH_NFC_FDO_ID)); \
                                  NFCWait(); }

#define NF_RD_STATUS()          { OUTREG32(&g_pNfcAxi->LAUNCH_NFC, CSP_BITFMASK(NFC_LAUNCH_NFC_FDO_STATUS)); \
                                  NFCWait(); }

#define NF_BUF_ADDR(num)        { INSREG32BF(&g_pNfcAxi->NFC_CONFIGURATION1, NFC_NFC_CONFIGURATION1_RBA, (num)); }

#define NF_IPC_ENABLE()         { INSREG32BF(&g_pNfcIp->NFC_IPC, NFC_NFC_IPC_CREQ, NFC_NFC_IPC_CREQ_REQUEST);    \
                                  while (!(INREG32(&g_pNfcIp->NFC_IPC) & CSP_BITFMASK(NFC_NFC_IPC_CACK))); }
                                  
#define NF_IPC_DISABLE()        { INSREG32BF(&g_pNfcIp->NFC_IPC, NFC_NFC_IPC_CREQ, NFC_NFC_IPC_CREQ_NO_REQUEST); }

#define NF_CLEAR_STATUS()       { OUTREG32(&g_pNfcIp->NFC_IPC, 0); \
                                  OUTREG32(&g_pNfcAxi->ECC_STATUS_SUM, 0);}
    
#define NF_WAIT_PROG_DONE()     {while(!(INREG32(&g_pNfcIp->NFC_IPC) & CSP_BITFMASK(NFC_NFC_IPC_AUTO_PROG_DONE)));}    
             
// Include NAND memory device definitions

#define NF_ADDR_COL(ColumnAddr)         {NF_ADDR_ATOMIC((ColumnAddr) & 0xFF);  \
                                         NF_ADDR_ATOMIC(((ColumnAddr) >> 8) & 0xFF); }

#define NF_ADDR_PAGE(PageAddr)          {NF_ADDR_ATOMIC((PageAddr) & 0xFF);    \
                                         NF_ADDR_ATOMIC(((PageAddr) >> 8) & 0xFF);  \
                                         NF_ADDR_ATOMIC(((PageAddr) >> 16) & 0xFF); }                                       

#define NF_ADDR_ERASE(Address)  NF_ADDR((Address), 0)

                                        
#ifdef __cplusplus
}
#endif                                        

#endif    // __CSPNAND_H__

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
//  Header: S3C6410_onenand.h
//
//  Defines the OneNAND Controller register layout and associated types and constants.
//
#ifndef __S3C6410_ONENAND_H
#define __S3C6410_ONENAND_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  S3C6410_ONENANDCON_REG
//
//  Defines the OneNAND control register layout. This register bank is 
//  located by the constant S3C6410_BASE_REG_XX_ONENANDCON in the configuration file
//  S3C6410_base_reg_cfg.h.
//
// OneNAND Bank0 Base Address = 0x70100000
// OneNAND Bank1 Base Address = 0x70180000

typedef struct  {
    UINT32 MEM_CFG          ; // 0x000
    UINT32 pad1[3];
    UINT32 BURST_LEN        ; // 0x010
    UINT32 pad2[3];
    UINT32 MEM_RESET0       ; // 0x020
    UINT32 pad3[3];
    UINT32 INT_ERR_STAT ; // 0x030
    UINT32 pad4[3];
    UINT32 INT_ERR_MASK ; // 0x040
    UINT32 pad5[3];
    UINT32 INT_ERR_ACK      ; // 0x050
    UINT32 pad6[3];
    UINT32 ECC_ERR_STAT ; // 0x060
    UINT32 pad7[3];
    UINT32 MANUFACT_ID      ; // 0x070
    UINT32 pad8[3];
    UINT32 DEVICE_ID        ; // 0x080
    UINT32 pad9[3];
    UINT32 DATA_BUF_SIZE    ; // 0x090
    UINT32 pad10[3];
    UINT32 BOOT_BUF_SIZE    ; // 0x0A0
    UINT32 pad11[3];
    UINT32 BUF_AMOUNT       ; // 0x0B0
    UINT32 pad12[3];
    UINT32 TECH         ; // 0x0C0
    UINT32 pad13[3];
    UINT32 FBA_WIDTH        ; // 0x0D0
    UINT32 pad14[3];
    UINT32 FPA_WIDTH        ; // 0x0E0
    UINT32 pad15[3];
    UINT32 FSA_WIDTH        ; // 0x0F0
    UINT32 pad16[3];
    UINT32 REVISION     ; // 0x100
    UINT32 pad17[3];
    UINT32 DATARAM0     ; // 0x110
    UINT32 pad18[3];
    UINT32 DATARAM1     ; // 0x120
    UINT32 pad19[3];
    UINT32 SYNC_MODE        ; // 0x130
    UINT32 pad20[3];
    UINT32 TRANS_SPARE      ; // 0x140
    UINT32 pad21[3];
    UINT32 LOCK_BIT     ; // 0x150
    UINT32 pad22[3];
    UINT32 DBS_DFS_WIDTH    ; // 0x160
    UINT32 pad23[3];
    UINT32 PAGE_CNT     ; // 0x170
    UINT32 pad24[3];
    UINT32 ERR_PAGE_ADDR    ; // 0x180
    UINT32 pad25[3];
    UINT32 BURST_RD_LAT ; // 0x190
    UINT32 pad26[3];
    UINT32 INT_PIN_ENABLE   ; // 0x1A0
    UINT32 pad27[3];
    UINT32 INT_MON_CYC      ; // 0x1B0
    UINT32 pad28[3];
    UINT32 ACC_CLOCK        ; // 0x1C0
    UINT32 pad29[3];
    UINT32 SLOW_RD_PATH ; // 0x1D0
    UINT32 pad30[3];
    UINT32 ERR_BLK_ADDR ; // 0x1E0
    UINT32 pad31[3];
    UINT32 FLASH_VER_ID ; // 0x1F0
    UINT32 pad32[67];
    UINT32 FLASH_AUX_CNTRL; //0x300
    UINT32 pad33[3];
    UINT32 FLASH_AFIFO_CNT; //0x310
} S3C6410_ONENANDCON_REG, *PS3C6410_ONENANDCON_REG;

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 

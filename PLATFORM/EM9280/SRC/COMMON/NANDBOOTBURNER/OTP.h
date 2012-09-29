//-----------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  otp.h
//
//  Contains otp support functions type and other parameter definitions.
//
//-----------------------------------------------------------------------------
#ifndef __OTP_H__
#define __OTP_H__
//-----------------------------------------------------------------------------
#include "common_otp.h"

// Defines
#define OTP_PROGRAM_FREQ (24*1000*1000)
#define OTP_PROGRAM_VOLTAGE (2800)

// when VDDIO_TRG is 0, VDDIO is 2.8.  Each step is 32 mV
#define VDDIO_TRG_2p8VOLTS 0

#define OCOTP_WR_UNLOCK_KEY_VALUE 0x3e770000

#define OCOTP_CRYPTO_LOCKED_READ_VALUE 0xbadabada

#define BM_OCOTP_LOCK_ALT_BITS \
   (BM_OCOTP_LOCK_HWSW_SHADOW_ALT | \
    BM_OCOTP_LOCK_CRYPTODCP_ALT   | \
    BM_OCOTP_LOCK_CRYPTOKEY_ALT)
    
// OCOTP eFuse registers start at REGS_OCOTP_BASE plus 0x20 and 
   // go up to plus 0x210.
   // mask off top 20 bits, shift R 4 bits, subtract 2
#define OCOTP_ADDR_2_REGNUM(regAddr) ((((UINT32)regAddr & 0xfff) >> 4) - 2)

// add 2, shift L 4 bits, add REGS_OCOTP_BASE.  Must be volatile.
#define OCOTP_REGNUM_2_ADDR(regNum) \
   ((volatile reg32_t *)(((regNum + 2) << 4) + REGS_OCOTP_BASE))

//-----------------------------------------------------------------------------
// Types
typedef struct 
{
   UINT32 isXtalSrc;
   UINT32 pllFreq;
   UINT32 cpuDiv;
   UINT32 cpuFracEn;
   UINT32 hclkDiv;
   UINT32 hclkFracEn;
} clk_settings_t;

// writing the OCOTP eFuse registers requires using the 
// register number.
enum ocotpRegNums{
 OCOTP_CUST0_REG_NUM   ,
 OCOTP_CUST1_REG_NUM   ,
 OCOTP_CUST2_REG_NUM   ,
 OCOTP_CUST3_REG_NUM   ,
 OCOTP_CRYPTO0_REG_NUM ,//4
 OCOTP_CRYPTO1_REG_NUM ,
 OCOTP_CRYPTO2_REG_NUM ,
 OCOTP_CRYPTO3_REG_NUM ,
 OCOTP_HWCAP0_REG_NUM  ,//8
 OCOTP_HWCAP1_REG_NUM  ,
 OCOTP_HWCAP2_REG_NUM  ,
 OCOTP_HWCAP3_REG_NUM  ,
 OCOTP_HWCAP4_REG_NUM  ,//12
 OCOTP_HWCAP5_REG_NUM  ,
 OCOTP_SWCAP_REG_NUM ,
 OCOTP_CUSTCAP_REG_NUM   ,
 OCOTP_LOCK_REG_NUM    ,//16
 OCOTP_OPS0_REG_NUM    ,
 OCOTP_OPS1_REG_NUM    ,
 OCOTP_OPS2_REG_NUM    ,
 OCOTP_OPS3_REG_NUM    ,//20
 OCOTP_UN0_REG_NUM     ,
 OCOTP_UN1_REG_NUM     ,
 OCOTP_UN2_REG_NUM     ,
 OCOTP_ROM0_REG_NUM    ,//24
 OCOTP_ROM1_REG_NUM    ,
 OCOTP_ROM2_REG_NUM    ,
 OCOTP_ROM3_REG_NUM    ,
 OCOTP_ROM4_REG_NUM    ,//28
 OCOTP_ROM5_REG_NUM    ,
 OCOTP_ROM6_REG_NUM    ,
 OCOTP_ROM7_REG_NUM    ,
 OCOTP_MAX_REGS
};

void InitOTP(void);
void DeInitOTP(void);
BOOL OTPProgram(POtpProgram pOtp);
BOOL OTPRead(POtpProgram pOtp);

// CS&ZHL APR-10-2012: address offset for OTPRead()/OTPProgram()
#define OCOTP_ADDR_OFFSET(regNum)		((regNum + 2) << 4)

#endif // __OTP_H__
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: common_kpp.h
//
//  Provides definitions for KPP (KeyPad Port) module 
//  that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_KPP_H
#define __COMMON_KPP_H

#if __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
     UINT16 KPCR;
     UINT16 KPSR;
     UINT16 KDDR;
     UINT16 KPDR;
} CSP_KPP_REGS, *PCSP_KPP_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define KPP_KPCR_OFFSET           0x0000
#define KPP_KPSR_OFFSET           0x0002
#define KPP_KDDR_OFFSET           0x0004
#define KPP_KPDR_OFFSET           0x0006


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define KPP_KPCR_KRE_LSH          0
#define KPP_KPCR_KCO_LSH          8

#define KPP_KPSR_KPKD_LSH         0
#define KPP_KPSR_KPKR_LSH         1
#define KPP_KPSR_KDSC_LSH         2
#define KPP_KPSR_KRSS_LSH         3
#define KPP_KPSR_KDIE_LSH         8
#define KPP_KPSR_KRIE_LSH         9
#define KPP_KPSR_KPP_EN_LSH       10

#define KPP_KDDR_KRDD_LSH         0
#define KPP_KDDR_KCDD_LSH         8

#define KPP_KPDR_KRD_LSH          0
#define KPP_KPDR_KCD_LSH          8


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define KPP_KPCR_KRE_WID          8
#define KPP_KPCR_KCO_WID          8

#define KPP_KPSR_KPKD_WID         1
#define KPP_KPSR_KPKR_WID         1
#define KPP_KPSR_KDSC_WID         1
#define KPP_KPSR_KRSS_WID         1
#define KPP_KPSR_KDIE_WID         1
#define KPP_KPSR_KRIE_WID         1
#define KPP_KPSR_KPP_EN_WID       1

#define KPP_KDDR_KRDD_WID         8
#define KPP_KDDR_KCDD_WID         8

#define KPP_KPDR_KRD_WID          8
#define KPP_KPDR_KCD_WID          8


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define KPP_KPSR_KPKD_CLEAR             1 // Clear keypad key depress
                                          // status bit (write-1-to-clear)

#define KPP_KPSR_KPKR_CLEAR             1 // Clear keypad key release
                                          // status bit (write-1-to-clear)

#define KPP_KPSR_KDSC_CLEAR             1 // Clear key depress synchronizer
                                          // Bit is self-negating

#define KPP_KPSR_KRSS_SET               1 // Set key release synchronizer
                                          // Bit is self-negating

#define KPP_KPSR_KDIE_INT_ENABLE        1 // Keypad key depress int enabled
#define KPP_KPSR_KDIE_INT_DISABLE       0 // Keypad key depress int disabled

#define KPP_KPSR_KRIE_INT_ENABLE        1 // Keypad key release int enabled
#define KPP_KPSR_KRIE_INT_DISABLE       0 // Keypad key release int disabled

#define KPP_KPSR_KPP_EN_ENABLE          1 // Enable high freq clk to keypad
#define KPP_KPSR_KPP_EN_DISABLE         0 // Disable high freq clk to keypad

#ifdef __cplusplus
}
#endif

#endif // __COMMON_KPP_H

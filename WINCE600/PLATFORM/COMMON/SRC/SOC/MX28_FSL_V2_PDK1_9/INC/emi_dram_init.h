//------------------------------------------------------------------------------
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
//  Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: emi_dram_init.h
//  Initialization function macro for EMI DRAM controller
//
//-----------------------------------------------------------------------------
//
// WARNING!  THIS FILE IS AUTOMATICALLY GENERATED.
//           DO NOT MODIFY THIS FILE DIRECTLY.
//
//-----------------------------------------------------------------------------//

#ifndef _EMI_DRAM_INIT_H
#define _EMI_DRAM_INIT_H  1

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "regsdram.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define DRAM_REG      ((volatile UINT32*) HW_DRAM_CTL00_ADDR)
/* To access a DRAM programmable register
 * such as DRAM_REG[0] */

//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Function Prototypes / Function Macros
//------------------------------------------------------------------------------
#define InitEmiDram() \
    { \
        DRAM_REG[ 0] = 0x01010001;  /* 0000000_1 ahb0_w_priority 0000000_1 ahb0_r_priority 0000000_0 ahb0_fifo_type_reg 0000000_1 addr_cmp_en */ \
        DRAM_REG[ 1] = 0x00010100;  /* 0000000_0 ahb2_fifo_type_reg 0000000_1 ahb1_w_priority 0000000_1 ahb1_r_priority 0000000_0 ahb1_fifo_type_reg */ \
        DRAM_REG[ 2] = 0x01000101;  /* 0000000_1 ahb3_r_priority 0000000_0 ahb3_fifo_type_reg 0000000_1 ahb2_w_priority 0000000_1 ahb2_r_priority */ \
        DRAM_REG[ 3] = 0x00000001;  /* 0000000_0 auto_refresh_mode 0000000_0 arefresh 0000000_0 ap 0000000_1 ahb3_w_priority */ \
        DRAM_REG[ 4] = 0x01000101;  /* 0000000_1 dll_bypass_mode 0000000_0 dlllockreg 0000000_1 concurrentap 0000000_1 bank_split_en */ \
        DRAM_REG[ 5] = 0x00000000;  /* 0000000_0 intrptreada 0000000_0 intrptapburst 0000000_0 fast_write 0000000_0 en_lowpower_mode */ \
        DRAM_REG[ 6] = 0x00010000;  /* 0000000_0 power_down 0000000_1 placement_en 0000000_0 no_cmd_init 0000000_0 intrptwritea */ \
        DRAM_REG[ 7] = 0x01000001;  /* 0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 rd2rd_turn 0000000_1 priority_en */ \
        DRAM_REG[ 9] = 0x00000001;  /* 000000_00 out_of_range_type 000000_00 out_of_range_source_id 0000000_0 write_modereg 0000000_1 writeinterp */ \
        DRAM_REG[10] = 0x07000200;  /* 00000_111 age_count 00000_000 addr_pins 000000_10 temrs 000000_00 q_fullness */ \
        DRAM_REG[11] = 0x00070303;  /* 00000_000 max_cs_reg 00000_111 command_age_count 00000_011 column_size 00000_011 caslat */ \
        DRAM_REG[12] = 0x02010000;  /* 00000_010 twr_int 00000_001 trrd 0000000000000_000 tcke */ \
        DRAM_REG[13] = 0x06060a01;  /* 0000_0110 caslat_lin_gate 0000_0110 caslat_lin 0000_1010 aprebit 00000_001 twtr */ \
        DRAM_REG[14] = 0x00000201;  /* -->Changed from 0x0000020f to 0x00000201<-- 0000_0000 max_col_reg 0000_0000 lowpower_refresh_enable 0000_0010 initaref 0000_1111 cs_map */ \
        DRAM_REG[15] = 0x01030000;  /* 0000_0001 trp 0000_0011 tdal 0000_0000 port_busy 0000_0000 max_row_reg */ \
        DRAM_REG[16] = 0x02000000;  /* 000_00010 tmrd 000_00000 lowpower_control 000_00000 lowpower_auto_enable 0000_0000 int_ack */ \
        DRAM_REG[17] = 0x3e005502;  /* 00111110 dll_start_point 00000000 dll_lock 01010101 dll_increment 000_00010 trc */ \
        DRAM_REG[18] = 0x00000000;  /* 0_0000000 dll_dqs_delay_1 0_0000000 dll_dqs_delay_0 000_00000 int_status 000_00000 int_mask */ \
        DRAM_REG[19] = 0x027f0202;  /* 00000010 dqs_out_shift_bypass 0_1111111 dqs_out_shift 00000010 dll_dqs_delay_bypass_1 00000010 dll_dqs_delay_bypass_0 */ \
        DRAM_REG[20] = 0x01020201;  /* 00000001 trcd_int 00000010 tras_min 00000010 wr_dqs_shift_bypass 0_0000001 wr_dqs_shift */ \
        DRAM_REG[21] = 0x00000002;  /* 00000000000000_0000000000 out_of_range_length 00000010 trfc */ \
        DRAM_REG[22] = 0x00080008;  /* 00000_00000001000 ahb0_wrcnt 00000_00000001000 ahb0_rdcnt */ \
        DRAM_REG[23] = 0x00200020;  /* 00000_00000100000 ahb1_wrcnt 00000_00000100000 ahb1_rdcnt */ \
        DRAM_REG[24] = 0x00200020;  /* 00000_00000100000 ahb2_wrcnt 00000_00000100000 ahb2_rdcnt */ \
        DRAM_REG[25] = 0x00200020;  /* 00000_00000100000 ahb3_wrcnt 00000_00000100000 ahb3_rdcnt */ \
        DRAM_REG[26] = 0x000000b3;  /* 00000000000000000000_000010110011 tref */ \
        DRAM_REG[27] = 0x00000000;  /* 00000000000000000000000000000000 */ \
        DRAM_REG[28] = 0x00000000;  /* 00000000000000000000000000000000 */ \
        DRAM_REG[29] = 0x00000000;  /* 0000000000000000 lowpower_internal_cnt 0000000000000000 lowpower_external_cnt */ \
        DRAM_REG[30] = 0x00000000;  /* 0000000000000000 lowpower_refresh_hold 0000000000000000 lowpower_power_down_cnt */ \
        DRAM_REG[31] = 0x00000000;  /* 0000000000000000 tdll 0000000000000000 lowpower_self_refresh_cnt */ \
        DRAM_REG[32] = 0x00020b37;  /* 0000000000000010 txsnr 0000101100110111 tras_max */ \
        DRAM_REG[33] = 0x00000002;  /* 0000000000000000 version 0000000000000010 txsr */ \
        DRAM_REG[34] = 0x00000020;  /* -->Shorten init time to speed up simulation<-- */ \
        DRAM_REG[35] = 0x00000000;  /* 0_0000000000000000000000000000000 out_of_range_addr */ \
        DRAM_REG[36] = 0x00000101;  /* 0000000_0 pwrup_srefresh_exit 0000000_0 enable_quick_srefresh 0000000_1 bus_share_enable 0000000_1 active_aging */ \
        DRAM_REG[37] = 0x00040001;  /* 00000000000001_0000000000 bus_share_timeout 0000000_1 tref_enable */ \
        DRAM_REG[38] = 0x00000000;  /* 000_0000000000000 emrs2_data_0 000_0000000000000 emrs1_data */ \
        DRAM_REG[39] = 0x00000000;  /* 000_0000000000000 emrs2_data_2 000_0000000000000 emrs2_data_1 */ \
        DRAM_REG[40] = 0x00010000;  /* 0000000000000001 tpdex 000_0000000000000 emrs2_data_3 */ \
        DRAM_REG[ 8] = 0x00010001;  /* 0000000_0 tras_lockout 0000000_1 start 0000000_0 srefresh 0000000_1 sdr_mode */ \
    }
#endif  // _EMI_DRAM_INIT_H


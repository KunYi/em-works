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
//  Copyright (C) 2009-2010 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include <xldr.h>

//-----------------------------------------------------------------------------
// Global Variables

void * volatile const pTOC = (void *) -1;
static BOOL s_b5V          = FALSE;
static BOOL s_bUsb         = FALSE;
static BOOL s_bBattery     = FALSE;
static UINT32 BattVoltage  = 0;

//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function:  start
//
//  This function is the main entrance of XLDR.
//
//  Parameters:
//          
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
int start()
{
    PBYTE pbOCRAM = (PBYTE)IMAGE_WINCE_IRAM_PA_START;
    
    EMI_MemType_t        type = EMI_DEV_DDR2;
	//SEP-14-2012 LQK: Enable WDT
	//HW_RTC_WATCHDOG_WR( 10000 );
	//HW_RTC_CTRL_SET(BM_RTC_CTRL_WATCHDOGEN);
	
    InitDebugSerial();
    InitPower(); 
    InitSdram(type);
    
    HW_RTC_PERSISTENT3_WR(0);
    
    // check for SDBoot
    if ( ((*(PBYTE)((DWORD)pbOCRAM + OCRAM_BOOT_MODE_OFFSET)) & BOOT_MODE_MASK) == BOOT_MODE_SDMMC)
    {
        HW_RTC_PERSISTENT3_WR(BOOT_MODE_SDMMC);
    }
    
    // check for SPIBoot
    if ( ((*(PBYTE)((DWORD)pbOCRAM + OCRAM_BOOT_MODE_OFFSET)) & BOOT_MODE_MASK) == BOOT_MODE_SPI)
    {
        HW_RTC_PERSISTENT3_WR(BOOT_MODE_SPI);
    }
    //--------------------------------------------------------------------------
    // Return to the ROM.
    //--------------------------------------------------------------------------
    return 0;
}

//-----------------------------------------------------------------------------
//
//  Function:  InitSdram
//
//  This function is to initial SDRAM.
//
//  Parameters:
//          type:DDR2 ,LVDDR2 or mDDR.
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID InitSdram(EMI_MemType_t type)
{
    if(!HW_CLKCTRL_EMI.B.CLKGATE)
        HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_EMI);

    HW_CLKCTRL_EMI_CLR(BM_CLKCTRL_EMI_CLKGATE);
    // Wait for the clock to settle.
    while(HW_CLKCTRL_EMI.B.BUSY_REF_XTAL);
    // Set divider for DIV_XTAL
    HW_CLKCTRL_EMI.B.DIV_XTAL = 1;

    // Enable the EMI block by clearing the Soft Reset and Clock Gate
    HW_CLKCTRL_EMI_CLR(BM_CLKCTRL_EMI_CLKGATE);

    ConfigureEmiPins(EMI_PIN_DRIVE_ADDRESS, EMI_PIN_DRIVE_CONTROL,EMI_PIN_DRIVE_CLOCK, EMI_PIN_DRIVE_DATA_SLICE_0, EMI_PIN_DRIVE_DATA_SLICE_1);
     // Set PAD mode
    if (type == EMI_DEV_MOBILE_DDR) 
      HW_PINCTRL_EMI_DS_CTRL.B.DDR_MODE = 0;
    else if (type == EMI_DEV_LVDDR2 )
      HW_PINCTRL_EMI_DS_CTRL.B.DDR_MODE = 2;
    else if (type == EMI_DEV_DDR2 )
      HW_PINCTRL_EMI_DS_CTRL.B.DDR_MODE = 3;

    HW_CLKCTRL_FRAC0_SET(BM_CLKCTRL_FRAC0_CLKGATEEMI);// needed as sort of a high freq. "filter" for peripherals prior to ramping up PLL
    HW_CLKCTRL_PLL0CTRL0_SET(BM_CLKCTRL_PLL0CTRL0_POWER);
    // todo There is a bit that can tell you when its stable, but it takes FOREVER, so we use this much lower time that Mike May said was good.
    XLDRStall(100);
    HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATEEMI);
    // EMI freq = 200MHz
    HW_CLKCTRL_FRAC0.B.EMIFRAC = 22;    //      odo - might be good to move this BEFORE the above line.
    // Set divider for DIV_EMI
    HW_CLKCTRL_EMI.B.DIV_EMI = 2;
    if(!HW_CLKCTRL_FRAC0.B.CLKGATEEMI)
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_EMI);

    //enable EMI_CLK output     
    HW_DRAM_CTL00_SET(BM_DRAM_CTL00_BRESP_TIMING);
    //Initialize EMI...
    HW_DRAM_CTL16_CLR(BM_DRAM_CTL16_START); //clear "start"
    DDR2EmiController_EDE1116_200MHz(); 

    HW_DRAM_CTL17.B.SREFRESH = 0;

    HW_DRAM_CTL16_SET(BM_DRAM_CTL16_START);//set "start"

    //Wait for EMI initization completed
    while ( (HW_DRAM_CTL58_RD() & 0x00100000) != 0x00100000 );
}

//-----------------------------------------------------------------------------
//
//  Function:  ConfigureEmiPins
//
//  This function is to initial EMI PIN.
//
//  Parameters:
//          pin_drive_addr:pin drive strength
//          pin_drive_ctrl:pin drive strength
//          pin_drive_clk:pin drive strength
//          pin_drive_data_slice_0:pin drive strength
//          pin_drive_data_slice_1:pin drive strength
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void ConfigureEmiPins(
    TPinDrive pin_drive_addr,
    TPinDrive pin_drive_ctrl,
    TPinDrive pin_drive_clk,
    TPinDrive pin_drive_data_slice_0,
    TPinDrive pin_drive_data_slice_1)
{
    // must clear the HW_PINCTRL_CTRL_CLR to enable pinmux block clock --APB clock
    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_CLKGATE | BM_PINCTRL_CTRL_SFTRST);
    //-------------------------------------------------------------------------    
    // First, set the voltage and drive strength of these pins as specified.
    // Second, set the pinmux value to 0x0 to enable the EMI connection.
    //-------------------------------------------------------------------------   
    // Set drive strength 
    HW_PINCTRL_EMI_DS_CTRL_SET(
        BF_PINCTRL_EMI_DS_CTRL_ADDRESS_MA(pin_drive_addr)        |
        BF_PINCTRL_EMI_DS_CTRL_CONTROL_MA(pin_drive_ctrl)        |
        BF_PINCTRL_EMI_DS_CTRL_DUALPAD_MA(pin_drive_clk)         |
        BF_PINCTRL_EMI_DS_CTRL_SLICE0_MA(pin_drive_data_slice_0) |
        BF_PINCTRL_EMI_DS_CTRL_SLICE1_MA(pin_drive_data_slice_1));
    
    // Set the pinmux for EMI
    
    // Configure Bank-6 EMI_D15 ~ EMI_D00 as EMI pins
    HW_PINCTRL_MUXSEL10_CLR(
        BM_PINCTRL_MUXSEL10_BANK5_PIN15 | 
        BM_PINCTRL_MUXSEL10_BANK5_PIN14 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN13 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN12 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN11 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN10 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN09 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN08 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN07 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN06 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN05 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN04 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN03 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN02 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN01 |
        BM_PINCTRL_MUXSEL10_BANK5_PIN00 );
    
    // EMI_DDR_OPEN ,EMI_DQS1,EMI_DQS0,EMI_CLK,EMI_DDR_OPEN_FB,EMI_DQM1,EMI_ODT1,EMI_DQM0,EMI_ODT0
    HW_PINCTRL_MUXSEL11_CLR(
        BM_PINCTRL_MUXSEL11_BANK5_PIN26 |
        BM_PINCTRL_MUXSEL11_BANK5_PIN23 |
        BM_PINCTRL_MUXSEL11_BANK5_PIN22 |
        BM_PINCTRL_MUXSEL11_BANK5_PIN21 |
        BM_PINCTRL_MUXSEL11_BANK5_PIN20 |
        BM_PINCTRL_MUXSEL11_BANK5_PIN19 |
        BM_PINCTRL_MUXSEL11_BANK5_PIN18 |
        BM_PINCTRL_MUXSEL11_BANK5_PIN17 |
        BM_PINCTRL_MUXSEL11_BANK5_PIN16 );
    
    // EMI_A14 ~ EMI_A00
    HW_PINCTRL_MUXSEL12_CLR(
        BM_PINCTRL_MUXSEL12_BANK6_PIN14 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN13 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN12 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN11 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN10 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN09 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN08 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN07 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN06 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN05 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN04 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN03 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN02 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN01 |
        BM_PINCTRL_MUXSEL12_BANK6_PIN00 );
    
    // EMI_CKE,EMI_CE1N,EMI_CE0N,EMI_WEN,EMI_RASN,EMI_CASN,EMI_BA2,EMI_BA1,EMI_BA0
    HW_PINCTRL_MUXSEL13_CLR(
        BM_PINCTRL_MUXSEL13_BANK6_PIN24 |
        BM_PINCTRL_MUXSEL13_BANK6_PIN23 |
        BM_PINCTRL_MUXSEL13_BANK6_PIN22 |
        BM_PINCTRL_MUXSEL13_BANK6_PIN21 |
        BM_PINCTRL_MUXSEL13_BANK6_PIN20 |
        BM_PINCTRL_MUXSEL13_BANK6_PIN19 |
        BM_PINCTRL_MUXSEL13_BANK6_PIN18 |
        BM_PINCTRL_MUXSEL13_BANK6_PIN17 |
        BM_PINCTRL_MUXSEL13_BANK6_PIN16 );
}

//-----------------------------------------------------------------------------
//
//  Function:  DDR2EmiController_EDE1116_200MHz
//
//  This function is to initial EMI controlller for DDR2 200MHz.
//
//  Parameters:
//          None.
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void DDR2EmiController_EDE1116_200MHz(void)
{
 volatile UINT32* DRAM_REG = (volatile UINT32*) HW_DRAM_CTL00_ADDR;

 DRAM_REG[0] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_0(RW) 
 DRAM_REG[1] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_1(RW) 
 DRAM_REG[2] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_2(RW) 
 DRAM_REG[3] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_3(RW) 
 DRAM_REG[4] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_4(RW) 
 DRAM_REG[5] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_5(RW) 
 DRAM_REG[6] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_6(RW) 
 DRAM_REG[7] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_7(RW) 
 DRAM_REG[8] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_0(RD) 
 DRAM_REG[9] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_1(RD) 
 DRAM_REG[10] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_2(RD) 
 DRAM_REG[11] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_3(RD) 
 DRAM_REG[12] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_4(RD) 
 DRAM_REG[13] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_5(RD) 
 DRAM_REG[14] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_6(RD) 
 DRAM_REG[15] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_7(RD) 
 DRAM_REG[16] = 0x00000000 ; //0000000_0 write_modereg(WR) 0000000_0 power_down(RW) 000000000000000_0 start(RW) 
 DRAM_REG[17] = 0x00000100 ; //0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_1 enable_quick_srefresh(RW) 0000000_0 srefresh(RW+) 
 DRAM_REG[18] = 0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[19] = 0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[20] = 0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[21] = 0x00000000 ; //00000_000 cke_delay(RW) 00000000 dll_lock(RD) 0000000_0 dlllockreg(RD) 0000000_0 dll_bypass_mode(RW) 
 DRAM_REG[22] = 0x00000000 ; //000000000000_0000 lowpower_refresh_enable(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 
 DRAM_REG[23] = 0x00000000 ; //0000000000000000 lowpower_internal_cnt(RW) 0000000000000000 lowpower_external_cnt(RW) 
 DRAM_REG[24] = 0x00000000 ; //0000000000000000 lowpower_self_refresh_cnt(RW) 0000000000000000 lowpower_refresh_hold(RW) 
 DRAM_REG[25] = 0x00000000 ; //00000000000000000000000000000000 lowpower_power_down_cnt(RW) 
 DRAM_REG[26] = 0x00010101 ; //000000000000000_1 priority_en(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 placement_en(RW) 
 DRAM_REG[27] = 0x01010101 ; //0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_1 bank_split_en(RW) 0000000_1 rw_same_en(RW) 
 DRAM_REG[28] = 0x000f0f01 ; //00000_000 q_fullness(RW) 0000_1111 age_count(RW) 0000_1111 command_age_count(RW) 0000000_1 active_aging(RW) 
 DRAM_REG[29] = 0x0f02020a ; //0000_1111 cs_map(RW) 00000_010 column_size(RW) 00000_010 addr_pins(RW) 0000_1010 aprebit(RW) 
 DRAM_REG[30] = 0x00000000 ; //0000000000000_000 max_cs_reg(RD) 0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 
 DRAM_REG[31] = 0x00010101 ; //000000000000000_1 eight_bank_mode(RW) 0000000_1 drive_dq_dqs(RW) 0000000_1 dqs_n_en(RW) 
 DRAM_REG[32] = 0x00000100 ; //00000000000000000000000_1 reduc(RW) 0000000_0 reg_dimm_enable(RW) 
 DRAM_REG[33] = 0x00000100 ; //00000000000000000000000_1 concurrentap(RW) 0000000_0 ap(RW) 
 DRAM_REG[34] = 0x00000000 ; //0000000_0 writeinterp(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
 DRAM_REG[35] = 0x00000002 ; //000000000000000_0 pwrup_srefresh_exit(RW) 0000000_0 no_cmd_init(RW) 0000_0010 initaref(RW) 
 DRAM_REG[36] = 0x01010000 ; //0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 000000000000000_0 fast_write(RW) 
 DRAM_REG[37] = 0x07080403 ; //0000_0111 caslat_lin_gate(RW) 0000_1000 caslat_lin(RW) 00000_100 caslat(RW) 0000_0011 wrlat(RW) 
 DRAM_REG[38] = 0x06005003 ; //000_00110 tdal(RW) 0000000001010000 tcpd(RW) 00000_011 tcke(RW) 
 DRAM_REG[39] = 0x0a0000c8 ; //00_001010 tfaw(RW) 000000000000000011001000 tdll(RW) 
 DRAM_REG[40] = 0x02009c40 ; //000_00010 tmrd(RW) 000000000111010100100010 tinit(RW) 
 DRAM_REG[41] = 0x0002030c ; //0000000000000010 tpdex(RW) 00000011 trcd_int(RW) 00_001100 trc(RW) 
 DRAM_REG[42] = 0x0036a609 ; //000000000011011010100110 tras_max(RW) 00001001 tras_min(RW) 
 DRAM_REG[43] = 0x031a0612 ; //0000_0011 trp(RW) 00011010 trfc(RW) 00_00011000010010 tref(RW) 
 DRAM_REG[44] = 0x02030202 ; //0000_0010 twtr(RW) 000_00011 twr_int(RW) 00000_010 trtp(RW) 00000_010 trrd(RW) 
 DRAM_REG[45] = 0x00c8001c ; //0000000011001000 txsr(RW) 0000000000011100 txsnr(RW) 
 DRAM_REG[46] = 0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[47] = 0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[48] = 0x00012100 ; //0_0000000 axi0_current_bdw(RD) 0000000_1 axi0_bdw_ovflow(RW) 0_0100001 axi0_bdw(RW) 000000_00 axi0_fifo_type_reg(RW) 
 //DRAM_REG[49] = 0x55550303 ; //0101010101010101 axi0_en_size_lt_width_instr(RW) 00000_011 axi0_w_priority(RW) 00000_011 axi0_r_priority(RW) 
 DRAM_REG[49] = 0xffff0303 ; //0101010101010101 axi0_en_size_lt_width_instr(RW) 00000_011 axi0_w_priority(RW) 00000_011 axi0_r_priority(RW) 
 DRAM_REG[50] = 0x00012100 ; //0_0000000 axi1_current_bdw(RD) 0000000_1 axi1_bdw_ovflow(RW) 0_0100001 axi1_bdw(RW) 000000_00 axi1_fifo_type_reg(RW) 
 DRAM_REG[51] = 0xffff0303 ; //1111111100000000 axi1_en_size_lt_width_instr(RW) 00000_011 axi1_w_priority(RW) 00000_011 axi1_r_priority(RW) 
 DRAM_REG[52] = 0x00012100 ; //0_0000000 axi2_current_bdw(RD) 0000000_1 axi2_bdw_ovflow(RW) 0_0100001 axi2_bdw(RW) 000000_00 axi2_fifo_type_reg(RW) 
 DRAM_REG[53] = 0xffff0303 ; //0000000000000001 axi2_en_size_lt_width_instr(RW) 00000_011 axi2_w_priority(RW) 00000_011 axi2_r_priority(RW) 
 DRAM_REG[54] = 0x00012100 ; //0_0000000 axi3_current_bdw(RD) 0000000_1 axi3_bdw_ovflow(RW) 0_0100001 axi3_bdw(RW) 000000_00 axi3_fifo_type_reg(RW) 
 DRAM_REG[55] = 0xffff0303 ; //0000000000000001 axi3_en_size_lt_width_instr(RW) 00000_011 axi3_w_priority(RW) 00000_011 axi3_r_priority(RW) 
 DRAM_REG[56] = 0x00000003 ; //00000000000000000000000000000_011 arb_cmd_q_threshold(RW) 
 DRAM_REG[57] = 0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[58] = 0x00000000 ; //00000_00000000000 int_status(RD) 00000_00000000000 int_mask(RW) 
 DRAM_REG[59] = 0x00000000 ; //00000000000000000000000000000000 out_of_range_addr(RD) 
 DRAM_REG[60] = 0x00000000 ; //000000000000000000000000000000_00
 DRAM_REG[61] = 0x00000000 ; //00_000000 out_of_range_type(RD) 0_0000000 out_of_range_length(RD) 000_0000000000000 out_of_range_source_id(RD) 
 DRAM_REG[62] = 0x00000000 ; //00000000000000000000000000000000 port_cmd_error_addr(RD) 
 DRAM_REG[63] = 0x00000000 ; //000000000000000000000000000000_00
 DRAM_REG[64] = 0x00000000 ; //00000000000_0000000000000 port_cmd_error_id(RD) 0000_0000 port_cmd_error_type(RD) 
 DRAM_REG[65] = 0x00000000 ; //00000000000_0000000000000 port_data_error_id(RD) 00000_000 port_data_error_type(RD) 
 DRAM_REG[66] = 0x00000612 ; //000000000000_0000 tdfi_ctrlupd_min(RD) 00_00011000010010 tdfi_ctrlupd_max(RW) 
 //DRAM_REG[67] = 0x01000002 ; //0000_0001 tdfi_dram_clk_enable(RW) 00000_000 tdfi_dram_clk_disable(RW) 0000_0000 dram_clk_disable(RW) 0000_0010 tdfi_ctrl_delay(RW) 
 DRAM_REG[67] = 0x01000f02 ; //0000_0001 tdfi_dram_clk_enable(RW) 00000_000 tdfi_dram_clk_disable(RW) 0000_0000 dram_clk_disable(RW) 0000_0010 tdfi_ctrl_delay(RW) 
 DRAM_REG[68] = 0x06120612 ; //00_00011000010010 tdfi_phyupd_type0(RW) 00_00011000010010 tdfi_phyupd_resp(RW) 
 DRAM_REG[69] = 0x00000200 ; //00000000000000000000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 
 DRAM_REG[70] = 0x00020007 ; //000000000000_0010 tdfi_rddata_en_base(RW) 0000_0000 tdfi_rddata_en(RD) 0000_0111 tdfi_phy_rdlat(RW) 
 DRAM_REG[71] = 0xf4004a27;
 DRAM_REG[72] = 0xf4004a27;
 DRAM_REG[73] = 0xf4004a27;
 DRAM_REG[74] = 0xf4004a27;
 DRAM_REG[75] = 0x07000300 ; //00000111000000000000001100000000 phy_ctrl_reg_1_0(RW) 
 DRAM_REG[76] = 0x07000300 ; //00000111000000000000001100000000 phy_ctrl_reg_1_1(RW) 
 DRAM_REG[77] = 0x07400300 ; //00000111010000000000001100000000 phy_ctrl_reg_1_2(RW) 
 DRAM_REG[78] = 0x07400300 ; //00000111010000000000001100000000 phy_ctrl_reg_1_3(RW) 
 DRAM_REG[79] = 0x00000005 ; //00000000000000000000000000000101 phy_ctrl_reg_2(RW) 
 DRAM_REG[80] = 0x00000000 ; //00000000000000000000000000000000 dft_ctrl_reg(RW) 
 DRAM_REG[81] = 0x00000000 ; //0000000000000000000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 
 DRAM_REG[82] = 0x01000000 ; //0000000_1 odt_alt_en(RW) 000000000000000000000000
 DRAM_REG[83] = 0x01020408 ; //0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
 DRAM_REG[84] = 0x08040201 ; //0000_1000 odt_wr_map_cs3(RW) 0000_0100 odt_wr_map_cs2(RW) 0000_0010 odt_wr_map_cs1(RW) 0000_0001 odt_wr_map_cs0(RW) 
 DRAM_REG[85] = 0x000f1133 ; //00000000000011110001000100110011 pad_ctrl_reg_0(RW) 
 DRAM_REG[86] = 0x00000000 ; //00000000000000000000000000000000 version(RD) 
 DRAM_REG[87] = 0x00001f04;
 DRAM_REG[88] = 0x00001f04;
 DRAM_REG[89] = 0x00001f04;
 DRAM_REG[90] = 0x00001f04;
 DRAM_REG[91] = 0x00001f04;
 DRAM_REG[92] = 0x00001f04;
 DRAM_REG[93] = 0x00001f04;
 DRAM_REG[94] = 0x00001f04;
 DRAM_REG[95] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_0(RD) 
 DRAM_REG[96] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_1(RD) 
 DRAM_REG[97] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_2(RD) 
 DRAM_REG[98] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_3(RD) 
 DRAM_REG[99] = 0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_0(RD) 
 DRAM_REG[100] =0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_1(RD) 
 DRAM_REG[101] =0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_2(RD) 
 DRAM_REG[102] =0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_3(RD) 
 DRAM_REG[103] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_0(RD) 
 DRAM_REG[104] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[105] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[106] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[107] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[108] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_1(RD) 
 DRAM_REG[109] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[110] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[111] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[112] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[113] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_2(RD) 
 DRAM_REG[114] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[115] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[116] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[117] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[118] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_3(RD) 
 DRAM_REG[119] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[120] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[121] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[122] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[123] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_0(RD) 
 DRAM_REG[124] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[125] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[126] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[127] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[128] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_1(RD) 
 DRAM_REG[129] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[130] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[131] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[132] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[133] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_2(RD) 
 DRAM_REG[134] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[135] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[136] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[137] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[138] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_3(RD) 
 DRAM_REG[139] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[140] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[141] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[142] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[143] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_0(RD) 
 DRAM_REG[144] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[145] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[146] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[147] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[148] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_1(RD) 
 DRAM_REG[149] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[150] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[151] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[152] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[153] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_2(RD) 
 DRAM_REG[154] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[155] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[156] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[157] =0x00000000 ; //00000000000000000000000_000000000
 DRAM_REG[158] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_3(RD) 
 DRAM_REG[159] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[160] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[161] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[162] =0x00010000 ; //00000_000 w2r_samecs_dly(RW) 00000_001 w2r_diffcs_dly(RW) 0000000_000000000
 DRAM_REG[163] =0x00030404 ; //00000000 dll_rst_adj_dly(RW) 0000_0011 wrlat_adj(RW) 0000_0100 rdlat_adj(RW) 0000_0100 dram_class(RW) 
 DRAM_REG[164] =0x00000003 ; //00000000000000_0000000000 int_ack(WR) 00000011 tmod(RW) 
 DRAM_REG[165] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[166] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[167] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[168] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[169] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[170] =0x00000000 ; //00000000000000000000000000000000
 DRAM_REG[171] =0x01010000 ; //0000000_1 axi5_bdw_ovflow(RW) 0000000_1 axi4_bdw_ovflow(RW) 0000000000000000 dll_rst_delay(RW) 
 DRAM_REG[172] =0x01000000 ; //0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 concurrentap_wr_only(RW) 0000000_0 cke_status(RD) 
 DRAM_REG[173] =0x03030000 ; //00000_011 axi4_w_priority(RW) 00000_011 axi4_r_priority(RW) 000000_00 axi5_fifo_type_reg(RW) 000000_00 axi4_fifo_type_reg(RW) 
 DRAM_REG[174] =0x00010303 ; //00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_011 axi5_w_priority(RW) 00000_011 axi5_r_priority(RW) 
 DRAM_REG[175] =0x01020202 ; //00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 
 DRAM_REG[176] =0x00000000 ; //0000_0000 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 0000_0000 add_odt_clk_difftype_diffcs(RW) 00000_000 w2w_samecs_dly(RW) 
 DRAM_REG[177] =0x02040303 ; //000_00010 tccd(RW) 0000_0100 trp_ab(RW) 0000_0011 cksrx(RW) 0000_0011 cksre(RW) 
 DRAM_REG[178] =0x21002103 ; //0_0100001 axi5_bdw(RW) 0_0000000 axi4_current_bdw(RD) 0_0100001 axi4_bdw(RW) 000_00011 tckesr(RW) 
 DRAM_REG[179] =0x00061200 ; //0000000000_00011000010010 tdfi_phyupd_type1(RW) 0_0000000 axi5_current_bdw(RD) 
 DRAM_REG[180] =0x06120612 ; //00_00011000010010 tdfi_phyupd_type3(RW) 00_00011000010010 tdfi_phyupd_type2(RW) 
 DRAM_REG[181] =0x04420442 ; //0_000010001000010 mr0_data_1(RW) 0_000010001000010 mr0_data_0(RW) 
 DRAM_REG[182] =0x04420442 ; //0_000010001000010 mr0_data_3(RW) 0_000010001000010 mr0_data_2(RW) 
 DRAM_REG[183] =0x00040004 ; //0_000000000000100 mr1_data_1(RW) 0_000000000000100 mr1_data_0(RW) 
 DRAM_REG[184] =0x00040004 ; //0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 
 DRAM_REG[185] =0x00000000 ; //0_000000000000000 mr2_data_1(RW) 0_000000000000000 mr2_data_0(RW) 
 DRAM_REG[186] =0x00000000 ; //0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 
 DRAM_REG[187] =0x00000000 ; //0_000000000000000 mr3_data_1(RW) 0_000000000000000 mr3_data_0(RW) 
 DRAM_REG[188] =0x00000000 ; //0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 
 DRAM_REG[189] =0xffffffff ; //0000000000000001 axi5_en_size_lt_width_instr(RW) 0000000000000001 axi4_en_size_lt_width_instr(RW) 

} 

//------------------------------------------------------------------------------
//
//  Function: InitDebugSerial
//      This function is to initial debug serial port.
//
//------------------------------------------------------------------------------

void InitDebugSerial()
{
    UINT32 UartReadDummy;

    // Make sure all debug UART interrupts are off
    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_SFTRST | BM_PINCTRL_CTRL_CLKGATE);

    HW_UARTDBGIMSC_WR(0x0);

    // Configure the GPIO UART pins.
//#ifdef	EM9280
//    HW_PINCTRL_MUXSEL7_SET(0xF000F);        // Switch both (GPIO3_16, GPIO3_17) and ((GPIO3_24, GPIO3_25)) to GPIO
//    HW_PINCTRL_MUXSEL6_SET(0xF0);			// Switch (GPIO3_2, GPIO3_3) to GPIO
//    HW_PINCTRL_MUXSEL6_CLR(1 << 6);			// DBG-TX (bank 3 pin 3) muxmode=10
//    HW_PINCTRL_MUXSEL6_CLR(1 << 4);			// DBG-RX (bank 3 pin 2) muxmode=10
//#else	// -> iMX28EVK
    HW_PINCTRL_MUXSEL7_SET(0xF000F);        // Switch both pins to GPIO
    HW_PINCTRL_MUXSEL7_CLR(1 << 2);			// DBG-TX (bank 3 pin 17) muxmode=10
    HW_PINCTRL_MUXSEL7_CLR(1 << 0);			// DBG-RX (bank 3 pin 16) muxmode=10
//#endif	//EM9280

    // Set the Baud Rate
    HW_UARTDBGIBRD_WR((HW_UARTDBGIBRD_RD() & BM_UARTDBGIBRD_UNAVAILABLE) | GET_UARTDBG_BAUD_DIVINT(DEBUG_BAUD));
    HW_UARTDBGFBRD_WR((HW_UARTDBGFBRD_RD() & BM_UARTDBGFBRD_UNAVAILABLE) | GET_UARTDBG_BAUD_DIVFRAC(DEBUG_BAUD));

    // Setting UART properties to 8N1
    HW_UARTDBGLCR_H_WR(BF_UARTDBGLCR_H_SPS(0)   |
                       BF_UARTDBGLCR_H_WLEN(3)  |
                       BF_UARTDBGLCR_H_FEN(1)   |
                       BF_UARTDBGLCR_H_STP2(0)  |
                       BF_UARTDBGLCR_H_EPS(0)   |
                       BF_UARTDBGLCR_H_PEN(0)   |
                       BF_UARTDBGLCR_H_BRK(0));

    // Clear Tx/Rx FIFO
    for(; (HW_UARTDBGFR_RD() & BM_UARTDBGFR_RXFE) == 0; )
        UartReadDummy = HW_UARTDBGDR_RD();

    // Clear Receive Status
    HW_UARTDBGRSR_ECR_WR((HW_UARTDBGRSR_ECR_RD() & ~BM_UARTDBGRSR_ECR_EC) | \
                         BF_UARTDBGRSR_ECR_EC(0xF));

    HW_UARTDBGIFLS_WR(0x9);

    // Enable the UART.
    HW_UARTDBGCR_WR( BM_UARTDBGCR_UARTEN | BM_UARTDBGCR_RXE | BM_UARTDBGCR_TXE );


}
//------------------------------------------------------------------------------
//
//  Function: XLDRWriteDebugByte
//      Transmits a character out the debug serial port.
//
//------------------------------------------------------------------------------
void XLDRWriteDebugByte(UINT8 ch)
{
    UINT32 loop = 0;
    while ( (HW_UARTDBGFR.B.TXFF) && (loop < 0x7FFF))
        loop++;

    // Write a character byte to the FIFO.
    if(!(HW_UARTDBGFR.B.TXFF))
        BW_UARTDBGDR_DATA(ch);
}

//------------------------------------------------------------------------------
//
//  Function: InitPower
//      Initial power state.
//
//------------------------------------------------------------------------------
void InitPower()
{
    s_bBattery = 0;
    s_bUsb = 0;
    s_b5V = 0;

    // LCD power turn off.
#ifdef	EM9280
	// use GPIO1_0 as output for LCD_PWR
	HW_PINCTRL_MUXSEL2_SET(BM_PINCTRL_MUXSEL2_BANK1_PIN00);
    HW_PINCTRL_DOE1_SET(1 << 0);
    HW_PINCTRL_DOUT1_SET(1 << 0);		//active low
#else // -> iMX28EVK
    HW_PINCTRL_MUXSEL7_SET(BM_PINCTRL_MUXSEL7_BANK3_PIN30);
    HW_PINCTRL_DOE3_SET(1 << 30);
    HW_PINCTRL_DOUT3_CLR(1 << 30);		//active high
#endif	//EM9280

#ifdef	EM9280 
	// GPIO1_1 is used as USB0_PWR, turn off USB0 VBUS
	HW_PINCTRL_MUXSEL2_SET(BM_PINCTRL_MUXSEL2_BANK1_PIN01);
    HW_PINCTRL_DOE1_SET(1 << 1);
    HW_PINCTRL_DOUT1_CLR(1 << 1);		//active high
#endif	//EM9280

#ifdef BSP_5V_FROM_VBUS
    CPUClock2XTAL();
#endif

    HW_POWER_VDDACTRL_WR((HW_POWER_VDDACTRL_RD() & ~BM_POWER_VDDACTRL_LINREG_OFFSET) | BF_POWER_VDDACTRL_LINREG_OFFSET(0x1));
    HW_POWER_VDDIOCTRL_WR((HW_POWER_VDDIOCTRL_RD() & ~BM_POWER_VDDIOCTRL_LINREG_OFFSET) | BF_POWER_VDDIOCTRL_LINREG_OFFSET(0x1));
    HW_POWER_VDDDCTRL_WR((HW_POWER_VDDDCTRL_RD() & ~BM_POWER_VDDDCTRL_LINREG_OFFSET) | BF_POWER_VDDDCTRL_LINREG_OFFSET(0x1));

    HW_POWER_VDDACTRL_SET(BM_POWER_VDDACTRL_DISABLE_STEPPING); 
    HW_POWER_VDDDCTRL_SET(BM_POWER_VDDDCTRL_DISABLE_STEPPING);
    HW_POWER_VDDIOCTRL_SET(BM_POWER_VDDIOCTRL_DISABLE_STEPPING);
    
    // Restore VDDA 1.800 volt
    HW_POWER_VDDACTRL_WR(HW_POWER_VDDACTRL_RD() | BM_POWER_VDDACTRL_BO_OFFSET);
    HW_POWER_VDDACTRL_WR((HW_POWER_VDDACTRL_RD() & ~BM_POWER_VDDACTRL_TRG) | VDDAVolt2Reg(1800));

    // need to wait more than 10 microsecond before the DC_OK is valid
    XLDRStall(30);
    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    // Restore the VDDIO 3.3V
    HW_POWER_VDDIOCTRL_WR(HW_POWER_VDDIOCTRL_RD() | BM_POWER_VDDIOCTRL_BO_OFFSET);    
    //HW_POWER_VDDIOCTRL_WR((HW_POWER_VDDIOCTRL_RD() & ~BM_POWER_VDDIOCTRL_TRG) | VDDIOVolt2Reg(3300));
	// CS&ZHL JUN-15-2012: 3.3V setting -> 3.415V, so change to 3.2V setting -> 3.311V
    HW_POWER_VDDIOCTRL_WR((HW_POWER_VDDIOCTRL_RD() & ~BM_POWER_VDDIOCTRL_TRG) | VDDIOVolt2Reg(3200));

    // need to wait more than 10 microsecond before the DC_OK is valid
    XLDRStall(30);
    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    //Set VDDD to 1.30V
    HW_POWER_VDDDCTRL_WR(HW_POWER_VDDDCTRL_RD() | BM_POWER_VDDDCTRL_BO_OFFSET);
    HW_POWER_VDDDCTRL_WR((HW_POWER_VDDDCTRL_RD() & ~BM_POWER_VDDDCTRL_TRG) | VDDDVolt2Reg(1300));

    // need to wait more than 10 microsecond before the DC_OK is valid
    XLDRStall(30);

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK)); 
    
    HW_POWER_VDDIOCTRL_CLR(BM_POWER_VDDIOCTRL_DISABLE_STEPPING);
    HW_POWER_VDDDCTRL_CLR(BM_POWER_VDDDCTRL_DISABLE_STEPPING);
    HW_POWER_VDDACTRL_CLR(BM_POWER_VDDACTRL_DISABLE_STEPPING);

    // JLY05-2012:LQK
#ifndef EM9283
	// Enable auto restart.
	HW_RTC_PERSISTENT0_SET(BM_RTC_PERSISTENT0_AUTO_RESTART);
#endif // EM9283 

    HW_POWER_5VCTRL.B.HEADROOM_ADJ = 4;

    //Enable battery voltage auto update
    EnableBatteryMeasure();
    //Print battery's voltage
    XLDRWriteDebugByte('\r');
    XLDRWriteDebugByte('\n');
    XLDRWriteDebugByte('B');
    XLDRWriteDebugByte('A');
    XLDRWriteDebugByte('T');
    XLDRWriteDebugByte('T');
    XLDRWriteDebugByte(':');
    PrintBatteryVoltage(BattVoltage);
    XLDRWriteDebugByte('\r');
    XLDRWriteDebugByte('\n');

#ifdef	EM9280
	// no battery for power supply in EM9280
    s_bBattery = FALSE;
    PowerStopCharger();		//stop charger unconditionally
#else	// -> EM9283 or iMX28EVK

#ifdef	EM9283  //lqk 2012-5-30
	//lqk:Jul-25-2012
	HW_POWER_BATTMONITOR.B.BRWNOUT_LVL=0;	// Set battery Brownout threshold to 2400mv
	HW_POWER_BATTMONITOR.B.PWDN_BATTBRNOUT=0;
	HW_POWER_BATTMONITOR.B.BRWNOUT_PWD=0;
	//Detect whether there is a good battery
	if(IsBatteryGood())
	{
		XLDRWriteDebugByte('B');
		XLDRWriteDebugByte('\r');
		XLDRWriteDebugByte('\n');
		s_bBattery = TRUE;
	}

	// Clear auto restart for automatic battery brownout shutdown. 
	HW_RTC_PERSISTENT0_CLR(BM_RTC_PERSISTENT0_AUTO_RESTART);

	if( s_bBattery )
	{
		// Set battery Brownout threshold to 3000mv
		HW_POWER_BATTMONITOR.B.BRWNOUT_LVL=15;	
	}
	else
	{
		HW_POWER_BATTMONITOR.B.BRWNOUT_LVL=16;	
	}

	HW_POWER_BATTMONITOR.B.PWDN_BATTBRNOUT_5VDETECT_ENABLE=1;
	HW_POWER_BATTMONITOR.B.PWDN_BATTBRNOUT=1;
	HW_POWER_BATTMONITOR.B.BRWNOUT_PWD=0;
	BF_WR(POWER_RESET,UNLOCK,BV_POWER_RESET_UNLOCK__KEY);
	HW_POWER_RESET_WR((BV_POWER_RESET_UNLOCK__KEY << BP_POWER_RESET_UNLOCK) | BM_POWER_RESET_FASTFALLPSWITCH_OFF);

#else
    //Detect whether there is a good battery
    if(IsBatteryGood())
    {
        XLDRWriteDebugByte('B');
        XLDRWriteDebugByte('\r');
        XLDRWriteDebugByte('\n');
        s_bBattery = TRUE;
    }
#endif   // EM9283
#endif	//EM9280

    //Detect whether 5V is present
    if(Is5VPresent())
    {
        XLDRWriteDebugByte('5');      
        XLDRWriteDebugByte('\r');
        XLDRWriteDebugByte('\n'); 
        s_b5V = TRUE;
    }
    
    //Detect whether USB is plugin
    if(IsUSBPlugin())
    {
        XLDRWriteDebugByte('U');
        XLDRWriteDebugByte('\r');
        XLDRWriteDebugByte('\n');
#ifdef BSP_5V_FROM_VBUS
        BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x20);
#endif    
        s_bUsb = TRUE;
    }

// JLY05-2012: LQK
#ifdef EM9283	// lqk 2012-5-30
	if( s_b5V == TRUE )
	{
		BootFrom4P2();
	}
	else
	{
		//Boot from battery,and turn on PLL
		BootFromBattery();
		CPUClock2PLL();
	}
#else
    //If battery is good
    if(s_bBattery == TRUE)
    {
        //If battery's voltage is not enough,charge it
        if((BattVoltage < BATTERY_BOOT) && (s_b5V == TRUE))
        {
            BootFrom4P2();
            ChargeBattery2Boot(); //Charge battery until it reach 3.4V to boot up system
        }
        //Boot from battery,and turn on PLL
        BootFromBattery();
        CPUClock2PLL();
        
    }
    //Direct boot from USB,current limited to 100mA
    else if(s_b5V == TRUE)
        BootFrom4P2();
#endif     //EM9283
}

//-----------------------------------------------------------------------------
//
//  Function:  EnableBatteryMeasure
//
//  This function is used to enable auto update battery voltage.
//
//  Parameters:
//          
//  Returns:
//          TRUE if success,and FALSE if fail.
//
//-----------------------------------------------------------------------------
BOOL EnableBatteryMeasure()
{
    //Detect Battery voltage
    HW_LRADC_CTRL0_CLR(BM_LRADC_CTRL0_CLKGATE);    //gate
    HW_LRADC_CTRL0_CLR(BM_LRADC_CTRL0_SFTRST);    //reset

    // Disable the channel interrupt
    HW_LRADC_CTRL1_CLR((1 << (BATTERY_VOLTAGE_CH + BP_LRADC_CTRL1_LRADC0_IRQ_EN)));

    HW_LRADC_CTRL1_CLR(BM_LRADC_CTRL1_LRADC7_IRQ);
    // Configure the battery conversion register
    // Set the scale factor of battery A-to-D conversion
    
    BF_WR(LRADC_CONVERSION, SCALE_FACTOR, 2);
    
    // Enable the automatic update mode of BATT_VALUE field in HW_POWER_MONITOR register
    BF_SET(LRADC_CONVERSION, AUTOMATIC);
    
    // Disable the divide-by-two of a LRADC channel
    BF_CLRV(LRADC_CTRL2, DIVIDE_BY_TWO, (1 << BATTERY_VOLTAGE_CH));

    // Clear the accumulator & NUM_SAMPLES
    HW_LRADC_CHn_CLR(BATTERY_VOLTAGE_CH, 0xFFFFFFFF);

    // Sets NUM_SAMPLES bitfield of HW_LRADC_CHn register.
    BF_WRn(LRADC_CHn, BATTERY_VOLTAGE_CH, NUM_SAMPLES, (0 & 0x1f));

    // Set ACCUMULATE bit of HW_LRADC_CHn register

    // Disable the accumulation of a LRADC channel
    BF_CLRn(LRADC_CHn, BATTERY_VOLTAGE_CH, ACCUMULATE);

    // schedule a conversion before the setting up of the delay channel
    // so the user can have a good value right after the function returns
    
    BF_SETV(LRADC_CTRL0, SCHEDULE, (1 << BATTERY_VOLTAGE_CH));
    
    //Set the TRIGGER_LRADCS bitfield of HW_LRADC_DELAYn register
    BF_SETVn(LRADC_DELAYn, LRADC_DELAY_TRIGGER3, TRIGGER_LRADCS,  1 << BATTERY_VOLTAGE_CH);

    //Set the TRIGGER_DELAYS bitfield of HW_LRADC_DELAYn register
    BF_SETVn(LRADC_DELAYn, LRADC_DELAY_TRIGGER3, TRIGGER_DELAYS,  1 << LRADC_DELAY_TRIGGER3);

    //Write the LOOP_COUNT bitfield of HW_LRADC_DELAYn register
    BF_WRn(LRADC_DELAYn, LRADC_DELAY_TRIGGER3, LOOP_COUNT,  0);

    //Write the DEALY bitfield of HW_LRADC_DELAYn register
    BF_WRn(LRADC_DELAYn, LRADC_DELAY_TRIGGER3, DELAY,  200);
    // Clear the accumulator & NUM_SAMPLES
    HW_LRADC_CHn_CLR(BATTERY_VOLTAGE_CH, 0xFFFFFFFF);
    
    // Kick off the LRADC battery measurement
    //Set the TRIGGER_LRADCS bitfield of HW_LRADC_DELAYn register
    BF_SETn(LRADC_DELAYn, LRADC_DELAY_TRIGGER3, KICK);

    // Wait for first measurement of battery.  Should occur in 13 LRADC clock
    // cycles from the time of channel kickoff.
    while(!BF_RD(LRADC_CTRL1, LRADC7_IRQ)) ;
    
    BattVoltage = HW_POWER_BATTMONITOR.B.BATT_VAL * 8;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  IsBatteryGood
//
//  This function is used to detect whether there is a battery.
//
//  Parameters:
//          
//  Returns:
//          TRUE if success,and FALSE if fail.
//
//-----------------------------------------------------------------------------
BOOL IsBatteryGood()
{
    
#ifdef EM9283
	int i;
#endif
	UINT32 BatteryVoltage = 0;
    BatteryVoltage = HW_POWER_BATTMONITOR.B.BATT_VAL * 8;
    if((BatteryVoltage > BATTERY_LOW) && (BatteryVoltage < BATTERY_HIGH))
	{
        //return TRUE;
		// JLY05-2012:LQK
#ifdef EM9283
		//LQK:Jul-23-2012
		for( i=0; i<10; i++ )
		{
			HW_POWER_REFCTRL.B.FASTSETTLING =1;
			XLDRStall(10000);
			if( HW_POWER_STS.B.BATT_BO == 1 )
				break;
		}

		HW_POWER_REFCTRL.B.FASTSETTLING=0;
		
		if( i<10 )
			return FALSE;
		else
		{
			return TRUE;
		}

		/*HW_POWER_REFCTRL.B.FASTSETTLING =1;
		if( HW_POWER_STS.B.BATT_BO == 0 )
		{
			HW_POWER_REFCTRL.B.FASTSETTLING =1;
			XLDRStall(10000);
			BattVoltage = HW_POWER_BATTMONITOR.B.BATT_VAL * 8;
			XLDRWriteDebugByte('B');
			XLDRWriteDebugByte('A');
			XLDRWriteDebugByte('T');
			XLDRWriteDebugByte('T');
			XLDRWriteDebugByte(':');
			PrintBatteryVoltage(BattVoltage);
			XLDRWriteDebugByte('\r');
			XLDRWriteDebugByte('\n');
			if( HW_POWER_STS.B.BATT_BO == 0 )
			{
				HW_POWER_REFCTRL.B.FASTSETTLING =0;
				return TRUE;
			}
		}
		HW_POWER_REFCTRL.B.FASTSETTLING =0;
		return FALSE;*/
#else
		return TRUE;
#endif
	}
    else
    {
        BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x3);
        HW_POWER_5VCTRL_CLR(BM_POWER_5VCTRL_PWD_CHARGE_4P2);  
        PowerSetCharger(0x3);
        XLDRStall(500000);
        BatteryVoltage = HW_POWER_BATTMONITOR.B.BATT_VAL * 8;
        PowerStopCharger();
        if(BatteryVoltage > BATTERY_BAD)
        {
            // Disable the automatic update mode of BATT_VALUE field in HW_POWER_MONITOR register
            BF_CLR(LRADC_CONVERSION, AUTOMATIC);
            BF_WR(POWER_BATTMONITOR,BATT_VAL,0);
            return FALSE;
        }
        else if(BatteryVoltage > BATTERY_LOW)
            return TRUE;
    }
    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function:  IsUSBPlugin
//
//  This function is used to detect whether USB cable is plugin.
//
//  Parameters:
//          
//  Returns:
//          TRUE if success,and FALSE if fail.
//
//-----------------------------------------------------------------------------
BOOL IsUSBPlugin()
{
    BOOL ret = FALSE;
    //Detect USB cable status
    if ((HW_POWER_5VCTRL_RD() & BM_POWER_5VCTRL_PWRUP_VBUS_CMPS) == 0)
        HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_PWRUP_VBUS_CMPS);
    if ((HW_USBPHY_CTRL_RD(0) & BM_USBPHY_CTRL_CLKGATE) == 1)
        HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_CLKGATE);
    if ((HW_USBPHY_CTRL_RD(0) & BM_USBPHY_CTRL_SFTRST) == 1)
        HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_SFTRST);
    HW_USBPHY_CTRL_SET(0,BM_USBPHY_CTRL_ENDEVPLUGINDETECT);
    XLDRStall(10000);
    if((HW_USBPHY_STATUS_RD(0) & BM_USBPHY_STATUS_DEVPLUGIN_STATUS)) 
        ret = TRUE;
    HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_ENDEVPLUGINDETECT);
    return ret;
}

//-----------------------------------------------------------------------------
//
//  Function:  Is5VPresent
//
//  This function is used to detect whether 5V is present.
//
//  Parameters:
//          
//  Returns:
//          TRUE if success,and FALSE if fail.
//
//-----------------------------------------------------------------------------
BOOL Is5VPresent()
{
    BF_SET(POWER_5VCTRL, VBUSVALID_5VDETECT);
    BF_WR(POWER_5VCTRL, VBUSVALID_TRSH, 1);//4.0V
    //Enable the 5V detect
    BF_SET(POWER_5VCTRL, VBUSVALID_5VDETECT);
    XLDRStall(100);
    return HW_POWER_STS.B.VBUSVALID0;
}
//-----------------------------------------------------------------------------
//
//  Function:  BootFromBattery
//
//  This function is used to set the PMU to boot from battery.
//
//  Parameters:
//          
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void BootFromBattery()
{
    //Disable DCDC
    HW_POWER_5VCTRL_CLR(BM_POWER_5VCTRL_ENABLE_DCDC);

    //Stop DCDC4P2
    BF_CLR(POWER_DCDC4P2, ENABLE_DCDC);       // Enable the DCDC.
    BF_CLR(POWER_DCDC4P2, ENABLE_4P2);      //enable DCDC 4P2 regulation circuitry
    BF_CLR(POWER_CHARGE,ENABLE_LOAD);

    BF_SET(POWER_5VCTRL, DCDC_XFER);
    XLDRStall(100);
    BF_CLR(POWER_5VCTRL, DCDC_XFER);

    //BF_SET(POWER_5VCTRL, PWRUP_VBUS_CMPS);
    BF_CLR(POWER_CTRL,ENIRQ_DCDC4P2_BO);
    BF_SET(POWER_5VCTRL, PWD_CHARGE_4P2);

    // Disable hardware power down when 5V is inserted or removed
    BF_CLR(POWER_5VCTRL, PWDN_5VBRNOUT);

    //Enable double FETs.
    HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_HALF_FETS);
    HW_POWER_MINPWR_SET(BM_POWER_MINPWR_DOUBLE_FETS);
    //set linger step below 25mV
    BF_WR(POWER_VDDIOCTRL, LINREG_OFFSET, 0x2);
    BF_WR(POWER_VDDACTRL, LINREG_OFFSET, 0x2);
    BF_WR(POWER_VDDDCTRL, LINREG_OFFSET, 0x2);

    HW_POWER_VDDDCTRL_CLR(BM_POWER_VDDDCTRL_DISABLE_FET);
    //Disable 4P2
    HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_PWD_CHARGE_4P2);
    //Enable DCDC
    HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_ENABLE_DCDC);
    //Enable BATADJ.
    HW_POWER_BATTMONITOR_SET(BM_POWER_BATTMONITOR_EN_BATADJ);    
    //Minimized the current consumption from 5V.    
    HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_ILIMIT_EQ_ZERO);  
}
//-----------------------------------------------------------------------------
//
//  Function:  BootFrom4P2
//
//  This function is used to set the PMU to boot from USB 5V.
//
//  Parameters:
//          
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void BootFrom4P2()
{
    // Disable FET and enable LINREG
    HW_POWER_VDDDCTRL_SET(BM_POWER_VDDDCTRL_DISABLE_FET | BM_POWER_VDDDCTRL_ENABLE_LINREG);
    HW_POWER_VDDACTRL_SET(BM_POWER_VDDACTRL_DISABLE_FET | BM_POWER_VDDACTRL_ENABLE_LINREG);
    HW_POWER_VDDIOCTRL_SET(BM_POWER_VDDIOCTRL_DISABLE_FET);

    // Config DCDC4P2 parameters.
    //HW_POWER_DCDC4P2.B.CMPTRIP = 31;
    HW_POWER_DCDC4P2.B.TRG = 0;
    HW_POWER_DCDC4P2.B.DROPOUT_CTRL = 0xA;
    
    BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x3F);

    //Enable 5V to 4p2 regulator
    BF_SET(POWER_DCDC4P2, ENABLE_4P2);      
    
    // This must be used if there is no static load on 4p2 as 4p2 will
    // become unstable with no load.
    BF_SET(POWER_CHARGE,ENABLE_LOAD);
    
    BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0);
    
    BF_CLR(POWER_5VCTRL, PWD_CHARGE_4P2);

    // Temporarily turn off the PWDN_5VBRNOUT,due to the errata.
    BF_CLR(POWER_5VCTRL, PWDN_5VBRNOUT);

    BF_SET(POWER_DCDC4P2, ENABLE_DCDC);//enable DCDC 4P2 capability
    XLDRStall(100);

    BF_SETV(POWER_DCDC4P2, BO, 0x16);//set the DCDC BO to 4.15V

    BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x3F);
#ifdef BSP_5V_FROM_VBUS
        // Set limit current to enable USB VBUS current limit
        // here we are not using the hardware current limit,but only
        // set up a flag and lower the power consumption as more as 
        // possiable.Using 400mA here.
        BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x20);
        //HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_ILIMIT_EQ_ZERO);
#endif
	//Lqk:Jul-16-2012
#ifdef EM9283
	// Always set current limit to 480mA at power up.    
	BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x24);
#endif

    //while(HW_POWER_STS.B.DCDC_4P2_BO)
    XLDRStall(50);//wait the DCDC to be stable

    BF_SETV(POWER_DCDC4P2, BO, 0);//set the DCDC BO to 3.6V
    HW_POWER_CTRL_CLR(BM_POWER_CTRL_DCDC4P2_BO_IRQ);

    // Enables DCDC during 5V connnection (for both battery or 4p2 powered DCDC)
    BF_SET(POWER_5VCTRL, ENABLE_DCDC);
    XLDRStall(100);

    BF_WR(POWER_VDDIOCTRL, LINREG_OFFSET, 0x2);
    BF_WR(POWER_VDDDCTRL, LINREG_OFFSET, 0x2);
    BF_WR(POWER_VDDACTRL, LINREG_OFFSET, 0x2);
    
    HW_POWER_VDDDCTRL_CLR(BM_POWER_VDDDCTRL_DISABLE_FET | BM_POWER_VDDDCTRL_ENABLE_LINREG);
    HW_POWER_VDDACTRL_CLR(BM_POWER_VDDACTRL_DISABLE_FET | BM_POWER_VDDACTRL_ENABLE_LINREG);
    HW_POWER_VDDIOCTRL_CLR(BM_POWER_VDDIOCTRL_DISABLE_FET);
    // Now we can disable the load.
    BF_CLR(POWER_CHARGE,ENABLE_LOAD);
    
    BF_SET(POWER_5VCTRL, PWDN_5VBRNOUT);
    
}
//-----------------------------------------------------------------------------
//
//  Function:  ChargeBattery2Boot
//
//  This function is used to charge the battery until it has enough power to boot the system..
//
//  Parameters:
//          
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void ChargeBattery2Boot()
{
    UINT32 i = 0,j = 0;
    //Print the info
    XLDRWriteDebugByte('C');
    XLDRWriteDebugByte('h');
    XLDRWriteDebugByte('a');
    XLDRWriteDebugByte('r');
    XLDRWriteDebugByte('g');
    XLDRWriteDebugByte('i');
    XLDRWriteDebugByte('n');
    XLDRWriteDebugByte('g');
    XLDRWriteDebugByte('.');
    XLDRWriteDebugByte('.');
    XLDRWriteDebugByte('.');
    XLDRWriteDebugByte('.');
    XLDRWriteDebugByte('.');
    XLDRWriteDebugByte('(');
    XLDRWriteDebugByte('3');
    XLDRWriteDebugByte('.');
#ifdef EM9283                   //JLY05-2012: LQK
    XLDRWriteDebugByte('2');
	XLDRWriteDebugByte('5');
#else
	XLDRWriteDebugByte('6');
#endif
    XLDRWriteDebugByte('V');
    XLDRWriteDebugByte(')');
    XLDRWriteDebugByte('\r');
    XLDRWriteDebugByte('\n');
    //Cheak the battery  voltage twice.
    while(i < 2)
    {
        BattVoltage = 0;
        while(BattVoltage < BATTERY_BOOT)
        {
            PowerStopCharger();
            XLDRStall(500000);
            BattVoltage = HW_POWER_BATTMONITOR.B.BATT_VAL * BATT_VOLTAGE_8_MV;
            XLDRWriteDebugByte('\r');
            XLDRWriteDebugByte('B');
            XLDRWriteDebugByte('A');
            XLDRWriteDebugByte('T');
            XLDRWriteDebugByte('T');
            XLDRWriteDebugByte(':');
            PrintBatteryVoltage(BattVoltage);
            XLDRWriteDebugByte(' ');
            XLDRWriteDebugByte(' ');
            XLDRWriteDebugByte(' ');            
#ifdef BSP_5V_FROM_VBUS
            PowerSetCharger(0x4);//50mA if it is VBUS 5V
#else
            PowerSetCharger(0xF);//180mA if it is Wall 5V
#endif
            //Print rotary cursor,wait for 20 seconds
            j = 0;
            while(j++ < 25)
            {
                XLDRWriteDebugByte('-');
                XLDRWriteDebugByte('\b');
                XLDRStall(200000);
                XLDRWriteDebugByte('\\');
                XLDRWriteDebugByte('\b');
                XLDRStall(200000);
                XLDRWriteDebugByte('|');
                XLDRWriteDebugByte('\b');
                XLDRStall(200000);
                XLDRWriteDebugByte('/');
                XLDRWriteDebugByte('\b');
                XLDRStall(200000); 
            }
        }
        i++;
    }
}

//------------------------------------------------------------------------------
//
//  Function: PrintHex
//
//  Print function
//
//------------------------------------------------------------------------------
void PrintHex(UINT32 value)
{
    XLDRWriteDebugByte('0');
    XLDRWriteDebugByte('x');

    if((UINT8)((value & 0xf0000000) >> 28) <= 9)
        XLDRWriteDebugByte((UINT8)((value & 0xf0000000) >> 28) + 0x30);
    else
        XLDRWriteDebugByte((UINT8)((value & 0xf0000000) >> 28) + 0x57);
    if((UINT8)((value & 0xf000000) >> 24) <= 9)
        XLDRWriteDebugByte((UINT8)((value & 0xf000000) >> 24) + 0x30);
    else
        XLDRWriteDebugByte((UINT8)((value & 0xf000000) >> 24) + 0x57);
    if((UINT8)((value & 0xf00000) >> 20) <= 9)
        XLDRWriteDebugByte((UINT8)((value & 0xf00000) >> 20) + 0x30);
    else
        XLDRWriteDebugByte((UINT8)((value & 0xf00000) >> 20) + 0x57);
    if((UINT8)((value & 0xf0000) >> 16) <= 9)
        XLDRWriteDebugByte((UINT8)((value & 0xf0000) >> 16) + 0x30);
    else
        XLDRWriteDebugByte((UINT8)((value & 0xf0000) >> 16) + 0x57);
    if((UINT8)((value & 0xf000) >> 12) <= 9)
        XLDRWriteDebugByte((UINT8)((value & 0xf000) >> 12) + 0x30);
    else
        XLDRWriteDebugByte((UINT8)((value & 0xf000) >> 12) + 0x57);
    if((UINT8)((value & 0xf00) >> 8) <= 9)
        XLDRWriteDebugByte((UINT8)((value & 0xf00) >> 8) + 0x30);
    else
        XLDRWriteDebugByte((UINT8)((value & 0xf00) >> 8) + 0x57);
    if((UINT8)((value & 0xf0) >> 4) <= 9)
        XLDRWriteDebugByte((UINT8)((value & 0xf0) >> 4) + 0x30);
    else
        XLDRWriteDebugByte((UINT8)((value & 0xf0) >> 4) + 0x57);    
    if((UINT8)((value & 0xf)) <= 9)
        XLDRWriteDebugByte((UINT8)((value & 0xf)) + 0x30);
    else
        XLDRWriteDebugByte((UINT8)((value & 0xf)) + 0x57);    

    XLDRWriteDebugByte('\r');
    XLDRWriteDebugByte('\n');

}


//------------------------------------------------------------------------------
//
//  Function: PrintBatteryVoltage
//
//  Print function
//
//------------------------------------------------------------------------------
void PrintBatteryVoltage(UINT32 value)
{
    UINT32 num[3] = {0,0,0};
    UINT32 i = 0;
    
    num[0] = value / 1000;
    num[1] = value % 1000 / 100;
    num[2] = value % 100 / 10;

    for(i = 0;i < 3;i++)
    {
        if(i == 1)
            XLDRWriteDebugByte('.');
        switch(num[i])  
        {
            case 0:
                XLDRWriteDebugByte('0');   
                break;
            case 1:
                XLDRWriteDebugByte('1');   
                break;
            case 2:
                XLDRWriteDebugByte('2');   
                break;   
            case 3:
                XLDRWriteDebugByte('3');   
                break;
            case 4:
                XLDRWriteDebugByte('4');   
                break;
            case 5:
                XLDRWriteDebugByte('5');   
                break; 
            case 6:
                XLDRWriteDebugByte('6');   
                break;
            case 7:
                XLDRWriteDebugByte('7');   
                break;
            case 8:
                XLDRWriteDebugByte('8');   
                break;   
            case 9:
                XLDRWriteDebugByte('9');   
                break;
            default:   
                break;
        }
    }
    XLDRWriteDebugByte('V');
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerSetCharger
//
//  This function is used to set the charger
//
//  Parameters:
//          current
//              [in] The current value of charger
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void PowerSetCharger(UINT32 current)
{
    BF_CLRV(POWER_CHARGE, STOP_ILIMIT, 0xF);
    BF_SETV(POWER_CHARGE, STOP_ILIMIT, 0x1);//stop limit current = 30mA  
    BF_CLRV(POWER_CHARGE, BATTCHRG_I, 0x3F);
    BF_SETV(POWER_CHARGE, BATTCHRG_I, current); 
    BF_CLR(POWER_CHARGE, PWD_BATTCHRG);
    BF_CLR(POWER_5VCTRL, PWD_CHARGE_4P2); 
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerStopCharger
//
//  This function is used to stop the charger
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void PowerStopCharger()
{
    BF_CLRV(POWER_CHARGE, STOP_ILIMIT, 0xF);
    BF_CLRV(POWER_CHARGE, BATTCHRG_I, 0x3F);
    BF_SET(POWER_CHARGE, PWD_BATTCHRG);  
}

//-----------------------------------------------------------------------------
//
//  Function:  XLDRStall
//
//  This function is used to delay micro seconds.
//
//  Parameters:
//          microSec,time to delay.
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void XLDRStall(UINT32 microSec)
{
    UINT32 currentTime = HW_DIGCTL_MICROSECONDS_RD();
    // Check if we wrapped on the expireTime
    while ((HW_DIGCTL_MICROSECONDS_RD() - currentTime) <  microSec);
}

//-----------------------------------------------------------------------------
//
//  Function:  CPUClock2XTAL
//
//  This function is to set CPU clock to use XTAL.
//
//  Parameters:
//          None.
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void CPUClock2XTAL()
{
    // let CPU sink the xtal clock
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU); 
}
//-----------------------------------------------------------------------------
//
//  Function:  CPUClock2PLL
//
//  This function is to set CPU clock to use PLL.
//
//  Parameters:
//          None.
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void CPUClock2PLL()
{
    DWORD dwRegFrac = 0;
    if((HW_CLKCTRL_PLL0CTRL0_RD() & BM_CLKCTRL_PLL0CTRL0_POWER) != BM_CLKCTRL_PLL0CTRL0_POWER)
        HW_CLKCTRL_PLL0CTRL0_SET(BM_CLKCTRL_PLL0CTRL0_POWER); // Turn on PLL
    XLDRStall(100);
    while((HW_CLKCTRL_PLL0CTRL1_RD() & BM_CLKCTRL_PLL0CTRL1_LOCK) == 0);
    // set ref.cpu 454MHZ
    HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATECPU);

    HW_CLKCTRL_FRAC0_WR((HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_CPUFRAC) | \
                        BF_CLKCTRL_FRAC0_CPUFRAC(18));		//18? Is it right?!

    dwRegFrac = HW_CLKCTRL_FRAC0_RD() & BM_CLKCTRL_FRAC0_CPU_STABLE;
    for(; (HW_CLKCTRL_FRAC0_RD() ^ dwRegFrac) == 0; ) ;

    // Config CLK_CPU driver for 240 MHz.
    HW_CLKCTRL_CPU_WR((BF_CLKCTRL_CPU_DIV_CPU(2)             | \
                       BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0)     | \
                       BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)      | \
                       BF_CLKCTRL_CPU_DIV_XTAL(1)            | \
                       BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0)));

    // Config CLK_HBUS as CPU/3 (80MHz)
    HW_CLKCTRL_HBUS_WR((BF_CLKCTRL_HBUS_DIV(3)                    | \
                        BF_CLKCTRL_HBUS_DIV_FRAC_EN(0)            | \
                        BF_CLKCTRL_HBUS_SLOW_DIV(5)               | \
                        BF_CLKCTRL_HBUS_ASM_ENABLE(1)             | \
                        BF_CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE(1)    | \
                        BF_CLKCTRL_HBUS_CPU_DATA_AS_ENABLE(1)     | \
                        BF_CLKCTRL_HBUS_TRAFFIC_AS_ENABLE(1)      | \
                        BF_CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE(1)  | \
                        BF_CLKCTRL_HBUS_APBXDMA_AS_ENABLE(1)      | \
                        BF_CLKCTRL_HBUS_APBHDMA_AS_ENABLE(1)      | \
                        BF_CLKCTRL_HBUS_ASM_EMIPORT_AS_ENABLE(1)  | \
                        BF_CLKCTRL_HBUS_PXP_AS_ENABLE(1)          | \
                        BF_CLKCTRL_HBUS_DCP_AS_ENABLE(1)));
    
    if((HW_CLKCTRL_CLKSEQ_RD() & BM_CLKCTRL_CLKSEQ_BYPASS_CPU) == BM_CLKCTRL_CLKSEQ_BYPASS_CPU)
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);// let CPU sink the xtal clock
}


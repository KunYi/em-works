//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#ifndef _SGTL5000_REGS_H_
#define _SGTL5000_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define REG_VDDA_VOLTAGE                    3.3

//------------------------------------------------------------------------------
// REGISTER I2C ADDRESSES
//------------------------------------------------------------------------------

#define SGTL5000_CHIP_ID_ADDR                       0x0000
#define SGTL5000_CHIP_DIG_POWER_ADDR                0x0002
#define SGTL5000_CHIP_CLK_CTRL_ADDR                 0x0004
#define SGTL5000_CHIP_I2S_CTRL_ADDR                 0x0006
#define SGTL5000_CHIP_SSS_CTRL_ADDR                 0x000A
#define SGTL5000_CHIP_ADCDAC_CTRL_ADDR              0x000E
#define SGTL5000_CHIP_DAC_VOL_ADDR                  0x0010
#define SGTL5000_CHIP_PAD_STRENGTH_ADDR             0x0014
#define SGTL5000_CHIP_ANA_ADC_CTRL_ADDR             0x0020
#define SGTL5000_CHIP_ANA_HP_CTRL_ADDR              0x0022
#define SGTL5000_CHIP_ANA_CTRL_ADDR                 0x0024
#define SGTL5000_CHIP_LINREG_CTRL_ADDR              0x0026
#define SGTL5000_CHIP_REF_CTRL_ADDR                 0x0028
#define SGTL5000_CHIP_MIC_CTRL_ADDR                 0x002A
#define SGTL5000_CHIP_LINE_OUT_CTRL_ADDR            0x002C
#define SGTL5000_CHIP_LINE_OUT_VOL_ADDR             0x002E
#define SGTL5000_CHIP_ANA_POWER_ADDR                0x0030
#define SGTL5000_CHIP_PLL_CTRL_ADDR                 0x0032
#define SGTL5000_CHIP_CLK_TOP_CTRL_ADDR             0x0034
#define SGTL5000_CHIP_ANA_STATUS_ADDR               0x0036
#define SGTL5000_CHIP_SHORT_CTRL_ADDR               0x003C
#define SGTL5000_DAP_CONTROL_ADDR                   0x0100
#define SGTL5000_DAP_PEQ_ADDR                       0x0102
#define SGTL5000_DAP_BASS_ENHANCE_ADDR              0x0104
#define SGTL5000_DAP_BASS_ENHANCE_CTRL_ADDR         0x0106
#define SGTL5000_DAP_AUDIO_EQ_ADDR                  0x0108  
#define SGTL5000_DAP_SGTL_SURROUND_ADDR             0x010A
#define SGTL5000_DAP_FILTER_COEF_ACCESS_ADDR        0x010C
#define SGTL5000_DAP_COEF_WR_B0_MSB_ADDR            0x010E
#define SGTL5000_DAP_COEF_WR_B0_LSB_ADDR            0x0110
#define SGTL5000_DAP_AUDIO_EQ_BASS_BAND0_ADDR       0x0116  
#define SGTL5000_DAP_AUDIO_EQ_BAND1_ADDR            0x0118
#define SGTL5000_DAP_AUDIO_EQ_BAND2_ADDR            0x011A
#define SGTL5000_DAP_AUDIO_EQ_BAND3_ADDR            0x011C
#define SGTL5000_DAP_AUDIO_EQ_TREBLE_BAND4_ADDR     0x011E
#define SGTL5000_DAP_MAIN_CHAN_ADDR                 0x0120
#define SGTL5000_DAP_MIX_CHAN_ADDR                  0x0122
#define SGTL5000_DAP_AVC_CTRL_ADDR                  0x0124
#define SGTL5000_DAP_AVC_THRESHOLD_ADDR             0x0126
#define SGTL5000_DAP_AVC_ATTACK_ADDR                0x0128
#define SGTL5000_DAP_AVC_DECAY_ADDR                 0x012A
#define SGTL5000_DAP_COEF_WR_B1_MSB_ADDR            0x012C
#define SGTL5000_DAP_COEF_WR_B1_LSB_ADDR            0x012E
#define SGTL5000_DAP_COEF_WR_B2_MSB_ADDR            0x0130
#define SGTL5000_DAP_COEF_WR_B2_LSB_ADDR            0x0132
#define SGTL5000_DAP_COEF_WR_A1_MSB_ADDR            0x0134
#define SGTL5000_DAP_COEF_WR_A1_LSB_ADDR            0x0136
#define SGTL5000_DAP_COEF_WR_A2_MSB_ADDR            0x0138
#define SGTL5000_DAP_COEF_WR_A2_LSB_ADDR            0x013A

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
//CHIP_ANA_POWER
#define RIGHT_DAC_POWERUP_LSH                   14
#define LINREG_SIMPLE_POWERUP_LSH               13
#define STARTUP_POWERUP_LSH                     12
#define VDDC_CHRGPMP_POWERUP_LSH                11
#define PLL_POWERUP_LSH                         10
#define LINREG_D_POWERUP_LSH                    9
#define VCOAMP_POWERUP_LSH                      8
#define VAG_POWERUP_LSH                         7
#define RIGHT_ADC_POWERUP_LSH                   6
#define REFTOP_POWERUP_LSH                      5
#define HEADPHONE_POWERUP_LSH                   4
#define DAC_POWERUP_LSH                         3
#define CAPLESS_HP_POWERUP_LSH                  2
#define ADC_POWERUP_LSH                         1
#define LINEOUT_POWERUP_LSH                     0

//CHIP_REF_CTRL
#define VAG_VAL_LSH                             4
#define BIAS_CTRL_LSH                           1
#define SMALL_POP_LSH                           0

//CHIP_SHORT_CTRL
#define LVLADJR_LSH                             12
#define LVLADJL_LSH                             8
#define LVLADJC_LSH                             4
#define MODE_LR_LSH                             2
#define MODE_CM_LSH                             0

//CHIP_LINE_OUT_CTRL
#define OUT_CURRENT_LSH                         8
#define LO_VAGCNTRL_LSH                         0

//CHIP_ANA_CTRL
#define MUTE_LO_LSH                             8
#define SELECT_HP_LSH                           6   
#define EN_ZCD_HP_LSH                           5   
#define MUTE_HP_LSH                             4
#define SELECT_ADC_LSH                          2   
#define EN_ZCD_ADC_LSH                          1   
#define MUTE_ADC_LSH                            0

//CHIP_PLL_CTRL
#define INT_DIVISOR_LSH                         11
#define FRAC_DIVISOR_LSH                        0

//CHIP_CLK_CTRL
#define SYS_FS_LSH                              2
#define MCLK_FREQ_LSH                           0

//CHIP_LINE_OUT_VOL
#define LO_VOL_RIGHT_LSH                        8
#define LO_VOL_LEFT_LSH                         0

//CHIP_DIG_POWER
#define ADC_DIG_POWERUP_LSH                     6
#define DAC_DIG_POWERUP_LSH                     5
#define DAP_DIG_POWERUP_LSH                     4
#define I2S_OUT_POWERUP_LSH                     1
#define I2S_IN_POWERUP_LSH                      0

//CHIP_MIC_CTRL
#define MIC_BIAS_RESISTOR_LSH                   8
#define MIC_BIAS_VOLTAGE_LSH                    4
#define MIC_GAIN_LSH                            0

//CHIP_SSS_CTRL
#define DAP_MIX_LRSWAP_LSH                      14
#define DAP_LRSWAP_LSH                          13
#define DAC_LRSWAP_LSH                          12
#define I2S_LRSWAP_LSH                          10
#define DAP_MIX_SELECT_LSH                      8
#define DAP_SELECT_LSH                          6
#define DAC_SELECT_LSH                          4
#define I2S_SELECT_LSH                          0

//DAP_CONTROL
#define MIX_EN_LSH                              4
#define DAP_EN_LSH                              0

//CHIP_DAC_VOL
#define DAC_VOL_RIGHT_LSH                       8
#define DAC_VOL_LEFT_LSH                        0

//CHIP_PAD_STRENGTH
#define I2S_LRCLK_LSH                           8
#define I2S_SCLK_LSH                            6
#define I2S_DOUT_LSH                            4
#define CTRL_DATA_LSH                           2
#define CTRL_CLK_LSH                            0

//CHIP_I2S_CTRL
#define SCLKFREQ_LSH                            8
#define MS_LSH                                  7
#define SCLK_INV_LSH                            6
#define DLEN_LSH                                4
#define I2S_MODE_LSH                            2
#define LRALIGN_LSH                             1
#define LRPOL_LSH                               0

//CHIP_ADCDAC_CTRL
#define VOL_BUSY_DAC_RIGHT_LSH                  13
#define VOL_BUSY_DAC_LEFT_LSH                   12
#define VOL_RAMP_EN_LSH                         9
#define VOL_EXPO_RAMP_LSH                       8
#define DAC_MUTE_RIGHT_LSH                      3
#define DAC_MUTE_LEFT_LSH                       2
#define ADC_HPF_FREEZE_LSH                      1
#define ADC_HPF_BYPASS_LSH                      0

//CHIP_ANA_ADC_CTRL
#define ADC_VOL_M6DB_LSH                        8
#define ADC_VOL_RIGHT_LSH                       4
#define ADC_VOL_LEFT_LSH                        0

//CHIP_CLK_TOP_CTRL
#define ENABLE_INT_OSC_LSH                      11
//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
    //CHIP_ANA_POWER
#define RIGHT_DAC_POWERUP_WID                   1
#define LINREG_SIMPLE_POWERUP_WID               1
#define STARTUP_POWERUP_WID                     1
#define VDDC_CHRGPMP_POWERUP_WID                1
#define PLL_POWERUP_WID                         1
#define LINREG_D_POWERUP_WID                    1
#define VCOAMP_POWERUP_WID                      1
#define VAG_POWERUP_WID                         1
#define RIGHT_ADC_POWERUP_WID                   1
#define REFTOP_POWERUP_WID                      1
#define HEADPHONE_POWERUP_WID                   1
#define DAC_POWERUP_WID                         1
#define CAPLESS_HP_POWERUP_WID                  1
#define ADC_POWERUP_WID                         1
#define LINEOUT_POWERUP_WID                     1

//CHIP_REF_CTRL
#define VAG_VAL_WID                             5
#define BIAS_CTRL_WID                           3
#define SMALL_POP_WID                           1

//CHIP_SHORT_CTRL
#define LVLADJR_WID                             3
#define LVLADJL_WID                             3
#define LVLADJC_WID                             3
#define MODE_LR_WID                             2
#define MODE_CM_WID                             2

//CHIP_LINE_OUT_CTRL
#define OUT_CURRENT_WID                         4
#define LO_VAGCNTRL_WID                         6

//CHIP_ANA_CTRL
#define MUTE_LO_WID                             1
#define SELECT_HP_WID                           1
#define EN_ZCD_HP_WID                           1
#define MUTE_HP_WID                             1
#define SELECT_ADC_WID                          1   
#define EN_ZCD_ADC_WID                          1   
#define MUTE_ADC_WID                            1

//CHIP_PLL_CTRL
#define INT_DIVISOR_WID                         5
#define FRAC_DIVISOR_WID                        11

//CHIP_CLK_CTRL
#define SYS_FS_WID                              2
#define MCLK_FREQ_WID                           2

//CHIP_LINE_OUT_VOL
#define LO_VOL_RIGHT_WID                        5
#define LO_VOL_LEFT_WID                         5

//CHIP_ANA_POWER
#define DAC_MONO_WID                            1
#define VAG_POWERUP_WID                         1
#define ADC_MONO_WID                            1
#define REFTOP_POWERUP_WID                      1
#define HP_POWERUP_WID                          1
#define CAPLESS_HP_POERUP_WID                   1
#define ADC_POWERUP_WID                         1
#define LINEOUT_POWERUP_WID                     1

//CHIP_DIG_POWER
#define ADC_DIG_POWERUP_WID                     1
#define DAC_DIG_POWERUP_WID                     1
#define DAP_DIG_POWERUP_WID                     1
#define I2S_OUT_POWERUP_WID                     1
#define I2S_IN_POWERUP_WID                      1

//CHIP_MIC_CTRL
#define MIC_BIAS_RESISTOR_WID                   2
#define MIC_BIAS_VOLTAGE_WID                    3
#define MIC_GAIN_WID                            2

//CHIP_SSS_CTRL
#define DAP_MIX_LRSWAP_WID                      1
#define DAP_LRSWAP_WID                          1
#define DAC_LRSWAP_WID                          1
#define I2S_LRSWAP_WID                          1
#define DAP_MIX_SELECT_WID                      2
#define DAP_SELECT_WID                          2
#define DAC_SELECT_WID                          2
#define I2S_SELECT_WID                          2

//DAP_CONTROL
#define MIX_EN_WID                              1
#define DAP_EN_WID                              1

//CHIP_DAC_VOL
#define DAC_VOL_RIGHT_WID                       8
#define DAC_VOL_LEFT_WID                        8

//CHIP_PAD_STRENGTH
#define I2S_LRCLK_WID                           2
#define I2S_SCLK_WID                            2
#define I2S_DOUT_WID                            2
#define CTRL_DATA_WID                           2
#define CTRL_CLK_WID                            2

//CHIP_I2S_CTRL
#define SCLKFREQ_WID                            1
#define MS_WID                                  1
#define SCLK_INV_WID                            1
#define DLEN_WID                                2
#define I2S_MODE_WID                            2
#define LRALIGN_WID                             1
#define LRPOL_WID                               1

//CHIP_ADCDAC_CTRL
#define VOL_BUSY_DAC_RIGHT_WID                  1
#define VOL_BUSY_DAC_LEFT_WID                   1
#define VOL_RAMP_EN_WID                         1
#define VOL_EXPO_RAMP_WID                       1
#define DAC_MUTE_RIGHT_WID                      1
#define DAC_MUTE_LEFT_WID                       1
#define ADC_HPF_FREEZE_WID                      1
#define ADC_HPF_BYPASS_WID                      1

//CHIP_ANA_ADC_CTRL
#define ADC_VOL_M6DB_WID                        1
#define ADC_VOL_RIGHT_WID                       4
#define ADC_VOL_LEFT_WID                        4

//CHIP_CLK_TOP_CTRL
#define ENABLE_INT_OSC_WID                      0
//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

//CHIP_ANA_POWER

//CHIP_REF_CTRL
#define VAG_VAL_MAX_MV                              1575
#define VAG_VAL_MIN_MV                              800
#define VAG_VAL(mV)                                 ((mV>=800 && mV<=1575)?((mV-800)/25):(0))
#define BIAS_CTRL_NOMINAL                           0x0
#define BIAS_CTRL_PLUS_12_5                         0x2
#define BIAS_CTRL_MINUS_12_5                        0x4
#define BIAS_CTRL_MINUS_25                          0x5
#define BIAS_CTRL_MINUS_37_5                        0x6
#define BIAS_CTRL_MINUS_50                          0x7
#define SMALL_POP_NORMAL_RAMP                       0x0
#define SMALL_POP_SLOWDOWN_RAMP                     0x1

//CHIP_SHORT_CTRL
#define LVLADJR_25MA                                0x3
#define LVLADJR_50MA                                0x2
#define LVLADJR_75MA                                0x1
#define LVLADJR_100MA                               0x0
#define LVLADJR_125MA                               0x4
#define LVLADJR_150MA                               0x5
#define LVLADJR_175MA                               0x6
#define LVLADJR_200MA                               0x7
                            
#define LVLADJL_25MA                                0x3
#define LVLADJL_50MA                                0x2
#define LVLADJL_75MA                                0x1 
#define LVLADJL_100MA                               0x0
#define LVLADJL_125MA                               0x4
#define LVLADJL_150MA                               0x5
#define LVLADJL_175MA                               0x6
#define LVLADJL_200MA                               0x7

#define LVLADJC_50MA                                0x3
#define LVLADJC_100MA                               0x2
#define LVLADJC_150MA                               0x1
#define LVLADJC_200MA                               0x0
#define LVLADJC_250MA                               0x4
#define LVLADJC_300MA                               0x5
#define LVLADJC_350MA                               0x6
#define LVLADJC_400MA                               0x7

#define MODE_LR_DISABLE_DETECTOR_RESET_LATCH        0x0
#define MODE_LR_ENABLE_DETECTOR_RESET_TIMEOUT       0x1
#define MODE_LR_ENABLE_DETECTOR_RESET_MANUAL        0x3

#define MODE_CM_DISABLE_DETECTOR_RESET_LATCH        0x0
#define MODE_CM_ENABLE_DETECTOR_RESET_TIMEOUT       0x1
#define MODE_CM_ENABLE_DETECTOR_RESET_AUTO          0x2
#define MODE_CM_ENABLE_DETECTOR_RESET_MANUAL        0x3

//CHIP_LINE_OUT_CTRL
#define OUT_CURRENT_18                              0x0     //.18mA
#define OUT_CURRENT_27                              0x1     //.27mA
#define OUT_CURRENT_36                              0x3     //.36mA
#define OUT_CURRENT_45                              0x7     //.45mA
#define OUT_CURRENT_54                              0xF     //.54mA
#define LO_VAGCNTRL_MAX_MV                          1675
#define LO_VAGCNTRL_MIN_MV                          800
#define LO_VAGCNTRL(mV)                             (mV >= 800 && mV <= 1675)?((mV - 800)/25):(0)  //input the voltage wanted in mV

//CHIP_ANA_CTRL
#define MUTE_LO_UNMUTE                              0
#define MUTE_LO_MUTE                                1
#define SELECT_HP_DAC                               0   
#define SELECT_HP_lINE_IN                           1   
#define EN_ZCD_HP_DISABLED                          0   
#define EN_ZCD_HP_ENABLED                           1
#define EN_ZCD_ADC_ENABLED                          1
#define EN_ZCD_ADC_DISABLED                         1
#define MUTE_HP_UNMUTE                              0
#define MUTE_HP_MUTE                                1
#define SELECT_ADC_MIC                              0
#define SELECT_ADC_LINE_IN                          1
#define EN_ZCD_ADC_DISABLE                          0
#define EN_ZCD_ADC_ENABLE                           1   
#define MUTE_ADC_UNMUTE                             0
#define MUTE_ADC_MUTE                               0


//CHIP_CLK_CTRL
#define SYS_FS_32_KHZ                               0
#define SYS_FS_44_1_KHZ                             1
#define SYS_FS_48_KHZ                               2
#define SYS_FS_96_KHZ                               3
#define MCLK_FREQ_256_FS                            0
#define MCLK_FREQ_384_FS                            1
#define MCLK_FREQ_512_FS                            2
#define MCLK_FREQ_PLL                               3


//CHIP_LINE_OUT_VOL
#define LO_VOL_RIGHT(dB)                            ((dB >= 0 && dB <= 16)?(dB*2):(0))   //dB = the decibal volume you want 0-16dB
#define LO_VOL_LEFT(dB)                             ((dB >= 0 && dB <= 16)?(dB*2):(0))

//CHIP_DIG_POWER
#define CHIP_DIG_POWER_ENABLE                       1
#define CHIP_DIG_POWER_DISABLE                      1

//CHIP_MIC_CTRL
#define MIC_BIAS_RESISTOR_OFF                   0
#define MIC_BIAS_RESISTOR_2K                    1
#define MIC_BIAS_RESISTOR_4K                    2
#define MIC_BIAS_RESISTOR_8K                    3
#define MIC_BIAS_VOLTAGE(mV)                    ((mV >= 1250 && mV <= 3000)?((mV-1250)/250):(0)) //REG_VDDA_VOLTAGE   //mV = wanted bias voltage in millivolts 
#define MIC_GAIN_0DB                          0
#define MIC_GAIN_20DB                         1
#define MIC_GAIN_30DB                         2
#define MIC_GAIN_40DB                         3

//CHIP_SSS_CTRL
#define DAP_MIX_LRSWAP_NORMAL                   0
#define DAP_MIX_LRSWAP_SWAP                     1
#define DAP_LRSWAP_NORMAL                       0
#define DAP_LRSWAP_SWAP                         1
#define DAC_LRSWAP_NORMAL                       0
#define DAC_LRSWAP_SWAP                         1
#define I2S_LRSWAP_NORMAL                       1
#define I2S_LRSWAP_SWAP                         1
#define DAP_MIX_SELECT_ADC                      0
#define DAP_MIX_SELECT_I2S_IN                   1
#define DAP_SELECT_ADC                          0
#define DAP_SELECT_I2S_IN                       1
#define DAC_SELECT_ADC                          0
#define DAC_SELECT_I2S_IN                       1
#define DAC_SELECT_DAP                          3
#define I2S_SELECT_ADC                          0
#define I2S_SELECT_I2S_IN                       1
#define I2S_SELECT_DAP                          3

//DAP_CONTROL
#define DAP_CONTROL_ENABLE                      1
#define DAP_CONTROL_DISABLE                     0

//CHIP_DAC_VOL
#define DAC_VAL_MAX                             0xFC
#define DAC_VAL_MIN                             0x3C

//CHIP_PAD_STRENGTH
#define CHIP_PAD_STRENGTH_DISABLE               0
#define CHIP_PAD_STRENGTH_VDDIO                 1
#define CHIP_PAD_STRENGTH_2_VDDIO               2
#define CHIP_PAD_STRENGTH_3_VDDIO               3

//CHIP_I2S_CTRL
#define SCLKFREQ_64_FS                          0
#define SCLKFREQ_32_FS                          1
#define MS_SLAVE                                0
#define MS_MASTER                               1
#define SCLK_INV_VALID_RISING                   0
#define SCLK_INV_VALID_FALLING                  1
#define DLEN_32_BITS                            0
#define DLEN_24_BITS                            1
#define DLEN_20_BITS                            2
#define DLEN_16_BITS                            3
#define I2S_MODE_I2S_OR_LJ                      0
#define I2S_MODE_RJ                             1
#define I2S_MODE_PCM_AB                         2
#define LRALIGN_1_SCLK_DELAY                    0
#define LRALIGN_AFTER_LRCLK_TRAN                1


//CHIP_ADCDAC_CTRL
#define VOL_BUSY_DAC_RIGHT_READY                0
#define VOL_BUSY_DAC_RIGHT_BUSY                 1
#define VOL_BUSY_DAC_LEFT_READY                 0
#define VOL_BUSY_DAC_LEFT_BUSY                  1
#define VOL_RAMP_EN_DISABLE                     0
#define VOL_RAMP_EN_ENABLE                      1
#define VOL_EXPO_RAMP_LINEAR                    0
#define VOL_EXPO_RAMP_EXPONENTIAL               1
#define DAC_MUTE_RIGHT_MUTED                    1
#define DAC_MUTE_RIGHT_UNMUTED                  0
#define DAC_MUTE_LEFT_MUTED                     1
#define DAC_MUTE_LEFT_UNMUTED                   0
#define ADC_HPF_FREEZE_NORMAL                   0
#define ADC_HPF_FREEZE_FREEZE                   1
#define ADC_HPF_BYPASS_NORMAL                   0
#define ADC_HPF_BYPASS_BYPASSED                 1

//CHIP_CLK_TOP_CTRL
#define ENABLE_INT_OSC_ENABLE                   1
#define ENABLE_INT_OSC_DISABLE                  0

//CHIP_ANA_ADC_CTRL
//#define ADC_VOL_M6DB_LSH                        8
//#define ADC_VOL_RIGHT_LSH                       4
//#define ADC_VOL_LEFT_LSH                        0

#ifdef __cplusplus
}
#endif 


#endif //_SGTL5000_REGS_H_

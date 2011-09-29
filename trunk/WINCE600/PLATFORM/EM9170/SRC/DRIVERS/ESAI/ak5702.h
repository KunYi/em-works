//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ak5702.h
//
//  Definition of AKM 4-Channel ADC AK5702.
//
//------------------------------------------------------------------------------
#ifndef __AK5702_H
#define __AK5702_H

#include "audiotypes.h"


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define AK5702_REG_PM                   0x00
#define AK5702_REG_PLL                  0x01
#define AK5702_REG_SIG                  0x02
#define AK5702_REG_MICGAIN              0x03
#define AK5702_REG_FMT                  0x04
#define AK5702_REG_FS                   0x05
#define AK5702_REG_CLKO                 0x06
#define AK5702_REG_VOL                  0x07
#define AK5702_REG_LINVOL               0x08
#define AK5702_REG_RINVOL               0x09
#define AK5702_REG_TIMER                0x0A
#define AK5702_REG_ALC1                 0x0B
#define AK5702_REG_ALC2                 0x0C
#define AK5702_REG_MOD1                 0x0D
#define AK5702_REG_MOD2                 0x0E
#define AK5702_REG_MOD3                 0x0F
#define AK5702_REG_PMB                  0x10
#define AK5702_REG_PLLB                 0x11
#define AK5702_REG_SIGB                 0x12
#define AK5702_REG_MICGAINB             0x13
#define AK5702_REG_FMTB                 0x14
#define AK5702_REG_FSB                  0x15
#define AK5702_REG_CLKOB                0x16
#define AK5702_REG_VOLB                 0x17
#define AK5702_REG_LINVOLB              0x18
#define AK5702_REG_RINVOLB              0x19
#define AK5702_REG_TIMERB               0x1A
#define AK5702_REG_ALC1B                0x1B
#define AK5702_REG_ALC2B                0x1C
#define AK5702_REG_MOD1B                0x1D
#define AK5702_REG_MOD2B                0x1E


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

// Power Managment 
#define AK5702_PM_PMADAL_LSH            0
#define AK5702_PM_PMADAR_LSH            1
#define AK5702_PM_PMVCM_LSH             2

#define AK5702_PM_PMADAL_WID            1
#define AK5702_PM_PMADAR_WID            1 
#define AK5702_PM_PMVCM_WID             1

#define AK5702_PMB_PMADBL_LSH           0
#define AK5702_PMB_PMADBR_LSH           1

#define AK5702_PMB_PMADBL_WID           0
#define AK5702_PMB_PMADBR_WID           1

//PLL CONTROL
#define AK5702_PLL_PMPLL_LSH            0
#define AK5702_PLL_MS_LSH               1
#define AK5702_PLL_PLL_LSH              2


#define AK5702_PLL_PMPLL_WID            1
#define AK5702_PLL_MS_WID               1
#define AK5702_PLL_PLL_WID              4


// SIGNAL SEL  
#define AK5702_SS_INAL_LSH              0
#define AK5702_SS_INAR_LSH              1
#define AK5702_SS_MDIFA1_LSH            2
#define AK5702_SS_MDIFA2_LSH            3
#define AK5702_SS_PMMPA_LSH             4
#define AK5702_SS_INA5L_LSH             5
#define AK5702_SS_INA5R_LSH             6

#define AK5702_SS_INAL_WID              1
#define AK5702_SS_INAR_WID              1
#define AK5702_SS_MDIFA1_WID            1
#define AK5702_SS_MDIFA2_WID            1
#define AK5702_SS_PMMPA_WID             1
#define AK5702_SS_INA5L_WID             1
#define AK5702_SS_INA5R_WID             1

#define AK5702_SSB_INBL_LSH              0
#define AK5702_SSB_INBR_LSH              1
#define AK5702_SSB_MDIFB1_LSH            2
#define AK5702_SSB_MDIFB2_LSH            3
#define AK5702_SSB_PMMPB_LSH             4
#define AK5702_SSB_INB5L_LSH             5
#define AK5702_SSB_INB5R_LSH             6

#define AK5702_SSB_INBL_WID              1
#define AK5702_SSB_INBR_WID              1
#define AK5702_SSB_MDIFB1_WID            1
#define AK5702_SSB_MDIFB2_WID            1
#define AK5702_SSB_PMMPB_WID             1
#define AK5702_SSB_INB5L_WID             1
#define AK5702_SSB_INB5R_WID             1


// MIC GAIN
#define AK5702_MIC_MGAINA1_LSH            0
#define AK5702_MIC_MGAINA2_LSH            1

#define AK5702_MIC_MGAINA1_WID            1
#define AK5702_MIC_MGAINA2_WID            1

#define AK5702_MICB_MGAINB1_LSH           0
#define AK5702_MICB_MGAINB2_LSH           1

#define AK5702_MICB_MGAINB1_WID           1
#define AK5702_MICB_MGAINB2_WID           1

//FORMAT SEL
#define AK5702_FMT_DIF_LSH             0
#define AK5702_FMT_BCKP_LSH            2
#define AK5702_FMT_MSBS_LSH            3
#define AK5702_FMT_MIXA_LSH            4
#define AK5702_FMT_TDM_LSH             6

#define AK5702_FMT_DIF_WID             2
#define AK5702_FMT_BCKP_WID            1
#define AK5702_FMT_MSBS_WID            1
#define AK5702_FMT_MIXA_WID            1
#define AK5702_FMT_TDM_WID             2

#define AK5702_FMTB_MIXB_LSH           4

#define AK5702_FMT_MIXB_WID            1

//FS SEL
#define AK5702_FS_FS0_LSH              0
#define AK5702_FS_FS1_LSH              1
#define AK5702_FS_FS2_LSH              2
#define AK5702_FS_FS3_LSH              3
#define AK5702_FS_BCKO_LSH             4
#define AK5702_FS_HAPFA0_LSH           6
#define AK5702_FS_HAPFA1_LSH           7


#define AK5702_FS_FS0_WID              1
#define AK5702_FS_FS1_WID              1
#define AK5702_FS_FS2_WID              1
#define AK5702_FS_FS3_WID              1
#define AK5702_FS_BCKO_WID             2
#define AK5702_FS_HAPFA0_WID           1
#define AK5702_FS_HAPFA1_WID           1

#define AK5702_FSB_HAPFB0_LSH          6
#define AK5702_FSB_HAPFB1_LSH          7

#define AK5702_FSB_HAPFB0_WID          1
#define AK5702_FSB_HAPFB1_WID          1


//CLOCK OUTPUT SEL
#define AK5702_CLKO_PS0_LSH            0
#define AK5702_CLKO_PS1_LSH            1
#define AK5702_CLKO_MCKO_LSH           2
#define AK5702_CLKO_INCA_LSH           7

#define AK5702_CLKO_PS0_WID            1
#define AK5702_CLKO_PS1_WID            1
#define AK5702_CLKO_MCKO_WID           1
#define AK5702_CLKO_INCA_WID           1

#define AK5702_CLKOB_INCB_LSH          7
#define AK5702_CLKOB_INCB_WID          1


//VOL CTRL
#define AK5702_VOL_IVOLAC_LSH           0

#define AK5702_VOL_IVOLAC_WID           1

#define AK5702_VOLB_IVOLBC_LSH          0

#define AK5702_VOLB_IVOLBC_WID          1

//LCH INPUT VOL CTRL
#define AK5702_LINVOL_IVAL_LSH      0
#define AK5702_LINVOL_IVAL_WID      8

#define AK5702_RINVOL_IVAR_LSH      0
#define AK5702_RINVOL_IVAR_WID      8

#define AK5702_LINVOLB_IVBL_LSH     0
#define AK5702_LINVOLB_IVBL_WID     8

#define AK5702_RINVOLB_IVBR_LSH     0
#define AK5702_RINVOLB_IVBR_WID     8


//TIMER CTRL
#define AK5702_TIMER_WTMA0_LSH      0
#define AK5702_TIMER_WTMA1_LSH      1
#define AK5702_TIMER_ZTMA_LSH       2
#define AK5702_TIMER_WTMA2_LSH      4
#define AK5702_TIMER_RFSTA_LSH      5

#define AK5702_TIMER_WTMA0_WID      1
#define AK5702_TIMER_WTMA1_WID      1
#define AK5702_TIMER_ZTMA_WID       2
#define AK5702_TIMER_WTMA2_WID      1
#define AK5702_TIMER_RFSTA_WID      2

#define AK5702_TIMERB_WTMB0_LSH     0
#define AK5702_TIMERB_WTMB1_LSH     1
#define AK5702_TIMERB_ZTMB_LSH      2
#define AK5702_TIMERB_WTMB2_LSH     4
#define AK5702_TIMERB_RFSTB_LSH     5

#define AK5702_TIMERB_WTMB0_WID     1
#define AK5702_TIMERB_WTMB1_WID     1
#define AK5702_TIMERB_ZTMB_WID      2
#define AK5702_TIMERB_WTMB2_WID     1
#define AK5702_TIMERB_RFSTB_WID     2


//ALC MODE 1
#define AK5702_ALC1_REFA_LSH        0

#define AK5702_ALC1_REFA_WID        8

#define AK5702_ALC1B_REFB_LSH       0

#define AK5702_ALC1B_REFB_WID       8


//ALC MODE 2
#define AK5702_ALC2_LMTHA_LSH       0
#define AK5702_ALC2_RGA_LSH         2
#define AK5702_ALC2_LMATA_LSH       4
#define AK5702_ALC2_ZELMNA_LSH      6
#define AK5702_ALC2_ALCA_LSH        7

#define AK5702_ALC2_LMTHA_WID       2
#define AK5702_ALC2_RGA_WID         2
#define AK5702_ALC2_LMATA_WID       2
#define AK5702_ALC2_ZELMNA_WID      1
#define AK5702_ALC2_ALCA_WID        1


//MODE CTRL 1
#define AK5702_MOD1_ALC4_LSH        0
#define AK5702_MOD1_LFST_LSH        1
#define AK5702_MOD1_TE_LSH          4

#define AK5702_MOD1_ALC4_WID        1
#define AK5702_MOD1_LFST_WID        1
#define AK5702_MOD1_TE_WID          4


//MODE CTRL 2
#define AK5702_MOD2_TMASTER_LSH     1

#define AK5702_MOD2_TMASTER_WID     1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

//PLL CTRL

#define AK5702_PLL_LRCK_1FS        0
#define AK5702_PLL_BICK_32FS       2
#define AK5702_PLL_BICK_64FS       3
#define AK5702_PLL_MCKI_11M2896    4
#define AK5702_PLL_MCKI_12M288     5
#define AK5702_PLL_MCKI_12M        6
#define AK5702_PLL_MCKI_24M        7
#define AK5702_PLL_MCKI_19M2       8
#define AK5702_PLL_MCKI_12M_DEF    9
#define AK5702_PLL_MCKI_13M5       12
#define AK5702_PLL_MCKI_27M        13
#define AK5702_PLL_MCKI_13M        14
#define AK5702_PLL_MCKI_26M        15

//FS 
#define AK5702_FS_8K    0
#define AK5702_FS_12K   1
#define AK5702_FS_16K   2
#define AK5702_FS_24K   3
#define AK5702_FS_32K   10
#define AK5702_FS_48K   11
#define AK5702_FS_44K1  15


// Power Management 2
#define AK5702_PM2_MCKO_DISABLE         0
#define AK5702_PM2_MCKO_ENABLE          1

#define AK5702_PM2_MS_SLAVE             0
#define AK5702_PM2_MS_MASTER            1

#define AK5702_PM2_HPMTN_MUTE           0
#define AK5702_PM2_HPMTN_NORMAL         1

// Mode Control 1
#define AK5702_CTL1_DIF_LSB             1   // ADC gets MSB anyway
#define AK5702_CTL1_DIF_MSB             2
#define AK5702_CTL1_DIF_I2S             3

#define AK5702_CTL1_BCKO_32FS           0
#define AK5702_CTL1_BCKO_64FS           1


// Mode Control 2
#define AK5702_CTL2_FS_8K               0x00
#define AK5702_CTL2_FS_12K              0x01
#define AK5702_CTL2_FS_16K              0x02
#define AK5702_CTL2_FS_24K              0x03
#define AK5702_CTL2_FS_7K35             0x04
#define AK5702_CTL2_FS_11K025           0x05
#define AK5702_CTL2_FS_14K7             0x06
#define AK5702_CTL2_FS_22K05            0x07
#define AK5702_CTL2_FS_32K              0x22
#define AK5702_CTL2_FS_48K              0x23
#define AK5702_CTL2_FS_29K4             0x26
#define AK5702_CTL2_FS_44K1             0x27

// Digital Volume Control
#define AK5702_LDV_DVL_0dB              0x18
#define AK5702_RDV_DVR_0dB              0x18

// Mode Control 3
#define AK5702_CTL3_BST_OFF             0
#define AK5702_CTL3_BST_MIN             1
#define AK5702_CTL3_BST_MID             2
#define AK5702_CTL3_BST_MAX             3

#define AK5702_CTL3_DVOLC_INDEP         0
#define AK5702_CTL3_DVOLC_DEPEN         1


//-----------------------------------------------------------------------------
// Defines
#define AK5702_REG_ADDR_MASK            0x1F


//-----------------------------------------------------------------------------
// Types
typedef enum
{
    PLL_MASTER_MODE,
    PLL_SLAVE_MODE_1,   // Refer from MCKI
    PLL_SLAVE_MODE_2,   // Refer from LRCK or BICK
    EXT_SLAVE_MODE
} AK5702_CLOCK_MODE;

typedef enum
{
    DIF_LSB = AK5702_CTL1_DIF_LSB,
    DIF_MSB = AK5702_CTL1_DIF_MSB,
    DIF_I2S = AK5702_CTL1_DIF_I2S
} AK5702_DIF_MODE;

typedef enum
{
    BCKO_32FS = AK5702_CTL1_BCKO_32FS,
    BCKO_64FS = AK5702_CTL1_BCKO_64FS
} AK5702_BCKO_MODE;

typedef enum
{
    PLL_LRCK_1FS        = AK5702_PLL_LRCK_1FS,
    PLL_BICK_32FS       = AK5702_PLL_BICK_32FS,
    PLL_BICK_64FS       = AK5702_PLL_BICK_64FS,
    PLL_MCKI_11M2896    = AK5702_PLL_MCKI_11M2896,
    PLL_MCKI_12M288     = AK5702_PLL_MCKI_12M288,
    PLL_MCKI_12M        = AK5702_PLL_MCKI_12M,
    PLL_MCKI_24M        = AK5702_PLL_MCKI_24M,
    PLL_MCKI_19M2       = AK5702_PLL_MCKI_19M2,
    PLL_MCKI_12M_DEF    = AK5702_PLL_MCKI_12M_DEF,
    PLL_MCKI_13M5       = AK5702_PLL_MCKI_13M5,
    PLL_MCKI_27M        = AK5702_PLL_MCKI_27M,
    PLL_MCKI_13M        = AK5702_PLL_MCKI_13M,
    PLL_MCKI_26M        = AK5702_PLL_MCKI_26M
} AK5702_PLL_MODE;

typedef enum
{
    FS_8K       = AK5702_CTL2_FS_8K,
    FS_12K      = AK5702_CTL2_FS_12K,
    FS_16K      = AK5702_CTL2_FS_16K,
    FS_24K      = AK5702_CTL2_FS_24K,
    FS_7K35     = AK5702_CTL2_FS_7K35,
    FS_11K025   = AK5702_CTL2_FS_11K025,
    FS_14K7     = AK5702_CTL2_FS_14K7,
    FS_22K05    = AK5702_CTL2_FS_22K05,
    FS_32K      = AK5702_CTL2_FS_32K,
    FS_48K      = AK5702_CTL2_FS_48K,
    FS_29K4     = AK5702_CTL2_FS_29K4,
    FS_44K1     = AK5702_CTL2_FS_44K1
} AK5702_FS_MODE;


typedef enum
{
    AK5702_CHN_AL   = 1,
    AK5702_CHN_AR   = 2,
    AK5702_CHN_BL   = 4,
    AK5702_CHN_BR   = 8
}AK5702_CHN_MASK;

typedef enum
{
    AK5702_POWER_FULLOFF,
    AK5702_POWER_STANDBY,
    AK5702_POWER_FULLON
}AK5702_POWER_MODE;

//-----------------------------------------------------------------------------
// Classes
class CAK5702
{
private:
    HANDLE m_hI2C;

    BOOL m_bInitialed;

    BOOL m_bPLLPower;
    BOOL m_bVComPower;
    BOOL m_bADCPower;

    AK5702_POWER_MODE m_pwMode;

    AUDIO_PROTOCOL m_audioProtocol;
    DWORD m_dwSampleRate;
    DWORD m_dwBitDepth;
    DWORD m_dwChnNum;
    DWORD m_dwChnMask;
    AK5702_PLL_MODE m_pllMode;

    BYTE ReadRegister(BYTE Reg);
    BOOL WriteRegister(BYTE Reg, BYTE Val);
    VOID DumpRegs(VOID);

public:
    CAK5702 ();
    ~CAK5702 ();

    BOOL InitAK5702(VOID);
    BOOL ConfigADC(AUDIO_PROTOCOL audioProtocol,DWORD dwSampleRate,DWORD dwBitDepth,
        DWORD dwChnNum, DWORD dwChnMask,AK5702_PLL_MODE pllMode);
    BOOL EnalbeADC(void);
    BOOL DisableADC(void);
    VOID SetPLLPower(BOOL bPowerOn);
    VOID SetCodecPower(AK5702_POWER_MODE pwMode);
    VOID SetInputGain(BYTE ucVolAL, BYTE ucVolAR,
    BYTE ucVolBL, BYTE ucVolBR, DWORD dwChnMask);
};

#endif  // __AK5702_H

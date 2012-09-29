//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------//
// file MC34704.h
// Support MC34704 PMIC via I2C interface
// Defines the public prototypes used for the PMIC regulator API
//-----------------------------------------------------------------------------
#ifndef __PMIC_MC34704_H__
#define __PMIC_MC34704_H__

#if    __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

#define MC34704_DVS_STEP_DELAY           50   // 50 us per step
    
// MC34704_DVS_RAMP_DELAY determines the delay in us of a ramp between two voltages
#define MC34704_DVS_RAMP_DELAY(upV, lowV)  \
        (MC34704_DVS_STEP_DELAY * 40 * (upV - lowV) / upV)
                                                                

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

typedef enum _MC34704_REGULATOR
{
    MC34704_REGULATOR1 = 0,
    MC34704_REGULATOR2 = 1,
    MC34704_REGULATOR3 = 2,
    MC34704_REGULATOR4 = 3,
    MC34704_REGULATOR5 = 4,
    MC34704_REGULATOR6 = 5,
    MC34704_REGULATOR7 = 6,
    MC34704_REGULATOR8 = 7
}MC34704_REGULATOR;

typedef enum _MC34704_REG3_LEVEL
{
    // Normal voltage
    MC34704_0      = 0x00,

    // Positive percentage
    MC34704_P_02_5 = 0x01,
    MC34704_P_05_0 = 0x02,
    MC34704_P_07_5 = 0x03,
    MC34704_P_10_0 = 0x04,
    MC34704_P_12_5 = 0x05,
    MC34704_P_15_0 = 0x06,
    MC34704_P_17_5 = 0x07,

    // Negative percentage
    MC34704_M_20_0 = 0x08,
    MC34704_M_17_5 = 0x09,
    MC34704_M_15_0 = 0x0A,
    MC34704_M_12_5 = 0x0B,
    MC34704_M_10_0 = 0x0C,
    MC34704_M_07_5 = 0x0D,
    MC34704_M_05_0 = 0x0E,
    MC34704_M_02_5 = 0x0F

}MC34704_REG3_LEVEL;

typedef enum {
    RESERVEC = 0x00,
    GENERAL1 = 0x01,
    GENERAL2 = 0x02,
    GENERAL3 = 0x03,
    VGSET1   = 0x04,
    VGSET2   = 0x05,
    REG2SET1 = 0x06,
    REG2SET2 = 0x07,
    REG3SET1 = 0x08,
    REG3SET2 = 0x09,
    REG4SET1 = 0x0A,
    REG4SET2 = 0x0B,
    REG5SET1 = 0x0C,
    REG5SET2 = 0x0D,
    REG5SET3 = 0x0E,
    REG6SET1 = 0x0F,
    REG6SET2 = 0x10,
    REG6SET3 = 0x11,
    REG7SET1 = 0x12,
    REG7SET2 = 0x13,
    REG7SET3 = 0x14,
    REG8SET1 = 0x15,
    REG8SET2 = 0x16,
    REG8SET3 = 0x17,
    FAULT    = 0x18,
    I2CSET1  = 0x19,
    REG3DAC  = 0x49,
    REG7CR0  = 0x58,
    REG7DAC  = 0x59     
} MC34704_REGISTER;


#define GEN2_ALLOFF (1<<4)
#define GEN2_ONOFFA (1<<3)
#define GEN2_ONOFFC (1<<2)
#define GEN2_ONOFFD (1<<1)
#define GEN2_ONOFFE (1<<0)


//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
BOOL PmicOpen(VOID);
VOID PmicClose(VOID);
VOID PmicEnable(BOOL bEnable);
BOOL PmicRegulatorSetVoltageLevel(MC34704_REGULATOR regulatorCode, BYTE voltageCode);
BOOL PmicRegulatorGetVoltageLevel(MC34704_REGULATOR regulatorCode, PBYTE voltageCode);
BOOL PmicGetRegister(MC34704_REGISTER reg, BYTE* pbyVal);
BOOL PmicSetRegister(MC34704_REGISTER reg, BYTE byVal);


#ifdef __cplusplus
}
#endif

#endif  //__PMIC_MC34704_H__

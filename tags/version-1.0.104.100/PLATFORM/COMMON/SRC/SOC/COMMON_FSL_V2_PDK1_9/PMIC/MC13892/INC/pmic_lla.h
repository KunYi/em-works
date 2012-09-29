//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: pmic_lla.h
//
//  Defines the public prototypes and types used for the PMIC Low Level Access
//  API.
//
//------------------------------------------------------------------------------

#ifndef __PMIC_LLA_H__
#define __PMIC_LLA_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define PMIC_MC13892_INT_ID_OFFSET        32
#define PMIC_MC13892_MAX_CLK_FREQ         4000000    // 20 MHz

#define MC13892_ON1_BUTTON_MASK  (1 << (PMIC_MC13892_INT_PWRON1I - 32))
#define MC13892_ON2_BUTTON_MASK  (1 << (PMIC_MC13892_INT_PWRON2I - 32))
#define MC13892_ON3_BUTTON_MASK  (1 << (PMIC_MC13892_INT_PWRON3I - 32))
#define MC13892_PWR_BUTTON_MASK  (MC13892_ON1_BUTTON_MASK | MC13892_ON2_BUTTON_MASK | MC13892_ON3_BUTTON_MASK)

#define MC13892_TODAM_MASK       (1 << (PMIC_MC13892_INT_TODAI - 32))
#define MC13892_1HZ_MASK       (1 << (PMIC_MC13892_INT_1HZI - 32))
#define MC13892_PWRON1I_MASK       (1 << (PMIC_MC13892_INT_PWRON1I - 32))
#define MC13892_RTCRSTI_MASK     (1 << (PMIC_MC13892_INT_RTCRSTI - 32))

#define MC13892_VER_11   0x04
#define MC13892_VER_20   0x08
#define MC13892_VER_20a  0x10

//------------------------------------------------------------------------------
// Types
typedef enum _PMIC_MC13892_INT_ID {
    PMIC_MC13892_INT_ADCDONEI = 0,
    PMIC_MC13892_INT_ADCBISDONEI = 1,
    PMIC_MC13892_INT_TSI = 2,
    PMIC_MC13892_INT_VBUSVALIDI = 3,
    PMIC_MC13892_INT_IDFACTORYI = 4,
    PMIC_MC13892_INT_USBOVI = 5,
    PMIC_MC13892_INT_CHGDETI = 6,
    PMIC_MC13892_INT_CHGFAULTI = 7,
    PMIC_MC13892_INT_CHGREVI = 8,
    PMIC_MC13892_INT_CHGSHORTI = 9,
    PMIC_MC13892_INT_CCCVI = 10,
    PMIC_MC13892_INT_CHGCURRI = 11,
    PMIC_MC13892_INT_BPONI = 12,
    PMIC_MC13892_INT_LOBATLI = 13,
    PMIC_MC13892_INT_LOBATHI = 14,
    PMIC_MC13892_INT_IDFLOATI = 19,
    PMIC_MC13892_INT_IDGNDI = 20,
    PMIC_MC13892_INT_CHRGSE1BI = 21,
    PMIC_MC13892_INT_1HZI = 32,
    PMIC_MC13892_INT_TODAI = 33,
    PMIC_MC13892_INT_PWRON3I = 34,
    PMIC_MC13892_INT_PWRON1I = 35,
    PMIC_MC13892_INT_PWRON2I = 36,
    PMIC_MC13892_INT_WDIRESETI = 37,
    PMIC_MC13892_INT_SYSRSTI = 38,
    PMIC_MC13892_INT_RTCRSTI = 39,
    PMIC_MC13892_INT_PCI = 40,
    PMIC_MC13892_INT_WARMI = 41,
    PMIC_MC13892_INT_MEMHLDI = 42,
    PMIC_MC13892_INT_LPBI = 43,
    PMIC_MC13892_INT_THWARNLI = 44,
    PMIC_MC13892_INT_THWARNHI = 45,
    PMIC_MC13892_INT_CLKI = 46,
    PMIC_MC13892_INT_SCPI = 48, 
    PMIC_MC13892_INT_BATTDETBI = 54,
    PMIC_INT_MAX_ID
} PMIC_INT_ID;

typedef enum _PMIC_MC13892_MEM_ID {
    PMIC_MC13892_MEM_A = 0,
    PMIC_MC13892_MEM_B = 1,
    PMIC_MEM_MAX_ID
} PMIC_MEM_ID;


typedef enum _PMIC_MC13892_VER_ID {
    PMIC_MC13892_VER_11  = 0,
    PMIC_MC13892_VER_20  = 1,
    PMIC_MC13892_VER_20a = 2,
    PMIC_MC13892_VER_NULL
} PMIC_VER_ID;


//------------------------------------------------------------------------------
// Functions

// Register access

PMIC_STATUS PmicRegisterRead(unsigned char index, UINT32* reg);
PMIC_STATUS PmicRegisterWrite(unsigned char index, UINT32 reg, UINT32 mask);
// Interrupt handling
PMIC_STATUS PmicInterruptRegister(PMIC_INT_ID int_id, LPTSTR event_name);
PMIC_STATUS PmicInterruptDeregister(PMIC_INT_ID int_id);
PMIC_STATUS PmicInterruptHandlingComplete(PMIC_INT_ID int_id);
PMIC_STATUS PmicInterruptDisable(PMIC_INT_ID int_id);
PMIC_STATUS PmicInterruptEnable(PMIC_INT_ID int_id);
//backup mem read/write
PMIC_STATUS PmicMemRead(PMIC_MEM_ID index, UINT32* reg);
PMIC_STATUS PmicMemWrite(PMIC_MEM_ID index, UINT32 reg);
PMIC_VER_ID PmicGetVersion();

#ifdef __cplusplus
}
#endif

#endif // __PMIC_LLA_H__

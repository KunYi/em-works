//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
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
#define PMIC_MC13783_INT_ID_OFFSET        32
#define PMIC_MC13783_MAX_CLK_FREQ         4000000    // 20 MHz

#define MC13783_ON1_BUTTON_MASK  (1 << (PMIC_MC13783_INT_ONOFD1I - 32))
#define MC13783_ON2_BUTTON_MASK  (1 << (PMIC_MC13783_INT_ONOFD2I - 32))
#define MC13783_ON3_BUTTON_MASK  (1 << (PMIC_MC13783_INT_ONOFD3I - 32))
#define MC13783_PWR_BUTTON_MASK  (MC13783_ON1_BUTTON_MASK | MC13783_ON2_BUTTON_MASK | MC13783_ON3_BUTTON_MASK)

#define MC13783_TODAM_MASK       (1 << (PMIC_MC13783_INT_TODAI - 32))
#define MC13783_RTCRSTI_MASK     (1 << (PMIC_MC13783_INT_RTCRSTI - 32))

//------------------------------------------------------------------------------
// Types
typedef enum _PMIC_MC13783_INT_ID {
    PMIC_MC13783_INT_ADCDONEI = 0,
    PMIC_MC13783_INT_ADCBISDONEI = 1,
    PMIC_MC13783_INT_TSI = 2,
    PMIC_MC13783_INT_WHI = 3,
    PMIC_MC13783_INT_WLI = 4,
    PMIC_MC13783_INT_CHGDETI = 6,
    PMIC_MC13783_INT_CHGOVI = 7,
    PMIC_MC13783_INT_CHGREVI = 8,
    PMIC_MC13783_INT_CHGSHORTI = 9,
    PMIC_MC13783_INT_CCCVI = 10,
    PMIC_MC13783_INT_CHGCURRI = 11,
    PMIC_MC13783_INT_BPONI = 12,
    PMIC_MC13783_INT_LOBATLI = 13,
    PMIC_MC13783_INT_LOBATHI = 14,
    PMIC_MC13783_INT_USBI = 16,
    PMIC_MC13783_INT_IDI = 19,
    PMIC_MC13783_INT_SE1I = 21,
    PMIC_MC13783_INT_CKDETI = 22,
    PMIC_MC13783_INT_1HZI = 32,
    PMIC_MC13783_INT_TODAI = 33,
    PMIC_MC13783_INT_ONOFD1I = 35,
    PMIC_MC13783_INT_ONOFD2I = 36,
    PMIC_MC13783_INT_ONOFD3I = 37,
    PMIC_MC13783_INT_SYSRSTI = 38,
    PMIC_MC13783_INT_RTCRSTI = 39,
    PMIC_MC13783_INT_PCI = 40,
    PMIC_MC13783_INT_WARMI = 41,
    PMIC_MC13783_INT_MEMHLDI = 42,
    PMIC_MC13783_INT_PWRRDYI = 43,
    PMIC_MC13783_INT_THWARNLI = 44,
    PMIC_MC13783_INT_THWARNHI = 45,
    PMIC_MC13783_INT_CLKI = 46,
    PMIC_MC13783_INT_SEMAFI = 47,
    PMIC_MC13783_INT_MC2BI = 49,
    PMIC_MC13783_INT_HSDETI = 50,
    PMIC_MC13783_INT_HSLI = 51,
    PMIC_MC13783_INT_ALSPTHI = 52,
    PMIC_MC13783_INT_AHSSHORTI = 53,
    PMIC_INT_MAX_ID
} PMIC_INT_ID;

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

#ifdef __cplusplus
}
#endif

#endif // __PMIC_LLA_H__

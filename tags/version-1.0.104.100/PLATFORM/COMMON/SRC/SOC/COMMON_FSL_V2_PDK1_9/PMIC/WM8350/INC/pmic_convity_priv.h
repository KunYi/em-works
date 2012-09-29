//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File:        pmic_convity_priv.h
//
// Description: This header file defines private data structures/defines
// for PMIC connectivity implementation for MC13783.
//
//------------------------------------------------------------------------------

#ifndef __PMIC_CONVITY_PRIV_H__
#define __PMIC_CONVITY_PRIV__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// OP values for private ioctls:

// PMIC_IOCTL_CONVITY_OP
#define OP_SET_MODE             1
#define OP_RESET                2

// PMIC_IOCTL_CONVITY_RS232_OP
#define OP_RS232_SET_CFG        1

// PMIC_IOCTL_CONVITY_CEA936_OP
#define OP_CEA936_SET_DETECT    1
#define OP_CEA936_EXIT          2

// Reset values for the MC13783 Connectivity registers
#define MC13783_USB0_ADDR_RESET      0x80060
#define MC13783_CHG_USB1_ADDR_RESET  0x6

// IOCTL structure used for Connectivity's private ioctls.
typedef struct {
    UINT32   op;
    union {
        PMIC_CONVITY_MODE   ifMode;
        struct {
            PMIC_CONVITY_USB_SPEED  usbSpeed;
            PMIC_CONVITY_USB_MODE   usbMode;
        } USB_SPEED;
        struct {
            PMIC_CONVITY_USB_POWER_IN   pwrin;
            PMIC_CONVITY_USB_POWER_OUT  pwrout;
        } USB_PWR;
        PMIC_CONVITY_USB_TRANSCEIVER_MODE  usbXcvrMode;
        PMIC_CONVITY_USB_DEVICE_TYPE       usbOtgType;
        PMIC_CONVITY_USB_OTG_CONFIG        usbOtgCfg;
        struct {
            PMIC_CONVITY_RS232_INTERNAL   cfgInternal;
            PMIC_CONVITY_RS232_EXTERNAL   cfgExternal;
            BOOL                          txTristated;
        } RS232_CFG;
        PMIC_CONVITY_CEA936_DETECTION_CONFIG  cea936DetectCfg;
        PMIC_CONVITY_CEA936_EXIT_SIGNAL       cea936ExitSignal;
    } PARAMS;
} PMIC_PARAM_CONVITY_OP;

#ifdef __cplusplus
}
#endif

#endif // __PMIC_CONVITY_PRIV_H__

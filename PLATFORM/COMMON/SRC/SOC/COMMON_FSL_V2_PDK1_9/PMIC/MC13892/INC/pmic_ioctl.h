//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:        pmic_ioctl.h
//
// Description: This header file defines IOCTL codes and parameter sturctures
// for PMIC stream interface driver.
//
//------------------------------------------------------------------------------

#ifndef __PMIC_IOCTL_H__
#define __PMIC_IOCTL_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// Register access
#define PMIC_IOCTL_LLA_READ_REG         CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1000,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_LLA_WRITE_REG        CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1001,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )
// Interrupt handling
#define PMIC_IOCTL_LLA_INT_REGISTER     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1002,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_LLA_INT_DEREGISTER   CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1003,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_LLA_INT_COMPLETE     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1004,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_LLA_INT_DISABLE      CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1005,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_LLA_INT_ENABLE       CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1006,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_ADC_TOUCH_READ       CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1007,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_ADC_SET_MODE         CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1008,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_ADC_GET_HS_CURRENT   CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1009,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_ADC_GET_MUL_CH_SPL   CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x100a,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_ADC_GET_SGL_CH_8SPL  CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x100b,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_ADC_GET_SGL_CH_1SPL  CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x100c,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_ADC_SET_CMPTR_TRHLD  CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x100d,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_CONVT_OP             CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x100e,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )


//------------------------------------------------------------------------------
// Enumerations and Structures

typedef struct {
    unsigned char addr;
    UINT32        data;
    UINT32        mask;
} PMIC_PARAM_LLA_WRITE_REG;

typedef struct {
    UINT32 int_id;
    LPTSTR event_name;
} PMIC_PARAM_INT_REGISTER;


#ifdef __cplusplus
}
#endif

#endif // __PMIC_IOCTL_H__

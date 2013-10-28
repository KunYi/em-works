//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
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


#define PMIC_IOCTL_CONVT_RS232_OP       CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x100f,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_CONVT_CEA936_OP      CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1010,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_CONVT_USB_SETSPEED   CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1011,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_CONVT_USB_SETPWR     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1012,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_CONVT_USB_SETXCVR    CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1013,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_CONVT_USB_BGNHNP     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1014,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_CONVT_USB_ENDHNP     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1015,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_CONVT_USB_SETCFG     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1016,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_CONVT_USB_CLRCFG     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1017,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )
//Touch Screen Power mode implementation
#define PMIC_IOCTL_ADC_POWER_MODE       CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1018,              \
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

//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @file   pmic_gpio.h
/// @brief  Public prototypes and types used for the PMIC GPIO API.
///
/// This file contains the interface for controlling the GPIOs on the WM8350.
///
/// @version $Id: pmic_ioctl.h 649 2007-06-15 22:31:01Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
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

#define PMIC_IOCTL_LLA_BLOCK_CONTROL_IF CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1018,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMIC_IOCTL_LLA_UNBLOCK_CONTROL_IF CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x1019,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

//------------------------------------------------------------------------------
// Enumerations and Structures

typedef struct {
    unsigned char addr;
    UINT32        data;
    UINT32        mask;
} PMIC_PARAM_LLA_WRITE_REG, PMIC_PARAM_LLA_READ_REG;

typedef struct {
    UINT32 int_id;
    LPTSTR event_name;
} PMIC_PARAM_INT_REGISTER;

//
// Shorthand macros
//
#define PMIC_WRITE_REG(_addr, _data, _mask, _pStatus) do {          \
    PMIC_PARAM_LLA_WRITE_REG    param;                              \
    param.addr = _addr;                                             \
    param.data = _data;                                             \
    param.mask = _mask;                                             \
    if(DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,      \
                       sizeof(param), NULL, 0, NULL, NULL)          \
      )                                                             \
    {                                                               \
        *_pStatus = PMIC_SUCCESS;                                   \
    }                                                               \
    else                                                            \
    {                                                               \
        *_pStatus = PMIC_ERROR;                                     \
    }                                                               \
} while (0)

#define PMIC_READ_REG(_addr, _pData, _pStatus) do {                 \
    PMIC_PARAM_LLA_READ_REG    param;                               \
    UINT32                     regval;                              \
    param.addr = _addr;                                             \
    param.data = 0;                                                 \
    param.mask = 0;                                                 \
    if(DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,      \
                       sizeof(param), &regval, sizeof(regval),      \
                       NULL, NULL)                                  \
      )                                                             \
    {                                                               \
        *_pStatus = PMIC_SUCCESS;                                   \
    }                                                               \
    else                                                            \
    {                                                               \
        *_pStatus = PMIC_ERROR;                                     \
    }                                                               \
} while (0)

#ifdef __cplusplus
}
#endif

#endif // __PMIC_IOCTL_H__

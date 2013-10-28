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
//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:           acc.h
//  Purpose:        this file define structure and protoytpe of API for acc module
//
//
//------------------------------------------------------------------------------
//

#ifndef __ACC_H__
#define __ACC_H__

#if __cplusplus
extern "C" {
#endif

    // Types
    typedef enum {
        ACC_MODE_STANDBY,          //default
        ACC_MODE_MEASUREMENT,
        ACC_MODE_LEVELDETECTION,
        ACC_MODE_PULSEDETECTION
    }ACC_MODE;

    typedef enum {
        ACC_GSEL_8G,               //default
        ACC_GSEL_2G,
        ACC_GSEL_4G
    }ACC_GSEL;

    typedef enum {
        ACC_POL_POSITIVE,          //default
        ACC_POL_NEGATIVE
    }ACC_POL;

    typedef enum {
        ACC_OUTPUT_8BIT,           //default
        ACC_OUTPUT_10BIT
    }ACC_OUTPUT_WIDTH;

    typedef enum {
        ACC_DFBW_62P5HZ,           //default
        ACC_DFBW_125HZ
    }ACC_DFBW;

    typedef enum {
        ACC_THRESHOLD_ABSOLUTE,    //default
        ACC_THRESHOLD_PNVALUE
    }ACC_THRESHOLD_OPT;

    typedef enum {
        ACC_DRPD_ENABLE,           //default
        ACC_DRPD_DISABLE
    }ACC_DRPD;

    //------------------------------------------------------------------------------
    // MACRO DEFINITIONS
    //------------------------------------------------------------------------------
    #define ACC_DX 0x01                    // for disable X detection
    #define ACC_DY 0x02                    // for disable Y detection
    #define ACC_DZ 0x04                    // for disable Z detection
    #define ACC_DA 0x0                     // for enable all 3 axis detection

    typedef struct ACC_OUTPUT
    {
        ACC_OUTPUT_WIDTH   eOutputWidth;    // 0 mean output is 8bit, 1 mean output is 10bit
        INT16              iXOutput10;
        INT16              iYOutput10;
        INT16              iZOutput10;
        INT8               iXOutput8;
        INT8               iYOutput8;
        INT8               iZOutput8;
        float              x;
        float              y;
        float              z;
    }ACC_OUTPUT;

    typedef struct ACC_STATUS
    {
    BYTE    INTR1:1;
    BYTE    INTR2:1;
    BYTE    PDZ:1;
    BYTE    PDY:1;
    BYTE    PDX:1;
    BYTE    LDZ:1;
    BYTE    LDY:1;
    BYTE    LDX:1;
    } ACC_STATUS;


    // IOCTL define
    /**********************SET command ***********************************/
    /* Set ACC Mode */
    #define IOCTL_ACC_SET_MODE              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4030, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC G-level */
    #define IOCTL_ACC_SET_GSEL              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4031, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC PD latency time */
    #define IOCTL_ACC_SET_LATENCYTIME       \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4032, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC PD pulse duration */
    #define IOCTL_ACC_SET_PULSEDURATION     \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4033, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC PD time window */
    #define IOCTL_ACC_SET_TIMEWINDOW        \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4034, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC LD threshold */
    #define IOCTL_ACC_SET_LDTH              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4035, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC PD threshold */
    #define IOCTL_ACC_SET_PDTH              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4036, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC LD polarity */
    #define IOCTL_ACC_SET_LDPOL             \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4037, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC PD polarity */
    #define IOCTL_ACC_SET_PDPOL             \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4038, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC output width */
    #define IOCTL_ACC_SET_OUTPUTWIDTH       \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4039, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC digital filter band width */
    #define IOCTL_ACC_SET_DFBW              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4040, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC threshod value */
    #define IOCTL_ACC_SET_THOPT             \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4041, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC detect */
    #define IOCTL_ACC_SET_DETECTION         \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4042, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC X Offset */
    #define IOCTL_ACC_SET_XOFFSET           \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4043, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC Y Offset */
    #define IOCTL_ACC_SET_YOFFSET           \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4044, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC Z Offset */
    #define IOCTL_ACC_SET_ZOFFSET           \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4045, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set/Unset DRPD bit to disable/enable Data Ready Signal output to INTI interrupt*/
    #define IOCTL_ACC_SET_DRPD           \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4046, METHOD_BUFFERED, FILE_ANY_ACCESS)    

    /**************************GET command *********************************/
    /* Get ACC Mode */
    #define IOCTL_ACC_GET_MODE              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4060, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC G-level */
    #define IOCTL_ACC_GET_GSEL              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4061, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC PD latency time */
    #define IOCTL_ACC_GET_LATENCYTIME       \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4062, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC PD pulse duration */
    #define IOCTL_ACC_GET_PULSEDURATION     \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4063, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC PD time window */
    #define IOCTL_ACC_GET_TIMEWINDOW        \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4064, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC LD threshold */
    #define IOCTL_ACC_GET_LDTH              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4065, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC PD threshold */
    #define IOCTL_ACC_GET_PDTH              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4066, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC LD polarity */
    #define IOCTL_ACC_GET_LDPOL             \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4067, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC PD polarity */
    #define IOCTL_ACC_GET_PDPOL             \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4068, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC output width */
    #define IOCTL_ACC_GET_OUTPUTWIDTH       \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4069, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC digital filter band width */
    #define IOCTL_ACC_GET_DFBW              \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4070, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Set ACC threshod value */
    #define IOCTL_ACC_GET_THOPT             \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4071, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC X Offset */
    #define IOCTL_ACC_GET_XOFFSET           \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4072, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC Y Offset */
    #define IOCTL_ACC_GET_YOFFSET           \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4073, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC Z Offset */
    #define IOCTL_ACC_GET_ZOFFSET           \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4074, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC 3-axis output */
    #define IOCTL_ACC_GET_OUTPUT            \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4075, METHOD_BUFFERED, FILE_ANY_ACCESS)
    /* Get ACC status */
    #define IOCTL_ACC_GET_STATUS            \
        CTL_CODE(FILE_DEVICE_UNKNOWN, 4076, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define ACC_DRDY_NAME1      L"ACC DRDY EVENT 1"
    #define ACC_DRDY_NAME2      L"ACC DRDY EVENT 2"

#ifdef __cplusplus
}
#endif

#endif // __ACC_H__

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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  Header: bulverde_iicbus.h
//
//  Defines the I2C bus controller CPU register layout and definitions.
//
#ifndef __BULVERDE_IICBUS_H
#define __BULVERDE_IICBUS_H

#if __cplusplus
    extern "C" 
    {
#endif

//
// I2C bit definitions.
//
#define I2C_START   0x1
#define I2C_STOP    0x2
#define I2C_TB      0x8
#define I2C_MA      0x10
#define I2C_SCLE    0x20
#define I2C_IUE     0x40
#define I2C_GCD     0x80
#define I2C_ITEIE   0x100
#define I2C_IRFIE   0x200
#define I2C_BEIE    0x400
#define I2C_SDIE    0x800
#define I2C_ALDIE   0x1000
#define I2C_SADIE   0x2000
#define I2C_UR      0x4000
#define I2C_FM      0x8000

#define I2C_ITE     0x40
#define I2C_ARB     0x20

//
// Power management change control register bits.
//
#define PVCR_VCSA                  (0x1<<14)
#define PVCR_CommandDelay          (0xf80)

//
// Power management general configuration register bits.
//
#define PCFR_FVC                   (0x1 << 10)
#define PCFR_PI2C_EN               (0x1 << 6)

//
// Power management I2C command register bits.
//
#define PCMD_MBC                  (0x1 << 12)
#define PCMD_DCE                  (0x1 << 11)
#define PCMD_LC                   (0x1 << 10)
#define PCMD_SQC                  (0x3 << 8)

//
// I2C controller register types.
//
enum   RegTypes {ReadWrite, ReadOnly, WriteOnly, Verify, Reserved};
struct RegistersTableInfo
{
    int  addr;
    int  data;
    int  code;
};

//------------------------------------------------------------------------------
//  Type: BULVERDE_IICBUS_REG
//
//  Defines IIC bus control register layout.
//

typedef struct
{
    UINT32    ibmr;     // I2C bus monitor register.
    UINT32    rsvd0;
    UINT32    idbr;     // I2C data buffer register.
    UINT32    rsvd1;
    UINT32    icr;      // I2C control register.
    UINT32    rsvd2;
    UINT32    isr;      // I2C status register.
    UINT32    rsvd3;
    UINT32    isar;     // I2C slave address register.
    UINT32    rsvd4;
    UINT32    i2ccr;    // I2C clock count register.

} BULVERDE_IICBUS_REG, *PBULVERDE_IICBUS_REG;

#if __cplusplus
    }
#endif

#endif 

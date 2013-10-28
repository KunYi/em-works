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
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File:  devoppmap.h
//
//
#ifndef _DEVOPPMAP_H_
#define _DEVOPPMAP_H_

#define POWERDOMAIN_NULL                (-1)

// optimize for ship builds
#ifndef SHIP_BUILD

#define DECL_DEVICE_OPP_MAP(x, y, z, d)    {x, y, z, d}
#define UPDATE_DEVICE_STATE(x, d, s)       x[d].deviceState = (DWORD)s

#else

#define DECL_DEVICE_OPP_MAP(x, y, z, d)    {z, d}
#define UPDATE_DEVICE_STATE(x, d, s)       

#endif

//-----------------------------------------------------------------------------
//  local structures
typedef struct
{
#ifndef SHIP_BUILD
    OMAP_DEVICE                     devId;
    DWORD                           deviceState;
#endif
    DWORD                           opm;
    DWORD                           powerDomain;
} DevicePerformanceMapTbl_t;


//-----------------------------------------------------------------------------
//  device to operation mode map
#define DEVICE_OPP_TABLE() \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SSI,         D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SDRC,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_D2D,         D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_HSOTGUSB,    D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_OMAPCTRL,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MAILBOXES,   D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP1,      D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP5,      D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER10,   D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER11,   D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_UART1,       D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_UART2,       D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_I2C1,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_I2C2,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_I2C3,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCSPI1,      D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCSPI2,      D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCSPI3,      D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCSPI4,      D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_HDQ,         D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MSPRO,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MMC1,        D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MMC2,        D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MMC3,        D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_DES2,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SHA12,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_AES2,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_ICR,         D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_DES1,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SHA11,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_RNG,         D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_AES1,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_PKA,         D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_USBTLL,      D4,     kOpm1, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_TS,          D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_EFUSE,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER1,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER12,   D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_32KSYNC,     D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_WDT1,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_WDT2,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO1,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SR1,         D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SR2,         D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_USIM,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO2,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO3,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO4,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO5,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO6,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP2,      D4,     kOpm0, POWERDOMAIN_PERIPHERAL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP3,      D4,     kOpm0, POWERDOMAIN_PERIPHERAL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP4,      D4,     kOpm0, POWERDOMAIN_PERIPHERAL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER2,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER3,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER4,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER5,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER6,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER7,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER8,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER9,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_UART3,       D4,     kOpm0, POWERDOMAIN_PERIPHERAL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_WDT3,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_DSS,         D4,     kOpm0, POWERDOMAIN_DSS), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_DSS1,        D4,     kOpm0, POWERDOMAIN_DSS), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_DSS2,        D4,     kOpm0, POWERDOMAIN_DSS), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_TVOUT,       D4,     kOpm1, POWERDOMAIN_DSS), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_CAMERA,      D4,     kOpm1, POWERDOMAIN_CAMERA), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_CSI2,        D4,     kOpm0, POWERDOMAIN_CAMERA), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_DSP,         D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_2D,          D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_3D,          D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SGX,         D4,     kOpm1, POWERDOMAIN_SGX), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_HSUSB1,      D4,     kOpm1, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_HSUSB2,      D4,     kOpm1, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_USBHOST1,    D4,     kOpm1, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_USBHOST2,    D4,     kOpm1, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_USBHOST3,    D4,     kOpm1, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GENERIC,     D4,     kOpm0, POWERDOMAIN_NULL)


#endif //_DEVOPPMAP_H_
//-----------------------------------------------------------------------------
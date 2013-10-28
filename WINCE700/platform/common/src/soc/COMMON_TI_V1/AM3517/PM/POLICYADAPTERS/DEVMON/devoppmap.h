// All rights reserved ADENEO EMBEDDED 2010
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

#include "am3517_clocks.h"
#include "am3517_prcm.h"

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
    int                           opm;
    int                           powerDomain;
} DevicePerformanceMapTbl_t;


//-----------------------------------------------------------------------------
//  device to operation mode map
#define DEVICE_OPP_TABLE() \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SDRC,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_IPSS,        D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SCMCTRL,     D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP1,      D4,     kOpm1, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP5,      D4,     kOpm1, POWERDOMAIN_CORE), \
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
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MMC1,        D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MMC2,        D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MMC3,        D4,     kOpm0, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_DES2,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SHA12,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_AES2,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_DES1,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SHA11,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_RNG,         D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_AES1,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_PKA,         D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_USBTLL,      D4,     kOpm2, POWERDOMAIN_CORE), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_TS,          D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_EFUSE,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER1,    D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPTIMER12,   D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_32KSYNC,     D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_WDT1,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_WDT2,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO1,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO2,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO3,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO4,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO5,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GPIO6,       D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP2,      D4,     kOpm1, POWERDOMAIN_PERIPHERAL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP3,      D4,     kOpm1, POWERDOMAIN_PERIPHERAL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_MCBSP4,      D4,     kOpm1, POWERDOMAIN_PERIPHERAL), \
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
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_TVOUT,       D4,     kOpm2, POWERDOMAIN_DSS), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_VPFE,        D4,     kOpm2, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_CSI2,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_2D,          D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_3D,          D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_USBHOST1,    D4,     kOpm2, POWERDOMAIN_USBHOST), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_USBHOST2,    D4,     kOpm2, POWERDOMAIN_USBHOST), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_USBHOST3,    D4,     kOpm2, POWERDOMAIN_USBHOST), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_OHCI,        D4,     kOpm2, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_EHCI,        D4,     kOpm2, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_VRFB,        D4,     kOpm0, POWERDOMAIN_NULL), \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_GENERIC,     D4,     kOpm0, POWERDOMAIN_NULL),  \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_STUB1,       D4,     kOpm0, POWERDOMAIN_NULL),  \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_STUB2,       D4,     kOpm0, POWERDOMAIN_NULL),  \
    DECL_DEVICE_OPP_MAP(OMAP_DEVICE_SGX,         D4,     kOpm2, POWERDOMAIN_SGX)
    

#endif //_DEVOPPMAP_H_
//-----------------------------------------------------------------------------
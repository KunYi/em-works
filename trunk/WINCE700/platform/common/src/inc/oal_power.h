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
//  Header: oal_power.h
//
#ifndef __OAL_POWER_H
#define __OAL_POWER_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

extern volatile UINT32 g_oalWakeSource;

//------------------------------------------------------------------------------

VOID
OEMPowerOff(
    );

BOOL
OALPowerWakeSource(
    UINT32 sysIntr
    );

BOOL
OALIoCtlHalPresuspend(
    UINT32 code, 
    __inout_bcount_opt(inSize) VOID *pInBuffer, 
    UINT32 inSize, 
    __inout_bcount_opt(outSize) VOID *pOutBuffer, 
    UINT32 outSize, 
    __out_opt UINT32 *pOutSize
    );

BOOL
OALIoCtlHalEnableWake(
    UINT32 code, 
    __inout_bcount_opt(inSize) VOID *pInBuffer, 
    UINT32 inSize, 
    __inout_bcount_opt(outSize) VOID *pOutBuffer, 
    UINT32 outSize, 
    __out_opt UINT32 *pOutSize
    );

BOOL 
OALIoCtlHalDisableWake(
    UINT32 code, 
    __inout_bcount_opt(inSize) VOID *pInBuffer, 
    UINT32 inSize, 
    __inout_bcount_opt(outSize) VOID *pOutBuffer, 
    UINT32 outSize, 
    __out_opt UINT32 *pOutSize
    );

BOOL 
OALIoCtlHalGetWakeSource(
    UINT32 code, 
    __inout_bcount_opt(inSize) VOID *pInBuffer, 
    UINT32 inSize, 
    __inout_bcount_opt(outSize) VOID *pOutBuffer, 
    UINT32 outSize, 
    __out_opt UINT32 *pOutSize
    );

//------------------------------------------------------------------------------

VOID
OALCPUPowerOff(
    );

VOID
OALCPUPowerOn(
    );

VOID
BSPPowerOff(
    );

VOID
BSPPowerOn(
    );

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

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
#ifndef __BOOT_TRANSPORT_SERIAL_H
#define __BOOT_TRANSPORT_SERIAL_H

#include <bootTransport.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

enum BootTransportSerialIoCtl_e {
    BOOT_TRANSPORT_SERIAL_IOCTL_OPTIONS = BOOT_TRANSPORT_IOCTL(0x8002)
};

//------------------------------------------------------------------------------

typedef 
bool_t 
(*pfnBootTransportSerialDmaInit_t)(
    uint32_t buffer,
    uint32_t size
    );

typedef 
bool_t 
(*pfnBootTransportSerialInit_t)(
    __in void *kitlSerialInfo
    );

typedef
void 
(*pfnBootTransportSerialDeinit_t)(
    );

typedef 
uint16_t
(*pfnBootTransportSerialSendFrame_t)(
    __in_bcount(length) uint8_t *pBuffer, 
    uint16_t length
    );

typedef
uint16_t
(*pfnBootTransportSerialRecvFrame_t)(
    __out_bcount(*pLength) uint8_t *pBuffer, 
    __inout uint16_t length
    );

typedef
void
(*pfnBootTransportSerialFilter_t)(
    flags_t filter
    );

typedef struct BootSerialDriver_t {
    pfnBootTransportSerialDmaInit_t pfnDmaInit;
    pfnBootTransportSerialInit_t pfnInit;
    pfnBootTransportSerialDeinit_t pfnDeinit;
    pfnBootTransportSerialSendFrame_t pfnSendFrame;
    pfnBootTransportSerialRecvFrame_t pfnRecvFrame;
    pfnBootTransportSerialFilter_t pfnFilter;
} BootSerialDriver_t;

//------------------------------------------------------------------------------

enum BootTransportSerialFlag_e {
    BOOT_TRANSPORT_SERIAL_FLAG_ENABLED    = 0x0001,   // Kitl enable
    BOOT_TRANSPORT_SERIAL_FLAG_PASSIVE    = 0x0002,   // Kitl passive mode
    BOOT_TRANSPORT_SERIAL_FLAG_POLL       = 0x0010,   // Use polling (no interrupt)
    BOOT_TRANSPORT_SERIAL_FLAG_EXTNAME    = 0x0020    // Extend device name
};

typedef 
bool_t
(*pfnBootTransportSerialName_t)(
    __in void *pContext,
    __in uint16_t mac[3],
    __out_bcount(bufferSize) string_t buffer,
    size_t bufferSize
    );

handle_t
BootTransportSerialInit(
    __in void *pContext,
    __in const BootSerialDriver_t *pDriver,
    __in pfnBootTransportSerialName_t pfnName,
    __in_opt uint8_t *pAddress,                 // unused, set to NULL
    uint32_t offset,
    __inout uint16_t mac[3],
    uint32_t ip                                 // unused, set to zero
    );

//------------------------------------------------------------------------------

typedef struct BootTransportSerialOptionsParams_t {
    uint32_t bootMeRetry;
    uint32_t bootMeDelay;
    uint32_t recvTimeout;
} BootTransportSerialOptionsParams_t;

//------------------------------------------------------------------------------

__inline
bool_t
BootTransportSerialOptions(
    handle_t hDriver,
    uint32_t bootMeRetry,
    uint32_t bootMeDelay,
    uint32_t recvTimeout
    )
{
    BootDriverVTable_t **pVTable = hDriver;
    BootTransportSerialOptionsParams_t params;

    params.bootMeRetry = bootMeRetry;
    params.bootMeDelay = bootMeDelay;
    params.recvTimeout = recvTimeout;

    return (*pVTable)->pfnIoCtl(
        hDriver, BOOT_TRANSPORT_SERIAL_IOCTL_OPTIONS, &params, sizeof(params)
        );
}
    
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

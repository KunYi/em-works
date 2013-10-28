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
#ifndef __BOOT_TRANSPORT_EDBG_NE2000_H
#define __BOOT_TRANSPORT_EDBG_NE2000_H

#include <bootTransportEdbg.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

bool_t
NE2000Init(
    uint8_t *pAddress, 
    uint32_t offset, 
    uint16_t mac[3]
    );

uint32_t
NE2000SendFrame(
    uint8_t *pBuffer, 
    uint32_t length
    );

uint32_t
NE2000GetFrame(
    uint8_t *pBuffer, 
    uint32_t *pLength
    );


//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_TRANSPORT_EDBG_NE2000_H


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
//  Header: bulverde_iisbus.h
//
//  Defines the IIS bus control register layout and associated constants 
//  and types.
//
//------------------------------------------------------------------------------

#ifndef __BULVERDE_IISBUS_H
#define __BULVERDE_IISBUS_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: BULVERDE_IISBUS_REG    
//
//  Defines IIS bus control register layout.
//

typedef struct
{
    UINT32    sacr0;        // Global control register.
    UINT32    sacr1;        // Serial Audio I2S/MSB justified control register.
    UINT32    rsvd0;
    UINT32    sasr0;        // Serial audio I2S/MSB justified interface and fifo status register.
    UINT32    rsvd1;
    UINT32    saimr;        // Serial audio intrerupt mask register.
    UINT32    saicr;        // Serial audio interrupt clear register.
    UINT32    rsvd2[17];
    UINT32    sadiv;        // Audio clock divider register.
    UINT32    rsvd3[7];
    UINT32    sadr;         // Serial audio data register.

} BULVERDE_IIS_REG, *PBULVERDE_IIS_REG;

#if __cplusplus
    }
#endif

#endif 

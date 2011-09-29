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
//  Header: bulverde_ssp.h
//
//  Defines the Synchronous Serial Port (SSP) controller CPU register layout and
//  definitions.
//
#ifndef __BULVERDE_SSP_H
#define __BULVERDE_SSP_H

#if __cplusplus
    extern "C" 
    {
#endif

//------------------------------------------------------------------------------
//  Type: BULVERDE_SSP_REG    
//
//  Defines the SSP register layout.
//

typedef struct
{
    UINT32    sscr0;	// SSP control register 0.
    UINT32    sscr1;	// SSP control register 1.
    UINT32    ssr;		// SSP status register.
    UINT32    ssitr;    // SSP interrupt test register.
    UINT32    ssdr;		// SSP data read/write register.

} BULVERDE_SSP_REG, *PBULVERDE_SSP_REG;

#if __cplusplus
    }
#endif

#endif 

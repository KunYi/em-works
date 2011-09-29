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

#include <kxmips.h>

//------------------------------------------------------------------------------
//
//  Function:  CacheErrorHandler()
//
//  This function handles cache errors that occur on a MIPS platform. The kernel
//  initializes the vector table to this function when a cache error occurs.
//
LEAF_ENTRY(CacheErrorHandler)
        .set    noreorder
        .globl  CacheErrorHandler_End

        nop
        nop
        break     1
        eret
        
CacheErrorHandler_End:

        .end CacheErrorHandler

//------------------------------------------------------------------------------
     

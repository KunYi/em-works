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

#include "kxmips.h"

//------------------------------------------------------------------------------
//
// Function:     OALMIPSGetProcessorId()
//
// This function return CP0 register 15 select 0 - processor identification
//

LEAF_ENTRY(OALMIPSGetProcessorId)
        .set    noreorder

        j       ra
        mfc0    v0, prid

        .set    noreorder
        .end    OALFlushDCache

//------------------------------------------------------------------------------


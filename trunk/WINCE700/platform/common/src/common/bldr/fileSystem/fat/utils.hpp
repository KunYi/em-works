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
#pragma once
#include <bootTypes.h>

//------------------------------------------------------------------------------

__inline
size_t
Items(
    size_t size,
    size_t itemSize
    )
{
    return (size + itemSize - 1) / itemSize;
}

//------------------------------------------------------------------------------

__inline
size_t
RoundUp(
    size_t size,
    size_t itemSize
    )
{
    return ((size + itemSize - 1) / itemSize) * itemSize;
}

//------------------------------------------------------------------------------


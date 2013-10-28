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
#include <windows.h>
#include <oal_io.h>
#include <ceddk.h>

//------------------------------------------------------------------------------

UCHAR
READ_PORT_UCHAR(
    volatile const UCHAR * const port
    )
{
    return (UCHAR)_inp((USHORT)(ULONG)port);
}

//------------------------------------------------------------------------------

void
WRITE_PORT_UCHAR(
    volatile UCHAR * const port,
    UCHAR const value
    )
{
    _outp((USHORT)(ULONG)port, value);
}

//------------------------------------------------------------------------------

USHORT
READ_PORT_USHORT(
    volatile const USHORT * const port
    )
{
    return _inpw((USHORT)(ULONG)port);
}

//------------------------------------------------------------------------------

void
WRITE_PORT_USHORT(
    volatile USHORT * const port,
    USHORT const value
    )
{
    _outpw((USHORT)(ULONG)port, value);
}

//------------------------------------------------------------------------------

ULONG
READ_PORT_ULONG(
    volatile const ULONG * const port
    )
{
    return _inpd((USHORT)(ULONG)port);
}

//------------------------------------------------------------------------------

void
WRITE_PORT_ULONG(
    volatile ULONG * const port,
    ULONG const value
    )
{
    _outpd((USHORT)(ULONG)port, value);
}

//------------------------------------------------------------------------------


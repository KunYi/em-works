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
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2007 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  atapiio.h
//
//  Structure declaration for ATA IO request
//
//------------------------------------------------------------------------------

#ifndef _ATAPIIO_H_
#define _ATAPIIO_H_

typedef struct _IOREQ {
    DWORD  dwChd;          // ATA command
    HANDLE hEvent;         // Win32 event to signal blocked requesting thread
    DWORD  dwStatus;       // request status
    HANDLE hProcess;       // calling process
    DWORD  dwReqPerm;      // rermissions required to access caller's buffers
    DWORD  dwCode;
    PBYTE  pInBuf;
    DWORD  dwInBufSize;
    PBYTE  pOutBuf;
    DWORD  dwOutBufSize;
    PDWORD pBytesReturned;
    DWORD  dwRef;          // reference returned to callback
} IOREQ, *PIOREQ;

#endif // _ATAPIIO_H_

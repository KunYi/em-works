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
/**

Abstract:
    This module defines the primitive functions to control

--*/
#ifndef __RNEUTIL_H_
#define __RNEUTIL_H_

typedef struct {
    PBYTE pBufferHead;
    WORD wBufferSize;// have to dword aligh buffer.
    WORD wQEmpty;
    WORD wQUsed;
} BufferDescript;

BOOL InitBuffer(BufferDescript * pDesc, PVOID pBuffer,WORD wBLength);
WORD AvaiableBufferSize ( BufferDescript * pBDescr );
PVOID AllocBuffer (BufferDescript * pBDescr, WORD wLength) ;
BOOL  FreeBuffer(BufferDescript * pBDescr, PVOID pPtr) ;
BOOL GetFirstUsedBuffer(BufferDescript * pBDescr, PVOID *ppPtr,PWORD pwLength);

#endif
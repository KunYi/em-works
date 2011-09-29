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
// sr
// local functions prototypes, which were originally inline
// ones
VOID* HdqOpen(void);
VOID HdqClose(HANDLE hContext);
BOOL HdqWrite8(HANDLE hContext, UCHAR address, UCHAR data);
BOOL HdqRead8(HANDLE hContext, UCHAR address, UCHAR *pData);
BOOL HdqWrite16(HANDLE hContext, UCHAR address, USHORT data);
BOOL HdqRead16(HANDLE hContext, UCHAR address, USHORT *pData);
VOID HdqSetMode(HANDLE hContext, DWORD mode);

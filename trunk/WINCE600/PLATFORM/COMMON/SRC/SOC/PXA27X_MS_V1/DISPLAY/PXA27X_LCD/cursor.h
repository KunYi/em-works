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
/* 
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
*/

#ifndef __CURSOR_H__
#define __CURSOR_H__

#ifdef __cplusplus
extern "C"{
#endif 

#define CURSOR_XSIZE			32
#define CURSOR_YSIZE			32
#define CURSOR_BYTES			(CURSOR_XSIZE * CURSOR_YSIZE)

#ifdef DEFINE_CURSOR_GLOBALS
#define CURSOR_GLOBALS /* */
#else
#define CURSOR_GLOBALS extern
#endif

CURSOR_GLOBALS DWORD	gxHot;
CURSOR_GLOBALS DWORD	gyHot;
CURSOR_GLOBALS USHORT	gCursorData[CURSOR_BYTES];
CURSOR_GLOBALS USHORT	gCursorMask[CURSOR_BYTES];
CURSOR_GLOBALS unsigned bpp;

#ifdef __cplusplus
}
#endif

#endif // __CURSOR_H__

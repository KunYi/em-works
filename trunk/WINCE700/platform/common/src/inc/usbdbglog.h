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

#ifndef __USBDBGETH_H_
#define __USBDBGETH_H_

#ifdef __cplusplus
extern "C" {
#endif
//
//  Interface: USBDbg interface to USB Composite logging
//
extern BOOL OEMDebugInit_USBComposite( __in LPCSTR pszBootmeName );
extern UINT16 OEMWriteDebugString_USBComposite( __in_bcount(size) UINT8* pData, __in DWORD size);
extern UINT16 OEMReadDebugString_USBComposite( __out_bcount(size) UINT8 *pData, __in UINT16 size);
extern void OEMDebugDeinit_USBComposite(void);

#ifdef __cplusplus
}
#endif

#endif
 

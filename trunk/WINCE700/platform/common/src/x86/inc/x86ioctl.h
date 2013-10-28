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
#ifndef _X86IOCTL_H_
#define _X86IOCTL_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


BOOL x86IoCtlHalDdkCall (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86PowerIoctl (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlHalSetDeviceInfo (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlHalGetCacheInfo (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlHalGetUUID (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlProcessorInfo (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlHalInitRegistry (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlHalInitRTC (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlHalReboot (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtllTiming (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlPostInit (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlQueryDispSettings (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlHalTranslateIrq (UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlHalUpdateMode(UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlGetFastCounter(UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlGetFastCounterInfo(UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);
BOOL x86IoCtlGetPowerDisposition(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _X86IOCTL_H_

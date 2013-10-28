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

#include <oemglobal.h>


// special FPSID values that can be passed to VfpOemInit()
//
#define VFP_FULL_EMULATION_FPSID    0x4d800000
#define VFP_AUTO_DETECT_FPSID       0x00000000


#ifdef __cplusplus
extern "C" {
#endif

typedef void (* PFN_VfpEnableCoproc) (void);

void SaveVfpCtrlRegs(LPDWORD lpExtra, int nMaxRegs);
void RestoreVfpCtrlRegs(LPDWORD lpExtra, int nMaxRegs);
DWORD VfpReadFpsid(void);
DWORD VfpReadMVFR(int index);
void VfpEnableCoproc(void);

BOOL VfpExecuteInstruction(DWORD dwInstr, PEXCEPTION_RECORD pExr, PCONTEXT pContext, DWORD* pdwExcpId, BOOL fInKMode);
BOOL VfpHandleException(ULONG fpexc, EXCEPTION_RECORD* pExr, CONTEXT* pContext, DWORD* pdwExcpId, BOOL fInKMode);

void VfpOemInit(OEMGLOBAL* pOemGlobal, DWORD dwFPSID);

void VfpOemInitEx(OEMGLOBAL* pOemGlobal, DWORD dwFPSID,
    PFN_VfpEnableCoproc         pfnVfpEnableCoproc,
    PFN_SaveRestoreVFPCtrlRegs  pfnSaveVFPCtrlRegs,
    PFN_SaveRestoreVFPCtrlRegs  pfnRestoreVFPCtrlRegs,
    PFN_HandleVFPException      pfnHandleVFPException,
    PFN_IsVFPFeaturePresent     pfnIsVFPFeaturePresent);

#ifdef __cplusplus
}
#endif



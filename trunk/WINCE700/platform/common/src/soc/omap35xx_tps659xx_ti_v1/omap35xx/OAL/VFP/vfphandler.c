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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  vfphandler.c
//
//
#include <windows.h>
#include <nkintr.h>
#include <pkfuncs.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>


#if (_WINCEOSVER==600)
//------------------------------------------------------------------------------
//
//  Function:  OALHandleVFPException
//
//  Handles exceptions from the VFP co-processor
//  Passes the exceptions on to the OS
//
BOOL OALHandleVFPException(
  EXCEPTION_RECORD* er,
  PCONTEXT pctx
)
{
    //  All VPF exceptions handed off to OS kernel
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  OALSaveVFPCtrlRegs
//
//  Saves of extra VFP registers on a context switch.
//  OMAP35XX does not have any "extra" VFP registers;
//  WinCE kernel saves off all OMAP35XX VFP registers
//  on context switches
//
void OALSaveVFPCtrlRegs(
  LPDWORD lpExtra,
  int nMaxRegs
)
{
}

//------------------------------------------------------------------------------
//
//  Function:  OALRestoreVFPCtrlRegs
//
//  Restores extra VFP registers on a context switch.
//  OMAP35XX does not have any "extra" VFP registers;
//  WinCE kernel restores off all OMAP35XX VFP registers
//
void OALRestoreVFPCtrlRegs(
  LPDWORD lpExtra,
  int nMaxRegs
)
{
}

//------------------------------------------------------------------------------
//
//  Function:  OALInitNeonSaveArea
//
//  Initializes the same area for Neon registers saved off during a thread
//  context switch.  Zeros out the memory area
//
void OALInitNeonSaveArea(
  LPBYTE pArea
)
{
}

// NEON has 32 64 bit registers, but half are managed by the CE6 kernel
#define NEON_SAVEAREA_SIZE      (16 * 8)

//------------------------------------------------------------------------------
//
//  Function:  OALVFPInitialize
//
//  Initializes the VFP and NEON co-processors
//
void 
OALVFPInitialize(OEMGLOBAL* pOemGlobal)
{
    //  Enable the VFP    
    OALVFPEnable();

    // Vector Floating Point support
    pOemGlobal->pfnHandleVFPExcp      = OALHandleVFPException;
    pOemGlobal->pfnSaveVFPCtrlRegs    = OALSaveVFPCtrlRegs;
    pOemGlobal->pfnRestoreVFPCtrlRegs = OALRestoreVFPCtrlRegs;

    // Note: VFP and NEON share the same registers, CE6 kernel saves/restores
    //       VFPv2 regs, need to save/restore extra VFPv3 VFP/NEON regs.
    pOemGlobal->pfnInitCoProcRegs    = OALInitNeonSaveArea;
    pOemGlobal->pfnSaveCoProcRegs    = OALSaveNeonRegisters;
    pOemGlobal->pfnRestoreCoProcRegs = OALRestoreNeonRegisters;
    pOemGlobal->cbCoProcRegSize      = NEON_SAVEAREA_SIZE;
    pOemGlobal->fSaveCoProcReg       = TRUE;
}

#endif



#if (_WINCEOSVER==700)


void VfpOemInit(OEMGLOBAL* pOemGlobal, DWORD dwFPSID);

//------------------------------------------------------------------------------
//
//  Function:  OALHandleVFPException
//
//  Handles exceptions from the VFP co-processor
//  Passes the exceptions on to the OS
//
BOOL OALHandleVFPException(
    ULONG fpexc, 
    EXCEPTION_RECORD* pExr, 
    CONTEXT* pContext, 
    DWORD* pdwExcpId, 
    BOOL fInKMode
)
{
    //  All VPF exceptions handed off to OS kernel
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  OALSaveVFPCtrlRegs
//
//  Saves of extra VFP registers on a context switch.
//  OMAP3430 does not have any "extra" VFP registers;
//  WinCE kernel saves off all OMAP3430 VFP registers
//  on context switches
//
void OALSaveVFPCtrlRegs(
  LPDWORD lpExtra,
  int nMaxRegs
)
{
}

//------------------------------------------------------------------------------
//
//  Function:  OALRestoreVFPCtrlRegs
//
//  Restores extra VFP registers on a context switch.
//  OMAP3430 does not have any "extra" VFP registers;
//  WinCE kernel restores off all OMAP3430 VFP registers
//
void OALRestoreVFPCtrlRegs(
  LPDWORD lpExtra,
  int nMaxRegs
)
{
}
//------------------------------------------------------------------------------
//
//  Function:  OALVFPInitialize
//
//  Initializes the VFP and NEON co-processors
//
void 
OALVFPInitialize(OEMGLOBAL* pOemGlobal)
{
    //  Enable the VFP    
    OALVFPEnable();
    
    //  Initialize the kernel VFP
//    VfpOemInit( pOemGlobal, 0 );
    
    pOemGlobal->pfnSaveVFPCtrlRegs    = OALSaveVFPCtrlRegs;
    pOemGlobal->pfnRestoreVFPCtrlRegs = OALRestoreVFPCtrlRegs;
    
}

#endif



//------------------------------------------------------------------------------

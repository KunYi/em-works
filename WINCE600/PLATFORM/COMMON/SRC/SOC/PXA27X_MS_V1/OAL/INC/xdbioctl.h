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
// -----------------------------------------------------------------------
// INTEL CORPORATION MAKES NO WARRANTY OF ANY KIND WITH REGARD TO THIS
// MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// INTEL CORPORATION ASSUMES NO RESPONSIBILITY FOR ANY ERRORS THAT MAY
// APPEAR IN THIS DOCUMENT. INTEL CORPORATION MAKES NO COMMITMENT TO
// UPDATE NOR TO KEEP CURRENT THE INFORMATION CONTAINED IN THIS DOCUMENT.
// -----------------------------------------------------------------------
//
// XDB Browser debugging extension definitions

#ifndef __XDBIOCTL_H__
#define __XDBIOCTL_H__


#include <winioctl.h>

//
// Added OEMIOcontrol codes
//
#define _XSDBG_FID   4050

#define IOCTL_XSDBG_READCOPROCESSOR  CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+0), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_WRITECOPROCESSOR CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+1), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_QUERYCTXREGS     CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+2), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_READCTXREGS      CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+3), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_WRITECTXREGS     CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+4), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_TRACECONTROL      CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+5), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_READTRACE        CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+6), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_SETTASK          CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+7), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_READKMEM         CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+8), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_GETTRACECONFIG   CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+9), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_XSDBG_GETVERSION       CTL_CODE(FILE_DEVICE_HAL, (_XSDBG_FID+10), METHOD_BUFFERED, FILE_ANY_ACCESS)


#define XDBTRACEBUFFERBLOCKSIZE 268

#define XSCBWR_V_MAJOR   1
#define XSCBWR_V_MINOR   2

// Structs for Read CoProc Reg command

//Read Input Buffer
typedef struct
{
    DWORD OpCode;
}XSCBwrRdRegIn, *PXSCBwrRdRegIn;

//Read Output Buffer
typedef struct
{
    DWORD Reg1;
    DWORD Reg2;
}XSCBwrRdRegOut, *PXSCBwrRdRegOut;

//
// Structs for Write CoProc Reg command - No output buffer needed

//Write Input Buffer
typedef struct
{
    DWORD OpCode;
    DWORD Reg1; 
    DWORD Reg2;
}XSCBwrWrteRegIn, *PXSCBwrWrteRegIn;

// Structs for ContextRegs accessing

//Set Task Input Struct - No Output buffer necessary
typedef struct
{
    DWORD ProcessID;
    DWORD ThreadID;
}XSCBwrSetTaskIn, *PXSCBwrSetTaskIn;

// WriteContextRegs Input Struct - No Output struct needed
typedef struct
{
    DWORD RegID;
    DWORD Reg1; 
    DWORD Reg2;
}XSCBwrWrteCxtRegIn, *PXSCBwrWrteCxtRegIn;

// ReadContextRegs Input Struct
typedef struct
{
    DWORD RegID;
}XSCBwrRdCxtRegIn, *PXSCBwrRdCxtRegIn;

// ReadContextRegs Output Struct
typedef struct
{
    DWORD Reg1;
    DWORD Reg2;
}XSCBwrRdCxtRegOut, *PXSCBwrRdCxtRegOut;


// read kernel memory
typedef struct 
{
  DWORD Address;
  DWORD Size;
} XSCBwrRdKMem, *PXSCBwrRdKMem;

//read trace
typedef struct
{
    DWORD ProcId;
    DWORD ThreadId; 
    DWORD Enable;
}XSCBtrcEna, *PXSCBtrcEna;

// version
typedef struct
{
    DWORD Major;
    DWORD Minor;
} XSCBwrVersion, * PXSCBwrVersion;

//  prototypes
//
extern DWORD XSCBwrThreadID;
extern DWORD XSCBwrProcessID;

extern void  XSCBwrExecuteCoProcCode(DWORD, DWORD *, DWORD *);
extern void  XSCBwrInitExecutionTrace( void *, DWORD );
extern void  XSCBwrExecutionTraceOff( DWORD);
extern void  XSCBwrExecutionTraceOn( DWORD);
extern DWORD XSCBwrReadTraceByte(void);
extern void  XSCBwrSaveTrace( LPVOID );
extern void  XSCBwrEnableTrace(void);
extern void  XSCBwrDisableTrace(void);
extern DWORD XSCBwrReadTraceByte(void);
extern void  XSCBwrTraceSetFillOnce (void);
extern void  XSCBwrDebugAbortHandler(void);
extern void  XSCBwrHandleTraceBufferException(void);
extern BOOL  XSCBwrIoControl(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);
extern void  XSCBwrTraceDataAbortHandler(void);

// debug IOCTLs implemented by the BSP
//
extern BOOL OALQueryContextRegistersIoctl(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);       //IOCTL_XSDBG_QUERYCTXREGS
extern BOOL OALReadContextRegisterIoctl(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);       //IOCTL_XSDBG_READCTXREGS
extern BOOL OALWriteContextRegisterIoctl(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);       //IOCTL_XSDBG_WRITECTXREGS

#endif //__XDBIOCTL_H__

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
/*++

Module Name:
    mp_dbg.h

Abstract:
    Debug definitions and macros

Revision History:

Notes:

--*/

#ifndef _MP_DBG_H
#define _MP_DBG_H

//
// Message verbosity: lower values indicate higher urgency
//
#define MP_OFF          0
#define MP_ERROR        1
#define MP_WARN         2
#define MP_TRACE        3
#define MP_INFO         4
#define MP_LOUD         5

// Define a macro so DbgPrint can work on win9x, 32-bit/64-bit NT's
#ifdef _WIN64
#define PTR_FORMAT      "%p"
#else
#define PTR_FORMAT      "%x"
#endif
                            
#if DBG

extern ULONG MPDebugLevel;
extern BOOLEAN          MPInitDone;
extern NDIS_SPIN_LOCK   MPMemoryLock;

#define DBGPRINT(Level, Fmt) \
{ \
    if (Level <= MPDebugLevel) \
    { \
        DbgPrint(NIC_DBG_STRING); \
        DbgPrint Fmt; \
    } \
}

#define DBGPRINT_RAW(Level, Fmt) \
{ \
    if (Level <= MPDebugLevel) \
    { \
      DbgPrint Fmt; \
    } \
}

#define DBGPRINT_S(Status, Fmt) \
{ \
    ULONG dbglevel; \
    if(Status == NDIS_STATUS_SUCCESS || Status == NDIS_STATUS_PENDING) dbglevel = MP_TRACE; \
    else dbglevel = MP_ERROR; \
    DBGPRINT(dbglevel, Fmt); \
}

#define DBGPRINT_UNICODE(Level, UString) \
{ \
    if (Level <= MPDebugLevel) \
    { \
        DbgPrint(NIC_DBG_STRING); \
      mpDbgPrintUnicodeString(UString); \
   } \
}

#undef ASSERT
#define ASSERT(x) if(!(x)) { \
    DBGPRINT(MP_ERROR, ("Assertion failed: %s:%d %s\n", __FILE__, __LINE__, #x)); \
    DbgBreakPoint(); }

//
// The MP_ALLOCATION structure stores all info about MPAuditAllocMemTag
//
typedef struct _MP_ALLOCATION
{
    LIST_ENTRY              List;
    ULONG                   Signature;
    ULONG                   FileNumber;
    ULONG                   LineNumber;
    ULONG                   Size;
    union {
        ULONGLONG           Alignment;        
        UCHAR               UserData;
    };
} MP_ALLOCATION, *PMP_ALLOCATION;

PVOID 
MPAuditAllocMemTag(
    UINT        Size,
    ULONG       FileNumber,
    ULONG       LineNumber,
    NDIS_HANDLE MiniportAdapterHandle
    );

VOID MPAuditFreeMem(
    PVOID       Pointer
    );
    
VOID mpDbgPrintUnicodeString(
    IN  PUNICODE_STRING UnicodeString);


VOID
Dump(
    __in_bcount(cb) CHAR*   p,
    ULONG                   cb,
    BOOLEAN                 fAddress,
    ULONG                   ulGroup );




#else   // !DBG

#define DBGPRINT(Level, Fmt)
#define DBGPRINT_RAW(Level, Fmt)
#define DBGPRINT_S(Status, Fmt)
#define DBGPRINT_UNICODE(Level, UString)
#define Dump(p,cb,fAddress,ulGroup)

#undef ASSERT
#define ASSERT(x)

#endif  // DBG

VOID
DumpLine(
    __in_bcount(cb) CHAR*   p,
    ULONG                   cb,
    BOOLEAN                 fAddress,
    ULONG                   ulGroup );


#endif  // _MP_DBG_H


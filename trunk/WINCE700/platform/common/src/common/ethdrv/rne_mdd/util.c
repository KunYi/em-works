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

Abstract:
    This module is used for buffer managing

--*/
#include <windows.h>
#include <ceddk.h>
#include <halether.h>
#include "rneutil.h"
#define __THIS_FILE__   TEXT("util.c")
#define MyASSERT(x) {if (!(x)) KITLOutputDebugString("ASSERT at File(%s) line (%d)\r\n",__THIS_FILE__, __LINE__);}

#define KERNAL_PCI_ACCESS 1

enum { BUFFER_EMPTY=0, BUFFER_ALLOCATE=1, BUFFER_RESERVED=2 };
#define CHECK_FLAG 0x5a
typedef struct {
    WORD wLength;
    WORD wUserLength;
    BYTE uFlag;
    BYTE uChkFlag;
    BYTE _reserved[2]; // DWORD align
} BufferHeader;


BOOL InitBuffer(BufferDescript * pDesc, PVOID pBuffer,WORD wBLength)
{
    MyASSERT ( (((DWORD)pBuffer)&3)==0);
    MyASSERT(pBuffer!=NULL);
    MyASSERT(wBLength>sizeof(BufferHeader)+sizeof(DWORD));
    MyASSERT(pDesc != NULL);

    pDesc->wBufferSize = ((wBLength + sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD)-sizeof(BufferHeader);
    pDesc->pBufferHead=(PBYTE)pBuffer;
    pDesc->wQEmpty=pDesc->wQUsed=0;
    return TRUE;
}
WORD AvaiableBufferSize ( BufferDescript * pBDescr )
{
    WORD wActUsedC=(pBDescr->wQUsed <= pBDescr->wQEmpty?
        pBDescr->wQUsed + pBDescr-> wBufferSize:pBDescr->wQUsed);
    return (wActUsedC>pBDescr->wQEmpty+sizeof(DWORD)+sizeof(BufferHeader)?wActUsedC - pBDescr->wQEmpty:0);
}
PVOID AllocBuffer (BufferDescript * pBDescr, WORD wLength)
{
    PBYTE pReturnPos = NULL;
    WORD wALength=((wLength+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD)
        +sizeof(BufferHeader); // Actual length required; wrapped to DWORD align

    if (pBDescr->wQEmpty == pBDescr->wQUsed ) { // Empty
        pBDescr->wQEmpty = pBDescr->wQUsed=0; // reset to zero
    }

    if (AvaiableBufferSize(pBDescr)>= wALength) {
        BufferHeader * pBHeader;
        if (wALength +pBDescr->wQEmpty <=pBDescr->wBufferSize ) { // availabe.
            pReturnPos=pBDescr->pBufferHead+pBDescr->wQEmpty;
            pBDescr->wQEmpty +=wALength;
            if (pBDescr->wQEmpty >= pBDescr->wBufferSize)
                pBDescr->wQEmpty=0; // reset to zero.
        }
        else  // DOes not have enough left at end. Check ed.
        if ( pBDescr-> wQUsed > wALength ) { //availabe at Header
            // First Mark end is reserved.
            pBHeader=(BufferHeader *)(pBDescr->pBufferHead+pBDescr->wQEmpty);
            pBHeader ->uFlag= BUFFER_RESERVED;
            pBHeader -> uChkFlag = CHECK_FLAG;
            pBHeader -> wLength= pBDescr->wBufferSize - pBDescr->wQEmpty;
            // Buffer from zero, set up wQEmpty
            pReturnPos = pBDescr->pBufferHead;
            pBDescr->wQEmpty= wALength;
        }
        else
            return NULL;

        pBHeader = (BufferHeader * ) pReturnPos ;
        pBHeader -> uFlag = BUFFER_ALLOCATE;
        pBHeader -> wLength = wALength;
        pBHeader -> wUserLength=wLength;
        pBHeader -> uChkFlag = CHECK_FLAG;
        return (pReturnPos + sizeof(BufferHeader));
    }
    else
        return NULL;// No buffer.
}
BOOL  FreeBuffer(BufferDescript * pBDescr, PVOID pPtr)
{

    BOOL bRet=FALSE;
    BufferHeader * pBHeader = (BufferHeader * )(((PBYTE)pPtr) - sizeof(BufferHeader));

    if ( (PBYTE)pPtr < pBDescr->pBufferHead || (PBYTE)pPtr >= pBDescr->pBufferHead+ pBDescr->wBufferSize) { // Illigle buffer
#pragma warning(suppress:4127)
        MyASSERT(FALSE);
        return FALSE;
    }

//  __try {
        if ( pBHeader -> uChkFlag== CHECK_FLAG && pBHeader -> uFlag == BUFFER_ALLOCATE) {
            pBHeader -> uFlag = BUFFER_RESERVED; // Can be Released
            if (pBDescr->pBufferHead+pBDescr->wQUsed == (PBYTE)pBHeader) {
                while (pBDescr->wQUsed != pBDescr->wQEmpty ) { // free all the buffer if we can
                    if (pBHeader -> uFlag == BUFFER_RESERVED) { // Can be Released
                        MyASSERT(pBHeader -> uChkFlag== CHECK_FLAG);
                        // Advance
                        pBDescr->wQUsed += pBHeader->wLength;
                        if ( pBDescr->wQUsed >= pBDescr->wBufferSize) // Wrap Arrond
                            pBDescr->wQUsed=0;
                        pBHeader->uFlag = BUFFER_EMPTY;
                        bRet=TRUE;
                        // Check Next
                        pBHeader = (BufferHeader * )(pBDescr->pBufferHead+pBDescr->wQUsed);
                    }
                    else
                        break;
                }
            }
            else { // Output of Order
#pragma warning(suppress:4127)
                MyASSERT(FALSE);
                // Try to recover
            }

        }
        else
#pragma warning(suppress:4127)
            MyASSERT(FALSE);

//  }
//  __except ( EXCEPTION_EXECUTE_HANDLER ) {
//      ASSERT(FALSE);
//  }
    MyASSERT(bRet==TRUE);
    return bRet;
}
BOOL GetFirstUsedBuffer(BufferDescript * pBDescr, PVOID *ppPtr,PWORD pwLength)
{
    BufferHeader * pBHeader;
    if (pBDescr->wQEmpty == pBDescr->wQUsed ) { // Empty
        return FALSE;
    }
    pBHeader = (BufferHeader * )(pBDescr->pBufferHead+pBDescr->wQUsed);
    if ( pBHeader -> uChkFlag== CHECK_FLAG && pBHeader -> uFlag == BUFFER_ALLOCATE) { // Good Packet
        if (ppPtr)
            *ppPtr=(PVOID)( pBHeader+1);
        if (pwLength)
            *pwLength = pBHeader->wUserLength;
        return TRUE;
    }
    else {
#pragma warning(suppress:4127)
        MyASSERT(FALSE); // Do know how it could happens But free it.Useless Buffer. Free it.
        FreeBuffer(pBDescr, pBHeader +1 );
        return FALSE;
    }
}

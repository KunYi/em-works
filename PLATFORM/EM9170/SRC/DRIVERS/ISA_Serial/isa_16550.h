//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

isa_16550.h

Abstract:

Definitions for EM9X60 UART(16550) on ISA bus 

Notes: 


--*/
#ifndef __ISA_16550_H__
#define __ISA_16550_H__
#include <ddkreg.h>
#include <serhw.h>

#ifdef __cplusplus
extern "C" {
#endif

	/*
    // 1# And now, all the function prototypes
    VOID SL4_Init(
        PVOID   pHead, //  points to device head
        PUCHAR  pRegBase, // Pointer to 16550 register base
        UINT8   RegStride, // Stride amongst the 16550 registers
        EVENT_FUNC EventCallback, // This callback exists in MDD
        PVOID   pMddHead,   // This is the first parm to callback
        PLOOKUP_TBL pBaudTable  // Pointer to baud rate table
        );
	*/

	// 2#
    BOOL SL4_PostInit(
        PVOID   pHead 
        );

	// 3#
    VOID SL4_Deinit(
        PVOID   pHead //  points to device head
        );

	// 4#
    VOID SL4_Open(
        PVOID   pHead 
        );

	// 5#
    VOID SL4_Close(
        PVOID   pHead
        );

	// 6#
    VOID SL4_ClearDTR(
        PVOID   pHead 
        );

	// 7#
    VOID SL4_SetDTR(
        PVOID   pHead 
        );

	// 8#
    VOID SL4_ClearRTS(
        PVOID   pHead 
        );

	// 9#
    VOID SL4_SetRTS(
        PVOID   pHead 
        );

	// 10#
    VOID SL4_ClearBreak(
        PVOID   pHead 
        );

	// 11#
    VOID SL4_SetBreak(
        PVOID   pHead 
        );

	// 12#
    ULONG SL4_GetByteNumber(
        PVOID   pHead	     
        );

	// 13#
    VOID SL4_DisableXmit(
        PVOID   pHead	
        );

	// 14#
    VOID SL4_EnableXmit(
        PVOID   pHead	
        );

	// 15#
    BOOL SL4_SetBaudRate(
        PVOID   pHead,
        ULONG   BaudRate	//      ULONG representing decimal baud rate.
        );

	// 16#
    BOOL SL4_SetDCB(
        PVOID   pHead,	
        LPDCB   lpDCB       //     Pointer to DCB structure
        );

	// 17#
    ULONG SL4_SetCommTimeouts(
        PVOID   pHead,	
        LPCOMMTIMEOUTS   lpCommTimeouts //  Pointer to CommTimeout structure
        );

	// 18#
    ULONG SL4_GetRxBufferSize(
        PVOID pHead
        );

	// 19#
    PVOID SL4_GetRxStart(
        PVOID   pHead
        );

	// 20#
    INTERRUPT_TYPE SL4_GetInterruptType(
        PVOID pHead
        );

	// 21#
    ULONG SL4_RxIntr(
        PVOID pHead,
        PUCHAR pRxBuffer,       // Pointer to receive buffer
        ULONG *pBufflen         //  In = max bytes to read, out = bytes read
        );

	// 22#
    ULONG SL4_PutBytes(
        PVOID   pHead,
        PUCHAR  pSrc,	    // 	Pointer to bytes to be sent.
        ULONG   NumberOfBytes,  // 	Number of bytes to be sent.
        PULONG  pBytesSent	    // 	Pointer to actual number of bytes put.
        );

	// 23#
    VOID SL4_TxIntr(
        PVOID pHead 
        );

	// 24#
    VOID SL4_LineIntr(
        PVOID pHead
        );

	// 25#
    VOID SL4_OtherIntr(
        PVOID pHead 
        );

	// 26#
    VOID SL4_ModemIntr(
        PVOID pHead 
        );

	// 27#
    ULONG SL4_GetStatus(
        PVOID	pHead,
        LPCOMSTAT	lpStat	// Pointer to LPCOMMSTAT to hold status.
        );

	// 28#
    VOID SL4_Reset(
        PVOID   pHead
        );

	// 29#
    VOID SL4_GetModemStatus(
        PVOID   pHead,
        PULONG  pModemStatus    //  PULONG passed in by user.
        );

	// 30#
    VOID SL4_PurgeComm(
        PVOID   pHead,
        DWORD   fdwAction	    //  Action to take. 
        );

	// 31#
    BOOL SL4_XmitComChar(
        PVOID   pHead,
        UCHAR   ComChar   //  Character to transmit. 
        );

	// 32#
    VOID SL4_PowerOn(
        PVOID   pHead
        );

	// 33#
    VOID SL4_PowerOff(
        PVOID   pHead
        );

	// 34#
    BOOL SL4_Ioctl(
        PVOID pHead,
        DWORD dwCode,
        PBYTE pBufIn,
        DWORD dwLenIn,
        PBYTE pBufOut,
        DWORD dwLenOut,
        PDWORD pdwActualOut);


	// 35#
    VOID SL4_TxIntrEx(
        PVOID pHead,
	    PUCHAR pTxBuffer,          // @parm Pointer to receive buffer
	    ULONG *pBufflen            // @parm In = max bytes to transmit, out = bytes transmitted
        );
    
#ifdef __cplusplus
}
#endif


#endif __ISA_16550_H__

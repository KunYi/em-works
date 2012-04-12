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

ser_ht45b0f.h

Abstract:

Definitions for HT45B0F UART on board

Notes: 


--*/
#ifndef __SER_HT45B0F_H__
#define __SER_HT45B0F_H__
#include <ddkreg.h>
#include <serhw.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	HT45B0F_DAT_READ			0x00
#define	HT45B0F_DAT_WRITE			0x08
#define	HT45B0F_CMD_READ			0x10
#define	HT45B0F_CMD_WRITE			0x18
#define	HT45B0F_REG_USR				0x00
#define	HT45B0F_REG_UCR1			0x01
#define	HT45B0F_REG_UCR2			0x02
#define	HT45B0F_REG_BRG				0x03
#define	HT45B0F_REG_UCR3			0x04
#define	HT45B0F_REG_DAT				0x07			// special flag

#define	HT45B0F_CMD_READ_DATA		0x00			// read UART Data Register
#define	HT45B0F_CMD_WRITE_DATA		0x08			// wriet UART Data Register
#define	HT45B0F_CMD_READ_USR		0x10			// read UART Staus Register
#define	HT45B0F_CMD_READ_UCR1		0x11			// read UART Control Register 1
#define	HT45B0F_CMD_READ_UCR2		0x12			// read UART Control Register 2
#define	HT45B0F_CMD_READ_BRG		0x13			// read Baud Rate Generator
#define	HT45B0F_CMD_READ_UCR3		0x14			// read UART Control Register 3
#define	HT45B0F_CMD_WRITE_UCR1		0x19			// write UART Control Register 1
#define	HT45B0F_CMD_WRITE_UCR2		0x1A			// write UART Control Register 2
#define	HT45B0F_CMD_WRITE_BRG		0x1B			// write Baud Rate Generator
#define	HT45B0F_CMD_WRITE_UCR3		0x1C			// write UART Control Register 3

#define	HT45B0F_USR_TXIF		(1 << 0)			//Tx Data Register Empty
#define	HT45B0F_USR_TIDLE		(1 << 1)			//Tx Shift-Transmitting is Idle
#define	HT45B0F_USR_RXIF		(1 << 2)			//Rx Data Register has available data
#define	HT45B0F_USR_RIDLE		(1 << 3)			//Rx Shift-Receiving is Idle
#define	HT45B0F_USR_OERR		(1 << 4)			//overrun error is detected
#define	HT45B0F_USR_FERR		(1 << 5)			//framing error is detected
#define	HT45B0F_USR_NF			(1 << 6)			//noise is detected
#define HT45B0F_USR_PERR		(1 << 7)			//parity error is detected

#define HT45B0F_UCR1_TX8_0			(0 << 0)			//Transmit data bit 8 for 9-bit data transfer format (write only)
#define HT45B0F_UCR1_TX8_1			(1 << 0)			//Transmit data bit 8 for 9-bit data transfer format (write only)
#define HT45B0F_UCR1_TX8_MASK		(1 << 0)			//Transmit data bit 8 for 9-bit data transfer format (write only)
#define HT45B0F_UCR1_RX8			(1 << 1)			//Receive data bit 8 for 9-bit data transfer format (read only)
#define HT45B0F_UCR1_TX_BREAK		(1 << 2)			//break characters transmit
#define HT45B0F_UCR1_STOPBIT_1		(0 << 3)			//one stop bit format is used
#define HT45B0F_UCR1_STOPBIT_2		(1 << 3)			//two stop bits format is used
#define HT45B0F_UCR1_STOPBIT_MASK	(1 << 3)			//two stop bits format is used
#define HT45B0F_UCR1_PARITY_EVEN	(0 << 4)			//even parity for parity generator
#define HT45B0F_UCR1_PARITY_ODD		(1 << 4)			//odd parity for parity generator
#define HT45B0F_UCR1_PARITY_MASK	(1 << 4)			//odd parity for parity generator
#define HT45B0F_UCR1_PARITY_DIS		(0 << 5)			//parity function is disabled
#define HT45B0F_UCR1_PARITY_EN		(1 << 5)			//parity function is enabled
#define HT45B0F_UCR1_DATABIT_8		(0 << 6)			//8-bit data transfer
#define HT45B0F_UCR1_DATABIT_9		(1 << 6)			//9-bit data transfer
#define HT45B0F_UCR1_DATABIT_MASK	(1 << 6)			//9-bit data transfer
#define HT45B0F_UCR1_UART_DIS		(0 << 7)			//disable UART. TX and RX pins are in the state of high impedance
#define HT45B0F_UCR1_UART_EN		(1 << 7)			//enable UART. TX and RX pins function as UART pins

#define HT45B0F_UCR2_TXIF_INTEN		(1 << 0)			//transmitter empty interrupt is enabled
#define HT45B0F_UCR2_TIDLE_INTEN	(1 << 1)			//transmitter idle interrupt is enabled
#define HT45B0F_UCR2_RX_INTEN		(1 << 2)			//receiver related interrupt is enabled
#define HT45B0F_UCR2_WAKE_DIS		(0 << 3)			//RX pin falling edge wake-up function is disabled
#define HT45B0F_UCR2_WAKE_EN		(1 << 3)			//RX pin falling edge wake-up function is enabled
#define HT45B0F_UCR2_ADDR_DIS		(0 << 4)			//address detect function is disabled
#define HT45B0F_UCR2_ADDR_EN		(1 << 4)			//address detect function is enabled
#define HT45B0F_UCR2_BAUD_DIV64		(0 << 5)			//low speed baud rate
#define HT45B0F_UCR2_BAUD_DIV16		(1 << 5)			//high speed baud rate
#define HT45B0F_UCR2_RX_DIS			(0 << 6)			//UART receiver is disabled
#define HT45B0F_UCR2_RX_EN			(1 << 6)			//UART receiver is enabled
#define HT45B0F_UCR2_TX_DIS			(0 << 7)			//UART transmitter is disabled
#define HT45B0F_UCR2_TX_EN			(1 << 7)			//UART transmitter is enabled

#define HT45B0F_UCR3_USRT			(1 << 7)			//UART reset occurs


	// 2#
    BOOL SL_PostInit(
        PVOID   pHead 
        );

	// 3#
    VOID SL_Deinit(
        PVOID   pHead //  points to device head
        );

	// 4#
    VOID SL_Open(
        PVOID   pHead 
        );

	// 5#
    VOID SL_Close(
        PVOID   pHead
        );

	// 6#
    VOID SL_ClearDTR(
        PVOID   pHead 
        );

	// 7#
    VOID SL_SetDTR(
        PVOID   pHead 
        );

	// 8#
    VOID SL_ClearRTS(
        PVOID   pHead 
        );

	// 9#
    VOID SL_SetRTS(
        PVOID   pHead 
        );

	// 10#
    VOID SL_ClearBreak(
        PVOID   pHead 
        );

	// 11#
    VOID SL_SetBreak(
        PVOID   pHead 
        );

	// 12#
    ULONG SL_GetByteNumber(
        PVOID   pHead	     
        );

	// 13#
    VOID SL_DisableXmit(
        PVOID   pHead	
        );

	// 14#
    VOID SL_EnableXmit(
        PVOID   pHead	
        );

	// 15#
    BOOL SL_SetBaudRate(
        PVOID   pHead,
        ULONG   BaudRate	//      ULONG representing decimal baud rate.
        );

	// 16#
    BOOL SL_SetDCB(
        PVOID   pHead,	
        LPDCB   lpDCB       //     Pointer to DCB structure
        );

	// 17#
	BOOL SL4_SetCommTimeouts(
		PVOID   pHead,	
		LPCOMMTIMEOUTS   lpCommTimeouts //  Pointer to CommTimeout structure
		);

	// 18#
    ULONG SL_GetRxBufferSize(
        PVOID pHead
        );

	// 19#
    PVOID SL_GetRxStart(
        PVOID   pHead
        );

	// 20#
    INTERRUPT_TYPE SL_GetInterruptType(
        PVOID pHead
        );

	// 21#
    ULONG SL_RxIntr(
        PVOID pHead,
        PUCHAR pRxBuffer,       // Pointer to receive buffer
        ULONG *pBufflen         //  In = max bytes to read, out = bytes read
        );

	// 22#
    ULONG SL_PutBytes(
        PVOID   pHead,
        PUCHAR  pSrc,	    // 	Pointer to bytes to be sent.
        ULONG   NumberOfBytes,  // 	Number of bytes to be sent.
        PULONG  pBytesSent	    // 	Pointer to actual number of bytes put.
        );

	// 23#
    VOID SL_TxIntr(
        PVOID pHead 
        );

	// 24#
    VOID SL_LineIntr(
        PVOID pHead
        );

	// 25#
    VOID SL_OtherIntr(
        PVOID pHead 
        );

	// 26#
    VOID SL_ModemIntr(
        PVOID pHead 
        );

	// 27#
    ULONG SL_GetStatus(
        PVOID	pHead,
        LPCOMSTAT	lpStat	// Pointer to LPCOMMSTAT to hold status.
        );

	// 28#
    VOID SL_Reset(
        PVOID   pHead
        );

	// 29#
    VOID SL_GetModemStatus(
        PVOID   pHead,
        PULONG  pModemStatus    //  PULONG passed in by user.
        );

	// 30#
    VOID SL_PurgeComm(
        PVOID   pHead,
        DWORD   fdwAction	    //  Action to take. 
        );

	// 31#
    BOOL SL_XmitComChar(
        PVOID   pHead,
        UCHAR   ComChar   //  Character to transmit. 
        );

	// 32#
    VOID SL_PowerOn(
        PVOID   pHead
        );

	// 33#
    VOID SL_PowerOff(
        PVOID   pHead
        );

	// 34#
    BOOL SL_Ioctl(
        PVOID pHead,
        DWORD dwCode,
        PBYTE pBufIn,
        DWORD dwLenIn,
        PBYTE pBufOut,
        DWORD dwLenOut,
        PDWORD pdwActualOut);


	// 35#
    VOID SL_TxIntrEx(
        PVOID pHead,
	    PUCHAR pTxBuffer,          // @parm Pointer to receive buffer
	    ULONG *pBufflen            // @parm In = max bytes to transmit, out = bytes transmitted
        );
    
#ifdef __cplusplus
}
#endif


#endif //__SER_HT45B0F_H__

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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

Abstract:

    Platform dependent Serial definitions for XSCALE 16550  controller.

Notes: 
--*/
#ifndef __BUL16550_H_
#define __BUL16550_H_
#include <bulverde_uart.h>
#define BUL_SERIAL_FIFO_DEPTH 64
// For Bulverde only
// Extended EIR
//
#define BUL_SERIAL_IER_RTOIE 0x10 
#define BUL_SERIAL_IER_NRZE  0x20
#define BUL_SERIAL_IER_UUE   0x40
#define BUL_SERIAL_IER_DMAE  0x80

// Bulverde 16550 Register Extra Bit.
//Additonal Bulvderde bits
#define SERIAL_FCR_TX_INTR_LEVEL       ((UCHAR)0x08)
#define SERIAL_FCR_TRAILING_BYTES      ((UCHAR)0x10)
#define SERIAL_FCR_PERIPHERAL_BUS      ((UCHAR)0x20)

//For Bulverde only: 
#define BUL_SLOW_IR_SELECT_REGISTER    0x08
//The following macros are used in IR mode only

//To enable Transmitter SIR
#define INFRARED_ISR_TRANSMIT_SIR_ENABLE   (0x01)

//To enable Receiver SIR
#define INFRARED_ISR_RECEIVE_SIR_ENABLE    (0x02)

//To select Transmit Pulse Width
//If 1, the transmit pulse width is 1.6us. 
//If 0, it is 3/16 of a bit time width.
#define INFRARED_ISR_XMODE                 (0x04)

//Transmit Data Polarity:
//If 1, negative pulse is generated for a zero data bit. 
//If 0, a positive pulse is generated for a zero data bit.
#define INFRARED_ISR_TX_DATA_POLARITY      (0x08)

//Receive Data Polarity:
//If 1, a negative pulse is taken as a zero data bit. 
//If 0, a positive pulse is taken as a zero data bit.
#define INFRARED_ISR_RX_DATA_POLARITY      (0x10)

//Defines for workaround

// Use nDSR (intead of nDCD) for automatic detection of docking for serial port
//
// DCD signal is incorrect on some boards: is always LOW and never changes
// This works around is to detect DSR signal instead
// as DSR & CD are jumpered for serial null-modem cable (DB9-DB9).
//
#define AUTODETECT_USING_DSR 1

// Clear alternate function of GPIO#33 to be 0
//
// Need to clear GPIO#33 Altternate Function to be zero,
// because currently in StartUp() of OAL, default value will set GPIO#33 as Input AF 2 (FFDSR).
// 2 FFDSR caused no FFDSR interrupt is generated.
//
#define CLEAR_GPIO33_AF 1


#include "pdd16550.h"

class CBulReg16550: public CReg16550 {
public:
    CBulReg16550(PBULVERDE_UART_REG pRegAddr);
    virtual BOOL    Init() { return (m_pRegAddr!=NULL); } ;
    // We do not virtual Read & Write data because of Performance Concern.
    virtual void    Write_DATA(UINT8 uData) { WRITE_REGISTER_ULONG((PULONG) &(m_pRegAddr->thr_rbr_dll), uData); };
    virtual UINT8   Read_Data() { return (UCHAR)READ_REGISTER_ULONG((PULONG)&(m_pRegAddr->thr_rbr_dll)); } ;
    virtual void    Write_IER(UINT8 uData) { WRITE_REGISTER_ULONG((PULONG)&(m_pRegAddr->ier_dlh), uData); };
    virtual UINT8   Read_IER() { return (UCHAR)READ_REGISTER_ULONG((PULONG)&(m_pRegAddr->ier_dlh)); };
    virtual void    Write_FCR(UINT8 uData) { WRITE_REGISTER_ULONG( (PULONG)&(m_pRegAddr->iir_fcr), uData);m_FCR = uData; };
    virtual UINT8   Read_FCR() { return m_FCR; };
    virtual UINT8   Read_IIR() { return (UCHAR)READ_REGISTER_ULONG( (PULONG)&(m_pRegAddr->iir_fcr)) ;};
    virtual void    Write_LCR(UINT8 uData) { WRITE_REGISTER_ULONG( (PULONG)&(m_pRegAddr->lcr), uData);};
    virtual UINT8   Read_LCR() { return (UCHAR)READ_REGISTER_ULONG( (PULONG)&(m_pRegAddr->lcr));};
    virtual void    Write_MCR(UINT8 uData) { WRITE_REGISTER_ULONG( (PULONG)&(m_pRegAddr->mcr), uData);};
    virtual UINT8   Read_MCR() { return (UCHAR) READ_REGISTER_ULONG( (PULONG)&(m_pRegAddr->mcr) );};
    virtual void    Write_LSR(UINT8 uData) { WRITE_REGISTER_ULONG( (PULONG)&(m_pRegAddr->lsr), uData);};
    virtual UINT8   Read_LSR() { return (UCHAR) READ_REGISTER_ULONG( (PULONG)&(m_pRegAddr->lsr) );};
    virtual void    Write_MSR(UINT8 uData) { WRITE_REGISTER_ULONG( (PULONG)&(m_pRegAddr->msr), uData) ; };
    virtual UINT8   Read_MSR() { return (UCHAR) READ_REGISTER_ULONG( (PULONG)&(m_pRegAddr->msr)); };
    virtual void    Write_SCR(UINT8 uData) { WRITE_REGISTER_ULONG( (PULONG)&(m_pRegAddr->scr), uData );};
    virtual UINT8   Read_SCR() { return (UCHAR) READ_REGISTER_ULONG( (PULONG)&(m_pRegAddr->scr)); };

    virtual void    Backup();
    virtual void    Restore();
// Here is specific for the XSCALE IR
    virtual void    Write_SIR(UINT8 uData) { WRITE_REGISTER_ULONG( (PULONG)&(m_pRegAddr->irdasel), uData);};
    virtual UINT8    Read_SIR() { return (UCHAR) READ_REGISTER_ULONG( (PULONG)&(m_pRegAddr->irdasel)); };
// Here is specific for the Bulverde
    virtual UINT8   Read_FIOR() { return (UCHAR) READ_REGISTER_ULONG( (PULONG)&(m_pRegAddr->fior)); };
    virtual UINT8   Read_ABR() { return (UCHAR) READ_REGISTER_ULONG( (PULONG)&(m_pRegAddr->abr));};
    virtual void    Write_ABR(UINT8 uData) {  WRITE_REGISTER_ULONG( (PULONG)&(m_pRegAddr->abr),uData);};
private:
    PBULVERDE_UART_REG m_pRegAddr;
    BYTE    m_SIRBackup;

};

#define PC_REG_SERIALIRCONNECTED_VAL_NAME TEXT("IRConnected")
#define PC_REG_SERIALIRCONNECTED_VAL_LEN  sizeof(DWORD)
class CBulPdd16550: public CPdd16550 {
public:
    CBulPdd16550 (LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj);
    virtual ~CBulPdd16550();
    virtual BOOL Init();
    virtual void PostInit();
    virtual BOOL    Open();
    virtual BOOL    Close();
    virtual BOOL MapHardware();
    virtual BOOL CreateHardwareAccess();
    CBulReg16550 *GetRegister() { return (CBulReg16550 *)m_pReg16550; };
protected:
    volatile PBULVERDE_UART_REG     m_pBaseAddress;
    volatile PBULVERDE_GPIO_REG     m_pGPIOReg;
    volatile PBULVERDE_CLKMGR_REG   m_pDCCLKReg;
// Power Callback.
    virtual void    SerialRegisterBackup() ;
// Received.
public:
    virtual DWORD   GetWaterMark();
    virtual BYTE    GetWaterMarkBit();
// Xmit
public:
    virtual BOOL    EnableXmitInterrupt(BOOL fEnable);
    virtual void    XmitInterruptHandler(PUCHAR pTxBuffer, ULONG *pBuffLen);
    virtual void    XmitComChar(UCHAR ComChar);
    virtual DWORD   GetWriteableSize();
// IR 
    virtual BOOL    InitIR(BOOL bSet);
    virtual void    SetOutputMode(BOOL UseIR, BOOL Use9Pin);
// Configuration 
    virtual BOOL    GetDivisorOfRate(ULONG BaudRate,PULONG pulDivisor);
    virtual BOOL    SetDCB(LPDCB lpDCB);
protected:
    BOOL Enable_IR_Rx_Tx(BOOL Rxenable, BOOL Txenable);
    BOOL m_fIRConnected;
};

#endif

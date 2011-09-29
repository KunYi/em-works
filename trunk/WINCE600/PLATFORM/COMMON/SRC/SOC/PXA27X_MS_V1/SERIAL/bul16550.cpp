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

BUL16550.cpp

Abstract:  

Type definitions and data for the serial port driver


Notes: 


--*/
#include <windows.h>
#include <types.h>
#include <ceddk.h>

#include <ddkreg.h>
#include <serhw.h>
#include <hw16550.h>
#include <Serdbg.h>

#include <bulverde.h>
#include <xllp_gpio.h>
#include "BUL16550.h"

CBulReg16550::CBulReg16550(PBULVERDE_UART_REG pRegAddr)
:   CReg16550 ( (PBYTE)pRegAddr, sizeof(DWORD))
{
    m_pRegAddr = (PBULVERDE_UART_REG)pRegAddr;
}

void CBulReg16550::Backup()
{
    CReg16550::Backup();
    m_SIRBackup = Read_SIR();
    // Backup happens during power off. So We need turn off the IRDA
    Write_SIR(0);
}
void CBulReg16550::Restore()
{
    if (m_fIsBackedUp) {
        CReg16550::Restore();
        Write_SIR(m_SIRBackup);
    }
}
CBulPdd16550::CBulPdd16550(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj)
: CPdd16550(lpActivePath, pMdd, pHwObj)
{
    m_pBaseAddress = NULL;
    m_pGPIOReg = NULL;
    m_pDCCLKReg = NULL;
    m_fIRConnected = FALSE;
}
CBulPdd16550::~CBulPdd16550()
{
    if (m_pBaseAddress) 
        MmUnmapIoSpace(m_pBaseAddress,0);
    
    if (m_pGPIOReg)
        MmUnmapIoSpace(m_pGPIOReg,0);
    
    if (m_pDCCLKReg)
        MmUnmapIoSpace(m_pDCCLKReg,0);
}
BOOL CBulPdd16550::Init()
{
    // IST Setup . This is only need when Root Bus driver does not allocate any resource for this driver.
    DDKISRINFO ddi;
    ddi.dwIrq = MAXDWORD; ddi.dwSysintr = MAXDWORD;
    if (GetIsrInfo(&ddi)==ERROR_SUCCESS && (ddi.dwSysintr==0 || ddi.dwSysintr>=0xff )) { // We need rework Interrupt
        if (ddi.dwIrq!=0 && ddi.dwIrq < 0xff &&
                KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &ddi.dwIrq, sizeof(UINT32), &ddi.dwSysintr, sizeof(UINT32), NULL)) {
            // We use correct IRQ to allocate SYSINTR from system.
            // We can put it back to the registry.
            RegSetValueEx(DEVLOAD_SYSINTR_VALNAME,REG_DWORD,(PBYTE)&ddi.dwSysintr, sizeof(UINT32));
        }
    }
    DWORD dwIRConnected=0;
    if (!GetRegValue(PC_REG_SERIALIRCONNECTED_VAL_NAME,(PBYTE)&dwIRConnected,PC_REG_SERIALIRCONNECTED_VAL_LEN)) {
        dwIRConnected=0;
    }
    m_fIRConnected = (dwIRConnected!=0);
    BOOL fReturn ;
    if ((fReturn = CPdd16550::Init())== TRUE) {
        if (!GetRegValue(PC_REG_SERIALWATERMARK_VAL_NAME,(PBYTE)&m_dwWaterMark,sizeof(DWORD))) {
            m_dwWaterMark = 32; // Default to Half of FIFO.
        }
        if (m_pReg16550) {
            // We have to initial FCR because it got bus specific content inside.
            m_pReg16550->Write_FCR(0);
#ifdef UART_32_BIT_PERIPHERAL_BUS
            m_pReg16550->Write_FCR(SERIAL_FCR_PERIPHERAL_BUS);
#endif
            GetRegister()->Write_ABR(0);
        }
    }
    return fReturn;
}
BOOL  CBulPdd16550::Open()
{
    BOOL bReturn =  CPdd16550::Open();
    if (bReturn) {        
        // Need Enable UART.
        m_HardwareLock.Lock();   
        GetRegister()->Write_IER(GetRegister()->Read_IER() | BUL_SERIAL_IER_UUE|BUL_SERIAL_IER_RTOIE);
        InitIR(TRUE);
        m_HardwareLock.Unlock();

    }
    return bReturn;
}
BOOL  CBulPdd16550::Close()
{
    BOOL bReturn =  CPdd16550::Close();
    if (bReturn) { 
        // Need Disable UART.
        m_HardwareLock.Lock();   
        InitIR(FALSE);
        GetRegister()->Write_IER(GetRegister()->Read_IER() &~( BUL_SERIAL_IER_UUE|BUL_SERIAL_IER_RTOIE));
        m_HardwareLock.Unlock();
    }
    return bReturn;
}

BOOL CBulPdd16550::MapHardware()
{
    if (!GetRegValue(PC_REG_REGSTRIDE_VAL_NAME,(PBYTE)&m_dwRegStride, PC_REG_REGSTRIDE_VAL_LEN)) {
        m_dwRegStride = 4;// Default is 4 for Bulverde
    }
    // Get IO Window From Registry
    if (m_pBaseAddress == NULL ) {
        DDKWINDOWINFO dwi;
        if ( GetWindowInfo( &dwi)==ERROR_SUCCESS &&
                dwi.dwNumMemWindows >= 1 &&  dwi.memWindows[0].dwBase != 0 && dwi.memWindows[0].dwLen >=  m_dwRegStride * 0x10)  {
            PHYSICAL_ADDRESS ioPhysicalBase = { dwi.memWindows[0].dwBase, 0};
            m_pBaseAddress = (PBULVERDE_UART_REG)MmMapIoSpace(ioPhysicalBase, dwi.memWindows[0].dwLen,FALSE);
        }
    }
    if (m_pGPIOReg == NULL) {
        PHYSICAL_ADDRESS ioPhysicalBase = {BULVERDE_BASE_REG_PA_GPIO,0};
        m_pGPIOReg = (PBULVERDE_GPIO_REG)MmMapIoSpace(ioPhysicalBase, sizeof(BULVERDE_GPIO_REG),FALSE);
    }
    if (m_pDCCLKReg == NULL ) {
        PHYSICAL_ADDRESS ioPhysicalBase = {BULVERDE_BASE_REG_PA_CLKMGR, 0 };
        m_pDCCLKReg = (PBULVERDE_CLKMGR_REG)MmMapIoSpace(ioPhysicalBase, sizeof(BULVERDE_CLKMGR_REG),FALSE);
    }    
    DEBUGMSG(ZONE_INIT,(TEXT("CBulPdd16550::MapHardware: m_pBaseAddress:%x m_pGPIOReg:%x m_pDCCLKReg:%x\r\n"),
        m_pBaseAddress,m_pGPIOReg,m_pDCCLKReg));
    
    return(m_pBaseAddress!=NULL && m_pGPIOReg!=NULL && m_pDCCLKReg!=NULL); 
}
BOOL CBulPdd16550::CreateHardwareAccess()
{
    if (m_pReg16550)
        return TRUE;
    if (m_pBaseAddress!=NULL) {
        m_pReg16550 = new CBulReg16550(m_pBaseAddress);
        if (m_pReg16550 && !m_pReg16550->Init()) { // FALSE.
            delete m_pReg16550 ;
            m_pReg16550 = NULL;
        }
            
    }
    return (m_pReg16550!=NULL);
}
static PAIRS s_HighWaterPairs[] = {
    {SERIAL_1_BYTE_HIGH_WATER, 1},
    {SERIAL_4_BYTE_HIGH_WATER, 8},
    {SERIAL_8_BYTE_HIGH_WATER, 16},
    {SERIAL_14_BYTE_HIGH_WATER, 32}
};

BYTE  CBulPdd16550::GetWaterMarkBit()
{
    BYTE bReturnKey = (BYTE)s_HighWaterPairs[0].Key;
    for (DWORD dwIndex=dim(s_HighWaterPairs)-1;dwIndex!=0; dwIndex --) {
        if (m_dwWaterMark>=s_HighWaterPairs[dwIndex].AssociatedValue) {
            bReturnKey = (BYTE)s_HighWaterPairs[dwIndex].Key;
            break;
        }
    }
    return bReturnKey;
}
DWORD CBulPdd16550::GetWaterMark()
{
    BYTE bReturnValue = (BYTE)s_HighWaterPairs[0].AssociatedValue;
    for (DWORD dwIndex=dim(s_HighWaterPairs)-1;dwIndex!=0; dwIndex --) {
        if (m_dwWaterMark>=s_HighWaterPairs[dwIndex].AssociatedValue) {
            bReturnValue = (BYTE)s_HighWaterPairs[dwIndex].AssociatedValue;
            break;
        }
    }
    return bReturnValue;
}
BOOL CBulPdd16550::GetDivisorOfRate(ULONG BaudRate,PULONG pulDivisor)
{
static const
PAIRS  s_LS_BaudPairs[] = {
    {50,        18432},
    {75,        12288},
    {150,       6144},
    {300,       3072},
    {600,       1536},
    {1200,      768},
    {1800,      512},
    {2400,      384},
    {3600,      256},
    {4800,      192},
    {7200,      128},
    {9600,      96},
    {12800,     72},
    {14400,     64},
    {19200,     48},
    {23040,     40},
    {28800,     32},
    {38400,     24},
    {57600,     16},
    {115200,    8},
    {230400,    4},
    {921600,    1}
};


    for (DWORD dwIndex =0 ; dwIndex <dim(s_LS_BaudPairs) && s_LS_BaudPairs[dwIndex].Key<=BaudRate; dwIndex ++) {
        if (s_LS_BaudPairs[dwIndex].Key== BaudRate){
            if (pulDivisor)
                *pulDivisor = s_LS_BaudPairs[dwIndex].AssociatedValue;
            return TRUE;
        }
    }
    return FALSE;    
}

BOOL CBulPdd16550::Enable_IR_Rx_Tx(BOOL Rxenable, BOOL Txenable)
{
    if (m_fIRConnected) {
        BYTE irdasel=0;
        //Enable/Disable IR receiver/transmitter (decoder/encoder) mask
        irdasel |= (Rxenable?INFRARED_ISR_RECEIVE_SIR_ENABLE:0 );
        irdasel |= (Txenable?INFRARED_ISR_TRANSMIT_SIR_ENABLE:0);
        m_HardwareLock.Lock();        
        if ((GetRegister()->Read_SIR()&(INFRARED_ISR_RECEIVE_SIR_ENABLE | INFRARED_ISR_TRANSMIT_SIR_ENABLE))!= irdasel) {
            //Disable UART In order to make the changes.
            BYTE bIER = GetRegister()->Read_IER();
            GetRegister()->Write_IER( bIER & ~BUL_SERIAL_IER_UUE);
            BYTE bIrStatus = GetRegister()->Read_SIR() & ~(INFRARED_ISR_RECEIVE_SIR_ENABLE|INFRARED_ISR_TRANSMIT_SIR_ENABLE);
            bIrStatus |= irdasel;
            GetRegister()->Write_SIR(bIrStatus);
            GetRegister()->Write_IER(bIER);
        }
        m_HardwareLock.Unlock();
        return TRUE;
    }
    else
        return TRUE;
    //Restore to original value of IER including Uart Unit setting
}
BOOL CBulPdd16550::InitIR(BOOL bSet)
{
    //Updating our copy of IER before disabling UART
    m_HardwareLock.Lock();        
    BYTE bIER = GetRegister()->Read_IER();

    //Disable the UART for sure
    GetRegister()->Write_IER(bIER & ~BUL_SERIAL_IER_UUE);
    if (m_fIRConnected && bSet) {
        //Updating our copy of pIRDASEL before enabling/disabling IR
        BYTE uIRSel = GetRegister()->Read_SIR() & ~INFRARED_ISR_TX_DATA_POLARITY;
        uIRSel |=  (INFRARED_ISR_XMODE| INFRARED_ISR_RX_DATA_POLARITY);
        GetRegister()->Write_SIR( uIRSel );
        Enable_IR_Rx_Tx( TRUE, FALSE);

    }
    else {
        GetRegister()->Write_SIR(0);
    }

    //Restoring to the original IER (including UART unit state).
    GetRegister()->Write_IER(bIER);
    m_HardwareLock.Unlock();
    return m_fIRConnected;
    
}
void CBulPdd16550::SetOutputMode(BOOL fUseIR, BOOL fUse9Pin)
{
    CPdd16550::SetOutputMode(fUseIR,fUse9Pin);
    if (m_fIREnable) {
        InitIR(TRUE);
        Enable_IR_Rx_Tx(TRUE,FALSE);
    }
    else
        InitIR(FALSE);
}
DWORD   CBulPdd16550::GetWriteableSize()
{
    BYTE bLSR = GetRegister()->Read_LSR( );
    DWORD dwByteCanWrite = 0;
    if (bLSR & SERIAL_LSR_TEMT) {
        dwByteCanWrite = (m_XmitFifoEnable?BUL_SERIAL_FIFO_DEPTH:1);
    }
    else 
    if (bLSR & SERIAL_LSR_THRE) {
        dwByteCanWrite = (m_XmitFifoEnable?(BUL_SERIAL_FIFO_DEPTH/2):1);
    }
    return dwByteCanWrite;
}
BOOL  CBulPdd16550::EnableXmitInterrupt(BOOL fEnable)
{
    if (m_fIREnable) {
        Enable_IR_Rx_Tx(!fEnable,fEnable);
    }
    return CPdd16550::EnableXmitInterrupt(fEnable);
}
void CBulPdd16550::XmitInterruptHandler(PUCHAR pTxBuffer, ULONG *pBuffLen)
{
    if (!(pTxBuffer!=NULL && pBuffLen!=NULL && *pBuffLen!=NULL)) { // Transfer End.
        // The Xmit Empty interrupt can be generated by two sources.
        // One is Xmit FIFO half empty, other is empty.
        // We have to make sure the FIFO is completely empty before return this.
        // Because after this return, user may shut down hardware or change baudrate. 
        for (DWORD dwIndex = 0; dwIndex< 500; dwIndex++) {
            if ((GetRegister()->Read_LSR( ) & SERIAL_LSR_TEMT) == 0 ) 
                Sleep(1);
            else
                break;
        }
    }
    if (m_fIREnable) {
        if (pTxBuffer!=NULL && pBuffLen!=NULL && *pBuffLen!=NULL) 
            Enable_IR_Rx_Tx(FALSE,TRUE);
        else
            Enable_IR_Rx_Tx(TRUE,FALSE);
    }
    CPdd16550::XmitInterruptHandler(pTxBuffer,pBuffLen);
}
void    CBulPdd16550::XmitComChar(UCHAR ComChar)
{
    if (m_fIREnable) {
        Enable_IR_Rx_Tx(FALSE,TRUE);
    }
    CPdd16550::XmitComChar(ComChar);
}
BOOL CBulPdd16550::SetDCB(LPDCB lpDCB)
{
    m_HardwareLock.Lock();
    //Disable the UART for sure
    Enable_IR_Rx_Tx(FALSE,FALSE);
    BOOL bReturn = CPdd16550::SetDCB(lpDCB);
    Enable_IR_Rx_Tx(TRUE,FALSE);
    m_HardwareLock.Unlock();        
    return bReturn;
}
void    CBulPdd16550::SerialRegisterBackup()
{
    CPdd16550::SerialRegisterBackup();
    //Actually power down UART and disable all interrupts
    GetRegister()->Write_IER(GetRegister()->Read_IER() &~BUL_SERIAL_IER_UUE);
}

void CBulPdd16550::PostInit()
{
    CPdd16550::PostInit();
}


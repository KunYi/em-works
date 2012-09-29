//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  externs.h
//
//   Header file for iMX51 FIR device.
//
//------------------------------------------------------------------------------
#ifndef __EXTERNS_H__
#define __EXTERNS_H__

//------------------------------------------------------------------------------
// Defines

// Externs for global data objects
extern pFirDevice_t gFirstFirDevice;
extern PCSP_FIRI_REG g_pVFiriReg;
extern PCSP_UART_REG g_pVSIRReg;
extern baudRateInfo supportedBaudRateTable[NUM_BAUDRATES];


//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Functions

extern VOID BSPIrdaEnable(BOOL bEnable);
extern VOID BSPIrdaSetMode(irMode_c mode);
extern UCHAR BSPUartCalRFDIV( ULONG* pRefFreq );
extern BOOL BSPUartGetIrq(ULONG HWAddr, PULONG pIrq);
extern BOOL BSPUartGetType(ULONG HWAddr, uartType_c * pType);
extern BOOL BSPUartEnableClock(ULONG HWAddr, BOOL bEnable);
extern BOOL BSPFirEnableClock(BOOL bEnable);
extern BOOL BSPFirSetClock(void);
extern BOOL BSPFirSetIOMUX(void);
extern BOOL BSPFirSetSIRIOMUX(void);
extern UINT8 BSPFirGetChannelPriority(void);
extern VOID CspSerialSetMux(PCSP_UART_REG pUartReg);
extern VOID CspSerialSetTxIvt(PCSP_UART_REG pUartReg);


NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
BOOLEAN IrFirCheckForHang(IN NDIS_HANDLE MiniportAdapterContext);
VOID IrFirDisableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
VOID IrFirEnableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
VOID IrFirHalt(IN NDIS_HANDLE MiniportAdapterContext);
VOID IrFirHandleInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
NDIS_STATUS IrFirInitialize(OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext);
NDIS_STATUS IrFirQueryInformation(IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded);

NDIS_STATUS IrFirSetInformation(IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded);

NDIS_STATUS IrFirReset(PBOOLEAN AddressingReset,
    NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS IrFirSend(IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags);

VOID IrFirReturnPacket(NDIS_HANDLE MiniportAdapterContext, PNDIS_PACKET Packet);

BOOLEAN DeliverFullBuffers(pFirDevice_t thisDev);
VOID ReturnPacketHandler(NDIS_HANDLE MiniportAdapterContext, PNDIS_PACKET Packet);


//  Externs for FIR control functions, from fir.cpp
BOOLEAN FirInitialize(pFirDevice_t thisDev);
VOID FirDeinitialize(pFirDevice_t thisDev);
VOID FirClose(pFirDevice_t thisDev);
BOOLEAN FirOpen(pFirDevice_t thisDev);
VOID FirDisableInterrupt(pFirDevice_t thisDev);
VOID FirEnableInterrupt(pFirDevice_t thisDev);
VOID FirInterruptHandler(pFirDevice_t thisDev);
NDIS_STATUS FirSend(pFirDevice_t thisDev, PNDIS_PACKET Packet);
NDIS_STATUS FirSendPacketQ(pFirDevice_t thisDev, BOOLEAN firstBufIsPending);
VOID FirReceive(pFirDevice_t thisDev);
baudRates FirSetSpeed(pFirDevice_t thisDev );

//  Externs for SIR control functions, from sir.cpp
BOOLEAN SirInitialize(pFirDevice_t thisDev);
VOID SirDeInitialize(pFirDevice_t thisDev);
VOID SirClose(pFirDevice_t thisDev);
BOOLEAN SirOpen(pFirDevice_t thisDev);
VOID SirEnableInterrupt(pFirDevice_t thisDev);
VOID SirDisableInterrupt(pFirDevice_t thisDev);
VOID SirInterruptHandler(pFirDevice_t thisDev);
NDIS_STATUS SirSend(pFirDevice_t thisDev, PNDIS_PACKET Packet);
NDIS_STATUS SirSendPacketQ(pFirDevice_t thisDev, BOOLEAN firstBufIsPending);
VOID SirReceive(pFirDevice_t thisDev);
baudRates SirSetSpeed(pFirDevice_t thisDev);

//  Externs for software resouces control functions, from resource.cpp
BOOLEAN OpenDevice(pFirDevice_t thisDev);
VOID CloseDevice(pFirDevice_t thisDev);
pFirDevice_t NewDevice();
VOID FreeDevice(pFirDevice_t dev);
PVOID MyMemAlloc( UINT size );
ULONG CspFirGetSIRBaseRegAddr();
ULONG CspFirGetFIRBaseRegAddr();
VOID CspFirConfig (PCSP_FIRI_REG pFirReg);

//  Externs for changing IR speed, from settings.cpp
BOOLEAN SetSpeed(pFirDevice_t thisDev);
void InsertBufferSorted( PLIST_ENTRY Head, rcvBuffer *rcvBuf );


//  Externs for other functions
USHORT ComputeFCS(UCHAR *data, UINT dataLen);
PLIST_ENTRY MyRemoveHeadList(IN PLIST_ENTRY ListHead);
PNDIS_IRDA_PACKET_INFO GetPacketInfo(PNDIS_PACKET packet);
VOID QueueReceivePacket(pFirDevice_t thisDev, PUCHAR data, UINT dataLen, BOOLEAN IsFIR);

#endif //  __EXTERNS_H__

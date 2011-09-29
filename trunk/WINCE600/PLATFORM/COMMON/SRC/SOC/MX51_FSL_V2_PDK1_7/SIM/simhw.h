//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Header:  simhw.h
//
//------------------------------------------------------------------------------


#ifndef __SIM_HW_H__
#define __SIM_HW_H__

#if __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// Defines

#define RFU     0       // Reserve for future use

#define MHz     1000000

#define FI_DEFAULT      0x10

#define COLDRESET_CWT       0x2574
#define COLDRESET_DELAY1     0x200
#define COLDRESET_DELAY2     0x9c40

#define RETRYS     100

#define CLK_SEL_DISABLE     0
#define CLK_SEL_CARD     1
#define CLK_SEL_RECEIVER     2
#define CLK_SEL_TRANSMIT     3


//------------------------------------------------------------------------------
// Types

//port select
typedef enum _PORT_SELECT {
    PORT0 = 0,
    PORT1
} PORT_SELECT;

//Clock rate factor structure
typedef struct {
    UINT8   FI;     //Bit form in ATR
    INT16   Fi;     //Clock rate conversion factor
    INT16   f;      //maximum frequency for the factor
}ClockRateFactor;

//Baud rate factor structure
typedef struct {
    UINT8   DI;     //Bit form in ATR
    UINT8   Di;     //Baud rate adjustment factor
}BaudRateFactor;


//------------------------------------------------------------------------------
// Functions

PVOID SIM_InternalMapRegisterAddresses(DWORD iobase);
VOID SIM_InternalUnMapRegisterAddresses(PCSP_SIM_REG pSIMReg);
void SIM_Init(PCSP_SIM_REG pSIMReg);
void SIM_Open(PCSP_SIM_REG pSIMReg);
void SIM_Close(PCSP_SIM_REG pSIMReg);
NTSTATUS SIM_ColdReset(PCSP_SIM_REG pSIMReg, UCHAR* Buffer, ULONG* BufferLength);
NTSTATUS SIM_WarmReset(PCSP_SIM_REG pSIMReg);
NTSTATUS SIM_Deactivate(PCSP_SIM_REG pSIMReg);
NTSTATUS SIM_WriteData(PCSP_SIM_REG pSIMReg, UCHAR *Buffer, ULONG Length);
NTSTATUS SIM_ReadData(PCSP_SIM_REG pSIMReg, UCHAR *Buffer, ULONG Length);
BOOL SIM_ConfigCLK(PCSP_SIM_REG pSIMReg, UINT8 Fi, UINT8 Di);
BOOL SIM_ConfigTime(PCSP_SIM_REG pSIMReg, UINT8 N, UINT8 WI);
BOOL SIM_CheckCard(PCSP_SIM_REG pSIMReg);
void SIM_FlushFIFO(PCSP_SIM_REG pSIMReg);
void SIM_SelectPort(DWORD Port_select);

#ifdef __cplusplus
}
#endif

#endif  //__SIM_HW_H__

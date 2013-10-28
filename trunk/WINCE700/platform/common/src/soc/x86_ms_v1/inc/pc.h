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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// in/out not protyped anywhere for debug builds.  Do it here.
int __cdecl _inp(unsigned short);
unsigned short __cdecl _inpw(unsigned short);
unsigned long __cdecl _inpd(unsigned short);

int __cdecl _outp(unsigned short, int);
unsigned short __cdecl _outpw(unsigned short, unsigned short);
unsigned long __cdecl _outpd(unsigned short, unsigned long);

// We assume the presence of 8254.  undefine this if you have an 8253
#define ASSUME_8254 1

// Defines for PC Platform control registers

VOID PICEnableInterrupt(UCHAR ucInterrupt, BOOL bEnabled);

#define INTR_TIMER0         0
#define INTR_KEYBOARD       1
#define INTR_PIC2           2
#define INTR_FLOPPY         6
#define INTR_RTC            8
#define INTR_MOUSE          12
#define INTR_COPROC         13

#define INTR_MAXIMUM        15

#define INTR_UNDEFINED      (-1)

#define PCINTR_RTCSHORT     (SYSINTR_FIRMWARE+0)


// Define the varios CMOS parameters used for RTC
#define CMOS_ADDR     0x70
#define CMOS_DATA     0x71

#define RTC_ADDR_MASK 0x80
#define RTC_SECOND    0x00
#define RTC_ALRM_SECOND    0x01
#define RTC_MINUTE    0x02
#define RTC_ALRM_MINUTE    0x03
#define RTC_HOUR      0x04
#define RTC_ALRM_HOUR      0x05
#define RTC_DO_WEEK   0x06
#define RTC_DO_MONTH  0x07
#define RTC_MONTH     0x08
#define RTC_YEAR      0x09


#define RTC_STATUS_A  0x0A
#define RTC_SRA_UIP   0x80
#define RTC_SRA_BASE  0x70
#define RTC_SRA_RATE  0x0F

#define RTC_STATUS_B  0x0B
#define RTC_SRB_UPDT  0x80  // Update - 0=enable, 1=disable
#define RTC_SRB_PI    0x40
#define RTC_SRB_AI    0x20
#define RTC_SRB_UI    0x10
#define RTC_SRB_SQU   0x08
#define RTC_SRB_DM    0x04   // Data Mode - 1=binary, 0=BCD
#define RTC_SRB_24HR  0x02
#define RTC_SRB_DLS   0x01

#define RTC_STATUS_C  0x0C
#define RTC_SRC_IRQ   0x80
#define RTC_SRC_PS    0x40
#define RTC_SRC_AS    0x20
#define RTC_SRC_US    0x10
#define RTC_SRC_RSVD  0x0F

#define DECODE_BCD(b)  (((b)>>4)*10 + ((b) & 0xF))
#define CREATE_BCD(b)  ( (((UCHAR)(b)/10)<<4) | ((UCHAR)(b)%10) )

// Some Serial port defines used for the debug serial routines
#define COM1_BASE           0x03F8
#define COM2_BASE           0x02F8
#define COM3_BASE           0x03E8
#define COM4_BASE           0x02E8

#define comTxBuffer         0x00
#define comRxBuffer         0x00
#define comDivisorLow       0x00
#define comDivisorHigh      0x01
#define comIntEnable        0x01
#define comIntId            0x02
#define comFIFOControl      0x02
#define comLineControl      0x03
#define comModemControl     0x04
#define comLineStatus       0x05
#define comModemStatus      0x06


// For the PC, we have some common routines for reading/writing the RTC.
BYTE ReadCMOSTimeReg(BYTE bAddr, BYTE bCmosOffset);
void GetRealTimeFromCMOS(SYSTEMTIME *pst);

#ifdef __cplusplus
}
#endif  // __cplusplus


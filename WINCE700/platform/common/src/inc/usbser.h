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
#ifndef __USBDBGSERIAL_H_
#define __USBDBGSERIAL_H_

//
//  Interface: USB Serial interface to SerialIfc
//
BOOL UsbSerial_Init();
VOID UsbSerial_Deinit(void);
UINT32 UsbSerial_EventHandler();
UINT32 UsbSerial_RecvData(PBYTE pbBuffer, DWORD cbBufSize);
DWORD UsbSerial_SendData(UINT8 *pData, UINT32 cbDataLen);
void UsbSerial_SetPower(BOOL fPowerOff);
BOOL UsbSerial_IsConnected();
void UsbSerial_SetProductId(DWORD dwProductId);
void UsbSerial_SetSerialNumberString(__in_ecount(ucNumWideChars) WCHAR* pwszString, UCHAR ucNumWideChars);


#define MANUFACTURER    L"Microsoft"
#define PRODUCT         L"Microsoft USBDBGSER for WindowsCE Devices"

#define RECVBUF_MAXSIZE             1536  

#endif

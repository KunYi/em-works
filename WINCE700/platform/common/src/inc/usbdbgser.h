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

#ifndef __USBDBGSER_H_
#define __USBDBGSER_H_

//
//  Interface: OAL_KITL_SERIAL_DRIVER interface to UsbDbgSerMdd
//
BOOL Serial_Init(KITL_SERIAL_INFO *pInfo);
void Serial_DeInit();
UINT16 Serial_Send(UINT8 *pData, UINT16 size);
UINT16 Serial_Recv(UINT8 *pData, UINT16 size);
void Serial_EnableInts();
void Serial_DisableInts();
void Serial_PowerOff();
void Serial_PowerOn();

#define OAL_KITLDRV_USBSERIAL  { \
    Serial_Init, \
    Serial_DeInit, \
    Serial_Send, \
    NULL, \
    Serial_Recv, \
    Serial_EnableInts, \
    Serial_DisableInts, \
    Serial_PowerOff, \
    Serial_PowerOn, \
    NULL \
}

#endif

//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  MX37_usbname.h
//
//  Provides definitions for usb module based on Freescale MX37 SoC.
//
//------------------------------------------------------------------------------
#ifndef ___USBKITLSERIAL_H
#define ___USBKITLSERIAL_H


#if __cplusplus
extern "C" {
#endif

UINT16 USBKitlSerialSend(UINT8 *pch, UINT16 cbSend);
UINT16 USBKitlSerialRecv(UINT8 *pch, UINT16 cbRead);
BOOL USBKitlSerialInit(KITL_SERIAL_INFO *pSerInfo);
VOID USBKitlSerialEnableInts();
VOID USBKitlSerialDisableInts();

#define USB_SERIAL_KITL {\
                        USBKitlSerialInit, \
                        NULL/*Deinit*/, \
                        USBKitlSerialSend, \
                        NULL/*Complete*/, \
                        USBKitlSerialRecv, \
                        USBKitlSerialEnableInts,\
                        USBKitlSerialDisableInts,\
                        NULL,\
                        NULL,\
                        NULL\
                        }

#ifdef __cplusplus
}
#endif

BOOL OALIoCtlKitlGetInfo(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);

#endif // ___USBKITLSERIAL_H

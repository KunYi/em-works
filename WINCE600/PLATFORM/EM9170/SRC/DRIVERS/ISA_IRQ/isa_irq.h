//**********************************************************************
//                                                                      
// Filename: Extern_IRQ.h
//                                                                      
// Description: Holds definitions for AT91SAM9260 USART serial interface.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Use of this source code is subject to the terms of the Emtronix end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to 
// use this source code. For a copy of the EULA, please see the 
// EULA.RTF on your install media.
//
// Copyright(c) Emtronix 2007, All Rights Reserved                       
//                                                                      
//**********************************************************************


#ifndef __EXTERN_IRQ_H__   
#define __EXTERN_IRQ_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _IRQINSTANCE_
{
	DWORD   DeviceArrayIndex;			// = 1, 2, 3
	DWORD   dwDeviceID;					// Atmel on-chip peripheral ID = 0 - 31
	DWORD   SysIntr;

	HANDLE	hIRQEvent;					// IRQ event
	HANDLE  hISTThread;					// IRQ IST thread
	HANDLE  hKillISTThread;             // kill thread event
	DWORD   KillThread;					// 

	BOOL	IsOpened;

	HANDLE	hExternIRQEvent;		     // Extern IRQ event

} IRQINSTANCE;


#ifdef __cplusplus
}
#endif

#endif __EXTERN_IRQ_H
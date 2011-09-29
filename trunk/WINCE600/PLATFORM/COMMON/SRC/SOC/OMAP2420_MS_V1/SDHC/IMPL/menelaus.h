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
//
// Copyright (c) Intrinsyc Software International.  All rights reserved.
//
//
//------------------------------------------------------------------------------
//
//  File: menelaus.h
//	
#ifndef __MENELAUS_H
#define __MENELAUS_H

#include <windows.h>

class CMenelaus
{
public:
	CMenelaus(
		LPCWSTR devicename,
		DWORD i2cAddress,
		DWORD i2cAddrSize);
	~CMenelaus();
    
	BOOL get_Slot1State() const {return m_bSlot1;}
	BOOL get_Slot2State() const {return m_bSlot2;}

	BOOL MenelausInit();			// initialize Menelaus Hardware
	BOOL MenelausDeinit();			// de-initialize Menelaus Hardware
	BOOL UpdateSlotsState();		// Gets the current slots states
	BOOL SlotsEnabled();			// deselect both slots 
	BOOL HandleSlot1Insert();		// enable power and select slot 1
	BOOL HandleSlot2Insert();		// select slot 2

    BOOL AckInterrupts();			// determin which irq fired & ack

private:
	CMenelaus(const CMenelaus&);	// hide copy ctor
	HANDLE m_hI2C;					// handle to the I2C bus
	LPCWSTR m_lpsI2CDeviceName;		// driver/device name
	DWORD m_dwI2CMenelausAddress;	// I2C Menelaus Address
	DWORD m_dwI2CMenelausAddrSize;	// I2C Address Size
	BOOL m_bSlot1;					// Slot 1 State TRUE : card is inserted
	BOOL m_bSlot2;					// Slot 2 State TRUE : card is inserted
	BOOL ReadData(UCHAR reg, UCHAR *pData); // read data from Menelaus on I2C
	BOOL WriteData(UCHAR reg, UCHAR data);  // wrtie data to Menelaus on I2C

    BOOL ClearMenelausIrq(UINT32 irq);
    BOOL UnMaskMenelausIrq(UINT32 irq);
    BOOL MaskMenelausIrq(UINT32 irq);
};
#endif //__MENELAUS_H
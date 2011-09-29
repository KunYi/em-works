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
/*

  Copyright(c) 2000 Microsoft

	Module Name:

		wavecpud.h


*/
#ifndef __WAVECPUDP_H_
#define __WAVECPUDP_H_
class DmaObject;

extern DmaObject * CreateDmaObject(BOOL bRecord);

extern "C" void DriverSleep(DWORD dwMS, BOOL bInPowerHandler);

class DmaObject {
public:
	DmaObject();
	virtual BOOL EnableDma()=0;
	virtual BOOL DisableDma()=0;
	virtual BOOL ContinueDma()=0;
	virtual BOOL ArmDma(BOOL bFirstBuffer,DWORD dwPhyAddr)=0;
	virtual BOOL StartDma(BOOL bGenInterrupt=TRUE)=0;
	virtual BOOL ReStartDma()=0;
	virtual BOOL StopDma()=0;
	virtual BOOL SetGlobalAddr(BOOL bFirst)=0;
	virtual DWORD GetDmaPhyAddr(BOOL bFirst)=0;
	virtual DWORD GetDmaSourceReg()=0;
	virtual DWORD GetDmaCountReg()=0;
	virtual DWORD GetAudioDmaPageSize()=0;
	virtual PVOID  GetDmaVirtualAddr()=0;
//  Global Address
	DWORD GetGlobalPlayAddress();
	DWORD SetGlobalPlayAddress(DWORD dwPhysicalAddr);

	USHORT GetGlobalOutInt();
	VOID ResetGlobalOutInt(BOOL bDecrement);

	DWORD GetGlobalRecAddress();
	DWORD SetGlobalRecAddress(DWORD dwPhysicalAddr);

	USHORT GetGlobalInInt();
	VOID ResetGlobalInInt(BOOL bDecrement);
private:
       DWORD m_dwplay_address;
       DWORD m_dwrec_address;
};


#endif
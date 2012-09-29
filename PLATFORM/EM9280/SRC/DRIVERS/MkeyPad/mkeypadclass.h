//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2011,Emtronix, Inc. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  mkeypadclass.h
//
//  Header file, for mkeypad driver.
//
//------------------------------------------------------------------------------


#ifndef __MKPDCLASS_H__
#define __MKPDCLASS_H__


#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define MAXIN   5
#define MAXOUT  5

extern DWORD g_uVkeyCode[MAXIN*MAXOUT];

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//--------------------------------------------------------------------------------


class MKPDClass
{

public:

	MKPDClass(UINT32 dwPollingTimeOut, UINT32 dwMkeyPadFromat );
	~MKPDClass(void);

	void MKeyPadHandle( );
	static BOOL g_bMkeyPadIsOpen;
	

private:
	int m_nMaxScanIn;
	int m_nMaxScanOut;
	UINT32 m_dwPollingTimeOut; 
	HANDLE m_hThread;

	int m_nKeyDelay;
	int m_nkeyState;
	BYTE m_ucScanWord;

	__inline BYTE GetDin( );
	void PutDout( int nIdx, int nVal=0 );
	void PutDoutAll( int nVal );

	void SetDelay( int nDelay );
	int IsTimeOut( );

	void KeyIoInit( int m_nMaxScanIn, int m_nMaxScanOut);
	void KeyIoDeInit( int m_nMaxScanIn, int m_nMaxScanOut );

};


#ifdef __cplusplus
}
#endif

#endif   // __MKPDCLASS_H__


//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  canclass.c
//
//  This file contains CAN module can only support  main MessageBox mode not support FIFO mode
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4127 4201)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <linklist.h>
#include <windev.h>
#include <ceddk.h>
#include <stdlib.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_can.h"
#include "canclass.h"


//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
RingBuffer::RingBuffer(UINT32 dwBufferSize)
{
	DWORD		i;

	dwPutIdx = 0;
	dwGetIdx = 0;
	dwMBArraySize = dwBufferSize;
	pMB = new MB_REG[dwMBArraySize];

	for( i=0; i<dwMBArraySize; i++ )
	{
		memset( &pMB[i], 0, sizeof( MB_REG) );
	}

	InitializeCriticalSection(&csRing);
}

RingBuffer::~RingBuffer()
{
	if( pMB != NULL )
		delete pMB;

	DeleteCriticalSection(&csRing);
}

BOOL RingBuffer::Put(PMB_REG pMssgBuf)
{
	DWORD		dwNext_PutIdx;

	if( pMB == NULL )
		return	FALSE;
	
	//EnterCriticalSection(&csRing);
	dwNext_PutIdx = (dwPutIdx + 1) % dwMBArraySize;
	if(dwNext_PutIdx == dwGetIdx)
	{
		//the rx ring buffer is full
		return FALSE;
	}

	//copy data into rx ring buffer
	memcpy(&pMB[dwPutIdx], pMssgBuf, sizeof(MB_REG));
	dwPutIdx = dwNext_PutIdx;
	//LeaveCriticalSection(&csRing);

	return TRUE;
}

BOOL RingBuffer::Get(PMB_REG pMssgBuf)
{
	if( pMB == NULL )
		return	FALSE;

	//EnterCriticalSection(&csRing);
	if(dwGetIdx == dwPutIdx)
	{
		//the tx ring buffer is empty
		return FALSE;
	}

	//copy data from rx ring buffer
	memcpy(pMssgBuf, &pMB[dwGetIdx], sizeof(MB_REG));
	dwGetIdx = (dwGetIdx + 1) % dwMBArraySize;
	//LeaveCriticalSection(&csRing);

	return TRUE;
}
	
BOOL RingBuffer::Purge()
{
	//EnterCriticalSection(&csRing);
	dwPutIdx = 0;
	dwGetIdx = 0;
	//LeaveCriticalSection(&csRing);
	return TRUE;
}



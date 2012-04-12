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

#ifndef _FMDWRAPPERMAIN_H_
#define _FMDWRAPPERMAIN_H_

#include <windows.h>
#include <diskio.h>
#include <devload.h>
#include <pm.h>
#include <storemgr.h>


//----------------------------- Forward class references ------------------------------
class FlashPddInterface;

//----------------------------- Debug zone information ------------------------------
extern  DBGPARAM    dpCurSettings;

//#define ZONE_INIT       DEBUGZONE(0)
//#define ZONE_ERROR      DEBUGZONE(1)
#define ZONE_WRITE_OPS  DEBUGZONE(2)
#define ZONE_READ_OPS   DEBUGZONE(3)
#define ZONE_FUNCTION   DEBUGZONE(4)

//----------------------------------------------------------------------------------
#include "FlashPdd.h"


//------------------------------- Public Interface ------------------------------
extern "C" 
{
DWORD FlashPdd_Init(DWORD Context);
BOOL  FlashPdd_Deinit(DWORD Context);
DWORD FlashPdd_Open(DWORD Data, DWORD Access, DWORD ShareMode);
BOOL  FlashPdd_Close(DWORD Handle);
DWORD FlashPdd_Read(DWORD Handle, LPVOID pBuffer, DWORD NumBytes);
DWORD FlashPdd_Write(DWORD Handle, LPCVOID pBuffer, DWORD InBytes);
DWORD FlashPdd_Seek(DWORD Handle, long Distance, DWORD MoveMethod);
BOOL  FlashPdd_IoControl(DWORD Handle, DWORD IoControlCode, PBYTE pInBuf, DWORD InBufSize,
                    PBYTE pOutBuf, DWORD OutBufSize, PDWORD pBytesReturned);
VOID  FlashPdd_PowerUp();
VOID  FlashPdd_PowerDown();
}

//
// CS&ZHL MAR-22-2012: RegionNumber = 0 is used for hive-based disk(SysFlash) with size = 128 * 128KB = 16MB
//
#define REG_NAND_DRIVER_REGION_NUMBER				TEXT("RegionNumber")

#define SMALL_PARTITION_BLOCKS						(128)			// 128 Blocks -> 128 * 128KB = 16MB
// end of CS&ZHL MAR-22-2012

//-------------------------------------------------------------------------------
#ifdef DEBUG

#define ReportError(Printf_exp) \
    DEBUGMSG(ZONE_ERROR,Printf_exp); \

#else

#define ReportError(Printf_exp) \
    RETAILMSG(ZONE_ERROR,(TEXT("Flash PDD Error: Function: %S,  Line: %d"), __FUNCTION__, __LINE__)); \

#endif

//-------------------------------------------------------------------------------

#endif _FMDWRAPPERMAIN_H_



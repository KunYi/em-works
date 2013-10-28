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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: cspiClass.h
//
// Header file for cspi bus driver.
//
//------------------------------------------------------------------------------
#ifndef __CSPICLASS_H__
#define __CSPICLASS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#ifdef DEBUG
#define ZONE_INIT		    DEBUGZONE(0)
#define ZONE_DEINIT		    DEBUGZONE(1)
#define ZONE_OPEN		    DEBUGZONE(2)
#define ZONE_CLOSE		    DEBUGZONE(3)
#define ZONE_IOCTL		    DEBUGZONE(4)
#define ZONE_THREAD         DEBUGZONE(5)
#define ZONE_FUNCTION	    DEBUGZONE(13)
#define ZONE_WARN		    DEBUGZONE(14)
#define ZONE_ERROR		    DEBUGZONE(15)
#endif // DEBUG

//------------------------------------------------------------------------------
// Types
typedef struct
{
    LIST_ENTRY  link;           // supports linked list of queue data
    CSPI_XCH_PKT_T xchPkt;      // client exchange packet with mapped pointers
} CSPI_XCH_LIST_ENTRY_T, *PCSPI_XCH_LIST_ENTRY_T;

class cspiClass
{
public:
    cspiClass();
    ~cspiClass(void);
    BOOL CspiInitialize(DWORD Index);
    void CspiRelease(void);
    BOOL CspiEnqueue(PCSPI_XCH_PKT_T pXchPkt);

    BOOL m_bUsePolling;
    
private:
    PCSP_CSPI_REGS m_pCSPI;
    LIST_ENTRY m_ListHead;
    CRITICAL_SECTION m_cspiCs;
    CRITICAL_SECTION m_cspiDataXchCs;
    UINT32 m_nIndex;
    UINT32 m_cspiOpenCount;
    HANDLE m_hIntrEvent;
    HANDLE m_hEnQEvent;
    HANDLE m_hThread;
    HANDLE m_hHeap;
    DWORD m_dwSysIntr;
    BOOL m_bTerminate;    

private:
    static DWORD WINAPI CspiProcessQueue(LPVOID lpParameter);
    UINT32 CspiDataExchange(PCSPI_XCH_PKT_T pXchPkt);
    static UINT32 CspiBufRd8(LPVOID pBuf);
    static UINT32 CspiBufRd16(LPVOID pBuf);
    static UINT32 CspiBufRd32(LPVOID pBuf);
    static void CspiBufWrt8(LPVOID pBuf, UINT32 data);
    static void CspiBufWrt16(LPVOID pBuf, UINT32 data);
    static void CspiBufWrt32(LPVOID pBuf, UINT32 data);
};

//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif // __CSPICLASS_H__


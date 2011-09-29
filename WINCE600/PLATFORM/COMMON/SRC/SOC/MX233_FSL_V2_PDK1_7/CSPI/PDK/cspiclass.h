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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  cspiclass.h
//
//  Header file, for CSPI driver.
//
//------------------------------------------------------------------------------

#ifndef __CSPI_CLASS_H
#define __CSPI_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pm.h>
#include "mx233_ssp.h"
#include "hw_spi.h"
//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define CSPI_MAX_QUEUE_LENGTH    100

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_INIT         0
#define ZONEID_DEINIT       1
#define ZONEID_OPEN         2
#define ZONEID_CLOSE        3
#define ZONEID_IOCTL        4
#define ZONEID_THREAD       5
#define ZONEID_FUNCTION     13
#define ZONEID_WARN         14
#define ZONEID_ERROR        15

// Debug zone masks
#define ZONEMASK_INIT       (1 << ZONEID_INIT)
#define ZONEMASK_DEINIT     (1 << ZONEID_DEINIT)
#define ZONEMASK_OPEN       (1 << ZONEID_OPEN)
#define ZONEMASK_CLOSE      (1 << ZONEID_CLOSE)
#define ZONEMASK_IOCTL      (1 << ZONEID_IOCTL)
#define ZONEMASK_THREAD     (1 << ZONEID_THREAD)
#define ZONEMASK_FUNCTION   (1 << ZONEID_FUNCTION)
#define ZONEMASK_WARN       (1 << ZONEID_WARN)
#define ZONEMASK_ERROR      (1 << ZONEID_ERROR)

#define ZONE_INIT       DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT     DEBUGZONE(ZONEID_DEINIT)
#define ZONE_OPEN       DEBUGZONE(ZONEID_OPEN)
#define ZONE_CLOSE      DEBUGZONE(ZONEID_CLOSE)
#define ZONE_IOCTL      DEBUGZONE(ZONEID_IOCTL)
#define ZONE_THREAD     DEBUGZONE(ZONEID_THREAD)
#define ZONE_FUNCTION   DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN       DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR      DEBUGZONE(ZONEID_ERROR)
#endif // DEBUG

//------------------------------------------------------------------------------
//#ifdef POLLING_MODE
// Types
typedef struct {
    PVOID  m_pLocalAsync;
    PVOID  m_pLocalSyncMarshalled;
    PVOID  m_pCallerUnmarshalled;
    DWORD  m_cbSize;
    DWORD  m_ArgumentDescriptor;
}CALLER_STUB_T, *PCALLER_STUB_T;

// CSPI exchange packet
typedef struct
{
    PCSPI_BUSCONFIG_T pBusCnfg;
    LPVOID pBuf;
    UINT32 xchCnt;
    HANDLE xchEvent;
} CSPI_XCH_PKT0_T, *PCSPI_XCH_PKT0_T;


typedef struct {
    CSPI_XCH_PKT0_T xchRealPkt;  // client exchange packet with mapped pointers
    CALLER_STUB_T marshalStub;
    CALLER_STUB_T marshalBusCnfgStub;
} MARSHALED_CSPI_XCH_PKT_T;


typedef struct
{
    LIST_ENTRY  link;                     // supports linked list of queue data
    MARSHALED_CSPI_XCH_PKT_T xchPkt;      // client exchange packet with mapped pointers
} CSPI_XCH_LIST_ENTRY_T, *PCSPI_XCH_LIST_ENTRY_T;
//#endif
class cspiClass
{
public:
    PCSP_SSP_REGS m_pCSPI;

    cspiClass();
    ~cspiClass(void);
    BOOL CspiInitialize(DWORD Index);
    void CspiRelease(void);
//#ifdef POLLING_MODE
    BOOL CspiEnqueue(PCSPI_XCH_PKT_T pXchPkt);
//#endif
    void CspiEnableLoopback(BOOL bEnable);
    BOOL ConfigureSSP(SSP_INIT * sInit);
    BOOL SSPResetAfterError(SSP_RESETCONFIG * sConfigparams);
    BOOL SSPCheckErrors(VOID);
    BOOL SSPGetIrqStatus(SSP_IRQ eIrq);
    BOOL SSPClearIrq(SSP_IRQ eIrq);
    BOOL SSPConfigTiming(SSP_SPEED eSpeed);
    BOOL SSPEnableErrIrq(BOOL bEnable);
    BOOL SSPDisableErrIrq(VOID);
    VOID SSP_Reset(VOID);
    BOOL DumpRegister(VOID);
    
    BOOL m_bUsePolling;
    CEDEVICE_POWER_STATE m_dxCurrent;
private:
    LIST_ENTRY m_ListHead;
    CRITICAL_SECTION m_cspiCs;
    CRITICAL_SECTION m_cspiDataXchCs;
    UINT32 m_cspiOpenCount;
    HANDLE m_hEnQEvent;
    HANDLE m_hEnQSemaphere;
    HANDLE m_hThread;
    HANDLE m_hHeap;
    BOOL m_bTerminate;
    UINT m_Index;
    BOOL m_bUseLoopBack;
    BOOL m_bAllowPolling;
    UINT32 m_nondmaTransferCount;
    BOOL m_bUseDMA;
private:
//#ifdef POLLING_MODE
    static DWORD WINAPI CspiProcessQueue(LPVOID lpParameter);
    UINT32 CspiNonDMADataExchange(PCSPI_XCH_PKT0_T pXchPkt);
    //UINT32 CspiExchangeSize(PCSPI_XCH_PKT0_T pXchPkt);
    static UINT32 CspiBufRd8(LPVOID pBuf);
    static UINT32 CspiBufRd16(LPVOID pBuf);
    static UINT32 CspiBufRd32(LPVOID pBuf);
    static void CspiBufWrt8(LPVOID pBuf, UINT32 data);
    static void CspiBufWrt16(LPVOID pBuf, UINT32 data);
    static void CspiBufWrt32(LPVOID pBuf, UINT32 data);
//#endif    
};
//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif


#endif // __CSPICLASS_H__


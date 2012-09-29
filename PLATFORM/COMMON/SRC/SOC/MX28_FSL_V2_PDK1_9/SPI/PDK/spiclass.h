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
//  File:  spiclass.h
//
//  Header file, for SPI driver.
//
//------------------------------------------------------------------------------

#ifndef __SPI_CLASS_H
#define __SPI_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pm.h>
#include "hw_spi.h"

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define SPI_MAX_QUEUE_LENGTH    100

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

// SPI exchange packet
typedef struct
{
    PSPI_BUSCONFIG_T pBusCnfg;
    LPVOID pBuf;
    UINT32 xchCnt;
    HANDLE xchEvent;
} SPI_XCH_PKT0_T, *PSPI_XCH_PKT0_T;


typedef struct 
{
    SPI_XCH_PKT0_T xchRealPkt;  // client exchange packet with mapped pointers
    CALLER_STUB_T marshalStub;
    CALLER_STUB_T marshalBusCnfgStub;
} MARSHALED_SPI_XCH_PKT_T;


typedef struct
{
    LIST_ENTRY  link;                     // supports linked list of queue data
    MARSHALED_SPI_XCH_PKT_T xchPkt;      // client exchange packet with mapped pointers
} SPI_XCH_LIST_ENTRY_T, *PSPI_XCH_LIST_ENTRY_T;
//#endif
class spiClass
{
public:
    PVOID pv_HWregSSP0;
    PVOID pv_HWregSSP1;
    PVOID pv_HWregSSP2;
    PVOID pv_HWregSSP3;
    spiClass();
    ~spiClass(void);
    BOOL SpiInitialize(DWORD Index);

	void SpiSetCSIndex(DWORD dwCSIndex) { m_dwCSIndex = dwCSIndex; }
    void SpiRelease(void);
//#ifdef POLLING_MODE
    BOOL SpiEnqueue(PSPI_XCH_PKT_T pXchPkt);
//#endif
    void SpiEnableLoopback(BOOL bEnable);
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

	// zxw 2012-06-11
	UINT m_Index;   

	//zxw 2012-6-27
	DWORD MasterRead(BOOL Lock_CS, PBYTE pBuf, DWORD dwLength , BYTE BitCount);
	DWORD MasterWrite(BOOL Lock_CS, PBYTE pBuf, DWORD dwLength , BYTE BitCount);
	//-----------------------------------------------------------------------------

private:
    LIST_ENTRY m_ListHead;
    CRITICAL_SECTION m_spiCs;
    CRITICAL_SECTION m_spiDataXchCs;
    UINT32 m_spiOpenCount;
    HANDLE m_hEnQEvent;
    HANDLE m_hEnQSemaphere;
    HANDLE m_hThread;
    HANDLE m_hHeap;
    BOOL m_bTerminate;
    //UINT m_Index;
    BOOL m_bUseLoopBack;
    BOOL m_bAllowPolling;
    UINT32 m_nondmaTransferCount;
    BOOL m_bUseDMA;
	
	//CS&ZHL MAY-15-2012: for SSP0 testing
	DWORD	m_dwCSIndex;

private:
//#ifdef POLLING_MODE
    static DWORD WINAPI SpiProcessQueue(LPVOID lpParameter);
    UINT32 SpiNonDMADataExchange(PSPI_XCH_PKT0_T pXchPkt);
    //UINT32 SpiExchangeSize(PSPI_XCH_PKT0_T pXchPkt);
    static UINT32 SpiBufRd8(LPVOID pBuf);
    static UINT32 SpiBufRd16(LPVOID pBuf);
    static UINT32 SpiBufRd32(LPVOID pBuf);
    static void SpiBufWrt8(LPVOID pBuf, UINT32 data);
    static void SpiBufWrt16(LPVOID pBuf, UINT32 data);
    static void SpiBufWrt32(LPVOID pBuf, UINT32 data);
//#endif    
};
//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif


#endif // __SPICLASS_H__


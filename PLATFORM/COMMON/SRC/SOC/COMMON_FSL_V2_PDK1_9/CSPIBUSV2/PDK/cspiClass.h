/*---------------------------------------------------------------------------
* Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
//
//  File:  cspiclass.h
//
//   Header file for cspi bus driver.
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
#define ZONE_INIT         DEBUGZONE(0)
#define ZONE_DEINIT       DEBUGZONE(1)
#define ZONE_OPEN         DEBUGZONE(2)
#define ZONE_CLOSE        DEBUGZONE(3)
#define ZONE_IOCTL        DEBUGZONE(4)
#define ZONE_THREAD       DEBUGZONE(5)
#define ZONE_DMA          DEBUGZONE(10)
#define ZONE_FUNCTION     DEBUGZONE(13)
#define ZONE_WARN         DEBUGZONE(14)
#define ZONE_ERROR        DEBUGZONE(15)
#endif // DEBUG

#define CSPI_SDMA_BUFFER_SIZE    0x2800
#define CSPI_DMA_WATERMARK_RX    (16)
#define CSPI_DMA_WATERMARK_TX    (16)
#define CSPI_TXMT_OFFSET         0x0
#define CSPI_RECV_OFFSET         0x1400

#define CSPI_MAX_QUEUE_LENGTH    100
//------------------------------------------------------------------------------
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
    LPVOID pTxBuf;
    LPVOID pRxBuf;
    UINT32 xchCnt;
    HANDLE xchEvent;
} CSPI_XCH_PKT0_T, *PCSPI_XCH_PKT0_T;


typedef struct {
    CSPI_XCH_PKT0_T xchRealPkt;  // client exchange packet with mapped pointers
    CALLER_STUB_T marshalTxStub;
    CALLER_STUB_T marshalRxStub;
    CALLER_STUB_T marshalBusCnfgStub;
} MARSHALED_CSPI_XCH_PKT_T;


typedef struct
{
    LIST_ENTRY  link;                     // supports linked list of queue data
    MARSHALED_CSPI_XCH_PKT_T xchPkt;      // client exchange packet with mapped pointers
} CSPI_XCH_LIST_ENTRY_T, *PCSPI_XCH_LIST_ENTRY_T;


class cspiClass
{
public:
    cspiClass();
    ~cspiClass(void);
    BOOL CspiInitialize(DWORD Index);
    void CspiRelease(void);
    BOOL CspiEnqueue(PCSPI_XCH_PKT_T pXchPkt);
    void CspiEnableLoopback(BOOL bEnable);
    BOOL CheckPort(void);
    BOOL CSPI2DataExchange(PCSPI_XCH_PKT_T pXchPkt);

    BOOL m_bUsePolling;
    CEDEVICE_POWER_STATE m_dxCurrent;

    // DMA specific
    BOOL InitCspiDMA(UINT32 Index);
    BOOL DeInitCspiDMA(void);
    BOOL InitChannelDMA(UINT32 Index);
    BOOL DeinitChannelDMA(void);
    BOOL UnmapDMABuffers(void);
    BOOL MapDMABuffers(void);
    VOID MoveDMABuffer(LPVOID pBuf, DWORD dwLen, BOOL bReceive);

private:
    PCSP_CSPI_REG m_pCSPI;
    LIST_ENTRY m_ListHead;
    CRITICAL_SECTION m_cspiCs;
    CRITICAL_SECTION m_cspiDataXchCs;
    UINT32 m_cspiOpenCount;
    HANDLE m_hIntrEvent;
    HANDLE m_hEnQEvent;
    HANDLE m_hEnQSemaphere;
    HANDLE m_hThread;
    HANDLE m_hHeap;
    DWORD m_dwSysIntr;
    BOOL m_bTerminate;
    UINT m_Index;
    BOOL m_bUseLoopBack;
    BOOL m_bAllowPolling;

    PHYSICAL_ADDRESS PhysDMABufferAddr;
    PBYTE             pVirtDMABufferAddr;
    UINT8             m_dmaChanCspiRx, m_dmaChanCspiTx; 
    DDK_DMA_REQ       m_dmaReqTx, m_dmaReqRx ; 
    BOOL m_isDMADone;
    UINT32 m_dmaTransferCount;
    UINT32 m_nondmaTransferCount;
    BOOL m_bUseDMA;
private:
    static DWORD WINAPI CspiProcessQueue(LPVOID lpParameter);
    UINT32 CspiDMADataExchange(PCSPI_XCH_PKT0_T pXchPkt);
    UINT32 CspiNonDMADataExchange(PCSPI_XCH_PKT0_T pXchPkt);
    UINT32 CspiExchangeSize(PCSPI_XCH_PKT0_T pXchPkt);
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

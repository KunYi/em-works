//-----------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  ATA.h
//
//  ATA device abstraction.
//
//------------------------------------------------------------------------------

#ifndef _ATA_H_
#define _ATA_H_

#include "common_ata.h"
#include "common_ddk.h"
#include "common_macros.h"
#include <atamain.h>

#define PIO_MODE        1
#define MDMA_MODE       2
#define UDMA_MODE       3

#define USE_SDMA
#define USE_SDMA_SG_LIST


#define ATA_MAX_SECTOR          512
#define ATA_MAX_DISKTRANSFER        (MAX_SECT_PER_COMMAND * ATA_MAX_SECTOR)
#define ATA_MAX_PAGE_COUNT      ((ATA_MAX_DISKTRANSFER / 4096) + 2)



class CMXDisk : public CDisk {
  public:

    // member variables
    static LONG  m_lDeviceCount;

    PCSP_ATA_REG    g_pVAtaReg;

    BYTE    bIntrPending;
    
    int    CLOCK_PERIOD;

    PHYSICAL_ADDRESS    PhysDMABufferAddr;
    PBYTE       pVirtDMABufferAddr;

    
    // constructors/destructors
    CMXDisk(HKEY hKey);
    virtual ~CMXDisk();

    // member functions
    virtual VOID ConfigureRegisterBlock(DWORD dwStride);
    virtual BOOL Init(HKEY hActiveKey);
    virtual DWORD MainIoctl(PIOREQ pIOReq);
    virtual BOOL WaitForInterrupt(DWORD dwTimeOut);
    virtual void EnableInterrupt();
    virtual BOOL ConfigPort();
    
    virtual BOOL SetupDMA(PSG_BUF pSgBuf, DWORD dwSgCount, BOOL fRead);
    virtual BOOL BeginDMA(BOOL fRead);
    virtual BOOL EndDMA();
    virtual BOOL AbortDMA();
    virtual BOOL CompleteDMA(PSG_BUF pSgBuf, DWORD dwSgCount, BOOL fRead);
    
    void CopyDiskInfoFromPort();
    void SetTimingRegisters(int speed, int mode, int ClkSpd);
    BOOL ConfigureIOMUXPad(int muxmode);
    BOOL InitializePort(void);
    void ShowRegisters(UCHAR usel);
    inline virtual void CMXDisk::TakeCS() {
        m_pPort->TakeCS();
    }
    inline virtual void CMXDisk::ReleaseCS() {
        m_pPort->ReleaseCS();
    }
    inline BOOL CMXDisk::DoesDeviceAlreadyExist() {
        return FALSE;
    }

    // DMA support
    PSG_BUF m_pDMASgBuf;
    DWORD   m_dwAlignedSgCount;
    DWORD   m_dwAlignedDescCount;
    DWORD   m_dwAlignedSgBytes;
    DWORD   dwDoubleBufferPos;
    BOOL    fSDMAIntrEnable;

    // ADMA support
    BOOL MapADMATable(void);
    BOOL UnmapADMATable(void);

    BOOL MapDMABuffers(void);
    BOOL UnmapDMABuffers(void);
    BOOL DeinitChannelDMA(void);
    BOOL InitChannelDMA(void);
    BOOL InitDMA(void) ;
    BOOL DeInitDMA(void); 
    DWORD MoveDMABuffer(PSG_BUF pSgBuf, DWORD dwSgIndex, DWORD dwSgCount, BOOL fRead);

    // atapi support
    PCDROM_READ m_pSterileCdRomReadRequest;
    DWORD m_cbSterileCdRomReadRequest;

    static BOOL SterilizeCdRomReadRequest(
                PCDROM_READ* ppSafe,
                LPDWORD      lpcbSafe,
                PCDROM_READ  pUnsafe,
                DWORD        cbUnsafe,
                DWORD        dwArgType,
                OUT PUCHAR * saveoldptrs    
                );
    DWORD AtapiIoctl(PIOREQ pIOReq);
    DWORD ScsiPassThrough( const SCSI_PASS_THROUGH& PassThrough,
                           SGX_BUF* pSgxBuf,
                           PSENSE_DATA pSenseData,
                           DWORD* pdwBytesReturned,
                           BOOL fAllowNoData = FALSE );
    DWORD ReadCdRom(CDROM_READ *pReadInfo, PDWORD pBytesReturned);
    DWORD SetupCdRomRead(BOOL bRawMode, DWORD dwLBAAddr, DWORD dwTransferLength, PATAPI_COMMAND_PACKET pCmdPkt);
    virtual DWORD ReadCdRomDMA(DWORD dwLBAAddr, DWORD dwTransferLength, WORD wSectorSize, DWORD dwSgCount, SGX_BUF *pSgBuf);
    BOOL AtapiSendCommand(PATAPI_COMMAND_PACKET pCmdPkt, WORD wCount = 0, BOOL fDMA = FALSE);
    BOOL AtapiReceiveData(PSGX_BUF pSgBuf, DWORD dwSgCount,LPDWORD pdwBytesRead, BOOL fNoDataIsOK = TRUE);
    BOOL AtapiSendData(PSGX_BUF pSgBuf, DWORD dwSgCount,LPDWORD pdwBytesWritten);
    BOOL AtapiIsUnitReady(PIOREQ pIOReq = NULL);
    BOOL AtapiIsUnitReadyEx();
    BOOL AtapiGetSenseInfo(SENSE_DATA *pSenseData);
    BOOL AtapiIssueInquiry(INQUIRY_DATA *pInqData);
    BOOL AtapiGetToc(CDROM_TOC *pTOC);
    DWORD AtapiGetDiscInfo(PIOREQ pIOReq);
    DWORD AtapiReadQChannel(PIOREQ pIOReq);
    DWORD AtapiLoadMedia(BOOL bEject=FALSE);
    DWORD AtapiStartDisc();
    BOOL AtapiDetectDVD();
    void AtapiDumpSenseData(SENSE_DATA* pSenseData = NULL);
    DWORD ControlAudio(PIOREQ pIOReq);
    DWORD DVDReadKey(PIOREQ pIOReq);
    DWORD DVDGetRegion(PIOREQ pIOReq);
    DWORD DVDSendKey(PIOREQ pIOReq);
    DWORD DVDSetRegion(PIOREQ pIOReq);
    BOOL DVDGetCopySystem(LPBYTE pbCopySystem, LPBYTE pbRegionManagement);
    
};

EXTERN_C
CDisk *
CreateMXHD(
    HKEY hDevKey
    );

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------
extern BOOL CSPATAEnableClock(BOOL bEnable);
extern DWORD CSPATAMaxSectDMA();

#endif //_ATA_H_


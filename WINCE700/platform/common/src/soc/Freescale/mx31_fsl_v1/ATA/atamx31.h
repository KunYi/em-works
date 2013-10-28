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
// Copyright (C) 2006-2007 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  ATAMX31.h
//
//  ATA MX31 device abstraction.
//
//------------------------------------------------------------------------------

#ifndef _ATAMX31_H_
#define _ATAMX31_H_

#include "csp.h"
#include <atamain.h>

#define CLOCK_PERIOD   15
#define PIO_MODE        1
#define MDMA_MODE       2
#define UDMA_MODE       3

#define USE_SDMA
#define USE_SDMA_SG_LIST

#define ATA_SDMA_BUFFER_SIZE    (MAX_SECT_PER_COMMAND_DMA*512)
#define ATA_DMA_WATERMARK       32  // ATA DMA watermark in bytes

#define ATA_MAX_SECTOR          512
#define ATA_MAX_TRANSFER        (MAX_SECT_PER_COMMAND * ATA_MAX_SECTOR)
#define ATA_MAX_DESC_COUNT      ((ATA_MAX_TRANSFER / 4096) * 2)

#define CACHE_LINE_SIZE_MASK    (32 - 1)    // L1 & L2 cache lines are 32 bytes

class CMX31Disk : public CDisk {
  public:

    // member variables
    static LONG  m_lDeviceCount;

    PCSP_ATA_REG    g_pVAtaReg;

    BYTE    bIntrPending;

    // (SDMA support)
    PHYSICAL_ADDRESS    PhysDMABufferAddr;
    PBYTE       pVirtDMABufferAddr;
    UINT8       DmaChanATARx, DmaChanATATx;
    DDK_DMA_REQ     DmaReqTx ;
    DDK_DMA_REQ     DmaReqRx ;

    // constructors/destructors
    CMX31Disk(HKEY hKey);
    virtual ~CMX31Disk();

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
    inline virtual void CMX31Disk::TakeCS() {
        m_pPort->TakeCS();
    }
    inline virtual void CMX31Disk::ReleaseCS() {
        m_pPort->ReleaseCS();
    }
    inline BOOL CMX31Disk::DoesDeviceAlreadyExist() {
        return FALSE;
    }

    // DMA support
    PSG_BUF m_pDMASgBuf;
    DWORD   m_dwAlignedSgCount;
    DWORD   m_dwAlignedDescCount;
    DWORD   m_dwAlignedSgBytes;
    UINT8   m_DMAChan;
    DWORD   dwDoubleBufferPos;
    BOOL    fSDMAIntrEnable;

    BOOL MapDMABuffers(void);
    BOOL UnmapDMABuffers(void);
    BOOL DeinitChannelDMA(void);
    BOOL InitChannelDMA(void);
    BOOL InitDMA(void) ;
    BOOL DeInitDMA(void);
    DWORD MoveDMABuffer(PSG_BUF pSgBuf, DWORD dwSgIndex, DWORD dwSgCount, BOOL fRead);

};

EXTERN_C
CDisk *
CreateMX31HD(
    HKEY hDevKey
    );

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

extern BOOL BSPATAEnableClock(BOOL bEnable);
extern BOOL BSPATAIOMUXConfig(void);
extern UINT8 BSPATASDMAchannelprio(void);
extern BOOL BSPATAPowerEnable(BOOL bEnable);
#endif //_ATAMX31_H_


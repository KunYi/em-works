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
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  hsi2cclass.h
//
//  Header file, for HSI2C bus driver.
//
//------------------------------------------------------------------------------

#ifndef _INC_HSI2CCLASS_H
#define _INC_HSI2CCLASS_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

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

#define HSI2C_MASTER_MODE                 1           // I2C Master Mode
#define HSI2C_SLAVE_MODE                  2           // I2C Slave Mode

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

enum { HSI2CSLAVEBUFSIZE=512 };
typedef struct tagHSI2CSLAVEBUF {
    BYTE byBuf[HSI2CSLAVEBUFSIZE];
    INT iBufSize;
} HSI2CSLAVEBUF, *PHSI2CSLAVEBUF ;


typedef struct
{
    LIST_ENTRY  Link;   // Required for <linklist.h> to maintain double-linked list
    HSI2C_PACKET Packet;  // Holds the I2C packet data
} HSI2C_PACKET_LIST_ENTRY, *PHSI2C_PACKET_LIST_ENTRY;

class HSI2CClass
{
public:
    //-------------------------------------
    // HSI2C CONSTRUCTOR/DESTRUCTOR METHODS
    //

    HSI2CClass(UINT32 index);
    ~HSI2CClass(void);

    //-------------------------------------
    // HSI2C PROPERTIES ACCESSOR METHODS
    //

    //
    // Clock Rate Divider
    //
    WORD GetClockRateDivider(void)
    {
        WORD i;
        EnterCriticalSection(&gcsHSI2CDataLock);
        i = wLastClkRate;
        LeaveCriticalSection(&gcsHSI2CDataLock);
        return i;
    }

    WORD SetClockRateDivider(WORD wClkRate)
    {
        EnterCriticalSection(&gcsHSI2CDataLock);
        wLastClkRate = wClkRate;
        RETAILMSG(1,(_T("wLastClkRate=0x%x\r\n"),wLastClkRate));
        LeaveCriticalSection(&gcsHSI2CDataLock);
        return wClkRate;
    }

    //
    // Last Operation Result
    //
    BOOL IsLastActionOK(void)
    {
        return (iResult >= 0);
    }

    INT GetLastResult(void)
    {
        return iResult;
    }

    //
    // Self Addressing (Slave Mode)
    //
    BYTE GetSelfAddress(void)
    {
        BYTE i;
        EnterCriticalSection(&gcsHSI2CDataLock);
        i = byLastSelfAddr;
        LeaveCriticalSection(&gcsHSI2CDataLock);
        return i;
    }

    BYTE SetSelfAddress(BYTE bySelfAddr)
    {
        EnterCriticalSection(&gcsHSI2CDataLock);
        byLastSelfAddr = bySelfAddr;
        LeaveCriticalSection(&gcsHSI2CDataLock);
        return bySelfAddr;
    }

    //
    // HSI2C Mode
    //
    BYTE GetMode(void)
    {
        return HSI2C_MASTER_MODE;
    }

    BYTE SetMode(BYTE byMode)
    {
        if(byMode != HSI2C_MASTER_MODE)
            SetLastError(ERROR_NOT_SUPPORTED);
        return HSI2C_MASTER_MODE;
    }

    //-------------------------------------
    // HSI2C TRANSACTION METHODS
    //
    BOOL ProcessPackets(HSI2C_PACKET [], INT32);
    
    //-------------------------------------
    // HSI2C CR METHODS
    //
    VOID Reset(void); // Only reset HSI2C, does not affect the class fields

    //-------------------------------------
    // HSI2C CLOCK METHODS
    //
    WORD CalculateClkRateDiv(DWORD dwFrequency);

    //-------------------------------------
    // HSI2C Slave IOCONTROL function
    //
    BOOL EnableSlave(void);
    BOOL DisableSlave(void);
    BOOL SetSlaveText(PBYTE pBufIn, DWORD dwLen);
    BOOL GetSlaveText(PBYTE pBufOut, DWORD dwLen, PDWORD pdwActualOut);
    BOOL SetSlaveSize(PBYTE pBufIn, DWORD dwLen);
    BOOL GetSlaveSize(PBYTE pBufOut, DWORD dwLen, PDWORD pdwActualOut);

    PHSI2CSLAVEBUF SetSlaveBuf(PHSI2CSLAVEBUF pSlaveBuf)
    {
        UNREFERENCED_PARAMETER(pSlaveBuf);
        SetLastError(ERROR_NOT_SUPPORTED);
        return NULL;
    }

    PHSI2CSLAVEBUF GetSlaveBuf(void)
    {
        SetLastError(ERROR_NOT_SUPPORTED);
        return NULL;
    }

private:

    //-------------------------------------
    // HSI2C Private Helper methods
    //
    BOOL GenerateStart(BYTE SlaveAddr, BOOL flag);
    VOID GenerateStop();
    BOOL WritePacketDMA(PHSI2C_PACKET packets, BOOL bFirst, BOOL bLast);
    BOOL ReadPacketDMA(PHSI2C_PACKET packets, BOOL bFirst, BOOL bLast);
    BOOL WritePacketFIFO(PHSI2C_PACKET packets, BOOL bFirst, BOOL bLast);
    BOOL ReadPacketFIFO(PHSI2C_PACKET packets, BOOL bFirst, BOOL bLast);
    
    BOOL HSI2CCheckArbitLost();
    BOOL HSI2CCheckAckStatus();
    BOOL HSI2CWaitForRDF(DWORD bytes, DWORD polling, DWORD timeout);
    BOOL HSI2CWaitForTDE(DWORD polling, DWORD timeout);
    
    BOOL HSI2CWaitForRDC(DWORD polling, DWORD timeout);
    BOOL HSI2CWaitForTDC(DWORD polling, DWORD timeout);
    BOOL HSI2CWaitAndClearByteTransfer(DWORD polling, DWORD timeout);
    
    BOOL HSI2CWaitForBusStatus(DWORD status, DWORD timout);
    BOOL HSI2CWaitForRestartStatus(DWORD status, DWORD timout);
    BOOL MasterModeInit(BOOL BitMode);
    
    BOOL InitChannelDMA(UINT32 Index);
    VOID DeInitChannelDMA(void);
    
    BOOL MapDMABuffers(void);
    VOID UnMapDMABuffers(void);
    
    BOOL InitDMA(UINT32 Index);
    VOID DeInitDMA(void);
    
    VOID DUMP(void);
    
    //-------------------------------------
    // I2C Module Index
    //
    UINT32 m_iModuleIndex;

    //-------------------------------------
    // I2C bus critical section
    //
    CRITICAL_SECTION gcsHSI2CBusLock;
    CRITICAL_SECTION gcsHSI2CDataLock;
    
    //-------------------------------------
    // I2C INTERRUPT HANDLING
    //
    HANDLE hInterrupted;                // Hardware Interrupt Occurence Event (Shared)
    
    //-------------------------------------
    // I2C PROPERTIES
    //
    DWORD dwFreq;
    WORD wLastClkRate;                  // Last Known Frequency
    INT iResult;                        // Last Known Result
    BYTE byLastMode;                    // Last Known Mode
    BYTE byLastSelfAddr;                // My Last Known Address
    
    //-------------------------------------
    // I2C TRANSACTION FIELDS   
    //
    DWORD dwSysIntr;                // System Interrupt ID
    BOOL m_bHSMode;
    BOOL m_bDMAMode;
    DDK_DMA_REQ       m_dmaReqTx, m_dmaReqRx ;
    PHYSICAL_ADDRESS  PhysDMABufferAddr;
    PBYTE             pVirtDMABufferAddr;
    UINT8             m_dmaChanRx, m_dmaChanTx; 
    PCSP_HSI2C_REG pHSI2CReg;           // I2C Register Base Mapped Virtual Address

public:
    CEDEVICE_POWER_STATE m_dxCurrent;
};



//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

//BSP
extern BOOL BSPHSI2CIOMUXConfig(UINT32 index);
extern BOOL BSPHSI2CEnableClock(UINT32 index, BOOL bEnable);
extern BOOL BSPHSI2CGetModuleClock(UINT32 index, PDWORD pdwFreq);
extern INT  BSPGetTimeoutValue(void);
extern DWORD  BSPGetThreadPriority(void);

//CSP/SOC
extern UINT32 HSI2CGetBaseRegAddr(UINT32 index);
extern DWORD HSI2CGetIRQ(UINT32 index);
extern DDK_DMA_REQ HSI2CGetDMATxReq(UINT32 index);
extern DDK_DMA_REQ HSI2CGetDMARxReq(UINT32 index);

#ifdef __cplusplus
}
#endif

#endif   // _INC_HSI2CCLASS_H


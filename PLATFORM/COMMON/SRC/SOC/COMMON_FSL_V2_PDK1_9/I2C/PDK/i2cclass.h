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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  i2cclass.h
//
//  Header file, for I2C bus driver.
//
//------------------------------------------------------------------------------

#ifndef _INC_I2CCLASS_H
#define _INC_I2CCLASS_H

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

#define I2C_DEF_NEW_ENTRY_WAIT_TIME     10000       // Default New List Entry Waiting Time

#define I2C_MASTER_MODE                 1           // I2C Master Mode

//Set slave mode from IOCTL is obsolete feature, use enable slave instead.
//Keep the definition here for back compatible purpose.
#define I2C_SLAVE_MODE                  2           // I2C Slave Mode

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

enum { I2CSLAVEBUFSIZE=512 };
typedef struct tagI2CSLAVEBUF {
    BYTE byBuf[I2CSLAVEBUFSIZE];
    INT iBufSize;
} I2CSLAVEBUF, *PI2CSLAVEBUF ;


typedef struct
{
    LIST_ENTRY  Link;   // Required for <linklist.h> to maintain double-linked list
    I2C_PACKET Packet;  // Holds the I2C packet data
} I2C_PACKET_LIST_ENTRY, *PI2C_PACKET_LIST_ENTRY;

class I2CClass
{
public:
    //-------------------------------------
    // I2C CONSTRUCTOR/DESTRUCTOR METHODS
    //

    I2CClass(UINT32 index);
    ~I2CClass(void);

    //-------------------------------------
    // I2C PROPERTIES ACCESSOR METHODS
    //

    //
    // Clock Rate Divider
    //
    WORD GetClockRateDivider(void)
    {
        return wLastClkRate;
    }

    WORD SetClockRateDivider(WORD wClkRate)
    {
        wLastClkRate = wClkRate;
        return wLastClkRate;
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
        return byLastSelfAddr;
    }

    BYTE SetSelfAddress(BYTE bySelfAddr)
    {
        byLastSelfAddr = bySelfAddr;
        return byLastSelfAddr;
    }

    //
    // I2C Mode
    //
    BYTE GetMode(void)
    {
        return byLastMode;
    }

    BYTE SetMode(BYTE byMode)
    {
        byLastMode = byMode;
        return byLastMode;
    }

    BYTE RealMode(void);

    //-------------------------------------
    // I2C TRANSACTION METHODS
    //

    BOOL ProcessPackets(I2C_PACKET [], INT32);
    BOOL MCU_ProcessPackets(I2C_PACKET [], INT32);

    //-------------------------------------
    // I2C CR METHODS
    //

    VOID Reset(void); // Only reset I2C, does not affect the class fields

    //-------------------------------------
    // I2C CLOCK METHODS
    //
    WORD CalculateClkRateDiv(DWORD dwFrequency);

    //-------------------------------------
    // I2C Slave IOCONTROL function
    //

    BOOL EnableSlave(void);
    BOOL DisableSlave(void);
    BOOL SetSlaveText(PBYTE pBufIn, DWORD dwLen);
    BOOL GetSlaveText(PBYTE pBufOut, DWORD dwLen, PDWORD pdwActualOut);
    BOOL SetSlaveSize(PBYTE pBufIn, DWORD dwLen);
    BOOL GetSlaveSize(PBYTE pBufOut, DWORD dwLen, PDWORD pdwActualOut);

    PI2CSLAVEBUF SetSlaveBuf(PI2CSLAVEBUF pSlaveBuf)
    {
        PI2CSLAVEBUF ret;
        ret = pSBuf;
        pSBuf = pSlaveBuf;
        return ret;
    }

    PI2CSLAVEBUF GetSlaveBuf(void)
    {
        return pSBuf;
    }

private:

    //-------------------------------------
    // I2C Private Helper methods
    //
    BOOL GenerateStart(PI2C_PACKET pI2CPkt);
    BOOL GenerateRepeatedStart(PI2C_PACKET pI2CPkt, BOOL bRSTACycleComplete=FALSE);
    VOID GenerateStop();
    BOOL WritePacket(PI2C_PACKET pI2CPkt, BOOL bLast);
    BOOL ReadPacket(PI2C_PACKET pI2CPkt, BOOL bLast, BOOL bAddrCycleComplete, BOOL *pbRSTACycleComplete);

    BOOL MCU_GenerateStart(PI2C_PACKET pI2CPkt);
    BOOL MCU_GenerateRepeatedStart(PI2C_PACKET pI2CPkt, BOOL bRSTACycleComplete=FALSE);
    VOID MCU_GenerateStop();
    BOOL MCU_WritePacket(PI2C_PACKET pI2CPkt, BOOL bLast);
    BOOL MCU_ReadPacket(PI2C_PACKET pI2CPkt, BOOL bLast, BOOL bAddrCycleComplete, BOOL *pbRSTACycleComplete);

    BOOL CleanupPendingInterrupt(void);
    //-------------------------------------
    // I2C Private Helper methods
    //    
    static void WINAPI I2CIST(LPVOID lpParameter);
    void I2CInterruptHandler(UINT32 timeout);


    //-------------------------------------
    // I2C Module Index
    //
    UINT32 m_iModuleIndex;

    //-------------------------------------
    // I2C bus critical section
    //
    CRITICAL_SECTION gcsI2CBusLock;
    CRITICAL_SECTION gcsI2CSlaveLock;
    
    //-------------------------------------
    // I2C INTERRUPT HANDLING
    //
    HANDLE m_hI2CIST;

    HANDLE hInterrupted;                // Hardware Interrupt Occurence Event (Shared)
    HANDLE m_hI2CIntrEvent;             // Interrupt Occurence Event (Shared)


    HANDLE m_hI2CUpdateSlaveEvent;      // Interrupt Occurence Event (Shared)
    volatile 
    BOOL   bSlaveEventNotified;         // Interrupt Occurence Event (Shared)
    BOOL   bTerminateISTThread;         // Boolean flag used to allow the IST
                                        // thread to gracefully terminate.

    BOOL bDispatchEvent;                // Interrupt routine need dispatch event


    //-------------------------------------
    // I2C PROPERTIES
    //

    WORD wLastClkRate;                  // Last Known Frequency
    INT iResult;                        // Last Known Result
    BYTE byLastMode;                    // Last Known Mode
    BYTE byLastSelfAddr;                // My Last Known Address
    INT iIntrWaitTimeout;               // Time to wait for interrupt before timeout
    INT iIntrWaitTimeoutRetry;          // Retry times before fail 

    //-------------------------------------
    // I2C TRANSACTION FIELDS   
    //

    PCSP_I2C_REG pI2CReg;           // I2C Register Base Mapped Virtual Address
    DWORD dwSysIntr;                // System Interrupt ID
    
public:
    CEDEVICE_POWER_STATE m_dxCurrent;
    BOOL m_bUsePolling;             // request I2C Master working in polling mode
    BOOL m_bDownGradeToPolling;          // I2C Master working in polling mode
    BOOL m_bDelayCleanupPendingEvent;    // I2C Cleanup request during turn to `power down' state

private:
    PI2CSLAVEBUF pSBuf;
    BYTE byInUseSlaveAddr;          // My Last Slave Address

public:
    BOOL bSlaveInUse;

};



//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

extern BOOL BSPI2CIOMUXConfig(UINT32 index);
extern BOOL BSPI2CEnableClock(UINT32 index, BOOL bEnable);
extern INT  BSPGetTimeoutValue(void);
extern UINT32 I2CGetBaseRegAddr(UINT32 index);
extern DWORD I2CGetIRQ(UINT32 index);
extern BOOL BSPI2CGetModuleClock(UINT32 index, PDWORD pdwFreq);

#ifdef __cplusplus
}
#endif

#endif   // _INC_I2C_H


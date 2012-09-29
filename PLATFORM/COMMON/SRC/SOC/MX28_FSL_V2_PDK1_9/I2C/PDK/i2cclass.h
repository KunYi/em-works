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
//  File:  i2cclass.h
//
//  Header file, for STMP I2C bus driver.
//
//------------------------------------------------------------------------------

#ifndef _INC_I2CCLASS_H
#define _INC_I2CCLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "csp.h"

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

// number of buffers in the DMA chain
#define NUM_BUFFERS_IN_CHAIN       8

// maximum bytes specifiable in XFER_COUNT 63.99KB
#define MAX_XFER_COUNT                  0xFFFF

// default frequency value for I2C transactions
#define DEFAULT_FREQUENCY           100000            // 100 khz



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

// I2C DMA Chain: each buffer can transfer a max of ~64K
typedef struct I2C_DMA_BUFFER
{
    DWORD pNextBuff;  // address of next buffer in the descriptor chain
    DWORD dwDMACmd; // DMA command word
    DWORD pbDataBufferAddress;        // PHYSICAL ADDRESS of data buffer to be read/written
    DWORD dwCtrl0;      // control word
} I2C_DMA_BUFFER;


typedef struct {
    DWORD   pSrcSize;
    PVOID   pSrcVirtualAddr;
    PVOID   pBufferedVirtualAddr;
    PHYSICAL_ADDRESS physicalAddress;    
} DMA_BUFFERED_BUFFER, *PDMA_BUFFERED_BUFFER;

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
    // Clock Rate to the I2C device
    //
    DWORD GetClockRate(void)
    {
        return dwLastClkRate;
    }

    DWORD SetClockRate(DWORD wClkRate)
    {
        dwLastClkRate = wClkRate;
        return dwLastClkRate;
    }

	//
	// CS&ZHL MAY-21-2012: move Pin config into open
	//
	BOOL  PinConfig(void);

	//-----------------------------------------------------------------------------
	// CS&ZHL JUN-14-2012: support simple I2C Read/Write in master mode
	//
	// input: 
	//          uHwAddr: 7-bit slave hardware address + 1-bit R/W flag in D0(LSB)
	//			dwCmd = 0xFFFFFFFF: invalid cmd, ignore
	//          dwCmd.D31 = 0: single-byte cmd
	//                    = 1: double-byte cmd, NOTE: hi-byte send first!
	//
	DWORD MasterRead(BYTE uHwAddr, DWORD dwCmd, PBYTE pBuf, DWORD dwLength);
	DWORD MasterWrite(BYTE uHwAddr, DWORD dwCmd, PBYTE pBuf, DWORD dwLength);
	//-----------------------------------------------------------------------------
	
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

    BOOL ProcessPackets(I2C_PACKET packets[], DWORD numPackets);

    VOID BusyWait(void);

    //-------------------------------------
    // I2C CR METHODS
    //

    VOID Reset(void); // Only reset I2C, does not affect the class fields

    //-------------------------------------
    // I2C CLOCK METHODS
    //
    
    VOID SetupTimingRegs(VOID);

    //-------------------------------------
    // I2C Slave IOCONTROL function
    //

    BOOL EnableSlave(void);
    BOOL DisableSlave(void);

private:

    //-------------------------------------
    // I2C Private Helper methods
    //

    //-------------------------------------
    // I2C bus critical section
    //
    CRITICAL_SECTION gcsI2CBusLock;
    
    //-------------------------------------
    // I2C INTERRUPT HANDLING
    //

    HANDLE hInterrupted;                // Hardware Interrupt Occurence Event (Shared)
    DWORD dwTimeout;                    // how long to wait for interrupt before timing out?

    //-------------------------------------
    // I2C PROPERTIES
    //

    DWORD dwLastClkRate;                  // Last Known Frequency
    INT iResult;                        // Last Known Result
    BYTE byLastMode;                    // Last Known Mode
    BYTE byLastSelfAddr;                // My Last Known Address

    //-------------------------------------
    // I2C TRANSACTION FIELDS   
    //

    PVOID pI2CReg0, pI2CReg1;           // I2C Register Base Mapped Virtual Address
    DWORD dwSysIntr;                // System Interrupt ID
    DWORD m_index;
public:
    CEDEVICE_POWER_STATE m_dxCurrent;

private:
    PI2CSLAVEBUF pSBuf;
    BYTE byInUseSlaveAddr;          // My Last Slave Address

    // DMA chain: since 16 buffers and each can transfer max ~64KB, max transfer in chain ~1MB
    I2C_DMA_BUFFER *m_pDescChain[NUM_BUFFERS_IN_CHAIN];
    DWORD m_dwDescChainPhysAddr;

    CE_DMA_ADAPTER m_DMAAdapter;
    DMA_BUFFERED_BUFFER m_DMABuffers[NUM_BUFFERS_IN_CHAIN];


public:
    BOOL bInUse;

};



//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

extern BOOL  BSPI2CIOMUXConfig(DWORD index);
extern DWORD BSPI2CGetTimeout();
extern BYTE	 BSPI2CGetHWAddr(LPVOID pBuf, DWORD dwLength);
extern DWORD BSPI2CGetCmd(LPVOID pBuf, DWORD dwLength);
extern PBYTE BSPI2CGetDataBuffer(LPVOID pBuf, DWORD dwLength);
extern DWORD BSPI2CGetDataLength(LPVOID pBuf, DWORD dwLength);

#ifdef __cplusplus
}
#endif

#endif   // _INC_I2C_H


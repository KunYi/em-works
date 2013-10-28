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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  oal_i2c.c
//
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <i2c.h>
#include <oal_prcm.h>
#include <oal_i2c.h>

//required for sharing I2C functions between kernel and other processes
#if (_WINCEOSVER<600)
extern HANDLE   SC_CreateMutex(LPSECURITY_ATTRIBUTES lpsa, BOOL bInitialOwner, LPCTSTR lpName);
extern DWORD    SC_WaitForMultiple(DWORD cObjects, CONST HANDLE *lphObjects, BOOL fWaitAll, DWORD dwTimeout);
extern BOOL     SC_ReleaseMutex(HANDLE hMutex);
#endif

#define DEVICE_ID(x)        (OMAP_DEVICE_I2C1 + x)

//-----------------------------------------------------------------------------
extern I2CDevice_t          _rgI2CDevice[];
extern I2CScaleTable_t      _rgScaleTable[];
extern DWORD const          I2CDefaultI2CTimeout;

//-----------------------------------------------------------------------------
static BOOL _bPostInit = FALSE;

//-----------------------------------------------------------------------------
static const UINT16 s_fifoLookup[] = {
    8,
    16,
    32,
    64
};

static const TCHAR sp_I2C_MutexNames[kI2C_Count][20] =
{
    TEXT("I2C_Mutex_1"),
    TEXT("I2C_Mutex_2"),
    TEXT("I2C_Mutex_3")
};


//-----------------------------------------------------------------------------
void 
OALI2CLock(
    UINT idI2C
    )
{
    if (_bPostInit == TRUE && INTERRUPTS_STATUS())
        {
#if (_WINCEOSVER<600)
        SC_WaitForMultiple(1, &_rgI2CDevice[idI2C].mutex, TRUE, INFINITE);
#else
        EnterCriticalSection(&_rgI2CDevice[idI2C].cs);
#endif
}
}

//-----------------------------------------------------------------------------
void 
OALI2CUnlock(
    UINT idI2C
    )
{
    if (_bPostInit == TRUE && INTERRUPTS_STATUS()) 
        {
#if (_WINCEOSVER<600)
        SC_ReleaseMutex(_rgI2CDevice[idI2C].mutex);
#else
        LeaveCriticalSection(&_rgI2CDevice[idI2C].cs);
#endif
}
}

//-----------------------------------------------------------------------------
static
DWORD 
OALI2CGetTickCount()
{
    return (_bPostInit == TRUE) ? OALGetTickCount() : 0;
}

//-----------------------------------------------------------------------------
static
BOOL
OALI2CSetDeviceBaudrate(
    I2CDevice_t      *pDevice, 
    DWORD             baudIndex
    )
{
    pDevice->currentBaudIndex = baudIndex;
    OUTREG16(&pDevice->pI2CRegs->PSC,  _rgScaleTable[pDevice->currentBaudIndex].psc);
    OUTREG16(&pDevice->pI2CRegs->SCLL, _rgScaleTable[pDevice->currentBaudIndex].scll);
    OUTREG16(&pDevice->pI2CRegs->SCLH, _rgScaleTable[pDevice->currentBaudIndex].sclh);

    return TRUE;
}

//-----------------------------------------------------------------------------
__inline
BOOL
OALI2CPollStatus(
    volatile unsigned short *pReg,
    UINT16              mask,
    UINT16              val,
    UINT32              timeout
    ) 
{
    BOOL rc = FALSE;
    UINT matchTime = OALI2CGetTickCount() + timeout;
    
    while ((INREG16(pReg) & mask) != val)
        {
        if (OALI2CGetTickCount() > matchTime)
            {
            OALMSG(OAL_LOG_WARN, (
                L"WARN: I2C::OALI2CPollStatus - check has timed out actual"
                L"(0x%04X), expect(0x%04X)\r\n", 
                (INREG16(pReg) & mask), val
                ));
            goto cleanUp;
            }
        }    
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
static
BOOL
OALI2CResetDevice(
    I2CDevice_t          *pDevice
    ) 
{
    UINT16 stat;
    BOOL bRet = TRUE;
    OMAP_I2C_REGS *pI2CRegs = pDevice->pI2CRegs;

    // disable i2c
    OUTREG16(&pI2CRegs->CON, 0);

    // put i2c in reset
    OUTREG16(&pI2CRegs->SYSC, I2C_SYSC_SRST);

    // pull i2c out of reset by setting I2C_CON_EN
    SETREG16(&pI2CRegs->CON, I2C_CON_EN);
    
    if (OALI2CPollStatus(&pI2CRegs->SYSS, I2C_SYSS_RDONE, I2C_SYSS_RDONE, 10) == FALSE)
        {
        OALMSG(OAL_LOG_WARN, (
            L"WARN: I2C::OALI2CResetDevice - failed to reset device\r\n"
            ));
        bRet = FALSE;
        }

    // disable i2c
    OUTREG16(&pI2CRegs->CON, 0);

    // clear status
    stat = INREG16(&pI2CRegs->STAT);
    OUTREG16(&pI2CRegs->STAT, stat);

    // Set prescaler and low/high periods
    OALI2CSetDeviceBaudrate(pDevice, pDevice->currentBaudIndex);

    // UNDONE:
    //   NEED remove hardcoding of thresholds
    OUTREG16(&pI2CRegs->BUF, 
        I2C_BUF_XTRSH(pDevice->txFifoThreshold) | 
        I2C_BUF_RTRSH(pDevice->rxFifoThreshold)
        );

    // configure own address
    OUTREG16(&pI2CRegs->OA0, I2C_MASTER_CODE | pDevice->ownAddress);

    return bRet;
}

//-----------------------------------------------------------------------------
__inline
UINT
OALI2CGetBufferSize(
    I2C_PACKET_INFO_t   *pPacket,
    UINT                 idxBuffer
    )
{
    return pPacket->rgBuffers[idxBuffer].size;
}

//-----------------------------------------------------------------------------
__inline
UINT
OALI2CGetPacketSize(
    I2C_PACKET_INFO_t   *pPacket
    )
{
    UINT i;
    UINT totalSize = 0;
    for (i = 0; i < pPacket->count; ++i)
        {
        totalSize += pPacket->rgBuffers[i].size;
        }
    return totalSize;
}

//-----------------------------------------------------------------------------
BOOL
OALI2CInit(
    UINT            devId, 
    OMAP_I2C_REGS  *pI2CRegs
    )
{
    // capture i2c id
    UINT idI2C = devId - OMAP_DEVICE_I2C1;
    
    // store reference to i2c registers
    I2CDevice_t *pDevice = (I2CDevice_t*)&_rgI2CDevice[idI2C];

    // Enable interface and functional clock of I2C's 
    PrcmDeviceEnableClocks(devId, TRUE);

    // current baud rate is always initially the default baud rate
    _rgI2CDevice[idI2C].currentBaudIndex = _rgI2CDevice[idI2C].defaultBaudIndex;

    // initialize i2c
    pDevice->pI2CRegs = pI2CRegs;
    OALI2CResetDevice(pDevice);

    // update fifo threshold size
    _rgI2CDevice[idI2C].fifoSize = INREG32(&pI2CRegs->BUFSTAT);
    _rgI2CDevice[idI2C].fifoSize &= I2C_BUFSTAT_FIFODEPTH_MASK;
    _rgI2CDevice[idI2C].fifoSize >>= I2C_BUFSTAT_FIFODEPTH_SHIFT;
    _rgI2CDevice[idI2C].fifoSize = s_fifoLookup[_rgI2CDevice[idI2C].fifoSize];

    // release clocks
    PrcmDeviceEnableClocks(devId, FALSE);

    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL
OALI2CPostInit(
    )
{
    // create mutex objects
    int idI2C;

    for (idI2C = 0; idI2C < kI2C_Count; ++idI2C)
        {
#if (_WINCEOSVER<600)
        _rgI2CDevice[idI2C].mutex = SC_CreateMutex(NULL, FALSE, sp_I2C_MutexNames[idI2C]);
#else
        InitializeCriticalSection(&_rgI2CDevice[idI2C].cs);
#endif
        }

    _bPostInit = TRUE;
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL
OALI2CSetSlaveAddress(
    void           *hCtx, 
    UINT16          slaveAddress
    )
{
    ((I2CContext_t*)hCtx)->slaveAddress = slaveAddress;
    return TRUE;
}

//-----------------------------------------------------------------------------
void*
OALI2COpen(
    UINT            devId,
    void           *pData
    )
{    
    UINT idI2C;
    I2CContext_t *pCtx = (I2CContext_t*)pData;
    idI2C = devId - OMAP_DEVICE_I2C1;

    //OALMSG(OAL_LOG_WARN, (L"I2C: OALI2COpen bus %d\n", idI2C));

    // initialize structure
    pCtx->idI2C = idI2C;
    pCtx->baudIndex = _rgI2CDevice[idI2C].defaultBaudIndex;
    pCtx->timeOut = I2CDefaultI2CTimeout;
    pCtx->slaveAddress = 0;
    pCtx->subAddressMode = I2C_SUBADDRESS_MODE_8;

    return pCtx;
}

//------------------------------------------------------------------------------
void
OALI2CClose(
    void           *hI2C
    )
{
    //OALMSG(OAL_LOG_WARN, (L"I2C: OALI2CClose\n"));
}

//------------------------------------------------------------------------------
UINT
OALI2CWrite(
    VOID           *hCtx,
    UINT32          subaddr,
    VOID           *pBuffer,
    UINT32          size
    )
{
    I2CResult_e             rc;
    UINT                    nAttempts;
    UINT16                  con_mask;
    UINT                    writeCount = -1;

    int                     payloadCount;
    I2C_PACKET_INFO_t       packetInfo;
    I2C_TRANSACTION_INFO_t  transactInfo;
    I2C_BUFFER_INFO_t       rgBufferInfo[2];

    I2CContext_t           *pCtx = (I2CContext_t*)hCtx;
    UINT                    idI2C = pCtx->idI2C;
    I2CDevice_t            *pDevice = (I2CDevice_t*)&_rgI2CDevice[pCtx->idI2C];
    OMAP_I2C_REGS          *pI2CRegs = _rgI2CDevice[pCtx->idI2C].pI2CRegs;

    // Get hardware
    OALI2CLock(idI2C);

    //OALMSG(OAL_LOG_WARN, (L"I2C: OALI2CWrite bus %d\n", idI2C));

    // Set this driver to Active Mode
    PrcmDeviceEnableClocks(DEVICE_ID(pCtx->idI2C), TRUE);

    if (PrcmDeviceGetContextState(DEVICE_ID(pCtx->idI2C), TRUE) == FALSE)
        {
        OALI2CResetDevice(pDevice);
        }

    // setup CONnection mask
    con_mask = I2C_CON_MST;
    if ((pDevice->ownAddress& 0x80) != 0)
        {
        con_mask |= I2C_CON_XSA;
        }

    // set transfer mode bits
    if (pCtx->baudIndex == SLOWSPEED_MODE)
        {
        con_mask |= I2C_CON_OPMODE_FS; 
        }
    else if (pCtx->baudIndex == FULLSPEED_MODE)
        {
        con_mask |= I2C_CON_OPMODE_FS;
        }
    else
        {
        con_mask |= I2C_CON_OPMODE_HS;
        }

    // fill in operation info
    payloadCount = 0;
    if (pCtx->subAddressMode != I2C_SUBADDRESS_MODE_0)
        {
        rgBufferInfo[payloadCount].size = pCtx->subAddressMode;
        rgBufferInfo[payloadCount].pBuffer = (UCHAR*)&subaddr;
        ++payloadCount;
        }
    
    rgBufferInfo[payloadCount].size = size;
    rgBufferInfo[payloadCount].pBuffer = (UCHAR*)pBuffer;
    ++payloadCount;

    packetInfo.count = payloadCount;
    packetInfo.opType = kI2C_Write;
    packetInfo.rgBuffers = rgBufferInfo;

    transactInfo.count = 1;
    transactInfo.con_mask = con_mask;
    transactInfo.rgPackets = &packetInfo;

    // check if baud rate is different from current settings
    if (pDevice->currentBaudIndex != pCtx->baudIndex)
        {
        OALI2CSetDeviceBaudrate(pDevice, pCtx->baudIndex);
        }
    
    // write data
    nAttempts = 0;
    do
        {
        OALMSG(OAL_LOG_WARN && nAttempts, 
            (L"Write: Attempts = %d\r\n", (nAttempts + 1)));

        nAttempts++;
        rc = OALI2CTransaction(&transactInfo, pCtx);
        }
        while (rc == kI2CRetry && (nAttempts < pDevice->maxRetries));
            
    if (rc != kI2CSuccess)
        {
        OALMSG(OAL_LOG_WARN, (L"WARN: I2C: Write failed "
            L"(SA=0x%02X, register=0x%08X)\r\n",
            pCtx->slaveAddress, subaddr
            ));
        
        writeCount = -1;
        goto cleanUp;
        }

    // report number of bytes written
    writeCount = rgBufferInfo[payloadCount - 1].size;
    
cleanUp:

    // disable device
    OUTREG16(&pDevice->pI2CRegs->CON, 0);

    // Set this driver to suspend
    PrcmDeviceEnableClocks(DEVICE_ID(pCtx->idI2C), FALSE);

    // Release hardware    
    OALI2CUnlock(idI2C);

    return writeCount;
}

//------------------------------------------------------------------------------
UINT
OALI2CRead(
    VOID       *hCtx,
    UINT32      subaddr,
    VOID       *pBuffer,
    UINT32      size
    )
{
    I2CResult_e             rc;
    UINT                    nAttempts;
    UINT16                  con_mask;
    UINT                    readCount = -1;

    int                     payloadCount;
    I2C_TRANSACTION_INFO_t  transactInfo;    
    I2C_PACKET_INFO_t       rgPacketInfo[2];
    I2C_BUFFER_INFO_t       rgBufferInfo[2];

    I2CContext_t           *pCtx = (I2CContext_t*)hCtx;
    UINT                    idI2C = pCtx->idI2C;
    I2CDevice_t            *pDevice = (I2CDevice_t*)&_rgI2CDevice[pCtx->idI2C];
    OMAP_I2C_REGS          *pI2CRegs = _rgI2CDevice[pCtx->idI2C].pI2CRegs;

    // Get hardware
    OALI2CLock(idI2C);

    //OALMSG(OAL_LOG_WARN, (L"I2C: OALI2CRead bus %d\n", idI2C));

    // Set this driver to Active Mode
    PrcmDeviceEnableClocks(DEVICE_ID(pCtx->idI2C), TRUE);

    if (PrcmDeviceGetContextState(DEVICE_ID(pCtx->idI2C), TRUE) == FALSE)
        {
        OALI2CResetDevice(pDevice);
        }

    // setup CONnection mask
    con_mask = I2C_CON_MST;
    if ((pDevice->ownAddress& 0x80) != 0)
        {
        con_mask |= I2C_CON_XSA;
        }

    // set transfer mode bits
    if (pCtx->baudIndex == SLOWSPEED_MODE)
        {
        con_mask |= I2C_CON_OPMODE_FS; 
        }
    else if (pCtx->baudIndex == FULLSPEED_MODE)
        {
        con_mask |= I2C_CON_OPMODE_FS;
        }
    else
        {
        con_mask |= I2C_CON_OPMODE_HS;
        }

    // fill in operation info
    payloadCount = 0;
    if (pCtx->subAddressMode != I2C_SUBADDRESS_MODE_0)
        {
        rgBufferInfo[payloadCount].size = pCtx->subAddressMode;
        rgBufferInfo[payloadCount].pBuffer = (UCHAR*)&subaddr;

        rgPacketInfo[payloadCount].count = 1;
        rgPacketInfo[payloadCount].opType = kI2C_Write;
        rgPacketInfo[payloadCount].rgBuffers = &rgBufferInfo[payloadCount];
        ++payloadCount;
        }

    rgBufferInfo[payloadCount].size = size;
    rgBufferInfo[payloadCount].pBuffer = (UCHAR*)pBuffer;

    rgPacketInfo[payloadCount].count = 1;
    rgPacketInfo[payloadCount].opType = kI2C_Read;
    rgPacketInfo[payloadCount].rgBuffers = &rgBufferInfo[payloadCount];
    ++payloadCount;

    // link packets
    transactInfo.count = payloadCount;
    transactInfo.con_mask = con_mask;
    transactInfo.rgPackets = rgPacketInfo;

    // check if baud rate is different from current settings
    if (pDevice->currentBaudIndex != pCtx->baudIndex)
        {
        OALI2CSetDeviceBaudrate(pDevice, pCtx->baudIndex);
        }
    
    // write data
    nAttempts = 0;
    do
        {
        OALMSG(OAL_LOG_WARN && nAttempts, 
            (L"Read: Attempts = %d\r\n", (nAttempts + 1)));

        nAttempts++;
        rc = OALI2CTransaction(&transactInfo, pCtx);
        }
        while (rc == kI2CRetry && (nAttempts < pDevice->maxRetries));
            
    if (rc != kI2CSuccess)
        {
        OALMSG(OAL_LOG_WARN, (L"WARN: I2C: Read failed "
            L"(slave addr=0x%02X, register=0x%08X)\r\n",
            pCtx->slaveAddress, subaddr
            ));
        
        readCount = -1;
        goto cleanUp;
        }

    // report number of bytes written
    readCount = rgBufferInfo[payloadCount - 1].size;
    
cleanUp:

    // disable device
    OUTREG16(&pDevice->pI2CRegs->CON, 0);

    // Set this driver to suspend
    PrcmDeviceEnableClocks(DEVICE_ID(pCtx->idI2C), FALSE);

    // Release hardware    
    OALI2CUnlock(idI2C);

    return readCount;
}

//------------------------------------------------------------------------------
static
BOOL 
OALI2CReprogramDevice(
    I2CContext_t             *pCtx
    )
{
    UINT startTime;
    BOOL result = FALSE;
    I2CDevice_t *pDevice = (I2CDevice_t*)&_rgI2CDevice[pCtx->idI2C];
    OMAP_I2C_REGS *pI2CRegs = pDevice->pI2CRegs;

    // Set slave address
    OUTREG16(&pI2CRegs->SA, pCtx->slaveAddress);

    startTime = OALI2CGetTickCount();
    while (((INREG16(&pI2CRegs->STAT) & I2C_STAT_BB) != 0))
        {        
        if ((OALI2CGetTickCount() - startTime) > pCtx->timeOut)
            {
            // reset the I2C controller
            result = OALI2CResetDevice(pDevice);            
            OALMSG(OAL_LOG_ERROR, (L"ERROR: OALI2CReprogramDevice: "
                L"Bus remains locked -- resetting I2C controller 0x%02x\r\n",
                pCtx->slaveAddress
                ));
            if (result == FALSE)
                {
                goto cleanUp;
                }
            }
        }

    result = TRUE;
    
cleanUp:
    return result;
}

//------------------------------------------------------------------------------
I2CResult_e
OALI2CTransaction(
    I2C_TRANSACTION_INFO_t *pInfo,
    I2CContext_t           *pCtx
    )
{
    UINT16                  stat;
    UINT                    con_mask;
    
    BOOL                    bHSMode;

    UINT                    maxTime;
    UINT                    startTime;

    UCHAR                  *pData;
    I2C_PACKET_INFO_t      *pPacket;    
    UINT                    idxBuffer, idxPacket;
    UINT                    copyBufferCount, copyPacketCount;
    UINT                    remainingInBuffer, remainingInPacket;

    UINT                    maxFifoSize;
    UINT                    TxFifoThreshold, RxFifoThreshold;

    I2CResult_e             rc = kI2CRetry;
    I2CDevice_t            *pDevice = (I2CDevice_t*)&_rgI2CDevice[pCtx->idI2C];
    OMAP_I2C_REGS          *pI2CRegs = _rgI2CDevice[pCtx->idI2C].pI2CRegs;

    // try to reprogram device
    if (OALI2CReprogramDevice(pCtx) == FALSE)
        {
        OALMSG(OAL_LOG_ERROR, (L"ERROR: WriteI2CData: "
            L"Unable to program i2c device\r\n"
            ));
            goto cleanUp;
        }

    // check for high speed mode
    bHSMode = (pInfo->con_mask & I2C_CON_OPMODE_HS) != 0;

    // clear status
    stat = INREG16(&pI2CRegs->STAT);
    OUTREG16(&pI2CRegs->STAT, stat);

    // get timeout
    maxTime = pCtx->timeOut;

    // get fifo size
    maxFifoSize = pDevice->fifoSize;

    // get fifo thresholds
    RxFifoThreshold = pDevice->rxFifoThreshold;
    TxFifoThreshold = pDevice->txFifoThreshold;
    
    // clear all FIFO's
    SETREG16(&pI2CRegs->BUF, I2C_BUF_TXFIFO_CLR | I2C_BUF_RXFIFO_CLR);

    // initialize for first operation
    //
    idxPacket = 0;
    idxBuffer = 0;
    remainingInPacket = 0;
    remainingInBuffer = 0;
    startTime = OALI2CGetTickCount();

    // operation starts with an ARDY event
    stat = I2C_STAT_ARDY;
    do
        {
        // check for errors
        if (stat & (I2C_STAT_NACK | I2C_STAT_AL | I2C_STAT_AERR))
            {
            // Reset i2c controller
            DEBUGMSG(OAL_LOG_WARN && (stat & I2C_STAT_NACK), (L"WARN: TransactI2CPacket: "
                L"No ACK from slave device with address 0x%02x, stat = 0x%x\r\n",
                pCtx->slaveAddress, stat
                ));

            DEBUGMSG(OAL_LOG_WARN && (stat & I2C_STAT_AL), (L"WARN: TransactI2CPacket: "
                L"Lost arbitration with address 0x%02x, stat = 0x%x\r\n",
                pCtx->slaveAddress, stat
                ));

            DEBUGMSG(OAL_LOG_WARN && (stat & I2C_STAT_AERR), (L"WARN: TransactI2CPacket: "
                L"Access error with address 0x%02x, stat = 0x%x\r\n",
                pCtx->slaveAddress, stat
                ));

            pPacket->result = 0;
            break;
            }
        
        // sent data
        if (stat & (I2C_STAT_XDR | I2C_STAT_XRDY | I2C_STAT_XUDF))
            {            
            if (remainingInPacket && pPacket->opType == kI2C_Write)
                {
                // determine number of bytes to copy
                if (stat & I2C_STAT_XUDF)
                    {
                    copyPacketCount = maxFifoSize;
                    }
                else if (stat & I2C_STAT_XDR)
                    {
                    copyPacketCount = INREG16(&pI2CRegs->BUFSTAT);
                    copyPacketCount &= I2C_BUFSTAT_TXSTAT_MASK;
                    copyPacketCount >>= I2C_BUFSTAT_TXSTAT_SHIFT;
                    }                
                else 
                    {
                    copyPacketCount = TxFifoThreshold;
                    }

                // only write expected amount
                copyPacketCount = min(copyPacketCount, remainingInPacket); 

                // update counters
                remainingInPacket -= copyPacketCount;
                pPacket->result += copyPacketCount;
                while (copyPacketCount)
                    {
                    // only write up to what is remaining in current buffer
                    copyBufferCount = min(copyPacketCount , remainingInBuffer);

                    // update buffer counter
                    copyPacketCount -= copyBufferCount;
                    remainingInBuffer -= copyBufferCount;
                    
                    // copy all data to fifo
                    while (copyBufferCount)
                        {
                        OUTREG8(&pI2CRegs->DATA, *pData);
                        ++pData;
                        --copyBufferCount;
                        }
                    
                    // update to next buffer if necessary
                    if (remainingInBuffer == 0 && ++idxBuffer < pPacket->count)
                        {
                        pData = pPacket->rgBuffers[idxBuffer].pBuffer;
                        remainingInBuffer = OALI2CGetBufferSize(pPacket, idxBuffer);
                        }
                    }
                }            
            OUTREG16(&pI2CRegs->STAT, stat & (I2C_STAT_XDR | I2C_STAT_XRDY | I2C_STAT_XUDF));
            }

        // received data
        if ((stat & (I2C_STAT_RDR | I2C_STAT_RRDY | I2C_STAT_ROVR)))
            {
            if (remainingInPacket && pPacket->opType == kI2C_Read)
                {
                // determine number of bytes to copy
                if (stat & I2C_STAT_ROVR)
                    {
                    copyPacketCount = maxFifoSize;
                    }
                else if (stat & I2C_STAT_RDR)
                    {
                    copyPacketCount = INREG16(&pI2CRegs->BUFSTAT);
                    copyPacketCount &= I2C_BUFSTAT_RXSTAT_MASK;
                    copyPacketCount >>= I2C_BUFSTAT_RXSTAT_SHIFT;
                    }                
                else
                    {
                    copyPacketCount = RxFifoThreshold;
                    }

                // only read expected amount
                copyPacketCount = min(copyPacketCount, remainingInPacket); 

                // update counters
                remainingInPacket -= copyPacketCount;
                pPacket->result += copyPacketCount;
                while (copyPacketCount)
                    {
                    // only write up to what is remaining in current buffer
                    copyBufferCount = min(copyPacketCount , remainingInBuffer);

                    // update buffer counter
                    copyPacketCount -= copyBufferCount;
                    remainingInBuffer -= copyBufferCount;
                    
                    // copy all data to fifo
                    while (copyBufferCount)
                        {
                        *pData = INREG8(&pI2CRegs->DATA);
                        ++pData;
                        --copyBufferCount;
                        }                   

                    // update to next buffer if necessary
                    if (remainingInBuffer == 0 && ++idxBuffer < pPacket->count)
                        {
                        pData = pPacket->rgBuffers[idxBuffer].pBuffer;
                        remainingInBuffer = OALI2CGetBufferSize(pPacket, idxBuffer);
                        }
                    }
                }
            OUTREG16(&pI2CRegs->STAT, stat & (I2C_STAT_RDR | I2C_STAT_RRDY | I2C_STAT_ROVR));
            }

        // packet complete
        if (stat & I2C_STAT_ARDY)
            {
            // this notificaiton is needed to progress to next packet
            OUTREG16(&pI2CRegs->STAT, I2C_STAT_ARDY);
            
            // move to next transaction            
            if (idxPacket < pInfo->count)
                {            
                // get next packet
                //                
                pPacket = &pInfo->rgPackets[idxPacket];
                ++idxPacket;

                // initialize packet info
                idxBuffer = 0;
                pData = pPacket->rgBuffers[idxBuffer].pBuffer;
                remainingInBuffer = OALI2CGetBufferSize(pPacket, idxBuffer);
                remainingInPacket = OALI2CGetPacketSize(pPacket);

                // Start transaction                
                con_mask = I2C_CON_EN | I2C_CON_STT | pInfo->con_mask;
                if (pPacket->opType == kI2C_Write) con_mask |= I2C_CON_TRX;                
                if (idxPacket == pInfo->count) con_mask |= I2C_CON_STP;
                OUTREG16(&pI2CRegs->CON, con_mask); 
                OUTREG16(&pI2CRegs->CNT, remainingInPacket);

                ASSERT(remainingInPacket != 0);
                
                // if writing to bus fill-up tx fifo to avoid underflows
                pPacket->result = 0;
                if (pPacket->opType == kI2C_Write)
                    {
                    stat = I2C_STAT_XDR | I2C_STAT_XRDY | I2C_STAT_XUDF;
                    continue;
                    }
                }
            else
                {
                // if we get here then all packets went through successfully
                rc = kI2CSuccess;
                break;
                }
            }

        // handle possible glitches on the i2c bus or ill behaved i2c device
        if (((stat == I2C_STAT_BF) || (stat == (I2C_STAT_BF | I2C_STAT_BB))) && 
            (remainingInPacket > 0))
            {
            // reprogram and try again
            DEBUGMSG(OAL_LOG_WARN && stat == (I2C_STAT_BF | I2C_STAT_BB), 
                (L"WARN: TransactI2CPacket: Lost transaction state "
                L"bus is both busy and free!! address 0x%02x, stat = 0x%x\r\n",
                pCtx->slaveAddress, stat
                ));

            // reprogram and try again
            DEBUGMSG(OAL_LOG_WARN && stat == I2C_STAT_BF, 
                (L"WARN: TransactI2CPacket: Lost transaction state "
                L"with address 0x%02x, stat = 0x%x\r\n",
                pCtx->slaveAddress, stat
                ));

            break;
            }

        // get current status
        stat = INREG16(&pI2CRegs->STAT);

        // workaround to support HS I2C
        // for high speed mode ARDY is not triggered all the time. 
        // we need to spoof this for the high speed case
        if (bHSMode && remainingInPacket == 0) 
            {
            // check buffer status to determine if all data is completely drained
            // before continuing to next packet            
            if (pPacket->opType == kI2C_Write)
                {
                copyPacketCount = INREG16(&pI2CRegs->BUFSTAT);
                copyPacketCount &= I2C_BUFSTAT_TXSTAT_MASK;
                copyPacketCount >>= I2C_BUFSTAT_TXSTAT_SHIFT;
                if (copyPacketCount == pPacket->result) stat |= I2C_STAT_ARDY; 
                }                       
            }
        } while ((OALI2CGetTickCount() - startTime) < maxTime);

cleanUp:
    if (rc != kI2CSuccess)
        {
        /*
        #define SHOW_I2C_REG(reg)   OALMSG(OAL_LOG_WARN, (L"I2C: %s = 0x%04x\n", TEXT(# reg), pI2CRegs->reg));
        SHOW_I2C_REG(REV)
        SHOW_I2C_REG(IE)
        SHOW_I2C_REG(STAT)
        SHOW_I2C_REG(WE)
        SHOW_I2C_REG(SYSS)
        SHOW_I2C_REG(BUF)
        SHOW_I2C_REG(CNT)
        //SHOW_I2C_REG(DATA)
        SHOW_I2C_REG(SYSC)
        SHOW_I2C_REG(CON)
        SHOW_I2C_REG(OA0)
        SHOW_I2C_REG(SA)
        SHOW_I2C_REG(PSC)
        SHOW_I2C_REG(SCLL)
        SHOW_I2C_REG(SCLH)
        SHOW_I2C_REG(SYSTEST)
        SHOW_I2C_REG(BUFSTAT)
        SHOW_I2C_REG(OA1)
        SHOW_I2C_REG(OA2)
        SHOW_I2C_REG(OA3)
        SHOW_I2C_REG(ACTOA)
        SHOW_I2C_REG(SBLOCK)
        */
        }       
        
    stat = INREG16(&pI2CRegs->STAT);
    OUTREG16(&pI2CRegs->STAT, stat);

    OUTREG16(&pI2CRegs->CON, 0);
    
    return rc;
}

//------------------------------------------------------------------------------

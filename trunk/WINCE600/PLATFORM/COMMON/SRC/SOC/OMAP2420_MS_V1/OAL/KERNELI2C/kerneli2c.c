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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#include <bsp.h>
#include "kerneli2c.h"

static OMAP2420_PRCM_REGS  *gpPRCMRegs;
static OMAP2420_I2C_REGS   *gpI2CRegs;

#ifndef BOOTLOADER
static CRITICAL_SECTION     gSec;
#else
#ifdef INTERRUPTS_ENABLE
#undef INTERRUPTS_ENABLE
#endif
#define INTERRUPTS_ENABLE(x)    0
#endif

#pragma optimize("", off)			// debug

static volatile DWORD gCurClockDivisor = I2C_CLOCK_DEFAULT;
    
void KERNELI2C_OEMInit(void)
{
    // get uncached register addresses
    gpI2CRegs = (OMAP2420_I2C_REGS *)OALPAtoUA(OMAP2420_I2C1_REGS_PA);
    gpPRCMRegs = (OMAP2420_PRCM_REGS *)OALPAtoUA(OMAP2420_PRCM_REGS_PA);

    // turn on the clocks - we are inside oeminit so don't need to use kerneliocontrol
    gpPRCMRegs->ulCM_FCLKEN1_CORE |= PRCM_FCLKEN1_CORE_EN_I2C1;
    gpPRCMRegs->ulCM_ICLKEN1_CORE |= PRCM_ICLKEN1_CORE_EN_I2C1;

    // Reset the I2C controller
    gpI2CRegs->I2C_SYSC = I2C_SYSC_SRST;

    // Set base clock as 12MHz & L/H period
    gpI2CRegs->I2C_PSC = 0;

    // Set default L/H periods
    gpI2CRegs->I2C_SCLL = (UINT16)gCurClockDivisor;
    gpI2CRegs->I2C_SCLH = (UINT16)gCurClockDivisor;

	//Write the OWN Address
    gpI2CRegs->I2C_OA = 0x0E;

    // Enable the I2C
    gpI2CRegs->I2C_CON = I2C_CON_EN;

    // Wait until resetting is done
    while (((gpI2CRegs->I2C_SYSS) & I2C_SYSS_RDONE) == 0);

	// Clear number of bytes in the data payload
	gpI2CRegs->I2C_CNT = 0;

    // Clear all interrupts
    gpI2CRegs->I2C_STAT = 0x3F;

    // Disable all interrupts
    gpI2CRegs->I2C_IE = 0x00;

    // now turn off the clocks - we are inside oeminit so don't need to use kernelIOControl
    gpPRCMRegs->ulCM_FCLKEN1_CORE &= ~PRCM_FCLKEN1_CORE_EN_I2C1;
    gpPRCMRegs->ulCM_ICLKEN1_CORE &= ~PRCM_ICLKEN1_CORE_EN_I2C1;
}

void KERNELI2C_HalPostInit(void)
{
#ifndef BOOTLOADER
    InitializeCriticalSection(&gSec);
#endif
}

static BOOL ValidateTransaction(I2CTRANS *pTrans)
{
    DWORD i;

    if (pTrans->mErrorCode)
        return FALSE;
    for(i=0;i<I2CTRANS_MAX_STREAMED_TRANSACTIONS;i++)
    {
        if (!pTrans->mOpCode[i])
            break;
        if (((pTrans->mOpCode[i] & I2C_OPCODE_MASK) != I2C_OPCODE_READ) &&
            ((pTrans->mOpCode[i] & I2C_OPCODE_MASK) != I2C_OPCODE_WRITE))
        {
            /* invalid opcode specified */
            pTrans->mErrorCode = KERNELI2C_ERR_INVALIDOPCODE;
            return FALSE;
        }
        if (!pTrans->mTransLen[i])
        {
            /* transfer length is invalid */
            pTrans->mErrorCode = KERNELI2C_ERR_INVALIDOPLENGTH;
            return FALSE;
        }
        if ((pTrans->mBufferOffset[i]>=I2CTRANS_BUFFER_BYTES) ||
            (pTrans->mBufferOffset[i]+pTrans->mTransLen[i]>I2CTRANS_BUFFER_BYTES))
        {
            /* at least part of transfer is outside range of transfer buffer */
            pTrans->mErrorCode = KERNELI2C_ERR_OPBUFFERINVALID;
            return FALSE;
        }
    }
    return TRUE;
}

static volatile DWORD gCurI2Caddr;
static volatile DWORD gCurI2CaddrSize;
static volatile DWORD gBytesLeft;
static volatile UCHAR *gpData;

static void INTSOFF_I2CTRANS_STARTOP(BOOL isWrite)
{
    DWORD regOut;
    OMAP2420_I2C_REGS *pChkReg = (OMAP2420_I2C_REGS *)OALPAtoUA(OMAP2420_I2C1_REGS_PA);

    /* ensure enabled */
    if (!(gpI2CRegs->I2C_CON & I2C_CON_EN))
    {
        /* i2c is not enabled. reenable/reset.  must happen here */
        gpI2CRegs->I2C_CON = I2C_CON_EN | I2C_CON_MST;
        while (!(gpI2CRegs->I2C_SYSS & I2C_SYSS_RDONE));
        gpI2CRegs->I2C_STAT = 0x3F;
    }

    /* set slave address */
    gpI2CRegs->I2C_SA = (UINT16)gCurI2Caddr;
    regOut = I2C_CON_EN | I2C_CON_MST | I2C_CON_STP | I2C_CON_STT;
    if (isWrite)
        regOut |= I2C_CON_TRX;
    if (gCurI2CaddrSize>7)
        regOut |= I2C_CON_XA;

    gpI2CRegs->I2C_CNT = (UINT16)gBytesLeft;
    gpI2CRegs->I2C_CON = (UINT16)regOut;
}

static void INTSOFF_I2CTRANS_UPDATEREAD()
{
    UINT16 stat;
    OMAP2420_I2C_REGS *pChkReg = (OMAP2420_I2C_REGS *)OALPAtoUA(OMAP2420_I2C1_REGS_PA);

    stat = gpI2CRegs->I2C_STAT;
    if (!(stat & (I2C_STAT_NACK | I2C_STAT_AL | I2C_STAT_RRDY)))
        return;

    /* clear status (except for receive ready) */
    gpI2CRegs->I2C_STAT = (I2C_STAT_NACK | I2C_STAT_AL | I2C_STAT_XRDY);

    if (stat & (I2C_STAT_NACK | I2C_STAT_AL))
    {
        /* error.  if it's a nack we need to reset the controller */
        if (stat & I2C_STAT_NACK)
        {
            gpI2CRegs->I2C_SYSC = (UINT16)I2C_SYSC_SRST;
            gpI2CRegs->I2C_CON = 0;
        }

        /* restart the operation with what is left */
        INTSOFF_I2CTRANS_STARTOP(FALSE);
        return;
    }

    /* received character came in ok (stat & I2C_STAT_RRDY) */
    if (gBytesLeft > 1)
    {
        stat = gpI2CRegs->I2C_DATA;
        *(gpData++) = (UCHAR)(stat & 0xFF);
        *(gpData++) = (UCHAR)((stat & 0xFF00)>>8);
        gBytesLeft -= 2;
    }
    else
    {
        *(gpData++) = (UCHAR)gpI2CRegs->I2C_DATA;
        gBytesLeft = 0;
    }

    /* clear receive status so next char can come in */
    gpI2CRegs->I2C_STAT = I2C_STAT_RRDY;
}

static void INTSOFF_I2CTRANS_UPDATEWRITE()
{
    OMAP2420_I2C_REGS *pChkReg = (OMAP2420_I2C_REGS *)OALPAtoUA(OMAP2420_I2C1_REGS_PA);
    UINT16 stat = gpI2CRegs->I2C_STAT;
    if (!(stat & (I2C_STAT_NACK | I2C_STAT_AL | I2C_STAT_XRDY)))
        return;

    /* clear status (except for transmit ready) */
    gpI2CRegs->I2C_STAT = (I2C_STAT_NACK | I2C_STAT_AL | I2C_STAT_RRDY);

    if (stat & (I2C_STAT_NACK | I2C_STAT_AL))
    {
        /* error.  if it's a nack we need to reset the controller */
        if (stat & I2C_STAT_NACK)
        {
            gpI2CRegs->I2C_SYSC = I2C_SYSC_SRST;
            gpI2CRegs->I2C_CON = 0;
        }

        /* restart the operation with what is left */
        INTSOFF_I2CTRANS_STARTOP(FALSE);
        return;
    }

    /* ready for transmit. load register then issue xmit */
    if (gBytesLeft > 1)
    {
        stat = (USHORT)(*(gpData++));
        stat |= ((USHORT)(*(gpData++)))<<8;
        gpI2CRegs->I2C_DATA = stat;
        gBytesLeft -= 2;
    }
    else
    {
        gpI2CRegs->I2C_DATA = *(gpData++);
        gBytesLeft = 0;
    }

    /* clear transmit status so next char can go out */
    gpI2CRegs->I2C_STAT = I2C_STAT_XRDY;
}

#define I2CSTATE_CLKSETUP   0
#define I2CSTATE_OPSTART    1
#define I2CSTATE_OPUPDATE   2
#define I2CSTATE_OPEND      3
#define I2CSTATE_OPDONE     4

#define I2C_OPEND_TIMEOUT   250 /* ms */

static DWORD gTimeOutTick;

static void ResumeTrans(volatile I2CTRANS *pTrans)
{
    DWORD op;

    /* wait for bus not to be busy (finish any outstanding requests) */
    while (gpI2CRegs->I2C_STAT & I2C_STAT_BB);

    /* pTrans is the currently executing transfer and bus activity for it needs to be
       completed before another transfer can be done.  the target bus address is in
       gCurI2Caddr,gCurI2Csize */

    do {
        if (!pTrans->mOpCode[pTrans->mReserved1])
            break;
        /* POINT A --------------------------*/

        op = pTrans->mOpCode[pTrans->mReserved1];
        /* if the high priority came in between point a and point b, then pTrans->mReserved1
           will be invalid, but pTrans->mReserved2 will be set to I2CSTATE_OPDONE here, and 
           all that will happen is a break (default) */
        switch(pTrans->mReserved2)
        {
        case I2CSTATE_CLKSETUP:
            /* turn on the module functional clock */
            gpPRCMRegs->ulCM_FCLKEN1_CORE |= PRCM_FCLKEN1_CORE_EN_I2C1;
            gpPRCMRegs->ulCM_ICLKEN1_CORE |= PRCM_ICLKEN1_CORE_EN_I2C1;
            /* check current clock setting of i2c and make sure it matches pTrans */
            if (pTrans->mClk_HL_Divisor!=gCurClockDivisor)
            {
                if (!pTrans->mClk_HL_Divisor)
                    gCurClockDivisor = I2C_CLOCK_DEFAULT;
                else
                    gCurClockDivisor = pTrans->mClk_HL_Divisor;            
                gpI2CRegs->I2C_PSC = 0;
                gpI2CRegs->I2C_SCLL = (UINT16)gCurClockDivisor;
                gpI2CRegs->I2C_SCLH = (UINT16)gCurClockDivisor;
            }
            pTrans->mReserved2 = I2CSTATE_OPSTART;
            break;
        case I2CSTATE_OPSTART:
            gBytesLeft = pTrans->mTransLen[pTrans->mReserved1];
            gpData = &pTrans->mBuffer[pTrans->mBufferOffset[pTrans->mReserved1]];
            INTSOFF_I2CTRANS_STARTOP(((op & I2C_OPCODE_MASK)==I2C_OPCODE_WRITE)?TRUE:FALSE);
            pTrans->mReserved2 = I2CSTATE_OPUPDATE;
            break;
        case I2CSTATE_OPUPDATE:
            if ((op & I2C_OPCODE_MASK)==I2C_OPCODE_READ)
                INTSOFF_I2CTRANS_UPDATEREAD();
            else
                INTSOFF_I2CTRANS_UPDATEWRITE();
            if (!gBytesLeft)
            {
                pTrans->mReserved2 = I2CSTATE_OPEND;
                gTimeOutTick = OALGetTickCount();
            }
            break;
        case I2CSTATE_OPEND:
            if (!(gpI2CRegs->I2C_STAT & I2C_STAT_BB))
            {
                /* bus is no longer  busy (transaction completed) */
                pTrans->mReserved1++;
                if ((pTrans->mReserved1==I2CTRANS_MAX_STREAMED_TRANSACTIONS) || 
                    (!pTrans->mOpCode[pTrans->mReserved1]))
                {
                    /* end of this op.  turn off the module functional clocks */
                    gpPRCMRegs->ulCM_FCLKEN1_CORE &= ~PRCM_FCLKEN1_CORE_EN_I2C1;
                    gpPRCMRegs->ulCM_ICLKEN1_CORE &= ~PRCM_ICLKEN1_CORE_EN_I2C1;
                    pTrans->mReserved2 = I2CSTATE_OPDONE;
                }
                else
                    pTrans->mReserved2 = I2CSTATE_OPSTART;
            }
            else if ((OALGetTickCount()-gTimeOutTick)>I2C_OPEND_TIMEOUT)
            {
                /* abort */
                OALMSG(1, (TEXT("***ERR: I2C-BUS ABORT.\r\n")));
                while (pTrans->mOpCode[pTrans->mReserved1])
                {
                    pTrans->mReserved1++;
                    if (pTrans->mReserved1==I2CTRANS_MAX_STREAMED_TRANSACTIONS)
                        break;
                }
                /* aborted this op.  turn off the module functional clocks */
                gpPRCMRegs->ulCM_FCLKEN1_CORE &= ~PRCM_FCLKEN1_CORE_EN_I2C1;
                gpPRCMRegs->ulCM_ICLKEN1_CORE &= ~PRCM_ICLKEN1_CORE_EN_I2C1;
                pTrans->mReserved2 = I2CSTATE_OPDONE;
                pTrans->mErrorCode = (DWORD)-2;
            }
            break;
        default:
            break;
        }

    } while (pTrans->mOpCode[pTrans->mReserved1]);
}

static volatile I2CTRANS *gpLowTrans = NULL;
void KERNELI2C_NonPreemptibleSubmit(BOOL inISR, DWORD i2cAddr, DWORD addrSize, I2CTRANS *pTrans)
{
    BOOL saveInts;

    /* always validate the i2c transaction first */
    pTrans->mErrorCode = 0;
    if (!ValidateTransaction(pTrans))
        return;
    if (((addrSize!=7) && (addrSize!=10)) || (!i2cAddr))
    {
        if ((i2cAddr) && (!addrSize))
            addrSize = 7;
        else
        {
            /* invalid address or address size */
            pTrans->mErrorCode = KERNELI2C_ERR_INVALIDADDR;
            return;
        }
    }

    /* set current operation index and state for that operation */
    pTrans->mReserved1 = 0;
    pTrans->mReserved2 = I2CSTATE_CLKSETUP;

    if (!inISR)
    {
        /* latch in the lower priority transaction now */
        saveInts = INTERRUPTS_ENABLE(FALSE);
        gCurI2Caddr = i2cAddr;
        gCurI2CaddrSize = addrSize;
        gpLowTrans = pTrans;
        INTERRUPTS_ENABLE(saveInts);
        pTrans = NULL;
    }
    else
    {
        OALMSG(0/*1*/, (TEXT("ki2c-NONPREEMPT()\r\n")));
    }

    /* work on gpLowTrans until it is done - if we're executing in an isr then
       that transaction is currently in progress and we need to finish it first */
    if (gpLowTrans)
    {
        ResumeTrans(gpLowTrans);
        gpLowTrans = NULL;
    }

    /* if pTrans is not NULL, do that one now - its the isr transaction */
    if (pTrans)
    {
        gCurI2Caddr = i2cAddr;
        gCurI2CaddrSize = addrSize;
        ResumeTrans(pTrans);
        gCurI2Caddr = 0;
        gCurI2CaddrSize = 0;
    }
}

void KERNELI2C_PreemptibleSubmit(DWORD i2cAddr, DWORD addrSize, I2CTRANS *pTrans)
{
    /* this is for KERNEL calls that need to use I2C while they are preemptible */
    /* DRIVERS should use the I2C driver to make calls */
#ifndef BOOTLOADER
    EnterCriticalSection(&gSec);
#endif
    KERNELI2C_NonPreemptibleSubmit(FALSE, i2cAddr, addrSize, pTrans);
#ifndef BOOTLOADER
    LeaveCriticalSection(&gSec);
#endif
}

#pragma optimize("", on)			// debug


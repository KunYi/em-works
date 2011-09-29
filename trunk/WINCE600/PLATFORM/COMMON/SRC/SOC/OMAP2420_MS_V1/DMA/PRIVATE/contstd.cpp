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
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <pm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap2420.h>
#include "dmasys.h"

/* this much of all controllers is common:
typedef volatile struct {
   UINT32 DMA4_REVISION;              //offset 0x0, Revision code
   UINT32 ulRESERVED_0x04;
   UINT32 DMA4_IRQSTATUS_L0; //offset 0x08,intr status over line L0
   UINT32 DMA4_IRQSTATUS_L1; //offset 0x0C,intr status over line L1
   UINT32 DMA4_IRQSTATUS_L2; //offset 0x10,intr status over line L2
   UINT32 DMA4_IRQSTATUS_L3; //offset 0x14,intr status over line L3
   UINT32 DMA4_IRQENABLE_L0; //offset 0x18,intr enable over line L0
   UINT32 DMA4_IRQENABLE_L1; //offset 0x1C,intr enable over line L1
   UINT32 DMA4_IRQENABLE_L2; //offset 0x20,intr enable over line L2
   UINT32 DMA4_IRQENABLE_L3; //offset 0x24,intr enable over line L3
   UINT32 DMA4_SYSSTATUS;    //offset 0x28,module status
   UINT32 DMA4_OCP_SYSCONFIG;//offset 0x2C,OCP i/f params control
   UINT32 ulRESERVED_0x30[13];
   UINT32 DMA4_CAPS_0;       //offset 0x64,DMA capabilities reg 0 LSW
   UINT32 ulRESERVED_0x68;
   UINT32 DMA4_CAPS_2;       //offset 0x6C,DMA capabilities reg 2
   UINT32 DMA4_CAPS_3;       //offset 0x70,DMA capabilities reg 3
   UINT32 DMA4_CAPS_4;       //offset 0x74,DMA capabilities reg 4
   UINT32 DMA4_GCR;          //offset 0x78,chnl DMA global register
   UINT32 ulRESERVED_0x7C;
    -- channel controls go here --
}
OMAP2420_SDMA_REGS;
*/

StandardDMAController::StandardDMAController(uint aNumChannels)
{
    mNumChannels = aNumChannels;
    uint i;
    for(i=0;i<aNumChannels;i++)
    {
        mCurFreeChannelMask |= (1<<i);
    }
}

uint StandardDMAController::AllocChannels(uint num, uint &aReqMask)
{
    /* count free bits in mCurFreeChannelMask, up to required # of channels */
    uint i;
    uint avail = 0;
    for(i=0;i<32;i++)
    {
        if (mCurFreeChannelMask & (1<<i))
        {
            avail++;
            if (avail==num)
                break;
        }
    }
    if (avail!=num)
        return DMADRVERR_RESOURCES;

    uint ret = 0;

    /* take required channels if they are desired and available */
    if (aReqMask)
    {
        if ((aReqMask&mCurFreeChannelMask)!=aReqMask)
            return DMADRVERR_RESOURCES; // at least one required channel is in use 
        ret = aReqMask;
        mCurFreeChannelMask &= ~aReqMask;
        for(i=0;i<32;i++)
        {
            if (aReqMask & (1<<i))
                num--;
        }
    }

    if (num)
    {
        /* some non-specific channels still needed */
        uint chk = 0;
        do {
            while (!(mCurFreeChannelMask & (1<<chk)))
                chk++;
            ret |= (1<<chk);
            mCurFreeChannelMask &= ~(1<<chk);
        } while (--num);
    }

    /* return allocated channel mask */
    aReqMask = ret;

    return 0;
}

uint StandardDMAController::FreeChannels(uint aChanMask)
{
    uint bitPos = 0;
    while (aChanMask)
    {
        if (aChanMask&1)
            mCurFreeChannelMask |= (1<<bitPos);
        aChanMask>>=1;
        bitPos++;
    }

    return 0;
}

bool StandardDMAController::GetRevision(volatile void *pBase, uint &major, uint &minor)
{
    if (!pBase)
        return false;

    /* pBase is mapped uncached address of standard dma controller subtype */
    volatile OMAP2420_SDMA_REGS	*pCont = (volatile OMAP2420_SDMA_REGS *)pBase;

    uint rev = pCont->DMA4_REVISION;

    if (!rev)
        return false;

    major = (rev&0xF0)>>4;
    minor = (rev&0x0F);
    return true;
}

void StandardDMAController::Reset(volatile void *pBase)
{
    /* pBase is mapped uncached address of standard dma controller subtype */
    volatile OMAP2420_SDMA_REGS	*pCont = (volatile OMAP2420_SDMA_REGS *)pBase;

    DEBUGMSG(ZONE_FUNCTION, (L"+StandardDMAController::Reset()\r\n"));

    /* initiate software reset of controller */
    pCont->DMA4_OCP_SYSCONFIG |= 0x2;
    /* wait for reset to complete */
    while (!(pCont->DMA4_SYSSTATUS&1))
        Sleep(100);

    uint i;
    for(i=0;i<mNumChannels;i++)
    {
        mCurFreeChannelMask |= (1<<i);
    }

    DEBUGMSG(ZONE_FUNCTION, (L"-StandardDMAController::Reset()\r\n"));
}

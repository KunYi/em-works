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
#ifndef __DMASYS_H
#define __DMASYS_H

#ifndef __cplusplus
#error C++ required for DMASYS
#endif

#include "dmadrv.h"
#include "omap2420_dma.h"

#ifdef DEBUG

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INIT           DEBUGZONE(3)

#endif

class DMAController;
class DMAUser;

class DMASystem
{
public:
    static DMASystem * Create(LPCTSTR szActiveKey);
    ~DMASystem(void);

    uint Init(void);
    uint Deinit(void);

    uint Open(DWORD dwAccessCode, DWORD dwShareMode, DWORD *retOpenContext);
    uint Close(DWORD dwOpenContext);

    uint Command(DWORD openContext, DWORD code, 
                 BYTE *pInBuffer, DWORD inSize, 
                 BYTE *pOutBuffer,
                 DWORD outSize, 
                 DWORD *pOutSize);

    void Lock(void) { EnterCriticalSection(&mSec); }
    void Unlock(void) { LeaveCriticalSection(&mSec); }

private:
    DMASystem(HKEY hActiveRegKey); 

    uint Ioctl_GetSize(BYTE *pOutBuffer, DWORD outSize, DWORD *pOutSize);
    uint Ioctl_GetEnum(BYTE *pOutBuffer, DWORD outSize, DWORD *pOutSize);
    uint Ioctl_Open(DWORD dwOpenContext, DMAIOCTL &parmIn, DMAIOCTL &parmOut);
    uint Ioctl_Acq(DMAIOCTL &parmIn, DMAIOCTL &parmOut);
    uint Ioctl_Free(DMAIOCTL &parmIn, DMAIOCTL &parmOut);
    uint Ioctl_Set(DMAIOCTL &parmIn, DMAIOCTL &parmOut);
    uint Ioctl_Get(DMAIOCTL &parmIn, DMAIOCTL &parmOut);
    uint Ioctl_Close(DMAIOCTL &parmIn, DMAIOCTL &parmOut);

    uint                mOpenContextUserId;
    DMAUser            *mpUserList;
    HKEY                mhRegKey;
    CRITICAL_SECTION    mSec;
};

class DMAUser
{
public:
    DMAUser(void)
    {
        mUserKey = 0;
        mChanMask = 0;
        mpCont = NULL;
        mpNext = NULL;
    }

    uint            mUserKey;
    uint            mChanMask;
    DMAController  *mpCont;
    DMAUser        *mpNext;
};

class DMAController
{
public:
    DMAController(void) {}
    virtual ~DMAController(void) {}

    virtual uint Init(void) = 0;
    virtual void ShutDown(void) = 0;

    virtual uint Set(uint aChanMask, DMA_CONT_PROPERTY aProp, uint aValue) = 0;
    virtual uint Get(uint aChanMask, DMA_CONT_PROPERTY aProp, uint *apRetValue) = 0;

    virtual uint AllocChannels(uint num, uint &aMaskIO) = 0;
    virtual uint FreeChannels(uint aChanMask) = 0;
};

class StandardDMAController : public DMAController
{
public:
    StandardDMAController(uint aInitFreeChanMask);

    uint AllocChannels(uint num, uint &aReqMask);
    uint FreeChannels(uint aChanMask);

protected:
    bool GetRevision(volatile void *pBase, uint &major, uint &minor);
    void Reset(volatile void *pBase);

private:
    uint mNumChannels;
    uint mCurFreeChannelMask;
};

class SysStandardDMAController : public StandardDMAController
{
public:
    SysStandardDMAController(void);
    ~SysStandardDMAController(void);

    /* DMAController inherited */
    uint Init(void);
    void ShutDown(void);
    uint Set(uint aChanMask, DMA_CONT_PROPERTY aProp, uint aValue);
    uint Get(uint aChanMask, DMA_CONT_PROPERTY aProp, uint *apRetValue);

private:
    volatile OMAP2420_SDMA_REGS	*mpDMARegs;	
};

class DSPStandardDMAController : public StandardDMAController
{
public:
    DSPStandardDMAController(void) : StandardDMAController(0) {}
    /* DMAController inherited */
    uint Init(void) { return 0; }
    void ShutDown(void) {}
    uint Set(uint aChanMask, DMA_CONT_PROPERTY aProp, uint aValue) { return 0; }
    uint Get(uint aChanMask, DMA_CONT_PROPERTY aProp, uint *apRetValue) { return 0; }
};

class CamStandardDMAController : public StandardDMAController
{
public:
    CamStandardDMAController(void);
    ~CamStandardDMAController(void);

    /* DMAController inherited */
    uint Init(void);
    void ShutDown(void);
    uint Set(uint aChanMask, DMA_CONT_PROPERTY aProp, uint aValue);
    uint Get(uint aChanMask, DMA_CONT_PROPERTY aProp, uint *apRetValue);

private:
    volatile OMAP2420_CAMDMA_REGS	*mpDMARegs;	
};

#endif // __DMASYS_H

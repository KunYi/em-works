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

SysStandardDMAController *gpSysDMACont = NULL;
DSPStandardDMAController *gpDSPDMACont = NULL;
CamStandardDMAController *gpCamDMACont = NULL;

enum DMAControllerType
{
    DCT_StandardSystem,
    DCT_StandardDSP,
    DCT_StandardCamera,
};
struct DMAControllerIdMapping
{
    DMACONTROLLER   mEntry;
    uint            mControllerType;
    void          **mppCont;
};
static DMAControllerIdMapping sgControllerIdMap[] = 
{
    {
        { 1, "System", },
        DCT_StandardSystem,
        (void **)&gpSysDMACont
    },
    {
        { 2, "DSP" },
        DCT_StandardDSP,
        (void **)&gpDSPDMACont
    },
    {
        { 3, "Camera" },
        DCT_StandardCamera,
        (void **)&gpCamDMACont
    }
};

DMASystem * DMASystem::Create(LPCTSTR szActiveKey)
{
    HKEY hActiveKey;
    DWORD dwRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szActiveKey,0,0,&hActiveKey);
    if (FAILED(dwRes))
        return NULL;
    return new DMASystem(hActiveKey);
}

DMASystem::DMASystem(HKEY hActiveRegKey)
{
    InitializeCriticalSection(&mSec);
    mOpenContextUserId = 1;
    mhRegKey = hActiveRegKey;
    /* initialize defaults here */
	mpUserList = NULL;
}

DMASystem::~DMASystem(void)
{
    RegCloseKey(mhRegKey);
    mhRegKey = NULL;
    DeleteCriticalSection(&mSec);
}

uint DMASystem::Init(void)
{
//    RETAILMSG(1, (TEXT("DMASystem::Init()\r\n")));

    uint err = 0;

    /* read stuff from registry here at mhRegKey  */

    /* do init here based on configured parameters (if any) */
    uint i;
    uint numEntries = sizeof(sgControllerIdMap)/sizeof(DMAControllerIdMapping);
    for(i=0;i<numEntries;i++)
    {
        switch (sgControllerIdMap[i].mControllerType)
        {
        case DCT_StandardSystem:
            *sgControllerIdMap[i].mppCont = new SysStandardDMAController;
            break;
        case DCT_StandardDSP:
            *sgControllerIdMap[i].mppCont = new DSPStandardDMAController;
            break;
        case DCT_StandardCamera:
            *sgControllerIdMap[i].mppCont = new CamStandardDMAController;
            break;
        default:
            *sgControllerIdMap[i].mppCont = NULL;
        }
        if (!(*sgControllerIdMap[i].mppCont))
            break;
        err = ((DMAController *)(*sgControllerIdMap[i].mppCont))->Init(); 
        if (err)
        {
            DEBUGMSG(ZONE_ERROR, (L"DMASystem::Init(): "
                L"Controller id %d init returned error code 0x%08X\r\n", sgControllerIdMap[i].mEntry.mSysId, err
            ));
            
            delete (DMAController *)(*sgControllerIdMap[i].mppCont);
            *sgControllerIdMap[i].mppCont = NULL;
            
            break;
        }
    }
    if (i!=numEntries)
    {
        /* failed to make all controllers.  clean up */
        while (i>0)
        {
            i--;
            ((DMAController *)(*sgControllerIdMap[i].mppCont))->ShutDown();
            delete (DMAController *)(*sgControllerIdMap[i].mppCont);
            *sgControllerIdMap[i].mppCont = NULL;
        }
        return DMADRVERR_INIT_FAILURE;
    }

    return err;
}

uint DMASystem::Deinit(void)
{
//    RETAILMSG(1, (TEXT("DMASystem::Deinit()\r\n")));

    /* do deinit here */
    while (mpUserList)
    {
        Close(mpUserList->mUserKey);
        /* mpUserList will have changed here inside Close() call */
    }

    uint i;
    uint numEntries = sizeof(sgControllerIdMap)/sizeof(DMAControllerIdMapping);
    for(i=0;i<numEntries;i++)
    {
        if (*sgControllerIdMap[i].mppCont)
        {
            DMAController *pCont = (DMAController *)(*sgControllerIdMap[i].mppCont);
            pCont->ShutDown();
            delete pCont;
            *sgControllerIdMap[i].mppCont = NULL;
        }
    }

    return 0;
}

uint DMASystem::Open(DWORD dwAccessCode, DWORD dwShareMode, DWORD *retOpenContext)
{
    /* do open here */
//    RETAILMSG(1, (TEXT("DMASystem::Open()\r\n")));
    *retOpenContext = mOpenContextUserId++;
    return 0;
}

uint DMASystem::Close(DWORD dwOpenContext)
{
    if (!dwOpenContext)
        return DMADRVERR_NOTOPEN;

    /* do close here */
//    RETAILMSG(1, (TEXT("DMASystem::Close(%d)\r\n"),dwOpenContext));

    DMAUser *pChk = mpUserList;
    DMAUser *pPrev = NULL;

    uint err = 0;
    while (pChk)
    {
        if (pChk->mUserKey==dwOpenContext)
        {
            /* remove pChk from dmauserlist */
            if (pPrev)
                pPrev->mpNext = pChk->mpNext;
            else
                mpUserList = pChk->mpNext;
            DMAUser *pKill = pChk;
            pChk = pChk->mpNext;
            
            /* now release the channels it was using */
            uint newErr = pKill->mpCont->FreeChannels(pKill->mChanMask);
            if (!err)
                err = newErr;

            delete pKill;
        }
        else
        {
            pPrev = pChk;
            pChk = pChk->mpNext;
        }
    }

    return err;
}

uint DMASystem::Command(DWORD dwOpenContext, DWORD code, 
                        BYTE *pInBuffer, DWORD inSize, 
                        BYTE *pOutBuffer,
                        DWORD outSize, 
                        DWORD *pOutSize)
{
    if (!dwOpenContext)
        return DMADRVERR_NOTOPEN;

    /* do command here */
//    RETAILMSG(1, (TEXT("DMASystem::Command(%d)\r\n"),code));

    bool getUser = false;
    switch (code)
    {
    case DMA_IOCTL_ENUM_GETSIZE:
        break;
    case DMA_IOCTL_ENUM_GETDATA:
        break;
    case DMA_IOCTL_ACQ:         
    case DMA_IOCTL_FREE:        
    case DMA_IOCTL_SET:         
    case DMA_IOCTL_GET:         
    case DMA_IOCTL_CLOSE:       
        getUser = true;
        /* fall thru */
    case DMA_IOCTL_OPEN:        
        if ((inSize<sizeof(DMAIOCTL)) ||
            (outSize<sizeof(DMAIOCTL)))
            return DMADRVERR_PARAM_SIZE_INVALID;
        break;
    default:
        return DMADRVERR_UNKNOWNIOCTL;
    };

    DMAUser *pUser = NULL;
    if (getUser)
    {
        /* find DMAUser for this command */
        pUser = mpUserList;
        while ((pUser) && (((DMAIOCTL *)pInBuffer)->mParam[0]!=(DWORD)pUser))
            pUser = pUser->mpNext;
        if (!pUser)
            return DMADRVERR_INVALIDHANDLE;
    }

    /* io validated.  do action */
    uint err = 0;
    switch (code)
    {
    case DMA_IOCTL_ENUM_GETSIZE:
        return Ioctl_GetSize(pOutBuffer,outSize,pOutSize);
    case DMA_IOCTL_ENUM_GETDATA:
        return Ioctl_GetEnum(pOutBuffer,outSize,pOutSize);
    case DMA_IOCTL_OPEN:        
        *pOutSize = sizeof(DMAIOCTL);
        ((DMAIOCTL *)pOutBuffer)->mParam[3] = Ioctl_Open(dwOpenContext, *(DMAIOCTL *)pInBuffer,*(DMAIOCTL *)pOutBuffer);
        break;
    case DMA_IOCTL_ACQ:         
        *pOutSize = sizeof(DMAIOCTL);
        ((DMAIOCTL *)pOutBuffer)->mParam[3] = Ioctl_Acq(*(DMAIOCTL *)pInBuffer,*(DMAIOCTL *)pOutBuffer);
        break;
    case DMA_IOCTL_FREE:        
        *pOutSize = sizeof(DMAIOCTL);
        ((DMAIOCTL *)pOutBuffer)->mParam[3] = Ioctl_Free(*(DMAIOCTL *)pInBuffer,*(DMAIOCTL *)pOutBuffer);
        break;
    case DMA_IOCTL_SET:         
        *pOutSize = sizeof(DMAIOCTL);
        ((DMAIOCTL *)pOutBuffer)->mParam[3] = Ioctl_Set(*(DMAIOCTL *)pInBuffer,*(DMAIOCTL *)pOutBuffer);
        break;
    case DMA_IOCTL_GET:         
        *pOutSize = sizeof(DMAIOCTL);
        ((DMAIOCTL *)pOutBuffer)->mParam[3] = Ioctl_Get(*(DMAIOCTL *)pInBuffer,*(DMAIOCTL *)pOutBuffer);
        break;
    case DMA_IOCTL_CLOSE:       
        *pOutSize = sizeof(DMAIOCTL);
        ((DMAIOCTL *)pOutBuffer)->mParam[3] = Ioctl_Close(*(DMAIOCTL *)pInBuffer,*(DMAIOCTL *)pOutBuffer);
        break;
    };

    return 0;
}

uint DMASystem::Ioctl_GetSize(BYTE *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
    if (outSize<sizeof(DWORD))
        return DMADRVERR_PARAM_SIZE_INVALID;
    if (outSize!=sizeof(DWORD))
        return DMADRVERR_PARAM_INVALID;
    uint numEntries = sizeof(sgControllerIdMap)/sizeof(DMAControllerIdMapping);
    (*(DWORD *)pOutBuffer) = numEntries*sizeof(DMACONTROLLER);
    *pOutSize = sizeof(DWORD);
    return 0;
}

uint DMASystem::Ioctl_GetEnum(BYTE *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
    uint numEntries = sizeof(sgControllerIdMap)/sizeof(DMAControllerIdMapping);
    if (outSize<numEntries*sizeof(DMACONTROLLER))
        return DMADRVERR_PARAM_SIZE_INVALID;
    DMACONTROLLER *pTarg = (DMACONTROLLER *)pOutBuffer;
    uint i;
    for(i=0;i<numEntries;i++)
    {
        *pTarg = sgControllerIdMap[i].mEntry;
        pTarg++;
    }
    *pOutSize = numEntries*sizeof(DMACONTROLLER);
    return 0;
}

uint DMASystem::Ioctl_Open(DWORD dwOpenContext, DMAIOCTL &parmIn, DMAIOCTL &parmOut)
{
    /* 
    input:  0 - sysid of controller to open
            1 - d/c
            2 - d/c
            3 - d/c
    output: 0 - pUser pointer for this open or 0 if error
            1 - 0
            2 - 0
            3 - 0
    */
    parmOut.mParam[0] = 0;
    parmOut.mParam[1] = 0;
    parmOut.mParam[2] = 0;
    parmOut.mParam[3] = 0;

    DMAUser *pNewUser = new DMAUser;
    if (!pNewUser)
        return DMADRVERR_RESOURCES;

    //parmIn.mParam[0] is id of controller to open
    uint i;
    uint numEntries = sizeof(sgControllerIdMap)/sizeof(DMAControllerIdMapping);

    for(i=0;i<numEntries;i++)
    {
        if (sgControllerIdMap[i].mEntry.mSysId==parmIn.mParam[0])
            break;
    }
    if (i==numEntries)
    {
        delete pNewUser;
        return DMADRVERR_UNKNOWNCONTROLLER;
    }

    DMAController *pCont = (DMAController *)(*sgControllerIdMap[i].mppCont);

    /* look for existing dwOpenContext/pCont mapping for this user */
    DMAUser *pUser = mpUserList;
    while (pUser)
    {
        if ((pUser->mUserKey==dwOpenContext) &&
            (pUser->mpCont==pCont))
            break;
        pUser = pUser->mpNext;
    }
    if (pUser)
    {
        delete pNewUser;
        return DMADRVERR_ALREADYOPEN;
    }

    /* ok.  we have a valid new open context for the controller for this user */
    pNewUser->mUserKey = dwOpenContext;
    pNewUser->mpCont = pCont;
    pNewUser->mpNext = mpUserList;
    mpUserList = pNewUser;

    parmOut.mParam[0] = (uint)pNewUser;
    parmOut.mParam[1] = 0;
    parmOut.mParam[2] = 0;
    parmOut.mParam[3] = 0;

    return 0;
}

uint DMASystem::Ioctl_Acq(DMAIOCTL &parmIn, DMAIOCTL &parmOut)
{
    /* 
    input:  0 - already-validated pUser pointer
            1 - # of channels to alloc
            2 - mask of required channels
            3 - d/c
    output: 0 - mask of allocated channels or 0 if error
            1 - 0
            2 - 0
            3 - result code on error
    */
    DMAUser *pUser = (DMAUser *)parmIn.mParam[0];

    uint maskIO = parmIn.mParam[2];
    parmOut.mParam[0] = 0;
    parmOut.mParam[1] = 0;
    parmOut.mParam[2] = 0;
    parmOut.mParam[3] = pUser->mpCont->AllocChannels(parmIn.mParam[1],maskIO);
    if (!parmOut.mParam[3])
    {
        parmOut.mParam[0] = maskIO;
        pUser->mChanMask |= maskIO;
    }
    return parmOut.mParam[3];
}

uint DMASystem::Ioctl_Free(DMAIOCTL &parmIn, DMAIOCTL &parmOut)
{
    /* 
    input:  0 - already-validated pUser pointer
            1 - mask of allocated channels to free
            2 - d/c
            3 - d/c
    output: 0 - 0
            1 - 0
            2 - 0
            3 - result code of operation
    */
    DMAUser *pUser = (DMAUser *)parmIn.mParam[0];

    parmIn.mParam[1] &= pUser->mChanMask;
    if (!parmIn.mParam[1])
        return DMADRVERR_PARAM_INVALID;

    parmOut.mParam[0] = 0;
    parmOut.mParam[1] = 0;
    parmOut.mParam[2] = 0;
    parmOut.mParam[3] = pUser->mpCont->FreeChannels(parmIn.mParam[1]);

    return parmOut.mParam[3];
}

uint DMASystem::Ioctl_Set(DMAIOCTL &parmIn, DMAIOCTL &parmOut)
{
    /* 
    input:  0 - already-validated pUser pointer
            1 - id of property to set
            2 - value to set for property
            3 - d/c
    output: 0 - 0
            1 - 0
            2 - 0
            3 - result code of operation
    */
    DMAUser *pUser = (DMAUser *)parmIn.mParam[0];

    parmOut.mParam[0] = 0;
    parmOut.mParam[1] = 0;
    parmOut.mParam[2] = 0;
    parmOut.mParam[3] = pUser->mpCont->Set(pUser->mChanMask,(DMA_CONT_PROPERTY)parmIn.mParam[1],parmIn.mParam[2]);

    return parmOut.mParam[3];
}

uint DMASystem::Ioctl_Get(DMAIOCTL &parmIn, DMAIOCTL &parmOut)
{
    /* 
    input:  0 - already-validated pUser pointer
            1 - id of property to get
            2 - d/c
            3 - d/c
    output: 0 - 0
            1 - 0
            2 - value retrieved for property
            3 - result code of operation
    */
    DMAUser *pUser = (DMAUser *)parmIn.mParam[0];

    parmOut.mParam[0] = 0;
    parmOut.mParam[1] = 0;
    parmOut.mParam[2] = 0;
    parmOut.mParam[3] = pUser->mpCont->Get(pUser->mChanMask,(DMA_CONT_PROPERTY)parmIn.mParam[1],&parmOut.mParam[2]);

    return parmOut.mParam[3];
}

uint DMASystem::Ioctl_Close(DMAIOCTL &parmIn, DMAIOCTL &parmOut)
{
    DMAUser *pUser = (DMAUser *)parmIn.mParam[0];
    DMAUser *pPrev = NULL;
    DMAUser *pChk = mpUserList;

    parmOut.mParam[0] = 0;
    parmOut.mParam[1] = 0;
    parmOut.mParam[2] = 0;
    parmOut.mParam[3] = 0;

    while ((pChk) && (pChk!=pUser))
    {
        pPrev = pChk;
        pChk = pChk->mpNext;
    }
    if (!pChk)
        parmOut.mParam[3] = DMADRVERR_INVALIDHANDLE;
    else
    {
        if (!pPrev)
            mpUserList = pUser->mpNext;
        else
            pPrev->mpNext = pUser->mpNext;

        pUser->mpCont->FreeChannels(pUser->mChanMask);

        delete pUser;
    }

    return parmOut.mParam[3];
}

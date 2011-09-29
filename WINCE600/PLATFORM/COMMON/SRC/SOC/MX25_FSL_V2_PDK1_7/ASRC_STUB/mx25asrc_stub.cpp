//------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File:  mx25_asrc_stub.cpp
//
// Provides stub routines for the ASRC, as it is not available on MX25 but 
// still needed for linking the ESAI driver
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include "csp.h"
#include "asrc_base.h"

BOOL AsrcInit(UINT32  *ppContext)
{
    UNREFERENCED_PARAMETER(ppContext);

    return FALSE;
}

BOOL AsrcDeinit(void)
{
    return FALSE;
}

ASRC_PAIR_INDEX AsrcOpenPair(ASRC_PAIR_INDEX pairIndex,
                UINT32 inputChnNum,UINT32 outputChnNum, 
                HANDLE hInputEvent, HANDLE hOutputEvent,
                ASRC_TRANS_MODE transMode)
{
    UNREFERENCED_PARAMETER(pairIndex);
    UNREFERENCED_PARAMETER(inputChnNum);
    UNREFERENCED_PARAMETER(outputChnNum);
    UNREFERENCED_PARAMETER(hInputEvent);
    UNREFERENCED_PARAMETER(hOutputEvent);
    UNREFERENCED_PARAMETER(transMode);

    return ASRC_PAIR_NONE;
}

BOOL AsrcStartConv(ASRC_PAIR_INDEX indexPair,BOOL bEnableChannel,
    BOOL bEanbleInput,BOOL bEnableOutput)
{
    UNREFERENCED_PARAMETER(indexPair);
    UNREFERENCED_PARAMETER(bEnableChannel);
    UNREFERENCED_PARAMETER(bEanbleInput);
    UNREFERENCED_PARAMETER(bEnableOutput);

    return FALSE;
}

BOOL AsrcStopConv(ASRC_PAIR_INDEX indexPair,BOOL bStopChannel,
    BOOL bStopInput,BOOL bStopOutput)
{
    UNREFERENCED_PARAMETER(indexPair);
    UNREFERENCED_PARAMETER(bStopChannel);
    UNREFERENCED_PARAMETER(bStopInput);
    UNREFERENCED_PARAMETER(bStopOutput);

    return FALSE;
}

UINT32 AsrcGetInputBufStatus(ASRC_PAIR_INDEX indexPair)
{
    UNREFERENCED_PARAMETER(indexPair);

    return 0;
}

void AsrcClosePair(ASRC_PAIR_INDEX indexPair)
{
    UNREFERENCED_PARAMETER(indexPair);
}

BOOL AsrcSetP2PDeviceWML(ASRC_PAIR_INDEX indexPair,UINT32 deviceWML)
{
    UNREFERENCED_PARAMETER(indexPair);
    UNREFERENCED_PARAMETER(deviceWML);

    return FALSE;
}

BOOL AsrcConfigPair(ASRC_PAIR_INDEX indexPair,PASRC_CONFIG_PARAM pParam)
{
    UNREFERENCED_PARAMETER(indexPair);
    UNREFERENCED_PARAMETER(pParam);

    return FALSE;
}

BOOL AsrcSetOutputBuf(ASRC_PAIR_INDEX indexPair, PHYSICAL_ADDRESS phyAddr,
    UINT32 numBytes, UINT32 index,PBYTE pVirtBuf )
{
    UNREFERENCED_PARAMETER(indexPair);
    UNREFERENCED_PARAMETER(phyAddr);
    UNREFERENCED_PARAMETER(numBytes);
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(pVirtBuf);

    return FALSE;
}

BOOL AsrcSetInputBuf(ASRC_PAIR_INDEX indexPair, PHYSICAL_ADDRESS phyAddr,
    UINT32 numBytes, UINT32 index,PBYTE pVirtBuf )
{
    UNREFERENCED_PARAMETER(indexPair);
    UNREFERENCED_PARAMETER(phyAddr);
    UNREFERENCED_PARAMETER(numBytes);
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(pVirtBuf);

    return FALSE;
}

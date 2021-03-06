//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//--------------------------------------------------------------------------------
//
// Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//--------------------------------------------------------------------------------

#ifndef __DEVCTXT_H
#define __DEVCTXT_H

#if __cplusplus
extern "C" {
#endif

#include <mmreg.h>

#define SECONDARYGAINCLASSMAX 4

// number of classes affected by the device gain
#define SECONDARYDEVICEGAINCLASSMAX 2


class DeviceContext
{
public:
    DeviceContext()
    {
        InitializeListHead(&m_StreamList);
        m_dwGain = 0xFFFF;
        m_dwDefaultStreamGain = 0xFFFF;
        for (int i=0;i<SECONDARYGAINCLASSMAX;i++)
        {
            m_dwSecondaryGainLimit[i]=0xFFFF;
        }
    }

    virtual BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    PBYTE TransferBuffer(PBYTE pBuffer, PBYTE pBufferEnd, DWORD *pNumStreams);

    void NewStream(StreamContext *pStreamContext);
    void DeleteStream(StreamContext *pStreamContext);

    DWORD GetGain()
    {
        return m_dwGain;
    }

    DWORD SetGain(DWORD dwGain)
    {
        m_dwGain = dwGain;
        RecalcAllGains();
        return MMSYSERR_NOERROR;
    }

    DWORD GetDefaultStreamGain()
    {
        return m_dwDefaultStreamGain;
    }

    DWORD SetDefaultStreamGain(DWORD dwGain)
    {
        m_dwDefaultStreamGain = dwGain;
        return MMSYSERR_NOERROR;
    }

    DWORD GetSecondaryGainLimit(DWORD GainClass)
    {
        return m_dwSecondaryGainLimit[GainClass];
    }

    DWORD SetSecondaryGainLimit(DWORD GainClass, DWORD Limit)
    {
        if (GainClass>=SECONDARYGAINCLASSMAX)
        {
            return MMSYSERR_ERROR;
        }
        m_dwSecondaryGainLimit[GainClass]=Limit;
        RecalcAllGains();
        return MMSYSERR_NOERROR;
    }

    void RecalcAllGains();

    DWORD OpenStream(LPWAVEOPENDESC lpWOD, DWORD dwFlags, StreamContext **ppStreamContext);
    virtual DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual DWORD GetDevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual void StreamReadyToRender(StreamContext *pStreamContext)=0;

    virtual StreamContext *CreateStream(LPWAVEOPENDESC lpWOD)=0;

protected:
    LIST_ENTRY  m_StreamList;         // List of streams rendering to/from this device
    DWORD       m_dwGain;
    DWORD       m_dwDefaultStreamGain;
    DWORD m_dwSecondaryGainLimit[SECONDARYGAINCLASSMAX];
};

class InputDeviceContext : public DeviceContext
{
public:
    BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    StreamContext *CreateStream(LPWAVEOPENDESC lpWOD);
    DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize);
    DWORD GetDevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);
};

class OutputDeviceContext : public DeviceContext
{
public:
    BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    StreamContext *CreateStream(LPWAVEOPENDESC lpWOD);
    DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize);
    DWORD GetDevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);
};

#ifdef __cplusplus
}
#endif

#endif // __DEVCTXT_H
